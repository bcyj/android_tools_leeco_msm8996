/*============================================================================

@file
sns_debug_la.c

@brief
Contains Linux Android specific Debug function implementation's.

Copyright (c) 2010-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary

============================================================================*/

/*============================================================================
  INCLUDE FILES
=============================================================================*/
#include "sns_common.h"
#include "sns_debug_api.h"
#include "sns_log_types.h" /* For bit mask types */
#include "sns_debug.h"
#include "sns_debug_str.h"


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
sns_err_code_e sns_fill_log_mask(sns_log_mask_t* log_mask)
{
  uint16_t qxdm_log_id_arr[] = SNS_LOG_QXDM_ID;
  sns_log_mask_t mask_bit;
  sns_log_id_e i;

  if (log_mask == NULL)
  {
    SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT,
                      "Set_log_mask ERROR. Input pointer not valid");
    return SNS_ERR_BAD_PARM;
  }

  /* Check for number of log mask bits */
  if (SNS_LOG_NUM_IDS > 128)
  {
    SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT,
                      "Set_log_mask ERROR. Number of bits required > 64");
    return SNS_ERR_FAILED;
  }

  log_mask[0] = 0;
  log_mask[1] = 0;

  for (i = 0; i < SNS_LOG_NUM_IDS; i++)
  {
    /* For debugging purposes. Remove later */
    SNS_PRINTF_STRING_MEDIUM_2(DBG_MOD_INIT,
                               "Set_log_mask: %x enabled(1)/disabled(0)= %d",
                               qxdm_log_id_arr[i],
                               log_status(qxdm_log_id_arr[i]));

    if (log_status(qxdm_log_id_arr[i]) == true)
    {
      mask_bit=0x0000000000000001;
      /* Construct the log mask */
      log_mask[i/64] |= (mask_bit << (i%64));
    } // end of if
  } // end of for

  /* invert the bits; 1 means disable logging, 0 means enable logging */
  log_mask[0] = ~log_mask[0];
  log_mask[1] = ~log_mask[1];
  SNS_PRINTF_STRING_MEDIUM_2(DBG_MOD_INIT,
                             "Set_log_mask: log_mask1 %lu, log_mask2 %lu",
                             log_mask[0], log_mask[1]);

  return SNS_SUCCESS;

} // end of function sns_fill_log_mask
