/*============================================================================
  @file sns_acm_priv.h

  @brief
    Common and private header for ACM implementation.

  <br><br>

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/

// OS Flags/Signals
#define SNS_ACM_SMR_RX_FLAG    0x0001
#define SNS_ACM_WRITABLE_FLAG  0x0002

#define SNS_MOD SNS_MODULE_APPS_ACM

#if defined( SNS_BLAST)
#define SNS_DBG_MOD_ACM           SNS_DBG_MOD_MDM_ACM
#define SNS_MODULE_ACM            SNS_MODULE_MDM_ACM
#define SNS_MODULE_PRI_ACM        SNS_MODULE_PRI_MDM_ACM
#define SNS_MODULE_PRI_ACM_MUTEX  SNS_MODULE_PRI_MDM_ACM_MUTEX
#else
#define SNS_DBG_MOD_ACM           SNS_DBG_MOD_APPS_ACM
#define SNS_MODULE_ACM            SNS_MODULE_APPS_ACM
#define SNS_MODULE_PRI_ACM        SNS_MODULE_PRI_APPS_ACM
#define SNS_MODULE_PRI_ACM_MUTEX  SNS_MODULE_PRI_APPS_ACM_MUTEX
#endif

/**
 * OS flag group for the ACM task.
 */
OS_FLAG_GRP *sns_acm_flag_grp;

/*============================================================================
  Function Declarations
  ============================================================================*/
sns_err_code_e sns_acm_mr_init();
sns_err_code_e sns_acm_mr_deinit();
sns_err_code_e sns_acm_mr_send( void* );
void sns_acm_mr_handle();
void sns_acm_mr_close( uint32_t );
void sns_acm_handle_rx( void* );
sns_err_code_e sns_acm_mr_cancel( uint8_t ext_clnt_id, uint8_t svc_num );
int sns_acm_mr_queue_clean( uint8_t ext_clnt_id );
