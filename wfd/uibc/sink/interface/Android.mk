LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS := -D_ANDROID_

# ---------------------------------------------------------------------------------
#            mm-sink-interface SRC
# ---------------------------------------------------------------------------------

LOCAL_SRC_FILES := ./src/UIBCSinkInterface.cpp

# ---------------------------------------------------------------------------------
#            mm-sink-interface INC
# ---------------------------------------------------------------------------------

mm-sink-intr := $(LOCAL_PATH)/./inc
mm-sink-intr += $(LOCAL_PATH)/../framework/inc
mm-sink-intr += $(LOCAL_PATH)/../../interface/inc
mm-sink-intr += $(LOCAL_PATH)/../../../utils/inc
mm-sink-intr += $(LOCAL_PATH)/../../../mm/interface/inc
mm-sink-intr += $(TARGET_OUT_HEADERS)/common/inc
mm-sink-intr += $(TARGET_OUT_HEADERS)/mm-osal/include
mm-sink-intr += $(TARGET_OUT_HEADERS)/mm-mux

LOCAL_C_INCLUDES := $(mm-sink-intr)

# ---------------------------------------------------------------------------------
#            SHARED LIBRARIES
# ---------------------------------------------------------------------------------
LOCAL_SHARED_LIBRARIES := libmmosal
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libc
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libwfduibcsink
LOCAL_SHARED_LIBRARIES += libwfdcommonutils

LOCAL_MODULE:= libwfduibcsinkinterface

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#            END
# ---------------------------------------------------------------------------------
