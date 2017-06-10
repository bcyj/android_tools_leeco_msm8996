#==============================================================================
# FILE: libqmi_client_qmux.mak
#
# SERVICE: QMI
#
# DESCRIPTION: makefile to build libqmi_client_qmux.so
#
#==============================================================================

default: all

RELPATHTOROOT = ../../../../..

TARGET = libqmi_client_qmux
MAKEFILETYPE = shared
MAKEFILENAME = libqmi_client_qmux.mak

C_SRC =\
   ../platform/linux_qmi_qmux_if_client.c\
   ../platform/qmi_platform.c\
   ../src/qmi_qmux_if.c\
   ../src/qmi_util.c\

NEEDINC=\
   vendor/qcom/proprietary/qmi/inc\
   vendor/qcom/proprietary/qmi/src\
   vendor/qcom/proprietary/qmi/platform\
   vendor/qcom/proprietary/qmi/proxy\
   vendor/qcom/proprietary/qmi/core/lib/inc\
   vendor/qcom/proprietary/qmi/services\
   vendor/qcom/proprietary/common/inc\
   vendor/qcom/proprietary/data/inc\
   vendor/qcom/proprietary/diag/include\
   system/core/include/cutils\

NEEDLIB =\
   vendor/qcom/proprietary/diag/src/libdiag\
   vendor/qcom/proprietary/qmi/services/libqmiservices\
   system/core/libcutils/libcutils\
   system/core/liblog/liblog\

LIBS += -lpthread

CFLAGS += -m32
CFLAGS += -DFEATURE_RILTEST
CFLAGS += -include string.h

CFLAGS += -DFEATURE_QMI_ANDROID

# Logging Features. Turn any one ON at any time
#CFLAGS  += -DFEATURE_DATA_LOG_STDERR
#CFLAGS += -DFEATURE_DATA_LOG_ADB
CFLAGS += -DFEATURE_DATA_LOG_QXDM

LDFLAGS += -m32

BTBUILD=vendor/qcom/proprietary/test/ril/btbuild
include $(RELPATHTOROOT)/$(BTBUILD)/bt.driver.mki

-include ./${MAKEFILENAME}.depend.mki

#======================================================================
# Add Mannual dependencies and makefile overrides here
#======================================================================

#EOF
