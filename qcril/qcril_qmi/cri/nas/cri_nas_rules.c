/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_nas_rules.h"
#include "cri_nas_utils.h"
#include "cri_nas.h"
#include "cri_nas_core.h"

int cri_nas_rules_rule_check_handler(void *rule_data)
{
    int ret_code = TRUE;
    return ret_code;
}

int cri_nas_rules_pref_mode_rule_check_handler(void *rule_data)
{
    int ret_code = FALSE;
    uint32_t current_mode_pref;
    cri_nas_rules_pref_mode_rule_data_type *pref_mode_rule_data = NULL;
    pref_mode_rule_data = (cri_nas_rules_pref_mode_rule_data_type*)rule_data;

    if ( cri_nas_sys_sel_pref_info.mode_pref_valid )
    {
        current_mode_pref = cri_nas_sys_sel_pref_info.mode_pref;
        if ( current_mode_pref == pref_mode_rule_data->pref_mode )
        {
            ret_code = TRUE;
        }
    }
    return ret_code;
}


int cri_nas_nw_selection_rule_check_handler(void *rule_data)
{
    uint32_t mcc;
    uint32_t mnc;
    uint8_t ret = FALSE;
    uint8_t is_nw_sel = FALSE;
    uint8_t is_current = FALSE;
    uint8_t is_registered = FALSE;
    cri_nas_nw_select_state_type nw_sel_state;
    is_nw_sel = cri_nas_is_nw_selection_in_progress();

    if ( is_nw_sel )
    {
        if ( cri_nas_nw_selection_info.is_automatic == FALSE )
        {
            mcc = cri_nas_nw_selection_info.mcc;
            mnc = cri_nas_nw_selection_info.mnc;
            is_current = cri_nas_is_current_plmn(mcc,mnc);
            if ( is_current )
            {
                cri_nas_nw_selection_set_state(CRI_NAS_NW_SELECT_DONE);
                ret = TRUE;
            }
        }
        else
        {
            is_registered = cri_nas_is_considered_registered();
            if ( is_registered )
            {
                cri_nas_nw_selection_set_state(CRI_NAS_NW_SELECT_DONE);
                ret = TRUE;
            }
        }
    }
    return ret;
}


void cri_nas_rules_pref_mode_rule_data_free_handler(void *rule_data)
{
    cri_nas_rules_pref_mode_rule_data_type *cri_nas_rule_data;

    cri_nas_rule_data = NULL;

    if(rule_data)
    {
        cri_nas_rule_data = (cri_nas_rules_pref_mode_rule_data_type*) rule_data;
        util_memory_free((void**) &cri_nas_rule_data);
    }
}



void cri_nas_rules_generic_rule_data_free_handler(void *rule_data)
{
    cri_nas_rules_generic_rule_data_type *cri_nas_rules_generic_rule_data;

    cri_nas_rules_generic_rule_data = NULL;

    if(rule_data)
    {
        cri_nas_rules_generic_rule_data = (cri_nas_rules_generic_rule_data_type*) rule_data;
        util_memory_free((void**) &cri_nas_rules_generic_rule_data);
    }
}



