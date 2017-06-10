LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := cnss-daemon
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_HEADERS)/qmi-framework/inc \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(TARGET_OUT_HEADERS)/qmi-framework/qcci/inc

LOCAL_SRC_FILES := \
	main.c \
	nl_loop.c \
	debug.c \
	cnss_qmi_client.c \
	wireless_lan_proxy_service_v01.c

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libqmi_cci \
	libqmi_common_so

LOCAL_CFLAGS += \
	-DCONFIG_DEBUG \
	-DCONFIG_DEBUG_LOGCAT

include $(BUILD_EXECUTABLE)
