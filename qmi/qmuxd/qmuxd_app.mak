# -*- Mode: text -*-
#==============================================================================
# FILE: qmuxd_app.mak
#
# SERVICE: 
#
# DESCRIPTION:
#
# Copyright 2012 Qualcomm Technologies, Inc. All rights reserved.
#==============================================================================

default: all

RELPATHTOROOT=../../../../..

TARGET = qmuxd
MAKEFILETYPE = app
MAKEFILENAME = qmuxd_app.mak

C_SRC =\
   ../platform/linux_qmi_qmux_if_server.c\
   ../platform/qmi_platform_qmux_io.c\
   ../platform/qmi_platform.c\
   ../src/qmi_qmux.c\
   ../src/qmi_util.c\

NEEDINC=\
   vendor/qcom/proprietary/qmi/inc\
   vendor/qcom/proprietary/qmi/src\
   vendor/qcom/proprietary/qmi/platform\
   vendor/qcom/proprietary/qmi/core/lib/inc\
   vendor/qcom/proprietary/common/inc\
   vendor/qcom/proprietary/diag/include\
   system/core/include/cutils\

NEEDLIB =\
   system/core/liblog/liblog\
   system/core/libcutils/libcutils\
   frameworks/native/libs/utils/libutils\
   vendor/qcom/proprietary/data/dsutils/src/libdsutils\
   vendor/qcom/proprietary/diag/src/libdiag\
   vendor/qcom/proprietary/test/ril/strllib/libstrl\

LIBS = -lz

CFLAGS += -m32
CFLAGS += -DFEATURE_RILTEST
CFLAGS += -DFEATURE_DATA_LOG_QXDM
CFLAGS += -include string.h
CFLAGS += -D_XOPEN_SOURCE=500 ## pthreads recursive mutex

CFLAGS += -DFEATURE_QMI_ANDROID

LDFLAGS += -m32

BTBUILD=vendor/qcom/proprietary/test/ril/btbuild
include $(RELPATHTOROOT)/$(BTBUILD)/bt.driver.mki

-include ./${MAKEFILENAME}.depend.mki

#======================================================================
# Add Mannual dependencies and makefile overrides here
#======================================================================

#
#ifneq (, $(filter msm7630_fusion, $(QCOM_TARGET_PRODUCT)))
#   CFLAGS += -DFEATURE_QMI_FUSION
#   CFLAGS += -DFEATURE_WAIT_FOR_MODEM_HACK
#endif
#
#ifneq (, $(filter msm8660_surf msm8960, $(QCOM_TARGET_PRODUCT)))
#   CFLAGS += -DFEATURE_WAIT_FOR_MODEM_HACK
#endif

#EOF
