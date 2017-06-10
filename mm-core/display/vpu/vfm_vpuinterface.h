/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
 /*****************************************************************************
    This file declares static methods around vpu driver ioctl calls.
    Note that these methods do not contain and instance specific information.
 *****************************************************************************/

#ifndef VFM_VPU_INTERFACE_H
#define VFM_VPU_INTERFACE_H
#include <sys/types.h>
#include <linux/videodev2.h>
#include "vfm_defs.h"
#include "vfm_utils.h"
#include "vpu.h"

#define IOCTL_ERR_CHK(result, str, retValue) {\
    if(result < 0) {\
        ALOGE("%s: %s failed ", __func__, str);\
        return retValue;\
    }\
}\

//using namespace android;
namespace vpu {

class VpuIntf {
public:
    static int32_t getFd();

    static status_t getCapabilities( const int32_t fd );

    static status_t querySessions( const int32_t fd, VPUCapabilities& vpuCap);

    static status_t attachToSession( const int32_t fd, const int32_t sessId);

    static status_t closeFd(const int32_t fd);

    static status_t setInputFormat(const int32_t fd, const InputCfg& inCfg);

    static status_t setOutputFormat(const int32_t fd,
                                        const OutputCfg& outCfg);

    static status_t setInputFrameRate(const int32_t fd,
                                        const InputCfg& inCfg);

    static status_t setOutputFrameRate(const int32_t fd,
                                        const OutputCfg& outCfg);

    static status_t setInputCrop(int32_t fd, const InputCfg& inCfg);

    static status_t setOutputCrop(int32_t fd, const OutputCfg& outCfg);

    static status_t setInputSource(int32_t fd, const InputCfg& inCfg);

    static status_t setOutputDest(const int32_t fd, const OutputCfg& outCfg);

    static status_t setOutputDest(const int32_t fd,
                                    const DestinationPipes& sDestPipes);

    static status_t reqInPortBufs(const int32_t fd, const InputCfg& inCfg,
                                    const int32_t numBufs);

    static status_t reqOutPortBufs(const int32_t fd, const OutputCfg& outCfg,
                                    const int32_t numBufs);

    static status_t qBuf(const int32_t fd, struct v4l2_buffer* bufPtr);

    static status_t fillAndQInBuf(const int32_t fd, struct v4l2_buffer* bufPtr,
        const Dim& sStride, const Dim& sDim, const int32_t pixFmt,
        const VFM_FIELD_ORDER eField, struct timeval timestamp,
        int32_t& releaseFenceFd);

    static status_t fillAndQOutBuf(const int32_t fd, struct v4l2_buffer* bufPtr,
        const Dim& sStride, const Dim& sDim, const int32_t pixFmt,
        const VFM_FIELD_ORDER eField, struct timeval timestamp);

    static status_t fillBufferInfo(struct v4l2_buffer* bufPtr,
        bool isInputPort, const Dim& sStride, const Dim& sDim,
        const int32_t pixFmt, const VFM_FIELD_ORDER eField,
        struct timeval timestamp);

    static status_t getBufferSize(const Dim& sDim, const int32_t pixFmt,
                                    int32_t& memsize, Dim& sStride);

    static status_t dQInBuf(const int32_t fd, int32_t& bufIndex);

    static status_t dQOutBuf(const int32_t fd, int32_t& bufIndex);

    static status_t streamOnInPort(const int32_t fd);

    static status_t streamOnOutPort(const int32_t fd);

    static status_t streamOffInPort(const int32_t fd);

    static status_t streamOffOutPort(const int32_t fd);

    static status_t setVpuControl(const int32_t fd, const struct vpu_control&);

    static status_t getVpuControl(const int32_t fd, struct vpu_control&);

    static status_t setVpuControlExtended(const int32_t fd,
            const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg);

    static status_t getVpuControlExtended(const int32_t fd,
            const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg);

    static status_t subscribeToEvents(const int32_t fd);

    static status_t unSubscribeToEvents(const int32_t fd);

    static status_t commitConfig(const int32_t fd);

    static status_t dQEvent(const int32_t fd, struct v4l2_event&);

    static status_t setInColorSpace(const int32_t fd, int32_t colorspace);
    static status_t setOutColorSpace(const int32_t fd, int32_t colorspace);
private:

    static status_t setFormat(const int32_t fd, const bool isInputPort,
                        const int32_t pixFmt, const Dim& sStride,
                        const Dim& sDim, const VFM_FIELD_ORDER eField,
                        const int32_t colorspace, const int32_t is3d);

    static status_t setCrop(const int32_t fd, const bool isInputPort,
                        const int32_t pixFmt, const Rect& cropRect);

    static status_t setFrameRate(const int32_t fd, const bool isInputPort,
                        const int32_t pixFmt, const int32_t framerate);

    static status_t reqBufs(const int32_t fd, const bool isInputPort,
                        const int32_t pixFmt, const int32_t numBufs);

    static status_t streamOn(const int32_t fd, const bool isInputPort);

    static status_t streamOff(const int32_t fd, const bool isInputPort);

    static status_t fillAndQBuf(const int32_t fd, struct v4l2_buffer* bufPtr,
        bool isInputPort, const Dim& sStride, const Dim& sDim,
        const int32_t pixFmt, const VFM_FIELD_ORDER eField,
        struct timeval timestamp);

    static status_t dQBuf(const int32_t fd, const bool isInputPort,
                            int32_t& bufIndex);

    static status_t setColorSpace(const int32_t fd, bool isInputPort,
                        int32_t colorspace);

    static status_t getV4L2pixelFormatBasedParams(const int32_t pixFmt,
        const Dim& sStride, const Dim& sDim, v4l2_format& format);

    static int32_t getV4L2numPlanes(const int32_t pixFmt);

    static v4l2_field getV4L2field(const VFM_FIELD_ORDER eField);

    static v4l2_colorspace getV4L2colorspace(const int32_t colorspace);

    /* loglevels: 1: Debug (generally config) 2: All */
    static int32_t logAll() { return  (mDebugLogs >= 2);}
    static int32_t isDebug() { return (mDebugLogs >= 1); }
    static int32_t mDebugLogs;
};

}; //namespace VPU


#endif /* end of include guard: VFM_VPU_INTERFACE_H */
