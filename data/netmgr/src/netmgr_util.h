
/******************************************************************************

                        N E T M G R _ U T I L . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_util.h
  @brief   Network Manager Utility Functions Header File

  DESCRIPTION
  Header file for NetMgr utility functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc. All Rights Reserved

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
02/10/10   ar         Initial version

******************************************************************************/

#ifndef __NETMGR_UTIL_H__
#define __NETMGR_UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/select.h>
#ifndef FEATURE_DS_LINUX_ANDROID
#include <string.h>
#endif
#ifdef FEATURE_DATA_LOG_ADB
#include <android/log.h>
#include <utils/Log.h>
#include "common_log.h"
#endif
#include "comdef.h"
#include "ds_util.h"
#include "netmgr_defs.h"
#include "netmgr.h"
#include "qmi_wds_srvc.h"

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

/* Macros for locking and unlocking a mutex */
#define NETMGR_LOCK_MUTEX(mutex)                                   \
  do                                                               \
  {                                                                \
    if (0 == pthread_mutex_lock(&mutex))                           \
    {                                                              \
      if (function_debug) {                                        \
        netmgr_log_low(">>>>>> LOCK MUTEX %p SUCCESS", &mutex);    \
      }                                                            \
    }                                                              \
    else                                                           \
    {                                                              \
      netmgr_log_err(">>>>>> LOCK MUTEX %p FAILURE", &mutex);      \
    }                                                              \
  }                                                                \
  while (0)

#define NETMGR_UNLOCK_MUTEX(mutex)                                 \
  do                                                               \
  {                                                                \
    if (0 == pthread_mutex_unlock(&mutex))                         \
    {                                                              \
      if (function_debug) {                                        \
        netmgr_log_low("<<<<<< UNLOCK MUTEX %p SUCCESS", &mutex);  \
      }                                                            \
    }                                                              \
    else                                                           \
    {                                                              \
      netmgr_log_err("<<<<<< UNLOCK MUTEX %p FAILURE", &mutex);    \
    }                                                              \
  }                                                                \
  while (0)

#define NETMGR_UTIL_MAX_CIRC_LIST_SIZE (10)

typedef struct
{
  void *data[NETMGR_UTIL_MAX_CIRC_LIST_SIZE+1];
  int r;
  int w;
} netmgr_util_circ_list_type;

#ifdef NETMGR_TEST
/* Suppress 'static' keyword in offtarget builds so testcases can
 * invoke internal functions. */
#define LOCAL
#define netmgr_malloc(size)  netmgr_debug_malloc(size)
#define netmgr_free(ptr)     netmgr_debug_free(ptr)
#define LOG(...)
#else
#ifdef LOCAL
#undef LOCAL
#define LOCAL static
#endif
#define netmgr_malloc(size)  malloc(size)
#define netmgr_free(ptr)     if( ptr ){ free(ptr); }
#endif /* NETMGR_TEST */

/*---------------------------------------------------------------------------
   Definition of 'assert' macro. This is needed as ONCRPC/DSM hijacks path
   to include another file with name assert.h, so that the standard library
   assert is not available to ONCRPC clients.
---------------------------------------------------------------------------*/
#define NETMGR_ASSERT(a)   ds_assert(a)

/*---------------------------------------------------------------------------
  Definition of 'abort' macro.  Diagnostic messages are typically sent
  to QXDM but there is no guarantee message will be displayed.  Need
  to ensure abort related message is sent to persistent Android log.
---------------------------------------------------------------------------*/
#ifdef FEATURE_DATA_LOG_ADB
#define NETMGR_ABORT(...)                            \
     LOG(LOG_ERROR, "QC-NETMGR", __VA_ARGS__);       \
     fprintf(stderr, __VA_ARGS__);                   \
     exit(-1);
#else
#define NETMGR_ABORT(...)                            \
     fprintf(stderr, __VA_ARGS__);                   \
     exit(-1);
#endif
/*---------------------------------------------------------------------------
  Definition of 'stop' macro.  Diagnostic messages are typically sent
  to QXDM but there is no guarantee message will be displayed.  Need
  to ensure abort related message is sent to persistent Android log.
---------------------------------------------------------------------------*/
#ifdef FEATURE_DATA_LOG_ADB
#define NETMGR_STOP(...)                             \
     LOG(LOG_ERROR, "QC-NETMGR", __VA_ARGS__);       \
     fprintf(stderr, __VA_ARGS__);                   \
     while(1) { sleep(0xFFFFFFFF); }
#else
#define NETMGR_STOP(...)                             \
     fprintf(stderr, __VA_ARGS__);                   \
     while(1) { sleep(0xFFFFFFFF); }
#endif

/*---------------------------------------------------------------------------
   Logging macros
---------------------------------------------------------------------------*/
#undef  DS_LOG_TAG
#define DS_LOG_TAG "QC-NETMGR-LIB"

#define  netmgr_log               ds_log
#define  netmgr_log_err           ds_log_err
#define  netmgr_log_high          ds_log_high
#define  netmgr_log_med           ds_log_med
#define  netmgr_log_low           ds_log_low
#define  netmgr_log_dflt          ds_log_dflt
#define  netmgr_log_sys_err       ds_log_sys_err

#define  netmgr_log_init          ds_log_init2

#define netmgr_l2s_max_msg_size 256
#define netmgr_l2s_invalid_link -1

#define netmgr_l2s_entry_params(fmt, ...)                                    \
  netmgr_log_med("L2S[pid:%d tid:%d type:%s fname:%s params:" fmt "]",       \
                 getpid(),                                                   \
                 gettid(),                                                   \
                 "FE",                                                       \
                 __func__,                                                   \
                 ##__VA_ARGS__);

#define netmgr_l2s_entry()   netmgr_l2s_entry_params("")

#define netmgr_l2s_exit_with_status(fmt,...)                                 \
  netmgr_log_med("L2S[pid:%d tid:%d type:%s fname:%s status:%s params:"fmt"]",\
                getpid(),                                                    \
                gettid(),                                                    \
                "FX",                                                        \
                __func__,                                                    \
                (ret==NETMGR_ERROR)?"ERROR":"SUCCESS",                       \
                ##__VA_ARGS__);

#define netmgr_l2s_exit()                                                    \
  netmgr_log_med("L2S[pid:%d tid:%d type:%s fname:%s]",                      \
                getpid(),                                                    \
                gettid(),                                                    \
                "FX",                                                        \
                __func__);


#define netmgr_l2s_msg(iface_id, msg)                                        \
  netmgr_log_med("L2S[pid:%d tid:%d type:%s fname:%s iface_id:%d msg:\"%s\"]",\
                  getpid(),                                                  \
                  gettid(),                                                  \
                  "ME",                                                      \
                  __func__,                                                  \
                  iface_id,                                                  \
                  msg);

#define netmgr_l2s_msg_seq(...)                                              \
  netmgr_log_med("L2S[pid:%d tid:%d type:%s fname:%s iface_id:%d msg:\"%s\"]",\
                getpid(),                                                    \
                gettid(),                                                    \
                "MS",                                                        \
                __func__,                                                    \
                iface_id,                                                    \
                msg);

#define netmgr_l2s_format_msg(iface_id,msg,...)                              \
  do{                                                                        \
    char l2s_msg[netmgr_l2s_max_msg_size] = "\0" ;                           \
    memset(l2s_msg,0,netmgr_l2s_max_msg_size);                               \
    snprintf(l2s_msg,netmgr_l2s_max_msg_size,msg, __VA_ARGS__);              \
    netmgr_l2s_msg(iface_id,l2s_msg);                                        \
  }while (0);


extern boolean function_debug;
#define  NETMGR_LOG_FUNC_ENTRY    if(function_debug){ ds_log_func_entry(); }
#define  NETMGR_LOG_FUNC_EXIT     if(function_debug){ ds_log_func_exit();  }

#define NETMGR_LOG_IPV4_ADDR(level, prefix, ip_addr)                        \
        DS_INET4_NTOP(level, prefix, ((unsigned char *)&ip_addr))

#define NETMGR_LOG_IPV6_ADDR(level, prefix, ip_addr)                        \
        DS_INET6_NTOP(level, prefix, ((unsigned char *)&ip_addr))

#if (!defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID))

#define NETMGR_NL_REPORT_ADDR( level, prefix, addr )         \
        if( AF_INET6 == (addr).ss_family ) {                 \
          NETMGR_LOG_IPV6_ADDR( level, prefix, addr.__data); \
        } else {                                             \
          NETMGR_LOG_IPV4_ADDR( level, prefix, addr.__data); \
        }

#else/*(!defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID))*/

#define NETMGR_NL_REPORT_ADDR( level, prefix, addr )               \
        if( AF_INET6 == (addr).ss_family ) {                       \
          NETMGR_LOG_IPV6_ADDR( level, prefix, addr.__ss_padding); \
        } else {                                                   \
          NETMGR_LOG_IPV4_ADDR( level, prefix, addr.__ss_padding); \
        }

#endif/*(!defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID))*/


/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

#ifdef NETMGR_TEST
/*===========================================================================
  FUNCTION  netmgr_debug_malloc
===========================================================================*/
/*!
@brief
  Debug wrapper for malloc

@return
  void* - Pointer to heap memeory allocation

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void * netmgr_debug_malloc( size_t memsize );

/*===========================================================================
  FUNCTION  netmgr_debug_free
===========================================================================*/
/*!
@brief
  Debug wrapper for free

@return
  None

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void netmgr_debug_free( void* ptr );
#endif /* NETMGR_TEST */

/*===========================================================================
  FUNCTION  netmgr_daemonize
===========================================================================*/
/*!
@brief
 Performs typical tasks required to run a program as a daemon process.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Original program will exit and a child is forked which will continue
      execution as the daemon.
*/
/*=========================================================================*/
void netmgr_daemonize (void);


#ifdef FEATURE_DATA_LOG_QXDM
/*=========================================================================
  FUNCTION:  netmgr_format_log_msg

===========================================================================*/
/*!
    @brief
    Format debug message for logging.

    @return
    None
*/
/*=========================================================================*/
void netmgr_format_log_msg
(
  char *buf_ptr,
  int buf_size,
  char *fmt,
  ...
);
#endif

#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_util_convert_ip_addr_to_str
===========================================================================*/
/*!
@brief
  Converts the given Netmgr IP address type to a string

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_util_convert_ip_addr_to_str
(
  netmgr_ip_address_t  *ip,
  unsigned int         prefix_len,
  char                 *buf,
  unsigned int         buf_len
);

/*===========================================================================
  FUNCTION  netmgr_util_convert_qmi_ip_addr_to_str
===========================================================================*/
/*!
@brief
  Converts the given QMI IP address type to a string

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_util_convert_qmi_ip_addr_to_str
(
  qmi_wds_ip_addr_type *ip,
  char                 *buf,
  unsigned int         buf_len
);

/*===========================================================================
  FUNCTION  netmgr_util_convert_qmi_ipsec_key_to_str
===========================================================================*/
/*!
@brief
  Converts the given IPSec hash/crypto key to a string

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_util_convert_qmi_ipsec_key_to_str
(
  qmi_wds_ipsec_key_type  *key,
  char                    *buf,
  unsigned int            buf_len
);

/*===========================================================================
  FUNCTION  netmgr_util_get_ipsec_proto_str
===========================================================================*/
/*!
@brief
  Returns the string representation of the given IPSec protocol

@return
  Pointer to the IPSec proto string on success or NULL on error

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
const char *netmgr_util_get_ipsec_proto_str
(
  qmi_wds_ipsec_sa_proto_type  proto
);

/*===========================================================================
  FUNCTION  netmgr_util_get_ipsec_algo_str
===========================================================================*/
/*!
@brief
  Returns the string representation of the given IPSec algorithm

@return
  Pointer to the IPSec algo string on success or NULL on error

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
const char *netmgr_util_get_ipsec_algo_str
(
  netmgr_ipsec_algo_type        type,
  qmi_wds_ipsec_sa_config_type  *sa_config
);

/*===========================================================================
  FUNCTION  netmgr_util_get_ipsec_mode_str
===========================================================================*/
/*!
@brief
  Returns the string representation of the given IPSec encapsulation mode

@return
  Pointer to the IPSec mode string on success or NULL on error

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
const char *netmgr_util_get_ipsec_mode_str
(
  qmi_wds_ipsec_encapsulation_mode  mode
);

/*===========================================================================
  FUNCTION  netmgr_util_circ_list_init
===========================================================================*/
/*!
@brief
  Initializes the given circular list

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_util_circ_list_init
(
  netmgr_util_circ_list_type  *clist
);

/*===========================================================================
  FUNCTION  netmgr_util_circ_list_destroy
===========================================================================*/
/*!
@brief
  Frees any remaining data on the list before initializing it

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_util_circ_list_destroy
(
  netmgr_util_circ_list_type  *clist
);

/*===========================================================================
  FUNCTION  netmgr_util_enq_circ_list
===========================================================================*/
/*!
@brief
  Enqueues data into the circular list

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_util_enq_circ_list
(
  netmgr_util_circ_list_type  *clist,
  void                        *data
);

/*===========================================================================
  FUNCTION  netmgr_util_deq_circ_list
===========================================================================*/
/*!
@brief
  Dequeues data from the circular list and returns it via the out pointer

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_util_deq_circ_list
(
  netmgr_util_circ_list_type  *clist,
  void                        **data
);
#endif /* FEATURE_DATA_IWLAN */

#endif /* __NETMGR_UTIL_H__ */
