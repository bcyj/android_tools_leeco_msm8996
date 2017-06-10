LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# --------------------------------------------------------------
#      Make the ipl library (libmmipl)
# --------------------------------------------------------------
include $(CLEAR_VARS)

#CPU     := -mcpu=arm1136j-s
MM_DEBUG:=false

libmmipl_cflags := -g -O3 \
	$(CPU)
	-Dlrintf=_ffix_r \
	-D__alignx\(x\)=__attribute__\(\(__aligned__\(x\)\)\)
	-D_POSIX_SOURCE \
	-DPOSIX_C_SOURCE=199506L \
	-D_XOPEN_SOURCE=600 \
	-D_XOPEN_SOURCE_EXTENDED=1 \
	-D_BSD_SOURCE=1 \
	-D_SVID_SOURCE=1 \
	-D_GNU_SOURCE \
	-DT_ARM \
	-D__MSMHW_MODEM_PROC__=1 \
	-D__MSMHW_APPS_PROC__=2 \
	-D__MSMHW_PROC_DEF__=__MSMHW_APPS_PROC__ \
	-DMSMHW_MODEM_PROC -DMSMHW_APPS_PROC \
	-DIMAGE_APPS_PROC \
	-DQC_MODIFIED -Dinline=__inline \
	-DASSERT=ASSERT_FATAL \
	-Dsvcsm_create=svcrtr_create \
	-DCONFIG_MSM7600 \
	-I./system/msm-rpc/libadsp/ -I./system/msm-rpc/librpc/rpc $(KERNEL_HEADERS:%=-I%) \

libmmipl_defines:= \
	-fno-short-enums \
	$(libmmipl_defines)\
	-DIPL_DEBUG_STANDALONE \
	-DFEATURE_QDSP_RTOS \
	-DTRACE_ARM_DSP \
	-DMSM7600 \
	-D_ANDROID_ \
	-D_DEBUG \
	-DTRUE="1" \
	-DFALSE="0"

ifeq ($(MM_DEBUG),true)
libmmipl_defines += -DVERBOSE -DMM_DEBUG
endif

libmmipl_includes        += $(LOCAL_PATH)/src
libmmipl_includes        += $(LOCAL_PATH)/inc
libmmipl_includes        += $(LOCAL_PATH)/../../mm-camera/common
ifneq ($(strip $(USE_BIONIC_HEADER)),true)
libmmipl_includes        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
libmmipl_includes        += $(LOCAL_PATH)/../../../../../hardware/qcom/camera

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_CFLAGS:=$(libmmipl_defines)

LOCAL_SRC_FILES:= \
	src/ipl_attic.c \
	src/ipl_compose.c \
	src/ipl_convert.c \
	src/ipl_downSize.c \
	src/ipl_efx.c \
	src/ipl_helper.c \
	src/ipl_hjr.c \
	src/ipl_rotAddCrop.c \
	src/ipl_upSize.c \
	src/ipl_util.c \
	src/ipl_xform.c

LOCAL_C_INCLUDES	:= $(libmmipl_includes)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../../../hardware/qcom/camera
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES	:= libcutils

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog libutils
endif

LOCAL_MODULE:=libmmipl

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qcom
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
