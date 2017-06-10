/*!
  @file
  qcril_pbm_api_map.c

  @brief
  Handles the mapping to modems' (PBM) APIs.

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

#include "qcril_pbm_api_map.h"


/*===========================================================================

                       EXTERN DEFINITIONS 

===========================================================================*/

extern void qcril_pbm_event_callback( void *pUserData, pbm_notify_data_s_type *pNotifyData );


/*===========================================================================

                                API MAP 

===========================================================================*/

PBM_NOTIFY_FUNC qcril_pbm_api_event_callback[ QCRIL_MAX_MODEM_ID ] =
{
  qcril_pbm_event_callback
  #ifdef FEATURE_QCRIL_FUSION
  , qcril_pbm_event_callback
  #endif  /* FEATURE_QCRIL_FUSION */
};

qcril_pbm_api_funcs_type qcril_pbm_api_funcs[ QCRIL_MAX_MODEM_ID ] = 
{
  {
    pbm_is_init_completed,
    pbm_notify_register,
    pbm_notify_unregister,
    #if defined(FEATURE_QCRIL_FUSION) || defined(FEATURE_QCRIL_DSDS)
    pbm_session_enum_rec_init,
    pbm_session_enum_next_rec_id,
    pbm_session_calculate_fields_size_from_id,
    pbm_session_record_read,
    pbm_session_find_number
    #else
    pbm_enum_rec_init,
    pbm_enum_next_rec_id,
    pbm_calculate_fields_size_from_id,
    pbm_record_read,
    pbm_find_number
    #endif /* FEATURE_QCRIL_FUSION || FEATURE_QCRIL_DSDS */
  }
  #if defined(FEATURE_QCRIL_FUSION) && !defined(FEATURE_QCRIL_DSDS)
  ,
  {
    pbm_is_init_completed_fusion,
    pbm_notify_register_fusion,
    pbm_notify_unregister_fusion,
    pbm_session_enum_rec_init_fusion,
    pbm_session_enum_next_rec_id_fusion,
    pbm_session_calculate_fields_size_from_id_fusion,
    pbm_session_record_read_fusion,
    pbm_session_find_number_fusion
  }
  #endif /* FEATURE_QCRIL_FUSION && !FEATURE_QCRIL_DSDS */
};
