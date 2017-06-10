LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifndef NO_GSIFF_DSPS
FEATURE_GSIFF_DSPS = 1
endif

$(call print-vars, LOCAL_PATH)

LOCAL_MODULE := slim_ap_daemon

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qti

LOCAL_PROPRIETARY_MODULE := true

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/apss/osal/ \
    $(LOCAL_PATH)/apss/listener \
    $(LOCAL_PATH)/apss/wrapper \
    $(LOCAL_PATH)/apss/daemon/ \
    $(LOCAL_PATH)/provider/common \
    $(LOCAL_PATH)/provider/ndk \
    $(LOCAL_PATH)/../daemon \
    $(TARGET_OUT_HEADERS)/common/inc \
    $(TARGET_OUT_HEADERS)/qmi-framework/inc \
    $(TARGET_OUT_HEADERS)/libloc_ds_api \
    $(TARGET_OUT_HEADERS)/gps.utils \
    $(TARGET_OUT_HEADERS)/libslimcommon \
    $(TARGET_OUT_HEADERS)/libslimclient \
    $(TARGET_OUT_HEADERS)/libloc_api_v02 \
    $(TARGET_OUT_HEADERS)/libloc_core \
    $(TARGET_OUT_HEADERS)/liblbs_core \
    $(TOP)/frameworks/base/native/include \
    $(TOP)/vendor/qcom/proprietary/diag/include \

LOCAL_SRC_FILES:= \
    apss/daemon/SlimDaemonManager.cpp \
    apss/listener/ClientListener.cpp \
    apss/listener/QLClientListener.cpp \
    apss/listener/SocketClientListener.cpp \
    apss/wrapper/SocketClientWrapper.cpp \
    provider/ndk/SlimNDKProvider.c \
    provider/ndk/SlimNDKProviderWrapper.cpp \
    provider/common/SlimProviderCommon.c \
    provider/common/slim_provider_conf.c \
    apss/osal/slim_internal.c \
     ../daemon/gpsone_thread_helper.c \
    provider/common/SlimProviderTimeSyncFilter.c


LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libgps.utils \
    liblog \
    libloc_api_v02 \
    libloc_core \
    liblbs_core \
    libandroid \
    libdiag

LOCAL_STATIC_LIBRARIES := \
    libslimcommon

LOCAL_COPY_HEADERS_TO:= libslimapdaemon/
LOCAL_COPY_HEADERS := \
    apss/osal/slim_processor.h \
    apss/wrapper/SocketClientWrapper.h \
    apss/listener/SocketClientTypes.h

LOCAL_CFLAGS+=$(GPS_FEATURES) \
    -D_ANDROID_ \
    -DON_TARGET_TEST \

ifeq ($(FEATURE_GSIFF_DSPS),1)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/provider/sensor1
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/sensors/inc

LOCAL_SRC_FILES += provider/sensor1/SlimSensor1Provider.c
LOCAL_SRC_FILES += provider/sensor1/SlimSensor1ProviderWrapper.cpp

LOCAL_SHARED_LIBRARIES += libsensor1

LOCAL_CFLAGS           += -DFEATURE_GSIFF_DSPS
endif

LOCAL_PRELINK_MODULE := false
include $(BUILD_EXECUTABLE)

#




