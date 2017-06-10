ifeq ($(call is-vendor-board-platform,QCOM),true)
ifneq ($(call is-board-platform,copper),true)

#RTP Decoder makefile
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := src/RTPDataSource.cpp
LOCAL_SRC_FILES += src/RTPParser.cpp
LOCAL_SRC_FILES += src/RTPStreamPort.cpp

LOCAL_COPY_HEADERS_TO := mm-rtp/decoder/include
LOCAL_COPY_HEADERS += inc/RTPDataSource.h
LOCAL_COPY_HEADERS += inc/RTPStreamPort.h

LOCAL_C_INCLUDES := $(JNI_H_INCLUDE)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common/inc

ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_C_INCLUDES +=  $(TOP)/frameworks/base/include/utils
else ifeq ("$(PLATFORM_VERSION)","4.3")
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/utils
else
LOCAL_C_INCLUDES += $(TOP)/system/core/include/utils
endif

LOCAL_CFLAGS := -D_ANDROID_

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := libutils liblog libcutils
LOCAL_SHARED_LIBRARIES += libstagefright
LOCAL_SHARED_LIBRARIES += libstagefright_foundation
LOCAL_SHARED_LIBRARIES += libmmosal
LOCAL_SHARED_LIBRARIES += libwfdcommonutils

LOCAL_MODULE := libmmrtpdecoder

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif # is-board-platform
endif #is-vendor-board-platform
