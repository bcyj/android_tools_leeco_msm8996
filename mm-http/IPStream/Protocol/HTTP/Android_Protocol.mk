# ---------------------------------------------------------------------------------
#                 Protocol
# ---------------------------------------------------------------------------------
ROOT_DIR := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PATH := $(ROOT_DIR)

LOCAL_COPY_HEADERS_TO := mm-http/include
LOCAL_COPY_HEADERS := inc/HTTPStackInterface.h

LOCAL_CFLAGS :=             \
    -D_ANDROID_

LOCAL_SRC_FILES := src/HTTPRequest.cpp
LOCAL_SRC_FILES += src/HTTPResponse.cpp
LOCAL_SRC_FILES += src/HTTPResponseStatusHandler.cpp
LOCAL_SRC_FILES += src/HTTPStack.cpp
LOCAL_SRC_FILES += src/HTTPStackCommon.cpp
LOCAL_SRC_FILES += src/HTTPStackHelper.cpp
LOCAL_SRC_FILES += src/HTTPStackInterface.cpp
LOCAL_SRC_FILES += src/HTTPStackStateMachine.cpp
LOCAL_SRC_FILES += src/HTTPStackStateObjects.cpp
LOCAL_SRC_FILES += src/TransportConnection.cpp
LOCAL_SRC_FILES += src/TransportConnectionTcp.cpp
LOCAL_SRC_FILES += src/HTTPRequestHandler.cpp
LOCAL_SRC_FILES += src/HTTPStateInfo.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Common/Network/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Common/StreamUtils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += libmmosal
LOCAL_SHARED_LIBRARIES += libmmipstreamutils
LOCAL_SHARED_LIBRARIES += libmmipstreamnetwork
LOCAL_SHARED_LIBRARIES += liblog

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libmmhttpstack

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
