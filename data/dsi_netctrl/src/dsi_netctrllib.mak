# -*- Mode: text -*-
#==============================================================================
# FILE: dsi_netctrllib.mak
#
# SERVICE: 
#
# DESCRIPTION:
#
# Copyright 2012 Qualcomm Technologies, Inc. All rights reserved.
#==============================================================================

default: all

RELPATHTOROOT=../../../../../..

TARGET = libdsi_netctrl
MAKEFILETYPE = shared
MAKEFILENAME = dsi_netctrllib.mak

C_SRC =\
   dsi_netctrl.c\
   dsi_netctrl_init.c\
   dsi_netctrli.c\
   dsi_netctrl_mni_cb.c\
   dsi_netctrl_mni.c\
   dsi_netctrl_multimodem.c\
   dsi_netctrl_netmgr.c\
   dsi_netctrl_platform.c\
   dsi_netctrl_qos.c\
   dsi_netctrl_cb_thrd.c\
   dsi_netctrl_embms.c\

NEEDINC=\
   vendor/qcom/proprietary/data/dsi_netctrl/inc\
   vendor/qcom/proprietary/data/qdi/inc\
   vendor/qcom/proprietary/qmi/inc\
   vendor/qcom/proprietary/qmi/platform\
   vendor/qcom/proprietary/diag/include\
   vendor/qcom/proprietary/data/dsutils/inc\
   vendor/qcom/proprietary/data/netmgr/inc\
   vendor/qcom/proprietary/common/inc\
   system/core/include\

NEEDLIB =\
   vendor/qcom/proprietary/data/dsutils/src/libdsutils\
   vendor/qcom/proprietary/data/netmgr/src/libnetmgr\
   vendor/qcom/proprietary/data/qdi/src/libqdi\

CFLAGS += -m32
CFLAGS += -DFEATURE_DATA_LOG_STDERR
CFLAGS += -DDSI_NETCTRL_OFFTARGET
CFLAGS += -DFEATURE_DSI_MULTIMODEM_ROUTELOOKUP
CFLAGS += -DFEATURE_DSI_TEST
CFLAGS += -Dstrlcpy=strncpy -Dstrlcat=strncat
CFLAGS += -include string.h

LDFLAGS += -m32

BTBUILD=vendor/qcom/proprietary/test/ril/btbuild
include $(RELPATHTOROOT)/$(BTBUILD)/bt.driver.mki

-include ./${MAKEFILENAME}.depend.mki

#======================================================================
# Add Mannual dependencies and makefile overrides here
#======================================================================

#EOF
