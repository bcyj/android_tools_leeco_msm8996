ifeq ($(TARGET_USES_QCA_NFC),true)
LOCAL_PATH:= $(call my-dir)

########################################
# com.android.nfc.helper - library
########################################
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := com.android.nfc.helper
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
 # Install to system/frameworks/lib
LOCAL_MODULE_PATH := $(TARGET_OUT_JAVA_LIBRARIES)

LOCAL_SRC_FILES += \
        android/nfc/IGetNFCByteArray.aidl \
        android/nfc/dta/INfcTagDta.aidl \
        android/nfc/dta/IDtaHelper.aidl \
        qcom/nfc/IQNfcExtras.aidl \
        qcom/nfc/IQNfcSecureElementManager.aidl \
        qcom/nfc/IQNfcSecureElementManagerCallbacks.aidl \
        $(call all-java-files-under, android) \
        $(call all-java-files-under, qcom)

LOCAL_CERTIFICATE := platform

include $(BUILD_JAVA_LIBRARY)
# ====  permissions ========================
include $(CLEAR_VARS)

LOCAL_MODULE := com.android.nfc.helper.xml

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_CLASS := ETC

# Install to /system/etc/permissions
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions

LOCAL_SRC_FILES := android/$(LOCAL_MODULE)

include $(BUILD_PREBUILT)

# the documentation
# ============================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-subdir-java-files) $(call all-subdir-html-files)

LOCAL_MODULE:= com.android.nfc.helper.doc
LOCAL_DROIDDOC_OPTIONS := com.android.nfc.helper
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_DROIDDOC_USE_STANDARD_DOCLET := true

include $(BUILD_DROIDDOC)

########################################
# NCI Configuration
########################################
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES += \
        $(call all-java-files-under, src)

LOCAL_SRC_FILES += \
        $(call all-java-files-under, nci)

LOCAL_RESOURCE_DIR := packages/apps/Nfc/res

LOCAL_PACKAGE_NAME := QNfc
LOCAL_OVERRIDES_PACKAGES := Nfc
LOCAL_CERTIFICATE := platform

LOCAL_STATIC_JAVA_LIBRARIES := QNfcLogTags
LOCAL_JAVA_LIBRARIES := com.android.nfc.helper

LOCAL_JNI_SHARED_LIBRARIES := libqnfc_nci_jni
LOCAL_REQUIRED_MODULES  := com.android.nfc.helper

LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_PACKAGE)

#####
# static lib for the log tags
#####
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := src/com/android/nfc/EventLogTags.logtags

LOCAL_MODULE:= QNfcLogTags
LOCAL_OVERRIDES_PACKAGE := NfcLogTags

include $(BUILD_STATIC_JAVA_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
endif
