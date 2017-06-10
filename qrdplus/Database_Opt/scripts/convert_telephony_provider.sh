#!/bin/bash
# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

set -e

if [ ${ANDROID_BUILD_TOP} ]; then
	
    #MIMERDIR=${ANDROID_BUILD_TOP}/external/mimer
    MIMERDIR=$( cd ${0%/*}/.. && pwd -P )
    
    MODULE_TYPE=provider
    MODULE_NAME=TelephonyProvider
    MODULE_SRC=${ANDROID_BUILD_TOP}/packages/providers/${MODULE_NAME}
    NEW_PACKAGE=com.android.providers.telephony
    ORG_PACKAGE=com.android.providers.telephony
    MIMER_IDENT=telephony
    MIMER_PWD=telephonypswd

    . ${MIMERDIR}/scripts/convert_module.inc


#######################################################
#                BEGIN MODULE SPECIFIC CODE           #
#######################################################


#######################################################
#                END MODULE SPECIFIC CODE             #
#######################################################
else
    echo "ANDROID_BUILD_TOP is not defined."2
fi
echo "=== Done"


