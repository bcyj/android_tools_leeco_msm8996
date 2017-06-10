ifeq ($(call is-vendor-board-platform,QCOM),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
#
# unit test executables
#

# Global flag and include definitions
TEST_CFLAGS := \
  -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags) \
  -DMSM_CAMERA_BIONIC


ifneq ($(call is-platform-sdk-version-at-least,17),true)
  TEST_CFLAGS += -include bionic/libc/kernel/common/linux/types.h
  TEST_CFLAGS += -include bionic/libc/kernel/common/linux/socket.h
  TEST_CFLAGS += -include bionic/libc/kernel/common/linux/in.h
  TEST_CFLAGS += -include bionic/libc/kernel/common/linux/un.h
endif

TEST_C_INCLUDES:=$(LOCAL_PATH)
TEST_C_INCLUDES+= $(LOCAL_PATH)/../includes/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/bus/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/controller/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/object/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/includes/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/tools/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/event/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/pipeline/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/stream/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/module/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/port/
TEST_C_INCLUDES+= \
 $(LOCAL_PATH)/../../../../../../hardware/qcom/camera/QCamera2/stack/common

#
# test_pipeline
#

include $(LOCAL_PATH)/../../local_additional_dependency.mk

#LOCAL_CFLAGS := $(TEST_CFLAGS)
#LOCAL_C_INCLUDES := $(TEST_C_INCLUDES)
#LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
#LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

#LOCAL_SRC_FILES:= test_pipeline.c

#LOCAL_SHARED_LIBRARIES:= libcutils liboemcamera.so

#LOCAL_MODULE:= test_pipeline
#LOCAL_MODULE_TAGS := optional

#include $(BUILD_EXECUTABLE)

#
# test_list
#
#include $(CLEAR_VARS)

LOCAL_CFLAGS := $(TEST_CFLAGS)
LOCAL_CFLAGS  += -D_ANDROID_

LOCAL_C_INCLUDES := $(TEST_C_INCLUDES)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES:= test_list.c

LOCAL_SHARED_LIBRARIES:= libcutils liboemcamera

LOCAL_MODULE:= test_list
LOCAL_32_BIT_ONLY := true
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

#
# test_sensor
#
include $(CLEAR_VARS)
CHROMATIX_VERSION := 0301
LOCAL_CFLAGS := $(TEST_CFLAGS)
LOCAL_CFLAGS  += -D_ANDROID_

LOCAL_C_INCLUDES := $(TEST_C_INCLUDES)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/module/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/sensors/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/actuators/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/actuator_libs/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/actuators/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/chromatix/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/csid/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/csiphy/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/eeprom/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/led_flash/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/strobe_flash/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/core/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/hw/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/hw/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/hw/axi/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/hw/pix/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/hw/pix/pix40/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/hw/pix/modules/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/hw/pix/modules/src/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/hw/pix/modules/scaler/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/iface/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/iface/src/

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES:= test_sensor.c test_isp.c

LOCAL_SHARED_LIBRARIES:= libcutils liboemcamera \
                                   libmmcamera2_isp_modules \
                                   libmmcamera2_sensor_modules
LOCAL_MODULE:= test_sensor
LOCAL_32_BIT_ONLY := true
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)


#
# test_cpp
#
include $(CLEAR_VARS)

LOCAL_CFLAGS := $(TEST_CFLAGS)
CHROMATIX_VERSION := 0301
LOCAL_CFLAGS  += -D_ANDROID_
LOCAL_C_INCLUDES := $(TEST_C_INCLUDES)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/pproc-new/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../server-imaging/
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES:= test_cpp.c

LOCAL_SHARED_LIBRARIES:= libcutils liboemcamera libmmcamera2_pproc_modules libdl

LOCAL_MODULE:= test_module_pproc
LOCAL_32_BIT_ONLY := true
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
#END
endif
