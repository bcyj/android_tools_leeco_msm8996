/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
 /*****************************************************************************
   This file contains the commands and command inargs and outargs structures
   for "VFM process" api
 *****************************************************************************/
#include "vfm_cmds.h"

/* Useful for clients */
status_t vfmParcelInArgs(uint32_t command, const vfmCmdInArg_t& inArg,
                            Parcel& data){
    status_t err = NO_ERROR;
    switch(command){
        case VFM_CMD_REGISTER_BUFFER:
            data.writeInt32(inArg.sessionId);
            data.writeInt32(inArg.flags);
            data.writeNativeHandle(inArg.iaBufReg.hnd);
            data.writeInt32(inArg.iaBufReg.id);
        break;

        case VFM_CMD_SET_VPU_CONTROL_EXTENDED:
        case VFM_CMD_GET_VPU_CONTROL_EXTENDED:
            data.writeInt32(inArg.vidFlowId);
            data.writeInt32(inArg.iaVpuCtrlExt.type);
            data.writeInt32(inArg.iaVpuCtrlExt.dataLength);
            data.write(inArg.iaVpuCtrlExt.dataBuffer,
                        inArg.iaVpuCtrlExt.dataLength);

        break;

        default:
        if(command >= VFM_CMD_START &&
            command <= VFM_CMD_END){

            vfmCmdEntry_t*  vfmCmdEntry =
                                vfmGetCmdEntry((VFM_COMMAND_TYPE)command);
            for(int32_t i = 0; i < vfmCmdEntry->numIntInArgs; i++){
                data.writeInt32(inArg.intList[i]);
            }
        }else{
            ALOGE("%s: Invalid command: %d", __func__, command);
            err = BAD_VALUE;
        }
    }
    return err;
}

/* Useful for clients */
status_t vfmUnparcelOutArgs(uint32_t command, vfmCmdOutArg_t& outArg,
                            const Parcel& reply){
    status_t err = NO_ERROR;
    switch(command){
        case VFM_CMD_SET_VPU_CONTROL_EXTENDED:
        case VFM_CMD_GET_VPU_CONTROL_EXTENDED:
            outArg.oaVpuCtrlExt.responseLength = reply.readInt32();
            reply.read(outArg.oaVpuCtrlExt.responseBuffer,
                        outArg.oaVpuCtrlExt.responseLength);
        break;

        default:
        if(command >= VFM_CMD_START &&
            command <= VFM_CMD_END){
            vfmCmdEntry_t*  vfmCmdEntry =
                                vfmGetCmdEntry((VFM_COMMAND_TYPE)command);
            for(int32_t i = 0; i < vfmCmdEntry->numIntOutArgs; i++){
                outArg.intList[i] = reply.readInt32();
            }
        }else{
            ALOGE("%s: Invalid command: %d", __func__, command);
            err = BAD_VALUE;
        }
    }
    return err;
}

