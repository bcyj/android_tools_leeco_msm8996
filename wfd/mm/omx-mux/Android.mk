ifneq ($(BUILD_TINY_ANDROID),true)

ROOT_DIR := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PATH := $(ROOT_DIR)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libmm-mux-def := -O3
libmm-mux-def += -DQCOM_OMX_VENC_EXT
libmm-mux-def += -D_ANDROID_
libmm-mux-def += -D_ANDROID_LOG_
libmm-mux-def += -D_ANDROID_LOG_ERROR
libmm-mux-def += -D_ANDROID_LOG_PROFILE
libmm-mux-def += -Du32="unsigned int"

# ---------------------------------------------------------------------------------
#             OMX_MUX INC
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

mm-mux-inc := $(LOCAL_PATH)/inc
mm-mux-inc += $(LOCAL_PATH)/src
mm-mux-inc += $(TARGET_OUT_HEADERS)/mm-core/omxcore
mm-mux-inc += $(TARGET_OUT_HEADERS)/common/inc
mm-mux-inc += $(TARGET_OUT_HEADERS)/mm-parser/include
mm-mux-inc += $(TARGET_OUT_HEADERS)/mm-osal/include
mm-mux-inc += $(TARGET_OUT_HEADERS)/mm-mux

LOCAL_MODULE := libOmxMux
LOCAL_CFLAGS := $(libmm-mux-def)
LOCAL_CFLAGS += -Wconversion
LOCAL_C_INCLUDES := $(mm-mux-inc)

# ---------------------------------------------------------------------------------
#             OMX_MUX SHARED LIB
# ---------------------------------------------------------------------------------

LOCAL_SHARED_LIBRARIES := liblog
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libmm-omxcore
LOCAL_SHARED_LIBRARIES += libFileMux
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libmmosal
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libOmxCore

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := src/omx_filemux.cpp
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif #BUILD_TINY_ANDROID
# ---------------------------------------------------------------------------------
#             END
# ---------------------------------------------------------------------------------
