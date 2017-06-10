LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#-----------------------------------------------------------------
# Define
#-----------------------------------------------------------------
LOCAL_CFLAGS := -D_ANDROID_
LOCAL_CFLAGS += -Wconversion
#----------------------------------------------------------------
# SRC CODE
#----------------------------------------------------------------
LOCAL_SRC_FILES := src/WFD_HdcpCP.cpp
LOCAL_SRC_FILES += src/HDCPManager.cpp

#----------------------------------------------------------------
# COPY HEADERS
#----------------------------------------------------------------
LOCAL_COPY_HEADERS_TO := wfd/mm/hdcp/

#----------------------------------------------------------------
# INCLUDE PATH
#----------------------------------------------------------------
LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TOP)/bionic/libc/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../source/utils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/omxcore
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/hdcp-mgr/inc
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/utils

#----------------------------------------------------------------
# Dx HDCP COMPILE TIME
#----------------------------------------------------------------
COMPILE_HDCP_LIB := false
WFD_NOSHIP_HDCP_PATH  := $(LOCAL_PATH)/../../../../wfd-noship/mm/hdcp
COMPILE_HDCP_LIB := $(shell if [ -f $(WFD_NOSHIP_HDCP_PATH)/Android.mk ] ; then echo true; fi)

LOCAL_SHARED_LIBRARIES := libmmosal
ifeq ($(COMPILE_HDCP_LIB),true)
LOCAL_C_INCLUDES += $(WFD_NOSHIP_HDCP_PATH)
LOCAL_C_INCLUDES += $(WFD_NOSHIP_HDCP_PATH)/HDCP_API
LOCAL_SHARED_LIBRARIES_32 += libDxHdcp
LOCAL_SHARED_LIBRARIES += libmm-hdcpmgr
LOCAL_CFLAGS_32 += -DWFD_HDCP_ENABLED
endif
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils

LOCAL_MODULE:= libwfdhdcpcp

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
