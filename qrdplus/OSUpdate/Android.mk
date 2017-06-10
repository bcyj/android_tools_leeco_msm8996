ifeq ($(strip $(TARGET_USES_QTIC_OSUPDATE)),true)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional eng

LOCAL_SRC_FILES := $(call all-java-files-under,src)

# TODO: Remove dependency of application on the test runner (android.test.runner)
# library.
LOCAL_JAVA_LIBRARIES := android.test.runner

LOCAL_PACKAGE_NAME := QRDUpdate

LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)

# Use the following include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
