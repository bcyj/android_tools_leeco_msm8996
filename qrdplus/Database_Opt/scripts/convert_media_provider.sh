#!/bin/bash
# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
set -e

if [ ${ANDROID_BUILD_TOP} ]; then
	
    #MIMERDIR=${ANDROID_BUILD_TOP}/external/mimer
    MIMERDIR=$( cd ${0%/*}/.. && pwd -P )

    MODULE_TYPE=provider
    MODULE_NAME=MediaProvider
    MODULE_SRC=${ANDROID_BUILD_TOP}/packages/providers/${MODULE_NAME}
    NEW_PACKAGE=com.android.providers.media
    ORG_PACKAGE=com.android.providers.media
    MIMER_IDENT=media
    MIMER_PWD=mediapswd

    . ${MIMERDIR}/scripts/convert_module.inc


#######################################################
#                BEGIN MODULE SPECIFIC CODE           #
#######################################################

    if [ "${ANDROID_MAJOR_VERSION}" = "4.0" -o "${ANDROID_MAJOR_VERSION}" = "4.1" -o "${ANDROID_MAJOR_VERSION}" = "4.2" -o "${ANDROID_MAJOR_VERSION}" = "4.3" -o "${ANDROID_MAJOR_VERSION}" = "4.4" ]; then
        if [ ${MODULE_SRC} != ${MODULE_DST} ]; then
	    cp ${MODULE_SRC}/src/com/android/providers/media/IMtpService.aidl ${MODULE_DST}/src/${NEW_PACKAGE_PATH}
	    if [ ${NEW_PACKAGE} != ${ORG_PACKAGE} ]; then
                sed -i "s/com\.android\.providers\.media/com.mimer.providers.media/g" ${MODULE_DST}/src/${NEW_PACKAGE_PATH}/IMtpService.aidl
                sed -i "s/com\/android/com\/mimer/g" ${MODULE_DST}/Android.mk
	    fi
	fi
    fi

    #This disables the special handling of removable storage
    sed -i "s/if (!Environment.isExternalStorageRemovable()) return;/\/\/if (!Environment.isExternalStorageRemovable()) return;\n            if(true) return;/g" ${MODULE_DST}/src/${NEW_PACKAGE_PATH}/MediaProvider.java
    #sed -i "s/if (Environment.isExternalStorageRemovable()) {/\/\/if (Environment.isExternalStorageRemovable()) {\n        	if(false) {/g" ${MODULE_DST}/src/${NEW_PACKAGE_PATH}/MediaProvider.java


#######################################################
#                END MODULE SPECIFIC CODE             #
#######################################################
else
    echo "ANDROID_BUILD_TOP is not defined."2
fi
echo "=== Done"

