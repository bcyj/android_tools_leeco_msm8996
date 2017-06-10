ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
libmm-abl-oem-def := \
	-g -O3 -Dlrintf=_ffix_r \
	-D__align=__alignx \
	-D__alignx\(x\)=__attribute__\(\(__aligned__\(x\)\)\) \
	-D_POSIX_SOURCE -DPOSIX_C_SOURCE=199506L \
	-D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED=1 \
	-D_BSD_SOURCE=1 -D_SVID_SOURCE=1 \
	-D_GNU_SOURCE -DT_ARM -DQC_MODIFIED \
	-Dinline=__inline -DASSERT=ASSERT_FATAL \
	-Dsvcsm_create=svcrtr_create \
	-DTRACE_ARM_DSP -D_ANDROID_ \
	-DIPL_DEBUG_STANDALONE -DVERBOSE -D_DEBUG
# ---------------------------------------------------------------------------------
#                Make the Shared library (libmm-abl-oem)
# ---------------------------------------------------------------------------------
LOCAL_MODULE                    := libmm-abl-oem
LOCAL_MODULE_TAGS               := optional
LOCAL_CFLAGS                    := $(libmm-abl-oem-def)
LOCAL_ADDITIONAL_DEPENDENCIES   := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_C_INCLUDES                := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES                += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES                += $(TARGET_OUT_HEADERS)/pp/inc
LOCAL_C_INCLUDES                += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_SRC_FILES                 := src/abl_oem.c
LOCAL_COPY_HEADERS_TO           := pp/inc
LOCAL_COPY_HEADERS              := inc/abl_oem.h
LOCAL_COPY_HEADERS              += inc/abl_core_api.h
LOCAL_COPY_HEADERS              += inc/lib-postproc.h
LOCAL_COPY_HEADERS              += inc/abl_driver.h
LOCAL_COPY_HEADERS              += inc/disp_osal.h
LOCAL_MODULE_OWNER              := qcom
LOCAL_PROPRIETARY_MODULE        := true

include $(BUILD_SHARED_LIBRARY)

endif
