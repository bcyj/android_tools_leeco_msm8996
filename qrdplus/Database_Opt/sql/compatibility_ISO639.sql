-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
--
-- This file contains SQL definitions for a table to map ISO639 language codes to Mimer collation names.
--
set sqlite on;
create table compatibility.ISO639 (code char(3),collation_name nvarchar(128) collate sql_identifier,primary key(code));
insert into compatibility.ISO639 values ('af','AFRIKAANS');
insert into compatibility.ISO639 values ('ar','ARABIC');
insert into compatibility.ISO639 values ('as','ASSAMESE');
insert into compatibility.ISO639 values ('ast','ASTURIAN');
insert into compatibility.ISO639 values ('az','AZERBAIJANI');
insert into compatibility.ISO639 values ('be','BELARUSIAN');
insert into compatibility.ISO639 values ('bg','BULGARIAN');
insert into compatibility.ISO639 values ('bn','BENGALI');
insert into compatibility.ISO639 values ('br','BRETON');
insert into compatibility.ISO639 values ('bo','TIBETAN');
insert into compatibility.ISO639 values ('bs','BOSNIAN');
insert into compatibility.ISO639 values ('ca','CATALAN');
insert into compatibility.ISO639 values ('ce','CHECHEN');
insert into compatibility.ISO639 values ('ceb','CEBUANO');
insert into compatibility.ISO639 values ('ch','CHAMORRO');
insert into compatibility.ISO639 values ('cho','CHOCTAW');
insert into compatibility.ISO639 values ('chr','CHEROKEE');
insert into compatibility.ISO639 values ('chy','CHEYENNE');
insert into compatibility.ISO639 values ('co','CORSICAN');
insert into compatibility.ISO639 values ('cr','CREE');
insert into compatibility.ISO639 values ('cs','CZECH');
insert into compatibility.ISO639 values ('csb','KASHUBIAN');
insert into compatibility.ISO639 values ('cu','OLD');
insert into compatibility.ISO639 values ('cv','CHUVASH');
insert into compatibility.ISO639 values ('cy','WELSH');
insert into compatibility.ISO639 values ('da','DANISH');
insert into compatibility.ISO639 values ('de','GERMAN');
insert into compatibility.ISO639 values ('diq','DIMLI');
insert into compatibility.ISO639 values ('dsb','LOWER');
insert into compatibility.ISO639 values ('dv','DIVEHI');
insert into compatibility.ISO639 values ('dz','DZONGKHA');
insert into compatibility.ISO639 values ('ee','EWE');
insert into compatibility.ISO639 values ('el','GREEK');
insert into compatibility.ISO639 values ('en','ENGLISH');
insert into compatibility.ISO639 values ('eo','ESPERANTO');
insert into compatibility.ISO639 values ('es','SPANISH');
insert into compatibility.ISO639 values ('et','ESTONIAN');
insert into compatibility.ISO639 values ('eu','BASQUE');
insert into compatibility.ISO639 values ('ext','EXTREMADURAN');
insert into compatibility.ISO639 values ('fa','PERSIAN');
insert into compatibility.ISO639 values ('ff','PEUL');
insert into compatibility.ISO639 values ('fi','FINNISH');
insert into compatibility.ISO639 values ('fil','FILIPINO');
insert into compatibility.ISO639 values ('fj','FIJIAN');
insert into compatibility.ISO639 values ('fo','FAROESE');
insert into compatibility.ISO639 values ('fr','FRENCH');
insert into compatibility.ISO639 values ('frp','ARPITAN');
insert into compatibility.ISO639 values ('fur','FRIULIAN');
insert into compatibility.ISO639 values ('fy','FRISIAN');
insert into compatibility.ISO639 values ('ga','IRISH');
insert into compatibility.ISO639 values ('gd','SCOTTISH_GAELIC');
insert into compatibility.ISO639 values ('gl','GALICIAN');
insert into compatibility.ISO639 values ('gn','GUARANI');
insert into compatibility.ISO639 values ('gu','GUJARATI');
insert into compatibility.ISO639 values ('gv','MANX');
insert into compatibility.ISO639 values ('ha','HAUSA');
insert into compatibility.ISO639 values ('he','HEBREW');
insert into compatibility.ISO639 values ('hi','HINDI');
insert into compatibility.ISO639 values ('hr','CROATIAN');
insert into compatibility.ISO639 values ('ht','HAITIAN');
insert into compatibility.ISO639 values ('hu','HUNGARIAN');
insert into compatibility.ISO639 values ('hy','ARMENIAN');
insert into compatibility.ISO639 values ('hz','HERERO');
insert into compatibility.ISO639 values ('id','INDONESIAN');
insert into compatibility.ISO639 values ('ig','IGBO');
insert into compatibility.ISO639 values ('ii','SICHUAN');
insert into compatibility.ISO639 values ('ik','INUPIAK');
insert into compatibility.ISO639 values ('ilo','ILOKANO');
insert into compatibility.ISO639 values ('io','IDO');
insert into compatibility.ISO639 values ('is','ICELANDIC');
insert into compatibility.ISO639 values ('it','ITALIAN');
insert into compatibility.ISO639 values ('iu','INUKTITUT');
insert into compatibility.ISO639 values ('ja','JAPANESE');
insert into compatibility.ISO639 values ('jbo','LOJBAN');
insert into compatibility.ISO639 values ('jv','JAVANESE');
insert into compatibility.ISO639 values ('ka','GEORGIAN');
insert into compatibility.ISO639 values ('kg','KONGO');
insert into compatibility.ISO639 values ('ki','KIKUYU');
insert into compatibility.ISO639 values ('kj','KUANYAMA');
insert into compatibility.ISO639 values ('kk','KAZAKH');
insert into compatibility.ISO639 values ('kl','GREENLANDIC');
insert into compatibility.ISO639 values ('km','KHMER');
insert into compatibility.ISO639 values ('kn','KANNADA');
insert into compatibility.ISO639 values ('ko','KOREAN');
insert into compatibility.ISO639 values ('kr','KANURI');
insert into compatibility.ISO639 values ('ks','KASHMIRI');
insert into compatibility.ISO639 values ('ksh','RIPUARIAN');
insert into compatibility.ISO639 values ('ku','KURDISH');
insert into compatibility.ISO639 values ('kv','KOMI');
insert into compatibility.ISO639 values ('kw','CORNISH');
insert into compatibility.ISO639 values ('ky','KIRGHIZ');
insert into compatibility.ISO639 values ('la','LATIN');
insert into compatibility.ISO639 values ('lad','LADINO');
insert into compatibility.ISO639 values ('lan','LANGO');
insert into compatibility.ISO639 values ('lb','LUXEMBOURGISH');
insert into compatibility.ISO639 values ('lg','GANDA');
insert into compatibility.ISO639 values ('li','LIMBURGIAN');
insert into compatibility.ISO639 values ('lij','LIGURIAN');
insert into compatibility.ISO639 values ('lmo','LOMBARD');
insert into compatibility.ISO639 values ('ln','LINGALA');
insert into compatibility.ISO639 values ('lo','LAO');
insert into compatibility.ISO639 values ('lt','LITHUANIAN');
insert into compatibility.ISO639 values ('lv','LATVIAN');
insert into compatibility.ISO639 values ('mg','MALAGASY');
insert into compatibility.ISO639 values ('mh','MARSHALLESE');
insert into compatibility.ISO639 values ('mi','MAORI');
insert into compatibility.ISO639 values ('mk','MACEDONIAN');
insert into compatibility.ISO639 values ('ml','MALAYALAM');
insert into compatibility.ISO639 values ('mn','MONGOLIAN');
insert into compatibility.ISO639 values ('mo','MOLDAVIAN');
insert into compatibility.ISO639 values ('mr','MARATHI');
insert into compatibility.ISO639 values ('ms','MALAY');
insert into compatibility.ISO639 values ('mt','MALTESE');
insert into compatibility.ISO639 values ('my','MYANMAR');
insert into compatibility.ISO639 values ('mus','CREEK');
insert into compatibility.ISO639 values ('na','NAURUAN');
insert into compatibility.ISO639 values ('nah','NAHUATL');
insert into compatibility.ISO639 values ('nap','NEAPOLITAN');
insert into compatibility.ISO639 values ('ne','NEPALI');
insert into compatibility.ISO639 values ('new','NEWAR');
insert into compatibility.ISO639 values ('ng','NDONGA');
insert into compatibility.ISO639 values ('nl','DUTCH');
insert into compatibility.ISO639 values ('nb','NORWEGIAN');
insert into compatibility.ISO639 values ('nn','NORWEGIAN');
insert into compatibility.ISO639 values ('no','NORWEGIAN');
insert into compatibility.ISO639 values ('nrm','NORMAN');
insert into compatibility.ISO639 values ('nv','NAVAJO');
insert into compatibility.ISO639 values ('ny','CHICHEWA');
insert into compatibility.ISO639 values ('oc','OCCITAN');
insert into compatibility.ISO639 values ('oj','OJIBWA');
insert into compatibility.ISO639 values ('om','OROMO');
insert into compatibility.ISO639 values ('or','ORIYA');
insert into compatibility.ISO639 values ('os','OSSETIAN');
insert into compatibility.ISO639 values ('pa','PUNJABI');
insert into compatibility.ISO639 values ('pag','PANGASINAN');
insert into compatibility.ISO639 values ('pam','KAPAMPANGAN');
insert into compatibility.ISO639 values ('pap','PAPIAMENTU');
insert into compatibility.ISO639 values ('pi','PALI');
insert into compatibility.ISO639 values ('pih','NORFOLK');
insert into compatibility.ISO639 values ('pl','POLISH');
insert into compatibility.ISO639 values ('pms','PIEDMONTESE');
insert into compatibility.ISO639 values ('ps','PASHTO');
insert into compatibility.ISO639 values ('pt','PORTUGUESE');
insert into compatibility.ISO639 values ('qu','QUECHUA');
insert into compatibility.ISO639 values ('rm','ROMANSCH');
insert into compatibility.ISO639 values ('rmy','ROMANI');
insert into compatibility.ISO639 values ('rn','KIRUNDI');
insert into compatibility.ISO639 values ('ro','ROMANIAN');
insert into compatibility.ISO639 values ('ru','RUSSIAN');
insert into compatibility.ISO639 values ('rup','ARUMANIAN');
insert into compatibility.ISO639 values ('rw','RWANDI');
insert into compatibility.ISO639 values ('sa','SANSKRIT');
insert into compatibility.ISO639 values ('sc','SARDINIAN');
insert into compatibility.ISO639 values ('scn','SICILIAN');
insert into compatibility.ISO639 values ('sco','SCOTS');
insert into compatibility.ISO639 values ('sd','SINDHI');
insert into compatibility.ISO639 values ('se','SAMI');
insert into compatibility.ISO639 values ('sg','SANGO');
insert into compatibility.ISO639 values ('sh','SERBO');
insert into compatibility.ISO639 values ('si','SINHALA');
insert into compatibility.ISO639 values ('sk','SLOVAK');
insert into compatibility.ISO639 values ('sl','SLOVENIAN');
insert into compatibility.ISO639 values ('sm','SAMOAN');
insert into compatibility.ISO639 values ('sn','SHONA');
insert into compatibility.ISO639 values ('so','SOMALIA');
insert into compatibility.ISO639 values ('sq','ALBANIAN');
insert into compatibility.ISO639 values ('sr','SERBIAN');
insert into compatibility.ISO639 values ('st','SESOTHO');
insert into compatibility.ISO639 values ('su','SUNDANESE');
insert into compatibility.ISO639 values ('sv','SWEDISH');
insert into compatibility.ISO639 values ('ta','TAMIL');
insert into compatibility.ISO639 values ('te','TELUGU');
insert into compatibility.ISO639 values ('th','THAI');
insert into compatibility.ISO639 values ('tk','TURKMEN');
insert into compatibility.ISO639 values ('tr','TURKISH');
insert into compatibility.ISO639 values ('tt','TATAR');
insert into compatibility.ISO639 values ('uk','UKRAINIAN');
insert into compatibility.ISO639 values ('ur','URDU');
insert into compatibility.ISO639 values ('uz','UZBEK');
insert into compatibility.ISO639 values ('vi','VIETNAMESE');
insert into compatibility.ISO639 values ('war','SORBIAN');
insert into compatibility.ISO639 values ('xal','KALMYK');
insert into compatibility.ISO639 values ('yi','YIDDISH');
insert into compatibility.ISO639 values ('yo','YORUBA');
insert into compatibility.ISO639 values ('zh','CHINESE_PINYIN');
insert into compatibility.ISO639 values ('zu','ZULU');
grant select on compatibility.ISO639 to public;

set ?='Now deleting all rows for which there is no Mimer collation';
delete from compatibility.ISO639 where not exists (select 1 from information_schema.collations where substring(information_schema.collations.collation_name from 1 for character_length(compatibility.ISO639.collation_name))=compatibility.ISO639.collation_name and begins(substring(information_schema.collations.collation_name from character_length(compatibility.ISO639.collation_name)+1),'_'));

set ?='Now Checking to ensure there is at least one compatibility.ISO639 mapping for each relevant Mimer collation';
select cast(information_schema.collations.collation_name as nvarchar(32)) from information_schema.collations where not exists (
select 1 from compatibility.ISO639 where substring(information_schema.collations.collation_name from 1 for character_length(compatibility.ISO639.collation_name))=compatibility.ISO639.collation_name and begins(substring(information_schema.collations.collation_name from character_length(compatibility.ISO639.collation_name)+1),'_')
) order by information_schema.collations.collation_name;


@
create function compatibility.getCollationName(codeName varchar(5))
returns varchar(10)
begin

    declare v_value varchar(10);
    declare shortCode varchar(10);
    
    set shortCode = case 
                        when position('_', codeName) > 0 
                                then substring(codeName from 1 for (position('_', codeName) -1)) 
                        else codeName end ;

    
    set v_value = case shortCode
                        WHEN 'af' THEN 'AFRIKAANS'
                        WHEN 'ar' THEN 'ARABIC'
                        WHEN 'as' THEN 'ASSAMESE'
                        WHEN 'ast' THEN 'ASTURIAN'
                        WHEN 'az' THEN 'AZERBAIJANI'
                        WHEN 'be' THEN 'BELARUSIAN'
                        WHEN 'bg' THEN 'BULGARIAN'
                        WHEN 'bn' THEN 'BENGALI'
                        WHEN 'bo' THEN 'TIBETAN'
                        WHEN 'br' THEN 'BRETON'
                        WHEN 'bs' THEN 'BOSNIAN'
                        WHEN 'ca' THEN 'CATALAN'
                        WHEN 'co' THEN 'CORSICAN'
                        WHEN 'cs' THEN 'CZECH'
                        WHEN 'cy' THEN 'WELSH'
                        WHEN 'da' THEN 'DANISH'
                        WHEN 'de' THEN 'GERMAN'
                        WHEN 'dz' THEN 'DZONGKHA'
                        WHEN 'el' THEN 'GREEK'
                        WHEN 'en' THEN 'ENGLISH'
                        WHEN 'eo' THEN 'ESPERANTO'
                        WHEN 'es' THEN 'SPANISH'
                        WHEN 'et' THEN 'ESTONIAN'
                        WHEN 'eu' THEN 'BASQUE'
                        WHEN 'fa' THEN 'PERSIAN'
                        WHEN 'fi' THEN 'FINNISH'
                        WHEN 'fil' THEN 'FILIPINO'
                        WHEN 'fo' THEN 'FAROESE'
                        WHEN 'fr' THEN 'FRENCH'
                        WHEN 'fur' THEN 'FRIULIAN'
                        WHEN 'fy' THEN 'FRISIAN'
                        WHEN 'ga' THEN 'IRISH'
                        WHEN 'gd' THEN 'SCOTTISH_GAELIC'
                        WHEN 'gl' THEN 'GALICIAN'
                        WHEN 'gu' THEN 'GUJARATI'
                        WHEN 'ha' THEN 'HAUSA'
                        WHEN 'he' THEN 'HEBREW'
                        WHEN 'hi' THEN 'HINDI'
                        WHEN 'hr' THEN 'CROATIAN'
                        WHEN 'hu' THEN 'HUNGARIAN'
                        WHEN 'hy' THEN 'ARMENIAN'
                        WHEN 'id' THEN 'INDONESIAN'
                        WHEN 'ig' THEN 'IGBO'
                        WHEN 'is' THEN 'ICELANDIC'
                        WHEN 'it' THEN 'ITALIAN'
                        WHEN 'ja' THEN 'JAPANESE'
                        WHEN 'jv' THEN 'JAVANESE'
                        WHEN 'ka' THEN 'GEORGIAN'
                        WHEN 'kk' THEN 'KAZAKH'
                        WHEN 'kl' THEN 'GREENLANDIC'
                        WHEN 'km' THEN 'KHMER'
                        WHEN 'kn' THEN 'KANNADA'
                        WHEN 'ko' THEN 'KOREAN'
                        WHEN 'ku' THEN 'KURDISH'
                        WHEN 'ky' THEN 'KIRGHIZ'
                        WHEN 'la' THEN 'LATIN'
                        WHEN 'lb' THEN 'LUXEMBOURGISH'
                        WHEN 'lo' THEN 'LAO'
                        WHEN 'lt' THEN 'LITHUANIAN'
                        WHEN 'lv' THEN 'LATVIAN'
                        WHEN 'mk' THEN 'MACEDONIAN'
                        WHEN 'ml' THEN 'MALAYALAM'
                        WHEN 'mo' THEN 'MOLDAVIAN'
                        WHEN 'mr' THEN 'MARATHI'
                        WHEN 'ms' THEN 'MALAY'
                        WHEN 'mt' THEN 'MALTESE'
                        WHEN 'my' THEN 'MYANMAR'
                        WHEN 'ne' THEN 'NEPALI'
                        WHEN 'nl' THEN 'DUTCH'
                        WHEN 'nb' THEN 'NORWEGIAN'
                        WHEN 'nn' THEN 'NORWEGIAN'
                        WHEN 'no' THEN 'NORWEGIAN'
                        WHEN 'oc' THEN 'OCCITAN'
                        WHEN 'or' THEN 'ORIYA'
                        WHEN 'pa' THEN 'PUNJABI'
                        WHEN 'pl' THEN 'POLISH'
                        WHEN 'pt' THEN 'PORTUGUESE'
                        WHEN 'rm' THEN 'ROMANSCH'
                        WHEN 'ro' THEN 'ROMANIAN'
                        WHEN 'ru' THEN 'RUSSIAN'
                        WHEN 'rup' THEN 'ARUMANIAN'
                        WHEN 'sa' THEN 'SANSKRIT'
                        WHEN 'sco' THEN 'SCOTS'
                        WHEN 'se' THEN 'SAMI'
                        WHEN 'si' THEN 'SINHALA'
                        WHEN 'sk' THEN 'SLOVAK'
                        WHEN 'sl' THEN 'SLOVENIAN'
                        WHEN 'sq' THEN 'ALBANIAN'
                        WHEN 'sr' THEN 'SERBIAN'
                        WHEN 'st' THEN 'SESOTHO'
                        WHEN 'sv' THEN 'SWEDISH'
                        WHEN 'ta' THEN 'TAMIL'
                        WHEN 'te' THEN 'TELUGU'
                        WHEN 'th' THEN 'THAI'
                        WHEN 'tk' THEN 'TURKMEN'
                        WHEN 'tr' THEN 'TURKISH'
                        WHEN 'tt' THEN 'TATAR'
                        WHEN 'uk' THEN 'UKRAINIAN'
                        WHEN 'ur' THEN 'URDU'
                        WHEN 'uz' THEN 'UZBEK'
                        WHEN 'vi' THEN 'VIETNAMESE'
                        WHEN 'war' THEN 'SORBIAN'
                        WHEN 'yi' THEN 'YIDDISH'
                        WHEN 'yo' THEN 'YORUBA'
                        WHEN 'zh' THEN 'CHINESE_PINYIN'
                        WHEN 'zu' THEN 'ZULU'
                  end;
    return v_value;

end
@
grant execute on function compatibility.getCollationName to public;
