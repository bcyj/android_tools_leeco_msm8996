#!/bin/bash
# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

set -e

    if [ "${MODULE_TYPE}" = "app" ]; then
        BASE_DST_POSTFIX=apps
    else
        BASE_DST_POSTFIX=providers
    fi
    SCRIPTDIR=${MIMERDIR}/scripts
    BASE_DST=${MIMERDIR}/${BASE_DST_POSTFIX}
    if [ "${MODULE_SRC}" = "" ]; then
        MODULE_SRC=${ANDROID_BUILD_TOP}/packages/${BASE_DST_POSTFIX}/${MODULE_NAME}
    fi
    MODULE_DST=${BASE_DST}/${MODULE_NAME}  
    MODULE_NAME_PREFIX=
    TEST_MODULE_NAME=${MODULE_NAME}Tests
    VENDOR=

    relpath () {
        #Get the relative path from $1 to $2
        [ $# -ge 1 ] && [ $# -le 2 ] || return 1
        local current="${2:+"$1"}"
        local target="${2:-"$1"}"
        [ "$target" != . ] || local target=/
        target="/${target##/}"
        [ "$current" != . ] || local current=/
        local current="${current:="/"}"
        local current="/${current##/}"
        local appendix="${target##/}"
        local relative=''
        while local appendix="${target#"$current"/}"
            [ "$current" != '/' ] && [ "$appendix" = "$target" ]; do
            if [ "$current" = "$appendix" ]; then
                local relative="${relative:-.}"
                echo "${relative#/}"
                return 0
            fi
            local current="${current%/*}"
            local relative="$relative${relative:+/}.."
        done
        local relative="$relative${relative:+${appendix:+/}}${appendix#/}"
        echo "$relative"
    }
    
    print_usage() {
        echo "Usage $0 [options]"
        echo "	-n <new package name> By default <new_package_name> and <old_package_name> are the same, i.e no rename of package names"
        echo "	-o <old package name>"
        echo "	-s <source directory> the path the source code relative from \$ANDROID_BUILD_TOP, for example \"vendor/xx/providers/ContactsProvider\". If left empty  \$ANDROID_BUILD_TOP/packages/providers/<module-name> will be used. If the providers are in the standard location, just ignore this option". 
        echo "	-t <target directory> relative from \$ANDROID_BUILD_TOP, for example \"vendor/xxx\", or \"packages\" to convert in place"
        echo "	-m <module-name-prefix> By default this is empty. This mean the converted module will have the same name as the original one. This will also mean that the make files for the orignal module is disabled. If a name is give, for example Mimer, the module is renamed, but the new name overides the orignal one. The orignal provider will still be built. But when doing make, the Mimer based provider will be installed."
    }
    
    while getopts “n:o:t:s:m:v:h” OPTION
    do
         case $OPTION in
             h)
                 print_usage
                 exit 1
                 ;;
             o)
                 ORG_PACKAGE=$OPTARG
                 ;;
             n)
                 NEW_PACKAGE=$OPTARG
                 ;;
             m)
                 MODULE_NAME_PREFIX=$OPTARG
                 ;;
             v)
                 VENDOR=$OPTARG
                 ;;
             s)
                 MODULE_SRC=${ANDROID_BUILD_TOP}/$OPTARG
                 ;;
             t)
                 BASE_DST=${ANDROID_BUILD_TOP}/${OPTARG}/${BASE_DST_POSTFIX}
                 MODULE_DST=${BASE_DST}/${MODULE_NAME}
                 ;;
             ?)
                 print_usage
                 exit
                 ;;
         esac
    done
    
    VENDOR=$(echo ${VENDOR} | tr "[:upper:]" "[:lower:]")
    #If we do the conversion in place, make sure we only run it once.
    if [ ${MODULE_SRC} = ${MODULE_DST} ]; then
        if [ -e ${MODULE_SRC}/converted ]; then
            echo "Provider already converted. To convert it again, restore the original source code first"
            exit
        else
            touch ${MODULE_SRC}/converted
        fi
    fi
    
    ORG_PACKAGE_PATH=$(echo "${ORG_PACKAGE}" | tr . //)
    NEW_PACKAGE_PATH=$(echo "${NEW_PACKAGE}" | tr . //)

    #Get the Android version.
    ANDROID_VERSION=$(cat ${ANDROID_BUILD_TOP}/build/core/version_defaults.mk | grep "PLATFORM_VERSION :=" | cut -f2 -d"=" | tr -d ' ')
    ## ANDROID_MAJOR_VERSION will hold the major version number, such as 2.1, 2.2, 2.3, 3.0, 4.0 etc (the minor version number is dropped)
    ANDROID_MAJOR_VERSION=`echo $ANDROID_VERSION | sed "s/\([0-9]\+.[0-9]\+\).*/\1/"`
    echo "Converting ${MODULE_NAME} for Android $ANDROID_VERSION from ${MODULE_SRC} to ${MODULE_DST}"
    
    rm -f /tmp/convert_${MODULE_NAME}_eventlogtags_$$.tmp
    
    if [ ! -d "${BASE_DST}" ]; then
        mkdir "${BASE_DST}"
    elif [ -d "${MODULE_DST}" ]; then
        if [ ${MODULE_SRC} != ${MODULE_DST} ]; then
            echo "Removing old converted ${MODULE_NAME}"
            ## Preserve the eventlogtags file, if it exists in order to avoid unnecessary 
            ## system rebuilds
            if [ -e ${MODULE_DST}/src/${NEW_PACKAGE_PATH}/EventLogTags.logtags ]; then cp -p ${MODULE_DST}/src/${NEW_PACKAGE_PATH}/EventLogTags.logtags /tmp/convert_${MODULE_NAME}_eventlogtags_$$.tmp; fi
            rm -rf ${MODULE_DST}
        fi
    fi

    #Restore the orginal make file before copying/converting
    if [ -e "${MODULE_SRC}/Android.mk.org" ]; then
        cp ${MODULE_SRC}/Android.mk.org ${MODULE_SRC}/Android.mk
    fi
    if [ -e "${MODULE_SRC}/tests/Android.mk.org" ]; then
        cp ${MODULE_SRC}/tests/Android.mk.org ${MODULE_SRC}/tests/Android.mk
    fi 
        
    if [ ${MODULE_SRC} != ${MODULE_DST} ]; then
        #Copy original code
        cp -r ${MODULE_SRC} ${MODULE_DST}
    fi
    
    if [ ! -d "${MODULE_DST}/assets" ]; then
        echo "Creating assets directory"
        mkdir ${MODULE_DST}/assets
    fi

    if [ ${MODULE_SRC} != ${MODULE_DST} ]; then
        if [ -e "${MODULE_DST}/.git" ]; then
            echo "Removing .git directory"
            rm -rf ${MODULE_DST}/.git
        fi
    fi

    if [ "${MODULE_NAME_PREFIX}" = "" ]; then
        if [ ${MODULE_SRC} != ${MODULE_DST} ]; then
            #Disable original code
            mv ${MODULE_SRC}/Android.mk ${MODULE_SRC}/Android.mk.org
            touch ${MODULE_SRC}/Android.mk
    
            #Disable original test code
            if [ -e ${MODULE_SRC}/tests/Android.mk ]; then
                mv ${MODULE_SRC}/tests/Android.mk ${MODULE_SRC}/tests/Android.mk.org
                touch ${MODULE_SRC}/tests/Android.mk
            fi
        fi
    else
        echo "Using alternate module name ${MODULE_NAME_PREFIX}${MODULE_NAME}, will not disable original code"
    fi
    # Convert ${MODULE_NAME}
    perl ${SCRIPTDIR}/Convert.pl -R --ident=${MIMER_IDENT} --password=${MIMER_PWD} --package=${NEW_PACKAGE} ${MODULE_DST}/src/${ORG_PACKAGE_PATH}
    if [ ${NEW_PACKAGE} != ${ORG_PACKAGE} ]; then
       rm -rf ${MODULE_DST}/src/com/android
    fi

    if [ "${MODULE_NAME_PREFIX}" != "" ]; then
        sed -i "s/LOCAL_PACKAGE_NAME := ${MODULE_NAME}/LOCAL_PACKAGE_NAME := ${MODULE_NAME_PREFIX}${MODULE_NAME}/g" ${MODULE_DST}/Android.mk
        perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_PACKAGE_NAME" --after "LOCAL_OVERRIDES_PACKAGES := ${MODULE_NAME}" ${MODULE_DST}/Android.mk
        perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_PACKAGE_NAME" --after "LOCAL_MODULE_STEM := ${MODULE_NAME}" ${MODULE_DST}/Android.mk
    fi
    perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_MODULE_TAGS" --before "WITH_DEXPREOPT := false" ${MODULE_DST}/Android.mk

    perl ${SCRIPTDIR}/addline.pl -b --match="BUILD_PACKAGE" --before "LOCAL_SRC_FILES += \$(call all-java-files-under, mimer_gen)" ${MODULE_DST}/Android.mk

    perl ${SCRIPTDIR}/addline.pl -b --match="BUILD_PACKAGE" --before "LOCAL_JAVA_LIBRARIES += MimerFramework" ${MODULE_DST}/Android.mk

    if [ ${NEW_PACKAGE} != ${ORG_PACKAGE} ]; then
        sed -i "s@${ORG_PACKAGE_PATH}@${NEW_PACKAGE_PATH}@g" ${MODULE_DST}/Android.mk
    fi
    
    echo "Fix the AndroidManifest.xml file"
    perl ${SCRIPTDIR}/addline.pl -b --match="</application>" --before "<uses-library android:name=\"MimerFramework\" android:required=\"true\"/>" ${MODULE_DST}/AndroidManifest.xml
    perl ${SCRIPTDIR}/addline.pl -b --match="</application>" --after "<uses-permission android:name=\"mimer.permission.MIMER_ACCESS\" />" ${MODULE_DST}/AndroidManifest.xml
        
    if [ ${NEW_PACKAGE} != ${ORG_PACKAGE} ]; then
        sed -i "s/${ORG_PACKAGE}/${NEW_PACKAGE}/g" ${MODULE_DST}/AndroidManifest.xml
    fi
    echo "Fix proguard.flags"
    cat ${SCRIPTDIR}/proguard.flags.default >> ${MODULE_DST}/proguard.flags
    sed -i "s/${ORG_PACKAGE}/${NEW_PACKAGE}/g" ${MODULE_DST}/proguard.flags
    
    if grep -Fxq "LOCAL_PROGUARD_FLAG_FILES" ${MODULE_DST}/Android.mk
    then
        echo "Proguard reference found, ignore"
    else
        echo "Proguard reference not found, add it"
        perl ${SCRIPTDIR}/addline.pl -b --match="BUILD_PACKAGE" --before "LOCAL_PROGUARD_FLAG_FILES := proguard.flags" ${MODULE_DST}/Android.mk
    fi
    
    

    # Convert the tests
    if [ -e ${MODULE_DST}/tests/Android.mk ]; then
        perl ${SCRIPTDIR}/Convert.pl -R --ident=${MIMER_IDENT} --password=${MIMER_PWD} --package=${NEW_PACKAGE} ${MODULE_DST}/tests/src/${ORG_PACKAGE_PATH}
        if [ ${NEW_PACKAGE} != ${ORG_PACKAGE} ]; then
            rm -rf ${MODULE_DST}/tests/src/com/android
        fi
        echo "Fix the AndroidManifest.xml file for ${TEST_MODULE_NAME}"
        perl ${SCRIPTDIR}/addline.pl -b --match="</application>" --before "<uses-library android:name=\"MimerFramework\" android:required=\"true\"/>" ${MODULE_DST}/tests/AndroidManifest.xml
        
        if [ "${MODULE_NAME_PREFIX}" != "" ]; then
            sed -i "s/LOCAL_PACKAGE_NAME := ${TEST_MODULE_NAME}/LOCAL_PACKAGE_NAME := ${MODULE_NAME_PREFIX}${TEST_MODULE_NAME}/g" ${MODULE_DST}/tests/Android.mk
            sed -i "s/LOCAL_INSTRUMENTATION_FOR := ${MODULE_NAME}/LOCAL_INSTRUMENTATION_FOR := ${MODULE_NAME_PREFIX}${MODULE_NAME}/g" ${MODULE_DST}/tests/Android.mk
            sed -i "s/LOCAL_PACKAGE_NAME := ${TEST_MODULE_NAME}/LOCAL_PACKAGE_NAME := ${MODULE_NAME_PREFIX}${TEST_MODULE_NAME}/g" ${MODULE_DST}/tests/Android.mk
            perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_PACKAGE_NAME" --after "LOCAL_OVERRIDES_PACKAGES := ${TEST_MODULE_NAME}" ${MODULE_DST}/tests/Android.mk
            perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_PACKAGE_NAME" --after "LOCAL_MODULE_STEM := ${TEST_MODULE_NAME}" ${MODULE_DST}/tests/Android.mk
        fi
    
        # Fix the Android.mk file for ${TEST_MODULE_NAME}
        perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_MODULE_TAGS" --before "WITH_DEXPREOPT := false" ${MODULE_DST}/tests/Android.mk
        perl ${SCRIPTDIR}/addline.pl -b --match="BUILD_PACKAGE" --before "LOCAL_JAVA_LIBRARIES += MimerFramework" ${MODULE_DST}/tests/Android.mk
        perl ${SCRIPTDIR}/addline.pl -b --match="BUILD_PACKAGE" --before "LOCAL_SRC_FILES += \$(call all-java-files-under, mimer_gen)" ${MODULE_DST}/tests/Android.mk
        if [ ${NEW_PACKAGE} != ${ORG_PACKAGE} ]; then
            sed -i "s@${ORG_PACKAGE_PATH}@${NEW_PACKAGE_PATH}@g" ${MODULE_DST}/tests/Android.mk
            sed -i "s/${ORG_PACKAGE}\.tests/${NEW_PACKAGE}.tests/g" ${MODULE_DST}/tests/AndroidManifest.xml
            sed -i "s/${ORG_PACKAGE}/${NEW_PACKAGE}/g" ${MODULE_DST}/tests/AndroidManifest.xml
        fi
    fi
    
    
    if [ "${ANDROID_MAJOR_VERSION}" = "2.3" -o "${ANDROID_MAJOR_VERSION}" = "4.0" -o "${ANDROID_MAJOR_VERSION}" = "4.1" -o "${ANDROID_MAJOR_VERSION}" = "4.2" -o "${ANDROID_MAJOR_VERSION}" = "4.3" -o "${ANDROID_MAJOR_VERSION}" = "4.4" ]; then
        if [ -e /tmp/convert_${MODULE_NAME}_eventlogtags_$$.tmp ]; then
            echo "Fixing EventLogTags for Android ${ANDROID_MAJOR_VERSION}"

            ## A maneuver to avoid updating the target file unless it will be changed. Apparently changing
            ## the file EventLogTags.logtags may otherwise trigger unconfortable system rebuilds.
            cp ${MODULE_SRC}/src/${ORG_PACKAGE_PATH}/EventLogTags.logtags /tmp/convert_${MODULE_NAME}_$$.tmp
            sed -i "s/${ORG_PACKAGE}/${NEW_PACKAGE}/g" /tmp/convert_${MODULE_NAME}_$$.tmp
            if ! diff -N /tmp/convert_${MODULE_NAME}_$$.tmp /tmp/convert_${MODULE_NAME}_eventlogtags_$$.tmp > /dev/null; then
                cp /tmp/convert_${MODULE_NAME}_$$.tmp                 ${MODULE_DST}/src/${NEW_PACKAGE_PATH}/EventLogTags.logtags
            else 
                cp -p /tmp/convert_${MODULE_NAME}_eventlogtags_$$.tmp ${MODULE_DST}/src/${NEW_PACKAGE_PATH}/EventLogTags.logtags
            fi
            rm /tmp/convert_${MODULE_NAME}_$$.tmp
            ##
        fi
    fi
    
    #
    # Enable the Mimer SQL Replacer for the converted code
    #
    ASSETDIR=${MODULE_DST}/assets
    if [ ! -e ${ASSETDIR} ]; then
        mkdir ${ASSETDIR}
    fi
    cp ${MIMERDIR}/sql/replacers/mimersqlreplace_empty.xml ${ASSETDIR}/mimersqlreplace.xml
    
    REPLACERS_21=
    REPLACERS_23=
    REPLACERS_40=
    REPLACERS_41=
 
    REPLACERS_TST_21=
    REPLACERS_TST_23=
    REPLACERS_TST_40=
    REPLACERS_TST_41=
    
    REPLACERS_ABSOLUTE_21=
    REPLACERS_ABSOLUTE_23=
    REPLACERS_ABSOLUTE_40=
    REPLACERS_ABSOLUTE_41=
    
    MODULE_REL_PATH=$(relpath ${MODULE_DST} ${MIMERDIR} )
    MODULE_TST_REL_PATH=$(relpath ${MODULE_DST}/.. ${MIMERDIR} )
    
    if [ "${REPLACER_MODULE_NAME}" = "" ]; then
        REPLACER_MODULE_NAME=${MODULE_NAME}
    fi

    TMP_REP=${MODULE_REL_PATH}/sql/replacers/mimersqlreplace_${REPLACER_MODULE_NAME}_21.xml
    TMP_REP_TST=${MODULE_REL_PATH}/../sql/replacers/mimersqlreplace_${REPLACER_MODULE_NAME}_21.xml
    if [ -e "${MODULE_DST}/${TMP_REP}" ]; then
        REPLACERS_21="\$(LOCAL_PATH)/${TMP_REP} "
        REPLACERS_TST_21="\$(LOCAL_PATH)/${TMP_REP_TST} "
        REPLACERS_ABSOLUTE_21="${MODULE_DST}/${TMP_REP} "
    fi
    if [ "${VENDOR}" != "" ]; then
    	TMP_REP=${MODULE_REL_PATH}/sql/schemas/${VENDOR}/replacers/mimersqlreplace_${REPLACER_MODULE_NAME}_21_${VENDOR}.xml
    	TMP_REP_TST=${MODULE_REL_PATH}/../sql/schemas/${VENDOR}/replacers/mimersqlreplace_${REPLACER_MODULE_NAME}_21_${VENDOR}.xml
        if [ -e "${MODULE_DST}/${TMP_REP}" ]; then
            REPLACERS_21="${REPLACERS_21}\$(LOCAL_PATH)/${TMP_REP} "
            REPLACERS_TST_21="${REPLACERS_TST_21}\$(LOCAL_PATH)/${TMP_REP_TST} "
            REPLACERS_ABSOLUTE_21="${REPLACERS_ABSOLUTE_21}${MODULE_DST}/${TMP_REP} "
        fi    
    fi

    TMP_REP=${MODULE_REL_PATH}/sql/replacers/mimersqlreplace_${REPLACER_MODULE_NAME}_23.xml
    TMP_REP_TST=${MODULE_REL_PATH}/sql/replacers/mimersqlreplace_${REPLACER_MODULE_NAME}_23.xml
    if [ -e "${MODULE_DST}/${TMP_REP}" ]; then
        REPLACERS_23="${REPLACERS_21}\$(LOCAL_PATH)/${TMP_REP} "
        REPLACERS_TST_23="${REPLACERS_TST_21}\$(LOCAL_PATH)/${TMP_REP_TST} "
        REPLACERS_ABSOLUTE_23="${REPLACERS_ABSOLUTE_21}${MODULE_DST}/${TMP_REP} "
    fi
    if [ "${VENDOR}" != "" ]; then
    	TMP_REP=${MODULE_REL_PATH}/sql/schemas/${VENDOR}/replacers/mimersqlreplace_${REPLACER_MODULE_NAME}_23_${VENDOR}.xml
    	TMP_REP_TST=${MODULE_REL_PATH}/../sql/schemas/${VENDOR}/replacers/mimersqlreplace_${REPLACER_MODULE_NAME}_23_${VENDOR}.xml
        if [ -e "${MODULE_DST}/${TMP_REP}" ]; then
           REPLACERS_23="${REPLACERS_23}\$(LOCAL_PATH)/${TMP_REP} "
           REPLACERS_TST_23="${REPLACERS_TST_23}\$(LOCAL_PATH)/${TMP_REP_TST} "
           REPLACERS_ABSOLUTE_23="${REPLACERS_ABSOLUTE_23}${MODULE_DST}/${TMP_REP} "
        fi    
    fi
        
    TMP_REP=${MODULE_REL_PATH}/sql/replacers/mimersqlreplace_${REPLACER_MODULE_NAME}_40.xml
    TMP_REP_TST=${MODULE_REL_PATH}/../sql/replacers/mimersqlreplace_${REPLACER_MODULE_NAME}_40.xml
    if [ -e "${MODULE_DST}/${TMP_REP}" ]; then
        REPLACERS_40="${REPLACERS_23}\$(LOCAL_PATH)/${TMP_REP} "
        REPLACERS_TST_40="${REPLACERS_TST_23}\$(LOCAL_PATH)/${TMP_REP_TST} "
        REPLACERS_ABSOLUTE_40="${REPLACERS_ABSOLUTE_23}${MODULE_DST}/${TMP_REP} "
    fi
    if [ "${VENDOR}" != "" ]; then
    	TMP_REP=${MODULE_REL_PATH}/sql/schemas/${VENDOR}/replacers/mimersqlreplace_${REPLACER_MODULE_NAME}_40_${VENDOR}.xml
    	TMP_REP_TST=${MODULE_REL_PATH}/../sql/schemas/${VENDOR}/replacers/mimersqlreplace_${REPLACER_MODULE_NAME}_40_${VENDOR}.xml
	    if [ -e "${MODULE_DST}/${MODULE_DST}/${TMP_REP}" ]; then
	       REPLACERS_40="${REPLACERS_40}\$(LOCAL_PATH)/${TMP_REP} "
	       REPLACERS_TST_40="${REPLACERS_TST_40}\$(LOCAL_PATH)/${TMP_REP_TST} "
	       REPLACERS_ABSOLUTE_40="${REPLACERS_ABSOLUTE_40}${MODULE_DST}/${TMP_REP} "
	    fi    
    fi
    
    TMP_REP=${MODULE_REL_PATH}/sql/replacers/mimersqlreplace_${REPLACER_MODULE_NAME}_41.xml
    TMP_REP_TST=${MODULE_REL_PATH}/../sql/replacers/mimersqlreplace_${REPLACER_MODULE_NAME}_41.xml
    if [ -e "${MODULE_DST}/${TMP_REP}" ]; then
        REPLACERS_41="${REPLACERS_40}\$(LOCAL_PATH)/${TMP_REP} "
        REPLACERS_TST_41="${REPLACERS_TST_40}\$(LOCAL_PATH)/${TMP_REP_TST} "
        REPLACERS_ABSOLUTE_41="${REPLACERS_ABSOLUTE_40}${MODULE_DST}/${TMP_REP} "
    fi
    if [ "${VENDOR}" != "" ]; then
    	TMP_REP=${MODULE_REL_PATH}/sql/schemas/${VENDOR}/replacers/mimersqlreplace_${REPLACER_MODULE_NAME}_41_${VENDOR}.xml
    	TMP_REP_TST=${MODULE_REL_PATH}/../sql/schemas/${VENDOR}/replacers/mimersqlreplace_${REPLACER_MODULE_NAME}_41_${VENDOR}.xml
	    if [ -e "${MODULE_DST}/${TMP_REP}" ]; then
	       REPLACERS_41="${REPLACERS_41}\$(LOCAL_PATH)/${TMP_REP} "
	       REPLACERS_TST_41="${REPLACERS_TST_41}\$(LOCAL_PATH)/${TMP_REP_TST} "
	       REPLACERS_ABSOLUTE_41="${REPLACERS_ABSOLUTE_41}${MODULE_DST}/${TMP_REP} "
	    fi    
    fi

#    echo "Android 2.1 replacer list: $REPLACERS_21"
#    echo "Android 2.1 ABSOLUTE replacer list: $REPLACERS_ABSOLUTE_21"
#    echo "Android 2.1 Test replacer list: $REPLACERS_TST_21"
#    echo "Android 2.3 replacer list: $REPLACERS_23"
#    echo "Android 2.3 ABSOLUTE replacer list: $REPLACERS_ABSOLUTE_23"
#    echo "Android 2.3 Test replacer list: $REPLACERS_TST_23"
#    echo "Android ICS replacer list: $REPLACERS_40"
#    echo "Android ICS ABSOLUTE replacer list: $REPLACERS_ABSOLUTE_40"
#    echo "Android ICS Test replacer list: $REPLACERS_TST_40"
#    echo "Android JB replacer list: $REPLACERS_41"
#    echo "Android JB ABSOLUTE replacer list: $REPLACERS_ABSOLUTE_41"
#    echo "Android JB Test replacer list: $REPLACERS_TST_41"

    if [ "${ANDROID_MAJOR_VERSION}" = "2.1" ]; then
        perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_MODULE_TAGS" --after "IGNORE:= \$(shell perl \$(LOCAL_PATH)/${MODULE_REL_PATH}/sql/join_mimersqlreplace.pl --android_version=${ANDROID_VERSION} --package=${NEW_PACKAGE} --javaoutputfile=\$(LOCAL_PATH)/mimer_gen/${NEW_PACKAGE_PATH}/MimerSQLReplace.java ${REPLACERS_21})" ${MODULE_DST}/Android.mk
    elif [ "${ANDROID_MAJOR_VERSION}" = "2.3" ]; then
        perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_MODULE_TAGS" --after "IGNORE:= \$(shell perl \$(LOCAL_PATH)/${MODULE_REL_PATH}/sql/join_mimersqlreplace.pl --android_version=${ANDROID_VERSION} --package=${NEW_PACKAGE} --javaoutputfile=\$(LOCAL_PATH)/mimer_gen/${NEW_PACKAGE_PATH}/MimerSQLReplace.java ${REPLACERS_23})" ${MODULE_DST}/Android.mk
    elif [ "${ANDROID_MAJOR_VERSION}" = "4.0" ]; then
        perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_MODULE_TAGS" --after "IGNORE:= \$(shell perl \$(LOCAL_PATH)/${MODULE_REL_PATH}/sql/join_mimersqlreplace.pl --android_version=${ANDROID_VERSION} --package=${NEW_PACKAGE} --javaoutputfile=\$(LOCAL_PATH)/mimer_gen/${NEW_PACKAGE_PATH}/MimerSQLReplace.java ${REPLACERS_40})" ${MODULE_DST}/Android.mk
    
        echo "Generating MimerSQLReplace.java"
        perl ${MIMERDIR}/sql/join_mimersqlreplace.pl --android_version=${ANDROID_VERSION} --package=${NEW_PACKAGE} --javaoutputfile=${MODULE_DST}/mimer_gen/${NEW_PACKAGE_PATH}/MimerSQLReplace.java ${REPLACERS_ABSOLUTE_40} > /dev/null
    
    elif [ "${ANDROID_MAJOR_VERSION}" = "4.1" -o "${ANDROID_MAJOR_VERSION}" = "4.2" -o "${ANDROID_MAJOR_VERSION}" = "4.3" -o "${ANDROID_MAJOR_VERSION}" = "4.4" ]; then
        perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_MODULE_TAGS" --after "IGNORE:= \$(shell perl \$(LOCAL_PATH)/${MODULE_REL_PATH}/sql/join_mimersqlreplace.pl --android_version=${ANDROID_VERSION} --package=${NEW_PACKAGE} --javaoutputfile=\$(LOCAL_PATH)/mimer_gen/${NEW_PACKAGE_PATH}/MimerSQLReplace.java ${REPLACERS_41})" ${MODULE_DST}/Android.mk
    
        echo "Generating MimerSQLReplace.java"
        perl ${MIMERDIR}/sql/join_mimersqlreplace.pl --android_version=${ANDROID_VERSION} --package=${NEW_PACKAGE} --javaoutputfile=${MODULE_DST}/mimer_gen/${NEW_PACKAGE_PATH}/MimerSQLReplace.java ${REPLACERS_ABSOLUTE_41} > /dev/null

    else
        echo "ERROR! Unsupported Android version"
    fi

    if [ -e ${MODULE_DST}/tests/Android.mk ]; then
        #
        # Enable the Mimer SQL Replacer for the tests
        #
        ASSETDIR=${MODULE_DST}/tests/assets
        if [ ! -e ${ASSETDIR} ]; then
            mkdir ${ASSETDIR}
        fi
        cp ${MIMERDIR}/sql/replacers/mimersqlreplace_empty.xml ${ASSETDIR}/mimersqlreplace.xml
        
        if [ "${ANDROID_MAJOR_VERSION}" = "2.1" ]; then
            perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_MODULE_TAGS" --after "IGNORE:= \$(shell perl \$(LOCAL_PATH)/${MODULE_TST_REL_PATH}/../sql/join_mimersqlreplace.pl --package=${NEW_PACKAGE} --javaoutputfile=\$(LOCAL_PATH)/mimer_gen/${NEW_PACKAGE_PATH}/MimerSQLReplace.java ${REPLACERS_TST_21})" ${MODULE_DST}/tests/Android.mk
        elif [ "${ANDROID_MAJOR_VERSION}" = "2.3" ]; then
            perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_MODULE_TAGS" --after "IGNORE:= \$(shell perl \$(LOCAL_PATH)/${MODULE_TST_REL_PATH}/../sql/join_mimersqlreplace.pl --package=${NEW_PACKAGE} --javaoutputfile=\$(LOCAL_PATH)/mimer_gen/${NEW_PACKAGE_PATH}/MimerSQLReplace.java ${REPLACERS_TST_23})" ${MODULE_DST}/tests/Android.mk
        elif [ "${ANDROID_MAJOR_VERSION}" = "4.0" ]; then
            perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_MODULE_TAGS" --after "IGNORE:= \$(shell perl \$(LOCAL_PATH)/${MODULE_TST_REL_PATH}/../sql/join_mimersqlreplace.pl --package=${NEW_PACKAGE} --javaoutputfile=\$(LOCAL_PATH)/mimer_gen/${NEW_PACKAGE_PATH}/MimerSQLReplace.java ${REPLACERS_TST_40})" ${MODULE_DST}/tests/Android.mk
        
            echo "Generating MimerSQLReplace.java for test"
            perl ${MIMERDIR}/sql/join_mimersqlreplace.pl --android_version=${ANDROID_VERSION} --package=${NEW_PACKAGE} --javaoutputfile=${MODULE_DST}/tests/mimer_gen/${NEW_PACKAGE_PATH}/MimerSQLReplace.java ${REPLACERS_ABSOLUTE_40} > /dev/null
        
        elif [ "${ANDROID_MAJOR_VERSION}" = "4.1" -o "${ANDROID_MAJOR_VERSION}" = "4.2" -o "${ANDROID_MAJOR_VERSION}" = "4.3" -o "${ANDROID_MAJOR_VERSION}" = "4.4" ]; then
            perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_MODULE_TAGS" --after "IGNORE:= \$(shell perl \$(LOCAL_PATH)/${MODULE_TST_REL_PATH}/sql/join_mimersqlreplace.pl --package=${NEW_PACKAGE} --javaoutputfile=\$(LOCAL_PATH)/mimer_gen/${NEW_PACKAGE_PATH}/MimerSQLReplace.java ${REPLACERS_TST_41})" ${MODULE_DST}/tests/Android.mk
            echo "Generating MimerSQLReplace.java for test"
            perl ${MIMERDIR}/sql/join_mimersqlreplace.pl --android_version=${ANDROID_VERSION} --package=${NEW_PACKAGE} --javaoutputfile=${MODULE_DST}/tests/mimer_gen/${NEW_PACKAGE_PATH}/MimerSQLReplace.java ${REPLACERS_ABSOLUTE_41} > /dev/null

        else
            echo "ERROR! Unsupported Android version"
        fi
        
    fi
       
