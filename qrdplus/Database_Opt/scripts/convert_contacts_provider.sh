#!/bin/bash
# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

set -e

if [ ${ANDROID_BUILD_TOP} ]; then
	
    #MIMERDIR=${ANDROID_BUILD_TOP}/external/mimer
    MIMERDIR=$( cd ${0%/*}/.. && pwd -P )
    
    MODULE_TYPE=provider
    MODULE_NAME=ContactsProvider
    MODULE_SRC=${ANDROID_BUILD_TOP}/packages/providers/${MODULE_NAME}
    NEW_PACKAGE=com.android.providers.contacts
    ORG_PACKAGE=com.android.providers.contacts
    MIMER_IDENT=contacts
    MIMER_PWD=contactspswd

    . ${MIMERDIR}/scripts/convert_module.inc


#######################################################
#                BEGIN MODULE SPECIFIC CODE           #
#######################################################

    #Fix a non Mimer releated resource problem that might occur while loading nick names with some ROMs
    #echo "Fix ContactsDatabaseHelper"
    sed -i 's/String\[\] names = strings\[clusterId\].split[(]",");/if(strings[clusterId] == null) { continue; }\n                String[] names = strings[clusterId].split(",");/g' ${MODULE_DST}/src/${NEW_PACKAGE_PATH}/ContactsDatabaseHelper.java


FIX_PRIVATE_MODE=0


USE_PATCH=1

    #
    # use patch to add URI for provider_type
    if [ $USE_PATCH = 1 ]; then
        echo "Using patches to modify ContactsProvider"
        if [ "${ANDROID_MAJOR_VERSION}" = "2.3" ]; 
        then
	    echo "Oops, no patch for 2.3 yet"
        fi
    
        if [ "${ANDROID_MAJOR_VERSION}" = "4.0" ]; 
        then
	    patch ${MODULE_DST}/src/${NEW_PACKAGE_PATH}/ContactsProvider2.java ${SCRIPTDIR}/mimer_ContactsProvider2_v4.patch
        elif [ "${ANDROID_MAJOR_VERSION}" = "4.1" -o "${ANDROID_MAJOR_VERSION}" = "4.2" -o "${ANDROID_MAJOR_VERSION}" = "4.3" -o "${ANDROID_MAJOR_VERSION}" = "4.4" ]; then
	    patch ${MODULE_DST}/src/${NEW_PACKAGE_PATH}/ContactsProvider2.java ${SCRIPTDIR}/mimer_ContactsProvider2_v4.patch
        fi

        if [ "${ANDROID_MAJOR_VERSION}" = "2.3" ]; 
        then
	    patch ${MODULE_DST}/src/${NEW_PACKAGE_PATH}/ContactsProvider2.java ${SCRIPTDIR}/mimerdemo_ContactsProvider2.patch 
	    patch ${MODULE_DST}/src/${NEW_PACKAGE_PATH}/ContactsDatabaseHelper.java ${SCRIPTDIR}/mimerdemo_ContactsDatabaseHelper.patch	
        fi
    
        if [ "${ANDROID_MAJOR_VERSION}" = "4.0" -o "${ANDROID_MAJOR_VERSION}" = "4.1" -o "${ANDROID_MAJOR_VERSION}" = "4.2" -o "${ANDROID_MAJOR_VERSION}" = "4.3" -o "${ANDROID_MAJOR_VERSION}" = "4.4" ]; 
        then
	    patch ${MODULE_DST}/src/${NEW_PACKAGE_PATH}/ContactsProvider2.java ${SCRIPTDIR}/mimerdemo_ContactsProvider2_v4.patch 
	    patch ${MODULE_DST}/src/${NEW_PACKAGE_PATH}/ContactsDatabaseHelper.java ${SCRIPTDIR}/mimerdemo_ContactsDatabaseHelper_v4.patch
        fi
    else
        echo "Ignore patching of ContactsProvider"
    fi

#######################################################
#                END MODULE SPECIFIC CODE             #
#######################################################
else
    echo "ANDROID_BUILD_TOP is not defined."2
fi
echo "=== Done"

