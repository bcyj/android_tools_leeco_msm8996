/*!
  @file
  qcril_other_api_map.c

  @brief
  Handles the mapping to modems' (NV, AUTH) APIs.

*/

/*===========================================================================

  Copyright (c) 2009 - 2010 Qualcomm Technologies, Inc. All Rights Reserved

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

when       who     what, where, why
--------   ---     ---------------------------------------------------------- 
04/06/10   pg      First cut.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "qcril_other_api_map.h"


/*===========================================================================

                               API MAP

===========================================================================*/

qcril_other_api_funcs_type qcril_other_api_funcs[ QCRIL_MAX_MODEM_ID ] =
{
  {
    #ifdef FEATURE_QCRIL_DSDS
    nv_cmd_ext_remote,
    #else
    nv_cmd_remote,             
    #endif /* FEATURE_QCRIL_DSDS */

    auth_validate_a_key,       
    auth_send_update_a_key_cmd 
  }
  #if !defined(FEATURE_QCRIL_DSDS) && defined(FEATURE_QCRIL_FUSION)
  ,
  {
    nv_cmd_remote,             
    auth_validate_a_key,       
    auth_send_update_a_key_cmd 
  }
  #endif /* !FEATURE_QCRIL_DSDS && FEATURE_QCRIL_FUSION */
};  
