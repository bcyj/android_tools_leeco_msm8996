# ---------------------------------------------------------------------------------
#                 AAL
# ---------------------------------------------------------------------------------
ROOT_DIR := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PATH := $(ROOT_DIR)


LOCAL_CFLAGS := -D_ANDROID_

ifeq ($(PLATFORM_SDK_VERSION),18)
LOCAL_CFLAGS += -DANDROID_JB_MR2
endif

LOCAL_SRC_FILES := src/DASHMMIMediaSource.cpp
LOCAL_SRC_FILES += src/DASHMMIMediaInfo.cpp
LOCAL_SRC_FILES += src/DASHMMIInterface.cpp
LOCAL_SRC_FILES += src/DASHHTTPMediaEntry.cpp
LOCAL_SRC_FILES += src/DASHHTTPLiveSource.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../IPStream/Common/StreamUtils/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../IPStream/MMI/HTTP/inc

#LOCAL_SRC_FILES += src/WFDSource.cpp
#LOCAL_SRC_FILES += src/WFDMediaEntry.cpp

ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_C_INCLUDES += $(TOP)/frameworks/base/media/libmediaplayerservice/nuplayer
LOCAL_C_INCLUDES += $(TOP)/frameworks/base/media/libstagefright/mpeg2ts
else
LOCAL_C_INCLUDES += $(TOP)/frameworks/av/media/libmediaplayerservice/nuplayer
LOCAL_C_INCLUDES += $(TOP)/frameworks/av/media/libstagefright/mpeg2ts
endif

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-rtp/decoder/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/mmi
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/omxcore
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-parser/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TOP)/frameworks/av/include/drm
LOCAL_C_INCLUDES += $(TOP)/external/mm-dash/dashplayer
LOCAL_C_INCLUDES += $(TOP)/hardware/qcom/media/mm-core/inc

LOCAL_SHARED_LIBRARIES := libdashplayer
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libmmosal
LOCAL_SHARED_LIBRARIES += libmmiipstreammmihttp
LOCAL_SHARED_LIBRARIES += libmmipstreamsourcehttp
LOCAL_SHARED_LIBRARIES += libmmipstreamutils
LOCAL_SHARED_LIBRARIES += libstagefright
LOCAL_SHARED_LIBRARIES += libstagefright_foundation
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libmmparser
LOCAL_SHARED_LIBRARIES += libdrmframework
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libbinder

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libmmipstreamaal

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

