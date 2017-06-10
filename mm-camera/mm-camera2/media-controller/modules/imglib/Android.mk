#======================================================================
# makefile for libmmcamera2_imglib_modules.so for mm-camera2
#======================================================================
ifeq ($(call is-vendor-board-platform,QCOM),true)

LOCAL_PATH := $(call my-dir)
LOCAL_IMGLIB_PATH  := $(LOCAL_PATH)

COMMON_DEFINES := -DHDR_LIB_GHOSTBUSTER

include $(CLEAR_VARS)


LOCAL_CFLAGS  := -D_ANDROID_
LOCAL_CFLAGS += $(COMMON_DEFINES)

ifeq ($(call is-board-platform-in-list, msm8909),true)
LOCAL_CFLAGS  += -DAF_2X13_FILTER_SUPPORT
COMMON_DEFINES += -DAF_2X13_FILTER_SUPPORT
endif

ifeq ($(MSM_VERSION), 8226)
  LOCAL_CFLAGS += -DCAMERA_FEATURE_WNR_SW
endif

ifeq ($(MSM_VERSION), 8610)
  LOCAL_CFLAGS += -DCAMERA_FEATURE_WNR_SW
endif

ifeq ($(MSM_VERSION), 8909)
  LOCAL_CFLAGS += -DCAMERA_FEATURE_WNR_SW
endif

LOCAL_MMCAMERA_PATH  := $(LOCAL_PATH)/../../../../mm-camera2

USE_CAC_V1:= false

LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/includes/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/tools/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/bus/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/controller/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/event/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/object/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/pipeline/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/port/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/stream/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/debug/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/includes/
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/common
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/hdr
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/wd
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/faceproc
ifeq ($(USE_CAC_V1),true)
  LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/cac
else
  LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/cac_v2
endif
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/include
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/lib
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/lib/faceproc
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/lib/cac
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/modules
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/modules/common
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/modules/base
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/utils

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += hardware/qcom/camera/QCamera2/stack/common
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/actuators/$(CHROMATIX_VERSION)

#LOCAL_CFLAGS  += -Werror

ifeq ($(strip $(USE_CAC_V1)),true)
  LOCAL_CFLAGS += -DUSE_CAC_V1
endif


LOCAL_SRC_DIR := $(LOCAL_PATH)/modules
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_MODULE           := libmmcamera2_imglib_modules
LOCAL_32_BIT_ONLY := true
LOCAL_SHARED_LIBRARIES := liblog libcutils liboemcamera libmmcamera_imglib
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_MODULE_TAGS      := optional eng
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

################ faceproc prebuilt library #############

LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)

ifeq ($(FEATURE_FACE_PROC),true)
  include $(CLEAR_VARS)
  LOCAL_PATH := $(LOCAL_IMGLIB_PATH)/components/lib/faceproc
  LOCAL_MODULE       := libmmcamera_faceproc
  LOCAL_32_BIT_ONLY := true
  LOCAL_MODULE_SUFFIX := .so
  LOCAL_MODULE_CLASS := SHARED_LIBRARIES
  LOCAL_SRC_FILES := libmmcamera_faceproc.so
  LOCAL_MODULE_TAGS := optional eng
  LOCAL_MODULE_OWNER := qcom 
  LOCAL_32_BIT_ONLY := true
  LOCAL_PROPRIETARY_MODULE := true

  include $(BUILD_PREBUILT)
endif

################ component library ######################
include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)

mmimg_defines:=  \
       -DAMSS_VERSION=$(AMSS_VERSION) \
       -g -O0 \
       -D_ANDROID_ \
       -include img_dbg.h \

mmimg_defines += -DLOGE=ALOGE

ifeq ($(strip $(TARGET_USES_ION)),true)
mmimg_defines += -DUSE_ION
endif

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/hdr
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/wd
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/faceproc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/include
ifeq ($(USE_CAC_V1),true)
  LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/cac
else
  LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/cac_v2
endif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/lib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/lib/faceproc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/lib/cac
LOCAL_C_INCLUDES += $(LOCAL_PATH)/utils

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)

mmimg_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/ion.h
mmimg_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/videodev2.h

LOCAL_CFLAGS := $(mmimg_defines)
LOCAL_CFLAGS += $(COMMON_DEFINES)

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/adsprpc/inc

ifeq ($(call is-board-platform-in-list,msm8610),true)
  LOCAL_CFLAGS  += -DUSE_SMMU_BUFFERS_FOR_WNR
endif #msm8610

LOCAL_SRC_FILES := utils/img_queue.c
LOCAL_SRC_FILES += utils/img_buffer.c
LOCAL_SRC_FILES += components/common/img_common.c
LOCAL_SRC_FILES += components/common/img_comp.c
LOCAL_SRC_FILES += components/common/img_comp_factory.c
LOCAL_SRC_FILES += components/wd/wd_comp.c
LOCAL_SRC_FILES += components/hdr/hdr_comp.c
LOCAL_SRC_FILES += components/faceproc/faceproc_comp_eng.c
LOCAL_SRC_FILES += components/faceproc/faceproc_comp.c
LOCAL_SRC_FILES += components/frameproc/frameproc_comp.c
ifeq ($(USE_CAC_V1),true)
 LOCAL_SRC_FILES += components/cac/cac_comp.c
else
  LOCAL_SRC_FILES += components/cac_v2/cac_v2_comp.c
endif

LOCAL_MODULE           := libmmcamera_imglib
LOCAL_PRELINK_MODULE   := false
LOCAL_SHARED_LIBRARIES := libdl libcutils liblog libadsprpc

LOCAL_MODULE_TAGS := optional
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

##########################test app ##############################
include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)

mmimg_defines:= \
       -DAMSS_VERSION=$(AMSS_VERSION) \
       -g -O0 \
       -D_ANDROID_ \
       -include img_dbg.h \

mmimg_defines += -DLOGE=ALOGE

ifeq ($(strip $(TARGET_USES_ION)),true)
mmimg_defines += -DUSE_ION
endif

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/hdr
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/wd
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/faceproc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/cac
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/include/faceproc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/test
LOCAL_C_INCLUDES += $(LOCAL_PATH)/utils

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)

mmimg_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/ion.h
mmimg_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/videodev2.h

LOCAL_CFLAGS := $(mmimg_defines)
LOCAL_CFLAGS += $(COMMON_DEFINES)

LOCAL_SRC_FILES := components/test/img_test.c
LOCAL_SRC_FILES += components/test/hdr_test.c
LOCAL_SRC_FILES += components/test/denoise_test.c
LOCAL_SRC_FILES += components/test/faceproc_test.c
LOCAL_SRC_FILES += components/test/cac_test.c

LOCAL_SHARED_LIBRARIES:= \
    libmmcamera_imglib libcutils libdl
LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_MODULE:= mm-imglib-test

LOCAL_MODULE_TAGS := optional

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

include $(BUILD_EXECUTABLE)

##########################hdr module test app ##############################
include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)

mmimg_defines:= \
       -DAMSS_VERSION=$(AMSS_VERSION) \
       -g -O0 \
       -D_ANDROID_ \

mmimg_defines += -DLOGE=ALOGE

ifeq ($(strip $(TARGET_USES_ION)),true)
mmimg_defines += -DUSE_ION
endif

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/includes/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/tools/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/bus/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/controller/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/event/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/object/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/pipeline/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/port/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/stream/

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/utils

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES += hardware/qcom/camera/QCamera2/stack/common

LOCAL_CFLAGS := $(mmimg_defines)
LOCAL_CFLAGS += $(COMMON_DEFINES)

LOCAL_SRC_FILES := test/test_module_hdr.c

LOCAL_SHARED_LIBRARIES:= \
    libmmcamera2_imglib_modules liboemcamera libcutils libdl
LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_MODULE:= mm-module-hdr-test

LOCAL_MODULE_TAGS := optional

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

include $(BUILD_EXECUTABLE)

##########################module test app ##############################
include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)

mmimg_defines:= \
       -DAMSS_VERSION=$(AMSS_VERSION) \
       -g -O0 \
       -D_ANDROID_ \

mmimg_defines += -DLOGE=ALOGE

ifeq ($(strip $(TARGET_USES_ION)),true)
mmimg_defines += -DUSE_ION
endif

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/includes/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/tools/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/bus/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/controller/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/event/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/object/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/pipeline/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/port/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/stream/

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/utils

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES += hardware/qcom/camera/QCamera2/stack/common

LOCAL_CFLAGS := $(mmimg_defines)
ifeq ($(call is-board-platform-in-list, msm8909),true)
LOCAL_CFLAGS  += -DAF_2X13_FILTER_SUPPORT
mmimg_defines  += -DAF_2X13_FILTER_SUPPORT
endif

LOCAL_SRC_FILES := test/test_module_imglib.c

LOCAL_CFLAGS := $(mmimg_defines)

LOCAL_SHARED_LIBRARIES:= \
    libmmcamera2_imglib_modules liboemcamera libcutils libdl
LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_MODULE:= mm-module-imglib-test

LOCAL_MODULE_TAGS := optional

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

include $(BUILD_EXECUTABLE)

##########################module test app ##############################
include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)

mmimg_defines:= \
       -DAMSS_VERSION=$(AMSS_VERSION) \
       -g -O0 \
       -D_ANDROID_ \
       -include img_dbg.h \

mmimg_defines += -DLOGE=ALOGE

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/include

LOCAL_CFLAGS := $(mmimg_defines)
LOCAL_CFLAGS += $(COMMON_DEFINES)

LOCAL_SRC_FILES := test/imgalgo_dummy.c

LOCAL_MODULE           := libmmcamera_dummyalgo
LOCAL_32_BIT_ONLY := true
LOCAL_PRELINK_MODULE   := false
LOCAL_SHARED_LIBRARIES := libdl libcutils liblog libmmcamera_imglib

LOCAL_MODULE_TAGS := optional
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif # if 8974
