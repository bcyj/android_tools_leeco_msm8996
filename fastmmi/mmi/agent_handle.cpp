/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "agent.h"

/**
*    Handle INIT command
*/
int handle_init(mmi_module_t * module, msg_t * req, msg_t * resp) {

    int ret = 0;

    hash_map < string, string > params;

    if(module == NULL || req == NULL || resp == NULL) {
        ALOGE("%s: Invalid param, NULL pointer", __FUNCTION__);
        return -1;
    }

    ALOGI("%s: %s start ", __FUNCTION__, req->module);

    /** Get the input params for module */
    parse_parameter(req->msg, params);
    params[KEY_MODULE_NAME] = req->module;
    ret = module->methods->module_init(module, params);

    /** Store the result to response msg */
    resp->result = ret;

    ALOGI("%s: %s finished with ret = %d", __FUNCTION__, req->module, ret);

    return ret;
}

/**
*    Handle DEINIT command
*/
int handle_deinit(mmi_module_t * module, msg_t * req, msg_t * resp) {

    int ret = 0;

    if(module == NULL || req == NULL || resp == NULL) {
        ALOGE("%s: Invalid param, NULL pointer", __FUNCTION__);
        return -1;
    }
    ALOGI("%s: %s start ", __FUNCTION__, req->module);

    ret = module->methods->module_deinit(module);

    /** Store the result to response msg */
    resp->result = ret;

    ALOGI("%s: %s finished with ret = %d", __FUNCTION__, req->module, ret);

    return ret;
}

/**
*    Handle QUERY command
*/
int handle_query(mmi_module_t * module, msg_t * req, msg_t * resp) {

    int ret = 0;

    if(module == NULL || req == NULL || resp == NULL) {
        ALOGE("%s: Invalid param, NULL pointer", __FUNCTION__);
        return -1;
    }

    ALOGI("%s: %s start ", __FUNCTION__, req->module);
    /**Contruct msg to be sended to server*/
    char *p = resp->msg;

    for(uint32_t i = 0; i < module->supported_cmd_list_size; i++) {
        snprintf(p, sizeof(resp->msg), "%s%d:%s;", KEY_STR_EXTRACMD, i, module->supported_cmd_list[i]);
        p = resp->msg + strlen(resp->msg);
    }
    snprintf(p, sizeof(resp->msg), "%s:%d;", KEY_STR_EXTRACMDSIZE, module->supported_cmd_list_size);
    ALOGI("%s: command list[%d]:  %s", __FUNCTION__, module->supported_cmd_list_size, resp->msg);
    /** Store the result to response msg */
    resp->result = ret;

    ALOGI("%s: %s inished with ret = %d", __FUNCTION__, req->module, ret);

    return ret;
}

/**
*    Handle RUN command
*/
int handle_run(mmi_module_t * module, msg_t * req, msg_t * resp) {

    int ret = 0;

    hash_map < string, string > params;

    if(module == NULL || req == NULL || resp == NULL) {
        ALOGE("%s: Invalid param, NULL pointer", __FUNCTION__);
        return -1;
    }

    ALOGI("%s: %s start with param:%s ", __FUNCTION__, req->module, req->msg);

    /** Get the input params for module */
    parse_parameter(req->msg, params);
    params[KEY_MODULE_NAME] = req->module;
    ret = module->methods->module_run(module, req->subcmd, params);

    /** Store the result to response msg */
    resp->result = ret;

    ALOGI("%s: %s finished with ret = %d", __FUNCTION__, req->module, ret);

    return ret;
}

/**
*    Handle STOP command
*/
int handle_stop(mmi_module_t * module, msg_t * req, msg_t * resp) {

    int ret = 0;

    if(module == NULL || req == NULL || resp == NULL) {
        ALOGE("%s: Invalid param, NULL pointer", __FUNCTION__);
        return -1;
    }
    ALOGI("%s: %s start ", __FUNCTION__, req->module);

    ret = module->methods->module_stop(module);

    /** Store the result to response msg */
    resp->result = ret;

    ALOGI("%s: %s inished with ret = %d", __FUNCTION__, req->module, ret);

    return ret;
}
