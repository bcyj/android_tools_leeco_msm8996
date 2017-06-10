set echo off;
-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
update statistics for ident media;
enter 'media_external' using 'nopass';
update statistics for ident media_external;
leave;
enter 'media_external_1' using 'nopass';
update statistics for ident media_external_1;
leave;
enter 'media_external_2' using 'nopass';
update statistics for ident media_external_2;
leave;
--enter 'media_external_3' using 'nopass';
--update statistics for ident media_external_3;
--leave;

