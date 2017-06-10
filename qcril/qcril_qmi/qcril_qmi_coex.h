/******************************************************************************
  @file    qcril_qmi_coex.h
  @brief   qcril qmi - lte coex

  DESCRIPTION
    Header file for qcril_qmi_coex.c .

  ---------------------------------------------------------------------------

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#ifndef QCRIL_QMI_COEX_H
#define QCRIL_QMI_COEX_H


/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <errno.h>
#include <cutils/memory.h>
#include <cutils/properties.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "ril.h"
#include "IxErrno.h"
#include "comdef.h"
#include "qcrili.h"
#include "network_access_service_v01.h"
#include "qcril_other.h"
//#include "qcril_qmi_nas.h"



/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

#define QCRIL_QMI_COEX_INITIATE_FOR_DATA_CHECK   1
#define QCRIL_QMI_COEX_INITIATE_FOR_RF_CHECK     2
#define QCRIL_COEX_RD_BAND_INFO_LENGTH           (2)

typedef struct qcril_coex_rf_band_info_type
{
    nas_rf_band_info_type_v01* rf_band_info;
    nas_radio_if_enum_v01      rat;
} qcril_coex_rf_band_info_type;



void qcril_qmi_coex_init();
void qcril_qmi_coex_release();
void qcril_qmi_coex_process_rf_band_info
(
    qcril_coex_rf_band_info_type rf_band_info[QCRIL_COEX_RD_BAND_INFO_LENGTH],
    int                          rf_band_info_len
);

void qcril_qmi_coex_initiate_report_lte_info_to_riva(int reason);
void qcril_qmi_coex_terminate_riva_thread();


#endif /* QCRIL_QMI_VOICE_H */

