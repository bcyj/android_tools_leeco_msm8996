LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS := -D_ANDROID_

ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_CFLAGS += -DWFD_ICS
endif
LOCAL_SRC_FILES := src/wdsm_mm_sink_interface.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/./inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/./../framework/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../interface/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../uibc/interface/inc
ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_C_INCLUDES += $(TOP)/frameworks/base/include/media
else
LOCAL_C_INCLUDES += $(TOP)/frameworks/av/include/media
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/gui
endif
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/omxcore
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../utils/inc
ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_C_INCLUDES += $(TOP)/frameworks/base/drm/libdrmframework/include
LOCAL_C_INCLUDES += $(TOP)/frameworks/base/include/drm
else
LOCAL_C_INCLUDES += $(TOP)/frameworks/av/drm/libdrmframework/include
LOCAL_C_INCLUDES += $(TOP)/frameworks/av/include/drm
endif
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -Wconversion
LOCAL_SHARED_LIBRARIES := libmmosal
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libmedia
LOCAL_SHARED_LIBRARIES += libgui
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libwfdcommonutils
LOCAL_SHARED_LIBRARIES += libwfdmmsink

LOCAL_MODULE := libmmwfdsinkinterface

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
