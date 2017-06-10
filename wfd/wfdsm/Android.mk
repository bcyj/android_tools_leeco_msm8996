LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_CFLAGS := -DWFD_ICS
endif
RTSP_PATH := ../rtsp/rtsplib
rtsp_inc := $(LOCAL_PATH)/$(RTSP_PATH)/inc

MM_INC_PATH := ../mm/interface/inc
mm_inc := $(LOCAL_PATH)/$(MM_INC_PATH)
mm_inc += $(TARGET_OUT_HEADERS)/mm-osal/include
ifeq ("$(PLATFORM_VERSION)","4.3")
mm_inc += $(TOP)/frameworks/base/include/utils
else
mm_inc += $(TOP)/system/core/include/utils
endif
mm_inc += $(TARGET_OUT_HEADERS)/common/inc
UIBC_INC_PATH := ../uibc/interface/inc
uibc_inc := $(LOCAL_PATH)/$(UIBC_INC_PATH)

WFD_UTILS_PATH := ../utils
wfd_utils_inc := $(LOCAL_PATH)/$(WFD_UTILS_PATH)/inc
WFD_MM_SRC_UTILS_PATH := ../mm/source/utils
wfd_utils_inc += $(LOCAL_PATH)/$(WFD_MM_SRC_UTILS_PATH)/inc

LOCAL_MODULE    := libwfdsm
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := src/SessionManager.cpp
LOCAL_SRC_FILES += src/MMAdaptor.cpp
LOCAL_SRC_FILES += src/RTSPSession.cpp
LOCAL_SRC_FILES += src/Device.cpp
LOCAL_SRC_FILES += src/MMCapability.cpp
LOCAL_SRC_FILES += src/WFDSession.cpp
LOCAL_SRC_FILES += src/wifidisplay.cpp
LOCAL_SRC_FILES += src/UIBCAdaptor.cpp


# Additional libraries, maybe more than actually needed
LOCAL_SHARED_LIBRARIES := libstlport
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libwfdrtsp
LOCAL_SHARED_LIBRARIES += libmmwfdinterface
LOCAL_SHARED_LIBRARIES += libwfduibcinterface
LOCAL_SHARED_LIBRARIES += libmmosal
LOCAL_SHARED_LIBRARIES += libwfdcommonutils
LOCAL_SHARED_LIBRARIES += libwfdmmutils
LNK_HDCP_LIB := false
WFD_NOSHIP_PATH  := $(TOP)/vendor/qcom/proprietary/wfd-noship/mm/hdcp
LNK_HDCP_LIB := $(shell if [ -f $(WFD_NOSHIP_HDCP_PATH)/Android.mk ] ; then echo true; fi)
ifeq ($(LNK_HDCP_LIB),true)
LOCAL_SHARED_LIBRARIES += libmm-hdcpmgr
LOCAL_CFLAGS += -DHDCP_DISPLAY_ENABLED
endif
LOCAL_SHARED_LIBRARIES += libqdutils

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(rtsp_inc)
LOCAL_C_INCLUDES += $(mm_inc)
LOCAL_C_INCLUDES += external/connectivity/stlport/stlport/
LOCAL_C_INCLUDES += $(uibc_inc)
ifeq ($(LNK_HDCP_LIB),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/hdcp-mgr/inc
endif
LOCAL_C_INCLUDES += $(TOP)/hardware/qcom/display/libqdutils/
LOCAL_C_INCLUDES += $(wfd_utils_inc)
ifneq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/utils
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/gui
endif

LOCAL_LDLIBS += -llog


LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

#include $(call all-makefiles-under,$(LOCAL_PATH))
