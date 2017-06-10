LOCAL_PATH := $(call my-dir)

# ---------------------------------------
include $(CLEAR_VARS)

LOCAL_SRC_FILES :=          \
    src/wifi_ftm.cpp

LOCAL_C_INCLUDES :=         \
    $(LOCAL_PATH)/include

LOCAL_CFLAGS += -DPLATFORM_ANDROID


LOCAL_SHARED_LIBRARIES +=   \
    libutils\
    libcutils

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libwifi_ftm

include $(BUILD_SHARED_LIBRARY)


# ------------------------------------------------
include $(CLEAR_VARS)

LOCAL_SRC_FILES :=      \
    src/wifi_ftm_jni.cpp

LOCAL_C_INCLUDES :=         \
    $(LOCAL_PATH)/include   \
    $(JNI_H_INCLUDE) \
    libcore/include \
    frameworks/base/include

LOCAL_CFLAGS += -DPLATFORM_ANDROID

LOCAL_SHARED_LIBRARIES +=   \
    libutils                \
    libcutils                \
    libnativehelper \
    libwifi_ftm

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libwifi_ftm_jni

include $(BUILD_SHARED_LIBRARY)


# ---------------------------------------
include $(CLEAR_VARS)

LOCAL_SRC_FILES :=          \
    src/WifiFtmController.cpp  \
    src/WifiFtmTest.cpp       \
    src/WifiFtmd.cpp

LOCAL_C_INCLUDES :=         \
    $(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES := libcutils libc

LOCAL_MODULE = wifi_ftmd
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)


