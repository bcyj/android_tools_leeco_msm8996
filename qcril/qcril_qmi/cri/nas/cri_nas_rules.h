
/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_NAS_RULES
#define CRI_NAS_RULES

#include "utils_common.h"

typedef struct cri_nas_rules_generic_rule_data_type
{
    int dummy_var;
}cri_nas_rules_generic_rule_data_type;

typedef struct cri_nas_rules_pref_mode_rule_data_type
{
    uint32_t pref_mode;
}cri_nas_rules_pref_mode_rule_data_type;


extern int cri_nas_rules_rule_check_handler(void *rule_data);

extern int cri_nas_rules_pref_mode_rule_check_handler(void *rule_data);

extern int cri_nas_nw_selection_rule_check_handler(void *rule_data);

extern void cri_nas_rules_generic_rule_data_free_handler(void *rule_data);

extern void cri_nas_rules_pref_mode_rule_data_free_handler(void *rule_data);

#endif
