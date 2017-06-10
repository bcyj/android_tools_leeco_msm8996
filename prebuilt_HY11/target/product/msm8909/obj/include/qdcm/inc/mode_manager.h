/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef MODE_MANAGER_H
#define MODE_MANAGER_H

//command ID will be shared across binder subcmd ID coming from libmm-disp-apis
//and request ID coming from display.qservice.
enum COMMAND_ID {
    CMD_SET_ACTIVE_MODE,
    CMD_GET_DEFAULT_MODE,
    CMD_SET_DEFAULT_MODE,
    CMD_GET_CB_RANGE,
    CMD_GET_CB,
    CMD_SET_CB,
    CMD_SAVE_MODE_V2,
    CMD_SET_PA_CONFIG,
    CMD_GET_PA_CONFIG,
    CMD_GET_PA_RANGE,
};

#ifdef __cplusplus

namespace qmode {

class ModeManager {

public:
    ModeManager() { }
    virtual ~ModeManager() { }

    /*
     * Interface to route the request to ppdaemon via socket channel.
     * return type: int --- indicates operation failing or success
     * 0 --- SUCCESS, < 0 --- Failing
     */
    virtual int requestRoute(const char* buf, const int32_t len,
                                            const int32_t& fd) = 0;

    /*
     * Interface to route the request to pbmm-qdcm.so service.
     * return type: int --- indicates operation failing or success
     * 0 --- SUCCESS, < 0 --- Failing
     */
    virtual int requestRoute(int cmd, const void* in, void* out) = 0;

    /*
     * Interface to allow applyDefaultMode() when system boots up.
     */
    virtual int applyDefaultMode(int32_t) = 0;
};

}
#endif

#endif
