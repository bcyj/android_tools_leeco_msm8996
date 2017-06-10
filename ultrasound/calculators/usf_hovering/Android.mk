ifneq ($(BUILD_TINY_ANDROID),true)


ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                               Common definitons
# ---------------------------------------------------------------------------------


# ---------------------------------------------------------------------------------
#                       Make the usf_hovering daemon
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE            := usf_hovering
LOCAL_MODULE_TAGS       := optional
LOCAL_C_INCLUDES        := $(LOCAL_PATH)/../../ual \
                           $(LOCAL_PATH)/../../ual_util \
                           $(LOCAL_PATH)/../stubs
LOCAL_SHARED_LIBRARIES  := liblog \
                           libcutils \
                           libual \
                           libualutil \
                           libqchovering
LOCAL_SRC_FILES         := usf_hovering.cpp

LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)


endif #BUILD_TINY_ANDROID
