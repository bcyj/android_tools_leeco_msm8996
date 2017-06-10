# -*- Mode: text -*-
#==============================================================================
# FILE: qdilib.mak
#
# SERVICE: 
#
# DESCRIPTION:
#
# Copyright 2012 Qualcomm Technologies, Inc. All rights reserved.
#==============================================================================

default: all

RELPATHTOROOT=../../../../../..

TARGET = libqdi
MAKEFILETYPE = shared
MAKEFILENAME = qdilib.mak

C_SRC =\
   qdi.c\
   qdi_netlink.c\

NEEDINC=\
   vendor/qcom/proprietary/data/qdi/inc\
   vendor/qcom/proprietary/diag/include\
   vendor/qcom/proprietary/qmi/inc\
   vendor/qcom/proprietary/qmi/platform\
   vendor/qcom/proprietary/data/dsutils/inc\
   vendor/qcom/proprietary/data/dsi_netctrl/inc\
   vendor/qcom/proprietary/common/inc\

NEEDLIB =\
   vendor/qcom/proprietary/qmi/libqmi\
   vendor/qcom/proprietary/diag/src/libdiag\
   vendor/qcom/proprietary/data/dsutils/src/libdsutils\

CFLAGS += -m32
CFLAGS += -DDSI_NETCTRL_OFFTARGET
CFLAGS += -DFEATURE_DSI_MULTIMODEM_ROUTELOOKUP
CFLAGS += -DFEATURE_DSI_TEST
CFLAGS += -Dstrlcpy=strncpy -Dstrlcat=strncat
CFLAGS += -DQDI_OFFTARGET
CFLAGS += -DFEATURE_DATA_LOG_QXDM

LDFLAGS += -m32

BTBUILD=vendor/qcom/proprietary/test/ril/btbuild
include $(RELPATHTOROOT)/$(BTBUILD)/bt.driver.mki

-include ./${MAKEFILENAME}.depend.mki

#======================================================================
# Add Mannual dependencies and makefile overrides here
#======================================================================

#EOF
