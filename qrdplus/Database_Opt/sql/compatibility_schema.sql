-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

whenever error exit;
set sqlite on;
create databank sqlite_comp of 100 pages;
create schema compatibility;
create unique sequence compatibility.sqlite;
grant usage on sequence compatibility.sqlite to public;

-- A predefined sequence for short lived tables, which may be reset at times by the application.
create sequence compatibility.temporary_sqlite;
grant usage on sequence compatibility.temporary_sqlite to public;

--
-- View to make it possible for anyone to check what the current collation is.
-- The current collation information is normally hidden in a SYSTEM table which is only
-- accessible by SYSADM.
--
--create view "compatibility"."current_collation"("language","collation") as
--select cast(replace(OBJECT_NAME,'_3','') as varchar(32)),cast(object_name as varchar(32))
--from system.server_info join system.objects on (cast(attribute_value as integer)=object_sysid) 
--where server_attribute='CURRENT_COLLATION_ID';
create view "compatibility"."current_collation"("language","collation") as
select cast(case substring(OBJECT_NAME from character_length(OBJECT_NAME)-1)
            when '_3' then substring(OBJECT_NAME from 1 for character_length(OBJECT_NAME)-2)
            else OBJECT_NAME
            end as varchar(32)),
       cast(object_name as varchar(32))
from system.server_info join system.objects on (cast(attribute_value as integer)=object_sysid) 
where server_attribute='CURRENT_COLLATION_ID';
grant select on "compatibility"."current_collation" to public;


create table compatibility.database_mappings(path nvarchar(256), databank nvarchar(128), schema nvarchar(128) collate SQL_IDENTIFIER, p_ident nvarchar(128), lru TIMESTAMP, primary key(path));
grant select on compatibility.database_mappings to public;
@
create procedure compatibility.map_database(database_file nvarchar(256), databank_name nvarchar(128), schema_name nvarchar(128), p_ident_name nvarchar(128), p_lru timestamp)
modifies sql data
begin
	delete from compatibility.database_mappings where databank = databank_name and schema = schema_name;
	insert into compatibility.database_mappings(path, databank, lru, schema, p_ident) values (database_file, databank_name, p_lru, schema_name, p_ident_name);
end
@
grant execute on procedure compatibility.map_database(nvarchar(256),nvarchar(128),nvarchar(128),nvarchar(128),timestamp) to public;

@
create procedure compatibility.map_database(database_file nvarchar(256), databank_name nvarchar(128), schema_name nvarchar(128), p_ident_name nvarchar(128))
modifies sql data
begin
	call compatibility.map_database(database_file, databank_name, schema_name, p_ident_name, LOCALTIMESTAMP);
end
@
grant execute on procedure compatibility.map_database(nvarchar(256),nvarchar(128),nvarchar(128),nvarchar(128)) to public;

@
create procedure compatibility.map_auxiliary_database(database_file nvarchar(256), databank_name nvarchar(128), schema_name nvarchar(128), p_ident_name nvarchar(128), p_lru timestamp)
modifies sql data
begin
	insert into compatibility.database_mappings(path, databank, lru, schema, p_ident) values (database_file, databank_name, p_lru, schema_name, p_ident_name);
end
@
grant execute on procedure compatibility.map_auxiliary_database(nvarchar(256), nvarchar(128), nvarchar(128), nvarchar(128), timestamp) to public;

@
create procedure compatibility.map_auxiliary_database(database_file nvarchar(256), databank_name nvarchar(128), schema_name nvarchar(128), p_ident_name nvarchar(128))
modifies sql data
begin
	insert into compatibility.database_mappings(path, databank, lru, schema, p_ident) values (database_file, databank_name, LOCALTIMESTAMP, schema_name, p_ident_name);
end
@
grant execute on procedure compatibility.map_auxiliary_database(nvarchar(256), nvarchar(128), nvarchar(128), nvarchar(128)) to public;



@
create procedure compatibility.remove_database_mapping(database_file nvarchar(256))
modifies sql data
begin
	delete from compatibility.database_mappings where path = database_file;
end
@
grant execute on procedure compatibility.remove_database_mapping to public;

CREATE TABLE compatibility.pragmas (
        pragma_environment VARCHAR(128),
	pragma_name VARCHAR(128),
	pragma_value VARCHAR(128),
	CONSTRAINT compatibility.pragmas_pk PRIMARY KEY(pragma_environment,pragma_name)
) in sqlite_comp;
insert into compatibility.pragmas(pragma_environment, pragma_name, pragma_value) values('global', 'locale', 'en');


@
create function compatibility.getLocale()
returns varchar(5)
reads sql data
begin
declare curLang varchar(4);
select pragma_value into curLang from compatibility.pragmas where pragma_environment='global' and pragma_name='locale';

return curLang;
end
@
grant execute on function compatibility.getLocale to public;

@
create procedure compatibility.set_pragma(p_environment varchar(128),p_name varchar(128),p_value varchar(128))
modifies sql data
begin
  update compatibility.pragmas 
    set pragma_value=p_value 
    where pragma_environment=p_environment and pragma_name=p_name;
end
@
grant execute on procedure compatibility.set_pragma to public;

-- COMPATIBLITY.SET_OR_CREATE_PRAGMA
--
-- Sets the value of a specific pragma. If the pragma is not yet set, an entry for the pragma is created.
-- Note that, as long as pragmas are quiered for via the procedure COMPATIBILITY.GET_PRAGMA if there is no
-- entry for the pragma, the default value for the pragma is returned as specified in COMPATIBILITY.GET_DEFAULT_PRAGMA.
-- 
-- @param p_environment The environment holding the pragma (a databank name or a schema name).
-- @param p_name The name of the pragma.
-- @param p_value The new value of the pragma.
@
create procedure compatibility.set_or_create_pragma(p_environment varchar(128),p_name varchar(128),p_value varchar(128))
modifies sql data
begin
  declare exit handler for not found begin
    insert into compatibility.pragmas(pragma_environment,pragma_name,pragma_value) values (p_environment,p_name,p_value);
  end;

  update compatibility.pragmas set pragma_value=p_value where pragma_environment=p_environment and pragma_name=p_name;
end
@
grant execute on procedure compatibility.set_or_create_pragma to public;

--
-- COMPATIBLITY.GET_DEFAULT_PRAGMA
--
-- Function to return the default value for a specific pragma.
--
-- @param p_name The name of the pragma to return the default value for.
-- @return The default value for the pragma.
--
@
create function compatibility.get_default_pragma(p_name varchar(128))
returns varchar(128)
begin
    declare v_value varchar(128);

    set v_value = case p_name
                  when 'auto_vacuum' THEN '0'
                  when 'automatic_index' THEN '1'
                  when 'cache_size' THEN '2000'
                  when 'case_sensitive_like' THEN '1'
                  when 'count_changes' THEN '1'
                  when 'user_version' THEN '0'
                  when 'default_cache_size' THEN '2000'
                  when 'empty_result_callbacks' THEN '0'
                  when 'encoding' THEN ''
                  when 'freelist_count' THEN '0'
                  when 'full_column_names' THEN '1'
                  when 'fullfsync' THEN '1'
                  when 'incremental_vacuum' THEN '0'
                  when 'integrity_check' THEN '100'
                  when 'journal_mode' THEN 'DELETE'
                  when 'journal_size_limit' THEN '-1'
                  when 'legacy_file_format' THEN '0'
                  when 'locking_mode' THEN '0'
                  when 'max_page_count' THEN '1073741823'
                  when 'page_size' THEN '2048'
                  when 'parser_trace' THEN '0'
                  when 'quick_check' THEN '100'
                  when 'read_uncommitted' THEN '0'
                  when 'recursive_triggers' THEN '1'
                  when 'reverse_unordered_selects' THEN '0'
                  when 'schema_version' THEN '0'
                  when 'secure_delete' THEN '0'
                  when 'short_column_names' THEN '0'
                  when 'synchronous' THEN '2'
                  when 'temp_store' THEN '0'
                  when 'temp_store_directory' THEN ''
                  when 'user_version' THEN '0'
                  when 'vdbe_listing' THEN '0'
                  when 'vdbe_trace' THEN '0'
                  when 'wal_autocheckpoint' THEN '1000'
                  end;
    return v_value;
end
@
grant execute on function compatibility.get_default_pragma to public;

-- 
-- 
@
create procedure compatibility.get_pragma(p_environment nvarchar(128),p_name nvarchar(128),p_value nvarchar(128))
values (nvarchar(128),nvarchar(128),nvarchar(128),nvarchar(128),nvarchar(128))
reads sql data
begin
  declare curs_database_list cursor for select databank_name,file_name from information_schema.ext_databanks where is_online='YES' and databank_creator<>'SYSTEM';
  declare curs_table_info cursor for select column_name,data_type,case is_nullable when 'Yes' then '1' else '0' end,column_default from information_schema.columns where table_name=p_value and 
  (
table_schema IN
    (SELECT "schema"
        FROM compatibility.database_mappings
        WHERE databank = p_environment)
OR
table_schema IN
    (SELECT DATABANK_CREATOR
        FROM INFORMATION_SCHEMA.EXT_DATABANKS
        WHERE DATABANK_NAME = p_environment)
)
   order by ordinal_position;
  declare v_value1,v_value2,v_value3,v_value4,v_value5 nvarchar(128);
  declare v_count integer;

  declare v_eot boolean;
  declare continue handler for not found set v_eot = true;
  set v_eot = false;

  case p_name
  when 'database_list' then 
    set v_count = 0;
    begin
      open curs_database_list;
      set v_count = 0;
      while not v_eot do
        fetch curs_database_list into v_value2,v_value3;
	if not v_eot then
          set v_count = v_count+1;
          return (cast(v_count as nvarchar(128)),v_value2,v_value3,null,null);
        end if;
      end while;
      close curs_database_list;
    end;
  when 'table_info' then
    -- The table_info pragma is no longer supported by this procedure
    signal sqlstate 'UE001';
  when 'collation_list' then
    set v_count = 0;
  when 'compile_options' then
    set v_count = 0;
  when 'foreign-key-list' then
    set v_count = 0;
  when 'index_info' then
    set v_count = 0;
  when 'index_list' then
    set v_count = 0;
  when 'integrity_check' then
    return ('ok',null,null,null,null);
  when 'quick_check' then
    return ('ok',null,null,null,null);
  else
    select pragma_value into v_value1 from pragmas where pragma_environment=p_environment and pragma_name=p_name;
    if v_eot then
      set v_value1 = get_default_pragma(p_name);
    end if;
    return (v_value1,null,null,null,null);
  end case;
end
@
grant execute on procedure compatibility.get_pragma to public;



@
create function compatibility.glob(pattern nchar varying(128), string nchar varying(128))
returns boolean
--
-- do a search using glob wildcards
--
-- supports
-- * zero or more arbitrary characters
-- ? one arbitrary character
-- [range] any of character in range (ranges specified by start-end not supported)
--    [range <eos> is treated as [range]
--    wildcards within [] are treated as ordinary characters
--    a leading ] is treated as an ordinary character
-- \ escape character. \<eos> is treated as \\
--
begin
    declare b boolean;
    declare i,j integer default 1;
    declare sl,pl,startBracket,endBracket integer;
    declare c1,c2 nchar;

    if  string is not null
    and pattern is not null then
        set b = true;
        set pl = char_length(pattern);
        set sl = char_length(string);
        l1:
        while i <= pl do
            set c1 = substring(pattern from i for 1);
            if  c1 = n'\' then
--'
-- handle escape
--
                if  i < pl then
                    set i = i + 1;
                end if;
                set c1 = substring(pattern from i for 1);
                if  c1 <> substring(string from j for 1) then
                    set b = false;
                    leave l1;
                end if;
                set j = j + 1;
            else
--
-- process possible wildcards
--
                case c1
                when n'?' then
                    set j = j + 1;
                when n'*' then
                    l2:
                    loop
                        if  substring(pattern from i for 1) <> n'*' then
                            leave l2;
                        end if;
                        set i = i + 1;
                    end loop;
                    set c2 = substring(pattern from i + 1 for 1);
                    if  c2 <> n'' then
                        l3:
                        loop
                            if  j > sl then
                                leave l1;
                            end if;
                            if  c2 = substring(string from j for 1) then
                                leave l3;
                            else
                                set b = false;
                                leave l1;
                            end if;
                            set j = j + 1;
                        end loop;
                    else
                        set j = sl;
                        leave l1;
                    end if;
                when n'[' then
                    set endbracket = i + 1;
                    l4:
                    loop
                        if  endbracket > pl then
                            leave l4;
                        end if;
                        if  substring(pattern from endbracket for 1) = n']' then
                            leave l4;
                        end if;
                        set endBracket = endBracket + 1;
                    end loop;
                    set startBracket = i + 1;
                    if  startBracket = endBracket then
                        set i = i + 1;
                    else
                        l5:
                        loop
                            if  j > sl then
                                set b = false;
                                leave l1;
                            end if;
                            if  startBracket = endBracket then
                                if  endBracket - i > 1 then
                                    set b = false;
                                    leave l1;
                                end if;
                                leave l5;
                            end if;
                            if  substring(string from j for 1) =
                                substring(pattern from startBracket for 1) then
                                set i = endBracket + 1;
                                set j = j + 1;
                                leave l5;
                            end if;
                            set startBracket = startBracket + 1;
                        end loop;
                    end if;
                else
                    if  substring(pattern from i for 1) <>
                        substring(string from j for 1) then
                        set b = false;
                        leave l1;
                    end if;
                    set j = j + 1;
                end case;
            end if;
            set i = i + 1;
        end while;
    end if;
    return b and i >= pl and j >= sl;
end
@
grant execute on function compatibility.glob to public;


CREATE TABLE compatibility.default_char_map (
	unicode_first NCHAR(1) NOT NULL,
	unicode_second NCHAR(1)
) in sqlite_comp;

INSERT INTO compatibility.default_char_map VALUES(U&'\00C6',    'A'    );
INSERT INTO compatibility.default_char_map VALUES(U&'\00DF',    'S'    );
INSERT INTO compatibility.default_char_map VALUES(U&'\1100', U&'\3131');
INSERT INTO compatibility.default_char_map VALUES(U&'\1101', U&'\3132');
INSERT INTO compatibility.default_char_map VALUES(U&'\1102', U&'\3134');
INSERT INTO compatibility.default_char_map VALUES(U&'\1103', U&'\3137');
INSERT INTO compatibility.default_char_map VALUES(U&'\1104', U&'\3138');
INSERT INTO compatibility.default_char_map VALUES(U&'\1105', U&'\3139');
INSERT INTO compatibility.default_char_map VALUES(U&'\1106', U&'\3141');
INSERT INTO compatibility.default_char_map VALUES(U&'\1107', U&'\3142');
INSERT INTO compatibility.default_char_map VALUES(U&'\1108', U&'\3143');
INSERT INTO compatibility.default_char_map VALUES(U&'\1109', U&'\3145');
INSERT INTO compatibility.default_char_map VALUES(U&'\110A', U&'\3146');
INSERT INTO compatibility.default_char_map VALUES(U&'\110B', U&'\3147');
INSERT INTO compatibility.default_char_map VALUES(U&'\110C', U&'\3148');
INSERT INTO compatibility.default_char_map VALUES(U&'\110D', U&'\3149');
INSERT INTO compatibility.default_char_map VALUES(U&'\110E', U&'\314A');
INSERT INTO compatibility.default_char_map VALUES(U&'\110F', U&'\314B');
INSERT INTO compatibility.default_char_map VALUES(U&'\1110', U&'\314C');
INSERT INTO compatibility.default_char_map VALUES(U&'\1111', U&'\314D');
INSERT INTO compatibility.default_char_map VALUES(U&'\1112', U&'\314E');
INSERT INTO compatibility.default_char_map VALUES(U&'\111A', U&'\3140');
INSERT INTO compatibility.default_char_map VALUES(U&'\1121', U&'\3144');
INSERT INTO compatibility.default_char_map VALUES(U&'\1161', U&'\314F');
INSERT INTO compatibility.default_char_map VALUES(U&'\1162', U&'\3150');
INSERT INTO compatibility.default_char_map VALUES(U&'\1163', U&'\3151');
INSERT INTO compatibility.default_char_map VALUES(U&'\1164', U&'\3152');
INSERT INTO compatibility.default_char_map VALUES(U&'\1165', U&'\3153');
INSERT INTO compatibility.default_char_map VALUES(U&'\1166', U&'\3154');
INSERT INTO compatibility.default_char_map VALUES(U&'\1167', U&'\3155');
INSERT INTO compatibility.default_char_map VALUES(U&'\1168', U&'\3156');
INSERT INTO compatibility.default_char_map VALUES(U&'\1169', U&'\3157');
INSERT INTO compatibility.default_char_map VALUES(U&'\116A', U&'\3158');
INSERT INTO compatibility.default_char_map VALUES(U&'\116B', U&'\3159');
INSERT INTO compatibility.default_char_map VALUES(U&'\116C', U&'\315A');
INSERT INTO compatibility.default_char_map VALUES(U&'\116D', U&'\315B');
INSERT INTO compatibility.default_char_map VALUES(U&'\116E', U&'\315C');
INSERT INTO compatibility.default_char_map VALUES(U&'\116F', U&'\315D');
INSERT INTO compatibility.default_char_map VALUES(U&'\1170', U&'\315E');
INSERT INTO compatibility.default_char_map VALUES(U&'\1171', U&'\315F');
INSERT INTO compatibility.default_char_map VALUES(U&'\1172', U&'\3160');
INSERT INTO compatibility.default_char_map VALUES(U&'\1173', U&'\3161');
INSERT INTO compatibility.default_char_map VALUES(U&'\1174', U&'\3162');
INSERT INTO compatibility.default_char_map VALUES(U&'\1175', U&'\3163');
INSERT INTO compatibility.default_char_map VALUES(U&'\11AA', U&'\3133');
INSERT INTO compatibility.default_char_map VALUES(U&'\11AC', U&'\3135');
INSERT INTO compatibility.default_char_map VALUES(U&'\11AD', U&'\3136');
INSERT INTO compatibility.default_char_map VALUES(U&'\11B0', U&'\313A');
INSERT INTO compatibility.default_char_map VALUES(U&'\11B1', U&'\313B');
INSERT INTO compatibility.default_char_map VALUES(U&'\11B3', U&'\313D');
INSERT INTO compatibility.default_char_map VALUES(U&'\11B4', U&'\313E');
INSERT INTO compatibility.default_char_map VALUES(U&'\11B5', U&'\313F');

@
CREATE FUNCTION  compatibility.map_character(c nchar(1))
RETURNS nchar varying(1)
READS SQL DATA
BEGIN
	DECLARE cm nchar(1);
 	SELECT unicode_second INTO cm FROM compatibility.default_char_map
 	WHERE unicode_first = c;
 	RETURN COALESCE(cm,unicode_char(0));
END
@
grant execute on function compatibility.map_character to public;

@
CREATE FUNCTION  compatibility.is_CJK(c_code nchar)
RETURNS boolean
BEGIN
	RETURN	(U&'\4e00'<= c_code AND c_code <= U&'\9fff')
   		OR (U&'\3400'<= c_code AND c_code <= U&'\4dbf')
   		OR (U&'\3000'<= c_code AND c_code <= U&'\303f')
   		OR (U&'\2e80'<= c_code AND c_code <= U&'\2eff')
   		OR (U&'\3300'<= c_code AND c_code <= U&'\33ff')
   		OR (U&'\fe30'<= c_code AND c_code <= U&'\fe4f')
   		OR (U&'\f900'<= c_code AND c_code <= U&'\faff');
 END
@ 
grant execute on function compatibility.is_CJK to public;

@
CREATE FUNCTION compatibility.getPhoneBookIndex(iter nchar varying(129),locale char(5))
RETURNS nchar varying(129)
BEGIN
	DECLARE code_point, c_mapped nchar varying(1);
	DECLARE min_output_size integer DEFAULT 1;
	IF char_length(iter) < min_output_size THEN
		signal sqlstate 'UE001';
	END IF;
	SET code_point = SUBSTRING(iter from 1 for 1);
	SET code_point = UPPER(code_point);
	SET c_mapped = map_character(code_point);
	IF c_mapped IS NOT NULL THEN
		return c_mapped;
	END IF;
	IF code_point BETWEEN U&'\3042' AND U&'\309F' THEN
		RETURN CASE
				WHEN code_point < U&'\304B' then U&'\3042'
	 			WHEN code_point < U&'\3055' then U&'\304B'
				WHEN code_point < U&'\305F' then U&'\3055'
				WHEN code_point < U&'\306A' then U&'\305F'
				WHEN code_point < U&'\306F' then U&'\306A'
				WHEN code_point < U&'\307E' then U&'\306F'
				WHEN code_point < U&'\3084' then U&'\307E'
				WHEN code_point < U&'\3089' then U&'\3084'
				WHEN code_point < U&'\308F' then U&'\3089'
				ELSE U&'\308F' 
			END;
	END IF;
	IF is_CJK(code_point) THEN
		IF locale = 'ja' THEN
			RETURN U&'\4ED6';
 		END IF;
 		RETURN n'';
	END IF;
	RETURN code_point;
END
@
grant execute on function compatibility.getPhoneBookIndex to public;

@
create function compatibility.phone_number_compare_inter(org_a nvarchar(128),org_b nvarchar(128),accept_thailand_case boolean)
returns boolean
return sysadm.phone_number_compare_inter(org_a,org_b,accept_thailand_case);
@
grant execute on function compatibility.phone_number_compare_inter to public;


@
create function compatibility.phone_numbers_equal(a nchar varying(129),
                                            b nchar varying(129))
returns boolean
SPECIFIC phone_numbers_equal_2
begin
	if a = b then
		return true;
	elseif a is null or b is null then
		return false;
	else
		return compatibility.phone_number_compare_inter(a,b, true);
	end if;
end
@
grant execute on specific function compatibility.phone_numbers_equal_2 to public;

@
create function compatibility.phone_numbers_equal(a nchar varying(129),
                                            b nchar varying(129), accept_thai integer)
returns boolean
SPECIFIC phone_numbers_equal_3
begin
	if a = b then
		return true;
	elseif a is null or b is null then
		return false;
	else
		return compatibility.phone_number_compare_inter(a,b, accept_thai<>0);
	end if;
end
@
grant execute on specific function compatibility.phone_numbers_equal_3 to public;

 create view compatibility.cur_collation as select object_name from system.objects o join system.server_info s on(cast(
o.object_sysid as varchar(20)) = s.attribute_value) where s.server_attribute = 
'CURRENT_COLLATION_ID';
grant select on compatibility.cur_collation to public;

@
create function compatibility.min(a nchar varying(129),
       				  b nchar varying(129))
returns nchar varying(129)
return 
       case when a < b then a
       else b end
@
grant execute on function compatibility.min(nchar varying(129),nchar varying(129)) to public;

@
create function compatibility.min(a integer, b integer)
returns integer
return 
	case when a < b then a
	else b end
@
grant execute on function compatibility.min(int, int) to public;


@
create function compatibility.strftime(format varchar(30),timest timestamp(9))
returns nchar varying(128)
--
-- initial implementation of sqlite function strftime,
-- see http://www.sqlite.org/lang_datefunc.html
--

begin
    declare t timestamp(9);
    declare i bigint;
    declare l,p integer;
    declare d decimal(15,3);
    declare c char;
    declare s nchar varying(128);

    set t = timest;
        set l = char_length(format);
        set p = 1;
        set s = n'';
        while p <= l do
            set c = substring(format from p for 1);
            case c
            when '%' then
                 set p = p + 1;
                 set c = substring(format from p for 1);
                 case c
                 when 's' then
                     set i = cast((t - timestamp '1970-01-01 00:00:00')
                         second(12,0) as bigint);
                     set s = s || cast(i as nchar varying(20));
                 when 'd' then
                     set i = extract(day from t);
                     set s = s || case when i < 10 then n'0' else n'' end ||
                         cast(i as nchar varying(20));
                 when 'f' then
                     set d = extract(second from t);
                     set s = s || case when d < 10 then n'0' else n'' end ||
                         cast(d as nchar varying(20));
                 when 'H' then
                     set i = extract(hour from t);
                     set s = s || case when i < 10 then n'0' else n'' end ||
                         cast(i as nchar varying(20));
                 when 'j' then
                      set i = dayofyear(t);
                      set s = s || case when i < 10 then n'00'
                                        when i < 100 then n'0'
                                        else n'' end ||
                          cast(i as nchar varying(20));
                 when 'J' then
                     set i = cast((t - timestamp '0001-01-01 00:00:00')
                         day(7) as bigint);
                     set i = i + 1721713;
                     set s = s || cast(i as nchar varying(20));
                when 'm' then
                    set i = extract(month from t);
                    set s = s || case when i < 10 then n'0' else n'' end ||
                        cast(i as nchar varying(20));
                when 'M' then
                    set i = extract(minute from t);
                    set s = s || case when i < 10 then n'0' else n'' end ||
                        cast(i as nchar varying(20));
                when 'S' then
                    set i = extract(second from t);
                    set s = s || case when i < 10 then n'0' else n'' end ||
                        cast(i as nchar varying(20));
                when 'w' then
                    set i = dayofweek(t);
                    set i = case when i = 7 then 0 else i end;
                    set s = s || cast(i as nchar varying(20));
                when 'W' then
                    set i = week(t);
                    set s = s || case when i < 10 then n'0' else n'' end ||
                        cast(i as nchar varying(20));
                when 'Y' then
                    set i = extract(year from t);
                    set s = s || case when i < 10 then n'000'
                                      when i < 100 then n'00'
                                      when i < 1000 then n'0'
                                      else n'' end ||
                        cast(i as nchar varying(20));
                when '%' then
                    set s = s || '%';
                else
                    begin end;
                end case;
            else
                set s = s || cast(c as nchar);
            end case;
            set p = p + 1;
        end while;
    return s;
end
@
grant execute on function compatibility.strftime to public;

-- The below function handles the additional modifiers which in SQLite are used in calls to strftime, datetime, date, time
-- and julianday functions. The idea is that the translator should generate subsequent calls to this function to handle
-- the modifiers for each modifier.
--
-- SQLite accepts any number of modifiers to these functions, likewise can the translator stack any number of calls to 
-- this function to mimic the SQLite behavior.
--
-- Examples:
--   ... datetime('2013-04-20') ...   =>   ... compatibility.datetime_modifiers(current_timestamp,'2013-04-20') ...
--   ... datetime('now','start of year') ...  =>  ...compatibility.datetime_modifiers(compatibility.datetime_modifiers(current_timestamp,'now'),'start of year') ...
--   --- datetime('now','start of year','1 months') ... => ...compatibility.datetime_modifiers(compatibility.datetime_modifiers(compatibility.datetime_modifiers(current_timestamp,'now'),'start of year'),'1 months') ...
--
-- Known incompatibilities with the following implementation:
--   - When doing ..(timestamp'2013-01-31','1 months'), this implementation returns timestamp'2013-02-28', while
--     the corresponding SQLite implementation (for example, datetime('2013-01-31','1 months) will return '2013-03-03'.
--   - This implementation always return null if an illegal date is encountered (for example 2013-02-31) while it is 
--     perfectly fine to do datetime('2013-02-31','1 days') in SQLite (it will yield '2013-03-04').
--
@
create function compatibility.datetime_modifiers(dt timestamp,imodifier varchar(20))
returns timestamp
begin
  declare exit handler for sqlexception
  return null;
  declare modifier varchar(20);

  set modifier = lower(imodifier);

  return case
       -- NNN days, is similar to adding an interval day to the timestamp
       when substring(modifier from character_length(modifier)-3)='days' then
         dt+cast(substring(modifier from 1 for character_length(modifier)-4) as interval day(7))
       -- NNN hours, is similar to adding an interval hour to the timestamp
       when substring(modifier from character_length(modifier)-4)='hours' then
         dt+cast(substring(modifier from 1 for character_length(modifier)-5) as interval hour(8))
       -- NNN minutes, is similar to adding an interval minute to the timestamp
       when substring(modifier from character_length(modifier)-6)='minutes' then
         dt+cast(substring(modifier from 1 for character_length(modifier)-7) as interval minute(9))
       -- NNN seconds, is similar to adding an interval second to the timestamp
       when substring(modifier from character_length(modifier)-6)='seconds' then
         dt+cast(substring(modifier from 1 for character_length(modifier)-7) as interval second(12,9))
       -- NNN months, is similar to adding an interval month to the timestamp
       when substring(modifier from character_length(modifier)-5)='months' then
         dt+cast(substring(modifier from 1 for character_length(modifier)-6) as interval month(6))
       -- NNN years, is similar to adding an interval year to the timestamp
       when substring(modifier from character_length(modifier)-4)='years' then
         dt+cast(substring(modifier from 1 for character_length(modifier)-5) as interval year(4))
       -- start of month, should bring the timestamp back to the beginning of the month
       when modifier='start of month' then
         cast((substring(cast(dt as varchar(40)) from 11 for 7)||'-01 00:00:00') as timestamp)
       -- start of year, should bring the timestamp back to the first day of the year
       when modifier='start of year' then
         cast((substring(cast(dt as varchar(40)) from 11 for 4)||'-01-01 00:00:00') as timestamp)
       -- start of day, should bring the timestamp back to the beginning of the day
       when modifier='start of day' then
         cast((substring(cast(dt as varchar(40)) from 11 for 10)||' 00:00:00') as timestamp)
       -- weekday NN, brings the timestamp forwards until the next specified weekday where sunday=0
       when substring(modifier from 1 for 7)='weekday' then
         dt+cast(mod(
                     cast(substring(modifier from 8) as integer)
                                          -dayofweek(dt+interval'1'day)+7,7)+1 as interval day)
       -- In SQLite, this modifier should always be preceded by an integer value but we have already used that
       -- as a Julian date to make a timestamp. Here we go back to a Julian date from the timestamp
       -- and make a Unix date from that number,
       when modifier='unixepoch' then
         timestamp'1970-01-01 00:00:00'+cast(cast(((current_timestamp-timestamp'2000-01-01 12:00:00') day(7)) as integer)+2451545 as interval second(12))
       -- Convert from universal time to local time.
       when modifier='localtime' then
         dt-(builtin.utc_timestamp()-current_timestamp) minute(4)
       -- Convert from local time to universal time
       when modifier='utc' then
         dt+(builtin.utc_timestamp()-current_timestamp) minute(4)
       -- From now on, a number of timestamp values are being tested for
       -- 1. YYYY-MM-DD, a regular date
       when regexp_match(modifier,'^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9] *$') then
         cast(cast(modifier as date) as timestamp)
       -- 2. YYYY-MM-DD HH:MM, a timestamp without seconds
       when regexp_match(modifier,'^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9] [0-9][0-9]:[0-9][0-9] *$') then
         cast(substring(modifier from 1 for 16)||':00' as timestamp)
       -- 3. YYYY-MM-DD HH:MM:SS, a regular timestamp
       -- 4. YYYY-MM-DD HH:MM:SS.SSSS
       when regexp_match(modifier,'^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9] [0-9][0-9]:[0-9][0-9]:[0-9][0-9].[0-9]+ *$') then
         cast(modifier as timestamp)
       -- 5. YYYY-MM-DDTHH:MM, a timestamp without seconds and a T separator
       when regexp_match(modifier,'^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]T[0-9][0-9]:[0-9][0-9] *$') then
         cast(substring(modifier from 1 for 10)||' '||substring(modifier from 12 for 5) ||':00' as timestamp)
       -- 6. YYYY-MM-DDTHH:MM:SS, a regular timestamp with a T separator
       -- 7. YYYY-MM-DDTHH:MM:SS.SSSS
       when regexp_match(modifier,'^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]T[0-9][0-9]:[0-9][0-9]:[0-9][0-9].[0-9]+ *$') then
         cast(substring(modifier from 1 for 10)||' '||substring(modifier from 12) as timestamp)
       -- 8. HH:MM, a time specification without seconds
       when regexp_match(modifier,'^[0-9][0-9]:[0-9][0-9] *$') then
         cast(substring(modifier from 1 for 5)||':00' as timestamp)
       -- 9. HH:MM:SS, a regular time specification
       --10. HH:MM:SS.SSS
       when regexp_match(modifier,'^[0-9][0-9]:[0-9][0-9]:[0-9][0-9].[0-9]+ *$') then
         cast(cast(modifier as time) as timestamp)
       --11. now
       when modifier='now' then
         builtin.utc_timestamp()
       --12. DDDDDDDD, if everything else is exhausted we treat the modifier as an integer denoting a Julian date
       else
         timestamp'2000-01-01 12:00:00'+cast(cast(modifier as integer)-2451545 as interval day(7))
       end;
end
@
grant execute on function compatibility.datetime_modifiers to public;

-- Function used to convert a timestamp to an SQLite Julian day. Used by the translator when a JULIANDAY function
-- is translated.
--
-- The expression has two parts. First the julian date is calculated based on the fact that we know 2000-01-01 is the
-- julian date 2451545. The second part of the expression calculates the fraction, that is how much of the day 
-- that has passed, simply by calcuating the number of seconds since midnight and dividing by 86400 (the number 
-- of seconds in 24 hours). The SQLite julian day is based on noon, but in order to calculate the fraction a little easier
-- (the time period between midnight and noon would otherwise have been trickier) we base our calculations on midnight
-- and adjust things afterwards by adding in total 0.5.
@
create function compatibility.to_julianday(dt timestamp)
returns numeric(20,6)
return (cast((dt-timestamp'2000-01-01 00:00:00')day(7) as numeric(20,6))+2451545)+
       cast((cast(dt as time)-time'00:00:00') second(12,9)/86400 as numeric(20,6))-0.5;
@
grant execute on function compatibility.to_julianday to public;

--
-- The following table contains a list of procedures to call before and after the
-- locale changes. The compatibility layer calls these, while specific applications
-- register their procedures here.
--
-- Any errors during the execution of these procedures are ignored, including nonexistent
-- procedures and lack of privileges to run them.
--
-- The routines in before_local_change have no parameters, such as:
-- 
-- create procedure <Schema>.<procedure>() begin end;
--
-- All these routines are called in no particular order using the following statement:
--
-- call <Schema>.<Procedure>()
--
create table compatibility.before_locale_change (
       "Schema" varchar(128),
       "Procedure" varchar(128),
       primary key ("Schema","Procedure")
) in sqlite_comp;
grant select,insert,delete on compatibility.before_locale_change to public;

--
-- The routines in after_local_change should have no parameters.
-- 
-- create procedure <Schema>.<procedure>() begin end;
--
-- All these routines are called in no particular order using the following statement:
--
-- call <Schema>.<Procedure>()
--
create table compatibility.after_locale_change (
       "Schema" varchar(128),
       "Procedure" varchar(128),
       primary key ("Schema","Procedure")
) in sqlite_comp;
grant select,insert,delete on compatibility.after_locale_change to public;

-- 
-- The following table holds those procedures that should be called upon connect.
--
-- The routines listed here should have no parameters, and should adhere to the format:
--
-- create procedure <Schema>.<procedure>() begin end;
--
-- All these routines are called in no particular order using the following statement:
--
-- call <Schema>.<Procedure>()
--
create table compatibility.after_connect (
       "Schema" varchar(128),
       "Procedure" varchar(128),
       primary key ("Schema","Procedure")
) in sqlite_comp;
grant select,insert,delete on compatibility.after_connect to public;


@
create function compatibility.sqlite_current_timestamp()
returns varchar(19)
	return substring(cast(localtimestamp(0) as varchar(30)) from 11 for 19)
@
grant execute on function compatibility.sqlite_current_timestamp() to public;


-- ##############
-- A dumb implementation of the FTS snippet function
-- ##############

@
CREATE FUNCTION compatibility.snippet(pre NVARCHAR(10),res NVARCHAR(70),post NVARCHAR(10), elipsis NVARCHAR(10),col INTEGER, nrTokens INTEGER)
RETURNS NVARCHAR(1000)
DETERMINISTIC
SPECIFIC snippet
BEGIN
        RETURN pre || res || post;
END
@
grant execute on specific function compatibility.snippet to public;



@
create function compatibility.bitand(x integer,y integer)
returns integer
begin
     declare c int default 1;
     declare r int default 0;
     declare a int default x;
     declare b int default y;
     while compatibility.min(a, b) > 0 do
 
        if mod(a,2) > 0 and mod(b,2) > 0 then
             set r = r + c;
        end if;
        set c = c * 2;
	set a = a / 2;
	set b = b / 2;
     end while;
     return r;
end
@
grant execute on function compatibility.bitand to public;

@
create function compatibility.CALLBACK_FUNC(id nchar(128),val bigint) returns integer
return sysadm.CALLBACK_FUNC(id,val);
@
grant execute on function compatibility.CALLBACK_FUNC to public;


@
create function compatibility.delete_file(path nvarchar(512))
returns integer
begin
declare exit handler for sqlexception
begin
	declare err integer;
	get diagnostics exception 1 err = native_error; 
	if err = -14212 then
		return 1;
	else
		signal sqlstate '99001';
	end if;
end;
return sysadm.delete_file(path);
end
@
grant execute on function compatibility.delete_file to public;


@
create function compatibility.returnWord(message nvarchar(3000),searchWord nvarchar(20))
    returns nvarchar(30)
begin
    declare startPos,messageLength,counter,currentPos integer;
    declare returnWord nvarchar(30) default '';
    declare search nvarchar(20);

    set search = replace(searchWord,n'*','');
    set currentPos = 0;
    set messageLength = char_length(message);

    l1:
    loop
        set startPos = currentPos + position(search collate current_collation_2
            in substring(message from currentPos + 1)
                collate current_collation_2);
        if  startPos = currentPos then
            leave l1;
        end if;
        if  startPos = 1
        or  regexp_match(substring(message from startpos - 1 for 1),
            '\s|\p{P}') then
            set counter = char_length(search);
            if  startPos + counter >= messagelength
            or  regexp_match(substring(
                message from startpos + counter for 1),'\s|\p{P}') then
                set returnWord = substring(message from startPos
                    for counter);
            else
                while startPos + counter <= messageLength
                  and regexp_match(substring(message from startPos +
                          counter for 1) ,
                          '\w')
                  and counter < 30 do
                      set counter = counter + 1;
                end while;
                set returnWord = substring(message from startPos for counter);
            end if;
            leave l1;
        end if;
        set currentPos = startpos;
    end loop;
    return returnWord;
end 
@
grant execute on function compatibility.returnWord to public;

