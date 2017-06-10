set echo off;
-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
set message off;

select count(*) from Account;
select count(*) from Body;
alter databank email_db drop filesize;
alter databank email_body_db drop filesize;

enter 'backup' using 'nopass';
select count(*) from Account;
alter databank email_backup_db drop filesize;
leave;
