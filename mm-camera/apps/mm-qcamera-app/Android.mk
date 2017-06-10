ifneq ($(call is-android-codename-in-list,JELLY_BEAN),true)
LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)

ifeq ($(call is-board-platform,msm8960),true)
LOCAL_CFLAGS:= \
        -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \
        $(USE_SERVER_TREE) \
        -include camera_defs_i.h

ifeq ($(strip $(TARGET_USES_ION)),true)
LOCAL_CFLAGS += -DUSE_ION
endif

LOCAL_SRC_FILES:= \
        mm_qcamera_main_menu.c \
        mm_qcamera_display.c \
        mm_qcamera_app.c \
        mm_qcamera_snapshot.c \
        mm_qcamera_video.c \
        mm_qcamera_preview.c

LOCAL_C_INCLUDES:=$(LOCAL_PATH)
LOCAL_C_INCLUDES+= \
        $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface \
        $(LOCAL_PATH)/../../common \
	$(TARGET_OUT_INTERMEDIATES)/include/mm-still/jpeg
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../../../../hardware/qcom/camera

include $(LOCAL_PATH)/../../local_additional_dependency.mk

ifeq ($(call is-board-platform,msm8960),true)
LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID
else
LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CAMERA_HEAP_ID
endif

LOCAL_SHARED_LIBRARIES:= \
         libcutils libdl

LOCAL_MODULE:= mm-qcamera-app

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
endif
endif
