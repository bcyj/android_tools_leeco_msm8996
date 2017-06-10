LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS := -D_ANDROID_
ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_CFLAGS += -DWFD_ICS
endif

LOCAL_CFLAGS += -DMR2
LOCAL_SRC_FILES := src/WFDMMSink.cpp
LOCAL_SRC_FILES += src/WFDMMSinkRenderer.cpp
LOCAL_SRC_FILES += src/WFDMMSinkVideoDecode.cpp
LOCAL_SRC_FILES += src/WFDMMSinkMediaSource.cpp
LOCAL_SRC_FILES += src/WFDMMSinkAACDecode.cpp
LOCAL_SRC_FILES += src/WFDMMSinkAudioDecode.cpp
LOCAL_SRC_FILES += src/WFDMMSinkStatistics.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/./inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../interface/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../uibc/interface/inc
ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_C_INCLUDES += $(TOP)/frameworks/base/include/media
else
LOCAL_C_INCLUDES += $(TOP)/frameworks/av/include/media
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/gui
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/ui
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/media/hardware
LOCAL_C_INCLUDES += $(TOP)/system/core/include/system
LOCAL_C_INCLUDES += $(TOP)/hardware/qcom/display/libqdutils
LOCAL_C_INCLUDES += $(TOP)/hardware/qcom/display/libgralloc
endif
LOCAL_C_INCLUDES += $(TOP)/external/aac/libAACdec/include
LOCAL_C_INCLUDES += $(TOP)/external/aac/libSYS/include

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-parser/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/omxcore
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-mux
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-rtp/decoder/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../utils/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../source/utils/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../hdcp/common/inc

ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_C_INCLUDES += $(TOP)/frameworks/base/include/drm
else
LOCAL_C_INCLUDES += $(TOP)/frameworks/av/include/drm
endif
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := libmmosal
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libmedia
LOCAL_SHARED_LIBRARIES += libgui
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libwfdcommonutils
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += libmmparser
LOCAL_SHARED_LIBRARIES += libwfdmmutils
LOCAL_SHARED_LIBRARIES += libmmrtpdecoder
LOCAL_SHARED_LIBRARIES += libFileMux
LOCAL_SHARED_LIBRARIES += libOmxCore
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libwfdhdcpcp
LOCAL_SHARED_LIBRARIES += libstagefright_soft_aacdec
LOCAL_SHARED_LIBRARIES += libqdMetaData
LOCAL_SHARED_LIBRARIES += libqdutils

LOCAL_MODULE := libwfdmmsink

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
