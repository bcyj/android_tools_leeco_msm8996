# -*- Mode: text -*-
#==============================================================================
# FILE: qcrillib.mak
#
# SERVICE: RIL
#
# DESCRIPTION:
#
#==============================================================================

default: all

RELPATHTOROOT = ../../../..

TARGET = libril-qc-qmi-1
MAKEFILETYPE = shared
MAKEFILENAME = qcrillib.mak

C_SRC =\
   qcril_qmi/qcril.c\
   qcril_qmi/qcril_arb.c\
   qcril_qmi/qcril_cm_ss.c\
   qcril_qmi/qcril_cm_util.c\
   qcril_qmi/qcril_event.c\
   qcril_qmi/qcril_log.c\
   qcril_qmi/qcril_other.c\
   qcril_qmi/qcril_pbm.c\
   qcril_qmi/qcril_qmi_client.c\
   qcril_qmi/qcril_qmi_ims.c\
   qcril_qmi/qcril_qmi_imss.c\
   qcril_qmi/qcril_qmi_nas.c\
   qcril_qmi/qcril_qmi_nas2.c\
   qcril_qmi/qcril_qmi_sms.c\
   qcril_qmi/qcril_qmi_sms_errors.c\
   qcril_qmi/qcril_qmi_voice.c\
   qcril_qmi/qcril_reqlist.c\
   qcril_qmi/qcril_qmi_imsa.c\
   qcril_qmi/services/qmi_embms_v01.c\
   qcril_qmi/services/qtuner_v01.c\
   qcril_qmi/qcril_qmi_coex.c\
   qcril_qmi/ims_socket/imsIF.pb-c.c\
   qcril_qmi/ims_socket/qcril_qmi_ims_if_pb.c\
   qcril_qmi/ims_socket/qcril_qmi_ims_misc.c\
   qcril_qmi/ims_socket/qcril_qmi_ims_packing.c\
   common/uim/qcril_gstk_qmi.c\
   common/uim/qcril_uim.c\
   common/uim/qcril_uim_card.c\
   common/uim/qcril_uim_file.c\
   common/uim/qcril_uim_security.c\
   common/uim/qcril_uim_util.c\
   common/uim/qcril_uim_qcci.c\
   common/uim/qcril_uim_queue.c\
   common/uim/qcril_uim_refresh.c\
   common/uim/qcril_uim_restart.c\
   common/uim/qcril_scws.c\
   common/uim/qcril_scws_opt.c\
   common/data/qcril_data_netctrl.c\
   common/data/qcril_data_qos.c\
   common/data/qcril_data_utils.c\
   common/data/qcril_data_embms.c\
   common/data/qcril_data_client.c\

C++_SRC=\
   qcril_qmi/ims_socket/qcril_qmi_ims_socket.cc\
   qcril_qmi/oem_socket/qcril_qmi_oem_socket.cc\
   qcril_qmi/qcril_qmi_generic_socket.cc\
   qcril_qmi/qcril_qmi_pil_monitor.cc\

NEEDINC=\
   frameworks/native/include\
   hardware/ril/include\
   hardware/ril/include/telephony \
   hardware/libhardware_legacy/include\
   vendor/qcom/proprietary/qmi/src\
   vendor/qcom/proprietary/common/inc/\
   vendor/qcom/proprietary/diag/include\
   vendor/qcom/proprietary\
   vendor/qcom/proprietary/qmi/inc/\
   vendor/qcom/proprietary/qmi/platform/\
   vendor/qcom/proprietary/qmi/services\
   vendor/qcom/proprietary/qmi/core/lib/inc\
   vendor/qcom/proprietary/data/dsi_netctrl/inc\
   vendor/qcom/proprietary/data/dsutils/inc\
   vendor/qcom/proprietary/data/qdp/inc\
   vendor/qcom/proprietary/qcril/qcril_qmi\
   vendor/qcom/proprietary/qcril/qcril_qmi/ims_socket\
   vendor/qcom/proprietary/qcril/qcril_qmi/oem_socket\
   vendor/qcom/proprietary/qcril/common/uim\
   vendor/qcom/proprietary/qcril/common/data\
   vendor/qcom/proprietary/qcril/qcrilhook_oem\
   vendor/qcom/proprietary/qcril\
   system/core/include\
   system/core/include/cutils\
   external/protobuf-c/src\

## Need this to ensure we pickup qmi/services dir instead of qcril_qmi/services dir 
NEEDINC +=\
   vendor/qcom/proprietary/qcril/qcril_qmi/services\
   vendor/qcom/proprietary/qmi/\


NEEDLIB =\
   vendor/qcom/proprietary/data/dsutils/src/libdsutils\
   vendor/qcom/proprietary/qmi/libqmi\
   vendor/qcom/proprietary/qmi/services/libqmiservices\
   vendor/qcom/proprietary/qmi/core/lib/src/libqmiidl\
   vendor/qcom/proprietary/diag/src/libdiag\
   vendor/qcom/proprietary/data/qdp/src/libqdp\
   vendor/qcom/proprietary/data/dsi_netctrl/src/libdsi_netctrl\
   vendor/qcom/proprietary/qcril/qcrilhook_oem/libril-qcril-hook-oem\
   external/protobuf-c/src/libprotobuf\

## Note: need strl functions for off-target, they are not present in GNU libs
NEEDLIB +=\
   vendor/qcom/proprietary/test/ril/strllib/libstrl\

LIBS +=\
   -lpthread\
   -lrt

CFLAGS += -m32
CFLAGS += -DRIL_SHLIB
CFLAGS += -DQCRIL_DATA_OFFTARGET
CFLAGS += -DFEATURE_TARGET_GLIBC_x86

## Define for UIM files using QMI
CFLAGS += -DFEATURE_QCRIL_UIM_QMI
CFLAGS += -DFEATURE_QCRIL_UIM_QMI_SIMIO_ASYNC_CALL
CFLAGS += -DFEATURE_QCRIL_QMI_CAT
CFLAGS += -DFEATURE_DATA_EMBMS
CFLAGS += -DFEATURE_QCRIL_USE_NETCTRL
CFLAGS += -DFEATURE_QCRIL_USE_QDP
CFLAGS += -DFEATURE_QCRIL_8960## use sys_info msgs instead of serving system

## Obsolete defines?
#
#CFLAGS += -DANDROID
## for asprinf
#CFLAGS += -D_GNU_SOURCE
## defines necessary for QCRIL code
#CFLAGS += -DFEATURE_MMGSDI_GSM
#CFLAGS += -DFEATURE_AUTH
#CFLAGS += -DPACKED=
#CFLAGS += -DFEATURE_QCRIL_UUS
#CFLAGS += -DFEATURE_QCRIL_HDR_RELB
#CFLAGS += -DFEATURE_QCRIL_NCELL
#
## defines common to all shared libraries
#CFLAGS += \
#  -DLOG_NDDEBUG=0 \
#  -D__packed__= \
#  -DIMAGE_APPS_PROC \
#  -DFEATURE_Q_SINGLE_LINK \
#  -DFEATURE_Q_NO_SELF_QPTR \
#  -DFEATURE_NATIVELINUX \
#  -DFEATURE_DSM_DUP_ITEMS \
#
## compiler options
#CFLAGS += -fno-inline
#CFLAGS += -fno-short-enums

LDFLAGS += -m32

BTBUILD=vendor/qcom/proprietary/test/ril/btbuild
include $(RELPATHTOROOT)/$(BTBUILD)/bt.driver.mki

#======================================================================
# Insert manual overrides here:
#======================================================================
