#ifndef CRI_UIM_CORE_H
#define CRI_UIM_CORE_H
/*===========================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.

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

$Header:  $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/25/13   tl      Initial version

===========================================================================*/

/*===========================================================================

                            INCLUDE FILES

===========================================================================*/

#include "cri_core.h"

#include "mcm_sim_v01.h"
#include "common_v01.h"
#include "user_identity_module_v01.h"

#include "utils_common.h"

/*===========================================================================

                           GLOBALS

===========================================================================*/

extern uim_card_status_type_v01      uim_card_status;
extern int                           uim_client_id;

/*===========================================================================

                           DEFINES

===========================================================================*/

#define MCM_UIM_FILE_ID_LEN                       4
#define MCM_UIM_SLOT_INDEX_1                      0
#define MCM_UIM_SLOT_INDEX_2                      1
#define MCM_UIM_IMSI_M_RAW_SIZE                   10
#define MCM_UIM_MNC_MIN_LEN                       2
#define MCM_UIM_MNC_MAX_LEN                       3
#define MCM_UIM_MSISDN_MIN_LEN                    14
#define MCM_UIM_MDN_LEN                           11
#define MCM_UIM_SLOT_INVALID                      0xFFFF
#define MCM_UIM_INVALID_SESSION_VALUE             0xFFFF

/*===========================================================================

                           FUNCTIONS

===========================================================================*/

/*=========================================================================

  FUNCTION:  cri_uim_init_client

===========================================================================*/
/*
    @brief
    Performs the following task to initalize the UIM service:
      - resets the uim cache
      - registers as a service client of QMI UIM
      - registers for card status and refresh events
      - initalizes the uim cache

    @return
    cri_core_error_type
*/
/*=========================================================================*/
cri_core_error_type cri_uim_init_client
(
  hlos_ind_cb_type hlos_ind_cb
);

/*=========================================================================

  FUNCTION:  cri_uim_core_unsol_ind_handler

===========================================================================*/
/*
    @brief
    Retrieves and calls the corresponding HLOS response handler callback function

    @return
    None
*/
/*=========================================================================*/
void cri_uim_core_unsol_ind_handler
(
  int qmi_service_client_id,
  unsigned long message_id,
  void *ind_data,
  int ind_data_len
);

/*=========================================================================

  FUNCTION:  cri_uim_core_async_resp_handler

===========================================================================*/
/*
    @brief
    Calls the corresponding HLOS response handler callback function

    @return
    None
*/
/*=========================================================================*/
void cri_uim_core_async_resp_handler
(
  int qmi_service_client_id,
  unsigned long message_id,
  void *resp_data,
  int resp_data_len,
  cri_core_context_type cri_core_context
);

#endif /* CRI_UIM_CORE_H */
