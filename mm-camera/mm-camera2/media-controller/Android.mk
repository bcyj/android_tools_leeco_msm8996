ifeq ($(call is-vendor-board-platform,QCOM),true)
#************* liboemcamera Start ************#
LOCAL_PATH := $(call my-dir)
LOCAL_DIR_PATH:= $(call my-dir)
include $(CLEAR_VARS)


LOCAL_CFLAGS :=  -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags) \
  -DMSM_CAMERA_BIONIC

LOCAL_CFLAGS  += -D_ANDROID_

ifeq ($(call is-board-platform-in-list, msm8610),true)
 FEATURE_GYRO := false
endif

#LOCAL_CFLAGS += -Werror

ifneq ($(call is-platform-sdk-version-at-least,17),true)
  LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/types.h
  LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/socket.h
  LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/in.h
  LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/un.h
endif

ifeq ($(strip $(TARGET_USES_ION)),true)
  LOCAL_CFLAGS += -DUSE_ION
  ifeq ($(MSM_VERSION),7x27A)
    LOCAL_CFLAGS += -DTARGET_7x27A
  endif
endif

ifeq ($(VFE_VERS),vfe40)
  LOCAL_CFLAGS += -DVFE_40
else ifeq ($(VFE_VERS),vfe32)
  LOCAL_CFLAGS += -DVFE_32
  ifeq ($(FEATURE_GYRO), true)
    LOCAL_CFLAGS += -DFEATURE_GYRO
  endif
else ifeq ($(VFE_VERS),vfe31)
  LOCAL_CFLAGS += -DVFE_31
  ifeq ($(MSM_VERSION), 7x30)
    LOCAL_CFLAGS += -DVFE_31_7x30
  else
    LOCAL_CFLAGS += -DVFE_31_8x60
  endif
else ifeq ($(VFE_VERS),vfe2x)
  LOCAL_CFLAGS += -DVFE_2X
endif

#ifeq ($(MM_DEBUG),true) # does not compile is not defined
LOCAL_CFLAGS+= -DIPL_DEBUG_STANDALONE
#endif

ifeq ($(MSM_VERSION), 8226)
  LOCAL_CFLAGS += -DCAMERA_FEATURE_WNR_SW
endif

ifeq ($(MSM_VERSION), 8610)
  LOCAL_CFLAGS += -DCAMERA_FEATURE_WNR_SW
endif

ifeq ($(MSM_VERSION), 8909)
  LOCAL_CFLAGS += -DCAMERA_FEATURE_WNR_SW
endif

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../common/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/mct/tools/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/mct/bus/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/mct/controller/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/mct/event/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/mct/module/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/mct/object/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/mct/pipeline/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/mct/port/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/mct/stream/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/mct/debug/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../server-imaging/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../server-tuning/tuning

LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/includes/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/includes/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/sensors/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/actuators/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/actuators/0301
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/chromatix/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/chromatix/$(CHROMATIX_VERSION)
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/csid/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/csiphy/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/eeprom/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/led_flash/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/sensors/strobe_flash/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/stats/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/stats/q3a/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/stats/q3a/aec/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/stats/q3a/awb/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/stats/q3a/af/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/includes/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/isp/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/isp/hw/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/isp/hw/modules/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/iface/
#LOCAL_C_INCLUDES += \
# $(LOCAL_PATH)/../../../../../../hardware/qcom/camera/QCamera2/stack/common

#LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../mm-camera-lib/stats/q3a/aec
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../mm-camera-lib/stats/q3a/awb
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../mm-camera-lib/stats/q3a/af


#LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../mm-camera-lib/stats/q3a/aec/algorithm
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../mm-camera-lib/stats/q3a/awb/algorithm
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../mm-camera-lib/stats/q3a/af/algorithm

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

# all files under mct compiled into liboemcamera.so
LOCAL_SRC_DIR := $(LOCAL_PATH)/mct/
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add helper tools
#LOCAL_SRC_DIR := $(LOCAL_PATH)/mct/tools
#LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add helper tools
#LOCAL_SRC_DIR := $(LOCAL_PATH)/mct/bus
#LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add object framework
#LOCAL_SRC_DIR := $(LOCAL_PATH)/mct/object
#LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add port framework
#LOCAL_SRC_DIR := $(LOCAL_PATH)/mct/port
#LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add module framework
#LOCAL_SRC_DIR := $(LOCAL_PATH)/mct/module
#LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add stream framework
#LOCAL_SRC_DIR := $(LOCAL_PATH)/mct/stream
#LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add pipeline framework
#LOCAL_SRC_DIR := $(LOCAL_PATH)/mct/pipeline
#LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add helper tools
#LOCAL_SRC_DIR := $(LOCAL_PATH)/mct/controller
#LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add module sensor
#LOCAL_SRC_DIR := $(LOCAL_PATH)/modules/sensors/module \
#                 $(LOCAL_PATH)/modules/sensors/sensors \
#                 $(LOCAL_PATH)/modules/sensors/chromatix/module \
#                 $(LOCAL_PATH)/modules/sensors/actuators \
#                 $(LOCAL_PATH)/modules/sensors/eeprom \
#                 $(LOCAL_PATH)/modules/sensors/led_flash \
#                 $(LOCAL_PATH)/modules/sensors/strobe_flash \
#                 $(LOCAL_PATH)/modules/sensors/csiphy \
#                 $(LOCAL_PATH)/modules/sensors/csid
#LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add module q3a
#LOCAL_SRC_DIR := $(LOCAL_PATH)/modules/stats
#LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add iface
#LOCAL_SRC_DIR := $(LOCAL_PATH)/modules/iface
#LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )
#LOCAL_SHARED_LIBRARIES:= libcutils liblog

#add isp
#LOCAL_SRC_DIR := $(LOCAL_PATH)/modules/isp/
#LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_SHARED_LIBRARIES:= libdl libcutils  liblog

LOCAL_MODULE:=liboemcamera
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

#include $(LOCAL_PATH)/../../malloc_wrap_static_library.mk

#************* sensor module libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/sensors/Android.mk
#************* sensor libs end ************#

#************* sensor libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/sensors/sensor_libs/Android.mk
#************* sensor libs end ************#

#************* sensor libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/sensors/eeprom_libs/Android.mk
#************* sensor libs end ************#

#************* actuator libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/sensors/actuators/$(CHROMATIX_VERSION)/Android.mk
#************* actuator libs end ************#

#************* actuator driver libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/sensors/actuator_libs/Android.mk
#************* actuator driver libs end ************#

#************* chromatix libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
#include $(LOCAL_PATH)/media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)/libchromatix/Android.mk
include $(LOCAL_PATH)/modules/sensors/chromatix/0301/libchromatix/Android.mk
#************* chromatix libs end ************#

#************* stats libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/stats/Android.mk
#************* stats libs end ************#

#************* isp libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/isp/Android.mk
#************* isp libs end ************#

#************* iface libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/iface/Android.mk
#************* iface libs end ************#

#************* pproc libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/pproc-new/Android.mk
#************* pproc libs end ************#

#************* imglib libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/imglib/Android.mk
#************* imglib libs end ************#

endif
