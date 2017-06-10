set echo off;
-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
set message off;
delete from android_metadata;
alter sequence android_metadata_id_seq restart with 1;

alter sequence addr_id_seq restart with 1;
delete from addr;

alter sequence attachmaents_id_seq restart with 1;
delete from attachments;


alter sequence canonical_address_id_seq restart with 1;
delete from canonical_addresses;

alter sequence drm_id_seq restart with 1;
delete from drm;

alter sequence part_id_seq restart with 1;
delete from part;

alter sequence pending_msgs_id_seq restart with 1;
delete from pending_msgs;

alter sequence rate_id_seq restart with 1;
delete from rate;

alter sequence raw_id_seq restart with 1;
delete from raw;

alter sequence sr_pending_id_seq restart with 1;
delete from sr_pending;

alter sequence threads_id_seq restart with 1;
delete from threads;


alter sequence pdu_id_seq restart with 1;
delete from pdu;

alter sequence sms_id_seq restart with 1;
delete from sms;

alter sequence carriers_id_seq restart with 1;
delete from carriers;

