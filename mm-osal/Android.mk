
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO := mm-osal/include
LOCAL_COPY_HEADERS := inc/AEEStdDef.h
LOCAL_COPY_HEADERS += inc/AEEstd.h
LOCAL_COPY_HEADERS += inc/AEEVaList.h
LOCAL_COPY_HEADERS += inc/MMCriticalSection.h
LOCAL_COPY_HEADERS += inc/MMDebugMsg.h
LOCAL_COPY_HEADERS += inc/MMFile.h
LOCAL_COPY_HEADERS += inc/MMMalloc.h
LOCAL_COPY_HEADERS += inc/MMMemory.h
LOCAL_COPY_HEADERS += inc/MMSignal.h
LOCAL_COPY_HEADERS += inc/MMThread.h
LOCAL_COPY_HEADERS += inc/MMTime.h
LOCAL_COPY_HEADERS += inc/MMTimer.h

LOCAL_CFLAGS := \
    -D_ANDROID_

LOCAL_SRC_FILES:=       \
     ./src/MMMalloc.c \
     ./src/MMCriticalSection.c \
     ./src/MMDebug.c \
     ./src/MMTimer.c \
     ./src/MMThread.c \
     ./src/MMSignal.c \
     ./src/MMFile.c  \
     ./src/AEEstd.c

LOCAL_C_INCLUDES:= \
    $(LOCAL_PATH)/./inc \
    $(LOCAL_PATH)/../common/inc  \
    $(LOCAL_PATH)/../../opensource/time-services

LOCAL_SHARED_LIBRARIES := \
    libutils  \
    libcutils \
    libtime_genoff

LOCAL_PRELINK_MODULE:= false

LOCAL_MODULE:= libmmosal

LOCAL_MODULE_TAGS := optional

LOCAL_LDLIBS +=
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

