/******************************************************************************
  @file    qcri_otheri.h
  @brief   qcril qmi - misc

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2008-2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef QCRIL_OTHERI_H
#define QCRIL_OTHERI_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "ril.h"
#include "qcrili.h"
#include "qcril_log.h"
#include "qmi.h"
#include "qmi_idl_lib.h"

#include "services/qmi_embms_v01.h"
#include "services/qtuner_v01.h"
#include "qmi_ims_vt_v01.h"
#include "ip_multimedia_subsystem_presence_v01.h"

/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

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
  QCRIL_OTHER_CDMA_SUBSCRIPTION_INFO_MAX
} qcril_other_cdma_subscription_info_e_type;

/*! @brief Structure used to compose response to RIL request
*/

/*! @brief Structure used to cache QCRIL OTHER data
*/
typedef struct
{
  qcril_other_mute_e_type uplink_mute_setting;         /*!< Uplink mute setting */
  uint8 curr_nam;                                      /*!< Current NAM */
} qcril_other_struct_type;

qmi_idl_service_object_type qmi_ril_oem_hook_qmi_idl_tunneling_get_service_object( qmi_ril_oem_hook_qmi_tunneling_service_id_type service_id );

#endif /* QCRIL_OTHERI_H */
