# ---------------------------------------------------------------------------------
#                 Network
# ---------------------------------------------------------------------------------
ROOT_DIR := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PATH := $(ROOT_DIR)

LOCAL_COPY_HEADERS_TO := mm-http/include
LOCAL_COPY_HEADERS := inc/StreamNetwork.h
LOCAL_COPY_HEADERS += inc/dsbsd.h

LOCAL_CFLAGS :=             \
    -D_ANDROID_

LOCAL_SRC_FILES := src/dsbsd.cpp
LOCAL_SRC_FILES += src/StreamNetwork.cpp
LOCAL_SRC_FILES += src/StreamNetworkBSD.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../StreamUtils/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_SHARED_LIBRARIES := libmmosal
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += liblog

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libmmipstreamnetwork

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
