ifeq ($(call is-vendor-board-platform,QCOM),true)
WFD_DISABLE_PLATFORM_LIST := msm8610 mpq8092 msm_bronze
ifneq ($(call is-board-platform-in-list,$(WFD_DISABLE_PLATFORM_LIST)),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#          Common definitons
# ---------------------------------------------------------------------------------

libmm-rtp-def := -O3
libmm-rtp-def += -D_ANDROID_
libmm-rtp-def += -D_ANDROID_LOG_
libmm-rtp-def += -D_ANDROID_LOG_ERROR
libmm-rtp-def += -D_ANDROID_LOG_PROFILE
libmm-rtp-def += -Du32="unsigned int"
libmm-rtp-def += -DENABLE_RTP_STATS


LOCAL_SRC_FILES := src/RTPEncoder.cpp
LOCAL_SRC_FILES += src/RTPPacketizer.cpp
LOCAL_SRC_FILES += src/RTPPacketTransmit.cpp
LOCAL_SRC_FILES += src/RTCPReceiver.cpp

LOCAL_COPY_HEADERS_TO := mm-rtp/encoder
LOCAL_COPY_HEADERS := inc/RTPEncoder.h
LOCAL_COPY_HEADERS += inc/RTCPReceiver.h
LOCAL_COPY_HEADERS += ../common/inc/DataSourcePort.h
LOCAL_COPY_HEADERS += ../common/inc/SourceBase.h

# ---------------------------------------------------------------------------------
#            Make the apps
# ---------------------------------------------------------------------------------

mm-rtp-inc := $(LOCAL_PATH)/inc
mm-rtp-inc += $(TARGET_OUT_HEADERS)/mm-osal/include
mm-rtp-inc += $(LOCAL_PATH)/../common/inc
mm-rtp-inc += $(TARGET_OUT_HEADERS)/common/inc
ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
mm-rtp-inc +=  $(TOP)/frameworks/base/include/utils
else ifeq ("$(PLATFORM_VERSION)","4.3")
mm-rtp-inc += $(TOP)/frameworks/native/include/utils
else
mm-rtp-inc += $(TOP)/system/core/include/utils
endif

LOCAL_C_INCLUDES := $(mm-rtp-inc)


LOCAL_MODULE := libmmrtpencoder
LOCAL_CFLAGS += $(libmm-rtp-def)



LOCAL_SHARED_LIBRARIES := libmm-omxcore
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libmmosal
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libwfdcommonutils
LOCAL_SHARED_LIBRARIES += libcutils

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif #is-board-platform
endif #is-vendor-board-platform
