/***************************************************************************************************
    @file
    cri_csvt_rules.h

    @brief
    Supports QMI CSVT rule related functions for abstracting QMI mechanism.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef CRI_CSVT_RULES
#define CRI_CSVT_RULES

#include "utils_common.h"

typedef struct cri_csvt_rules_generic_rule_data_type
{
    int hlos_call_id;
}cri_csvt_rules_generic_rule_data_type;






/***************************************************************************************************
    @function
    cri_csvt_rules_originating_rule_check_handler

    @brief
    Checks whether a QMI CSVT call with a specific HLOS call id has been
    originated or not.

    @param[in]
        rule_data
            pointer to rule data that is being used to check whether a QMI CSVT call with
            a specific HLOS call id has been originated or not

    @param[out]
        none

    @retval
    TRUE if a QMI CSVT call with a specific HLOS call id has been originated, FALSE otherwise
***************************************************************************************************/
int cri_csvt_rules_originating_rule_check_handler(void *rule_data);





/***************************************************************************************************
    @function
    cri_csvt_rules_answering_rule_check_handler

    @brief
    Checks whether a QMI CSVT call with a specific HLOS call id has been
    answered or not.

    @param[in]
        rule_data
            pointer to rule data that is being used to check whether a QMI CSVT call with
            a specific HLOS call id has been answered or not

    @param[out]
        none

    @retval
    TRUE if a QMI CSVT call with a specific HLOS call id has been answered, FALSE otherwise
***************************************************************************************************/
int cri_csvt_rules_answering_rule_check_handler(void *rule_data);




/***************************************************************************************************
    @function
    cri_csvt_rules_ending_rule_check_handler

    @brief
    Checks whether a QMI CSVT call with a specific HLOS call id has been
    ended or not.

    @param[in]
        rule_data
            pointer to rule data that is being used to check whether a QMI CSVT call with
            a specific HLOS call id has been ended or not

    @param[out]
        none

    @retval
    TRUE if a QMI CSVT call with a specific HLOS call id has been ended, FALSE otherwise
***************************************************************************************************/
int cri_csvt_rules_ending_rule_check_handler(void *rule_data);





/***************************************************************************************************
    @function
    cri_csvt_rules_generic_rule_data_free_handler

    @brief
    Frees rule related data.

    @param[in]
        rule_data
            pointer to rule data that was used for checking if corresponding rule has been met

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_csvt_rules_generic_rule_data_free_handler(void *rule_data);

#endif
