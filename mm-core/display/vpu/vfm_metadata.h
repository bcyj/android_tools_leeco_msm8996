/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
 /*****************************************************************************
   This file contains the commands and command inargs and outargs structures
   for "VFM process" api
 *****************************************************************************/

#ifndef VFM_METADATA_H
#define VFM_METADATA_H
#include <sys/types.h>
#include "qdMetaData.h"

/*****************************************************************************/
/* Enumerations                                                              */
/*****************************************************************************/
/*! MetaData list */
typedef enum VFM_METADATA_TYPE
{
    //! Type id for sessionId
    VFM_SESSION_ID          = 0x01,

    //! Type id for VFM_FIELD_ORDER eField
    VFM_INTLC_FLD_ORDER     = 0x02,

    //! Type id for VfmVcapData_t
    VFM_VCAP_PIPE_INFO      = 0x04,

    //! Type id for VFM_3D_FORMAT e3Dformat
    VFM_PARAM_3D            = 0x08,

    //! Type id for VFM_COLORSPACE eColorSpace
    VFM_PARAM_COLORSPACE    = 0x10,

    //! Type id for VfmVideoFrame_t
    VFM_VIDEO_FRAME         = 0x20,

    //! Type id for VfmDummyBuf_t
    VFM_DUMMY_BUF           = 0x40,

    //! Type id for VfmBufCrop_t
    VFM_BUFFER_GEOMETRY     = 0x80,

    //! Type id for VfmCodecInfo_t
    VFM_CODEC_INFO          = 0x100,
} VFM_METADATA_TYPE;

/*! filled by decoder client - typically OMX component
    Interlace field order */
typedef enum
{
    VFM_FIELD_ORDER_NONE = 0x00,
    VFM_FIELD_ORDER_ANY,
    VFM_FIELD_ORDER_TOP,
    VFM_FIELD_ORDER_BOTTOM,
    VFM_FIELD_ORDER_SEQ_TB,
    VFM_FIELD_ORDER_SEQ_BT,
    VFM_FIELD_ORDER_ALTERNATE,
    VFM_FIELD_ORDER_INTERLACED_TB,
    VFM_FIELD_ORDER_INTERLACED_BT,
    VFM_FIELD_ORDER_INV,
} VFM_FIELD_ORDER;

typedef enum
{
    VFM_3D_HALF_SIDE_BY_SIDE        = 0x10000,
    VFM_3D_HALF_TOP_BOTTOM          = 0x20000,
    VFM_3D_FRAME_PACKED             = 0x40000,
    VFM_3D_STACKED                  = 0x80000,
    VFM_3D_FULL_TOP_BOTTOM          = 0x100000,
    VFM_3D_FULL_SIDE_BY_SIDE        = 0x200000,
} VFM_3D_FORMAT;

typedef enum
{
    VFM_CLR_SMPTE_601_FULL              = 0x00000,
    VFM_CLR_SMPTE_709_FULL,
    VFM_CLR_xvYCC_FULL,
    VFM_CLR_xvYCC601_FULL,
    VFM_CLR_xvYCC709_FULL,
    VFM_CLR_sYCC601_FULL,
    VFM_CLR_AdobeRGB_FULL,
    VFM_CLR_sRGB_FULL,
    VFM_CLR_SMPTE_601_LTD               = 0x10000,
    VFM_CLR_SMPTE_709_LTD,
    VFM_CLR_xvYCC_LTD,
    VFM_CLR_xvYCC601_LTD,
    VFM_CLR_xvYCC709_LTD,
    VFM_CLR_sYCC601_LTD,
    VFM_CLR_AdobeRGB_LTD,
    VFM_CLR_sRGB_LTD
} VFM_COLORSPACE;

typedef enum
{
    VFM_CODEC_MPEG2 = 0x00,
    VFM_CODEC_AVC
} VFM_CODEC_TYPE;

typedef enum
{
    VFM_PROFILE_4_0 = 0x00,
} VFM_CODEC_PROFILE;

/*****************************************************************************/
/* Structures                                                                */
/*****************************************************************************/
/*! filled by player for vcap source cases */
/*  The data in this buffer is not read if this is set */
/*  Payload for VFM_VCAP_PIPE_INFO */
struct VfmVcapData_t{
    int32_t numVcapPipes;         //<! numVcapPipes used for tunneling.
    int32_t vcapPipeId[2];        //<! VCap pipe id used for tunneling.
};

/*! Frame rate details filled by mediaplayer */
struct VfmVideoFrame_t{
    /*! fp100s = 100*fps of content frame rate.
        Frames per hundred seconds to accomodate fractional frame rates
        Set this to -1 if fps is unknown. Default value = 3000 (30 fps) */
    int32_t fp100s;

    /*! Unaltered timestamp from the container.
        Might be used for fps calculation if required (and  fp100s = -1)
        Not respected if VCAP_PIPE_INFO is set or isDataSFbypass=1 */
    int32_t containerTS;

};

/* Payload for VFM_DUMMY_BUF */
struct VfmDummyBuf_t{
    /*! If set, data buffers are queued directly to VFM bypassing SF
        This flag is irrelavent if VCAP_PIPE_INFO is set.
        Data from this buffer is ignored and not "drawn" but only the metadata
        is used  */
    int32_t isDataTunnelVFM;

    /*! Indicates that this dummy buffer is to setup VCAP tunnelling */
    int32_t isDataTunnelVCAP;
};

/* Payload for VFM_BUFFER_GEOMETRY */
struct VfmBufGeometry_t{
    int32_t width;          //<!Buffer stride width
    int32_t height;         //<!Buffer stride height
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
    int32_t pixelFormat;    //defined in gralloc_priv.h and graphics.h
};

/* Payload for VFM_CODEC_INFO */
struct VfmCodecInfo_t{
    VFM_CODEC_TYPE      codec;
    VFM_CODEC_PROFILE   profile;
};

/*! Generic Metadata structure for all VFM metadata */
struct VfmMetaData_t {
    VFM_METADATA_TYPE type;

    union {
        /* dummy place holder to create the size of this structure */
        char data[MAX_VFM_DATA_SIZE];

        /*! Video session id, to identify a session of buffers from a media
            player */
        int32_t sessionId;

        /*! Interlace field order.
            Respected if PP_PARAM_INTERLACED/interlaced=1 is set */
        VFM_FIELD_ORDER eField;

        /*! Vcap related data for Vcap tunneling cases */
        VfmVcapData_t  vcapData;

        /*! Content 3D format */
        VFM_3D_FORMAT e3Dformat;

        /*! Content color space */
        /*! This is mandatory argument. If this is not present in the source,
             it has to be set based on source characteristics */
        VFM_COLORSPACE eColorSpace;

        /*! Frame rate, container timestamp and other related info */
        VfmVideoFrame_t videoFrame;

        /*! Indicate that data in this buffer is garbage */
        VfmDummyBuf_t dummyBuf;

        /*! If the actual (tunnelled) buffer dimensions are different */
        /*   than the native window (or dummy buffer), set this */
        VfmBufGeometry_t  bufGmtry;

        /*! Codec and profile info required for custom postproc algorithm */
        VfmCodecInfo_t codecInfo;
    };

};

#endif /* end of include guard: VFM_METADATA_H */
