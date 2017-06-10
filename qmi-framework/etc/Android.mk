LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := qmi_fw.conf
LOCAL_MODULE := qmi_fw.conf
LOCAL_MODULE_TAGS := optional debug
LOCAL_MODULE_CLASS := ETC
include $(BUILD_PREBUILT)
