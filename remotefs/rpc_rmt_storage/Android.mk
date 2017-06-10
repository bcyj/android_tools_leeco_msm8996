LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= rmt_storage.c

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_SHARED_LIBRARIES := libcutils

LOCAL_MODULE:= rmt_storage

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -D_LARGEFILE64_SOURCE \
		-DLOG_NIDEBUG=0

LDLIBS += -lpthread
include $(BUILD_EXECUTABLE)

recovery_rmt_storage_binary := $(call intermediates-dir-for,EXECUTABLES,rmt_storage)/rmt_storage
