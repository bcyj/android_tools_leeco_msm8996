LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
                  $(LOCAL_PATH)/../../../../securemsm/drm/drmprov/inc \
                  $(LOCAL_PATH)/../../securemsm/tzcommon/inc \
                  $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                  $(TARGET_OUT_HEADERS)/common/inc \
                  $(LOCAL_PATH)/../../securemsm-noship/drm/widevine/inc \
                  $(LOCAL_PATH)/../../securemsm/QSEEComAPI \
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils \
        libdl \
        libQSEEComAPI \
        libdrmfs \

OEMCRYPTO_LVL-def := -D_OEMCRYPTO_L1
LOCAL_CFLAGS    := $(OEMCRYPTO_LVL-def)
LOCAL_MODULE := QHCK
LOCAL_SRC_FILES := SafeTest.c
LOCAL_MODULE_TAGS := optional
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_SHARED_LIBRARIES += $(commonSharedLibraries)
LOCAL_STATIC_LIBRARIES := liboemcrypto_L1
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)
