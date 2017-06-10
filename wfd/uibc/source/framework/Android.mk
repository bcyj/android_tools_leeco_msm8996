ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)

include $(CLEAR_VARS)

LOCAL_CFLAGS := -D_ANDROID_
LOCAL_CFLAGS += -DHID_USE_KERNEL
LOCAL_MODULE_TAGS := optional

# ---------------------------------------------------------------------------------
#            UIBC SOURCE FILES
# ---------------------------------------------------------------------------------

LOCAL_SRC_FILES := ./src/UIBCInputReceiver.cpp
LOCAL_SRC_FILES += ./src/UIBCInputParser.cpp
LOCAL_SRC_FILES += ./src/UIBCInputInjector.cpp
LOCAL_SRC_FILES += ./src/UIBCHIDInjector.cpp

# ---------------------------------------------------------------------------------
#            UIBC INCLUDE FILES
# ---------------------------------------------------------------------------------

uibc-src-inc := $(LOCAL_PATH)/./inc
uibc-src-inc += $(LOCAL_PATH)/../../interface/inc
uibc-src-inc += $(TARGET_OUT_HEADERS)/common/inc
uibc-src-inc += $(TARGET_OUT_HEADERS)/mm-osal/include
uibc-src-inc += $(TOP)/system/core/include/utils


LOCAL_C_INCLUDES:= $(uibc-src-inc)

LOCAL_SHARED_LIBRARIES := libmmosal
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libc libcutils

LOCAL_MODULE:= libwfduibcsrc

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#      END
# ---------------------------------------------------------------------------------
