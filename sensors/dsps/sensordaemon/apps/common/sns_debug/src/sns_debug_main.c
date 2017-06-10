/*============================================================================
@file
sns_debug_main.c

@brief
Contains main implementation of the Debug thread (the entry point for
the debug module on Apps processor).

Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary
============================================================================*/

/*============================================================================
  INCLUDE FILES
=============================================================================*/
#include "sns_common.h"
#ifdef SNS_BLAST
#include "sns_debug_str_mdm.h"
#else
#include "sns_debug_str.h"
#endif
#include "sns_diagproxy.h"
#include "sns_log_api.h"
#include "sns_debug_interface_v01.h"
#include "sns_diag_dsps_v01.h"
#include "sns_file_internal_v01.h"
#include "sns_init.h"
#include "sns_memmgr.h"
#include "sns_osa.h"
#include "sns_queue.h"
#include "sns_smr_util.h"
#include "sns_debug_api.h"
#include "sns_log_types.h" /* For bit mask types */
#include "sns_em.h"
#include "sns_debug.h"

#include "msg.h"
#include "msgcfg.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/*============================================================================
  MACRO DEFINES
  ==============================================================================*/

/*============================================================================
  GLOBAL DEFINES
  ============================================================================*/
/* OS Flag group for Diag Task */
OS_FLAG_GRP *sns_diag_flag_grp = (OS_FLAG_GRP*)NULL;

/* Global variable to track if sns_debug module has been initializied */
static boolean sns_debug_test_initialized = false;

uint64_t debug_options_arr[SNS_DIAG_MAX_OPTIONS_V01];

#if !defined( SNS_BLAST )
/* Timer object, used one time for delaying getting of log masks from
 * DIAG. This delay (~10 seconds) is required after DIAG_LSM_Init is called
 * We need the delay so that DIAG kernel code can get all the log
 * masks from QXDM.
 */
static sns_em_timer_obj_t sns_diag_logmask_delay_tmr_ptr = (sns_em_timer_obj_t)NULL;
#endif

/*===========================================================================
  FUNCTIONS
  ============================================================================*/
#if defined( SNS_BLAST )
static void sns_debug_init_mdm_dbg( void );
static sns_err_code_e sns_fill_debug_str_mask_mdm(uint64_t* debug_str_mask);
#else
static void sns_debug_timer_cb(void* cb_data);
static sns_err_code_e sns_read_debug_input(sns_debug_mask_t* debug_str_mask,
                                           uint64_t *debug_options);
#endif
void sns_diag_handle_rx_indication( void );

#if defined( SNS_BLAST )

/*===========================================================================

  FUNCTION:   sns_fill_debug_str_mask_mdm

===========================================================================*/
/*!
  @brief
  Fills the debug string mask for modules printing debug strings.
  Every module has one bit in the mask. That bit specifies if the debug
  strings from the module are enabled or disabled. We are using a 64 bit
  bit mask for debug strings for modules on DSPS, Apps and Modem. So
  there can be a total of only 64 modules across all the three processors
  outputting debug strings.

  @param[i] debug_str_mask  : Pointer where the debug string mask needs
                              to be filled in.

  @return
  sns_err_code_e: Error code indicating if init was successful.
*/
/*=========================================================================*/
static sns_err_code_e sns_fill_debug_str_mask_mdm(sns_debug_mask_t* debug_str_mask)
{
  /* Local arrays to help construct the bit mask
   * If modules are added then the arrays below need to be edited to
   * add support for the new modules
   */

  const char *module_name_arr[] = { "MdmSMR",
                                    "MdmDiag",
                                    "MdmACM",
                                    "MdmInit",
                                    "MdmMain",
                                    "MdmEM"
                                   };
  uint16_t module_mask_arr[SNS_DBG_NUM_MOD_IDS];
  FILE *dbg_config_file_ptr;
  char line[100],*token_str_ptr;
  sns_debug_module_id_e i;
  sns_debug_mask_t bit_mask = 1;

  const char *mdm_id = "Mdm";
  uint16_t mdm_id_len = (uint16_t)strlen(mdm_id);


  /* Input checks */
  if (debug_str_mask == NULL)
  {
    SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT,
                      "Set_debug_str_mask ERROR. Input pointer not valid");
    return SNS_ERR_BAD_PARM;
  }

  /* Open and read the mask from file */
  if ((dbg_config_file_ptr = fopen(SNS_DBG_CONFIG_FILE_NAME,"r")) == NULL)
  {
    SNS_PRINTF_STRING_MEDIUM_0(DBG_MOD_INIT,
                              "Debug Config File missing in EFS!");
    return SNS_ERR_FAILED;
  }
  /* Read a line and parse */
  while ( fgets(line,sizeof(line),dbg_config_file_ptr ) != NULL )
  {
    /* Comment lines start with #, If comment skip line */
    if (line[0] != '#')
    {
      /* The lines in the file will look like this:
       * SMGR=0. So its fine if the token is =
       */
      char *last_tok;
      token_str_ptr = strtok_r(line,"=", &last_tok);

      /* If modem, also skip dsps modules, they will be taken care in Apps */
      if (strncmp(token_str_ptr, mdm_id, mdm_id_len) != 0)
      {
        continue;
      }

      if (token_str_ptr != NULL)
      {
        /* First token is the module name, Match the module name */
        for (i=0; i < SNS_DBG_NUM_MOD_IDS; i++)
        {
          if (strncmp(token_str_ptr,
                      module_name_arr[i],
                      strlen(module_name_arr[i])) == 0)
          {
            break;
          }
        }

        /* Second token is the value disabled or enabled */
        token_str_ptr = strtok_r(NULL,"=", &last_tok);
        if ( (token_str_ptr != NULL) &&
             ( i < SNS_DBG_NUM_MOD_IDS) )
        {
          module_mask_arr[i] = strtol(token_str_ptr,NULL,10);
        }
      }
    } // end of outer if
  } // end of while

  /* Construct the bit mask from the array */
  *debug_str_mask = 0;
  for(i=0; i < SNS_DBG_NUM_MOD_IDS; i++)
  {
    /* Note: In the file 1 means enabled and 0 means disabled
     * This is easier for users to enter in a file
     * In the Mask however 1 means Disabled and 0 means enabled.
     * The meaning is reversed in the mask
     */
    if (module_mask_arr[i] == 0)
    {
      *debug_str_mask = (*debug_str_mask) | bit_mask;
    }
    bit_mask = bit_mask << 1;
  } // end of for

  SNS_PRINTF_STRING_MEDIUM_1(DBG_MOD_INIT,
                             "Debug Str BitMask=%lu",
                 (*debug_str_mask));

#if 0
  /* Does not work on target...Hangs need to be debugged */
  fclose(dbg_config_file_ptr);
#endif

  return SNS_SUCCESS;
}

/*===========================================================================

  FUNCTION:   sns_debug_init_mdm_dbg

  ===========================================================================*/
/*!
  @brief
  Entry point to the sns_debug module. Registers and initializes with DIAG.
  This would help to see debug messages during sensors initialization in
  QXDM.

  @param[i]
  No input parameters.

  @return
  sns_err_code_e: Error code indicating if init was successful.
*/
/*=========================================================================*/
static void sns_debug_init_mdm_dbg( void )
{
  sns_debug_mask_t debug_str_bit_mask;

  /* Get mask info */
  if (sns_fill_debug_str_mask_mdm(&debug_str_bit_mask) == SNS_SUCCESS)
  {
    /* Set masks on the modem processor */
    sns_debug_set_dbg_str_mask(debug_str_bit_mask);

    /* Mask on DSPS is controlled by Apps processor only */
  }

  sns_init_done();
}

#endif /* SNS_BLAST */

/*===========================================================================

  FUNCTION:   sns_diag_thread_main

  ===========================================================================*/
/*!
  @brief
  Main thread loop for the sns_diag module

  @param[i]
  Not used.

  @return
  None.
*/
/*=========================================================================*/
static void sns_diag_thread_main(void *p_arg)
{
  uint8_t err;
  sns_debug_mask_t debug_str_bit_mask;
  OS_FLAGS flags;

  UNREFERENCED_PARAMETER(p_arg);

  SNS_PRINTF_STRING_MEDIUM_0( DBG_MOD_INIT,
                              "SNS DEBUG thread started" );

  /* Get mask info */
  sns_read_debug_input(&debug_str_bit_mask, debug_options_arr);

  sns_init_done();

  for( ; ; )
  {
    flags = sns_os_sigs_pend(sns_diag_flag_grp,
                             SNS_DIAG_RX_SIG,
                             OS_FLAG_WAIT_SET_ANY | OS_FLAG_CONSUME,
                             0,
                             &err );

    if( err != 0 )
    {
      SNS_PRINTF_STRING_ERROR_1(DBG_MOD_DIAG,
                                "Diag Thread: Error in sigs_pend %u",
                                (uint32_t)err );

    }

    if( false == sns_debug_test_initialized )
    {
      break;
    }

    /* Received something */
    if( flags & SNS_DIAG_RX_SIG )
    {
      sns_diag_handle_rx_indication();
    }
  } /* end of for loop */

  /* De Init Diag */
  sns_diag_deinit();
  sns_os_task_del( SNS_MODULE_PRI_APPS_DIAG );
} // end of function sns_diag_thread_main

/*===========================================================================

  FUNCTION:   sns_debug_timer_cb

===========================================================================*/
/*!
  @brief
  When timer expires we call the funciton to get all the log mask values
  using log_status DIAG api. We sent the log mask to the DSPS.

  @param[i] cb_data  :  Data used when registering callbacks.
                        Notused currently
  @return
  No return value
*/
/*=========================================================================*/
static void sns_debug_timer_cb(void* cb_data)
{
  sns_log_mask_t logging_mask[2];

  UNREFERENCED_PARAMETER(cb_data);
  SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT,
                    "Debug Timer Callback called");

  if (sns_fill_log_mask(logging_mask) == SNS_SUCCESS)
  {
    /* Note: There is no need to check on the log mask on apps processor
     * because malloc through DIAG does that for us for free!
     */
    sns_diag_send_mask_info(LOG_MASK_INFO, logging_mask[0], logging_mask[1]);
  }

  /* Send the debug options */
  SNS_PRINTF_STRING_HIGH_0(DBG_MOD_INIT,
                    "Sending debug options called");
  sns_diag_send_debug_options(debug_options_arr);

  /* Delete timer as well */
  sns_em_delete_timer_obj(sns_diag_logmask_delay_tmr_ptr);
}

/*===========================================================================

  FUNCTION:   sns_fill_debug_str_mask

===========================================================================*/
/*!
  @brief
  Reads the debug input and creates two inputs
  1) Debug options - an array of options, value pairs
    Debug options are interpreted by the recipient only. We can currently
    send 16 debug options in  one message.

  and

  2) a debug string mask
  Fills the debug string mask for modules printing debug strings.
  Every module has one bit in the mask. That bit specifies if the debug
  strings from the module are enabled or disabled. We are using a 64 bit
  bit mask for debug strings for modules on DSPS, Apps and Modem. So
  there can be a total of only 64 modules across all the three processors
  outputting debug strings.

  @param[i] debug_str_mask  : Pointer where the debug string mask needs
                              to be filled in.

  @return
  sns_err_code_e: Error code indicating if init was successful.
*/
/*=========================================================================*/
static sns_err_code_e sns_read_debug_input(sns_debug_mask_t* debug_str_mask,
                                           uint64_t *debug_options
                                           )
{
#if defined(SNS_LA) || defined(SNS_LA_SIM)
  /* Local arrays to help construct the bit mask
   * If modules are added then the arrays below need to be edited to
   * add support for the new modules
   */
  static const char *module_name_arr[] = {
                                    "AppsSMR",
                                    "AppsDiag",
                                    "AppsACM",
                                    "AppsInit",
                                    "DSPSSMGR",
                                    "DSPSSAM",
                                    "DSPSSMR",
                                    "DDAccel",
                                    "DDGyro",
                                    "DDAlsPrx",
                                    "DDMag8975",
                                    "AppsMain",
                                    "EM",
                                    "AppsPWR",
                                    "AppsSAM",
                                    "DSPSSCM",
                                    "AppsSCM",
                                    "MDMSMR",
                                    "MDMDiag",
                                    "MDMACM",
                                    "MDMInit",
                                    "MDMMain",
                                    "MDMPwr",
                                    "DSPSDAL",
                                    "DSPSDDF",
                                    "AppsReg",
                                    "AppsTime",
                                    "DSPSDiag",
                                    "DSPSPWR",
                                    "AppsFile",
                                    "AppsFSA",
                                    "SAM"
                                  };

  static const char *options_name_arr[] = {
                                            "opt_uimage_vote",
                                            "opt_ext_smr_ind",
                                            "opt_generic_01",
                                            "opt_generic_02",
                                            "opt_generic_03",
                                            "opt_generic_04",
                                          };

  uint16_t module_mask_arr[SNS_DBG_NUM_MOD_IDS];
  FILE *dbg_config_file_ptr;
  char line[100],*token_str_ptr, *val_str_ptr;
  sns_debug_module_id_e i;
  sns_debug_mask_t bit_mask = 1;
  uint16_t j;
  uint64_t option_val;
  uint8_t ignore_module_names = 0;


  /* Input checks */
  if (debug_str_mask == NULL)
  {
    SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT,
                      "Set_debug_str_mask ERROR. Input pointer not valid");
    return SNS_ERR_BAD_PARM;
  }

  /* Open and read the mask from file */
  if ((dbg_config_file_ptr = fopen(SNS_DBG_CONFIG_FILE_NAME,"r")) == NULL)
  {
    SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT,
                              "Debug Config File missing in EFS!");
    return SNS_ERR_FAILED;
  }

  if (SNS_DBG_NUM_MOD_IDS !=
      sizeof(module_name_arr) / sizeof(*module_name_arr))
  {
    SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT,
                    "Length of module IDs not same as module names, Ignoring");

    ignore_module_names = 1;
  }

  /* Initialize all modules in the mask array to disabled. */
  for ( j = 0; j < SNS_DBG_NUM_MOD_IDS && !ignore_module_names; j++ )
  {
    module_mask_arr[j] = 1;
  }

  for (j = 0; j < SNS_DIAG_MAX_OPTIONS_V01; j++)
  {
    debug_options[j] = 0;
  }

  /* Read a line and parse */
  while ( fgets(line,sizeof(line),dbg_config_file_ptr ) != NULL )
  {
    /* Comment lines start with #, If comment skip line */
    if (line[0] != '#')
    {
      /* The lines in the file will look like this:
       * SMGR=0. So its fine if the token is =
       */
      char *last_tok;
      token_str_ptr = strtok_r(line,"=", &last_tok);
      val_str_ptr = strtok_r(NULL,"=", &last_tok);
      if (val_str_ptr)
      {
        option_val = strtol(val_str_ptr,NULL,10);
        SNS_PRINTF_STRING_HIGH_1(DBG_MOD_INIT, "Setting option val to %d", option_val);
      }
      else
      {
        SNS_PRINTF_STRING_HIGH_0(DBG_MOD_INIT, "defaulting option val to 0 ");
        option_val = 0; // Default
      }

      if (token_str_ptr != NULL)
      {
        /* Lets check if this is a debug option */
        for (i=0; i < sizeof(options_name_arr)/sizeof(options_name_arr[0]); i++)
        {
          if (strncmp(token_str_ptr,
                      options_name_arr[i],
                      strlen(options_name_arr[i])) == 0)
          {
            option_val = 1; // Bug in routing to read option
            SNS_PRINTF_STRING_HIGH_2(DBG_MOD_INIT,
                            "Setting option %d to %d", i, option_val);
            debug_options[i] = option_val;
            break;
          }
        }

        if (i == sizeof(options_name_arr)/sizeof(options_name_arr[0]) && !ignore_module_names)
        {
          /* Match the module name */
          for (i=0; i < SNS_DBG_NUM_MOD_IDS; i++)
          {
            if (strncmp(token_str_ptr,
                        module_name_arr[i],
                        strlen(module_name_arr[i])) == 0)
            {
              module_mask_arr[i] = option_val;
              break;
            }
          } // End loop over module names
        } // End should check modules
      } // End valid token ptr
    } // end of outer if
  } // end of while

  /* Construct the bit mask from the array */
  *debug_str_mask = 0;
  for(i=0; i < SNS_DBG_NUM_MOD_IDS; i++)
  {
    /* Note: In the file 1 means enabled and 0 means disabled
     * This is easier for users to enter in a file
     * In the Mask however 1 means Disabled and 0 means enabled.
     * The meaning is reversed in the mask
     */
    if (module_mask_arr[i] == 0)
    {
      *debug_str_mask = (*debug_str_mask) | bit_mask;
    }
    bit_mask = bit_mask << 1;
  } // end of for

  SNS_PRINTF_STRING_MEDIUM_1(DBG_MOD_INIT,
                             "Debug Str BitMask=%lu",
                             (*debug_str_mask));

  if (ignore_module_names)
  {
    return SNS_ERR_FAILED;
  }

#if 0
  /* Does not work on target...Hangs need to be debugged */
  fclose(dbg_config_file_ptr);
#endif
#elif defined(_WIN32) /* defined(SNS_LA_SIM) || defined(SNS_LA) */
  /* Input checks */
  if (debug_str_mask == NULL)
  {
    SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT,
                              "Set_debug_str_mask ERROR. Input pointer not valid");
    return SNS_ERR_BAD_PARM;
  }

  *debug_str_mask = SNS_DIAG_DBG_MASK_DEFAULT;

  SNS_PRINTF_STRING_MEDIUM_1(DBG_MOD_INIT,
                             "Debug Str BitMask=%lu",
                             (int32_t)(*debug_str_mask));
#endif /* defined(_WIN32) */
  return SNS_SUCCESS;
}

/*===========================================================================

  FUNCTION:   sns_diag_handle_rx_indication

  ===========================================================================*/
/*!
  @brief
  Handles the received messages received. These are messages from the
  DSPS modules to
  - log a packet
  - send F3 messages

  @param[i]
  No input parameters.

  @return
  No return value
*/
/*=========================================================================*/
void sns_diag_handle_rx_indication( void )
{
  void *msg_ptr;
  static sns_debug_string_id_ind_msg_v01* print_msg_ptr;
  static debug_params_s format_params_var;
  static sns_debug_log_ind_msg_v01 *log_qmi_msg_ptr;
  sns_smr_header_s smr_header;
  bool int_err_occurred = false;

  for( ; ; )
  {
    /* Only Apps diag will receive a message on its queue since app takes care
     * of logging all the f3/log packets from dsps through diag, modem won't receive
     * anything as of now.
     */
    msg_ptr = sns_debug_rcv();
    if(msg_ptr == NULL)
    {
      return;
    }

    sns_smr_get_hdr(&smr_header,msg_ptr );

    SNS_PRINTF_STRING_MEDIUM_2(DBG_MOD_DIAG,
                               "SNS Diag Rcv thread: msg_id=%d,body_len=%d",
                               smr_header.msg_id,
                               smr_header.body_len);

    /* Check if the message matches the service number we expect */
    if (SNS_DEBUG_SVC_ID_V01 == smr_header.svc_num)
    {
      if ( SNS_DEBUG_STRING_ID_IND_V01 == smr_header.msg_id )
      {
        static char filename[ SNS_DEBUG_MAX_FILENAME_SIZE_V01
                              + sizeof(SNS_DEBUG_DSPS_FILENAME_HDR) ]
          = SNS_DEBUG_DSPS_FILENAME_HDR;

        print_msg_ptr = (sns_debug_string_id_ind_msg_v01*) msg_ptr;
        SNS_OS_MEMCOPY(filename + sizeof(SNS_DEBUG_DSPS_FILENAME_HDR) - 1,
                      print_msg_ptr->file_name,
                      print_msg_ptr->file_name_len);
        format_params_var.filename = filename;
        format_params_var.line_num = (uint16_t)print_msg_ptr->line_number;
        format_params_var.num_params_valid = (uint8_t)print_msg_ptr->param_values_len;
        format_params_var.param1 = (intptr_t)print_msg_ptr->param_values[0];
        format_params_var.param2 = (intptr_t)print_msg_ptr->param_values[1];
        format_params_var.param3 = (intptr_t)print_msg_ptr->param_values[2];

        sns_debug_printf_string_id(print_msg_ptr->module_id,
                                  print_msg_ptr->str_priority,
                                  print_msg_ptr->string_identifier,
                                  (const debug_params_s*)(&format_params_var)
                                  );

      }
      else if ( SNS_DEBUG_LOG_IND_V01 == smr_header.msg_id )
      {
        void *log_ptr;
        sns_err_code_e fn_ret_err_code;

        log_qmi_msg_ptr = (sns_debug_log_ind_msg_v01*) msg_ptr;

        /* Allocate memory for log packet */
        fn_ret_err_code = sns_logpkt_malloc(log_qmi_msg_ptr->log_pkt_type,
                                            log_qmi_msg_ptr->logpkt_size,
                                            &log_ptr);

        if ( (fn_ret_err_code == SNS_SUCCESS) && (log_ptr != NULL) )
        {
          /* Copy log packet */
          SNS_OS_MEMCOPY((((char*)log_ptr)+sizeof(log_hdr_type)),
                        (void*)((char*)log_qmi_msg_ptr->log_pkt_contents
                                +sizeof(log_hdr_type)),
                        (log_qmi_msg_ptr->logpkt_size-sizeof(log_hdr_type)));

          /* Commit log packet */
          sns_logpkt_commit(log_qmi_msg_ptr->log_pkt_type,
                            log_ptr);
        }
      }
      else
      {
        sns_smr_msg_free( msg_ptr );
        SNS_PRINTF_STRING_HIGH_2(DBG_MOD_DIAG,
                                 "SNS Diag Rcv: unknown msg id %i %i",
                                 smr_header.svc_num,
                                 smr_header.msg_id);
        continue;
      }
    }
    else
    {
        sns_smr_msg_free( msg_ptr );
        SNS_PRINTF_STRING_HIGH_1(DBG_MOD_DIAG,
                                 "SNS Diag Rcv: unknown svc num %i",
                                 smr_header.svc_num);
        continue;
    }

    if( !int_err_occurred )
    {
      /* Free the Message in our queue */
      sns_smr_msg_free( msg_ptr );
    }
  } // end of while loop
} // end of function sns_diag_handle_smr_rx_indication

/*===========================================================================

  FUNCTION:   sns_debug_test_init1

  ===========================================================================*/
/*!
  @brief
  Entry point to the sns_debug module. Registers and initializes with DIAG.
  This would help to see debug messages during sensors initialization in
  QXDM.

  @param[i]
  No input parameters.

  @return
  sns_err_code_e: Error code indicating if init was successful.
*/
/*=========================================================================*/
sns_err_code_e sns_debug_test_init1( void )
{
  if (sns_debug_test_initialized == true)
  {
    SNS_PRINTF_STRING_FATAL_0(DBG_MOD_INIT,
                              "DIAG init: Already initialized. Nothing to do!");

    return SNS_SUCCESS;
  }

  sns_debug_set_dbg_str_mask( SNS_DIAG_DBG_MASK_DEFAULT );

  /* Initialize DIAG */
  sns_diag_init();
  sns_init_done();

  return SNS_SUCCESS;
}

/*===========================================================================

  FUNCTION:   sns_debug_test_init2

  ===========================================================================*/
/*!
  @brief
  Second entry point to the sns_debug module.
  Creates and initializes the debug thread.

  @param[i]
  No input parameters.

  @return
  sns_err_code_e: Error code indicating if init was successful.
*/
/*=========================================================================*/
sns_err_code_e sns_debug_test_init2( void )
{
#if defined( SNS_BLAST)
  sns_debug_init_mdm_dbg();
#else
  sns_err_code_e error_code;
  uint8_t os_err;

  SNS_PRINTF_STRING_MEDIUM_0(DBG_MOD_INIT,
                             "Init: In DIAG_Debug_Test_Init function!" );

  if (sns_debug_test_initialized == true)
  {
    SNS_PRINTF_STRING_FATAL_0(DBG_MOD_INIT,
                              "DIAG init: Already initialized. Nothing to do!" );
    return SNS_SUCCESS;
  }

  sns_diag_flag_grp = sns_os_sigs_create((OS_FLAGS)0, &os_err);
  if( sns_diag_flag_grp == NULL )
  {
    SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT,
                              "Sensors DIAG init: Can't create OS signal" );
    return SNS_ERR_NOMEM;
  }

  error_code = sns_debug_mr_init();
  if (error_code != 0)
  {
    SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT,
                              "Sensors DIAG init: Can't register with debug" );
    return SNS_ERR_FAILED;
  }

  error_code = sns_os_task_create(sns_diag_thread_main,
                                  NULL,
                                  NULL,
                                  SNS_MODULE_PRI_APPS_DIAG);
  if (error_code != 0)
  {
    SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT,
                              "Sensors DIAG init: can't create thread" );
    return SNS_ERR_FAILED;
  }

#ifdef SNS_QDSP_SIM
  /* On QDSP sim send log mask info to adsp as this will not be done
   * through sns_debug_timer_cb.
   */
  {
    sns_log_mask_t logging_bit_mask[2];
    if (sns_fill_log_mask(logging_bit_mask) == SNS_SUCCESS)
    {
        sns_diag_send_mask_info(LOG_MASK_INFO, logging_bit_mask[0],
                                logging_bit_mask[1]);
    }
  }
#endif

  error_code = sns_em_create_timer_obj( sns_debug_timer_cb, NULL,
                                        SNS_EM_TIMER_TYPE_ONESHOT,
                                        &sns_diag_logmask_delay_tmr_ptr);
  if (error_code == SNS_SUCCESS)
  {
    SNS_PRINTF_STRING_MEDIUM_0(DBG_MOD_INIT,
                              "Sensors Diag Init:Can create EM timer object");
    /* Register with timer now */
    sns_em_register_timer(sns_diag_logmask_delay_tmr_ptr,
                          sns_em_convert_usec_to_localtick(SNS_DIAG_LOGMASK_DELAY_USEC) );
    /* We dont return in case of ERRORs...this is not so FATAL that we
     * should bail out
     */
  }

  sns_debug_test_initialized = true;

  SNS_PRINTF_STRING_MEDIUM_0(DBG_MOD_INIT,
                             "Sensors DIAG Init: Initialization complete");

#endif /* defined( SNS_BLAST ) */
  return SNS_SUCCESS;
}

/*===========================================================================

  FUNCTION:   sns_debug_test_deinit2

  ===========================================================================*/
/*!
  @brief
  Clean up debug resources. Destroy debug thread.

  @param[i]
  No input parameters.

  @return
  sns_err_code_e: Error code indicating if init was successful.
*/
/*=========================================================================*/
sns_err_code_e sns_debug_test_deinit2( void )
{
    uint8_t err = 0;
    sns_err_code_e sns_err;

    sns_debug_test_initialized = FALSE;

    // Deinit MR
    sns_err = sns_debug_mr_deinit();
    if(SNS_SUCCESS != sns_err)
    {
        SNS_PRINTF_STRING_ERROR_1( DBG_MOD_INIT, "sns_acm_mr_deinit: failed %d", sns_err );
    }

    // signal thread to exit
    if(NULL != sns_diag_flag_grp)
    {
        sns_os_sigs_post( sns_diag_flag_grp,
                          SNS_DIAG_RX_SIG,
                          OS_FLAG_SET, &err );

        sns_os_task_del_req(SNS_MODULE_PRI_APPS_DIAG);

        sns_os_sigs_del(sns_diag_flag_grp, 0, &err);
        sns_diag_flag_grp = (OS_FLAG_GRP*)NULL;
    }

    if(NULL != sns_diag_logmask_delay_tmr_ptr)
    {
        sns_em_cancel_timer(sns_diag_logmask_delay_tmr_ptr);
        sns_em_delete_timer_obj(sns_diag_logmask_delay_tmr_ptr);
        sns_diag_logmask_delay_tmr_ptr = (sns_em_timer_obj_t)NULL;
    }

    return SNS_SUCCESS;
}
