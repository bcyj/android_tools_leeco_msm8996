ifeq ($(call is-vendor-board-platform,QCOM),true)

ifneq ($(BUILD_TINY_ANDROID),true)


ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
# 				Common definitons
# ---------------------------------------------------------------------------------

mm-audio-alsa-def := -g -O3
mm-audio-alsa-def += -D_ANDROID_

ifeq ($(call is-board-platform-in-list,msm8660 msm8960 msm8974 msm8226 msm8610 copper apq8084 msm8994 msm8909),true)
mm-audio-alsa-def += -DQDSP6V2
endif

# ---------------------------------------------------------------------------------
# 			Make the Shared library (libaudioalsa)
# ---------------------------------------------------------------------------------

LOCAL_MODULE		:= libaudioalsa
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS	  	:= $(mm-audio-alsa-def)
LOCAL_C_INCLUDES        := $(LOCAL_PATH)/inc
LOCAL_PRELINK_MODULE	:= false
LOCAL_SRC_FILES		:= src/hw.c
LOCAL_COPY_HEADERS_TO   := mm-audio/audio-alsa
LOCAL_COPY_HEADERS      := inc/control.h

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
# 			Make the apps-test (mm-audio-alsa-test)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
	
LOCAL_MODULE		:= mm-audio-alsa-test
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS	  	:= $(mm-audio-alsa-def)
LOCAL_C_INCLUDES	:= $(LOCAL_PATH)/inc
LOCAL_PRELINK_MODULE	:= false
LOCAL_SHARED_LIBRARIES	:= libaudioalsa
LOCAL_SRC_FILES		:= test/client.c

include $(BUILD_EXECUTABLE)

endif #BUILD_TINY_ANDROID
endif # is-vendor-board-platform

# ---------------------------------------------------------------------------------
# 					END
# ---------------------------------------------------------------------------------

