#!/bin/bash
# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

set -e

if [ ${ANDROID_BUILD_TOP} ]; then
	
    #MIMERDIR=${ANDROID_BUILD_TOP}/external/mimer
    MIMERDIR=$( cd ${0%/*}/.. && pwd -P )

    MODULE_TYPE=provider
    MODULE_NAME=CalendarProvider
    MODULE_SRC=${ANDROID_BUILD_TOP}/packages/providers/${MODULE_NAME}
    NEW_PACKAGE=com.android.providers.calendar
    ORG_PACKAGE=com.android.providers.calendar
    MIMER_IDENT=calendar
    MIMER_PWD=calendarpswd

    . ${MIMERDIR}/scripts/convert_module.inc


#######################################################
#                BEGIN MODULE SPECIFIC CODE           #
#######################################################

   if [ "${ANDROID_MAJOR_VERSION}" = "4.0" -o "${ANDROID_MAJOR_VERSION}" = "4.1" -o "${ANDROID_MAJOR_VERSION}" = "4.2" -o  "${ANDROID_MAJOR_VERSION}" = "4.3" -o "${ANDROID_MAJOR_VERSION}" = "4.4" ]; 
   then
	patch ${MODULE_DST}/src/${NEW_PACKAGE_PATH}/CalendarProvider2.java ${SCRIPTDIR}/mimer_CalendarProvider2_v4.patch 
   fi

#######################################################
#                END MODULE SPECIFIC CODE             #
#######################################################
else
    echo "ANDROID_BUILD_TOP is not defined."2
fi
echo "=== Done"


