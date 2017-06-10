set echo off;
-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
set message off;
select count(*) from mimetypes;
alter databank contacts2_db drop filesize;
alter databank presence_data drop filesize;

enter 'profile' using 'nopass';
select count(*) from mimetypes;
alter databank profile_db drop filesize;
leave;
