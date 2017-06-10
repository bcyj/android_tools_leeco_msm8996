set echo off;
-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
set message off;
--delete from "_sync_state";
--alter sequence sync_state_seq restart with 1;

alter sequence attendees_seq restart with 1;
delete from Attendees;

alter sequence calendaralerts_seq restart with 1;
delete from CalendarAlerts;

alter sequence calendars_seq restart with 1;
delete from Calendars;

alter sequence colors_seq restart with 1;
delete from Colors;

alter sequence events_seq restart with 1;
delete from Events;

alter sequence eventsrawtimes_seq restart with 1;
delete from EventsRawTimes;

alter sequence extendedproperties_seq restart with 1;
delete from ExtendedProperties;

alter sequence instances_seq restart with 1;
delete from Instances;

alter sequence reminders_seq restart with 1;
delete from Reminders;

--alter sequence calendarcache_seq restart with 1;
--delete from CalendarCache;

--alter sequence calendarmetadata_seq restart with 1;
--delete from CalendarMetaData;
