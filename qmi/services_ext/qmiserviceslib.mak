# -*- Mode: text -*-
#==============================================================================
# FILE: qmiserviceslib.mak
#
# SERVICE: 
#
# DESCRIPTION:
#
# Copyright 2012 Qualcomm Technologies, Inc. All rights reserved.
#==============================================================================

default: all

RELPATHTOROOT = ../../../../..

TARGET = libqmiservices
MAKEFILETYPE = shared
MAKEFILENAME = qmiserviceslib.mak
DEFAULT_TARGET_ARCH=-m32

C_SRC =\
   common_v01.c\
   voice_service_v02.c\
   wireless_data_service_v01.c\
   wireless_messaging_service_v01.c\
   device_management_service_v01.c\
   network_access_service_v01.c\
   user_identity_module_v01.c\
   card_application_toolkit_v02.c\
   phonebook_manager_service_v01.c\
   control_service_v01.c\
   specific_absorption_rate_v01.c\
   qmi_ims_vt_v01.c\
   circuit_switched_video_telephony_v01.c\
   coexistence_manager_v01.c\
   ip_multimedia_subsystem_presence_v01.c\
   ip_multimedia_subsystem_settings_v01.c\
   ip_multimedia_subsystem_application_v01.c\
   radio_frequency_radiated_performance_enhancement_v01.c \
   persistent_device_configuration_v01.c

NEEDINC=\
   vendor/qcom/proprietary/qmi/inc\
   vendor/qcom/proprietary/qmi/src\
   vendor/qcom/proprietary/qmi/platform\
   vendor/qcom/proprietary/qmi/core/lib/inc\
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
