# -*- Mode: text -*-
#==============================================================================
# FILE: netmgrlib.mak
#
# SERVICE: 
#
# DESCRIPTION:
#
# Copyright 2012 Qualcomm Technologies, Inc. All rights reserved.
#==============================================================================

default: all

RELPATHTOROOT=../../../../../..

TARGET = libnetmgr
MAKEFILETYPE = shared
MAKEFILENAME = netmgrlib.mak

C_SRC =\
   netmgr_client.c\
   netmgr_netlink.c\
   netmgr_util.c

#   netmgr.c \
   netmgr_exec.c \
   netmgr_kif.c \
   netmgr_main.c \
   netmgr_netlink.c \
   netmgr_platform.c \
   netmgr_qmi.c \
   netmgr_sm.c \
   netmgr_sm_int.c \
   netmgr_tc.c \
   netmgr_util.c \
#netmgr_stubs.c \
#netmgr_test.c


NEEDINC=\
   vendor/qcom/proprietary/data/netmgr/inc\
   vendor/qcom/proprietary/qmi/inc\
   vendor/qcom/proprietary/qmi/platform\
   vendor/qcom/proprietary/data/dsutils/inc\
   vendor/qcom/proprietary/diag/include\
   vendor/qcom/proprietary/common/inc\

NEEDLIB =\

CFLAGS += -m32
CFLAGS += -DNETMGR_OFFTARGET
CFLAGS += -DFEATURE_DATA_LOG_QXDM
CFLAGS += -DFEATURE_DS_LINUX_NO_RPC
CFLAGS += -DFEATURE_DS_LINUX_ANDROID
CFLAGS += -DNETMGR_QOS_ENABLE
CFLAGS += -include string.h## need for memset in netmgr_client.c

LDFLAGS += -m32

BTBUILD=vendor/qcom/proprietary/test/ril/btbuild
include $(RELPATHTOROOT)/$(BTBUILD)/bt.driver.mki

-include ./${MAKEFILENAME}.depend.mki

#======================================================================
# Add Mannual dependencies and makefile overrides here
#======================================================================

#EOF
