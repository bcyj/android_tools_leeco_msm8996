set echo off;
-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

set message off;

---------------------
---------------------

@
begin
   declare attendee_cnt integer;
   declare n_event_id int;
   declare n_attendeeName nvarchar(100);
   declare n_attendeeEmail nvarchar(100);
   declare n_attendeeStatus int;
   declare n_attendeeRelationship int;
   declare n_attendeeType int;
   declare n_attendeeIdentity nvarchar(100);
   declare n_attendeeIdNamespace nvarchar(100);

   set attendee_cnt = 0;
   
   WHILE attendee_cnt <= 200 DO
    set n_event_id = mod(IRAND(), 100);
    set n_attendeeName = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 3)+1);
    set n_attendeeEmail = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 3)+1)||'@mimer.se';
    
    if mod(attendee_cnt, 5) = 0 then 
    	set n_attendeeStatus = 1;
    	set n_attendeeRelationship = 1;
    	set n_attendeeType = 1;
    	set n_attendeeIdentity = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 3)+1);
    	set n_attendeeIdNamespace = REPEAT(ASCII_CHAR(MOD(IRAND(), 4) + 65)||ASCII_CHAR(MOD(IRAND(), 4) + 65)||ASCII_CHAR(MOD(IRAND(), 4) + 65), MOD(IRAND(), 3)+1);
    else 
    	set n_attendeeStatus = 0;
    	set n_attendeeRelationship = 2;
    	set n_attendeeType = 0;
    	set n_attendeeIdentity = null;
    	set n_attendeeIdNamespace = null;
    end if;

    insert into Attendees ("_id", event_id, attendeeName, attendeeEmail, attendeeStatus, attendeeRelationship, attendeeType, attendeeIdentity, attendeeIdNamespace) values (attendee_cnt+1, n_event_id, n_attendeeName, n_attendeeEmail, n_attendeeStatus, n_attendeeRelationship, n_attendeeType, n_attendeeIdentity, n_attendeeIdNamespace);


    set attendee_cnt = attendee_cnt + 1;
    END WHILE;
end 
@

---------------------
---------------------


select next_value of events_seq from system.onerow;


---------------------
---------------------

@
begin
   declare events_cnt integer;
   declare n_sync_id varchar(100);
   declare n_dirty int;
   declare n_lastSynced int;
   declare n_calendar_id bigint;
   declare n_title nvarchar(100);
   declare n_eventLocation nvarchar(100);
   declare n_description nvarchar(100);
   declare n_eventColor int;
   declare n_eventColor_index varchar(100);
   declare n_eventStatus int;
   declare n_selfAttendeeStatus int;
   declare n_dtstart bigint;
   declare n_dtend bigint;
   declare n_eventTimezone nvarchar(128);
   declare n_duration nvarchar(128);
   declare n_allDay int;
   declare n_accessLevel int;
   declare n_availability int;
   declare n_hasAlarm int;
   declare n_hasExtendedProperties int;
   declare n_rrule nvarchar(128);
   declare n_rdate nvarchar(128);
   declare n_exrule nvarchar(128);
   declare n_exdate nvarchar(128);
   declare n_original_id int;
   declare n_original_sync_id  varchar(512);
   declare n_originalInstanceTime bigint;
   declare n_originalAllDay int;
   declare n_lastDate bigint;
   declare n_hasAttendeeData int;
   declare n_guestsCanModify int;
   declare n_guestsCanInviteOthers int;
   declare n_guestsCanSeeGuests int;
   declare n_organizer nvarchar(128);
   declare n_deleted int;
   declare n_eventEndTimezone nvarchar(128);
   declare n_sync_data1  varchar(512);
   declare n_sync_data2  varchar(512);
   declare n_sync_data3  varchar(512);
   declare n_sync_data4  varchar(512);
   declare n_sync_data5  varchar(512);
   declare n_sync_data6  varchar(512);
   declare n_sync_data7  varchar(512);
   declare n_sync_data8  varchar(512);
   declare n_sync_data9  varchar(512);
   declare n_sync_data10  varchar(512);

   set events_cnt = 0;
   
   WHILE events_cnt <= 500 DO
    
    set n_sync_id = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 3)+1);
    set n_calendar_id = mod(IRAND(), 10);
    set n_title = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 3)+1);
    set n_eventLocation = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 3)+1);
    set n_description = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 3)+1);
    set n_eventColor = mod(IRAND(), 10);
    set n_eventColor_index = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 3)+1);
    set n_dtstart = mod(IRAND(), 10000)*1000;
    set n_dtend = mod(IRAND(), 10000)*1000;
    set n_accessLevel = mod(IRAND(), 4);
    set n_availability = mod(IRAND(), 2);
    set n_hasExtendedProperties = 0;
    set n_rrule = null;
    set n_rdate = null;
    set n_exrule = null;
    set n_exdate = null;
    
    if mod(events_cnt, 5) = 0 then 
        set n_original_id = mod(IRAND(), 100);
        set n_original_sync_id = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 3)+1);
        set n_originalInstanceTime = mod(IRAND(), 10000)*1000;
        set n_originalAllDay = mod(IRAND(), 2);
    else
        set n_original_id = null;
        set n_original_sync_id = null;
        set n_originalInstanceTime = null;
        set n_originalAllDay = null;
    end if;
    set n_lastDate = mod(IRAND(), 10000)*1000;
    set n_hasAttendeeData = mod(IRAND(), 2);
    set n_guestsCanModify = mod(IRAND(), 2);
    set n_guestsCanInviteOthers = mod(IRAND(), 2);
    set n_guestsCanSeeGuests = mod(IRAND(), 2);
    set n_organizer = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 3)+1);
    set n_sync_data1 = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 10)+1);
    set n_sync_data2 = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 10)+1);
    set n_sync_data3 = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 10)+1);
    set n_sync_data4 = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 10)+1);
    set n_sync_data5 = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 10)+1);
    set n_sync_data6 = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 10)+1);
    set n_sync_data7 = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 2)+1);
    set n_sync_data8 = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 10)+1);
    set n_sync_data9 = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 10)+1);
    set n_sync_data10 = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 10)+1);
    
    if mod(events_cnt, 10) = 0 then 
    	set n_dirty = 1;
    	set n_lastSynced = 1;
    	set n_eventStatus = 0;
    	set n_selfAttendeeStatus = 1;
    	set n_eventTimezone = null;
    	set n_duration = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 3)+1);
    	set n_allDay = 1;
    	set n_hasAlarm = 0;
    	set n_deleted = 1;
    	set n_eventEndTimezone = null;
    else 
    	set n_dirty = 0;
    	set n_lastSynced = 0;
    	set n_eventStatus = 1;
    	set n_selfAttendeeStatus = 0;
    	set n_eventTimezone = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 3)+1);
    	set n_duration = null;
    	set n_allDay = 0;
    	set n_hasAlarm = 1;
    	set n_deleted = 0;
    	set n_eventEndTimezone = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 3)+1);
    end if;

    insert into Events ("_id", "_sync_id", dirty, lastSynced, calendar_id, title, eventLocation, description, eventColor, eventColor_index, eventStatus, selfAttendeeStatus, dtstart, dtend, eventTimezone, duration, allDay, accessLevel, availability, hasAlarm, hasExtendedProperties, rrule, rdate, exrule, exdate, original_id, original_sync_id, originalInstanceTime, originalAllDay, lastDate, hasAttendeeData, guestsCanModify, guestsCanInviteOthers, guestsCanSeeGuests, organizer, deleted, eventEndTimezone, sync_data1, sync_data2, sync_data3, sync_data4, sync_data5, sync_data6, sync_data7, sync_data8, sync_data9, sync_data10) values (events_cnt+1, n_sync_id, n_dirty, n_lastSynced, n_calendar_id, n_title, n_eventLocation, n_description, n_eventColor, n_eventColor_index, n_eventStatus, n_selfAttendeeStatus, n_dtstart, n_dtend, n_eventTimezone, n_duration, n_allDay, n_accessLevel, n_availability, n_hasAlarm, n_hasExtendedProperties, n_rrule, n_rdate, n_exrule, n_exdate, n_original_id, n_original_sync_id, n_originalInstanceTime, n_originalAllDay, n_lastDate, n_hasAttendeeData, n_guestsCanModify, n_guestsCanInviteOthers, n_guestsCanSeeGuests, n_organizer, n_deleted, n_eventEndTimezone, n_sync_data1, n_sync_data2, n_sync_data3, n_sync_data4, n_sync_data5, n_sync_data6, n_sync_data7, n_sync_data8, n_sync_data9, n_sync_data10);


    set events_cnt = events_cnt + 1;
    END WHILE;
end 
@
---------------------
---------------------

@
begin
   declare event_raw_time_cnt integer;
   declare n_event_id int;
   declare n_dtstart2445 nvarchar(100);
   declare n_dtend2445 nvarchar(100);
   declare n_originalInstanceTime2445 nvarchar(100);
   declare n_lastDate2445 nvarchar(100);

   set event_raw_time_cnt = 0;
   
   WHILE event_raw_time_cnt <= 200 DO
    set n_event_id = event_raw_time_cnt+1;
    set n_dtstart2445 = cast((20130100+event_raw_time_cnt) as nvarchar(10))||'T000000Z';
    set n_dtend2445 = cast((20130101+event_raw_time_cnt) as nvarchar(10))||'T000000Z';
    set n_lastDate2445 = cast((20130103+event_raw_time_cnt) as nvarchar(10))||'T000000Z';
    
    if mod(event_raw_time_cnt, 5) = 0 then 
    	set n_originalInstanceTime2445 = cast((20130102+event_raw_time_cnt) as nvarchar(10))||'T000000Z';
    else 
    	set n_originalInstanceTime2445 = null;
    end if;

    insert into EventsRawTimes ("_id", event_id, dtstart2445, dtend2445, originalInstanceTime2445, lastDate2445) values (event_raw_time_cnt+1, n_event_id, n_dtstart2445, n_dtend2445, n_originalInstanceTime2445, n_lastDate2445);

    set event_raw_time_cnt = event_raw_time_cnt + 1;
    END WHILE;
end 
@
---------------------
---------------------
@
begin
   declare instance_cnt integer;
   declare n_event_id int;
   declare n_begin bigint;
   declare n_end bigint;
   declare n_startDay int;
   declare n_endDay int;
   declare n_startMinute int;
   declare n_endMinute int;

   set instance_cnt = 0;
   
   WHILE instance_cnt <= 200 DO
    set n_event_id = mod(IRAND(), 200);
    set n_begin = mod(IRAND(), 10000)*1000;
    set n_end = mod(IRAND(), 10000)*1000;
    set n_startDay = mod(IRAND(), 10000)*1000;
    set n_endDay = mod(IRAND(), 10000)*1000;
    set n_startMinute = mod(IRAND(), 10000);
    set n_endMinute = mod(IRAND(), 10000);

    insert into Instances ("_id", event_id, "begin", "end", startDay, endDay, startMinute, endMinute) values (instance_cnt+1, n_event_id, n_begin, n_end, n_startDay, n_endDay, n_startMinute, n_endMinute);

    set instance_cnt = instance_cnt + 1;
    END WHILE;
end 
@



insert into Reminders ("_id", event_id, minutes, "method") values (29, 0, 10, 1);
insert into Reminders ("_id", event_id, minutes, "method") values (38, 0, 30, 2);
insert into Reminders ("_id", event_id, minutes, "method") values (39, 0, 30, 1);
insert into Reminders ("_id", event_id, minutes, "method") values (44, 0, 30, 2);
insert into Reminders ("_id", event_id, minutes, "method") values (45, 0, 30, 1);


insert into Calendars ("_id", account_name, account_type, "_sync_id", dirty, name, calendar_displayName, calendar_color, calendar_color_index, calendar_access_level, visible, sync_events, calendar_location, calendar_timezone, ownerAccount, canOrganizerRespond, canModifyTimeZone, canPartiallyUpdate, maxReminders, allowedReminders, allowedAvailability, allowedAttendeeTypes, deleted, cal_sync1, cal_sync2, cal_sync3, cal_sync4, cal_sync5, cal_sync6, cal_sync7, cal_sync8, cal_sync9, cal_sync10, isPrimary) values (1, 'simin.test@gmail.com', 'com.google', null, null, 'simin.test@gmail.com', 'simin.test@gmail.com', -14069085, null, 700, 1, 1, null, 'Europe/Stockholm', 'simin.test@gmail.com', 1, 1, 1, 5, '0,1,2', '0,1', '0,1,2,3', 0, 'https://www.google.com/calendar/feeds/simin.test%40gmail.com/private/full', 'https://www.google.com/calendar/feeds/default/allcalendars/full/simin.test%40gmail.com', 'https://www.google.com/calendar/feeds/default/allcalendars/full/simin.test%40gmail.com', '1', '0', null, null, '1383063042244', null, null, null);
insert into Calendars ("_id", account_name, account_type, "_sync_id", dirty, name, calendar_displayName, calendar_color, calendar_color_index, calendar_access_level, visible, sync_events, calendar_location, calendar_timezone, ownerAccount, canOrganizerRespond, canModifyTimeZone, canPartiallyUpdate, maxReminders, allowedReminders, allowedAvailability, allowedAttendeeTypes, deleted, cal_sync1, cal_sync2, cal_sync3, cal_sync4, cal_sync5, cal_sync6, cal_sync7, cal_sync8, cal_sync9, cal_sync10, isPrimary) values (2, 'mimerkungs@gmail.com', 'com.google', null, null, 'mimerkungs@gmail.com', 'mimerkungs@gmail.com', -14069085, null, 700, 1, 1, null, 'Asia/Calcutta', 'mimerkungs@gmail.com', 1, 1, 1, 5, '0,1,2', '0,1', '0,1,2,3', 0, 'https://www.google.com/calendar/feeds/mimerkungs%40gmail.com/private/full', 'https://www.google.com/calendar/feeds/default/allcalendars/full/mimerkungs%40gmail.com', 'https://www.google.com/calendar/feeds/default/allcalendars/full/mimerkungs%40gmail.com', '1', '0', null, null, '1383063069622', null, null, null);
insert into Calendars ("_id", account_name, account_type, "_sync_id", dirty, name, calendar_displayName, calendar_color, calendar_color_index, calendar_access_level, visible, sync_events, calendar_location, calendar_timezone, ownerAccount, canOrganizerRespond, canModifyTimeZone, canPartiallyUpdate, maxReminders, allowedReminders, allowedAvailability, allowedAttendeeTypes, deleted, cal_sync1, cal_sync2, cal_sync3, cal_sync4, cal_sync5, cal_sync6, cal_sync7, cal_sync8, cal_sync9, cal_sync10, isPrimary) values (3, 'simin.test@gmail.com', 'com.google', null, null, 'Contacts'' birthdays and events', 'Contacts'' birthdays and events', -5159922, null, 200, 1, 1, null, 'Europe/Stockholm', '#contacts@group.v.calendar.google.com', 1, 1, 1, 5, '0,1,2', '0,1', '0,1,2', 0, 'https://www.google.com/calendar/feeds/%23contacts%40group.v.calendar.google.com/private/full', 'https://www.google.com/calendar/feeds/default/allcalendars/full/%23contacts%40group.v.calendar.google.com', 'https://www.google.com/calendar/feeds/default/allcalendars/full/%23contacts%40group.v.calendar.google.com', '1', '0', null, null, '1383063042299', null, null, null);
insert into Calendars ("_id", account_name, account_type, "_sync_id", dirty, name, calendar_displayName, calendar_color, calendar_color_index, calendar_access_level, visible, sync_events, calendar_location, calendar_timezone, ownerAccount, canOrganizerRespond, canModifyTimeZone, canPartiallyUpdate, maxReminders, allowedReminders, allowedAvailability, allowedAttendeeTypes, deleted, cal_sync1, cal_sync2, cal_sync3, cal_sync4, cal_sync5, cal_sync6, cal_sync7, cal_sync8, cal_sync9, cal_sync10, isPrimary) values (4, 'simin.test@gmail.com', 'com.google', null, null, 'Svenska helgdagar', 'Svenska helgdagar', -13671671, null, 200, 1, 1, null, 'Europe/Stockholm', 'sv.swedish#holiday@group.v.calendar.google.com', 1, 1, 1, 5, '0,1,2', '0,1', '0,1,2', 0, 'https://www.google.com/calendar/feeds/sv.swedish%23holiday%40group.v.calendar.google.com/private/full', 'https://www.google.com/calendar/feeds/default/allcalendars/full/sv.swedish%23holiday%40group.v.calendar.google.com', 'https://www.google.com/calendar/feeds/default/allcalendars/full/sv.swedish%23holiday%40group.v.calendar.google.com', '1', '0', null, null, '1383063042335', null, null, null);
insert into Calendars ("_id", account_name, account_type, "_sync_id", dirty, name, calendar_displayName, calendar_color, calendar_color_index, calendar_access_level, visible, sync_events, calendar_location, calendar_timezone, ownerAccount, canOrganizerRespond, canModifyTimeZone, canPartiallyUpdate, maxReminders, allowedReminders, allowedAvailability, allowedAttendeeTypes, deleted, cal_sync1, cal_sync2, cal_sync3, cal_sync4, cal_sync5, cal_sync6, cal_sync7, cal_sync8, cal_sync9, cal_sync10, isPrimary) values (5, 'simin.test@gmail.com', 'com.google', null, null, 'Veckonummer', 'Veckonummer', -7908087, null, 200, 1, 1, null, 'Europe/Stockholm', 'e_2_sv#weeknum@group.v.calendar.google.com', 1, 1, 1, 5, '0,1,2', '0,1', '0,1,2', 0, 'https://www.google.com/calendar/feeds/e_2_sv%23weeknum%40group.v.calendar.google.com/private/full', 'https://www.google.com/calendar/feeds/default/allcalendars/full/e_2_sv%23weeknum%40group.v.calendar.google.com', 'https://www.google.com/calendar/feeds/default/allcalendars/full/e_2_sv%23weeknum%40group.v.calendar.google.com', '1', '0', null, null, '1383063042364', null, null, null);
insert into Calendars ("_id", account_name, account_type, "_sync_id", dirty, name, calendar_displayName, calendar_color, calendar_color_index, calendar_access_level, visible, sync_events, calendar_location, calendar_timezone, ownerAccount, canOrganizerRespond, canModifyTimeZone, canPartiallyUpdate, maxReminders, allowedReminders, allowedAvailability, allowedAttendeeTypes, deleted, cal_sync1, cal_sync2, cal_sync3, cal_sync4, cal_sync5, cal_sync6, cal_sync7, cal_sync8, cal_sync9, cal_sync10, isPrimary) values (6, 'mimerkungs@gmail.com', 'com.google', null, null, 'Kontakters födelsedagar och händelser', 'Kontakters födelsedagar och händelser', -5159922, null, 200, 0, 0, null, 'Asia/Calcutta', '#contacts@group.v.calendar.google.com', 1, 1, 1, 5, '0,1,2', '0,1', '0,1,2', 0, 'https://www.google.com/calendar/feeds/%23contacts%40group.v.calendar.google.com/private/full', 'https://www.google.com/calendar/feeds/default/allcalendars/full/%23contacts%40group.v.calendar.google.com', 'https://www.google.com/calendar/feeds/default/allcalendars/full/%23contacts%40group.v.calendar.google.com', '1', '0', null, null, '1383063069714', null, null, null);
insert into Calendars ("_id", account_name, account_type, "_sync_id", dirty, name, calendar_displayName, calendar_color, calendar_color_index, calendar_access_level, visible, sync_events, calendar_location, calendar_timezone, ownerAccount, canOrganizerRespond, canModifyTimeZone, canPartiallyUpdate, maxReminders, allowedReminders, allowedAvailability, allowedAttendeeTypes, deleted, cal_sync1, cal_sync2, cal_sync3, cal_sync4, cal_sync5, cal_sync6, cal_sync7, cal_sync8, cal_sync9, cal_sync10, isPrimary) values (7, 'mimerkungs@gmail.com', 'com.google', null, null, 'Svenska helgdagar', 'Svenska helgdagar', -13671671, null, 200, 0, 0, null, 'Asia/Calcutta', 'sv.swedish#holiday@group.v.calendar.google.com', 1, 1, 1, 5, '0,1,2', '0,1', '0,1,2', 0, 'https://www.google.com/calendar/feeds/sv.swedish%23holiday%40group.v.calendar.google.com/private/full', 'https://www.google.com/calendar/feeds/default/allcalendars/full/sv.swedish%23holiday%40group.v.calendar.google.com', 'https://www.google.com/calendar/feeds/default/allcalendars/full/sv.swedish%23holiday%40group.v.calendar.google.com', '1', '0', null, null, '1383063069737', null, null, null);
insert into Calendars ("_id", account_name, account_type, "_sync_id", dirty, name, calendar_displayName, calendar_color, calendar_color_index, calendar_access_level, visible, sync_events, calendar_location, calendar_timezone, ownerAccount, canOrganizerRespond, canModifyTimeZone, canPartiallyUpdate, maxReminders, allowedReminders, allowedAvailability, allowedAttendeeTypes, deleted, cal_sync1, cal_sync2, cal_sync3, cal_sync4, cal_sync5, cal_sync6, cal_sync7, cal_sync8, cal_sync9, cal_sync10, isPrimary) values (8, 'mimerkungs@gmail.com', 'com.google', null, null, 'Veckonummer', 'Veckonummer', -7908087, null, 200, 0, 0, null, 'Asia/Calcutta', 'e_2_sv#weeknum@group.v.calendar.google.com', 1, 1, 1, 5, '0,1,2', '0,1', '0,1,2', 0, 'https://www.google.com/calendar/feeds/e_2_sv%23weeknum%40group.v.calendar.google.com/private/full', 'https://www.google.com/calendar/feeds/default/allcalendars/full/e_2_sv%23weeknum%40group.v.calendar.google.com', 'https://www.google.com/calendar/feeds/default/allcalendars/full/e_2_sv%23weeknum%40group.v.calendar.google.com', '1', '0', null, null, '1383063069761', null, null, null);



