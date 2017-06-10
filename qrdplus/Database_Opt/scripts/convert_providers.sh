#!/bin/sh
# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

if [ ${ANDROID_BUILD_TOP} ]; then

    MIMERDIR=${ANDROID_BUILD_TOP}/external/mimer
    SCRIPTDIR=${MIMERDIR}/scripts
    INC_MEDIA="YES"
    MEDIA_SOURCE=packages/providers/MediaProvider
    INC_TELEPHONY="YES"
    TELEPHONY_SOURCE=packages/providers/TelephonyProvider
    INC_CALENDAR="YES"
    CALENDAR_SOURCE=packages/providers/CalendarProvider
    INC_CONTACTS="YES"
    USE_DEMO_PROVIDER="YES"
    CONTACTS_SOURCE=packages/providers/ContactsProvider
    INC_EMAIL="YES"
    EMAIL_SOURCE=packages/apps/Email
CONV_ARGS=
MODULE_NAME_ARG="-m Mimer "
TARGET_ARG="-t out/tmp_src/mimer "
    print_usage(){
        echo "Usage convert_providers.sh [options]"
        echo "	-t target directory relative from \$ANDROID_BUILD_TOP, for example vendor/xxx/providers"
        echo "	-v vendor name"
        echo "	-m <module-name-prefix> By default this is empty. This mean the converted module will have the same name as the original one. This will also mean that the make files for the orignal module is disabled. If a name is give, for example Mimer, the module is renamed, but the new name overides the orignal one. The orignal provider will still be built. But when doing make, the Mimer based provider will be installed."

    }

    while getopts “h:t:v:m:” OPTION
    do
         case $OPTION in
             h)
                 print_usage
                 exit 1
                 ;;
             t)
                TARGET_ARG="-t ${OPTARG} "
                ;;
             m)
                MODULE_NAME_ARG="-m ${OPTARG} "
                ;;
             v)
                VEND_ARG="-v ${OPTARG} "
                ;;
             ?)
                 print_usage
                 exit
                 ;;
         esac
    done

CONV_ARGS=${TARGET_ARG}${MODULE_NAME_ARG}${VEND_ARG}


	if [ "${INC_MEDIA}" = "YES" ]; then
		echo "Converting Media"
		sh ${SCRIPTDIR}/convert_media_provider.sh -s ${MEDIA_SOURCE} -o com.android.providers.media -n com.android.providers.media ${CONV_ARGS} || return 1
	fi

	if [ "${INC_TELEPHONY}" = "YES" ]; then
		echo "Converting Telephony"
		sh ${SCRIPTDIR}/convert_telephony_provider.sh -s ${TELEPHONY_SOURCE} -o com.android.providers.telephony -n com.android.providers.telephony ${CONV_ARGS} || return 1
	fi
	if [ "${INC_CALENDAR}" = "YES" ]; then
		echo "Converting Calendar"
		sh ${SCRIPTDIR}/convert_calendar_provider.sh -s ${CALENDAR_SOURCE} -o com.android.providers.calendar -n com.android.providers.calendar ${CONV_ARGS} || return 1
	fi
	if [ "${INC_CONTACTS}" = "YES" ]; then
		echo "Converting Contacts"
		sh ${SCRIPTDIR}/convert_contacts_provider.sh -s ${CONTACTS_SOURCE} -o com.android.providers.contacts -n com.android.providers.contacts ${CONV_ARGS} || return 1

	fi
	if [ "${INC_EMAIL}" = "YES" ]; then
		echo "Converting Email"
		sh ${SCRIPTDIR}/convert_email_provider.sh -s ${EMAIL_SOURCE} ${CONV_ARGS} || return 1
	fi


else
    echo "ANDROID_BUILD_TOP is not defined."
fi
echo "=== Done"
