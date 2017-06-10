-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

whenever error continue;

drop databank email_db cascade;
whenever error exit;
set sqlite on;
-- ###############################################################################
-- Databanks & schemas
-- ###############################################################################

create databank email_db set goalsize 100K;
call compatibility.map_database('/data/data/com.android.email/databases/EmailProvider.db', 'email_db', 'email', '', timestamp '0001-01-01 00:00:00');
call compatibility.map_auxiliary_database('/data/user/0/com.android.email/databases/EmailProvider.db', 'email_db', 'email', '', timestamp '0001-01-01 00:00:00');


create ident backup as program identified by 'nopass';
grant databank to backup;
grant schema to backup;

call compatibility.map_database('/data/data/com.android.email/databases/EmailProviderBackup.db', 'email_backup_db', 'backup', 'backup', timestamp '0001-01-01 00:00:00');
call compatibility.map_auxiliary_database('/data/user/0/com.android.email/databases/EmailProviderBackup.db', 'email_backup_db', 'backup', 'backup', timestamp '0001-01-01 00:00:00');

create schema email_body;
create databank email_body_db set goalsize 100K;
call compatibility.map_database('/data/data/com.android.email/databases/EmailProviderBody.db', 'email_body_db', 'email_body', '', timestamp '0001-01-01 00:00:00');
call compatibility.map_auxiliary_database('/data/user/0/com.android.email/databases/EmailProviderBody.db', 'email_body_db', 'email_body', '', timestamp '0001-01-01 00:00:00');


call compatibility.set_or_create_pragma('email_db','user_version','123');
call compatibility.set_or_create_pragma('email_backup_db','user_version','123');
call compatibility.set_or_create_pragma('email_body_db','user_version','100');
	
--Is this sequence really needed?
CREATE UNIQUE SEQUENCE android_metadata_id_seq in email_db;

CREATE TABLE android_metadata(locale nvarchar(128)) in email_db;

