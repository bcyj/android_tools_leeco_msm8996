ifeq ($(TARGET_ARCH),arm)
ifeq ($(call is-board-platform-in-list,msm8960 msm8974 apq8084),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libmdmdload
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc/
LOCAL_SRC_FILES := comm.c common.c crc.c dload_protocol.c streaming_protocol.c hdlc.c log.c mdm_file_transfer_interface.c
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := mdmfiletransfer
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc/
LOCAL_SRC_FILES := comm.c common.c crc.c dload_protocol.c streaming_protocol.c hdlc.c log.c mdmfiletransfer.c
LOCAL_SHARED_LIBRARIES := libc libcutils libutils
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -Wall
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

endif
endif
