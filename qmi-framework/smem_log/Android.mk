LOCAL_PATH :=$(call my-dir)

include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO   := smem_log/inc
LOCAL_COPY_HEADERS      := smem_log.h

common_libsmem_log_cflags := -g
common_libsmem_log_cflags += -O2
common_libsmem_log_cflags += -fpic
common_libsmem_log_cflags += -Wall
common_libsmem_log_cflags += -Werror
common_libsmem_log_cflags += -Wno-unused-parameter

LOCAL_CFLAGS := $(common_libsmem_log_cflags)


LOCAL_C_INCLUDES := smem_log.h
LOCAL_SRC_FILES  := smem_log.c


LOCAL_MODULE := libsmemlog

LOCAL_MODULE_TAGS := optional

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

