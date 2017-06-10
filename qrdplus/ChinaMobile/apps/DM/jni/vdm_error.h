#ifndef _VDM_ERROR_H_
#define _VDM_ERROR_H_

#include "rdm_error.h"

typedef IS16 VDM_Error;

/*! \defgroup VDM_ERR_defs RDM Errors
 * @{
 */

/*! Success */
#define VDM_ERR_OK                          (RDM_ERR_OK)

/*! Unspecific error */
#define VDM_ERR_UNSPECIFIC                  (RDM_ERR_UNSPECIFIC)

/*! Memory error */
#define VDM_ERR_MEMORY                      (RDM_ERR_MEMORY)

/*! Routine called when not allowed or bad params */
#define VDM_ERR_INVALID_CALL                (RDM_ERR_INVALID_CALL)

/*! Attempt to call TRG_RDM_run with non-resume trigger when suspended */
#define VDM_ERR_IS_SUSPENDED                (RDM_ERR_IS_SUSPENDED)

/*! SyncML message Protocol or version error */
#define VDM_ERR_INVALID_PROTO_OR_VERSION    (RDM_ERR_INVALID_PROTO_OR_VERSION)

/*! RTK workspace buffer too small */
#define VDM_ERR_RTK_BUFFER_OVERFLOW         (RDM_ERR_RTK_BUFFER_OVERFLOW)

/*! Supplied buffer is too small */
#define VDM_ERR_BUFFER_OVERFLOW             (RDM_ERR_BUFFER_OVERFLOW)

/*! Badly formatted input */
#define VDM_ERR_BAD_INPUT                   (RDM_ERR_BAD_INPUT)

/*! Tree node already exists */
#define VDM_ERR_NODE_ALREADY_EXISTS             (RDM_ERR_ALREADY_EXISTS)

/*! Tree node is missing */
#define VDM_ERR_NODE_MISSING                (RDM_ERR_NODE_MISSING)

/*! Parent node is missing */
#define VDM_ERR_PARENT_MISSING              (RDM_ERR_PARENT_MISSING)

/*! Error in leaf node */
#define VDM_ERR_LEAF_NODE                   (RDM_ERR_LEAF_NODE)

/*! Leaf node expected */
#define VDM_ERR_NOT_LEAF_NODE               (RDM_ERR_NOT_LEAF_NODE)

/*! Unknown property */
#define VDM_ERR_UNKNOWN_PROPERTY            (RDM_ERR_UNKNOWN_PROPERTY)

/*! An attempt was made to delete a permanent node */
#define VDM_ERR_PERMANENT_NODE              (RDM_ERR_PERMANENT_NODE)

/*! Not allowed by AccessType */
#define VDM_ERR_NOT_ALLOWED                 (RDM_ERR_NOT_ALLOWED)

/*! Client aborted */
#define VDM_ERR_ABORT                       (RDM_ERR_ABORT)

/*! Client access denied */
#define VDM_ERR_TREE_ACCESS_DENIED          (RDM_ERR_PROGRAM_NAME)

/*! Partial write of external data not allowed */
#define VDM_ERR_TREE_EXT_NOT_PARTIAL        (RDM_ERR_EXT_NOT_PARTIAL)

/*! Write of external data not allowed at this time */
#define VDM_ERR_TREE_EXT_NOT_ALLOWED        (RDM_ERR_EXT_NOT_ALLOWED)

/*! May not replace */
#define VDM_ERR_TREE_MAY_NOT_REPLACE        (RDM_ERR_MAY_NOT_REPLACE)

/*! Persistent storage read error */
#define VDM_ERR_STORAGE_READ                (RDM_ERR_TREE_READ)

/*! Persistent storage write error */
#define VDM_ERR_STORAGE_WRITE               (RDM_ERR_TREE_WRITE)

/*! Authentication failure */
#define VDM_ERR_AUTHENTICATION              (RDM_ERR_AUTHENTICATION)

/*! Access denied by ACL */
#define VDM_ERR_NODE_ACCESS_DENIED          (RDM_ERR_ACCESS_DENIED)

/*! External data value is not readable */
#define VDM_ERR_NODE_VALUE_NOT_READABLE     (RDM_ERR_VALUE_NOT_READABLE)

/*! External data value is not writable */
#define VDM_ERR_NODE_VALUE_NOT_WRITEABLE    (RDM_ERR_VALUE_NOT_WRITEABLE)

/*! Node not registered for execute */
#define VDM_ERR_NODE_NOT_EXECUTABLE         (RDM_ERR_NOT_EXECUTABLE)

/*! Tree open error */
#define VDM_ERR_STORAGE_OPEN                (RDM_ERR_TREE_OPEN)

/*! Tree commit error */
#define VDM_ERR_STORAGE_COMMIT              (RDM_ERR_TREE_COMMIT)

/*! No more commands */
#define VDM_ERR_NO_MORE_COMMANDS            (RDM_ERR_NO_MORE_COMMANDS)

/*! Missing start message command */
#define VDM_ERR_MISSING_START_MESSAGE_CMD   (RDM_ERR_MISSING_START_MESSAGE_CMD)

/*! Missing end message command */
#define VDM_ERR_MISSING_STATUS_CMD          (RDM_ERR_MISSING_STATUS_CMD)

/*! Optional feature not implemented */
#define VDM_ERR_NOT_IMPLEMENTED             (RDM_ERR_OPTIONAL_FEATURE_NOT_IMPLEMENTED)

/*! Alert options parsing error */
#define VDM_ERR_ALERT_PARSING_ERROR         (RDM_ERR_ALERT_PARSING_ERROR)

/*! Not enough items with Alert */
#define VDM_ERR_ALERT_MISSING_ITEMS         (RDM_ERR_ALERT_MISSING_ITEMS)

/*! Not enough items with Alert */
#define VDM_ERR_ALERT_MISSING_DATA          (RDM_ERR_ALERT_MISSING_DATA)

/*! Not enough items with Alert */
#define VDM_ERR_NO_DATA                     (RDM_ERR_NO_DATA)

/*! User canceled or aborted the Alert */
#define VDM_ERR_ALERT_USER_ABORTED          (RDM_ERR_ALERT_USER_ABORTED)

/*! Too many choices passed to implementation */
#define VDM_ERR_ALERT_TOO_MANY_CHOICES      (RDM_ERR_ALERT_TOO_MANY_CHOICES)

/*! Server has sent a session-abort alert */
#define VDM_ERR_ALERT_SESSION_ABORTED       (RDM_ERR_ALERT_SESSION_ABORTED)

/*! Large object item has been handled - this is not really an error */
#define VDM_ERR_LO_HANDLED                  (RDM_ERR_LO_HANDLED)

/*! Data is too long to pass back as a large object */
#define VDM_ERR_TOO_BIG                     (RDM_ERR_TOO_BIG)

/*! Command status code is failed */
#define VDM_ERR_COMMAND_FAILED              (RDM_ERR_COMMAND_FAILED)

/*! Notification message has invalid length */
#define VDM_ERR_NOTIF_BAD_LENGTH            (RDM_ERR_NOTIF_BAD_LENGTH)

/*! Notification message has invalid digest */
#define VDM_ERR_NOTIF_BAD_DIGEST            (RDM_ERR_NOTIF_BAD_DIGEST)

/*! Boot message has invalid digest */
#define VDM_ERR_BOOT_DIGEST                 (RDM_ERR_BOOT_DIGEST)

/*! Could not get NSS for bootstrap */
#define VDM_ERR_BOOT_NSS                    (RDM_ERR_BOOT_NSS)

/*! Could not get PIN for bootstrap */
#define VDM_ERR_BOOT_PIN                    (RDM_ERR_BOOT_PIN)

/*! Bad bootstrap PIN length */
#define VDM_ERR_BOOT_PINLENGTH              (RDM_ERR_BOOT_PINLENGTH)

/*! Bad bootstrap SEC value */
#define VDM_ERR_BOOT_BAD_SEC                (RDM_ERR_BOOT_BAD_SEC)

/*! Bad bootstrap MAC */
#define VDM_ERR_BOOT_BAD_MAC                (RDM_ERR_BOOT_BAD_MAC)

/*! Bad bootstrap message */
#define VDM_ERR_BOOT_BAD_MESSAGE            (RDM_ERR_BOOT_BAD_MESSAGE)

/*! Bad bootstrap profile */
#define VDM_ERR_BOOT_BAD_PROF               (RDM_ERR_BOOT_BAD_PROF)

/*! Bad trigger reason */
#define VDM_ERR_TRG_BAD_REASON              (RDM_ERR_TRG_BAD_REASON)

/*! Notification message contains unsupported version info */
#define VDM_ERR_NOTIF_UNSUPPORTED_VERSION   (RDM_ERR_NOTIF_UNSUPPORTED_VERSION)

/*! Bootstrap not currently allowed */
#define VDM_ERR_BOOT_DISABLED               (RDM_ERR_BOOT_DISABLED)

/*! non-DM Bootstrap message  */
#define VDM_ERR_CP_BOOSTRAP_WITHOUT_DM_PROFILE  (RDM_ERROR_CP_BOOSTRAP_WITHOUT_DM_PROFILE)

/*
 *  Communication errors
 */

/*! Unsupported protocol */
#define VDM_ERR_COMMS_BAD_PROTOCOL          (RDM_ERR_COMMS_BAD_PROTOCOL)

/*! Mis-matched reply (XML received when WBXML sent or vice-versa) */
#define VDM_ERR_COMMS_MIME_MISMATCH         (RDM_ERR_COMMS_MIME_MISMATCH)

/*! General fatal transport error */
#define VDM_ERR_COMMS_FATAL                 (RDM_ERR_COMMS_FATAL)

/*! General non-fatal (re triable) transport error */
#define VDM_ERR_COMMS_NON_FATAL             (RDM_ERR_COMMS_NON_FATAL)

/*
 * These are FUMO specific error codes.
 */

/*! Internal error has occurred */
#define VDM_ERR_INTERNAL                    (RDM_ERR_INTERNAL)

/*! Error accessing MO external storage */
#define VDM_ERR_MO_STORAGE              (RDM_ERR_FUMO_STORAGE)

/*! User canceled update/download */
#define VDM_ERR_CANCEL                      (RDM_ERR_CANCEL)

/*! Could not initiate update client */
#define VDM_ERR_UPDATE_INIT                 (RDM_ERR_UPDATE_INIT)

/*! DL URL is malformed or bad */
#define VDM_ERR_BAD_URL                     (RDM_ERR_BAD_URL)

/*! DD of DL is bad */
#define VDM_ERR_BAD_DD                      (RDM_ERR_BAD_DD)

/*! etag changed, 412 returned because of If-match: */
#define VDM_ERR_COMMS_OBJECT_CHANGED        (RDM_ERR_COMMS_OBJECT_CHANGED)

/*
 *  State machine (event driven) specific errors
 */

/*! When calling to RDM_TRG_Run with unexpected trigger */
#define VDM_ERR_OUT_OF_SYNC                 (RDM_ERR_OUT_OF_SYNC)

#define VDM_ERR_MISSING_DATASTORE           (RDM_ERR_MISSING_DATASTORE)

#define VDM_ERR_NO_ANCHOR                   (RDM_ERR_NO_ANCHOR)

#define VDM_ERR_SUCCESS_ADD                 (RDM_ERR_SUCCESS_ADD)

#define VDM_ERR_CLIENT_PREVAIL              (RDM_ERR_CLIENT_PREVAIL)

#define VDM_ERR_DELETED_WITHOUT_ARCHIVE     (RDM_ERR_DELETED_WITHOUT_ARCHIVE)

#define VDM_ERR_DATA_NOT_DELETED            (RDM_ERR_DATA_NOT_DELETED)

#define VDM_ERR_PASS_AUTHNTICATION          (RDM_ERR_PASS_AUTHNTICATION)

#define VDM_ERR_DELIVER_LARGE_OBJ           (RDM_ERR_DELIVER_LARGE_OBJ)

#define VDM_ERR_AUTH_ERR                    (RDM_ERR_AUTH_ERR)

#define VDM_ERR_AUTH_ERR_ANOTHER_SYNCML     (RDM_ERR_AUTH_ERR_ANOTHER_SYNCML)

#define VDM_ERR_DELETE_NO_DATE              (RDM_ERR_DELETE_NO_DATE)

#define VDM_ERR_AUTH_REQUESTED              (RDM_ERR_AUTH_REQUESTED)

#define VDM_ERR_REVISE_CONFLICT             (RDM_ERR_REVISE_CONFLICT)

#define VDM_ERR_UNINTEGRATED_CMD            (RDM_ERR_UNINTEGRATED_CMD)

#define VDM_ERR_TARGET_DATA_EXISTS          (RDM_ERR_TARGET_DATA_EXISTS)

#define VDM_ERR_CONFLICT_SRV_PREVAIL        (RDM_ERR_CONFLICT_SRV_PREVAIL)

#define VDM_ERR_LIMIT_CAPACITY              (RDM_ERR_LIMIT_CAPACITY)

#define VDM_ERR_DEFAULT_SERVER_ERR          (RDM_ERR_DEFAULT_SERVER_ERR)

#define VDM_ERR_BUSY_SERVER                 (RDM_ERR_BUSY_SERVER)

#define VDM_ERR_MORE_SLOW_SYNC              (RDM_ERR_MORE_SLOW_SYNC)

#define VDM_ERR_ACCESS_DATABASE             (RDM_ERR_ACCESS_DATABASE)

#define VDM_ERR_SERVER_ERROR                (RDM_ERR_SERVER_ERROR)

#define VDM_ERR_FAILED_SYNCML               (RDM_ERR_FAILED_SYNCML)

#define VDM_ERR_UNSUPPORTED_PROT_VER        (RDM_ERR_UNSUPPORTED_PROT_VER)

/* @} */
#endif

