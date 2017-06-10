#======================================================================
#makefile for libmmcamera2_stats_modules.so form mm-camera2
#======================================================================
ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(call is-board-platform-in-list,msm8974 msm8916 msm8960 msm7627a msm8660 msm8226 msm8610 msm8909),true)


LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(call is-board-platform-in-list, msm8610),true)
 LOCAL_CFLAGS  := -D_ANDROID_ -DFEATURE_SKIP_STATS
 FEATURE_GYRO := false
else
 LOCAL_CFLAGS  := -D_ANDROID_
 FEATURE_GYRO := false
endif

ifeq ($(call is-board-platform-in-list, msm8909),true)
LOCAL_CFLAGS  += -DAF_2X13_FILTER_SUPPORT
endif

LOCAL_MMCAMERA_PATH  := $(LOCAL_PATH)/../../../../mm-camera2
LOCAL_ALGORITHM_PATH  := $(LOCAL_PATH)/../../../../../mm-camera-lib/stats/

LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/debug-data
LOCAL_C_INCLUDES += $(LOCAL_ALGORITHM_PATH)/q3a/aec
LOCAL_C_INCLUDES += $(LOCAL_ALGORITHM_PATH)/q3a/awb
LOCAL_C_INCLUDES += $(LOCAL_ALGORITHM_PATH)/q3a/af
LOCAL_C_INCLUDES += $(LOCAL_ALGORITHM_PATH)/asd
LOCAL_C_INCLUDES += $(LOCAL_ALGORITHM_PATH)/afd
LOCAL_C_INCLUDES += $(LOCAL_ALGORITHM_PATH)/q3a/aec/algorithm
LOCAL_C_INCLUDES += $(LOCAL_ALGORITHM_PATH)/q3a/awb/algorithm
LOCAL_C_INCLUDES += $(LOCAL_ALGORITHM_PATH)/q3a/af/algorithm
LOCAL_C_INCLUDES += $(LOCAL_ALGORITHM_PATH)/afd/algorithm
LOCAL_C_INCLUDES += $(LOCAL_ALGORITHM_PATH)/asd/algorithm

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/actuators/$(CHROMATIX_VERSION)/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/actuator_libs/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/aec
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/af
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/awb
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/asd
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/afd

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/is
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../mm-camera-lib/is/sensor_lib
ifeq ($(FEATURE_GYRO), true)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../sensors/dsps/api
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../qmi/core/lib/inc
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/gyro
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/gyro/dsps
endif

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/tools/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/bus/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/controller/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/event/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/object/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/pipeline/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/port/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/stream/


#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/includes/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/includes/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/sensors/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/actuators/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/chromatix/

#LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/modules/includes/
#LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/modules/isp/
#LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/modules/isp/includes/
#LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/modules/isp/core/
#LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/modules/isp/hw/
#LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/modules/isp/hw/includes/
#LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/modules/isp/hw/axi/
#LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/modules/isp/hw/pix/
#LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/modules/isp/hw/pix/pix40/
#LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/modules/isp/hw/pix/modules/

#add gyro sensor API header locations
ifeq ($(FEATURE_GYRO), true)
 LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/sensors/inc
 LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc
endif
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/adreno

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += \
 $(LOCAL_PATH)/../../../../../../../../hardware/qcom/camera/QCamera2/stack/common


#LOCAL_CFLAGS  := -Werror 

ifeq ($(FEATURE_GYRO), false)
  LOCAL_AFD_DIR := $(LOCAL_PATH)/afd
  LOCAL_SRC_FILES += $(shell find $(LOCAL_AFD_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )
  LOCAL_ASD_DIR := $(LOCAL_PATH)/asd
  LOCAL_SRC_FILES += $(shell find $(LOCAL_ASD_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )
  LOCAL_Q3A_DIR := $(LOCAL_PATH)/q3a
  LOCAL_SRC_FILES += $(shell find $(LOCAL_Q3A_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )
  LOCAL_IS_DIR := $(LOCAL_PATH)/is
  LOCAL_SRC_FILES += $(shell find $(LOCAL_IS_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )
  LOCAL_SRC_DIR := $(LOCAL_PATH)
  LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -maxdepth 1 -name '*.c' | sed s:^$(LOCAL_PATH)::g )
else
  LOCAL_SRC_DIR := $(LOCAL_PATH)
  LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )
endif


LOCAL_MODULE           := libmmcamera2_stats_modules
LOCAL_32_BIT_ONLY := true

LOCAL_SHARED_LIBRARIES := libdl libcutils liboemcamera libmmcamera2_is libmmcamera2_stats_algorithm

ifeq ($(FEATURE_GYRO), true)
 LOCAL_SHARED_LIBRARIES += libsensor1
endif

LOCAL_MODULE_TAGS      := optional eng
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif # if 8960
endif # is-vendor-board-platform,QCOM
