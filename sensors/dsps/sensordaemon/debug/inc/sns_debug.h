#ifndef _SNS_DEBUG_H_
#define _SNS_DEBUG_H_

/*============================================================================
  @file sns_debug.h

  @brief Debug service provides a route for outgoing log messages from the SSC,
  and incoming DIAG commands to the Sensors Daemon.

  <br><br>

  DEPENDENCIES:

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*===========================================================================
                  DEFINES
============================================================================*/

#if defined( SNS_BLAST)
#define SNS_DBG_CONFIG_FILE_NAME "/sns/cfg/sensors_dbg_config.txt"
#else
#define SNS_DBG_CONFIG_FILE_NAME "/persist/sensors/sensors_dbg_config.txt"
#endif

#if defined( SNS_BLAST)
#define DBG_MOD_DIAG    SNS_DBG_MOD_MDM_DIAG
#define DBG_MOD_INIT    SNS_DBG_MOD_MDM_INIT
#else
#define DBG_MOD_DIAG    SNS_DBG_MOD_APPS_DIAG
#define DBG_MOD_INIT    SNS_DBG_MOD_APPS_INIT
#endif

#define DEBUG_STR_MASK_INFO 1
#define LOG_MASK_INFO 2

#define SNS_DIAG_LOGMASK_DELAY_USEC 60000000 /* 60 Seconds */

/* This string header is prepended to all of the DSPS filenames */
#define SNS_DEBUG_DSPS_FILENAME_HDR "DSPS file:"
#ifdef _WIN32
#define SNS_DIAG_DBG_MASK_DEFAULT 0xFFFFFFFFFFFFFFFF //WIN32: disable all debug messages by default
#else
#define SNS_DIAG_DBG_MASK_DEFAULT 0x2 /* All bits are set except DIAG */
#endif /* _WIN32 */
#define SNS_DIAG_RX_SIG        0x00000001

/* Function Declarations */

/*===========================================================================
  FUNCTION:   sns_debug_mr_init
  ===========================================================================*/
/*!
  @brief
  Initialize with message router. Register with debug/diag services.

  @param[i]
  Not used.

  @return
  None.
*/
/*=========================================================================*/
sns_err_code_e sns_debug_mr_init(void);

/*===========================================================================
  FUNCTION:   sns_debug_mr_deinit
  ===========================================================================*/
/*!
  @brief
  Deinitialize with message router.

  @param[i]
  Not used.

  @return
  None.
*/
/*=========================================================================*/
sns_err_code_e sns_debug_mr_deinit(void);

/*===========================================================================
  FUNCTION:   sns_debug_rcv
  ===========================================================================*/
/*!
  @brief
  Get message pointer from receiving queue.

  @param[i]
  Not used.

  @return
  Message pointer from the queue.
*/
/*=========================================================================*/
void * sns_debug_rcv(void);

/*===========================================================================
  FUNCTION:   sns_diag_send_mask_info
  ===========================================================================*/
/*!
  @brief
  Sends mask info to DSPS through SMR

  @param[i] mask_type    : Indicates what mask is being set (log/debugstr)
  @param[i] bit_mask     : Bit mask to be sent to DSPS
  @param[i] bit_mask_ext : Extended Bit mask to be sent to DSPS

  @return
  No return value
*/
/*=========================================================================*/
void sns_diag_send_mask_info(uint8_t mask_type,
                             uint64_t bit_mask,
                             uint64_t bit_mask_ext);

/*===========================================================================
  FUNCTION:   sns_diag_send_debug_options
  ===========================================================================*/
/*!
  @brief
  Sends debug options to DSPS through SMR

  @param[i] options    : Debug options array

  @return
  No return value
*/
/*=========================================================================*/
void sns_diag_send_debug_options(uint64_t *options);

#endif /* _SNS_DEBUG_H_ */
