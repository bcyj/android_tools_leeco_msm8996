ifeq ($(NFC_D), true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
#LOCAL_JAVA_LIBRARIES := com.android.qcom.nfc_extras
LOCAL_SRC_FILES :=$(call all-java-files-under, src)

LOCAL_JAVA_LIBRARIES := com.android.nfc.helper

LOCAL_MODULE := com.vzw.nfc
include $(BUILD_JAVA_LIBRARY)

# put the classes.jar, with full class files instead of classes.dex inside, into the dist directory
$(call dist-for-goals, droidcore, $(full_classes_jar):com.vzw.nfc.jar)

include $(CLEAR_VARS)
LOCAL_MODULE := com.vzw.nfc.xml
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
# Install to /system/etc/permissions
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

endif

