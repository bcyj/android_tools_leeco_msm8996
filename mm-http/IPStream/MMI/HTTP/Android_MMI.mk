# ---------------------------------------------------------------------------------
#                 MMI
# ---------------------------------------------------------------------------------

ROOT_DIR := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PATH := $(ROOT_DIR)


LOCAL_COPY_HEADERS_TO := mm-http/include
LOCAL_COPY_HEADERS := inc/HTTPMMIComponent.h
LOCAL_COPY_HEADERS += inc/HTTPSourceMMIEntry.h

LOCAL_CFLAGS :=             \
    -D_ANDROID_

LOCAL_SRC_FILES := src/HTTPDataRequestHandler.cpp
LOCAL_SRC_FILES += src/HTTPSourceMMI.cpp
LOCAL_SRC_FILES += src/HTTPSourceMMIEntry.cpp
LOCAL_SRC_FILES += src/HTTPSourceMMIHelper.cpp
LOCAL_SRC_FILES += src/HTTPSourceMMIPropertiesHandler.cpp
LOCAL_SRC_FILES += src/HTTPSourceMMIStreamPortHandler.cpp
LOCAL_SRC_FILES += src/HTTPSourceMMITrackHandler.cpp
LOCAL_SRC_FILES += src/HTTPSourceMMIExtensionHandler.cpp
LOCAL_SRC_FILES += src/HTTPSourceMMIExtensionEventHandler.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Source/HTTP/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Common/StreamUtils/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Protocol/HTTP/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Common/Network/inc
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-parser/FileSource/inc
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-parser/HTTPSource/MAPI08/API
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/omxcore
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-qsm/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/mmi
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../opensource/time-services

LOCAL_SHARED_LIBRARIES := libutils
LOCAL_SHARED_LIBRARIES += libmmosal
LOCAL_SHARED_LIBRARIES += libmmipstreamutils
LOCAL_SHARED_LIBRARIES += libmmparser
LOCAL_SHARED_LIBRARIES += libstagefright
LOCAL_SHARED_LIBRARIES += libmmipstreamsourcehttp
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libtime_genoff

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libmmiipstreammmihttp

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

