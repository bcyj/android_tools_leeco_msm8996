/***************************************************************************************************
    @file
    cri_csvt_rules.c

    @brief
    Implements functions supported in cri_csvt_rules.h.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#include "cri_csvt_rules.h"
#include "cri_csvt_utils.h"






/***************************************************************************************************
    @function
    cri_csvt_rules_originating_rule_check_handler

    @implementation detail
    None.
***************************************************************************************************/
int cri_csvt_rules_originating_rule_check_handler(void *rule_data)
{
    int iter_call_object;
    int ret_code;
    cri_csvt_rules_generic_rule_data_type *cri_csvt_rules_generic_rule_data;

    iter_call_object = NIL;
    ret_code = FALSE;
    cri_csvt_rules_generic_rule_data = NULL;

    if(rule_data)
    {
        cri_csvt_rules_generic_rule_data = (cri_csvt_rules_generic_rule_data_type*) rule_data;
        for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
        {
            if(TRUE == csvt_calls[iter_call_object].is_valid &&
               (cri_csvt_rules_generic_rule_data->hlos_call_id ==
               csvt_calls[iter_call_object].hlos_call_id) &&
               (csvt_calls[iter_call_object].csvt_info.event_type == CSVT_EVENT_TYPE_CONFIRM_V01 ||
               csvt_calls[iter_call_object].csvt_info.event_type == CSVT_EVENT_TYPE_END_V01))
            {
                ret_code = TRUE;
                break;
            }
        }
    }

    return ret_code;
}

/***************************************************************************************************
    @function
    cri_csvt_rules_answering_rule_check_handler

    @implementation detail
    None.
***************************************************************************************************/
int cri_csvt_rules_answering_rule_check_handler(void *rule_data)
{
    int iter_call_object;
    int ret_code;
    cri_csvt_rules_generic_rule_data_type *cri_csvt_rules_generic_rule_data;

    iter_call_object = NIL;
    ret_code = FALSE;
    cri_csvt_rules_generic_rule_data = NULL;

    if(rule_data)
    {
        cri_csvt_rules_generic_rule_data = (cri_csvt_rules_generic_rule_data_type*) rule_data;
        for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
        {
            if(TRUE == csvt_calls[iter_call_object].is_valid &&
               (cri_csvt_rules_generic_rule_data->hlos_call_id ==
               csvt_calls[iter_call_object].hlos_call_id) &&
               (csvt_calls[iter_call_object].csvt_info.event_type == CSVT_EVENT_TYPE_CONNECT_V01 ||
               csvt_calls[iter_call_object].csvt_info.event_type == CSVT_EVENT_TYPE_END_V01)
               )
            {
                ret_code = TRUE;
                break;
            }
        }
    }

    return ret_code;
}

/***************************************************************************************************
    @function
    cri_csvt_rules_ending_rule_check_handler

    @implementation detail
    None.
***************************************************************************************************/
int cri_csvt_rules_ending_rule_check_handler(void *rule_data)
{
    int iter_call_object;
    int ret_code;
    cri_csvt_rules_generic_rule_data_type *cri_csvt_rules_generic_rule_data;

    iter_call_object = NIL;
    ret_code = FALSE;
    cri_csvt_rules_generic_rule_data = NULL;

    if(rule_data)
    {
        cri_csvt_rules_generic_rule_data = (cri_csvt_rules_generic_rule_data_type*) rule_data;
        for(iter_call_object = 0; iter_call_object < CRI_CSVT_MAX_CALLS; iter_call_object++)
        {
            if(TRUE == csvt_calls[iter_call_object].is_valid &&
               (cri_csvt_rules_generic_rule_data->hlos_call_id ==
               csvt_calls[iter_call_object].hlos_call_id) &&
               csvt_calls[iter_call_object].csvt_info.event_type == CSVT_EVENT_TYPE_END_V01
               )
            {
                ret_code = TRUE;
                break;
            }
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
void cri_csvt_rules_generic_rule_data_free_handler(void *rule_data)
{
    cri_csvt_rules_generic_rule_data_type *cri_csvt_rules_generic_rule_data;

    cri_csvt_rules_generic_rule_data = NULL;

    if(rule_data)
    {
        cri_csvt_rules_generic_rule_data = (cri_csvt_rules_generic_rule_data_type*) rule_data;
        util_memory_free((void**) &cri_csvt_rules_generic_rule_data);
    }
}

