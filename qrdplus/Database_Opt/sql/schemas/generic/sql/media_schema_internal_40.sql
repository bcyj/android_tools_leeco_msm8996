-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

-- ###############################################################################
-- Tables
-- ###############################################################################
CREATE TABLE album_art (
	album_id BIGINT PRIMARY KEY,
	"_data" NVARCHAR(512)
);
GRANT ALL PRIVILEGES ON TABLE album_art to media_group;


CREATE UNIQUE SEQUENCE albums_id_seq as BIGINT;
CREATE TABLE albums (
	album_id BIGINT PRIMARY KEY default NEXT_VALUE OF albums_id_seq with check option,
	album_key NVARCHAR(256) NOT NULL UNIQUE,
	album NVARCHAR(120) NOT NULL--,
	--ROWID integer default current_value of albums_id_seq
);
GRANT ALL PRIVILEGES ON TABLE albums to media_group;

CREATE TABLE android_metadata (
	locale NVARCHAR(120)
);
GRANT ALL PRIVILEGES ON TABLE android_metadata to media_group;

CREATE UNIQUE SEQUENCE artist_id_seq as BIGINT;
CREATE TABLE artists (
	artist_id BIGINT PRIMARY KEY default NEXT_VALUE OF artist_id_seq with check option,
	artist_key NVARCHAR(150) NOT NULL UNIQUE,
	artist NVARCHAR(120) NOT NULL
);
GRANT ALL PRIVILEGES ON TABLE artists to media_group;

CREATE UNIQUE SEQUENCE thumbnails_id_seq as BIGINT;
CREATE TABLE thumbnails (
	"_id" BIGINT PRIMARY KEY default NEXT_VALUE OF thumbnails_id_seq with check option,
	"_data" NVARCHAR(512),
	image_id BIGINT,
	kind INTEGER,
	width INTEGER,
	height INTEGER
);
GRANT ALL PRIVILEGES ON TABLE thumbnails to media_group;

CREATE UNIQUE SEQUENCE videothumbnails_id_seq as BIGINT;
CREATE TABLE videothumbnails (
	"_id" BIGINT PRIMARY KEY default NEXT_VALUE OF videothumbnails_id_seq with check option,
	"_data" NVARCHAR(512),
	video_id BIGINT,
	kind INTEGER,
	width INTEGER,
	height INTEGER
);
GRANT ALL PRIVILEGES ON TABLE videothumbnails to media_group;

CREATE UNIQUE SEQUENCE files_id_seq as BIGINT;
CREATE TABLE files
    (
        "_id" BIGINT PRIMARY KEY DEFAULT next_value of files_id_seq with check option,
        "_data" NVARCHAR(256) UNIQUE,
        "_size" BIGINT,
        format INTEGER,
        parent BIGINT,
        date_added BIGINT,
        date_modified BIGINT,
        mime_type VARCHAR(128),
        title NVARCHAR(200),
        description NVARCHAR(128),
        "_display_name" NVARCHAR(256),
        picasa_id NVARCHAR(40),
        orientation INTEGER,
        latitude decimal(17,10),
        longitude decimal(17,10),
        datetaken BIGINT,
        mini_thumb_magic BIGINT,
        bucket_id VARCHAR(20),
        bucket_display_name NVARCHAR(256),
        isprivate INTEGER,
        --title_key VARBINARY(200),
	title_key VARCHAR(512),
        artist_id BIGINT,
        album_id BIGINT,
        composer NVARCHAR(200),
        track INTEGER,
        "YEAR" INTEGER CHECK("YEAR"<>0),
        is_ringtone INTEGER,
        is_music INTEGER,
        is_alarm INTEGER,
        is_notification INTEGER,
        is_podcast INTEGER,
        album_artist NVARCHAR(50),
        duration INTEGER,
        bookmark INTEGER,
        artist NVARCHAR(100),
        album NVARCHAR(100),
        resolution VARCHAR(12),
        tags NVARCHAR(80),
        category NVARCHAR(40),
        "language" VARCHAR(30),
        mini_thumb_data BLOB,
        name NVARCHAR(100),
        media_type INTEGER,
        old_id BIGINT,
        storage_id BIGINT,
        is_drm INTEGER,
        width INTEGER,
        height INTEGER

    );
GRANT ALL PRIVILEGES ON TABLE files to media_group;



--add table log:
--CREATE TABLE log (time DATETIME PRIMARY KEY, message TEXT);
CREATE UNIQUE sequence log_seq AS BIGINT;
CREATE TABLE "log" (
	"_id" BIGINT PRIMARY KEY DEFAULT next_value OF log_seq with check option,
	"time" VARCHAR(25) NOT NULL,
	"message" NVARCHAR(512)
);
GRANT ALL PRIVILEGES ON TABLE "log" to media_group;

create index "log_idx" on "log"( 
     "time" DESC
);



-- ###############################################################################
-- Indexes 
-- ###############################################################################

CREATE INDEX album_id_idx ON files (album_id);

CREATE INDEX album_idx ON albums (album);

CREATE INDEX artist_id_idx ON files (artist_id);

CREATE INDEX artist_idx ON artists (artist);

CREATE INDEX albumkey_index on albums(album_key);

CREATE INDEX artistkey_index on artists(artist_key);

CREATE INDEX bucket_index ON files 
(
        bucket_id,
        media_type,
        datetaken
);

CREATE INDEX bucket_name
ON files
(
        bucket_id,
        media_type,
        bucket_display_name
);
CREATE INDEX format_index ON files(format);


CREATE INDEX image_id_index on thumbnails(image_id);

CREATE INDEX media_type_index ON files(media_type);

CREATE INDEX parent_index ON files (parent);


CREATE INDEX path_collate_format_index ON files ("_data" collate current_collation_2, format);

CREATE INDEX sort_index 
ON files
(
    datetaken ASC
);

CREATE INDEX title_idx ON files (title);

CREATE INDEX titlekey_index ON files (title_key);

CREATE INDEX video_id_index ON videothumbnails(video_id);


create index "date_added_idx" on "files"( 
     "date_added" desc
);


-- Optimering
--create index images_bucket_id_idx on images(bucket_id);
--create index video_bucket_id_idx on video(bucket_id);

--create index images_bucket_display_name_bucket_id_idx on images(bucket_display_name collate current_collation_2,bucket_id);
--create index video_bucket_display_name_bucket_id_idx on video(bucket_display_name collate current_collation_2,bucket_id);

-- ###############################################################################
-- Views 
-- ###############################################################################
CREATE VIEW audio_meta AS
SELECT "_id",
    "_data",
    "_display_name",
    "_size",
    mime_type,
    date_added,
    is_drm,
    date_modified,
    title,
    title_key,
    duration,
    artist_id,
    composer,
    album_id,
    track,
    "YEAR",
    is_ringtone,
    is_music,
    is_alarm,
    is_notification,
    is_podcast,
    bookmark,
    album_artist

FROM files
WHERE media_type=2
;


CREATE VIEW video AS
SELECT "_id",
    "_data",
    "_display_name",
    "_size",
    mime_type,
    date_added,
    date_modified,
    title,
    duration,
    artist,
    album,
    resolution,
    description,
    isprivate,
    tags,
    category,
    "language",
    mini_thumb_data,
    latitude,
    longitude,
    datetaken,
    mini_thumb_magic,
    bucket_id,
    bucket_display_name,
    bookmark,
    width,
    height
FROM files
WHERE media_type=3
;

CREATE VIEW audio AS 

SELECT * 
FROM audio_meta 
NATURAL LEFT OUTER JOIN artists  
NATURAL LEFT OUTER JOIN albums;


CREATE VIEW searchhelpertitle AS 
SELECT * FROM audio
ORDER BY title_key;

GRANT ALL PRIVILEGES ON audio to media_group;

CREATE VIEW album_info AS 
SELECT audio.album_id AS "_id",
	album AS "album",
	album_key AS "album_key",
	MIN("year") AS minyear,
	MAX("year") AS maxyear,
	artist AS "artist",
	artist_id AS "artist_id",
	artist_key AS "artist_key",
	count(*) AS numsongs,
	album_art."_data" AS album_art 
FROM audio 
LEFT OUTER JOIN album_art ON audio.album_id=album_art.album_id
WHERE is_music=1 
GROUP BY audio.album_id;

CREATE VIEW artist_info AS 
SELECT 	artist_id AS "_id",
	artist AS "artist",
	artist_key,
	COUNT(DISTINCT album) AS number_of_albums,
	COUNT(*) AS number_of_tracks 
FROM audio 
WHERE is_music=1 
GROUP BY artist_key;

CREATE VIEW artists_albums_map AS 
SELECT DISTINCT artist_id,
	album_id 
FROM audio_meta;



CREATE VIEW search AS 
SELECT "_id",
	'artist' AS mime_type,
	artist,
	NULL AS album,
	NULL AS title,
	artist AS text1,
	NULL AS text2,
	number_of_albums AS data1,
	number_of_tracks AS data2,
	artist_key AS "match",
	'content://media/external/audio/artists/'||cast("_id" AS VARCHAR(10)) AS suggest_intent_data,
	1 AS grouporder 

FROM artist_info

WHERE (artist!='<unknown>') 
UNION ALL 
SELECT "_id",
	'album' AS mime_type,
	artist,
	album,
	NULL AS title,
	album AS text1,
	artist AS text2,
	NULL AS data1,
	NULL AS data2,
	artist_key||' '||album_key AS "match",
	'content://media/external/audio/albums/'||cast("_id" AS VARCHAR(10)) AS suggest_intent_data,
	2 AS grouporder 

FROM album_info 

WHERE (album!='<unknown>') 
UNION ALL 
SELECT searchhelpertitle."_id" AS "_id",
	mime_type,
	artist,
	album,
	title,
	title AS text1,
	artist AS text2,
	NULL AS data1,
	NULL AS data2,
	artist_key||' '||album_key||' '|| cast(title_key as varchar(200)) AS "match",
	'content://media/external/audio/media/'||cast(searchhelpertitle."_id" AS VARCHAR(10)) AS suggest_intent_data,
	3 AS grouporder 
FROM searchhelpertitle 
WHERE (title != '') 
;

CREATE VIEW images AS
SELECT "_id",
    "_data",
    "_size",
    "_display_name",
    mime_type,
    title,
    date_added,
    date_modified,
    description,
    picasa_id,
    isprivate,
    latitude,
    longitude,
    datetaken,
    orientation,
    mini_thumb_magic,
    bucket_id,
    bucket_display_name,
    width,
    height
FROM files
WHERE media_type=1
;





-- ###############################################################################
-- Triggers
-- ###############################################################################

-- Replaced by files_cleanup that combines images_cleanup and video_cleanup
-- There are one version of files_cleanup for the internal schema and one for the external
--@
--CREATE TRIGGER images_cleanup AFTER DELETE ON files
--REFERENCING OLD TABLE AS old_images
--BEGIN ATOMIC
--  declare res int;
--  FOR SELECT "_id" AS old_id, "_data" as old_data, media_type FROM old_images DO
--    IF media_type = 1 then
--      DELETE FROM thumbnails WHERE image_id = old_id;
--      set res = compatibility.delete_file(old_data);
--    END IF;
--  END FOR;
--END
--@

-- Replaced by media_files_cleanup that combines images_cleanup and video_cleanup
-- There are one version of files_cleanup for the internal schema and one for the external
--@
--CREATE TRIGGER video_cleanup AFTER DELETE ON files
--REFERENCING OLD TABLE AS old_video
--BEGIN ATOMIC
--  declare res int;
--  FOR SELECT "_id" AS old_id, "_data" as old_data, media_type FROM old_video DO
--    IF media_type = 3 then
--      DELETE FROM thumbnails WHERE image_id = old_id;
--      set res = compatibility.delete_file(old_data);
--    END IF;
--  END FOR;
--END
--@













@
create procedure clear_data()
modifies sql data
begin
declare res int;
set res = BUILTIN.USERINFO(1001,0,1);
delete from album_art;
delete from albums;
delete from artists;
delete from log;
delete from thumbnails;
delete from videothumbnails;
delete from files;
set res = BUILTIN.USERINFO(1001,0,0);
end
@

grant select on albums to mimer$permission$media_read;
grant select on artists to mimer$permission$media_read;
grant select on files to mimer$permission$media_read;



