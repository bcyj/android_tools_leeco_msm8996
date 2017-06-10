LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES:= \
    ims_camera_jni.cpp \
    jni_main.cpp \

LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE) \

LOCAL_SHARED_LIBRARIES := \
    libnativehelper \
    libcutils \
    libutils  \
    libdl \

LOCAL_CFLAGS += -O0 -g
LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE := libimscamera_jni
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_SHARED_LIBRARY)

