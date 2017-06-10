LOCAL_PATH:= $(call my-dir)

#=============================================
#  DigitalPenSDK
#=============================================
include $(CLEAR_VARS)

LOCAL_MODULE := DigitalPenSDK
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src) \
                   $(call all-Iaidl-files-under, src)

LOCAL_PROGUARD_ENABLED := disabled

# the library
# ============================================================
LOCAL_MODULE_OWNER := qcom
include $(BUILD_STATIC_JAVA_LIBRARY)

# Include this library in the build server's output directory
$(call dist-for-goals, dist_files, $(LOCAL_BUILT_MODULE):DigitalPenSDK.jar)
