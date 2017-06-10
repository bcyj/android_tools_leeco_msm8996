/*!
  @file
  qcril_other_api_map.h

  @brief

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

#ifndef QCRIL_OTHER_API_MAP_H
#define QCRIL_OTHER_API_MAP_H

#include "comdef.h"
#include "nv.h"
#include "auth.h"
#include "qcrili.h"


/*===========================================================================

                    INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

#ifdef FEATURE_QCRIL_DSDS
typedef nv_stat_enum_type (nv_cmd_ext_remote_f_type) (
  
   nv_func_enum_type cmd,  
   nv_items_enum_type item,  
   nv_item_type *data_ptr,
   uint16 context
);
#else
typedef nv_stat_enum_type (nv_cmd_remote_f_type) (

    nv_func_enum_type cmd,  
    nv_items_enum_type item,  
    nv_item_type *data_ptr

);
#endif /* FEATURE_QCRIL_DSDS */

typedef boolean (auth_validate_a_key_f_type) (

  byte *a_key
    /* The A key to be validated */

);

typedef boolean (auth_send_update_a_key_cmd_f_type) (

  byte *a_key,
  int num_a_key_digits,
  int nam

);

typedef struct {

  #ifdef FEATURE_QCRIL_DSDS
  nv_cmd_ext_remote_f_type          *nv_cmd_remote_func;
  #else
  nv_cmd_remote_f_type              *nv_cmd_remote_func;
  #endif

  auth_validate_a_key_f_type        *auth_validate_a_key_func;
  auth_send_update_a_key_cmd_f_type *auth_send_update_a_key_cmd_func;

} qcril_other_api_funcs_type;


/*===========================================================================

                          EXTERN API MAP 

===========================================================================*/

extern qcril_other_api_funcs_type qcril_other_api_funcs[ QCRIL_MAX_MODEM_ID ];

#endif /* QCRIL_OTHER_API_MAP_H */
