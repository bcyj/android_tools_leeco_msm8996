/*!
  @file
  qcri_otheri.h

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_otheri.h#9 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
07/22/09   pg      Added support for PRL version.
                   Get MDN value from NV_DIR_NUMBER_PCS_I instead of NV_DIR_NUMBER_I.
                   Added support to return the whole home SID/NID list under FEATURE_NEW_RIL_API. 
05/30/09   pg      Fixed get MDN implementation.
05/18/09   fc      Changes to log debug messages to Diag directly instead
                   of through logcat.
05/14/09   pg      Added support for CDMA phase II under FEATURE_MULTIMODE_ANDROID_2.
04/28/09   fc      Standardize xxx_enum_type to xxx_e_type.
04/05/09   fc      Cleanup log macros.
01/26/08   fc      Logged assertion info.
11/05/08   fc      Added support for RIL_REQUEST_GET_MUTE.
10/06/08   pg      Added support for RIL_REQUEST_CDMA_SUBSCRIPTION.
09/22/08   pg      Do not cache ESN and MEID any more.  They should be read
                   from NV each time since their values are different with
                   different CDMA scriptions.
08/18/08   pg      Added support for GET BASEBAND_VERSION.
08/04/08   pg      Added support for GET IMEI, IMEISV.
07/28/08   fc      First cut implementation.

===========================================================================*/

#ifndef QCRIL_OTHERI_H
#define QCRIL_OTHERI_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "ril.h"
#include "qcrili.h"
#include "qcril_log.h"


/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

/* Maximum length of NV or SIM items */
#define QCRIL_OTHER_IMEI_ASCII_MAX_LEN             ( NV_UE_IMEI_SIZE - 1 ) * 2
#define QCRIL_OTHER_IMEISV_ASCII_MAX_LEN           3
#define QCRIL_OTHER_BASEBAND_VERSION_ASCII_MAX_LEN ( NV_MAX_SW_VERSION_INFO_SIZ + 1 )
#define QCRIL_OTHER_ESN_ASCII_MAX_LEN              9
#define QCRIL_OTHER_MEID_ASCII_MAX_LEN             15
#define QCRIL_OTHER_AKEY_ASCII_MAX_LEN             26
#define QCRIL_OTHER_MDN_ASCII_MAX_LEN              NV_DIR_NUMB_PCS_SIZ + 1 
#define QCRIL_OTHER_SID_NID_ASCII_MAX_LEN          6
#define QCRIL_OTHER_MIN_ASCII_MAX_LEN              11
#define QCRIL_OTHER_SID_NID_LIST_ASCII_MAX_LEN     QCRIL_OTHER_SID_NID_ASCII_MAX_LEN * NV_MAX_HOME_SID_NID + 1

/* Mute setting */
typedef enum
{
  QCRIL_OTHER_MUTE_DISABLED,
  QCRIL_OTHER_MUTE_ENABLED
} qcril_other_mute_e_type;

/* Device ID */
typedef enum
{
  QCRIL_OTHER_DEVICE_ID_IMEI,
  QCRIL_OTHER_DEVICE_ID_IMEISV,
  QCRIL_OTHER_DEVICE_ID_ESN,
  QCRIL_OTHER_DEVICE_ID_MEID,
  QCRIL_OTHER_DEVICE_ID_MAX
} qcril_other_device_id_e_type;

/* CDMA subscription info */
typedef enum
{
  QCRIL_OTHER_CDMA_SUBSCRIPTION_INFO_MDN,
  QCRIL_OTHER_CDMA_SUBSCRIPTION_INFO_H_SID,
  QCRIL_OTHER_CDMA_SUBSCRIPTION_INFO_H_NID,
  QCRIL_OTHER_CDMA_SUBSCRIPTION_INFO_MIN,
  QCRIL_OTHER_CDMA_SUBSCRIPTION_INFO_PRL_VER,
  QCRIL_OTHER_CDMA_SUBSCRIPTION_INFO_MAX
} qcril_other_cdma_subscription_info_e_type;

/*! @brief Structure used to compose response to RIL request
*/
typedef struct
{
  char imei[ QCRIL_OTHER_IMEI_ASCII_MAX_LEN ]; 
} qcril_other_imei_type;

typedef struct
{
  char imeisv[ QCRIL_OTHER_IMEISV_ASCII_MAX_LEN ];
} qcril_other_imeisv_type;

typedef struct
{
  char baseband_version[ QCRIL_OTHER_BASEBAND_VERSION_ASCII_MAX_LEN ];
} qcril_other_baseband_version_type;

typedef struct
{
  char *device_identity[ QCRIL_OTHER_DEVICE_ID_MAX ];
  char imei[ QCRIL_OTHER_IMEI_ASCII_MAX_LEN ]; 
  char imeisv[ QCRIL_OTHER_IMEISV_ASCII_MAX_LEN ];
  char esn[ QCRIL_OTHER_ESN_ASCII_MAX_LEN ];
  char meid[ QCRIL_OTHER_MEID_ASCII_MAX_LEN ];
} qcril_other_device_identity_type;

typedef struct
{
  char *cdma_subscription[QCRIL_OTHER_CDMA_SUBSCRIPTION_INFO_MAX];
  char mob_dir_number[ QCRIL_OTHER_MDN_ASCII_MAX_LEN ];
  char home_sid[ QCRIL_OTHER_SID_NID_LIST_ASCII_MAX_LEN ];
  char home_nid[ QCRIL_OTHER_SID_NID_LIST_ASCII_MAX_LEN ];
  char min_s[ QCRIL_OTHER_MIN_ASCII_MAX_LEN ];
  char prl_ver[ QCRIL_CM_PRL_VERSION_ASCII_MAX_LEN ];
} qcril_other_cdma_subscription_type;

/*! @brief Structure used to cache QCRIL OTHER data
*/
typedef struct
{
  qcril_other_mute_e_type uplink_mute_setting;         /*!< Uplink mute setting */
  uint8 curr_nam;                                      /*!< Current NAM */
} qcril_other_struct_type;

#endif /* QCRIL_OTHERI_H */
