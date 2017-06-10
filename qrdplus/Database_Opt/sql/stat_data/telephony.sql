set echo off;
-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
set message off;


@
begin
   declare addr_cnt integer;
   declare n_addr_id integer;
   declare n_msg_id int;
   declare n_contact_id int;
   declare n_address varchar(20);
   declare n_type int;
   declare n_charset int;
   
   set addr_cnt = 0;
   set n_charset = 106;
  
   WHILE addr_cnt <= 300 DO
      set n_addr_id = next_value of addr_id_seq;
      set n_msg_id = MOD(IRAND(), 100);
      if MOD(IRAND(), 10) = 0 then
        set n_contact_id = null;
      else        
        set n_contact_id = MOD(IRAND(), 30);
      end if;
      set n_address = cast(n_addr_id as varchar(10))||'7654321';
      if MOD(IRAND(), 3) = 0 then 
      	set n_type = 137;
      elseif MOD(IRAND(), 3) = 1 then
      	set n_type = 151;
      else
        set n_type = 139;
      end if;
      
      insert into addr ("_id", msg_id, contact_id, address, type, charset) values (n_addr_id, n_msg_id, n_contact_id, n_address, n_type, n_charset);
      
      set addr_cnt = addr_cnt + 1;
   END WHILE;
end
@


----
----

@
begin
   declare canonical_cnt integer;
   declare n_canonical_id integer;
   declare n_address varchar(20);
   
   set canonical_cnt = 0;
  
   WHILE canonical_cnt <= 100 DO
      set n_canonical_id = next_value of canonical_address_id_seq;
      set n_address =  cast(MOD(IRAND(), 1000000) as varchar(20));
      
      
      insert into canonical_addresses ("_id", address) values (n_canonical_id, n_address);
      
      set canonical_cnt = canonical_cnt + 1;
   END WHILE;
end
@

----
----

@
begin
   declare threads_cnt integer;
   declare n_threads_id integer;
   declare n_date bigint;
   declare n_message_count int;
   declare n_recipient_ids varchar(500);
   declare n_snippet nvarchar(500);
   declare n_read int;
   declare n_type int;
   declare n_error int;
   declare n_has_attachment int;
   declare loop_cnt int;

   
   set threads_cnt = 0;
  
   WHILE threads_cnt <= 50 DO
      set n_threads_id = next_value of threads_id_seq;
      set n_date = MOD(IRAND(), 1000)*1000;
      set n_message_count = MOD(IRAND(), 100);
      
      set n_recipient_ids = cast(MOD(IRAND(), 100) as varchar(3));
      set loop_cnt = MOD(IRAND(), 10);
      WHILE loop_cnt > 1 DO
        set n_recipient_ids = n_recipient_ids||' '||cast(MOD(IRAND(), 100) as varchar(3));
        set loop_cnt = loop_cnt - 1;
      END WHILE;

      set n_snippet = cast(threads_cnt*10 as nvarchar(100))|| ' 1 urna enim. dictum,  velit,  volutpat consectetur et elit pulvinar Suspendisse Donec Phasellus ut non. test threads';
     
      if MOD(IRAND(), 5) = 0 then
        set n_read = 0;
      else        
        set n_read = 1;
      end if;

      if MOD(IRAND(), 4) = 0 then 
      	set n_type = 0;
      elseif MOD(IRAND(), 4) = 1 then
      	set n_type = 1;
      elseif MOD(IRAND(), 4) = 2 then
      	set n_type = 2;
      else
      	set n_type = 3;
      end if;

      
      if MOD(IRAND(), 3) = 0 then
        set n_has_attachment = 1;
        set n_error = 1;
      else        
        set n_has_attachment = 0;
        set n_error = 0;
      end if;

      insert into threads ("_id", date, message_count, recipient_ids, snippet, snippet_cs, read, type, error, has_attachment) values (n_threads_id, n_date, n_message_count, n_recipient_ids, n_snippet, 0, n_read, n_type, n_error, n_has_attachment);

      set threads_cnt = threads_cnt + 1;
   END WHILE;
end
@

----
----

@
begin
   declare part_cnt integer;
   declare n_part_id integer;
   declare n_mid int;
   declare n_seq int;
   declare n_ct varchar(20);
   declare n_name nvarchar(20);
   declare n_cl varchar(20);
   declare n_data varchar(200);
   declare n_text nvarchar(1000);
   
   set part_cnt = 0;
   set n_cl = null;
  
   WHILE part_cnt <= 400 DO
      set n_part_id = next_value of part_id_seq;
      set n_name = cast(MOD(IRAND(), 1000000) as nvarchar(20));
      set n_mid = MOD(IRAND(), 100);
      if MOD(IRAND(), 2) = 0 then
        set n_seq = 0;
      else        
        set n_seq = -1;
      end if;
      if MOD(IRAND(), 3) = 0 then 
      	set n_ct = 'application/smil';
        set n_data = null;
        set n_text = 'smilheadlayoutroot-layout width=128px height=123px background-color=#FFFFFF /region id=Image width=128px height=61px top=0px left=0px fit=hidden /region id=Text width=128px height=61px top=61px left=0px fit=hidden //layout/headbodypar dur=5000ms img src=wallpaper_nexusrain.jpg region=Image /text src=text-1.txt region=Text param name=foreground-color value=#000000 /param name=textsize value=normal //text/par/body/smil'||cast(n_part_id as nvarchar(10));
      elseif MOD(IRAND(), 3) = 1 then
      	set n_ct = 'image/jpeg';
        set n_data = '/data/data/com.mimer.providers.telephony/app_parts/PART_'||cast(n_part_id as varchar(10));
      else
        set n_ct = 'text/plain';
        set n_data = null;
	set n_cl = n_name;
        set n_text = '19 tincidunt In at åöä sem adipiscing turpis a Donec luctus,  ut Donec Suspendisse rutrum sagittis,  mi. ultricies. nulla '||cast(n_part_id as nvarchar(10));
      end if;


      insert into part ("_id", mid, seq, ct, name, chset, cd, fn, cid, cl, ctt_s, ctt_t, "_data", text) values (n_part_id, n_mid, n_seq, n_ct, n_name, 106, null, null, n_name, n_cl, null, null, n_data, n_text);
      
      set part_cnt = part_cnt + 1;
   END WHILE;
end
@

----
----

@
begin
   declare pdu_cnt integer;
   declare n_pdu_id integer;
   declare n_thread_id bigint;
   declare n_date bigint;
   declare n_date_sent bigint;
   declare n_msg_box int;
   declare n_read int;
   declare n_m_id varchar(200);
   declare n_sub nvarchar(100);
   declare n_exp bigint;
   declare n_m_type int;
   declare n_m_size int;
   declare n_tr_id varchar(100);
   declare n_locked int;
   declare n_seen int;

   
   set pdu_cnt = 0;
  
   WHILE pdu_cnt <= 300 DO
      set n_pdu_id = next_value of pdu_id_seq;
      set n_thread_id = MOD(IRAND(), 30);
      set n_date = MOD(IRAND(), 1000)*1000;
      set n_date_sent = MOD(IRAND(), 1000)*1000;
      set n_msg_box = MOD(IRAND(), 10);
      if MOD(IRAND(), 2) = 0 then
        set n_read = 0;
      else        
        set n_read = 1;
      end if;
      set n_m_id = cast(pdu_cnt as varchar(3))||'@example.com';
      if MOD(IRAND(), 10) = 0 then
        set n_exp = null;
        set n_sub = null;
      else        
        set n_exp = MOD(IRAND(), 10000000);
        set n_sub = cast(pdu_cnt as varchar(3))||' Test sub';
      end if;

      if MOD(IRAND(), 4) = 0 then 
      	set n_m_type = 128;
      elseif MOD(IRAND(), 4) = 1 then
      	set n_m_type = 130;
      elseif MOD(IRAND(), 4) = 2 then
      	set n_m_type = 132;
      else
      	set n_m_type = 131;
      end if;

      set n_m_size = MOD(IRAND(), 50)*1000;
      set n_tr_id =  cast(pdu_cnt as varchar(100));

      if MOD(IRAND(), 3) = 0 then
        set n_locked = 1;
        set n_seen = 1;
      else        
        set n_locked = 0;
        set n_seen = 0;
      end if;


      insert into pdu ("_id", thread_id, date, date_sent, msg_box, read, m_id, sub, sub_cs, ct_t, ct_l, exp, m_cls, m_type, v, m_size, pri, rr, rpt_a, resp_st, st, tr_id, retr_st, retr_txt, retr_txt_cs, read_status, ct_cls, resp_txt, d_tm, d_rpt, locked, seen) values (n_pdu_id, n_thread_id, n_date, n_date_sent, n_msg_box, n_read, n_m_id, n_sub, 106, 'application/vnd.wap.multipart.related', null, n_exp, 'personal', n_m_type, 17, n_m_size, 129, 129, null, null, null, n_tr_id, null, null, null, null, null, null, null, 129, n_locked, n_seen);

      
      set pdu_cnt = pdu_cnt + 1;
   END WHILE;
end
@

----
----
@
begin
   declare sms_cnt integer;
   declare n_sms_id integer;
   declare n_thread_id bigint;
   declare n_address varchar(100);
   declare n_person int;
   declare n_date bigint;
   declare n_date_sent bigint;
   declare n_read int;
   declare n_type int;
   declare n_body nvarchar(500);
   declare n_locked int;
   declare n_seen int;

   
   set sms_cnt = 0;
  
   WHILE sms_cnt <= 1000 DO
      set n_sms_id = next_value of sms_id_seq;
      set n_thread_id = MOD(IRAND(), 30);
      set n_date = MOD(IRAND(), 1000)*1000;
      set n_date_sent = MOD(IRAND(), 1000)*1000;
      set n_person = MOD(IRAND(), 50);
      set n_address = cast(sms_cnt*10 as varchar(100));
      
      if MOD(IRAND(), 5) = 0 then
        set n_read = 0;
      else        
        set n_read = 1;
      end if;

      if MOD(IRAND(), 4) = 0 then 
      	set n_type = 0;
      elseif MOD(IRAND(), 4) = 1 then
      	set n_type = 1;
      elseif MOD(IRAND(), 4) = 2 then
      	set n_type = 2;
      else
      	set n_type = 3;
      end if;

      set n_body = cast(sms_cnt*10 as nvarchar(100))|| ' 1 urna enim. dictum,  velit,  volutpat consectetur et elit pulvinar Suspendisse Donec Phasellus ut non. test sms body';


      if MOD(IRAND(), 3) = 0 then
        set n_locked = 1;
        set n_seen = 1;
      else        
        set n_locked = 0;
        set n_seen = 0;
      end if;

      insert into sms ("_id", thread_id, address, person, date, date_sent, protocol, read, status, type, reply_path_present, subject, body, service_center, locked, error_code, seen) values (n_sms_id, n_thread_id, n_address, n_person, n_date, n_date_sent, null, n_read, -1, n_type, null, null, n_body, null, n_locked, 0, n_seen);

      set sms_cnt = sms_cnt + 1;
   END WHILE;
end
@

----
----
@
begin
   declare pending_msgs_cnt integer;
   declare n_pending_msgs_id integer;
   declare n_proto_type integer;
   declare n_msg_id int;
   declare n_msg_type int;
   declare n_err_type int;
   declare n_due_time bigint;
   declare n_last_try bigint;

   
   set pending_msgs_cnt = 0;
  
   WHILE pending_msgs_cnt <= 50 DO
      set n_pending_msgs_id = next_value of pending_msgs_id_seq;
      set n_proto_type = MOD(IRAND(), 4);
      set n_msg_id = MOD(IRAND(), 1000);
      if MOD(IRAND(), 4) = 0 then 
      	set n_msg_type = 130;
      elseif MOD(IRAND(), 4) = 1 then
      	set n_msg_type = 128;
      elseif MOD(IRAND(), 4) = 2 then
      	set n_msg_type = 132;
      else
      	set n_msg_type = 3;
      end if;
      
      set n_err_type = MOD(IRAND(), 4);
      set n_due_time = MOD(IRAND(), 1000)*1000;
      set n_last_try = MOD(IRAND(), 1000)*1000;
     
      insert into pending_msgs ("_id", proto_type, msg_id, msg_type, err_type, due_time, last_try) values (n_pending_msgs_id, n_proto_type, n_msg_id, n_msg_type, n_err_type, n_due_time, n_last_try);

      set pending_msgs_cnt = pending_msgs_cnt + 1;
   END WHILE;
end
@
----
