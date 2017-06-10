LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS := -D_ANDROID_

# ---------------------------------------------------------------------------------
#            mm-source-interface SRC
# ---------------------------------------------------------------------------------

LOCAL_SRC_FILES := ./src/UIBCSourceInterface.cpp

# ---------------------------------------------------------------------------------
#            mm-source-interface INC
# ---------------------------------------------------------------------------------

mm-src-intr := $(LOCAL_PATH)/./inc
mm-src-intr += $(LOCAL_PATH)/../framework/inc
mm-src-intr += $(LOCAL_PATH)/../../interface/inc
mm-src-intr += $(LOCAL_PATH)/../../../utils/inc
mm-src-intr += $(LOCAL_PATH)/../../../mm/interface/inc
mm-src-intr += $(TARGET_OUT_HEADERS)/common/inc
mm-src-intr += $(TARGET_OUT_HEADERS)/mm-osal/include

LOCAL_C_INCLUDES := $(mm-src-intr)

# ---------------------------------------------------------------------------------
#            SHARED LIBRARIES
# ---------------------------------------------------------------------------------
LOCAL_SHARED_LIBRARIES := libmmosal
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libc
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libwfduibcsrc
LOCAL_SHARED_LIBRARIES += libwfdcommonutils


LOCAL_MODULE:= libwfduibcsrcinterface

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#            END
# ---------------------------------------------------------------------------------
