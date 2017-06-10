
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := com.quicinc.wbcservice
LOCAL_SRC_FILES := $(call all-java-files-under,java)
LOCAL_JAVA_LIBRARIES := com.quicinc.wbc
include $(BUILD_JAVA_LIBRARY)

include $(CLEAR_VARS)
include $(LOCAL_PATH)/jni/Android.mk

