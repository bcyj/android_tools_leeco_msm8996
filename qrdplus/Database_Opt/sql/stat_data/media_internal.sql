-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

delete from albums;
delete from artists;
delete from files;

insert into albums (album_id, album_key, album) values (1, '	''	=	''	I	?	K	-183121033', 'alarms');
insert into albums (album_id, album_key, album) values (2, '	A	C	M	7	1	7	+	''	M	7	C	A	K	-1608219629', 'notifications');
insert into albums (album_id, album_key, album) values (3, '	I	7	A	3	M	C	A	/	K	293500348', 'ringtones');
insert into albums (album_id, album_key, album) values (4, '	O	A	;	A	C	S	A	-1608219629', 'Unknown');
insert into albums (album_id, album_key, album) values (5, '	O	7	78638153', 'ui');

insert into artists (artist_id, artist_key, artist) values (1, '', '<unknown>');
insert into artists (artist_id, artist_key, artist) values (2, '	E	-	U	', 'pdx');
insert into artists (artist_id, artist_key, artist) values (3, '	O	A	;	A	C	S	A	', 'Unknown');
insert into artists (artist_id, artist_key, artist) values (4, '	-	I		?	''	-	', 'Dr. MAD');
insert into artists (artist_id, artist_key, artist) values (5, '	7	A	;		?	C	A	K	M	/	I	', 'Ink Monster');
insert into artists (artist_id, artist_key, artist) values (6, '	+							''	A	-	I	C	7	-		C	E	/	A		K	C	O	I	+	/		E	I	C	9	/	+	M	', '(c) 2010 Android Open Source Project');

insert into album_art (album_id, "_data") values(1, '_xxx_sdcard/Music/1');
insert into album_art (album_id, "_data") values(2, '_xxx_sdcard/Music/2');
insert into album_art (album_id, "_data") values(3, '_xxx_sdcard/Music/3');
insert into album_art (album_id, "_data") values(4, '_xxx_sdcard/Music/4');
insert into album_art (album_id, "_data") values(5, '_xxx_sdcard/Music/5');

----
----

@
begin
   declare files_cnt integer;
   declare n_data NVARCHAR(256);
   declare n_size BIGINT;
   declare n_format INTEGER;
   declare n_parent BIGINT;
   declare n_date_added BIGINT;
   declare n_date_modified BIGINT;
   declare n_mime_type VARCHAR(128);
   declare n_title NVARCHAR(200);
   declare n_description NVARCHAR(128);
   declare n_display_name NVARCHAR(256);
   declare n_picasa_id NVARCHAR(40);
   declare n_orientation INTEGER;
   declare n_latitude decimal(17,10);
   declare n_longitude decimal(17,10);
   declare n_datetaken BIGINT;
   declare n_mini_thumb_magic BIGINT;
   declare n_bucket_id VARCHAR(20);
   declare n_bucket_display_name NVARCHAR(256);
   declare n_isprivate INTEGER;
   declare n_title_key VARCHAR(512);
   declare n_artist_id BIGINT;
   declare n_album_id BIGINT;
   declare n_composer NVARCHAR(200);
   declare n_track INTEGER;
   declare n_YEAR INTEGER;
   declare n_is_ringtone INTEGER;
   declare n_is_music INTEGER;
   declare n_is_alarm INTEGER;
   declare n_is_notification INTEGER;
   declare n_is_podcast INTEGER;
   declare n_album_artist NVARCHAR(50);
   declare n_duration INTEGER;
   declare n_bookmark INTEGER;
   declare n_artist NVARCHAR(100);
   declare n_album NVARCHAR(100);
   declare n_resolution VARCHAR(12);
   declare n_tags NVARCHAR(80);
   declare n_category NVARCHAR(40);
   declare n_language VARCHAR(30);
   declare n_mini_thumb_data BLOB;
   declare n_name NVARCHAR(100);
   declare n_media_type INTEGER;
   declare n_old_id BIGINT;
   declare n_storage_id BIGINT;
   declare n_is_drm INTEGER;
   declare n_width INTEGER;
   declare n_height INTEGER;

   set files_cnt = 0;
   
   WHILE files_cnt <= 200 DO

    set n_data = '_xxx_sdcard/Music/'||cast(files_cnt as nvarchar(10));
    set n_size = mod(IRAND(), 200)*1000;
    set n_format = mod(IRAND(), 10);
    set n_parent = mod(IRAND(), 200)*1000;
    set n_date_added = mod(IRAND(), 1000)*1000;
    set n_date_modified = mod(IRAND(), 1000)*1000;
    if mod(files_cnt,4)=0 then
    	set n_mime_type = 'image/jpg';
    	set n_media_type = 1;
    elseif mod(files_cnt,4)=1 then
    	set n_mime_type = 'audio/mpeg';
    	set n_media_type = 2;
	--set n_mini_thumb_data = '	+	''	I	''	?	/	=		E	I	7	K	C	A	/	I	';
    elseif mod(files_cnt,4)=2 then
    	set n_mime_type = 'video/mp4';
    	set n_media_type = 3;
	--set n_mini_thumb_data = '	)	O	?	E		5	/	''	-	K		S	7	M		?	/	';
    else
    	set n_mime_type = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 3)+1);
    	set n_media_type = 0;
    end if;
    set n_mini_thumb_data = null;

    set n_title = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||' ', MOD(IRAND(), 3)+1);
    set n_description = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 10)+1);
    set n_display_name = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||' ', MOD(IRAND(), 3)+1);
    set n_picasa_id = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 3)+1);
    set n_orientation = mod(IRAND(), 2);
    set n_latitude = cast(mod(IRAND(), 100)/3.0 as decimal(17,10));
    set n_longitude = cast(mod(IRAND(), 100)/3.0 as decimal(17,10));
    set n_datetaken = mod(IRAND(), 1000)*1000;
    set n_mini_thumb_magic = mod(IRAND(), 1000)*1000;
    set n_bucket_id = cast(mod(IRAND(), 1000)*1234 as VARCHAR(20));
    set n_bucket_display_name = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 10)+1);
    set n_isprivate = mod(IRAND(), 2);
    set n_title_key = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65), MOD(IRAND(), 10)+1);
    set n_artist_id = mod(IRAND(), 20);
    set n_album_id = mod(IRAND(), 20);
    set n_composer = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||' ', MOD(IRAND(), 3)+1);
    set n_track = mod(IRAND(), 20);
    set n_YEAR = mod(IRAND(), 2000)+1;
    set n_is_ringtone = mod(IRAND(), 2);
    set n_is_music = mod(IRAND(), 2);
    set n_is_alarm = mod(IRAND(), 2);
    set n_is_notification = mod(IRAND(), 2);
    set n_is_podcast = mod(IRAND(), 2);
    set n_album_artist = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||' ', MOD(IRAND(), 3)+1);
    set n_duration = mod(IRAND(), 10000);
    set n_bookmark = mod(IRAND(), 20);
    set n_artist = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||' ', MOD(IRAND(), 3)+1);
    set n_album = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||' ', MOD(IRAND(), 3)+1);
    set n_resolution = cast(mod(IRAND(), 1000) as varchar(5))||'*512';
    set n_tags = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||' ', MOD(IRAND(), 3)+1);
    set n_category = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||' ', MOD(IRAND(), 3)+1);
    set n_language = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||' ', MOD(IRAND(), 3)+1);
    set n_name = REPEAT(ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||ASCII_CHAR(MOD(IRAND(), 25) + 65)||' ', MOD(IRAND(), 3)+1);
    set n_old_id = mod(IRAND(), 1000);
    set n_storage_id = mod(IRAND(), 10);
    set n_is_drm = mod(IRAND(), 2);
    set n_width = mod(IRAND(), 1000);
    set n_height = mod(IRAND(), 1000);

    insert into files ("_id", "_data", "_size", format, parent, date_added, date_modified, mime_type, title, description, "_display_name", picasa_id, orientation, latitude, longitude, datetaken, mini_thumb_magic, bucket_id, bucket_display_name, isprivate, title_key, artist_id, album_id, composer, track, "YEAR", is_ringtone, is_music, is_alarm, is_notification, is_podcast, album_artist, duration, bookmark, artist, album, resolution, tags, category, "language", mini_thumb_data, name, media_type, old_id, storage_id, is_drm, width, height) values (files_cnt + 1, n_data, n_size, n_format, n_parent, n_date_added, n_date_modified, n_mime_type, n_title, n_description, n_display_name, n_picasa_id, n_orientation, n_latitude, n_longitude, n_datetaken, n_mini_thumb_magic, n_bucket_id, n_bucket_display_name, n_isprivate, n_title_key, n_artist_id, n_album_id, n_composer, n_track, n_YEAR, n_is_ringtone, n_is_music, n_is_alarm, n_is_notification, n_is_podcast, n_album_artist, n_duration, n_bookmark, n_artist, n_album, n_resolution, n_tags, n_category, n_language, n_mini_thumb_data, n_name, n_media_type, n_old_id, n_storage_id, n_is_drm, n_width, n_height);
    
    set files_cnt = files_cnt + 1;
    END WHILE;
end 
@


insert into thumbnails ("_id", "_data", image_id, kind, width, height) values (1000, null, 2635, null, null, null);
insert into thumbnails ("_id", "_data", image_id, kind, width, height) values (1001, null, 2636, null, null, null);
insert into thumbnails ("_id", "_data", image_id, kind, width, height) values (1002, null, 2637, null, null, null);
insert into thumbnails ("_id", "_data", image_id, kind, width, height) values (1003, null, 2638, null, null, null);
insert into thumbnails ("_id", "_data", image_id, kind, width, height) values (1004, null, 2639, null, null, null);
insert into thumbnails ("_id", "_data", image_id, kind, width, height) values (1005, null, 2640, null, null, null);


insert into videothumbnails ("_id", "_data", video_id, kind, width, height) values (1001, null, 1, null, null, null);
insert into videothumbnails ("_id", "_data", video_id, kind, width, height) values (1002, null, 2, null, null, null);
insert into videothumbnails ("_id", "_data", video_id, kind, width, height) values (1003, null, 3, null, null, null);
insert into videothumbnails ("_id", "_data", video_id, kind, width, height) values (1004, null, 4, null, null, null);
insert into videothumbnails ("_id", "_data", video_id, kind, width, height) values (1005, null, 5, null, null, null);
insert into videothumbnails ("_id", "_data", video_id, kind, width, height) values (1006, null, 6, null, null, null);

