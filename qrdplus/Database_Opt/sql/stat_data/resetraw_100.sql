set echo off;
-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
set message off;

delete from raw_contacts where "_id" > (select max("_id") -100 from raw_contacts);

enter 'profile' using 'nopass';
delete from raw_contacts where "_id" > (select max("_id") -100 from raw_contacts);
leave;
