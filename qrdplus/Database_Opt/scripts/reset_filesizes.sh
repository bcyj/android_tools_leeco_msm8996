#!/bin/sh



# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

if [ $# -ne 1 ]
then
	DB=mimerserver
else
	DB=$1
fi

echo "Using database $DB"

bsql -uSYSADM -pSYSADM --query="alter databank transdb drop filesize" $DB
bsql -uSYSADM -pSYSADM --query="alter databank sqldb drop filesize" $DB

bsql -ucontacts -pcontactspswd --query="read '${ANDROID_BUILD_TOP}/external/mimer/sql/stat_data/reset_filesizes_contacts.sql'" $DB

bsql -utelephony -ptelephonypswd --query="read '${ANDROID_BUILD_TOP}/external/mimer/sql/stat_data/reset_filesizes_telephony.sql'" $DB


bsql -umedia -pmediapswd --query="read '${ANDROID_BUILD_TOP}/external/mimer/sql/stat_data/reset_filesizes_media.sql'" $DB

bsql -ucalendar -pcalendarpswd --query="read '${ANDROID_BUILD_TOP}/external/mimer/sql/stat_data/reset_filesizes_calendar.sql'" $DB

sql -uemail -pemailpswd --query="read '${ANDROID_BUILD_TOP}/external/mimer/sql/stat_data/reset_filesizes_email.sql'" $DB












b


