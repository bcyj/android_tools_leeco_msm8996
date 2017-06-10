ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)

include $(CLEAR_VARS)

LOCAL_CFLAGS := -D_ANDROID_
LOCAL_MODULE_TAGS := optional

# ---------------------------------------------------------------------------------
#            UIBC SINK FILES
# ---------------------------------------------------------------------------------

LOCAL_SRC_FILES := ./src/UIBCPacketizer.cpp
LOCAL_SRC_FILES += ./src/UIBCPacketTransmitter.cpp
LOCAL_SRC_FILES += ./src/UIBCSinkManager.cpp

# ---------------------------------------------------------------------------------
#            UIBC INCLUDE FILES
# ---------------------------------------------------------------------------------

LOCAL_C_INCLUDES := $(JNI_H_INCLUDE)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/./inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../interface/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../mm/interface/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-mux

LOCAL_SHARED_LIBRARIES := libmmosal
LOCAL_SHARED_LIBRARIES += libFileMux
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += liblog

LOCAL_MODULE := libwfduibcsink

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#      END
# ---------------------------------------------------------------------------------
