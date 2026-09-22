//
// Copyright (C) 2026 Brian Nahuel Götte
// SPDX-License-Identifier: Apache-2.0
//
// Minimal HAL1 shim for Samsung Exynos7870 (j7xelte).
//
// Purpose:
//   1. Force camera devices to report as HAL1 (CAMERA_DEVICE_API_VERSION_1_0)
//   2. Declare cameras as mutually exclusive (resource_cost=100 + conflicting_devices)
//      so the CameraService never opens both simultaneously (the ISP can't handle it)
//   3. Route device opens through open_legacy with HAL1 version
//
// Everything else is passed through to the vendor HAL untouched.
//

#define LOG_TAG "ExynosCameraShim_HAL1"

#include <hardware/hardware.h>
#include <hardware/camera.h>

#include <dlfcn.h>
#include <utils/Log.h>
#include <stdlib.h>
#include <string.h>

static camera_module_t* gVendorModule = nullptr;

// Static conflict declarations — these must persist for the lifetime of the process.
// CameraService reads these pointers directly; they cannot be stack/temp allocations.
static char  sCam1Id[] = "1";
static char  sCam0Id[] = "0";
static char* sCam0Conflicts[] = { sCam1Id };
static char* sCam1Conflicts[] = { sCam0Id };

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

static int shim_get_camera_info(int camera_id, struct camera_info *info) {
    if (load_vendor_module() != 0) return -EINVAL;

    memset(info, 0, sizeof(struct camera_info));
    int ret = gVendorModule->get_camera_info(camera_id, info);

    if (ret == 0) {
        // Force HAL1
        info->device_version = CAMERA_DEVICE_API_VERSION_1_0;

        // Declare cameras as mutually exclusive.
        // resource_cost = 100 means "this camera uses 100% of the ISP".
        // conflicting_devices explicitly tells CameraService to never have both open.
        // This replaces the need for a mock camera device or state tracking.
        info->resource_cost = 100;
        if (camera_id == 0) {
            info->conflicting_devices = sCam0Conflicts;
            info->conflicting_devices_length = 1;
        } else if (camera_id == 1) {
            info->conflicting_devices = sCam1Conflicts;
            info->conflicting_devices_length = 1;
        }

        ALOGI("get_camera_info(%d): forzado HAL1, resource_cost=100, conflictos declarados", camera_id);
    }
    return ret;
}

static int shim_device_open(const hw_module_t* /*module*/, const char* name, hw_device_t** device) {
    if (load_vendor_module() != 0) return -EINVAL;

    ALOGI("Peticion de apertura recibida para camara %s", name);

    // Use open_legacy to force HAL1 path if available, otherwise fall back to regular open.
    int ret;
    if (gVendorModule->open_legacy != nullptr) {
        ret = gVendorModule->open_legacy(
            (const hw_module_t*)gVendorModule, name,
            CAMERA_DEVICE_API_VERSION_1_0, device);
    } else {
        ret = gVendorModule->common.methods->open(
            (const hw_module_t*)gVendorModule, name, device);
    }

    if (ret == 0) {
        ALOGI("Camara %s abierta exitosamente via %s", name,
              gVendorModule->open_legacy ? "open_legacy" : "open");
    } else {
        ALOGE("Fallo al abrir camara %s: %d", name, ret);
    }

    return ret;
}

// --- Pass-through module functions ---

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
        .name = "HAL1 Robust Shim",
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