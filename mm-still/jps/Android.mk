JPS_PATH := $(call my-dir)

include $(BUILD_MULTI_PREBUILT)

# ---------------------------------------------------------------------------------
#                       Common definitons
# ---------------------------------------------------------------------------------

#CPU := -mcpu=arm1136j-s
MM_DEBUG :=false

#libmmjpeg_defines := -g -O3 $(CPU)
libmmjps_defines := -g -O3
libmmjps_defines += $(KERNEL_HEADERS:%=-I%)

ifeq ($(MM_DEBUG),true)
libmmjps_defines += -DVERBOSE -DMM_DEBUG
endif

common_libmmjps_cflags := -fno-short-enums
common_libmmjps_cflags += $(libmmjps_defines)
common_libmmjps_cflags += -D_ANDROID_
common_libmmjps_cflags += -D_DEBUG

ifeq ($(strip $(NEW_LOG_API)),true)
common_libmmjps_cflags += -DNEW_LOG_API
endif

# ---------------------------------------------------------------------------------
# 			Make the shared library (libmmjpeg)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(JPS_PATH)

libmmjps_includes += $(LOCAL_PATH)/inc
libmmjps_includes += $(LOCAL_PATH)/src
libmmjps_includes += $(LOCAL_PATH)/../jpeg2/src
libmmjps_includes += $(LOCAL_PATH)/../jpeg2/src/os
libmmjps_includes += $(LOCAL_PATH)/../jpeg2/inc
libmmjps_includes += hardware/qcom/camera
ifneq ($(strip $(USE_BIONIC_HEADER)),true)
libmmjps_includes += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
libmmjps_includes += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
else
libmmjps_includes+= bionic/libc/kernel/common/media
endif

#LOCAL_COPY_HEADERS_TO := mm-still/jps
#LOCAL_COPY_HEADERS := inc/jpse.h

LOCAL_MODULE           := libmmjps
LOCAL_32_BIT_ONLY       := true
LOCAL_MODULE_TAGS      := optional
LOCAL_CFLAGS           := $(common_libmmjps_cflags)
LOCAL_C_INCLUDES       := $(libmmjps_includes)
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/../../../../../hardware/qcom/camera
LOCAL_PRELINK_MODULE   := false
LOCAL_SHARED_LIBRARIES := libcutils libmmjpeg

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog libutils
endif

LOCAL_SRC_FILES := src/jpse.c
LOCAL_SRC_FILES += src/jps_writer.c
LOCAL_SRC_FILES += src/jpse_englist_sw_only.c

LOCAL_MODULE_OWNER := qcom
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
# 			Make the tests (mm-jps-enc-test)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(JPS_PATH)

mm-jps-inc += $(LOCAL_PATH)/inc
mm-jps-inc += $(LOCAL_PATH)/../jpeg2/inc
mm-jps-inc += $(LOCAL_PATH)/../jpeg2/src
mm-jps-inc += $(LOCAL_PATH)/../jpeg2/src/os
mm-jps-inc += $(LOCAL_PATH)/src
ifneq ($(strip $(USE_BIONIC_HEADER)),true)
mm-jps-inc += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
mm-jps-inc += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_MODULE		:= mm-jps-enc-test
LOCAL_32_BIT_ONLY       := true
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS            := $(common_libmmjps_cflags)
LOCAL_C_INCLUDES  	:= $(mm-jps-inc)
LOCAL_C_INCLUDES    += $(LOCAL_PATH)/../../../../../hardware/qcom/camera
LOCAL_PRELINK_MODULE	:= false
LOCAL_SHARED_LIBRARIES	:= libcutils libmmjpeg libmmjps

ifeq ($(strip $(USES_GEMINI)),true)
LOCAL_CFLAGS+= -DGEMINI_HW_ENCODE
endif

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES 	+= liblog libutils
endif

LOCAL_SRC_FILES	:= test/jpse_test.c
LOCAL_SRC_FILES += test/ppf_jpeg_header.c

include $(BUILD_EXECUTABLE)


# ---------------------------------------------------------------------------------
#                                      END
# ---------------------------------------------------------------------------------
