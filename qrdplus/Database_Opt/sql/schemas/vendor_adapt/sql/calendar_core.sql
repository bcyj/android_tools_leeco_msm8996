-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
whenever error continue;
drop databank calendar_db cascade;
whenever error exit;
-- ###############################################################################
-- Databanks & schemas
-- ###############################################################################

create databank calendar_db set goalsize 100K;
call compatibility.map_database('/data/data/com.android.providers.calendar/databases/calendar.db', 'calendar_db', 'calendar', '');
call compatibility.map_auxiliary_database('/data/user/0/com.android.providers.calendar/databases/calendar.db', 'calendar_db', 'calendar','');
call compatibility.map_database('/data/data/com.android.providers.calendar/databases/test/calendar.db', 'db_data_data_com_mimer_providers_calendar_databases_test_calendar_db', 'calendar', '');

CREATE UNIQUE SEQUENCE android_metadata_id_seq in calendar_db;
CREATE TABLE android_metadata(
	locale nvarchar(128)
) in calendar_db;


-- ###############################################################################
-- Tables
-- ###############################################################################

-- ###############################################################################
-- Indexes 
-- ###############################################################################
