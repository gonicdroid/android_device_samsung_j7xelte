LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libinit_j7xelte
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := init_j7xelte.cpp
LOCAL_STATIC_LIBRARIES := libbase
LOCAL_C_INCLUDES := \
    system/libbase/include \
    system/core/init

include $(BUILD_STATIC_LIBRARY)
