/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/*****************************************************************************
    This file contains utility function definitions
*****************************************************************************/
#include <sys/types.h>
#include <stdlib.h>
#include "vfm_utils.h"

namespace vpu {

/******************************************************************************
 * Function: fileDump
 * Description: This is a utility function to dump buffers into a file
 *
 * Input parameters:
 *  fn              - File name string
 *  data            - pointer to character buffer that needs to be dumped
 *  length          - Length of the buffer to be dumped
 *  frmCnt         - Pointer to frame count. This count is incremented by this
 *                      function on successful file write
 * Return values:
 *      0   Success
 *      -1  Error
 * Notes: none
 *****************************************************************************/
int32_t dumpFile(const char* fn, const char* data, const int32_t width,
                 const int32_t height, const int32_t stride,
                 const int32_t pixFmt, int32_t& frmCnt)
{
    /* File open */
    FILE *fp = NULL;
    if (0 == frmCnt) {
        fp = fopen(fn, "wb");
        if (NULL == fp) {
            ALOGE("%s: Error in opening %s", __func__, fn);
        }
        fclose(fp);
    }
    fp = fopen(fn, "ab");
    if (NULL == fp) {
        ALOGE("%s: Error in opening %s", __func__, fn);
    }

    /* File write based on the pixel format */
    switch(pixFmt){
        case HAL_PIXEL_FORMAT_YCbCr_420_SP_VENUS:
        {
            /* Dump Y plane */
            for(int32_t i = 0; i < height; i++){
                //TODO: fwrite Y plane
            }

            /* Dump UV plane */
            for(int32_t i = 0; i < height; i++){
            }

            break;
        }
        case HAL_PIXEL_FORMAT_RGB_888:
        {
            //TODO: fwrite
            break;
        }

        default:
            ALOGE("%s: Unsupported format: %d", __func__, pixFmt);
    }
    fclose(fp);
    frmCnt++;

    return 0;
}

/* Utility function to unparcel inargs */
status_t unparcelInArgs(uint32_t command, vfmCmdInArg_t& inArg,
                            const Parcel* data){
    status_t err = NO_ERROR;
    switch(command){
        case VFM_CMD_REGISTER_BUFFER:
            inArg.sessionId     = data->readInt32();
            inArg.flags         = data->readInt32();
            inArg.iaBufReg.hnd  = (private_handle_t *)data->readNativeHandle();
            inArg.iaBufReg.id   = data->readInt32();
        break;

        case VFM_CMD_SET_VPU_CONTROL_EXTENDED:
        case VFM_CMD_GET_VPU_CONTROL_EXTENDED:
            inArg.vidFlowId                 = data->readInt32();
            inArg.iaVpuCtrlExt.type         = data->readInt32();
            inArg.iaVpuCtrlExt.dataLength   = data->readInt32();
            data->read(inArg.iaVpuCtrlExt.dataBuffer,
                        inArg.iaVpuCtrlExt.dataLength);

        break;

        default:
        if(command >= VFM_CMD_START &&
            command <= VFM_CMD_END){

            vfmCmdEntry_t*  vfmCmdEntry
                        = vfmGetCmdEntry((VFM_COMMAND_TYPE)command);
            for(int32_t i = 0; i < vfmCmdEntry->numIntInArgs; i++){
                inArg.intList[i] = data->readInt32();
            }
        }else{
            ALOGE("%s: Invalid command", __func__);
            err = BAD_VALUE;
        }
    }
    return err;
}

/* Utility function to parcel outargs */
status_t parcelOutArgs(uint32_t command, const vfmCmdOutArg_t& outArg,
                            Parcel* reply){
    status_t err = NO_ERROR;
    switch(command){
        case VFM_CMD_SET_VPU_CONTROL_EXTENDED:
        case VFM_CMD_GET_VPU_CONTROL_EXTENDED:
            reply->writeInt32(outArg.oaVpuCtrlExt.responseLength);
            reply->write(outArg.oaVpuCtrlExt.responseBuffer,
                        outArg.oaVpuCtrlExt.responseLength);
        break;

        default:
        if(command >= VFM_CMD_START &&
            command <= VFM_CMD_END){
            vfmCmdEntry_t*  vfmCmdEntry
                        = vfmGetCmdEntry((VFM_COMMAND_TYPE)command);
            for(int32_t i = 0; i < vfmCmdEntry->numIntOutArgs; i++){
                reply->writeInt32(outArg.intList[i]);
            }
        }else{
            ALOGE("%s: Invalid command", __func__);
            err = BAD_VALUE;
        }
    }
    return err;
}

}; //namespace vpu
