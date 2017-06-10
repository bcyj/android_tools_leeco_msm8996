-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

-- ###############################################################################
--Upgrade from 625 to 704

--alter table: 
--Table accounts: 
--		add column	_id INTEGER PRIMARY KEY AUTOINCREMENT
--Table groups:  	
--		add column	account_id INTEGER REFERENCES accounts(_id)
--		delete column	account_name STRING DEFAULT NULL, 
--				account_type STRING DEFAULT NULL, 
--				data_set STRING DEFAULT NULL
--Table raw_contacts
--		add column	account_id INTEGER REFERENCES accounts(_id)
--		delete column	account_name STRING DEFAULT NULL, 
--				account_type STRING DEFAULT NULL, 
--				data_set STRING DEFAULT NULL,


--alter view:
--VIEW view_data, view_entities, view_groups, view_raw_contacts, view_raw_entities, view_stream_items, view_v1_contact_methods, view_v1_extensions, view_v1_group_membership, view_v1_groups, view_v1_organizations, view_v1_people, view_v1_phones, view_v1_photos

--drop table:
--activities

--drop index:
--CREATE INDEX groups_source_id_data_set_index ON groups (sourceid, account_type, account_name, data_set);
--CREATE INDEX groups_source_id_index ON groups (sourceid, account_type, account_name);
--CREATE INDEX raw_contacts_source_id_data_set_index ON raw_contacts (sourceid, account_type, account_name, data_set);
--CREATE INDEX raw_contacts_source_id_index ON raw_contacts (sourceid, account_type, account_name);

--add index:
--groups_source_id_account_id_index
--raw_contacts_source_id_account_id_index

--alter trigger:
--groups_auto_add_updated1


--Others:
-- In table data_usage_stat, column "times_used" is changed from Integer to Bigint.
-- This is because in ContactsProvider 4.1, DataUsageStatColumns.TIMES_USED is assigned Long.MAX_VALUE (see ContactsProvider2.java, ProjectionMap sStrequentStarredProjectionMap )

-- ###############################################################################

-- ###############################################################################
-- Tables
-- ###############################################################################

-- NEXT_SEQUENCE_IS_PROFILE_VALUE
CREATE UNIQUE SEQUENCE raw_contacts_id_seq AS BIGINT in contacts2_db;
CREATE TABLE raw_contacts (
	"_id" BIGINT default NEXT_VALUE OF raw_contacts_id_seq with check option, 
	account_id BIGINT,
	sourceid VARCHAR(40),
	raw_contact_is_read_only INTEGER NOT NULL DEFAULT 0,
	version INTEGER NOT NULL DEFAULT 1,
	dirty INTEGER NOT NULL DEFAULT 0,
	deleted INTEGER NOT NULL DEFAULT 0,
	contact_id BIGINT, 
	aggregation_mode INTEGER NOT NULL DEFAULT 0,
	aggregation_needed INTEGER NOT NULL DEFAULT 1,
	custom_ringtone NVARCHAR(120),
	send_to_voicemail INTEGER NOT NULL DEFAULT 0,
	times_contacted INTEGER NOT NULL DEFAULT 0,
	last_time_contacted BIGINT,
	starred INTEGER NOT NULL DEFAULT 0,
	pinned INTEGER NOT NULL DEFAULT 2147483647,
	display_name NVARCHAR(120),
        display_name_alt NVARCHAR(120),
	display_name_source INTEGER NOT NULL DEFAULT 0,
        phonetic_name NVARCHAR(120),
        phonetic_name_style NVARCHAR(120),
        sort_key NVARCHAR(120) collate CURRENT_COLLATION_2,
	phonebook_label nvarchar(128),
	phonebook_bucket INTEGER,
        sort_key_alt NVARCHAR(120) collate CURRENT_COLLATION_2,
	phonebook_label_alt nvarchar(128),
	phonebook_bucket_alt INTEGER,
        name_verified INTEGER NOT NULL DEFAULT 0,
	sync1 NVARCHAR(256),
	sync2 NVARCHAR(256),
	sync3 NVARCHAR(120),
	sync4 NVARCHAR(120),

--	sectionIndex INTEGER default 0,  -- Section index for display name
        constraint raw_contacts_pk PRIMARY KEY ("_id") 
) in contacts2_db;




CREATE UNIQUE SEQUENCE packages_id_seq as BIGINT in contacts2_db;

CREATE TABLE packages (
	"_id" BIGINT default NEXT_VALUE OF packages_id_seq with check option, 
	package VARCHAR(120) NOT NULL,
        constraint packages_pk PRIMARY KEY ("_id")
) in contacts2_db;


CREATE UNIQUE SEQUENCE mimetypes_id_seq as BIGINT in contacts2_db;
CREATE TABLE mimetypes(
	"_id" BIGINT default NEXT_VALUE OF mimetypes_id_seq with check option,
	mimetype VARCHAR(120) NOT NULL,
        constraint mimetypes_pk PRIMARY KEY ("_id") 
) in contacts2_db;


-- NEXT_SEQUENCE_IS_PROFILE_VALUE
create unique sequence data_id_seq AS BIGINT in contacts2_db;

CREATE TABLE data (
	"_id" BIGINT default NEXT_VALUE OF data_id_seq with check option, 
	package_id BIGINT ,
	mimetype_id BIGINT NOT NULL,
	raw_contact_id BIGINT NOT NULL,
	is_read_only BOOLEAN NOT NULL DEFAULT false,
	is_primary BOOLEAN NOT NULL DEFAULT false,
	is_super_primary BOOLEAN NOT NULL DEFAULT false,
	data_version INTEGER NOT NULL DEFAULT 0,
	data1 NVARCHAR(512),
	data2 NVARCHAR(120),
	data3 NVARCHAR(120),
	data4 NVARCHAR(512),
	data5 NVARCHAR(120),
	data6 NVARCHAR(120),
	data7 NVARCHAR(120),
	data8 NVARCHAR(120),
	data9 NVARCHAR(120),
	data10 NVARCHAR(120),
	data11 NVARCHAR(120),
	data12 NVARCHAR(120),
	data13 NVARCHAR(120),
	data14 NVARCHAR(120),
	data15 BLOB,--NVARCHAR(120) --image
	data_sync1 NVARCHAR(512),
	data_sync2 NVARCHAR(256),
	data_sync3 NVARCHAR(256),
	data_sync4 NVARCHAR(256),
        constraint data_pk PRIMARY KEY ("_id") 
) in contacts2_db;


create unique sequence data_usage_stat_id_seq AS BIGINT in contacts2_db;
create table data_usage_stat(
        stat_id BIGINT default next_value of data_usage_stat_id_seq with check option,
        data_id BIGINT NOT NULL,
        usage_type INTEGER NOT NULL DEFAULT 0,
        times_used BIGINT NOT NULL DEFAULT 0,
        last_time_used BIGINT NOT NULL DEFAULT 0,
	constraint data_usage_stat_pk primary key(stat_id)--,
        --Foreing keys are not enforced by SQLite by default constraint data_usage_stat_fk_data FOREIGN KEY(data_id) REFERENCES data("_id")
);

CREATE TABLE default_directory(
        "_id" BIGINT PRIMARY KEY
);

CREATE TABLE deleted_contacts (
	contact_id BIGINT PRIMARY KEY,
	contact_deleted_timestamp BIGINT NOT NULL default 0
)  in contacts2_db;

-- NEXT_SEQUENCE_IS_PROFILE_VALUE
create unique sequence directories_id_seq AS BIGINT in contacts2_db;
CREATE TABLE directories(
        "_id" BIGINT PRIMARY KEY default next_value of directories_id_seq with check option,
        packageName varchar(128) NOT NULL,
        authority nvarchar(128) NOT NULL,
        typeResourceId INTEGER,
        typeResourceName nvarchar(128),
        accountType varchar(128),
        accountName varchar(128),
        displayName nvarchar(120),
        exportSupport INTEGER NOT NULL DEFAULT 0,
        shortcutSupport INTEGER NOT NULL DEFAULT 0,
        photoSupport INTEGER NOT NULL DEFAULT 0
);

-- NEXT_SEQUENCE_IS_PROFILE_VALUE
CREATE UNIQUE SEQUENCE contacts_id_seq AS BIGINT in contacts2_db;

CREATE TABLE contacts (
	"_id" BIGINT default NEXT_VALUE OF contacts_id_seq with check option,
	name_raw_contact_id BIGINT,
	photo_id BIGINT,
	photo_file_id BIGINT, -- The foreing is not enforced in SQLite REFERENCES photo_files(_id), 
	custom_ringtone NVARCHAR(120),
	send_to_voicemail INTEGER NOT NULL DEFAULT 0,
	times_contacted INTEGER NOT NULL DEFAULT 0,
	last_time_contacted BIGINT,
	starred INTEGER NOT NULL DEFAULT 0,
	pinned INTEGER NOT NULL DEFAULT 2147483647,
	has_phone_number INTEGER NOT NULL DEFAULT 0,
	lookup VARCHAR(512),
	status_update_id BIGINT,
    contact_last_updated_timestamp BIGINT,
        constraint contacts_pk PRIMARY KEY ("_id")
) in contacts2_db;

CREATE TABLE phone_lookup (
	data_id BIGINT NOT NULL,
	raw_contact_id BIGINT NOT NULL,
	normalized_number VARCHAR(120) NOT NULL,
	min_match NVARCHAR(120) NOT NULL--,
        -- Removed in android 4 constraint phone_lookup_pk PRIMARY KEY (data_id),
	-- Added in Android 4, but foreign keys are not enforced: constraint phone_lookup_data_fk foreign key(data_id) references data("_id")
) in contacts2_db;

-- NEXT_SEQUENCE_IS_PROFILE_VALUE
CREATE UNIQUE SEQUENCE photo_files_id_seq AS BIGINT in contacts2_db;
CREATE TABLE photo_files
    (
        "_id" BIGINT PRIMARY KEY default next_value of photo_files_id_seq with check option,
        height INTEGER NOT NULL,
        width INTEGER NOT NULL,
        filesize INTEGER NOT NULL
    );

CREATE TABLE name_lookup (
	data_id BIGINT NOT NULL,
	raw_contact_id BIGINT NOT NULL,
	normalized_name VARCHAR(100) NOT NULL,
	name_type INTEGER NOT NULL,
	--PRIMARY KEY (data_id, normalized_name, name_type)
	constraint name_lookup_pk PRIMARY KEY (normalized_name, name_type, data_id)
) in contacts2_db;

CREATE TABLE nickname_lookup (
	name VARCHAR(120),
	--cluster VARCHAR(120),
	cluster integer,
        constraint nickname_lookup_pk PRIMARY KEY (name, cluster)
) in contacts2_db;
set echo off;
INSERT INTO nickname_lookup VALUES('292B393529393F',0);
INSERT INTO nickname_lookup VALUES('292B2B3931',0);
INSERT INTO nickname_lookup VALUES('3529393F',0);
INSERT INTO nickname_lookup VALUES('3529593F31',0);
INSERT INTO nickname_lookup VALUES('292B31',1);
INSERT INTO nickname_lookup VALUES('292B4B29372941',1);
INSERT INTO nickname_lookup VALUES('2935353931',2);
INSERT INTO nickname_lookup VALUES('2935294F3729',2);
INSERT INTO nickname_lookup VALUES('293543314D',2);
INSERT INTO nickname_lookup VALUES('293F2B314B4F',3);
INSERT INTO nickname_lookup VALUES('293F',3);
INSERT INTO nickname_lookup VALUES('2B314B4F',3);
INSERT INTO nickname_lookup VALUES('2B314B4F3931',3);
INSERT INTO nickname_lookup VALUES('293F315729432F314B',4);
INSERT INTO nickname_lookup VALUES('293F',4);
INSERT INTO nickname_lookup VALUES('293F312D',4);
INSERT INTO nickname_lookup VALUES('293F3157',4);
INSERT INTO nickname_lookup VALUES('3F3157',4);
INSERT INTO nickname_lookup VALUES('4D294D3729',4);
INSERT INTO nickname_lookup VALUES('293F315729432F4B29',5);
INSERT INTO nickname_lookup VALUES('293F',5);
INSERT INTO nickname_lookup VALUES('293F3F3931',5);
INSERT INTO nickname_lookup VALUES('293F3F59',5);
INSERT INTO nickname_lookup VALUES('3F3157',5);
INSERT INTO nickname_lookup VALUES('3F31573931',5);
INSERT INTO nickname_lookup VALUES('4D29432F4B29',5);
INSERT INTO nickname_lookup VALUES('4D29432F59',5);
INSERT INTO nickname_lookup VALUES('4D294D3729',5);
INSERT INTO nickname_lookup VALUES('293F33',6);
INSERT INTO nickname_lookup VALUES('293F334B312F',6);
INSERT INTO nickname_lookup VALUES('293F334B312F45',6);
INSERT INTO nickname_lookup VALUES('293F333931',6);
INSERT INTO nickname_lookup VALUES('293F392D31',7);
INSERT INTO nickname_lookup VALUES('293F3F3931',7);
INSERT INTO nickname_lookup VALUES('293F3F59',7);
INSERT INTO nickname_lookup VALUES('293F394D4543',8);
INSERT INTO nickname_lookup VALUES('293F3F3931',8);
INSERT INTO nickname_lookup VALUES('293F3F59',8);
INSERT INTO nickname_lookup VALUES('293F3F394D4543',9);
INSERT INTO nickname_lookup VALUES('293F3F3931',9);
INSERT INTO nickname_lookup VALUES('293F3F59',9);
INSERT INTO nickname_lookup VALUES('294129432F29',10);
INSERT INTO nickname_lookup VALUES('4129432F39',10);
INSERT INTO nickname_lookup VALUES('4129432F59',10);
INSERT INTO nickname_lookup VALUES('29432F4B3129',11);
INSERT INTO nickname_lookup VALUES('29432F3931',11);
INSERT INTO nickname_lookup VALUES('29432F4B3155',12);
INSERT INTO nickname_lookup VALUES('29432F59',12);
INSERT INTO nickname_lookup VALUES('2F4B3155',12);
INSERT INTO nickname_lookup VALUES('29434331',13);
INSERT INTO nickname_lookup VALUES('2943433931',13);
INSERT INTO nickname_lookup VALUES('294343314F4F31',13);
INSERT INTO nickname_lookup VALUES('29434F37454359',14);
INSERT INTO nickname_lookup VALUES('4F454359',14);
INSERT INTO nickname_lookup VALUES('4F454339',14);
INSERT INTO nickname_lookup VALUES('4F454331',14);
INSERT INTO nickname_lookup VALUES('294B4F37514B',15);
INSERT INTO nickname_lookup VALUES('294B4F',15);
INSERT INTO nickname_lookup VALUES('294B4F59',15);
INSERT INTO nickname_lookup VALUES('2B294B2B294B29',16);
INSERT INTO nickname_lookup VALUES('2B292B4D',16);
INSERT INTO nickname_lookup VALUES('2B294B2B',16);
INSERT INTO nickname_lookup VALUES('2B294B2B3931',16);
INSERT INTO nickname_lookup VALUES('2B31433B29413943',17);
INSERT INTO nickname_lookup VALUES('2B3143',17);
INSERT INTO nickname_lookup VALUES('2B31433B39',17);
INSERT INTO nickname_lookup VALUES('2B31434359',17);
INSERT INTO nickname_lookup VALUES('2B314B43294B2F',18);
INSERT INTO nickname_lookup VALUES('2B314B43',18);
INSERT INTO nickname_lookup VALUES('2B314B433931',18);
INSERT INTO nickname_lookup VALUES('2B294B433931',18);
INSERT INTO nickname_lookup VALUES('2B314B4F4B2941',19);
INSERT INTO nickname_lookup VALUES('2B314B4F',19);
INSERT INTO nickname_lookup VALUES('2B314B4F3931',19);
INSERT INTO nickname_lookup VALUES('2B4B292F3F59',20);
INSERT INTO nickname_lookup VALUES('2B4B292F',20);
INSERT INTO nickname_lookup VALUES('2D293F533943',21);
INSERT INTO nickname_lookup VALUES('2D293F',21);
INSERT INTO nickname_lookup VALUES('2D294F37314B394331',22);
INSERT INTO nickname_lookup VALUES('2D294F',22);
INSERT INTO nickname_lookup VALUES('2D294F31',22);
INSERT INTO nickname_lookup VALUES('2D294F37',22);
INSERT INTO nickname_lookup VALUES('2D294F3931',22);
INSERT INTO nickname_lookup VALUES('2D294F3759',22);
INSERT INTO nickname_lookup VALUES('3D294F',22);
INSERT INTO nickname_lookup VALUES('3D294F31',22);
INSERT INTO nickname_lookup VALUES('3D294F3931',22);
INSERT INTO nickname_lookup VALUES('3D294F3759',22);
INSERT INTO nickname_lookup VALUES('2D294B4B3931',23);
INSERT INTO nickname_lookup VALUES('2D294B453F394331',23);
INSERT INTO nickname_lookup VALUES('2D294B453F5943',23);
INSERT INTO nickname_lookup VALUES('2D37294B3F314D',24);
INSERT INTO nickname_lookup VALUES('2D37512D3D',24);
INSERT INTO nickname_lookup VALUES('2D37295B',24);
INSERT INTO nickname_lookup VALUES('2D37294B3F3931',24);
INSERT INTO nickname_lookup VALUES('2B512D3D',24);
INSERT INTO nickname_lookup VALUES('2D374B394D4F394331',25);
INSERT INTO nickname_lookup VALUES('2D374B394D4D59',25);
INSERT INTO nickname_lookup VALUES('2D374B394D4D3931',25);
INSERT INTO nickname_lookup VALUES('2D374B394D4F454737314B',26);
INSERT INTO nickname_lookup VALUES('2D374B394D',26);
INSERT INTO nickname_lookup VALUES('2D3F39434F4543',27);
INSERT INTO nickname_lookup VALUES('2D3F39434F',27);
INSERT INTO nickname_lookup VALUES('2D59434F373929',28);
INSERT INTO nickname_lookup VALUES('2D39432F59',28);
INSERT INTO nickname_lookup VALUES('2D59434F37',28);
INSERT INTO nickname_lookup VALUES('2F294339313F',29);
INSERT INTO nickname_lookup VALUES('2F2943',29);
INSERT INTO nickname_lookup VALUES('2F29434359',29);
INSERT INTO nickname_lookup VALUES('2F2953392F',30);
INSERT INTO nickname_lookup VALUES('2F295331',30);
INSERT INTO nickname_lookup VALUES('2F312B454B2937',31);
INSERT INTO nickname_lookup VALUES('2F312B',31);
INSERT INTO nickname_lookup VALUES('2F312B2B3931',31);
INSERT INTO nickname_lookup VALUES('2F314343394D',32);
INSERT INTO nickname_lookup VALUES('2F3143',32);
INSERT INTO nickname_lookup VALUES('2F31434359',32);
INSERT INTO nickname_lookup VALUES('2F453F454B314D',33);
INSERT INTO nickname_lookup VALUES('2F453F3F59',33);
INSERT INTO nickname_lookup VALUES('2F4543293F2F',34);
INSERT INTO nickname_lookup VALUES('2F4543',34);
INSERT INTO nickname_lookup VALUES('2F45434359',34);
INSERT INTO nickname_lookup VALUES('2F454343294F313F3F29',35);
INSERT INTO nickname_lookup VALUES('2F45434329',35);
INSERT INTO nickname_lookup VALUES('2F454B454F373129',36);
INSERT INTO nickname_lookup VALUES('2F454F',36);
INSERT INTO nickname_lookup VALUES('2F454F4F59',36);
INSERT INTO nickname_lookup VALUES('2F454B454F3759',37);
INSERT INTO nickname_lookup VALUES('2F454F',37);
INSERT INTO nickname_lookup VALUES('2F454F4F59',37);
INSERT INTO nickname_lookup VALUES('2F4551353F294D',38);
INSERT INTO nickname_lookup VALUES('2F455135',38);
INSERT INTO nickname_lookup VALUES('312F55294B2F',39);
INSERT INTO nickname_lookup VALUES('312F',39);
INSERT INTO nickname_lookup VALUES('312F2F3931',39);
INSERT INTO nickname_lookup VALUES('43312F',39);
INSERT INTO nickname_lookup VALUES('43312F2F3931',39);
INSERT INTO nickname_lookup VALUES('43312F2F59',39);
INSERT INTO nickname_lookup VALUES('4F312F',39);
INSERT INTO nickname_lookup VALUES('4F312F2F59',39);
INSERT INTO nickname_lookup VALUES('4F312F2F3931',39);
INSERT INTO nickname_lookup VALUES('313F312943454B',40);
INSERT INTO nickname_lookup VALUES('313F3F29',40);
INSERT INTO nickname_lookup VALUES('313F3F3931',40);
INSERT INTO nickname_lookup VALUES('313F3F31',40);
INSERT INTO nickname_lookup VALUES('313F394D292B314F4F29',41);
INSERT INTO nickname_lookup VALUES('2B314F4F29',41);
INSERT INTO nickname_lookup VALUES('313F395B292B314F37',42);
INSERT INTO nickname_lookup VALUES('2B314F37',42);
INSERT INTO nickname_lookup VALUES('2B314D4D',42);
INSERT INTO nickname_lookup VALUES('2B314D4D3931',42);
INSERT INTO nickname_lookup VALUES('2B314F4D59',42);
INSERT INTO nickname_lookup VALUES('2B314F4F59',42);
INSERT INTO nickname_lookup VALUES('2B314F4F31',42);
INSERT INTO nickname_lookup VALUES('313F395B29',42);
INSERT INTO nickname_lookup VALUES('3F394D29',42);
INSERT INTO nickname_lookup VALUES('3F395B29',42);
INSERT INTO nickname_lookup VALUES('3F395B',42);
INSERT INTO nickname_lookup VALUES('3141393F59',43);
INSERT INTO nickname_lookup VALUES('3141',43);
INSERT INTO nickname_lookup VALUES('31414D',43);
INSERT INTO nickname_lookup VALUES('31414159',43);
INSERT INTO nickname_lookup VALUES('31414129',44);
INSERT INTO nickname_lookup VALUES('3141',44);
INSERT INTO nickname_lookup VALUES('31414D',44);
INSERT INTO nickname_lookup VALUES('31414159',44);
INSERT INTO nickname_lookup VALUES('315135314331',45);
INSERT INTO nickname_lookup VALUES('35314331',45);
INSERT INTO nickname_lookup VALUES('332943433931',46);
INSERT INTO nickname_lookup VALUES('3329434359',46);
INSERT INTO nickname_lookup VALUES('333F454B31432D31',47);
INSERT INTO nickname_lookup VALUES('333F45',47);
INSERT INTO nickname_lookup VALUES('334B29432D314D',48);
INSERT INTO nickname_lookup VALUES('334B2943',48);
INSERT INTO nickname_lookup VALUES('334B29432D3931',48);
INSERT INTO nickname_lookup VALUES('334B29432D394D',49);
INSERT INTO nickname_lookup VALUES('334B2943',49);
INSERT INTO nickname_lookup VALUES('334B29433D',49);
INSERT INTO nickname_lookup VALUES('334B29433D3931',49);
INSERT INTO nickname_lookup VALUES('334B312F314B392D3D',50);
INSERT INTO nickname_lookup VALUES('334B312F',50);
INSERT INTO nickname_lookup VALUES('334B312F2F59',50);
INSERT INTO nickname_lookup VALUES('35292B4B39313F',51);
INSERT INTO nickname_lookup VALUES('35292B31',51);
INSERT INTO nickname_lookup VALUES('35314B293F2F',52);
INSERT INTO nickname_lookup VALUES('35314B4B59',52);
INSERT INTO nickname_lookup VALUES('35314B294B2F',53);
INSERT INTO nickname_lookup VALUES('35314B4B59',53);
INSERT INTO nickname_lookup VALUES('354B3135454B59',54);
INSERT INTO nickname_lookup VALUES('354B3135',54);
INSERT INTO nickname_lookup VALUES('354B313535',54);
INSERT INTO nickname_lookup VALUES('37294B453F2F',55);
INSERT INTO nickname_lookup VALUES('37293F',55);
INSERT INTO nickname_lookup VALUES('37294B4B59',55);
INSERT INTO nickname_lookup VALUES('3731434B59',56);
INSERT INTO nickname_lookup VALUES('37293F',56);
INSERT INTO nickname_lookup VALUES('3729433D',56);
INSERT INTO nickname_lookup VALUES('37294B4B59',56);
INSERT INTO nickname_lookup VALUES('37314B2B314B4F',57);
INSERT INTO nickname_lookup VALUES('2B314B4F',57);
INSERT INTO nickname_lookup VALUES('2B314B4F3931',57);
INSERT INTO nickname_lookup VALUES('394B53394335',58);
INSERT INTO nickname_lookup VALUES('394B53',58);
INSERT INTO nickname_lookup VALUES('394D292B313F3F29',59);
INSERT INTO nickname_lookup VALUES('394D29',59);
INSERT INTO nickname_lookup VALUES('395B5B59',59);
INSERT INTO nickname_lookup VALUES('2B313F3F29',59);
INSERT INTO nickname_lookup VALUES('3B292D452B',60);
INSERT INTO nickname_lookup VALUES('3B293D31',60);
INSERT INTO nickname_lookup VALUES('3B292D4951313F394331',61);
INSERT INTO nickname_lookup VALUES('3B292D3D3931',61);
INSERT INTO nickname_lookup VALUES('3B2941314D',62);
INSERT INTO nickname_lookup VALUES('3B3941',62);
INSERT INTO nickname_lookup VALUES('3B39414159',62);
INSERT INTO nickname_lookup VALUES('3B29413931',62);
INSERT INTO nickname_lookup VALUES('3B452D3D',62);
INSERT INTO nickname_lookup VALUES('3B2943314F',63);
INSERT INTO nickname_lookup VALUES('3B2943',63);
INSERT INTO nickname_lookup VALUES('3B2943392D31',64);
INSERT INTO nickname_lookup VALUES('3B2943',64);
INSERT INTO nickname_lookup VALUES('3B294D4543',65);
INSERT INTO nickname_lookup VALUES('3B2959',65);
INSERT INTO nickname_lookup VALUES('3B313333314B4D4543',66);
INSERT INTO nickname_lookup VALUES('3B313333',66);
INSERT INTO nickname_lookup VALUES('3B3133334B3159',67);
INSERT INTO nickname_lookup VALUES('3B313333',67);
INSERT INTO nickname_lookup VALUES('3B3143433933314B',68);
INSERT INTO nickname_lookup VALUES('3B3143',68);
INSERT INTO nickname_lookup VALUES('3B31434359',68);
INSERT INTO nickname_lookup VALUES('3B314B454131',69);
INSERT INTO nickname_lookup VALUES('3B314B4B59',69);
INSERT INTO nickname_lookup VALUES('3B314D4D392D29',70);
INSERT INTO nickname_lookup VALUES('3B314D4D3931',70);
INSERT INTO nickname_lookup VALUES('3B453743',71);
INSERT INTO nickname_lookup VALUES('3B4537434359',71);
INSERT INTO nickname_lookup VALUES('3B4543',71);
INSERT INTO nickname_lookup VALUES('3B292D3D',72);
INSERT INTO nickname_lookup VALUES('3B292D3D59',72);
INSERT INTO nickname_lookup VALUES('3B4543294F372943',73);
INSERT INTO nickname_lookup VALUES('3B4543',73);
INSERT INTO nickname_lookup VALUES('3B454D314737',74);
INSERT INTO nickname_lookup VALUES('3B4531',74);
INSERT INTO nickname_lookup VALUES('3B453159',74);
INSERT INTO nickname_lookup VALUES('3B454D375129',75);
INSERT INTO nickname_lookup VALUES('3B454D37',75);
INSERT INTO nickname_lookup VALUES('3D29394F3F5943',76);
INSERT INTO nickname_lookup VALUES('2D294F',76);
INSERT INTO nickname_lookup VALUES('2D294F31',76);
INSERT INTO nickname_lookup VALUES('2D294F3931',76);
INSERT INTO nickname_lookup VALUES('2D294F37',76);
INSERT INTO nickname_lookup VALUES('2D294F3759',76);
INSERT INTO nickname_lookup VALUES('3D294F',76);
INSERT INTO nickname_lookup VALUES('3D294F31',76);
INSERT INTO nickname_lookup VALUES('3D294F3931',76);
INSERT INTO nickname_lookup VALUES('3D294F3759',76);
INSERT INTO nickname_lookup VALUES('3D294F37314B394331',77);
INSERT INTO nickname_lookup VALUES('2D294F',77);
INSERT INTO nickname_lookup VALUES('2D294F31',77);
INSERT INTO nickname_lookup VALUES('2D294F3931',77);
INSERT INTO nickname_lookup VALUES('2D294F37',77);
INSERT INTO nickname_lookup VALUES('2D294F3759',77);
INSERT INTO nickname_lookup VALUES('3D294F',77);
INSERT INTO nickname_lookup VALUES('3D294F31',77);
INSERT INTO nickname_lookup VALUES('3D294F3931',77);
INSERT INTO nickname_lookup VALUES('3D294F3759',77);
INSERT INTO nickname_lookup VALUES('3D294F373F313143',78);
INSERT INTO nickname_lookup VALUES('2D294F',78);
INSERT INTO nickname_lookup VALUES('2D294F31',78);
INSERT INTO nickname_lookup VALUES('2D294F3931',78);
INSERT INTO nickname_lookup VALUES('2D294F37',78);
INSERT INTO nickname_lookup VALUES('2D294F3759',78);
INSERT INTO nickname_lookup VALUES('3D294F',78);
INSERT INTO nickname_lookup VALUES('3D294F31',78);
INSERT INTO nickname_lookup VALUES('3D294F3931',78);
INSERT INTO nickname_lookup VALUES('3D294F3759',78);
INSERT INTO nickname_lookup VALUES('3D294F4B394329',79);
INSERT INTO nickname_lookup VALUES('2D294F',79);
INSERT INTO nickname_lookup VALUES('2D294F31',79);
INSERT INTO nickname_lookup VALUES('2D294F3931',79);
INSERT INTO nickname_lookup VALUES('2D294F37',79);
INSERT INTO nickname_lookup VALUES('2D294F3759',79);
INSERT INTO nickname_lookup VALUES('3D294F',79);
INSERT INTO nickname_lookup VALUES('3D294F31',79);
INSERT INTO nickname_lookup VALUES('3D294F3931',79);
INSERT INTO nickname_lookup VALUES('3D294F3759',79);
INSERT INTO nickname_lookup VALUES('3D314343314F37',80);
INSERT INTO nickname_lookup VALUES('3D3143',80);
INSERT INTO nickname_lookup VALUES('3D31533943',81);
INSERT INTO nickname_lookup VALUES('3D3153',81);
INSERT INTO nickname_lookup VALUES('3D3941',82);
INSERT INTO nickname_lookup VALUES('3D39412B314B3F59',82);
INSERT INTO nickname_lookup VALUES('3F29514B29',83);
INSERT INTO nickname_lookup VALUES('3F29514B39',83);
INSERT INTO nickname_lookup VALUES('3F29514B3931',83);
INSERT INTO nickname_lookup VALUES('3F29514B3143',84);
INSERT INTO nickname_lookup VALUES('3F29514B39',84);
INSERT INTO nickname_lookup VALUES('3F29514B3931',84);
INSERT INTO nickname_lookup VALUES('3F29554B31432D31',85);
INSERT INTO nickname_lookup VALUES('3F294B4B59',85);
INSERT INTO nickname_lookup VALUES('3F314543294B2F',86);
INSERT INTO nickname_lookup VALUES('3F3145',86);
INSERT INTO nickname_lookup VALUES('3F3143',86);
INSERT INTO nickname_lookup VALUES('3F31434359',86);
INSERT INTO nickname_lookup VALUES('3F314547453F2F',87);
INSERT INTO nickname_lookup VALUES('3F3145',87);
INSERT INTO nickname_lookup VALUES('3F3143',87);
INSERT INTO nickname_lookup VALUES('3F31434359',87);
INSERT INTO nickname_lookup VALUES('41292F313F394331',88);
INSERT INTO nickname_lookup VALUES('41292F2F3931',88);
INSERT INTO nickname_lookup VALUES('41292F2F59',88);
INSERT INTO nickname_lookup VALUES('41294B35294B314F',89);
INSERT INTO nickname_lookup VALUES('41294B3531',89);
INSERT INTO nickname_lookup VALUES('41294B35',89);
INSERT INTO nickname_lookup VALUES('412935353931',89);
INSERT INTO nickname_lookup VALUES('4129354D',89);
INSERT INTO nickname_lookup VALUES('413135',89);
INSERT INTO nickname_lookup VALUES('4731353559',89);
INSERT INTO nickname_lookup VALUES('354B314F29',89);
INSERT INTO nickname_lookup VALUES('354B314F2D373143',89);
INSERT INTO nickname_lookup VALUES('41294B4F3943',90);
INSERT INTO nickname_lookup VALUES('41294B4F3931',90);
INSERT INTO nickname_lookup VALUES('41294B4F59',90);
INSERT INTO nickname_lookup VALUES('41294F4F373155',91);
INSERT INTO nickname_lookup VALUES('41294F4F',91);
INSERT INTO nickname_lookup VALUES('41294F4F3931',91);
INSERT INTO nickname_lookup VALUES('4129514B313143',92);
INSERT INTO nickname_lookup VALUES('4145',92);
INSERT INTO nickname_lookup VALUES('4129514B392D31',93);
INSERT INTO nickname_lookup VALUES('4145',93);
INSERT INTO nickname_lookup VALUES('41295755313F3F',94);
INSERT INTO nickname_lookup VALUES('412957',94);
INSERT INTO nickname_lookup VALUES('4129573941393F392943',95);
INSERT INTO nickname_lookup VALUES('4129573941',95);
INSERT INTO nickname_lookup VALUES('412957',95);
INSERT INTO nickname_lookup VALUES('4131352943',96);
INSERT INTO nickname_lookup VALUES('413135',96);
INSERT INTO nickname_lookup VALUES('41392D3729313F',97);
INSERT INTO nickname_lookup VALUES('41392D3D3159',97);
INSERT INTO nickname_lookup VALUES('41392D3D',97);
INSERT INTO nickname_lookup VALUES('41393D31',97);
INSERT INTO nickname_lookup VALUES('41393D3159',97);
INSERT INTO nickname_lookup VALUES('41454B4B394D',98);
INSERT INTO nickname_lookup VALUES('4145',98);
INSERT INTO nickname_lookup VALUES('4329432D59',99);
INSERT INTO nickname_lookup VALUES('432943',99);
INSERT INTO nickname_lookup VALUES('43294F372943',100);
INSERT INTO nickname_lookup VALUES('43294F',100);
INSERT INTO nickname_lookup VALUES('43294F31',100);
INSERT INTO nickname_lookup VALUES('43294F37294339313F',101);
INSERT INTO nickname_lookup VALUES('43294F',101);
INSERT INTO nickname_lookup VALUES('43294F31',101);
INSERT INTO nickname_lookup VALUES('43392D37453F294D',102);
INSERT INTO nickname_lookup VALUES('43392D3D',102);
INSERT INTO nickname_lookup VALUES('43392D453F31',103);
INSERT INTO nickname_lookup VALUES('43392D3D59',103);
INSERT INTO nickname_lookup VALUES('43392D3D3931',103);
INSERT INTO nickname_lookup VALUES('43393D3D59',103);
INSERT INTO nickname_lookup VALUES('472941313F29',104);
INSERT INTO nickname_lookup VALUES('472941',104);
INSERT INTO nickname_lookup VALUES('47294F4B392D3929',105);
INSERT INTO nickname_lookup VALUES('47294F',105);
INSERT INTO nickname_lookup VALUES('47294F4D59',105);
INSERT INTO nickname_lookup VALUES('47294F4F59',105);
INSERT INTO nickname_lookup VALUES('4F4B394D37',105);
INSERT INTO nickname_lookup VALUES('4F4B392D3929',105);
INSERT INTO nickname_lookup VALUES('47294F4B392D3D',106);
INSERT INTO nickname_lookup VALUES('47294F',106);
INSERT INTO nickname_lookup VALUES('47294F4F314B',106);
INSERT INTO nickname_lookup VALUES('473143313F454731',107);
INSERT INTO nickname_lookup VALUES('4731434359',107);
INSERT INTO nickname_lookup VALUES('47314F314B',108);
INSERT INTO nickname_lookup VALUES('47314F31',108);
INSERT INTO nickname_lookup VALUES('4B29594145432F',109);
INSERT INTO nickname_lookup VALUES('4B2959',109);
INSERT INTO nickname_lookup VALUES('4737393F3947',110);
INSERT INTO nickname_lookup VALUES('4737393F',110);
INSERT INTO nickname_lookup VALUES('4B312B312D2D29',111);
INSERT INTO nickname_lookup VALUES('2B312D2D29',111);
INSERT INTO nickname_lookup VALUES('2B312D3D59',111);
INSERT INTO nickname_lookup VALUES('4B392D37294B2F',112);
INSERT INTO nickname_lookup VALUES('4B392D3D',112);
INSERT INTO nickname_lookup VALUES('4B392D37',112);
INSERT INTO nickname_lookup VALUES('2F392D3D',112);
INSERT INTO nickname_lookup VALUES('4B452B314B4F',113);
INSERT INTO nickname_lookup VALUES('2B452B',113);
INSERT INTO nickname_lookup VALUES('4B452B',113);
INSERT INTO nickname_lookup VALUES('4B452B2B3931',113);
INSERT INTO nickname_lookup VALUES('2B452B2B59',113);
INSERT INTO nickname_lookup VALUES('4B292B',113);
INSERT INTO nickname_lookup VALUES('4B452F4331594B452F',114);
INSERT INTO nickname_lookup VALUES('4B4543293F2F',115);
INSERT INTO nickname_lookup VALUES('4B4543',115);
INSERT INTO nickname_lookup VALUES('4B4543433931',115);
INSERT INTO nickname_lookup VALUES('4B454D3141294B59',116);
INSERT INTO nickname_lookup VALUES('4B454D3931',116);
INSERT INTO nickname_lookup VALUES('4B454D31',116);
INSERT INTO nickname_lookup VALUES('4B514D4D313F3F',117);
INSERT INTO nickname_lookup VALUES('4B514D4D',117);
INSERT INTO nickname_lookup VALUES('4B514D4F59',117);
INSERT INTO nickname_lookup VALUES('4B592943',118);
INSERT INTO nickname_lookup VALUES('4B59',118);
INSERT INTO nickname_lookup VALUES('4D294129434F3729',119);
INSERT INTO nickname_lookup VALUES('4D2941',119);
INSERT INTO nickname_lookup VALUES('4D294151313F',120);
INSERT INTO nickname_lookup VALUES('4D2941',120);
INSERT INTO nickname_lookup VALUES('4D29414159',120);
INSERT INTO nickname_lookup VALUES('4D4547373929',121);
INSERT INTO nickname_lookup VALUES('4D4547373931',121);
INSERT INTO nickname_lookup VALUES('4D4F31473729433931',122);
INSERT INTO nickname_lookup VALUES('4D4F314737',122);
INSERT INTO nickname_lookup VALUES('4D4F3147373931',122);
INSERT INTO nickname_lookup VALUES('4D4F3147373143',123);
INSERT INTO nickname_lookup VALUES('4D4F315331',123);
INSERT INTO nickname_lookup VALUES('4D4F31533143',124);
INSERT INTO nickname_lookup VALUES('4D4F315331',124);
INSERT INTO nickname_lookup VALUES('4D4F51294B4F',125);
INSERT INTO nickname_lookup VALUES('4D4F51',125);
INSERT INTO nickname_lookup VALUES('4D514D2943',126);
INSERT INTO nickname_lookup VALUES('4D5131',126);
INSERT INTO nickname_lookup VALUES('4D514D3931',126);
INSERT INTO nickname_lookup VALUES('4D515B3931',126);
INSERT INTO nickname_lookup VALUES('4D515B29434331',127);
INSERT INTO nickname_lookup VALUES('4D5131',127);
INSERT INTO nickname_lookup VALUES('4D514D3931',127);
INSERT INTO nickname_lookup VALUES('4D515B3931',127);
INSERT INTO nickname_lookup VALUES('4F2941294B29',128);
INSERT INTO nickname_lookup VALUES('4F29414159',128);
INSERT INTO nickname_lookup VALUES('4F37314B314D29',129);
INSERT INTO nickname_lookup VALUES('4F314B314D29',129);
INSERT INTO nickname_lookup VALUES('4F3731452F454B29',130);
INSERT INTO nickname_lookup VALUES('4F312F2F3931',130);
INSERT INTO nickname_lookup VALUES('4F373129',130);
INSERT INTO nickname_lookup VALUES('4F373145',130);
INSERT INTO nickname_lookup VALUES('4F3731452F454B31',131);
INSERT INTO nickname_lookup VALUES('4F312F',131);
INSERT INTO nickname_lookup VALUES('4F312F2F59',131);
INSERT INTO nickname_lookup VALUES('4F373145',131);
INSERT INTO nickname_lookup VALUES('4F374541294D',132);
INSERT INTO nickname_lookup VALUES('4F4541',132);
INSERT INTO nickname_lookup VALUES('4F374541',132);
INSERT INTO nickname_lookup VALUES('4F45414159',132);
INSERT INTO nickname_lookup VALUES('4F3941454F3759',133);
INSERT INTO nickname_lookup VALUES('4F3941',133);
INSERT INTO nickname_lookup VALUES('4F39414159',133);
INSERT INTO nickname_lookup VALUES('53293F314B3931',134);
INSERT INTO nickname_lookup VALUES('53293F',134);
INSERT INTO nickname_lookup VALUES('53314B4543392D29',135);
INSERT INTO nickname_lookup VALUES('4B4543433931',135);
INSERT INTO nickname_lookup VALUES('4B454339',135);
INSERT INTO nickname_lookup VALUES('43392D29',135);
INSERT INTO nickname_lookup VALUES('43393D3D39',135);
INSERT INTO nickname_lookup VALUES('43393D3D29',135);
INSERT INTO nickname_lookup VALUES('53392D4F454B',136);
INSERT INTO nickname_lookup VALUES('53392D',136);
INSERT INTO nickname_lookup VALUES('53392D4F454B3929',137);
INSERT INTO nickname_lookup VALUES('53392D3D59',137);
INSERT INTO nickname_lookup VALUES('53392D3D39',137);
INSERT INTO nickname_lookup VALUES('53392D3D3931',137);
INSERT INTO nickname_lookup VALUES('4F454B39',137);
INSERT INTO nickname_lookup VALUES('5339432D31434F',138);
INSERT INTO nickname_lookup VALUES('5339432D31',138);
INSERT INTO nickname_lookup VALUES('533943',138);
INSERT INTO nickname_lookup VALUES('533943433931',138);
INSERT INTO nickname_lookup VALUES('533953392943',139);
INSERT INTO nickname_lookup VALUES('53395339',139);
INSERT INTO nickname_lookup VALUES('55293F4F314B',140);
INSERT INTO nickname_lookup VALUES('55293F4F',140);
INSERT INTO nickname_lookup VALUES('55293F3F59',140);
INSERT INTO nickname_lookup VALUES('5531432F59',141);
INSERT INTO nickname_lookup VALUES('553143',141);
INSERT INTO nickname_lookup VALUES('5531432F313F',141);
INSERT INTO nickname_lookup VALUES('55393F3F392941',142);
INSERT INTO nickname_lookup VALUES('2B393F3F',142);
INSERT INTO nickname_lookup VALUES('2B393F3F59',142);
INSERT INTO nickname_lookup VALUES('55393F3F',142);
INSERT INTO nickname_lookup VALUES('55393F3F59',142);
INSERT INTO nickname_lookup VALUES('3F392941',142);
INSERT INTO nickname_lookup VALUES('595345434329',143);
INSERT INTO nickname_lookup VALUES('5345434329',143);
INSERT INTO nickname_lookup VALUES('5B292D37294B59',144);
INSERT INTO nickname_lookup VALUES('5B292D37',144);
INSERT INTO nickname_lookup VALUES('5B292D3D',144);
INSERT INTO nickname_lookup VALUES('5B292D',144);
set echo on;

-- NEXT_SEQUENCE_IS_PROFILE_VALUE
CREATE UNIQUE SEQUENCE groups_id_seq AS BIGINT in contacts2_db;

CREATE TABLE groups (
	"_id" BIGINT default NEXT_VALUE OF groups_id_seq with check option,
	package_id BIGINT,
	account_id BIGINT,
	sourceid NVARCHAR(120),
	version INTEGER NOT NULL DEFAULT 1,
	dirty INTEGER NOT NULL DEFAULT 0,
	title NVARCHAR(120),
	title_res INTEGER,
	notes NVARCHAR(120),
	system_id NVARCHAR(120),
	deleted INTEGER NOT NULL DEFAULT 0,
	group_visible INTEGER NOT NULL DEFAULT 0,
	should_sync INTEGER NOT NULL DEFAULT 1,
        auto_add INTEGER NOT NULL DEFAULT 0,
        favorites INTEGER NOT NULL DEFAULT 0,
        group_is_read_only INTEGER NOT NULL DEFAULT 0,
	sync1 NVARCHAR(120),
	sync2 NVARCHAR(120),
	sync3 NVARCHAR(120),
	sync4 NVARCHAR(120),
        constraint groups_pk PRIMARY KEY ("_id") 
) in contacts2_db;





CREATE UNIQUE SEQUENCE agg_exceptions_seq AS BIGINT in contacts2_db;

CREATE TABLE  agg_exceptions (
	"_id" BIGINT default NEXT_VALUE OF agg_exceptions_seq with check option,
	type INTEGER NOT NULL, 
	raw_contact_id1 BIGINT ,
	raw_contact_id2 BIGINT,
        constraint agg_exceptions_pk PRIMARY KEY ("_id")
) in contacts2_db;


CREATE TABLE settings (
	account_name NVARCHAR(128) NOT NULL,
	account_type NVARCHAR(128) NOT NULL,
	data_set VARCHAR(128),
	ungrouped_visible INTEGER NOT NULL DEFAULT 0,
	should_sync INTEGER NOT NULL DEFAULT 1--,
	-- Removed in Android -4. constraint settings_pk PRIMARY KEY (account_name, account_type)
) in contacts2_db;

-- NEXT_SEQUENCE_IS_PROFILE_VALUE
CREATE UNIQUE SEQUENCE calls_id_seq AS BIGINT in contacts2_db;

CREATE TABLE calls (
	"_id" BIGINT default NEXT_VALUE OF calls_id_seq with check option,
	number NVARCHAR(120),
	presentation INTEGER NOT NULL DEFAULT 1,
	date BIGINT,
	duration BIGINT, 
	type INTEGER,
	"new" INTEGER,
	name NVARCHAR(120),
	numbertype INTEGER,
	numberlabel NVARCHAR(120),
    countryiso VARCHAR(10),
    voicemail_uri VARCHAR(128),
    is_read INTEGER,
    geocoded_location NVARCHAR(128),
    lookup_uri VARCHAR(256),
    matched_number varchar(40),
    normalized_number varchar(40),
    photo_id BIGINT NOT NULL DEFAULT 0,
    formatted_number varchar(40),
    "_data" varchar(512),
    has_content INTEGER,
    mime_type VARCHAR(120),
    source_data VARCHAR(40),
    source_package VARCHAR(40),
    state INTEGER,

    constraint calls_pk PRIMARY KEY ("_id")
) in contacts2_db;



CREATE TABLE status_updates(
	status_update_data_id BIGINT PRIMARY KEY , 
	status NVARCHAR(512),
	status_ts BIGINT,
	status_res_package NVARCHAR(120), 
	status_label INTEGER, 
	status_icon INTEGER
) in contacts2_db;


-- NEXT_SEQUENCE_IS_PROFILE_VALUE
CREATE UNIQUE SEQUENCE stream_item_photos_id_seq AS BIGINT in contacts2_db;
CREATE TABLE stream_item_photos(
        "_id" BIGINT default next_value of stream_item_photos_id_seq with check option,
        stream_item_id BIGINT NOT NULL,
        sort_index INTEGER,
        photo_file_id BIGINT NOT NULL,
        stream_item_photo_sync1 NVARCHAR(128),
        stream_item_photo_sync2 NVARCHAR(128),
        stream_item_photo_sync3 NVARCHAR(128),
        stream_item_photo_sync4 NVARCHAR(128),
	constraint stream_item_photos_pk primary key("_id")
        -- Foreign keys not enforced by SQLite. FOREIGN KEY(stream_item_id) REFERENCES stream_items(_id)
);

-- NEXT_SEQUENCE_IS_PROFILE_VALUE
CREATE UNIQUE SEQUENCE stream_items_id_seq AS BIGINT in contacts2_db;
CREATE TABLE stream_items(
        "_id" BIGINT default next_value of stream_items_id_seq with check option,
        raw_contact_id BIGINT NOT NULL,
        res_package varchar(128),
        icon varchar(128),
        label nvarchar(128),
        text nvarchar(512),
        "timestamp" BIGINT NOT NULL,
        comments nvarchar(512),
        stream_item_sync1 nvarchar(128),
        stream_item_sync2 nvarchar(128),
        stream_item_sync3 nvarchar(128),
        stream_item_sync4 nvarchar(128),
	constraint stream_items_pk primary key("_id")
        -- Foreign keys not enforced by SQLite. FOREIGN KEY(raw_contact_id) REFERENCES raw_contacts(_id)
);


CREATE TABLE visible_contacts(
        "_id" BIGINT,
	constraint visible_contacts_pk PRIMARY KEY("_id")
);

CREATE UNIQUE SEQUENCE voicemail_status_id_seq as BIGINT in contacts2_db;
CREATE TABLE voicemail_status(
        "_id" BIGINT default next_value of voicemail_status_id_seq with check option,
        source_package varchar(128) UNIQUE NOT NULL,
        settings_uri varchar(128),
        voicemail_access_uri varchar(128),
        configuration_state INTEGER,
        data_channel_state INTEGER,
        notification_channel_state INTEGER,
	constraint voicemail_status_pk PRIMARY KEY("_id")
);

CREATE TABLE properties (
	property_key NVARCHAR(120), 
	property_value NVARCHAR(1024),
        constraint properties_pk PRIMARY KEY (property_key)
) in contacts2_db;

insert into properties(property_key, property_value) values('directoryScanComplete', '0');

create unique sequence accounts_id_seq in contacts2_db;
CREATE TABLE accounts (
	"_id" BIGINT default next_value of accounts_id_seq  with check option,
	account_name NVARCHAR(120),
	account_type NVARCHAR(120),
	data_set NVARCHAR(120),
	constraint accounts_id_pk primary key("_id")
) in contacts2_db;

CREATE UNIQUE SEQUENCE sync_state_seq in contacts2_db;

--CREATE TABLE "_sync_state" (
--	"_id" INTEGER PRIMARY KEY DEFAULT NEXT_VALUE OF sync_state_seq,
--	account_name NVARCHAR(120) NOT NULL,
--	account_type NVARCHAR(120) NOT NULL,
--	data VARBINARY(1024),
--	UNIQUE(account_name, account_type) 
--) in contacts2_db;

CREATE TABLE "_sync_state" (
	"_id" BIGINT NOT NULL DEFAULT NEXT_VALUE OF sync_state_seq with check option,
	account_name NVARCHAR(120) NOT NULL,
	account_type NVARCHAR(120) NOT NULL,
	data VARBINARY(2048),
	constraint sync_state_pk PRIMARY KEY(account_name, account_type),
	constraint sync_state_unique_id UNIQUE("_id")
) in contacts2_db;

CREATE TABLE "_sync_state_metadata" (
	version INTEGER
) in contacts2_db;

INSERT INTO "_sync_state_metadata"(version) VALUES(1);

--INSERT INTO accounts VALUES(NULL, NULL);

CREATE UNIQUE SEQUENCE v1_settings_seq as BIGINT in contacts2_db;

CREATE TABLE v1_settings (
	"_id" BIGINT default next_value of v1_settings_seq with check option,
	"_sync_account" NVARCHAR(120),
	"_sync_account_type" NVARCHAR(120),
	key NVARCHAR(120) NOT NULL,
	"value" NVARCHAR(255), -- This must be text column for the CTS test to work. For some reason it was: "value" INTEGER NOT NULL DEFAULT 1,
	constraint v1_settings_pk PRIMARY KEY ("_id") 
) in contacts2_db;


-- ###############################################################################
-- FTS (Free Text Search) tables. This is implemented in another way, 
-- this is for rerence only
-- ###############################################################################
-- CREATE VIRTUAL TABLE search_index USING FTS4 (contact_id INTEGER REFERENCES contacts(_id) NOT NULL,
-- content TEXT, name TEXT, tokens TEXT);
-- CREATE TABLE 'search_index_content'
--     (
--         docid INTEGER PRIMARY KEY,
--         'c0contact_id',
--         'c1content',
--         'c2name',
--         'c3tokens'
--     );
-- CREATE TABLE 'search_index_docsize'
--     (
--         docid INTEGER PRIMARY KEY,
--         size BLOB
--     );
-- CREATE TABLE 'search_index_segdir'
--     (
--         level INTEGER,
--         idx INTEGER,
--         start_block INTEGER,
--         leaves_end_block INTEGER,
--         end_block INTEGER,
--         root BLOB,
--         PRIMARY KEY(level, idx)
--     );
-- CREATE TABLE 'search_index_segments'
--     (
--         blockid INTEGER PRIMARY KEY,
--         block BLOB
--     );
-- CREATE TABLE 'search_index_stat'
--     (
--         id INTEGER PRIMARY KEY,
--         value BLOB
--     );

-- ##############################################################################
-- End of FTS tables
-- ##############################################################################

-- I have created this table because it appears that this table beahaves just like
-- any other table. The system inserts rows of data into it which should be
-- indexed. If this table does not exists, these inserts will fail. The column
-- lengths here are somwehat arbitrary but should at least be at least as large as 
-- the columns which should be indexed (such as name, title etc) in the contacts table.
CREATE TABLE search_index (
	contact_id BIGINT NOT NULL,
	"content" nvarchar(2000), 
	"name" nvarchar(2000),
	"tokens" nvarchar(512)
--constraint search_index_contacts_fk foreign key(contact_id) REFERENCES contacts("_id")
)  
in contacts2_db;
create index search_index_contact_id_idx on search_index(contact_id);
-- Word search index used for contacs search.
create index search_index_content_idx on search_index(content for word_search);
create index search_index_name_idx on search_index(name for word_search);
create index search_index_tokens_idx on search_index(tokens for word_search);

-- ###############################################################################
-- Indexes 
-- ###############################################################################
CREATE INDEX aggregation_exception_index1 ON agg_exceptions (raw_contact_id1, raw_contact_id2); -- rm unique

CREATE INDEX aggregation_exception_index2 ON agg_exceptions (raw_contact_id2, raw_contact_id1); -- rm unique

CREATE INDEX contacts_contact_last_updated_timestamp_index ON contacts(contact_last_updated_timestamp);
CREATE INDEX deleted_contacts_contact_deleted_timestamp_index ON deleted_contacts(contact_deleted_timestamp);


CREATE INDEX contacts_has_phone_index ON contacts (has_phone_number);

CREATE INDEX data_mimetype_data1_index ON data (mimetype_id,data1);

create index "data_mimetype_data1_ci_index" on "data"(
     "mimetype_id" ASC,
     "data1" collate CURRENT_COLLATION_2
);

CREATE INDEX data_raw_contact_id ON data (raw_contact_id);

CREATE UNIQUE INDEX data_usage_stat_index ON data_usage_stat(data_id, usage_type);


CREATE INDEX name_lookup_index ON name_lookup (normalized_name,name_type, raw_contact_id);
-- 23 CREATE INDEX name_lookup_index ON name_lookup (normalized_name,name_type, raw_contact_id, data_id);

CREATE INDEX name_lookup_raw_contact_id_index ON name_lookup (raw_contact_id);
CREATE INDEX name_lookup_data_id_index on name_lookup(data_id);

--CREATE UNIQUE INDEX nickname_lookup_index ON nickname_lookup (name, cluster);
CREATE UNIQUE INDEX nickname_lookup_index ON nickname_lookup (name, cluster);

CREATE INDEX phone_lookup_data_id_min_match_index ON phone_lookup(data_id, min_match);

CREATE INDEX phone_lookup_index ON phone_lookup (normalized_number,raw_contact_id,data_id);

CREATE INDEX raw_contacts_contact_id_index ON raw_contacts (contact_id);



--new in 23
CREATE INDEX contacts_name_raw_contact_id_index ON contacts (name_raw_contact_id);
CREATE UNIQUE INDEX mime_type ON mimetypes (mimetype);
CREATE INDEX phone_lookup_min_match_index ON phone_lookup (min_match,raw_contact_id,data_id);
CREATE INDEX raw_contact_sort_key1_index ON raw_contacts (sort_key);
CREATE INDEX raw_contact_sort_key2_index ON raw_contacts (sort_key_alt);

-- new in 41
CREATE INDEX raw_contacts_source_id_account_id_index ON raw_contacts(sourceid, account_id);
CREATE INDEX groups_source_id_account_id_index ON groups (sourceid, account_id);

--Extra index for performance optimizations in Android 4.3
create index raw_contacts_buck_key_idx on raw_contacts( phonebook_bucket,sort_key, display_name, account_id);

-- Pinyin index for search.

--CREATE INDEX raw_contacts_display_name_pinyin ON raw_contacts (display_name for pinyin_start);

-- ###############################################################################
-- Functions specific to Contacts
-- ###############################################################################
@
create function "_PHONE_NUMBER_STRIPPED_REVERSED"(phone nvarchar(128))
returns nvarchar(128)
deterministic
begin
declare have_seen_plus boolean default false;
declare in_len integer;
declare res varchar(128);
declare c char(1);
set in_len = CHAR_LENGTH(phone);
set res = '';

while in_len >= 1 do
	set c = substring(phone, in_len, 1);
	if ((c >= '0' and c <= '9') or c = '*' or c = '#' or c = 'N') then
		set res = res || c;
	else
		case 
			when c = '+' then
				if have_seen_plus = false then
					set res = res || c;
					set have_seen_plus = true;
				end if;
			when c = ',' or c = ';' then
			     set res = '';
			else
			     set res = res || '';
		end case;
	end if;
        set in_len = in_len - 1;
end while;
return res;
end
@


--bool phone_number_stripped_reversed_inter(const char* in, char* out, const int len, int *outlen) {
--    int in_len = strlen(in);
--    int out_len = 0;
--    bool have_seen_plus = false;
--    for (int i = in_len; --i >= 0;) {
--        char c = in[i];
--        if ((c >= '0' && c <= '9') || c == '*' || c == '#' || c == 'N') {
--            if (out_len < len) {
--                out[out_len++] = c;
--            }
--        } else {
--            switch (c) {
--              case '+':
--                  if (!have_seen_plus) {
--                      if (out_len < len) {
--                          out[out_len++] = c;
--                      }
--                      have_seen_plus = true;
--                  }
--                  break;
--              case ',':
--              case ';':
--                  out_len = 0;
--                  break;
--          }
--        }
--    }
--
--    *outlen = out_len;
--    return true;
--}


-- ###############################################################################
-- Views 
-- ###############################################################################



create view name_lookup_v as select data_id, raw_contact_id, normalized_name, name_type from name_lookup {INDEX name_lookup_pk};

CREATE VIEW view_data AS 
SELECT 	data."_id" AS "_id",
	raw_contact_id, 
	raw_contacts.contact_id AS contact_id, 
	raw_contacts.account_id,
	accounts.account_name AS account_name,
	accounts.account_type AS account_type,
	accounts.data_set AS data_set,
    	(
        CASE
            WHEN accounts.data_set IS NULL
            THEN accounts.account_type
            ELSE accounts.account_type || '/' ||accounts.data_set
        END) AS account_type_and_data_set,
	raw_contacts.sourceid AS sourceid,
	raw_contacts.name_verified AS name_verified,
	raw_contacts.version AS version,
	raw_contacts.dirty AS dirty,
	raw_contacts.sync1 AS sync1,
	raw_contacts.sync2 AS sync2,
	raw_contacts.sync3 AS sync3,
	raw_contacts.sync4 AS sync4, 
	is_primary, 
	is_super_primary, 
	data_version,
	data.package_id,
	package AS res_package,
	data.mimetype_id,
	mimetype AS mimetype,
	is_read_only,
	data1, 
	data2, 
	data3, 
	data4, 
	data5, 
	data6, 
	data7, 
	data8, 
	data9, 
	data10, 
	data11, 
	data12, 
	data13, 
	data14, 
	data15, 
	data_sync1, 
	data_sync2, 
	data_sync3, 
	data_sync4,
	contacts.custom_ringtone AS custom_ringtone,
	contacts.send_to_voicemail AS send_to_voicemail,
	contacts.last_time_contacted AS last_time_contacted,
	contacts.times_contacted AS times_contacted,
	contacts.pinned AS pinned,
	contacts.starred AS starred,
	name_raw_contact.display_name_source AS display_name_source, 
	name_raw_contact.display_name AS display_name, 
	name_raw_contact.display_name_alt AS display_name_alt, 
	name_raw_contact.phonetic_name AS phonetic_name, 
	name_raw_contact.phonetic_name_style AS phonetic_name_style, 
	name_raw_contact.sort_key AS sort_key,
    name_raw_contact.phonebook_label      AS phonebook_label,
    name_raw_contact.phonebook_bucket     AS phonebook_bucket,
	name_raw_contact.sort_key_alt AS sort_key_alt,
    name_raw_contact.phonebook_label_alt  AS phonebook_label_alt,
    name_raw_contact.phonebook_bucket_alt AS phonebook_bucket_alt,
	has_phone_number,
	name_raw_contact_id,
	lookup,
	photo_id,
	photo_file_id,
	case when (EXISTS
		(SELECT "_id"
        	FROM visible_contacts
        	WHERE contacts."_id"=visible_contacts."_id")) then 1 else 0 end  AS in_visible_group,
	status_update_id,
	contacts.contact_last_updated_timestamp,
	(
        CASE
            WHEN photo_file_id IS NULL
            THEN (
                    CASE
                        WHEN photo_id IS NULL
                        OR  photo_id=0
                        THEN cast(NULL as varchar(20))
                        ELSE 'content://com.android.contacts/contacts/'|| cast(raw_contacts.contact_id as varchar(20)) ||
                            '/photo'
                    END)
            ELSE 'content://com.android.contacts/display_photo/'|| cast(photo_file_id as varchar(20))
        END) AS photo_uri,
	(
        CASE
            WHEN photo_id IS NULL
            OR  photo_id=0
            THEN cast(NULL as varchar(20))
            ELSE 'content://com.android.contacts/contacts/'|| cast(raw_contacts.contact_id as varchar(20))|| '/photo'
        END)        AS photo_thumb_uri,
-- NEXT_ZERO_IS_PROFILE_VALUE 
    0               AS raw_contact_is_user_profile,
	groups.sourceid AS group_sourceid 
FROM data JOIN mimetypes ON (data.mimetype_id=mimetypes."_id") 
JOIN raw_contacts ON (data.raw_contact_id=raw_contacts."_id") 
JOIN accounts ON (raw_contacts.account_id=accounts."_id")
JOIN contacts ON (raw_contacts.contact_id=contacts."_id") 
JOIN raw_contacts AS name_raw_contact ON(name_raw_contact_id=name_raw_contact."_id") 
LEFT OUTER JOIN packages ON (data.package_id=packages."_id")
LEFT OUTER JOIN groups ON (
	mimetypes.mimetype='vnd.android.cursor.item/group_membership' 
	AND groups."_id" = data.data1
);



CREATE VIEW view_data_usage_stat AS
SELECT data_usage_stat.stat_id AS stat_id,
    data_id,
    raw_contacts.contact_id AS contact_id,
    mimetypes.mimetype AS mimetype,
    usage_type,
    times_used,
    last_time_used
FROM data_usage_stat
JOIN data
ON  (
        data."_id"=data_usage_stat.data_id)
JOIN raw_contacts
ON  (
        raw_contacts."_id"=data.raw_contact_id )
JOIN mimetypes
ON  (
        mimetypes."_id"=data.mimetype_id);


CREATE VIEW view_entities      AS
SELECT raw_contacts.contact_id AS "_id",
    raw_contacts.contact_id    AS contact_id,
    raw_contacts.deleted       AS deleted,
    is_primary,
    is_super_primary,
    data_version,
    data.package_id,
    PACKAGE AS res_package,
    data.mimetype_id,
    mimetype AS mimetype,
    is_read_only,
    data1,
    data2,
    data3,
    data4,
    data5,
    data6,
    data7,
    data8,
    data9,
    data10,
    data11,
    data12,
    data13,
    data14,
    data15,
    data_sync1,
    data_sync2,
    data_sync3,
    data_sync4,
    raw_contacts.account_id,
    accounts.account_name AS account_name,
    accounts.account_type AS account_type,
    accounts.data_set     AS data_set,
    (
        CASE
            WHEN accounts.data_set IS NULL
            THEN accounts.account_type
            ELSE accounts.account_type||'/'||accounts.data_set
        END
    )                                    AS account_type_and_data_set,
    raw_contacts.sourceid                AS sourceid,
    raw_contacts.name_verified           AS name_verified,
    raw_contacts.version                 AS version,
    raw_contacts.dirty                   AS dirty,
    raw_contacts.sync1                   AS sync1,
    raw_contacts.sync2                   AS sync2,
    raw_contacts.sync3                   AS sync3,
    raw_contacts.sync4                   AS sync4,
    contacts.custom_ringtone             AS custom_ringtone,
    name_raw_contact.display_name_source AS display_name_source,
    name_raw_contact.display_name        AS display_name,
    name_raw_contact.display_name_alt    AS display_name_alt,
    name_raw_contact.phonetic_name       AS phonetic_name,
    name_raw_contact.phonetic_name_style AS phonetic_name_style,
    name_raw_contact.sort_key            AS sort_key,
    name_raw_contact.phonebook_label      AS phonebook_label,
    name_raw_contact.phonebook_bucket     AS phonebook_bucket,
	name_raw_contact.sort_key_alt AS sort_key_alt,
    name_raw_contact.phonebook_label_alt  AS phonebook_label_alt,
    name_raw_contact.phonebook_bucket_alt AS phonebook_bucket_alt,
    has_phone_number,
    name_raw_contact_id,
    lookup,
    photo_id,
    photo_file_id,
    case when (EXISTS
		(SELECT "_id"
        	FROM visible_contacts
        	WHERE contacts."_id"=visible_contacts."_id")) then 1 else 0 end  AS in_visible_group,
    status_update_id,
	contacts.contact_last_updated_timestamp,
    contacts.last_time_contacted AS last_time_contacted,
    contacts.send_to_voicemail   AS send_to_voicemail,
    contacts.starred             AS starred,
    contacts.times_contacted     AS times_contacted,
    contacts.pinned 			 AS pinned,
    (
        CASE
            WHEN photo_file_id IS NULL
            THEN (
                    CASE
                        WHEN photo_id IS NULL
                        OR  photo_id=0
                        THEN NULL
                        ELSE 'content://com.android.contacts/contacts/'||cast(raw_contacts.contact_id as varchar(20))||
                            '/photo'
                    END)
            ELSE 'content://com.android.contacts/display_photo/'||cast(photo_file_id  as varchar(20))
        END) AS photo_uri,(
        CASE
            WHEN photo_id IS NULL
            OR  photo_id=0
            THEN NULL
            ELSE 'content://com.android.contacts/contacts/'||cast(raw_contacts.contact_id as varchar(20))|| '/photo'
        END
    )          AS photo_thumb_uri,
-- NEXT_ZERO_IS_PROFILE_VALUE 
    0          AS is_user_profile,
    data_sync1 as data2_sync1,
    data_sync2 as data2_sync2,
    data_sync3 as data2_sync3,
    data_sync4 as data2_sync4, 
    raw_contacts."_id" AS raw_contact_id,
    data."_id"         AS data_id,
    groups.sourceid  AS group_sourceid
FROM raw_contacts
JOIN accounts 
ON  (
	raw_contacts.account_id=accounts."_id")
JOIN contacts
ON  (
        raw_contacts.contact_id=contacts."_id")
JOIN raw_contacts AS name_raw_contact
ON  (
        name_raw_contact_id=name_raw_contact."_id")
LEFT OUTER JOIN data
ON  (
        data.raw_contact_id=raw_contacts."_id")
LEFT OUTER JOIN packages
ON  (
        data.package_id=packages."_id")
LEFT OUTER JOIN mimetypes
ON  (
        data.mimetype_id=mimetypes."_id")
LEFT OUTER JOIN groups
ON  (
        mimetypes.mimetype='vnd.android.cursor.item/group_membership'
    AND groups."_id" = data.data1);



CREATE VIEW view_contacts AS 
SELECT  
	contacts."_id" AS "_id",
	contacts.custom_ringtone AS custom_ringtone,
	name_raw_contact.display_name_source AS display_name_source,
	name_raw_contact.display_name AS display_name,
	name_raw_contact.display_name_alt AS display_name_alt,
	name_raw_contact.phonetic_name AS phonetic_name,
	name_raw_contact.phonetic_name_style AS phonetic_name_style,
	name_raw_contact.sort_key AS sort_key,
    name_raw_contact.phonebook_label      AS phonebook_label,
    name_raw_contact.phonebook_bucket     AS phonebook_bucket,
	name_raw_contact.sort_key_alt AS sort_key_alt,
    name_raw_contact.phonebook_label_alt  AS phonebook_label_alt,
    name_raw_contact.phonebook_bucket_alt AS phonebook_bucket_alt,
	has_phone_number,
	name_raw_contact_id,
	lookup,
	photo_id,
    	photo_file_id,
    	case when (EXISTS
		(SELECT "_id"
        	FROM visible_contacts
        	WHERE contacts."_id"=visible_contacts."_id")) then 1 else 0 end  AS in_visible_group,
    	status_update_id,
    	contacts.contact_last_updated_timestamp,
	contacts.last_time_contacted AS last_time_contacted,
	contacts.send_to_voicemail AS send_to_voicemail,
	contacts.pinned AS pinned,
	contacts.starred AS starred,
	contacts.times_contacted AS times_contacted,
	(
        CASE
            WHEN photo_file_id IS NULL
            THEN (
                    CASE
                        WHEN photo_id IS NULL
                        OR  photo_id=0
                        THEN NULL
                        ELSE 'content://com.android.contacts/contacts/'||cast(contacts."_id" as varchar(20))|| '/photo'
                    END)
            ELSE 'content://com.android.contacts/display_photo/'||cast(photo_file_id as varchar(20))
        END) AS photo_uri,
    	(
        CASE
            WHEN photo_id IS NULL
            OR  photo_id=0
            THEN NULL
            ELSE 'content://com.android.contacts/contacts/'||cast(contacts."_id" as varchar(20))|| '/photo'
        END) AS photo_thumb_uri,
-- NEXT_ZERO_IS_PROFILE_VALUE 
    	0 AS is_user_profile --,
--        name_raw_contact.sectionIndex as sectionIndex
FROM contacts
JOIN raw_contacts AS name_raw_contact ON(name_raw_contact_id=name_raw_contact."_id")
;


CREATE VIEW view_groups AS 
SELECT 	groups."_id" AS "_id",
	groups.account_id AS account_id,
	accounts.account_name AS account_name,
	accounts.account_type AS account_type,
    	accounts.data_set AS data_set,
	(
        CASE
            WHEN accounts.data_set IS NULL
            THEN accounts.account_type
            ELSE accounts.account_type||'/'||accounts.data_set
        END) AS account_type_and_data_set,
	sourceid,
	version,
	dirty,
	title,
	title_res,
	notes,
	system_id,
	deleted,
	group_visible,
	should_sync,
	auto_add,
	favorites,
	group_is_read_only,
	sync1,
	sync2,
	sync3,
	sync4,
	package AS res_package 
FROM groups 
JOIN accounts 
ON (groups.account_id=accounts."_id")
LEFT OUTER JOIN packages 
ON (groups.package_id = packages."_id");

CREATE VIEW view_raw_contacts AS 
SELECT 
	raw_contacts."_id" AS "_id",
	contact_id,
	aggregation_mode,
	raw_contact_is_read_only,
	deleted,
	display_name_source, 
	display_name, 
	display_name_alt, 
	phonetic_name, 
	phonetic_name_style, 
	sort_key, 
    phonebook_label,
    phonebook_bucket,
	sort_key_alt,
    phonebook_label_alt,
    phonebook_bucket_alt,
-- NEXT_ZERO_IS_PROFILE_VALUE 
	0 AS raw_contact_is_user_profile,
	custom_ringtone,
	send_to_voicemail,
	last_time_contacted,
	times_contacted,
	starred,
    pinned,
	raw_contacts.account_id,
	accounts.account_name AS account_name,
	accounts.account_type AS account_type,
    	accounts.data_set     AS data_set,
	(
        CASE
            WHEN accounts.data_set IS NULL
            THEN accounts.account_type
            ELSE accounts.account_type||'/'||accounts.data_set
        END) AS account_type_and_data_set,
	raw_contacts.sourceid AS sourceid,
	raw_contacts.name_verified AS name_verified,
	raw_contacts.version AS version,
	raw_contacts.dirty AS dirty,
	raw_contacts.sync1 AS sync1,
	raw_contacts.sync2 AS sync2,
	raw_contacts.sync3 AS sync3,
	raw_contacts.sync4 AS sync4 
FROM raw_contacts
JOIN accounts 
ON (raw_contacts.account_id=accounts."_id")
;


CREATE VIEW view_raw_entities AS
SELECT contact_id,
    raw_contacts.deleted AS deleted,
    is_primary,
    is_super_primary,
    data_version,
    data.package_id,
    PACKAGE AS res_package,
    data.mimetype_id,
    mimetype AS mimetype,
    is_read_only,
    data1,
    data2,
    data3,
    data4,
    data5,
    data6,
    data7,
    data8,
    data9,
    data10,
    data11,
    data12,
    data13,
    data14,
    data15,
    data_sync1,
    data_sync2,
    data_sync3,
    data_sync4,
    raw_contacts.account_id,
    accounts.account_name AS account_name,
    accounts.account_type AS account_type,
    accounts.data_set     AS data_set,(
        CASE
            WHEN accounts.data_set IS NULL
            THEN accounts.account_type
            ELSE accounts.account_type||'/'||accounts.data_set
        END)                   AS account_type_and_data_set,
    raw_contacts.sourceid      AS sourceid,
    raw_contacts.name_verified AS name_verified,
    raw_contacts.version       AS version,
    raw_contacts.dirty         AS dirty,
    raw_contacts.sync1         AS sync1,
    raw_contacts.sync2         AS sync2,
    raw_contacts.sync3         AS sync3,
    raw_contacts.sync4         AS sync4,
    data_sync1 as data2_sync1,
    data_sync2 as data2_sync2,
    data_sync3 as data2_sync3,
    data_sync4 as data2_sync4,
    raw_contacts."_id"     AS "_id",
    data."_id"             AS data_id,
    raw_contacts.starred AS starred,
-- NEXT_ZERO_IS_PROFILE_VALUE 
    0                    AS raw_contact_is_user_profile,
    groups.sourceid      AS group_sourceid
FROM raw_contacts
JOIN accounts 
ON (
	raw_contacts.account_id=accounts."_id")
LEFT OUTER JOIN data
ON  (
        data.raw_contact_id=raw_contacts."_id")
LEFT OUTER JOIN packages
ON  (
        data.package_id=packages."_id")
LEFT OUTER JOIN mimetypes
ON  (
        data.mimetype_id=mimetypes."_id")
LEFT OUTER JOIN groups
ON  (
        mimetypes.mimetype='vnd.android.cursor.item/group_membership'
    AND groups."_id" = data.data1);



CREATE VIEW view_stream_items AS
SELECT stream_items."_id",
    contacts."_id"    AS contact_id,
    contacts.lookup AS contact_lookup,
    accounts.account_name,
    accounts.account_type,
    accounts.data_set,
    stream_items.raw_contact_id AS raw_contact_id,
    raw_contacts.sourceid       AS raw_contact_source_id,
    stream_items.res_package,
    stream_items.icon,
    stream_items.label,
    stream_items.text,
    stream_items."timestamp",
    stream_items.comments,
    stream_items.stream_item_sync1,
    stream_items.stream_item_sync2,
    stream_items.stream_item_sync3,
    stream_items.stream_item_sync4
FROM stream_items
JOIN raw_contacts
ON  (
        stream_items.raw_contact_id=raw_contacts."_id")
JOIN accounts 
ON (
	raw_contacts.account_id=accounts."_id")
JOIN contacts
ON  (
        raw_contacts.contact_id=contacts."_id");


CREATE VIEW view_v1_people AS 
SELECT 	raw_contacts."_id" AS "_id",
	name.data1 AS name,
	raw_contacts.display_name AS display_name,
	trim(trim(COALESCE(name.data7,	' ')||' '||COALESCE(name.data8,	' '))||' '||COALESCE(name.data9,' '))  AS phonetic_name ,
	note.data1 AS notes,
	accounts.account_name,
	accounts.account_type,
	raw_contacts.times_contacted AS times_contacted,

	raw_contacts.last_time_contacted AS last_time_contacted,
	raw_contacts.custom_ringtone AS custom_ringtone,
	raw_contacts.send_to_voicemail AS send_to_voicemail,
	raw_contacts.starred AS starred,
	organization."_id" AS primary_organization,
	email."_id" AS primary_email,
	phone."_id" AS primary_phone,
	phone.data1 AS number,
	phone.data2 AS type,
	phone.data3 AS label,
	"_PHONE_NUMBER_STRIPPED_REVERSED"(phone.data1) AS number_key 
FROM raw_contacts 
JOIN accounts ON (raw_contacts.account_id=accounts."_id")
LEFT OUTER JOIN data name 
	ON (	raw_contacts."_id" = name.raw_contact_id 
		AND (SELECT mimetype FROM mimetypes WHERE mimetypes."_id" = name.mimetype_id)='vnd.android.cursor.item/name'
	) 
LEFT OUTER JOIN data organization 
	ON (	raw_contacts."_id" = organization.raw_contact_id 
		AND (SELECT mimetype FROM mimetypes 
			WHERE mimetypes."_id" = organization.mimetype_id)='vnd.android.cursor.item/organization' 
		AND organization.is_primary
	)
LEFT OUTER JOIN data email 
	ON (	raw_contacts."_id" = email.raw_contact_id 
		AND (SELECT mimetype FROM mimetypes 
			WHERE mimetypes."_id" = email.mimetype_id)='vnd.android.cursor.item/email_v2' 
		AND email.is_primary
	) 
LEFT OUTER JOIN data note 
	ON (	raw_contacts."_id" = note.raw_contact_id 
		AND (SELECT mimetype FROM mimetypes 
			WHERE mimetypes."_id" = note.mimetype_id)='vnd.android.cursor.item/note'
	) 
LEFT OUTER JOIN data phone 
ON (	raw_contacts."_id" = phone.raw_contact_id 
	AND (SELECT mimetype FROM mimetypes 
		WHERE mimetypes."_id" = phone.mimetype_id)='vnd.android.cursor.item/phone_v2' 
	AND phone.is_primary
) 
WHERE raw_contacts.deleted=0;



CREATE VIEW view_v1_organizations AS SELECT data."_id" AS "_id",
	 raw_contact_id AS person,
	 is_primary AS isprimary,
	 accounts.account_name,
	 accounts.account_type,
	 data1 AS company,
	 data2 AS type,
	 data3 AS label,
	 data4 AS title 
FROM data 
JOIN mimetypes ON (data.mimetype_id = mimetypes."_id") 
JOIN raw_contacts ON (data.raw_contact_id = raw_contacts."_id") 
JOIN accounts ON (raw_contacts.account_id=accounts."_id")
WHERE mimetypes.mimetype='vnd.android.cursor.item/organization' 
AND raw_contacts.deleted=0;


CREATE VIEW view_v1_extensions AS
SELECT data."_id" AS "_id", 
	data.raw_contact_id AS person, 
	accounts.account_name, 
	accounts.account_type, 
	data1 AS name, 
	data2 AS "value" 
FROM data 
JOIN mimetypes 
ON (data.mimetype_id = mimetypes."_id") 
JOIN raw_contacts 
ON (data.raw_contact_id = raw_contacts."_id") 
JOIN accounts 
ON (raw_contacts.account_id=accounts."_id") 
WHERE mimetypes.mimetype='vnd.android.cursor.item/contact_extensions' 
	AND raw_contacts.deleted=0;


CREATE VIEW view_v1_contact_methods AS 
SELECT 	data."_id" AS "_id",
	data.raw_contact_id AS person,
	CAST ((CASE WHEN mimetype='vnd.android.cursor.item/email_v2' THEN 1 ELSE (CASE WHEN mimetype='vnd.android.cursor.item/im' THEN 3 ELSE (CASE WHEN mimetype='vnd.android.cursor.item/postal-address_v2' THEN 2 ELSE NULL END) END) END) AS INTEGER) AS kind,
	data.is_primary AS isprimary,
	(CASE WHEN mimetype='vnd.android.cursor.item/im' THEN (CASE WHEN  + cast(data.data5 AS integer)=-1 THEN 'custom:'||data.data6 ELSE 'pre:'||data.data5 END) ELSE data.data1 END) AS data,
	data.data14 AS aux_data,
	name.data1 AS name,
	raw_contacts.display_name AS display_name,
	 trim(trim(COALESCE(name.data7,' ')||' '||COALESCE(name.data8,' '))||' '||COALESCE(name.data9,' '))  AS phonetic_name ,
	note.data1 AS notes,
	accounts.account_name,
	accounts.account_type,
	raw_contacts.times_contacted AS times_contacted,
	raw_contacts.last_time_contacted AS last_time_contacted,
	raw_contacts.custom_ringtone AS custom_ringtone,
	raw_contacts.send_to_voicemail AS send_to_voicemail,
	raw_contacts.starred AS starred,
	organization."_id" AS primary_organization,
	email."_id" AS primary_email,
	phone."_id" AS primary_phone,
	phone.data1 AS number,
	data.data2 AS type,
	data.data3 AS label,
	"_PHONE_NUMBER_STRIPPED_REVERSED"(phone.data1) AS number_key 
FROM data 
JOIN mimetypes ON (mimetypes."_id" = data.mimetype_id) 
JOIN raw_contacts ON (raw_contacts."_id" = data.raw_contact_id) 
JOIN accounts ON (raw_contacts.account_id=accounts."_id")
LEFT OUTER JOIN data name ON (
	raw_contacts."_id" = name.raw_contact_id 
	AND (SELECT mimetype FROM mimetypes WHERE mimetypes."_id" = name.mimetype_id)='vnd.android.cursor.item/name'
) 
LEFT OUTER JOIN data organization ON (
	raw_contacts."_id" = organization.raw_contact_id 
	AND (	SELECT mimetype FROM mimetypes 
		WHERE mimetypes."_id" = organization.mimetype_id)='vnd.android.cursor.item/organization' 
	AND organization.is_primary
) 
LEFT OUTER JOIN data email ON (
	raw_contacts."_id" = email.raw_contact_id 
	AND (	SELECT mimetype FROM mimetypes 
		WHERE mimetypes."_id" = email.mimetype_id)='vnd.android.cursor.item/email_v2' 
	AND email.is_primary
) 
LEFT OUTER JOIN data note ON (
	raw_contacts."_id" = note.raw_contact_id 
	AND (SELECT mimetype FROM mimetypes WHERE mimetypes."_id" = note.mimetype_id)='vnd.android.cursor.item/note'
) 
LEFT OUTER JOIN data phone ON (
	raw_contacts."_id" = phone.raw_contact_id 
	AND (SELECT mimetype FROM mimetypes WHERE mimetypes."_id" = phone.mimetype_id)='vnd.android.cursor.item/phone_v2' 
	AND phone.is_primary
) 
WHERE CAST ((CASE WHEN mimetype='vnd.android.cursor.item/email_v2' THEN 1 ELSE (CASE WHEN mimetype='vnd.android.cursor.item/im' THEN 3 ELSE (CASE WHEN mimetype='vnd.android.cursor.item/postal-address_v2' THEN 2 ELSE NULL END) END) END) AS INTEGER) IS NOT NULL 
AND raw_contacts.deleted=0;


CREATE VIEW view_v1_phones            AS
SELECT DISTINCT data."_id"            AS "_id",
    data.raw_contact_id               AS person,
    data.is_primary                   AS isprimary,
    data.data1                        AS "NUMBER",
    data.data2                        AS type,
    data.data3                        AS label,
    "_PHONE_NUMBER_STRIPPED_REVERSED"(data.data1)    AS number_key,
    name.data1                        AS name,
    raw_contacts.display_name         AS display_name,
    trim(trim(COALESCE(name.data7,
	' ')||' '||COALESCE(name.data8,
	' '))||' '||COALESCE(name.data9,
	' '))                        AS phonetic_name,
    note.data1                       AS notes,
    accounts.account_name,
    accounts.account_type,
    raw_contacts.times_contacted     AS times_contacted,
    raw_contacts.last_time_contacted AS last_time_contacted,
    raw_contacts.custom_ringtone     AS custom_ringtone,
    raw_contacts.send_to_voicemail   AS send_to_voicemail,
    raw_contacts.starred             AS starred,
    organization."_id"               AS primary_organization,
    email."_id"                      AS primary_email,
    phone."_id"                      AS primary_phone,
    phone.data1                      AS "PHONE_NUMBER",
    phone.data2                      AS phone_type,
    phone.data3                      AS phone_label,
    "_PHONE_NUMBER_STRIPPED_REVERSED"(phone.data1) AS phone_number_key
FROM data
JOIN phone_lookup
ON  (
        data."_id" = phone_lookup.data_id)
JOIN mimetypes
ON  (
        mimetypes."_id" = data.mimetype_id)
JOIN raw_contacts
ON  (
        raw_contacts."_id" = data.raw_contact_id)
JOIN accounts 
ON (
	raw_contacts.account_id=accounts."_id")
LEFT OUTER JOIN data name
ON  (
        raw_contacts."_id" = name.raw_contact_id
    AND (SELECT mimetype
            FROM mimetypes
            WHERE mimetypes."_id" = name.mimetype_id)='vnd.android.cursor.item/name')
LEFT OUTER JOIN data organization
ON  (
        raw_contacts."_id" = organization.raw_contact_id
    AND (SELECT mimetype
            FROM mimetypes
            WHERE mimetypes."_id" = organization.mimetype_id)='vnd.android.cursor.item/organization'
    AND organization.is_primary)
LEFT OUTER JOIN data email
ON  (
        raw_contacts."_id" = email.raw_contact_id
    AND (SELECT mimetype
            FROM mimetypes
            WHERE mimetypes."_id" = email.mimetype_id)='vnd.android.cursor.item/email_v2'
    AND email.is_primary)
LEFT OUTER JOIN data note
ON  (
        raw_contacts."_id" = note.raw_contact_id
    AND (SELECT mimetype
            FROM mimetypes
            WHERE mimetypes."_id" = note.mimetype_id)='vnd.android.cursor.item/note')
LEFT OUTER JOIN data phone
ON  (
        raw_contacts."_id" = phone.raw_contact_id
    AND (SELECT mimetype
            FROM mimetypes
            WHERE mimetypes."_id" = phone.mimetype_id)='vnd.android.cursor.item/phone_v2'
    AND phone.is_primary)
WHERE mimetypes.mimetype='vnd.android.cursor.item/phone_v2'
AND raw_contacts.deleted=0;


CREATE VIEW view_v1_groups AS SELECT groups."_id" AS "_id",
	 accounts.account_name,
	 accounts.account_type,
	 title AS name,
	 notes AS notes ,
	 system_id AS system_id 
FROM groups
JOIN accounts ON (groups.account_id=accounts."_id");


CREATE VIEW view_v1_group_membership AS SELECT data."_id" AS "_id",
	 data.raw_contact_id AS person,
	 accounts.account_name AS account_name,
	 accounts.account_type AS account_type,
	 data1 AS group_id,
	 title AS name,
	 notes AS notes,
	 system_id AS system_id,
	 groups.sourceid AS group_sync_id,
	 accounts.account_name AS group_sync_account,
	 accounts.account_type AS group_sync_account_type 
FROM data 
JOIN mimetypes ON (data.mimetype_id = mimetypes."_id") 
JOIN raw_contacts ON (data.raw_contact_id = raw_contacts."_id") 
JOIN accounts ON (raw_contacts.account_id=accounts."_id")
LEFT OUTER JOIN packages ON (data.package_id = packages."_id") 
LEFT OUTER JOIN groups ON (
	mimetypes.mimetype='vnd.android.cursor.item/group_membership'
	AND groups."_id" = data.data1
)  
WHERE mimetypes.mimetype='vnd.android.cursor.item/group_membership' 
AND raw_contacts.deleted=0;


CREATE VIEW view_v1_photos AS SELECT data."_id" AS "_id",
	 data.raw_contact_id AS person,
	 accounts.account_name,
	 accounts.account_type,
	 data.data15 AS data,
	 legacy_photo.data4 AS exists_on_server,
	 legacy_photo.data3 AS download_required,
	 legacy_photo.data2 AS local_version,
	 legacy_photo.data5 AS sync_error 
FROM data JOIN mimetypes ON (mimetypes."_id" = data.mimetype_id) 
JOIN raw_contacts ON (raw_contacts."_id" = data.raw_contact_id)
JOIN accounts ON (raw_contacts.account_id=accounts."_id") 
LEFT OUTER JOIN data name ON (
	raw_contacts."_id" = name.raw_contact_id 
	AND (SELECT mimetype FROM mimetypes WHERE mimetypes."_id" = name.mimetype_id)='vnd.android.cursor.item/name'
) 
LEFT OUTER JOIN data organization ON (
	raw_contacts."_id" = organization.raw_contact_id 
	AND (	SELECT mimetype FROM mimetypes 
		WHERE mimetypes."_id" = organization.mimetype_id)='vnd.android.cursor.item/organization' 
	AND organization.is_primary
) 
LEFT OUTER JOIN data email ON (
	raw_contacts."_id" = email.raw_contact_id 
	AND (SELECT mimetype FROM mimetypes WHERE mimetypes."_id" = email.mimetype_id)='vnd.android.cursor.item/email_v2' 
	AND email.is_primary
) 
LEFT OUTER JOIN data note ON (
	raw_contacts."_id" = note.raw_contact_id 
	AND (SELECT mimetype FROM mimetypes WHERE mimetypes."_id" = note.mimetype_id)='vnd.android.cursor.item/note'
) 
LEFT OUTER JOIN data phone ON (
	raw_contacts."_id" = phone.raw_contact_id 

	AND (SELECT mimetype FROM mimetypes WHERE mimetypes."_id" = phone.mimetype_id)='vnd.android.cursor.item/phone_v2' 
	AND phone.is_primary
) 
LEFT OUTER JOIN data legacy_photo ON (
	raw_contacts."_id" = legacy_photo.raw_contact_id 
	AND (	SELECT mimetype FROM mimetypes 
		WHERE mimetypes."_id" = legacy_photo.mimetype_id)='vnd.android.cursor.item/photo_v1_extras' 
	AND data."_id" = CAST(legacy_photo.data1 AS integer)
) 
WHERE mimetypes.mimetype='vnd.android.cursor.item/photo' 
AND raw_contacts.deleted=0;


-- ###############################################################################
-- Triggers
-- ###############################################################################

--@
--create trigger after_insert_v1_settings after insert on v1_settings
--referencing new table as new_table
--begin atomic
--        for select "_id" as new_id from new_table DO
--                insert into settings(account_name, account_type)
--                        select "_sync_account", "_sync_account_type"
--                        from v1_settings
--                        where "_id"= new_id;
--        end for;
--end
--@

--@
--create trigger after_delete_v1_settings after delete on v1_settings
--referencing old table as old_table
--begin atomic
--        for select "_sync_account" as old_account, "_sync_account_type" as old_type from old_table DO
--                delete from settings where "account_name" = old_account and "account_type" = old_type;
--        end for;
--end
--@


@
CREATE TRIGGER raw_contacts_deleted AFTER DELETE ON raw_contacts 
REFERENCING OLD TABLE AS old_raw
BEGIN ATOMIC 
	FOR SELECT contact_id AS old_contact_id,"_id" AS old_id FROM old_raw DO
		DELETE FROM data WHERE raw_contact_id = old_id;

		DELETE FROM agg_exceptions WHERE raw_contact_id1 = old_id;
		DELETE FROM agg_exceptions WHERE raw_contact_id2 = old_id;

		DELETE FROM visible_contacts
		WHERE "_id"=old_contact_id;

		DELETE FROM default_directory
		WHERE "_id"=old_contact_id;
		DELETE FROM contacts WHERE "_id"= old_contact_id AND 
			(
				SELECT COUNT(*) FROM raw_contacts 
				WHERE raw_contacts.contact_id=old_contact_id
			)=0; 
-- Better with old SQL compiler
--		DELETE FROM contacts as C
--  			WHERE "_id"= :old_contact_id
--    			AND NOT EXISTS (SELECT 1 FROM raw_contacts
--         		WHERE raw_contacts.contact_id = c."_id"); 
	END FOR; 
END 
@
	

@
CREATE TRIGGER data_deleted AFTER DELETE ON data
REFERENCING OLD TABLE AS old_data  
BEGIN ATOMIC
    declare flag int;
    set flag = BUILTIN.USERINFO(1002,0);
    if flag = 0 then
	FOR SELECT "_id" AS old_data_id, raw_contact_id AS old_raw_contact_id FROM old_data DO
		--UPDATE raw_contacts SET version=version+1 WHERE "_id"=old_raw_contact_id;
		DELETE FROM phone_lookup WHERE data_id=old_data_id;
		DELETE FROM status_updates WHERE status_update_data_id=old_data_id;
		DELETE FROM name_lookup WHERE data_id=old_data_id;
	END FOR;
    end if;
END
@

@
create trigger data_updated_2 before update on data
referencing new row as n old row as o for each row
    set n.data_version = o.data_version + 1;
@

@
create trigger data_updated_after after update on data
--Recursion fixed
REFERENCING OLD TABLE AS old_data
BEGIN ATOMIC 
    for select raw_contact_id as old_raw_contact_id from old_data do
        update raw_contacts set version = version + 1
        where "_id" = old_raw_contact_id;
    end for;
END
@ 

@
create trigger groups_update1 before update on groups
referencing new row as n old row as o for each row
    set n.version = o.version + 1;
@

@
CREATE TRIGGER raw_contacts_marked_deleted BEFORE UPDATE ON raw_contacts 
referencing new row as n old row as o for each row
begin atomic
if(n."_id" = o."_id" and n.deleted != o.deleted)  then
        set n.version=o.version + 1;
end if;
end
@



@ 
CREATE TRIGGER groups_auto_add_updated1 AFTER UPDATE ON groups
REFERENCING OLD TABLE AS og NEW TABLE AS ng
BEGIN ATOMIC

L1:
FOR select ng."_id" as group_id from og, ng where ng.MIMER_ROWID = og.MIMER_ROWID and og.auto_add <> ng.auto_add DO
	DELETE
	FROM default_directory;

	INSERT {IGNORE}
	INTO default_directory
	SELECT contact_id
	FROM raw_contacts
	WHERE raw_contacts.account_id=
		(
			SELECT "_id" 
			FROM accounts 
			WHERE account_name IS NULL 
			AND account_type IS NULL 
			AND data_set IS NULL);

	INSERT {IGNORE}
	INTO default_directory
	SELECT contact_id
	FROM raw_contacts
	WHERE NOT EXISTS(SELECT "_id"
		FROM groups
		WHERE raw_contacts.account_id = groups.account_id
		AND auto_add != 0);

	INSERT {IGNORE}
	INTO default_directory
	SELECT contact_id
	FROM raw_contacts
	JOIN data
	ON  (
		raw_contacts."_id"=raw_contact_id)
	WHERE mimetype_id=(SELECT "_id"
		FROM mimetypes
		WHERE mimetype='vnd.android.cursor.item/group_membership')
	AND EXISTS(SELECT "_id"
		FROM groups
		WHERE raw_contacts.account_id = groups.account_id
		AND auto_add != 0);
	LEAVE L1;
END FOR;

END
@



grant select on calls to mimer$permission$contacts_read;
grant select on data to mimer$permission$contacts_read;
grant select on search_index to mimer$permission$contacts_read;
grant select on contacts to mimer$permission$contacts_read;
grant select on raw_contacts to mimer$permission$contacts_read;

grant select on name_lookup_v to mimer$permission$contacts_read;
grant select on view_contacts to mimer$permission$contacts_read;
grant select on view_data to mimer$permission$contacts_read;
grant select on view_raw_contacts to mimer$permission$contacts_read;



