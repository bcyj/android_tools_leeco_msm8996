/*!
  @file
  qcril_pbmi.h

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_pbmi.h# $

when        who     what, where, why
--------   ---     ----------------------------------------------------------
06/01/09   sk       Initial framework

===========================================================================*/

#ifndef QCRIL_PBMI_H
#define QCRIL_PBMI_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "ril.h"
#include "IxErrno.h"
#include "qcrili.h"
#include "pbmlib.h"


/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

  /* (5 in SIM + 2 Hardcoded when sim is present ) or
  10 hardcoded numbers considering both CDMA, GW when sim is absent
  (911,112,119,118,000,999,08,110,*911,#911) */
#define QCRIL_PBM_MAX_CARD_HCODED_ECC_NUMBERS  10

typedef struct
{
  boolean registered;
  boolean client_is_primary;  /*! Indicates whether this client info is associated with primary QCRIL instance */
} qcril_pbm_client_info_type;

typedef struct
{
  boolean card_ecc_is_present;

  int  num_nv_ecc_entries;
  char nv_ecc[ PBM_NV_EMERGENCY_NUMBERS ][ PBM_NUM_SIZE ];

  int num_card_hcoded_ecc_entries;
  char card_hcoded_ecc[ QCRIL_PBM_MAX_CARD_HCODED_ECC_NUMBERS ][ PBM_NUM_SIZE ];

  int num_ota_ecc_entries;
  char ota_ecc[ CM_MAX_EMERGENCY_NUM_COUNT ][ CM_MAX_NUMBER_CHARS + 1 ]; 
} qcril_pbm_ecc_info_type;

typedef struct
{
  qcril_modem_id_e_type ecc_modem_id;
  qcril_pbm_client_info_type client_info[ QCRIL_MAX_MODEM_ID ];
  qcril_pbm_ecc_info_type ecc_info[ QCRIL_MAX_MODEM_ID ];
  boolean oprt_mode_check_enabled; /*! Indicates whether operating mode should be checked before honoring PBM event processing */
} qcril_pbm_struct_type;

#endif /* QCRIL_PBMI_H */


