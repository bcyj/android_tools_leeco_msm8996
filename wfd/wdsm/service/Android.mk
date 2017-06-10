LOCAL_PATH := $(call my-dir)

#=============================================
#  Wfd Common lib for service and client
#=============================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := src/com/qualcomm/wfd/WfdEnums.java \
                src/com/qualcomm/wfd/WfdStatus.java \
                src/com/qualcomm/wfd/WfdDevice.java \
                src/com/qualcomm/wfd/service/ISessionManagerService.aidl \
                src/com/qualcomm/wfd/service/IWfdActionListener.aidl \
                src/com/qualcomm/wfd/service/IHIDEventListener.aidl

ifeq ("$(PLATFORM_VERSION)","4.3")
LOCAL_SRC_FILES += src/com/qualcomm/compat/Old/ExtendedRemoteDisplay.java
else
LOCAL_SRC_FILES += src/com/qualcomm/compat/New/ExtendedRemoteDisplay.java
endif
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := WfdCommon

include $(BUILD_JAVA_LIBRARY)

#=============================================
#  Wfd Service APK
#=============================================
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src/com/qualcomm/wfd) \
                src/com/qualcomm/compat/VersionedInputManager.java \
                src/com/qualcomm/wfd/service/ISessionManagerService.aidl \
                src/com/qualcomm/wfd/service/IWfdActionListener.aidl \
                src/com/qualcomm/wfd/service/IHIDEventListener.aidl

ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_SRC_FILES += src/com/qualcomm/compat/IcsInputManager.java
else
LOCAL_SRC_FILES += src/com/qualcomm/compat/JbInputManager.java
endif

LOCAL_PACKAGE_NAME := WfdService

LOCAL_CERTIFICATE := platform

LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
