set echo off;
-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
set message off;
set sqlite on;

-- reset sequence for profile 
alter sequence raw_contacts_id_seq restart with 1;
alter sequence data_id_seq restart with 1;
alter sequence contacts_id_seq restart with 1;
alter sequence calls_id_seq restart with 1;

insert into "_sync_state" ("_id", account_name, account_type, data) values (1, 'mimer.test@gmail.com', 'com.google', x'');
insert into "_sync_state" ("_id", account_name, account_type, data) values (2, 'mimer.test1@gmail.com', 'com.google', x'');
insert into "_sync_state" ("_id", account_name, account_type, data) values (3, 'mimer.test2@gmail.com', 'com.google', x'');
insert into "_sync_state" ("_id", account_name, account_type, data) values (4, 'mimer.test3@gmail.com', 'com.google', x'');


insert into agg_exceptions ("_id", type, raw_contact_id1, raw_contact_id2) values (1, 1, 150, 153);
insert into agg_exceptions ("_id", type, raw_contact_id1, raw_contact_id2) values (2, 1, 116, 154);
insert into agg_exceptions ("_id", type, raw_contact_id1, raw_contact_id2) values (3, 1, 144, 155);
insert into agg_exceptions ("_id", type, raw_contact_id1, raw_contact_id2) values (4, 1, 145, 156);
insert into agg_exceptions ("_id", type, raw_contact_id1, raw_contact_id2) values (5, 1, 115, 157);
insert into agg_exceptions ("_id", type, raw_contact_id1, raw_contact_id2) values (6, 1, 119, 158);
insert into agg_exceptions ("_id", type, raw_contact_id1, raw_contact_id2) values (7, 1, 111, 159);
insert into agg_exceptions ("_id", type, raw_contact_id1, raw_contact_id2) values (8, 1, 117, 160);
insert into agg_exceptions ("_id", type, raw_contact_id1, raw_contact_id2) values (9, 1, 110, 161);
insert into agg_exceptions ("_id", type, raw_contact_id1, raw_contact_id2) values (10, 1, 109, 162);
insert into agg_exceptions ("_id", type, raw_contact_id1, raw_contact_id2) values (11, 1, 113, 163);
insert into agg_exceptions ("_id", type, raw_contact_id1, raw_contact_id2) values (12, 1, 131, 164);
insert into agg_exceptions ("_id", type, raw_contact_id1, raw_contact_id2) values (13, 1, 114, 165);
insert into agg_exceptions ("_id", type, raw_contact_id1, raw_contact_id2) values (14, 1, 133, 166);
insert into agg_exceptions ("_id", type, raw_contact_id1, raw_contact_id2) values (15, 1, 112, 167);
insert into agg_exceptions ("_id", type, raw_contact_id1, raw_contact_id2) values (16, 1, 151, 168);


insert into data_usage_stat (stat_id, data_id, usage_type, times_used, last_time_used) values (1, 1183, 0, 7, 1360848842910);
insert into data_usage_stat (stat_id, data_id, usage_type, times_used, last_time_used) values (2001, 1162, 0, 3, 1360771330732);
insert into data_usage_stat (stat_id, data_id, usage_type, times_used, last_time_used) values (2002, 774, 1, 1, 1360744984432);
insert into data_usage_stat (stat_id, data_id, usage_type, times_used, last_time_used) values (2003, 1156, 1, 1, 1360744984432);
insert into data_usage_stat (stat_id, data_id, usage_type, times_used, last_time_used) values (2004, 1225, 0, 1, 1360763967582);
insert into data_usage_stat (stat_id, data_id, usage_type, times_used, last_time_used) values (2005, 1088, 0, 1, 1360764521762);
insert into data_usage_stat (stat_id, data_id, usage_type, times_used, last_time_used) values (2006, 918, 0, 1, 1360773865597);
insert into data_usage_stat (stat_id, data_id, usage_type, times_used, last_time_used) values (2007, 4002, 0, 1, 1360775452880);
insert into data_usage_stat (stat_id, data_id, usage_type, times_used, last_time_used) values (2008, 1200, 0, 1, 1360849352152);

insert into packages("_id", package) values(1, 'xxx');
insert into packages("_id", package) values(2, 'yyy');
insert into packages("_id", package) values(3, 'zzz');
insert into packages("_id", package) values(4, 'rrr');

INSERT INTO settings(account_name, account_type,data_set,ungrouped_visible,should_sync)VALUES('mimer.test@gmail.com','google.com','',0,0);
INSERT INTO settings(account_name, account_type,data_set,ungrouped_visible,should_sync)VALUES('mimer.test2@gmail.com','google.com','',0,0);
INSERT INTO settings(account_name, account_type,data_set,ungrouped_visible,should_sync)VALUES('mimer.test3@gmail.com','google.com','',0,0);
INSERT INTO settings(account_name, account_type,data_set,ungrouped_visible,should_sync)VALUES('mimer.test4@gmail.com','google.com','',0,0);

insert into photo_files ("_id", height, width, filesize) values (1, 452, 452, 39343);
insert into photo_files ("_id", height, width, filesize) values (2, 402, 402, 39500);
insert into photo_files ("_id", height, width, filesize) values (3, 400, 400, 17388);
insert into photo_files ("_id", height, width, filesize) values (4, 452, 452, 51423);
insert into photo_files ("_id", height, width, filesize) values (5, 540, 540, 30782);
insert into photo_files ("_id", height, width, filesize) values (6, 696, 696, 94690);
insert into photo_files ("_id", height, width, filesize) values (7, 500, 500, 24862);
insert into photo_files ("_id", height, width, filesize) values (8, 480, 480, 24207);
insert into photo_files ("_id", height, width, filesize) values (9, 384, 384, 33662);
insert into photo_files ("_id", height, width, filesize) values (10, 180, 180, 8489);
insert into photo_files ("_id", height, width, filesize) values (11, 540, 540, 19416);
insert into photo_files ("_id", height, width, filesize) values (12, 510, 510, 31347);
insert into photo_files ("_id", height, width, filesize) values (13, 452, 452, 25500);
insert into photo_files ("_id", height, width, filesize) values (14, 540, 540, 51135);
insert into photo_files ("_id", height, width, filesize) values (15, 192, 192, 8388);
insert into photo_files ("_id", height, width, filesize) values (16, 480, 480, 41221);
insert into photo_files ("_id", height, width, filesize) values (1001, 537, 537, 29713);
insert into photo_files ("_id", height, width, filesize) values (1002, 480, 480, 41231);

insert into status_updates (status_update_data_id, status, status_ts, status_res_package, status_label, status_icon) values (351, '@E1-01D, 847-523-1059', null, null, 17040102, null);
insert into status_updates (status_update_data_id, status, status_ts, status_res_package, status_label, status_icon) values (352, '@E1-01F, 847-523-1059', null, null, 17040103, null);
insert into status_updates (status_update_data_id, status, status_ts, status_res_package, status_label, status_icon) values (353, '@E1-01E, 847-523-1059', null, null, 17040103, null);
insert into status_updates (status_update_data_id, status, status_ts, status_res_package, status_label, status_icon) values (354, '@E1-01C, 847-523-1059', null, null, 17040103, null);

insert into stream_item_photos ("_id", stream_item_id, sort_index, photo_file_id, stream_item_photo_sync1, stream_item_photo_sync2, stream_item_photo_sync3, stream_item_photo_sync4) values (1, 3008, 1, 1001, 'fbphoto', '1015117105311173', null, null);
insert into stream_item_photos ("_id", stream_item_id, sort_index, photo_file_id, stream_item_photo_sync1, stream_item_photo_sync2, stream_item_photo_sync3, stream_item_photo_sync4) values (2, 3009, 1, 1001, 'fbphoto', '10151171053911174', null, null);
insert into stream_item_photos ("_id", stream_item_id, sort_index, photo_file_id, stream_item_photo_sync1, stream_item_photo_sync2, stream_item_photo_sync3, stream_item_photo_sync4) values (3, 30010, 1, 1001, 'fbphoto', '10151171053911175', null, null);
insert into stream_item_photos ("_id", stream_item_id, sort_index, photo_file_id, stream_item_photo_sync1, stream_item_photo_sync2, stream_item_photo_sync3, stream_item_photo_sync4) values (4, 30011, 1, 1001, 'fbphoto', '10151171053911176', null, null);

insert into stream_items ("_id", raw_contact_id, res_package, icon, label, text, timestamp, comments, stream_item_sync1, stream_item_sync2, stream_item_sync3, stream_item_sync4) values (1, 48, null, null, 'imProtocolGoogleTalk', '@E1-01D, 333-33-33', 3333, '', null, null, null, null);
insert into stream_items ("_id", raw_contact_id, res_package, icon, label, text, timestamp, comments, stream_item_sync1, stream_item_sync2, stream_item_sync3, stream_item_sync4) values (3004, 165, 'org.mots.haxsync', null, '33333', 'Jaaaaaaaaaaaaaaaaa', 1335038248000, '<img src="res://org.mots.haxsync/3333"/> 1&nbsp;<img src="res://org.mots.haxsync/123"/> 2', '698911172_33333', '698911172', 'http://www.facebook.com/xxxx/posts/dfdfdf', null);
insert into stream_items ("_id", raw_contact_id, res_package, icon, label, text, timestamp, comments, stream_item_sync1, stream_item_sync2, stream_item_sync3, stream_item_sync4) values (3005, 165, 'org.mots.haxsync', null, '2131099794', 'Oj', 144964000, '<img src="res://org.mots.haxsync/2130837538"/> 1', '698911172_1015dfdfdf6173', '64411172', 'http://www.facebook.com/xxx/posts/10154444467756173', null);
insert into stream_items ("_id", raw_contact_id, res_package, icon, label, text, timestamp, comments, stream_item_sync1, stream_item_sync2, stream_item_sync3, stream_item_sync4) values (3006, 165, 'org.mots.haxsync', null, '2131099794', 'Safnta d', 1340716350000, '<img src="res://org.mots.haxsync/2133337547"/> 2', '698911172_28333244998104', '633172', 'http://www.facebook.com/xxx/posts/28433333998104', null);
insert into stream_items ("_id", raw_contact_id, res_package, icon, label, text, timestamp, comments, stream_item_sync1, stream_item_sync2, stream_item_sync3, stream_item_sync4) values (3007, 165, 'org.mots.haxsync', null, '2131099794', 'ddf ff, RF', 222, '<img src="res://org.mots.haxsync/2130837538"/> 5', '22210150936633516173', '698911172', 'http://www.facebook.com/xxx/posts/sss', null);
insert into stream_items ("_id", raw_contact_id, res_package, icon, label, text, timestamp, comments, stream_item_sync1, stream_item_sync2, stream_item_sync3, stream_item_sync4) values (3008, 165, 'org.mots.haxsync', null, '2131099794', '8 meters "granen"', 1354564246000, '<img src="res://org.mots.haxsync/4444"/> 2&nbsp;<img src="res://org.mots.haxsync/33333"/> 6', '698911172_33333', '698911172', 'http://www.facebook.com/photo.php?fbid=s342334&set=a.xxx.193783.dd&type=1', null);
insert into stream_items ("_id", raw_contact_id, res_package, icon, label, text, timestamp, comments, stream_item_sync1, stream_item_sync2, stream_item_sync3, stream_item_sync4) values (3009, 155, 'org.mots.haxsync', null, '2131099794', 'asdfynfi fnjh ue', 1279809053000, null, '720218927_122531751226286', '720218927', 'http://www.facebook.com/yyy/posts/11', null);
insert into stream_items ("_id", raw_contact_id, res_package, icon, label, text, timestamp, comments, stream_item_sync1, stream_item_sync2, stream_item_sync3, stream_item_sync4) values (3010, 155, 'org.mots.haxsync', null, '234', 'Tack för alla gratulationer', 1302435809200, '<img src="res://org.mots.haxsync/2rc333"/> 1&nbsp;<img src="res://org.mots.haxsync/222"/> 2', '720218927_56', '720218927', 'http://www.facebook.com/yyy/posts/10150161370228928', null);


INSERT INTO voicemail_status("_id",source_package,settings_uri,voicemail_access_uri,configuration_state,data_channel_state,notification_channel_state)
VALUES (0,'aaa','file://xdfaddsfsdf','http://xdfasdfsdf',1,2,3);

INSERT INTO voicemail_status("_id",source_package,settings_uri,voicemail_access_uri,configuration_state,data_channel_state,notification_channel_state)
VALUES (1,'bbb','file://shadf','http://xdfas22sdf1',2,2,3);

INSERT INTO voicemail_status("_id",source_package,settings_uri,voicemail_access_uri,configuration_state,data_channel_state,notification_channel_state)
VALUES (2,'ccc','file://xdfsdrsfsdf','http://xdfa33sdfsdf1',1,5,3);

INSERT INTO voicemail_status("_id",source_package,settings_uri,voicemail_access_uri,configuration_state,data_channel_state,notification_channel_state)
VALUES (3,'ddd','file://xdfafffsffsdf','http://xdfasxxxxdfsdf3',3,4,3);

INSERT INTO v1_settings ("_id","_sync_account","_sync_account_type","key","value")
VALUES(0,'mimer.test@gmail.com','com.google','113','aac');

INSERT INTO v1_settings ("_id","_sync_account","_sync_account_type","key","value")
VALUES(1,'mimer.test1@gmail.com','com.google','223','axbc');

INSERT INTO v1_settings ("_id","_sync_account","_sync_account_type","key","value")
VALUES(2,'mimer.test2@gmail.com','com.google','133','ccc');

INSERT INTO v1_settings ("_id","_sync_account","_sync_account_type","key","value")
VALUES(3,'mimer.test3@gmail.com','com.google','1234','axc');



delete from mimetypes;
insert into mimetypes ("_id", mimetype) values (1, 'vnd.android.cursor.item/email_v2');
insert into mimetypes ("_id", mimetype) values (2, 'vnd.android.cursor.item/im');
insert into mimetypes ("_id", mimetype) values (3, 'vnd.android.cursor.item/nickname');
insert into mimetypes ("_id", mimetype) values (4, 'vnd.android.cursor.item/organization');
insert into mimetypes ("_id", mimetype) values (5, 'vnd.android.cursor.item/phone_v2');
insert into mimetypes ("_id", mimetype) values (6, 'vnd.android.cursor.item/sip_address');
insert into mimetypes ("_id", mimetype) values (7, 'vnd.android.cursor.item/name');
insert into mimetypes ("_id", mimetype) values (8, 'vnd.android.cursor.item/postal-address_v2');
insert into mimetypes ("_id", mimetype) values (9, 'vnd.android.cursor.item/identity');
insert into mimetypes ("_id", mimetype) values (10, 'vnd.android.cursor.item/photo');
insert into mimetypes ("_id", mimetype) values (11, 'vnd.android.cursor.item/group_membership');
alter sequence mimetypes_id_seq restart with 12;


--Contacts
@
BEGIN
DECLARE CNT INTEGER;
DECLARE NUM_CONTACTS INTEGER;
DECLARE n_photo_id INTEGER;
DECLARE n_photo_file_id INTEGER;
DECLARE n_custom_ringtone INTEGER;
DECLARE n_status_update_id INTEGER;
DECLARE n_last_contacted INTEGER;
DECLARE n_times_contacted INTEGER;
DECLARE n_starred INTEGER;
DECLARE n_has_phone_number INTEGER;
DECLARE n_voice_mail INTEGER;
DECLARE n_contact_id BIGINT;
DECLARE n_raw_contact_id BIGINT;
DECLARE n_dirty_agg INTEGER;
DECLARE n_source_id varchar(20);
DECLARE n_sync1 varchar(100);
DECLARE n_sync2 varchar(100);
DECLARE n_sync3 varchar(100);
DECLARE n_sync4 varchar(100);
DECLARE fname_base varchar(10);
DECLARE lname_base varchar(10);

DECLARE n_lname varchar(100);
DECLARE n_fname varchar(100);

DECLARE n_raw_read_only integer;
DECLARE n_deleted integer;
declare visible integer;
DECLARE n_data_id BIGINT;
DECLARE n_verified integer;
DECLARE n_primary boolean;
DECLARE n_super_primary boolean;
DECLARE n_data_read_only boolean;
DECLARE DATA_PER_CONTACT integer;
DECLARE DATA_CNT INTEGER;
DECLARE n_data1 varchar(100);
DECLARE n_data2 varchar(100);
DECLARE n_data3 varchar(100);
DECLARE n_data4 varchar(100);
DECLARE n_data5 varchar(100);
DECLARE n_data6 varchar(100);
DECLARE n_data7 varchar(100);
DECLARE n_data8 varchar(100);
DECLARE n_data9 varchar(100);
DECLARE n_data10 varchar(100);
DECLARE n_data11 varchar(100);
DECLARE n_data12 varchar(100);
DECLARE n_data13 varchar(100);
DECLARE n_data14 varchar(100);
DECLARE n_data15 BLOB;
DECLARE n_mimetype_id integer;
DECLARE n_package_id integer;

 SET CNT = 1;
 SET NUM_CONTACTS = 7000;
 SET DATA_PER_CONTACT = 5;

 set fname_base = 'Fredrik';
 set lname_base = 'Johson';

 WHILE CNT <= NUM_CONTACTS DO
 
    SET n_contact_id = next_value of contacts_id_seq;
    --Each 10 contacts have a photo
    if mod(cnt, 10) = 0 then
        set n_photo_id = CNT;
        set n_photo_file_id = CNT;
    else
        set n_photo_id = null;
        set n_photo_file_id = null;
    end if;
    
    --Each 20 contacts have a custom ringtome
    if mod(cnt, 20) = 0 then
        set n_custom_ringtone = MOD(IRAND(), 100);
    else
        set n_custom_ringtone = null;
    end if;
        
    --Each 30 contact have a status update
    if mod(cnt, 30) = 0 then
        set n_status_update_id = CNT;
    else
        set n_status_update_id = null;
    end if;    
    
    
    --Each 30 contacts have been contacted
    if mod(cnt, 30) = 0 then
        set n_last_contacted = MOD(IRAND(), 1000000);
        set n_times_contacted = MOD(IRAND(), 10) + 1;
    else
        set n_last_contacted = 0;
        set n_times_contacted = 0;
    end if;
    
    --Each 100 contacts doesn't have a phone number
    if mod(cnt, 100) = 0 then
        set n_has_phone_number = 0;
    else
        set n_has_phone_number = 1;
    end if;   
    
    --Each 50 contacts are starred.
    if mod(cnt, 50) = 0 then
        set n_starred = 1;
    else
        set n_starred = 0;
    end if;
    
    
    --Each 100 contacts are sent to voice mail
    if mod(cnt, 50) = 0 then
        set n_voice_mail = 1;
    else
        set n_voice_mail = 0;
    end if;     
   
   
   --Each 100 contacs are dirty and needs aggregation
    if mod(cnt, 100) = 0 then
        set n_dirty_agg = 1;
    else
        set n_dirty_agg = 0;
    end if;      
    
    set n_raw_contact_id = next_value of raw_contacts_id_seq;
    --CONTACTS
    insert into contacts ("_id", name_raw_contact_id, photo_id, photo_file_id, custom_ringtone, send_to_voicemail, times_contacted, last_time_contacted, starred, has_phone_number, lookup, status_update_id) values (n_contact_id, n_raw_contact_id, n_photo_id, n_photo_file_id, n_custom_ringtone, n_voice_mail, n_times_contacted, n_last_contacted, n_starred, n_has_phone_number, cast(CNT as varchar(6)) || '0r1-273B2F' || cast(CNT as varchar(6)), n_status_update_id);    
    
    --RAW_CONTACTS
    set n_data_id = next_value of data_id_seq;
    set n_lname =  ASCII_CHAR(MOD(IRAND(), 25) + 65) || lname_base;   
    set n_fname =  ASCII_CHAR(MOD(IRAND(), 25) + 65) || fname_base;
    set n_source_id = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 97)||cast(MOD(IRAND(), 10) as varchar(2)), MOD(IRAND(), 5)+1);
    set n_sync1 = 'https://www.google.com/xxx/' || cast(n_source_id as varchar(20));
    set n_sync2 = substring(n_fname from 1 for 1) || '.fljadshf';
    set n_sync3 = '2013-09-09T10:09:13.' || cast(n_raw_contact_id as varchar(20));
    set n_sync4 = 'x.' || cast(n_raw_contact_id as varchar(20));
    

   --Each 100 raw_contacts are read only and deleted
    if mod(cnt, 100) = 0 then
        set n_raw_read_only = 1;
        set n_deleted = 1;
    else
        set n_raw_read_only = 0;
        set n_deleted = 0;
    end if;     
    
    --Each 10 raw_contacts have been verified
    if mod(cnt, 10) = 0 then
        set n_verified = 1;
    else
        set n_verified = 0;
    end if;
    
    insert into raw_contacts ("_id", account_id,  sourceid, raw_contact_is_read_only, version, dirty, deleted, contact_id, aggregation_mode, aggregation_needed, custom_ringtone, send_to_voicemail, times_contacted, last_time_contacted, starred, display_name, display_name_alt, display_name_source, phonetic_name, phonetic_name_style, sort_key, sort_key_alt, name_verified, sync1, sync2, sync3, sync4) 
    values (n_raw_contact_id, MOD(IRAND(), 7), n_source_id, n_raw_read_only, MOD(IRAND(), 7), n_dirty_agg, n_deleted, n_contact_id, MOD(IRAND(), 3), n_dirty_agg, n_custom_ringtone, n_voice_mail, n_times_contacted, n_last_contacted, n_starred, n_fname || ' ' || n_lname, n_lname || ', ' || n_fname, n_data_id, n_fname || ' ' || n_lname, MOD(IRAND(), 3), n_fname || ' ' || n_lname, n_lname || ', ' || n_fname, n_verified, n_sync1, n_sync2, n_sync3, n_sync4);
    
    --Each 5 contacts have duplicate raw_contacts
    if mod(cnt, 5) = 0 then
        set n_data_id = next_value of data_id_seq;
        set n_raw_contact_id = next_value of raw_contacts_id_seq;
        set n_lname =  ASCII_CHAR(MOD(IRAND(), 25) + 65) || lname_base;   
        set n_fname =  ASCII_CHAR(MOD(IRAND(), 25) + 65) || fname_base;
    
        insert into raw_contacts ("_id", account_id,  sourceid, raw_contact_is_read_only, version, dirty, deleted, contact_id, aggregation_mode, aggregation_needed, custom_ringtone, send_to_voicemail, times_contacted, last_time_contacted, starred, display_name, display_name_alt, display_name_source, phonetic_name, phonetic_name_style, sort_key, sort_key_alt, name_verified, sync1, sync2, sync3, sync4) 
    values (n_raw_contact_id, MOD(IRAND(), 7), n_source_id, 0, MOD(IRAND(), 7), n_dirty_agg, 0, n_contact_id, MOD(IRAND(), 3), n_dirty_agg, n_custom_ringtone, n_voice_mail, n_times_contacted, n_last_contacted, n_starred, n_fname || ' ' || n_lname, n_lname || ', ' || n_fname, n_data_id, n_fname || ' ' || n_lname, MOD(IRAND(), 3), n_fname || ' ' || n_lname, n_lname || ', ' || n_fname, 0, n_sync1, n_sync2, n_sync3, n_sync4);
    end if;
    

   --Each 100 contacts are not visible (0% in earlier stats)
   if mod(cnt,100) <> 0 then 
       insert into visible_contacts values(CNT);
   end if;
   --Each 200 contacts are not visible (0% in earlier stats)
   if mod(cnt, 200) <> 0 then 
       insert into default_directory values(CNT);
   end if;

    --DATA

    set DATA_CNT = 1;
    WHILE DATA_CNT <= DATA_PER_CONTACT DO
	    set n_data_id = next_value of data_id_seq;
	    if MOD(IRAND(), 7) = 0 then    
		set n_primary = false;
	    else
		set n_primary = true;
	    end if;
	    
	    if MOD(IRAND(), 7) = 0 then    
		set  n_super_primary = true;
	    else
		set  n_super_primary = false;
	    end if;
	    
	    if MOD(IRAND(), 50) = 0 then    
		set  n_data_read_only = true;
		set n_package_id = MOD(IRAND(), 50);
	    else
		set n_data_read_only = false;
		set n_package_id = null;
	    end if;
	    
	    set n_data1 = null;
	    set n_data2 = null;
	    set n_data3 = null;
	    set n_data4 = null;
	    set n_data5 = null;
	    set n_data6 = null;
	    set n_data7 = null;
	    set n_data8 = null;
	    set n_data9 = null;
	    set n_data10 = null;
	    set n_data11 = null;
	    set n_data12 = null;
	    set n_data13 = null;
	    set n_data14 = null;
	    set n_data15 = null;
	       
	    if data_cnt = 1 then --name
	       set n_data1 = n_fname || ' ' || n_lname;
	       set n_data2 = n_fname;
	       set n_data3 = n_lname;
	       set n_data10 = '0';
	       set n_data11 = '1';
	       set n_mimetype_id = 1;
	    elseif data_cnt = 2 then --address
	       set n_data1 = ASCII_CHAR(MOD(IRAND(), 25) + 65) || 'gatan 1 Uppsala 751 ' || cast(MOD(IRAND(), 27) as varchar(10));
	       set n_data4 = ASCII_CHAR(MOD(IRAND(), 25) + 65) || 'gatan 1';
	       set n_data7 = ASCII_CHAR(MOD(IRAND(), 25) + 65) || 'sala';
	       set n_data9 = cast(MOD(IRAND(), 10000) as varchar(10));
	       set n_mimetype_id = 2;
	    elseif data_cnt = 3 then -- phone
	       set n_data1 = cast(MOD(IRAND(), 100000) as varchar(10));
	       set n_data2 = cast(MOD(IRAND(), 5) as varchar(10));
	       set n_mimetype_id = 3;
	    elseif data_cnt = 4 then -- email
	       set n_data1 = n_fname || '.' || n_lname || '@abc.se';
	       set n_data2 = cast(MOD(IRAND(), 5) as varchar(10));
	       set n_data4 = n_fname || ' ' || n_lname;
	       set n_mimetype_id = 4;
	    else -- Random, for example photo (data15)
	       if MOD(IRAND(), 50) = 0 then --photo 
	           --set n_data15 = cast(cast(MOD(IRAND(), 1000000) as varchar(20)) as varbinary(20));
	           set n_mimetype_id = 5;
	       elseif MOD(IRAND(), 50) = 0 then
	           set n_data8= cast(MOD(IRAND(), 5) as varchar(10));
	           set n_data5= cast(MOD(IRAND(), 5) as varchar(10));
	           set n_mimetype_id = MOD(IRAND(), 10) + 5;
	       elseif MOD(IRAND(), 50) = 0 then
	           set n_data6= cast(MOD(IRAND(), 5) as varchar(10));
	           set n_data10= cast(MOD(IRAND(), 5) as varchar(10));
	           set n_data11= cast(MOD(IRAND(), 5) as varchar(10));
	           set n_data12= cast(MOD(IRAND(), 5) as varchar(10));
	           set n_data13= cast(MOD(IRAND(), 5) as varchar(10));
	           set n_data14= cast(MOD(IRAND(), 5) as varchar(10));
	           set n_mimetype_id = MOD(IRAND(), 10) + 5;
	       elseif MOD(IRAND(), 2) = 0 then --Extra phone
	          set n_data1 = cast(MOD(IRAND(), 100000) as varchar(10));
	          set n_data2 = cast(MOD(IRAND(), 5) as varchar(10));
	          set n_mimetype_id = 4;
	       else --Extra email
	          set n_data1 = n_fname || '.' || n_lname || '@abc.se';
	          set n_data2 = cast(MOD(IRAND(), 5) as varchar(10));
	          set n_data4 = n_fname || ' ' || n_lname;
	          set n_mimetype_id = 3;
	       end if;
	    end if;
	    
	    --Name
	    insert into data ("_id", package_id, mimetype_id, raw_contact_id, is_read_only, is_primary, is_super_primary, data_version, data1, data2, data3, data4, data5, data6, data7, data8, data9, data10, data11, data12, data13, data14, data15, data_sync1, data_sync2, data_sync3, data_sync4) values (n_data_id, n_package_id, n_mimetype_id, n_raw_contact_id, n_data_read_only, n_primary, n_super_primary, MOD(IRAND(), 7), n_data1, n_data2, n_data3,  n_data4,  n_data5,  n_data6,  n_data7,  n_data8,  n_data9,  n_data10,  n_data11,  n_data12,  n_data13,  n_data14,  n_data15, n_sync1, n_sync2, n_sync3, n_sync4);
	   
	    set DATA_CNT = DATA_CNT + 1;
    END WHILE;
    
    
    --NAME_LOOKUP
    insert into name_lookup (data_id, raw_contact_id, normalized_name, name_type) values (n_data_id, n_raw_contact_id, cast(MOD(IRAND(), 1000000) as varchar(20)) || '.' || cast(MOD(IRAND(), 1000000) as varchar(20)), MOD(IRAND(), 3));
    insert into name_lookup (data_id, raw_contact_id, normalized_name, name_type) values (n_data_id, n_raw_contact_id, cast(MOD(IRAND(), 1000000) as varchar(20)) || '.' || cast(MOD(IRAND(), 1000000) as varchar(20)), MOD(IRAND(), 3) + 6);
    insert into name_lookup (data_id, raw_contact_id, normalized_name, name_type) values (n_data_id, n_raw_contact_id, cast(MOD(IRAND(), 1000000) as varchar(20)) || '.' || cast(MOD(IRAND(), 1000000) as varchar(20)), MOD(IRAND(), 3) + 12);
    
    --PHONE_LOOKUP
    insert into phone_lookup (data_id, raw_contact_id, normalized_number, min_match) values (n_data_id, n_raw_contact_id, cast(MOD(IRAND(), 1000000) as varchar(20)), cast(MOD(IRAND(), 1000000) as varchar(20)));
    if MOD(IRAND(), 2) = 0 then
        set n_data_id = next_value of data_id_seq;
        insert into phone_lookup (data_id, raw_contact_id, normalized_number, min_match) values (n_data_id, n_raw_contact_id, cast(MOD(IRAND(), 1000000) as varchar(20)), cast(MOD(IRAND(), 1000000) as varchar(20)));
    end if;

    --SEARCH_INDEX
    insert into search_index (contact_id, content, name, tokens) values (n_contact_id, ASCII_CHAR(MOD(IRAND(), 25) + 65) || 'gatan 1 Uppsala 751 ' || cast(MOD(IRAND(), 27) as varchar(10)), cast(MOD(IRAND(), 1000000) as varchar(20)) || ' ' || cast(MOD(IRAND(), 1000000) as varchar(20)), cast(MOD(IRAND(), 1000000) as varchar(20)));
    SET CNT = CNT + 1;
    
 END WHILE;

END
@

----
----


delete from properties;
insert into properties (property_key, property_value) values ('aggregation_v2', '2');
insert into properties (property_key, property_value) values ('contacts_imported_v1', '1');
insert into properties (property_key, property_value) values ('directoryScanComplete', '1');
insert into properties (property_key, property_value) values ('search_index', '1');

@
begin
   declare call_cnt integer;
   declare n_call_id integer;
   declare n_number varchar(20);
   declare n_name varchar(20);
   declare n_date varchar(20);
   declare n_voicemail_uri varchar(100);
   declare n_lookup_uri varchar(100);
   declare n_photo_id integer;
   declare n_geocoded_location varchar(10);
   declare n_is_read integer;
   
   set call_cnt = 0;
   set n_number = cast(MOD(IRAND(), 1000000) as varchar(20));
   set n_name = cast(MOD(IRAND(), 1000000) as varchar(20));
   set n_voicemail_uri = null;
   set n_geocoded_location = 'Sverige';
   set n_lookup_uri  = 'content://com.android.contacts/contacts/lookup/746i28abcb4b091a8698';
   set n_photo_id = 0;
   
   WHILE call_cnt <= 300 DO
      set n_call_id = next_value of calls_id_seq;
      set n_date = cast(MOD(IRAND(), 1000000) as varchar(20));
      if MOD(IRAND(), 30) = 0 then 
      	set n_is_read = 1;
      else
        set n_is_read = 0;
      end if;
      if MOD(IRAND(), 8) = 0 then
         set n_number = cast(MOD(IRAND(), 1000000) as varchar(20));
         set n_name = cast(MOD(IRAND(), 1000000) as varchar(20));
         if MOD(IRAND(), 10) = 0 then
            set n_voicemail_uri = 'content://com.android.contacts/contacts/voice/' || cast(n_call_id as varchar(20));
         else
            set n_voicemail_uri = null;
         end if;
         set n_geocoded_location = ASCII_CHAR(MOD(IRAND(), 5) + 65)||ASCII_CHAR(MOD(IRAND(), 5) + 65) || 'land';
         set n_lookup_uri  = 'content://com.android.contacts/contacts/lookup/' || cast(n_call_id as varchar(20));
         if MOD(IRAND(), 10) = 0 then
            set n_photo_id = 0;      
         else
            set n_photo_id = n_call_id;
         end if;
      end if;
      insert into calls ("_id",number, date, duration, type, "new", name, numbertype, numberlabel, countryiso, voicemail_uri, is_read, geocoded_location, lookup_uri, matched_number, normalized_number, photo_id, formatted_number, "_data", has_content, mime_type, source_data, source_package, state) values (n_call_id, n_number, n_date, MOD(IRAND(), 100), MOD(IRAND(), 5), MOD(IRAND(), 5), n_name, MOD(IRAND(), 5), null, ASCII_CHAR(MOD(IRAND(), 5) + 65)||ASCII_CHAR(MOD(IRAND(), 5) + 65), n_voicemail_uri, n_is_read, n_geocoded_location, n_lookup_uri, n_number, n_number, n_photo_id, n_number, null, null, null, null, null, null);
      
      set call_cnt = call_cnt + 1;
   END WHILE;
end
@

delete from accounts;
insert into accounts ("_id", account_name, account_type, data_set) values (1, 'mimer1', 'com.google', null);
insert into accounts ("_id", account_name, account_type, data_set) values (2, 'mimer2', 'com.mimer', null);
insert into accounts ("_id", account_name, account_type, data_set) values (3, 'mimer3', 'com.google', null);
insert into accounts ("_id", account_name, account_type, data_set) values (4, 'mimer4', 'com.google', null);
insert into accounts ("_id", account_name, account_type, data_set) values (5, 'mimer1', 'com.xxx', null);

delete from groups;
insert into groups ("_id", package_id, account_id, sourceid, version, dirty, title, title_res, notes, system_id, deleted, group_visible, should_sync, auto_add, favorites, group_is_read_only, sync1, sync2, sync3, sync4) values (1001, null, 1, '6', 16, 0, 'My Contacts', null, 'System Group: My Contacts', 'Contacts', 0, 1, 1, 1, 0, 1, null, '"YDwteyI."', '1970-01-01T00:00:00.000Z', null);
insert into groups ("_id", package_id, account_id, sourceid, version, dirty, title, title_res, notes, system_id, deleted, group_visible, should_sync, auto_add, favorites, group_is_read_only, sync1, sync2, sync3, sync4) values (1002, null, 1, '7a8a2aa70f963122', 2, 0, 'Starred in Android', null, 'Starred in Android', null, 0, 1, 1, 0, 1, 1, 'https://www.google.com/m8/feeds/groups/mimer.test%40gmail.com/base2_property-android/7a8a2aa70f963122', '"QX0-fTVSLy17ImA9WxFUF0wIQQY."', '2010-06-28T08:10:20.355Z', null);
insert into groups ("_id", package_id, account_id, sourceid, version, dirty, title, title_res, notes, system_id, deleted, group_visible, should_sync, auto_add, favorites, group_is_read_only, sync1, sync2, sync3, sync4) values (1003, null, 1, 'd', 1, 0, 'Friends', null, 'System Group: Friends', 'Friends', 0, 0, 1, 0, 0, 1, null, '"YDwteyI."', '1970-01-01T00:00:00.000Z', null);
insert into groups ("_id", package_id, account_id, sourceid, version, dirty, title, title_res, notes, system_id, deleted, group_visible, should_sync, auto_add, favorites, group_is_read_only, sync1, sync2, sync3, sync4) values (1004, null, 1, 'e', 1, 0, 'Family', null, 'System Group: Family', 'Family', 0, 0, 1, 0, 0, 1, null, '"YDwteyI."', '1970-01-01T00:00:00.000Z', null);
insert into groups ("_id", package_id, account_id, sourceid, version, dirty, title, title_res, notes, system_id, deleted, group_visible, should_sync, auto_add, favorites, group_is_read_only, sync1, sync2, sync3, sync4) values (1005, null, 1, 'f', 1, 0, 'Coworkers', null, 'System Group: Coworkers', 'Coworkers', 0, 0, 1, 0, 0, 1, null, '"YDwteyI."', '1970-01-01T00:00:00.000Z', null);
insert into groups ("_id", package_id, account_id, sourceid, version, dirty, title, title_res, notes, system_id, deleted, group_visible, should_sync, auto_add, favorites, group_is_read_only, sync1, sync2, sync3, sync4) values (2001, null, 1, null, 2, 0, 'hans.mimer.test', null, null, null, 0, 1, 1, 0, 0, 0, null, null, null, null);

delete from directories;
insert into directories ("_id", packageName, authority, typeResourceId, typeResourceName, accountType, accountName, displayName, exportSupport, shortcutSupport, photoSupport) values (0, 'com.mimer.providers.contacts', 'com.android.contacts', 2130968583, 'com.mimer.providers.contacts:string/default_directory', null, null, null, 0, 2, 3);
insert into directories ("_id", packageName, authority, typeResourceId, typeResourceName, accountType, accountName, displayName, exportSupport, shortcutSupport, photoSupport) values (1, 'com.mimer.providers.contacts', 'com.android.contacts', 2130968584, 'com.mimer.providers.contacts:string/local_invisible_directory', null, null, null, 0, 2, 3);



