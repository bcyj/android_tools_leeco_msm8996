/*!
  @file
  qcril_cm_ons.h

  @brief
  Encapsulates the information related to operator name display.

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_cm_ons.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/01/10   fc      Re-architecture to support split modem.
02/26/09   fc      First cut.


===========================================================================*/

#ifndef QCRIL_CM_ONS_H
#define QCRIL_CM_ONS_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "IxErrno.h"
#include "comdef.h"
#include "cm.h"
#include "qcrili.h"
#include "qcril_cm.h"


/*===========================================================================

                      DEFINITIONS AND TYPES

===========================================================================*/

/* Maximum length of MCCMNC ASCII representation */
#define QCRIL_CM_ONS_MCC_MNC_ASCII_MAX_LEN              8

/* Maximum ONS length */
#define QCRIL_CM_ONS_MAX_LENGTH                         300


/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/

void qcril_cm_ons_init( void );

void qcril_cm_ons_convert_mcc_mnc_to_bcd( const char *ascii_mcc_mnc, uint8 len, sys_plmn_id_s_type *plmn_ptr );

boolean qcril_cm_ons_match_plmn( sys_plmn_id_s_type  plmn_1, sys_plmn_id_s_type  plmn_2 );

void qcril_cm_ons_store_nitz( qcril_instance_id_e_type instance_id, const qcril_cm_ss_info_type *ss_info_ptr );

void qcril_cm_ons_lookup_current_operator( qcril_instance_id_e_type instance_id, const qcril_cm_ss_info_type *ss_info_ptr, 
                                           char **long_ons_ptr, char **short_ons_ptr, char **mcc_mnc_ptr ); 

boolean qcril_cm_ons_lookup_available_operator( qcril_instance_id_e_type instance_id, const qcril_cm_ss_info_type *ss_info_ptr, 
                                                const sys_plmn_id_s_type plmn, char **long_ons_ptr,
                                                char **short_ons_ptr, char **mcc_mnc_ptr );

#endif /* QCRIL_CM_ONS_H */
