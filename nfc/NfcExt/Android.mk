ifeq ($(TARGET_USES_QCA_NFC),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_JAVA_LIBRARIES := com.android.nfc.helper

LOCAL_MODULE:= com.android.qcom.nfc_extras

include $(BUILD_JAVA_LIBRARY)

# put the classes.jar, with full class files instead of classes.dex inside, into the dist directory
$(call dist-for-goals, droidcore, $(full_classes_jar):com.android.qcom.nfc_extras.jar)

# ====  permissions ========================
include $(CLEAR_VARS)

LOCAL_MODULE := com.android.qcom.nfc_extras.xml

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_CLASS := ETC

# Install to /system/etc/permissions
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)

endif
