#ifndef _RDM_RDM_ERROR_H_
#define _RDM_RDM_ERROR_H_

/*
 * This is defined to match the RTK type Ret_t
 */
typedef IS16 RDM_Error;

/*! \defgroup RDM_ERR_defs RDM Errors
 * These error codes are chosen to match the RTK error codes.
 * @{
 */

/*! Success */
#define RDM_ERR_OK                                  0x0000  /*     0 */

/*! Unspecific error */
#define RDM_ERR_UNSPECIFIC                          0x0010  /*    16 */

/*! Memory error */
#define RDM_ERR_MEMORY                              0x0011  /*    17 */

/*! Routine called when not allowed or bad params */
#define RDM_ERR_INVALID_CALL                        0x0012  /*    18 */

/*! Attempt to call TRG_RDM_run with non-resume trigger when suspended */
#define RDM_ERR_IS_SUSPENDED                        0x0013  /*    19 */

/*! SyncML message Protocol or version error */
#define RDM_ERR_INVALID_PROTO_OR_VERSION            0x0020  /*    32 */

/*! RTK workspace buffer too small */
#define RDM_ERR_RTK_BUFFER_OVERFLOW                 0x2002  /*  8194 */

/*
 * These are RDM specific error codes.
 */

/*! Supplied buffer isn't long enough */
#define RDM_ERR_BUFFER_OVERFLOW                     0x6000  /* 24576 */

/*! Badly formatted input */
#define RDM_ERR_BAD_INPUT                           0x6001  /* 24577 */

/*! Tree node already exists */
#define RDM_ERR_ALREADY_EXISTS                      0x6002  /* 24578 */

/*! Tree node is missing */
#define RDM_ERR_NODE_MISSING                        0x6003  /* 24579 */

/*! Parent node is missing */
#define RDM_ERR_PARENT_MISSING                      0x6004  /* 24580 */

/*! Error in leaf node */
#define RDM_ERR_LEAF_NODE                           0x6005  /* 24581 */

/*! Leaf node expected */
#define RDM_ERR_NOT_LEAF_NODE                       0x6006  /* 24582 */

/*! Unknown property */
#define RDM_ERR_UNKNOWN_PROPERTY                    0x6007  /* 24583 */

/*! An attempt was made to delete a permanent node */
#define RDM_ERR_PERMANENT_NODE                      0x6008  /* 24584 */

/*! Not allowed by AccessType */
#define RDM_ERR_NOT_ALLOWED                         0x6009  /* 24585 */

/*! Client aborted */
#define RDM_ERR_ABORT                               0x600a  /* 24586 */

/*! Bad program name passed to Tree Access API */
#define RDM_ERR_PROGRAM_NAME                        0x600b  /* 24587 */

/*! Partial write of external data not allowed */
#define RDM_ERR_EXT_NOT_PARTIAL                     0x600c  /* 24588 */

/*! Write of external data not allowed at this time */
#define RDM_ERR_EXT_NOT_ALLOWED                     0x600d  /* 24589 */

/*! May not replace */
#define RDM_ERR_MAY_NOT_REPLACE                     0x600e  /* 24590 */

/*! Tree read error */
#define RDM_ERR_TREE_READ                           0x600f  /* 24591 */

/*! Tree write error */
#define RDM_ERR_TREE_WRITE                          0x6010  /* 24592 */

/*! Authentication failure */
#define RDM_ERR_AUTHENTICATION                      0x6011  /* 24593 */

/*! Access denied by ACL */
#define RDM_ERR_ACCESS_DENIED                       0x6012  /* 24594 */

/*! External data value is not readable */
#define RDM_ERR_VALUE_NOT_READABLE                  0x6013  /* 24595 */

/*! External data value is not writeable */
#define RDM_ERR_VALUE_NOT_WRITEABLE                 0x6014  /* 24596 */

/*! Node not registered for execute */
#define RDM_ERR_NOT_EXECUTABLE                      0x6015  /* 24597 */

/*! Tree open error */
#define RDM_ERR_TREE_OPEN                           0x6016  /* 24598 */

/*! Tree commit error */
#define RDM_ERR_TREE_COMMIT                         0x6017  /* 24599 */

/*! No more commands */
#define RDM_ERR_NO_MORE_COMMANDS                    0x6100 /* 24832 */

/*! Missing start message command */
#define RDM_ERR_MISSING_START_MESSAGE_CMD           0x6101 /* 24833 */

/*! Missing end message command */
#define RDM_ERR_MISSING_STATUS_CMD                  0x6102 /* 24834 */

/*! Optional feature not implemented */
#define RDM_ERR_OPTIONAL_FEATURE_NOT_IMPLEMENTED    0x6103 /* 24835 */

/*      SPARE                                       0x6104    24836 */

/*! Alert options parsing error */
#define RDM_ERR_ALERT_PARSING_ERROR                 0x6105 /* 24837 */

/*! Not enough items with Alert */
#define RDM_ERR_ALERT_MISSING_ITEMS                 0x6106 /* 24838 */

/*! Not enough items with Alert */
#define RDM_ERR_ALERT_MISSING_DATA                  0x6107 /* 24839 */

/*! Not enough items with Alert */
#define RDM_ERR_NO_DATA                             0x6108 /* 24840 */

/*! User cancelled or aborted the Alert */
#define RDM_ERR_ALERT_USER_ABORTED                  0x6109 /* 24841 */

/*! Too many choices passed to implementation */
#define RDM_ERR_ALERT_TOO_MANY_CHOICES              0x610a /* 24842 */

/*! Server has sent a session-abort alert */
#define RDM_ERR_ALERT_SESSION_ABORTED               0x610b /* 24843 */

/*! Large object item has been handled - this is not really an error */
#define RDM_ERR_LO_HANDLED                          0x610c /* 24844 */

/*! Data is too long to pass back as a large object */
#define RDM_ERR_TOO_BIG                             0x610d /* 24845 */

/*! Command status code is failed */
#define RDM_ERR_COMMAND_FAILED                      0x610e /* 24846 */

/*! Notification message has invalid length */
#define RDM_ERR_NOTIF_BAD_LENGTH                    0x6200  /* 25088 */

/*! Notification message has invalid digest */
#define RDM_ERR_NOTIF_BAD_DIGEST                    0x6201  /* 25089 */

/*! Boot message has invalid digest */
#define RDM_ERR_BOOT_DIGEST                         0x6202  /* 25090 */

/*! Could not get NSS for bootstrap */
#define RDM_ERR_BOOT_NSS                            0x6203  /* 25091 */

/*! Could not get PIN for bootstrap */
#define RDM_ERR_BOOT_PIN                            0x6204  /* 25092 */

/*! Bad bootstrap PIN length */
#define RDM_ERR_BOOT_PINLENGTH                      0x6205  /* 25093 */

/*! Bad bootstrap SEC value */
#define RDM_ERR_BOOT_BAD_SEC                        0x6206  /* 25094 */

/*! Bad bootstrap MAC */
#define RDM_ERR_BOOT_BAD_MAC                        0x6207  /* 25095 */

/*! Bad bootstrap message */
#define RDM_ERR_BOOT_BAD_MESSAGE                    0x6208  /* 25096 */

/*! Bad bootstrap profile */
#define RDM_ERR_BOOT_BAD_PROF                       0x6209  /* 25097 */

/*! Bad trigger reason */
#define RDM_ERR_TRG_BAD_REASON                      0x6210  /* 25104 */

/*! Notification message contains unsupported version info */
#define RDM_ERR_NOTIF_UNSUPPORTED_VERSION           0x6211  /* 25105 */

/*! Bootstrap not currently allowed */
#define RDM_ERR_BOOT_DISABLED                       0x6212  /* 25106 */

/* non-DM Bootstrap message  */
#define RDM_ERROR_CP_BOOSTRAP_WITHOUT_DM_PROFILE    0x6213  /* 25107 */

/*
 *  Communication errors
 */

/*! Unsuppported protocol */
#define RDM_ERR_COMMS_BAD_PROTOCOL                  0x6300 /* 25344 */

/*! Mis-matched reply (XML received when WBXML sent or vice-versa) */
#define RDM_ERR_COMMS_MIME_MISMATCH                 0x6301 /* 25345 */

/*! General fatal transport error */
#define RDM_ERR_COMMS_FATAL                         0x6302 /* 25346 */

/*! General non-fatal (retriable) transport error */
#define RDM_ERR_COMMS_NON_FATAL                     0x6303 /* 25347 */

/*
 * These are FUMO specific error codes.
 */

/*! Something impossible happened */
#define RDM_ERR_INTERNAL                            0x6401 /* 25601 */

/*! Error accessing FUMO storage */
#define RDM_ERR_FUMO_STORAGE                        0x6402 /* 25602 */

/*! User cancelled update/download */
#define RDM_ERR_CANCEL                              0x6404 /* 25604 */

/*! Could not initiate update client */
#define RDM_ERR_UPDATE_INIT                         0x6405 /* 25605 */

/*! DL URL is malformed or bad */
#define RDM_ERR_BAD_URL                             0x6406 /* 25606 */

/*! DD of DL is bad */
#define RDM_ERR_BAD_DD                              0x6407 /* 25607 */

/*! etag changed, 412 returned because of If-match: */
#define RDM_ERR_COMMS_OBJECT_CHANGED                0x6408 /* 25608 */

/*
 *  State machine (event driven) specific errors
 */

/*! When calling to RDM_TRG_Run with unexpected trigger */
#define RDM_ERR_OUT_OF_SYNC                         0x6500 /* 25856 */
/*
 * These are DS specific error codes.
 */
#define RDM_ERR_MISSING_DATASTORE       0x7001
#define RDM_ERR_NO_ANCHOR               0x7002
#define RDM_ERR_SUCCESS_ADD             0x7201
#define RDM_ERR_CLIENT_PREVAIL          0x7208
#define RDM_ERR_DELETED_WITHOUT_ARCHIVE 0x7210
#define RDM_ERR_DATA_NOT_DELETED        0x7211
#define RDM_ERR_PASS_AUTHNTICATION      0x7212
#define RDM_ERR_DELIVER_LARGE_OBJ       0x7213
#define RDM_ERR_AUTH_ERR                0x7401
#define RDM_ERR_AUTH_ERR_ANOTHER_SYNCML 0x7403
#define RDM_ERR_DELETE_NO_DATE          0x7404
#define RDM_ERR_AUTH_REQUESTED          0x7407
#define RDM_ERR_REVISE_CONFLICT         0x7409
#define RDM_ERR_UNINTEGRATED_CMD        0x7412
#define RDM_ERR_TARGET_DATA_EXISTS      0x7418
#define RDM_ERR_CONFLICT_SRV_PREVAIL    0x7419
#define RDM_ERR_LIMIT_CAPACITY          0x7420
#define RDM_ERR_DEFAULT_SERVER_ERR      0x7500
#define RDM_ERR_BUSY_SERVER             0x7503
#define RDM_ERR_MORE_SLOW_SYNC          0x7508
#define RDM_ERR_ACCESS_DATABASE         0x7510
#define RDM_ERR_SERVER_ERROR            0x7511
#define RDM_ERR_FAILED_SYNCML           0x7512
#define RDM_ERR_UNSUPPORTED_PROT_VER    0x7513

/* @} */

#endif /* !_RDM_RDM_ERROR_H_ */
