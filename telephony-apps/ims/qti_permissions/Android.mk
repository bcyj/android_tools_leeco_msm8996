LOCAL_PATH := $(my-dir)

########################
include $(CLEAR_VARS)

LOCAL_MODULE := qti_permissions.xml

LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/etc/permissions
#
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

LOCAL_MODULE_CLASS := ETC

LOCAL_MODULE_TAGS := optional

include $(BUILD_PREBUILT)
