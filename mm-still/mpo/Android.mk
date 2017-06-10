MPO_PATH := $(call my-dir)

include $(BUILD_MULTI_PREBUILT)

# ---------------------------------------------------------------------------------
#                       Common definitons
# ---------------------------------------------------------------------------------

#CPU := -mcpu=arm1136j-s
MM_DEBUG :=false

#libmmjpeg_defines := -g -O3 $(CPU)
libmmmpo_defines := -g -O3
libmmmpo_defines += $(KERNEL_HEADERS:%=-I%)

ifeq ($(MM_DEBUG),true)
libmmmpo_defines += -DVERBOSE -DMM_DEBUG
endif

common_libmmmpo_cflags := -fno-short-enums
common_libmmmpo_cflags += $(libmmmpo_defines)
common_libmmmpo_cflags += -D_ANDROID_
common_libmmmpo_cflags += -D_DEBUG

ifeq ($(strip $(NEW_LOG_API)),true)
common_libmmmpo_cflags += -DNEW_LOG_API
endif

# ---------------------------------------------------------------------------------
# 			Make the shared library (libmmjpeg)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(MPO_PATH)

libmmmpo_includes += $(LOCAL_PATH)/inc
libmmmpo_includes += $(LOCAL_PATH)/src
libmmmpo_includes += $(LOCAL_PATH)/../jpeg2/src
libmmmpo_includes += $(LOCAL_PATH)/../jpeg2/src/os
libmmmpo_includes += $(LOCAL_PATH)/../jpeg2/inc
ifneq ($(strip $(USE_BIONIC_HEADER)),true)
libmmmpo_includes += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
else
libmmmpo_includes += bionic/libc/kernel/common/media
endif

#LOCAL_COPY_HEADERS_TO := mm-still/mpo
#LOCAL_COPY_HEADERS := inc/mpoe.h

LOCAL_MODULE           := libmmmpo
LOCAL_32_BIT_ONLY       := true
LOCAL_MODULE_TAGS      := optional
LOCAL_CFLAGS           := $(common_libmmmpo_cflags)
LOCAL_C_INCLUDES       := $(libmmmpo_includes)
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/../../../../../hardware/qcom/camera
LOCAL_PRELINK_MODULE   := false
LOCAL_SHARED_LIBRARIES := libcutils libmmjpeg

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog libutils
endif

LOCAL_SRC_FILES := src/mpoe.c
LOCAL_SRC_FILES += src/mpo.c
#LOCAL_SRC_FILES += src/mpo_info.c
LOCAL_SRC_FILES += src/mpo_writer.c


LOCAL_MODULE_OWNER := qcom
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)


# ---------------------------------------------------------------------------------
# 			Make the tests (mm-mpo-enc-test)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(MPO_PATH)

mm-mpo-inc += $(LOCAL_PATH)/inc
mm-mpo-inc += $(LOCAL_PATH)/../jpeg2/inc
mm-mpo-inc += $(LOCAL_PATH)/../jpeg2/src
mm-mpo-inc += $(LOCAL_PATH)/../jpeg2/src/os
mm-mpo-inc += $(LOCAL_PATH)/src
ifneq ($(strip $(USE_BIONIC_HEADER)),true)
mm-mpo-inc += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_MODULE		:= mm-mpo-enc-test
LOCAL_32_BIT_ONLY       := true
LOCAL_MODULE_TAGS      := optional
LOCAL_CFLAGS            := $(common_libmmmpo_cflags)
LOCAL_C_INCLUDES  	:= $(mm-mpo-inc)
LOCAL_C_INCLUDES    += $(LOCAL_PATH)/../../../../../hardware/qcom/camera
LOCAL_PRELINK_MODULE	:= false
LOCAL_SHARED_LIBRARIES	:= libcutils libmmjpeg libmmmpo

ifeq ($(strip $(USES_GEMINI)),true)
LOCAL_CFLAGS+= -DGEMINI_HW_ENCODE
endif

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES 	+= liblog libutils
endif

LOCAL_SRC_FILES	:= test/mpoe_test.c
include $(BUILD_EXECUTABLE)




# ---------------------------------------------------------------------------------
#                                      END
# ---------------------------------------------------------------------------------
