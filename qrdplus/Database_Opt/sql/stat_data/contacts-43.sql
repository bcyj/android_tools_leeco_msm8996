set echo off;
-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
set message off;
set sqlite on;

update raw_contacts set phonebook_label = substring(sort_key from 1 for 1);
update raw_contacts set phonebook_bucket = ascii_code(phonebook_label) - 64;
update raw_contacts set phonebook_label_alt = substring(sort_key_alt from 1 for 1);
update raw_contacts set phonebook_bucket_alt = ascii_code(phonebook_label_alt) - 64;

update contacts set contact_last_updated_timestamp = 12345678944 where "_id" in (87,76,65,64,54,3,32,21,42,63);
update contacts set contact_last_updated_timestamp = 12345278944 where "_id" in (98,76,54,32,11);
update contacts set contact_last_updated_timestamp = 23345678944 where "_id" in (99,88,77,66,55,44,33);
update contacts set contact_last_updated_timestamp = 22345678944 where "_id" in (89,78,67,56,45,34,23,12);
update contacts set contact_last_updated_timestamp = 32345678944 where "_id" in (13,24,35,47,57,68,79);
update contacts set contact_last_updated_timestamp = 42345678944 where "_id" in (97,86,75,74,53);
update contacts set contact_last_updated_timestamp = 52345678944 where "_id" in (76,64,54,344,222,111,556);


@
begin
 declare i integer;
 SET i = 1;
 WHILE i <= 200 DO
   insert into deleted_contacts (contact_id, contact_deleted_timestamp) values(i, i + 2434449745338);
   set i = i + 1;
 END WHILE;
end
@


enter 'profile' using 'nopass';
update raw_contacts set phonebook_label = substring(sort_key from 1 for 1);
update raw_contacts set phonebook_bucket = ascii_code(phonebook_label) - 64;
update raw_contacts set phonebook_label_alt = substring(sort_key_alt from 1 for 1);
update raw_contacts set phonebook_bucket_alt = ascii_code(phonebook_label_alt) - 64;

update contacts set contact_last_updated_timestamp = 12345678944 where "_id" in (87,76,65,64,54,3,32,21,42,63);
update contacts set contact_last_updated_timestamp = 12345278944 where "_id" in (98,76,54,32,11);
update contacts set contact_last_updated_timestamp = 23345678944 where "_id" in (99,88,77,66,55,44,33);
update contacts set contact_last_updated_timestamp = 22345678944 where "_id" in (89,78,67,56,45,34,23,12);
update contacts set contact_last_updated_timestamp = 32345678944 where "_id" in (13,24,35,47,57,68,79);
update contacts set contact_last_updated_timestamp = 42345678944 where "_id" in (97,86,75,74,53);
update contacts set contact_last_updated_timestamp = 52345678944 where "_id" in (76,64,54,344,222,111,556);


@
begin
 declare i integer;
 SET i = 1;
 WHILE i <= 200 DO
   insert into deleted_contacts (contact_id, contact_deleted_timestamp) values(i, i + 2434449745338);
   set i = i + 1;
 END WHILE;
end
@
leave;

