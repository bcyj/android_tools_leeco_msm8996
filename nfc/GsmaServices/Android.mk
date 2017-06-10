ifeq ($(TARGET_USES_QCA_NFC),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/gsmaapi/src/com/gsma/services/nfc/IGsmaService.aidl \
                       $(LOCAL_PATH)/gsmaapi/src/com/gsma/services/nfc/IGsmaServiceCallbacks.aidl

LOCAL_PACKAGE_NAME := GsmaNfcService
LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES := com.android.nfc_extras com.android.qcom.nfc_extras com.gsma.services.nfc

LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_PACKAGE)

# ====  permissions ========================
include $(CLEAR_VARS)

LOCAL_MODULE := com.gsma.services.nfc.xml

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_CLASS := ETC

# Install to /system/etc/permissions
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE := com.gsma.services.utils.xml

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_CLASS := ETC

# Install to /system/etc/permissions
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)

include $(call all-makefiles-under,$(LOCAL_PATH))

endif
