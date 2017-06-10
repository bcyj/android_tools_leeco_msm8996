set echo off;
-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
set message off;
select count(*) from pdu;
select count(*) from carriers;
alter databank mmssms_db drop filesize;
alter databank telephony_db drop filesize;

