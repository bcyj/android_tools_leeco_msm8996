LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# Logging Features. Enable only one at any time
#LOCAL_CFLAGS += -DFEATURE_DATA_LOG_SYSLOG
LOCAL_CFLAGS += -DFEATURE_DATA_LOG_ADB
#LOCAL_CFLAGS += -DFEATURE_DATA_LOG_QXDM

# Additional Logging
# Uncomment for VERBOSE level messages
#LOCAL_CFLAGS += -DLOG_NDEBUG=0

# Additional Logging
# Uncomment for DEBUG level messages
LOCAL_CFLAGS += -DLOG_NDDEBUG=0

# Additional Logging
# Uncomment for INFO level messages
LOCAL_CFLAGS += -DLOG_NIDEBUG=0
LOCAL_CFLAGS += -Wno-psabi -Wno-write-strings -DANDROID_NDK -DTARGET_ANDROID -DLINUX -DQCC_OS_GROUP_POSIX -DQCC_OS_ANDROID -DQCC_CPU_ARM -DANDROID

LOCAL_DEFAULT_CPP_EXTENSION := cc
LOCAL_SRC_FILES:= \
    portbridge_common.c \
    portbridge_core_SM.c \
    portbridge_core_xfer.c \
    portbridge_ext_host_mon.c \
    platform_call_arb_SM.c \
    platform_call_arb_kevents.c



LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/data/inc/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/command-svc

LOCAL_SHARED_LIBRARIES := libdsutils libutils libcutils liblog libdiag libCommandSvc

LOCAL_MODULE:= dun-server
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
