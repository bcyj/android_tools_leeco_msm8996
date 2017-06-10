ifneq ($(BUILD_TINY_ANDROID),true)


ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                               Common definitons
# ---------------------------------------------------------------------------------


# ---------------------------------------------------------------------------------
#                       Make the Shared library (libualutil)
# ---------------------------------------------------------------------------------

LOCAL_MODULE            := libualutil
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := -DLOG_NIDEBUG=0
ifeq ($(call is-android-codename,JELLY_BEAN),true)
LOCAL_CFLAGS            += -DJELLY_BEAN
endif
LOCAL_C_INCLUDES        := $(LOCAL_PATH)/../ual \
                           $(TARGET_OUT_HEADERS)/ultrasound/inc \
                           $(LOCAL_PATH)/../calculators/usf_epos
LOCAL_SRC_FILES         := ual_util.cpp               \
                           usf_unix_domain_socket.cpp \
                           usf_validation.cpp         \
                           ual_util_frame_file.cpp    \
                           usf_coord_transformer.cpp  \
                           usf_geometry.cpp           \
                           usf_dynamic_lib_proxy.cpp
LOCAL_SHARED_LIBRARIES  := libcutils  \
                           libutils   \
                           liblog     \
                           libmedia   \
                           libgui     \
                           libual     \
                           libdl

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif #BUILD_TINY_ANDROID
