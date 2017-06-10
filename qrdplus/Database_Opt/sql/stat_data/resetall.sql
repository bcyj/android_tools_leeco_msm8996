set echo off;
-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
set message off;

delete from raw_contacts;
delete from calls;
delete from data;
delete from contacts;
alter sequence calls_id_seq restart with 1;
alter sequence raw_contacts_id_seq restart with 1;
alter sequence data_id_seq restart with 1;
alter sequence contacts_id_seq restart with 1;
delete from "_sync_state_metadata";
delete from "_sync_state";
alter sequence sync_state_seq restart with 1;
insert into "_sync_state_metadata" (version) values (1);
delete from groups;
alter sequence groups_id_seq restart with 1;
delete from agg_presence;
delete from agg_exceptions;
delete from search_index;
delete from name_lookup;
delete from phone_lookup;
delete from accounts where account_name is not null;
delete from default_directory;
delete from directories;
alter sequence directories_id_seq restart with 1;
delete from visible_contacts;
delete from properties;
delete from "_sync_state";
alter sequence sync_state_seq restart with 1;
delete from agg_exceptions;
alter sequence agg_exceptions_seq restart with 1;
delete from data_usage_stat;
alter sequence data_usage_stat_id_seq restart with 1;
delete from packages;
alter sequence packages_id_seq restart with 1;
delete from photo_files;
alter sequence photo_files_id_seq restart with 1;
delete from settings;
delete from status_updates;
delete from stream_item_photos;
alter sequence stream_item_photos_id_seq restart with 1;
delete from stream_items;
alter sequence stream_items_id_seq restart with 1;
delete from voicemail_status;
alter sequence voicemail_status_id_seq restart with 1;
delete from v1_settings;
alter sequence v1_settings_seq restart with 1;

enter 'profile' using 'nopass';
delete from raw_contacts;
delete from calls;
delete from data;
delete from contacts;
alter sequence calls_id_seq restart with 9223372034707292160;
alter sequence raw_contacts_id_seq restart with 9223372034707292160;
alter sequence data_id_seq restart with 9223372034707292160;
alter sequence contacts_id_seq restart with 9223372034707292160;
delete from "_sync_state_metadata";
delete from "_sync_state";
insert into "_sync_state_metadata" (version) values (1);
delete from groups;
alter sequence groups_id_seq restart with 9223372034707292160;
delete from agg_presence;
delete from agg_exceptions;
delete from search_index;
delete from name_lookup;
delete from phone_lookup;
delete from accounts where account_name is not null;
delete from default_directory;
delete from directories;
alter sequence directories_id_seq restart with 9223372034707292160;
delete from visible_contacts;
delete from properties;
delete from "_sync_state";
alter sequence sync_state_seq restart with 1;
delete from agg_exceptions;
alter sequence agg_exceptions_seq restart with 1;
delete from data_usage_stat;
alter sequence data_usage_stat_id_seq restart with 1;
delete from packages;
alter sequence packages_id_seq restart with 1;
delete from photo_files;
alter sequence photo_files_id_seq restart with 9223372034707292160;
delete from settings;
delete from status_updates;
delete from stream_item_photos;
alter sequence stream_item_photos_id_seq restart with 9223372034707292160;
delete from stream_items;
alter sequence stream_items_id_seq restart with 9223372034707292160;
delete from voicemail_status;
alter sequence voicemail_status_id_seq restart with 1;
delete from v1_settings;
alter sequence v1_settings_seq restart with 1;


leave;