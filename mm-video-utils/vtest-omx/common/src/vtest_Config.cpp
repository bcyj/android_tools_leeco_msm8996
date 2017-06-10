/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential

Copyright (c) 2010 The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of The Linux Foundation nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--------------------------------------------------------------------------*/

#include "vtest_ComDef.h"
#include "vtest_Debug.h"
#include "vtest_Config.h"
#include "vtest_Parser.h"
#include "vtest_File.h"
#include <stdlib.h>
#include <sys/limits.h>
#include "OMX_IndexExt.h"
#include "graphics.h"

#undef LOG_TAG
#define LOG_TAG "VTEST_CONFIG"

#define INVALID_VALUE -1

namespace vtest {

// Codecs
static ConfigEnum g_pCodecEnums[] =
{
    { (OMX_STRING)"MP4",   OMX_VIDEO_CodingMPEG4 },
    { (OMX_STRING)"H263",  OMX_VIDEO_CodingH263 },
    { (OMX_STRING)"H264",  OMX_VIDEO_CodingAVC },
    { (OMX_STRING)"VP8",   OMX_VIDEO_CodingVP8 },
    { (OMX_STRING)"VC1",   OMX_VIDEO_CodingWMV },
    { (OMX_STRING)"DIVX",  QOMX_VIDEO_CodingDivx },
    { (OMX_STRING)"MPEG2", OMX_VIDEO_CodingMPEG2 },
    { (OMX_STRING)"HEVC",  QOMX_VIDEO_CodingHevc },
    { 0, 0 }
};

// Rate control flavors
static ConfigEnum g_pVencRCEnums[] =
{
    { (OMX_STRING)"RC_OFF",     (int)OMX_Video_ControlRateDisable },
    { (OMX_STRING)"RC_VBR_VFR", (int)OMX_Video_ControlRateVariableSkipFrames },       // Camcorder
    { (OMX_STRING)"RC_VBR_CFR", (int)OMX_Video_ControlRateVariable },                 // Camcorder
    { (OMX_STRING)"RC_CBR_VFR", (int)OMX_Video_ControlRateConstantSkipFrames },       // QVP
    { (OMX_STRING)"RC_CBR_CFR", (int)OMX_Video_ControlRateConstant },                 // WFD
    { 0, 0 }
};

// Resync marker types
static ConfigEnum g_pResyncMarkerType[] =
{
    { (OMX_STRING)"NONE",        (int)RESYNC_MARKER_NONE },
    { (OMX_STRING)"BITS",        (int)RESYNC_MARKER_BITS },
    { (OMX_STRING)"MB",          (int)RESYNC_MARKER_MB },
    { (OMX_STRING)"GOB",         (int)RESYNC_MARKER_GOB },
    { 0, 0 }
};

// Codec Profile Type
static ConfigEnum g_pCodecProfileType[] =
{
    { (OMX_STRING)"MPEG4_SP",			(int)MPEG4ProfileSimple },
    { (OMX_STRING)"MPEG4_ASP",			(int)MPEG4ProfileAdvancedSimple },
    { (OMX_STRING)"H263_BASELINE",		(int)H263ProfileBaseline },
    { (OMX_STRING)"H264_BASELINE",		(int)AVCProfileBaseline },
    { (OMX_STRING)"H264_HIGH",			(int)AVCProfileHigh },
    { (OMX_STRING)"H264_MAIN",			(int)AVCProfileMain },
    { (OMX_STRING)"VP8_MAIN",			(int)VP8ProfileMain },
    { (OMX_STRING)"HEVC_MAIN",			(int)HEVCProfileMain },
    { (OMX_STRING)"HEVC_MAIN10",		(int)HEVCProfileMain10 },
    { 0, 0 }
};

// Codec Level Type
static ConfigEnum g_pCodecLevelType[] =
{
    { (OMX_STRING)"DEFAULT",            (int)DefaultLevel },
    { (OMX_STRING)"VP8_VERSION_0",      (int)VP8Version0 },
    { (OMX_STRING)"VP8_VERSION_1",      (int)VP8Version1 },
    { 0, 0 }
};

// Playback Modes
static ConfigEnum g_pPlaybackMode[] =
{
    { (OMX_STRING)"DYNAMIC_PORT_RECONFIG",     DynamicPortReconfig },
    { (OMX_STRING)"ADAPTIVE_SMOOTH_STREAMING", AdaptiveSmoothStreaming },
    { (OMX_STRING)"QC_SMOOTH_STREAMING",       QCSmoothStreaming },
    { (OMX_STRING)"DYNAMIC_BUFFER_MODE",       DynamicBufferMode },
    { 0, 0 }
};

// Playback Modes
static ConfigEnum g_pSinkType[] =
{
    { (OMX_STRING)"NATIVE_WINDOW", NativeWindow_Sink },
    { (OMX_STRING)"MDP_OVERLAY",   MDPOverlay_Sink },
    { (OMX_STRING)"FILE",          File_Sink },
    { 0, 0 }
};

// YuvColorSpace Types
static ConfigEnum g_pYuvColorSpaceType[] =
{
    { (OMX_STRING)"601",   ITUR601 },
    { (OMX_STRING)"601FR", ITUR601FR },
    { (OMX_STRING)"709",   ITUR709 },
    { 0, 0 }
};

static ConfigEnum g_pPostProcType[] =
{
    { (OMX_STRING)"C2DCC", C2dColorConversion },
    { (OMX_STRING)"MMCC",  MmColorConversion },
    { (OMX_STRING)"GPUPP", GpuPostProcessing },
    { 0, 0 }
};

static ConfigEnum g_pColorFormat[] =
{
    { (OMX_STRING)"VENUS",          QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m },
    { (OMX_STRING)"PLANAR_CBCR",    OMX_COLOR_FormatYUV420Planar },
    { (OMX_STRING)"PLANAR_CRCB",    HAL_PIXEL_FORMAT_YV12 },
    { (OMX_STRING)"RGBA8888",       OMX_COLOR_Format32bitARGB8888 },
    { 0, 0 }
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_S32 ParseEnum(ConfigEnum *pConfigEnum, OMX_STRING pEnumName) {

    OMX_S32 idx = 0;
    while (pConfigEnum[idx].pEnumName) {
        if (Parser::StringICompare(pEnumName,
                                   pConfigEnum[idx].pEnumName) == 0) {
            return pConfigEnum[idx].eEnumVal;
        }
        idx++;
    }
    VTEST_MSG_ERROR("error could not find enum");
    return -1;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Config::Config() {

    static const OMX_STRING pInFileName = (OMX_STRING)"";

    // set some default values
    memset(&m_sCodecConfig, 0, sizeof(m_sCodecConfig));
    m_sCodecConfig.eCodec = OMX_VIDEO_CodingAVC;
    m_sCodecConfig.eFileType = FILE_TYPE_ARBITRARY_BYTES;
    m_sCodecConfig.eCodecProfile = MPEG4ProfileSimple;
    m_sCodecConfig.eCodecLevel = DefaultLevel;
    m_sCodecConfig.eControlRate = OMX_Video_ControlRateDisable;
    m_sCodecConfig.nFrameWidth = 640;
    m_sCodecConfig.nFrameHeight = 480;
    m_sCodecConfig.nOutputFrameWidth = m_sCodecConfig.nFrameWidth;
    m_sCodecConfig.nOutputFrameHeight = m_sCodecConfig.nFrameHeight;
    m_sCodecConfig.nDVSXOffset = 0;
    m_sCodecConfig.nDVSYOffset = 0;
    m_sCodecConfig.nBitrate = INVALID_VALUE;
    m_sCodecConfig.nFramerate = 30;
    m_sCodecConfig.nTimeIncRes = 30;
    m_sCodecConfig.nRotation = 0;
    m_sCodecConfig.nInBufferCount = 3;
    m_sCodecConfig.nOutBufferCount = 3;
    m_sCodecConfig.nHECInterval = 0;
    m_sCodecConfig.nResyncMarkerSpacing = 0;
    m_sCodecConfig.eResyncMarkerType = RESYNC_MARKER_NONE;
    m_sCodecConfig.nIntraRefreshMBCount = 0;
    m_sCodecConfig.nFrames = INT_MAX;
    m_sCodecConfig.bEnableShortHeader = OMX_FALSE;
    m_sCodecConfig.nIntraPeriod = m_sCodecConfig.nFramerate * 2;
    m_sCodecConfig.nMinQp = INVALID_VALUE;
    m_sCodecConfig.nMaxQp = INVALID_VALUE;
    m_sCodecConfig.nIDRPeriod = INVALID_VALUE;
    m_sCodecConfig.bProfileMode = OMX_FALSE;
    m_sCodecConfig.bCABAC = OMX_FALSE;
    m_sCodecConfig.nDeblocker = INVALID_VALUE;
    m_sCodecConfig.id = 0;
    m_sCodecConfig.cancel_flag = 1;
    m_sCodecConfig.type = 0;
    m_sCodecConfig.quincunx_sampling_flag = 0;
    m_sCodecConfig.content_interpretation_type = 0;
    m_sCodecConfig.spatial_flipping_flag = 0;
    m_sCodecConfig.frame0_flipped_flag = 0;
    m_sCodecConfig.field_views_flag = 0;
    m_sCodecConfig.current_frame_is_frame0_flag = 0;
    m_sCodecConfig.frame0_self_contained_flag = 0;
    m_sCodecConfig.frame1_self_contained_flag = 0;
    m_sCodecConfig.frame0_grid_position_x = 0;
    m_sCodecConfig.frame0_grid_position_y = 0;
    m_sCodecConfig.frame1_grid_position_x = 0;
    m_sCodecConfig.frame1_grid_position_y = 0;
    m_sCodecConfig.reserved_byte = 0;
    m_sCodecConfig.repetition_period = 0;
    m_sCodecConfig.extension_flag = 0;
    m_sCodecConfig.nLTRMode = 0;
    m_sCodecConfig.nLTRCount = 0;
    m_sCodecConfig.nLTRPeriod = 0;
    m_sCodecConfig.bSecureSession = OMX_FALSE;
    m_sCodecConfig.bMdpFrameSource = OMX_FALSE;
    m_sCodecConfig.bInsertInbandVideoHeaders = OMX_FALSE;
    m_sCodecConfig.bInsertAUDelimiters = OMX_FALSE;
    m_sCodecConfig.ePlaybackMode = DynamicPortReconfig;
    m_sCodecConfig.nAdaptiveWidth = m_sCodecConfig.nFrameWidth;
    m_sCodecConfig.nAdaptiveHeight = m_sCodecConfig.nFrameHeight;
    m_sCodecConfig.bRotateDisplay = OMX_FALSE;
    m_sCodecConfig.eDecoderPictureOrder = QOMX_VIDEO_DISPLAY_ORDER;
    m_sCodecConfig.eSinkType = NativeWindow_Sink;
    m_sCodecConfig.bMetaMode = OMX_TRUE;
    m_sCodecConfig.eMetaBufferType = CameraSource;
    m_sCodecConfig.eYuvColorSpace = ITUR601;
    m_sCodecConfig.nIDRPeriod = m_sCodecConfig.nIntraPeriod;
    m_sCodecConfig.bDecoderDownScalar = OMX_FALSE;
    m_sCodecConfig.ePostProcType = DefaultMemcopy;
    m_sCodecConfig.nInputColorFormat = QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m;
    m_sCodecConfig.nOutputColorFormat = QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m;

    // default dynamic config
    m_sDynamicConfig.nIFrameRequestPeriod = 0;
    m_sDynamicConfig.nUpdatedBitrate = m_sCodecConfig.nBitrate;
    m_sDynamicConfig.nUpdatedFramerate = m_sCodecConfig.nFramerate;
    m_sDynamicConfig.nUpdatedFrames = m_sCodecConfig.nFrames;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Config::~Config() {}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Config::Parse(OMX_STRING pFileName,
        CodecConfigType *pCodecConfig, DynamicConfigType *pDynamicConfig) {

    static const OMX_S32 maxLineLen = 256;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_S32 nLine = 0;
    ParserStrVector v;
    char pBuf[maxLineLen];
    char *pTrimmed;
    File f;

    result = f.Open(pFileName, OMX_TRUE);

    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("error opening file");
        return OMX_ErrorBadParameter;
    }

    while (Parser::ReadLine(&f, maxLineLen, pBuf) != -1) {

        OMX_S32 nTok = 0;
        (void)Parser::RemoveComments(pBuf);
        pTrimmed = Parser::Trim(pBuf);
        if (strlen(pTrimmed) != 0) nTok = Parser::TokenizeString(&v, pTrimmed, (OMX_STRING)"\t =");
        VTEST_MSG_LOW("ntok = %d", (int)nTok);

        switch (v.size()) {

        case 0: // blank line
            break;
        case 1: // use default value
            break;
        case 2: // user has a preferred config

            ///////////////////////////////////////////
            ///////////////////////////////////////////
            // Codec config
            ///////////////////////////////////////////
            ///////////////////////////////////////////
            if (Parser::StringICompare((OMX_STRING)"FrameWidth", v[0]) == 0) {
                m_sCodecConfig.nFrameWidth = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FrameWidth = %d", (int)m_sCodecConfig.nFrameWidth);
            } else if (Parser::StringICompare((OMX_STRING)"FrameHeight", v[0]) == 0) {
                m_sCodecConfig.nFrameHeight = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FrameHeight = %d", (int)m_sCodecConfig.nFrameHeight);
            } else if (Parser::StringICompare((OMX_STRING)"OutputFrameWidth", v[0]) == 0) {
                m_sCodecConfig.nOutputFrameWidth = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("OutputFrameWidth = %d", (int)m_sCodecConfig.nOutputFrameWidth);
            } else if (Parser::StringICompare((OMX_STRING)"OutputFrameHeight", v[0]) == 0) {
                m_sCodecConfig.nOutputFrameHeight = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("OutputFrameHeight = %d", (int)m_sCodecConfig.nOutputFrameHeight);
            } else if (Parser::StringICompare((OMX_STRING)"DVSXOffset", v[0]) == 0) {
                m_sCodecConfig.nDVSXOffset = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("DVSXOffset = %d", (int)m_sCodecConfig.nDVSXOffset);
            } else if (Parser::StringICompare((OMX_STRING)"DVSYOffset", v[0]) == 0) {
                m_sCodecConfig.nDVSYOffset = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("DVSYOffset = %d", (int)m_sCodecConfig.nDVSYOffset);
            } else if (Parser::StringICompare((OMX_STRING)"Rotation", v[0]) == 0) {
                m_sCodecConfig.nRotation = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("Rotation = %d", (int)m_sCodecConfig.nRotation);
            } else if (Parser::StringICompare((OMX_STRING)"FPS", v[0]) == 0) {
                m_sCodecConfig.nFramerate = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FPS = %d", (int)m_sCodecConfig.nFramerate);
            } else if (Parser::StringICompare((OMX_STRING)"Bitrate", v[0]) == 0) {
                m_sCodecConfig.nBitrate = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("Bitrate = %d", (int) m_sCodecConfig.nBitrate);
            } else if (Parser::StringICompare((OMX_STRING)"RC", v[0]) == 0) {
                m_sCodecConfig.eControlRate =
                    (OMX_VIDEO_CONTROLRATETYPE)ParseEnum(g_pVencRCEnums, v[1]);
                VTEST_MSG_LOW("RC = %d", m_sCodecConfig.eControlRate);
            } else if (Parser::StringICompare((OMX_STRING)"Codec", v[0]) == 0) {
                m_sCodecConfig.eCodec = (OMX_VIDEO_CODINGTYPE)ParseEnum(g_pCodecEnums, v[1]);
                VTEST_MSG_LOW("Codec = %d", m_sCodecConfig.eCodec);
            } else if (Parser::StringICompare((OMX_STRING)"FileType", v[0]) == 0) {
                m_sCodecConfig.eFileType = (FileType)atoi(v[1]);
                VTEST_MSG_LOW("FileType = %d", m_sCodecConfig.eFileType);
            } else if (Parser::StringICompare((OMX_STRING)"Profile", v[0]) == 0) {
                m_sCodecConfig.eCodecProfile = (CodecProfileType)ParseEnum(g_pCodecProfileType, v[1]);
                VTEST_MSG_LOW("Profile = %d", m_sCodecConfig.eCodecProfile);
            } else if (Parser::StringICompare((OMX_STRING)"Level", v[0]) == 0) {
                m_sCodecConfig.eCodecLevel = (CodecLevelType)ParseEnum(g_pCodecLevelType, v[1]);
                VTEST_MSG_LOW("Level = %d", m_sCodecConfig.eCodecLevel);
            } else if (Parser::StringICompare((OMX_STRING)"InBufferCount", v[0]) == 0) {
                m_sCodecConfig.nInBufferCount = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("InBufferCount = %d", (int)m_sCodecConfig.nInBufferCount);
            } else if (Parser::StringICompare((OMX_STRING)"OutBufferCount", v[0]) == 0) {
                m_sCodecConfig.nOutBufferCount = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("OutBufferCount = %d", (int)m_sCodecConfig.nOutBufferCount);
            } else if (Parser::StringICompare((OMX_STRING)"HECInterval", v[0]) == 0) {
                m_sCodecConfig.nHECInterval = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("HECInterval = %d\n", (int)m_sCodecConfig.nHECInterval);
            } else if (Parser::StringICompare((OMX_STRING)"ResyncMarkerSpacing", v[0]) == 0) {
                m_sCodecConfig.nResyncMarkerSpacing = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("ResyncMarkerSpacing = %d\n", (int)m_sCodecConfig.nResyncMarkerSpacing);
            } else if (Parser::StringICompare((OMX_STRING)"ResyncMarkerType", v[0]) == 0) {
                m_sCodecConfig.eResyncMarkerType = (ResyncMarkerType)ParseEnum(g_pResyncMarkerType, v[1]);
                VTEST_MSG_LOW("ResyncMarkerType = %d\n", (int)m_sCodecConfig.eResyncMarkerType);
            } else if (Parser::StringICompare((OMX_STRING)"IntraRefreshMBCount", v[0]) == 0) {
                m_sCodecConfig.nIntraRefreshMBCount = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("IntraRefreshMBCount = %d\n", (int)m_sCodecConfig.nIntraRefreshMBCount);
            } else if (Parser::StringICompare((OMX_STRING)"TimeIncRes", v[0]) == 0) {
                m_sCodecConfig.nTimeIncRes = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("TimeIncRes = %d\n", (int)m_sCodecConfig.nTimeIncRes);
            } else if (Parser::StringICompare((OMX_STRING)"EnableShortHeader", v[0]) == 0) {
                m_sCodecConfig.bEnableShortHeader = (OMX_BOOL)atoi(v[1]);
                VTEST_MSG_LOW("EnableShortHeader = %d\n", (int)m_sCodecConfig.bEnableShortHeader);
            } else if (Parser::StringICompare((OMX_STRING)"IntraPeriod", v[0]) == 0) {
                m_sCodecConfig.nIntraPeriod = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("IntraPeriod = %d\n", (int)m_sCodecConfig.nIntraPeriod);
            } else if (Parser::StringICompare((OMX_STRING)"InFile", v[0]) == 0) {
                memcpy(m_sCodecConfig.cInFileName, v[1], (strlen(v[1]) + 1));
                VTEST_MSG_LOW("InFile = %s\n", m_sCodecConfig.cInFileName);
            } else if (Parser::StringICompare((OMX_STRING)"OutFile", v[0]) == 0) {
                memcpy(m_sCodecConfig.cOutFileName, v[1], (strlen(v[1]) + 1));
                VTEST_MSG_LOW("OutFile = %s\n", m_sCodecConfig.cOutFileName);
            } else if (Parser::StringICompare((OMX_STRING)"NumFrames", v[0]) == 0) {
                m_sCodecConfig.nFrames = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("NumFrames = %d", (int)m_sCodecConfig.nFrames);
            } else if (Parser::StringICompare((OMX_STRING)"MinQp", v[0]) == 0) {
                m_sCodecConfig.nMinQp = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("MinQp = %d", (int)m_sCodecConfig.nMinQp);
            } else if (Parser::StringICompare((OMX_STRING)"MaxQp", v[0]) == 0) {
                m_sCodecConfig.nMaxQp = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("MaxQp = %d", (int)m_sCodecConfig.nMaxQp);
            } else if (Parser::StringICompare((OMX_STRING)"CABAC", v[0]) == 0) {
                m_sCodecConfig.bCABAC = (OMX_BOOL)atoi(v[1]);
                VTEST_MSG_LOW("CABAC = %d", m_sCodecConfig.bCABAC);
            } else if (Parser::StringICompare((OMX_STRING)"Deblock", v[0]) == 0) {
                m_sCodecConfig.nDeblocker = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("Deblock = %d", (int)m_sCodecConfig.nDeblocker);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackId", v[0]) == 0) {
                m_sCodecConfig.id = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackId = %d", (int)m_sCodecConfig.id);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackCancelFlag", v[0]) == 0) {
                m_sCodecConfig.cancel_flag = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackCancelFlag = %d", (int)m_sCodecConfig.cancel_flag);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackType", v[0]) == 0) {
                m_sCodecConfig.type = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackType = %d", (int)m_sCodecConfig.type);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackQuincunxSamplingFlag", v[0]) == 0) {
                m_sCodecConfig.quincunx_sampling_flag = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackQuincunxSamplingFlag = %d", (int)m_sCodecConfig.quincunx_sampling_flag);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackContentInterpretationType", v[0]) == 0) {
                m_sCodecConfig.content_interpretation_type = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackContentInterpretationType = %d", (int)m_sCodecConfig.content_interpretation_type);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackSpatialFlippingFlag", v[0]) == 0) {
                m_sCodecConfig.spatial_flipping_flag = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackSpatialFlippingFlag = %d", (int)m_sCodecConfig.spatial_flipping_flag);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackFrame0FlippedFlag", v[0]) == 0) {
                m_sCodecConfig.frame0_flipped_flag = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackFrame0FlippedFlag = %d", (int)m_sCodecConfig.frame0_flipped_flag);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackFieldViewsFlag", v[0]) == 0) {
                m_sCodecConfig.field_views_flag = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackFieldViewsFlag = %d", (int)m_sCodecConfig.field_views_flag);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackCurrentFrameIsFrame0Flag", v[0]) == 0) {
                m_sCodecConfig.current_frame_is_frame0_flag = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackCurrentFrameIsFrame0Flag = %d", (int)m_sCodecConfig.current_frame_is_frame0_flag);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackFrame0SelfContainedFlag", v[0]) == 0) {
                m_sCodecConfig.frame0_self_contained_flag = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackFrame0SelfContainedFlag = %d", (int)m_sCodecConfig.frame0_self_contained_flag);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackFrame1SelfContainedFlag", v[0]) == 0) {
                m_sCodecConfig.frame1_self_contained_flag = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackFrame1SelfContainedFlag = %d", (int)m_sCodecConfig.frame1_self_contained_flag);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackFrame0GridPositionX", v[0]) == 0) {
                m_sCodecConfig.frame0_grid_position_x = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackFrame0GridPositionX = %d", (int)m_sCodecConfig.frame0_grid_position_x);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackFrame0GridPositionY", v[0]) == 0) {
                m_sCodecConfig.frame0_grid_position_y = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackFrame0GridPositionY = %d", (int)m_sCodecConfig.frame0_grid_position_y);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackFrame1GridPositionX", v[0]) == 0) {
                m_sCodecConfig.frame1_grid_position_x = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackFrame1GridPositionX = %d", (int)m_sCodecConfig.frame1_grid_position_x);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackFrame1GridPositionY", v[0]) == 0) {
                m_sCodecConfig.frame1_grid_position_y = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackFrame1GridPositionY = %d", (int)m_sCodecConfig.frame1_grid_position_y);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackReservedByte", v[0]) == 0) {
                m_sCodecConfig.reserved_byte = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackReservedByte = %d", (int)m_sCodecConfig.reserved_byte);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackRepetitionPeriod", v[0]) == 0) {
                m_sCodecConfig.repetition_period = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackRepetitionPeriod = %d", (int)m_sCodecConfig.repetition_period);
            } else if (Parser::StringICompare((OMX_STRING)"FramePackExtensionFlag", v[0]) == 0) {
                m_sCodecConfig.extension_flag = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("FramePackExtensionFlag = %d", (int)m_sCodecConfig.extension_flag);
            } else if (Parser::StringICompare((OMX_STRING)"LTRMode", v[0]) == 0) {
                m_sCodecConfig.nLTRMode = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("LTRMode = %d", (int)m_sCodecConfig.nLTRMode);
            } else if (Parser::StringICompare((OMX_STRING)"LTRCount", v[0]) == 0) {
                m_sCodecConfig.nLTRCount = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("LTRCount = %d", (int)m_sCodecConfig.nLTRCount);
            } else if (Parser::StringICompare((OMX_STRING)"LTRPeriod", v[0]) == 0) {
                m_sCodecConfig.nLTRPeriod = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("LTRPeriod = %d", (int)m_sCodecConfig.nLTRPeriod);
            } else if (Parser::StringICompare((OMX_STRING)"Extradata", v[0]) == 0) {
                m_sCodecConfig.bExtradata = (OMX_BOOL)atoi(v[1]);
                VTEST_MSG_LOW("Extradata = %d", m_sCodecConfig.bExtradata);
            } else if (Parser::StringICompare((OMX_STRING)"IDRPeriod", v[0]) == 0) {
                m_sCodecConfig.nIDRPeriod = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("Extradata = %d", (int)m_sCodecConfig.nIDRPeriod);
            } else if (Parser::StringICompare((OMX_STRING) "HierPNumLayers", v[0]) == 0) {
               m_sCodecConfig.nHierPNumLayers = (OMX_U32) atoi(v[1]);
               VTEST_MSG_LOW("HierPNumLayers = %d", m_sCodecConfig.nHierPNumLayers);
            }  else if (Parser::StringICompare((OMX_STRING) "VencPerfMode", v[0]) == 0) {
               m_sCodecConfig.nPerfMode = (OMX_U32) atoi(v[1]);
               VTEST_MSG_LOW("PerfMode = %d", m_sCodecConfig.nPerfMode);
            }
            ///////////////////////////////////////////
            ///////////////////////////////////////////
            // Dynamic config
            ///////////////////////////////////////////
            ///////////////////////////////////////////
            else if (Parser::StringICompare((OMX_STRING)"DynamicFile", v[0]) == 0) {
                memcpy(m_sCodecConfig.cDynamicFile, v[1], (strlen(v[1]) + 1));
                VTEST_MSG_LOW("DynamicFile");
                m_sCodecConfig.dynamic_config_array_size = Parser::CountLine(m_sCodecConfig.cDynamicFile);
                VTEST_MSG_MEDIUM("Allocating %u units of Dynamic Config",
                                     (unsigned int)m_sCodecConfig.dynamic_config_array_size);
                m_pDynamicProperties = new DynamicConfig[m_sCodecConfig.dynamic_config_array_size];
                for (OMX_U32 i = 0; i < m_sCodecConfig.dynamic_config_array_size; i++) {
                    memset(&m_pDynamicProperties[i], 0, sizeof(struct DynamicConfig));
                }

                OMX_S32 nLine = 0;

                ParserStrVector vec;
                char pBuffer[maxLineLen];
                char *pTrimmed;

                File dynamicFile;
                OMX_ERRORTYPE result = OMX_ErrorNone;
                result = dynamicFile.Open(m_sCodecConfig.cDynamicFile, OMX_TRUE);

                if (result != OMX_ErrorNone) {
                    VTEST_MSG_ERROR("Cannot open dynamic config file: %s", m_sCodecConfig.cDynamicFile);
                    return OMX_ErrorBadParameter;
                }

                while (Parser::ReadLine(&dynamicFile, maxLineLen, pBuffer) != -1) {
                    OMX_S32 nTok = 0;
                    (void)Parser::RemoveComments(pBuffer);
                    pTrimmed = Parser::Trim(pBuffer);
                    if (strlen(pTrimmed) != 0) nTok = Parser::TokenizeString(&vec, pTrimmed, (OMX_STRING)"\t =");

                    VTEST_MSG_LOW("ntok = %d, size: %d\n", (int)nTok, (int) vec.size());
                    switch (vec.size()) {
                    case 0: // blank line
                        break;
                    case 1: // use default value
                        break;
                    case 2:
                        if (Parser::StringICompare((OMX_STRING)"ivoprefresh", vec[1]) == 0) {
                            m_pDynamicProperties[nLine].frame_num = (OMX_S32)atoi(vec[0]);
                            m_pDynamicProperties[nLine].config_param = OMX_IndexConfigVideoIntraVOPRefresh;
                            m_pDynamicProperties[nLine].config_data.intravoprefresh.nPortIndex = (OMX_S32)PORT_INDEX_OUT;
                            m_pDynamicProperties[nLine].config_data.intravoprefresh.IntraRefreshVOP = OMX_TRUE;
                        } else if (Parser::StringICompare((OMX_STRING)"useltr", vec[1]) == 0) {
                            m_pDynamicProperties[nLine].frame_num = (OMX_S32) atoi(vec[0]);
                            if (m_sCodecConfig.eCodec == OMX_VIDEO_CodingVP8) {
                                m_pDynamicProperties[nLine].config_param = (OMX_INDEXTYPE)OMX_IndexConfigVideoVp8ReferenceFrame;
                                m_pDynamicProperties[nLine].config_data.vp8refframe.nPortIndex = (OMX_U32)PORT_INDEX_IN;
                                m_pDynamicProperties[nLine].config_data.vp8refframe.bUseGoldenFrame = OMX_TRUE;
                                m_pDynamicProperties[nLine].config_data.vp8refframe.bGoldenFrameRefresh = OMX_FALSE;
                                m_pDynamicProperties[nLine].config_data.vp8refframe.bAlternateFrameRefresh = OMX_FALSE;
                                m_pDynamicProperties[nLine].config_data.vp8refframe.bPreviousFrameRefresh = OMX_FALSE;
                                m_pDynamicProperties[nLine].config_data.vp8refframe.bUsePreviousFrame = OMX_FALSE;
                            } else {
                                m_pDynamicProperties[nLine].config_param = (OMX_INDEXTYPE)OMX_QcomIndexConfigVideoLTRUse;
                                m_pDynamicProperties[nLine].config_data.useltr.nPortIndex = (OMX_U32)PORT_INDEX_IN;
                                m_pDynamicProperties[nLine].config_data.useltr.nFrames = 0;
                                m_pDynamicProperties[nLine].config_data.useltr.nID = 0;
                            }
                        } else if (Parser::StringICompare((OMX_STRING)"markltr", vec[1]) == 0) {
                            m_pDynamicProperties[nLine].frame_num = (OMX_S32) atoi(vec[0]);
                            if (m_sCodecConfig.eCodec == OMX_VIDEO_CodingVP8) {
                                m_pDynamicProperties[nLine].config_param = (OMX_INDEXTYPE)OMX_IndexConfigVideoVp8ReferenceFrame;
                                m_pDynamicProperties[nLine].config_data.vp8refframe.nPortIndex = (OMX_U32)PORT_INDEX_IN;
                                m_pDynamicProperties[nLine].config_data.vp8refframe.bUseGoldenFrame = OMX_FALSE;
                                m_pDynamicProperties[nLine].config_data.vp8refframe.bGoldenFrameRefresh = OMX_TRUE;
                                m_pDynamicProperties[nLine].config_data.vp8refframe.bAlternateFrameRefresh = OMX_FALSE;
                                m_pDynamicProperties[nLine].config_data.vp8refframe.bPreviousFrameRefresh = OMX_FALSE;
                                m_pDynamicProperties[nLine].config_data.vp8refframe.bUsePreviousFrame = OMX_FALSE;
                            } else {
                                m_pDynamicProperties[nLine].config_param = (OMX_INDEXTYPE) OMX_QcomIndexConfigVideoLTRMark;
                                m_pDynamicProperties[nLine].config_data.markltr.nPortIndex = (OMX_U32) PORT_INDEX_IN;
                                m_pDynamicProperties[nLine].config_data.markltr.nID = 0;
                            }
                        }
                        break;
                    case 3: // dynamic config
                        if (Parser::StringICompare((OMX_STRING)"bitrate", vec[1]) == 0) {
                            m_pDynamicProperties[nLine].frame_num = (OMX_S32)atoi(vec[0]);
                            m_pDynamicProperties[nLine].config_param = OMX_IndexConfigVideoBitrate;
                            m_pDynamicProperties[nLine].config_data.bitrate.nPortIndex = (OMX_S32)PORT_INDEX_OUT;
                            m_pDynamicProperties[nLine].config_data.bitrate.nEncodeBitrate = (OMX_S32)atoi(vec[2]);
                        } else if (Parser::StringICompare((OMX_STRING)"framerate", vec[1]) == 0) {
                            m_pDynamicProperties[nLine].frame_num = (OMX_S32)atoi(vec[0]);
                            m_pDynamicProperties[nLine].config_param = OMX_IndexConfigVideoFramerate;
                            m_pDynamicProperties[nLine].config_data.framerate.nPortIndex = (OMX_S32)PORT_INDEX_OUT;
                            m_pDynamicProperties[nLine].config_data.f_framerate = (OMX_S32)atoi(vec[2]);
                        } else if (Parser::StringICompare((OMX_STRING)"iperiod", vec[1]) == 0) {
                            m_pDynamicProperties[nLine].frame_num = (OMX_S32)atoi(vec[0]);
                            m_pDynamicProperties[nLine].config_param = (OMX_INDEXTYPE)QOMX_IndexConfigVideoIntraperiod;
                            m_pDynamicProperties[nLine].config_data.intraperiod.nPortIndex = (OMX_S32)PORT_INDEX_OUT;
                            m_pDynamicProperties[nLine].config_data.intraperiod.nPFrames = (OMX_U32)atoi(vec[2]) - 1;
                            m_pDynamicProperties[nLine].config_data.intraperiod.nBFrames = 0;
                            m_pDynamicProperties[nLine].config_data.intraperiod.nIDRPeriod = 1;
                        } else if (Parser::StringICompare((OMX_STRING)"useltr", vec[1]) == 0) {
                            m_pDynamicProperties[nLine].frame_num = (OMX_S32) atoi(vec[0]);
                            m_pDynamicProperties[nLine].config_param = (OMX_INDEXTYPE)OMX_QcomIndexConfigVideoLTRUse;
                            m_pDynamicProperties[nLine].config_data.useltr.nPortIndex = (OMX_U32)PORT_INDEX_IN;
                            m_pDynamicProperties[nLine].config_data.useltr.nFrames = 0;
                            m_pDynamicProperties[nLine].config_data.useltr.nID = (OMX_U32)atoi(vec[2]);
                        } else if (Parser::StringICompare((OMX_STRING)"markltr", vec[1]) == 0) {
                            m_pDynamicProperties[nLine].frame_num = (OMX_S32) atoi(vec[0]);
                            m_pDynamicProperties[nLine].config_param = (OMX_INDEXTYPE)OMX_QcomIndexConfigVideoLTRMark;
                            m_pDynamicProperties[nLine].config_data.markltr.nPortIndex = (OMX_U32)PORT_INDEX_IN;
                            m_pDynamicProperties[nLine].config_data.markltr.nID = (OMX_U32)atoi(vec[2]);
                        } else if (Parser::StringICompare((OMX_STRING) "VencPerfMode", vec[1]) == 0) {
                            m_pDynamicProperties[nLine].frame_num = (OMX_S32) atoi(vec[0]);
                            m_pDynamicProperties[nLine].config_param = (OMX_INDEXTYPE)OMX_QcomIndexConfigVideoVencPerfMode;
                            m_pDynamicProperties[nLine].config_data.perfmode.nPerfMode = (OMX_U32) atoi(vec[2]);
                        }
                        break;
                    default:
                        VTEST_MSG_ERROR("Error with dynamic config parsing line %d", (int)nLine);
                        break;
                    }
                    vec.clear();

                    ++nLine;
                }
                (void)dynamicFile.Close();

                m_sCodecConfig.pDynamicProperties = m_pDynamicProperties;
            } else if (Parser::StringICompare((OMX_STRING)"IFrameRequestPeriod", v[0]) == 0) {
                m_sDynamicConfig.nIFrameRequestPeriod = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("IFrameRequestPeriod = %d\n", (int)m_sDynamicConfig.nIFrameRequestPeriod);
            } else if (Parser::StringICompare((OMX_STRING)"UpdatedBitrate", v[0]) == 0) {
                m_sDynamicConfig.nUpdatedBitrate = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("UpdatedBitrate = %d\n", (int)m_sDynamicConfig.nUpdatedBitrate);
            } else if (Parser::StringICompare((OMX_STRING)"UpdatedFPS", v[0]) == 0) {
                m_sDynamicConfig.nUpdatedFramerate = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("UpdatedFPS = %d\n", (int)m_sDynamicConfig.nUpdatedFramerate);
            } else if (Parser::StringICompare((OMX_STRING)"UpdatedNumFrames", v[0]) == 0) {
                m_sDynamicConfig.nUpdatedFrames = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("UpdatedNumFrames = %d\n", (int)m_sDynamicConfig.nUpdatedFrames);
            } else if (Parser::StringICompare((OMX_STRING)"UpdatedIntraPeriod", v[0]) == 0) {
                m_sDynamicConfig.nUpdatedIntraPeriod = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("UpdatedIntraPeriod = %d\n",
                    (int)m_sDynamicConfig.nUpdatedIntraPeriod);
            } else if (Parser::StringICompare((OMX_STRING)"ProfileMode", v[0]) == 0) {
                m_sCodecConfig.bProfileMode = (OMX_BOOL)atoi(v[1]);
                VTEST_MSG_LOW("ProfileMode = %d\n", (int)m_sCodecConfig.bProfileMode);
            } else if (Parser::StringICompare((OMX_STRING)"SecureMode", v[0]) == 0) {
                m_sCodecConfig.bSecureSession = (OMX_BOOL)atoi(v[1]);
                VTEST_MSG_LOW("SecureMode = %d\n", (int)m_sCodecConfig.bSecureSession);
            } else if (Parser::StringICompare((OMX_STRING)"MDPFrameSource", v[0]) == 0) {
                m_sCodecConfig.bMdpFrameSource = (OMX_BOOL)atoi(v[1]);
                VTEST_MSG_LOW("MdpFrameSource = %d\n", (int)m_sCodecConfig.bMdpFrameSource);
            } else if (Parser::StringICompare((OMX_STRING)"InsertInbandVideoHeaders", v[0]) == 0) {
                m_sCodecConfig.bInsertInbandVideoHeaders = (OMX_BOOL)atoi(v[1]);
                VTEST_MSG_LOW("SPS/PPS I-frame headers enabled = %d",
                                  (int)m_sCodecConfig.bInsertInbandVideoHeaders);
            } else if (Parser::StringICompare((OMX_STRING)"InsertAUDelimiters", v[0]) == 0) {
                m_sCodecConfig.bInsertAUDelimiters = (OMX_BOOL)atoi(v[1]);
                VTEST_MSG_LOW("H264 AU delimiters enabled = %d",
                                  (int)m_sCodecConfig.bInsertAUDelimiters);
            } else if (Parser::StringICompare((OMX_STRING)"PlaybackMode", v[0]) == 0) {
                m_sCodecConfig.ePlaybackMode = (PlaybackModeType)ParseEnum(g_pPlaybackMode, v[1]);
                VTEST_MSG_LOW("PlaybackMode = %d", m_sCodecConfig.ePlaybackMode);
            } else if (Parser::StringICompare((OMX_STRING)"AdaptiveWidth", v[0]) == 0) {
                m_sCodecConfig.nAdaptiveWidth = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("AdaptiveWidth = %d", (int)m_sCodecConfig.nAdaptiveWidth);
            } else if (Parser::StringICompare((OMX_STRING)"AdaptiveHeight", v[0]) == 0) {
                m_sCodecConfig.nAdaptiveHeight = (OMX_S32)atoi(v[1]);
                VTEST_MSG_LOW("AdaptiveHeight = %d", (int)m_sCodecConfig.nAdaptiveHeight);
            } else if (Parser::StringICompare((OMX_STRING)"RotateDisplay", v[0]) == 0) {
                m_sCodecConfig.bRotateDisplay = (OMX_BOOL)atoi(v[1]);
                VTEST_MSG_LOW("RotateDisplay = %d\n", m_sCodecConfig.bRotateDisplay);
            } else if (Parser::StringICompare((OMX_STRING)"DecoderPictureOrder", v[0]) == 0) {
                m_sCodecConfig.eDecoderPictureOrder = (QOMX_VIDEO_PICTURE_ORDER)atoi(v[1]);
                VTEST_MSG_LOW("DecoderPictureOrder = %d\n", m_sCodecConfig.eDecoderPictureOrder);
            } else if (Parser::StringICompare((OMX_STRING)"SinkType", v[0]) == 0) {
                m_sCodecConfig.eSinkType = (SinkType)ParseEnum(g_pSinkType, v[1]);
                VTEST_MSG_LOW("SinkType = %d", m_sCodecConfig.eSinkType);
            } else if (Parser::StringICompare((OMX_STRING)"MetaMode", v[0]) == 0) {
                m_sCodecConfig.bMetaMode = (OMX_BOOL)atoi(v[1]);
                VTEST_MSG_LOW("MetaMode = %d", (int)m_sCodecConfig.bMetaMode);
            } else if (Parser::StringICompare((OMX_STRING)"Clamp_601to709", v[0]) == 0) {
                /* Adding this to match the legacy config option */
                OMX_BOOL bClamp_601To709 = (OMX_BOOL)atoi(v[1]);
                VTEST_MSG_LOW("Clamp_601To709 = %d", bClamp_601To709);
                m_sCodecConfig.eYuvColorSpace = ITUR709;
            } else if (Parser::StringICompare((OMX_STRING)"YuvColorSpace", v[0]) == 0) {
                m_sCodecConfig.eYuvColorSpace = (YuvColorSpace)ParseEnum(g_pYuvColorSpaceType, v[1]);
                VTEST_MSG_LOW("YuvColorSpace = %x", m_sCodecConfig.eYuvColorSpace);
            } else if (Parser::StringICompare((OMX_STRING)"DecoderDownScalar", v[0]) == 0) {
                m_sCodecConfig.bDecoderDownScalar = (OMX_BOOL)atoi(v[1]);
                VTEST_MSG_LOW("DecoderDownScalar = %d", (int)m_sCodecConfig.bMetaMode);
            } else if (Parser::StringICompare((OMX_STRING)"PostProcType", v[0]) == 0) {
                m_sCodecConfig.ePostProcType = (PostProcType)ParseEnum(g_pPostProcType, v[1]);
                VTEST_MSG_LOW("PostProcType = %d", m_sCodecConfig.ePostProcType);
            } else if (Parser::StringICompare((OMX_STRING)"InputColorFormat", v[0]) == 0) {
                m_sCodecConfig.nInputColorFormat = (OMX_U32)ParseEnum(g_pColorFormat, v[1]);
                VTEST_MSG_LOW("nInputColorFormat = %d", m_sCodecConfig.nInputColorFormat);
            } else if (Parser::StringICompare((OMX_STRING)"OutputColorFormat", v[0]) == 0) {
                m_sCodecConfig.nOutputColorFormat = (OMX_U32)ParseEnum(g_pColorFormat, v[1]);
                VTEST_MSG_LOW("nOutputColorFormat = %d", m_sCodecConfig.nOutputColorFormat);
            } else {
                VTEST_MSG_ERROR("invalid config file key line %d", (int)nLine);
            }
            break;
        default:
            VTEST_MSG_ERROR("error with config parsing line %d", (int)nLine);
            break;
        }
        v.clear();

        ++nLine;
    }

    (void)f.Close();
    memcpy(pCodecConfig, &m_sCodecConfig, sizeof(m_sCodecConfig));
    if (pDynamicConfig != NULL) { // optional param
        memcpy(pDynamicConfig, &m_sDynamicConfig, sizeof(m_sDynamicConfig));
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Config::GetCodecConfig(CodecConfigType *pCodecConfig) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    if (pCodecConfig != NULL) {
        memcpy(pCodecConfig, &m_sCodecConfig, sizeof(m_sCodecConfig));
    } else {
        result = OMX_ErrorBadParameter;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Config::GetDynamicConfig(DynamicConfigType *pDynamicConfig) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    if (pDynamicConfig != NULL) {
        memcpy(pDynamicConfig, &m_sDynamicConfig, sizeof(m_sDynamicConfig));
    } else {
        result = OMX_ErrorBadParameter;
    }
    return result;
}

} // namespace vtest
