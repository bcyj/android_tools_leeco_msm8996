/*!
  @file
  qcril_pbm.h

  @brief

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

when         who     what, where, why
--------   ---     ----------------------------------------------------------
06/18/10   pg      Initial version.

===========================================================================*/

#ifndef QCRIL_PBM_H
#define QCRIL_PBM_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "comdef.h"
#include "qcrili.h"


/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

typedef enum
{
  QCRIL_PBM_DEVICE_TYPE_ECC,
  QCRIL_PBM_DEVICE_TYPE_FDN
} qcril_pbm_device_type;


/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/

void qcril_pbm_enable_oprt_mode_check
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id
);

errno_enum_type qcril_pbm_get_ecc( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, 
                                   pbm_field_id_e_type pbm_field_id, boolean lock_ph_mutex );

void qcril_pbm_update_ecc_property( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, 
                                    boolean update_from_pbm_or_sub_event );

#ifdef FEATURE_QCRIL_FUSION
void qcril_pbm_set_pb_id_for_svlte_1( qcril_instance_id_e_type instance_id, pbm_phonebook_type *pb_id_ptr,
                                      qcril_pbm_device_type device_type );
#endif /* FEATURE_QCRIL_FUSION */

#ifdef FEATURE_QCRIL_DSDS
errno_enum_type qcril_pbm_set_pb_id_from_subscription( qcril_instance_id_e_type instance_id, pbm_phonebook_type *pb_id_ptr,
                                                       qcril_pbm_device_type device_type, boolean lock_ph_mutex );
#endif /* FEATURE_QCRIL_DSDS */

#endif /* QCRIL_PBM_H */
