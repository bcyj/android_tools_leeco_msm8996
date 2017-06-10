LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS := -D_ANDROID_
LOCAL_CFLAGS += -Wconversion
# ---------------------------------------------------------------------------------
#            mm-source-interface SRC
# ---------------------------------------------------------------------------------

LOCAL_SRC_FILES := ./src/wdsm_mm_source_interface.cpp

# ---------------------------------------------------------------------------------
#            mm-source-interface INC
# ---------------------------------------------------------------------------------

mm-src-intr := $(LOCAL_PATH)/./inc
ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
mm-src-intr += $(TOP)/frameworks/base/media/libstagefright/include
mm-src-intr += $(TOP)/frameworks/base/media/libstagefright/codecs/common/include
else
mm-src-intr += $(TOP)/frameworks/av/media/libstagefright/include
mm-src-intr += $(TOP)/frameworks/av/media/libstagefright/codecs/common/include
endif
mm-src-intr += $(LOCAL_PATH)/../framework/inc
mm-src-intr += $(LOCAL_PATH)/../../interface/inc
mm-src-intr += $(LOCAL_PATH)/../utils/inc
mm-src-intr += $(LOCAL_PATH)/../../../uibc/interface/inc
mm-src-intr += $(TARGET_OUT_HEADERS)/common/inc
mm-src-intr += $(TARGET_OUT_HEADERS)/mm-osal/include
mm-src-intr += $(TARGET_OUT_HEADERS)/mm-core/omxcore
mm-src-intr += external/connectivity/stlport/stlport
mm-src-intr += $(LOCAL_PATH)/../../../utils/inc
mm-src-intr += $(LOCAL_PATH)/../../hdcp/common/inc
mm-src-intr += $(TOP)/external/tinyalsa/include

LOCAL_C_INCLUDES := $(mm-src-intr)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

# ---------------------------------------------------------------------------------
#            SHARED LIBRARIES
# ---------------------------------------------------------------------------------
LOCAL_SHARED_LIBRARIES := libmmosal
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libwfdmmsrc
LOCAL_SHARED_LIBRARIES += libc
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libtinyalsa
LOCAL_SHARED_LIBRARIES += libmedia
LOCAL_SHARED_LIBRARIES += libwfdcommonutils
LOCAL_SHARED_LIBRARIES += libwfdhdcpcp

LOCAL_MODULE:= libmmwfdsrcinterface

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#            END
# ---------------------------------------------------------------------------------
