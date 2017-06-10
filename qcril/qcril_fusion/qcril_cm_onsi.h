/*!
  @file
  qcril_cm_onsi.h

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_cm_onsi.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/01/10   fc      Re-architecture to support split modem.
04/05/09   fc      Cleanup log macros.
02/26/09   fc      First cut.


===========================================================================*/

#ifndef QCRIL_CM_ONSI_H
#define QCRIL_CM_ONSI_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <pthread.h>
#include "cm.h"
#include "qcril_cm_ons.h"


/*===========================================================================

                      DEFINITIONS AND TYPES

===========================================================================*/

/* NITZ ONS Persistent System Properties */ 
#define QCRIL_CM_ONS_NITZ_PLMN                "persist.radio.nitz_plmn_" 
#define QCRIL_CM_ONS_NITZ_LONS_0              "persist.radio.nitz_lons_0_" 
#define QCRIL_CM_ONS_NITZ_LONS_1              "persist.radio.nitz_lons_1_" 
#define QCRIL_CM_ONS_NITZ_LONS_2              "persist.radio.nitz_lons_2_" 
#define QCRIL_CM_ONS_NITZ_LONS_3              "persist.radio.nitz_lons_3_" 
#define QCRIL_CM_ONS_NITZ_SONS_0              "persist.radio.nitz_sons_0_" 
#define QCRIL_CM_ONS_NITZ_SONS_1              "persist.radio.nitz_sons_1_" 
#define QCRIL_CM_ONS_NITZ_SONS_2              "persist.radio.nitz_sons_2_" 
#define QCRIL_CM_ONS_NITZ_SONS_3              "persist.radio.nitz_sons_3_" 

#define QCRIL_CM_ONS_7BIT_MASK                     0x7F

/* Replacement character (space) specified by 23.038 */
#define QCRIL_CM_ONS_REPLACEMENT_CHAR              0x20

/* International Reference Alphabet T.50 */
#define QCRIL_CM_ONS_ALPHA_IRA                     0

/* minimum and maximum acceptable value for characters in these alphabets:
   0 value cannot be accepted for NULL terminated strings used throughout
   ATCOP and 0x7F is max possible in 7 bit alphabet */
#define QCRIL_CM_ONS_MIN_IRA_GSM_CHAR              0x01
#define QCRIL_CM_ONS_MAX_GSM_CHAR                  0x7F

/*---------------------------------------------------------------------------
     Length of a single character in each of these character sets:
     e.g. 4 hexadecimal characters to represent a single UCS2 character, etc.
---------------------------------------------------------------------------*/
#define QCRIL_CM_ONS_UCS2_CHAR_LEN                 4
#define QCRIL_CM_ONS_HEX_CHAR_LEN                  2
#define QCRIL_CM_ONS_IRA_CHAR_LEN                  1

#define QCRIL_CM_ONS_MAX_VAL_NUM_ITEM              0xFFFFFFFF 

/*! @brief Memory List Entry
*/
typedef struct
{
  uint32 mcc;                          /* Mobile Network Code */
  char mnc[4];                        /* Mobile Country Code (convert to string to differenciate between 2 and 3 digit mnc*/
  char *short_name_ptr;                /* Pointer to a null terminated string containing the network's short name */
  char *full_name_ptr;                 /* Pointer to a null terminated string containing the network's full name */
} qcril_cm_ons_memory_entry_type;


/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

/*! @brief Structure used to cache QCRIL ONS data
*/
typedef struct
{
  boolean nitz_available;                         /*! Indicates whether NITZ info is available */
  char nitz_long_ons[ QCRIL_CM_ONS_MAX_LENGTH ];  /*! Buffer long ONS of the last received NITZ */
  char nitz_short_ons[ QCRIL_CM_ONS_MAX_LENGTH ]; /*! Buffer short ONS of the last received NITZ */
  pthread_mutex_t nitz_mutex;                     /*! Mutex to control access/update to NITZ info */
} qcril_cm_ons_struct_type;

#endif /* QCRIL_CM_ONSI_H */
