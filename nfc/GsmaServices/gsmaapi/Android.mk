ifeq ($(NFC_D), true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES :=$(call all-java-files-under, src)
LOCAL_SRC_FILES += src/com/gsma/services/nfc/IGsmaService.aidl \
                   src/com/gsma/services/nfc/IGsmaServiceCallbacks.aidl

LOCAL_MODULE := com.gsma.services.nfc
include $(BUILD_JAVA_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_JAVA_LIBRARIES := com.gsma.services.nfc
LOCAL_SRC_FILES := src/com/gsma/services/utils/Handset.java
LOCAL_MODULE := com.gsma.services.utils
include $(BUILD_JAVA_LIBRARY)

# put the classes.jar, with full class files instead of classes.dex inside, into the dist directory
$(call dist-for-goals, droidcore, $(full_classes_jar):com.gsma.services.nfc.jar com.gsma.services.utils.jar)

endif
