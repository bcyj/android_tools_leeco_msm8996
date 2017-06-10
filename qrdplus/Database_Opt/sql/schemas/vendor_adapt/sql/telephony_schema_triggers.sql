-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

-- ###############################################################################
-- Triggers
-- ###############################################################################

set sqlite on;


@
CREATE TRIGGER pdu_update_thread_read_on_update AFTER  UPDATE ON pdu   
REFERENCING OLD TABLE AS ot NEW TABLE AS nt 
BEGIN ATOMIC
	FOR SELECT nt.thread_id as nthread_id FROM nt join ot on nt.MIMER_ROWID = ot.MIMER_ROWID and ot.read <> nt.read and nt.m_type in (132,130,128) DO
		UPDATE threads 
			SET read = CASE WHEN EXISTS 
				(SELECT "_id" FROM pdu WHERE read = 0 
					AND pdu.thread_id = nthread_id AND m_type in(132,130,128))
			THEN 0 ELSE 1 END  



	        WHERE "_id" = nthread_id; 
	END FOR;
END
@

--@
--CREATE TRIGGER update_threads_error_on_delete_mms AFTER DELETE ON pdu  
--WHEN OLD._id IN (SELECT DISTINCT msg_id FROM pending_msgs WHERE err_type >= 10) 
--REFERENCING OLD TABLE AS ot FOR EACH STATEMENT
--BEGIN ATOMIC   
--	UPDATE threads SET error = error - 1  
--	WHERE "_id" IN (SELECT ot.thread_id from ot WHERE ot."_id" IN (SELECT msg_id FROM pending_msgs WHERE err_type >= 10)); 
--END
--@


-- @
-- CREATE TRIGGER sms_words_delete AFTER DELETE ON sms 
-- REFERENCING OLD TABLE AS ot 
-- BEGIN ATOMIC
-- 	FOR SELECT ot."_id" AS oid FROM ot DO
-- 		DELETE FROM words WHERE words.source_id = oid AND words.table_to_use = 1; 
-- 	END FOR;
-- END
-- @

-- @
-- CREATE TRIGGER sms_words_update AFTER UPDATE ON sms 
-- REFERENCING OLD TABLE AS ot NEW TABLE AS nt
-- BEGIN ATOMIC
-- 	FOR SELECT nt.body as nbody, "_id" as nid FROM nt JOIN ot USING("_id") DO
-- 		UPDATE words SET index_text = nbody WHERE (source_id=nid AND table_to_use=1);  
-- 	END FOR;
-- END 
-- @

-- Not needed anmore. Handled by sms_update_thread_date_subject_on_update
--@
--CREATE TRIGGER sms_update_thread_read_on_update AFTER  UPDATE ON sms
--REFERENCING OLD TABLE AS ot NEW TABLE AS nt
--BEGIN ATOMIC
--	FOR SELECT nt.thread_id as nthread_id FROM nt DO
--		UPDATE threads 
--		SET read = CASE WHEN EXISTS
--			(SELECT "_id" FROM sms WHERE read = 0 AND sms.thread_id = nthread_id)
--			THEN 0 ELSE 1 END  
--		WHERE "_id" = nthread_id; 
--	END FOR;
--END
--@

--@
--CREATE TRIGGER sms_update_thread_date_subject_on_update AFTER  UPDATE  ON sms
--REFERENCING OLD TABLE AS ot NEW TABLE AS nt
--BEGIN ATOMIC
--      DECLARE pthid int;
--	FOR SELECT nt.body as nbody,nt.thread_id as nthread_id FROM nt 
--        DO
--		if pthid <> nthread_id or pthid is null
--		then
--		set pthid = nthread_id;
--		UPDATE threads
--			SET date = cast((builtin.utc_timestamp() - timestamp '1970-01-01 00:00:00') second(12,0) as integer) * 1000,
--                                   snippet = nbody, snippet_cs = 0,
--                            message_count =
--                                        (SELECT COUNT(*) FROM sms LEFT JOIN threads ON sms.thread_id = threads."_id"
--                                                WHERE sms.thread_id = nthread_id AND sms.type != 3) + 
--                                        (SELECT COUNT(*) FROM pdu LEFT JOIN threads  ON pdu.thread_id = threads."_id"
--                                                WHERE pdu.thread_id = nthread_id AND m_type IN(132,130,128) AND msg_box = 3),
--                            read = CASE WHEN EXISTS
--                                        (SELECT "_id" FROM sms WHERE read = 0 AND sms.thread_id = nthread_id)
--                                        THEN 0 ELSE 1 END
--                WHERE "_id" = nthread_id; 
--		end if;
--        END FOR;
--END
--@


@
CREATE TRIGGER sms_update_thread_date_subject_on_update AFTER  UPDATE  ON sms
REFERENCING OLD TABLE AS ot NEW TABLE AS nt
BEGIN ATOMIC
      DECLARE pthid bigint;
	FOR SELECT nt.body as nbody,nt.thread_id as nthread_id FROM nt
        DO
		if pthid <> nthread_id or pthid is null
		then
		set pthid = nthread_id;
		UPDATE threads			
		SET	snippet = nbody,
			snippet_cs = 0,
            message_count =
				(SELECT COUNT(*) FROM sms LEFT JOIN threads ON sms.thread_id = threads."_id"
					WHERE sms.thread_id = nthread_id AND sms.type != 3) + 
				(SELECT COUNT(*) FROM pdu LEFT JOIN threads  ON pdu.thread_id = threads."_id"
					WHERE pdu.thread_id = nthread_id AND m_type IN(132,130,128) AND msg_box = 3)
			,
			read = CASE WHEN EXISTS
						(SELECT "_id" FROM sms WHERE read = 0 AND sms.thread_id = nthread_id)
					THEN 0 ELSE 1 END
                	WHERE "_id" = nthread_id; 
		end if;
        END FOR;
END
@


-- Previous version of the above.
-- @
-- CREATE TRIGGER sms_update_thread_date_subject_on_update AFTER  UPDATE  ON sms 
-- REFERENCING OLD TABLE AS ot NEW TABLE AS nt 
-- BEGIN ATOMIC
-- 	FOR SELECT nt.body as nbody,nt.thread_id as nthread_id
-- 		FROM nt DO
-- 		UPDATE threads 
-- 			SET date = cast((builtin.utc_timestamp() - timestamp '1970-01-01 00:00:00') second(12,0) as integer) * 1000,
-- 			           snippet = nbody, snippet_cs = 0,
-- 				message_count =
-- 					(SELECT COUNT(*) FROM sms LEFT JOIN threads ON sms.thread_id = threads."_id" 
-- 						WHERE sms.thread_id = nthread_id AND sms.type != 3) + 
-- 					(SELECT COUNT(*) FROM pdu LEFT JOIN threads  ON pdu.thread_id = threads."_id" 
-- 						WHERE pdu.thread_id = nthread_id AND m_type IN(132,130,128) AND msg_box = 3),
-- 				read = CASE WHEN EXISTS
-- 					(SELECT COUNT(*) FROM sms WHERE read = 0 AND sms.thread_id = nthread_id)
-- 					THEN 0 ELSE 1 END  
-- 		WHERE "_id" = nthread_id; 
-- 	END FOR;
-- END
-- @

--the delete triggers on pdu below are merged into pdu_on_delete
--@
--CREATE TRIGGER addr_cleanup AFTER DELETE ON pdu 
--REFERENCING OLD table AS ot
--BEGIN ATOMIC
--	FOR SELECT "_id" AS ot_id FROM ot DO
--	DELETE FROM addr WHERE msg_id=ot_id;
--	END FOR;
--END
--@

--@
--create trigger cleanup_delivery_and_read_report after delete on pdu
--referencing old table as ot for each statement
--begin atomic
--	if exists (select * from ot where m_type = 128) then
--		delete from pdu
--		where m_type in (134,136)
--		and m_id in
--		(select m_id
--		from ot
--		where ot.m_type = 128);
--	end if;
--end 
--@

--@
--CREATE TRIGGER delete_mms_pending_on_delete AFTER DELETE ON pdu 
--REFERENCING OLD TABLE AS ot
--BEGIN ATOMIC
--	FOR SELECT "_id" AS ot_id FROM ot DO
--		DELETE FROM pending_msgs WHERE msg_id=ot_id;
--	END FOR;
--END
--@

--merged into pdu_on_update
--@
--CREATE TRIGGER delete_mms_pending_on_update AFTER UPDATE ON pdu 
--REFERENCING OLD TABLE AS ot NEW TABLE AS nt for each statement
--BEGIN ATOMIC
--	DELETE FROM pending_msgs WHERE msg_id IN (
--		SELECT ot."_id" FROM ot JOIN nt ON ot.MIMER_ROWID = nt.MIMER_ROWID 
--			WHERE ot.msg_box = 4 AND nt.msg_box <> 4 );
--END
--@

@
create trigger insert_mms_pending_on_insert after insert on pdu
referencing new table as nt for each statement
begin atomic

	insert into	pending_msgs(proto_type,msg_id,msg_type,err_type,err_code,retry_index,due_time)
		select 1,"_id",m_type,0,0,0,0
		from nt
		where m_type in (130,135);

end
@

--insert_mms_pending_on_update merged into pdu_on_update

@
create trigger pdu_update_thread_date_subject_on_update after update on pdu
referencing new table as nt old table as ot for each statement
begin atomic
        DECLARE pthid bigint;
	for select nt.m_type as nmtype,nt.thread_id as nthread_id,nt.sub as nsub,nt.sub_cs as nsub_cs
		from nt join ot using("_id")
		where (nt.date <> ot.date or nt.sub <> nt.sub or nt.msg_box <> ot.msg_box)
		and nt.m_type in (132,130,128) 
                DO
		if pthid <> nthread_id or pthid is null
		then
		set pthid = nthread_id;
		
		update threads set date = cast((builtin.utc_timestamp() - timestamp '1970-01-01 00:00:00') second(12,0) as integer) * 1000,
			snippet = nsub,snippet_cs = nsub_cs,
			message_count =	(select count(*)from sms 
			left join threads on sms.thread_id = threads."_id" where sms.thread_id = nthread_id
			and sms.type <> 3) +
			(select count(*) from pdu
				left join threads on pdu.thread_id = threads."_id"
				where pdu.thread_id = nthread_id
				and pdu.m_type in (132,130,128)
				and pdu.msg_box <> 3),
			read = case when exists
				(select * from pdu where read = 0 and pdu.thread_id = nthread_id
					and pdu.m_type in (132,130,128))
				then 0 else 1 end
		where "_id" = nthread_id;
		end if;
	end for;
end
@


@
CREATE TRIGGER pdu_update_thread_on_insert AFTER INSERT ON pdu   
REFERENCING NEW TABLE AS nt
BEGIN ATOMIC
        DECLARE pthid bigint;
	FOR SELECT nt.m_type as nmtype, nt.sub AS nsub, nt.sub_cs AS nsub_cs, thread_id AS nthread_id
		FROM nt WHERE nt.m_type IN (132,130,128)
                DO
		if pthid <> nthread_id or pthid is null
		then
		set pthid = nthread_id; 
			UPDATE threads SET 
				date = cast((builtin.utc_timestamp() - timestamp '1970-01-01 00:00:00') second(12,0) as integer) * 1000, 
				snippet = nsub, 


				snippet_cs = nsub_cs,
				message_count =	
					(SELECT COUNT(*) FROM sms 
						LEFT JOIN threads ON sms.thread_id = threads."_id"
						WHERE sms.thread_id = nthread_id AND sms.type <> 3) +
					(SELECT COUNT(*) FROM pdu  
						LEFT JOIN threads ON pdu.thread_id = threads."_id"  
						WHERE pdu.thread_id = nthread_id AND m_type IN(132,130,128) AND msg_box <> 3)
					,

				read = CASE WHEN EXISTS
					(SELECT * FROM pdu WHERE read = 0 AND pdu.thread_id = nthread_id AND m_type IN(132,130,128))  
					THEN 0 ELSE 1 END
			WHERE 
			"_id" = nthread_id;
		end if;
		END FOR;
	END
@

--pdu_update_thread_on_delete merged into pdu_on_delete


@
CREATE TRIGGER sms_update_thread_on_insert AFTER INSERT ON sms 
REFERENCING NEW TABLE AS nt
BEGIN ATOMIC
        DECLARE pthid bigint;
	FOR SELECT nt.body as nbody, thread_id as nthread_id, nt."date" as ndate
		FROM nt DO
         	if pthid <> nthread_id or pthid is null
	        then
        	set pthid = nthread_id;
        	
		UPDATE threads SET 
				date = cast((builtin.utc_timestamp() - timestamp '1970-01-01 00:00:00') second(12,0) as integer) * 1000,

				snippet = nbody,

				snippet_cs = 0,

				message_count = 
					(SELECT COUNT(*) FROM sms 
						LEFT JOIN threads ON sms.thread_id = threads."_id"      
						WHERE sms.thread_id = nthread_id AND sms.type != 3) + 
					(SELECT COUNT(*) FROM pdu 
						LEFT JOIN threads ON pdu."_id" = threads."_id"
						WHERE pdu.thread_id = nthread_id AND m_type IN (132,130,128) AND msg_box != 3)


					,
					read = CASE WHEN EXISTS 
					(SELECT * FROM sms WHERE read = 0 AND sms.thread_id = nthread_id)
					THEN 0 ELSE 1  END

		WHERE "_id" = nthread_id;
		end if;

	END FOR;
END
@

-- @
-- CREATE TRIGGER mms_words_delete AFTER DELETE ON part 
-- REFERENCING OLD TABLE AS ot
-- BEGIN ATOMIC 
-- 	FOR SELECT "_id" AS old_id FROM ot DO
-- 		DELETE FROM words WHERE source_id = old_id AND table_to_use = 2;
-- 	END FOR;
-- END
-- @

-- @
-- CREATE TRIGGER mms_words_update AFTER UPDATE ON part 
-- REFERENCING OLD TABLE AS ot NEW TABLE AS nt
-- BEGIN ATOMIC
-- 	FOR SELECT "_id" AS new_id, text as new_text FROM nt DO
-- 	UPDATE words SET index_text = new_text WHERE (source_id=new_id AND table_to_use=2);
-- 	END FOR;
-- END
-- @

@
CREATE TRIGGER part_cleanup AFTER DELETE ON pdu 
REFERENCING OLD TABLE AS ot
BEGIN ATOMIC   
	FOR SELECT "_id" AS old_id FROM ot DO  
		DELETE FROM part WHERE mid=old_id;
	END FOR;
END
@

--@
--create trigger update_threads_error_on_move_mms after update on pdu
--referencing old table as old_pdu new table as new_pdu for each statement
--begin atomic
--	update threads
--	set error = error - 1
--	where "_id" in
--	(select old_pdu.thread_id 
--		from old_pdu
--		join new_pdu
--		on old_pdu."_id" = new_pdu."_id" -- primary key column(s)
--		where old_pdu.msg_box <> 4
--		and new_pdu.msg_box = 4
--		and old_pdu."_id" in
--		(select msg_id
--		from pending_msgs
--		where err_type >= 10));
--end 
--@


@
CREATE TRIGGER update_threads_error_on_update_mms AFTER UPDATE ON pending_msgs   
REFERENCING NEW TABLE AS nt OLD TABLE AS ot 
BEGIN ATOMIC
	FOR SELECT nt.err_type AS nerr_type, nt.msg_id AS nmsg_id
		FROM nt JOIN ot ON nt.MIMER_ROWID = ot.MIMER_ROWID
		WHERE (ot.err_type < 10 AND nt.err_type >= 10  
			) 
			OR (ot.err_type >= 10 AND nt.err_type < 10) DO
	
		UPDATE threads SET error = CASE WHEN (nerr_type >= 10) THEN error + 1 ELSE -1 END
			WHERE "_id" IN (SELECT thread_id FROM pdu WHERE pdu."_id" = nmsg_id);
	END FOR;
END
@

@
CREATE TRIGGER update_threads_on_delete_part AFTER DELETE ON part  
REFERENCING OLD TABLE AS ot 
BEGIN ATOMIC
		FOR SELECT ot.ct AS o_ct 
		FROM ot 
			WHERE ot.ct <> 'text/plain' AND ot.ct <> 'application/smil' DO
			
			UPDATE threads SET has_attachment = CASE WHEN EXISTS 
				(SELECT * FROM part JOIN pdu ON part."mid" = pdu."_id" WHERE pdu.thread_id = threads."_id"
				AND part.ct <> 'text/plain' 
				AND part.ct <> 'application/smil') 
				THEN 1 ELSE 0 END

;
		END FOR;
END
@

@
CREATE TRIGGER update_threads_on_insert_part AFTER INSERT ON part  
REFERENCING NEW TABLE AS nt
BEGIN ATOMIC
	FOR SELECT nt."_id" AS n_id FROM nt
		WHERE nt.ct <> 'text/plain' AND nt.ct <> 'application/smil' DO
		
		UPDATE threads SET has_attachment=1 WHERE "_id" IN 
		(SELECT pdu.thread_id FROM part JOIN pdu ON pdu."_id"=part.mid WHERE part."_id"= n_id); --fetch first row only);
	END FOR;
END
@

@
CREATE TRIGGER update_threads_on_update_part AFTER UPDATE ON part
REFERENCING NEW TABLE AS nt OLD TABLE AS ot
BEGIN ATOMIC
	FOR SELECT nt."_id" AS n_id FROM nt join ot on nt.MIMER_ROWID = ot.MIMER_ROWID and nt.mid <> ot.mid
		WHERE nt.ct <> 'text/plain' AND nt.ct <> 'application/smil' DO
		UPDATE threads SET has_attachment=1 WHERE "_id" IN    
		(SELECT pdu.thread_id FROM part JOIN pdu ON pdu."_id"=part.mid      
		WHERE part."_id"=n_id); --LIMIT 1);  
	END FOR;
END
@


@
CREATE TRIGGER update_threads_on_update_pdu AFTER UPDATE ON pdu  
REFERENCING NEW TABLE AS nt OLD TABLE AS ot
BEGIN ATOMIC
	for select count(*) as update_cnt from nt join ot on nt.MIMER_ROWID = ot.MIMER_ROWID where ot.thread_id <> nt.thread_id DO
	    if update_cnt > 0 then
	        UPDATE threads SET has_attachment=1 WHERE "_id" IN    
	        (SELECT pdu.thread_id FROM part JOIN pdu ON part."_id" = pdu."_id"     
	        WHERE part.ct <> 'text/plain' AND part.ct <> 'application/smil'
	        AND part.mid = pdu."_id");
	    end if;
	end for;
END
@


--merge delete triggers on pdu
@
CREATE TRIGGER pdu_on_delete AFTER DELETE ON pdu
REFERENCING OLD TABLE AS old_t
BEGIN ATOMIC
--pdu_update_thread_on_delete
--keep HTC's own pdu_update_thread_on_delete
    DECLARE d,d1 bigint;
    DECLARE s,s1 nvarchar(128);
    DECLARE s_cs,s_cs1 int;
    DECLARE pthid bigint;

    FOR SELECT thread_id AS old_thread_id, "_id" AS ot_id FROM old_t DO
        SELECT date*1000 AS d,
			sub,
			sub_cs INTO d,s,s_cs 
			FROM pdu WHERE thread_id = old_thread_id
			ORDER BY d DESC FETCH FIRST ROW ONLY;
        SELECT date AS d,body,0 INTO d1,s1,s_cs1
          	FROM sms WHERE thread_id = old_thread_id
         	ORDER BY d DESC FETCH FIRST ROW ONLY;

        IF d1 > d THEN
            SET s = s1;
            SET s_cs = s_cs1;
        END IF;

	if pthid <> old_thread_id or pthid is null
	then
 	set pthid = old_thread_id; 
           UPDATE threads
              SET date = cast((builtin.utc_timestamp() - timestamp '1970-01-01 00:00:00') second(12,0) as integer) * 1000,
              	  snippet = s,
                  snippet_cs = s_cs,
                  message_count =
                 (SELECT COUNT(*)
                    FROM sms
                   WHERE sms.thread_id = old_thread_id
                     AND sms.type <> 3) +
                 (SELECT COUNT(*)
                    FROM pdu
                   WHERE pdu.thread_id = old_thread_id
                     AND pdu.m_type in (132,130,128)
                     AND pdu.msg_box <> 3)

           WHERE threads."_id" = old_thread_id;
	end if;


--addr_cleanup
	DELETE FROM addr WHERE msg_id=ot_id;
--delete_mms_pending_on_delete
	DELETE FROM pending_msgs WHERE msg_id=ot_id;

    END FOR;

--cleanup_delivery_and_read_report
if exists (select * from old_t where m_type = 128) then
		delete from pdu
		where m_type in (134,136)
		and m_id in
		(select m_id
		from old_t
		where old_t.m_type = 128);
	end if;

--update_threads_error_on_delete_mms
	UPDATE threads SET error = error - 1  
	WHERE "_id" IN (SELECT old_t.thread_id from old_t WHERE old_t."_id" IN (SELECT msg_id FROM pending_msgs WHERE err_type >= 10)); 

END 
@

--merge update triggers on pdu
@
CREATE TRIGGER pdu_on_update AFTER UPDATE ON pdu 
REFERENCING OLD TABLE AS ot NEW TABLE AS nt for each statement
BEGIN ATOMIC
--delete_mms_pending_on_update
	DELETE FROM pending_msgs WHERE msg_id IN (
		SELECT ot."_id" FROM ot JOIN nt ON ot.MIMER_ROWID = nt.MIMER_ROWID 
			WHERE ot.msg_box = 4 AND nt.msg_box <> 4 );


--insert_mms_pending_on_update
	INSERT INTO pending_msgs(proto_type,msg_id,msg_type,err_type,err_code,retry_index,due_time)
		SELECT 1,"_id",nt.m_type,0,0,0,0
		FROM nt join ot using("_id")
		where nt.m_type = 128 AND nt.msg_box=4 AND ot.msg_box <> 4;

--update_threads_error_on_move_mms
	FOR SELECT ot.thread_id as o_thread_id
        from ot join nt on ot.MIMER_ROWID = nt.MIMER_ROWID
        where (ot.msg_box <> nt.msg_box) 
		and (ot.msg_box = 4 and nt.msg_box != 4) 
		and (ot."_id" IN (SELECT DISTINCT msg_id FROM pending_msgs WHERE err_type >= 10))

        DO
            UPDATE threads set 
                   error = error -1 where "_id" = o_thread_id; 
        END FOR;

END
@





