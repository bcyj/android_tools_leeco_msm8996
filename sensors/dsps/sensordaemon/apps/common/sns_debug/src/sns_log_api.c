/*============================================================================
@file
sns_log_api.c

@brief
Contains implementation of Sensors Logging API's on the Apps processor.

Copyright (c) 2010, 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*=====================================================================
                 INCLUDE FILES
=======================================================================*/
#include "sns_log_types.h"
#include "sns_log_api.h"
#include "sns_debug_api.h"
#ifdef SNS_BLAST
#include "sns_debug_str_mdm.h"
#else
#include "sns_debug_str.h"
#endif

#include "log.h"

#ifdef ENABLE_APPS_PLAYBACK
#include "sns_memmgr.h"
#include "sns_playback.h"
#endif

/*===========================================================================
                    GLOBAL VARIABLES
===========================================================================*/
static uint16_t qxdm_log_codes[] = SNS_LOG_QXDM_ID;

/*===========================================================================
                    FUNCTIONS
===========================================================================*/

/*===========================================================================

  FUNCTION:   sns_logpkt_malloc

===========================================================================*/
/*!
  @brief
  Allocates memory for the log packet. On the Apps processor this function
  would implement a call to the DIAG logging api's.
   
  @param[i] log_pkt_type  : Internal (sensors team allocated) Log Packet id
  @param[i] pkt_size      : Size
  @param[i] log_pkt_ptr   : Pointer of the location in which to place the 
                            allocated log packet

  @return
  sns_err_code_e: SNS_SUCCESS if the allocation had no errors.
                  All other values indicate an error has occurred.
*/
/*=========================================================================*/
sns_err_code_e sns_logpkt_malloc(log_pkt_t log_pkt_type,
                                 uint32_t pkt_size,
                                 void** log_pkt_ptr)
{
  /* Input Checks */
  if ( (pkt_size == 0) || (log_pkt_type >= SNS_LOG_NUM_IDS) )
  {
    SNS_PRINTF_STRING_ERROR_1(SNS_DBG_MOD_APPS_DIAG,
                              "Logpkt_Malloc: Size zero or log code (%x)incorrect!",
                              log_pkt_type);
    return SNS_ERR_BAD_PARM;
  }

#ifdef ENABLE_APPS_PLAYBACK
  {
    /* re-use the log message to hold packet plus header info */
    sns_debug_log_ind_msg_v01 *msg_ptr;
    uint16_t hdr_offset = offsetof(sns_debug_log_ind_msg_v01, log_pkt_contents);
    msg_ptr = SNS_OS_MALLOC(SNS_DBG_MOD_APPS_DIAG,hdr_offset + pkt_size);
    if (msg_ptr == NULL)
    {
      return SNS_ERR_NOMEM;
    }
    msg_ptr->log_pkt_type = log_pkt_type;
    msg_ptr->logpkt_size = pkt_size;
    *log_pkt_ptr = (void*)(msg_ptr->log_pkt_contents);
    return SNS_SUCCESS;
  }
#else
  /* Allocate log packet */
  *log_pkt_ptr = log_alloc(qxdm_log_codes[log_pkt_type],pkt_size);

  /* Check to see if log packet allocated */
  if (*log_pkt_ptr == NULL)
  {
    return SNS_ERR_NOMEM;
  }
  else
  {
    return SNS_SUCCESS;
  } 
#endif
} // end of function sns_logpkt_malloc


/*===========================================================================

  FUNCTION:   sns_logpkt_shorten

===========================================================================*/
/*!
  @brief
  Shortens a log packet allocated with sns_logpkt_malloc. This will reduce
  the size of the log when it is sent via the DIAG protocol. The memory
  allocated in sns_logpkt_malloc will still be released when sns_logpkt_commit
  is called.

  This must be called prior to calling sns_logpkt_commit.
   
  @param[i] log_pkt_ptr   : Pointer to the allocated log packet
  @param[i] new_pkt_size  : New size

  @return
  sns_err_code_e: SNS_SUCCESS if the allocation had no errors.
                  All other values indicate an error has occurred.
*/
/*=========================================================================*/
sns_err_code_e sns_logpkt_shorten(void* log_pkt_ptr,
                                  uint32_t new_pkt_size)
                                  
{
  if ( (log_pkt_ptr == NULL) )
  {
    SNS_PRINTF_STRING_ERROR_0(SNS_DBG_MOD_APPS_DIAG,
                              "sns_logpkt_shorten: Input Pointer NULL!");
    return SNS_ERR_BAD_PARM;
  } 

  log_shorten( log_pkt_ptr, new_pkt_size );
  return SNS_SUCCESS;
}
/*===========================================================================

  FUNCTION:   sns_logpkt_commit

===========================================================================*/
/*!
  @brief
  Commits the log packet to DIAG. On the Apps processor this function
  would implement a call to the DIAG logging api's.
   
  @param[i] log_pkt_type: Internal (sensors team allocated) Log Packet id
  @param[i] log_pkt_ptr : Pointer to the log packet to commit 

  @return
  sns_err_code_e: SNS_SUCCESS if the commit had no errors.
                  All other values indicate an error has occurred.
*/
/*=========================================================================*/
sns_err_code_e sns_logpkt_commit(log_pkt_t log_pkt_type,
                                 void* log_pkt_ptr)
{
  UNREFERENCED_PARAMETER( log_pkt_type );

  /* Input Checks */
  if ( (log_pkt_ptr == NULL) )
  {
    SNS_PRINTF_STRING_ERROR_0(SNS_DBG_MOD_APPS_DIAG,
                              "Logpkt_Commit: Input Pointer NULL!");
    return SNS_ERR_BAD_PARM;
  } 
  
#ifdef ENABLE_APPS_PLAYBACK
  {
    sns_debug_log_ind_msg_v01 *msg_ptr;
    uint16_t hdr_offset = offsetof(sns_debug_log_ind_msg_v01, log_pkt_contents);
    msg_ptr = (sns_debug_log_ind_msg_v01*)((char*)log_pkt_ptr - hdr_offset);
    sns_playback_log_pkt(msg_ptr);
    SNS_OS_FREE(msg_ptr);
  }
#else
  /* Send the packet to DIAG */
  log_commit(log_pkt_ptr);
#endif

  return SNS_SUCCESS;

} // end of function sns_logpkt_commit
