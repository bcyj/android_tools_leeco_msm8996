ifneq ($(BUILD_TINY_ANDROID),true)

ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)
include $(CLEAR_VARS)

# ------------------------------------------------------------------------------
#         Common definitons
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
#       Make the Shared library (libusndroute)
# ------------------------------------------------------------------------------
LOCAL_C_INCLUDES = \
        external/tinyalsa/include \
        external/expat/lib
LOCAL_SRC_FILES:= usnd_route.c
LOCAL_MODULE := libusndroute
LOCAL_SHARED_LIBRARIES:= liblog libcutils libutils libexpat libtinyalsa
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)


endif #BUILD_TINY_ANDROID

