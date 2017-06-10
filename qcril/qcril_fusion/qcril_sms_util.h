/*!
  @file
  qcril_sms_util.h

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_sms_util.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/02/09   sb      Added support for getting and setting the SMSC address.
04/04/09   fc      Cleanup log macros.
07/15/08   sb      First cut implementation.


===========================================================================*/

#ifndef QCRIL_SMS_UTIL_H
#define QCRIL_SMS_UTIL_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "comdef.h"
#include "cm.h"
#include "wms.h"


/*===========================================================================

                   EXTERNAL DEFINITIONS AND TYPES

===========================================================================*/

/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/

void qcril_sms_ts_convert_to_bcd_address
(
  const wms_address_s_type          *addr_ptr,      // INPUT
  cm_called_party_bcd_no_T            *bcd_addr_ptr   // OUTPUT
);

void qcril_sms_ts_convert_from_bcd_address
(
  const cm_called_party_bcd_no_T      *bcd_addr_ptr,   // INPUT
  wms_address_s_type                *addr_ptr        // OUTPUT
);

void qcril_sms_hex_to_byte
(
  const char * hex_pdu,   // INPUT
  byte * byte_pdu,        // OUTPUT
  uint32 num_hex_chars
);

void qcril_sms_byte_to_hex
(
  byte * byte_pdu,   // INPUT
  char * hex_pdu,    // OUTPUT
  uint32 num_bytes
);

const char *qcril_sms_lookup_cmd_name
(
  wms_cmd_id_e_type cmd_id
);

boolean qcril_sms_convert_smsc_address_to_wms_format
(
const char * input_smsc_address,
wms_address_s_type * output_smsc_address
);

boolean qcril_sms_convert_smsc_address_to_ril_format
(
const wms_address_s_type * input_smsc_address_ptr,
char * output_smsc_address_ptr
);

#endif /* QCRIL_SMS_UTIL_H */
