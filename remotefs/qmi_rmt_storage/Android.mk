LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := rmt_storage

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi-framework/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES  := remote_storage_v01.c rmt_storage_svc.c

LOCAL_SHARED_LIBRARIES := libqmi_csi libqmi_common_so libcutils

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -D_LARGEFILE64_SOURCE
LOCAL_CFLAGS += -DLOG_NIDEBUG=0

# Remove linking the lpthreads library. lpthreads is included by
# default and explicitly including can cause compiler errors.
#
# Uncomment line below if
# building on very old builds that require explicitly including the lib.
#
# LDLIBS += -lpthread

include $(BUILD_EXECUTABLE)

recovery_rmt_storage_binary := $(call intermediates-dir-for,EXECUTABLES,rmt_storage)/rmt_storage
