ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#===============================================================================
#             Deploy the headers that can be exposed
#===============================================================================

LOCAL_COPY_HEADERS_TO := mm-still/gemini
LOCAL_COPY_HEADERS += inc/gemini_lib.h
LOCAL_COPY_HEADERS += inc/gemini_lib_common.h
LOCAL_COPY_HEADERS += inc/gemini_app_util_mmap.h
LOCAL_COPY_HEADERS += inc/gemini_app_calc_param.h
LOCAL_COPY_HEADERS += inc/gemini_inline.h

#===============================================================================
#             Compile Shared library libgemini.so
#===============================================================================

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
ifneq ($(strip $(USE_BIONIC_HEADER)),true)
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../../../hardware/qcom/camera

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
else
LOCAL_C_INCLUDES+= bionic/libc/kernel/common/media
endif

ifeq ($(strip $(TARGET_USES_ION)),true)
LOCAL_CFLAGS += -DUSE_ION
endif

ifeq ($(strip $(NEW_LOG_API)),true)
LOCAL_CFLAGS += -DNEW_LOG_API
endif

LOCAL_SRC_FILES := src/gemini_lib.c
LOCAL_SRC_FILES += src/gemini_lib_hw.c
LOCAL_SRC_FILES += src/gemini_app_util_mmap.c
LOCAL_SRC_FILES += src/gemini_app_calc_param.c
LOCAL_SRC_FILES += src/gemini_inline.c

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_MODULE :=libgemini
LOCAL_MODULE_OWNER := qcom
LOCAL_32_BIT_ONLY := true
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

#===============================================================================
#             Compile test app test_gemini
#===============================================================================

include $(CLEAR_VARS)

LOCAL_C_INCLUDES:= $(LOCAL_PATH)/inc
ifneq ($(strip $(USE_BIONIC_HEADER)),true)
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../../../hardware/qcom/camera

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
else
LOCAL_C_INCLUDES+= bionic/libc/kernel/common/media
endif
ifeq ($(strip $(TARGET_USES_ION)),true)
LOCAL_CFLAGS += -DUSE_ION
endif

ifeq ($(strip $(NEW_LOG_API)),true)
LOCAL_CFLAGS += -DNEW_LOG_API
endif

LOCAL_SRC_FILES := test/test_gemini.c
LOCAL_SRC_FILES += test/gemini_app.c

LOCAL_SHARED_LIBRARIES 	:= libgemini libcutils
LOCAL_MODULE :=test_gemini
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom
LOCAL_32_BIT_ONLY := true


include $(BUILD_EXECUTABLE)

endif #BUILD_TINY_ANDROID
