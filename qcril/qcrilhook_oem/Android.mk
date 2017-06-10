# LOCAL_PATH and the include path need to be hard-coded because qcrilhook is inside
# the qcril directory (bug in the Android makefile system).

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := qcrilhook_oem.c

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libril
LOCAL_SHARED_LIBRARIES += libdiag

# for asprinf
LOCAL_CFLAGS := -D_GNU_SOURCE

LOCAL_C_INCLUDES := system/kernel_headers/
LOCAL_C_INCLUDES += hardware/ril/include/telephony/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

# defines necessary for QCRIL code
LOCAL_CFLAGS += -DRIL_SHLIB
LOCAL_CFLAGS += -DPACKED=

# defines common to all shared libraries
LOCAL_CFLAGS += \
  -DLOG_NDDEBUG=0 \
  -D__packed__= \
  -DIMAGE_APPS_PROC \
  -DFEATURE_Q_SINGLE_LINK \
  -DFEATURE_Q_NO_SELF_QPTR \
  -DFEATURE_NATIVELINUX \
  -DFEATURE_DSM_DUP_ITEMS \

# compiler options
LOCAL_CFLAGS += -g
LOCAL_CFLAGS += -O0
LOCAL_CFLAGS += -fno-inline
LOCAL_CFLAGS += -fno-short-enums

# Google defines
ifeq ($(TARGET_PRODUCT),sooner)
  LOCAL_CFLAGS += -DOMAP_CSMI_POWER_CONTROL -DUSE_TI_COMMANDS 
endif

ifeq ($(call is-board-platform-in-list, msm7627_surf msm7627a msm7630_surf msm7630_fusion msm8660 msm7627_6x),true)
  LOCAL_CFLAGS += -DPOLL_CALL_STATE -DUSE_QMI
endif

ifeq ($(TARGET_PRODUCT),dream)
  LOCAL_CFLAGS += -DPOLL_CALL_STATE -DUSE_QMI
endif

LOCAL_MODULE:= libril-qcril-hook-oem

LOCAL_MODULE_TAGS := optional debug

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
