#ifndef CRI_UIM_UTILS
#define CRI_UIM_UTILS
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
#include "user_identity_module_v01.h"
#include "utils_common.h"

/*===========================================================================

                           FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  cri_uim_util_has_card_status_changed

===========================================================================*/
/*
    @brief
    Function determines if the status of the card has changed

    @return
    int
      TRUE   - changed card status
      FALSE  - cannot change card status
*/
/*=========================================================================*/
int cri_uim_util_has_card_status_changed
(
  const uim_card_status_type_v01        * cache_card_status_ptr,
  const uim_card_status_type_v01        * new_card_status_ptr,
  unsigned short                        * slot_index_ptr
);

/*===========================================================================

  FUNCTION:  cri_uim_util_retrieve_card_status_global

===========================================================================*/
/*
    @brief
    Returns pointer to the cached card status

    @return
    uim_card_status_type_v01*
*/
/*=========================================================================*/
uim_card_status_type_v01* cri_uim_util_retrieve_card_status_global
(
  void
);

/*===========================================================================

  FUNCTION:  cri_uim_util_retrieve_client_id

===========================================================================*/
/*
    @brief
    Returns the cached client id

    @return
    int
*/
/*=========================================================================*/
int cri_uim_util_retrieve_client_id
(
  void
);

#endif

