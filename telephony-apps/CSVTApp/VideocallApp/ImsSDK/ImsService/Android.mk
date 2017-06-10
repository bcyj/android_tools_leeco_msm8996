#/******************************************************************************
#*@file Android.mk
#*brief Rules for compiling the source files
#*******************************************************************************/
LOCAL_PATH:= $(call my-dir)


# ==========================================================================
# Build the service
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional debug

LOCAL_PACKAGE_NAME := imssdkservice
LOCAL_CERTIFICATE := platform
LOCAL_PROGUARD_ENABLED := disabled

LOCAL_JAVA_LIBRARIES := imslibrary core framework telephony-common voip-common telephony-msim com.android.services.telephony.common

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_SRC_FILES += src/com/qualcomm/qti/ims/internal/telephony/IImsCallService.aidl
LOCAL_SRC_FILES += src/com/qualcomm/qti/ims/internal/telephony/IImsCallServiceListener.aidl


include $(BUILD_PACKAGE)
