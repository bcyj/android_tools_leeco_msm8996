LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_USES_QCA_NFC),true)

# NFCEE access control
ifeq ($(TARGET_BUILD_VARIANT),user)
    include $(CLEAR_VARS)
    LOCAL_MODULE       := nfcee_access.xml
    LOCAL_MODULE_TAGS  := optional
    LOCAL_MODULE_CLASS := ETC
    LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)
    LOCAL_SRC_FILES    := nfcee_access.xml
    include $(BUILD_PREBUILT)
else
    include $(CLEAR_VARS)
    LOCAL_MODULE       := nfcee_access.xml
    LOCAL_MODULE_TAGS  := optional
    LOCAL_MODULE_CLASS := ETC
    LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)
    LOCAL_SRC_FILES    := nfcee_access_debug.xml
    include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE       := nfc-nci.conf
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := hardfault.cfg
LOCAL_MODULE_TAGS  := eng
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/nfc
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)


endif
