-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
-- ##############################################################################
-- NOTES:
-- * There seems to be a new function called: _PHONE_NUMBER_STRIPPED_REVERSED that is used in the v1 views.
-- * 
-- ##############################################################################
set sqlite on;
whenever error exit;



--create databank db_data_data_com_mimer_providers_contacts_databases_contacts2_db set filesize 1M, goalsize 2M;
-- ###############################################################################
-- Databanks & schemas
-- ###############################################################################

-- call compatibility.map_database('/data/data/com.android.providers.contacts/databases/test/contacts2.db', 'contacts2_db', 'contacts2','');

	call compatibility.map_database('/data/data/com.android.providers.contacts/databases/profile.db', 'profile_db', 'profile','profile', timestamp '0001-01-01 00:00:00');

--	the path to profile.db is changed into /data/user/0/com.android.providers.contacts/databases/profile.db
	call compatibility.map_auxiliary_database('/data/user/0/com.android.providers.contacts/databases/profile.db', 'profile_db', 'profile','profile', timestamp '0001-01-01 00:00:00');

create databank local_groups_db of 10 pages;
call compatibility.map_database('/data/data/com.android.providers.contacts/databases/local_groups.db', 'local_groups_db', 'contacts','', timestamp '0001-01-01 00:00:00');
call compatibility.map_auxiliary_database('/data/user/0/com.android.providers.contacts/databases/local_groups.db', 'local_groups_db', 'contacts','', timestamp '0001-01-01 00:00:00');

create ident profile as program identified by 'nopass';
grant databank to profile;
grant schema to profile;

create databank contacts2_db set goalsize 2M;
call compatibility.map_database('/data/data/com.android.providers.contacts/databases/contacts2.db', 'contacts2_db', 'contacts','', timestamp '0001-01-01 00:00:00');
call compatibility.map_auxiliary_database('/data/user/0/com.android.providers.contacts/databases/contacts2.db', 'contacts2_db', 'contacts','', timestamp '0001-01-01 00:00:00');
--The presence schema, used instead of "attach database presence_db"
create schema presence_db;
create databank presence_data of 100 pages;-- set option work;
call compatibility.map_database('/data/data/com.android.providers.contacts/databases/presence.db', 'presence_data', 'presence_db','', timestamp '0001-01-01 00:00:00');

read 'contacts_schema_41_core.sql';
call compatibility.set_or_create_pragma('contacts2_db','user_version','805');

--create databank db_data_data_com_mimer_providers_contacts_databases_test_contacts2_db of 10 pages;

CREATE TABLE presence_db.presence (
	presence_data_id BIGINT,
	protocol INTEGER NOT NULL,
	custom_protocol NVARCHAR(120),
	im_handle NVARCHAR(120),
	im_account NVARCHAR(120),
	presence_contact_id BIGINT ,
	presence_raw_contact_id BIGINT ,
	mode INTEGER,
        chat_capability INTEGER NOT NULL DEFAULT 0,
	constraint presence_pk PRIMARY KEY (presence_data_id),
	constraint presence_unique UNIQUE(protocol, custom_protocol, im_handle, im_account)
) in presence_data;
CREATE SYNONYM presence for presence_db.presence;

CREATE TABLE  presence_db.agg_presence (
	presence_contact_id BIGINT,
	mode INTEGER,
        chat_capability INTEGER NOT NULL DEFAULT 0,
	constraint agg_presence_pk PRIMARY KEY (presence_contact_id)
) in presence_data;
CREATE SYNONYM agg_presence for presence_db.agg_presence;

CREATE INDEX presence_db.presenceIndex ON presence_db.presence (presence_raw_contact_id);

CREATE INDEX presence_db.presenceIndex2 ON presence_db.presence (presence_contact_id);

--create databank for local_group_db, create local_groups table
CREATE UNIQUE SEQUENCE local_group_seq as BIGINT in local_groups_db;
create table local_groups (
	"_id" BIGINT default next_value of local_group_seq,
	"title" nvarchar(100),
	"count" integer
) in local_groups_db;



@
create procedure clear_data()
modifies sql data
begin
declare res int;
set res = BUILTIN.USERINFO(1001,0,1);
delete from search_index;
delete from name_lookup;
delete from phone_lookup;
delete from contacts;
delete from calls;
delete from "_sync_state";
delete from groups;
delete from agg_presence;
delete from agg_exceptions;
delete from accounts where account_name is not null;
delete from default_directory;
delete from directories;
delete from visible_contacts;
delete from properties;
delete from data_usage_stat;
delete from packages;
delete from photo_files;
delete from settings;
delete from status_updates;
delete from stream_item_photos;
delete from stream_items;
delete from voicemail_status;
delete from data;
delete from raw_contacts;
delete from v1_settings;
delete from presence_db.agg_presence;
delete from presence_db.presence;
delete from deleted_contacts;
delete from local_groups;
set res = BUILTIN.USERINFO(1001,0,0);
end
@

--Should this be used in Androd 4? The in_visible_group column do not longer exists, but there is another table called visible_contacs
--that I suppose should be used.
@
create procedure updateAggregate(IN in_contact_id BIGINT, IN in_raw_contact_id BIGINT, IN in_mimetype BIGINT, IN set_presence boolean)
modifies sql data
begin
--setContactIdAndMarkAggregated
	update raw_contacts set contact_id = in_contact_id, aggregation_needed=0 
	where "_id" = in_raw_contact_id;

--updateContactVisible
-- call update_in_visible_group(in_mimetype, in_contact_id);

--setPresenceContactId
	if set_presence then
		update presence set presence_contact_id = in_contact_id
		where presence_raw_contact_id = in_raw_contact_id;
	end if;

--updateAggregatedPresence
begin
	declare continue handler for sqlstate '23000'
        begin
            update agg_presence set presence_contact_id=in_contact_id,mode=(
            select MAX(mode) from presence where presence_contact_id = in_contact_id);
        end;
	insert into agg_presence(presence_contact_id,mode) 
	select in_contact_id,MAX(mode) from presence where presence_contact_id = in_contact_id;
end;
end
@


grant select,update,delete,insert on table presence_db.agg_presence to profile;
grant select,update,delete,insert on table presence_db.presence to profile;

enter 'profile' using 'nopass';
create databank profile_db set goalsize 32k;
CREATE SYNONYM agg_presence for presence_db.agg_presence;
CREATE SYNONYM presence for presence_db.presence;
read 'contacts_schema_41_profile.sql';
call compatibility.set_or_create_pragma('profile_db','user_version','805');

@
create procedure clear_data()
modifies sql data
begin
delete from raw_contacts;
delete from calls;
delete from data;
delete from contacts;
delete from "_sync_state";
delete from groups;
delete from agg_presence;
delete from agg_exceptions;
delete from search_index;
delete from name_lookup;
delete from phone_lookup;
delete from accounts;
delete from default_directory;
delete from directories;
delete from visible_contacts;
delete from properties;
delete from data_usage_stat;
delete from packages;
delete from photo_files;
delete from settings;
delete from status_updates;
delete from stream_item_photos;
delete from stream_items;
delete from voicemail_status;
delete from v1_settings;
delete from deleted_contacts;
end
@


-- Profile "high value" sequences
--alter sequence raw_contacts_id_seq restart with 9223372034707292160;
--alter sequence data_id_seq restart with 9223372034707292160;
--alter sequence directories_id_seq restart with 9223372034707292160;
--alter sequence contacts_id_seq restart with 9223372034707292160;
--alter sequence photo_files_id_seq restart with 9223372034707292160;
--alter sequence groups_id_seq restart with 9223372034707292160;
--alter sequence calls_id_seq restart with 9223372034707292160;
--alter sequence stream_item_photos_id_seq restart with 9223372034707292160;
--alter sequence stream_items_id_seq restart with 9223372034707292160;

--@
--create trigger fix_contact_profile_id before insert on contacts
--referencing new row as n for each row
--begin atomic
--      set n."_id" = next_value of contacts_id_seq + 9223372034707292160;
--end
--@

--@
--create trigger fix_raw_contact_profile_id before insert on raw_contacts
--referencing new row as n for each row
--begin atomic
--      set n."_id" = next_value of raw_contacts_id_seq + 9223372034707292160;
--end
--@

--@
--create trigger fix_data_profile_id before insert on data
--referencing new row as n for each row
--begin atomic
--      set n."_id" = next_value of data_id_seq + 9223372034707292160;
--end
--@


--@
--create trigger fix_stream_items_profile_id before insert on stream_items
--referencing new row as n for each row
--begin atomic
--      set n."_id" = next_value of stream_items_id_seq + 9223372034707292160;
--end
--@

--@
--create trigger fix_stream_item_photos_profile_id before insert on stream_item_photos
--referencing new row as n for each row
--begin atomic
--      set n."_id" = next_value of stream_item_photos_id_seq + 9223372034707292160;
--end
--@


--@
--create trigger fix_groups_id before insert on groups
--referencing new row as n for each row
--begin atomic
--      set n."_id" = next_value of groups_id_seq + 9223372034707292160;
--end
--@


--@
--create trigger fix_calls_id before insert on calls
--referencing new row as n for each row
--begin atomic
--      set n."_id" = next_value of calls_id_seq + 9223372034707292160;
--end
--@

--@
--create trigger fix_directories_id before insert on directories
--referencing new row as n for each row
--begin atomic
--      set n."_id" = next_value of directories_id_seq + 9223372034707292160;
--end
--@


-- The below is the result of the query select * from sqlite_master where type='table' on the
-- profile.db database (from a Android 4.0 system) after it has been created by the original
-- Google Contacts Provider.
--
-- table|android_metadata|android_metadata|3|CREATE TABLE android_metadata (locale TEXT)
-- table|_sync_state|_sync_state|4|CREATE TABLE _sync_state (_id INTEGER PRIMARY KEY,account_name TEXT NOT NULL,account_type TEXT NOT NULL,data TEXT,UNIQUE(account_name, account_type))
-- table|_sync_state_metadata|_sync_state_metadata|6|CREATE TABLE _sync_state_metadata (version INTEGER)
-- table|contacts|contacts|7|CREATE TABLE contacts (_id INTEGER PRIMARY KEY AUTOINCREMENT,name_raw_contact_id INTEGER REFERENCES raw_contacts(_id),photo_id INTEGER REFERENCES data(_id),photo_file_id INTEGER REFERENCES photo_files(_id),custom_ringtone TEXT,send_to_voicemail INTEGER NOT NULL DEFAULT 0,times_contacted INTEGER NOT NULL DEFAULT 0,last_time_contacted INTEGER,starred INTEGER NOT NULL DEFAULT 0,has_phone_number INTEGER NOT NULL DEFAULT 0,lookup TEXT,status_update_id INTEGER REFERENCES data(_id))
-- table|sqlite_sequence|sqlite_sequence|8|CREATE TABLE sqlite_sequence(name,seq)
-- table|raw_contacts|raw_contacts|11|CREATE TABLE raw_contacts (_id INTEGER PRIMARY KEY AUTOINCREMENT,account_name STRING DEFAULT NULL, account_type STRING DEFAULT NULL, data_set STRING DEFAULT NULL, sourceid TEXT,raw_contact_is_read_only INTEGER NOT NULL DEFAULT 0,version INTEGER NOT NULL DEFAULT 1,dirty INTEGER NOT NULL DEFAULT 0,deleted INTEGER NOT NULL DEFAULT 0,contact_id INTEGER REFERENCES contacts(_id),aggregation_mode INTEGER NOT NULL DEFAULT 0,aggregation_needed INTEGER NOT NULL DEFAULT 1,custom_ringtone TEXT,send_to_voicemail INTEGER NOT NULL DEFAULT 0,times_contacted INTEGER NOT NULL DEFAULT 0,last_time_contacted INTEGER,starred INTEGER NOT NULL DEFAULT 0,display_name TEXT,display_name_alt TEXT,display_name_source INTEGER NOT NULL DEFAULT 0,phonetic_name TEXT,phonetic_name_style TEXT,sort_key TEXT COLLATE PHONEBOOK,sort_key_alt TEXT COLLATE PHONEBOOK,name_verified INTEGER NOT NULL DEFAULT 0,sync1 TEXT, sync2 TEXT, sync3 TEXT, sync4 TEXT )
-- table|stream_items|stream_items|15|CREATE TABLE stream_items (_id INTEGER PRIMARY KEY AUTOINCREMENT, raw_contact_id INTEGER NOT NULL, res_package TEXT, icon TEXT, label TEXT, text TEXT, timestamp INTEGER NOT NULL, comments TEXT, stream_item_sync1 TEXT, stream_item_sync2 TEXT, stream_item_sync3 TEXT, stream_item_sync4 TEXT, FOREIGN KEY(raw_contact_id) REFERENCES raw_contacts(_id))
-- table|stream_item_photos|stream_item_photos|16|CREATE TABLE stream_item_photos (_id INTEGER PRIMARY KEY AUTOINCREMENT, stream_item_id INTEGER NOT NULL, sort_index INTEGER, photo_file_id INTEGER NOT NULL, stream_item_photo_sync1 TEXT, stream_item_photo_sync2 TEXT, stream_item_photo_sync3 TEXT, stream_item_photo_sync4 TEXT, FOREIGN KEY(stream_item_id) REFERENCES stream_items(_id))
-- table|photo_files|photo_files|17|CREATE TABLE photo_files (_id INTEGER PRIMARY KEY AUTOINCREMENT, height INTEGER NOT NULL, width INTEGER NOT NULL, filesize INTEGER NOT NULL)
-- table|packages|packages|18|CREATE TABLE packages (_id INTEGER PRIMARY KEY AUTOINCREMENT,package TEXT NOT NULL)
-- table|mimetypes|mimetypes|19|CREATE TABLE mimetypes (_id INTEGER PRIMARY KEY AUTOINCREMENT,mimetype TEXT NOT NULL)
-- table|data|data|21|CREATE TABLE data (_id INTEGER PRIMARY KEY AUTOINCREMENT,package_id INTEGER REFERENCES package(_id),mimetype_id INTEGER REFERENCES mimetype(_id) NOT NULL,raw_contact_id INTEGER REFERENCES raw_contacts(_id) NOT NULL,is_read_only INTEGER NOT NULL DEFAULT 0,is_primary INTEGER NOT NULL DEFAULT 0,is_super_primary INTEGER NOT NULL DEFAULT 0,data_version INTEGER NOT NULL DEFAULT 0,data1 TEXT,data2 TEXT,data3 TEXT,data4 TEXT,data5 TEXT,data6 TEXT,data7 TEXT,data8 TEXT,data9 TEXT,data10 TEXT,data11 TEXT,data12 TEXT,data13 TEXT,data14 TEXT,data15 TEXT,data_sync1 TEXT, data_sync2 TEXT, data_sync3 TEXT, data_sync4 TEXT )
-- table|phone_lookup|phone_lookup|24|CREATE TABLE phone_lookup (data_id INTEGER REFERENCES data(_id) NOT NULL,raw_contact_id INTEGER REFERENCES raw_contacts(_id) NOT NULL,normalized_number TEXT NOT NULL,min_match TEXT NOT NULL)
-- table|name_lookup|name_lookup|28|CREATE TABLE name_lookup (data_id INTEGER REFERENCES data(_id) NOT NULL,raw_contact_id INTEGER REFERENCES raw_contacts(_id) NOT NULL,normalized_name TEXT NOT NULL,name_type INTEGER NOT NULL,PRIMARY KEY (data_id, normalized_name, name_type))
-- table|nickname_lookup|nickname_lookup|31|CREATE TABLE nickname_lookup (name TEXT,cluster TEXT)
-- table|groups|groups|33|CREATE TABLE groups (_id INTEGER PRIMARY KEY AUTOINCREMENT,package_id INTEGER REFERENCES package(_id),account_name STRING DEFAULT NULL, account_type STRING DEFAULT NULL, data_set STRING DEFAULT NULL, sourceid TEXT,version INTEGER NOT NULL DEFAULT 1,dirty INTEGER NOT NULL DEFAULT 0,title TEXT,title_res INTEGER,notes TEXT,system_id TEXT,deleted INTEGER NOT NULL DEFAULT 0,group_visible INTEGER NOT NULL DEFAULT 0,should_sync INTEGER NOT NULL DEFAULT 1,auto_add INTEGER NOT NULL DEFAULT 0,favorites INTEGER NOT NULL DEFAULT 0,group_is_read_only INTEGER NOT NULL DEFAULT 0,sync1 TEXT, sync2 TEXT, sync3 TEXT, sync4 TEXT )
-- table|agg_exceptions|agg_exceptions|36|CREATE TABLE agg_exceptions (_id INTEGER PRIMARY KEY AUTOINCREMENT,type INTEGER NOT NULL, raw_contact_id1 INTEGER REFERENCES raw_contacts(_id), raw_contact_id2 INTEGER REFERENCES raw_contacts(_id))
-- table|settings|settings|39|CREATE TABLE settings (account_name STRING NOT NULL,account_type STRING NOT NULL,data_set STRING,ungrouped_visible INTEGER NOT NULL DEFAULT 0,should_sync INTEGER NOT NULL DEFAULT 1)
-- table|visible_contacts|visible_contacts|40|CREATE TABLE visible_contacts (_id INTEGER PRIMARY KEY)
-- table|default_directory|default_directory|41|CREATE TABLE default_directory (_id INTEGER PRIMARY KEY)
-- table|calls|calls|42|CREATE TABLE calls (_id INTEGER PRIMARY KEY AUTOINCREMENT,number TEXT,date INTEGER,duration INTEGER,type INTEGER,new INTEGER,name TEXT,numbertype INTEGER,numberlabel TEXT,countryiso TEXT,voicemail_uri TEXT,is_read INTEGER,geocoded_location TEXT,lookup_uri TEXT,matched_number TEXT,normalized_number TEXT,photo_id INTEGER NOT NULL DEFAULT 0,formatted_number TEXT,_data TEXT,has_content INTEGER,mime_type TEXT,source_data TEXT,source_package TEXT,state INTEGER)
-- table|voicemail_status|voicemail_status|43|CREATE TABLE voicemail_status (_id INTEGER PRIMARY KEY AUTOINCREMENT,source_package TEXT UNIQUE NOT NULL,settings_uri TEXT,voicemail_access_uri TEXT,configuration_state INTEGER,data_channel_state INTEGER,notification_channel_state INTEGER)
-- table|activities|activities|45|CREATE TABLE activities (_id INTEGER PRIMARY KEY AUTOINCREMENT,package_id INTEGER REFERENCES package(_id),mimetype_id INTEGER REFERENCES mimetype(_id) NOT NULL,raw_id TEXT,in_reply_to TEXT,author_contact_id INTEGER REFERENCES raw_contacts(_id),target_contact_id INTEGER REFERENCES raw_contacts(_id),published INTEGER NOT NULL,thread_published INTEGER NOT NULL,title TEXT NOT NULL,summary TEXT,link TEXT, thumbnail BLOB)
-- table|status_updates|status_updates|46|CREATE TABLE status_updates (status_update_data_id INTEGER PRIMARY KEY REFERENCES data(_id),status TEXT,status_ts INTEGER,status_res_package TEXT, status_label INTEGER, status_icon INTEGER)
-- table|properties|properties|47|CREATE TABLE properties (property_key TEXT PRIMARY KEY, property_value TEXT )
-- table|accounts|accounts|49|CREATE TABLE accounts (account_name TEXT, account_type TEXT, data_set TEXT)
-- table|directories|directories|50|CREATE TABLE directories(_id INTEGER PRIMARY KEY AUTOINCREMENT,packageName TEXT NOT NULL,authority TEXT NOT NULL,typeResourceId INTEGER,typeResourceName TEXT,accountType TEXT,accountName TEXT,displayName TEXT, exportSupport INTEGER NOT NULL DEFAULT 0,shortcutSupport INTEGER NOT NULL DEFAULT 0,photoSupport INTEGER NOT NULL DEFAULT 0)
-- table|search_index|search_index|0|CREATE VIRTUAL TABLE search_index USING FTS4 (contact_id INTEGER REFERENCES contacts(_id) NOT NULL,content TEXT, name TEXT, tokens TEXT)
-- table|search_index_content|search_index_content|51|CREATE TABLE 'search_index_content'(docid INTEGER PRIMARY KEY, 'c0contact_id', 'c1content', 'c2name', 'c3tokens')
-- table|search_index_segments|search_index_segments|52|CREATE TABLE 'search_index_segments'(blockid INTEGER PRIMARY KEY, block BLOB)
-- table|search_index_segdir|search_index_segdir|53|CREATE TABLE 'search_index_segdir'(level INTEGER,idx INTEGER,start_block INTEGER,leaves_end_block INTEGER,end_block INTEGER,root BLOB,PRIMARY KEY(level, idx))
-- table|search_index_docsize|search_index_docsize|55|CREATE TABLE 'search_index_docsize'(docid INTEGER PRIMARY KEY, size BLOB)
-- table|search_index_stat|search_index_stat|56|CREATE TABLE 'search_index_stat'(id INTEGER PRIMARY KEY, value BLOB)
-- table|data_usage_stat|data_usage_stat|57|CREATE TABLE data_usage_stat(stat_id INTEGER PRIMARY KEY AUTOINCREMENT, data_id INTEGER NOT NULL, usage_type INTEGER NOT NULL DEFAULT 0, times_used INTEGER NOT NULL DEFAULT 0, last_time_used INTERGER NOT NULL DEFAULT 0, FOREIGN KEY(data_id) REFERENCES data(_id))
-- table|v1_settings|v1_settings|62|CREATE TABLE v1_settings (_id INTEGER PRIMARY KEY,_sync_account TEXT,_sync_account_type TEXT,key STRING NOT NULL,value STRING )
-- table|sqlite_stat1|sqlite_stat1|63|CREATE TABLE sqlite_stat1(tbl,idx,stat)


leave;


