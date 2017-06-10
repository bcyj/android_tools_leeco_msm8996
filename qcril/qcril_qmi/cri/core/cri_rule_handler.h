/***************************************************************************************************
    @file
    cri_rule_handler.h

    @brief
    Supports functions for performing operations on/using rules
    Rule is a set of conditions which need to be met for a callback to be called.
    Primary use of rules would be in abstracting the QMI transaction mechanism.
    QMI async response and QMI indication can be combined and the HLOS provided callback can be
    called only when the CRI cache meets all the conditions which were set while creating the rule.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef CRI_RULE_HANDLER
#define CRI_RULE_HANDLER

#include "utils_common.h"
#include "cri_core.h"

typedef struct cri_rule_handler_user_rule_info_type
{
    void *rule_data;
    int (*rule_check_handler)(void *rule_data);
    void (*rule_data_free_handler)(void *rule_data);
    void *cri_resp_util_data;
    void* (*cri_resp_data_calculator)(cri_core_error_type cri_core_error,
                                        void *cri_resp_util_data);
    void (*cri_resp_util_data_free_handler)(void *cri_resp_util_data);
    void (*cri_resp_data_free_handler)(void *cri_resp_data);
}cri_rule_handler_user_rule_info_type;

typedef struct cri_rule_handler_core_rule_info_type
{
    cri_core_context_type context;
    int timer_id;
    const void *hlos_resp_cb_data;
    hlos_resp_cb_type hlos_resp_cb;
    void *qmi_resp_data;
    void *qmi_cb_data;
}cri_rule_handler_core_rule_info_type;

typedef struct cri_rule_handler_rule_info_type
{
    cri_rule_handler_core_rule_info_type core_rule_info;
    cri_rule_handler_user_rule_info_type user_rule_info;
}cri_rule_handler_rule_info_type;





/***************************************************************************************************
    @function
    cri_rule_handler_init

    @brief
    Initializes the CRI rule framework. Needs to be called before performing operations
    on/using rules.

    @param[in]
        none

    @param[out]
        none

    @retval
    ESUCCESS if CRI rule framework has been successfully initialized,
    appropriate error code otherwise
***************************************************************************************************/
int cri_rule_handler_init();





/***************************************************************************************************
    @function
    cri_rule_handler_rule_add

    @brief
    Creates a rule with the given set of conditions.

    @param[in]
        cri_rule_handler_rule_info
            conditions and other related information to be used for creating the rule
        rule_timeout
            relative time before which the rule has to be met

    @param[out]
        none

    @retval
    ESUCCESS if rule has been successfully created, appropriate error code otherwise
***************************************************************************************************/
int cri_rule_handler_rule_add(cri_rule_handler_rule_info_type *cri_rule_handler_rule_info,
                              struct timeval *rule_timeout);





/***************************************************************************************************
    @function
    cri_rule_handler_rule_check

    @brief
    Checks If a rule has been met.
    Corresponding callback of the rule would be called If the rule has been met.

    @param[in]
        context
            context of the rule, If a specific rule needs to be checked (used in
            case of async responses),
            NIL otherwise (used in case of indications)
        cri_core_error
            calculated error for the rule, If a specific rule is being checked (used in
            case of async responses),
            QMI_ERR_NONE_V01 otherwise (used in case of indications)
        cri_resp_data
            calculated response data for the rule, If a specific rule is being checked (used in
            case of async responses),
            NULL otherwise (used in case of indications)

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_rule_handler_rule_check(cri_core_context_type context,
                                 cri_core_error_type cri_core_error,
                                 void *cri_resp_data);





/***************************************************************************************************
    @function
    cri_rule_handler_rule_delete

    @brief
    Deletes a specific rule.

    @param[in]
        cri_core_context
            context of the rule that needs to be deleted

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_rule_handler_rule_delete(cri_core_context_type cri_core_context);






/***************************************************************************************************
    @function
    cri_rule_handler_rule_delete_all

    @brief
    Deletes all existing rules.

    @param[in]
        none

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_rule_handler_rule_delete_all();


#endif
