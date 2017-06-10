# LOCAL_PATH and the include path need to be hard-coded because wmsts is inside
# the qcril directory (bug in the Android makefile system).
# LOCAL_PATH reflects parent directory to ensure common objects are in
# separate pools for each RIL variant.

LOCAL_PATH := $(QCRIL_DIR)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := qcril_fusion/qcril.c
LOCAL_SRC_FILES += qcril_fusion/qcril_arb.c
LOCAL_SRC_FILES += qcril_fusion/qcril_cm.c
LOCAL_SRC_FILES += qcril_fusion/qcril_cm_api_map.c
LOCAL_SRC_FILES += qcril_fusion/qcril_cm_clist.c
LOCAL_SRC_FILES += qcril_fusion/qcril_cm_ons.c
LOCAL_SRC_FILES += qcril_fusion/qcril_cm_ss.c
LOCAL_SRC_FILES += qcril_fusion/qcril_cm_util.c
LOCAL_SRC_FILES += qcril_fusion/qcril_event.c

ifeq ($(call is-board-platform,msm7627_6x),true)
LOCAL_SRC_FILES += qcril_fusion/qcril_gstk.c
endif

LOCAL_SRC_FILES += qcril_fusion/qcril_log.c
LOCAL_SRC_FILES += qcril_fusion/qcril_map.c
LOCAL_SRC_FILES += qcril_fusion/qcril_other.c
LOCAL_SRC_FILES += qcril_fusion/qcril_other_api_map.c
LOCAL_SRC_FILES += qcril_fusion/qcril_pbm.c
LOCAL_SRC_FILES += qcril_fusion/qcril_pbm_api_map.c
LOCAL_SRC_FILES += qcril_fusion/qcril_reqlist.c
LOCAL_SRC_FILES += qcril_fusion/qcril_sms.c
LOCAL_SRC_FILES += qcril_fusion/qcril_sms_api_map.c
LOCAL_SRC_FILES += qcril_fusion/qcril_sms_util.c

ifeq ($(call is-board-platform,msm7627_6x),true)
LOCAL_SRC_FILES += qcril_fusion/qcril_mmgsdi.c
LOCAL_SRC_FILES += qcril_fusion/qcril_mmgsdi_common.c
LOCAL_SRC_FILES += qcril_fusion/qcril_mmgsdi_sec.c
endif

LOCAL_SRC_FILES += common/uim/qcril_gstk_qmi.c
LOCAL_SRC_FILES += common/uim/qcril_uim.c
LOCAL_SRC_FILES += common/uim/qcril_uim_card.c
LOCAL_SRC_FILES += common/uim/qcril_uim_file.c
LOCAL_SRC_FILES += common/uim/qcril_uim_security.c
LOCAL_SRC_FILES += common/uim/qcril_uim_util.c
LOCAL_SRC_FILES += common/uim/qcril_uim_queue.c
LOCAL_SRC_FILES += common/uim/qcril_uim_refresh.c
LOCAL_SRC_FILES += common/uim/qcril_uim_restart.c
LOCAL_SRC_FILES += common/uim/qcril_scws.c
LOCAL_SRC_FILES += common/uim/qcril_scws_opt.c
LOCAL_SRC_FILES += common/uim/qcril_uim_qcci.c

LOCAL_SHARED_LIBRARIES := libcutils 
LOCAL_SHARED_LIBRARIES += libutils 
LOCAL_SHARED_LIBRARIES += libril
LOCAL_SHARED_LIBRARIES += liboncrpc
LOCAL_SHARED_LIBRARIES += libauth
LOCAL_SHARED_LIBRARIES += libcm

ifeq ($(call is-board-platform,msm7627_6x),true)
LOCAL_SHARED_LIBRARIES += libmmgsdilib
LOCAL_SHARED_LIBRARIES += libgsdi_exp
LOCAL_SHARED_LIBRARIES += libgstk_exp
endif

LOCAL_SHARED_LIBRARIES += libnv
LOCAL_SHARED_LIBRARIES += libpbmlib
LOCAL_SHARED_LIBRARIES += libwms
LOCAL_SHARED_LIBRARIES += libwmsts

ifeq ($(call is-board-platform,msm7630_fusion),true)
  LOCAL_SHARED_LIBRARIES += libcm_fusion
  LOCAL_SHARED_LIBRARIES += libpbmlib_fusion
  LOCAL_SHARED_LIBRARIES += libwms_fusion
endif

LOCAL_SHARED_LIBRARIES += libril-qcril-hook-oem
LOCAL_SHARED_LIBRARIES += libqmi
LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libhardware_legacy
LOCAL_SHARED_LIBRARIES += libdsutils

ifneq ($(call is-board-platform,msm7627_6x),true)
# These targets have route look up available on modem
  LOCAL_SHARED_LIBRARIES += libdsi_netctrl
  LOCAL_SHARED_LIBRARIES += libqdp
else
# These targets do not have route look up available on modem
  LOCAL_SHARED_LIBRARIES += libdss
endif

# for asprinf
LOCAL_CFLAGS := -D_GNU_SOURCE

LOCAL_C_INCLUDES += $(LOCAL_PATH)/qcril_fusion/
LOCAL_C_INCLUDES += system/kernel_headers/
LOCAL_C_INCLUDES += bionic/libc/include/

LOCAL_C_INCLUDES += hardware/ril/include/telephony/

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/auth/inc/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/cm/inc/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/nv/inc/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/pbmlib/inc/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/wms/inc/

ifeq ($(call is-board-platform,msm7630_fusion),true)
  LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/cm_fusion/inc/
  LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/pbmlib_fusion/inc/
  LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/wms_fusion/inc/
endif

ifeq ($(call is-board-platform,msm7627_6x),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/gstk_exp/inc/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mmgsdilib/inc/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/gsdi_exp/inc/
endif


LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/oncrpc/inc/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/common/data/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/common/uim/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/wmsts/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/qcrilhook_oem/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/data/inc/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc/

# For API Definitions and enables 
LOCAL_CFLAGS   += $(remote_api_defines)
LOCAL_CFLAGS   += $(remote_api_enables)

# defines necessary for QCRIL code
LOCAL_CFLAGS += -DRIL_SHLIB
LOCAL_CFLAGS += -DFEATURE_MMGSDI_GSM
LOCAL_CFLAGS += -DFEATURE_AUTH
LOCAL_CFLAGS += -DPACKED=

LOCAL_CFLAGS += -DFEATURE_QCRIL_UUS
ifneq ($(call is-board-platform,msm7627_6x),true)
LOCAL_CFLAGS += -DFEATURE_QCRIL_HDR_RELB
endif
LOCAL_CFLAGS += -DFEATURE_QCRIL_PRL_INIT

# Define for UIM files using QMI

ifneq ($(call is-board-platform,msm7627_6x),true)
LOCAL_CFLAGS += -DFEATURE_QCRIL_UIM_QMI
LOCAL_CFLAGS += -DFEATURE_QCRIL_UIM_QMI_SIMIO_ASYNC_CALL
LOCAL_CFLAGS += -DFEATURE_QCRIL_UIM_QMI_RPC_QCRIL
LOCAL_CFLAGS += -DFEATURE_QCRIL_QMI_CAT
endif

ifdef FEATURE_QCRIL_TOOLKIT_SKIP_SETUP_EVT_LIST_FILTER
LOCAL_CFLAGS += -DFEATURE_QCRIL_TOOLKIT_SKIP_SETUP_EVT_LIST_FILTER
endif

# Define for DATA files using route look up available on modem
ifneq ($(call is-board-platform,msm7627_6x),true)
# These targets have route look up available on modem
  LOCAL_SRC_FILES += common/data/qcril_data_netctrl.c
  LOCAL_SRC_FILES += common/data/qcril_data_qos.c
  LOCAL_SRC_FILES += common/data/qcril_data_client.c
  LOCAL_SRC_FILES += common/data/qcril_data_utils.c
  LOCAL_CFLAGS += -DFEATURE_QCRIL_USE_NETCTRL
else
# These targets do not have route look up available on modem
  LOCAL_SRC_FILES += common/data/qcril_data.c
  LOCAL_SRC_FILES += common/data/qcril_datai.c
endif

# Define for DATA files when modem boots up after apps proc
ifneq ($(call is-board-platform,msm7627_6x),true)
# define for the case when modem boots up after apps proc
  LOCAL_CFLAGS += -DFEATURE_WAIT_FOR_MODEM
  LOCAL_CFLAGS += -DFEATURE_QCRIL_USE_QDP
endif

#Define for 7630 available modem support
ifeq ($(call is-board-platform,msm7630_fusion),true)
  LOCAL_CFLAGS += -DFEATURE_QCRIL_FUSION
  LOCAL_CFLAGS += -DFEATURE_QCRIL_ADB_LOG_ENABLE
  LOCAL_CFLAGS += -DFEATURE_QCRIL_EHRPD
  endif

ifeq ($(call is-board-platform-in-list, msm7630_fusion msm7630_surf),true)
  LOCAL_CFLAGS += -DFEATURE_QCRIL_LTE
  LOCAL_CFLAGS += -DFEATURE_QCRIL_IMS
endif

#Define for DSDS
ifeq ($(call is-board-platform-in-list,msm7627_surf msm7627a),true)
  LOCAL_CFLAGS += -DFEATURE_QCRIL_DSDS
endif

ifeq ($(call is-board-platform,msm7627a),true)
  LOCAL_CFLAGS += -DFEATURE_QCRIL_SUPS_CC_EXTEN
endif

ifeq ($(call is-board-platform-in-list,msm7630_surf msm7627_surf msm7627a),true)
  LOCAL_CFLAGS += -DFEATURE_QCRIL_WMS_PM
endif

ifeq ($(call is-board-platform-in-list, msm7627a),true)
  LOCAL_CFLAGS += -DFEATURE_QCRIL_WMS_ETWS
endif

ifeq ($(call is-board-platform-in-list, msm7627a),true)
  LOCAL_CFLAGS += -DFEATURE_QCRIL_SAR
  LOCAL_SHARED_LIBRARIES += librfm_sar
  LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/rfm_sar/inc/
endif

ifeq ($(call is-board-platform-in-list,msm7630_fusion msm7627_6x msm7630_surf),true)
LOCAL_CFLAGS += -DFEATURE_QCRIL_NCELL
endif


ifeq ($(call is-board-platform,msm7627_6x),true)
  LOCAL_CFLAGS += -DFEATURE_QCRIL_SUBS_CTRL
endif

ifeq ($(call is-board-platform,msm7627a),true)
  LOCAL_CFLAGS += -DFEATURE_QCRIL_EHRPD
endif

#Define for CM Call Control

# _cc functions not available
# remove targets from list as remote apis are updated
ifeq ($(call is-board-platform-in-list,msm7627_surf msm7627a msm7627_6x),true)
  LOCAL_CFLAGS += -DFEATURE_QCRIL_SUPS_CC_UNSUPP
endif

# _exten functions available
# cm_call_cmd_orig_exten3 function available
ifeq ($(call is-board-platform-in-list,msm7630_fusion msm7630_surf),true)
  LOCAL_CFLAGS += -DFEATURE_QCRIL_SUPS_CC_EXTEN
  LOCAL_CFLAGS += -DFEATURE_QCRIL_CALL_ORIG_EXTEN3
endif

ifeq ($(HAVE_QC_TIME_SERVICES),true)
  LOCAL_CFLAGS += -DFEATURE_QC_TIME_SERVICES
  LOCAL_C_INCLUDES += vendor/qcom/opensource/time-services
  LOCAL_SHARED_LIBRARIES += libtime_genoff
endif # HAVE_QC_TIME_SERVICES

# ONCRPC defines
LOCAL_CFLAGS += \
  -DFEATURE_ONCRPC \
  -Doncrpc_printf=printf \
  -DONCRPC_ERR_FATAL=ERR_FATAL \
  -DFEATURE_ONCRPC_ROUTER \
  -DFEATURE_ONCRPC_SERVER_CLEANUP_SUPPORT \

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

ifeq ($(call is-board-platform-in-list,msm7627_surf msm7627a msm7630_fusion msm7627_6x),true)
  LOCAL_CFLAGS += -DPOLL_CALL_STATE -DUSE_QMI
endif

ifeq ($(TARGET_PRODUCT),dream)
  LOCAL_CFLAGS += -DPOLL_CALL_STATE -DUSE_QMI
endif

LOCAL_CFLAGS += -include $(TOP)/bionic/libc/include/assert.h

LOCAL_LDLIBS += -lpthread
LOCAL_LDLIBS += -lrt

LOCAL_MODULE:= libril-qc-1

LOCAL_MODULE_TAGS := debug

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

