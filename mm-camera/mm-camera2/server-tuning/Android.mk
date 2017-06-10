ifeq ($(call is-vendor-board-platform,QCOM),true)

LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS:= \
        -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \

LOCAL_CFLAGS  += -D_ANDROID_
LOCAL_CPPFLAGS  += -D_ANDROID_ -std=c++11

LOCAL_SRC_FILES:= tuning/eztune_interface.cpp \
                  tuning/eztune_protocol.cpp \
                  tuning/eztune_process.cpp \
                  tuning/eztune_cam_adapter.cpp \
                  tuning/mmcam_log_utils.cpp \
                  tuning/mmcam_socket_utils.cpp \
                  tuning/eztune_items_0301.c

LOCAL_HAL_PATH = $(LOCAL_PATH)/../../../../../../hardware/qcom/camera
LOCAL_FASTCV_PATH = $(LOCAL_PATH)/../../../fastcv-noship

LOCAL_C_INCLUDES:=$(LOCAL_PATH)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/tuning/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/modules/sensors/chromatix/0301/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/modules/sensors/actuators/0301/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/modules/sensors/actuator_libs/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../mm-camera2/includes/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../includes/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/bus/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/controller/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/object/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/includes/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/modules/includes/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/tools/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/event/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/pipeline/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/stream/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/debug/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/module/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/port/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/modules/sensors/module/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/modules/sensors/includes/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/modules/sensors/actuators/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/modules/stats/q3a/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/modules/stats/q3a/aec/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/modules/stats/q3a/awb/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/modules/stats/q3a/af/

LOCAL_C_INCLUDES+= $(LOCAL_HAL_PATH)/QCamera2/stack/common/

LOCAL_C_INCLUDES+= $(LOCAL_FASTCV_PATH)/inc/

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES:=  liboemcamera \
                          libmmcamera2_stats_modules \
                          libmmcamera2_iface_modules \
                          libmmcamera2_isp_modules \
                          libmmcamera2_sensor_modules \
                          libmmcamera2_pproc_modules \
                          libmmcamera2_imglib_modules \
                          libmm-qcamera \
                          liblog \
                          libdl \
                          libcutils \
                          libfastcvopt

LOCAL_C_INCLUDES += external/connectivity/stlport/stlport bionic/libstdc++/include

LOCAL_MODULE_TAGS := debug

# Build tuning library
LOCAL_MODULE:= libmmcamera_tuning
LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)

endif
