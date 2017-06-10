LOCAL_PATH:= $(call my-dir)
LOCAL_DIR_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS:= \
        -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \
        -include camera_defs_i.h

LOCAL_SRC_FILES:= \
        params_test.c

LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../../../common
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/include
LOCAL_C_INCLUDES+= hardware/qcom/camera
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../hardware/cpp/
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/include/mm-still/jpeg

LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES:= libcutils libdl

LOCAL_MODULE:= cpp-params-test

LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

#******CPP****#
LOCAL_PATH:= $(LOCAL_DIR_PATH)
include $(CLEAR_VARS)

LOCAL_CFLAGS:= \
        -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \
        -include camera_defs_i.h

LOCAL_SRC_FILES:= \
        test-app.c

LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../../../common
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/include
LOCAL_C_INCLUDES+= hardware/qcom/camera
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../hardware/cpp/

LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/include/mm-still/jpeg
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES:= libcutils libdl

LOCAL_MODULE:= cpp-test-app

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)


