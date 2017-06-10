/*!
  @file
  qcril_pbm_api_map.h

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

#ifndef QCRIL_PBM_API_MAP_H
#define QCRIL_PBM_API_MAP_H

#include "comdef.h"
#include "pbmlib.h"
#ifdef FEATURE_QCRIL_FUSION
#include "pbmlib_fusion.h"
#endif /* FEATURE_QCRIL_FUSION */
#include "qcrili.h"


/*===========================================================================

                    INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

typedef boolean (pbm_is_init_completed_f_type) (void);

typedef pbm_return_type (pbm_notify_register_f_type) (
  PBM_NOTIFY_FUNC func, 
  void            *user_data
);

typedef pbm_return_type (pbm_notify_unregister_f_type) (
  PBM_NOTIFY_FUNC func, 
  void            *user_data
);

#if defined(FEATURE_QCRIL_FUSION) || defined(FEATURE_QCRIL_DSDS)
typedef pbm_return_type (pbm_session_enum_rec_init_f_type) (
  pbm_phonebook_type  pb_id,  
  uint16              category,
  pbm_field_id_e_type field_id,
  const uint8         *data_ptr,
  uint16              data_size,
  uint32              flags
);

typedef pbm_return_type (pbm_session_enum_next_rec_id_f_type) (
  pbm_record_id_type *rec_id_ptr
);

typedef int (pbm_session_calculate_fields_size_from_id_f_type) (
  pbm_record_id_type rec_id
);

typedef pbm_return_type (pbm_session_record_read_f_type) (
  pbm_record_id_type rec_id,
  uint16             *category_ptr,
  int                *num_fields_ptr,
  uint8              *data_buf,
  uint32              data_buf_size
);

typedef pbm_return_type (pbm_session_find_number_f_type) (
  pbm_phonebook_type pb_id,
  const byte         *number, 
  int                loc_number_len,
  pbm_record_s_type  *record,
  PBM_FIND_CB_FUNC   proc_func
);
#else 
typedef pbm_return_type (pbm_enum_rec_init_f_type) (
  pbm_device_type     pb_id,
  uint16              category,
  pbm_field_id_e_type field_id,
  const uint8         *data_ptr,
  uint16              data_size,
  uint32              flags
);

typedef pbm_return_type (pbm_enum_next_rec_id_f_type) (
  uint16 *rec_id_ptr
);

typedef int (pbm_calculate_fields_size_from_id_f_type) (
  uint16 rec_id 
);

typedef pbm_return_type (pbm_record_read_f_type) (
  uint16 rec_id,
  uint16 *category_ptr,
  int    *num_fields_ptr,
  uint8  *data_buf,
  uint32 data_buf_size
);

typedef pbm_return_type (pbm_find_number_f_type) (
  pbm_device_type   pb_id,
  const byte        *number, 
  int               loc_number_len,
  pbm_record_s_type *record,
  PBM_FIND_CB_FUNC  proc_func
);
#endif /* FEATURE_QCRIL_FUSION || FEATURE_QCRIL_DSDS */

typedef struct 
{
  pbm_is_init_completed_f_type                     *pbm_is_init_completed_func;
  pbm_notify_register_f_type                       *pbm_notify_register_func;
  pbm_notify_unregister_f_type                     *pbm_notify_unregister_func;

  #if defined(FEATURE_QCRIL_FUSION) || defined(FEATURE_QCRIL_DSDS)
  pbm_session_enum_rec_init_f_type                 *pbm_enum_rec_init_func;
  pbm_session_enum_next_rec_id_f_type              *pbm_enum_next_rec_id_func;
  pbm_session_calculate_fields_size_from_id_f_type *pbm_calculate_fields_size_from_id_func;
  pbm_session_record_read_f_type                   *pbm_record_read_func;
  pbm_session_find_number_f_type                   *pbm_find_number_func;
  #else
  pbm_enum_rec_init_f_type                         *pbm_enum_rec_init_func;
  pbm_enum_next_rec_id_f_type                      *pbm_enum_next_rec_id_func;
  pbm_calculate_fields_size_from_id_f_type         *pbm_calculate_fields_size_from_id_func;
  pbm_record_read_f_type                           *pbm_record_read_func;
  pbm_find_number_f_type                           *pbm_find_number_func;
  #endif /* FEATURE_QCRIL_FUSION || FEATURE_QCRIL_DSDS */

} qcril_pbm_api_funcs_type;


/*===========================================================================

                          EXTERN API MAP 

===========================================================================*/

extern PBM_NOTIFY_FUNC          qcril_pbm_api_event_callback[ QCRIL_MAX_MODEM_ID ];
extern qcril_pbm_api_funcs_type qcril_pbm_api_funcs[ QCRIL_MAX_MODEM_ID ];

#endif /* QCRIL_PBM_API_MAP_H */
