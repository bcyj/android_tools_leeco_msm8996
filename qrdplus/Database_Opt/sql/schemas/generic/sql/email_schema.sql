-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

set sqlite on;
-- create schema for email_body_db
CREATE UNIQUE sequence "email_body".body_seq as BIGINT;
CREATE TABLE "email_body"."Body" (
        "_id" bigint NOT NULL DEFAULT next_value OF "email_body".body_seq with check option,
        "messageKey" bigint CONSTRAINT body_pk PRIMARY KEY NOT NULL,
        "htmlContent" NCLOB(1G),
        "textContent" NCLOB(1G),
        "htmlReply" NCLOB(1G),
        "textReply" NCLOB(1G),
        "sourceMessageKey" nvarchar(128),
        "introText" NCLOB(1G)
		,"quotedTextStartPos" int
    ) IN "email_body_db";
    

CREATE TABLE "email_body"."android_metadata"(
        "locale" nvarchar(128)
    ) IN "email_body_db";
    
    
CREATE INDEX "email_body"."body_id" ON "email_body"."Body"("_id");

CREATE SYNONYM Body FOR "email_body"."Body";



-- create schema for email_db
-- ###############################################################################
-- Tables
-- ###############################################################################
CREATE UNIQUE sequence account_seq AS BIGINT;
CREATE TABLE "Account" (
        "_id" BIGINT PRIMARY KEY DEFAULT next_value OF account_seq with check option,
        "displayName" nvarchar(128),
        "emailAddress" nvarchar(128),
        "syncKey" nvarchar(128),
        "syncLookback" INT,
        "syncInterval" nvarchar(128),
        "hostAuthKeyRecv" INT,
        "hostAuthKeySend" INT,
        "flags" INT,
        "isDefault" INT,
        "compatibilityUuid" nvarchar(128),
        "senderName" nvarchar(128),
        "ringtoneUri" nvarchar(128),
        "protocolVersion" nvarchar(128),
        "newMessageCount" INT,
        "securityFlags" INT,
        "securitySyncKey" nvarchar(128),
        "signature" nvarchar(1024),
        policyKey INT,
        pingDuration integer
    ) IN "email_db";

CREATE UNIQUE sequence attachment_seq AS BIGINT;
CREATE TABLE "Attachment"(
        "_id" bigint PRIMARY KEY DEFAULT next_value OF attachment_seq with check option,
        "fileName" nvarchar(128),
        "mimeType" nvarchar(128),
        "size" INT,
        "contentId" nvarchar(128),
        "contentUri" nvarchar(128),
        "messageKey" INT,
        "location" nvarchar(128),
        "encoding" nvarchar(128),
        "content" nvarchar(128),
        "flags" INT,
        "content_bytes" BLOB,
        "accountKey" INT
	,"uiState" int,
	"uiDestination" int, 
	"uiDownloadedSize" int
        ,cachedFile nvarchar(128)
    ) IN "email_db";
    

    
CREATE UNIQUE sequence hostauth_seq as BIGINT;
CREATE TABLE "HostAuth" (
        "_id" BIGINT PRIMARY KEY DEFAULT next_value OF hostauth_seq with check option,
        "protocol" nvarchar(128),
        "address" nvarchar(128),
        "port" INT,
        "flags" INT,
        "login" nvarchar(128),
        "password" nvarchar(128),
        "domain" nvarchar(128),
        "accountKey" INT
	,"certAlias" nvarchar(128)
        ,serverCert blob
        
    ) IN "email_db";

CREATE UNIQUE sequence mailbox_seq AS BIGINT;
CREATE TABLE "Mailbox" (
        "_id" bigint PRIMARY KEY DEFAULT next_value OF mailbox_seq with check option,
        "displayName" nvarchar(128),
        "serverId" nvarchar(128),
        "parentServerId" nvarchar(128),
        "parentKey" INT,
        "accountKey" INT,
        "type" INT,
        "delimiter" INT,
        "syncKey" nvarchar(128),
        "syncLookback" INT,
        "syncInterval" INT,
        "syncTime" bigint,
        "unreadCount" INT,
        "flagVisible" INT,
        "flags" INT,
        "visibleLimit" INT,
        "syncStatus" nvarchar(128),
        "messageCount" INT NOT NULL DEFAULT 0,
        "lastSeenMessageKey" INT,
        "lastTouchedTime" bigint DEFAULT 0
	,"uiSyncStatus" int default 0, 
	"uiLastSyncResult" int default 0, 
	"lastNotifiedMessageKey" int not null default 0, 
	"lastNotifiedMessageCount" int not null default 0, 
	"totalCount" int
        ,hierarchicalName nvarchar(50),
        lastFullSyncTime integer
    ) IN "email_db";

CREATE UNIQUE sequence message_seq AS BIGINT;
CREATE TABLE "Message" (
        "_id" bigint PRIMARY KEY DEFAULT next_value OF message_seq with check option,
        "syncServerId" nvarchar(128),
        "syncServerTimeStamp" bigint,
        "displayName" nvarchar(128),
        "timeStamp" bigint,
        "subject" nvarchar(256),
        "flagRead" INT,
        "flagLoaded" INT,
        "flagFavorite" INT,
        "flagAttachment" INT,
        "flags" INT,
        "clientId" INT,
        "messageId" nvarchar(128),
        "mailboxKey" BIGINT,
        "accountKey" INT,
        "fromList" NCLOB,
        "toList" NCLOB,
        "ccList" NCLOB,
        "bccList" NCLOB,
        "replyToList" NCLOB,
        "meetingInfo" nvarchar(128),
        "snippet" nvarchar(256)
	,         "protocolSearchInfo" nvarchar(128)
        ,"threadTopic" nvarchar(128)
        ,syncData nvarchar(128), 
        flagSeen integer,
        mainMailboxKey BIGINT
    ) IN "email_db";

    
CREATE UNIQUE sequence MessageMove_seq AS BIGINT;
CREATE TABLE MessageMove (
	"_id" bigint primary key DEFAULT next_value OF MessageMove_seq with check option,
	messageKey bigint, 
	messageServerId varchar(50), 
	accountKey integer, 
	status integer, 
	srcFolderKey integer, 
	dstFolderKey integer, 
	srcFolderServerId varchar(50), 
	dstFolderServerId varchar(50)
) IN email_db;
	
CREATE UNIQUE sequence MessageStateChange_seq AS BIGINT;
CREATE TABLE MessageStateChange (
	"_id" bigint primary key DEFAULT next_value OF MessageStateChange_seq with check option,
	messageKey integer, 
	messageServerId varchar(50), 
	accountKey integer, 
	status integer, 
	oldFlagRead integer, 
	newFlagRead integer, 
	oldFlagFavorite integer, 
	newFlagFavorite integer
) IN email_db;

CREATE INDEX messagemove_accountKey on MessageMove (accountKey);
CREATE INDEX messagemove_messageKey on MessageMove (messageKey);
CREATE INDEX messagestatechange_accountKey on MessageStateChange (accountKey);
CREATE INDEX messagestatechange_messageKey on MessageStateChange (messageKey);


CREATE TABLE "Message_Deletes"(
        "_id" BIGINT UNIQUE,
        "syncServerId" nvarchar(128),
        "syncServerTimeStamp" bigint,
        "displayName" nvarchar(128),
        "timeStamp" bigint,
        "subject" nvarchar(256),
        "flagRead" INT,
        "flagLoaded" INT,
        "flagFavorite" INT,
        "flagAttachment" INT,
        "flags" INT,
        "clientId" INT,
        "messageId" nvarchar(128),
        "mailboxKey" BIGINT,
        "accountKey" INT,
        "fromList" NCLOB,
        "toList" NCLOB,
        "ccList" NCLOB,
        "bccList" NCLOB,
        "replyToList" NCLOB,
        "meetingInfo" nvarchar(128),
        "snippet" nvarchar(256)
	,  "protocolSearchInfo" nvarchar(128)
        ,"threadTopic" nvarchar(128)
        ,syncData nvarchar(128), 
        flagSeen integer
      ,mainMailboxKey BIGINT
    ) IN "email_db";

CREATE TABLE "Message_Updates"(
        "_id" BIGINT UNIQUE,
        "syncServerId" nvarchar(128),
        "syncServerTimeStamp" bigint,
        "displayName" nvarchar(128),
        "timeStamp" bigint,
        "subject" nvarchar(256),
        "flagRead" INT,
        "flagLoaded" INT,
        "flagFavorite" INT,
        "flagAttachment" INT,
        "flags" INT,
        "clientId" INT,
        "messageId" nvarchar(128),
        "mailboxKey" BIGINT,
        "accountKey" INT,
        "fromList" NCLOB,
        "toList" NCLOB,
        "ccList" NCLOB,
        "bccList" NCLOB,
        "replyToList" NCLOB,
        "meetingInfo" nvarchar(128),
        "snippet" nvarchar(256)
	,  "protocolSearchInfo" nvarchar(128)
        ,"threadTopic" nvarchar(128)
        ,syncData nvarchar(128), 
        flagSeen integer,
        mainMailboxKey BIGINT
    ) IN "email_db";


CREATE UNIQUE sequence policy_seq AS BIGINT;
CREATE TABLE "Policy"(
        "_id" bigint PRIMARY KEY DEFAULT next_value OF policy_seq with check option,
        "passwordMode" INT,
        "passwordMinLength" INT,
        "passwordExpirationDays" INT,
        "passwordHistory" INT,
        "passwordComplexChars" INT,
        "passwordMaxFails" INT,
        "maxScreenLockTime" bigint,
        "requireRemoteWipe" INT,
        "requireEncryption" INT,
        "requireEncryptionExternal" INT,
        "requireManualSyncRoaming" INT,
        "dontAllowCamera" INT,
        "dontAllowAttachments" INT,
        "dontAllowHtml" INT,
        "maxAttachmentSize" INT,
        "maxTextTruncationSize" INT,
        "maxHTMLTruncationSize" INT,
        "maxEmailLookback" INT,
        "maxCalendarLookback" INT,
        "passwordRecoveryEnabled" INT
        
        ,"protocolPoliciesEnforced" varchar(128),
        "protocolPoliciesUnsupported" varchar(128)
    ) IN "email_db";



CREATE UNIQUE sequence quickresponse_seq AS BIGINT;
CREATE TABLE "QuickResponse" (
        "_id" bigint PRIMARY KEY DEFAULT next_value OF quickresponse_seq with check option,
        "quickResponse" nvarchar(128),
        "accountKey" INT
    ) IN "email_db";

    

    
INSERT INTO "android_metadata" ("locale") VALUES ('en_US');

 create index "attachment_messageKey" on "Attachment"("messageKey");

 create index "mailbox_accountKey" on "Mailbox"("accountKey");

 create index "mailbox_serverId" on "Mailbox"("serverId");

 create index "message_flagLoaded" on "Message"("flagLoaded");

 create index "message_flagRead" on "Message"("flagRead");

 create index "message_mailboxKey" on "Message"("mailboxKey");

 create index "message_syncServerId" on "Message"("syncServerId");

 create index "message_timeStamp" on "Message"("timeStamp");



@
 create trigger "account_delete" after delete on "Account"
 referencing old table as old_table 
 
 begin atomic
  for select "_id" as old_id, hostAuthKeyRecv as old_key_rec, hostAuthKeySend as old_key_send, policyKey as old_policy_key from old_table DO

	delete from "Mailbox" where ("accountKey" = old_id);

	delete from "HostAuth" where ("_id" = old_key_rec);

	delete from "HostAuth" where ("_id" = old_key_send);

	delete from "Policy" where ("_id" = old_policy_key);

	

  end for;

 end;
@


@
 create trigger "mailbox_delete" after delete on "Mailbox" 
 referencing old table as old_table 

 begin atomic
  for select "_id" as old_id from old_table DO

	delete from "Message" where ("mailboxKey" = old_id);

	delete from "Message_Updates" where ("mailboxKey" = old_id);

	delete from "Message_Deletes" where ("mailboxKey" = old_id);


  end for;

 end;
@

--combined 3 triggers message_count_message_delete , unread_message_delete and message_delete
--into message_unread_count_message_delete for optimization purpose.
@
CREATE TRIGGER "message_unread_count_message_delete" after delete on "Message" 
 referencing old table as ot

 begin atomic
 
 declare cnt int default 0;
 declare cnt_unread int default 0;
 declare okey int default null;
 
   for select "mailboxKey" as o_mailboxKey, "flagRead" as o_flagRead, "_id" as o_id from ot DO
    delete from "Attachment" where "messageKey" = o_id;


    
      if (okey = o_mailboxkey) then
          if(o_flagRead = 0) then
               set cnt_unread = cnt_unread + 1;
          end if;
               
          set cnt = cnt + 1;
      else
          if (okey is not null) then
              
              update "Mailbox" set "messageCount" = "messageCount" - cnt where "_id" = okey;
              update "Mailbox" set "unreadCount" = "unreadCount"- cnt_unread where "_id" = okey;
   
          end if;
          
          set okey = o_mailboxkey;
          set cnt = 1;
	      if(o_flagRead = 0) then
                set cnt_unread = 1;
          end if;
      end if;
            
  end for;

  update "Mailbox" set "messageCount" = "messageCount" - cnt where "_id" = okey;
  
  update "Mailbox" set "unreadCount" = "unreadCount"- cnt_unread where "_id" = okey; 
 
end
@

@
 create trigger "message_count_message_insert" after insert on "Message" 
 referencing new table as new_table 

 begin atomic
  for select "mailboxKey" as new_mail_key, "_id" as nt_id from new_table DO

	update "Mailbox" set "messageCount" = "messageCount"+1 where ("_id" = new_mail_key);



  end for;

 end;
@


@
 create trigger "message_count_message_move" after update on "Message" 
 referencing old table as old_table new table as new_table

 begin atomic
-- This query asumes that the "_id" column of the updated row has not changed
  for select old_table."mailboxKey" as old_mailboxKey, new_table."mailboxKey" as new_mailboxKey from old_table join new_table on new_table.MIMER_ROWID = old_table.MIMER_ROWID where old_table."mailboxKey" <> new_table."mailboxKey" DO
    	update "Mailbox" set "messageCount" = "messageCount"-1 where (Mailbox."_id" = old_mailboxKey);
	update "Mailbox" set "messageCount" = "messageCount"+1 where (Mailbox."_id" = new_mailboxKey);
  end for;
 end;
@


@
 create trigger "unread_message_insert" after insert on "Message" 
 referencing new table as new_table

 begin atomic
  for select "mailboxKey" as new_mail_key from new_table where new_table.flagRead=0 DO
	update "Mailbox" set "unreadCount" = "unreadCount"+1 where ("_id" = new_mail_key);
  end for;

 end;
@


@
 create trigger "unread_message_move" after update on "Message" 
 referencing old table as old_table new table as new_table

 begin atomic
    for select old_table."mailboxKey" as old_mailboxKey, new_table."mailboxKey" as new_mailboxKey from old_table join new_table on new_table.MIMER_ROWID = old_table.MIMER_ROWID 		where old_table."mailboxKey" <> new_table."mailboxKey" and old_table.flagRead=0 DO
	update "Mailbox" set "unreadCount" = "unreadCount"-1 where ("_id" = old_mailboxKey);

	update "Mailbox" set "unreadCount" = "unreadCount"+1 where ("_id" = new_mailboxKey);

    end for;

 end;
@

@
 create trigger "unread_message_read" after update on "Message" 
 referencing old table as old_table new table as new_table

 begin atomic
    for select old_table."flagRead" as old_flagRead, old_table.mailboxKey as old_mailboxKey from old_table join new_table on new_table.MIMER_ROWID = old_table.MIMER_ROWID where old_table.flagRead <> new_table.flagRead DO
    	update "Mailbox" set "unreadCount" = "unreadCount"+ case old_flagRead when 0 then -1 else 1 end where ("_id" = old_mailboxKey);

    end for;
	
 end;
@

--This trigger also implement the original  MessageStateChange_delete_account
@
 create trigger MessageMove_delete_account after delete on Account
 referencing old table as ot
 
 begin atomic
  for select "_id" as old_id from ot DO
    delete from MessageMove where accountKey=old_id;
    delete from MessageStateChange where accountKey=old_id;
  end for;

 end;
@

--This trigger also implement the original  MessageStateChange_delete_message
@
 create trigger MessageMove_delete_message after delete on Message
 referencing old table as ot
 
 begin atomic
  for select "_id" as old_id from ot DO
    delete from MessageMove where messageKey=old_id;
    delete from MessageStateChange where messageKey=old_id;
  end for;

 end;
@

@
 create trigger message_delete_duplicates_on_insert after insert on Message 
 referencing new table as nt
 
 begin atomic
  for select "_id" as new_id, MailBoxKey as newMailBoxKey, syncServerid as newSyncServerId, accountKey as newAccountKey from nt where syncServerid is not null and (select type from Mailbox where "_id"=nt.mailBoxKey) <> 8 and (select HostAuth.protocol from HostAuth,Account where nt.accountKey=Account."_id" and Account.hostAuthKeyRecv=HostAuth."_id")='eas' DO
    delete from Message where "_id" <> new_id and syncServerId=newSyncServerId and accountKey=newAccountKey and (select Mailbox.type from Mailbox where "_id"=mailboxKey) <> 8;
  end for;

 end;
@



@
create procedure clear_data()
modifies sql data
begin
delete from Attachment;
delete from Account;
delete from HostAuth;
delete from Mailbox;
delete from Message;
delete from Message_Deletes;
delete from Message_Updates;
delete from Policy;
delete from QuickResponse;
delete from "email_body".Body;
end
@

-- entering email_backup schema
read 'email_schema_backup.sql';




grant select on Message to mimer$permission$email_read;
grant select on email_body.Body to mimer$permission$email_read;
