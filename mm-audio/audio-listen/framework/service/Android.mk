ifeq ($(strip $(AUDIO_FEATURE_ENABLED_LISTEN)),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS += -DAUDIO_LISTEN_ENABLED -DLOG_NDEBUG

LOCAL_SRC_FILES:= \
    src/IListenReceiver.cpp \
    src/IListenDeathNotifier.cpp \
    src/IListenService.cpp \
    src/IListenClientDeathNotifier.cpp \
    src/ListenService.cpp \
    src/ListenReceiver.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils libutils libbinder libhardware

LOCAL_MODULE:= liblisten
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/inc \
    $(TOP)/hardware/libhardware/include \
    $(TOP)/vendor/qcom/proprietary/mm-audio/audio-listen/hal/inc

LOCAL_COPY_HEADERS_TO   := mm-audio/audio-listen
LOCAL_COPY_HEADERS      := \
    inc/ListenService.h \
    inc/ListenNativeTypes.h \
    inc/IListenService.h \
    inc/IListenDeathNotifier.h \
    inc/IListenClientDeathNotifier.h \
    inc/IListenReceiver.h \
    inc/ListenReceiver.h

include $(BUILD_SHARED_LIBRARY)

endif
