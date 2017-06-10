-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com


set sqlite on;

CREATE UNIQUE SEQUENCE pdu_id_seq as BIGINT in mmssms_db;
CREATE TABLE pdu (
	"_id" BIGINT PRIMARY KEY default NEXT_VALUE OF pdu_id_seq with check option,
	thread_id BIGINT,
	date BIGINT,
	date_sent BIGINT DEFAULT 0,
	msg_box INTEGER,
	read INTEGER DEFAULT 0,
	m_id nvarchar(128),
	sub nvarchar(512),
	sub_cs INTEGER,
	ct_t nvarchar(128),
	ct_l nvarchar(128),
	exp BIGINT,
	m_cls nvarchar(128),
	m_type INTEGER,
	v INTEGER,
	m_size INTEGER,
	pri INTEGER,
	rr INTEGER,
	rpt_a INTEGER,
	resp_st INTEGER,
	st INTEGER,
	tr_id nvarchar(128),
	retr_st INTEGER,
	retr_txt nvarchar(128),
	retr_txt_cs INTEGER,
	read_status INTEGER,
	ct_cls INTEGER,
	resp_txt nvarchar(128),
	d_tm INTEGER,
	d_rpt INTEGER,
	locked INTEGER DEFAULT 0,
	seen INTEGER DEFAULT 0
, "text_only" INTEGER DEFAULT 0
	,sub_id BIGINT DEFAULT 0
) in mmssms_db;


CREATE UNIQUE SEQUENCE sms_id_seq as BIGINT in mmssms_db;
CREATE TABLE sms (
	"_id" BIGINT PRIMARY KEY default NEXT_VALUE OF sms_id_seq with check option,
	thread_id BIGINT,
	address nvarchar(128),
	person INTEGER,
	date BIGINT,
	date_sent BIGINT DEFAULT 0,
	protocol INTEGER,
	read INTEGER DEFAULT 0,
	status INTEGER DEFAULT -1,
	type INTEGER,
	reply_path_present INTEGER,
	subject nvarchar(128),
	body nvarchar(2048),
	service_center nvarchar(128),
	locked INTEGER DEFAULT 0,
	sub_id BIGINT DEFAULT 0,
	error_code INTEGER DEFAULT 0,
	seen INTEGER DEFAULT 0
	,pri INTEGER DEFAULT -1
) in mmssms_db;


	create index pdu_thread_id_idx on pdu(thread_id);


CREATE TABLE mimer_master (
  type varchar(128),
  name varchar(128),
  tbl_name varchar(128),
  rootpage INTEGER,
  "sql" varchar(128)
) in mmssms_db;
insert into mimer_master values ('table', 'threads', null, null, 'AUTOINCREMENT');
insert into mimer_master values ('table', 'canonical_addresses', null, null, 'AUTOINCREMENT');
insert into mimer_master values ('table', 'pdu', null, null, 'AUTOINCREMENT');
insert into mimer_master values ('table', 'part', null, null, 'AUTOINCREMENT');



-- ###############################################################################
-- Indexes 
-- ###############################################################################

CREATE INDEX typeThreadIdIndex ON sms (type, thread_id);
CREATE INDEX threadIdTypeIndex ON sms (thread_id,type);


create index threads_date on threads("date");
create index threads_recipients_id on threads(recipient_ids);
create index canonical_address_idx on canonical_addresses(address);
create index part_mid_idx on part(mid);


call compatibility.set_or_create_pragma('mmssms_db','user_version','59');

CREATE UNIQUE SEQUENCE carriers_id_seq as BIGINT in telephony_db;
CREATE TABLE carriers(
	"_id" BIGINT PRIMARY KEY default NEXT_VALUE OF carriers_id_seq with check option,
	name nvarchar(50),
	"numeric" varchar(20),
	mcc varchar(10),
	mnc varchar(10),
	apn nvarchar(50),
	"user" nvarchar(128),
	server nvarchar(128),
	password nvarchar(128),
	proxy nvarchar(128),
	port varchar(50),
	mmsproxy varchar(50),
	mmsport varchar(10),
	mmsc nvarchar(128),
	authtype INTEGER,
	type varchar(50),
	"current" INTEGER,
	protocol varchar(50),
	roaming_protocol varchar(50),
	carrier_enabled SMALLINT,
	bearer INTEGER,
	mvno_type nvarchar(128),
	mvno_match_data nvarchar(128)
) in telephony_db;






-- This is a hack. We want to run upgrade on telephony because that
-- will insert carrier data. We replace the few create statements to
-- empty strings to avoid creating the schema in the provider.
call compatibility.set_or_create_pragma('telephony_db','user_version','0');



@
create procedure clear_data()
modifies sql data
begin
delete from addr;
delete from attachments;
delete from canonical_addresses;
delete from drm;
delete from part;
delete from pending_msgs;
delete from rate;
delete from raw;
delete from sr_pending;
delete from threads;
delete from pdu;
delete from sms;
delete from carriers;
end
@

grant select on addr to mimer$permission$telephony_read;
grant select on canonical_addresses to mimer$permission$telephony_read;
grant select on part to mimer$permission$telephony_read;
grant select on pdu to mimer$permission$telephony_read;
grant select on sms to mimer$permission$telephony_read;
grant select on threads to mimer$permission$telephony_read;

