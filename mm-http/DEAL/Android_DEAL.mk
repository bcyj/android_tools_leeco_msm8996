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

#LOCAL_SRC_FILES += src/ExtractorCryptoAdaptationLayer.cpp
LOCAL_SRC_FILES += src/DashExtractorAdaptationLayer.cpp
LOCAL_SRC_FILES += src/DEALUtils.cpp
LOCAL_SRC_FILES += src/DashExtractor.cpp
LOCAL_SRC_FILES += src/DashMediaSource.cpp
#LOCAL_SRC_FILES += src/DashCryptoInterface.cpp
#LOCAL_SRC_FILES += src/DashExtendedExtractor.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../IPStream/Common/StreamUtils/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../IPStream/MMI/HTTP/inc

ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_C_INCLUDES += $(TOP)/frameworks/base/media/libstagefright/mpeg2ts
else
LOCAL_C_INCLUDES += $(TOP)/frameworks/av/media/libstagefright/mpeg2ts
LOCAL_C_INCLUDES += $(TOP)/frameworks/av/media/libstagefright/include
endif

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/mmi
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/omxcore
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TOP)/frameworks/av/include/drm
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/ultimateplayerfwk/native/inc
LOCAL_C_INCLUDES += $(TOP)/hardware/qcom/media/mm-core/inc
LOCAL_C_INCLUDES += $(TOP)/frameworks/av/media/libmediaplayerservice
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/binder
LOCAL_C_INCLUDES += $(TOP)/frameworks/av/include/media
LOCAL_C_INCLUDES += $(TOP)/frameworks/av/include/media/stagefright
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libmmosal
LOCAL_SHARED_LIBRARIES += libmmiipstreammmihttp
LOCAL_SHARED_LIBRARIES += libstagefright
LOCAL_SHARED_LIBRARIES += libmmipstreamsourcehttp
LOCAL_SHARED_LIBRARIES += libstagefright_foundation
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libdrmframework
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libmedia
#LOCAL_SHARED_LIBRARIES += libmediaplayerservice
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libextendedmediaextractor


LOCAL_MODULE_TAGS := optional
LOCAL_32_BIT_ONLY := true
LOCAL_MODULE := libmmipstreamdeal

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
