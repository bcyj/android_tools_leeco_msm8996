LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#-----------------------------------------------------------------
# Define
#-----------------------------------------------------------------
LOCAL_CFLAGS := -D_ANDROID_

#----------------------------------------------------------------
# SRC CODE
#----------------------------------------------------------------
LOCAL_SRC_FILES := src/WFDDDPWrapper.cpp

#----------------------------------------------------------------
# INCLUDE PATH
#----------------------------------------------------------------
LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../source/framework/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../sink/framework/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../utils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/omxcore


#----------------------------------------------------------------
# Dolby COMPILE TIME
#----------------------------------------------------------------
COMPILE_DOLBY_FEATURE := false
DOLBY_LIB_PATH  := $(TOP)/thirdpartylibs/dolby
COMPILE_DOLBY_FEATURE := $(shell if [ -f $(DOLBY_LIB_PATH)/Android.mk ] ; then echo true; fi)


ifeq ($(COMPILE_DOLBY_FEATURE),true)
LOCAL_C_INCLUDES += $(TOP)/thirdpartylibs/dolby/include

LOCAL_STATIC_LIBRARIES += libddp_enc_ce_res_nbe
LOCAL_STATIC_LIBRARIES += libdlb_intrinsics

LOCAL_CFLAGS += -DDOLBY_ENCODER_ENABLE
LOCAL_SRC_FILES += src/WFDDDPWrapper.cpp
endif

LOCAL_SHARED_LIBRARIES := libmmosal
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils

LOCAL_MODULE:= libwfdac3wrapper

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
