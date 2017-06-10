#add API versions for each android release
CUPCAKE_SDK_VERSIONS := 3
DONUT_SDK_VERSIONS   := 4
ECLAIR_SDK_VERSIONS  := 5 6 7
FROYO_SDK_VERSIONS   := 8
GINGERBREAD_SDK_VERSIONS := 9 10
HONEYCOMB_SDK_VERSIONS := 11 12 13
ICECREAMSANDWICH_SDK_VERSIONS := 14 15
JELLYBEAN_SDK_VERSIONS := 16 17 18
KITKAT_SDK_VERSIONS := 19
LOLLIPOP_SDK_VERSIONS := 21 22

ifneq ($(BUILD_TINY_ANDROID),true)

MPEG4:=1

#Fill Version info for the component
ITTIAM_COMP_VERSION:=ITTIAM_OMX_VDEC_09_02_AUG_03_2015

#Enable/Disable Ittiam core
ITTIAM_CORE:=0

#Enable/Disable YUV420 SP instead of 420P - only if dequeue supports 420SP buffers
YUV420SP:=1

#Enable/Disable logs - Enable only for debugging
ITTIAM_DEBUG_LOGS:=1

#Enable/Disable shared mode to avoid a copy
SHARE_DISP_BUF:=0

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_STATIC_LIBRARIES  :=

# ---------------------------------------------------------------------------------
# 				Common definitons
# ---------------------------------------------------------------------------------

LOCAL_CFLAGS += -g -O2 -DCOMP_VERSION=\"$(ITTIAM_COMP_VERSION)\"
LOCAL_CFLAGS += -Wno-unused-parameter

ifeq ($(ITTIAM_DEBUG_LOGS),1)
LOCAL_CFLAGS += -DENABLE_LOGS
else
LOCAL_CFLAGS += -UENABLE_LOGS
endif

ifeq ($(YUV420SP),1)
LOCAL_CFLAGS += -DUSE_YUV420SP
endif

ifeq ($(SHARE_DISP_BUF),1)
LOCAL_CFLAGS += -DSHARE_DISP_BUF=1
else
LOCAL_CFLAGS += -DSHARE_DISP_BUF=0
endif

ifeq ($(MPEG4),1)
LOCAL_CFLAGS += -DENABLE_MPEG4
LOCAL_STATIC_LIBRARIES  += iv_mpeg4_dec_lib
endif

ifneq ($(filter $(PLATFORM_SDK_VERSION),$(JELLYBEAN_SDK_VERSIONS)),)
#Add JellyBean Specific flags
LOCAL_CFLAGS += -UENABLE_DESCRIBE_COLOR_FORMAT
LOCAL_CFLAGS += -UENABLE_ADAPTIVE_PLAYBACK
else ifneq ($(filter $(PLATFORM_SDK_VERSION),$(KITKAT_SDK_VERSIONS)),)
#Add Kitkat Specific flags
LOCAL_CFLAGS += -UENABLE_DESCRIBE_COLOR_FORMAT
LOCAL_CFLAGS += -DENABLE_ADAPTIVE_PLAYBACK
else ifneq ($(filter $(PLATFORM_SDK_VERSION),$(LOLLIPOP_SDK_VERSIONS)),)
#Add Lollipop Specific flags
	LOCAL_CFLAGS += -DENABLE_DESCRIBE_COLOR_FORMAT
	LOCAL_CFLAGS += -DENABLE_ADAPTIVE_PLAYBACK
else
#Add default flags
	LOCAL_CFLAGS += -DENABLE_DESCRIBE_COLOR_FORMAT
	LOCAL_CFLAGS += -DENABLE_ADAPTIVE_PLAYBACK
endif


LOCAL_CFLAGS += -DUSE_MIN_TIMESTAMPS

#Following disables support for interlaced streams
LOCAL_CFLAGS += -DDISABLE_INTERLACED

# ---------------------------------------------------------------------------------
# 			Make the Shared library (libOmxVdec)
# ---------------------------------------------------------------------------------

LOCAL_C_INCLUDES          := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES          += $(TOP)/system/core/include/
LOCAL_C_INCLUDES          += $(TOP)/frameworks/native/include
LOCAL_C_INCLUDES          += $(TOP)/frameworks/native/include/ui
LOCAL_C_INCLUDES          += $(TOP)/frameworks/av/include/media/stagefright
LOCAL_C_INCLUDES          += $(TOP)/frameworks/native/include/media/hardware
LOCAL_C_INCLUDES          += $(TOP)/frameworks/native/include/media/openmax

LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_MODULE             := libOmxIttiamVdec
LOCAL_MODULE_TAGS        := optional

LOCAL_MODULE_OWNER       := qti
LOCAL_PROPRIETARY_MODULE := true

LOCAL_LDFLAGS			 :=  -Wl,-Bsymbolic,--no-warn-shared-textrel -Wno-error

LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := liblog libutils libcutils libdl libui

LOCAL_LDLIBS += -ldl

LOCAL_SRC_FILES         := src/OMX_Ittiam_Vdec_API.c
LOCAL_SRC_FILES         += src/OMX_Ittiam_Vdec_Process.c
LOCAL_SRC_FILES         += src/ithread.c

ifeq ($(ITTIAM_CORE),1)
LOCAL_SRC_FILES         += src/omx_vdec_ittiam_wrapper.cpp
LOCAL_CFLAGS			+= -DITTIAM_CORE
LOCAL_CFLAGS			+= -UENABLE_RUNTIME_LOGCONTROL
else
LOCAL_SRC_FILES         += src/omx_vdec_qcom_wrapper.cpp
LOCAL_C_INCLUDES        += $(TOP)/hardware/qcom/media/mm-core/inc/
endif

include $(BUILD_SHARED_LIBRARY)

endif #BUILD_TINY_ANDROID

# ---------------------------------------------------------------------------------
#                END
# ---------------------------------------------------------------------------------
