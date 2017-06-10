/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_dms_rules.h"
#include "cri_dms_utils.h"




/***************************************************************************************************
    @function
    cri_dms_rules_originating_rule_check_handler

    @implementation detail
    None.
***************************************************************************************************/
int cri_dms_rules_set_modem_rule_check_handler(void *rule_data)
{
    int ret_code;
    dms_operating_mode_enum_v01 opr_mode;
    cri_dms_rules_generic_rule_data_type *cri_dms_rules_generic_rule_data;

    ret_code = FALSE;
    cri_dms_rules_generic_rule_data = (cri_dms_rules_generic_rule_data_type*)rule_data;

    if(rule_data)
    {
        cri_dms_utils_get_current_operating_mode(&opr_mode);
        if ( opr_mode == cri_dms_rules_generic_rule_data->operating_mode )
        {
            ret_code = TRUE;
        }
        else
        {
            ret_code = FALSE;
        }
    }

    return ret_code;
}


/***************************************************************************************************
    @function
    cri_csvt_rules_generic_rule_data_free_handler

    @implementation detail
    None.
***************************************************************************************************/
void cri_dms_rules_generic_rule_data_free_handler(void *rule_data)
{
    cri_dms_rules_generic_rule_data_type *cri_dms_rules_generic_rule_data;

    cri_dms_rules_generic_rule_data = NULL;

    if(rule_data)
    {
        cri_dms_rules_generic_rule_data = (cri_dms_rules_generic_rule_data_type*) rule_data;
        util_memory_free((void**) &cri_dms_rules_generic_rule_data);
    }
}



