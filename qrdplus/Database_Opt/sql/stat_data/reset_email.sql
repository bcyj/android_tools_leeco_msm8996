set echo off;
-- Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
set message off;
alter sequence account_seq restart with 1;
delete from Account;

alter sequence attachment_seq restart with 1;
delete from Attachment;

alter sequence hostauth_seq restart with 1;
delete from HostAuth;

delete from android_metadata;
insert into android_metadata (locale) values ('en_US');

alter sequence mailbox_seq restart with 1;
delete from Mailbox;

alter sequence message_seq restart with 1;

delete from Message_Deletes;
delete from Message_Updates;

delete from Body;

delete from Message;



alter sequence policy_seq restart with 1;
delete from Policy;

alter sequence quickresponse_seq restart with 1;
delete from QuickResponse;


-- Either sync to fetch email settings, mailboxes OR use below insert statement.

-- inserting account
--insert into Account ("_id", displayName, emailAddress, syncKey, syncLookback, syncInterval, hostAuthKeyRecv, hostAuthKeySend, flags, isDefault, compatibilityUuid, senderName, ringtoneUri, protocolVersion, newMessageCount, securityFlags, securitySyncKey, signature, policyKey, notifiedMessageId, notifiedMessageCount) values (5, 'devoid@mimer.se', 'devoid@mimer.se', null, -1, '15', 9, 10, 2329, 0, '9460f198-64cf-4db0-8c2b-b9ea6ee22cb5', null, 'content://settings/system/notification_sound', null, 0, null, null, null, 0, 0, 0);

-- inserting HostAuth
--insert into HostAuth ("_id", protocol, address, port, flags, login, password, domain, accountKey, certAlias) values (9, 'imap', 'mail2.mimer.se', 993, 13, 'devoid', 'void15void', null, 0, null);
--insert into HostAuth ("_id", protocol, address, port, flags, login, password, domain, accountKey, certAlias) values (10, 'smtp', 'mail.upright.se', 25, 0, null, null, null, 0, null);


-- inserting mailbox
--insert into Mailbox ("_id", displayName, serverId, parentServerId, parentKey, accountKey, type, delimiter, syncKey, syncLookback, syncInterval, syncTime, unreadCount, flagVisible, flags, visibleLimit, syncStatus, messageCount, lastSeenMessageKey, lastTouchedTime) values (5, 'Drafts', 'Drafts', null, -1, 5, 3, 47, null, 0, 0, 0, 0, 1, 24, 25, null, 0, 0, 1343999819930);
--insert into Mailbox ("_id", displayName, serverId, parentServerId, parentKey, accountKey, type, delimiter, syncKey, syncLookback, syncInterval, syncTime, unreadCount, flagVisible, flags, visibleLimit, syncStatus, messageCount, lastSeenMessageKey, lastTouchedTime) values (6, 'Sent', 'Sent', null, -1, 5, 5, 47, null, 0, 0, 0, 0, 1, 24, 25, null, 0, 0, 1343999819930);
--insert into Mailbox ("_id", displayName, serverId, parentServerId, parentKey, accountKey, type, delimiter, syncKey, syncLookback, syncInterval, syncTime, unreadCount, flagVisible, flags, visibleLimit, syncStatus, messageCount, lastSeenMessageKey, lastTouchedTime) values (7, 'Trash', 'Trash', null, -1, 5, 6, 47, null, 0, 0, 0, 0, 1, 24, 25, null, 0, 0, 0);
--insert into Mailbox ("_id", displayName, serverId, parentServerId, parentKey, accountKey, type, delimiter, syncKey, syncLookback, syncInterval, syncTime, unreadCount, flagVisible, flags, visibleLimit, syncStatus, messageCount, lastSeenMessageKey, lastTouchedTime) values (8, 'INBOX', 'INBOX', null, -1, 5, 0, 0, null, 0, 0, 0, 0, 1, 24, 25, null, 0, 0, 0);

