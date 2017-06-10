
/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_DMS_UTILS
#define CRI_DMS_UTILS

#include "utils_common.h"
#include "cri_core.h"
#include "device_management_service_v01.h"

// DMS cache
typedef struct cri_dms_operating_mode
{
    int is_valid;
    dms_operating_mode_enum_v01 current_operating_mode;
}cri_dms_operating_mode;

qmi_error_type_v01 cri_dms_utils_init_client(hlos_ind_cb_type hlos_ind_cb);

qmi_error_type_v01 cri_dms_init_client_state();

void cri_dms_utils_release_client(int qmi_service_client_id);

void cri_dms_utils_update_operating_mode(dms_operating_mode_enum_v01 opr_mode);

uint32_t cri_dms_utils_is_valid_operating_mode();

void cri_dms_utils_get_current_operating_mode(dms_operating_mode_enum_v01 *opr_mode);


#endif

