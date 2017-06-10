set echo off;
-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
set message off;

delete from local_groups;
delete from deleted_contacts;


enter 'profile' using 'nopass';
delete from deleted_contacts;
leave;
