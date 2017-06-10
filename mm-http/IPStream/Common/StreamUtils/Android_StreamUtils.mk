# ---------------------------------------------------------------------------------
#                 StreamUtils
# ---------------------------------------------------------------------------------
ROOT_DIR := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PATH := $(ROOT_DIR)

include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO := mm-http/include
LOCAL_COPY_HEADERS := inc/DeepMap.h
LOCAL_COPY_HEADERS += inc/DefaultTrackSelectionPolicy.h
LOCAL_COPY_HEADERS += inc/EventNotifierRegistry.h
LOCAL_COPY_HEADERS += inc/IPStreamProtocolHeaders.h
LOCAL_COPY_HEADERS += inc/HTTPCookieStore.h
LOCAL_COPY_HEADERS += inc/IPStreamSourceUtils.h
LOCAL_COPY_HEADERS += inc/IReferenceCountable.h
LOCAL_COPY_HEADERS += inc/ITrackList.h
LOCAL_COPY_HEADERS += inc/ITrackSelectionPolicy.h
LOCAL_COPY_HEADERS += inc/ReferenceCountedPointer.h
LOCAL_COPY_HEADERS += inc/Scheduler.h
LOCAL_COPY_HEADERS += inc/SourceMemDebug.h
LOCAL_COPY_HEADERS += inc/StreamDataQueue.h
LOCAL_COPY_HEADERS += inc/StreamHash.h
LOCAL_COPY_HEADERS += inc/StreamMediaHelper.h
LOCAL_COPY_HEADERS += inc/StreamQueue.h
LOCAL_COPY_HEADERS += inc/StreamSourceClock.h
LOCAL_COPY_HEADERS += inc/StreamSourceTimeUtils.h
LOCAL_COPY_HEADERS += inc/Streamlist.h
LOCAL_COPY_HEADERS += inc/TrackGrouping.h
LOCAL_COPY_HEADERS += inc/TrackList.h
LOCAL_COPY_HEADERS += inc/Url.h
LOCAL_COPY_HEADERS += inc/deeplist.h
LOCAL_COPY_HEADERS += inc/oscl_string.h
LOCAL_COPY_HEADERS += inc/oscl_string_utils.h
LOCAL_COPY_HEADERS += inc/oscl_types.h
LOCAL_COPY_HEADERS += inc/qtv_msg.h

LOCAL_CFLAGS :=             \
    -D_ANDROID_

LOCAL_SRC_FILES := src/DefaultTrackSelectionPolicy.cpp
LOCAL_SRC_FILES += src/EventNotifierRegistry.cpp
LOCAL_SRC_FILES += src/IPStreamProtocolHeaders.cpp
LOCAL_SRC_FILES += src/HTTPCookieStore.cpp
LOCAL_SRC_FILES += src/oscl_string_utils.cpp
LOCAL_SRC_FILES += src/Scheduler.cpp
LOCAL_SRC_FILES += src/StreamHash.cpp
LOCAL_SRC_FILES += src/Streamlist.cpp
LOCAL_SRC_FILES += src/StreamMediaHelper.cpp
LOCAL_SRC_FILES += src/StreamQueue.cpp
LOCAL_SRC_FILES += src/StreamSourceClock.cpp
LOCAL_SRC_FILES += src/StreamSourceTimeUtils.cpp
LOCAL_SRC_FILES += src/TrackList.cpp
LOCAL_SRC_FILES += src/Url.cpp


LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../Network/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_SHARED_LIBRARIES := libmmosal
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += liblog

LOCAL_MODULE_TAGS:= optional

LOCAL_MODULE:= libmmipstreamutils

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
