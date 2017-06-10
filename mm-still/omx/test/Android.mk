OMX_TEST_CLIENT_PATH := $(call my-dir)

# ---------------------------------------------------------------------------------
#                       Common definitons
# ---------------------------------------------------------------------------------

#CPU := -mcpu=arm1136j-s
MM_DEBUG :=false

#libmmjpeg_defines := -g -O3 $(CPU)
libmmomx_enc_test_defines := -g -O3
libmmomx_enc_test_defines += $(KERNEL_HEADERS:%=-I%)

ifeq ($(MM_DEBUG),true)
libmmomx_enc_test_defines += -DVERBOSE -DMM_DEBUG
endif

common_libmmmpo_cflags := -fno-short-enums
common_libmmmpo_cflags += $(libmmomx_enc_test_defines)
common_libmmmpo_cflags += -D_ANDROID_
common_libmmmpo_cflags += -D_DEBUG

ifeq ($(strip $(TARGET_USES_ION)),true)
common_libmmmpo_cflags += -DUSE_ION
endif

ifeq ($(strip $(NEW_LOG_API)),true)
common_libmmmpo_cflags += -DNEW_LOG_API
endif

# ---------------------------------------------------------------------------------
#                       Make the tests (mm-jpeg-enc-test-client)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(OMX_TEST_CLIENT_PATH)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg2/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg2/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg2/src/os
LOCAL_C_INCLUDES += $(OMX_HEADER_DIR)
LOCAL_C_INCLUDES += omx_core.h
LOCAL_C_INCLUDES += hardware/qcom/camera
ifneq ($(strip $(USE_BIONIC_HEADER)),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_CFLAGS += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/msm_ion.h
endif

LOCAL_SRC_FILES := omx_jpeg_enc_test.c

LOCAL_MODULE            := mm-jpeg-enc-test-client
LOCAL_32_BIT_ONLY       := true
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS           := $(common_libmmmpo_cflags)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libcutils libmmstillomx libmmjpeg

LOCAL_CFLAGS+= $(KERNEL_HEADERS:%=-I%)
ifeq ($(strip $(USES_GEMINI)),true)
LOCAL_CFLAGS+= -DGEMINI_HW_ENCODE
endif

include $(BUILD_EXECUTABLE)

ifeq ($(BUILD_UNIFIED_CODE),false)
# ---------------------------------------------------------------------------------
#                        Make the tests (mm-jpeg-dec-test)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(OMX_TEST_CLIENT_PATH)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg2/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg2/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg2/src/os
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/omxcore
LOCAL_C_INCLUDES += omx_core.h
LOCAL_C_INCLUDES += hardware/qcom/camera
ifneq ($(strip $(USE_BIONIC_HEADER)),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_CFLAGS += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/msm_ion.h
endif

LOCAL_SRC_FILES	:= omx_jpeg_dec_test.c

LOCAL_MODULE		:= mm-jpeg-dec-test-client
LOCAL_32_BIT_ONLY       := true
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS           := $(common_libmmmpo_cflags)
LOCAL_PRELINK_MODULE	:= false
LOCAL_SHARED_LIBRARIES	:= libcutils libmmstillomx libmmjpeg libmmmpod

LOCAL_CFLAGS+= $(KERNEL_HEADERS:%=-I%)
ifeq ($(strip $(USES_GEMINI)),true)
LOCAL_CFLAGS+= -DGEMINI_HW_ENCODE
endif

include $(BUILD_EXECUTABLE)
endif

