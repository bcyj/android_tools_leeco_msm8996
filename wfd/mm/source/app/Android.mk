ROOT_DIR := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PATH := $(ROOT_DIR)

# ---------------------------------------------------------------------------------
#          Common definitons
# ---------------------------------------------------------------------------------

libmm-venc-def := -DQCOM_OMX_VENC_EXT
libmm-venc-def += -DWIDI_720p_ENCODE_ENABLE
libmm-venc-def += -O3
libmm-venc-def += -D_ANDROID_
libmm-venc-def += -D_ANDROID_LOG_
libmm-venc-def += -D_ANDROID_LOG_ERROR
libmm-venc-def += -D_ANDROID_LOG_PROFILE
libmm-venc-def += -Du32="unsigned int"

# ---------------------------------------------------------------------------------
#          MM-SRC APP
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

mm-venc-test-inc := $(LOCAL_PATH)/../../interface/inc
mm-venc-test-inc += $(LOCAL_PATH)/../utils/inc
mm-venc-test-inc += $(LOCAL_PATH)/../../Framework/inc
mm-venc-test-inc += $(LOCAL_PATH)/../../uibc/source/inc
mm-venc-test-inc += $(TARGET_OUT_HEADERS)/mm-core/omxcore
mm-venc-test-inc += $(TARGET_OUT_HEADERS)/common/inc
mm-venc-test-inc += $(TARGET_OUT_HEADERS)/mm-osal/include

LOCAL_MODULE := venc-widi
LOCAL_CFLAGS := $(libmm-venc-def)
LOCAL_CFLAGS += -Wconversion
LOCAL_C_INCLUDES := $(mm-venc-test-inc)

# ---------------------------------------------------------------------------------
#          MM-SRC APP SHARED LIB
# ---------------------------------------------------------------------------------

LOCAL_SHARED_LIBRARIES := libmmosal
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libmmrtpencoder
LOCAL_SHARED_LIBRARIES += libmmwfdinterface
LOCAL_SHARED_LIBRARIES += libwfdmmsrc
LOCAL_SHARED_LIBRARIES += libcutils

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES :=./src/WFDMMSourceApp.cpp

include $(BUILD_EXECUTABLE)

# ---------------------------------------------------------------------------------
#             END
# ---------------------------------------------------------------------------------
