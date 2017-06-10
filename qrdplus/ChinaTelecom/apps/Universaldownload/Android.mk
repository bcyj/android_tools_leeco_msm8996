LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src) \
	src/com/qualcomm/universaldownload/IDownloadListener.aidl \
	src/com/qualcomm/universaldownload/IDownloadService.aidl

LOCAL_JAVA_LIBRARIES := telephony-common qcrilhook
LOCAL_PACKAGE_NAME := CtUniversalDownload
LOCAL_CERTIFICATE := platform
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/ChinaTelecom/system/app

include $(BUILD_PACKAGE)
