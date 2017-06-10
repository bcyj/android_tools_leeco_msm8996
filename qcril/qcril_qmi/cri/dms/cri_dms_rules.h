
/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_DMS_RULES
#define CRI_DMS_RULES

#include "utils_common.h"

typedef struct cri_dms_rules_generic_rule_data_type
{
    int operating_mode;
}cri_dms_rules_generic_rule_data_type;

void cri_dms_rules_generic_rule_data_free_handler(void *rule_data);

int cri_dms_rules_set_modem_rule_check_handler(void *rule_data);



#endif
