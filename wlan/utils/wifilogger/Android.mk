BUILD_WIFI_LOGGER_APP:=1

ifeq ($(BUILD_WIFI_LOGGER_APP),1)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES += \
	wifiLogger.c

LOCAL_CFLAGS += \
	-fno-short-enums

#LOCAL_CFLAGS += \
	-DANI_DEBUG

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../asf/inc
LOCAL_C_INCLUDES += $(call include-path-for, system-core)/cutils
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

$(info $(LOCAL_C_INCLUDES))
LOCAL_SHARED_LIBRARIES := \
	libutils \
	libc \
	liblog \
	libcutils \
	libdiag

LOCAL_STATIC_LIBRARIES := \
	libAniAsf

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := WifiLogger_app
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

endif
