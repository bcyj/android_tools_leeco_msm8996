-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
set sqlite on;

-- ###############################################################################
-- Tables 
-- ###############################################################################

CREATE UNIQUE sequence attendees_seq as BIGINT;

CREATE TABLE "Attendees" (
        "_id" bigint PRIMARY KEY DEFAULT next_value OF attendees_seq with check option,
        "event_id" BIGINT,
        "attendeeName" nvarchar(128),
        "attendeeEmail" nvarchar(128),
        "attendeeStatus" INT,
        "attendeeRelationship" INT,
        "attendeeType" INT
    ) IN "calendar_db";

CREATE UNIQUE sequence calendaralerts_seq as BIGINT;

CREATE TABLE "CalendarAlerts" (
        "_id" bigint PRIMARY KEY DEFAULT next_value OF calendaralerts_seq with check option,
        "event_id" BIGINT,
        "begin" bigint NOT NULL,
        "end" bigint NOT NULL,
        "alarmTime" bigint NOT NULL,
        "creationTime" bigint NOT NULL,
        "receivedTime" bigint NOT NULL,
        "notifyTime" bigint NOT NULL,
        "state" INT NOT NULL,
        "minutes" INT,
        UNIQUE("alarmTime", "begin", "event_id")
    ) IN "calendar_db";

CREATE UNIQUE sequence calendarcache_seq as BIGINT;

CREATE TABLE "CalendarCache" (
        "_id" BIGINT PRIMARY KEY DEFAULT next_value OF calendarcache_seq with check option,
        KEY nvarchar(128) NOT NULL,
        "value" nvarchar(128)
    ) IN "calendar_db";

CREATE UNIQUE sequence calendarmetadata_seq as BIGINT;

CREATE TABLE "CalendarMetaData" (
        "_id" BIGINT PRIMARY KEY DEFAULT next_value OF calendarmetadata_seq with check option,
        "localTimezone" nvarchar(128),
        "minInstance" bigint,
        "maxInstance" bigint
    ) IN "calendar_db";

CREATE UNIQUE sequence calendars_seq as BIGINT;

CREATE TABLE "Calendars" (
        "_id" bigint PRIMARY KEY DEFAULT next_value OF calendars_seq with check option,
        "account_name" nvarchar(128),
        "account_type" nvarchar(128),
        "_sync_id" nvarchar(256),
        "dirty" INT,
        "name" nvarchar(128),
        "calendar_displayName" nvarchar(128),
        "calendar_color" INT,
        "calendar_color_index" nvarchar(128),
        "calendar_access_level" INT,
        "visible" INT NOT NULL DEFAULT 1,
        "sync_events" INT NOT NULL DEFAULT 0,
        "calendar_location" nvarchar(128),
        "calendar_timezone" nvarchar(128),
        "ownerAccount" nvarchar(128),
        "canOrganizerRespond" INT NOT NULL DEFAULT 1,
        "canModifyTimeZone" INT DEFAULT 1,
        "canPartiallyUpdate" INT DEFAULT 0,
        "maxReminders" INT DEFAULT 5,
        "allowedReminders" nvarchar(128) DEFAULT '0,1',
        "allowedAvailability" nvarchar(128) DEFAULT '0,1',
        "allowedAttendeeTypes" nvarchar(128) DEFAULT '0,1,2',
        "deleted" INT NOT NULL DEFAULT 0,
        "cal_sync1" nvarchar(256),
        "cal_sync2" nvarchar(256),
        "cal_sync3" nvarchar(256),
        "cal_sync4" nvarchar(256),
        "cal_sync5" nvarchar(256),
        "cal_sync6" nvarchar(256),
        "cal_sync7" nvarchar(256),
        "cal_sync8" nvarchar(256),
        "cal_sync9" nvarchar(256),
        "cal_sync10" nvarchar(256)
    ) IN "calendar_db";

CREATE UNIQUE sequence colors_seq as BIGINT;

CREATE TABLE "Colors" (
        "_id" BIGINT PRIMARY KEY DEFAULT next_value OF colors_seq with check option,
        "account_name" nvarchar(128) NOT NULL,
        "account_type" nvarchar(128) NOT NULL,
        "data" nvarchar(128),
        "color_type" INT NOT NULL,
        "color_index" nvarchar(128) NOT NULL,
        "color" INT NOT NULL
    ) IN "calendar_db";

CREATE UNIQUE sequence events_seq as BIGINT;

CREATE TABLE "Events" (
        "_id" bigint PRIMARY KEY DEFAULT next_value OF events_seq with check option,
        "_sync_id" VARCHAR(512),
        "dirty" INT,
        "lastSynced" INT DEFAULT 0,
        "calendar_id" BIGINT NOT NULL,
        "title" nvarchar(128),
        "eventLocation" nvarchar(128),
        "description" NCLOB,
        "eventColor" INT,
        "eventColor_index" nvarchar(128),
        "eventStatus" INT,
        "selfAttendeeStatus" INT NOT NULL DEFAULT 0,
        "dtstart" bigint,
        "dtend" bigint,
        "eventTimezone" nvarchar(128),
        "duration" nvarchar(128),
        "allDay" INT NOT NULL DEFAULT 0,
        "accessLevel" INT NOT NULL DEFAULT 0,
        "availability" INT NOT NULL DEFAULT 0,
        "hasAlarm" INT NOT NULL DEFAULT 0,
        "hasExtendedProperties" INT NOT NULL DEFAULT 0,
        "rrule" nvarchar(128),
        "rdate" nvarchar(128),
        "exrule" nvarchar(128),
        "exdate" nvarchar(128),
        "original_id" INT,
        "original_sync_id" VARCHAR(512),
        "originalInstanceTime" bigint,
        "originalAllDay" INT,
        "lastDate" bigint,
        "hasAttendeeData" INT NOT NULL DEFAULT 0,
        "guestsCanModify" INT NOT NULL DEFAULT 0,
        "guestsCanInviteOthers" INT NOT NULL DEFAULT 1,
        "guestsCanSeeGuests" INT NOT NULL DEFAULT 1,
        "organizer" nvarchar(128),
        "deleted" INT NOT NULL DEFAULT 0,
        "eventEndTimezone" nvarchar(128),
        "sync_data1" VARCHAR(512),
        "sync_data2" VARCHAR(512),
        "sync_data3" VARCHAR(512),
        "sync_data4" VARCHAR(512),
        "sync_data5" VARCHAR(512),
        "sync_data6" VARCHAR(512),
        "sync_data7" VARCHAR(512),
        "sync_data8" VARCHAR(512),
        "sync_data9" VARCHAR(512),
        "sync_data10" VARCHAR(512)
    ) IN "calendar_db";

CREATE UNIQUE sequence eventsrawtimes_seq as BIGINT;

CREATE TABLE "EventsRawTimes" (
        "_id" bigint PRIMARY KEY DEFAULT next_value OF eventsrawtimes_seq with check option,
        "event_id" BIGINT NOT NULL,
        "dtstart2445" nvarchar(128),
        "dtend2445" nvarchar(128),
        "originalInstanceTime2445" nvarchar(128),
        "lastDate2445" nvarchar(128),
        UNIQUE("event_id")
    ) IN "calendar_db";

CREATE UNIQUE sequence extendedproperties_seq as BIGINT;

CREATE TABLE "ExtendedProperties" (
        "_id" bigint PRIMARY KEY DEFAULT next_value OF extendedproperties_seq with check option,
        "event_id" BIGINT,
        "name" nvarchar(128),
        "value" nvarchar(128)
    ) IN "calendar_db";


CREATE UNIQUE sequence instances_seq as BIGINT;

CREATE TABLE "Instances" (
        "_id" bigint PRIMARY KEY DEFAULT next_value OF instances_seq with check option,
        "event_id" BIGINT,
        "begin" bigint,
        "end" bigint,
        "startDay" BIGINT,
        "endDay" BIGINT,
        "startMinute" BIGINT,
        "endMinute" BIGINT,
        UNIQUE("event_id", "begin", "end")
    ) IN "calendar_db";

CREATE UNIQUE sequence reminders_seq as BIGINT;

CREATE TABLE "Reminders" (
        "_id" bigint PRIMARY KEY DEFAULT next_value OF reminders_seq with check option,
        "event_id" BIGINT,
        "minutes" BIGINT,
        "method" INT NOT NULL DEFAULT 0
    ) IN "calendar_db";

CREATE UNIQUE sequence sync_state_seq as BIGINT;

CREATE TABLE "_sync_state" (
        "_id" BIGINT PRIMARY KEY DEFAULT next_value OF sync_state_seq with check option,
        "account_name" nvarchar(128) NOT NULL,
        "account_type" nvarchar(128) NOT NULL,
        "data" VARBINARY(10000),
        UNIQUE("account_name", "account_type")
    ) IN "calendar_db";

CREATE TABLE "_sync_state_metadata" (
        "version" INT
    ) IN "calendar_db";

INSERT INTO "_sync_state_metadata"(version) VALUES(1);






insert into "android_metadata" ("locale") values('en_US');   

insert into "CalendarCache" ("_id", "key", "value") values (-2140311132, 'timezoneDatabaseVersion', '2011l');
insert into "CalendarCache" ("_id", "key", "value") values (495220580, 'timezoneInstancesPrevious', 'GMT');
insert into "CalendarCache" ("_id", "key", "value") values (1126213331, 'timezoneType', 'auto');
insert into "CalendarCache" ("_id", "key", "value") values (1167965829, 'timezoneInstances', 'GMT');

insert into "CalendarMetaData" ("_id", "localTimezone", "minInstance", "maxInstance") values (1, 'GMT', 1330560000000, 1335916800000);     

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
    "sync_events"
FROM "Events"
JOIN "Calendars"
ON (("Events"."calendar_id" = "Calendars"."_id")) ;





 create index "attendeesEventIdIndex" on "Attendees"("event_id");

 create index "attendeesEmailIndex" on "Attendees"("attendeeEmail" collate CURRENT_COLLATION_2 ASC);

 create index "calendarAlertsEventIdIndex" on "CalendarAlerts"("event_id");

 create index "eventsCalendarIdIndex" on "Events"("calendar_id");

 create index "extendedPropertiesEventIdIndex" on "ExtendedProperties"("event_id");

create index "instancesStartDayIndex" on "Instances"("startDay", "endDay");

 create index "eventsdtstartIndex" on "Events"("dtstart");

 create index "remindersEventIdIndex" on "Reminders"("event_id");



@
create trigger calendar_cleanup after delete on Calendars
referencing old table as old_table
begin atomic
        for select "_id" as old_id from old_table DO
                delete from Events where "calendar_id" = old_id;
        end for;

end
@

@
create trigger calendar_color_update before update on "Calendars"
referencing old row as o new row as n for each row
begin atomic
    if o.calendar_color_index <> n.calendar_color_index or o.calendar_color_index is null and n.calendar_color_index is not null then

	set n.calendar_color = 
	(select "color" from "Colors" where (account_name = n.account_name) and (account_type = 			
	n.account_type) and (color_index = n.calendar_color_index));

    end if;
end
@

@
create trigger event_color_update before update on "Events"
referencing old row as o new row as n for each row
begin atomic
    if o.eventColor_index <> n.eventColor_index or o.eventColor_index is null and n.eventColor_index is not null then

	set n.eventColor = 
	(select "color" from "Colors" 
		where ("account_name" = 
		(select "account_name" from "Calendars" where ("_id" = "n"."calendar_id"))) and ("account_type" = (select "account_type" 			from "Calendars" where ("_id" = "n"."calendar_id"))) and ("color_index" = "n"."eventColor_index"));

    end if;
end
@

@
create trigger events_cleanup_delete after delete on Events
referencing old table as old_table
begin atomic
        for select "_id" as old_id from old_table DO
                delete from Instances where "event_id" = old_id;

		delete from "EventsRawTimes" where ("event_id" = old_id);

		delete from "Attendees" where ("event_id" = old_id);

		delete from "Reminders" where ("event_id" = old_id);

		delete from "CalendarAlerts" where ("event_id" = old_id);

		delete from "ExtendedProperties" where ("event_id" = old_id);
        end for;
end
@


@
create trigger original_sync_update before update on "Events"
referencing old row as o new row as n 
for each row
begin atomic
    if o."_sync_id" <> n."_sync_id" or o."_sync_id" is null and n."_sync_id" is not null then
	set n.original_sync_id = "n"."_sync_id";
	
    end if;
end
@

create table "Changes"("lastrow_id" int, "changes_count" int) in "calendar_db"; 

@
create trigger events_insert after insert on "Events"
referencing new table as new_table
begin atomic

declare cnt int default -1;
declare last_id int;

  for select "_id" as new_id from new_table DO
	set cnt = cnt + 1;
	set last_id = new_id;
  end for;

  update Changes set "lastrow_id" = last_id, "changes_count"=cnt;
	
end
@


-- ###############################################################################
-- Indexes 
-- ###############################################################################

call compatibility.set_or_create_pragma('calendar_db','user_version','308');
@
create function cal_group_concat("pid" int, "column_name" nvarchar(100))
returns nvarchar(2000)
reads sql data
deterministic
begin  
    declare T nvarchar(2000);
    case "column_name"
    when 'attendeeEmail' then
        for select "attendeeEmail" from "Attendees" where "Attendees"."event_id" = "pid" do
            set T = coalesce(T||',','')||"attendeeEmail";
        end for;
    when 'attendeeName' then
        for select "attendeeName" from "Attendees" where "Attendees"."event_id" = "pid" do
            set T = coalesce(T||',','')||"attendeeName";
        end for;
    end case;
    return T;
end
@ 


