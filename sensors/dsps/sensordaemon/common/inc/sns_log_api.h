#ifndef SNS_LOG_API_H
#define SNS_LOG_API_H

/*============================================================================

@file 
sns_log_api.h

@brief
Contains function prototypes of loggings API's.

Copyright (c) 2010, 2013 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary

============================================================================*/

/*===========================================================================

			EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //source/qcom/qct/core/sensors/dsps/common/main/latest/inc/sns_log_api.h#1 $

when       who    what, where, why
(mm/dd/yyyy)
--------   ---    ----------------------------------------------------------
11/09/2010  ad     added support for log filtering 
09/17/2010  sj     Usage of <type>_t (avoids including comdef)
8/12/2010   sj     Created
===========================================================================*/

/*=====================================================================
                 INCLUDE FILES
=======================================================================*/
#include "sns_common.h"
#include "sns_log_types.h"
#include "sns_diag_dsps_v01.h"
#include <stddef.h>

/*===========================================================================

  FUNCTION:   sns_logpkt_malloc

===========================================================================*/
/*!
  @brief
  Allocates memory for the log packet. On the Apps processor this function
  would implement a call to the DIAG logging api's.
   
  @param[i] 
  log_pkt_type  : Log Packet type
  pkt_size      : Size
  log_pkt_ptr   : Pointer of the location in which to place the 
                  allocated log packet

  @return
  sns_err_code_e: SNS_SUCCESS if the allocation had no errors.
                  All other values indicate an error has occurred.
*/
/*=========================================================================*/
sns_err_code_e sns_logpkt_malloc(log_pkt_t log_pkt_type,
                                 uint32_t pkt_size,
                                 void** log_pkt_ptr);

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
                                  uint32_t new_pkt_size);

/*===========================================================================

  FUNCTION:   sns_logpkt_commit

===========================================================================*/
/*!
  @brief
  Commits the log packet to DIAG. On the Apps processor this function
  would implement a call to the DIAG logging api's.
   
  @param[i] 
  log_pkt_type: Log Packet type
  log_pkt_ptr : Pointer to the log packet to commit 

  @return
  sns_err_code_e: SNS_SUCCESS if the commit had no errors.
                  All other values indicate an error has occurred.
*/
/*=========================================================================*/
sns_err_code_e sns_logpkt_commit(log_pkt_t log_pkt_type,
                                 void* log_pkt_ptr);

/*===========================================================================

  FUNCTION:   sns_diag_dsps_set_log_mask

===========================================================================*/
/*!
  @brief
  Sets the log mask on DSPS
  
   
  @param[i] 
  msg_ptr: pointer to message containing log mask that indicates which
           log packets are enabled/disabled.

  @return
  none
*/
/*=========================================================================*/
void sns_diag_dsps_set_log_mask(sns_diag_set_log_mask_req_msg_v01* msg_ptr);


/*===========================================================================

  FUNCTION:   sns_fill_log_mask

===========================================================================*/
/*!
  @brief
  Fills the log mask for log packets supported. The information
    on which log packets are enabled in QXDM is obtained from DIAG api.
    Also note that the log mask is used only on the DSPS.
    For apps and modem proc the DIAG api
    to malloc returns NULL if the log packet is not enabled in QXDM.

  @param[i]  log_mask  : Pointer where the log mask needs to be filled in

  @return
  sns_err_code_e: Error code indicating if init was successful.
*/
/*=========================================================================*/
sns_err_code_e sns_fill_log_mask(sns_log_mask_t* log_mask);

#endif /* SNS_LOG_API_H */
