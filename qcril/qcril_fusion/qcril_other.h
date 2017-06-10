/*!
  @file
  qcril_other.h

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
08/11/09   nrn      Initial supporint code for NAM programming.

===========================================================================*/

#ifndef QCRIL_OTHER_H
#define QCRIL_OTHER_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/


/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

#define QCRIL_OTHER_CDMAMIN              1
#define QCRIL_OTHER_FMMIN                0
#define QCRIL_OTHER_IMSI_S1_0            16378855
#define QCRIL_OTHER_IMSI_S2_0            999
#define QCRIL_OTHER_IMSI_11_12_0         99
#define QCRIL_OTHER_NID_DEFAULTS         65535
#define QCRIL_OTHER_IMSI_MCC_0           999          /* 000 */
#define QCRIL_OTHER_IMSI_CLASS0_ADDR_NUM 0xFF
#define QCRIL_OTHER_PCH_A_DEFAULT        283          /* Primary Channel A Default */
#define QCRIL_OTHER_PCH_B_DEFAULT        384          /* Primary Channel B Default */
#define QCRIL_OTHER_SCH_A_DEFAULT        691          /* Secondary Channel A Default */
#define QCRIL_OTHER_SCH_B_DEFAULT        777          /* Secondary Channel B Default */
#define QCRIL_OTHER_NID_DEFAULTS         65535

#define QCRIL_OTHER_OEM_NAME_LENGTH      8            /* 8 bytes */
#define QCRIL_OTHER_OEM_REQUEST_ID_LEN   4            /* 4 bytes */
#define QCRIL_OTHER_OEM_REQUEST_DATA_LEN 4            /* 4 bytes */
#define QCRIL_OTHER_OEM_ITEMID_LEN       4            /* 4 bytes */
#define QCRIL_OTHER_OEM_ITEMID_DATA_LEN  4            /* 4 bytes */

/* Macro to check whether a CDMA band-class 0 channel is in the A side.
*/
#define QCRIL_OTHER_PRL_IS_IN_CHAN_CELLULAR_SYS_A( channel ) \
  ( ( ( channel ) >= 1   &&  ( channel ) <= 333 )  ||        \
    ( ( channel ) >= 667 &&  ( channel ) <= 716 )  ||        \
    ( ( channel ) >= 991 &&  ( channel ) <= 1323 ) )

/* Macro to check whether a CDMA band-class 0 channel is in the B side.
*/
#define QCRIL_OTHER_PRL_IS_IN_CHAN_CELLULAR_SYS_B( channel ) \
  ( ( ( channel ) >= 334 && ( channel ) <= 666 ) ||          \
    ( ( channel ) >= 717 && ( channel ) <= 799 ) )

typedef struct
{
  uint32 nv_item;
  uint32 nv_item_size;
  uint32 nv_item_offset;
  char * name;
} qcril_other_nv_table_entry_type;

boolean qcril_other_read_mdn_from_nv ( qcril_instance_id_e_type instance_id,
                                       qcril_modem_id_e_type modem_id,
                                       char *mdn_rpt_ptr );

boolean qcril_other_read_home_sid_nid_from_nv ( qcril_instance_id_e_type instance_id,
                                                qcril_modem_id_e_type modem_id,
                                                char *sid_rpt_ptr,
                                                char *nid_rpt_ptr );

boolean qcril_other_read_min_from_nv ( qcril_instance_id_e_type instance_id,
                                       qcril_modem_id_e_type modem_id,
                                       char *min_rpt_ptr );

boolean qcril_other_read_managed_roaming_from_nv( qcril_instance_id_e_type instance_id,
                                                  qcril_modem_id_e_type modem_id );
#endif /* QCRIL_OTHER_H */
