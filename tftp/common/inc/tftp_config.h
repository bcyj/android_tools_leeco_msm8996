/***********************************************************************
 * tftp_config.h
 *
 * The TFTP configuration module.
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * The TFTP configuration module.
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2014-12-30   dks   Fixes to config and log module.
2014-12-10   dks   Separate client and server configs.
2014-09-19   dks   Add hooks to extract performance numbers.
2014-07-28   rp    Move log-buffer from global to stack.
2014-07-18   rp    tftp and ipc-router integration changes from target.
2014-06-30   nr    Support connected sockets and muti-poll.
2014-06-04   rp    Switch to IPCRouter sockets.
2014-01-20   dks   Fix compilation on windows builds.
2013-12-26   rp    Add tftp-client module.
2013-12-06   rp    Correct the TFTP_MAX_DATA_BLOCK_SIZE macro name.
2013-12-05   nr    Modify the server code to compile across all HLOS.
2013-11-14   rp    Create

===========================================================================*/

#ifndef __TFTP_CONFIG_H__
#define __TFTP_CONFIG_H__

#if defined (TFTP_NHLOS_BUILD)
  #include "customer.h"
#endif

// todo: get this ID allocated from mproc-team
#define TFTP_SERVER_SERVICE_ID   4096


enum tftp_server_instance_id_type
{
  TFTP_SERVER_INSTANCE_ID_INVALID      = 0,
  TFTP_SERVER_INSTANCE_ID_MSM_MPSS     = 1,
  TFTP_SERVER_INSTANCE_ID_MSM_ADSP     = 2,
  TFTP_SERVER_INSTANCE_ID_MDM_MPSS     = 3,
  TFTP_SERVER_INSTANCE_ID_MDM_ADSP     = 4,
  TFTP_SERVER_INSTANCE_ID_MDM_SPARROW  = 5,
  TFTP_SERVER_INSTANCE_ID_APQ_GSS      = 6,
  TFTP_SERVER_INSTANCE_ID_MAX          = 7,
};


#ifndef TFTP_SERVER_DEFAULT_DATA_BLOCK_SIZE
  #define TFTP_SERVER_DEFAULT_DATA_BLOCK_SIZE      512
#endif

/* Limits the RAM size allocated on server to acceptable number */
#ifndef TFTP_MAX_DATA_BLOCK_SIZE
  #define TFTP_MAX_DATA_BLOCK_SIZE             7680
#endif

/* IPCR pkt size is 8k. Leaving space for IPCR header keep tftp blk at 7.5K */
#ifndef TFTP_CLIENT_DATA_BLOCK_SIZE
  #define TFTP_CLIENT_DATA_BLOCK_SIZE          7680
#endif

#ifndef TFTP_SERVER_DEFAULT_TIMEOUT_IN_MS
  #define TFTP_SERVER_DEFAULT_TIMEOUT_IN_MS    1000
#endif

#ifndef TFTP_MAX_TIMEOUT_IN_MS
  #define TFTP_MAX_TIMEOUT_IN_MS               5000
#endif

#ifndef TFTP_CLIENT_TIMEOUT_IN_MS
  #define TFTP_CLIENT_TIMEOUT_IN_MS            1500
#endif

#ifndef TFTP_SERVER_DEFAULT_WINDOW_SIZE
  #define TFTP_SERVER_DEFAULT_WINDOW_SIZE      1 /* 0 is infinite window size*/
#endif

#ifndef TFTP_MAX_WINDOW_SIZE
  #define TFTP_MAX_WINDOW_SIZE                 10
#endif

#ifndef TFTP_CLIENT_WINDOW_SIZE
  #define TFTP_CLIENT_WINDOW_SIZE              10
#endif

#ifndef TFTP_MAX_PKT_RETRY_COUNT
  #define TFTP_MAX_PKT_RETRY_COUNT             5
#endif

#ifndef TFTP_MAX_WRONG_PKT_COUNT
  #define TFTP_MAX_WRONG_PKT_COUNT             100
#endif

#ifndef TFTP_MAX_TFTP_PACKET_SIZE
  #define TFTP_MAX_TFTP_PACKET_SIZE (TFTP_MAX_DATA_BLOCK_SIZE + 4)
#endif

#ifndef TFTP_MAX_FILE_NAME_LEN
  #define TFTP_MAX_FILE_NAME_LEN  1024
#endif

#ifndef TFTP_LOG_LINE_BUF_SIZE_IN_BYTES
  #define TFTP_LOG_LINE_BUF_SIZE_IN_BYTES   (128)
#endif

#ifndef TFTP_SERVER_INFO_FILEPATH
  #define TFTP_SERVER_INFO_FILEPATH "shared/server_info.txt"
#endif

#ifndef TFTP_SERVER_TEMP_FILEPATH
  #define TFTP_SERVER_TEMP_FILEPATH "/readwrite/server_check.txt"
#endif

#ifndef TFTP_SERVER_TEMP_FILEPATH_DATA
  #define TFTP_SERVER_TEMP_FILEPATH_DATA "hello"
#endif


#endif /* __TFTP_CONFIG_H__ */
