OMX_TEST_PATH := $(call my-dir)


############ encoder ################################################
include $(CLEAR_VARS)
LOCAL_PATH := $(OMX_TEST_PATH)
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID

ifneq (,$(findstring $(PLATFORM_VERSION), 4.4 4.4.1 4.4.2 4.4.3))
LOCAL_CFLAGS += -Werror
endif

LOCAL_CFLAGS += -D_ANDROID_
LOCAL_CFLAGS += -include QIDbg.h

ifeq ($(strip $(TARGET_USES_ION)),true)
LOCAL_CFLAGS += -DUSE_ION
endif

OMX_HEADER_DIR := frameworks/native/include/media/openmax
OMX_CORE_DIR := hardware/qcom/camera/mm-image-codec

LOCAL_C_INCLUDES := $(OMX_HEADER_DIR)
LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qomx_core
LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qexif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../utils
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../exif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)

ifneq ($(strip $(USE_BIONIC_HEADER)),true)
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
endif
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr


LOCAL_SRC_FILES := buffer_test.c
LOCAL_SRC_FILES += qomx_jpeg_enc_test.c

LOCAL_MODULE           := mm-qomx-ienc-test
LOCAL_32_BIT_ONLY       := true
LOCAL_PRELINK_MODULE   := false
LOCAL_SHARED_LIBRARIES := libcutils libdl libqomx_core

include $(BUILD_EXECUTABLE)


############ decoder ################################################
include $(CLEAR_VARS)
LOCAL_PATH := $(OMX_TEST_PATH)
LOCAL_MODULE_TAGS := optional

ifneq (,$(findstring $(PLATFORM_VERSION), 4.4 4.4.1 4.4.2 4.4.3))
LOCAL_CFLAGS += -Werror
endif

LOCAL_CFLAGS += -D_ANDROID_
LOCAL_CFLAGS += -include QIDbg.h

ifeq ($(strip $(TARGET_USES_ION)),true)
LOCAL_CFLAGS += -DUSE_ION
endif

OMX_HEADER_DIR := frameworks/native/include/media/openmax
OMX_CORE_DIR := hardware/qcom/camera/mm-image-codec

LOCAL_C_INCLUDES := $(OMX_HEADER_DIR)
LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qomx_core
LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qexif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../utils
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../exif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)

ifneq ($(strip $(USE_BIONIC_HEADER)),true)
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES+= $(LOCAL_PATH)../../../../../../hardware/qcom/camera
endif
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr


LOCAL_SRC_FILES := buffer_test.c
LOCAL_SRC_FILES += qomx_jpeg_dec_test.c

LOCAL_MODULE           := mm-qomx-idec-test
LOCAL_32_BIT_ONLY       := true
LOCAL_PRELINK_MODULE   := false
LOCAL_SHARED_LIBRARIES := libcutils libdl libqomx_core

include $(BUILD_EXECUTABLE)
