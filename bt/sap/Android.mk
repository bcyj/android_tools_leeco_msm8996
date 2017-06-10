LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# Logging Features. Enable only one at any time
#LOCAL_CFLAGS += -DFEATURE_DATA_LOG_SYSLOG
LOCAL_CFLAGS += -DFEATURE_DATA_LOG_ADB
#LOCAL_CFLAGS += -DFEATURE_DATA_LOG_QXDM

# Additional Logging
# Uncomment for VERBOSE level messages
LOCAL_CFLAGS += -DLOG_NDEBUG=0

# Additional Logging
# Uncomment for DEBUG level messages
LOCAL_CFLAGS += -DLOG_NDDEBUG=0

# Additional Logging
# Uncomment for INFO level messages
LOCAL_CFLAGS += -DLOG_NIDEBUG=0
LOCAL_CFLAGS += -Wno-psabi -Wno-write-strings -DANDROID_NDK -DTARGET_ANDROID -DLINUX -DQCC_OS_GROUP_POSIX -DQCC_OS_ANDROID -DQCC_CPU_ARM -DANDROID
# TODO: Below flags will be enabled once alljoyn
# dependant headers are cleaned up.
# there are no warnings in this module,
#LOCAL_CFLAGS += -Werror -Wall -Wextra

LOCAL_DEFAULT_CPP_EXTENSION := cc
LOCAL_SRC_FILES:= \
	src/sap_server.c \
	src/sap_routines.c \
	src/qmi_uim_handler.c

LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc

LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libidl
LOCAL_SHARED_LIBRARIES += libqmi libqcci_legacy libqmiservices libqmi_client_qmux
LOCAL_MODULE:= sapd

LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)
