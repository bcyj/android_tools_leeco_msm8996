ROOT_DIR := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PATH := $(ROOT_DIR)

# ---------------------------------------------------------------------------------
#            Common definitons
# ---------------------------------------------------------------------------------

LOCAL_CFLAGS := -D_ANDROID_
LOCAL_CFLAGS += -Wconversion
LOCAL_MODULE_TAGS := optional

# ---------------------------------------------------------------------------------
#            MM-INTERFACE SRC
# ---------------------------------------------------------------------------------

LOCAL_SRC_FILES := ./src/wdsm_mm_interface.cpp

# ---------------------------------------------------------------------------------
#            MM-INTERFACE INC
# ---------------------------------------------------------------------------------

LOCAL_C_INCLUDES := $(LOCAL_PATH)/./inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sink/interface/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../source/interface/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../hdcp/common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../utils/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../source/utils/inc
LOCAL_C_INCLUDES += $(TOP)/frameworks/base/include/media
LOCAL_C_INCLUDES += $(TOP)/frameworks/base/include/media/stagefright
LOCAL_C_INCLUDES += $(TOP)/external/tinyalsa/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/omxcore
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-mux
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include

# ---------------------------------------------------------------------------------
#            MM-INTERFACE SHARED LIB
# ---------------------------------------------------------------------------------

LOCAL_SHARED_LIBRARIES := libmmosal
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libwfdmmsrc
LOCAL_SHARED_LIBRARIES += libmedia
LOCAL_SHARED_LIBRARIES += libc
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libstagefright
LOCAL_SHARED_LIBRARIES += libgui
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libtinyalsa
LOCAL_SHARED_LIBRARIES += libmmwfdsrcinterface
LOCAL_SHARED_LIBRARIES += libmmwfdsinkinterface
LOCAL_SHARED_LIBRARIES += libwfdhdcpcp

LOCAL_MODULE:= libmmwfdinterface
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#            END
# ---------------------------------------------------------------------------------
