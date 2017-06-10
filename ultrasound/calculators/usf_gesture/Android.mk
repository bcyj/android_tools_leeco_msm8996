ifneq ($(BUILD_TINY_ANDROID),true)


ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                               Common definitons
# ---------------------------------------------------------------------------------


# ---------------------------------------------------------------------------------
#                       Make the usf_gesture daemon
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE            := usf_gesture
LOCAL_MODULE_TAGS       := optional
LOCAL_C_INCLUDES        := $(LOCAL_PATH)/../../ual \
                           $(LOCAL_PATH)/../../ual_util \
                           $(LOCAL_PATH)/../stubs \
                           $(LOCAL_PATH)/../../adapter
LOCAL_SHARED_LIBRARIES  := liblog \
                           libcutils \
                           libual \
                           libualutil \
                           libqcgesture \
                           libdl

LOCAL_CFLAGS            := -DLOG_NIDEBUG=0 -DLOG_NDDEBUG=0
LOCAL_SRC_FILES         := usf_gesture.cpp \
                           ../../adapter/us_adapter_factory.cpp

LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

include $(call all-makefiles-under, $(LOCAL_PATH))

endif #BUILD_TINY_ANDROID
