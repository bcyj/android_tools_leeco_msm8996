
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(call is-platform-sdk-version-at-least,18),true)
 # JB MR2 or later
 LOCAL_CFLAGS += -DANDROID_JB_MR2=1
endif

LOCAL_C_INCLUDES := \
system/kernel_headers/ \
        $(LOCAL_PATH) \
        $(TARGET_OUT_HEADERS)/qmi/inc/ \
        $(TARGET_OUT_HEADERS)/common/inc/ \
        $(TARGET_OUT_HEADERS)/libmdmdetect/inc \

LOCAL_SRC_FILES := \
        atfwd_daemon.c \
        sendcmd.cpp \
        IAtCmdFwd.cpp

LOCAL_SHARED_LIBRARIES += libcutils libutils libc libqmi libbinder libmdmdetect

LOCAL_MODULE:= ATFWD-daemon

LOCAL_MODULE_TAGS := optional

LDLIBS += -lpthread

LOCAL_MODULE_OWNER := qcom

include $(BUILD_EXECUTABLE)
