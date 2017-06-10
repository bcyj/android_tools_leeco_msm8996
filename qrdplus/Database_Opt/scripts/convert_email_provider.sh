#!/bin/bash
# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

set -e

if [ ${ANDROID_BUILD_TOP} ]; then
	
    #MIMERDIR=${ANDROID_BUILD_TOP}/external/mimer
    MIMERDIR=$( cd ${0%/*}/.. && pwd -P )

    MODULE_TYPE=app
    MODULE_NAME=Email
    MODULE_SRC=${ANDROID_BUILD_TOP}/packages/apps/${MODULE_NAME}
    #A fix for now, to avoid renaming all replacer files...
    REPLACER_MODULE_NAME=EmailProvider
    NEW_PACKAGE=com.android.email.provider
    ORG_PACKAGE=com.android.email.provider
    MIMER_IDENT=email
    MIMER_PWD=emailpswd

    . ${MIMERDIR}/scripts/convert_module.inc


#######################################################
#                BEGIN MODULE SPECIFIC CODE           #
#######################################################


    if [ "${MODULE_NAME_PREFIX}" != "" ]; then
        echo "Fix alternative module name for emailcommon"
        sed -i "s/LOCAL_MODULE := com\.android\.emailcommon/LOCAL_MODULE := com.android.emailcommon2/g" ${MODULE_DST}/emailcommon/Android.mk
        perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_MODULE" --after "LOCAL_OVERRIDES_PACKAGES := com.android.emailcommon" ${MODULE_DST}/emailcommon/Android.mk
        perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_MODULE" --after "LOCAL_MODULE_STEM := com.android.emailcommon" ${MODULE_DST}/emailcommon/Android.mk
        if [ "$ANDROID_MAJOR_VERSION" \> "4.3" ]; then
        	echo "Fix alternative module name for emailsync"
        	sed -i "s/LOCAL_MODULE := com\.android\.emailsync/LOCAL_MODULE := com.android.emailsync2/g" ${MODULE_DST}/emailsync/Android.mk
        	perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_MODULE" --after "LOCAL_OVERRIDES_PACKAGES := com.android.emailsync" ${MODULE_DST}/emailsync/Android.mk
        	perl ${SCRIPTDIR}/addline.pl -b --match="LOCAL_MODULE" --after "LOCAL_MODULE_STEM := com.android.emailsync" ${MODULE_DST}/emailsync/Android.mk
        fi        
    fi

    androot=$(relpath ${MODULE_DST} ${ANDROID_BUILD_TOP} )
    if [ ${MODULE_SRC} != ${MODULE_DST} ]; then
	sed -i "s@\.\./\.\./\.\./frameworks/ex/chips/res@${androot}/frameworks/ex/chips/res@g" ${MODULE_DST}/Android.mk
	sed -i "s@\.\./\.\./\.\./frameworks/opt/mailcommon/res@${androot}/frameworks/opt/mailcommon/res@g" ${MODULE_DST}/Android.mk
	if [ "$ANDROID_MAJOR_VERSION" \> "4.3" ]; then
	    sed -i "s@\.\./\.\./\.\./frameworks/opt/photoviewer/res@${androot}/frameworks/opt/photoviewer/res@g" ${MODULE_DST}/Android.mk
	    sed -i "s@\.\./\.\./\.\./frameworks/opt/photoviewer/activity/res@${androot}/frameworks/opt/photoviewer/activity/res@g" ${MODULE_DST}/Android.mk
	    sed -i "s@\.\./\.\./\.\./frameworks/support/v7/gridlayout/res@${androot}/frameworks/support/v7/gridlayout/res@g" ${MODULE_DST}/Android.mk
	    sed -i "s@\.\./UnifiedEmail@${androot}/packages/apps/UnifiedEmail@g" ${MODULE_DST}/Android.mk
	    sed -i "s@\.\./\.\./UnifiedEmail@../${androot}/packages/apps/UnifiedEmail@g" ${MODULE_DST}/emailcommon/Android.mk
	fi
    fi
    
    if [ ${NEW_PACKAGE} != ${ORG_PACKAGE} ]; then
	    sed -i "s/\.provider\.AttachmentProvider/com.mimer.email.provider.AttachmentProvider/g" ${MODULE_DST}/AndroidManifest.xml
	    sed -i "s/\.provider\.EmailProvider/com.mimer.email.provider.EmailProvider/g" ${MODULE_DST}/AndroidManifest.xml	
	    sed -i "s/\.provider\.WidgetProvider/com.mimer.email.provider.WidgetProvider/g" ${MODULE_DST}/AndroidManifest.xml	
    fi
   # echo "Fix application files for provider references"

#used this funtion instead of hardcoding filepath. It recursivily search for files having com.android.email.provider and replace with com.mimer.email.provider.
if [ ${NEW_PACKAGE} != ${ORG_PACKAGE} ]; then
files()
{

	cd $1

	for file in *
	
	do

		if [ -d "$file" ] 
		then
                        cd $file
			files .
                        cd ..
		else
			sed -i "s/ ${ORG_PACKAGE}/${NEW_PACKAGE}/g" "$file"
		fi

	done
	
}

files ${MODULE_DST}/src/com/android/email/
fi


#######################################################
#                END MODULE SPECIFIC CODE             #
#######################################################
else
    echo "ANDROID_BUILD_TOP is not defined."2
fi
echo "=== Done"


