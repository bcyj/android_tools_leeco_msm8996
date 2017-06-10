/******************************************************************************
  @file    IxErrno.h
  @brief   qcril qmi error codes

  DESCRIPTION
    This contains the definition of the return codes (error numbers).
    Functions using this definition either return an error code, or set
    a global variable to the appropriate value.

  ---------------------------------------------------------------------------

  Copyright (c) 2000-2005, 2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef IXERRNO_H
#define IXERRNO_H

/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/

/*===========================================================================

              DEFINITIONS AND CONSTANTS FOR ERROR CODES

===========================================================================*/

typedef enum
{
  /*** Generic outcomes of operations ***/
  E_SUCCESS            =  0,    /* Operation successful                  */
  E_FAILURE            =  1,    /* Operation failed due to unknown err   */
  E_NOT_ALLOWED        =  2,    /* Operation currently not allowed       */
  E_NOT_AVAILABLE      =  3,    /* Operation currently not available     */
  E_NOT_SUPPORTED      =  4,    /* Operation not yet implemented         */
  E_CANCELED           =  5,    /* Operation was scheduled but canceled  */
  E_ABORTED            =  6,    /* Operation was started but interrp'd   */
  E_INTERRUPTED        =  7,    /* Operation was started but interrp'd   */
  E_DEADLOCK           =  8,    /* Operation would cause a deadlock      */
  E_AGAIN              =  9,    /* Caller should retry operation         */
  E_RESET              = 10,    /* Executing module was reset            */
  E_WOULD_BLOCK        = 11,    /* nonblocking IO operation would block  */
  E_IN_PROGRESS        = 12,    /* Operation in progress                 */
  E_ALREADY_DONE       = 13,    /* Operation already completed           */

  /*** Memory management related error conditions ***/
  E_NO_DSM_ITEMS       = 14,    /* Out of DSM items                      */
  E_NO_MEMORY          = 15,    /* Allocation from a memory pool failed  */

  /*** Parameter or data parsing related error conditions ***/
  E_INVALID_ARG        = 16,    /* Arg is not recognized                 */
  E_OUT_OF_RANGE       = 17,    /* Arg value is out of range             */
  E_BAD_ADDRESS        = 18,    /* Ptr arg is bad address                */
  E_NO_DATA            = 19,    /* Expected data, received none          */
  E_BAD_DATA           = 20,    /* Data failed sanity check, e.g. CRC    */
  E_DATA_INVALID       = 21,    /* Data is correct, but contents invalid */
  E_DATA_EXPIRED       = 22,    /* Data is not yet or not any more valid */
  E_DATA_TOO_LARGE     = 23,    /* Data is too large to process          */

  /*** Hardware related error conditions ***/
  E_NO_DEV             = 24,    /* Hardware resource not available       */
  E_DEV_FAILURE        = 25,    /* Hardware failure                      */
  E_NV_READ_ERR        = 26,    /* Operation failed due to NV read err   */
  E_NV_WRITE_ERR       = 27,    /* Operation failed due to NV write err  */
  E_EFS_ERROR          = 28,    /* EFS interface error.                  */
  E_DSP_ERROR          = 29,    /* DSP returned error on cmd (-1)        */

  /*** Protocol operation related error conditions ***/
  E_TIMER_EXP          = 30,    /* Operation not completed (timer exp)   */
  E_VERSION_MISMATCH   = 31,    /* Unexpected software or protocol ver.  */
  E_TASK_SIG_ERR       = 32,    /* Signal unknown or no handler          */
  E_TASK_Q_FULL        = 33,    /* Task queue is full                    */
  E_PROT_Q_FULL        = 34,    /* Protocol queue is full                */
  E_PROT_TX_Q_FULL     = 35,    /* Protocol tx data queue is full        */
  E_PROT_RX_Q_FULL     = 36,    /* Protocol rx data queue is full        */
  E_PROT_UNKN_CMD      = 37,    /* Protocol doesn't understand cmd       */
  E_PROT_UNKN_IND      = 38,    /* Protocol doesn't understand ind       */
  E_PROT_UNKN_MSG      = 39,    /* Protocol doesn't understand msg       */
  E_PROT_UNKN_SIG      = 40,    /* Protocol doesn't understand signal    */

  /*** File related error conditions ***/
  E_NO_ENTRY           = 41,    /* No such file or directory             */
  E_FILENAME_TOO_LONG  = 42,    /* File name too long                    */
  E_DIR_NOT_EMPTY      = 43,    /* Directory not empty                   */
  E_IS_DIR             = 44,    /* Is a directory                        */
  E_EOF                = 45,    /* End Of File                           */
  E_XDEV               = 46,    /* Attempt to cross device               */
  E_BAD_F              = 47,    /* Bad file descriptor                   */
  E_MAX_OPEN_FILES     = 48,    /* Too many open files                   */
  E_BAD_FMT            = 49,    /* Bad formatted media                   */
  E_FILE_EXISTS        = 50,    /* File already exists                   */
  E_EFS_FULL           = 51,    /* No space left on device               */
  E_NOT_DIR            = 52,    /* Not a directory                       */
  E_NOT_EFS_ITEM       = 53,    /* Not an EFS2 item                      */
  E_ALREADY_OPEN       = 54,    /* File/Directory already open           */
  E_BUSY               = 55,    /* Device/Item is busy                   */
  E_OUT_OF_NODES       = 56,    /* Too many symbolic links encountered   */

  /*** SQL related error conditions ***/
  E_SQL_INTERNAL       = 57,    /* An internal logic error in SQLite     */
  E_SQL_BUSY           = 58,    /* The database file is locked           */
  E_SQL_LOCKED         = 59,    /* A table in the database is locked     */
  E_SQL_READONLY       = 60,    /* Attempt to write a readonly database  */
  E_SQL_IOERR          = 61,    /* Some kind of disk I/O error occurred  */
  E_SQL_CORRUPT        = 62,    /* The database disk image is malformed  */
  E_SQL_NOTFOUND       = 63,    /* Table or record not found             */
  E_SQL_FULL           = 64,    /* Insertion failed because DB is full   */
  E_SQL_CANTOPEN       = 65,    /* Unable to open database file          */
  E_SQL_PROTOCOL       = 66,    /* Database lock protocol error          */
  E_SQL_EMPTY          = 67,    /* Database is empty                     */
  E_SQL_SCHEMA         = 68,    /* The database schema changed           */
  E_SQL_CONSTRAINT     = 69,    /* Abort due to constraint violation     */
  E_SQL_MISMATCH       = 70,    /* Data type mismatch                    */
  E_SQL_MISUSE         = 71,    /* Library used incorrectly              */
  E_SQL_NOLFS          = 72,    /* uses OS features not supported        */
  E_SQL_FORMAT         = 73,    /* Auxiliary database format error       */
  E_SQL_NOTADB         = 74,    /* file opened that is not a db file     */
  E_SQL_ROW            = 75,    /* sqlite3_step() has another row ready  */
  E_SQL_DONE           = 76,    /* sqlite3_step() has finished executing */
  E_NO_ATTR            = 77,    /* No Attr found                         */
  E_INVALID_ITEM       = 78,    /* Invalid Item found                    */
  /*** Networking related error conditions ***/
  E_NO_NET             = 100,   /* Network is not connected (up)         */
  E_NOT_SOCKET         = 101,   /* Socket operation on non-socket        */
  E_NO_PROTO_OPT       = 102,   /* Network protocol not available        */
  E_SHUTDOWN           = 103,   /* Socket is being shut down             */
  E_CONN_REFUSED       = 104,   /* Connection refused by peer            */
  E_CONN_ABORTED       = 105,   /* Connection aborted                    */
  E_IS_CONNECTED       = 106,   /* Connection is already established     */
  E_NET_UNREACH        = 107,   /* Network destination is unreachable    */
  E_HOST_UNREACH       = 108,   /* Host destination is unreachable       */
  E_NO_ROUTE           = 109,   /* No route to destination               */
  E_ADDR_IN_USE        = 110,   /* IP address is already in use          */

  /*** BCMCS related error conditions ***/
  E_DB_INVALID         = 111,   /* Entire database is invalid            */
  E_FLOW_NOT_IN_ZONE   = 112,   /* The flow is not in the specified zone */
  E_ZONE_NOT_IN_DB     = 113,   /* The specified zone is not in database */
  E_BUF_OVERFLOW       = 114,   /* The output buffer is too small        */
  // E_EOF              = 115,   /* End of file is reached                */
  E_DB_OVERFLOW        = 116,   /* The db is full, no further updates    */
  E_DB_NO_OVERWRITE    = 117,   /* value exists and overwrite is false   */
  E_NOT_FOUND          = 118,   /* Data not found                        */
  E_NO_RIGHTS          = 119,   /* No Rights/Permission to access data   */

  /*** Concurrency related error ****/
  E_BLOCKED_BY_OUTSTANDING_REQ = 120, /* For concurrency reason blocked by outstanding req(s) */

  /*** HTTP related error conditions ***/
  E_CONTINUE               = 200,   /* Server response status code of continue */

  /* Values to be deleted */
  IxSUCCESS           =  E_SUCCESS,
  IxFAILED,
  IxNOMEMORY,
  IxBADPARAM,
  IxNOTSUPPORTED,

  E_RESERVED           = 0x7FFFFFFF
} IxErrnoType;

/* Backwards compatibility */
typedef IxErrnoType errno_enum_type;

#endif /* IXERRNO_H */
