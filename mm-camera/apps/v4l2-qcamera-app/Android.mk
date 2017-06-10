LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS:= \
        -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \
        $(USE_SERVER_TREE) \
        -include camera_defs_i.h

LOCAL_SRC_FILES:= \
        v4l2-qcamera-app.c

LOCAL_C_INCLUDES:=$(LOCAL_PATH)
LOCAL_C_INCLUDES+= \
	$(LOCAL_PATH)/../appslib \
        $(LOCAL_PATH)/../../common \
        $(LOCAL_PATH)/../../../mm-still/ipl/inc \
        $(LOCAL_PATH)/../../../mm-still/jpeg/inc

include $(LOCAL_PATH)/../../local_additional_dependency.mk
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../../../../hardware/qcom/camera

ifeq ($(MSM_VERSION), 8974)
LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID
else ifeq ($(MSM_VERSION), 8226)
LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID
else ifeq ($(MSM_VERSION), 8610)
LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID
else ifeq ($(MSM_VERSION), 8960)
LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID
else
LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CAMERA_HEAP_ID
endif

ifeq ($(strip $(TARGET_USES_ION)),true)
LOCAL_CFLAGS += -DUSE_ION
endif


LOCAL_SHARED_LIBRARIES:= \
	 liboemcamera libcutils libdl

LOCAL_MODULE:= v4l2-qcamera-app

LOCAL_MODULE_TAGS := optional


include $(BUILD_EXECUTABLE)
