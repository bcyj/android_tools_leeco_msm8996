ifneq ($(BUILD_TINY_ANDROID),true)


ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                               Common definitons
# ---------------------------------------------------------------------------------


# ---------------------------------------------------------------------------------
#                       Make the usf_epos daemon
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE            := usf_epos
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := -DLOG_NIDEBUG=0 -DLOG_NDDEBUG=0
LOCAL_C_INCLUDES        := $(LOCAL_PATH)/../../ual      \
                           $(LOCAL_PATH)/../../ual_util \
                           $(TARGET_OUT_HEADERS)/ultrasound/inc
LOCAL_SHARED_LIBRARIES  := liblog     \
                           libcutils  \
                           libutils   \
                           libgui     \
                           libual     \
                           libualutil

LOCAL_SRC_FILES         := usf_epos.cpp \
                           usf_epos_ps.cpp \
                           usf_epos_feedback_handlers.cpp \
                           usf_epos_dynamic_lib_proxy.cpp

LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

include $(call all-makefiles-under, $(LOCAL_PATH))

endif #BUILD_TINY_ANDROID
