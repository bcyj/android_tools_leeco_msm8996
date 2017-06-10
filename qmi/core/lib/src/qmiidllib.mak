# -*- Mode: text -*-
#==============================================================================
# FILE: qmiidllib.mak
#
# SERVICE: 
#
# DESCRIPTION:
#
# Copyright 2012 Qualcomm Technologies, Inc. All rights reserved.
#==============================================================================

default: all

RELPATHTOROOT = ../../../../../../..

TARGET = libqmiidl
MAKEFILETYPE = shared
MAKEFILENAME = qmiidllib.mak
DEFAULT_TARGET_ARCH=-m32

C_SRC =\
   qmi_idl_lib.c\
   qmi_idl_accessor.c\

NEEDINC=\
   vendor/qcom/proprietary/qmi/core/lib/inc\
   vendor/qcom/proprietary/qmi/inc\
   vendor/qcom/proprietary/qmi/platform\
   vendor/qcom/proprietary/common/inc\

NEEDLIB =\

BTBUILD=vendor/qcom/proprietary/test/ril/btbuild
include $(RELPATHTOROOT)/$(BTBUILD)/bt.driver.mki

-include ./${MAKEFILENAME}.depend.mki

#======================================================================
# Add Mannual dependencies and makefile overrides here
#======================================================================
x86_64:
	make -f ${MAKEFILENAME} TARGET_ARCH=-m64

#EOF
