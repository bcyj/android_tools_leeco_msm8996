LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#-----------------------------------------------------------------
# Define
#-----------------------------------------------------------------
LOCAL_CFLAGS := -D_ANDROID_

#----------------------------------------------------------------
# SRC CODE
#----------------------------------------------------------------
LOCAL_SRC_FILES := src/IWiFiDisplayService.cpp
LOCAL_SRC_FILES += src/IWiFiDisplayListener.cpp
LOCAL_SRC_FILES += src/WiFiDisplayService.cpp
LOCAL_SRC_FILES += src/WiFiDisplayClient.cpp
#----------------------------------------------------------------
# INCLUDE PATH
#----------------------------------------------------------------

WFDSM_PATH = $(LOCAL_PATH)/../../wfdsm
UIBC_PATH = $(LOCAL_PATH)/../../uibc/interface

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(WFDSM_PATH)/inc
LOCAL_C_INCLUDES += $(UIBC_PATH)/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include

LOCAL_SHARED_LIBRARIES := liblog
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libgui
LOCAL_SHARED_LIBRARIES += libwfdsm

LOCAL_MODULE:= libwfdservice

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

############################################################
##### Build the main executable for WiFiDisplayService #####
############################################################
include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := liblog
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libwfdservice

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(WFDSM_PATH)/inc
LOCAL_C_INCLUDES += $(UIBC_PATH)/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include
LOCAL_SRC_FILES = WiFiDisplayService_main.cpp

LOCAL_MODULE:= wfdservice
LOCAL_32_BIT_ONLY := true

include $(BUILD_EXECUTABLE)

###End of executable
