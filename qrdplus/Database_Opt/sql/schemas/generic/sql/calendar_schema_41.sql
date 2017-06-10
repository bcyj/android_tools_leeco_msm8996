-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
set sqlite on;
--Set correct schema version
  call compatibility.set_or_create_pragma('calendar_db','user_version','600');
-- update calendar schema for Android 4.1, based on calendar_schema_40.sql

-- update table

ALTER TABLE Attendees ADD "attendeeIdentity" nvarchar(128); 
ALTER TABLE Attendees ADD "attendeeIdNamespace" nvarchar(128); 

ALTER TABLE Events ADD "customAppPackage" nvarchar(128); 
ALTER TABLE Events ADD "customAppUri" nvarchar(128); 

ALTER TABLE Events ADD "isOrganizer" INTEGER; 
ALTER TABLE Events ADD "uid2445" varchar(128); 


ALTER TABLE Calendars ADD "isPrimary" INTEGER;

ALTER TABLE Calendars ADD mutators nvarchar(128);
ALTER TABLE Events ADD mutators nvarchar(128);



-- update view

drop view "view_events";


CREATE VIEW "view_events" AS
SELECT "Events"."_id"     AS "_id",
    "title",
    "description",
    "eventLocation",
    "eventColor",
    "eventColor_index",
    "eventStatus",
    "selfAttendeeStatus",
    "dtstart",
    "dtend",
    "duration",
    "eventTimezone",
    "eventEndTimezone",
    "allDay",
    "accessLevel",
    "availability",
    "hasAlarm",
    "hasExtendedProperties",
    "rrule",
    "rdate",
    "exrule",
    "exdate",
    "original_sync_id",
    "original_id",
    "originalInstanceTime",
    "originalAllDay",
    "lastDate",
    "hasAttendeeData",
    "calendar_id",
    "guestsCanInviteOthers",
    "guestsCanModify",
    "guestsCanSeeGuests",
    "organizer",
     COALESCE(isOrganizer, organizer = ownerAccount) as isOrganizer,
    "customAppPackage",
    "customAppUri",
    uid2445,
    "sync_data1",
    "sync_data2",
    "sync_data3",
    "sync_data4",
    "sync_data5",
    "sync_data6",
    "sync_data7",
    "sync_data8",
    "sync_data9",
    "sync_data10",
    "Events"."deleted"  AS "deleted",
    "Events"."_sync_id" AS "_sync_id",
    "Events"."dirty"    AS "dirty",
    Events.mutators AS mutators,
    "lastSynced",
    "Calendars"."account_name" AS "account_name",
    "Calendars"."account_type" AS "account_type",
    "calendar_timezone",
    "calendar_displayName",
    "calendar_location",
    "visible",
    "calendar_color",
    "calendar_color_index",
    "calendar_access_level",
    "maxReminders",
    "allowedReminders",
    "allowedAttendeeTypes",
    "allowedAvailability",
    "canOrganizerRespond",
    "canModifyTimeZone",
    "canPartiallyUpdate",
    "cal_sync1",
    "cal_sync2",
    "cal_sync3",
    "cal_sync4",
    "cal_sync5",
    "cal_sync6",
    "cal_sync7",
    "cal_sync8",
    "cal_sync9",
    "cal_sync10",
    "ownerAccount",
    "sync_events",
    COALESCE("eventColor","calendar_color") AS "displayColor"
FROM "Events"
JOIN "Calendars"
ON  (("Events"."calendar_id" = "Calendars"."_id"));


-- update triggers 

drop trigger calendar_color_update;

@
create trigger calendar_color_update before update on "Calendars"
referencing old row as o new row as n for each row
begin atomic
    if o.calendar_color_index <> n.calendar_color_index or o.calendar_color_index is null and n.calendar_color_index is not null then

	set n.calendar_color = 
	(select "color" from "Colors" where (account_name = n.account_name) and (account_type = 			
	n.account_type) and (color_index = n.calendar_color_index) 
	and (color_type = 0)
	);
    end if;
end
@

drop trigger event_color_update;

@
create trigger event_color_update before update on "Events"
referencing old row as o new row as n for each row
begin atomic
    if o.eventColor_index <> n.eventColor_index or o.eventColor_index is null and n.eventColor_index is not null then

	set n.eventColor = 
	(select "color" from "Colors" 
		where ("account_name" = 
		(select "account_name" from "Calendars" where ("_id" = "n"."calendar_id"))) and ("account_type" = (select "account_type" from "Calendars" 		where ("_id" = "n"."calendar_id"))) and ("color_index" = "n"."eventColor_index") 
	and ("color_type" = 1)
	);
    end if;
end
@


@
create procedure clear_data()
modifies sql data
begin
delete from Calendars;
delete from "_sync_state";
delete from Attendees;
delete from CalendarAlerts;
delete from Colors;
delete from Events;
delete from EventsRawTimes;
delete from ExtendedProperties;
delete from Instances;
delete from Reminders;
delete from Changes;
end
@

grant select on Attendees to mimer$permission$calendar_read;
grant select on Instances to mimer$permission$calendar_read;
grant select on view_events to mimer$permission$calendar_read;
