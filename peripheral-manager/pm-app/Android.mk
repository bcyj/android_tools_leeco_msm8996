#ifeq ($(QCOM_PM_TEST_ENABLE),1)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS += -D_ANDROID_
LOCAL_SRC_FILES := pm-app.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../
LOCAL_SHARED_LIBRARIES := libperipheral_client libcutils
LOCAL_MODULE := pm-app
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti

include $(BUILD_EXECUTABLE)

#endif
