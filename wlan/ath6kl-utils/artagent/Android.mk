LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := artagent

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/libtcmd \

LOCAL_CFLAGS+=

LOCAL_SRC_FILES:= \ artagent.c

LOCAL_LDLIBS += -lpthread -lrt

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libnl_2
LOCAL_STATIC_LIBRARIES += libtcmd

LOCAL_MODULE_OWNER := qcom

include $(BUILD_EXECUTABLE)
