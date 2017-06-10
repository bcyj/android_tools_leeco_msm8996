set echo off;
-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
read 'media_internal.sql';
enter 'media_external' using 'nopass';
read 'media_external.sql';
leave;
enter 'media_external_1' using 'nopass';
read 'media_external.sql';
leave;
enter 'media_external_2' using 'nopass';
read 'media_external.sql';
leave;
--enter 'media_external_3' using 'nopass';
--read 'media_external.sql';
--leave;
