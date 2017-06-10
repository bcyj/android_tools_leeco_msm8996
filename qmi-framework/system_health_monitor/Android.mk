LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO   := system_health_monitor/inc
LOCAL_COPY_HEADERS      += sys_health_mon.h

common_shm_cflags := -g
common_shm_cflags += -O2
common_shm_cflags += -fno-inline
common_shm_cflags += -fno-short-enums
common_shm_cflags += -fpic
common_shm_cflags += -Wall
common_shm_cflags += -Wextra
common_shm_cflags += -Wno-unused-parameter

LOCAL_CFLAGS := $(common_shm_cflags)
LOCAL_CFLAGS += -DSHM_ANDROID

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/common/inc

LOCAL_SRC_FILES  := sys_health_mon.c

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += liblog

LOCAL_MODULE := libsystem_health_mon

LOCAL_MODULE_TAGS := optional

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
