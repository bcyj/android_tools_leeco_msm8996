# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
MIMER_DIST_PATH:= $(call my-dir)
MIMER_OUT:= out/tmp_src/mimer
MIMER_OUT_PATH:= $(ANDROID_BUILD_TOP)/$(MIMER_OUT)
MIMER_LOG_PATH:= $(MIMER_OUT_PATH)/logs
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

ALT_NAME_ARG := "-m Mimer"
ENABLE_MIMER := FALSE
ifeq ($(ENABLE_MIMER),TRUE)

IGNORE:= $(shell mkdir -p $(MIMER_OUT_PATH)/providers))
IGNORE:= $(shell mkdir -p $(MIMER_OUT_PATH)/apps))
IGNORE:= $(shell mkdir -p $(MIMER_LOG_PATH)))

include $(MIMER_DIST_PATH)/native/mimcomm/Android.mk
include $(MIMER_DIST_PATH)/native/mimmicroapi/Android.mk
include $(MIMER_DIST_PATH)/native/SQLiteTranslator/Android.mk
include $(MIMER_DIST_PATH)/framework/Android.mk
include $(MIMER_DIST_PATH)/mimerservice/Android.mk
include $(MIMER_DIST_PATH)/db/Android.mk

# ----------- Contacts ------------------------------------
MIMER_CONTACTS:= $(MIMER_OUT_PATH)/providers/ContactsProvider
    CONTACTS_SOURCE:= packages/providers/ContactsProvider
$(MIMER_CONTACTS)/Android.mk: $(shell find $(ANDROID_BUILD_TOP)/$(CONTACTS_SOURCE)/src)
	echo "Converting ContactsProvider and log to $(MIMER_LOG_PATH)/convert_contacts_provider.log"
	@sh $(MIMER_DIST_PATH)/scripts/convert_contacts_provider.sh $(ALT_NAME_ARG) -s $(CONTACTS_SOURCE) -t $(MIMER_OUT) 2>&1 > $(MIMER_LOG_PATH)/convert_contacts_provider.log
	
# ----------- Media ---------------------------------------
MIMER_MEDIA:= $(MIMER_OUT_PATH)/providers/MediaProvider
    MEDIA_SOURCE:= packages/providers/MediaProvider
$(MIMER_MEDIA)/Android.mk: $(shell find $(ANDROID_BUILD_TOP)/$(MEDIA_SOURCE)/src)
	echo "Converting MediaProvider and log to $(MIMER_LOG_PATH)/convert_media_provider.log"
	@sh $(MIMER_DIST_PATH)/scripts/convert_media_provider.sh $(ALT_NAME_ARG) -s $(MEDIA_SOURCE) -t $(MIMER_OUT) 2>&1 > $(MIMER_LOG_PATH)/convert_media_provider.log

# ----------- Calendar ------------------------------------
MIMER_CALENDAR:= $(MIMER_OUT_PATH)/providers/CalendarProvider
    CALENDAR_SOURCE:= packages/providers/CalendarProvider

$(MIMER_CALENDAR)/Android.mk: $(shell find $(ANDROID_BUILD_TOP)/$(CALENDAR_SOURCE)/src)
	echo "Converting CalendarProvider and log to $(MIMER_LOG_PATH)/convert_calendar_provider.log"
	@sh $(MIMER_DIST_PATH)/scripts/convert_calendar_provider.sh $(ALT_NAME_ARG) -s $(CALENDAR_SOURCE) -t $(MIMER_OUT) 2>&1 > $(MIMER_LOG_PATH)/convert_calendar_provider.log

# ----------- Telephony ------------------------------------
MIMER_TELEPHONY:= $(MIMER_OUT_PATH)/providers/TelephonyProvider
    TELEPHONY_SOURCE:= packages/providers/TelephonyProvider
$(MIMER_TELEPHONY)/Android.mk: $(shell find $(ANDROID_BUILD_TOP)/$(TELEPHONY_SOURCE)/src)
	echo "Converting TelephonyProvider and log to $(MIMER_LOG_PATH)/convert_telephony_provider.log"
	@sh $(MIMER_DIST_PATH)/scripts/convert_telephony_provider.sh $(ALT_NAME_ARG) -s $(TELEPHONY_SOURCE) -t $(MIMER_OUT) 2>&1 > $(MIMER_LOG_PATH)/convert_telephony_provider.log

# ----------- Email ---------------------------------------
MIMER_EMAIL:= $(MIMER_OUT_PATH)/apps/Email
    EMAIL_SOURCE:= packages/apps/Email
$(MIMER_EMAIL)/Android.mk: $(shell find $(ANDROID_BUILD_TOP)/$(EMAIL_SOURCE)/src)
	echo "Converting Email app and log to $(MIMER_LOG_PATH)/convert_email_provider.log"
	@sh $(MIMER_DIST_PATH)/scripts/convert_email_provider.sh $(ALT_NAME_ARG) -s $(EMAIL_SOURCE) -t $(MIMER_OUT) 2>&1 > $(MIMER_LOG_PATH)/convert_email.log



#___include $(call all-sudbir-makefiles-under,$(MIMER_OUT_PATH))
include $(MIMER_CONTACTS)/Android.mk
include $(MIMER_MEDIA)/Android.mk
include $(MIMER_CALENDAR)/Android.mk
include $(MIMER_TELEPHONY)/Android.mk
include $(MIMER_EMAIL)/Android.mk

endif






