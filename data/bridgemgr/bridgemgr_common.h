/******************************************************************************

                   B R I D G E M G R _ C O M M O N . H

******************************************************************************/

/******************************************************************************

  @file    bridgemgr_common.h
  @brief   Bridge Manager Common Definitions Header File

  DESCRIPTION
  Header file for BridgeMgr common definitions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
04/25/11   sg         Initial version

******************************************************************************/

#ifndef __BRIDGEMGR_COMMON_H__
#define __BRIDGEMGR_COMMON_H__

#include <pthread.h>

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

/* Subsystem modules from which we can expect to receive msgs/events */
typedef enum
{
  BRIDGEMGR_SYS_INVALID = -1,
  BRIDGEMGR_SYS_FIRST,
  BRIDGEMGR_SYS_USB_QMI = BRIDGEMGR_SYS_FIRST, /* QMI msgs to/from USB subsystem */
  BRIDGEMGR_SYS_QMI_PROXY,                     /* QMI msgs to/from QMI Proxy */
  BRIDGEMGR_SYS_MDM_QMI,                       /* QMI msgs to/from 9K modem */
  BRIDGEMGR_SYS_PS_NETLINK,                    /* USB netlink msgs from kernel for port switching */
  BRIDGEMGR_SYS_PS_QMI_IND,                    /* QMI indications from 9k for port switching */

  BRIDGEMGR_SYS_REINIT,                        /* Special system for reiniting other systems */

  /* Should be the last entry */
  BRIDGEMGR_SYS_MAX,
} bridgemgr_sys_type;

/* Definition for different types of re-initialization */
typedef enum
{
  BRIDGEMGR_REINIT_TYPE_CLEANUP = (1 << 0),
  BRIDGEMGR_REINIT_TYPE_INIT    = (1 << 1),
  BRIDGEMGR_REINIT_TYPE_FULL    = (BRIDGEMGR_REINIT_TYPE_CLEANUP |
                                   BRIDGEMGR_REINIT_TYPE_INIT)
} bridgemgr_reinit_type;


/* Forward declaration */
typedef struct bridgemgr_client_callbacks_type bridgemgr_client_callbacks_type;

/* Cleanup function pointer type */
typedef void (*bridgemgr_cleanup_func_type)(bridgemgr_sys_type sys,
                                            bridgemgr_client_callbacks_type *cb);

/* Read funtion pointer type */
typedef int (*bridgemgr_read_func_type)(int fd, void *data, int size, int *fill_size);

/* Process funtion pointer type */
typedef int (*bridgemgr_process_func_type)(const void *data, int size);

/* Client callback info structure */
struct bridgemgr_client_callbacks_type
{
  int                          fd;             /* fd to wait on or FD_NONE */
  bridgemgr_process_func_type  process_func;   /* Data processing callback */
  bridgemgr_cleanup_func_type  cleanup_func;   /* Cleanup callback */
  pthread_t                    read_thread_id; /* Client's read thread */
};


/*--------------------------------------------------------------------------- 
   Logging macros
---------------------------------------------------------------------------*/
#undef  DS_LOG_TAG
#define DS_LOG_TAG "QC-BRIDGEMGR"

#ifdef FEATURE_DATA_INTERNAL_LOG_TO_FILE

#define  bridgemgr_log_err(...)      do                                                       \
                                     {                                                        \
                                       if (bm_fp)                                             \
                                       {                                                      \
                                         struct timeval tv;                                   \
                                         gettimeofday(&tv, NULL);                             \
                                         fprintf(bm_fp, "%lu.%-10lu", tv.tv_sec, tv.tv_usec); \
                                         fprintf(bm_fp, __VA_ARGS__);                         \
                                         fflush(bm_fp);                                       \
                                       }                                                      \
                                       ds_log_err(__VA_ARGS__);                               \
                                     }                                                        \
                                     while(0)
#define  bridgemgr_log_high(...)     do                                                       \
                                     {                                                        \
                                       if (bm_fp)                                             \
                                       {                                                      \
                                         struct timeval tv;                                   \
                                         gettimeofday(&tv, NULL);                             \
                                         fprintf(bm_fp, "%lu.%-10lu", tv.tv_sec, tv.tv_usec); \
                                         fprintf(bm_fp, __VA_ARGS__);                         \
                                         fflush(bm_fp);                                       \
                                       }                                                      \
                                       ds_log_high(__VA_ARGS__);                              \
                                     }                                                        \
                                     while(0)
#define  bridgemgr_log_med(...)      do                                                       \
                                     {                                                        \
                                       if (bm_fp)                                             \
                                       {                                                      \
                                         struct timeval tv;                                   \
                                         gettimeofday(&tv, NULL);                             \
                                         fprintf(bm_fp, "%lu.%-10lu", tv.tv_sec, tv.tv_usec); \
                                         fprintf(bm_fp, __VA_ARGS__);                         \
                                         fflush(bm_fp);                                       \
                                       }                                                      \
                                       ds_log_med(__VA_ARGS__);                               \
                                     }                                                        \
                                     while(0)
#define  bridgemgr_log_low(...)      do                                                       \
                                     {                                                        \
                                       if (bm_fp)                                             \
                                       {                                                      \
                                         struct timeval tv;                                   \
                                         gettimeofday(&tv, NULL);                             \
                                         fprintf(bm_fp, "%lu.%-10lu", tv.tv_sec, tv.tv_usec); \
                                         fprintf(bm_fp, __VA_ARGS__);                         \
                                         fflush(bm_fp);                                       \
                                       }                                                      \
                                       ds_log_low(__VA_ARGS__);                               \
                                     }                                                        \
                                     while(0)

#else

#define  bridgemgr_log_err           ds_log_err
#define  bridgemgr_log_high          ds_log_high
#define  bridgemgr_log_med           ds_log_med
#define  bridgemgr_log_low           ds_log_low

#endif /* FEATURE_DATA_INTERNAL_LOG_TO_FILE */

#define  bridgemgr_log               ds_log
#define  bridgemgr_log_dflt          ds_log_dflt
#define  bridgemgr_log_sys_err       ds_log_sys_err
#define  bridgemgr_log_init          ds_log_init2


/*===========================================================================
                            GLOBAL FUNCTION DECLARATIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  bridgemgr_common_read
===========================================================================*/
/*!
@brief
  This function reads data from the given fd into the buffer pointed by data.
  If the available data exceeds the buffer capacity, excess data is read and
  discarded and an error is returned

@param
  fd   - registered file descriptor
  data - buffer to read the data into
  size - allocated size of the data buffer
  fill_size [out] - size of the buffer actually filled by this function

@return
  BRIDGEMGR_SUCCESS - If data was successfully read
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
int bridgemgr_common_read
(
  bridgemgr_sys_type  sys,
  int                 fd,
  void                *data,
  int                 size,
  int                 *fill_size
);


/*===========================================================================
  FUNCTION  bridgemgr_common_wait_for_init
===========================================================================*/
/*!
@brief
  This function delays the calling thread until the bridgemgr initialization
  is complete

@param
  sys - module enum

@return
  none

*/
/*=========================================================================*/
void bridgemgr_common_wait_for_init
(
  bridgemgr_sys_type sys
);


/*===========================================================================
  FUNCTION  bridgemgr_common_get_sys_str
===========================================================================*/
/*!
@brief
  This function returns the string representation of the given module enum

@param
  sys - module enum

@return
  String corresponding to the given enum

*/
/*=========================================================================*/
const char *bridgemgr_common_get_sys_str
(
  bridgemgr_sys_type sys
);


/*===========================================================================
  FUNCTION  bridgemgr_common_get_reinit_str
===========================================================================*/
/*!
@brief
  This function returns the string representation of the given module enum

@param
  sys - module enum

@return
  String corresponding to the given enum

*/
/*=========================================================================*/
const char *bridgemgr_common_get_reinit_str
(
  bridgemgr_reinit_type reinit
);


/*===========================================================================
  FUNCTION  bridgemgr_common_print_qmux_msg
===========================================================================*/
/*!
@brief
  This function prints the hexadecimal representation of the given message

@param
  sys  - module enum
  data - integer data
  size - size of the data

@return
  none

*/
/*=========================================================================*/
void bridgemgr_common_print_qmux_msg
(
  bridgemgr_sys_type sys,
  const void *msg,
  int size
);


/*===========================================================================
  FUNCTION  bridgemgr_common_signal_handler
===========================================================================*/
/*!
@brief
  This is the signal handler function which causes the calling thread to exit

@param
  sig - Signal being delivered

@return
  None

*/
/*=========================================================================*/
void bridgemgr_common_signal_handler
(
  int sig
);


/*===========================================================================
  FUNCTION  bridgemgr_common_issue_reinit_request
===========================================================================*/
/*!
@brief
  This function is used to issue requests to SYS_REINIT module for
  re-initializing other modules

@param
  sys         - Module to re-initialize
  reinit_type - Type of reinit request

@return
  BRIDGEMGR_SUCCESS - If request was successfully issued
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
int bridgemgr_common_issue_reinit_request
(
  bridgemgr_sys_type     sys,
  bridgemgr_reinit_type  reinit_type
);


/*===========================================================================
  FUNCTION  bridgemgr_common_cleanup_cb
===========================================================================*/
/*!
@brief
  This is the common cleanup callback registered by different modules

@param
  cb - pointer to client registered callbacks

@return
  None

*/
/*=========================================================================*/
void bridgemgr_common_cleanup_cb
(
  bridgemgr_sys_type               sys,
  bridgemgr_client_callbacks_type  *cb
);


/*===========================================================================
  FUNCTION  bridgemgr_common_stop_thread
===========================================================================*/
/*!
@brief
  This function is used to stop a given thread

@param
  thread_id - ID of the thread to stop

@return
  BRIDGEMGR_SUCCESS - If thread was successfully stopped
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
int bridgemgr_common_stop_thread
(
  pthread_t  thread_id
);

#endif /* __BRIDGEMGR_COMMON_H__ */

