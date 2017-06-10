-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com


whenever error exit;
set sqlite on;
-- ###############################################################################
-- ToDo	
-- ###############################################################################
-- "match" and "language" columns has to be quoted by transformer
-- Determine data types replacing TEXT
-- _DELETE_FILE handling in triggers
-- Folj upp om borttagande av ORDER BY i CREATE VIEW skapar problem
-- MIN-fix for kolumner som ej ar med i GROUP BY ar inte helt bra
-- audio_playlists_map tabell saknas??? 
-- audio_genres_map  tabell saknas???
-- ###############################################################################
-- Databanks & schemas & idents
-- ###############################################################################


create ident media_group as group;

--Internal DB

create databank "media_internal_db" set goalsize 100K;
call compatibility.map_database('/data/data/com.android.providers.media/databases/internal.db', 'media_internal_db', '','', timestamp '0001-01-01 00:00:00');
call compatibility.map_auxiliary_database('/data/user/0/com.android.providers.media/databases/internal.db', 'media_internal_db', '','', timestamp '0001-01-01 00:00:00');

read 'media_schema_internal_40.sql';
call compatibility.set_or_create_pragma('media_internal_db','user_version','700');

--Builtin sdcard schema
create ident "media_external" as program identified by 'nopass';
grant databank to "media_external";
grant schema to "media_external";
enter 'media_external' using 'nopass';
create databank "media_external_db" set goalsize 100K;
read 'media_schema_external_40.sql';
call compatibility.set_or_create_pragma('media_external_db','user_version','700');
leave;
grant member on media_group to media_external;
call compatibility.map_database('/data/data/com.android.providers.media/databases/external.db', 'media_external_db', '','media_external', timestamp '0001-01-01 00:00:00');
call compatibility.map_auxiliary_database('/data/user/0/com.android.providers.media/databases/external.db', 'media_external_db', '','media_external', timestamp '0001-01-01 00:00:00');

--External DB
call compatibility.map_database('/data/data/com.android.providers.media/databases/external-1.db', 'media_external_1_db', '','media_external_1', timestamp '0001-01-01 00:00:00');
call compatibility.map_auxiliary_database('/data/user/0/com.android.providers.media/databases/external-1.db', 'media_external_1_db', '','media_external_1', timestamp '0001-01-01 00:00:00');
call compatibility.map_database('/data/data/com.android.providers.media/databases/external-2.db', 'media_external_2_db', '','media_external_2', timestamp '0001-01-01 00:00:00');
call compatibility.map_auxiliary_database('/data/user/0/com.android.providers.media/databases/external-2.db', 'media_external_2_db', '','media_external_2', timestamp '0001-01-01 00:00:00');

create ident "media_external_1" as program identified by 'nopass';
grant databank to "media_external_1";
grant schema to "media_external_1";
enter 'media_external_1' using 'nopass';
create databank "media_external_1_db" set goalsize 100K;
read 'media_schema_external_40.sql';
call compatibility.set_or_create_pragma('media_external_1_db','user_version','700');
leave;

grant member on media_group to media_external_1;

create ident "media_external_2" as program identified by 'nopass';
grant databank to "media_external_2";
grant schema to "media_external_2";
enter 'media_external_2' using 'nopass';
create databank "media_external_2_db" set goalsize 100K;
read 'media_schema_external_40.sql';
call compatibility.set_or_create_pragma('media_external_2_db','user_version','700');
leave;

grant member on media_group to media_external_2;

