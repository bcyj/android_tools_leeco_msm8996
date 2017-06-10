LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES:= \
    videophone_ims_jni.cpp \
    videophone.cpp \


LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE) \

LOCAL_SHARED_LIBRARIES := \
    libnativehelper \
    libcutils \
    libutils  \
    libdl \

LOCAL_CFLAGS += -O0 -g

LOCAL_MODULE := libsdkvt_jni
LOCAL_MODULE_TAGS := optional debug
LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_SHARED_LIBRARY)
