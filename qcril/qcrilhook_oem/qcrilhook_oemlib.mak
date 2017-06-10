# -*- Mode: text -*-
#==============================================================================
# FILE: qcrilhook_oemlib.mak
#
# SERVICE: 
#
# DESCRIPTION:
#
#==============================================================================

default: all

RELPATHTOROOT=../../../../..

TARGET = libril-qcril-hook-oem
MAKEFILETYPE = shared
MAKEFILENAME = qcrilhook_oemlib.mak

C_SRC =\
   qcrilhook_oem.c

NEEDINC=\
   hardware/ril/include/telephony\
   hardware/ril/include\
   vendor/qcom/proprietary/common/inc\

NEEDLIB =\

CFLAGS += -m32
LDFLAGS += -m32

BTBUILD=vendor/qcom/proprietary/test/ril/btbuild
include $(RELPATHTOROOT)/$(BTBUILD)/bt.driver.mki

-include ./${MAKEFILENAME}.depend.mki

#======================================================================
# Add Mannual dependencies and makefile overrides here
#======================================================================

#EOF
