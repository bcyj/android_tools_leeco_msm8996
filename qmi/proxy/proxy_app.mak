# -*- Mode: text -*-
#==============================================================================
# FILE: proxy_app.mak
#
# SERVICE: RIL Test
#
# DESCRIPTION: Compiles proxy for off-target test
#
#==============================================================================

default: all

RELPATHTOROOT=../../../../..

TARGET = qmiproxy
MAKEFILETYPE = app
MAKEFILENAME = proxy_app.mak

C_SRC =\
   qmi_proxy.c\
   ../src/qmi_client.c\
#   ../platform/linux_qmi_qmux_if_client.c\
#   ../platform/qmi_platform.c\
#   ../src/qmi_qmux_if.c\
#   ../src/qmi_util.c\

NEEDINC=\
   vendor/qcom/proprietary/qmi/inc\
   vendor/qcom/proprietary/qmi/src\
   vendor/qcom/proprietary/qmi/platform\
   vendor/qcom/proprietary/qmi/core/lib/inc\
   vendor/qcom/proprietary/qmi/services\
   vendor/qcom/proprietary/common/inc\
   vendor/qcom/proprietary/data/inc\
   vendor/qcom/proprietary/diag/include\
   system/core/include\
   frameworks/native/include\

NEEDLIB =\
   vendor/qcom/proprietary/qmi/libqmi\
   vendor/qcom/proprietary/qmi/services/libqmiservices\
   vendor/qcom/proprietary/qmi/core/lib/src/libqmiidl\
   vendor/qcom/proprietary/qmi/qmi_client_qmux/libqmi_client_qmux\
   vendor/qcom/proprietary/data/dsutils/src/libdsutils\
   vendor/qcom/proprietary/diag/src/libdiag\
   vendor/qcom/proprietary/test/ril/strllib/libstrl\
   system/core/liblog/liblog\
   system/core/libcutils/libcutils\

#libqmi_client_qmux
#libqcci_legacy

LIBS = -lpthread

CFLAGS += -m32

# Logging Features. Turn any one ON at any time
#CFLAGS  += -DFEATURE_DATA_LOG_STDERR
#CFLAGS += -DFEATURE_DATA_LOG_ADB
CFLAGS += -DFEATURE_DATA_LOG_QXDM
CFLAGS += -include string.h

CFLAGS += -DFEATURE_QMI_ANDROID
CFLAGS += -DFEATURE_DSS_LINUX_ANDROID

CFLAGS += -DFEATURE_RILTEST

LDFLAGS += -m32

BTBUILD=vendor/qcom/proprietary/test/ril/btbuild
include $(RELPATHTOROOT)/$(BTBUILD)/bt.driver.mki

-include ./${MAKEFILENAME}.depend.mki

#======================================================================
# Add Mannual dependencies and makefile overrides here
#======================================================================

#EOF




