/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/*****************************************************************************
	This file defines static methods around vpu driver ioctl calls. Note that
    these methods do not contain and instance specific information.
*****************************************************************************/

#define ATRACE_TAG (ATRACE_TAG_GRAPHICS | ATRACE_TAG_HAL)
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <linux/videodev2.h>
#ifdef VPU_DRIVER_AVAILABLE
#include <media/msm_vpu.h>
#endif
#include <utils/Trace.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include "vfm_utils.h"
#include "vfm_vpuinterface.h"

#ifdef VENUS_COLOR_FORMAT
#include <media/msm_media_info.h>
#else
#define VENUS_Y_STRIDE(args...) 0
#define VENUS_Y_SCANLINES(args...) 0
#define VENUS_UV_STRIDE(args...) 0
#define VENUS_UV_SCANLINES(args...) 0
#define COLOR_FMT_NV12             (0)
#endif

using namespace android;
namespace vpu {

#define VPU_DRIVER_NAME "msm_vpu"

#define V4L2_DIR_NAME "/sys/class/video4linux"

#define MAX_PATH_LEN (128)

int32_t VpuIntf::mDebugLogs = 0;

/**
  * int getFd
  * Searches and opens vpu driver. Returns valid fd or -1 if it fails
  *
  */
int VpuIntf::getFd()
{
    status_t err = NO_ERROR;
    String8    dev, devName;
    DIR        *dir;
    struct     dirent *de;
    char       name[MAX_PATH_LEN] = "";
    int        fd = -1, fd_devname, result;

    mDebugLogs = getProperty("persist.vfm.logs");

    ATRACE_CALL();
    ALOGD_IF(isDebug(), "%s: E", __func__);

    dir = opendir(V4L2_DIR_NAME);
    if(dir == NULL){
        ALOGE("%s: Could not open %s", __func__, V4L2_DIR_NAME);
        return NO_INIT;
    }
    //Loop through the directory
    while((de = readdir(dir)))
    {
        //Skip . and .. directories
        if(de->d_name[0] == '.' &&
           (de->d_name[1] == '\0' ||
           (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;

        /* find and read device node names from sys/class/video4linux dir*/
        devName = String8(V4L2_DIR_NAME) + "/" + de->d_name + "/name";
        ALOGD_IF(isDebug(), "%s: Opening %s ...", __func__, devName.string());

#if !DBG_DISABLE_IOCTL
        fd_devname = open(devName.string(), O_RDONLY);
        if(fd_devname < 0){
            ALOGE("%s: could not find device name %s",
                __func__, devName.string());
            //TODO: AM Do we exit the function here?
            //return ERR_NO_DEVICE;
        }
        memset(name, 0, MAX_PATH_LEN);
        result = read(fd_devname, name, MAX_PATH_LEN);
#else
        result = 0;
#endif
        if(result < 0) {
            ALOGE("%s: could not read device name from %s",
              __func__, devName.string());
        }
        else {
#if !DBG_DISABLE_IOCTL
            if(!strncmp(name, VPU_DRIVER_NAME, strlen(VPU_DRIVER_NAME)))
            {
                close(fd_devname);
                fd_devname = -1;
                /* open the vpu driver node found from /dev dir */
                dev = String8("/dev/") + de->d_name;
                fd = open(dev, O_RDWR | O_NONBLOCK);
                if(fd < 0){
                    ALOGE("%s Error opening device %s", __func__,dev.string());
                }
                else
                {
                    ALOGD_IF(isDebug(), "%s: Successfully opened fd: %d",
                        __func__, fd);
                    break;
                }
            }
#else
        // dummy fd
        fd = 100;
#endif
        }
        // close(-1) is valid
        close(fd_devname);
        fd_devname = -1;
    }
    closedir(dir);
    ALOGE_IF(fd < 0, "%s: couldn't find '%s' in '%s'",
        __func__, VPU_DRIVER_NAME, V4L2_DIR_NAME);
    return fd;
}

/**
  * int queryCapabilities - Helper Function. Querys VPU capabilities.
  *
  * @client: name of the client.
  */
status_t VpuIntf::getCapabilities( const int32_t fd)
{
    status_t err = NO_ERROR;
    struct v4l2_capability cap;
    int result;

    ALOGD_IF(isDebug(), "%s: fd: %d E", __func__, fd);

    memset(&cap, 0, sizeof( struct v4l2_capability));

#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if (result < 0)
    {
        ALOGE("%s VIDIOC_QUERYCAP failed. fd = %d", __func__, fd);
        //TBD: R Close fd and clean-up
        return UNKNOWN_ERROR;
    }

    /* See if streaming is supported*/
    if ((cap.capabilities & V4L2_CAP_STREAMING) == 0)
    {
        ALOGE("%s: Device does not support streaming", __func__);
        return UNKNOWN_ERROR;
    }

    /* See if multi-planar API is supported for input. */
    if( (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT_MPLANE) == 0 )
    {
        ALOGE("%s: Device does not support multi-planar API"
                 "through Video Output Interface.", __func__);
        return UNKNOWN_ERROR;
    }
#endif
    ALOGD_IF(isDebug(), "%s: Device supports streaming, and multi-planar "
             "API through Video Output", __func__);
    return err; /* Success. */
}

/**
  * int querySessions - Queries number of available
  *                     sessions on VPU. Fills client with number
  *                     of sessions available.
  *
  * @fd:     client FD
  * @vpuCap: Structure to fill capabilities
  */
status_t VpuIntf::querySessions(const int32_t fd, VPUCapabilities& vpuCap)
{
    status_t err = NO_ERROR;
    int result;

    ALOGD_IF(isDebug(), "%s: fd: %d E", __func__, fd);

    ERR_CHK((fd < 0), NO_INIT);
    vpuCap.numSessions = 1;
    /* Check how many sessions are suported by H/W */
#ifdef VPU_DRIVER_AVAILABLE
#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VPU_QUERY_SESSIONS, &vpuCap.numSessions);
    if (result < 0)
    {
        ALOGE("%s VPU_QUERY_SESSIONS failed fd: %d", __func__, fd);
        //TBD: R close fd and clean-up
        return UNKNOWN_ERROR;
    }
#endif //!DBG_DISABLE_IOCTL
#endif //VPU_DRIVER_AVAILABLE
    ALOGD_IF(isDebug(), "%s: The number of sessions supported are %d fd: %d",
               __func__, vpuCap.numSessions, fd);

    return err;
}

/**
  * int attachToSession - attaches a client to a particular session.
  * @sessId: the number of the session client wants to connect to.
  */
status_t VpuIntf::attachToSession(const int32_t fd, const int32_t sessId)
{
    status_t err = NO_ERROR;
    int result = 0;

    ATRACE_CALL();
    ALOGD_IF(isDebug(), "%s: fd: %d sessId: %d E", __func__, fd, sessId);

    if( fd < 0 || sessId < 0)
    {
        ALOGE("%s: Invalid values provided fd: %d sessId: %d",
            __func__, fd, sessId);
        return BAD_VALUE;
    }

    /* Attach Client to Session. */
#ifdef VPU_DRIVER_AVAILABLE
#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VPU_ATTACH_TO_SESSION, &sessId);
    if (result < 0)
    {
        if( errno == EINVAL )
            ALOGE("%s: session %d is out of valid range fd: %d",
                __func__, sessId, fd);
        else if( errno == EBUSY)
            ALOGE("%s: max. allowed num of clients attached to session fd: %d",
                    __func__, fd);
        else
            ALOGE("%s: ATTACH_TO_SESS failed errno: %d fd: %d",
                __func__, errno, fd);
            return UNKNOWN_ERROR;
    }
#endif //!DBG_DISABLE_IOCTL
#endif //VPU_DRIVER_AVAILABLE
    ALOGD_IF(isDebug(), "%s client successfully attached to session.", __func__);

    return err;
}

/**
  * int closeFd: closes this FD on the driver
  *
  * @fd: client fd opened on vpu (Obtained in getFd() ).
  * @sessId: the number of the session client wants to connect to.
  */
status_t VpuIntf::closeFd(const int32_t fd)
{
    status_t err = NO_ERROR;
    int result = 0;
    ALOGD_IF(isDebug(), "%s E", __func__);

#if !DBG_DISABLE_IOCTL
    close(fd);
#endif
    return err;
} //closeFd

/**
  * int setFormat - Helper function. Sets format on VPU driver. Same format for
  *	                    both ports, different formats for different sessions.
  *
  * @client: name of the client (must be initiated by client init).
  * @videoFormat: fill this according to video format expected as input to VPU.
  */
status_t VpuIntf::setInputFormat(const int32_t fd, const InputCfg& inCfg)
{
    ATRACE_CALL();
    return setFormat(fd, true, inCfg.pixFmt, inCfg.sStride, inCfg.sDim,
        inCfg.eField, inCfg.colorspace, inCfg.is3d);
}

status_t VpuIntf::setOutputFormat(const int32_t fd, const OutputCfg& outCfg)
{
    Dim sStride = outCfg.sDim;
    Dim sTgtDim;

    ATRACE_CALL();
    sTgtDim.width = outCfg.tgtRect.right - outCfg.tgtRect.left;
    sTgtDim.height= outCfg.tgtRect.bottom - outCfg.tgtRect.top;

    return setFormat(fd, false, outCfg.pixFmt, sStride, sTgtDim,
        outCfg.eField, outCfg.colorspace, outCfg.is3d);
}

status_t VpuIntf::setFormat(const int32_t fd, const bool isInputPort,
    const int32_t pixFmt, const Dim& sStride, const Dim& sDim,
    const VFM_FIELD_ORDER eField, const int32_t clrSpc, const int32_t is3d)
{
    status_t err = NO_ERROR;
    int result = 0;
    struct v4l2_format format;

    ALOGD_IF(isDebug(), "%s E", __func__);

    /* Format setup. */
    memset (&format, 0, sizeof (struct v4l2_format));

    //VPU driver always expects to fill the multiplanar structure,
    //even though its single plane
    format.type = (isInputPort) ? V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE :
            V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    format.fmt.pix_mp.width       = sDim.width; /* width in pixels */
    format.fmt.pix_mp.height      = sDim.height; /* height in pixels */
    format.fmt.pix_mp.field       = getV4L2field(eField);
    format.fmt.pix_mp.colorspace  = getV4L2colorspace(clrSpc);
    err = getV4L2pixelFormatBasedParams(pixFmt, sStride, sDim, format);
    if(NO_ERROR != err){
        return err;
    }
    //The colorspace is in this structure is ignored by VPU driver.
    //Colorspace is instead set using private ioctl call
#ifdef VPU_DRIVER_AVAILABLE
#ifdef MPQ_VPU_DRIVER_EXTENSIONS
    /* 3d specific format extension */
    {
        struct v4l2_format_vpu_extension* extFmt =
            (struct v4l2_format_vpu_extension*) &format.fmt.pix_mp.reserved[0];
        extFmt->is_3d           = 0;
        extFmt->gap_in_lines    = 0;
    }
#endif //MPQ_VPU_DRIVER_EXTENSIONS
#endif //VPU_DRIVER_AVAILABLE
    ALOGD_IF(isDebug(), "%s: B4 S_FMT fd: %d, isInputPort: %d  w:%d"
        " h:%d clrspce:%d fld:%d pixFmt: %d numPlanes: %d",__func__,
        fd, isInputPort, format.fmt.pix_mp.width, format.fmt.pix_mp.height,
        format.fmt.pix_mp.colorspace, format.fmt.pix_mp.field,
        format.fmt.pix_mp.pixelformat, format.fmt.pix_mp.num_planes);

    /* Format the port. */
#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VIDIOC_S_FMT, &format);
    if (result < 0)
    {
        ALOGE("%s: S_FMT failed fd: %d, isInputPort: %d  w:%d"
        " h:%d clrspce:%d fld:%d pixFmt: %d numPlanes: %d",__func__,
        fd, isInputPort, format.fmt.pix_mp.width, format.fmt.pix_mp.height,
        format.fmt.pix_mp.colorspace, format.fmt.pix_mp.field,
        format.fmt.pix_mp.pixelformat, format.fmt.pix_mp.num_planes);
        return UNKNOWN_ERROR;
    }
#endif
    {
        ALOGD_IF(isDebug(), "%s: S_FMT success fd: %d, isInputPort: %d  w:%d"
        " h:%d clrspce:%d fld:%d pixFmt: %d numPlanes: %d",__func__,
        fd, isInputPort, format.fmt.pix_mp.width, format.fmt.pix_mp.height,
        format.fmt.pix_mp.colorspace, format.fmt.pix_mp.field,
        format.fmt.pix_mp.pixelformat, format.fmt.pix_mp.num_planes);
    }

    return err;
}

status_t VpuIntf::setInputFrameRate(const int32_t fd, const InputCfg& inCfg)
{
    ATRACE_CALL();
    return setFrameRate(fd, true, inCfg.pixFmt, inCfg.framerate);
}

status_t VpuIntf::setOutputFrameRate(const int32_t fd, const OutputCfg& outCfg)
{
    ATRACE_CALL();
    return setFrameRate(fd, false, outCfg.pixFmt, outCfg.framerate);
}

/**
  * setFrameRate - Helper function. Sets frame rate.
  *
  */
status_t VpuIntf::setFrameRate(const int32_t fd, bool isInputPort,
    const int32_t pixFmt, const int32_t framerate)
{
    status_t err = NO_ERROR;
    int result;
    ALOGD_IF(isDebug(), "%s:  E", __func__);

#ifdef VPU_DRIVER_AVAILABLE
#ifdef MPQ_VPU_DRIVER_EXTENSIONS
    struct vpu_port_control pctrl;

    memset (&pctrl, 0, sizeof (struct vpu_port_control));

    //VPU driver always expects _MPLANE
    pctrl.port = (isInputPort) ? 0 : 1;
    pctrl.control_id = VPU_CTRL_FPS;
    pctrl.data.value = (framerate / 100) << 16 | (framerate % 100);

#if !DBG_DISABLE_IOCTL
    /* Set frame rate. */
    result = ioctl(fd, VPU_S_CONTROL_PORT, &pctrl);
    if (result < 0)
    {
        ALOGE("%s: VPU_S_CONTROL_PORT: failed fp100s:%d fps16.16: %d isInputPort:%d ",
              __func__, framerate, pctrl.data.value, isInputPort);
        return UNKNOWN_ERROR;
    }
    else
#endif //!DBG_DISABLE_IOCTL
    {
        ALOGD_IF(isDebug(), "%s: VPU_S_CONTROL_PORT: success fp100s:%d "
        "fps16.16: %d isInputPort:%d ", __func__, framerate,
        pctrl.data.value, isInputPort);
    }
#endif //MPQ_VPU_DRIVER_EXTENSIONS
#endif //VPU_DRIVER_AVAILABLE
    return err;
}

status_t VpuIntf::setInputCrop(const int32_t fd, const InputCfg& inCfg)
{
    ATRACE_CALL();
    return setCrop(fd, true, inCfg.pixFmt, inCfg.sCrop);
}

status_t VpuIntf::setOutputCrop(const int32_t fd, const OutputCfg& outCfg)
{
    ATRACE_CALL();
    return setCrop(fd, false, outCfg.pixFmt, outCfg.dstRect);
}
/*
 * int crop - set crop based on input w, h, l, t params.
 *            for now, just S_CROP the crop params found
 *            with G_CROP.
 *
 */

status_t VpuIntf::setCrop(const int32_t fd, const bool isInputPort,
    const int32_t pixFmt, const Rect& cropRect)
{
    status_t err = NO_ERROR;
    int result = 0;
    struct v4l2_crop crop;

    ALOGD_IF(isDebug(), "%s: E", __func__);
    memset(&crop, 0, sizeof(struct v4l2_crop));

    //VPU driver always expects _MPLANE
    crop.type = (isInputPort) ? V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE :
        V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

    /* Set new crop params. */
    crop.c.width  = cropRect.right - cropRect.left;
    crop.c.height = cropRect.bottom - cropRect.top;
    crop.c.left   = cropRect.left;
    crop.c.top    = cropRect.top;

#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VIDIOC_S_CROP, &crop);
    if(result < 0)
    {
        ALOGE("%s: VIDIOC_S_CROP: failed errno: %d isInput:%d "
              "crop: l:%d t:%d w:%d h:%d", __func__, errno, isInputPort,
              crop.c.left, crop.c.top, crop.c.width, crop.c.height);
        return UNKNOWN_ERROR;
    }
    else
#endif
    {
        ALOGD_IF(isDebug(), "%s: VIDIOC_S_CROP: succeeded. w: %d h: %d l: %d t: %d",
             __func__, crop.c.width, crop.c.height, crop.c.left, crop.c.top);
    }

    return err;
}

status_t VpuIntf::setInputSource(const int32_t fd, const InputCfg& inCfg)
{
    status_t err = NO_ERROR;
    int32_t result = 0;
    int32_t src = VPU_INPUT_TYPE_HOST;

    ATRACE_CALL();
    ALOGD_IF(isDebug(), "%s: E", __func__);

#ifdef VPU_DRIVER_AVAILABLE
    switch(inCfg.inSrc.eInputSrcType){
        case INPUT_SRC_VCAP_TUNNEL:
            src = VPU_INPUT_TYPE_VCAP;
            for(int32_t i = 0; i < inCfg.inSrc.sSourcePipes.numPipes; i++){
                if(SRC_VCAP_PIPE0 == inCfg.inSrc.sSourcePipes.pipe[i]){
                    src |= VPU_PIPE_VCAP0;
                }
                if(SRC_VCAP_PIPE1 == inCfg.inSrc.sSourcePipes.pipe[i]){
                    src |= VPU_PIPE_VCAP1;
                }
            }
        break;

        case INPUT_SRC_SF_QBUF:
        case INPUT_SRC_VFM_TUNNEL:
            src = VPU_INPUT_TYPE_HOST;
            break;

        default:
            ALOGE("%s: Invalid source type: %d", __func__,
                    inCfg.inSrc.eInputSrcType);
            return BAD_VALUE;
    }
#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VIDIOC_S_INPUT, &src);
    if(result < 0){
        ALOGE("%s: VIDIOC_S_INPUT failed err: %d src: %x",__func__,errno,src);
        return UNKNOWN_ERROR;
    }else
#endif//!DBG_DISABLE_IOCTL
#endif//ifdef VPU_DRIVER_AVAILABLE
    {
        ALOGD_IF(isDebug(), "%s: VIDIOC_S_INPUT success src: %x",__func__,src);
    }

    return err;
}

/* Sets output destination - HOST/Tunnel and MDSS pipes */
status_t VpuIntf::setOutputDest(const int32_t fd,
                            const DestinationPipes& sDestPipes)
{
    status_t err = NO_ERROR;
    int32_t result = 0;

    ATRACE_CALL();
    ALOGD_IF(isDebug(), "%s: E", __func__);
#if !DBG_VPUOUT_HOST
    if (1 > sDestPipes.numPipes){
        ALOGE("%s: Invalid number of pipes allocated: %d", __func__,
                sDestPipes.numPipes);
        return UNKNOWN_ERROR;
    }
    int32_t dest = VPU_OUTPUT_TYPE_DISPLAY;
    for(int32_t i = 0; i < sDestPipes.numPipes; i++){
        if(sDestPipes.pipe[i] == 1) dest |= VPU_PIPE_DISPLAY0;
        if(sDestPipes.pipe[i] == 2) dest |= VPU_PIPE_DISPLAY1;
        if(sDestPipes.pipe[i] == 4) dest |= VPU_PIPE_DISPLAY2;
        if(sDestPipes.pipe[i] >= 8) dest |= VPU_PIPE_DISPLAY3;
    }

#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VIDIOC_S_OUTPUT, &dest);
    if(result < 0){
        ALOGE("%s: VIDIOC_S_OUTPUT: failed with err: %d dest: %d",
            __func__, errno, dest);
        return UNKNOWN_ERROR;
    }else
#endif
    {
        ALOGD_IF(isDebug(), "%s: VIDIOC_S_OUTPUT succeeded. dest: %d",
            __func__, dest);
    }
#endif

    return err;
}

status_t VpuIntf::reqInPortBufs(const int32_t fd, const InputCfg& inCfg,
    const int32_t numBufs)
{
    ATRACE_CALL();
    return reqBufs(fd, true, inCfg.pixFmt, numBufs);
}

status_t VpuIntf::reqOutPortBufs(const int32_t fd, const OutputCfg& outCfg,
    const int32_t numBufs)
{
    ATRACE_CALL();
    return reqBufs(fd, false, outCfg.pixFmt, numBufs);
}

status_t VpuIntf::reqBufs(const int32_t fd, bool isInputPort,
    const int32_t pixFmt, const int32_t numBufs)
{
    status_t err = NO_ERROR;
    int result = 0;
    struct v4l2_requestbuffers reqbuf;

    ALOGD_IF(isDebug(), "%s: E", __func__);

    memset(&reqbuf, 0, sizeof(struct v4l2_requestbuffers));
    reqbuf.type = (isInputPort) ? V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE :
            V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    reqbuf.memory  = V4L2_MEMORY_USERPTR;
    reqbuf.count   = numBufs;

#if !DBG_DISABLE_IOCTL
    /* Request buffers. */
    result = ioctl(fd, VIDIOC_REQBUFS, &reqbuf);
    if (result < 0){
        if(errno == EINVAL){
            ALOGE("%s: VIDIOC_REQBUFS: does not support user pointer method"
                    " isInputPort: %d", __func__, isInputPort);
        }
        else{
            ALOGE("%s: VIDIOC_REQBUFS: failed reqbufs isInputPort: %d",
                    __func__, isInputPort);
        }
        return UNKNOWN_ERROR;
    }
#endif
    if ((unsigned)reqbuf.count < (unsigned)numBufs){
        if (numBufs){
            /* TODO: S You may need to free the buffers here. */
            ALOGE ("VIDIOC_REQBUFS: Not enough buffer memory requested: %d "
              "avail: %d isInPort: %d", numBufs, reqbuf.count, isInputPort);
            return NO_MEMORY;
        }
    }
    ALOGD_IF(isDebug(), "%s: VIDIOC_REQBUFS success: numbufs requested %d "
            "avail: %d isInPort: %d", __func__, numBufs, reqbuf.count,
            isInputPort);

    return err;
}

status_t VpuIntf::qBuf(const int32_t fd, struct v4l2_buffer* bufPtr)
{
    status_t err = NO_ERROR;
    int32_t result;

    ALOGD_IF(logAll(), "%s: E", __func__);
#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VIDIOC_QBUF, bufPtr);
    if(result < 0){
        ALOGE("VIDIOC_QBUF failed for bufIdx: %d, errno: %d",
                bufPtr->index, errno);
    }
#endif
    ALOGD_IF(logAll(), "%s: VIDIOC_QBUF %d success", __func__, bufPtr->index);
    return err;
}

status_t VpuIntf::dQInBuf(const int32_t fd, int32_t& bufIndex)
{
    ATRACE_CALL();
    return dQBuf(fd, true, bufIndex);
}

status_t VpuIntf::dQOutBuf(const int32_t fd, int32_t& bufIndex)
{
    ATRACE_CALL();
    return dQBuf(fd, false, bufIndex);
}

status_t VpuIntf::dQBuf(const int32_t fd, const bool isInputPort,
                        int32_t& bufIndex)
{
    status_t err = NO_ERROR;
    int32_t result;
    struct v4l2_buffer buffer;
    struct v4l2_plane planes[VIDEO_MAX_PLANES];

    memset(&buffer, 0, sizeof(struct v4l2_buffer));
    memset(&planes, 0, sizeof(struct v4l2_plane) * VIDEO_MAX_PLANES);

    buffer.memory = V4L2_MEMORY_USERPTR;
    buffer.m.planes = planes;
    buffer.length = VIDEO_MAX_PLANES;
    buffer.type = (isInputPort) ? V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE :
        V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

    ALOGD_IF(isDebug(), "%s: E", __func__);
#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VIDIOC_DQBUF, &buffer);
    if(result < 0){
        ALOGE("%s: VIDIOC_DQBUF failed. err: %d isInputPort: %d",
            __func__, errno, isInputPort);
        return UNKNOWN_ERROR;
    }
    else if (buffer.flags & V4L2_BUF_FLAG_ERROR) {
        ALOGE("%s: V4L2_BUF_FLAG_ERROR err: %d bufIndex: %d isInputPort: %d",
            __func__, errno, buffer.index, isInputPort);
        return UNKNOWN_ERROR;
    }
    bufIndex = buffer.index;
#else
    bufIndex = 0;
#endif
    ALOGD_IF(logAll(), "%s: VIDIOC_DQBUF %d success isInputPort: %d",
            __func__, buffer.index, isInputPort);

    return err;
}

status_t VpuIntf::streamOnInPort(const int32_t fd)
{
    ATRACE_CALL();
    return streamOn(fd, true);
}

status_t VpuIntf::streamOnOutPort(const int32_t fd)
{
    ATRACE_CALL();
    return streamOn(fd, false);
}

status_t VpuIntf::streamOn(const int32_t fd, bool isInputPort)
{
    status_t err = NO_ERROR;
    int32_t result = 0;
    enum v4l2_buf_type bufType;

    ALOGD_IF(isDebug(), "%s: E", __func__);

    bufType = (isInputPort) ? V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE :
        V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VIDIOC_STREAMON, &bufType);
    if(result < 0){
        ALOGE("VIDIOC_STREAMON failed. isInputPort: %d ", isInputPort);
        return UNKNOWN_ERROR;
    }
#endif
    ALOGD_IF(isDebug(), "%s: VIDIOC_STREAMON success isInputPort: %d",
        __func__, isInputPort);
    return err;
}

status_t VpuIntf::streamOffInPort(const int32_t fd)
{
    ATRACE_CALL();
    return streamOff(fd, true);
}

status_t VpuIntf::streamOffOutPort(const int32_t fd)
{
    ATRACE_CALL();
    return streamOff(fd, false);
}

status_t VpuIntf::streamOff(const int32_t fd, bool isInputPort)
{
    status_t err = NO_ERROR;
    int32_t result = 0;
    enum v4l2_buf_type bufType;

    ALOGD_IF(isDebug(), "%s: E", __func__);

    bufType = (isInputPort) ? V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE :
        V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VIDIOC_STREAMOFF, &bufType);
    if(result < 0){
        ALOGE("%s: VIDIOC_STREAMOFF failed isInputPort: %d",
            __func__, isInputPort);
        return UNKNOWN_ERROR;
    }
#endif
    ALOGD_IF(isDebug(), "%s: VIDIOC_STREAMOFF success isInputPort:%d",
            __func__, isInputPort);
    return err;
}

//Subscribes to all ports of vpu session
status_t VpuIntf::subscribeToEvents(const int32_t fd)
{
    status_t err = NO_ERROR;
    int32_t result = 0;

    ALOGD_IF(isDebug(), "%s: E", __func__);

    struct v4l2_event_subscription es;

    memset(&es, 0, sizeof(struct v4l2_event_subscription));
    es.type = V4L2_EVENT_ALL; // All events

#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VIDIOC_SUBSCRIBE_EVENT, &es);
    if(result < 0){
        ALOGE("%s: VIDIOC_SUBSCRIBE_EVENT failed fd: %d",
                __func__, fd);
        return UNKNOWN_ERROR;
    }
#endif
    ALOGD_IF(isDebug(), "%s: VIDIOC_SUBSCRIBE_EVENT success. fd: %d",
                        __func__, fd);

    return err;
}

//Unsubscribes to all ports of vpu session
status_t VpuIntf::unSubscribeToEvents(const int32_t fd)
{
    status_t err = NO_ERROR;
    int32_t result = 0;

    ALOGD_IF(isDebug(), "%s: E", __func__);

    struct v4l2_event_subscription es;

    memset(&es, 0, sizeof(struct v4l2_event_subscription));
    es.type = V4L2_EVENT_ALL; // All events

#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VIDIOC_UNSUBSCRIBE_EVENT, &es);
    if(result < 0){
        ALOGE("%s: VIDIOC_UNSUBSCRIBE_EVENT failed fd: %d",
                __func__, fd);
        return UNKNOWN_ERROR;
    }
#endif
    ALOGD_IF(isDebug(), "%s: VIDIOC_UNSUBSCRIBE_EVENT success. fd: %d",
                        __func__, fd);

    return err;
}

//Commits the configuration of the vpu session
status_t VpuIntf::commitConfig(const int32_t fd)
{
    status_t err = NO_ERROR;
    int32_t result = 0;

    ATRACE_CALL();
    ALOGD_IF(isDebug(), "%s: E", __func__);

#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VPU_COMMIT_CONFIGURATION, 0);
    if(result < 0){
        ALOGE("%s: VPU_COMMIT_CONFIGURATION failed fd: %d",
                __func__, fd);
        return UNKNOWN_ERROR;
    }
#endif
    ALOGD_IF(isDebug(), "%s: VPU_COMMIT_CONFIGURATION success fd: %d",
                __func__, fd);
    return err;
}

status_t VpuIntf::dQEvent(const int32_t fd, struct v4l2_event& event)
{
    status_t err = NO_ERROR;
    int32_t result = 0;
    enum v4l2_buf_type bufType;

    ATRACE_CALL();
    ALOGD_IF(isDebug(), "%s: E", __func__);
    memset(&event, 0, sizeof(struct v4l2_event));

    result = ioctl(fd, VIDIOC_DQEVENT, &event);
    IOCTL_ERR_CHK(result, "VIDIOC_DQEVENT", UNKNOWN_ERROR);

    //while loop to read all the events
    while(!result)
    {
        /* Handle events. */
        switch(event.type)
        {
        /* Add specific event handling code here. */
        case VPU_EVENT_INVALID_CONFIG:
            ALOGE("%s: error VPU_EVENT_INVALID_CONFIG", __func__);
            break;
        case VPU_EVENT_FLUSH_DONE:
            ALOGD_IF(isDebug(), "%s: VPU_EVENT_FLUSH_DONE", __func__);
            break;
        case VPU_EVENT_ACTIVE_REGION_CHANGED:
            ALOGD_IF(isDebug(), "%s: ACTIVE_REGION_CHANGED", __func__);
            break;
        case VPU_EVENT_SESSION_TIMESTAMP:
            ALOGD_IF(isDebug(), "%s: VPU_EVENT_SESSION_TIMESTAMP", __func__);
            break;
        case VPU_EVENT_HW_ERROR:
            ALOGD_IF(isDebug(), "%s: VPU_EVENT_HW_ERROR", __func__);
            break;
        case VPU_EVENT_FAILED_SESSION_STREAMING:
            ALOGD_IF(isDebug(), "%s: VPU_EVENT_FAILED_SESSION_STREAMING",
                                __func__);
            break;
        default:
            ALOGD_IF(isDebug(), "%s: event recvd %d", __func__, event.type);
            break;
        }

        /* Repeat for any pending events. */
        if( event.pending != 0 ) {
            result = ioctl(fd, VIDIOC_DQEVENT, &event);
        }else {
            break; /* No pending events, leave event loop. */
        }
    }

    ALOGD_IF(isDebug(), "%s: VIDIOC_DQEVENT success", __func__);

    return err;
}

status_t VpuIntf::fillAndQInBuf(const int32_t fd, struct v4l2_buffer* bufPtr,
    const Dim& sStride, const Dim& sDim, const int32_t pixFmt,
    const VFM_FIELD_ORDER eField, struct timeval timestamp,
    int32_t& releaseFenceFd)
{
    status_t err = NO_ERROR;
    int cf = COLOR_FMT_NV12;

    ATRACE_CALL();
    if(!bufPtr){
        ALOGE("%s: bufPtr is NULL", __func__);
        return BAD_VALUE;
    }
    bufPtr->type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    bufPtr->memory      = V4L2_MEMORY_USERPTR;
    bufPtr->bytesused   = 0; //for multiplanar structure
    bufPtr->field       = getV4L2field(eField);
    bufPtr->timestamp   = timestamp;

    switch(pixFmt) {
        case HAL_PIXEL_FORMAT_YCbCr_420_SP_VENUS:
            cf = COLOR_FMT_NV12;
            bufPtr->length    = 2; // For multiplanar, its the number of planes

            bufPtr->m.planes[0].data_offset = 0;
            bufPtr->m.planes[0].bytesused = sDim.width * sDim.height;
            //VENUS_Y_STRIDE and VENUS_Y_SCANLINES are not required as sStride
            //already has this info
            bufPtr->m.planes[0].length    = VENUS_Y_STRIDE(cf, sStride.width)
                                        * VENUS_Y_SCANLINES(cf, sStride.height);
            bufPtr->m.planes[0].reserved[0] = V4L2_PLANE_MEM_OFFSET;
            bufPtr->m.planes[1].data_offset = 0;
            bufPtr->m.planes[1].bytesused = bufPtr->m.planes[0].bytesused / 2;
            bufPtr->m.planes[1].length    = bufPtr->m.planes[0].length / 2;
            bufPtr->m.planes[1].reserved[0] = bufPtr->m.planes[0].length +
                                              bufPtr->m.planes[0].reserved[0];
            break;

        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            bufPtr->length    = 2; // For multiplanar, its the number of planes

            bufPtr->m.planes[0].data_offset = 0;
            bufPtr->m.planes[0].bytesused = sDim.width * sDim.height;
            //FIXME: Check if any buffer alignment is implied here
            bufPtr->m.planes[0].length    = sStride.width * sStride.height;
            bufPtr->m.planes[0].reserved[0] = V4L2_PLANE_MEM_OFFSET;
            bufPtr->m.planes[1].data_offset = 0;
            bufPtr->m.planes[1].bytesused = bufPtr->m.planes[0].bytesused / 2;
            bufPtr->m.planes[1].length    = bufPtr->m.planes[0].length / 2;
            bufPtr->m.planes[1].reserved[0] = bufPtr->m.planes[0].length +
                                              bufPtr->m.planes[0].reserved[0];
            break;

        case HAL_PIXEL_FORMAT_YCbCr_422_I:
        case HAL_PIXEL_FORMAT_YCrCb_422_I:
            bufPtr->length    = 1; // For multiplanar, its the number of planes

            bufPtr->m.planes[0].data_offset = 0;
            bufPtr->m.planes[0].bytesused = sDim.width * sDim.height;
            bufPtr->m.planes[0].length    = sStride.width * sStride.height;
            bufPtr->m.planes[0].reserved[0] = V4L2_PLANE_MEM_OFFSET;
            break;

        default:
            ALOGE("%s: Unsupported format %d", __func__, pixFmt);
            return BAD_VALUE;
    }
    ALOGD_IF(logAll(), "%s: planes: %d [0].reserved: %d [0].length: %d",
        __func__, bufPtr->length, bufPtr->m.planes[0].reserved[0],
        bufPtr->m.planes[0].length);

    if(bufPtr->length > 1){
        ALOGD_IF(logAll(), "%s: [1].reserved: %d [1].length: %d", __func__,
            bufPtr->m.planes[1].reserved[0], bufPtr->m.planes[1].length);
    }
    if(bufPtr->length > 2){
        ALOGD_IF(logAll(), "%s: [2].reserved: %d [2].length: %d", __func__,
            bufPtr->m.planes[2].reserved[0], bufPtr->m.planes[2].length);
    }

    //TODO: Add fence api when the driver supports it
    releaseFenceFd = -1;

    err = qBuf(fd, bufPtr);
    ERR_CHK((NO_ERROR != err), err);

    return err;
}

status_t VpuIntf::fillAndQOutBuf(const int32_t fd, struct v4l2_buffer* bufPtr,
    const Dim& sStride, const Dim& sDim, const int32_t pixFmt,
    const VFM_FIELD_ORDER eField, struct timeval timestamp)
{
    status_t err = NO_ERROR;
    int cf = COLOR_FMT_NV12;

    ATRACE_CALL();
    if(!bufPtr){
        ALOGE("%s: bufPtr is NULL", __func__);
        return BAD_VALUE;
    }
    bufPtr->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    bufPtr->memory      = V4L2_MEMORY_USERPTR;
    bufPtr->bytesused   = 0; //for multiplanar structure
    bufPtr->field       = getV4L2field(eField);
    bufPtr->timestamp   = timestamp;

    switch(pixFmt) {
        case HAL_PIXEL_FORMAT_RGB_888:
            bufPtr->length    = 1; // For multiplanar, its the number of planes
            bufPtr->m.planes[0].data_offset = 0;
            bufPtr->m.planes[0].bytesused   = 0;//sDim.width * sDim.height * 3;
            bufPtr->m.planes[0].length      = 0;
            //sDim.width * sDim.height * 3;
                                        //sStride.width * sStride.height * 3;
            bufPtr->m.planes[0].reserved[0] = V4L2_PLANE_MEM_OFFSET;

            break;
        case HAL_PIXEL_FORMAT_YCbCr_422_I_10BIT:
        case HAL_PIXEL_FORMAT_YCbCr_422_I_10BIT_COMPRESSED:
            bufPtr->length    = 1; // For multiplanar, its the number of planes
            bufPtr->m.planes[0].data_offset = 0;
            bufPtr->m.planes[0].bytesused   = sDim.width * sDim.height * 2;
            bufPtr->m.planes[0].length      = 0;
                                        //sStride.width * sStride.height * 2;
            bufPtr->m.planes[0].reserved[0] = V4L2_PLANE_MEM_OFFSET;

            break;

        default:
            ALOGE("%s: Unsupported format %d", __func__, pixFmt);
            return BAD_VALUE;
    }

    ALOGD_IF(logAll(), "%s: tv_sec: %ld tv_usec: %ld", __func__,
        timestamp.tv_sec, timestamp.tv_usec);

    ALOGD_IF(logAll(), "%s: [0].reserved: %d [0].length: %d", __func__,
            bufPtr->m.planes[0].reserved[0], bufPtr->m.planes[0].length);
    if(bufPtr->length > 1){
        ALOGD_IF(logAll(), "%s: [1].reserved: %d [1].length: %d", __func__,
            bufPtr->m.planes[1].reserved[0], bufPtr->m.planes[1].length);
    }
    if(bufPtr->length > 2){
        ALOGD_IF(logAll(), "%s: [2].reserved: %d [2].length: %d", __func__,
            bufPtr->m.planes[2].reserved[0], bufPtr->m.planes[2].length);
    }

    err = qBuf(fd, bufPtr);
    ERR_CHK((NO_ERROR != err), err);

    return err;
}

status_t VpuIntf::fillAndQBuf(const int32_t fd, struct v4l2_buffer* bufPtr,
    bool isInputPort, const Dim& sStride, const Dim& sDim, const int32_t pixFmt,
    const VFM_FIELD_ORDER eField, struct timeval timestamp)
{
    status_t err = NO_ERROR;
    err = fillBufferInfo(bufPtr, isInputPort, sStride, sDim, pixFmt, eField,
            timestamp);
    ERR_CHK((NO_ERROR != err), err);

    err = qBuf(fd, bufPtr);
    ERR_CHK((NO_ERROR != err), err);

    return err;
}

/* sets colorspace on input port */
status_t VpuIntf::setInColorSpace(const int32_t fd, int32_t colorspace)
{
    ATRACE_CALL();
    //colorspace parameter is part of v4l2_format
    return NO_ERROR;//setColorSpace(fd, true, colorspace);
}

/* sets colorspace on output port */
status_t VpuIntf::setOutColorSpace(const int32_t fd, int32_t colorspace)
{
    ATRACE_CALL();
    //colorspace parameter is part of v4l2_format
    return NO_ERROR;//setColorSpace(fd, false, colorspace);
}

status_t VpuIntf::setVpuControl(const int32_t fd,
                                const struct vpu_control& vpuCtrl)
{
    status_t err = NO_ERROR;
    int32_t result = 0;

    ATRACE_CALL();
    ALOGD_IF(logAll(), "%s: E control_id: %d", __func__, vpuCtrl.control_id);

#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VPU_S_CONTROL, &vpuCtrl);
    if(result < 0){
        ALOGE("%s: VPU_S_CONTROL failed CtrlId: %d value: %d enable: %d"
        " automode: %d, auto_manual.value: %d", __func__,
        vpuCtrl.control_id, vpuCtrl.data.value,
        vpuCtrl.data.auto_manual.enable, vpuCtrl.data.auto_manual.auto_mode,
        vpuCtrl.data.auto_manual.value);
        return UNKNOWN_ERROR;
    }
#endif
    ALOGD_IF(isDebug(), "%s: VPU_S_CONTROL success CtrlId: %d value: %d "
        "enable: %d automode: %d, auto_manual.value: %d", __func__,
        vpuCtrl.control_id, vpuCtrl.data.value,
        vpuCtrl.data.auto_manual.enable, vpuCtrl.data.auto_manual.auto_mode,
        vpuCtrl.data.auto_manual.value);

    return err;
}

status_t VpuIntf::getVpuControl(const int32_t fd,
                                struct vpu_control& vpuCtrl)
{
    status_t err = NO_ERROR;
    int32_t result = 0;
    ATRACE_CALL();
    ALOGD_IF(logAll(), "%s: E control_id: %d", __func__, vpuCtrl.control_id);

#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VPU_G_CONTROL, &vpuCtrl);
    if(result < 0){
        ALOGE("%s: VPU_G_CONTROL failed CtrlId: %d value: %d enable: %d"
        " automode: %d, auto_manual.value: %d", __func__,
        vpuCtrl.control_id, vpuCtrl.data.value,
        vpuCtrl.data.auto_manual.enable, vpuCtrl.data.auto_manual.auto_mode,
        vpuCtrl.data.auto_manual.value);
        return UNKNOWN_ERROR;
    }
#endif
    ALOGE("%s: VPU_G_CONTROL success CtrlId: %d value: %d enable: %d"
        " automode: %d, auto_manual.value: %d", __func__,
        vpuCtrl.control_id, vpuCtrl.data.value,
        vpuCtrl.data.auto_manual.enable, vpuCtrl.data.auto_manual.auto_mode,
        vpuCtrl.data.auto_manual.value);

    return err;
}

status_t VpuIntf::setVpuControlExtended(const int32_t fd,
            const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg)
{
    status_t err = NO_ERROR;
    int32_t result = 0;
    struct vpu_control_extended vpuCtrlExt;

    ATRACE_CALL();
    ALOGD_IF(logAll(), "%s: E ", __func__);

    vpuCtrlExt.type         = inArg.iaVpuCtrlExt.type;
    vpuCtrlExt.data_len     = inArg.iaVpuCtrlExt.dataLength;
    vpuCtrlExt.data_ptr     = (void*)inArg.iaVpuCtrlExt.dataBuffer;

#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VPU_S_CONTROL_EXTENDED, &vpuCtrlExt);
    if(result < 0){
        ALOGE("%s: VPU_S_CONTROL_EXT failed type: %d length: %d", __func__,
        vpuCtrlExt.type, vpuCtrlExt.data_len);
        return UNKNOWN_ERROR;
    }
    outArg.oaVpuCtrlExt.responseLength = vpuCtrlExt.buf_size;
    if(outArg.oaVpuCtrlExt.responseLength > VPU_MAX_EXT_DATA_SIZE){
        memcpy(outArg.oaVpuCtrlExt.responseBuffer, vpuCtrlExt.buf_ptr,
            outArg.oaVpuCtrlExt.responseLength);
    }else{
        outArg.oaVpuCtrlExt.responseLength = 0;
    }

#endif
    ALOGD_IF(isDebug(), "%s: VPU_S_CONTROL_EXT success type: %d length: %d "
            "data[0]: %c respLen: %d, resp[0]: %c", __func__,
        vpuCtrlExt.type, vpuCtrlExt.data_len, inArg.iaVpuCtrlExt.dataBuffer[0],
        vpuCtrlExt.buf_size, outArg.oaVpuCtrlExt.responseBuffer[0]);

    return err;
}

status_t VpuIntf::getVpuControlExtended(const int32_t fd,
            const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg)
{
    status_t err = NO_ERROR;
    int32_t result = 0;
    struct vpu_control_extended vpuCtrlExt;

    ATRACE_CALL();
    ALOGD_IF(logAll(), "%s: E ", __func__);

    vpuCtrlExt.type         = inArg.iaVpuCtrlExt.type;
    vpuCtrlExt.data_len     = inArg.iaVpuCtrlExt.dataLength;
    vpuCtrlExt.data_ptr     = (void*)inArg.iaVpuCtrlExt.dataBuffer;

#if !DBG_DISABLE_IOCTL
    result = ioctl(fd, VPU_G_CONTROL_EXTENDED, &vpuCtrlExt);
    if(result < 0){
        ALOGE("%s: VPU_G_CONTROL_EXT failed type: %d length: %d", __func__,
        vpuCtrlExt.type, vpuCtrlExt.data_len);
        return UNKNOWN_ERROR;
    }
    outArg.oaVpuCtrlExt.responseLength = vpuCtrlExt.buf_size;
    if(outArg.oaVpuCtrlExt.responseLength > VPU_MAX_EXT_DATA_SIZE){
        memcpy(outArg.oaVpuCtrlExt.responseBuffer, vpuCtrlExt.buf_ptr,
            outArg.oaVpuCtrlExt.responseLength);
    }else{
        outArg.oaVpuCtrlExt.responseLength = 0;
    }

#endif
    ALOGD_IF(isDebug(), "%s: VPU_G_CONTROL_EXT success type: %d length: %d "
            "data[0]: %c respLen: %d, resp[0]: %c", __func__,
        vpuCtrlExt.type, vpuCtrlExt.data_len, inArg.iaVpuCtrlExt.dataBuffer[0],
        vpuCtrlExt.buf_size, outArg.oaVpuCtrlExt.responseBuffer[0]);

    return err;
}

/*
    Fills pixelFormat, sizeimage and bytesperline parameters
    numPlanes and sizeImage is ignored by the vpu driver implementation
    bytesperline: should be a multiple of 128. otherwise, its ignored by
    the driver and instead a calculated value is used internally
*/
status_t VpuIntf::getV4L2pixelFormatBasedParams(const int32_t pixFmt,
    const Dim& sStride, const Dim& sDim, v4l2_format& format)
{
    status_t err = NO_ERROR;
    int32_t cf = COLOR_FMT_NV12;
    switch(pixFmt){
        case HAL_PIXEL_FORMAT_RGB_888:
            format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_BGR24;
            format.fmt.pix_mp.num_planes  = 1;
            format.fmt.pix_mp.plane_fmt[0].bytesperline = sStride.width * 3;
            format.fmt.pix_mp.plane_fmt[0].sizeimage =
                format.fmt.pix_mp.plane_fmt[0].bytesperline * sStride. height;
            break;
        case HAL_PIXEL_FORMAT_RGBX_8888:
            format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_RGB32;
            format.fmt.pix_mp.num_planes  = 1;
            format.fmt.pix_mp.plane_fmt[0].bytesperline = sStride.width * 4;
            format.fmt.pix_mp.plane_fmt[0].sizeimage =
                format.fmt.pix_mp.plane_fmt[0].bytesperline * sStride. height;
            break;
        case HAL_PIXEL_FORMAT_BGRX_8888:
            format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_BGR32;
            format.fmt.pix_mp.num_planes  = 1;
            format.fmt.pix_mp.plane_fmt[0].bytesperline = sStride.width * 4;
            format.fmt.pix_mp.plane_fmt[0].sizeimage =
                format.fmt.pix_mp.plane_fmt[0].bytesperline * sStride.height;
            break;
        case HAL_PIXEL_FORMAT_YCbCr_422_I:
            format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_YUYV;
            format.fmt.pix_mp.num_planes  = 1;
            format.fmt.pix_mp.plane_fmt[0].bytesperline = sStride.width * 2;
            format.fmt.pix_mp.plane_fmt[0].sizeimage =
                format.fmt.pix_mp.plane_fmt[0].bytesperline * sStride.height;
            break;
        case HAL_PIXEL_FORMAT_YCrCb_422_I:
            format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_YVYU;
            format.fmt.pix_mp.num_planes  = 1;
            format.fmt.pix_mp.plane_fmt[0].bytesperline = sStride.width * 2;
            format.fmt.pix_mp.plane_fmt[0].sizeimage
                = format.fmt.pix_mp.plane_fmt[0].bytesperline * sStride.height;
            break;
        case HAL_PIXEL_FORMAT_YCbCr_422_I_10BIT_COMPRESSED:
            format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_YUYV10BWC;
            format.fmt.pix_mp.plane_fmt[0].bytesperline = sStride.width * 8/3;
            format.fmt.pix_mp.plane_fmt[0].sizeimage =
                format.fmt.pix_mp.plane_fmt[0].bytesperline * sStride.height;
            break;
        case HAL_PIXEL_FORMAT_YCbCr_422_I_10BIT:
            format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_YUYV10;
            format.fmt.pix_mp.plane_fmt[0].bytesperline = sStride.width * 3;
            format.fmt.pix_mp.plane_fmt[0].sizeimage =
                format.fmt.pix_mp.plane_fmt[0].bytesperline * sStride.height;
            break;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV21;
            format.fmt.pix_mp.num_planes  = 2;
            format.fmt.pix_mp.plane_fmt[0].bytesperline = sStride.width;
            format.fmt.pix_mp.plane_fmt[0].sizeimage
                = format.fmt.pix_mp.plane_fmt[0].bytesperline * sStride.height;
            format.fmt.pix_mp.plane_fmt[1].bytesperline = sStride.width;
            format.fmt.pix_mp.plane_fmt[1].sizeimage
            = format.fmt.pix_mp.plane_fmt[1].bytesperline * sStride.height / 2;

            break;
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
            format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12;
            format.fmt.pix_mp.num_planes  = 2;
            format.fmt.pix_mp.plane_fmt[0].bytesperline = sStride.width;
            format.fmt.pix_mp.plane_fmt[0].sizeimage
                = format.fmt.pix_mp.plane_fmt[0].bytesperline * sStride.height;
            format.fmt.pix_mp.plane_fmt[1].bytesperline = sStride.width;
            format.fmt.pix_mp.plane_fmt[1].sizeimage
            = format.fmt.pix_mp.plane_fmt[1].bytesperline * sStride.height / 2;

            break;
        case HAL_PIXEL_FORMAT_YCbCr_420_SP_VENUS:
            cf = COLOR_FMT_NV12;
            format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12;
            format.fmt.pix_mp.num_planes  = 2;
            //VENUS_Y_STRIDE and VENUS_Y_SCANLINES are not required as sStride
            //already has this info
            format.fmt.pix_mp.plane_fmt[0].bytesperline =
                   VENUS_Y_STRIDE(cf, sStride.width);
            format.fmt.pix_mp.plane_fmt[0].sizeimage    =
                   VENUS_Y_STRIDE(cf, sStride.width) * VENUS_Y_SCANLINES(cf,
                       sStride.height);
            format.fmt.pix_mp.plane_fmt[1].bytesperline =
                   VENUS_UV_STRIDE(cf, sStride.width);
            format.fmt.pix_mp.plane_fmt[1].sizeimage    =
                   VENUS_UV_STRIDE(cf, sStride.width) * VENUS_UV_SCANLINES(cf,
                       sStride.height);

            break;
        default:
            ALOGE("%s: Unsupported format %d", __func__, pixFmt);
            err = BAD_VALUE;
    }
    //Bytes per line set to 0 so that driver calculates the smallest
    //possible bytes per line
    for(int32_t i = 0; i < VIDEO_MAX_PLANES; i++){
        format.fmt.pix_mp.plane_fmt[i].bytesperline = 0;
        format.fmt.pix_mp.plane_fmt[i].sizeimage = 0;
    }
    ALOGD_IF(isDebug(), "%s: pix: %d, num_planes: %d, [0].bpl:%d "
        "[0].sizeimage: %d [1].bpl: %d [1].sizeimage: %d", __func__,
        format.fmt.pix_mp.pixelformat, format.fmt.pix_mp.num_planes,
        format.fmt.pix_mp.plane_fmt[0].bytesperline,
        format.fmt.pix_mp.plane_fmt[0].sizeimage,
        format.fmt.pix_mp.plane_fmt[1].bytesperline,
        format.fmt.pix_mp.plane_fmt[1].sizeimage);
    return err;
}

int32_t VpuIntf::getV4L2numPlanes(const int32_t pixFmt)
{
    int32_t numPlanes = 2;

    switch(pixFmt){
        case HAL_PIXEL_FORMAT_RGB_888:
        case HAL_PIXEL_FORMAT_YCbCr_422_I_10BIT:
        case HAL_PIXEL_FORMAT_YCbCr_422_I_10BIT_COMPRESSED:
        case HAL_PIXEL_FORMAT_YCbCr_422_I:
        case HAL_PIXEL_FORMAT_YCrCb_422_I:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRX_8888:
            numPlanes = 1;
            break;

        case HAL_PIXEL_FORMAT_YCbCr_420_SP_VENUS:
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
            numPlanes = 2;
            break;
        default:
            ALOGE("%s: Unsupported format %d", __func__, pixFmt);
    }
    ALOGD_IF(isDebug(), "%s: num_planes: %d", __func__, numPlanes);
    return numPlanes;
}

/*
    The buffer "fd" and index is not populated by this function. It is prefilled
    in bufPtr before calling this function
    bytesused parameter is ignored by the driver
    data_offset=0 for all planes
    plane[i + 1].reserved[0] = plane[i].length + plane[i].reserved[0]
*/
status_t VpuIntf::fillBufferInfo(struct v4l2_buffer* bufPtr, bool isInputPort,
    const Dim& sStride, const Dim& sDim, const int32_t pixFmt,
    const VFM_FIELD_ORDER eField, struct timeval timestamp)
{
    status_t err = NO_ERROR;
    int cf = COLOR_FMT_NV12;

    bufPtr->type = (isInputPort) ? V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE :
                                        V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    bufPtr->memory      = V4L2_MEMORY_USERPTR;
    bufPtr->bytesused   = 0; //for multiplanar structure
    bufPtr->field       = getV4L2field(eField);
    bufPtr->timestamp   = timestamp;

    switch(pixFmt) {
        case HAL_PIXEL_FORMAT_YCbCr_420_SP_VENUS:
            cf = COLOR_FMT_NV12;
            bufPtr->length    = 2; // For multiplanar, its the number of planes

            bufPtr->m.planes[0].data_offset = 0;
            bufPtr->m.planes[0].bytesused = sDim.width * sDim.height;
            //VENUS_Y_STRIDE and VENUS_Y_SCANLINES are not required as sStride
            //already has this info
            bufPtr->m.planes[0].length    = VENUS_Y_STRIDE(cf, sStride.width)
                                        * VENUS_Y_SCANLINES(cf, sStride.height);
            bufPtr->m.planes[0].reserved[0] = V4L2_PLANE_MEM_OFFSET;
            bufPtr->m.planes[1].data_offset = 0;
            bufPtr->m.planes[1].bytesused = bufPtr->m.planes[0].bytesused / 2;
            bufPtr->m.planes[1].length    = bufPtr->m.planes[0].length / 2;
            bufPtr->m.planes[1].reserved[0] = bufPtr->m.planes[0].length +
                                              bufPtr->m.planes[0].reserved[0];

            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            bufPtr->length    = 1; // For multiplanar, its the number of planes
            bufPtr->m.planes[0].data_offset = 0;
            bufPtr->m.planes[0].bytesused   = sDim.width * sDim.height * 3;
            bufPtr->m.planes[0].length      =
                                            sStride.width * sStride.height * 3;
            bufPtr->m.planes[0].reserved[0] = V4L2_PLANE_MEM_OFFSET;

            break;
        case HAL_PIXEL_FORMAT_YCbCr_422_I_10BIT:
        case HAL_PIXEL_FORMAT_YCbCr_422_I_10BIT_COMPRESSED:
            bufPtr->length    = 1; // For multiplanar, its the number of planes
            bufPtr->m.planes[0].data_offset = 0;
            bufPtr->m.planes[0].bytesused   = sDim.width * sDim.height * 2;
            bufPtr->m.planes[0].length      =
                                            sStride.width * sStride.height * 2;
            bufPtr->m.planes[0].reserved[0] = V4L2_PLANE_MEM_OFFSET;

            break;

        default:
            ALOGE("%s: Unsupported format %d", __func__, pixFmt);
            return BAD_VALUE;
    }
    ALOGD_IF(logAll(), "%s: [0].reserved: %d [0].length: %d", __func__,
            bufPtr->m.planes[0].reserved[0], bufPtr->m.planes[0].length);
    if(bufPtr->length > 1){
        ALOGD_IF(logAll(), "%s: [1].reserved: %d [1].length: %d", __func__,
            bufPtr->m.planes[1].reserved[0], bufPtr->m.planes[1].length);
    }
    if(bufPtr->length > 2){
        ALOGD_IF(logAll(), "%s: [2].reserved: %d [2].length: %d", __func__,
            bufPtr->m.planes[2].reserved[0], bufPtr->m.planes[2].length);
    }

    return err;
}

v4l2_field VpuIntf::getV4L2field(VFM_FIELD_ORDER eField){
    switch(eField){
        case VFM_FIELD_ORDER_NONE:          return V4L2_FIELD_NONE;
        case VFM_FIELD_ORDER_ANY:           return V4L2_FIELD_ANY;
        case VFM_FIELD_ORDER_TOP:           return V4L2_FIELD_TOP;
        case VFM_FIELD_ORDER_BOTTOM:        return V4L2_FIELD_BOTTOM;
        case VFM_FIELD_ORDER_SEQ_TB:        return V4L2_FIELD_SEQ_TB;
        case VFM_FIELD_ORDER_SEQ_BT:        return V4L2_FIELD_SEQ_BT;
        case VFM_FIELD_ORDER_ALTERNATE:     return V4L2_FIELD_ALTERNATE;
        case VFM_FIELD_ORDER_INTERLACED_TB: return V4L2_FIELD_INTERLACED_TB;
        case VFM_FIELD_ORDER_INTERLACED_BT: return V4L2_FIELD_INTERLACED_BT;
        default:                            return V4L2_FIELD_NONE;
    }
}

status_t VpuIntf::getBufferSize(const Dim& sDim, const int32_t pixFmt,
                                    int32_t& memsize, Dim& sStride)
{
    float bpp[3] = {0, 0, 0};
    int32_t paddedBufferWidth = 0;
    int32_t bufferHeight = 0;
    int32_t pitch = 0;

    switch(pixFmt){
        case HAL_PIXEL_FORMAT_RGB_888:
        //TODO: Over-allocating memory for YCbCr_422_I_10BIT until the feature
        //stabilized
        case HAL_PIXEL_FORMAT_YCbCr_422_I_10BIT:
        case HAL_PIXEL_FORMAT_YCbCr_422_I_10BIT_COMPRESSED:
            pitch = align(sDim.width * 3, 128);
            memsize = pitch * sDim.height;
            break;
        default:
            ALOGE("%s: Unsupported format %d", __func__, pixFmt);
            return BAD_VALUE;
    }

    ALOGD_IF(isDebug(), "%s: memsize: %d pitch: %d stride w: %d h:%d",
       __func__, memsize, pitch, sStride.width, sStride.height);
    return NO_ERROR;
}

/* Maps colorspace to driver enums */
// Default value is not defined in the VPU driver yet
#define VPU_CS_DEFAULT (0)
v4l2_colorspace VpuIntf::getV4L2colorspace(const int32_t colorspace){
    switch(colorspace){
#ifdef VPU_DRIVER_AVAILABLE
#ifdef MPQ_VPU_DRIVER_EXTENSIONS
        case VFM_CLR_SMPTE_601_FULL:return VPU_CS_REC601_FULL;
        case VFM_CLR_SMPTE_709_FULL:return VPU_CS_REC709_FULL;
        case VFM_CLR_sRGB_FULL:     return VPU_CS_RGB_FULL;
        case VFM_CLR_SMPTE_601_LTD: return VPU_CS_REC601_LIMITED;
        case VFM_CLR_SMPTE_709_LTD: return VPU_CS_REC709_LIMITED;
        case VFM_CLR_sRGB_LTD:      return VPU_CS_RGB_LIMITED;
#endif //MPQ_VPU_DRIVER_EXTENSIONS
#endif //VPU_DRIVER_AVAILABLE
        default:                    return (v4l2_colorspace)VPU_CS_DEFAULT;
    }
}


}; //namespace VPU
