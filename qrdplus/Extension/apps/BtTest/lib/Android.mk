LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=      \
    src/bt_jni.cpp

LOCAL_C_INCLUDES :=         \
    $(LOCAL_PATH)/include   \
    $(JNI_H_INCLUDE) \
    libcore/include \
    frameworks/base/include

LOCAL_CFLAGS += -DPLATFORM_ANDROID

ifeq ($(BOARD_HAVE_BLUETOOTH_QCOM),true)
LOCAL_CFLAGS += -DQCOM_BLUETOOTH
endif

LOCAL_SHARED_LIBRARIES +=   \
    libutils                \
    libcutils                \
    libnativehelper         \
    libhardware

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libbt_jni

include $(BUILD_SHARED_LIBRARY)
