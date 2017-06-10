/*!
  @file
  IAtCmdFwd.cpp

  @brief
  Stub class for AtCmdFwd

*/

/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.


when       who      what, where, why
--------   ---      ---------------------------------------------------------
05/12/11   jaimel   Adding the stub class for AtCmdFwd service to the project
                    in order to decouple QCOM proprietary & OSS/HAL code

===========================================================================*/

#define LOG_NDEBUG 0
#define LOG_NIDEBUG 0
#define LOG_NDDEBUG 0
#define LOG_TAG "IAtCmdFwd.cpp"
#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <utils/String16.h>
#include "common_log.h"
#include "IAtCmdFwdService.h"
#include <sys/time.h>
#include <cutils/jstring.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

extern void freeAtCmdResponse(AtCmdResponse *response)
{
    if (!response) return;

    if (response->response)
        free(response->response);
    delete(response);
}

extern void freeAtCmd(AtCmd *cmd)
{
    int i;

    if (!cmd) return;

    if (cmd->name) free(cmd->name);

    if (cmd->ntokens != 0 && cmd->tokens) {
        for (i = 0; i < cmd->ntokens; i++) {
            free(cmd->tokens[i]);
        }
        free(cmd->tokens);
    }
    free(cmd);
}

extern AtCmd *copyAtCmd(AtCmd *cmd)
{
    AtCmd *ret = NULL;
    int i;

    if (!cmd) return NULL;

    ret = (AtCmd *)calloc(1, sizeof(AtCmd));
    if (!ret) goto error;

    if (ret) {
        ret->opcode = cmd->opcode;
        if (cmd->name) {
            ret->name = strdup(cmd->name);
            if (!ret->name) goto error;
        }
        ret->ntokens = cmd->ntokens;
        if (cmd->ntokens && cmd->tokens) {
            ret->tokens = (char **)calloc(cmd->ntokens, sizeof(char *));
            if (!ret->tokens) goto error;
            for (i = 0; i < ret->ntokens; i++) {
                ret->tokens[i] = strdup(cmd->tokens[i]);
                if (!ret->tokens[i]) goto error;
            }
        }
    }
    return ret;

error:
    if (ret) {
        free(ret->name);
        ret->name = NULL;
        if(ret->ntokens && ret->tokens) {
            for (i = 0; i < ret->ntokens; i++) {
                free(ret->tokens[i]);
                ret->tokens[i] = NULL;
            }
            free(ret->tokens);
            ret->tokens = NULL;
        }
        free(ret);
        ret = NULL;
    }
    return NULL;
}

#ifdef __cplusplus
}
#endif


namespace android {

/* Specifies the function id that has to be remotely invoked */
enum {
    processAtCmd = IBinder::FIRST_CALL_TRANSACTION,
};

/* Stub class for AtCmdFwd service */
class BpAtCmdFwdService : public BpInterface<IAtCmdFwdService>
{
public:
    BpAtCmdFwdService(const sp<IBinder>& impl)
        : BpInterface<IAtCmdFwdService>(impl)
    {
    }

/*===========================================================================
  FUNCTION  processCommand
===========================================================================*/
/*!
@brief
  Bundles the parameters required for processAtCmd function into a parcel
  and invokes it

@return
  Returns an AtCmdResponse struct

@note


*/
/*=========================================================================*/
    virtual AtCmdResponse *processCommand(const AtCmd &cmd)
    {
        LOGD("processCommand()");
        Parcel data, reply;
        size_t len = 0;
        char16_t *s16;

        data.writeInterfaceToken(IAtCmdFwdService::getInterfaceDescriptor());
        data.writeInt32(1);                     //specify there is an input parameter
        data.writeInt32(cmd.opcode);            //opcode
        String16 cmdname(cmd.name);
        s16 = strdup8to16(cmd.name, &len);
        data.writeString16(s16, len);            //command name
        free(s16);
        data.writeInt32(cmd.ntokens);
        for (int i=0; i < cmd.ntokens; i++) {
            s16 = strdup8to16(cmd.tokens[i], &len);
            data.writeString16(s16,len);
            free(s16);
        }

        status_t status = remote()->transact(processAtCmd, data, &reply);//RPC call
        LOGI("Status: %d",status);
        if (status != NO_ERROR) {
          LOGE("Error in RPC Call to AtCdmFwd Service (Exception occurred?)");
          return NULL;
        }
        AtCmdResponse *response = new AtCmdResponse();
        if (response) {
            int code = reply.readInt32();
            if (code) {
                String16 msg16(reply.readString16());
                const char *msg = strndup16to8(msg16.string(), msg16.size());
                LOGI("Exception occured (%d): %s", code, msg);
                freeAtCmdResponse(response);
                return NULL;
            }
            int isnull = !reply.readInt32();
            if (isnull) {
                freeAtCmdResponse(response);
                return NULL;
            }
            response->result = reply.readInt32();
            String16 msg16(reply.readString16());
            const char *msg = strndup16to8(msg16.string(), msg16.size());
            if(NULL != msg) {
                response->response = strdup(msg);
                LOGD("response. code: %d. msg: %s", response->result, response->response);
            }
        }
        return response;
    }
};  // class BpAtCmdFwdService

#ifdef ANDROID_JB_MR2
IMPLEMENT_META_INTERFACE(AtCmdFwdService, "com.qualcomm.atfwd.IAtCmdFwd")
#else
IMPLEMENT_META_INTERFACE(AtCmdFwdService, "com.android.internal.atfwd.IAtCmdFwd")
#endif

};  // namespace android

