ROOT_DIR := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PATH := $(ROOT_DIR)

# ---------------------------------------------------------------------------------
#            Common definitons
# ---------------------------------------------------------------------------------
WFD_VDS_ARCH = 1

libmm-wfd-def := -DQCOM_OMX_VENC_EXT
libmm-wfd-def += -O3
libmm-wfd-def += -D_ANDROID_
libmm-wfd-def += -D_ANDROID_LOG_
libmm-wfd-def += -D_ANDROID_LOG_ERROR
libmm-wfd-def += -D_ANDROID_LOG_PROFILE
libmm-wfd-def += -Du32="unsigned int"
libmm-wfd-def += -DENABLE_WFD_STATS
libmm-wfd-def += -DENABLE_H264_DUMP
libmm-wfd-def += -DWFD_DUMP_AUDIO_DATA
libmm-wfd-def += -DAAC_DUMP_ENABLE
ifeq ($(WFD_VDS_ARCH),1)
libmm-wfd-def += -DWFD_VDS_ARCH
endif

ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
libmm-wfd-def += -DWFD_ICS
endif

# ---------------------------------------------------------------------------------
#            MM-FRAMEWORK INC
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)



mm-wfd-inc := $(LOCAL_PATH)/inc
mm-wfd-inc += $(LOCAL_PATH)/../utils/inc
mm-wfd-inc += $(LOCAL_PATH)/../../interface/inc
mm-wfd-inc += $(LOCAL_PATH)/../../hdcp/common/inc
mm-wfd-inc += $(LOCAL_PATH)/../../../utils/inc
mm-wfd-inc += $(TARGET_OUT_HEADERS)/common/inc
mm-wfd-inc += $(TOP)/external/aac/libAACenc/include
mm-wfd-inc += $(TOP)/external/aac/libSYS/include
mm-wfd-inc += $(TOP)/frameworks/native/include/media/hardware
mm-wfd-inc += $(TOP)/hardware/qcom/media/libstagefrighthw/
mm-wfd-inc += $(TOP)/hardware/qcom/display/libqdutils/
mm-wfd-inc += $(TOP)/hardware/qcom/display/libgralloc/

ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
mm-wfd-inc += $(TOP)/frameworks/base/include/media/stagefright
mm-wfd-inc += $(TOP)/frameworks/base/media/libstagefright/include
mm-wfd-inc += $(TOP)/frameworks/base/include/utils
else
mm-wfd-inc += $(TOP)/frameworks/av/media/libstagefright/include
ifeq ("$(PLATFORM_VERSION)","4.3")
mm-wfd-inc += $(TOP)/frameworks/native/include/utils
else
mm-wfd-inc += $(TOP)/system/core/include/utils
mm-wfd-inc += $(TOP)/system/core/include/system
endif
mm-wfd-inc += $(TOP)/frameworks/native/include/gui
mm-wfd-inc += $(TOP)/frameworks/av/include
mm-wfd-inc += $(TOP)/hardware/libhardware_legacy/include/hardware_legacy
mm-wfd-inc += $(TOP)/external/tinyalsa/include
endif
mm-wfd-inc += $(TARGET_OUT_HEADERS)/mm-core/omxcore
mm-wfd-inc += $(TARGET_OUT_HEADERS)/mm-osal/include
mm-wfd-inc += $(TARGET_OUT_HEADERS)/mm-rtp/encoder
mm-wfd-inc += $(TOP)/external/tinyalsa/include
mm-wfd-inc += $(TARGET_OUT_HEADERS)/mm-rtp/common
mm-wfd-inc += $(TARGET_OUT_HEADERS)/mm-mux
mm-wfd-inc += $(TARGET_OUT_HEADERS)/mm-parser/include
mm-wfd-inc += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
mm-wfd-inc += $(LOCAL_PATH)/../../../uibc/interface/inc
mm-wfd-inc += $(LOCAL_PATH)/../../dolby/enc/inc

LOCAL_MODULE :=  libwfdmmsrc
LOCAL_CFLAGS := $(libmm-wfd-def)
LOCAL_CFLAGS += -Wconversion
LOCAL_C_INCLUDES := $(mm-wfd-inc)

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

# ---------------------------------------------------------------------------------
#            MM-FRAMEWORK SHARED LIB
# ---------------------------------------------------------------------------------

LOCAL_SHARED_LIBRARIES := libOmxCore
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libmmosal
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libmmrtpencoder
LOCAL_SHARED_LIBRARIES += libFileMux
LOCAL_SHARED_LIBRARIES += libwfdmmutils
LOCAL_SHARED_LIBRARIES += libc
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libtinyalsa
LOCAL_SHARED_LIBRARIES += libstagefright_enc_common
LOCAL_SHARED_LIBRARIES += libmedia
LOCAL_SHARED_LIBRARIES += libgui
LOCAL_SHARED_LIBRARIES += libwfdhdcpcp
LOCAL_SHARED_LIBRARIES += libwfdcommonutils
LOCAL_SHARED_LIBRARIES += libstagefright_soft_aacenc
LOCAL_SHARED_LIBRARIES += libstagefright
LOCAL_SHARED_LIBRARIES += libqdutils
LOCAL_SHARED_LIBRARIES += libdl

LOCAL_MODULE_TAGS := optional

# ---------------------------------------------------------------------------------
#            MM-FRAMEWORK SRC
# ---------------------------------------------------------------------------------

LOCAL_SRC_FILES := src/WFDMMSource.cpp
LOCAL_SRC_FILES += src/WFDMMSourceAudioSource.cpp
ifeq ($(WFD_VDS_ARCH),1)
LOCAL_SRC_FILES += src/WFDMMSourceVideoSource.cpp
LOCAL_SRC_FILES += src/WFDMMSourceVideoEncode.cpp
LOCAL_SRC_FILES += src/WFDMMSourceVideoCapture.cpp
else
LOCAL_SRC_FILES += src/WFDMMSourceScreenSource.cpp
endif
LOCAL_SRC_FILES += src/WFDMMSourceMux.cpp
LOCAL_SRC_FILES += src/WFDMMIonMemory.cpp
LOCAL_SRC_FILES += src/WFDMMSourceAudioEncode.cpp
LOCAL_SRC_FILES += src/WFDMMSourceAACEncode.cpp
LOCAL_SRC_FILES += src/WFDMMSourceAC3Encode.cpp

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

LOCAL_LDFLAGS                   += -Wl,--no-fatal-warnings

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#      END
# ---------------------------------------------------------------------------------
