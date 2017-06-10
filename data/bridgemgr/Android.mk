LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

# Logging Features. Enable only one at any time
#LOCAL_CFLAGS += -DFEATURE_DATA_LOG_STDERR
#LOCAL_CFLAGS += -DFEATURE_DATA_LOG_SYSLOG
#LOCAL_CFLAGS += -DFEATURE_DATA_LOG_ADB
#LOCAL_CFLAGS += -DFEATURE_DATA_INTERNAL_LOG_TO_FILE
LOCAL_CFLAGS += -DFEATURE_DATA_LOG_QXDM

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../dsutils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include

LOCAL_LDLIBS += -lpthread

LOCAL_SHARED_LIBRARIES := \
   libdsutils             \
   libdiag                \
   liblog                 \
   libqmi

LOCAL_SRC_FILES :=        \
   bridgemgr.c            \
   bridgemgr_cmdq.c       \
   bridgemgr_common.c     \
   bridgemgr_usb_qmi.c    \
   bridgemgr_mdm_qmi.c    \
   bridgemgr_qmi_proxy.c  \
   bridgemgr_port_switch.c

LOCAL_MODULE := bridgemgrd
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom

#include $(BUILD_EXECUTABLE)
