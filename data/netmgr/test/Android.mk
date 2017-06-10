# sources and intermediate files are separated

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

# Logging Features. Enable only one at any time
LOCAL_CFLAGS += -DFEATURE_DATA_LOG_STDERR
#LOCAL_CFLAGS += -DFEATURE_DATA_LOG_SYSLOG
#LOCAL_CFLAGS += -DFEATURE_DATA_LOG_ADB
#LOCAL_CFLAGS += -DFEATURE_DATA_LOG_QXDM

LOCAL_CFLAGS += -DFEATURE_DS_LINUX_NO_RPC
LOCAL_CFLAGS += -DFEATURE_DS_LINUX_ANDROID


LOCAL_C_INCLUDES := system/core/libnetutils/
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../src/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qmi/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qmi/platform
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../dsutils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include

LOCAL_SRC_FILES := \
	nl_listener.c

LOCAL_MODULE := nl_listener
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
	libdsutils  \
	libqmi  \
	liblog  \
	libdiag  \
	libnetutils \
	libnetmgr

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom

include $(BUILD_EXECUTABLE)
