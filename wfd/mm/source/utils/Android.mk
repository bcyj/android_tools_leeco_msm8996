ROOT_DIR := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PATH := $(ROOT_DIR)

# ---------------------------------------------------------------------------------
#            Common definitons
# ---------------------------------------------------------------------------------

libmm-wfd-def := -DQCOM_OMX_VENC_EXT
libmm-wfd-def += -O3
libmm-wfd-def += -D_ANDROID_
libmm-wfd-def += -D_ANDROID_LOG_
libmm-wfd-def += -D_ANDROID_LOG_ERROR
libmm-wfd-def += -D_ANDROID_LOG_PROFILE
libmm-wfd-def += -Du32="unsigned int"

ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
libmm-wfd-def += -DWFD_ICS
endif

# ---------------------------------------------------------------------------------
#            MM-UTILS INCLUDE
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

mm-utils-inc := $(LOCAL_PATH)/../framework/inc
mm-utils-inc += $(LOCAL_PATH)/./inc
mm-utils-inc += $(LOCAL_PATH)/../../inc
mm-utils-inc += $(TARGET_OUT_HEADERS)/common/inc
mm-utils-inc += $(TARGET_OUT_HEADERS)/mm-osal/include
mm-utils-inc += $(TARGET_OUT_HEADERS)/mm-core/omxcore

LOCAL_MODULE := libwfdmmutils
LOCAL_CFLAGS := $(libmm-wfd-def)
LOCAL_CFLAGS += -Wconversion
LOCAL_C_INCLUDES := $(mm-utils-inc)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

# ---------------------------------------------------------------------------------
#            MM-UTILS SHARED LIB
# ---------------------------------------------------------------------------------

LOCAL_SHARED_LIBRARIES := libmm-omxcore
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libmmosal
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libc
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_STATIC_LIBRARIES += libstagefright_aacenc

LOCAL_MODULE_TAGS := optional

# ---------------------------------------------------------------------------------
#            MM-UTILS SOURCE
# ---------------------------------------------------------------------------------

LOCAL_SRC_FILES := src/wfd_util_queue.c
LOCAL_SRC_FILES += src/wfd_util_mutex.c
LOCAL_SRC_FILES += src/wfd_util_signal.c
LOCAL_SRC_FILES += src/wfdmmsource_queue.cpp
LOCAL_SRC_FILES += src/WFDMMSourceMutex.cpp
LOCAL_SRC_FILES += src/WFDMMSourcePmem.cpp
LOCAL_SRC_FILES += src/WFDMMSourceQueue.cpp
LOCAL_SRC_FILES += src/WFDMMSourceSignal.cpp
LOCAL_SRC_FILES += src/WFDMMSourceSignalQueue.cpp



LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#      END
# ---------------------------------------------------------------------------------
