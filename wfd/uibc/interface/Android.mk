ROOT_DIR := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PATH := $(ROOT_DIR)

# ---------------------------------------------------------------------------------
#            Common definitons
# ---------------------------------------------------------------------------------

LOCAL_CFLAGS := -D_ANDROID_
LOCAL_CFLAGS += -DWFD_SINK_ENABLED
LOCAL_MODULE_TAGS := optional

# ---------------------------------------------------------------------------------
#            MM-INTERFACE SRC
# ---------------------------------------------------------------------------------

LOCAL_SRC_FILES := ./src/UIBCInterface.cpp

# ---------------------------------------------------------------------------------
#            MM-INTERFACE INC
# ---------------------------------------------------------------------------------

LOCAL_C_INCLUDES := $(LOCAL_PATH)/./inc
# Add some platform based logic that if no-ship folder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sink/interface/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../source/interface/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include

# ---------------------------------------------------------------------------------
#            MM-INTERFACE SHARED LIB
# ---------------------------------------------------------------------------------

LOCAL_SHARED_LIBRARIES := libmmosal
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libc
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libwfduibcsrcinterface
# Add some platform based logic that if no-ship folder
LOCAL_SHARED_LIBRARIES += libwfduibcsinkinterface

LOCAL_MODULE:= libwfduibcinterface
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#            END
# ---------------------------------------------------------------------------------
