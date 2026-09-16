//
// Copyright (C) 2026 Brian Nahuel Götte
// SPDX-License-Identifier: Apache-2.0
//

#define LOG_TAG "ExynosCameraShim_HAL1"

#include <hardware/hardware.h>
#include <hardware/camera.h>

#include <dlfcn.h>
#include <utils/Log.h>
#include <mutex>
#include <stdlib.h>
#include <string.h>
#include <string>

static camera_module_t* gVendorModule = nullptr;
static std::mutex gCameraMutex;

static bool gCamera0Active = false;
static std::string gCachedCam1Params = "";

static int load_vendor_module() {
    if (gVendorModule) return 0;
    void* handle = dlopen("/vendor/lib/hw/camera.exynos5_vendor.so", RTLD_NOW);
    if (!handle) {
        ALOGE("Error loading Samsung camera.exynos5_vendor.so HAL: %s", dlerror());
        return -EINVAL;
    }
    gVendorModule = (camera_module_t*) dlsym(handle, HAL_MODULE_INFO_SYM_AS_STR);
    return gVendorModule ? 0 : -EINVAL;
}

// Camera 1 mock used when Camera 0 owns the ISP.
// CameraService may open both cameras simultaneously to query capabilities.
static char* mock_get_parameters(struct camera_device * /*dev*/) {
    if (!gCachedCam1Params.empty()) {
        return strdup(gCachedCam1Params.c_str());
    }
    // Contingency string if camera1 params are not cached
    return strdup("preview-size-values=1280x720,960x720,640x480;picture-size-values=2576x1932,1920x1080;jpeg-thumbnail-quality=90;");
}

static void mock_put_parameters(struct camera_device * /*dev*/, char *params) {
    if (params) free(params);
}

static int mock_device_close(struct hw_device_t* dev) {
    if (dev) {
        camera_device_t* cam_dev = (camera_device_t*)dev;
        if (cam_dev->ops) delete cam_dev->ops;
        delete cam_dev;
    }
    return 0;
}

static camera_device_t* create_mock_camera1_device() {
    camera_device_t* dev = new camera_device_t();
    memset(dev, 0, sizeof(camera_device_t));
    
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = CAMERA_DEVICE_API_VERSION_1_0;
    dev->common.close = mock_device_close;
    
    camera_device_ops_t* ops = new camera_device_ops_t();
    memset(ops, 0, sizeof(camera_device_ops_t));
    ops->get_parameters = mock_get_parameters;
    ops->put_parameters = mock_put_parameters;
    
    dev->ops = ops;
    return dev;
}

// Intercept close camera 0
static int (*gRealCam0Close)(struct hw_device_t* dev) = nullptr;
static int shim_cam0_close(struct hw_device_t* dev) {
    std::lock_guard<std::mutex> lock(gCameraMutex);
    gCamera0Active = false;
    return gRealCam0Close(dev);
}

static int shim_get_camera_info(int camera_id, struct camera_info *info) {
    if (load_vendor_module() != 0) return -EINVAL;
    
    memset(info, 0, sizeof(struct camera_info));
    int ret = gVendorModule->get_camera_info(camera_id, info);
    
    if (ret == 0) {
        // Force Both cameras to HAL 1.0
        info->device_version = CAMERA_DEVICE_API_VERSION_1_0;
        
        // Prevent Android from planning arbitrary evictions
        info->resource_cost = 50;
        info->conflicting_devices = nullptr;
        info->conflicting_devices_length = 0;
    }
    return ret;
}

static int shim_device_open(const hw_module_t* /*module*/, const char* name, hw_device_t** device) {
    if (load_vendor_module() != 0) return -EINVAL;
    std::lock_guard<std::mutex> lock(gCameraMutex);
    
    int camera_id = atoi(name);

    // If Camera 0 is active, return a mock Camera 1 device.
    // The Samsung ISP cannot handle both cameras simultaneously.
    if (camera_id == 1 && gCamera0Active) {
        ALOGW("Conflict detected: Camera 0 active. Intercepting Camera 1 opening with Mock");
        *device = (hw_device_t*)create_mock_camera1_device();
        return 0;
    }

    // Legacy native open
    int ret = -ENOSYS;
    if (gVendorModule->open_legacy != nullptr) {
        ret = gVendorModule->open_legacy((const hw_module_t*)gVendorModule, name, CAMERA_DEVICE_API_VERSION_1_0, device);
    } else {
        ret = gVendorModule->common.methods->open((const hw_module_t*)gVendorModule, name, device);
    }

    if (ret == 0 && *device != nullptr) {
        if (camera_id == 0) {
            gCamera0Active = true;
            gRealCam0Close = (*device)->close;
            (*device)->close = shim_cam0_close;
            ALOGI("Camara 0 abierta exitosamente en hardware");
        } else if (camera_id == 1) {
            // Save the real parameters of the front camera in the first clean reading
            camera_device_t* cam_dev = (camera_device_t*)(*device);
            if (cam_dev->ops && cam_dev->ops->get_parameters && gCachedCam1Params.empty()) {
                char* p = cam_dev->ops->get_parameters(cam_dev);
                if (p) {
                    gCachedCam1Params = p;
                    cam_dev->ops->put_parameters(cam_dev, p);
                }
            }
        }
    }

    return ret;
}

static struct hw_module_methods_t shim_module_methods = { .open = shim_device_open };

static int shim_set_callbacks(const camera_module_callbacks_t *c) {
    if (load_vendor_module() != 0) return -EINVAL;
    if (gVendorModule->set_callbacks) return gVendorModule->set_callbacks(c);
    return 0;
}
static int shim_set_torch_mode(const char* id, bool e) {
    if (load_vendor_module() != 0) return -EINVAL;
    if (gVendorModule->set_torch_mode) return gVendorModule->set_torch_mode(id, e);
    return -ENOSYS;
}
static int shim_init() {
    if (load_vendor_module() != 0) return -EINVAL;
    if (gVendorModule->init) return gVendorModule->init();
    return 0;
}
static void shim_get_vendor_tag_ops(vendor_tag_ops_t* ops) {
    if (load_vendor_module() != 0) return;
    if (gVendorModule->get_vendor_tag_ops) gVendorModule->get_vendor_tag_ops(ops);
}

camera_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = CAMERA_MODULE_API_VERSION_2_4, 
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = CAMERA_HARDWARE_MODULE_ID,
        .name = "HAL1 Shim",
        .author = "Gonic",
        .methods = &shim_module_methods,
        .dso = nullptr,
        .reserved = {0},
    },
    .get_number_of_cameras = []() -> int {
        if (load_vendor_module() != 0) return 0;
        return gVendorModule->get_number_of_cameras();
    },
    .get_camera_info = shim_get_camera_info,
    .set_callbacks = shim_set_callbacks,
    .get_vendor_tag_ops = shim_get_vendor_tag_ops,
    .open_legacy = nullptr,
    .set_torch_mode = shim_set_torch_mode,
    .init = shim_init,
    .reserved = {0}
};