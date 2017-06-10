LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../QSEEComAPI \
                    $(LOCAL_PATH)/../../../../sse/SecureUILib \
                    $(LOCAL_PATH)/../../../SecureUI \
                    $(TARGET_OUT_HEADERS)/common/inc

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES := \
        libcutils \
        libutils \
        libQSEEComAPI \
        libsecureui_svcsock \
        libSecureUILib \

LOCAL_MODULE := secure_ui_sample_client
LOCAL_SRC_FILES := secure_ui_sample_client.c
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := $(QSEECOM_CFLAGS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)
