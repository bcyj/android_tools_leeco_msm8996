/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/*****************************************************************************
    This file contains inline utility functions and utility structures
*****************************************************************************/

#ifndef VFM_UTILS_H
#define VFM_UTILS_H
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <linux/msm_ion.h>
#include <sys/mman.h>
#include <pthread.h>
#include <utils/Timers.h>
#include <binder/Parcel.h>
#include <utils/Mutex.h>
#include "utils/String8.h"
#include "gralloc_priv.h"
#include <alloc_controller.h>
#include <memalloc.h>
#include "qdMetaData.h"
#include <cutils/properties.h>

#include "vfm_defs.h"
#include "vfm_metadata.h"
#include "vfm_cmds.h"
#include "vfm_eventthread.h"

//using namespace android;
namespace vpu {

/*****************************************************************************/
/* Constant / Define Declarations                                            */
/*****************************************************************************/
#define MAX_PROPERTY_LENGTH (128)

#define MAX_VCAP_PIPE_PER_SESSION (2)

#define ERR_CHK(error_condition, error_to_be_returned) {\
            if(error_condition) {\
                return error_to_be_returned; \
            }\
}\

#define ERR_CHK_LOG(err_condition, ret_err, err_str) {\
            if(err_condition) {\
                ALOGE("%s: %s", __func__, err_str);\
                return ret_err; \
            }\
}\

/*****************************************************************************/
/* Enumerations                                                              */
/*****************************************************************************/
/*! Source of the input data */
typedef enum {
    //! Data is queued from SF to HWC/VFM
    INPUT_SRC_SF_QBUF       = 0x00,

    //! VCAP h/w tunnels the data to VPU h/w
    INPUT_SRC_VCAP_TUNNEL   = 0x01,

    //! Data is queued directly to VFM bypassing SF
    INPUT_SRC_VFM_TUNNEL    = 0x02,

    INPUT_SRC_INV
} INPUT_SOURCE_TYPE;

//! Output destination for each VPU session
typedef enum {
    OUT_DEST_MDSS_TUNNEL = 0x00,  //<! Most common case
    OUT_DEST_VDP_WB,              //<! TAD cases
    OUT_DEST_HLOS_DQ,             //<! Not used
    OUT_DEST_FRC_WB,              //<! Not used
    OUT_DEST_VPU_WB               //<! Not used
} OUTPUT_DEST_TYPE;

/*! VCAP data pipe associated with a VPU session */
typedef enum {
    SRC_VCAP_PIPE0 = 0x01,
    SRC_VCAP_PIPE1 = 0x02,
    SRC_VCAP_PIPE_INV
} SRC_VCAP_PIPE;

//! FRC Modes
//TODO: W API
typedef enum
{
    FRC_BYPASS = 0x00, //<! FRC will be bypassed
    FRC_ON,            //<! FRC is inline and interpolation is turned ON
    FRC_OFF            //<! FRC is inline but interpolation is turned OFF
} FRC_MODE;

//! Deinterlacer mode
typedef enum
{
    DEI_MODE_AUTO               = 0x00,
    DEI_MODE_VIDEO              = 0x01,
    DEI_MODE_FILM               = 0x02,
    DEI_MODE_INV
} DEI_MODE;

//! Scaler's filter modes
typedef enum
{
    CHR_FILT_NONE                = 0x00,
    CHR_FILT_VERTICAL            = 0x01,
    CHR_FILT_CROSS               = 0x02,
    CHR_FILT_INV
} CHROMA_FILTER;

//! Event list reported in callbacks
//TODO: W exhaustive event list
typedef enum
{
    EVENT_ACTIVE_REGION_CHANGED,
    EVENT_INV
} EVENTS;

/*****************************************************************************/
/* Type declarations                                                         */
/*****************************************************************************/
struct VPUCapabilities {
    int numSessions;
};

//! Source pipe information
struct SourcePipes
{
    int32_t        numPipes;
    SRC_VCAP_PIPE  pipe[MAX_VCAP_PIPE_PER_SESSION];
public:
    inline bool operator==(const SourcePipes& rhs) const {
        if(numPipes == rhs.numPipes) {
            for(int32_t i = 0; i < numPipes; i++) {
                if(pipe[i] != rhs.pipe[i])
                    return false;
            }
            return true;
        }else {
            return false;
        }
    }
    inline bool operator!=(const SourcePipes& rhs) const {
        return (!operator==(rhs));
    }
    String8 dump() const{
        String8 str("\n");
        char s[128];

        snprintf(s, 128,"numPipes: %d \n", numPipes); str += s;
        for(int i = 0; i < MAX_VCAP_PIPE_PER_SESSION; i++){
            snprintf(s, 128,"pipe[%d]: %d \n", i, pipe[i]); str += s;
        }
        return str;
    }
};

struct InputSource
{
    INPUT_SOURCE_TYPE eInputSrcType;
    SourcePipes       sSourcePipes;          //<! Valid only if source is VCAP
public:
    inline bool operator==(const InputSource& rhs) const {
        if(eInputSrcType != rhs.eInputSrcType) return false;
        if(eInputSrcType == INPUT_SRC_VCAP_TUNNEL)
            return (sSourcePipes == rhs.sSourcePipes);
        else
            return true;
    }
    inline bool operator!=(const InputSource& rhs) const {
        return (!operator==(rhs));
    }
    String8 dump() const{
        String8 str("\n");
        char s[128];

        snprintf(s, 128,"srcType: %d \n", eInputSrcType); str += s;
        if(eInputSrcType == INPUT_SRC_VCAP_TUNNEL){
            snprintf(s, 128,"srcPipes: %s \n", sSourcePipes.dump().string());
            str += s;
        }
        return str;
    }
};

struct OutputDestination
{
    OUTPUT_DEST_TYPE eOutputDestType;
    DestinationPipes sDestPipes;
public:
    inline bool operator==(const OutputDestination& rhs) const {
        if(eOutputDestType != rhs.eOutputDestType) return false;
        if(eOutputDestType == OUT_DEST_MDSS_TUNNEL)
            return (sDestPipes == rhs.sDestPipes);
        else
            return true;
    }
    inline bool operator!=(const OutputDestination& rhs) const {
        return (!operator==(rhs));
    }
    String8 dump() const{
        String8 str("\n");
        char s[128];

        snprintf(s, 128,"destType: %d \n", eOutputDestType); str += s;
        if(eOutputDestType == OUT_DEST_MDSS_TUNNEL){
            snprintf(s, 128,"destPipes: %s \n", sDestPipes.dump().string());
            str += s;
        }
        return str;
    }
};

//! Geometry and format of the buffers that will be Queued
struct InputCfg {
    /*!
        sStride is the memory stride of the buffers - typically decided by
            the hardware core's alignment and padding requirements
        sDim is the resolution / actual dimension of the video - typically
            the decoded output dimension or vcap capture dimension
        sCrop is the region of interest to be cropped for scaler's input
        sCrop is used to achieve view modes - aspect ratio corrections
        Relation:
            sStride >= sDim >= sCrop
    */
    Dim               sStride;
    Dim               sDim;
    Rect              sCrop;

    int32_t           isInterlaced;
    VFM_FIELD_ORDER   eField;

    int32_t           is3d;
    //! Defined in gralloc_priv.h/vfm_metadata.h
    int32_t           e3Dformat;

    //! Defined in graphics.h and gralloc_priv.h
    int32_t           pixFmt;

    //!  colorspace listed in vfm_matadata.h
    int32_t           colorspace;

    //! inputFps in frames per 100 seconds. fps * 100
    int32_t           framerate;

    InputSource       inSrc;

    //! Secure session flag
    int32_t           isSecureSession;
public:
    inline bool operator==(const InputCfg& rhs) const {
        if(sStride  != rhs.sStride)         return false;
        if(sDim  != rhs.sDim)               return false;
        if(sCrop != rhs.sCrop)              return false;
        if(isInterlaced  != rhs.isInterlaced)  return false;
        if(isInterlaced){
            if(eField != rhs.eField)        return false;
        }
        if(is3d  != rhs.is3d)               return false;
        if(is3d) {
            if(e3Dformat != rhs.e3Dformat)  return false;
        }
        if(pixFmt != rhs.pixFmt)            return false;
        if(colorspace != rhs.colorspace)    return false;
        if(framerate != rhs.framerate)      return false;
        if(inSrc != rhs.inSrc)              return false;
        return true;
    }
    inline bool operator!=(const InputCfg& rhs) const {
        return(! operator==(rhs));
    }
    String8 dump() const {
        String8 str("\n");
        char s[128];

        snprintf(s, 128,"Stride: %s \n", sStride.dump().string()); str += s;
        snprintf(s, 128,"Dim: %s \n", sDim.dump().string()); str += s;
        snprintf(s, 128,"Crop: %s \n", sCrop.dump().string()); str += s;
        snprintf(s, 128,"isInterlace: %d \n", isInterlaced); str += s;
        if(isInterlaced) { snprintf(s, 128,"eField: %d \n", eField); str += s;}
        snprintf(s, 128,"is3d: %d \n", is3d); str += s;
        if(is3d) { snprintf(s, 128,"e3Dformat: %d \n", e3Dformat); str += s;}
        snprintf(s, 128,"pixFmt: %d \n", pixFmt); str += s;
        snprintf(s, 128,"colorspace: %d \n", colorspace); str += s;
        snprintf(s, 128,"framerate: %d \n", framerate); str += s;
        snprintf(s, 128,"inSrc: %s \n", inSrc.dump().string()); str += s;
        return str;
    }
};

struct OutputCfg {
    /* Out buffer dimension. sDim >= tgtRect */
    Dim                 sDim;
    /*!
     Target rect is the "Surface" dimension set by the app. Output cannot be
        scaled beyond this rectangle.
     Destination rectangle is the "active" region into which output is
        written into.
     When tgtRect > dstRect, the gap is padded with color
     When tgtRect < dstRect, dstRect is clamped to the intersecting rectangle
    */
    Rect                tgtRect;
    Rect                dstRect;
    int32_t             is3d;
    int32_t             e3Dformat;
    int32_t             framerate;
    //eField is VFM_FIELD_ORDER_NONE for MDSS tunneling
    VFM_FIELD_ORDER     eField;
    int32_t             pixFmt;
    int32_t             colorspace;
    //! TODO: Secure session flag
public:
    inline bool operator==(const OutputCfg& rhs) const {
        if(sDim        != rhs.sDim)         return false;
        if(tgtRect     != rhs.tgtRect)      return false;
        if(dstRect     != rhs.dstRect)      return false;
        if(is3d  != rhs.is3d)               return false;
        if(is3d) {
            if(e3Dformat != rhs.e3Dformat)  return false;
        }
        if(framerate   != rhs.framerate)    return false;
        if(eField      != rhs.eField)       return false;
        if(pixFmt      != rhs.pixFmt)       return false;
        if(colorspace  != rhs.colorspace)   return false;
        return true;
    }
    inline bool operator!=(const OutputCfg& rhs) const {
        return(! operator==(rhs));
    }
    String8 dump() const{
        String8 str("\n");
        char s[128];

        snprintf(s, 128,"sDim(outbuf): %s \n", sDim.dump().string()); str += s;
        snprintf(s, 128,"tgtRect: %s \n", tgtRect.dump().string()); str += s;
        snprintf(s, 128,"dstRect: %s \n", dstRect.dump().string()); str += s;
        snprintf(s, 128,"is3d: %d \n", is3d); str += s;
        if(is3d) { snprintf(s, 128,"e3Dformat: %d \n", e3Dformat); str += s;}
        snprintf(s, 128,"framerate: %d \n", framerate); str += s;
        snprintf(s, 128,"eField: %d \n", eField); str += s;
        snprintf(s, 128,"pixFmt: %d \n", pixFmt); str += s;
        snprintf(s, 128,"colorspace: %d \n", colorspace); str += s;
        return str;
    }
};

/*****************************************************************************/
/* inline functions                                                          */
/*****************************************************************************/
inline int32_t getProperty(const char* propKeyStr)
{
    char propValueStr[MAX_PROPERTY_LENGTH] = {0};
    int32_t propValue = 0;
    property_get(propKeyStr, propValueStr, "0");
    propValue = atoi(propValueStr);
    return propValue;
}

inline int32_t gettimeofdayMs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

inline struct timeval ns2timeval(int64_t timeNs){
    struct timeval tv;

    tv.tv_sec = ns2s(timeNs);
    timeNs -= s2ns(tv.tv_sec);
    tv.tv_usec = ns2us(timeNs);

    return tv;
}

/* returns appropriate VFM metadata structure from the array */
inline VfmMetaData_t* getVfmMData(MetaData_t *metadata, VFM_METADATA_TYPE type)
{
    VfmMetaData_t* pVfmData = NULL;

    if(metadata->vfmDataBitMap & type) {
        int32_t idx = 0;
        idx = getVfmDataIdx(type);
        pVfmData = reinterpret_cast<VfmMetaData_t *>(&metadata->vfmData[idx]);
    }
    return pVfmData;
}

inline int32_t isVcapSource(private_handle_t *hnd){
    if(!hnd)    return 0;
    MetaData_t* metadata = (MetaData_t *)hnd->base_metadata;

    ALOGD_IF(DBG_MD, "md->op: %x vfm->bits: %x", metadata->operation,
        metadata->vfmDataBitMap);
    if(metadata && (metadata->operation & PP_PARAM_VFM_DATA)){
        VfmMetaData_t* pVfmData= getVfmMData(metadata, VFM_DUMMY_BUF);
        if(pVfmData){
            ALOGD_IF(DBG_MD, "%s: isDataTunnelVCAP: %d", __func__,
                        pVfmData->dummyBuf.isDataTunnelVCAP);
            return pVfmData->dummyBuf.isDataTunnelVCAP;
        }
    }
    return 0;
}

inline int32_t isVfmTunnel(private_handle_t *hnd){
    if(!hnd)    return 0;
    MetaData_t* metadata = (MetaData_t *)hnd->base_metadata;

    ALOGD_IF(DBG_MD, "md->op: %x vfm->bits: %x", metadata->operation,
        metadata->vfmDataBitMap);
    if(metadata && (metadata->operation & PP_PARAM_VFM_DATA)){
        VfmMetaData_t* pVfmData= getVfmMData(metadata, VFM_DUMMY_BUF);
        if(pVfmData){
            ALOGD_IF(DBG_MD, "%s: isDataTunnelVFM: %d", __func__,
                        pVfmData->dummyBuf.isDataTunnelVFM);
            return pVfmData->dummyBuf.isDataTunnelVFM;
        }
    }
    return 0;
}

inline int32_t isYuvBuffer(private_handle_t *hnd){
    return (hnd && (hnd->bufferType == BUFFER_TYPE_VIDEO));
}

inline void metadata2InBufGeometry(private_handle_t *hnd, InputCfg& inCfg){
    if(!hnd)        return;
    MetaData_t* metadata = (MetaData_t *)hnd->base_metadata;

    if(metadata && (metadata->operation & PP_PARAM_VFM_DATA)){
        VfmMetaData_t* pVfmData= getVfmMData(metadata, VFM_BUFFER_GEOMETRY);
        if(pVfmData){
            inCfg.sStride.width     = pVfmData->bufGmtry.width;
            inCfg.sStride.height    = pVfmData->bufGmtry.height;
            inCfg.sDim.width        = pVfmData->bufGmtry.right -
                                        pVfmData->bufGmtry.left;
            inCfg.sDim.height       = pVfmData->bufGmtry.bottom -
                                        pVfmData->bufGmtry.top;
            inCfg.sCrop.right       = pVfmData->bufGmtry.right;
            inCfg.sCrop.top         = pVfmData->bufGmtry.top;
            inCfg.sCrop.left        = pVfmData->bufGmtry.left;
            inCfg.sCrop.bottom      = pVfmData->bufGmtry.bottom;
            inCfg.pixFmt            = pVfmData->bufGmtry.pixelFormat;
        }
    }
    return;
}

/* translates layer stride and source rectangle to inCfg params */
inline void layer2InBufGeometry(Layer* layer, InputCfg& inCfg){
    inCfg.sStride       = layer->srcStride;
    //TODO: sDim should not be derived from the source crop. sDim should be an
    //independent information set in metadata
    inCfg.sDim.width    = layer->srcRect.right - layer->srcRect.left;
    inCfg.sDim.height   = layer->srcRect.bottom - layer->srcRect.top;

    inCfg.sCrop         = layer->srcRect;
}

inline void metadata2InInterlace(private_handle_t *hnd, InputCfg& inputCfg){
    if(!hnd)        return;
    MetaData_t* metadata = (MetaData_t *)hnd->base_metadata;

    //Default
    inputCfg.isInterlaced = 0;
    inputCfg.eField = VFM_FIELD_ORDER_NONE;
    if(metadata && (metadata->operation & PP_PARAM_INTERLACED) &&
        metadata->interlaced){
        inputCfg.isInterlaced = 1;
    }

    if(inputCfg.isInterlaced){
        //Default
        inputCfg.eField = VFM_FIELD_ORDER_INTERLACED_TB;
        if(metadata && (metadata->operation & PP_PARAM_VFM_DATA)){
            VfmMetaData_t* pVfmData= getVfmMData(metadata,
                                         VFM_INTLC_FLD_ORDER);
            if(pVfmData){
                inputCfg.eField = pVfmData->eField;
            }
        }
    }
    return;
}

inline void layer2InInterlace(Layer* layer, InputCfg& inCfg){
    return metadata2InInterlace(layer->handle, inCfg);
}

inline void metadata2In3D(private_handle_t *hnd, InputCfg& inputCfg){
    if(!hnd)        return;
    MetaData_t* metadata = (MetaData_t *)hnd->base_metadata;
    //Default
    inputCfg.is3d = 0;
    ALOGD_IF(DBG_MD, "md->op: %x vfm->bits: %x", metadata->operation,
        metadata->vfmDataBitMap);
    if(metadata && (metadata->operation & PP_PARAM_VFM_DATA)){
        VfmMetaData_t* pVfmData= getVfmMData(metadata, VFM_PARAM_3D);
        if(pVfmData){
            inputCfg.is3d = 1;
            inputCfg.e3Dformat = pVfmData->e3Dformat;
            ALOGD_IF(DBG_MD, "%s: is3d: %d 3dfmt: %d", __func__,
                inputCfg.is3d, inputCfg.e3Dformat);
        }
    }
    return;
}

inline void layer2In3D(Layer* layer, InputCfg& inCfg){
    return metadata2In3D(layer->handle, inCfg);
}

inline void layer2InPixFmt(Layer* layer, InputCfg& inCfg){
    private_handle_t* hnd = layer->handle;
    if(!hnd)        return;
    inCfg.pixFmt = hnd->format;
}

inline void metadata2InColorSpace(const private_handle_t* hnd,
                int32_t& colorspace){
    if(!hnd)        return;

    //default
    colorspace = VFM_CLR_SMPTE_709_LTD;
    if(hnd){
        MetaData_t *metadata = (MetaData_t *)hnd->base_metadata;
        if(metadata && (metadata->operation & PP_PARAM_VFM_DATA)){
            VfmMetaData_t* pVfmData = getVfmMData(metadata,
                                        VFM_PARAM_COLORSPACE);
            if(pVfmData){
                colorspace = pVfmData->eColorSpace;
            }
        }
    }
    return;
}

inline void metadata2InColorSpace(const private_handle_t* hnd,
                InputCfg& inCfg){
    return metadata2InColorSpace(hnd, inCfg.colorspace);
}

inline void layer2InColorSpace(const Layer* layer, int32_t& colorspace){
    return metadata2InColorSpace(layer->handle, colorspace);
}

inline void layer2InColorSpace(const Layer* layer, InputCfg& inCfg){
    return layer2InColorSpace(layer, inCfg.colorspace);
}

inline void metadata2InFramerate(const private_handle_t* hnd,
                int32_t& framerate){
    if(!hnd)        return;
    //default
    framerate = 2400;
    if(hnd){
        MetaData_t *metadata = (MetaData_t *)hnd->base_metadata;
        if(metadata && (metadata->operation & PP_PARAM_VFM_DATA)){
            VfmMetaData_t* pVfmData = getVfmMData(metadata, VFM_VIDEO_FRAME);
            if(pVfmData){
                framerate = pVfmData->videoFrame.fp100s;
            }
        }
    }
    return;
}

inline void metadata2InFramerate(const private_handle_t* hnd,
                InputCfg& inCfg){
    return metadata2InFramerate(hnd, inCfg.framerate);
}

inline void layer2InFramerate(const Layer* layer, int32_t& framerate){
    return metadata2InFramerate(layer->handle, framerate);
}

inline void layer2InFramerate(const Layer* layer, InputCfg& inCfg){
    return layer2InFramerate(layer, inCfg.framerate);
}

inline void layer2InSrcPipes(const Layer* layer, InputCfg& inCfg){
    private_handle_t* hnd = layer->handle;

    if(!hnd)    return;
    MetaData_t *metadata = (MetaData_t *)hnd->base_metadata;
    if(metadata && (metadata->operation & PP_PARAM_VFM_DATA)){
        VfmMetaData_t* pVfmData = getVfmMData(metadata, VFM_VCAP_PIPE_INFO);
        if(pVfmData){
            inCfg.inSrc.eInputSrcType = INPUT_SRC_VCAP_TUNNEL;
            inCfg.inSrc.sSourcePipes.numPipes = pVfmData->vcapData.numVcapPipes;
            for(int32_t i = 0; i < pVfmData->vcapData.numVcapPipes; i++){
                //TODO: updated source pipes based on vcap interface
                inCfg.inSrc.sSourcePipes.pipe[i] =
                    (SRC_VCAP_PIPE)pVfmData->vcapData.vcapPipeId[i];
            }
        }
    }
    return;
}

inline int64_t getTimeStamp(const private_handle_t* hnd){
    int64_t timestamp = 0;
    MetaData_t* metadata = (MetaData_t *)hnd->base_metadata;

    if(!hnd)    return 0;

    if(metadata && (metadata->operation & PP_PARAM_TIMESTAMP)){
        timestamp = metadata->timestamp;
    }
    ALOGD_IF(DBG_MD, "isTimeStampSet:: %d timestamp: %lld",
        metadata->operation & PP_PARAM_TIMESTAMP, metadata->timestamp);
    return timestamp;
}

inline int32_t getStride(const private_handle_t *hnd){
    return hnd->width;
}

inline int32_t getWidth(const InputCfg& inCfg){
    return (inCfg.sCrop.right - inCfg.sCrop.left);
}

inline int32_t getHeight(const InputCfg& inCfg){
    return (inCfg.sCrop.bottom - inCfg.sCrop.top);
}

inline int32_t getPixFmt(const private_handle_t *hnd){
    if(hnd)
        return hnd->format;
    else{
        ALOGE("%s: NULL handle. Returning default pixel fmt", __func__);
        return HAL_PIXEL_FORMAT_RGB_888;
    }
}

/* Returns 0 if mediaSessId is not set in the metadata */
inline int32_t getMediaSessId(private_handle_t *hnd){
    int32_t mediaSessId = 0;
    if(hnd){
        MetaData_t *metadata = (MetaData_t *)hnd->base_metadata;
        ALOGD_IF(DBG_MD, "md->op: %x vfm->bits: %x", metadata->operation,
                            metadata->vfmDataBitMap);
        if(metadata && (metadata->operation & PP_PARAM_VFM_DATA)){
            VfmMetaData_t* pVfmData = getVfmMData(metadata, VFM_SESSION_ID);
            if(pVfmData){
                mediaSessId = pVfmData->sessionId;
                ALOGD_IF(DBG_MD, "md: sessionId: %d", pVfmData->sessionId);
            }
        }
    }
    return mediaSessId;
}

inline void layer2OutTgtRect(const Layer* layer, Rect& tgtRect){
    tgtRect = layer->tgtRect;
    return;
}

inline void layer2OutTgtRect(const Layer* layer, OutputCfg& outCfg){
    return layer2OutTgtRect(layer, outCfg.tgtRect);
}

inline int32_t align(int32_t num, int32_t alignment){
    return (num + alignment - 1) & ~(alignment - 1);
}

void layer2outDest(const Layer* layer, OutputDestination& outDest);

void layer2inputcfg(Layer* layer, InputCfg& inputCfg);

void layer2outputcfg(Layer* layer, OutputCfg& outCfg);

int32_t getProperty(const char*);

status_t unparcelInArgs(uint32_t command, vfmCmdInArg_t& inArg,
                            const Parcel* data);

status_t parcelOutArgs(uint32_t command, const vfmCmdOutArg_t& outArg,
                            Parcel* reply);

int32_t dumpFile(const char* fileName, const char* data, const int32_t width,
                 const int32_t height, const int32_t stride,
                 const int32_t pixFmt, int32_t& frmCnt);

}; //namespace VPU


#endif /* end of include guard: VFM_VPU_UTILS_H */
