/******************************************************************************
  @file    qcril_qmi_nas2.h
  @brief   qcril qmi - NAS 2nd portion

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI NAS.

  ---------------------------------------------------------------------------

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#ifndef QCRIL_QMI_NAS2_H
#define QCRIL_QMI_NAS2_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "comdef.h"
#include "qmi_client.h"
#include "network_access_service_v01.h"

/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

#define QMI_NAS_RAT_MODE_PREF_CDMA                          ( 1 << QMI_NAS_RAT_MODE_PREF_CDMA2000_1X_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_HRPD                          ( 1 << QMI_NAS_RAT_MODE_PREF_CDMA2000_HRPD_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_GSM                           ( 1 << QMI_NAS_RAT_MODE_PREF_GSM_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_UMTS                          ( 1 << QMI_NAS_RAT_MODE_PREF_UMTS_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_TDSCDMA                       ( 1 << QMI_NAS_RAT_MODE_PREF_TDSCDMA_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_LTE                           ( 1 << QMI_NAS_RAT_MODE_PREF_LTE_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_CDMA_HRPD                     ( QMI_NAS_RAT_MODE_PREF_CDMA + QMI_NAS_RAT_MODE_PREF_HRPD )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS                      ( QMI_NAS_RAT_MODE_PREF_GSM + QMI_NAS_RAT_MODE_PREF_UMTS )
#define QMI_NAS_RAT_MODE_PREF_UMTS_TDSCDMA                  ( QMI_NAS_RAT_MODE_PREF_UMTS + QMI_NAS_RAT_MODE_PREF_TDSCDMA )
#define QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA                   ( QMI_NAS_RAT_MODE_PREF_GSM + QMI_NAS_RAT_MODE_PREF_TDSCDMA )
#define QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_LTE               ( QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA + QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA              ( QMI_NAS_RAT_MODE_PREF_GSM + QMI_NAS_RAT_MODE_PREF_UMTS + QMI_NAS_RAT_MODE_PREF_TDSCDMA )
#define QMI_NAS_RAT_MODE_PREF_UMTS_LTE                      ( QMI_NAS_RAT_MODE_PREF_UMTS + QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_TDSCDMA_LTE                   ( QMI_NAS_RAT_MODE_PREF_TDSCDMA + QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_LTE                  ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS + QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_LTE          ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS + QMI_NAS_RAT_MODE_PREF_TDSCDMA + QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_UMTS_TDSCDMA_LTE              ( QMI_NAS_RAT_MODE_PREF_UMTS + QMI_NAS_RAT_MODE_PREF_TDSCDMA_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_CDMA_HRPD            ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS + QMI_NAS_RAT_MODE_PREF_CDMA_HRPD )
#define QMI_NAS_RAT_MODE_PREF_CDMA_HRPD_LTE                 ( QMI_NAS_RAT_MODE_PREF_CDMA_HRPD + QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_CDMA_HRPD_LTE             ( QMI_NAS_RAT_MODE_PREF_GSM + QMI_NAS_RAT_MODE_PREF_CDMA_HRPD + QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_CDMA_HRPD_LTE        ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS_LTE + QMI_NAS_RAT_MODE_PREF_CDMA_HRPD )
#define QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_CDMA_HRPD_LTE     ( QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_LTE + QMI_NAS_RAT_MODE_PREF_CDMA_HRPD )
#define QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_CDMA_HRPD_UMTS     ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA + QMI_NAS_RAT_MODE_PREF_CDMA_HRPD )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_LTE + QMI_NAS_RAT_MODE_PREF_CDMA_HRPD )



void qcril_qmi_nas2_find_startic_operator_name
(
  char * mcc_str,
  char * mnc_str,
  char * mcc_mnc_str_ref,
  char **long_ons_ptr,
  char **short_ons_ptr
);

int qcril_qmi_nas2_find_3gpp2_static_operator_name
(
  char * mcc_str,
  char * mnc_str,
  uint16_t sid,
  uint16_t nid,
  char **long_ons_ptr,
  char **short_ons_ptr
);

void qcril_qmi_nas2_find_elaboration_static_name( char * mcc_mnc_str, char** long_name_res, char ** short_name_res );

void qcril_qmi_nas2_set_max_transmit_power
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas2_get_sar_rev_key
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_set_initial_attach_apn
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

mode_pref_mask_type_v01 qcril_qmi_nas2_convert_rat_to_mode_pref(int rat);

nas_radio_if_enum_v01
    qcril_qmi_nas2_convert_qcril_rat_to_qmi_rat(RIL_RadioTechnology qcril_rat);

RIL_Errno qmi_ril_nwreg_request_mode_pref( int android_mode_pref, uint8 *is_change );

char* qcril_qmi_nas2_retrieve_mcc_from_iccid(char *iccid);

unsigned int qcril_qmi_nas_get_radio_tech(uint16_t mode_pref);

void qcril_qmi_nas_request_set_preferred_network_band_pref
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_request_set_preferred_network_acq_order
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_request_get_preferred_network_band_pref
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_request_get_preferred_network_acq_order(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qmi_ril_nas_cache_deferred_acq_order
(
    uint32_t acq_order_len,
    qcril_qmi_acq_order_e_type acq_order_map,
    nas_radio_if_enum_v01 *acq_order
);
uint8_t qmi_ril_nas_get_deferred_acq_order_map( qcril_qmi_acq_order_e_type *acq_order_map );
uint8_t qcril_qmi_nas_get_gw_acq_order_pref (uint16_t *gw_acq_order_pref);
uint8_t qcril_qmi_nas_get_acq_order(uint32_t *acq_order_len, nas_radio_if_enum_v01 *acq_order);
uint8_t qmi_ril_nas_get_deferred_acq_order( uint32_t *acq_order_len, nas_radio_if_enum_v01 *acq_order );
uint8 qcril_qmi_nas_check_is_indication_received();
RIL_Errno qcril_qmi_nas2_create_reqlist_setup_timer_helper( const qcril_request_params_type *const params_ptr );

#endif /* QCRIL_QMI_NAS2_H */

