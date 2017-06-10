LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_CFLAGS := -DWFD_ICS
endif

UIBC_PATH := ../../../uibc/interface
uibc_inc := $(LOCAL_PATH)/$(UIBC_PATH)/inc

LOCAL_MODULE    := libwfdnative
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := src/WFDNative.cpp

# Additional libraries, maybe more than actually needed
LOCAL_SHARED_LIBRARIES := libandroid_runtime
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += libinput
LOCAL_SHARED_LIBRARIES += libnativehelper
LOCAL_SHARED_LIBRARIES += libutils liblog
LOCAL_SHARED_LIBRARIES += libwfdservice
ifneq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_SHARED_LIBRARIES += libandroidfw
endif

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(uibc_inc)
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../wfdSvc/inc
LOCAL_C_INCLUDES += $(TOP)/frameworks/base/core/jni
ifneq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/gui
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/input
LOCAL_C_INCLUDES += $(TOP)/frameworks/base/include/androidfw
LOCAL_C_INCLUDES += external/connectivity/stlport/stlport
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../wfdsm/inc
endif

# JNI headers
LOCAL_C_INCLUDES += $(JNI_H_INCLUDE)

LOCAL_LDLIBS += -llog


LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

WFDSM_PATH := ../../../wfdsm
wfdsm_inc := $(LOCAL_PATH)/$(WFDSM_PATH)/inc

ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_CFLAGS := -DWFD_ICS
endif
MM_PATH := ../../../mm/interface
mm_inc := $(LOCAL_PATH)/$(MM_PATH)/inc
mm_inc += $(TARGET_OUT_HEADERS)/mm-osal/include
mm_inc += $(TARGET_OUT_HEADERS)/common/inc
mm_inc += $(TOP)/hardware/qcom/display/libgralloc

UIBC_PATH := ../../../uibc/interface
uibc_inc := $(LOCAL_PATH)/$(UIBC_PATH)/inc

LOCAL_MODULE    := libextendedremotedisplay
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := src/ExtendedRemoteDisplay.cpp
LOCAL_SRC_FILES += src/com_qualcomm_wfd_ExtendedRemoteDisplay.cpp

# Additional libraries, maybe more than actually needed
LOCAL_SHARED_LIBRARIES := libandroid_runtime
LOCAL_SHARED_LIBRARIES += libnativehelper
LOCAL_SHARED_LIBRARIES += libstagefright
LOCAL_SHARED_LIBRARIES += libutils liblog
LOCAL_SHARED_LIBRARIES += libmmosal
ifneq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_SHARED_LIBRARIES += libandroidfw
endif

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(wfdsm_inc)
LOCAL_C_INCLUDES += $(mm_inc)
LOCAL_C_INCLUDES += $(uibc_inc)
LOCAL_C_INCLUDES += $(TOP)/frameworks/base/core/jni
ifneq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/gui
LOCAL_C_INCLUDES += $(TOP)/frameworks/base/include/androidfw
endif

# JNI headers
LOCAL_C_INCLUDES += $(JNI_H_INCLUDE)

LOCAL_LDLIBS += -llog

LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
