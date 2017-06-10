/*!
  @file
  qcril_mmgsdi_sec.c

  @brief
  Handles RIL requests for MMGSDI on security related functions.

*/

/*===========================================================================

  Copyright (c) 2008 - 2009 Qualcomm Technologies, Inc. All Rights Reserved

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

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_mmgsdi_sec.c#4 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
11/24/11   rp	   Corrected in Mapping for input pin info with corresponding variables
01/25/11   at      Changed get/set pin status parameters per updated ril.h
07/09/10   at      Merged branch changes to qcril_fusion folder
04/05/10   js      Fixed compilation warning
03/09/10   js      Support for providing number of perso retries in perso
                   deactivate response
03/01/10   fc      Re-architecture to support split modem.
11/02/09   js      Check for valid client before sending command to MMGSDI.
10/24/09   js      QCRIL MMGSDI response function cleanup
09/29/09   mib     Fixed pin operation confirmation to set curr pin correctly
08/29/09   js      Fixed PIN operation response to remove req_table entry
07/24/09   sb      Renamed FEATURE_MMGSDI_PIN_RETRY to FEATURE_NEW_RIL_API
07/08/09   js      Pin Retry/Unblock Support
06/26/09   fc      Fixed the issue of bogus RIL Request reported in call flow
                   log packet.
04/05/09   fc      Cleanup log macros.
01/09/08   tml     Featurize for 1x non ruim build
10/08/08   tml     Added multimode support
08/04/08   tml     Updated debug message. Fixed enabled/disable values
07/22/08   tml     Fixed compilation on target
07/14/08   tml     Clean up message levels
05/23/08   tml     Added perso and pin enable/disable support
05/20/08   tml     Update formatting
05/19/08   tml     initial version


===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#if !defined (FEATURE_QCRIL_UIM_QMI)

#ifndef FEATURE_CDMA_NON_RUIM

#include "ril.h"
#include "IxErrno.h"
#include "qcrili.h"
#include <string.h>
#include "qcril_reqlist.h"
#include "qcril_mmgsdii.h"
#include "qcril_arb.h"


/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/





/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/


/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/


/*===========================================================================

                               INTERNAL FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_sec_process_pin_op_cnf

===========================================================================*/
/*!
    @brief
    Handle PIN Operation confirmation 

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_sec_process_pin_op_cnf 
(
  qcril_instance_id_e_type    instance_id,
  mmgsdi_return_enum_type     status,
  mmgsdi_cnf_enum_type        cnf,
  const mmgsdi_cnf_type       *cnf_ptr
)
{

  QCRIL_LOG_DEBUG( "qcril_mmgsdi_sec_process_pin_op_cnf: status = 0x%x, cnf = 0x%x \n",
                     status, cnf);

  if (cnf_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "  Null cnf ptr\n");
    return;
  }

  QCRIL_LOG_DEBUG("mmgsdi status 0x%x to process PIN operation 0x%x, with PIN retries 0x%x, unblock retries 0x%x", 
                   status, 
                   cnf_ptr->pin_operation_cnf.pin_op,
                   cnf_ptr->pin_operation_cnf.pin_info.num_retries,
                   cnf_ptr->pin_operation_cnf.pin_info.num_unblock_retries);

  qcril_mmgsdi_set_curr_pin_ptr(cnf_ptr->pin_operation_cnf.pin_info.pin_id);

  if((cnf_ptr->pin_operation_cnf.pin_op == MMGSDI_PIN_OP_VERIFY) ||
     (cnf_ptr->pin_operation_cnf.pin_op == MMGSDI_PIN_OP_ENABLE) ||
     (cnf_ptr->pin_operation_cnf.pin_op == MMGSDI_PIN_OP_DISABLE) ||
     (cnf_ptr->pin_operation_cnf.pin_op == MMGSDI_PIN_OP_CHANGE) ||
     (cnf_ptr->pin_operation_cnf.pin_op == MMGSDI_PIN_OP_DISABLE_AND_REPLACE))
  {
    qcril_mmgsdi_response(
      instance_id,
      (RIL_Token) cnf_ptr->pin_operation_cnf.response_header.client_data,
      status,
      (void*)&cnf_ptr->pin_operation_cnf.pin_info.num_retries,
      sizeof(int),
      TRUE,
      NULL);
  }
  else if(cnf_ptr->pin_operation_cnf.pin_op == MMGSDI_PIN_OP_UNBLOCK)
  {
    qcril_mmgsdi_response(
      instance_id,
      (RIL_Token) cnf_ptr->pin_operation_cnf.response_header.client_data,
      status,
      (void*)&cnf_ptr->pin_operation_cnf.pin_info.num_unblock_retries,
      sizeof(int),
      TRUE,
      NULL);
  }

  if (MMGSDI_SUCCESS == status)
  {
    qcril_mmgsdi_set_curr_pin_ptr(MMGSDI_PIN1);
  }
} /* qcril_mmgsdi_sec_process_pin_op_cnf() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_sec_process_cnf

===========================================================================*/
/*!
    @brief
    General callback function for MMGSDI security related operations

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_sec_process_cnf 
(
  qcril_instance_id_e_type    instance_id,
  mmgsdi_return_enum_type     status,                              
  mmgsdi_cnf_enum_type        cnf,                                    
  const mmgsdi_cnf_type       *cnf_ptr
)
{
  QCRIL_LOG_DEBUG( "qcril_mmgsdi_sec_process_cmd_cb: cnf = 0x%x, status = 0x%x\n", cnf, status);

  switch (cnf)
  {
  case MMGSDI_PIN_OPERATION_CNF:
    qcril_mmgsdi_sec_process_pin_op_cnf( instance_id, status, cnf, cnf_ptr);
    break;
  
  default:
    QCRIL_LOG_ERROR( "Unhandled cnf = 0x%x, status = 0x%x\n", cnf, status);
    break;
  }
} /* qcril_mmgsdi_sec_process_cnf() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_sec_process_perso_deact_cnf

===========================================================================*/
/*!
    @brief
    Handle Perso Deactivation confirmation 

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_sec_process_perso_deact_cnf 
(
  qcril_instance_id_e_type    instance_id,
  gsdi_returns_T              status,
  uint32                      ril_token,
  gsdi_perso_enum_type        perso_feature,
  uint32                      num_retries
)
{
  mmgsdi_return_enum_type mmgsdi_status = MMGSDI_SUCCESS;
  
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "qcril_mmgsdi_sec_process_perso_deact_cnf: status = 0x%x, perso_feature = 0x%x \n",
                   status, perso_feature);

  mmgsdi_status = qcril_mmgsdi_convert_from_gsdi_status(status);
  qcril_mmgsdi_response( instance_id,
                         (RIL_Token) ril_token,
                         mmgsdi_status,
                         (void*)&num_retries,
                         sizeof(int),
                         TRUE,
                         NULL );

} /* qcril_mmgsdi_sec_process_perso_deact_cnf() */


/*===========================================================================

                               QCRIL INTERFACE FUNCTIONS

===========================================================================*/


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_request_enter_sim_pin

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_ENTER_SIM_PIN/RIL_REQUEST_ENTER_SIM_PIN2.

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_request_enter_pin
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type    instance_id;
  qcril_modem_id_e_type       modem_id;
  mmgsdi_return_enum_type     mmgsdi_status = MMGSDI_SUCCESS;
  mmgsdi_data_type            pin_data;
  mmgsdi_pin_enum_type        pin_id        = MMGSDI_PIN1;
  qcril_reqlist_public_type   reqlist_entry; 
  uint8                       **in_data_ptr   = NULL;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/


  QCRIL_LOG_INFO( "%s", "qcril_mmgsdi_request_enter_pin: RIL_REQUEST_ENTER_SIM_PIN or RIL_REQUEST_ENTER_SIM_PIN2 \n");

  QCRIL_MMGSDI_RETURN_ERR_RSP_IF_INVALID_CLIENT( instance_id, qcril_mmgsdi.client_id, params_ptr->t, FALSE );

  /* Obtain input pin info */
    in_data_ptr = (uint8 **)(params_ptr->data);

  /* Null pointer check for input data */
  QCRIL_ASSERT(  in_data_ptr    != NULL );
  QCRIL_ASSERT(  in_data_ptr[0] != NULL );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,modem_id,
                               QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_NONE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  /* Populate the mmgsdi required members */
  pin_data.data_len = (mmgsdi_len_type)(strlen((const char*)in_data_ptr[0])); 
  pin_data.data_ptr = in_data_ptr[0];

 switch (params_ptr->event_id)
  {
  case RIL_REQUEST_ENTER_SIM_PIN:
    pin_id = MMGSDI_PIN1;
    break;
  case RIL_REQUEST_ENTER_SIM_PIN2:
    pin_id = MMGSDI_PIN2;
    break;
  default:
    /* Invalid PIN ID */
    QCRIL_LOG_ERROR( "  Invalid Pin from RIL Request ID 0x%x\n", params_ptr->event_id);
    mmgsdi_status = MMGSDI_ERROR;
    break;
  }
  
  if (MMGSDI_SUCCESS == mmgsdi_status)
  {
    QCRIL_LOG_RPC( modem_id, "mmgsdi_verify_pin()", "pin_id", pin_id);
    qcril_mmgsdi_print_byte_data(pin_data.data_len, pin_data.data_ptr);
    mmgsdi_status = mmgsdi_verify_pin (qcril_mmgsdi.client_id, 
                                       MMGSDI_SLOT_1, 
                                       pin_id, 
                                       pin_data, 
                                       qcril_mmgsdi_command_callback,
                                       (mmgsdi_client_data_type)params_ptr->t);
  }
  if (MMGSDI_SUCCESS != mmgsdi_status)
  {
    qcril_mmgsdi_response( instance_id, params_ptr->t, mmgsdi_status, NULL, 0, TRUE, NULL );
  }
} /* qcril_mmgsdi_request_enter_pin() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_request_enter_puk

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_ENTER_SIM_PUK/RIL_REQUEST_ENTER_SIM_PUK2.

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_request_enter_puk
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type    instance_id;
  qcril_modem_id_e_type       modem_id;
  mmgsdi_return_enum_type     mmgsdi_status = MMGSDI_SUCCESS;
  mmgsdi_data_type            puk_data;
  mmgsdi_data_type            new_pin_data;
  mmgsdi_pin_enum_type        pin_id        = MMGSDI_PIN1;
  qcril_reqlist_public_type   reqlist_entry;
  uint8                       **in_data_ptr   = NULL;
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/
  
  QCRIL_LOG_INFO( "%s", "qcril_mmgsdi_request_enter_puk: RIL_REQUEST_ENTER_SIM_PUK or RIL_REQUEST_ENTER_SIM_PUK2 \n");

  QCRIL_MMGSDI_RETURN_ERR_RSP_IF_INVALID_CLIENT( instance_id, qcril_mmgsdi.client_id, params_ptr->t, FALSE );

  /* Obtain input pin info:
     ((const char **)data)[0] is PUK value
     ((const char **)data)[1] is new PIN value  */
  in_data_ptr = (uint8 **)params_ptr->data;

  /* Null pointer check for extracted input data */
  QCRIL_ASSERT(  in_data_ptr     != NULL );
  QCRIL_ASSERT(  in_data_ptr[0]  != NULL );
  QCRIL_ASSERT(  in_data_ptr[1]  != NULL );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id,
                               QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_NONE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  /* Populate the mmgsdi required members */
  puk_data.data_len     = (mmgsdi_len_type)(strlen((const char*)in_data_ptr[0])); 
  puk_data.data_ptr     = in_data_ptr[0]; 
  new_pin_data.data_len = (mmgsdi_len_type)(strlen((const char*)in_data_ptr[1])); 
  new_pin_data.data_ptr = in_data_ptr[1];

  switch (params_ptr->event_id)
  {
  case RIL_REQUEST_ENTER_SIM_PUK:
    pin_id = MMGSDI_PIN1;
    break;
  case RIL_REQUEST_ENTER_SIM_PUK2:
    pin_id = MMGSDI_PIN2;
    break;
  default:
    /* Invalid PIN ID */
    QCRIL_LOG_ERROR( "  Invalid Puk from RIL Request ID 0x%x\n", params_ptr->event_id);
    mmgsdi_status = MMGSDI_ERROR;
    break;
  }
  
  if (MMGSDI_SUCCESS == mmgsdi_status)
  {
    QCRIL_LOG_RPC( modem_id, "mmgsdi_unblock_pin()", "pin_id", pin_id);
    qcril_mmgsdi_print_byte_data(puk_data.data_len, puk_data.data_ptr);
    qcril_mmgsdi_print_byte_data(new_pin_data.data_len, new_pin_data.data_ptr);
    mmgsdi_status = mmgsdi_unblock_pin (qcril_mmgsdi.client_id, 
                                        MMGSDI_SLOT_1, 
                                        pin_id, 
                                        puk_data,
                                        new_pin_data, 
                                        qcril_mmgsdi_command_callback,
                                        (mmgsdi_client_data_type)params_ptr->t);
  }
  if (MMGSDI_SUCCESS != mmgsdi_status)
  {
    qcril_mmgsdi_response( instance_id, params_ptr->t, mmgsdi_status, NULL, 0, TRUE, NULL );
  }

} /* qcril_mmgsdi_request_enter_puk() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_request_change_pin

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CHANGE_SIM_PIN/RIL_REQUEST_CHANGE_SIM_PIN2.

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_request_change_pin
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type    instance_id;
  qcril_modem_id_e_type       modem_id;
  mmgsdi_return_enum_type     mmgsdi_status = MMGSDI_SUCCESS;
  mmgsdi_data_type            old_pin_data;
  mmgsdi_data_type            new_pin_data;
  mmgsdi_pin_enum_type        pin_id        = MMGSDI_PIN1;
  qcril_reqlist_public_type   reqlist_entry;
  uint8                       **in_data_ptr   = NULL;
  /*-----------------------------------------------------------------------*/
  
  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s", "qcril_mmgsdi_request_change_pin: RIL_REQUEST_CHANGE_SIM_PIN or RIL_REQUEST_CHANGE_SIM_PIN2 \n");

  QCRIL_MMGSDI_RETURN_ERR_RSP_IF_INVALID_CLIENT( instance_id, qcril_mmgsdi.client_id, params_ptr->t, FALSE );

  /* Obtain input pin info:
     ((const char **)data)[0] is old PIN value
     ((const char **)data)[1] is new PIN value  */
  in_data_ptr = (uint8 **)params_ptr->data;

  /* Null pointer check for extracted input data */
  QCRIL_ASSERT(  in_data_ptr     != NULL );
  QCRIL_ASSERT(  in_data_ptr[0]  != NULL );
  QCRIL_ASSERT(  in_data_ptr[1]  != NULL );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id,
                               QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_NONE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  /* Populate the mmgsdi required members */
  old_pin_data.data_len = (mmgsdi_len_type)(strlen((const char*)in_data_ptr[0]));
  old_pin_data.data_ptr = in_data_ptr[0];
  new_pin_data.data_len = (mmgsdi_len_type)(strlen((const char*)in_data_ptr[1]));
  new_pin_data.data_ptr = in_data_ptr[1];

  switch (params_ptr->event_id)
  {
  case RIL_REQUEST_CHANGE_SIM_PIN:
    pin_id = MMGSDI_PIN1;
    break;
  case RIL_REQUEST_CHANGE_SIM_PIN2:
    pin_id = MMGSDI_PIN2;
    break;
  default:
    /* Invalid PIN ID */
    QCRIL_LOG_ERROR( "  Invalid Puk from RIL Request ID 0x%x\n", params_ptr->event_id);
    mmgsdi_status = MMGSDI_ERROR;
    break;
  }
  
  if (MMGSDI_SUCCESS == mmgsdi_status)
  {
    QCRIL_LOG_RPC( modem_id, "mmgsdi_change_pin()", "pin_id", pin_id);
    qcril_mmgsdi_print_byte_data(old_pin_data.data_len, old_pin_data.data_ptr);
    qcril_mmgsdi_print_byte_data(new_pin_data.data_len, new_pin_data.data_ptr);
    mmgsdi_status = mmgsdi_change_pin (qcril_mmgsdi.client_id, 
                                       MMGSDI_SLOT_1, 
                                       pin_id, 
                                       old_pin_data,
                                       new_pin_data, 
                                       qcril_mmgsdi_command_callback,
                                       (mmgsdi_client_data_type)params_ptr->t);
  }
  if (MMGSDI_SUCCESS != mmgsdi_status)
  {
    qcril_mmgsdi_response( instance_id, params_ptr->t, mmgsdi_status, NULL, 0, TRUE, NULL );
  }
} /* qcril_mmgsdi_request_change_pin() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_request_set_pin_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS
    (RIL request: RIL_REQUEST_SET_FACILITY_LOCK - SC)

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_request_set_pin_status
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type    instance_id;
  qcril_modem_id_e_type       modem_id;
  mmgsdi_return_enum_type     mmgsdi_status = MMGSDI_SUCCESS;
  mmgsdi_data_type            pin_data;
  uint8                       **in_data_ptr   = NULL;
  mmgsdi_pin_enum_type        pin_id        = MMGSDI_PIN1;
  qcril_reqlist_public_type   reqlist_entry; 

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s", "qcril_mmgsdi_request_set_pin_status \n");

  QCRIL_MMGSDI_RETURN_ERR_RSP_IF_INVALID_CLIENT( instance_id, qcril_mmgsdi.client_id, params_ptr->t, FALSE );

  /* Obtain input pin info:
     ((const char **)data)[0] = facility string code from TS 27.007 7.4
     (eg "AO" for BAOC)
     ((const char **)data)[1] = "0" for "unlock" and "1" for "lock"
     ((const char **)data)[2] = password
     ((const char **)data)[3] = string representation of decimal TS 27.007
                                service class bit vector. Eg, the string
                                "1" means "set this facility for voice services"  */
  in_data_ptr = (uint8 **)(params_ptr->data);

  /* Null pointer check for extracted input data */
  QCRIL_ASSERT(  in_data_ptr    != NULL );
  QCRIL_ASSERT(  in_data_ptr[0] != NULL );
  QCRIL_ASSERT(  in_data_ptr[1] != NULL );
  QCRIL_ASSERT(  in_data_ptr[2] != NULL );

  QCRIL_LOG_INFO( "qcril_mmgsdi_request_set_pin_status(%s, %s, %s)\n",
                  in_data_ptr[0],
                  in_data_ptr[1],
                  in_data_ptr[2]
                  );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id,
                               QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_NONE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }
 
  if (memcmp(in_data_ptr[0], "SC", 2) != 0)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid Input Parameter [0] for QCRIL_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS\n" );
    qcril_mmgsdi_response( instance_id, params_ptr->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    return;
  }

  /* Populate the mmgsdi required members */
  pin_data.data_len = (mmgsdi_len_type)(strlen((const char*)in_data_ptr[2]));
  pin_data.data_ptr = in_data_ptr[2];

  if ('1' == *in_data_ptr[1])
  {
    QCRIL_LOG_RPC( modem_id, "mmgsdi_enable_pin()", "pin_id", pin_id);
    qcril_mmgsdi_print_byte_data(pin_data.data_len, pin_data.data_ptr);
    mmgsdi_status = mmgsdi_enable_pin (qcril_mmgsdi.client_id, 
                                       MMGSDI_SLOT_1, 
                                       pin_id, 
                                       pin_data, 
                                       qcril_mmgsdi_command_callback,
                                       (mmgsdi_client_data_type)params_ptr->t);
  }
  else if ('0' == *in_data_ptr[1])
  {
    QCRIL_LOG_RPC( modem_id, "mmgsdi_disable_pin()", "pin_id", pin_id);
    qcril_mmgsdi_print_byte_data(pin_data.data_len, pin_data.data_ptr);
    mmgsdi_status = mmgsdi_disable_pin (qcril_mmgsdi.client_id, 
                                        MMGSDI_SLOT_1, 
                                        pin_id, 
                                        pin_data, 
                                        qcril_mmgsdi_command_callback,
                                        (mmgsdi_client_data_type)params_ptr->t);
  }
  else
  {
    QCRIL_LOG_ERROR( "Invalid Input Parameter [1]: 0x%x for QCRIL_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS \n",
                     *in_data_ptr[1]);
    mmgsdi_status = MMGSDI_ERROR;
  }
  if (MMGSDI_SUCCESS != mmgsdi_status)
  {
    qcril_mmgsdi_response( instance_id, params_ptr->t, mmgsdi_status, NULL, 0, TRUE, NULL );
  }
} /* qcril_mmgsdi_request_set_pin_status() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_request_get_pin_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS
    (RIL request: RIL_REQUEST_QUERY_FACILITY_LOCK - SC)

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_request_get_pin_status
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type      instance_id;
  qcril_modem_id_e_type         modem_id = QCRIL_DEFAULT_MODEM_ID;
  uint8                       **in_data_ptr     = NULL;
  RIL_PinState                 *pin_status_ptr  = NULL;
  mmgsdi_return_enum_type       mmgsdi_status   = MMGSDI_SUCCESS;
  int                           enable          = 0;
  qcril_reqlist_public_type     reqlist_entry; 

  /*-----------------------------------------------------------------------*/
  
  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/
  
  QCRIL_LOG_INFO( "%s", "qcril_mmgsdi_request_get_pin_status \n");

  /* Obtain input pin info:
     ((const char **)data)[0] is the AID (applies only in case of FDN, or "" otherwise)
     ((const char **)data)[1] is the facility string code from TS 27.007 7.4
                          (eg "AO" for BAOC, "SC" for SIM lock)
     ((const char **)data)[2] is the password, or "" if not required
     ((const char **)data)[3] is the TS 27.007 service class bit vector of
                                services to query  */
  in_data_ptr = (uint8 **)(params_ptr->data);

  /* Null pointer check for extracted input data */
  QCRIL_ASSERT(  in_data_ptr    != NULL );
  QCRIL_ASSERT(  in_data_ptr[1] != NULL );

  QCRIL_LOG_INFO( "qcril_mmgsdi_request_get_pin_status(%s, %s, %s, %s, %s)\n",
                  (in_data_ptr[0] != NULL) ? (const char *)in_data_ptr[0] : "NULL",
                  in_data_ptr[1],
                  (in_data_ptr[2] != NULL) ? (const char *)in_data_ptr[2] : "NULL");

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id,
                               QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_NONE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }
  
  if (memcmp(in_data_ptr[1], "SC", 2) != 0)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid Input Parameter [1] for QCRIL_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS\n" );
    qcril_mmgsdi_response( instance_id, params_ptr->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    return;
  }
  
  if ((!qcril_mmgsdi.curr_pin.valid) || 
      (qcril_mmgsdi.curr_pin.app_index >= RIL_CARD_MAX_APPS))
  {
    QCRIL_LOG_ERROR( "%s", "Unknown current pin1 status\n" );
    qcril_mmgsdi_response( instance_id, params_ptr->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    return;
  }

  pin_status_ptr = &qcril_mmgsdi.curr_card_status.applications[qcril_mmgsdi.curr_pin.app_index].pin1;
  QCRIL_ASSERT( pin_status_ptr != NULL );

  switch(*pin_status_ptr)
  {
  case RIL_PINSTATE_ENABLED_NOT_VERIFIED:
  case RIL_PINSTATE_ENABLED_VERIFIED:
  case RIL_PINSTATE_ENABLED_BLOCKED:
  case RIL_PINSTATE_ENABLED_PERM_BLOCKED:
    QCRIL_LOG_INFO( "%s", "PIN enable\n" );
    enable = 1;
    break;
  case RIL_PINSTATE_DISABLED:
    QCRIL_LOG_INFO( "%s", "PIN disable\n" );
    enable = 0;
    break;
  default:
    QCRIL_LOG_INFO( "Unknown pin status 0x%x \n", *pin_status_ptr);
    mmgsdi_status = MMGSDI_ERROR;
    break;
  }

  qcril_mmgsdi_response( instance_id, params_ptr->t,mmgsdi_status, &enable, sizeof(int),TRUE, NULL );
  
} /* qcril_mmgsdi_request_get_pin_status() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_request_enter_perso_key

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_request_enter_perso_key
( 
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type          instance_id;
  qcril_modem_id_e_type             modem_id;
  mmgsdi_return_enum_type           mmgsdi_status = MMGSDI_SUCCESS;
  gsdi_returns_T                    gsdi_status   = GSDI_SUCCESS;
  gsdi_perso_control_key_data_type  perso_dck;
  qcril_reqlist_public_type         reqlist_entry;
  const RIL_Depersonalization*      perso_request_ptr = NULL;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/
  
  QCRIL_LOG_INFO( "%s", "qcril_mmgsdi_request_enter_perso_key \n");

  /* Obtain input perso key info */  
  perso_request_ptr = (const RIL_Depersonalization*)params_ptr->data;

  /* Null pointer check for input data */
  QCRIL_ASSERT( perso_request_ptr != NULL );
  QCRIL_ASSERT( perso_request_ptr->depersonalizationCode != NULL );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id,
                               QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_NONE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  /* Populate the gsdi perso required members */
  perso_dck.slot = GSDI_SLOT_1;
  perso_dck.num_bytes = (int32)strlen(perso_request_ptr->depersonalizationCode);
  perso_dck.control_key_p = perso_request_ptr->depersonalizationCode;

  switch (params_ptr->event_id)
  {
  case RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE:
    perso_dck.feature = GSDI_PERSO_NW;
    break;
  default:
    /* Invalid Request ID */
    QCRIL_LOG_ERROR( "  Invalid Request ID from RIL 0x%x\n", params_ptr->event_id);
    gsdi_status = GSDI_ERROR;
    break;
  }

  if (GSDI_SUCCESS == gsdi_status)
  {
    /* When framework sends a null input key then provide number of depersonalization
       attempts remaining */
    if(perso_dck.num_bytes == 0)
    {
      QCRIL_LOG_RPC( modem_id, "gsdi_perso_get_dck_num_retries()", "perso_feature", perso_dck.feature);
      gsdi_status = gsdi_perso_get_dck_num_retries(GSDI_SLOT_1,
                                                   (uint32)params_ptr->t,
                                                   qcril_mmgsdi_gsdi_command_callback);
    }
    else
    {
      QCRIL_LOG_RPC( modem_id, "gsdi_perso_deactivate_feature_indicator()", "perso_feature", perso_dck.feature);
      qcril_mmgsdi_print_byte_data(perso_dck.num_bytes, perso_dck.control_key_p);
      gsdi_status = gsdi_perso_deactivate_feature_indicator(&perso_dck,
                                                            (uint32)params_ptr->t,
                                                            qcril_mmgsdi_gsdi_command_callback);
    }
  }
  if (GSDI_SUCCESS != gsdi_status)
  {
    mmgsdi_status = qcril_mmgsdi_convert_from_gsdi_status(gsdi_status);
    qcril_mmgsdi_response( instance_id, params_ptr->t, mmgsdi_status, NULL, 0, TRUE, NULL );
  }

} /* qcril_mmgsdi_request_enter_perso_key() */


/*=========================================================================

  FUNCTION:  qcril_mmgsdi_request_oem_hook_me_depersonalization

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_OEM_HOOK_RAW (ME Depersonalization).

    @return
    None
*/
/*=========================================================================*/
void qcril_mmgsdi_request_oem_hook_me_depersonalization
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type              instance_id;
  qcril_modem_id_e_type                 modem_id;
  mmgsdi_return_enum_type               mmgsdi_status = MMGSDI_SUCCESS;
  gsdi_returns_T                        gsdi_status = GSDI_SUCCESS;
  gsdi_perso_control_key_data_type      perso_dck;
  qcril_mmgsdi_perso_category_enum_type perso_cat;
  const char                            *perso_data;
  qcril_request_resp_params_type        resp;
  qcril_reqlist_public_type             reqlist_entry; 

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/
  
  QCRIL_LOG_INFO( "%s", "qcril_mmgsdi_request_oem_hook_me_depersonalization \n");

  perso_data = (const char*)params_ptr->data;

  /* Null pointer check for extracted input data */
  if( perso_data == NULL )
  {
    QCRIL_LOG_ERROR( "%s", "Personalization data received is null");
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( & resp );
    return;
  }

  /* check the data length received. Taking into consideration the Perso key length 
  which should be between 6 and 16 as per 3GPP TS 22.022 */
  if( (params_ptr->datalen < (QCRIL_MMGSDI_PERSO_CATEGORY_LENGTH + QCRIL_MMGSDI_MIN_PERSO_KEY_LENGTH)) ||
        (params_ptr->datalen > (QCRIL_MMGSDI_PERSO_CATEGORY_LENGTH + QCRIL_MMGSDI_MAX_PERSO_KEY_LENGTH)) )
  {
    QCRIL_LOG_ERROR( "  Invalid data length (%d) for Personalization 0x%x\n", params_ptr->datalen, params_ptr->event_id);
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( & resp );
    return;
  }

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id,
                               QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_NONE, NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  /* Get the perso category */
  memcpy(&perso_cat, perso_data, QCRIL_MMGSDI_PERSO_CATEGORY_LENGTH);

  /* Now move past the perso category to point to perso control key */
  perso_data += QCRIL_MMGSDI_PERSO_CATEGORY_LENGTH;

  /* Populate the gsdi perso required members */
  perso_dck.slot = GSDI_SLOT_1;
  perso_dck.num_bytes = (int32)strlen(perso_data);
  perso_dck.control_key_p = (uint8 *)perso_data;
  QCRIL_LOG_DEBUG( "Perso category received is %d\n", perso_cat);

  /* Map to the gsdi perso feature */
  switch ( perso_cat )
  {
    case QCRIL_MMGSDI_PERSO_NETWORK:
      perso_dck.feature = GSDI_PERSO_NW;
      break;

    case QCRIL_MMGSDI_PERSO_NETWORK_SUBSET:
      perso_dck.feature = GSDI_PERSO_NS;
      break;

    case QCRIL_MMGSDI_PERSO_SERVICE_PROVIDER:
      perso_dck.feature = GSDI_PERSO_SP;
      break;

    case QCRIL_MMGSDI_PERSO_CORPORATE:
      perso_dck.feature = GSDI_PERSO_CP;
      break;

    case QCRIL_MMGSDI_PERSO_SIM_USIM:
      perso_dck.feature = GSDI_PERSO_SIM;
      break;

    case QCRIL_MMGSDI_PERSO_CDMA_NETWORK1:
      perso_dck.feature = GSDI_PERSO_RUIM_NW1;
      break;

    case QCRIL_MMGSDI_PERSO_CDMA_NETWORK2:
      perso_dck.feature = GSDI_PERSO_RUIM_NW2;
      break;

    case QCRIL_MMGSDI_PERSO_CDMA_HRPD:
      perso_dck.feature = GSDI_PERSO_RUIM_HRPD;
      break;

    case QCRIL_MMGSDI_PERSO_CDMA_SERVICE_PROVIDER:
      perso_dck.feature = GSDI_PERSO_RUIM_SP;
      break;

    case QCRIL_MMGSDI_PERSO_CDMA_CORPORATE:
      perso_dck.feature = GSDI_PERSO_RUIM_CP;
      break;

    case QCRIL_MMGSDI_PERSO_CDMA_RUIM:
      perso_dck.feature = GSDI_PERSO_RUIM_RUIM;
      break;

    default:
      QCRIL_LOG_ERROR( "  Invalid personalization category (%d)\n", perso_cat);
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( & resp );
      return;
  }

  QCRIL_LOG_RPC( modem_id, "gsdi_perso_deactivate_feature_indicator()", "perso_feature", perso_dck.feature);
  gsdi_status = gsdi_perso_deactivate_feature_indicator( &perso_dck,
                                                         (uint32)params_ptr->t,
                                                         qcril_mmgsdi_gsdi_command_callback);

  if (GSDI_SUCCESS != gsdi_status)
  {
    mmgsdi_status = qcril_mmgsdi_convert_from_gsdi_status(gsdi_status);
    qcril_mmgsdi_response( instance_id, params_ptr->t, mmgsdi_status, NULL, 0, TRUE, NULL );
  }

} /* qcril_mmgsdi_request_oem_hook_me_depersonalization */
#endif /*  FEATURE_CDMA_NON_RUIM */

#endif /* !defined (FEATURE_QCRIL_UIM_QMI) */
