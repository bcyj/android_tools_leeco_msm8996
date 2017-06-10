-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

whenever error continue;

drop databank mmssms_db cascade;
drop databank telephony2_db cascade;
drop databank cdmacalloption cascade;

whenever error exit;
set sqlite on;
-- ###############################################################################
-- Databanks & schemas
-- ###############################################################################

create databank mmssms_db set filesize 100K, goalsize 100K;
call compatibility.map_database('/data/data/com.android.providers.telephony/databases/mmssms.db', 'mmssms_db', 'telephony','', timestamp '0001-01-01 00:00:00');
call compatibility.map_auxiliary_database('/data/user/0/com.android.providers.telephony/databases/mmssms.db', 'mmssms_db', 'telephony','', timestamp '0001-01-01 00:00:00');
create databank telephony_db set filesize 100K, goalsize 100K;
call compatibility.map_database('/data/data/com.android.providers.telephony/databases/telephony.db', 'telephony_db', 'telephony','', timestamp '0001-01-01 00:00:00');
call compatibility.map_auxiliary_database('/data/user/0/com.android.providers.telephony/databases/telephony.db', 'telephony_db', 'telephony','', timestamp '0001-01-01 00:00:00');
create databank cdmacalloption_db set filesize 100K, goalsize 100K;
call compatibility.map_database('/data/data/com.android.providers.telephony/databases/cdmacalloption.db', 'cdmacalloption_db', 'telephony','', timestamp '0001-01-01 00:00:00');

CREATE UNIQUE SEQUENCE android_metadata_id_seq in telephony_db;
CREATE TABLE android_metadata(
	locale nvarchar(128)
) in telephony_db;

-- ###############################################################################
-- Tables
-- ###############################################################################

CREATE UNIQUE SEQUENCE addr_id_seq as BIGINT in mmssms_db;
CREATE TABLE addr (
	"_id" BIGINT PRIMARY KEY default NEXT_VALUE OF addr_id_seq with check option,
	msg_id BIGINT,
	contact_id BIGINT,
	address nvarchar(128),
	type INTEGER,
	charset INTEGER
) in mmssms_db;

CREATE UNIQUE SEQUENCE attachmaents_id_seq as BIGINT in mmssms_db;
CREATE TABLE attachments (
	sms_id BIGINT,
	content_url nvarchar(128),
	"offset" INTEGER
) in mmssms_db;




CREATE UNIQUE SEQUENCE canonical_address_id_seq as BIGINT in mmssms_db;
CREATE TABLE canonical_addresses (
	"_id" BIGINT PRIMARY KEY default NEXT_VALUE OF canonical_address_id_seq with check option,
	address nvarchar(256)
) in mmssms_db;

CREATE UNIQUE SEQUENCE drm_id_seq as BIGINT in mmssms_db;
CREATE TABLE drm (
	"_id" BIGINT PRIMARY KEY default NEXT_VALUE OF drm_id_seq with check option,
	"_data" nvarchar(128)
) in mmssms_db;

CREATE UNIQUE SEQUENCE part_id_seq as BIGINT in mmssms_db;
CREATE TABLE part (
	"_id" BIGINT PRIMARY KEY default NEXT_VALUE OF part_id_seq with check option,
	mid BIGINT,
	seq INTEGER DEFAULT 0,
	ct nvarchar(1024),
	name nvarchar(128),
	chset INTEGER,
	cd nvarchar(128),
	fn nvarchar(128),
	cid nvarchar(128),
	cl nvarchar(1024),
	ctt_s INTEGER,
	ctt_t nvarchar(128),
	"_data" nvarchar(128),
	text NCLOB
) in mmssms_db;


CREATE UNIQUE SEQUENCE pending_msgs_id_seq as BIGINT in mmssms_db;
CREATE TABLE pending_msgs (
	"_id" BIGINT PRIMARY KEY default NEXT_VALUE OF pending_msgs_id_seq with check option,
	proto_type INTEGER,
	msg_id BIGINT,
	msg_type INTEGER,
	err_type INTEGER,
	err_code INTEGER,
	retry_index INTEGER NOT NULL DEFAULT 0,
	due_time BIGINT,
	last_try BIGINT
) in mmssms_db;


--Is this sequence used?
CREATE UNIQUE SEQUENCE rate_id_seq as BIGINT in mmssms_db;
CREATE TABLE rate (
	sent_time BIGINT
) in mmssms_db;

CREATE UNIQUE SEQUENCE raw_id_seq as BIGINT in mmssms_db;
CREATE TABLE raw (
	"_id" BIGINT PRIMARY KEY default NEXT_VALUE OF raw_id_seq with check option,
	date BIGINT,
	reference_number INTEGER,
	count INTEGER,
	sequence INTEGER,
	destination_port INTEGER,
	address nvarchar(128),
pdu varchar(1000)
) in mmssms_db;


--Is this sequence used?
CREATE UNIQUE SEQUENCE sr_pending_id_seq as BIGINT in mmssms_db;
CREATE TABLE sr_pending (
	reference_number INTEGER,
	action nvarchar(128),
	data nvarchar(128)
) in mmssms_db;


CREATE UNIQUE SEQUENCE threads_id_seq as BIGINT in mmssms_db;
CREATE TABLE threads (
	"_id" BIGINT PRIMARY KEY default NEXT_VALUE OF threads_id_seq with check option,
	date BIGINT DEFAULT 0,
	message_count INTEGER DEFAULT 0,
	recipient_ids varchar(3000),
	snippet nvarchar(2048),
	snippet_cs INTEGER DEFAULT 0,
	read INTEGER DEFAULT 1,
	type INTEGER DEFAULT 0,
	error INTEGER DEFAULT 0,
	has_attachment INTEGER DEFAULT 0

) in mmssms_db;


-- CREATE UNIQUE SEQUENCE words_id_seq in mmssms_db;
-- CREATE TABLE words (
-- 	"_id" INTEGER default NEXT_VALUE OF words_id_seq,
-- 	index_text nvarchar(1600), 
-- 	source_id INTEGER, 
-- 	table_to_use INTEGER
-- ) in mmssms_db;
-- CREATE INDEX words_id ON words ("_id");

--CREATE UNIQUE SEQUENCE words_content_id_seq in mmssms_db;
--CREATE TABLE words_content(
--	docid INTEGER PRIMARY KEY default NEXT_VALUE OF words_content_id_seq,
--	c0_id INTEGER,
--	c1index_text nvarchar(2000),
--	c2source_id INTEGER,
--	c3table_to_use INTEGER
--) in mmssms_db;

--CREATE TABLE words_segdir(
--	level INTEGER,
--	idx INTEGER,
--	start_block INTEGER,
--	leaves_end_block INTEGER,
--	end_block INTEGER,
--	root BLOB,
--	PRIMARY KEY(level, idx)
--) in mmssms_db;

--CREATE UNIQUE SEQUENCE words_segments_id_seq in mmssms_db;
--CREATE TABLE words_segments(
--	blockid INTEGER PRIMARY KEY default NEXT_VALUE OF words_segments_id_seq,
--	block BLOB
--) in mmssms_db;

-- ###############################################################################
-- Indexes 
-- ###############################################################################

