/*==============================================================================
*        @file OMX_FileMux.cpp
*
*  @par DESCRIPTION:
*       This is the definition of the file Mux MMI interface class
*       This file has the implementation details of the class.
*
*
*  Copyright (c)  2011 - 2014  Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/omx/mm-mux/omxmux/src/omx_filemux.cpp#3 $


when        who         what, where, why
--------    ------      --------------------------------------------------------
08/18/09    sanal       Changed minimum port required to 1
08/17/09    sanal       Changes Malloc/ free apis to MM_Malloc and MM_Free
08/12/09    sanal       Changes for supporting Audio file formats
08/12/09    sanal       Fixes while Initial testing
07/28/09    sanal       tables for h263 and avc levels
07/28/09    sanal       Fixes after conformance testing.
07/27/09    sanal       Implemnted pause resume and flush, function headers
07/15/09    sanal       Created file
================================================================================
*/
#include "omx_filemux_extensions.h"
#include "omx_filemux.h"
#include "filemux.h"
#include "filemuxtypes.h"
#include "QOMX_CoreExtensions.h"
#include "OMX_CoreExt.h"
#include "QOMX_VideoExtensions.h"
#include "QOMX_SourceExtensions.h"
#include "QOMX_AudioIndexExtensions.h"
#include "omx_filemux_defines.h"
#include "MMFile.h"

#ifdef FEATURE_FILEMUX_MMI_BMP_DYNAMIC_MODULE_REGISTRATION
#include "qc_omx_core.h"
#endif

typedef union ExtraPtr
{
    OMX_U32     *U32Ptr;
    OMX_U8   *dataPtr;
}
ExtraDataPtr;

typedef union TickPtr
{
    OMX_TICKS   *numTicks;
    OMX_U8      *dataPtr;
    OMX_U64     *sizePtr;
}
OmxTickPtr;

#define OMX_INIT_STRUCT(_s_, _name_)            \
   memset((_s_), 0x0, sizeof(_name_));          \
   (_s_)->nSize = sizeof(_name_);               \
   (_s_)->nVersion.nVersion = OMX_SPEC_VERSION

/**-----------------------------------------------------------------------------
  OMX and MMI level paramaters for MUX
--------------------------------------------------------------------------------
*/
#define COMPONENT_NAME  "OMX.qcom.file.muxer"
#define COMPONENT_UUID  "UUID_OMX.QCOM.MUX.1.0.0"


OMX_BOOL             test_cmpnt_async_device = OMX_TRUE;





/**----------------------------------------------------------------------------
        File format  major brands,
-----------------------------------------------------------------------------*/
#ifdef WIN32
static const wchar_t DEFAULT_FILE_NAME[] = L"c:\\movie.3gp";
#else
static const wchar_t DEFAULT_FILE_NAME[] = L"/movie.3gp";
#endif
static const char *MP4_MAJOR_BRAND = "isom";
static const char *MP4_COMPAT_BRANDS [] = {
    "mp41", "isom"
};
static const char *AMC_MAJOR_BRAND = "isom";
static const char *AMC_COMPAT_BRANDS [] = {
    "mp41"
};
static const char *_3GP_MAJOR_BRAND = "3gp5";
static const char *_3GP_COMPAT_BRANDS [] = {
    "3gp5", "isom"
};
static const char *_3G2_MAJOR_BRAND = "3g2a";
static const char *_3G2_COMPAT_BRANDS [] = {
    "3g2a"
};
static const char *K3G_MAJOR_BRAND = "k3g1";
static const char *K3G_COMPAT_BRANDS [] = {
    "k3g1"
};
static const char *SKM_MAJOR_BRAND = "skm2";
static const char *SKM_COMPAT_BRANDS [] = {
    "skm2", "k3g1", "3gp5"
};
static const char *FRAG_3G2_MAJOR_BRAND = "kddi";
static const char *FRAG_3G2_COMPAT_BRANDS [] = {
    "kddi", "3g2a"
};
static const char *FRAG_3G2_MAJOR_BRAND_NOT_AMR = "kddi";
static const char *FRAG_3G2_COMPAT_BRANDS_NOT_AMR [] = {
    "kddi", "3g2a", "mp42"
};

/**-----------------------------------------------------------------------------
   AVC supported profiles.
--------------------------------------------------------------------------------
*/
static const OMX_U8 AVC_Profile_Table[16] =
{
    66,77, 88, 100, 110, 122, 144, 0 // TBD
};

/**-----------------------------------------------------------------------------
   AVC supported Levels.
--------------------------------------------------------------------------------
*/
static const OMX_U8 AVC_Level_Table[32] =
{
    10,
    11,
    11,
    12,
    13,
    20,
    21,
    22,
    30,
    31,
    32,
    40,
    41,
    42,
    50,
    51,
    0
};



/**-----------------------------------------------------------------------------
  Level table for H263
--------------------------------------------------------------------------------
*/
static const OMX_U8 H263_Level_Table[8] =
{
    10,
    20,
    30,
    40,
    45,
    50,
    60,
    70
};

/**-----------------------------------------------------------------------------
  Default Initialization for port as AMR
--------------------------------------------------------------------------------
*/
static const QOMX_AUDIO_PARAM_AMRWBPLUSTYPE sAMRDefaults =
{
    32,                          /**< size of the structure in bytes          */
    {0,0,0,0},                   /**< OMX specification version information   */
    OMX_MUX_INDEX_PORT_AUDIO,    /**< port that this structure applies to     */
    1,                           /**< Number of channels                      */
    12200,                       /**< Bit rate read only field                */
    8000,                         /**< Sample Rate                             */
    OMX_AUDIO_AMRBandModeNB7,    /**< AMR Band Mode enumeration               */
    OMX_AUDIO_AMRDTXModeOff,     /**< AMR DTX Mode enumeration                */
    OMX_AUDIO_AMRFrameFormatConformance/**< AMR frame format enumeration     */
};

/**-----------------------------------------------------------------------------
  Default Initialization for port as MPEG4
--------------------------------------------------------------------------------
*/
static const OMX_VIDEO_PARAM_MPEG4TYPE sMPEG4Defaults =
{
    68,                          /**<OMX_U32 nSize;                           */
    {0,0,0,0},                   /**<OMX_VERSIONTYPE nVersion;                */
    OMX_MUX_INDEX_PORT_VIDEO,    /**<OMX_U32 nPortIndex;                      */
    0,                           /**<OMX_U32 nSliceHeaderSpacing;             */
    OMX_FALSE,                   /**<OMX_BOOL bSVH;                           */
    OMX_TRUE,                    /**<OMX_BOOL bGov;                           */
    29,                          /**<OMX_U32 nPFrames;                        */
    0,                           /**<OMX_U32 nBFrames;                        */
    3000,                        /**<OMX_U32 nIDCVLCThreshold;                */
    OMX_TRUE,                    /**<OMX_BOOL bACPred;                        */
    100000,                      /**<OMX_U32 nMaxPacketSize;                  */
    30,                          /**<OMX_U32 nTimeIncRes;                     */
    OMX_VIDEO_MPEG4ProfileSimple,/**<OMX_VIDEO_MPEG4PROFILETYPE eProfile;     */
    OMX_VIDEO_MPEG4Level3,       /**<OMX_VIDEO_MPEG4LEVELTYPE eLevel;         */
    2,                           /**<OMX_U32 nAllowedPictureTypes;            */
    0,                           /**<OMX_U32 nHeaderExtension;                */
    OMX_FALSE                    /**<OMX_BOOL bReversibleVLC;                 */
};

/**-----------------------------------------------------------------------------
@brief All supported Audio codecs are stored here.

@detail
       When client enumerates the supported codecs by the port these are
       passed one by one.
--------------------------------------------------------------------------------
*/
static const OMX_AUDIO_CODINGTYPE
             gAudioSupported[] =
{
    OMX_AUDIO_CodingAAC,
    OMX_AUDIO_CodingAMR,
    OMX_AUDIO_CodingQCELP13,
    OMX_AUDIO_CodingEVRC,
    OMX_AUDIO_CodingUnused,
    OMX_AUDIO_CodingPCM,
    OMX_AUDIO_CodingG711,
    (OMX_AUDIO_CODINGTYPE)QOMX_AUDIO_CodingEVRCB,
    (OMX_AUDIO_CODINGTYPE)QOMX_AUDIO_CodingEVRCWB
};

static const OMX_VIDEO_CODINGTYPE
             gVideoSupported[] =
{
    OMX_VIDEO_CodingAVC,
    OMX_VIDEO_CodingMPEG4,
    OMX_VIDEO_CodingH263,
    OMX_VIDEO_CodingUnused
};

/**-----------------------------------------------------------------------------
@brief All supported Filemux roles are stored here.

@detail
       When client enumerates the supported roles these are
       passed one by one.
--------------------------------------------------------------------------------
*/
static const char  gRolesSupported
          [OMX_MUX_MAX_SUPPORTED_ROLES]
          [OMX_MAX_STRINGNAME_SIZE] =
{
    "container_muxer.3g2",
    "container_muxer.mp4",
    "container_muxer.3gp",
    "container_muxer.amc",
    "container_muxer.skm",
    "container_muxer.k3g",
    "container_muxer.wav",
    "container_muxer.amr",
    "container_muxer.aac",
    "container_muxer.evr",
    "container_muxer.evw",
    "container_muxer.qcp",
    "container_muxer.mp2"
};


/**-----------------------------------------------------------------------------
  AAC silence frames used for AVSync
--------------------------------------------------------------------------------
*/
static  OMX_U8 AAC8Kmonosilence[128] =
{
    0x01, 0x40, 0x20, 0x06, 0xf6, 0xc0, 0x00, 0x00, /* 8k Mono Silence        */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Mono Silence        */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Mono Silence        */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Mono Silence        */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Mono Silence        */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Mono Silence        */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Mono Silence        */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Mono Silence        */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Mono Silence        */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Mono Silence        */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Mono Silence        */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Mono Silence        */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Mono Silence        */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Mono Silence        */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Mono Silence        */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f  /* 8k Mono Silence        */
};
/**-----------------------------------------------------------------------------
  AAC silence frames used for AVSync
--------------------------------------------------------------------------------
*/
static  OMX_U8 AAC8Kstereosilence[256] =
{
    0x21, 0x10, 0x05, 0x00, 0xa0, 0x1b, 0xfa, 0x80, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8k Stereo Silence      */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3a  /* 8k Stereo Silence      */
};

static  OMX_U8 AAC11Kmonosilence[140] =
{
    0x01, 0x40, 0x20, 0x06, 0xf7, 0x80, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Mono Silence       */
    0x00, 0x00, 0x00, 0x0E                          /* 11k Mono Silence       */
};

static  OMX_U8 AAC11Kstereosilence[279] =
{
    0x21, 0x10, 0x05, 0x00, 0xa0, 0x1b, 0xff, 0xc0, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 11k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x30, 0x80, 0x72        /* 11k Stereo Silence     */
};

static  unsigned char AAC12Kmonosilence[128] =
{
    0x01, 0x40, 0x20, 0x06, 0xf6, 0xc0, 0x00, 0x00, /* 12k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f  /* 12k Mono Silence       */
};

static  unsigned char AAC12Kstereosilence[256] =
{
    0x21, 0x10, 0x05, 0x00, 0xa0, 0x1b, 0xfa, 0x80, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 12k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f  /* 12k Stereo Silence     */
};

static  unsigned char AAC16Kmonosilence[128] =
{
    0x01, 0x40, 0x20, 0x06, 0xf6, 0xc0, 0x00, 0x00, /* 16k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e  /* 16k Mono Silence       */
};

static  unsigned char AAC16Kstereosilence[256] =
{
    0x21, 0x10, 0x05, 0x00, 0xa0, 0x1b, 0xfa, 0x80, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16k Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3d  /* 16k Stereo Silence     */
};

static  unsigned char AAC22Kmonosilence[139] =
{
    0x01, 0x40, 0x20, 0x06, 0xf7, 0x70, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Mono Silence       */
    0x00, 0x00, 0x0f                                /* 22K Mono Silence       */
};

static  unsigned char AAC22Kstereosilence[263] =
{
    0x21, 0x10, 0x05, 0x00, 0xa0, 0x1b, 0xff, 0xc0, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 22K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x30, 0x80, 0x73        /* 22K Stereo Silence     */
};

static  unsigned char AAC24Kmonosilence[128] =
{
    0x01, 0x40, 0x20, 0x06, 0xf6, 0xc0, 0x00, 0x00, /* 24K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e  /* 24K Mono Silence       */
};

static  unsigned char AAC24Kstereosilence[255] =
{
    0x21, 0x10, 0x05, 0x00, 0xa0, 0x1b, 0xfa, 0x80, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        /* 24K Stereo Silence     */
};

static  unsigned char AAC32Kmonosilence[128] =
{
    0x01, 0x40, 0x20, 0x06, 0xf6, 0xc0, 0x00, 0x00, /* 32K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e  /* 32K Mono Silence       */
};

static  unsigned char AAC32Kstereosilence[256] =
{
    0x21, 0x10, 0x05, 0x00, 0xa0, 0x1b, 0xfa, 0x80, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 32K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f  /* 32K Stereo Silence     */
};


static  unsigned char AAC44Kmonosilence[139] =
{
    0x01, 0x40, 0x20, 0x06, 0x8d, 0x02, 0x80, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x0d, 0xed, 0xc0, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Mono Silence       */
    0x00, 0x00, 0x1e
};

static  unsigned char AAC44Kstereosilence[279] =
{
    0x21, 0x10, 0x05, 0x00, 0xa0, 0x1b, 0xff, 0xc0, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 44K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x30, 0x80, 0x7e        /* 44K Stereo Silence     */
};

static  unsigned char AAC48Kmonosilence[128] =
{
    0x01, 0x40, 0x20, 0x06, 0xf6, 0xc0, 0x00, 0x00, /* 48K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Mono Silence       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e  /* 48K Mono Silence       */
};


static  unsigned char AAC48Kstereosilence[256] =
{
    0x21, 0x10, 0x05, 0x00, 0xa0, 0x1b, 0xfa, 0x80, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 48K Stereo Silence     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3d  /* 48K Stereo Silence     */
};


static OMX_U8 AMR_SILENT_FRAME_DATA[]=
{
    0x7c                                            /* AMR Silence            */
};

/**-----------------------------------------------------------------------------
   QCELP and EVRC currently uses 0 bytes frame for silence. This is just to have
   a valid pointer.
--------------------------------------------------------------------------------
*/
static OMX_U8 QCELP_EVRC_SILENT_FRAME_DATA[] =
{
    0x00                                            /* QCELP EVRC Silence     */
};

#define MAX_AVSYNC_SILENCE_DURATION    10000

/**-----------------------------------------------------------------------------
@brief All AAC Sampling Rates.

@detail
       This table helps to convert sampling rate index to sampling rate.
--------------------------------------------------------------------------------
*/
static const uint32 nAACSamplingRates[16] =
{
    96000,
    88200,
    64000,
    48000,
    44100,
    32000,
    24000,
    22050,
    16000,
    12000,
    11025,
    8000,
    0
};


/*******************************************************************************
*         FUNCTION DEFINITIONS FOR THE CURRENT IMPLEMENTATION OF OMX INTERFACE
********************************************************************************
*/
/*==============================================================================

         FUNCTION:         get_omx_component_factory_fn

         DESCRIPTION:
*//**       @brief         entry function which creates instance
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:


*//*==========================================================================*/

OMX_API   void* get_omx_component_factory_fn(void)
   {
     return (void*)OMX_FileMux::get_instance();
   }

/*==============================================================================

         FUNCTION:         get_instance

         DESCRIPTION:
*//**       @brief         Creates OMX FileMux instance
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_FileMux*

@par     SIDE EFFECTS:
*//*==========================================================================*/
OMX_FileMux* OMX_FileMux::get_instance()
{
    OMX_HANDLETYPE pHandle;
    OMX_FileMux_Open(&pHandle);

    return (OMX_FileMux*)pHandle;
}
/*==============================================================================

         FUNCTION:         get_component_version

         DESCRIPTION:
*//**       @brief         Creates MMI FileMux instance
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent
                           pComponentName
                           pComponentVersion
                           pSpecVersion
                           pComponentUUID
*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::get_component_version(
                          OMX_IN  OMX_HANDLETYPE hComponent,
                          OMX_OUT OMX_STRING pComponentName,
                          OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                          OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
                          OMX_OUT OMX_UUIDTYPE* pComponentUUID)
{ ///@todo finish implement
    if (hComponent == NULL ||
        pComponentName == NULL ||
        pComponentVersion == NULL ||
        pSpecVersion == NULL ||
        pComponentUUID == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    if(m_pComponentName && m_pComponentName)
    {
        memcpy(pComponentName, m_pComponentName, OMX_MAX_STRINGNAME_SIZE);
    }
    pSpecVersion->nVersion = OMX_SPEC_VERSION;
    return OMX_ErrorNone;
}


/*==============================================================================

         FUNCTION:         Set_Target_State

         DESCRIPTION:
*//**       @brief         Sets the next state to move to
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         State

*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:


*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::Set_Target_State(OMX_STATETYPE eState)
{
    m_eTargetState = eState ;
    OMX_BOOL  eStateTransitionDone = OMX_FALSE;

    OMX_ERRORTYPE eError = OMX_ErrorNone;

    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,"Mux, state change req, current = %d, next = %d", (int32)m_eState, (int32)m_eTargetState);

    switch((int32)m_eState)
    {
        case OMX_StateInvalid:
            eError = OMX_ErrorIncorrectStateTransition;
            break;

        case OMX_StateLoaded:
            if(eState == OMX_StateIdle)
            {
                eError = OMX_FileMux_LoadResources();
                break;
            }
            else if(eState == OMX_StateWaitForResources)
            {
                eStateTransitionDone = OMX_TRUE;
                break;
            }
            else if(eState == OMX_StateInvalid)
            {
                eStateTransitionDone = OMX_TRUE;
                break;
            }
            else
            {
                eError = OMX_ErrorIncorrectStateTransition;
            }
            break;

        case OMX_StateIdle:
            if(eState == OMX_StateExecuting)
            {
                eError = OMX_FileMux_Start();
                eStateTransitionDone = OMX_TRUE;
                break;
            }
            else if(eState == OMX_StateLoaded)
            {
                eError = OMX_FileMux_ReleaseResources();
                break;
            }
            else if(eState == OMX_StatePause)
            {
                eError = OMX_FileMux_Pause();
                if(m_eState == OMX_StatePause)
                {
                    eStateTransitionDone = OMX_TRUE;
                }
                break;
            }
            else if(eState == OMX_StateInvalid)
            {
                eError = OMX_FileMux_Stop();
                break;
            }
            else
            {
                eError = OMX_ErrorIncorrectStateTransition;
            }
            break;


        case OMX_StateExecuting:
            if(eState == OMX_StateIdle)
            {

                eError = OMX_FileMux_Stop();

                break;
            }
            else if(eState == OMX_StatePause)
            {

                eError = OMX_FileMux_Pause();
                if(m_eState == OMX_StatePause)
                {
                    eStateTransitionDone = OMX_TRUE;
                }
                break;
            }
            else if(eState == OMX_StateInvalid)
            {
                eError = OMX_FileMux_Stop();
                break;
            }
            else
            {
                eError = OMX_ErrorIncorrectStateTransition;
            }
            break;


        case OMX_StatePause:
            if(eState == OMX_StateExecuting)
            {
                eError = OMX_FileMux_Resume();
                eStateTransitionDone = OMX_TRUE;
                break;
            }
            else if(eState == OMX_StateIdle)
            {
                eError = OMX_FileMux_Stop();
                break;
            }
            else if(eState == OMX_StateInvalid)
            {
                eError = OMX_FileMux_Stop();
                break;
            }
            else
            {
                eError = OMX_ErrorIncorrectStateTransition;
            }
            break;

        case OMX_StateWaitForResources:
            if(eState == OMX_StateIdle)
            {
                eError = OMX_FileMux_LoadResources();
                break;
            }
            else if(eState == OMX_StateLoaded)
            {
                eStateTransitionDone = OMX_TRUE;
                break;
            }
            else
            {
                eError = OMX_ErrorIncorrectStateTransition;
            }
            break;
    }
    if(eError != OMX_ErrorNone)
    {
        m_pCallbacks->EventHandler(m_hSelf, m_pAppData
                                   ,OMX_EventError,
                                   eError,
                                   0, NULL);
    }
    if(eError == OMX_ErrorNone && eStateTransitionDone == OMX_TRUE)
    {
        m_eState = eState;
        m_pCallbacks->EventHandler(
                           m_hSelf, m_pAppData, OMX_EventCmdComplete,
                           OMX_CommandStateSet, eState, NULL);
    }
    return eError;
}

/*==============================================================================

         FUNCTION:         send_command

         DESCRIPTION:
*//**       @brief         Receives the commands from IL client here
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent- Component instance handle
                           nCmd      - Command
                           nParam1   - Data with command
                           pCmdData  - Additional data structure

*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::send_command(OMX_IN  OMX_HANDLETYPE hComponent,
                                 OMX_IN  OMX_COMMANDTYPE Cmd,
                                 OMX_IN  OMX_U32 nParam1,
                                 OMX_IN  OMX_PTR pCmdData)
{
    (void) hComponent;
    (void) pCmdData;
    switch (Cmd)
    {
    case OMX_CommandStateSet:
        if ((OMX_STATETYPE)nParam1 == m_eState)
        {
            return OMX_ErrorSameState;
        }
        else
        {
            if ((OMX_STATETYPE)nParam1 == m_eTargetState)
            {
                return  OMX_ErrorNone;
            }
            else
            {
                Set_Target_State((OMX_STATETYPE)nParam1);
            }
        }
        break;
    case OMX_CommandFlush:
        OMX_FileMux_Flush(nParam1);
        break;
    case OMX_CommandPortDisable:
        OMX_FileMux_DisablePort(nParam1);
        break;
    case OMX_CommandPortEnable:
        OMX_FileMux_EnablePort(nParam1);
        break;
    case OMX_CommandMarkBuffer:
        return OMX_ErrorNotImplemented;
    default:
        return OMX_ErrorBadParameter;
    }


    return OMX_ErrorNone;
}
/*==============================================================================

         FUNCTION:         get_parameter

         DESCRIPTION:
*//**       @brief         Gets paramaters from component
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent  - Component handle
                           nParamIndex - Index type to get
                           pCompParam  - associated data


*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::get_parameter(
                                  OMX_IN  OMX_HANDLETYPE hComponent,
                                  OMX_IN  OMX_INDEXTYPE nParamIndex,
                                  OMX_INOUT OMX_PTR pCompParam)
{
    if(!hComponent)
    {
        return OMX_ErrorBadParameter;
    }

    return OMX_FileMux_GetSetParam(
                                   OMX_MUX_CMD_GET_STD_OMX_PARAM,
                                   nParamIndex,
                                   pCompParam);
}

/*==============================================================================

         FUNCTION:         set_parameter

         DESCRIPTION:
*//**       @brief         Sets paramaters to component
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent  - Component handle
                           nParamIndex - Index type to set
                           pCompParam  - associated data


*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::set_parameter(
                                  OMX_IN  OMX_HANDLETYPE hComponent,
                                  OMX_IN  OMX_INDEXTYPE nIndex,
                                  OMX_IN  OMX_PTR pCompParam)
{

    if(!hComponent)
    {
        return OMX_ErrorBadParameter;
    }

    return OMX_FileMux_GetSetParam(
                                   OMX_MUX_CMD_SET_STD_OMX_PARAM,
                                   nIndex,
                                   pCompParam);
}

/*==============================================================================

         FUNCTION:         get_config

         DESCRIPTION:
*//**       @brief         Gets runtime config from component
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent  - Component handle
                           nParamIndex - Index type to get
                           pCompParam  - associated data


*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::get_config(
                               OMX_IN  OMX_HANDLETYPE hComponent,
                               OMX_IN  OMX_INDEXTYPE nIndex,
                               OMX_INOUT OMX_PTR pCompConfig)
{
    if(!hComponent)
    {
        return OMX_ErrorBadParameter;
    }


    return  OMX_FileMux_GetSetParam(
                                   OMX_MUX_CMD_GET_STD_OMX_PARAM,
                                   nIndex,
                                   pCompConfig);
}

/*==============================================================================

         FUNCTION:         set_config

         DESCRIPTION:
*//**       @brief         sets runtime config to component
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent  - Component handle
                           nParamIndex - Index type to get
                           pCompParam  - associated data


*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::set_config(
                               OMX_IN  OMX_HANDLETYPE hComponent,
                               OMX_IN  OMX_INDEXTYPE nIndex,
                               OMX_IN  OMX_PTR pCompConfig)
{
    if(!hComponent)
    {
        return OMX_ErrorBadParameter;
    }


    return OMX_FileMux_GetSetParam(
                                   OMX_MUX_CMD_SET_STD_OMX_PARAM,
                                   nIndex,
                                   pCompConfig);
}

/*==============================================================================

         FUNCTION:         get_extension_index

         DESCRIPTION:
*//**       @brief         Gets the index for a particular extension
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent     - Component handle
                           cParameterName - Index name as a string
                           pIndexType      - Pointer to store index


*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::get_extension_index(
                                   OMX_IN  OMX_HANDLETYPE hComponent,
                                   OMX_IN  OMX_STRING cParameterName,
                                   OMX_OUT OMX_INDEXTYPE* pIndexType)
{
    (void) hComponent;
    return OMX_FileMux_GetExtensionIndex(cParameterName, pIndexType);
}

/*==============================================================================

         FUNCTION:         get_state

         DESCRIPTION:
*//**       @brief         Gets the component state
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent  - Component handle
                           pState      - pointer to store OMX_STATETYPE


*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::get_state(OMX_IN  OMX_HANDLETYPE hComponent,
                              OMX_OUT OMX_STATETYPE* pState)
{
    (void) hComponent;
    if (pState == NULL)
    {
        return OMX_ErrorBadParameter;
    }
    *pState = m_eState;
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         component_tunnel_request

         DESCRIPTION:
*//**       @brief         Setup tunneling between two components
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComp  -  component handle
                           nPort  - port to setup tunnel
                           hTunneledComp - component to which a tunnel to setup
                           nTunneledPort - Port of other end component
                           pTunnelSetup - pointer to store tunnelsetptype

*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::component_tunnel_request(
                           OMX_IN  OMX_HANDLETYPE hComp,
                           OMX_IN  OMX_U32 nPort,
                           OMX_IN  OMX_HANDLETYPE hTunneledComp,
                           OMX_IN  OMX_U32 nTunneledPort,
                           OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
    (void) hComp;
    (void) nPort;
    (void) hTunneledComp;
    (void) nTunneledPort;
    (void) pTunnelSetup;
    return OMX_ErrorTunnelingUnsupported;
}
/*==============================================================================

         FUNCTION:         use_buffer

         DESCRIPTION:
*//**       @brief         Clients registers a buffer to use
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent
                           ppBufferHdr
                           nPortIndex
                           pAppPrivate
                           nSizeBytes
                           pBuffer


*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::use_buffer(
                               OMX_IN OMX_HANDLETYPE hComponent,
                               OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                               OMX_IN OMX_U32 nPortIndex,
                               OMX_IN OMX_PTR pAppPrivate,
                               OMX_IN OMX_U32 nSizeBytes,
                               OMX_IN OMX_U8* pBuffer)
{
    (void)hComponent;
    if (ppBufferHdr == NULL || nSizeBytes == 0 || pBuffer == NULL)
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"bad param");
        return OMX_ErrorBadParameter;
    }

    if (nPortIndex == (OMX_U32) OMX_MUX_INDEX_PORT_AUDIO)
    {
        OMX_FileMuxPortInfoType *pAudPortConfig =
                                     (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO);

        OMX_PARAM_PORTDEFINITIONTYPE *pAudPortDef =
                                      &pAudPortConfig->sPortDef;

        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_LOW,
                           "client allocated input buffer for component");

        if (nSizeBytes != pAudPortDef->nBufferSize)
        {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,
                "buffer size does not match our requirements");
            return OMX_ErrorBadParameter;
        }

        if(! IS_AUDIO_PORT_USED)
        {
            return OMX_ErrorPortsNotCompatible;
        }

        if(!pAudPortConfig->pBuffHeaderArray)
        {
            pAudPortConfig->pBuffHeaderArray
                = (OMX_BUFFERHEADERTYPE*)MM_Malloc(
             sizeof(OMX_BUFFERHEADERTYPE) * pAudPortDef->nBufferCountActual);

            if(!pAudPortConfig->pBuffHeaderArray)
            {
                return OMX_ErrorInsufficientResources;
            }

            OMX_MUX_MEM_SET(
            pAudPortConfig->pBuffHeaderArray,
            0, sizeof(OMX_BUFFERHEADERTYPE) * pAudPortDef->nBufferCountActual);

            if(pAudPortConfig->pbComponentAllocated)
            {
                MM_Free((arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)
                   ->pbComponentAllocated);
            }
            pAudPortConfig->pbComponentAllocated
                    = (OMX_BOOL*)MM_Malloc(sizeof(OMX_BOOL) *
                     pAudPortDef->nBufferCountActual);
            if(
             !pAudPortConfig->pbComponentAllocated
              )
            {
                return OMX_ErrorInsufficientResources;
            }
            OMX_MUX_MEM_SET(
            pAudPortConfig->pbComponentAllocated,
            0, sizeof(OMX_BOOL) * pAudPortDef->nBufferCountActual);
        }

        unsigned int i;
        OMX_BUFFERHEADERTYPE * pAudBuffHeaders =
                                            pAudPortConfig->pBuffHeaderArray;

        OMX_BOOL   *bComponentAllocated =
                                         pAudPortConfig->pbComponentAllocated;

        for (i = 0; i < pAudPortDef->nBufferCountActual; i++)
        {
            if (pAudBuffHeaders[i].nAllocLen == 0)
            {
                if(pAudPortConfig->nNumBuffAllocated
                    < pAudPortDef->nBufferCountActual)
                {
                    pAudPortConfig->nNumBuffAllocated++;
                }
                OMX_INIT_STRUCT(&pAudBuffHeaders[i], OMX_BUFFERHEADERTYPE);

                pAudPortConfig->bUnPopulated         = OMX_FALSE;
                pAudBuffHeaders[i].pBuffer           = pBuffer;
                pAudBuffHeaders[i].nAllocLen         = nSizeBytes;
                pAudBuffHeaders[i].pAppPrivate       = pAppPrivate;
                pAudBuffHeaders[i].nInputPortIndex   =
                                         (OMX_U32) OMX_MUX_INDEX_PORT_AUDIO;
                pAudBuffHeaders[i].nOutputPortIndex  =
                                          (OMX_U32) OMX_MUX_INDEX_PORT_NONE;
                pAudBuffHeaders[i].pInputPortPrivate = &bComponentAllocated[i];
                bComponentAllocated[i]               = OMX_FALSE;

               *ppBufferHdr = &pAudBuffHeaders[i];

                if(pAudPortConfig->nNumBuffAllocated
                                            == pAudPortDef->nBufferCountActual)
                {
                    pAudPortConfig->bPopulated = OMX_TRUE;
                    if(pAudPortConfig->bEnableRequested)
                    {
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW,
                        "Port Populated.. Enable Video Port");
                        m_pCallbacks->EventHandler(
                                              m_hSelf, m_pAppData,
                                              OMX_EventCmdComplete,
                                              OMX_CommandPortEnable,
                                              OMX_MUX_INDEX_PORT_AUDIO, NULL);

                        pAudPortConfig->bEnableRequested = OMX_FALSE;
                        pAudPortConfig->sPortDef.bEnabled = OMX_TRUE;
                    }
                }
                break;
            }
        }
        if (i == pAudPortDef->nBufferCountActual)
        {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,
                                        "could not find free buffer");
            return OMX_ErrorUndefined;
        }
    }
    else if (nPortIndex == (OMX_U32) OMX_MUX_INDEX_PORT_VIDEO)
    {
        OMX_FileMuxPortInfoType *pVidPortConfig =
                                     (arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO);
        OMX_PARAM_PORTDEFINITIONTYPE *pVidPortDef =
                                      &pVidPortConfig->sPortDef;

        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_LOW,
                               "client allocated output buffer for component");

        if (nSizeBytes < pVidPortDef->nBufferSize)
        {
            MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_ERROR,
                       "buffer size does not match our requirements, %ld %ld",
                         nSizeBytes, pVidPortDef->nBufferSize);
            return OMX_ErrorBadParameter;
        }

        if(! IS_VIDEO_PORT_USED)
        {
            return OMX_ErrorPortsNotCompatible;
        }

        if(!pVidPortConfig->pBuffHeaderArray)
        {
            pVidPortConfig->pBuffHeaderArray
                                         = (OMX_BUFFERHEADERTYPE*)MM_Malloc(
                                            sizeof(OMX_BUFFERHEADERTYPE) *
                                           pVidPortDef->nBufferCountActual);

            if(!pVidPortConfig->pBuffHeaderArray)
            {
                return OMX_ErrorInsufficientResources;
            }

            OMX_MUX_MEM_SET(
            pVidPortConfig->pBuffHeaderArray,
            0, sizeof(OMX_BUFFERHEADERTYPE) * pVidPortDef->nBufferCountActual);


            if(pVidPortConfig->pbComponentAllocated)
            {
                MM_Free(pVidPortConfig->pbComponentAllocated);
            }


            pVidPortConfig->pbComponentAllocated
                                      = (OMX_BOOL*)MM_Malloc(sizeof(OMX_BOOL) *
                                         pVidPortDef->nBufferCountActual);

            if(!pVidPortConfig->pbComponentAllocated)
            {
                return OMX_ErrorInsufficientResources;
            }

            OMX_MUX_MEM_SET(pVidPortConfig->pbComponentAllocated,
                         0, sizeof(OMX_BOOL) * pVidPortDef->nBufferCountActual);
        }

        unsigned int i;
        OMX_BUFFERHEADERTYPE * pVidBuffHeaders =
                           pVidPortConfig->pBuffHeaderArray;
        OMX_BOOL   *bComponentAllocated =
                           pVidPortConfig->pbComponentAllocated;


        for (i = 0; i < pVidPortDef->nBufferCountActual; i++)
        {
            if (pVidBuffHeaders[i].nAllocLen == 0)
            {
                if(pVidPortConfig->nNumBuffAllocated
                               < pVidPortDef->nBufferCountActual)
                {
                    pVidPortConfig->nNumBuffAllocated++;
                    if(pVidPortConfig->nNumBuffAllocated
                                      == pVidPortDef->nBufferCountActual)
                    {
                        pVidPortConfig->bPopulated = OMX_TRUE;
                    }
                }

                pVidPortConfig->bUnPopulated = OMX_FALSE;
                OMX_INIT_STRUCT(&pVidBuffHeaders[i], OMX_BUFFERHEADERTYPE);
                pVidBuffHeaders[i].pBuffer           = pBuffer;
                pVidBuffHeaders[i].nAllocLen         = nSizeBytes;
                pVidBuffHeaders[i].pAppPrivate       = pAppPrivate;
                pVidBuffHeaders[i].nInputPortIndex   =
                                            (OMX_U32) OMX_MUX_INDEX_PORT_VIDEO;
                pVidBuffHeaders[i].nOutputPortIndex  =
                                             (OMX_U32) OMX_MUX_INDEX_PORT_NONE;
                pVidBuffHeaders[i].pInputPortPrivate =
                                                   (&bComponentAllocated[i]);
                bComponentAllocated[i] = OMX_FALSE;

               *ppBufferHdr = &pVidBuffHeaders[i];

               if(pVidPortConfig->nNumBuffAllocated
                                  == pVidPortDef->nBufferCountActual)
               {
                   pVidPortConfig->bPopulated = OMX_TRUE;
                   if(pVidPortConfig->bEnableRequested)
                   {
                       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW,
                        "Port Populated.. Enable Video Port");

                       m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                   OMX_EventCmdComplete,
                                   OMX_CommandPortEnable,
                                   OMX_MUX_INDEX_PORT_VIDEO, NULL);

                       pVidPortConfig->sPortDef.bEnabled = OMX_TRUE;
                       pVidPortConfig->bEnableRequested = OMX_FALSE;
                   }
               }

               break;
           }
       }
       if (i == pVidPortDef->nBufferCountActual)
       {
           MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"could not find free buffer");
           return OMX_ErrorUndefined;
       }
    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"invalid port index");
        return OMX_ErrorBadPortIndex;
    }

    if(m_eState == OMX_StateLoaded && m_eTargetState == OMX_StateIdle)
    {
        if(
          ((arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO)->bPopulated == OMX_TRUE
            || (!IS_VIDEO_PORT_USED))
           &&
          ((arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->bPopulated == OMX_TRUE
            || (!IS_AUDIO_PORT_USED))
          )
        {
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"mux go to idle ");
            m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                       OMX_EventCmdComplete,
                                       OMX_CommandStateSet,
                                       OMX_StateIdle, NULL);
            m_eState = OMX_StateIdle;
        }
    }

    return OMX_ErrorNone;
}
/*==============================================================================

         FUNCTION:         allocate_buffer

         DESCRIPTION:
*//**       @brief         Clients requests component to allocate buffer
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent
                           ppBufferHdr
                           nPortIndex
                           pAppPrivate
                           nSizeBytes


*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::allocate_buffer(
                                    OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
                                    OMX_IN OMX_U32 nPortIndex,
                                    OMX_IN OMX_PTR pAppPrivate,
                                    OMX_IN OMX_U32 nSizeBytes)
{
    (void) hComponent;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    if (ppBuffer == NULL || nSizeBytes == 0 )
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"allocate_buffer::bad param");
        return OMX_ErrorBadParameter;
    }

    if (nPortIndex == (OMX_U32) OMX_MUX_INDEX_PORT_AUDIO)
    {
        OMX_PARAM_PORTDEFINITIONTYPE *pAudPortDef =
            &(arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->sPortDef;
        OMX_FileMuxPortInfoType  *pAudPortConfig =
                              (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO);

        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_LOW,
                              "client allocated input buffer for component");

        if (nSizeBytes != pAudPortDef->nBufferSize)
        {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,
                               "buffer size does not match our requirements");

            return OMX_ErrorBadParameter;
        }

        if(!pAudPortConfig->pBuffHeaderArray)
        {
            OMX_U32 nAllocSize = (OMX_U32)(sizeof(OMX_BUFFERHEADERTYPE) *
                (pAudPortDef->nBufferCountActual));

            pAudPortConfig->pBuffHeaderArray
                               = (OMX_BUFFERHEADERTYPE*)MM_Malloc(nAllocSize);


            if(!pAudPortConfig->pBuffHeaderArray)
            {
                return OMX_ErrorInsufficientResources;
            }

            OMX_MUX_MEM_SET(
                 pAudPortConfig->pBuffHeaderArray,
                 0, nAllocSize);

            if(pAudPortConfig->pbComponentAllocated)
            {
                MM_Free(pAudPortConfig->pbComponentAllocated);
            }

            nAllocSize = (OMX_U32)(
                sizeof(OMX_BOOL) * pAudPortDef->nBufferCountActual);

            pAudPortConfig->pbComponentAllocated
                                          = (OMX_BOOL*)MM_Malloc(nAllocSize);

            if(
              !pAudPortConfig->pbComponentAllocated)
            {
                return OMX_ErrorInsufficientResources;
            }
            OMX_MUX_MEM_SET(
               pAudPortConfig->pbComponentAllocated,
               0, nAllocSize);
        }

        unsigned int i;

        OMX_BUFFERHEADERTYPE * pAudBuffHeaders =
                                        pAudPortConfig->pBuffHeaderArray;

        OMX_BOOL   *bComponentAllocated =
                                     pAudPortConfig->pbComponentAllocated;

        for (i = 0; i < pAudPortDef->nBufferCountActual; i++)
        {
            if (pAudBuffHeaders[i].nAllocLen == 0)
            {
                if(pAudPortConfig->nNumBuffAllocated
                    <  pAudPortDef->nBufferCountActual
                )
                {
                    pAudPortConfig->nNumBuffAllocated++;
                }

                OMX_INIT_STRUCT(&pAudBuffHeaders[i], OMX_BUFFERHEADERTYPE);

                pAudPortConfig->bUnPopulated = OMX_FALSE;

                pAudBuffHeaders[i].pBuffer = (OMX_U8*)MM_Malloc(nSizeBytes);

                if(!pAudBuffHeaders[i].pBuffer)
                {
                    return OMX_ErrorInsufficientResources;
                }

                pAudBuffHeaders[i].nAllocLen = nSizeBytes;
                pAudBuffHeaders[i].pAppPrivate = pAppPrivate;

                pAudBuffHeaders[i].nInputPortIndex =
                                          (OMX_U32) OMX_MUX_INDEX_PORT_AUDIO;
                pAudBuffHeaders[i].nOutputPortIndex =
                                          (OMX_U32) OMX_MUX_INDEX_PORT_NONE;

                pAudBuffHeaders[i].pInputPortPrivate = &bComponentAllocated[i];
                bComponentAllocated[i] = OMX_TRUE;

                *ppBuffer = &pAudBuffHeaders[i];

                if(pAudPortConfig->nNumBuffAllocated
                                  == pAudPortDef->nBufferCountActual)
                {
                    pAudPortConfig->bPopulated = OMX_TRUE;
                    if(pAudPortConfig->bEnableRequested)
                    {
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW,
                              "Port Populated.. Enable Video Port");

                        m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                   OMX_EventCmdComplete,
                                   OMX_CommandPortEnable,
                                   OMX_MUX_INDEX_PORT_AUDIO, NULL);
                        pAudPortConfig->bEnableRequested = OMX_FALSE;
                        pAudPortConfig->sPortDef.bEnabled = OMX_TRUE;
                    }
                }

                break;
           }
       }
       if (i == pAudPortDef->nBufferCountActual)
       {
           MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,
                                    "could not find free buffer");
           return OMX_ErrorUndefined;
       }

    }
    else if (nPortIndex == (OMX_U32) OMX_MUX_INDEX_PORT_VIDEO)
    {
        OMX_PARAM_PORTDEFINITIONTYPE *pVidPortDef =
            &(arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO)->sPortDef;
        OMX_FileMuxPortInfoType *pVidPortConfig =
                             (arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO);

        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_LOW,
                        "client allocated output buffer for component");


        if (nSizeBytes != pVidPortDef->nBufferSize)
        {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,
                        "buffer size does not match our requirements");
            return OMX_ErrorBadParameter;
        }

        if(!pVidPortConfig->pBuffHeaderArray)
        {
            pVidPortConfig->pBuffHeaderArray
                = (OMX_BUFFERHEADERTYPE*)MM_Malloc(
                    sizeof(OMX_BUFFERHEADERTYPE) *
                     pVidPortDef->nBufferCountActual);

            if(!pVidPortConfig->pBuffHeaderArray)
            {
                return OMX_ErrorInsufficientResources;
            }

            OMX_MUX_MEM_SET(
                pVidPortConfig->pBuffHeaderArray,
                0,
                sizeof(OMX_BUFFERHEADERTYPE) *
                pVidPortDef->nBufferCountActual);

            if(pVidPortConfig->pbComponentAllocated)
            {
                MM_Free(pVidPortConfig->pbComponentAllocated);
            }


            pVidPortConfig->pbComponentAllocated
                    = (OMX_BOOL*)MM_Malloc(sizeof(OMX_BOOL) *
                      pVidPortDef->nBufferCountActual);

            if(!pVidPortConfig->pbComponentAllocated)
            {
                return OMX_ErrorInsufficientResources;
            }

            OMX_MUX_MEM_SET(pVidPortConfig->pbComponentAllocated,
                            0, sizeof(OMX_BOOL) *
                            pVidPortDef->nBufferCountActual);

        }
        unsigned int i;
        OMX_BUFFERHEADERTYPE * pVidBuffHeaders =
                               pVidPortConfig->pBuffHeaderArray;

        OMX_BOOL   *bComponentAllocated =
                           pVidPortConfig->pbComponentAllocated;

        for (i = 0; i < pVidPortDef->nBufferCountActual; i++)
        {
            if (pVidBuffHeaders[i].nAllocLen == 0)
            {

                if(pVidPortConfig->nNumBuffAllocated
                  < pVidPortDef->nBufferCountActual)
                {
                    pVidPortConfig->nNumBuffAllocated++;
                }
                pVidPortConfig->bUnPopulated = OMX_FALSE;


                OMX_INIT_STRUCT(&pVidBuffHeaders[i], OMX_BUFFERHEADERTYPE);
                pVidBuffHeaders[i].pBuffer = (OMX_U8*)MM_Malloc(nSizeBytes);
                if(!pVidBuffHeaders[i].pBuffer)
                {
                    return OMX_ErrorInsufficientResources;
                }
                pVidBuffHeaders[i].nAllocLen         = nSizeBytes;
                pVidBuffHeaders[i].pAppPrivate       = pAppPrivate;
                pVidBuffHeaders[i].nInputPortIndex   =
                                 (OMX_U32) OMX_MUX_INDEX_PORT_VIDEO;
                pVidBuffHeaders[i].nOutputPortIndex  =
                                 (OMX_U32) OMX_MUX_INDEX_PORT_NONE;
                pVidBuffHeaders[i].pInputPortPrivate =
                                           (&bComponentAllocated[i]);
                bComponentAllocated[i] = OMX_TRUE;

               *ppBuffer = &pVidBuffHeaders[i];
                if(pVidPortConfig->nNumBuffAllocated
                 == pVidPortDef->nBufferCountActual)
                {
                    pVidPortConfig->bPopulated = OMX_TRUE;

                    if(pVidPortConfig->bEnableRequested)
                    {
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW,
                                  "Port Populated.. Enable Video Port");
                        m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                   OMX_EventCmdComplete,
                                   OMX_CommandPortEnable,
                                   OMX_MUX_INDEX_PORT_VIDEO, NULL);
                        pVidPortConfig->sPortDef.bEnabled = OMX_TRUE;
                        pVidPortConfig->bEnableRequested = OMX_FALSE;
                    }
                }
                break;
            }
        }
        if (i == pVidPortDef->nBufferCountActual)
        {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"could not find free buffer");
            return OMX_ErrorUndefined;
        }

    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"invalid port index");
        return OMX_ErrorBadPortIndex;
    }
    if(m_eState == OMX_StateLoaded && m_eTargetState == OMX_StateIdle)
    {
        if(
          ((arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO)->bPopulated == OMX_TRUE
            || (!IS_VIDEO_PORT_USED))
           &&
          ((arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->bPopulated == OMX_TRUE
            || (!IS_AUDIO_PORT_USED))
          )
        {
            m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                      OMX_EventCmdComplete,
                                      OMX_CommandStateSet,
                                      OMX_StateIdle, NULL);
            m_eState = OMX_StateIdle;
        }
    }
    return OMX_ErrorNone;
}


/*==============================================================================

         FUNCTION:         free_buffer

         DESCRIPTION:
*//**       @brief         Clients requests to free a buffer
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent
                           nPortIndex
                           pBuffer


*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::free_buffer(OMX_IN  OMX_HANDLETYPE hComponent,
                                OMX_IN  OMX_U32 nPortIndex,
                                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    (void) hComponent;
    if (pBuffer == NULL || pBuffer->pBuffer == NULL)
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"free buffer::null param");
        return OMX_ErrorBadParameter;
    }

    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_LOW,"freeing %p", pBuffer->pBuffer);

    if (nPortIndex == OMX_MUX_INDEX_PORT_AUDIO)
    {
        OMX_PARAM_PORTDEFINITIONTYPE *pAudPortDef =
            &(arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->sPortDef;
        OMX_BUFFERHEADERTYPE * pAudBuffHeaders =
            (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->pBuffHeaderArray;
        OMX_FileMuxPortInfoType *pAudPortConfig =
                             (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO);

        OMX_BOOL *pCompAllocated = (OMX_BOOL*)pBuffer->pInputPortPrivate;


        if(!pAudBuffHeaders || !pCompAllocated)
        {
            return OMX_ErrorPortUnpopulated;
        }

        for (unsigned int i = 0; i < pAudPortDef->nBufferCountActual; i++)
        {
            if (pBuffer->pBuffer == pAudBuffHeaders[i].pBuffer)
            {
                if(pAudPortConfig->nNumBuffAllocated)
                {
                    pAudPortConfig->nNumBuffAllocated--;
                }
                if (*pCompAllocated == OMX_TRUE)
                {
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_LOW,
                        "component allocated output buffer, freeing buffer");
                    OMX_FILEMUX_FREEIF(pAudBuffHeaders->pBuffer);
                }
                else
                {
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_LOW,
                        "no need to free output buffer, allocated by client");
                }
                pAudBuffHeaders->nAllocLen = 0;
                pAudPortConfig->bPopulated = OMX_FALSE;

                if(!pAudPortConfig->nNumBuffAllocated)
                {
                    pAudPortConfig->bUnPopulated = OMX_TRUE;


                    if(pAudPortConfig->bDisableRequested)
                    {
                        /**-----------------------------------------------------
                       Once all buffers are released we can disable port
                        --------------------------------------------------------
                        */
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW,
                         "All buffers freed, move AUDIO port to disabled");
                        m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                   OMX_EventCmdComplete,
                                   OMX_CommandPortDisable,
                                   OMX_MUX_INDEX_PORT_AUDIO, NULL);
                        pAudPortConfig->bDisableRequested = OMX_FALSE;
                        pAudPortConfig->sPortDef.bEnabled = OMX_FALSE;
                    }

                    OMX_FILEMUX_FREEIF(pAudPortConfig->pBuffHeaderArray);
                    OMX_FILEMUX_FREEIF(pAudPortConfig->pbComponentAllocated);
                }
                break;
            }
        }
    }
    else if (nPortIndex == OMX_MUX_INDEX_PORT_VIDEO)
    {
        OMX_PARAM_PORTDEFINITIONTYPE *pVidPortDef =
               &(arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO)->sPortDef;
        OMX_BUFFERHEADERTYPE * pVidBuffHeaders =
                (arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO)->pBuffHeaderArray;
        OMX_BOOL *pCompAllocated = (OMX_BOOL*)pBuffer->pInputPortPrivate;
        OMX_FileMuxPortInfoType *pVidPortConfig =
                (arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO);

        if(!pVidBuffHeaders || !pCompAllocated)
        {
            return OMX_ErrorPortUnpopulated;
        }
        for (unsigned int i = 0; i < pVidPortDef->nBufferCountActual; i++)
        {
            if (pBuffer->pBuffer == pVidBuffHeaders[i].pBuffer)
            {
                if(pVidPortConfig->nNumBuffAllocated)
                {
                    pVidPortConfig->nNumBuffAllocated--;
                }
                if (*pCompAllocated)
                {
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_LOW,
                          "component allocated input buffer, freeing buffer");
                    OMX_FILEMUX_FREEIF(pBuffer->pBuffer);
                }
                else
                {
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_LOW,
                        "no need to free input buffer, allocated by client");
                }
                pVidBuffHeaders[i].nAllocLen = 0;
                pVidPortConfig->bPopulated = OMX_FALSE;

                if(!pVidPortConfig->nNumBuffAllocated)
                {
                    pVidPortConfig->bUnPopulated = OMX_TRUE;
                    OMX_FILEMUX_FREEIF(pVidPortConfig->pBuffHeaderArray);
                    OMX_FILEMUX_FREEIF(pVidPortConfig->pbComponentAllocated);

                    if(pVidPortConfig->bDisableRequested)
                    {
                        /**---------------------------------------------------------
                        Once all buffers are released we can disable port
                        ------------------------------------------------------------
                        */
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW,
                          "All buffers freed, move VIDEO port to disabled");
                        m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                   OMX_EventCmdComplete,
                                   OMX_CommandPortDisable,
                                   OMX_MUX_INDEX_PORT_VIDEO, NULL);
                        pVidPortConfig->bDisableRequested = OMX_FALSE;
                        pVidPortConfig->sPortDef.bEnabled = OMX_FALSE;
                    }
                }
                break;
            }
        }
    }

    if(m_eState == OMX_StateIdle && m_eTargetState == OMX_StateLoaded)
    {
        if(((arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO)->bUnPopulated == OMX_TRUE
            || (!IS_VIDEO_PORT_USED))
           &&
          ((arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->bUnPopulated == OMX_TRUE
            || (!IS_AUDIO_PORT_USED))
         )
        {
            m_pCallbacks->EventHandler(m_hSelf, m_pAppData, OMX_EventCmdComplete,
                                      OMX_CommandStateSet, OMX_StateLoaded, NULL);
            m_eState = OMX_StateLoaded;
        }
    }
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         empty_this_buffer

         DESCRIPTION:
*//**       @brief         Client requests to process a buffer
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent
                           pInBuffer

*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:


*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::empty_this_buffer(OMX_IN  OMX_HANDLETYPE hComponent,
                                      OMX_IN  OMX_BUFFERHEADERTYPE* pInBuffer)
{
    if(!hComponent)
    {
        return OMX_ErrorBadParameter;
    }

    if(m_eState != OMX_StateExecuting)
    {
        return OMX_ErrorInvalidState;
    }
    OMX_Mux_BufferCmdType sBufCmd;

    sBufCmd.pBufferHdr = pInBuffer;
    sBufCmd.nPortIndex = pInBuffer->nInputPortIndex;

    return OMX_FileMux_QueueStreamBuffer(&sBufCmd);
}
/*==============================================================================

         FUNCTION:         fill_this_buffer

         DESCRIPTION:
*//**       @brief         Clients request to fill the buffer with o/p data
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent
                           pBuffer

*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::fill_this_buffer(OMX_IN  OMX_HANDLETYPE hComponent,
                                     OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_LOW,"filling buffer...");
    (void) hComponent;
    (void) pBuffer;
    return OMX_ErrorNotImplemented;
}

/*==============================================================================

         FUNCTION:         set_callbacks

         DESCRIPTION:
*//**       @brief         sets callbacks component can use to notify client
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent
                           pCallbacks
                           pAppData   - Client data to be used with callback

*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::set_callbacks(OMX_IN  OMX_HANDLETYPE hComponent,
                                  OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
                                  OMX_IN  OMX_PTR pAppData)
{
    if (pCallbacks == NULL ||
        hComponent == NULL ||
        pCallbacks->EmptyBufferDone == NULL ||
        pCallbacks->EventHandler == NULL)
    {
       return OMX_ErrorBadParameter;
    }

    m_sCallbacks.EmptyBufferDone = pCallbacks->EmptyBufferDone;
    m_sCallbacks.EventHandler = pCallbacks->EventHandler;
    m_sCallbacks.FillBufferDone = pCallbacks->FillBufferDone;

    m_pCallbacks = &m_sCallbacks;
    m_pAppData = pAppData;
    m_hSelf = hComponent;

    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         component_deinit

         DESCRIPTION:
*//**       @brief         deinitializes the component
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent

*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::component_deinit(OMX_IN  OMX_HANDLETYPE hComponent)
{
    (void)hComponent;
    (void)OMX_FileMux_ReleaseResources();
    return OMX_ErrorNone;
}
/*==============================================================================

         FUNCTION:         use_EGL_image

         DESCRIPTION:
*//**       @brief         Use EGL image
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent
                           ppBufferHeader
                           nPortIndex
                           pAppPrivate
                           eglImage

*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:


*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::use_EGL_image(OMX_IN OMX_HANDLETYPE hComponent,
                                  OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                  OMX_IN OMX_U32 nPortIndex,
                                  OMX_IN OMX_PTR pAppPrivate,
                                  OMX_IN void* eglImage)
{
    (void) hComponent;
    (void) ppBufferHdr;
    (void) nPortIndex;
    (void) pAppPrivate;
    (void) eglImage;
    return OMX_ErrorNotImplemented;
}

/*==============================================================================

         FUNCTION:         component_role_enum

         DESCRIPTION:
*//**       @brief         Client can enumerate roles using this
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         hComponent
                           cRole
                           nIndex

*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:


*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::component_role_enum(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_OUT OMX_U8 *cRole,
                                        OMX_IN OMX_U32 nIndex)
{
    (void)hComponent;
    if (nIndex < OMX_MUX_MAX_SUPPORTED_ROLES)
    {
        /* Compare given role to component supported roles */
        if ( cRole )
        {
            OMX_MUX_MEM_COPY(cRole,
                 gRolesSupported[nIndex],OMX_MAX_STRINGNAME_SIZE);
        }
        else
        {
            return OMX_ErrorBadParameter;
        }
    }
    else
    {
        OMX_ErrorNoMore;
    }
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         component_init

         DESCRIPTION:
*//**       @brief         Initializes the component
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         pComponentName

*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:


*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::component_init(OMX_IN OMX_STRING pComponentName)
{

    if(pComponentName && m_pComponentName)
    {
        OMX_MUX_MEM_COPY(m_pComponentName,
                         pComponentName, OMX_MAX_STRINGNAME_SIZE);
    }
    return OMX_ErrorNone;
}


/*==============================================================================

         FUNCTION:         OMX_FileMux_Open

         DESCRIPTION:
*//**       @brief         Creates MMI FileMux instance
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:
                           OMX_ErrorInsufficientResources
                           OMX_ErrorNone

*//*==========================================================================*/
OMX_U32 OMX_FileMux::OMX_FileMux_Open(OMX_HANDLETYPE *pFd)
{
    class OMX_FileMux *p_mmi_file_mux = new OMX_FileMux();

    *pFd = (OMX_HANDLETYPE) p_mmi_file_mux;

    if(!*pFd)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL, "OMX_FileMux Create Failed!!!");
        return OMX_ErrorInsufficientResources;
    }
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux Open Called!!!");
    return OMX_ErrorNone;
}


/*==============================================================================

         FUNCTION:         OMX_FileMux

         DESCRIPTION:
*//**       @brief         MMI FileMux interface constructor
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_FileMux::OMX_FileMux()
{

   /**--------------------------------------------------------------------------
            Initialize the File MUX MMI interface
    ----------------------------------------------------------------------------
    */

    bStatus = OMX_TRUE;
   /**--------------------------------------------------------------------------
    *      Create the array of port parameters.
    *---------------------------------------------------------------------------
    */
    arrPortConfig =
        (OMX_FileMuxPortInfoType*)
                   OMX_FILEMUX_MALLOC
                             (((NUM_AUDIO_PORT +
                                 NUM_VIDEO_PORT + NUM_TEXT_PORT) *
                                 sizeof(OMX_FileMuxPortInfoType)));
    if(!arrPortConfig)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                    "Malloc failed while instantiating MMIFileMux!!!");
        bStatus = OMX_FALSE;
        return;
    }
    /**-------------------------------------------------------------------------
    *    Do zero initialization
    *---------------------------------------------------------------------------
    */
    OMX_MUX_MEM_SET
          (arrPortConfig, 0, (NUM_AUDIO_PORT + NUM_VIDEO_PORT + NUM_TEXT_PORT) *
                                               sizeof(OMX_FileMuxPortInfoType));

   /**--------------------------------------------------------------------------
       Init Audio ports. Remember someof these are default settings, will be
       modified by client before enabling

       Another important fact to be known is that currently there is no use
       case that uses multiple ports of each type, when there is one there
       can be multiple streams of same type, they will have same paramaters.
    *---------------------------------------------------------------------------
    */

    for(int i = 0; i < NUM_AUDIO_PORT; i++)
    {
        (arrPortConfig + i)->sPortDef.nSize =
                                     sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
        (arrPortConfig + i)->nPortNumber = i;
//        (arrPortConfig + i)->nVersion =
        (arrPortConfig + i)->sPortDef.bBuffersContiguous = OMX_FALSE;
        (arrPortConfig + i)->sPortDef.bEnabled = OMX_TRUE;
        (arrPortConfig + i)->sPortDef.bPopulated = OMX_FALSE;
        (arrPortConfig + i)->sPortDef.eDir = OMX_DirInput;
        (arrPortConfig + i)->sPortDef.eDomain = OMX_PortDomainAudio;
        (arrPortConfig + i)->sPortDef.format.audio.bFlagErrorConcealment =
                                                   OMX_FALSE;
        (arrPortConfig + i)->sPortDef.format.audio.cMIMEType = NULL;
        (arrPortConfig + i)->sPortDef.format.audio.eEncoding =
                                                    OMX_AUDIO_CodingAMR;
        (arrPortConfig + i)->sPortDef.format.audio.pNativeRender = 0;
        (arrPortConfig + i)->sPortDef.nBufferAlignment = 0;
        (arrPortConfig + i)->sPortDef.nBufferCountActual = NUM_AUDIO_BUFFERS;
        (arrPortConfig + i)->sPortDef.nBufferCountMin = NUM_AUDIO_BUFFERS;
        (arrPortConfig + i)->sPortDef.nBufferSize = DEFAULT_AUDIO_BUFFER_SIZE;
        (arrPortConfig + i)->sPortDef.nPortIndex = i;

        (arrPortConfig + i)->sFormatSpecificInfo.sAMRWBInfo = sAMRDefaults;
        (arrPortConfig + i)->sFormatSpecificInfo.sAMRWBInfo.nPortIndex = i;
        qmm_ListInit(&(arrPortConfig + i)->pBufferQueue);

        (arrPortConfig + i)->pSyntaxHdr = NULL;
        (arrPortConfig + i)->nSyntaxHeaderLen = 0;

        (arrPortConfig + i)->pBuffHeaderArray = NULL;
        (arrPortConfig + i)->pbComponentAllocated = NULL;
        (arrPortConfig + i)->bPopulated = OMX_FALSE;
        (arrPortConfig + i)->bUnPopulated = OMX_TRUE;
    }

   /*---------------------------------------------------------------------------
        Init Video ports. Remember some of these are default settings, will be
         modified by client
    ----------------------------------------------------------------------------
    */
    for(int i = NUM_AUDIO_PORT;
                   i < (NUM_VIDEO_PORT + NUM_AUDIO_PORT); i++)
    {
        (arrPortConfig + i)->sPortDef.nSize =
                                     sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
        (arrPortConfig + i)->nPortNumber = i;
        (arrPortConfig + i)->sPortDef.bBuffersContiguous = OMX_FALSE;
        (arrPortConfig + i)->sPortDef.bEnabled = OMX_TRUE;
        (arrPortConfig + i)->sPortDef.bPopulated = OMX_FALSE;
        (arrPortConfig + i)->sPortDef.eDir = OMX_DirInput;
        (arrPortConfig + i)->sPortDef.eDomain = OMX_PortDomainVideo;
        (arrPortConfig + i)->sPortDef.format.video.nBitrate = 384000;
        (arrPortConfig + i)->sPortDef.format.video.bFlagErrorConcealment =
                                             OMX_FALSE;
        (arrPortConfig + i)->sPortDef.format.video.cMIMEType = NULL;
        (arrPortConfig + i)->sPortDef.format.video.eColorFormat =
                                             OMX_COLOR_FormatUnused;
        (arrPortConfig + i)->sPortDef.format.video.eCompressionFormat =
                                             OMX_VIDEO_CodingMPEG4;
        (arrPortConfig + i)->sPortDef.nBufferAlignment = 0;
        (arrPortConfig + i)->sPortDef.nBufferCountActual = NUM_VIDEO_BUFFERS;
        (arrPortConfig + i)->sPortDef.nBufferCountMin = NUM_VIDEO_BUFFERS;
        (arrPortConfig + i)->sPortDef.nBufferSize = DEFAULT_VIDEO_BUFFER_SIZE;
        (arrPortConfig + i)->sPortDef.nPortIndex = i;


        (arrPortConfig + i)->sFormatSpecificInfo.sMPEG4Info = sMPEG4Defaults;
        (arrPortConfig + i)->sFormatSpecificInfo.sMPEG4Info.nPortIndex = i;

        (arrPortConfig + i)->pSyntaxHdr = NULL;
        (arrPortConfig + i)->nSyntaxHeaderLen = 0;

        (arrPortConfig + i)->pBuffHeaderArray = NULL;
        (arrPortConfig + i)->pbComponentAllocated = NULL;
        (arrPortConfig + i)->bPopulated = OMX_FALSE;
        (arrPortConfig + i)->bUnPopulated = OMX_TRUE;
    }
   /*---------------------------------------------------------------------------
         Init Text ports. Remember Someof these are default settings, will be
         modified by client
    ----------------------------------------------------------------------------
    */
    for(int i = NUM_VIDEO_PORT + NUM_AUDIO_PORT;
                   i < (NUM_VIDEO_PORT + NUM_AUDIO_PORT + NUM_TEXT_PORT); i++)
    {
        (arrPortConfig + i)->sPortDef.nSize =
                                     sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
        (arrPortConfig + i)->nPortNumber = i;
        (arrPortConfig + i)->sPortDef.bBuffersContiguous = OMX_FALSE;
        (arrPortConfig + i)->sPortDef.bEnabled = OMX_FALSE;
        (arrPortConfig + i)->sPortDef.bPopulated = OMX_FALSE;
        (arrPortConfig + i)->sPortDef.eDir = OMX_DirInput;
        (arrPortConfig + i)->sPortDef.eDomain = OMX_PortDomainOther;
        (arrPortConfig + i)->sPortDef.format.other.eFormat =
                                             OMX_OTHER_FormatVendorReserved;
        (arrPortConfig + i)->sPortDef.nBufferAlignment = 0;
        (arrPortConfig + i)->sPortDef.nBufferCountActual = NUM_TEXT_BUFFERS;
        (arrPortConfig + i)->sPortDef.nBufferCountMin = NUM_TEXT_BUFFERS;
        (arrPortConfig + i)->sPortDef.nBufferSize = DEFAULT_TEXT_BUFFER_SIZE;
        (arrPortConfig + i)->sPortDef.nPortIndex = i;


        (arrPortConfig + i)->pBuffHeaderArray = NULL;
        (arrPortConfig + i)->pbComponentAllocated = NULL;
        (arrPortConfig + i)->bPopulated = OMX_FALSE;
        (arrPortConfig + i)->bUnPopulated = OMX_TRUE;

        (arrPortConfig + i)->pSyntaxHdr = NULL;
        (arrPortConfig + i)->nSyntaxHeaderLen = 0;
    }
    /**-------------------------------------------------------------------------
        Initialize the default format as MP4 and default brand as 3gp
    ----------------------------------------------------------------------------
    */
    m_eState = OMX_StateLoaded;
    m_eTargetState = OMX_StateLoaded;
    m_pComponentName = (OMX_STRING)OMX_FILEMUX_MALLOC(OMX_MAX_STRINGNAME_SIZE);

    if(m_pComponentName)
    {
        OMX_FileMux_CopyString(m_pComponentName,"OMX.qcom.file.muxer",
                                                       OMX_MAX_STRINGNAME_SIZE);
    }

    nFileFormat = MUX_FMT_MP4;

    nFileBrand = QOMX_FORMAT_3G2;

 //   OMX_FileMux_InitMediaInfo();

    /**-------------------------------------------------------------------------
      Temporary buffer header pointer to hold one buffer delay buff header
    ----------------------------------------------------------------------------
    */
    pTmpVidBufferHdr = NULL;
   /**--------------------------------------------------------------------------
         Disable reordering by default
    ----------------------------------------------------------------------------
    */
    bReorder = OMX_FALSE;

   /**--------------------------------------------------------------------------
          Fragment duration for Fragmented files
    ----------------------------------------------------------------------------
    */
    nFragmentDuration = 0;

   /**--------------------------------------------------------------------------
          Audio video interlace period
    ----------------------------------------------------------------------------
    */
    nAVInterlacePeriod = DEAFULT_INTERLACE_PERIOD_MS;

    /**-------------------------------------------------------------------------
           structures for initializing FileMux
    ----------------------------------------------------------------------------
    */
    pFileMuxParams = NULL;

    /**-------------------------------------------------------------------------
           structures for initializing FileMux
    ----------------------------------------------------------------------------
    */
    pFileMuxStreams = NULL;

    /**-------------------------------------------------------------------------
        Lets Initialize rest of the class variables here
    ----------------------------------------------------------------------------
    */
    for(int i = 0; i < OMX_MUX_MAX_STREAMS; i++)
    {
        sStreamInfo[i].nCurrDelta = 0;
        sStreamInfo[i].nPrevDelta = 0;
        sStreamInfo[i].nPrevTimeStamp = 0;
        sStreamInfo[i].nStartTime = 0;
    }

    nContentURISize = 0;

    pContentURI = NULL;

    pIStreamPort = NULL;

    bMuxOpen = OMX_FALSE;
    pFileMux = NULL;

    nStatisticsInterval = 0;
    nFileDurationLimit = 0;
    nFileSizeLimit = 0;
    pCSHandle  = NULL;

    OMX_MUX_MEM_SET(&m_HDCPInfo, 0, sizeof(m_HDCPInfo));

    OMX_MUX_MEM_SET(&m_FmtPvtInfo, 0, sizeof(m_FmtPvtInfo));

    OMX_MUX_MEM_SET(&nEncryptTypeConfigParameters, 0, sizeof(QOMX_ENCRYPTIONTYPE));

    bStatus = (OMX_BOOL)MM_CriticalSection_Create(&pCSHandle);
}



/*==============================================================================

         FUNCTION:         ~OMX_FileMux

         DESCRIPTION:
*//**       @brief         FileMux MMI interface destructor
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_FileMux::~OMX_FileMux()
{
    (void)OMX_FileMux_ReleaseResources();
    (void)MM_CriticalSection_Release(pCSHandle);

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "OMX MUX destructor");
    for(int i = 0; i < (NUM_VIDEO_PORT + NUM_AUDIO_PORT + NUM_TEXT_PORT); i++)
    {
        OMX_FILEMUX_FREEIF((arrPortConfig + i)->pSyntaxHdr);
        (arrPortConfig + i)->nSyntaxHeaderLen = 0;

        OMX_FILEMUX_FREEIF((arrPortConfig + i)->pBuffHeaderArray);
        OMX_FILEMUX_FREEIF((arrPortConfig + i)->pbComponentAllocated);
    }
 //   OMX_FileMux_DeInitMediaInfo();
    OMX_FILEMUX_FREEIF(m_pComponentName);
    OMX_FILEMUX_FREEIF(arrPortConfig);
    OMX_FILEMUX_FREEIF(pContentURI);
}

#if 0
/*==============================================================================

         FUNCTION:         OMX_FileMux_RegEvtHndlr

         DESCRIPTION:
*//**       @brief         Registers base class's event handler with MMI
                           interface
*//**

@par     DEPENDENCIES:
                           This function can be called only after all set
                           parameters have been successfully performed.
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ErrorNone
                           OMX_ErrorBadParameter


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_U32 OMX_FileMux::OMX_FileMux_RegEvtHndlr
(
    OMX_HANDLETYPE             hFd,
    MMI_CmpntEvtHandlerType    pfnEvtHdlr,
    OMX_PTR                    pClientData
)
{
    OMX_FileMux*  pMMIFileMux = (OMX_FileMux *)hFd;

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux RegEvtHndlr Called!!!");

    if(!hFd || !pfnEvtHdlr)
    {
        return OMX_ErrorBadParameter;
    }
   /**--------------------------------------------------------------------------
    *      Keep a copy of callback details to notify client of events
    *---------------------------------------------------------------------------
    */
    pMMIFileMux->sClientEvtNofity.pfnEvtHdlr = pfnEvtHdlr;

    pMMIFileMux->sClientEvtNofity.pClientData = pClientData;

    return OMX_ErrorNone;
}
#endif


/*==============================================================================

         FUNCTION:         OMX_FileMux_Close

         DESCRIPTION:
*//**       @brief         Closes the MMI layer instance for  file mux
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ErrorNone


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_U32 OMX_FileMux::OMX_FileMux_Close(OMX_HANDLETYPE hFd)
{
    OMX_FileMux*  pMMIFileMux = (OMX_FileMux *)hFd;

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux Close Called!!!");

    if(!hFd)
    {
        return OMX_ErrorBadParameter;
    }

    delete (pMMIFileMux);
   /**--------------------------------------------------------------------------
    *      Release all memory here TBD
    *---------------------------------------------------------------------------
    */
    return OMX_ErrorNone;
}


/*==============================================================================

         FUNCTION:         OMX_FileMux_Command

         DESCRIPTION:
*//**       @brief         This is the entry function for processing all
                           commands from base class layer.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ErrorNone
                           OMX_ErrorNone
                           other appropriate errors


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_Command
(
    OMX_HANDLETYPE hFd,
    OMX_U32 nCode,
    OMX_PTR pData
)
{

    OMX_FileMux*  pMMIFileMux;

    if(!hFd)
    {
        return OMX_ErrorBadParameter;
    }

    pMMIFileMux = this;

    switch(nCode)
    {
        /**---------------------------------------------------------------------
        *   Lets use same function for set and get to save on code.
        *-----------------------------------------------------------------------
        */
   /*     case OMX_MUX_CMD_GET_CUSTOM_PARAM:
        case OMX_MUX_CMD_SET_CUSTOM_PARAM:
        {
            MMI_CustomParamCmdType *pPortParam =
                        (MMI_CustomParamCmdType *) pData;
            return pMMIFileMux->OMX_FileMux_GetSetCustomParam(
                                                nCode,
                                                pPortParam->nParamIndex,
                                                pPortParam->pParamStruct);
        }*/

      /*  case OMX_MUX_CMD_GET_STD_OMX_PARAM:
        case OMX_MUX_CMD_SET_STD_OMX_PARAM:
        {
            MMI_OmxParamCmdType *pCmd = (MMI_OmxParamCmdType *)pData;

            return pMMIFileMux->OMX_FileMux_GetSetParam(nCode,
                                           pCmd->nParamIndex,
                                           pCmd->pParamStruct);
        }

        case OMX_MUX_CMD_GET_EXTENSION_INDEX:
        {
            MMI_GetExtensionCmdType    *getExtCmd
                                      = (MMI_GetExtensionCmdType*)pData;
            return pMMIFileMux->OMX_FileMux_GetExtensionIndex(
                                                             getExtCmd);
        }
*/
        case OMX_MUX_CMD_ALLOC_BUFFER:
        {
         //   return pMMIFileMux->OMX_FileMux_AllocBuffer(
          //                                (MMI_AllocBufferCmdType *)pData);
        }

        case OMX_MUX_CMD_FREE_BUFFER:
        {
         //   return pMMIFileMux->OMX_FileMux_FreeBuffer(
           //                                (MMI_FreeBufferCmdType *)pData);
        }

        case OMX_MUX_CMD_USE_BUFFER:
        {
       /*     return pMMIFileMux->OMX_FileMux_UseBuffer(
                                            (MMI_UseBufferCmdType *)pData);*/
        }

        case OMX_MUX_CMD_ENABLE_PORT:
        {
//            MMI_PortCmdType *pCmd = (MMI_PortCmdType*)pData;
//            return  pMMIFileMux->OMX_FileMux_EnablePort(pCmd->nPortIndex);
        }

        case OMX_MUX_CMD_DISABLE_PORT:
        {
//          MMI_PortCmdType *pCmd = (MMI_PortCmdType*)pData;
//            return pMMIFileMux->OMX_FileMux_DisablePort(pCmd->nPortIndex);
        }

        case OMX_MUX_CMD_START:
        {
            return pMMIFileMux->OMX_FileMux_Start();
        }

        case OMX_MUX_CMD_STOP:
        {
            return pMMIFileMux->OMX_FileMux_Stop();
        }

        case OMX_MUX_CMD_PAUSE:
        {
            return pMMIFileMux->OMX_FileMux_Pause();
        }

        case OMX_MUX_CMD_RESUME:
        {
            return pMMIFileMux->OMX_FileMux_Resume();
        }

        case OMX_MUX_CMD_EMPTY_THIS_BUFFER:
        {
            return pMMIFileMux->OMX_FileMux_QueueStreamBuffer(
                                             (OMX_Mux_BufferCmdType*)pData);
        }

        case OMX_MUX_CMD_FILL_THIS_BUFFER:
        {

        }

        case OMX_MUX_CMD_FLUSH:
        {
			//           return pMMIFileMux->OMX_FileMux_Flush(
            //                         ((MMI_PortCmdType*)pData)->nPortIndex);
        }

        case OMX_MUX_CMD_LOAD_RESOURCES:
        {
            return pMMIFileMux->OMX_FileMux_LoadResources();
        }

        case OMX_MUX_CMD_RELEASE_RESOURCES:
        {
            return pMMIFileMux->OMX_FileMux_ReleaseResources();
        }
        /**---------------------------------------------------------------------
            Nothing much to do here.
        ------------------------------------------------------------------------
        */
        case OMX_MUX_CMD_WAIT_FOR_RESOURCES:
        case OMX_MUX_CMD_RELEASE_WAIT_ON_RESOURCES:
            return OMX_ErrorNone;
    }
    return OMX_ErrorUnsupportedIndex;
}


/*==============================================================================

         FUNCTION:         OMX_FileMux_Pause

         DESCRIPTION:
*//**       @brief         Pauses processing in the File Mux layer.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ErrorNone on success
                           OMX_ErrorInvalidState on failure


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_Pause()
{
    MUX_STATUS eStatus;

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux Pause Called!!!");

    if(!bMuxOpen)
    {
        /**---------------------------------------------------------------------
             We havent started processing yet. return COMPLETE
        ------------------------------------------------------------------------
        */
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                    "Pause received before MMIFileMuxStart!!!");
        m_eState = OMX_StatePause;
        return OMX_ErrorNone;
    }
    else
    {
        /**---------------------------------------------------------------------
             We have to dequeue any buffers present in any queue
        ------------------------------------------------------------------------
        */
        OMX_FileMux_DeQueueAudioBuffers(OMX_TRUE, OMX_MUX_INDEX_PORT_AUDIO);
        eStatus = pFileMux->MUX_pause_Processing(this);

        if(eStatus == MUX_SUCCESS)
        {
            return OMX_ErrorNone;
        }
        else if(eStatus == MUX_DONE)
        {
            m_eState = OMX_StatePause;
            return OMX_ErrorNone;
        }

    }
    return OMX_ErrorInvalidState;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_Resume

         DESCRIPTION:
*//**       @brief         Resume processing in the File Mux layer.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ErrorNone on success
                           OMX_ErrorInvalidState on failure


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_Resume()
{

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux Resume Called!!!");

    for(int i = 0; i < OMX_MUX_MAX_STREAMS; i++)
    {
        sStreamInfo[i].nCurrDelta = 0;
        sStreamInfo[i].nPrevDelta = 0;
        sStreamInfo[i].nPrevTimeStamp = 0;
        sStreamInfo[i].nStartTime = 0;
    }
    /**-------------------------------------------------------------------------
         Reset AV SYnc Parameters
    ----------------------------------------------------------------------------
    */
    sAVSyncInfo.bAVSyncDone = OMX_FALSE;
    sAVSyncInfo.nAVTimeDiff = 0;

    if(!sAVSyncInfo.bAudioStreamEnded ||
       !sAVSyncInfo.bVideoStreamEnded)
    {
        /**---------------------------------------------------------------------
           Once Audio and Video streams ends while pause we can find any
           difference in audio and video stream duration
        ------------------------------------------------------------------------
        */
        if(sAVSyncInfo.nCurrentAudioTime !=
               sAVSyncInfo.nCurrentVideoTime)
        {
            sAVSyncInfo.nAVTimeDiffAdjust
                         = ((OMX_S64)sAVSyncInfo.nCurrentVideoTime -
                            (OMX_S64)sAVSyncInfo.nCurrentAudioTime);

            MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM,
              "MMi Mux Pause. total Time Video = %ld, Audio = %ld, Diff = %ld",
              (OMX_S32)sAVSyncInfo.nCurrentVideoTime,
              (OMX_S32)sAVSyncInfo.nCurrentAudioTime,
              (OMX_S32)sAVSyncInfo.nAVTimeDiffAdjust);

        }
        sAVSyncInfo.bVideoStarted = OMX_FALSE;
        sAVSyncInfo.bAudioStarted = OMX_FALSE;
    }
    sAVSyncInfo.bAudioStreamEnded = OMX_FALSE;
    sAVSyncInfo.bVideoStreamEnded = OMX_FALSE;


    if(!pFileMux || !bMuxOpen ||
       pFileMux->MUX_resume_Processing(this) == MUX_SUCCESS)
    {
        return OMX_ErrorNone;
    }
    return OMX_ErrorUndefined;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_CloseFileMux

         DESCRIPTION:
*//**       @brief         Closes the file Mux instance.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_CloseFileMux()
{
    MM_CriticalSection_Enter(pCSHandle);
    if(!pFileMux || !bMuxOpen)
    {
        /**--------------------------------------------------------------------
         We are being told to stop without start or after another stop
         lets return OMX_ErrorNone.
        -----------------------------------------------------------------------
        */
        MM_CriticalSection_Leave(pCSHandle);
        return OMX_ErrorNone;
    }

    /**-------------------------------------------------------------------------
        Send any buffers that we have cached in this layer to FileMux.
    ----------------------------------------------------------------------------
    */
    OMX_FileMux_DeQueueAudioBuffers(OMX_TRUE, OMX_MUX_INDEX_PORT_AUDIO);

    /**-------------------------------------------------------------------------
     Write all the meta data that has been accumulated over the session
     before closing the session
    ----------------------------------------------------------------------------
    */
//    OMX_FileMux_WriteMediaInfoAll();

    pFileMux->MUX_end_Processing(this);
    MM_CriticalSection_Leave(pCSHandle);
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux Close(Stop) Called!!!");
    return OMX_ErrorNone;
}


/*==============================================================================

         FUNCTION:         OMX_FileMux_Start

         DESCRIPTION:
*//**       @brief         This function makes all resources ready for execution
                           . In our case we need to make File Mux layer ready
                           for processing.
*//**

@par     DEPENDENCIES:
                           This function can be called only after all set
                           parameters have been successfully performed.
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_Start()
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux Start Called!!!");

    if(pFileMux)
    {
        /**--------------------------------------------------------------------
          This is the instance of a previous session. Lets delete
           it and create new.
        -----------------------------------------------------------------------
        */
        delete pFileMux;
        pFileMux = NULL;
    }

    /**-------------------------------------------------------------------------
        There is no resource to acquire for FileMux, lets validate if we have
        all information to START and EXECUTE
    ----------------------------------------------------------------------------
    */

    if(!OMX_FileMux_PopulateFileMuxParams())
    {
        return OMX_ErrorUndefined;
    }


    /**-------------------------------------------------------------------------
       Initialize stream info structures which are session specific.
    ----------------------------------------------------------------------------
    */
    OMX_MUX_MEM_SET(&sStreamInfo,0, sizeof(OMX_FileMux_StreamInfoType) *
                                                          OMX_MUX_MAX_STREAMS);

    /**-------------------------------------------------------------------------
       Initialize session specific params in statistics to 0
    ----------------------------------------------------------------------------
    */

    sMuxStats.nAudioBytes    = 0;
    sMuxStats.nAudioDuration = 0;
    sMuxStats.nNumAudFramesDropped = 0;
    sMuxStats.nNumAudFramesWritten = 0;
    sMuxStats.nNumVidFramesDropped = 0;
    sMuxStats.nNumVidFramesWritten = 0;
    sMuxStats.nSilentFramesInserted = 0;
    sMuxStats.nVideoBytes  = 0;
    sMuxStats.nVideoDuration = 0;

    /**-------------------------------------------------------------------------
       Initialize session specific AV sync params
    ----------------------------------------------------------------------------
    */
    sAVSyncInfo.nVideoStartTime = 0;
    sAVSyncInfo.nAudioStartTime = 0;
    sAVSyncInfo.bAVSyncDone     = OMX_FALSE;
    sAVSyncInfo.nAVTimeDiff     = 0;
    sAVSyncInfo.nAVTimeDiffAdjust = 0;
    sAVSyncInfo.bVideoStarted = sAVSyncInfo.bAudioStarted = OMX_FALSE;
    sAVSyncInfo.bVideoStreamEnded = OMX_FALSE;
    sAVSyncInfo.bAudioStreamEnded = OMX_FALSE;
    sAVSyncInfo.nCurrentAudioTime = sAVSyncInfo.nCurrentVideoTime = 0;

    /**-------------------------------------------------------------------------
       Initialize video 1-delay buffer to NULL
    ----------------------------------------------------------------------------
    */
    pTmpVidBufferHdr = NULL;
    /**-------------------------------------------------------------------------
       Do AVSync only if both audio and video ports are active
    ----------------------------------------------------------------------------
    */
    if(!IS_VIDEO_PORT_USED || !IS_AUDIO_PORT_USED ||
       (nFileFormat != MUX_FMT_MP4))  // && nFileFormat != MUX_FMT_MP2))
    {
        sAVSyncInfo.bAVSyncDone = OMX_TRUE;
		 MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                    "AvsyncDone setting TRUE by default ");
    }

    /**-------------------------------------------------------------------------
      Reset flag which determines if mux output is reached.
    ----------------------------------------------------------------------------
    */
    bOutputLimitReached = OMX_FALSE;

    bUUIDWritten = (nFileFormat == MUX_FMT_MP4) ? OMX_FALSE: OMX_TRUE;

    return OMX_FileMux_CreateFileMux();
}



/*==============================================================================

         FUNCTION:         OMX_FileMux_Stop

         DESCRIPTION:
*//**       @brief         Stop processing. The settings should be retained.
                           After giving another start another execution session
                           should be possible.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_Stop()
{

    /*TBD*/
    return OMX_FileMux_CloseFileMux();
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_Flush

         DESCRIPTION:
*//**       @brief         Flushes the buffers waiting to be processed on a
                           particular port. Return unused buffers back to client
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

            nPortIndex[in] : Port on which flush should be done


*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_Flush
(
    OMX_U32  nPortIndex
)
{
    MUX_STATUS nStatus = MUX_SUCCESS;

    if(!pFileMux)
    {
        return OMX_ErrorUndefined;
    }

    /**-------------------------------------------------------------------------
    *   This flushes all queues nothing port specific, TBD.
    *---------------------------------------------------------------------------
    */
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                 "Flush command received MMIFileMux!!!");

    /**-------------------------------------------------------------------------
        First queue any audio buffer we have held for AVSync. Then issue flush
    ----------------------------------------------------------------------------
    */
    OMX_FileMux_DeQueueAudioBuffers(OMX_TRUE, OMX_MUX_INDEX_PORT_AUDIO);


    if(nPortIndex == OMX_ALL)
    {
        pFileMux->MUX_Flush();
    }
    else
    {
        nStatus = pFileMux->MUX_Flush((uint32)nPortIndex);
    }
    if(nStatus == MUX_DONE)
    {
         m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                   OMX_EventCmdComplete,
                                   OMX_CommandFlush,
                                   nPortIndex, NULL);
        return OMX_ErrorNone;
    }
    else if(nStatus == MUX_SUCCESS)
    {
        return OMX_ErrorNone;
    }
    return OMX_ErrorUndefined;


}



/*==============================================================================

         FUNCTION:         OMX_FileMux_LoadResources

         DESCRIPTION:
*//**       @brief         All resources required for processing are allocated
                           in this call.
*//**

@par     DEPENDENCIES:
                           This function can be called only after all set
                           parameters have been successfully performed.
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           MMI_S_COMLPETE or error codes


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_LoadResources
(
    void
)
{
    int64 freeSpace = 0;
    char* pCharTempName = (char*)MM_Malloc(FS_FILENAME_MAX_LENGTH_P);

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux Load Res. Called!!!");
    /**-------------------------------------------------------------------------
           Allocate structures for initializing FileMux
    ----------------------------------------------------------------------------
    */
    pFileMuxParams = OMX_FILEMUX_MALLOC(sizeof(MUX_create_params_type));

    if(!pFileMuxParams)
    {
         if(pCharTempName)
         {
              MM_Free(pCharTempName);
              pCharTempName = NULL;
         }
         return OMX_ErrorInsufficientResources;
    }

    OMX_MUX_MEM_SET(pFileMuxParams, 0,sizeof(MUX_create_params_type));

    OMX_MUX_MEM_SET(&sMuxStats, 0 , sizeof(OMX_FileMuxStatistics));
    /**-------------------------------------------------------------------------
           Allocate structures for initializing FileMux
    ----------------------------------------------------------------------------
    */
    pFileMuxStreams = OMX_FILEMUX_MALLOC(sizeof(MUX_stream_create_params_type) *
                                                           OMX_MUX_MAX_STREAMS);
    if(!pFileMuxStreams)
    {
        if(pCharTempName)
        {
             MM_Free(pCharTempName);
             pCharTempName = NULL;
        }
        return OMX_ErrorInsufficientResources;
    }

    OMX_MUX_MEM_SET(pFileMuxStreams, 0,sizeof(MUX_stream_create_params_type)*
                                                           OMX_MUX_MAX_STREAMS);

    /**-------------------------------------------------------------------------
      Lets validate and pass content URI to FileMux, all params should be
      available now.
    ----------------------------------------------------------------------------
    */
    if(nContentURISize > (FS_FILENAME_MAX_LENGTH_P))
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                    "Filename Longer than max length!!!");
        if(pCharTempName)
        {
            MM_Free(pCharTempName);
            pCharTempName = NULL;
        }
        return OMX_ErrorInsufficientResources;
    }
    else if (0 == nContentURISize)
    {
        /**---------------------------------------------------------------------
           No movie name selected lets try
        ------------------------------------------------------------------------
        */

#ifdef CONF_TEST
        pContentURI = (OMX_U16*)OMX_FILEMUX_MALLOC(FS_FILENAME_MAX_LENGTH_P*2);
        if(!pContentURI)
        {
            if(pCharTempName)
            {
                MM_Free(pCharTempName);
                pCharTempName = NULL;
            }
            return OMX_ErrorInsufficientResources;
        }
        nContentURISize = FS_FILENAME_MAX_LENGTH_P;

        OMX_FileMux_CopyString(pContentURI, DEFAULT_FILE_NAME,
                                                      FS_FILENAME_MAX_LENGTH_P);
#else
        return OMX_ErrorUndefined;
#endif

    }

 /* This change is introduced temporarily in order to initialize
  * file system while loading resources so that it doesn’t take
  * longer time during open files( which in turn causes AV sync issue )
  */
    OMX_FileMux_CopyString (pCharTempName,pContentURI,
                                                    FS_FILENAME_MAX_LENGTH_P);
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                    "Calling MM_File_GetFreeSpace in OMX_FileMux_LoadResources: start ");

     (void)MM_File_GetFreeSpace(pCharTempName, (uint64*)(&freeSpace));

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                    "Calling MM_File_GetFreeSpace in OMX_FileMux_LoadResources: Completed ");

    if(pCharTempName)
    {
        MM_Free(pCharTempName);
        pCharTempName = NULL;
    }
    return OMX_ErrorNone;
}


/*==============================================================================

         FUNCTION:         OMX_FileMux_ReleaseResources

         DESCRIPTION:
*//**       @brief         All resources required for processing are released
                           in this call.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ErrorNone or error codes


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_ReleaseResources
(
    void
)
{

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux Release Res. Called!!!");
    /**-------------------------------------------------------------------------
           Free structures for initializing FileMux
    ----------------------------------------------------------------------------
    */
    OMX_FILEMUX_FREEIF(pFileMuxParams);

    /**-------------------------------------------------------------------------
           Free structures for initializing FileMux
    ----------------------------------------------------------------------------
    */
    OMX_FILEMUX_FREEIF(pFileMuxStreams);

    /**-------------------------------------------------------------------------
         If component was not stopped we need to throw error here.
    ----------------------------------------------------------------------------
    */
    if(pFileMux)
    {
        delete pFileMux;
        pFileMux = NULL;
    }

    pIStreamPort = NULL;

    return OMX_ErrorNone;

}


/*==============================================================================

         FUNCTION:         OMX_FileMux_CreateFileMux

         DESCRIPTION:
*//**       @brief         This function creates the lower fileMux layer.
*//**

@par     DEPENDENCIES:
                           This function can be called only after all set
                           parameters have been successfully performed.
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ErrorNone or appropriate error code


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_CreateFileMux
(
    void
)
{
    MUX_handle_type sFileHandle;

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux Create Filemux!!!");
    /**------------------------------------------------------------------------
        See if ISource port is set. If yes use it otherwise use filename
    ---------------------------------------------------------------------------
    */
    if(pIStreamPort != NULL)
    {
        sFileHandle.method = MUX_METHOD_PORT;
        sFileHandle.OputPort.method = MUX_METHOD_PORT;
        sFileHandle.OputPort.pOputStream = pIStreamPort;
        sFileHandle.OputPort.client_data = NULL;
    }
    else
    {
        sFileHandle.efs.method = MUX_METHOD_EFS;

        OMX_FileMux_CopyString(sFileHandle.efs.filename,
                               pContentURI,
                               (int)nContentURISize);
    }
    pFileMux = new FileMux((MUX_create_params_type *)pFileMuxParams,
                           (MUX_fmt_type)nFileFormat,
                           (MUX_brand_type)
                           OMX_FileMux_ConvertOMXBrandToFileMuxBrand(),
                           &sFileHandle,
                           FALSE,
                           &OMX_FileMux_DriverCallback,
                           (void*)this);

    if(NULL == pFileMux || MUX_SUCCESS != pFileMux->MUX_get_Status())
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                    "Creating FileMux instance Failed!!!");
        return OMX_ErrorUndefined;
    }
    bMuxOpen = OMX_TRUE;

    if(0 != nStatisticsInterval)
    {
      pFileMux->MUX_set_StatisticsInterval(nStatisticsInterval);
    }
    return OMX_ErrorNone;


}


/*==============================================================================

         FUNCTION:         OMX_FileMux_EnablePort

         DESCRIPTION:
*//**       @brief         A particular port is enabled by this call. This
                           port needs to be populated for the component to
                           start, once enabled.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_EnablePort(OMX_U32 nPortIndex)
{

    if(nPortIndex == OMX_ALL)
    {
        if((arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO)->bPopulated)
        {
            (arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO)->sPortDef.bEnabled
                      = OMX_TRUE;
             m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                   OMX_EventCmdComplete,
                                   OMX_CommandPortEnable,
                                   OMX_MUX_INDEX_PORT_VIDEO, NULL);
        }
        else
        {
            (arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO)
                                     ->bEnableRequested = OMX_TRUE;
        }

        if((arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->bPopulated)
        {
            (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->sPortDef.bEnabled
                      = OMX_TRUE;
             m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                   OMX_EventCmdComplete,
                                   OMX_CommandPortEnable,
                                   OMX_MUX_INDEX_PORT_AUDIO, NULL);
        }
        else
        {
            (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)
                                     ->bEnableRequested = OMX_TRUE;
        }
    }
    else
    {

        if((arrPortConfig + nPortIndex)->bPopulated)
        {
            (arrPortConfig + nPortIndex)->sPortDef.bEnabled
                      = OMX_TRUE;
             m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                   OMX_EventCmdComplete,
                                   OMX_CommandPortEnable,
                                   nPortIndex, NULL);
        }
        else
        {
            (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)
                                     ->bEnableRequested = OMX_TRUE;
        }
    }
    return OMX_ErrorNone;
}


/*==============================================================================

         FUNCTION:         OMX_FileMux_DisablePort

         DESCRIPTION:
*//**       @brief         A particular port is disabled by this call. Free
                           buffer calls may follow this and port may be
                           depopulated.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param
                           nPortIndex : Selected port for the operation

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_DisablePort(OMX_U32 nPortIndex)
{
    if(nPortIndex == OMX_ALL)
    {
        /**---------------------------------------------------------------------
          Video
        ------------------------------------------------------------------------
        */

        if((arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO)->nNumBuffAllocated == 0)
        {
            (arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO)->sPortDef.bEnabled
                      = OMX_FALSE;

             m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                   OMX_EventCmdComplete,
                                   OMX_CommandPortDisable,
                                   OMX_MUX_INDEX_PORT_VIDEO, NULL);
        }
        else
        {
            (arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO)->bDisableRequested
                                                            = OMX_TRUE;
            /**-----------------------------------------------------------------
              here we are not flushing as it will create confusion if another
              flsh command comes from IL client. So let things take its natural
              course and let the buffers come out after being procesed.
            --------------------------------------------------------------------
            */
        }
        /**---------------------------------------------------------------------
          Audio
        ------------------------------------------------------------------------
        */

        if((arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->nNumBuffAllocated == 0)
        {
            (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->sPortDef.bEnabled
                      = OMX_FALSE;

             m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                   OMX_EventCmdComplete,
                                   OMX_CommandPortDisable,
                                   OMX_MUX_INDEX_PORT_AUDIO, NULL);
        }
        else
        {
            (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->bDisableRequested
                                                            = OMX_TRUE;
            /**-----------------------------------------------------------------
              here we are not flushing as it will create confusion if another
              flsh command comes from IL client. So let things take its natural
              course and let the buffers come out after being procesed.
            --------------------------------------------------------------------
            */

        }
    }
    else if (nPortIndex < (NUM_VIDEO_PORT + NUM_AUDIO_PORT + NUM_TEXT_PORT))
    {
        if((arrPortConfig + nPortIndex)->nNumBuffAllocated == 0)
        {
            (arrPortConfig + nPortIndex)->sPortDef.bEnabled
                      = OMX_FALSE;

             m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                   OMX_EventCmdComplete,
                                   OMX_CommandPortDisable,
                                   nPortIndex, NULL);
        }
        else
        {
            (arrPortConfig + nPortIndex)->bDisableRequested
                                                            = OMX_TRUE;
            /**-----------------------------------------------------------------
              here we are not flushing as it will create confusion if another
              flsh command comes from IL client. So let things take its natural
              course and let the buffers come out after being procesed.
            --------------------------------------------------------------------
            */
        }
    }
    else
    {
        return OMX_ErrorBadPortIndex;
    }
    return OMX_ErrorNone;
}


/*==============================================================================

         FUNCTION:         OMX_FileMux_FreeBuffer

         DESCRIPTION:
*//**       @brief         Free the memory allocated for port buffers on each
                           call. This will be called only if this layer has
                           allocated the memory.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
//int OMX_FileMux::OMX_FileMux_FreeBuffer(MMI_FreeBufferCmdType *pBuffCmd)
//{
    /**-------------------------------------------------------------------------
      Lets straight away free this as it will be only called if we have
     allocated the
    ----------------------------------------------------------------------------
    */
 //   OMX_FILEMUX_FREEIF(pBuffCmd->pBuffer);
  //  return OMX_ErrorNone;
//}


/*==============================================================================

         FUNCTION:         OMX_FileMux_UseBuffer

         DESCRIPTION:
*//**       @brief         Client passes the buffer information for each buffer
                           that is going to be used for the port by these calls.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
//int OMX_FileMux::OMX_FileMux_UseBuffer(MMI_UseBufferCmdType *pBuffCmd)
//{
    /**-------------------------------------------------------------------------
        Nothing much to do here. Let's just return
    ----------------------------------------------------------------------------
    */
//    return OMX_ErrorNone;
//}


/*==============================================================================

         FUNCTION:         OMX_FileMux_AllocBuffer

         DESCRIPTION:
*//**       @brief         Allocates memory for port buffers as requested by
                           client.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ErrorNone or OMX_ErrorInsufficientResources


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
/*int OMX_FileMux::OMX_FileMux_AllocBuffer(MMI_AllocBufferCmdType *pBuffCmd)
{
    pBuffCmd->pBuffer = (OMX_U8*)OMX_FILEMUX_MALLOC(pBuffCmd->nSize);
    if(!pBuffCmd->pBuffer)
    {
        return OMX_ErrorInsufficientResources;
    }
    return OMX_ErrorNone;
}*/


/*==============================================================================

         FUNCTION:         OMX_FileMux_GetSetCustomParam

         DESCRIPTION:
*//**       @brief         This function Gets/Sets MMI custom params.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ErrorNone or appropriate error codes


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_GetSetCustomParam
(
    int           nCode,
    OMX_U32       nParamIndex,
    void         *pParamStructure
)
{
#if 0
    if(NULL == pParamStructure)
    {
        return OMX_ErrorBadParameter;
    }

    switch(nParamIndex)
    {
        case MMI_IndexDomainDef:
        {
            MMI_ParamDomainDefType        *pParam;
            OMX_U32                        nPortType;

            OMX_AUDIO_PORTDEFINITIONTYPE  *pAudioSrc;
            OMX_AUDIO_PORTDEFINITIONTYPE  *pAudioDst;
            OMX_VIDEO_PORTDEFINITIONTYPE  *pVideoSrc;
            OMX_VIDEO_PORTDEFINITIONTYPE  *pVideoDst;

            pParam = (MMI_ParamDomainDefType *)pParamStructure;

            /**-----------------------------------------------------------------
             In Get param populate the contents of pParam with contents
             from our context else do the otherway around.
            --------------------------------------------------------------------
            */
            if(nCode  == OMX_MUX_CMD_GET_CUSTOM_PARAM)
            {
                pAudioSrc = &(arrPortConfig +
                             pParam->nPortIndex)->sPortDef.format.audio;
                pAudioDst = &pParam->format.audio;
                pVideoSrc = &(arrPortConfig +
                             pParam->nPortIndex)->sPortDef.format.video;
                pVideoDst = &pParam->format.video;
            }
            else
            {
                pAudioDst = &(arrPortConfig +
                             pParam->nPortIndex)->sPortDef.format.audio;
                pAudioSrc = &pParam->format.audio;
                pVideoDst = &(arrPortConfig +
                             pParam->nPortIndex)->sPortDef.format.video;
                pVideoSrc = &pParam->format.video;
            }


            /**-----------------------------------------------------------------
                 Get Port Type from port Index
            --------------------------------------------------------------------
            */
            nPortType = GET_PORT_TYPE(pParam->nPortIndex);

            switch (nPortType)
            {

                /**-------------------------------------------------------------
                   Send the default Audio port parameters
                ----------------------------------------------------------------
                */
                case OMX_MUX_INDEX_PORT_AUDIO:
                {
                 //   if(pAudioSrc->cMIMEType)
                    //    strlcpy(pAudioDst->cMIMEType, pAudioSrc->cMIMEType,
                         //                      sizeof(pAudioDst->cMIMEType));
                    pAudioDst->bFlagErrorConcealment
                                             = pAudioSrc->bFlagErrorConcealment;
                    pAudioDst->eEncoding     = pAudioSrc->eEncoding;
                    pAudioDst->pNativeRender = pAudioSrc->pNativeRender;
                    break;
                }
                /**-------------------------------------------------------------
                   Send the default video port parameters
                ----------------------------------------------------------------
                */
                case OMX_MUX_INDEX_PORT_VIDEO:
                {
                    //if(pVideoSrc->cMIMEType)
                     //   strlcpy(pVideoDst->cMIMEType, pVideoSrc->cMIMEType,
                        //                        sizeof(pVideoDst->cMIMEType));
                    pVideoDst->bFlagErrorConcealment
                                             = pVideoSrc->bFlagErrorConcealment;
                    pVideoDst->eColorFormat  = pVideoSrc->eColorFormat;
                    pVideoDst->eCompressionFormat
                                             = pVideoSrc->eCompressionFormat;
                    pVideoDst->nBitrate      = pVideoSrc->nBitrate;
                    pVideoDst->nFrameHeight  = pVideoSrc->nFrameHeight;
                    pVideoDst->nFrameWidth   = pVideoSrc->nFrameWidth;
                    pVideoDst->nSliceHeight  = pVideoSrc->nSliceHeight;
                    pVideoDst->nStride       = pVideoSrc->nStride;
                    pVideoDst->pNativeRender = pVideoSrc->pNativeRender;
                    pVideoDst->pNativeWindow = pVideoSrc->pNativeWindow;
                    pVideoDst->xFramerate    = pVideoSrc->xFramerate;
                    break;
                }
                /**-------------------------------------------------------------
                   Send the default Text port parameters
                ----------------------------------------------------------------
                */
                case OMX_MUX_INDEX_PORT_TEXT:
                    return OMX_ErrorNotImplemented;


                default:
                    return OMX_ErrorBadParameter;


            }
            break;
        }
        case MMI_IndexBuffersReq:
        {
            MMI_ParamBuffersReqType *pParam =
                               (MMI_ParamBuffersReqType *)pParamStructure;
            MMI_ParamBuffersReqType *pBuffInfo =
                          &(arrPortConfig + pParam->nPortIndex)->sBuffInfo;


            if(nCode  == OMX_MUX_CMD_GET_CUSTOM_PARAM)
            {
                /**-------------------------------------------------------------
                 All default parameters can be copied in get param
                ----------------------------------------------------------------
                */
                pParam->bBuffersContiguous = pBuffInfo->bBuffersContiguous;
                pParam->nAlignment         = pBuffInfo->nAlignment;
                pParam->nBufferPoolId      = pBuffInfo->nBufferPoolId;
                pParam->nCount             = pBuffInfo->nCount;
                pParam->nDataSize          = pBuffInfo->nDataSize;
                pParam->nMinCount          = pBuffInfo->nMinCount;
                pParam->nSuffixSize        = pBuffInfo->nSuffixSize;

             }
            else
            {
                /**-------------------------------------------------------------
                   Only This is allowed by spec
                ----------------------------------------------------------------
                */
                pBuffInfo->nCount = pParam->nCount;
            }
            break;
        }
        default:
            return OMX_ErrorBadParameter;
    }
#else
(void)nCode;
(void)nParamIndex;
(void)pParamStructure;
#endif
    return OMX_ErrorNone;

}



/*==============================================================================

         FUNCTION:         OMX_FileMux_GetSetParam

         DESCRIPTION:
*//**       @brief         This function Sets/Gets openmax defined technology
                           specific parameters.

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ErrorNone or appropriate error code


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_GetSetParam
(
    int           nCode,
    OMX_INDEXTYPE eIndex,
    void          *pParamStructure
)
{
    switch((OMX_U32)eIndex)
    {

        case OMX_IndexParamAudioInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam =
                            (OMX_PORT_PARAM_TYPE *)pParamStructure;

            pPortParam->nPorts = NUM_AUDIO_PORT;
            pPortParam->nStartPortNumber = OMX_MUX_INDEX_PORT_AUDIO;
            return OMX_ErrorNone;
        }

        case OMX_IndexParamVideoInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam =
                            (OMX_PORT_PARAM_TYPE *)pParamStructure;

            pPortParam->nPorts = NUM_VIDEO_PORT;
            pPortParam->nStartPortNumber = OMX_MUX_INDEX_PORT_VIDEO;
            return OMX_ErrorNone;
        }

        case OMX_IndexParamImageInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam =
                            (OMX_PORT_PARAM_TYPE *)pParamStructure;

            pPortParam->nPorts = 0;
            pPortParam->nStartPortNumber = 0;
            return OMX_ErrorNone;
        }

        case OMX_IndexParamOtherInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam =
                            (OMX_PORT_PARAM_TYPE *)pParamStructure;

            pPortParam->nPorts = NUM_TEXT_PORT;
            pPortParam->nStartPortNumber = 0;
            return OMX_ErrorNone;
        }

        case OMX_IndexParamContentURI:
        {

            OMX_PARAM_CONTENTURITYPE *pParam =
                              (OMX_PARAM_CONTENTURITYPE *)pParamStructure;
            OMX_U32 nBufSize;

            nBufSize = OMX_U32(pParam->nSize - sizeof(pParam->nVersion)-
                                                         sizeof(pParam->nSize));
            if(!pParam->contentURI)
            {
                return OMX_ErrorBadParameter;
            }
            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                /**------------------------------------------------------------
                  Lets allocate buffer size required for content URI. If
                 allocated buffer is not sufficient realloc.
                ----------------------------------------------------------------
                */
                if(!pContentURI || nContentURISize < nBufSize)
                {
                    OMX_FILEMUX_FREEIF(pContentURI);
                    pContentURI = (OMX_U16*)OMX_FILEMUX_MALLOC(nBufSize * 2);
                    if(NULL == pContentURI)
                    {
                        return OMX_ErrorInsufficientResources;
                    }
                    nContentURISize = nBufSize;

                }
                OMX_FileMux_CopyString(pContentURI,pParam->contentURI,
                                       (int)nBufSize);
            }
            else
            {
                if(nContentURISize <= nBufSize)
                {
                    OMX_FileMux_CopyString(pParam->contentURI, pContentURI,
                                   (int)nBufSize);
                }
                else
                {
                    return OMX_ErrorBadParameter;
                }
            }
            return OMX_ErrorNone;
        }
        case QOMX_FilemuxIndexParamContainerInfo:
        {
            QOMX_CONTAINER_INFOTYPE *pFileInfo =
                            (QOMX_CONTAINER_INFOTYPE *)pParamStructure;

            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                if(!(VALIDATE_FILE_FORMAT(pFileInfo->eFmtType)))
                {
                    return OMX_ErrorUnsupportedSetting;
                }
                nFileBrand = pFileInfo->eFmtType;
            }
            else
            {
                pFileInfo->eFmtType = nFileBrand;
            }

            nFileFormat = GET_FILE_FORMAT(nFileBrand);
            return OMX_ErrorNone;

        }

        case QOMX_FilemuxIndexConfigMediaInfo:
        {
            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                QOMX_MEDIAINFOTYPE *ptempMediaInfo =
                     (QOMX_MEDIAINFOTYPE*)pParamStructure;
                OMX_U64 *tempCdataSize;
                OMX_TICKS *tempCdataDuration;
                OmxTickPtr tempPtr;
                /**-------------------------------------------------------------
                   If file duration or size limit is set by IL client, there is
                  no need to write mediainfo, store the duration/size limit
                  which will be passed to video fmt writer. These values should
                  be set to mux create params before creating mux instanse.
                ----------------------------------------------------------------
                */
                if (ptempMediaInfo->eTag == QOMX_MediaInfoDuration)
                {
                    tempPtr.dataPtr = (&ptempMediaInfo->cData[0]);
                    tempCdataDuration = (OMX_TICKS*)tempPtr.dataPtr;
                    nFileDurationLimit = *tempCdataDuration;
                    return OMX_ErrorNone;
                }
                else if (ptempMediaInfo->eTag == QOMX_MediaInfoSize)
                {
                    tempPtr.dataPtr = (&ptempMediaInfo->cData[0]);
                    tempCdataSize = (OMX_U64 *)tempPtr.dataPtr;
                    nFileSizeLimit = *tempCdataSize;
                    return OMX_ErrorNone;
                }
                else
                {
      //      return OMX_FileMux_MediaInfoUpdate(
           //     (QOMX_MEDIAINFOTYPE*)pParamStructure);
        }
            }
            else
            {
                /** We dont support GET parm for media info*/
                return OMX_ErrorUnsupportedSetting;
            }
        }

        case QOMX_FilemuxIndexParamVideoSyntaxHdr:
        {
            QOMX_VIDEO_SYNTAXHDRTYPE* pSyntaxHeader =
                            (QOMX_VIDEO_SYNTAXHDRTYPE*)pParamStructure;

            if(GET_PORT_TYPE(pSyntaxHeader->nPortIndex)
                                                    != OMX_MUX_INDEX_PORT_VIDEO)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                  "Syntax Header set for audio port. Error!!!");
                return OMX_ErrorBadParameter;
            }

            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                if((arrPortConfig + pSyntaxHeader->nPortIndex)->nSyntaxHeaderLen
                    < pSyntaxHeader->nBytes)
                {
                    /**---------------------------------------------------------
                       If there is already a syntax header buffer allocated
                       use it if the new header can fit in. Other wise free
                        it and allocate again for the required size.
                    ------------------------------------------------------------
                    */
                    OMX_FILEMUX_FREEIF(
                        (arrPortConfig + pSyntaxHeader->nPortIndex)->pSyntaxHdr);

                    (arrPortConfig + pSyntaxHeader->nPortIndex)->pSyntaxHdr =
                            (OMX_U8*)OMX_FILEMUX_MALLOC(pSyntaxHeader->nBytes);

                    if(!(arrPortConfig + pSyntaxHeader->nPortIndex)->pSyntaxHdr)
                    {
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                     "MMi Mux Set Syntax Hdr Malloc Fail !!!");
                        return OMX_ErrorInsufficientResources;
                    }
                }
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                        "Mux received out of band header!!!");

                OMX_MUX_MEM_COPY(
                       (arrPortConfig + pSyntaxHeader->nPortIndex)->pSyntaxHdr,
                        pSyntaxHeader->data,
                        pSyntaxHeader->nBytes);
                (arrPortConfig + pSyntaxHeader->nPortIndex)->nSyntaxHeaderLen =
                               pSyntaxHeader->nBytes;
            }
            else
            {
                /**-------------------------------------------------------------
                  If IL client has allocated sufficient memory copy the header
                  Otherwise just return the size.
                ----------------------------------------------------------------
                */
                if(pSyntaxHeader->nBytes >=
                (arrPortConfig + pSyntaxHeader->nPortIndex)->nSyntaxHeaderLen &&
                  pSyntaxHeader->data &&
                  (arrPortConfig + pSyntaxHeader->nPortIndex)->pSyntaxHdr)
                {
                    OMX_MUX_MEM_COPY(
                         pSyntaxHeader->data,
                    (arrPortConfig +
                         pSyntaxHeader->nPortIndex)->pSyntaxHdr,
                    (arrPortConfig +
                         pSyntaxHeader->nPortIndex)->nSyntaxHeaderLen);
                }
                pSyntaxHeader->nBytes =
                    (arrPortConfig +
                         pSyntaxHeader->nPortIndex)->nSyntaxHeaderLen;
            }
            return OMX_ErrorNone;
        }

        case QOMX_FilemuxIndexParamIStreamPort:
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                         "OMX_FileMux_SetParam Set ISTREAMPORT INTERFACE");
            QOMX_CONTENTINTERFACETYPE* pIntfInfo =
                   (QOMX_CONTENTINTERFACETYPE*)pParamStructure;

            if(!pIntfInfo ||
                pIntfInfo->nInterfaceSize < sizeof(pIStreamPort))
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "OMX_FileMux_SetGetParm ISTREAMPORT failed,badparams");
                return OMX_ErrorBadParameter;
            }
            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                MM_MSG_PRIO(MM_GENERAL,
                            MM_PRIO_MEDIUM,
                            "OMX_FileMux_SetParam ISTREAMPORT  updated..");
                memcpy(&pIStreamPort,pIntfInfo->pInterface,
                           sizeof(pIStreamPort));
                /**-------------------------------------------------------------
                 If filemux is running already. we can update the stream port
                 to be used.
                ----------------------------------------------------------------
                */
                if(pFileMux)
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                            "OMXMUX::Update Stream Port");
                    pFileMux->MUX_update_streamport(reinterpret_cast<uint64_t>(pIStreamPort));
                }
            }
            else
            {
                MM_MSG_PRIO(MM_GENERAL,
                            MM_PRIO_MEDIUM,
                            "OMX_FileMux_GetParam ISTREAMPORT  updated..");
                memcpy(pIntfInfo->pInterface,&pIStreamPort,
                           sizeof(pIStreamPort));
            }
            return OMX_ErrorNone;
        }
        /**----------------------------------------------------------------------
          This is implemented in baseclass, need to remove. --- 8< ---
        -------------------------------------------------------------------------
        */
        case OMX_IndexParamPortDefinition:
        {
            /**------------------------------------------------------------------
                     Lets define one source and dest param structures
            ---------------------------------------------------------------------
            */
            OMX_PARAM_PORTDEFINITIONTYPE *pDst;

            OMX_PARAM_PORTDEFINITIONTYPE *pSrc;

            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                /**-------------------------------------------------------------
                 Lets switch source and destination for set/get
                ----------------------------------------------------------------
                */
                pSrc = (OMX_PARAM_PORTDEFINITIONTYPE *)pParamStructure;
                pDst = &(arrPortConfig + pSrc->nPortIndex)->sPortDef;

                VALIDATE_PORT_INDEX_AND_RETURN(pSrc->nPortIndex);
            }
            else
            {
                pDst = (OMX_PARAM_PORTDEFINITIONTYPE *)pParamStructure;
                pSrc = &(arrPortConfig + pDst->nPortIndex)->sPortDef;

                VALIDATE_PORT_INDEX_AND_RETURN(pDst->nPortIndex);
                /**-------------------------------------------------------------
                These setting as read only allowed only in GET param
                ----------------------------------------------------------------
                */
                pDst->bBuffersContiguous = pSrc->bBuffersContiguous;
                pDst->bEnabled           = pSrc->bEnabled;
                pDst->bPopulated         = pSrc->bPopulated;
                pDst->eDir               = pSrc->eDir;
                pDst->eDomain            = pSrc->eDomain;
            }
            /**-----------------------------------------------------------------
             Read Write parameters goes here
            --------------------------------------------------------------------
            */
            pDst->nBufferCountActual = pSrc->nBufferCountActual;
            pDst->nBufferCountMin    = pSrc->nBufferCountMin;
            pDst->nBufferSize        = pSrc->nBufferSize;
            pDst->nBufferAlignment   = pSrc->nBufferAlignment;



            switch(pSrc->eDomain)
            {
                case OMX_PortDomainAudio:
                {

                    OMX_AUDIO_PORTDEFINITIONTYPE *pAudioSrc =
                                                 &pSrc->format.audio;
                    OMX_AUDIO_PORTDEFINITIONTYPE *pAudioDst =
                                                 &pDst->format.audio;


                  //  strlcpy(pAudioDst->cMIMEType, pAudioSrc->cMIMEType,
                    //                           sizeof(pAudioDst->cMIMEType));
                    pAudioDst->bFlagErrorConcealment
                                             = pAudioSrc->bFlagErrorConcealment;
                    pAudioDst->eEncoding     = pAudioSrc->eEncoding;
                    pAudioDst->pNativeRender = pAudioSrc->pNativeRender;
                    return OMX_ErrorNone;
                }
                case OMX_PortDomainVideo:
                {
                    OMX_VIDEO_PORTDEFINITIONTYPE *pVideoSrc =
                                                 &pSrc->format.video;
                    OMX_VIDEO_PORTDEFINITIONTYPE *pVideoDst =
                                                 &pDst->format.video;
                   // strlcpy(pVideoDst->cMIMEType, pVideoSrc->cMIMEType,
                    //                           sizeof(pAudioDst->cMIMEType));
                    pVideoDst->bFlagErrorConcealment
                                             = pVideoSrc->bFlagErrorConcealment;
                    pVideoDst->eColorFormat  = pVideoSrc->eColorFormat;
                    pVideoDst->eCompressionFormat
                                             = pVideoSrc->eCompressionFormat;
                    pVideoDst->nBitrate      = pVideoSrc->nBitrate;
                    pVideoDst->nFrameHeight  = pVideoSrc->nFrameHeight;
                    pVideoDst->nFrameWidth   = pVideoSrc->nFrameWidth;
                    pVideoDst->nSliceHeight  = pVideoSrc->nSliceHeight;
                    pVideoDst->nStride       = pVideoSrc->nStride;
                    pVideoDst->pNativeRender = pVideoSrc->pNativeRender;
                    pVideoDst->pNativeWindow = pVideoSrc->pNativeWindow;
                    pVideoDst->xFramerate    = pVideoSrc->xFramerate;
                    return OMX_ErrorNone;
                }
                /**-------------------------------------------------------------
                       Text port treated as other now.
                ----------------------------------------------------------------
                */
                case OMX_PortDomainOther:
                {
                    return OMX_ErrorNotImplemented;

                }
                default:
                    return OMX_ErrorBadParameter;
            }
        }

        case OMX_IndexParamAudioPortFormat:
        {
            OMX_AUDIO_PARAM_PORTFORMATTYPE *pParam
                            = (OMX_AUDIO_PARAM_PORTFORMATTYPE*)pParamStructure;

            if(GET_PORT_TYPE(pParam->nPortIndex) != OMX_MUX_INDEX_PORT_AUDIO)
            {
                return OMX_ErrorBadParameter;
            }

            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                /**-------------------------------------------------------------
                  Check if codec is supported with file brand
                ----------------------------------------------------------------
                */
                if(nFileFormat == MUX_FMT_MP4 &&
                       ( pParam->eEncoding != OMX_AUDIO_CodingPCM &&
                         pParam->eEncoding != OMX_AUDIO_CodingG711 &&
                         pParam->eEncoding != (OMX_AUDIO_CODINGTYPE)QOMX_AUDIO_CodingEVRCB &&
                         pParam->eEncoding != (OMX_AUDIO_CODINGTYPE)QOMX_AUDIO_CodingEVRCWB))
                {
                    if(!OMX_FileMux_ValidateFileFormat(nFileBrand,
                                                   pParam->eEncoding,
                                                   OMX_VIDEO_CodingUnused))
                    {
                        return OMX_ErrorUnsupportedSetting;
                    }
                }
                if(nFileFormat == MUX_FMT_MP2 &&
                       ( pParam->eEncoding != OMX_AUDIO_CodingPCM &&
                         pParam->eEncoding != OMX_AUDIO_CodingAAC ))
                {
                    return OMX_ErrorUnsupportedSetting;
                }

                if(AUDIO_COMPRESSION_FORMAT != pParam->eEncoding)
                {
                    /**---------------------------------------------------------
                        Port Configuration changed. Lets reset Params.
                    ------------------------------------------------------------
                    */
                    OMX_MUX_MEM_SET(&(arrPortConfig + pParam->nPortIndex)
                            ->sFormatSpecificInfo, 0,
                            sizeof(OMX_FileMux_MediaFormatConfigType));
                }
                AUDIO_COMPRESSION_FORMAT = pParam->eEncoding;
                /**-------------------------------------------------------------
                  If container format was set before portdefinition,
                  we would have detected the format as MUX_FMT_INVALID.
                  So if client is setting EVRCB/WB here lets move it to
                  appropriate file format.
                ----------------------------------------------------------------
                */
                if(nFileFormat == MUX_FMT_INVALID)
                {
                    if(AUDIO_COMPRESSION_FORMAT == (OMX_AUDIO_CODINGTYPE)QOMX_AUDIO_CodingEVRCB)
                    {
                        nFileFormat = MUX_FMT_EVRCB;
                    }
                    else if(AUDIO_COMPRESSION_FORMAT == (OMX_AUDIO_CODINGTYPE)QOMX_AUDIO_CodingEVRCWB)
                    {
                        nFileFormat = MUX_FMT_EVRCWB;
                    }
                }
            }
            else
            {
                /**-------------------------------------------------------------
                  As per spec nIndex = [0, N-1], first element of pur array
                  gives num of supported formats. lets index nIndex + 1.
                ----------------------------------------------------------------
                */
                if(pParam->nIndex < sizeof(gAudioSupported)/
                                               sizeof(OMX_AUDIO_CODINGTYPE))
                {
                    /*----------------------------------------------------------
                     TODO :: Give supported type based on file format
                    ------------------------------------------------------------
                    */
                    pParam->eEncoding = gAudioSupported[pParam->nIndex];
                }
                else
                {
                    return OMX_ErrorNoMore;
                }

            }
            return OMX_ErrorNone;
        }

        case OMX_IndexParamVideoPortFormat:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *pParam
                            = (OMX_VIDEO_PARAM_PORTFORMATTYPE*)pParamStructure;
            OMX_VIDEO_PORTDEFINITIONTYPE *pPortParam
                            = &((arrPortConfig + pParam->nPortIndex)->
                                                         sPortDef.format.video);

            if(GET_PORT_TYPE(pParam->nPortIndex) != OMX_MUX_INDEX_PORT_VIDEO)
            {
                return OMX_ErrorBadParameter;
            }
            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                if((nFileFormat == MUX_FMT_MP4) &&
                         !OMX_FileMux_ValidateFileFormat(nFileBrand,
                                                   OMX_AUDIO_CodingUnused,
                                                   pParam->eCompressionFormat))
                {
                    return OMX_ErrorUnsupportedSetting;
                }

                if(nFileFormat == MUX_FMT_MP2 &&
                    pParam->eCompressionFormat != OMX_VIDEO_CodingAVC)
                {
                    return OMX_ErrorUnsupportedSetting;
                }

                if(VIDEO_COMPRESSION_FORMAT != pParam->eCompressionFormat)
                {
                    /**---------------------------------------------------------
                        Port Configuration changed. Lets reset Params.
                    ------------------------------------------------------------
                    */
                    OMX_MUX_MEM_SET(&(arrPortConfig + pParam->nPortIndex)
                            ->sFormatSpecificInfo, 0,
                            sizeof(OMX_FileMux_MediaFormatConfigType));
                }
                pPortParam->eColorFormat       = pParam->eColorFormat;
                pPortParam->eCompressionFormat = pParam->eCompressionFormat;
                pPortParam->xFramerate         = pParam->xFramerate;

            }
            else
            {
                /**-------------------------------------------------------------
                  As per spec nIndex = [0, N-1], first element of pur array
                  gives num of supported formats. lets index nIndex + 1.
                ----------------------------------------------------------------
                */
                if(pParam->nIndex < sizeof(gVideoSupported) /
                                                   sizeof(OMX_VIDEO_CODINGTYPE))
                {
                    pParam->eCompressionFormat =
                               gVideoSupported[pParam->nIndex];

                    /*----------------------------------------------------------
                     TODO :: Give supported type based on file fornmat
                    ------------------------------------------------------------
                    */
                }
                else
                {
                    return OMX_ErrorNoMore;/* return err_no_more*/
                }

            }

            return OMX_ErrorNone;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            OMX_PARAM_COMPONENTROLETYPE *pCompRole =
                               (OMX_PARAM_COMPONENTROLETYPE*)pParamStructure;

            if ( sizeof(OMX_PARAM_COMPONENTROLETYPE) != pCompRole->nSize)
            {
                return OMX_ErrorBadParameter;
            }
            else if (NULL == pCompRole->cRole)
            {
                return OMX_ErrorBadParameter;
            }

            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                OMX_U32 nIndex = 0;
                OMX_ERRORTYPE nRet = OMX_ErrorUnsupportedSetting;

                while (nIndex < OMX_MUX_MAX_SUPPORTED_ROLES)
                {
                    /* Compare given role to component supported roles */
                    if ( 0 == strcmp((char*)pCompRole->cRole,
                                   gRolesSupported[nIndex]))
                    {
                        sMuxStats.pRole = (OMX_U8*)gRolesSupported[nIndex];
                        nRet = OMX_ErrorNone;
                        break;
                    }
                    nIndex++;
                }
                return nRet;
            }
            else
            {
                OMX_MUX_MEM_COPY(pCompRole->cRole,gRolesSupported[0],
                                                OMX_MAX_STRINGNAME_SIZE);
                return OMX_ErrorNone;
            }
        }

        /**---------------------------------------------------------------------
           Handle codec specific params case by case
        ------------------------------------------------------------------------
        */
        case OMX_IndexParamAudioAmr:
        {
            OMX_AUDIO_PARAM_AMRTYPE *pParam =
                (OMX_AUDIO_PARAM_AMRTYPE *)pParamStructure;
            QOMX_AUDIO_PARAM_AMRWBPLUSTYPE *pPortAudioParam;

            pPortAudioParam = &(arrPortConfig + pParam->nPortIndex)->
                                             sFormatSpecificInfo.sAMRWBInfo;

            if(GET_PORT_TYPE(pParam->nPortIndex) != OMX_MUX_INDEX_PORT_AUDIO)
            {
                return OMX_ErrorBadParameter;
            }
            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                /**-------------------------------------------------------------
                        Exchange parameters based on set or get. No _RO_ here
                ----------------------------------------------------------------
                */
                pPortAudioParam->eAMRBandMode = pParam->eAMRBandMode;
                pPortAudioParam->eAMRDTXMode  = pParam->eAMRDTXMode;
                pPortAudioParam->eAMRFrameFormat = pParam->eAMRFrameFormat;
                pPortAudioParam->nBitRate     = pParam->nBitRate;
                pPortAudioParam->nChannels    = pParam->nChannels;

            }
            else
            {
                pParam->eAMRBandMode = pPortAudioParam->eAMRBandMode;
                pParam->eAMRDTXMode  = pPortAudioParam->eAMRDTXMode;
                pParam->eAMRFrameFormat = pPortAudioParam->eAMRFrameFormat;
                pParam->nBitRate     = pPortAudioParam->nBitRate;
                pParam->nChannels    = pPortAudioParam->nChannels;
            }
            return OMX_ErrorNone;
        }

        case QOMX_FilemuxIndexParamAudioAmrWbPlus:
        {
            QOMX_AUDIO_PARAM_AMRWBPLUSTYPE *pParam =
                (QOMX_AUDIO_PARAM_AMRWBPLUSTYPE *)pParamStructure;
            QOMX_AUDIO_PARAM_AMRWBPLUSTYPE *pPortAudioParam;

            pPortAudioParam = &(arrPortConfig + pParam->nPortIndex)->
                                             sFormatSpecificInfo.sAMRWBInfo;

            if(GET_PORT_TYPE(pParam->nPortIndex) != OMX_MUX_INDEX_PORT_AUDIO)
            {
                return OMX_ErrorBadParameter;
            }
            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                /**-------------------------------------------------------------
                        Exchange parameters based on set or get. No _RO_ here
                ----------------------------------------------------------------
                */
                if((pParam->eAMRBandMode > OMX_AUDIO_AMRBandModeWB8) &&
                      nFileFormat != MUX_FMT_MP4)
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                              "AMRWBplus not supported for non MP4 formats!!!");
                    return OMX_ErrorUnsupportedSetting;
                }
                return OMX_FileMux_SetMediaParams(eIndex,
                                                (void*)pParam,
                                                (void*)pPortAudioParam);
            }
            return OMX_FileMux_SetMediaParams(eIndex,
                                             (void*)pPortAudioParam,
                                             (void*)pParam);
        }

        case OMX_IndexParamAudioPcm:
        {
            OMX_AUDIO_PARAM_PCMMODETYPE *pParam =
                (OMX_AUDIO_PARAM_PCMMODETYPE *)pParamStructure;
            OMX_AUDIO_PARAM_PCMMODETYPE *pPortAudioParam;

            pPortAudioParam = &(arrPortConfig + pParam->nPortIndex)->
                                             sFormatSpecificInfo.sPCMInfo;

            if(GET_PORT_TYPE(pParam->nPortIndex) != OMX_MUX_INDEX_PORT_AUDIO)
            {
                return OMX_ErrorBadParameter;
            }
            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                /**-------------------------------------------------------------
                        Exchange parameters based on set or get. No _RO_ here
                ----------------------------------------------------------------
                */
                return OMX_FileMux_SetMediaParams(eIndex,
                                                (void*)pParam,
                                                (void*)pPortAudioParam);
            }
            return OMX_FileMux_SetMediaParams(eIndex,
                                             (void*)pPortAudioParam,
                                             (void*)pParam);
MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux GetSetParam sFormatSpecificInfo.sPCMInfo.nSamplingRate = %ld", pPortAudioParam->nSamplingRate);

        }

        case OMX_IndexParamAudioQcelp13:
        {

            OMX_AUDIO_PARAM_QCELP13TYPE *pParam =
                (OMX_AUDIO_PARAM_QCELP13TYPE *)pParamStructure;
            OMX_AUDIO_PARAM_QCELP13TYPE *pPortAudioParam;

            pPortAudioParam = &(arrPortConfig + pParam->nPortIndex)->
                                             sFormatSpecificInfo.sQCELPInfo;

            if(GET_PORT_TYPE(pParam->nPortIndex) != OMX_MUX_INDEX_PORT_AUDIO)
            {
                return OMX_ErrorBadParameter;
            }
            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                return OMX_FileMux_SetMediaParams(eIndex,
                                                (void*)pParam,
                                                (void*)pPortAudioParam);
            }
            return OMX_FileMux_SetMediaParams(eIndex,
                                            (void*)pPortAudioParam,
                                            (void*)pParam);

        }

        case OMX_IndexParamAudioEvrc:
        case QOMX_FilemuxIndexParamAudioEvrcb:
        case QOMX_FilemuxIndexParamAudioEvrcwb:
        {
            OMX_AUDIO_PARAM_EVRCTYPE *pParam =
                (OMX_AUDIO_PARAM_EVRCTYPE *)pParamStructure;
            OMX_AUDIO_PARAM_EVRCTYPE *pPortAudioParam;

            pPortAudioParam = &(arrPortConfig + pParam->nPortIndex)->
                                               sFormatSpecificInfo.sEVRCInfo;

            if(GET_PORT_TYPE(pParam->nPortIndex) != OMX_MUX_INDEX_PORT_AUDIO)
            {
                return OMX_ErrorBadParameter;
            }
            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                return OMX_FileMux_SetMediaParams(eIndex,
                                                (void*)pParam,
                                                (void*)pPortAudioParam);
            }
            return OMX_FileMux_SetMediaParams(eIndex,
                                             (void*)pPortAudioParam,
                                             (void*)pParam);
        }

        case OMX_IndexParamAudioAac:
        {
            OMX_AUDIO_PARAM_AACPROFILETYPE *pParam =
                (OMX_AUDIO_PARAM_AACPROFILETYPE *)pParamStructure;
            OMX_AUDIO_PARAM_AACPROFILETYPE *pPortAudioParam;

            pPortAudioParam = &(arrPortConfig + pParam->nPortIndex)->
                                                   sFormatSpecificInfo.sAACInfo;

            if(GET_PORT_TYPE(pParam->nPortIndex) != OMX_MUX_INDEX_PORT_AUDIO)
            {
                return OMX_ErrorBadParameter;
            }
            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                OMX_ERRORTYPE nError;
                nError = OMX_FileMux_SetMediaParams(eIndex,
                                                (void*)pParam,
                                                (void*)pPortAudioParam);

                if(nError != OMX_ErrorNone)
                {
                    return nError;
                }
                if(pPortAudioParam->eAACStreamFormat !=
                              OMX_AUDIO_AACStreamFormatMP4ADTS ||
                   pPortAudioParam->eAACStreamFormat !=
                              OMX_AUDIO_AACStreamFormatADIF    ||
                   pPortAudioParam->eAACStreamFormat !=
                              OMX_AUDIO_AACStreamFormatMP4FF)
                {
                    return OMX_ErrorNone;//MMI_S_ENOTSUPP;
                }

            }
            return OMX_FileMux_SetMediaParams(eIndex,
                                             (void*)pPortAudioParam,
                                             (void*)pParam);

        }

        case QOMX_IndexParamAudioAc3:
        {
            QOMX_AUDIO_PARAM_AC3TYPE *pParam =
                (QOMX_AUDIO_PARAM_AC3TYPE *)pParamStructure;
            QOMX_AUDIO_PARAM_AC3TYPE *pPortAudioParam;

            pPortAudioParam = &(arrPortConfig + pParam->nPortIndex)->
                                          sFormatSpecificInfo.sAC3Info;

            if(GET_PORT_TYPE(pParam->nPortIndex) != OMX_MUX_INDEX_PORT_AUDIO)
            {
                return OMX_ErrorBadParameter;
            }
            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                OMX_ERRORTYPE nError;
                nError = OMX_FileMux_SetMediaParams(eIndex,
                                                    (void*)pParam,
                                                    (void*)pPortAudioParam);

                if(nError != OMX_ErrorNone)
                {
                    return nError;
                }


            }
            return OMX_FileMux_SetMediaParams(eIndex,
                                           (void*)pPortAudioParam,
                                           (void*)pParam);
        }


        case OMX_IndexParamVideoAvc:
        {
            /**-----------------------------------------------------------------
                     Lets do get/ set of all avc parameters here
            --------------------------------------------------------------------
            */
            OMX_VIDEO_PARAM_AVCTYPE *pParam =
                (OMX_VIDEO_PARAM_AVCTYPE *)pParamStructure;
            OMX_VIDEO_PARAM_AVCTYPE *pPortVideoParam;

            pPortVideoParam = &(arrPortConfig + pParam->nPortIndex)
                                           ->sFormatSpecificInfo.sAVCInfo;

            if(GET_PORT_TYPE(pParam->nPortIndex) != OMX_MUX_INDEX_PORT_VIDEO)
            {
                return OMX_ErrorBadParameter;
            }
            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                return OMX_FileMux_SetMediaParams(eIndex,
                                                (void*)pParam,
                                                (void*)pPortVideoParam);
            }
            return OMX_FileMux_SetMediaParams(eIndex,
                                             (void*)pPortVideoParam,
                                             (void*)pParam);

        }

        case OMX_IndexParamVideoH263:
        {
            OMX_VIDEO_PARAM_H263TYPE *pParam =
                (OMX_VIDEO_PARAM_H263TYPE *)pParamStructure;
            OMX_VIDEO_PARAM_H263TYPE *pPortVideoParam;

            pPortVideoParam = &(arrPortConfig + pParam->nPortIndex)->
                                             sFormatSpecificInfo.sH263Info;

            if(GET_PORT_TYPE(pParam->nPortIndex) != OMX_MUX_INDEX_PORT_VIDEO)
            {
                return OMX_ErrorBadParameter;
            }
            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                return OMX_FileMux_SetMediaParams(eIndex,
                                                (void*)pParam,
                                                (void*)pPortVideoParam);
            }
            return OMX_FileMux_SetMediaParams(eIndex,
                                             (void*)pPortVideoParam,
                                             (void*)pParam);

        }

        case OMX_IndexParamVideoMpeg4:
        {
            OMX_VIDEO_PARAM_MPEG4TYPE *pParam =
                (OMX_VIDEO_PARAM_MPEG4TYPE *)pParamStructure;
            OMX_VIDEO_PARAM_MPEG4TYPE *pPortVideoParam;

            pPortVideoParam = &(arrPortConfig + pParam->nPortIndex)->
                                            sFormatSpecificInfo.sMPEG4Info;

            if(GET_PORT_TYPE(pParam->nPortIndex) != OMX_MUX_INDEX_PORT_VIDEO)
            {
                return OMX_ErrorBadParameter;
            }
            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                return OMX_FileMux_SetMediaParams(eIndex,
                                                (void*)pParam,
                                                (void*)pPortVideoParam);
            }
            return OMX_FileMux_SetMediaParams(eIndex,
                                             (void*)pPortVideoParam,
                                             (void*)pParam);

        }
        case QOMX_FilemuxIndexConfigRecordingStatisticsInterval:
        {
            QOMX_RECORDINGSTATISTICSINTERVALTYPE *pParam =
                (QOMX_RECORDINGSTATISTICSINTERVALTYPE *)pParamStructure;
            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                nStatisticsInterval = pParam->interval;
                if(bMuxOpen)
                {
                    pFileMux->MUX_set_StatisticsInterval(nStatisticsInterval);
                }
            }
            else
            {
                pParam->interval = nStatisticsInterval;
            }
            return OMX_ErrorNone;
        }
        case QOMX_FilemuxIndexConfigRecordingStatisticsStatus:
        {
            if(OMX_MUX_CMD_GET_STD_OMX_PARAM == nCode)
            {
                QOMX_RECORDINGSTATISTICSTYPE *pParam =
                  (QOMX_RECORDINGSTATISTICSTYPE *)pParamStructure;
                pParam->nRecordedTime = nRecStatistics.nRecordedTime;
                pParam->nTimeCanRecord= nRecStatistics.nTimeCanRecord;
                pParam->nSpaceConsumed = nRecStatistics.nSpaceConsumed;
                pParam->nSpaceLeft = nRecStatistics.nSpaceLeft;
                return OMX_ErrorNone;
            }
            else
            {
                return OMX_ErrorUnsupportedSetting;
            }
        }

      case QOMX_FilemuxIndexParamEncryptType:
        {
            QOMX_ENCRYPTIONTYPE *pParam =
                (QOMX_ENCRYPTIONTYPE *)pParamStructure;
            if(OMX_MUX_CMD_SET_STD_OMX_PARAM == nCode)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "OMX_FileMux::OMX_FileMux_GetSetParam setting encrypt params");
                nEncryptTypeConfigParameters.nStreamEncrypted = pParam->nStreamEncrypted;
                nEncryptTypeConfigParameters.nType = pParam->nType;
                nEncryptTypeConfigParameters.nEncryptVersion = pParam->nEncryptVersion;
                return OMX_ErrorNone;
            }
            else
            {
                return OMX_ErrorUnsupportedSetting;
            }
        }

        default:
            break;

    }
    return OMX_ErrorUnsupportedIndex;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_GetExtensionIndex

         DESCRIPTION:
*//**       @brief         Gives the extension index of a string.

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ErrorNone or appropriate error code


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_GetExtensionIndex
(
    OMX_STRING     cParamName,
    OMX_INDEXTYPE  *pIndex
)
{
    if(!cParamName || !pIndex)
    {
        return OMX_ErrorBadParameter;
    }

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux GetExtnIndex Called!!!");

    if(strcmp(cParamName, QOMX_INDEX_CONTAINER_INFO_STRING)
        == 0)
    {
        *(OMX_S32*)pIndex =  QOMX_FilemuxIndexParamContainerInfo;
    }

    else if(strcmp(cParamName, OMX_QCOM_INDEX_PARAM_EVRCB)
            == 0)
    {
        *(OMX_S32*)pIndex = QOMX_FilemuxIndexParamAudioEvrcb;
    }
    else if(strcmp(cParamName, OMX_QCOM_INDEX_PARAM_EVRCWB)
           == 0)
    {
        *(OMX_S32*)pIndex = QOMX_FilemuxIndexParamAudioEvrcwb;
    }

    else if(strcmp(cParamName, OMX_QCOM_INDEX_PARAM_AMRWBPLUS)
           == 0)
    {
        *(OMX_S32*)pIndex = QOMX_FilemuxIndexParamAudioAmrWbPlus;
    }

    else if(strcmp(cParamName, OMX_QCOM_INDEX_CONFIG_MEDIA_INFO)
           == 0)
    {
        *(OMX_S32*)pIndex = QOMX_FilemuxIndexConfigMediaInfo;
    }

    else if(strcmp(cParamName,OMX_QCOM_INDEX_PARAM_VID_SYNTXHDR)
           == 0)
    {
        *(OMX_S32*)pIndex = QOMX_FilemuxIndexParamVideoSyntaxHdr;
    }

    else if(strcmp(cParamName,
                 OMX_QCOM_INDEX_PARM_CONTENTINTERFACE_ISTREAMPORT)
           == 0)
    {
        *(OMX_S32*)pIndex = QOMX_FilemuxIndexParamIStreamPort;
    }
    else if(strcmp(cParamName,
                 OMX_QCOM_INDEX_CONFIG_RECORDINGSTATISTICS_INTERVAL)
           == 0)
    {
        *(OMX_S32*)pIndex =
            QOMX_FilemuxIndexConfigRecordingStatisticsInterval;
    }
    else if(strcmp(cParamName,
             OMX_QCOM_INDEX_CONFIG_RECORDINGSTATISTICS_STATUS)
       == 0)
    {
        *(OMX_S32*)pIndex =
            QOMX_FilemuxIndexConfigRecordingStatisticsStatus;
    }

    else if(strcmp(cParamName,
             QOMX_INDEX_CONFIG_ENCRYPT_TYPE)
       == 0)
    {
        *(OMX_S32*)pIndex =
            QOMX_FilemuxIndexParamEncryptType;
    }

    else
    {
        return OMX_ErrorUnsupportedIndex;
    }
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_QueueStreamBuffer

         DESCRIPTION:
*//**       @brief         This function queues stream buffer to inner layer for
                           decode.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_QueueStreamBuffer
(
    OMX_Mux_BufferCmdType
    *pBuffCmd
)
{
    OMX_U32 nPortIndex;
    OMX_ERRORTYPE     nRetStatus;

    MM_CriticalSection_Enter(pCSHandle);

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "OMX_FileMux ETB Called!!!");
    /**-------------------------------------------------------------------------
        Use port index to identify if it is audio or video
    ----------------------------------------------------------------------------
    */
    nPortIndex = GET_PORT_TYPE(pBuffCmd->pBufferHdr->nInputPortIndex);


    if((arrPortConfig + nPortIndex)->bDisableRequested)
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR, "ETB on disable requested port");
        return OMX_ErrorNone;
    }

    /**-------------------------------------------------------------------------
      If there is any UUID lets write it before writing any sample
    ----------------------------------------------------------------------------
    */
    if(!bUUIDWritten)
    {
       // (void)OMX_FileMux_WriteMediaInfoUUIDAll();
        bUUIDWritten = OMX_TRUE;
    }

    if(nPortIndex == OMX_MUX_INDEX_PORT_AUDIO)
    {
        nRetStatus = OMX_FileMux_QueueAudioStreamBuffer(pBuffCmd);
    }
    else if(nPortIndex == OMX_MUX_INDEX_PORT_VIDEO)
    {
        nRetStatus = OMX_FileMux_QueueVideoStreamBuffer(pBuffCmd);
    }
    else
    {
        nRetStatus = OMX_ErrorBadParameter;
    }

    MM_CriticalSection_Leave(pCSHandle);
    return nRetStatus;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_QueueVideoStreamBuffer

         DESCRIPTION:
*//**       @brief         This function queues stream video buffer to inner
                           layer for muxing.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_QueueVideoStreamBuffer
(
    OMX_Mux_BufferCmdType *pBuffCmd
)
{
    OMX_U32                 nStreamType;
    OMX_U32                 nDelta = 0;
    OMX_ERRORTYPE           nRetStatus = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE   *pBuffHdr;
    OMX_BOOL                bEOS         =
        (pBuffCmd->pBufferHdr->nFlags & OMX_BUFFERFLAG_EOS)?OMX_TRUE:OMX_FALSE;
    MUX_stream_create_params_type *pStreams =
             (MUX_stream_create_params_type*) pFileMuxStreams;

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "OMX_FileMux ETB Video Called!!!");

    /**-------------------------------------------------------------------------
       Check if audio and video are present. If only one of them is present
       the stream ID will be 0 irrespective of video or audio.
    ----------------------------------------------------------------------------
    */
    OMX_BOOL bAudioPresent = IS_AUDIO_PORT_USED;

    pBuffHdr = pBuffCmd->pBufferHdr;
    /**-------------------------------------------------------------------------
       Check if audio is present if it is then video takes stream ID of 2
    ----------------------------------------------------------------------------
    */
    nStreamType = bAudioPresent ? MMI_VIDEO_STREAM_NUM :
                                       MMI_AUDIO_STREAM_NUM;

    if(!sStreamInfo[nStreamType].bHeaderReceived &&
       pBuffHdr->nFlags & OMX_BUFFERFLAG_CODECCONFIG)
    {
        /**---------------------------------------------------------------------
           Assuming header is passed correctly in separate buffer.
        ------------------------------------------------------------------------
        */
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Header Received in buffer!!!");
        pFileMux->MUX_write_header((uint32)nStreamType, true, (uint32)pBuffHdr->nFilledLen,
                                   pBuffHdr->pBuffer + pBuffHdr->nOffset,
                                   pBuffHdr);
        pBuffHdr->nFilledLen = 0;
        sStreamInfo[nStreamType].bHeaderReceived = OMX_TRUE;
        return OMX_ErrorNone;
    }

    if(!sStreamInfo[nStreamType].bHeaderReceived)
    {
        /**---------------------------------------------------------------------
            No Inband Header received. Put if any out of band syntax header
        ------------------------------------------------------------------------
        */
        OMX_U32 nheaderLen =
            (arrPortConfig + pBuffHdr->nInputPortIndex)->nSyntaxHeaderLen;
        OMX_U8 *pHeader    =
            (arrPortConfig + pBuffHdr->nInputPortIndex)->pSyntaxHdr;
        if(nheaderLen && pHeader)
        {
            /**-----------------------------------------------------------------
               Write Header Synchronously Here
            --------------------------------------------------------------------
            */
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                               "Write Out of band header!!!");

            pFileMux->MUX_write_header((uint32)nStreamType,
                                       (uint32)nheaderLen,
                                       (uint8*)pHeader);
            sStreamInfo[nStreamType].bHeaderReceived = OMX_TRUE;
        }
        else
        {
            /**-----------------------------------------------------------------
              No Syntax Header for video??? Possibly h263
            --------------------------------------------------------------------
            */
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                                               "No Syntax Header for Video!!!");
        }
    }

    if(pBuffHdr->nFilledLen &&
     (!sAVSyncInfo.bVideoStarted || pBuffHdr->nFlags &OMX_BUFFERFLAG_STARTTIME))
    {
        sAVSyncInfo.nVideoStartTime = pBuffHdr->nTimeStamp;
        sAVSyncInfo.bVideoStarted   = OMX_TRUE;
        sAVSyncInfo.bVideoStreamEnded = OMX_FALSE;

        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                                  "Queue Video.. Video Start Time = %ld%ld!!!",
                                  (OMX_U32)(sAVSyncInfo.nVideoStartTime >> 32),
                                  (OMX_U32)(sAVSyncInfo.nVideoStartTime));
    }


    if((  pBuffHdr->nFlags  & OMX_BUFFERFLAG_STARTTIME ||
          pTmpVidBufferHdr  == NULL)
       &&
          (!bEOS))
    {
        /**---------------------------------------------------------------------
         This is the first frame that we are receiving at video port.
         We need to wait for the next frame to come as we need to calculate
         the delta of this frame.
         For ISO14496-12 and derived file formats
         delta_n = tn+1 - tn

         Here we will keep the header temporarily and process when next frame
         comes. We will use a temp pointer to store rather than a queue as
         allocating so many queue elements will result in unncessary heap
         fragmentation.
        ------------------------------------------------------------------------
        */

        if(pTmpVidBufferHdr != NULL)
        {
            /**-----------------------------------------------------------------
             Start time is received with a frame which is not first frame.
            --------------------------------------------------------------------
            */
            return OMX_ErrorUndefined;
        }

        if(nFileFormat == MUX_FMT_MP4)
        {
            pTmpVidBufferHdr = pBuffHdr;

            /**-----------------------------------------------------------------
            Lets not do anything and return here. We will process this later.
            --------------------------------------------------------------------
            */
            return OMX_ErrorNone;
        }
    }
    else if(pTmpVidBufferHdr)
    {
        /**---------------------------------------------------------------------
            We enter here only if pTmpVidBufferHdr is valid, which means we had
            received frames already. If pTmpVidBuffHdr is NULL and still we
            skipped the if case it is an interesting case. User has given EOS
            flah with first frame, which valid in case of post card recording
            like feature.
        ------------------------------------------------------------------------
        */

        /**---------------------------------------------------------------------
          If it is not first frame calcuate frame delta.
        ------------------------------------------------------------------------
        */
        if(pBuffCmd->pBufferHdr->nFilledLen || bEOS)
        {
            pBuffHdr = pTmpVidBufferHdr;

            pTmpVidBufferHdr = pBuffCmd->pBufferHdr;
        }
        else
        {
            /**-----------------------------------------------------------------
               If current frame is empty and has no EOS we need to just return
               it back to client asynchronously. We will retain the previous
               buffer in pTmpVidBufferHdr until the next valid frame comes.
               This scenario can occur very common if encoder drops frames.
            --------------------------------------------------------------------
            */
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                    "Queue Video. Empty Buffer without EOS just return!!!");
            pBuffHdr = pBuffCmd->pBufferHdr;
        }

        /**---------------------------------------------------------------------
           Delta(n) in units of Media Timescale =
                       (T(n+1) - T(n)) * MedTimeScl / 1000;
        ------------------------------------------------------------------------
        */
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                     "OMX_FileMux ETB video StartTime %d %d!!!",
                     (uint32)((uint64)pBuffHdr->nTimeStamp >> 32),
                     (uint32)((uint64)pBuffHdr->nTimeStamp ) );
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                     "OMX_FileMux ETB video EndTime %d %d!!!",
                      (uint32)((uint64)pTmpVidBufferHdr->nTimeStamp >> 32),
                      (uint32)((uint64)pTmpVidBufferHdr->nTimeStamp ) );

        nDelta = (uint32) (GET_SAMPLE_DELTA(
                                  pTmpVidBufferHdr->nTimeStamp,
                                  pBuffHdr->nTimeStamp,
                                  pStreams[nStreamType].media_timescale));

        if((OMX_S32)nDelta < 0)
        {
            /**-----------------------------------------------------------------
             If B frames are involved we will get frames out of order.
             Send negative delta values. Videofmt will convert it.
            --------------------------------------------------------------------
            */
            nDelta = (uint32) (-(GET_SAMPLE_DELTA(
                                  pBuffHdr->nTimeStamp,
                                  pTmpVidBufferHdr->nTimeStamp,
                                  pStreams[nStreamType].media_timescale)));
        }

        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                     "OMX_FileMux ETB video nDelta %ld !!!",nDelta);

        if(bEOS && pTmpVidBufferHdr->nFilledLen == 0)
        {
            /**-----------------------------------------------------------------
             Client has given an empty buffer with EOS. Cannot guarantee
             time is currect in this as this buffer may not be originated from
             source component. Putting a default delta in this case as it does
             no harm as it is last frame to be muxed into clip.
            --------------------------------------------------------------------
            */
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                    "Queue Video. Empty Buffer with EOS!!!");
            nDelta = 0;
        }

    }

    for(int i = 0; i < 2 && pBuffHdr; i++)
    {
        if(pBuffHdr->nFlags & OMX_BUFFERFLAG_EXTRADATA)
        {
            OMX_FileMux_ProcessExtraData(pBuffHdr);
        }

        /**---------------------------------------------------------------------
          If nDelta is 0 let us put default.
        ------------------------------------------------------------------------
        */
        if(nDelta == 0)
        {
            if(GET_VIDEO_FRAMERATE)
            {
                nDelta = pStreams[nStreamType].media_timescale/
                                                       GET_VIDEO_FRAMERATE;
            }
            /**-----------------------------------------------------------------
               Still delta is 0 means many of the required params are
               uninitialized by IL client. Lets forgive and proceed with
               Default delta.
            --------------------------------------------------------------------
            */
            if(!nDelta)
            {
                nDelta = 33;
            }
        }

        nRetStatus = OMX_FileMux_PushSampleToFileMux(nStreamType,
                                                     nDelta,
                                                     pBuffHdr);

        if(nRetStatus != OMX_ErrorNone && nRetStatus != OMX_ErrorNone)
        {
            return OMX_ErrorUndefined;
        }

        /**---------------------------------------------------------------------
          Let us check if we have a EOS in the buffer header that we last
          received. If it is so, then we need to queue it here as we wont get
          another buffer on this port. Let us loop once again. Otherwise we will
          break here;
        ------------------------------------------------------------------------
        */
        /**---------------------------------------------------------------------
          If we are looping here again, that means it is the last buffer. Last
          frame will be written with default delta for 3gp clips as there wont
          be a frame at tn+1. Making nDelta 0 here will trigger default delta.
        ------------------------------------------------------------------------
        */
        nDelta = 0;

        if(!pTmpVidBufferHdr ||
            (!(pTmpVidBufferHdr->nFlags & OMX_BUFFERFLAG_EOS)))
        {
            break;
        }
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                       "Queue Video. Last frame pushed after EOS!!!");

        sAVSyncInfo.bVideoStreamEnded = OMX_TRUE;
        if(sAVSyncInfo.bVideoStreamEnded && sAVSyncInfo.bAudioStreamEnded)
        {
            /**-----------------------------------------------------------------
               Once Audio and Video streams ends while pause we can find any
               difference in audio and video stream duration
            --------------------------------------------------------------------
            */
            if(sAVSyncInfo.nCurrentAudioTime !=
                   sAVSyncInfo.nCurrentVideoTime)
            {
                sAVSyncInfo.nAVTimeDiffAdjust
                             = ((OMX_S64)sAVSyncInfo.nCurrentVideoTime -
                                (OMX_S64)sAVSyncInfo.nCurrentAudioTime);

                MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM,
                  "MMi Mux Pause. total Time Video = %ld, Audio = %ld, Diff = %ld",
                  (OMX_S32)sAVSyncInfo.nCurrentVideoTime,
                  (OMX_S32)sAVSyncInfo.nCurrentAudioTime,
                  (OMX_S32)sAVSyncInfo.nAVTimeDiffAdjust);

            }
            for(int i = 0; i < OMX_MUX_MAX_STREAMS; i++)
            {
                sStreamInfo[i].nCurrDelta = 0;
                sStreamInfo[i].nPrevDelta = 0;
                sStreamInfo[i].nPrevTimeStamp = 0;
                sStreamInfo[i].nStartTime = 0;
            }
            /**-----------------------------------------------------------------
                 Reset AV SYnc Parameters
            --------------------------------------------------------------------
            */
            sAVSyncInfo.bAVSyncDone = OMX_FALSE;
            sAVSyncInfo.nAVTimeDiff = 0;
            sAVSyncInfo.bVideoStarted = OMX_FALSE;
            sAVSyncInfo.bAudioStarted = OMX_FALSE;

        }

        pBuffHdr = pTmpVidBufferHdr;
        pTmpVidBufferHdr = NULL;
    }


    return nRetStatus;
}


/*==============================================================================

         FUNCTION:         OMX_FileMux_QueueAudioStreamBuffer

         DESCRIPTION:
*//**       @brief         This function queues stream buffer to inner layer for
                           decode for audio stream.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_QueueAudioStreamBuffer
(
    OMX_Mux_BufferCmdType *pBuffCmd
)
{
    OMX_BUFFERHEADERTYPE    *pBuffHdr;
    QMM_ListSizeType         nListSize;
    OMX_U32                  nStreamType   = MMI_AUDIO_STREAM_NUM;
    OMX_U32                  nPortIndex    = pBuffCmd->nPortIndex;
    OMX_ERRORTYPE            nRetStatus    = OMX_ErrorUndefined;
    OMX_BOOL                 bAudioPresent = IS_AUDIO_PORT_USED;

    OMX_FileMux_BuffHdrLinkType   *pBuffHdrLink = NULL;
    MUX_stream_create_params_type *pStreams     =
             (MUX_stream_create_params_type*) pFileMuxStreams;
    OMX_BOOL                       bEOS         =
        (pBuffCmd->pBufferHdr->nFlags & OMX_BUFFERFLAG_EOS)?OMX_TRUE:OMX_FALSE;

 //   MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux ETB Audio Called!!!");
    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "OMX_FileMux ETB Audio Called %ld %lld!!!",
                     pBuffCmd->pBufferHdr->nFilledLen, pBuffCmd->pBufferHdr->nTimeStamp);


    if(!bAudioPresent)
    {
        return OMX_ErrorBadParameter;
    }

    pBuffHdr = pBuffCmd->pBufferHdr;

    if(pBuffHdr->nFlags & OMX_BUFFERFLAG_CODECCONFIG)
    {
        /**---------------------------------------------------------------------
           Assuming header is passed correctly in separate buffer.
        ------------------------------------------------------------------------
        */
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                    "Queue Audio. Recevied header from client!!!");
        pFileMux->MUX_write_header((uint32)nStreamType, true,(uint32) pBuffHdr->nFilledLen,
                                   pBuffHdr->pBuffer + pBuffHdr->nOffset,
                                   pBuffHdr);
        pBuffHdr->nFilledLen = 0;

        sStreamInfo[nStreamType].bHeaderReceived = OMX_TRUE;
        return OMX_ErrorNone;
    }

    /**-------------------------------------------------------------------------
      Maintain a copy of audio start time for AV Sync
    ----------------------------------------------------------------------------
    */
    if(pBuffHdr->nFilledLen &&
     (!sAVSyncInfo.bAudioStarted || pBuffHdr->nFlags &OMX_BUFFERFLAG_STARTTIME))
    {
        sAVSyncInfo.nAudioStartTime = pBuffHdr->nTimeStamp;
        sAVSyncInfo.bAudioStarted   = OMX_TRUE;
        sAVSyncInfo.bAudioStreamEnded = OMX_FALSE;


        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
           "Queue Audio.. Audio Start Time = %ld%ld!!!",
            (OMX_U32)(sAVSyncInfo.nAudioStartTime >> 32),
            (OMX_U32)(sAVSyncInfo.nAudioStartTime));
    }


    /**-------------------------------------------------------------------------
        Following portion of code is Part of AV Sync Logic:
        --------------------------------------------------
        If we need to do AV Sync we will not give the buffer to filemux.
        We will hold it in a queue until video start time is known.
        We will trim the number of packets as required in audio stream.

        There are two levels or precision in which we can do AV sync
        according to the following method of data delivery from client.

        1. Single Frame per call:
             We can do avsync with a precision of 20ms for audio and frame
             duration for AAC. This is a fine precision AV sync:

        2. Multiple frame per call:
             a) Constant bitrate speech codec and AAC
             We can do avsync with a precision of 20ms for audio and frame
             duration for AAC. This is a fine precision AV sync:
             b) Variable bitrate speech codecs
             Our precision of AVsync will be the duration of data present
             in the callback as we do not know the frame boundary.
             It is not a supported feature in movie recording as of now.

    ----------------------------------------------------------------------------
    */
    if(sAVSyncInfo.bAVSyncDone)
    {
        /**---------------------------------------------------------------------
            If AV Sync done we will pass the buffers synchronously to FileMux
        ------------------------------------------------------------------------
        */

        if(nPortIndex == OMX_MUX_INDEX_PORT_AUDIO &&
         (AUDIO_COMPRESSION_FORMAT) == OMX_AUDIO_CodingAAC)
        {
            /**-----------------------------------------------------------------
               Strip ADTS header for AAC in movie file formats
            --------------------------------------------------------------------
            */
            if( (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                sFormatSpecificInfo.sAACInfo.eAACStreamFormat ==
                   OMX_AUDIO_AACStreamFormatMP4ADTS &&
                      nFileFormat == MUX_FMT_MP4)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                        "Skipping 7 byte ADTS header!!!");
                pBuffHdr->nOffset += 7;
            }
            /**-----------------------------------------------------------------
              If no separate header received for AAC we can generate if from
              settings. Put the generated one.
            --------------------------------------------------------------------
            */
            if(!sStreamInfo[nStreamType].bHeaderReceived &&
              nFileFormat == MUX_FMT_MP4)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                     "No AAC header received, put the generated AAC header!!!");
                pFileMux->MUX_write_header((uint32)nStreamType, 2,
                                     (uint8*)&nAACStreamHeader[0]);
                sStreamInfo[nStreamType].bHeaderReceived = OMX_TRUE;
            }
        }
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "OMX_FileMux ETB Audio Push %ld %lld!!!",
                     pBuffCmd->pBufferHdr->nFilledLen, pBuffCmd->pBufferHdr->nTimeStamp);
        nRetStatus = OMX_FileMux_PushSampleToFileMux(
                 nStreamType,
                 pStreams[MMI_AUDIO_STREAM_NUM].sample_delta,
                 pBuffHdr);

        if(nRetStatus != OMX_ErrorNone)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                    "Queue Audio. Pushing sample to filemux failed!!!");
            return OMX_ErrorUndefined;
        }
    }
    else if(!sAVSyncInfo.bVideoStarted || !sAVSyncInfo.bAudioStarted)
    {
        if(!pBuffHdr->nFilledLen)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                "OMX_FileMux ETB Audio: Zero Size buffer. return asynchly!!!");
            nRetStatus = OMX_FileMux_PushSampleToFileMux(
                 nStreamType,
                 pStreams[MMI_AUDIO_STREAM_NUM].sample_delta,
                 pBuffHdr);
            return nRetStatus;
        }

        OMX_FileMux_BuffHdrLinkType *pBuffHdrLink;

        pBuffHdrLink = (OMX_FileMux_BuffHdrLinkType *)
                         OMX_FILEMUX_MALLOC(
                             sizeof(OMX_FileMux_BuffHdrLinkType));

        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "OMX_FileMux ETB Audio: Video Not Started Queue Audio!!!");

        if(!pBuffHdrLink)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                "OMX_FileMux ETB Audio: Malloc Fail!!!");
            return OMX_ErrorInsufficientResources;
        }
        /**---------------------------------------------------------------------
          Video has not started. So let us buffer audio until we know the video
          start time and number of audio packets to be dropped.
        ------------------------------------------------------------------------
        */

        pBuffHdrLink->pBuffHdr = pBuffHdr;

        QMM_ListErrorType nListError = qmm_ListPushRear(&
                            (arrPortConfig + nPortIndex)->pBufferQueue,
                            &pBuffHdrLink->pLink);


        if(nListError != QMM_LIST_ERROR_NONE &&
           nListError != QMM_LIST_ERROR_NOT_PRESENT)
        {
            /**-----------------------------------------------------------------
             A failure to queue will jeopardise all AV sync operations.
             So make AVSyncDone to TRUE
            --------------------------------------------------------------------
            */
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                             "OMX_FileMux ETB Audio: AV Sync Fail to queue!!!");

            nRetStatus = OMX_FileMux_PushSampleToFileMux(
                 nStreamType,
                 pStreams[MMI_AUDIO_STREAM_NUM].sample_delta,
                 pBuffHdr);

            OMX_FileMux_DeQueueAudioBuffers(OMX_TRUE, nPortIndex);
            OMX_FILEMUX_FREEIF(pBuffHdrLink);
            sAVSyncInfo.bAVSyncDone = OMX_TRUE;
            sAVSyncInfo.nAVTimeDiffAdjust = 0;

            return nRetStatus;
        }

        qmm_ListSize(&(arrPortConfig + nPortIndex)->pBufferQueue,
                     &nListSize);

        if(bEOS || nListSize >= MAX_AUDIO_QUEUE_LEN)
        {
            /**-----------------------------------------------------------------
                 If we have not received any video frames and still receive an
                 EOS lets ignore AV sync and proceed.

                 If our audio queue is full, even in that case we need to abort
                 any AV sync operation
            --------------------------------------------------------------------
            */
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                             "OMX_FileMux ETB Audio: Calling OMX_FileMux_DeQueueAudioBuffers!!!");

            nRetStatus  = OMX_FileMux_DeQueueAudioBuffers(OMX_FALSE, nPortIndex);
            if(nRetStatus != OMX_ErrorNone)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "Queue Audio. Failed to dequeue audio buffers!!!");
                return OMX_ErrorUndefined;
            }
            sAVSyncInfo.bAVSyncDone = OMX_FALSE;
            sAVSyncInfo.bAudioStarted = OMX_FALSE;
            sAVSyncInfo.nAVTimeDiffAdjust = 0;
            sAVSyncInfo.nAudioStartTime = pBuffHdr->nTimeStamp;
        }
    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Third Else case");
        OMX_BOOL bAudioInsert = OMX_FALSE;

        /**---------------------------------------------------------------------
           Once both audio and video time stamps are available do the following.

           1. Check if there is any adjustment required. Adjustment can be a
            positive or negative value and is the adjustment required for audio
            track compared to video track.

           2. Check if adjusted audio time is leading audio i.e it comes first
            then we need to drop audio packets.

           3. If adjusted audio time is trailing then we need to insert silence
        ------------------------------------------------------------------------
        */

        if(!sAVSyncInfo.nAVTimeDiff)
        {

            if(sAVSyncInfo.nVideoStartTime >
              (sAVSyncInfo.nAudioStartTime + sAVSyncInfo.nAVTimeDiffAdjust))
            {
                /**-------------------------------------------------------------
                  Audio packets to be dropped based on the time diff
                ----------------------------------------------------------------
                */
                sAVSyncInfo.nAVTimeDiff =
                  sAVSyncInfo.nVideoStartTime -
                  (sAVSyncInfo.nAudioStartTime + sAVSyncInfo.nAVTimeDiffAdjust);

                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                                    "MMI Mux AVSYNC. Drop Audio Time = %ld!!!",
                                            (OMX_U32)sAVSyncInfo.nAVTimeDiff );

            }
            else
            {
                /**-------------------------------------------------------------
                  Silent Audio packets to be inserted based on the time diff
                ----------------------------------------------------------------
                */
                sAVSyncInfo.nAVTimeDiff =
                 (sAVSyncInfo.nAudioStartTime + sAVSyncInfo.nAVTimeDiffAdjust) -
                                                 sAVSyncInfo.nVideoStartTime;
                bAudioInsert = OMX_TRUE;

                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                                   "MMI Mux AVSYNC. Insert Audio Time = %lld!!!",
                                            sAVSyncInfo.nAVTimeDiff );

            }
        }
        /**---------------------------------------------------------------------
          Push the current audio buffer to queue as we always perform
          audio packet drops from audio queue.
        ------------------------------------------------------------------------
        */
        pBuffHdrLink = (OMX_FileMux_BuffHdrLinkType *)OMX_FILEMUX_MALLOC(
                             sizeof(OMX_FileMux_BuffHdrLinkType));

        if(!pBuffHdrLink)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "Queue Audio. Malloc fail for queue node!!!");
            return OMX_ErrorInsufficientResources;
        }

        pBuffHdrLink->pBuffHdr = pBuffHdr;

        QMM_ListErrorType nListError = qmm_ListPushRear(&
                            (arrPortConfig + nPortIndex)->pBufferQueue,
                            &pBuffHdrLink->pLink);

        if(nListError != QMM_LIST_ERROR_NONE &&
           nListError != QMM_LIST_ERROR_NOT_PRESENT)
        {
            /**-----------------------------------------------------------------
             A failure to queue will jeopardise all AV sync. So make AVSyncDone
             to TRUE
            --------------------------------------------------------------------
            */
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                         "OMX_FileMux ETB Audio: Fail to queue!!!");

            nRetStatus = OMX_FileMux_PushSampleToFileMux(
                 nStreamType,
                 pStreams[MMI_AUDIO_STREAM_NUM].sample_delta,
                 pBuffHdr);

            OMX_FileMux_DeQueueAudioBuffers(OMX_TRUE, nPortIndex);

            OMX_FILEMUX_FREEIF(pBuffHdrLink);

            sAVSyncInfo.bAVSyncDone = OMX_TRUE;
            sAVSyncInfo.nAVTimeDiffAdjust = 0;

            return nRetStatus;
        }
        if(!sAVSyncInfo.nAVTimeDiff)
        {
            /**-----------------------------------------------------------------
                Audio video start time matches no AVSync.
            --------------------------------------------------------------------
            */
            sAVSyncInfo.bAVSyncDone = OMX_TRUE;
            sAVSyncInfo.nAVTimeDiffAdjust = 0;
            sAVSyncInfo.nAVTimeDiff = 0;
        }
        else if(bAudioInsert == OMX_TRUE)
        {
            /**-----------------------------------------------------------------
               Find the number of packets to be inserted for AVSYNC.
            --------------------------------------------------------------------
            */
            OMX_U32 nNumPacketInsert =
                      OMX_FileMux_AVSyncFindNumAudioPacketsToInsert(nPortIndex);

            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                                   "MMI Mux AVSYNC. Num Packets Insert = %ld!!!",
                                            (OMX_U32)nNumPacketInsert);

            if(nNumPacketInsert)
            {
                OMX_FileMux_AVSyncInsertSilentAudioPackets(nPortIndex,
                                                       nNumPacketInsert);
            }
            sMuxStats.nSilentFramesInserted  += nNumPacketInsert;
            sAVSyncInfo.bAVSyncDone = OMX_TRUE;
            sAVSyncInfo.nAVTimeDiffAdjust = 0;
MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "bAudioInsert: sAVSyncInfo.nAudioStartTime b4 = %lld",sAVSyncInfo.nAudioStartTime );
//sAVSyncInfo.nAudioStartTime -= nNumPacketInsert*10667;
MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "bAudioInsert: sAVSyncInfo.nAudioStartTime After= %lld",sAVSyncInfo.nAudioStartTime );

        }
        else
        {
            nRetStatus = OMX_FileMux_DoAVSyncCoarse(nPortIndex);

if(qmm_ListPeekFront(&(arrPortConfig + nPortIndex)->pBufferQueue,
				  (QMM_ListLinkType **)(&pBuffHdrLink))
		   == QMM_LIST_ERROR_EMPTY_LIST)
{
	MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
							  "Fine Av sync. No buffer to process!!!");
	return OMX_ErrorUndefined;
}
sAVSyncInfo.nAudioStartTime = pBuffHdrLink->pBuffHdr->nTimeStamp;
MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "Final AV SYNC: sAVSyncInfo.nAudioStartTime = %lld",sAVSyncInfo.nAudioStartTime );

            if(nRetStatus == OMX_ErrorNone)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                                "MMI Mux AVSYNC. Coarse Av Sync complete!!!");
                /**-------------------------------------------------------------
                 The first level of AV Sync is done. Start the finer level of
                 AV Sync.
                 Lets not process any error from this function as this looks
                 into the content of the buffer. If client wants to mux a
                 particular content let us not stop it. We will leave any format
                 level error to decoder which decodes the content.
                 So after this function call we assume AVSync is done.
                ----------------------------------------------------------------
                */
                if(sAVSyncInfo.nAVTimeDiff)
                {
                    (void)OMX_FileMux_DoAVSyncFine(nPortIndex);
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                                "MMI Mux AVSYNC. Fine Av Sync complete!!!");
                }

                sAVSyncInfo.bAVSyncDone = OMX_TRUE;
                sAVSyncInfo.nAVTimeDiffAdjust = 0;
            }
            else if(nRetStatus != OMX_ErrorNone)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "Queue Audio. Do Avsync course failed!!!");
                return OMX_ErrorUndefined;
            }
            /**-----------------------------------------------------------------
                If return from first level AV Sync is OMX_ErrorNone,
                we do not have enough buffers to do AVsync. Wait for more
                buffers.
            --------------------------------------------------------------------
            */
        }
        if(sAVSyncInfo.bAVSyncDone || bEOS)
        {
            /**-----------------------------------------------------------------
              Process the buffers that are held for AV SYnc
            --------------------------------------------------------------------
            */
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "sAVSyncInfo.bAVSyncDone || bEOS");
            nRetStatus = OMX_FileMux_DeQueueAudioBuffers(OMX_TRUE, nPortIndex);
            if(nRetStatus != OMX_ErrorNone)
            {
                return OMX_ErrorUndefined;
            }
        }
    }
    if(bEOS)
    {
        sAVSyncInfo.bAudioStreamEnded = OMX_TRUE;
        if(sAVSyncInfo.bVideoStreamEnded && sAVSyncInfo.bAudioStreamEnded)
        {
            /**-----------------------------------------------------------------
               Once Audio and Video streams ends while pause we can find any
               difference in audio and video stream duration
            --------------------------------------------------------------------
            */
            if(sAVSyncInfo.nCurrentAudioTime !=
                   sAVSyncInfo.nCurrentVideoTime)
            {
                sAVSyncInfo.nAVTimeDiffAdjust
                             = ((OMX_S64)sAVSyncInfo.nCurrentVideoTime -
                                (OMX_S64)sAVSyncInfo.nCurrentAudioTime);

                MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM,
                  "MMi Mux Pause. total Time Video = %ld, Audio = %ld, Diff = %ld",
                  (OMX_S32)sAVSyncInfo.nCurrentVideoTime,
                  (OMX_S32)sAVSyncInfo.nCurrentAudioTime,
                  (OMX_S32)sAVSyncInfo.nAVTimeDiffAdjust);

            }
            for(int i = 0; i < OMX_MUX_MAX_STREAMS; i++)
            {
                sStreamInfo[i].nCurrDelta = 0;
                sStreamInfo[i].nPrevDelta = 0;
                sStreamInfo[i].nPrevTimeStamp = 0;
                sStreamInfo[i].nStartTime = 0;
                sAVSyncInfo.bVideoStarted = OMX_FALSE;
                sAVSyncInfo.bAudioStarted = OMX_FALSE;
            }
            /**-----------------------------------------------------------------
                 Reset AV SYnc Parameters
            --------------------------------------------------------------------
            */
            sAVSyncInfo.bAVSyncDone = OMX_FALSE;
            sAVSyncInfo.nAVTimeDiff = 0;
        }
    }

    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_DeQueueAudioBuffers

         DESCRIPTION:
*//**       @brief         This function queues stream buffer to inner layer for
                           decode for audio stream.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         bProcess: OMX_TRUE if buffers need to be sent to
                                               filemu
                                     OMX_FALSE if buffers need to be just
                                               returned.

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_DeQueueAudioBuffers
(
    OMX_BOOL bProcess,
    OMX_U32  nPortIndex
)
{
    OMX_U32 nStreamType         = MMI_AUDIO_STREAM_NUM;
    int                           nRetStatus = OMX_ErrorUndefined;
    OMX_FileMux_BuffHdrLinkType  *pBuffHdrLink = NULL;
    MUX_stream_create_params_type *pStreams =
             (MUX_stream_create_params_type*) pFileMuxStreams;

    while(qmm_ListPopFront(&(arrPortConfig + nPortIndex)->pBufferQueue,
                      (QMM_ListLinkType **)(&pBuffHdrLink))
          != QMM_LIST_ERROR_EMPTY_LIST)
    {
        if(bProcess)
        {
            /**-----------------------------------------------------------------
              Send the sample for processing
            --------------------------------------------------------------------
            */

            /**-----------------------------------------------------------------
               Strip ADTS header for AAC in movie file formats
            --------------------------------------------------------------------
            */
            if(nPortIndex == OMX_MUX_INDEX_PORT_AUDIO &&
            (AUDIO_COMPRESSION_FORMAT) == OMX_AUDIO_CodingAAC)
            {
                if( (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                     sFormatSpecificInfo.sAACInfo.eAACStreamFormat
                                 == OMX_AUDIO_AACStreamFormatMP4ADTS
                 &&
                     nFileFormat == MUX_FMT_MP4)
                {
                    /**---------------------------------------------------------
                      If the delevery method for AAC packets is in ADTS format
                      We have to skip the header bytes before putting into
                      3gp container. For .aac we have store as such
                    ------------------------------------------------------------
                    */
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                        "Skipping 7 byte ADTS header!!!");
                    pBuffHdrLink->pBuffHdr->nOffset += 7;
                }

                if(!sStreamInfo[nStreamType].bHeaderReceived &&
                   nFileFormat == MUX_FMT_MP4)
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                    "No AAC header received, put the generated AAC header!!!");
                    /**---------------------------------------------------------
                      If by the time we receive aac frames we have not
                      received any header we can send the generated header from
                      settings. This will work for AAC LC only.
                    ------------------------------------------------------------
                    */
                    pFileMux->MUX_write_header((uint32)nStreamType, 2,
                                     (uint8*)&nAACStreamHeader[0]);

                    sStreamInfo[nStreamType].bHeaderReceived = OMX_TRUE;
                }
            }

            nRetStatus = OMX_FileMux_PushSampleToFileMux(
                     nStreamType,
                     pStreams[nStreamType].sample_delta,
                     pBuffHdrLink->pBuffHdr);

            OMX_FILEMUX_FREEIF(pBuffHdrLink);

            if(nRetStatus != OMX_ErrorNone)
            {
                return OMX_ErrorUndefined;
            }
        }
        else
        {
            /**-----------------------------------------------------------------
              Push sample with 0 bytes and 0 delta
            --------------------------------------------------------------------
            */
            pBuffHdrLink->pBuffHdr->nFilledLen = 0;
            nRetStatus = OMX_FileMux_PushSampleToFileMux(
                     nStreamType,
                     0,
                     pBuffHdrLink->pBuffHdr);
            if(nRetStatus != OMX_ErrorNone)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "DeQueue Audio. Push sample to filemux failed!!!");
                return OMX_ErrorUndefined;
            }
        }
    }
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_PushSampleToFileMux

         DESCRIPTION:
*//**       @brief         This function queues stream buffer to inner layer for
                           muxing.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_PushSampleToFileMux
(
    OMX_U32                 nStreamNum,
    OMX_U32                 nDelta,
    OMX_BUFFERHEADERTYPE   *pBuffHdr
)
{

    MUX_sample_info_type          *pSampleInfo;
    MUX_STATUS                     nStatus;
    MUX_stream_create_params_type *pStreams =
             (MUX_stream_create_params_type*) pFileMuxStreams;

    if(pBuffHdr->nFlags & OMX_BUFFERFLAG_EXTRADATA)
    {
        (void)OMX_FileMux_ProcessExtraData(pBuffHdr);
    }

    /**-------------------------------------------------------------------------
     Allocate Sample Info structure, this will be freed in callback
    ----------------------------------------------------------------------------
    */
    pSampleInfo = (MUX_sample_info_type*)
                         OMX_FILEMUX_MALLOC(sizeof(MUX_sample_info_type));

    if(!pSampleInfo)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "PushSampleToFilemux. Failed to alloc sample info!!!");
        return OMX_ErrorInsufficientResources;
    }

    OMX_MUX_MEM_SET (pSampleInfo,0,sizeof(MUX_sample_info_type));
    if(pBuffHdr->nFilledLen >= pBuffHdr->nOffset)
    {
        pSampleInfo->size  = (uint32)(pBuffHdr->nFilledLen - pBuffHdr->nOffset);
    }
    else
    {
        pSampleInfo->size  = 0;
    }
    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "OMX_FileMux ETB About to push %d %ld!!!",
                     pSampleInfo->size, pBuffHdr->nOffset);
    pSampleInfo->delta  = (uint32)nDelta;


    if(m_FmtPvtInfo.nExtraSize != 0)
    {
        pSampleInfo->fmt_pvtdata_offset = (uint32)m_FmtPvtInfo.nExtraOffset;
        pSampleInfo->fmt_pvtdata_size   = (uint32)m_FmtPvtInfo.nExtraSize;
        pSampleInfo->fmt_pvtdata_ptr    = m_FmtPvtInfo.pExtra;
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
        "OMX_FileMux: FmtPvt Info ExtraData Offset (%lu) DataPtr(%p)",
        m_FmtPvtInfo.nExtraOffset,m_FmtPvtInfo.pExtra);
        m_FmtPvtInfo.nExtraSize = 0;
        m_FmtPvtInfo.nExtraOffset = 0;
    }


    if(m_HDCPInfo.nExtraSize != 0)
    {
        pSampleInfo->extra_data_offset = (uint32)m_HDCPInfo.nExtraOffset;
        pSampleInfo->extra_data_size   = (uint32)m_HDCPInfo.nExtraSize;
        pSampleInfo->extra_data_ptr    = m_HDCPInfo.pExtra;
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
        "OMX_FileMux: ExtraData Offset (%lu) DataPtr(%p)",
        m_HDCPInfo.nExtraOffset,m_HDCPInfo.pExtra);
        m_HDCPInfo.nExtraSize = 0;
        m_HDCPInfo.nExtraOffset = 0;
    }

    if(pStreams[nStreamNum].type == MUX_STREAM_VIDEO)
    {
        if(nFileFormat == MUX_FMT_MP2)
        {
            pSampleInfo->time  = (uint32)(((uint64)pBuffHdr->nTimeStamp + 500)/1000);
        }
        else
        {
            pSampleInfo->time  = (uint32)(GET_SAMPLE_DELTA(
                                  pBuffHdr->nTimeStamp,
                                  sAVSyncInfo.nVideoStartTime,
                                  pStreams[nStreamNum].media_timescale));
        }
    }
    else if (pStreams[nStreamNum].type == MUX_STREAM_AUDIO)
    {
        if(nFileFormat == MUX_FMT_MP2)
        {
            pSampleInfo->time  = (uint32)(((uint64)pBuffHdr->nTimeStamp)/1000);
        }
        else
        {
            pSampleInfo->time  = (uint32)(GET_SAMPLE_DELTA(
                                  pBuffHdr->nTimeStamp,
                                  sAVSyncInfo.nAudioStartTime,
                                  pStreams[nStreamNum].media_timescale));
        }
MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM, "OMX_FileMux ETB sAVSyncInfo.nAudioStartTime,pBuffHdr->nTimeStamp,pSampleInfo->time %lld %lld %d!!!",
    sAVSyncInfo.nAudioStartTime,pBuffHdr->nTimeStamp,pSampleInfo->time);
MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "OMX_FileMux ETB pStreams[nStreamNum].media_timescale = %d!",
pStreams[nStreamNum].media_timescale);
    }
    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "OMX_FileMux ETB Audio About to push time = %d %ld!!!",
                     pSampleInfo->time, pBuffHdr->nOffset);
    if(pBuffHdr->nFlags & OMX_BUFFERFLAG_SYNCFRAME)
    {
        pSampleInfo->sync = TRUE;
    }

    /**-------------------------------------------------------------------------
         Pass the buffer to FileMux layer.
    ----------------------------------------------------------------------------
    */
    nStatus = pFileMux->MUX_Process_Sample((uint32)nStreamNum,
                             1,
                             pSampleInfo,
                             pBuffHdr->pBuffer + pBuffHdr->nOffset,
                             (void*)pBuffHdr);


    if(nStatus != MUX_SUCCESS)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "Pushsample to filemux. Failed to process sample!!!");
        OMX_FILEMUX_FREEIF(pSampleInfo);
        return OMX_ErrorUndefined;
    }
    /**-------------------------------------------------------------------------
      Keep Track of Audio and video time stamps. This will help in compensating
      any difference in audio and video duration during pause resume.
    ----------------------------------------------------------------------------
    */
    if(pStreams[nStreamNum].type == MUX_STREAM_VIDEO )
    {
        if(pSampleInfo->size)
        {
            sAVSyncInfo.nCurrentVideoTime +=
                            (pSampleInfo->delta * 1000000 +
                            (pStreams[nStreamNum].media_timescale >> 1) )  /
                             pStreams[nStreamNum].media_timescale;

            sMuxStats.nVideoDuration +=
                            (pSampleInfo->delta * 1000000 +
                            (pStreams[nStreamNum].media_timescale >> 1) )  /
                             pStreams[nStreamNum].media_timescale;
            sMuxStats.nNumVidFramesWritten++;

            sMuxStats.nVideoBytes += pSampleInfo->size;
        }
        else
        {
            sMuxStats.nNumVidFramesDropped++;
        }
    }
    else if(pStreams[nStreamNum].type == MUX_STREAM_AUDIO)
    {
        if(pSampleInfo->size)
        {
            sAVSyncInfo.nCurrentAudioTime +=
                            (pSampleInfo->delta * 1000000 +
                            (pStreams[nStreamNum].media_timescale >> 1) )  /
                             pStreams[nStreamNum].media_timescale;

            sMuxStats.nAudioDuration +=
                            (pSampleInfo->delta * 1000000 +
                            (pStreams[nStreamNum].media_timescale >> 1) )  /
                             pStreams[nStreamNum].media_timescale;
            sMuxStats.nNumAudFramesWritten++;

            sMuxStats.nAudioBytes += pSampleInfo->size;
        }
        else
        {
            sMuxStats.nNumAudFramesDropped++;
        }
    }

    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_DoAVSyncCoarse

         DESCRIPTION:
*//**       @brief         This function does AVsync at a higher level. Drops
                           buffer headers and return it back, if required.
                           However this does not look into the content of'the
                           buffer.

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         nPortIndex

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_DoAVSyncCoarse
(
    OMX_U32  nPortIndex
)
{
    OMX_U64 nTimeDelta;
    QMM_ListSizeType nListSize = 0;
    OMX_FileMux_BuffHdrLinkType  *pBuffHdrLink = NULL;
    OMX_FileMux_BuffHdrLinkType  *pBuffHdrLinkBase = NULL;

    if(qmm_ListPopFront(&(arrPortConfig + nPortIndex)->pBufferQueue,
                      (QMM_ListLinkType **)(&pBuffHdrLinkBase))
          == QMM_LIST_ERROR_EMPTY_LIST)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                               "Coarse Av sync. No buffers Yet! Wait!!!");
        return OMX_ErrorNone;
    }
    qmm_ListSize(&(arrPortConfig + nPortIndex)->pBufferQueue,
                     &nListSize);
    if(nListSize == 0)
    {
        /**---------------------------------------------------------------------
         We need at least two buffers to do coarse AVsync, requeue it back
        ------------------------------------------------------------------------
        */
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                       "Coarse Av sync. Only one buffer! Requeue and wait!!!");
        qmm_ListPushFront(&(arrPortConfig + nPortIndex)->pBufferQueue,
                             (QMM_ListLinkType *)pBuffHdrLinkBase);
        return OMX_ErrorNone;
    }

    while(qmm_ListPopFront(&(arrPortConfig + nPortIndex)->pBufferQueue,
                      (QMM_ListLinkType **)(&pBuffHdrLink))
          != QMM_LIST_ERROR_EMPTY_LIST)
    {

        nTimeDelta = (OMX_U64)(pBuffHdrLink->pBuffHdr->nTimeStamp -
                               pBuffHdrLinkBase->pBuffHdr->nTimeStamp);

        if((int64)nTimeDelta > (int64)sAVSyncInfo.nAVTimeDiff)
        {
            /**-----------------------------------------------------------------
             First buffer itself holds more data than we need to do AVSync.
             Lets put them back to queue and proceed.
            --------------------------------------------------------------------
            */
            qmm_ListPushFront(&(arrPortConfig + nPortIndex)->pBufferQueue,
                              (QMM_ListLinkType *)pBuffHdrLink);
            qmm_ListPushFront(&(arrPortConfig + nPortIndex)->pBufferQueue,
                              (QMM_ListLinkType *)pBuffHdrLinkBase);
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                  "Coarse Av sync. AV Sync complete!!!");
            return OMX_ErrorNone;
        }
        else if(nTimeDelta == sAVSyncInfo.nAVTimeDiff)
        {
            qmm_ListPushFront(&(arrPortConfig + nPortIndex)->pBufferQueue,
                              (QMM_ListLinkType *)pBuffHdrLink);
            /**-----------------------------------------------------------------
                 The first buffer completes the AYSYnc.Send the buffer taken as
                 base .here to filemux with 0 size and 0 delta so that this
                 does not make any contribution to movie file.
            --------------------------------------------------------------------
            */
            pBuffHdrLinkBase->pBuffHdr->nFilledLen = 0;
            OMX_FileMux_PushSampleToFileMux(
                                  MMI_AUDIO_STREAM_NUM,
                                  0, /* Pass nDelta as 0 */
                                  pBuffHdrLinkBase->pBuffHdr);

            OMX_FILEMUX_FREEIF(pBuffHdrLinkBase);
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                  "Coarse Av sync. AV Sync complete!!!");
            sAVSyncInfo.nAVTimeDiff = 0;
            return OMX_ErrorNone;
        }
        else
        {
            /**-----------------------------------------------------------------
                 We need more buffers to do AVSync. Send the buffer taken as
                 base .here to filemux with 0 size and 0 delta so that this
                 does not make any contribution to movie file.
            --------------------------------------------------------------------
            */
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                  "MMi Mux AVSYNC. Drop Audio Frame!!!");
            pBuffHdrLinkBase->pBuffHdr->nFilledLen = 0;
            OMX_FileMux_PushSampleToFileMux(
                                  MMI_AUDIO_STREAM_NUM,
                                  0, /* Pass nDelta as 0 */
                                  pBuffHdrLinkBase->pBuffHdr);
            OMX_FILEMUX_FREEIF(pBuffHdrLinkBase);
            pBuffHdrLinkBase = pBuffHdrLink;
            sAVSyncInfo.nAVTimeDiff -= nTimeDelta;
        }
    }
    /**-------------------------------------------------------------------------
      We have run out of buffers while doing AV SYnc. Let us wait for more
      buffers. Push the last popped buffer back to queue.
    ----------------------------------------------------------------------------
    */
    qmm_ListPushFront(&(arrPortConfig + nPortIndex)->pBufferQueue,
                      (QMM_ListLinkType *)pBuffHdrLink);
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_DoAVSyncFine

         DESCRIPTION:
*//**       @brief         This function does AVsync at a buffer level. Drops
                           frames as required from within a buffer. This
                           function can perform only when frames are of fixed
                           delta and samples are of fixed size.

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         nPortIndex
                           nBuffHdr

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_DoAVSyncFine
(
    OMX_U32               nPortIndex
)
{
    OMX_U32 nFrameSize;
    OMX_U32 nFrameDelta;
    OMX_U32 nFrameDeltaMs;
    OMX_U32 nBytesDrop;

    MUX_stream_create_params_type *pStreams =
             (MUX_stream_create_params_type*) pFileMuxStreams;
    OMX_FileMux_BuffHdrLinkType  *pBuffHdrLink = NULL;

    nFrameSize  = pStreams->sample_size;
    nFrameDelta = pStreams->sample_delta;

    if(!nFrameSize || !nFrameDelta)
    {
        /**---------------------------------------------------------------------
         For AAC when samples are delivered as ADTS samples there is a 7byte
         header attached to each sample from where we can extract individual
         frame boundary and delta. So to see if the format we are handling
         has this flexibility, try calling the function that handles these kind
         of samples. Currently only ADTS is the supported format.
        ------------------------------------------------------------------------
        */
        OMX_FileMux_DoAVSyncFineADTS(nPortIndex);

        /**---------------------------------------------------------------------
          Cannot do further AV Sync as we do not know the frame boundaries
          in size and time units within the buffer. Return as complete as
          this is the best that we can do.
        ------------------------------------------------------------------------
        */
        sAVSyncInfo.nAVTimeDiff = 0;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                  "Fine Av sync. ADTS AV Sync complete!!!");
        return OMX_ErrorNone;
    }

    if(qmm_ListPeekFront(&(arrPortConfig + nPortIndex)->pBufferQueue,
                      (QMM_ListLinkType **)(&pBuffHdrLink))
               == QMM_LIST_ERROR_EMPTY_LIST)
    {
        /**---------------------------------------------------------------------
          We cannot do fine AVSync without a buffer. Error!!!!
        ------------------------------------------------------------------------
        */
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                  "Fine Av sync. No buffer to process!!!");
        return OMX_ErrorUndefined;
    }


    nFrameDeltaMs = (nFrameDelta * 1000)/ pStreams->media_timescale;

    nBytesDrop    = (OMX_U32)(nFrameSize * ((sAVSyncInfo.nAVTimeDiff/1000) /
                                                              nFrameDeltaMs));

    pBuffHdrLink->pBuffHdr->nOffset += nBytesDrop;

    sAVSyncInfo.nAVTimeDiff = 0;

    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_DoAVSyncFineADTS

         DESCRIPTION:
*//**       @brief         This function does AVsync at a buffer level. Drops
                           frames as required from within a buffer. This
                           function can perform only for AAC frames send
                           in ADTS format.

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         nPortIndex
                           nBuffHdr

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_DoAVSyncFineADTS
(
    OMX_U32               nPortIndex
)
{
    OMX_U32 nFrameSize;
    OMX_U32 nFrameDeltaMs;
    OMX_U32 nFrameDelta;
    OMX_U32 nNumFramesToDrop;
    OMX_U32 nBytesDrop = 0;
    OMX_U32 nSampFreq;
    OMX_U8  *pBuffCurrLoc = NULL;

    MUX_stream_create_params_type *pStreams =
             (MUX_stream_create_params_type*) pFileMuxStreams;
    OMX_FileMux_BuffHdrLinkType  *pBuffHdrLink = NULL;

    if((AUDIO_COMPRESSION_FORMAT) != OMX_AUDIO_CodingAAC ||
        (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                sFormatSpecificInfo.sAACInfo.eAACStreamFormat !=
                 OMX_AUDIO_AACStreamFormatMP4ADTS)
    {
        /**---------------------------------------------------------------------
             Not AAC return
        ------------------------------------------------------------------------
        */
        return OMX_ErrorNone;
    }
    if(qmm_ListPeekFront(&(arrPortConfig + nPortIndex)->pBufferQueue,
                      (QMM_ListLinkType **)(&pBuffHdrLink))
               == QMM_LIST_ERROR_EMPTY_LIST)
    {
        /**---------------------------------------------------------------------
          We cannot do fine AVSync without a buffer. Error!!!!
        ------------------------------------------------------------------------
        */
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                  "Fine Av syncADTS. No buffer to process!!!");
        return OMX_ErrorUndefined;
    }

    pBuffCurrLoc = pBuffHdrLink->pBuffHdr->pBuffer +
                      pBuffHdrLink->pBuffHdr->nOffset;

    nFrameDelta = pStreams->sample_delta;

    if(!nFrameDelta)
    {
        nFrameDelta = AAC_SAMPLES_PER_FRAME;
    }


    /**-------------------------------------------------------------------------
        Extract sampling frequency index from stream. We will convert this
        to sample frequency and find the delta.
    ----------------------------------------------------------------------------
    */
    nSampFreq = nAACSamplingRates[((pBuffCurrLoc [2] >> 2) & 0x0F)];
    if(0 == nSampFreq)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                            "Fine Av sync ADTS. No Sampling frequency    !!!");
        return OMX_ErrorUndefined;
    }

    nFrameDeltaMs = (nFrameDelta * 1000)/ nSampFreq;

    nNumFramesToDrop = (OMX_U32)
         ((sAVSyncInfo.nAVTimeDiff / 1000)/nFrameDeltaMs);

    while( nNumFramesToDrop-- )
    {
        /**---------------------------------------------------------------------
             Validate ADTS sync word.
        ------------------------------------------------------------------------
        */
        if ((pBuffCurrLoc[0] != 0xFF) ||
             ((pBuffCurrLoc[1] & 0xF0) != 0xF0) ||
                 ((pBuffCurrLoc[1] & 0x06) != 0x00))
        {
            return OMX_ErrorUndefined;
        }
        /**---------------------------------------------------------------------
         Extract frame length information from the buffer.
        ------------------------------------------------------------------------
        */
        nFrameSize = ((pBuffCurrLoc [3] & 0x03) << 11) |
                        ((pBuffCurrLoc [4]) << 3) |
                        ((pBuffCurrLoc [5] & 0xE0) >> 5);

        if(nFrameSize > (pBuffHdrLink->pBuffHdr->nFilledLen -
                                 pBuffHdrLink->pBuffHdr->nOffset))
        {
            /**-----------------------------------------------------------------
              Something wrong in parsing.. Failed!!!!!!
            --------------------------------------------------------------------
            */
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                  "Fine Av sync. ADTS header corrupt!!!");
            return OMX_ErrorUndefined;
        }
        nBytesDrop += nFrameSize;
        pBuffCurrLoc += nFrameSize;
    }

    pBuffHdrLink->pBuffHdr->nOffset += nBytesDrop;

    sAVSyncInfo.nAVTimeDiff = 0;
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_ProcessExtraData

         DESCRIPTION:
*//**       @brief         Process any extra data at the end of buffer.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         OMX_BUFFERHEADERTYPE

*//*     RETURN VALUE:
*//**       @return
                           OMX_ErrorNone or appropriate error


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_ProcessExtraData
(
    OMX_BUFFERHEADERTYPE   *pBuffHdr
)
{
    OMX_OTHER_EXTRADATATYPE *pExtra;
    OMX_U8 *pTmp = pBuffHdr->pBuffer +
                       pBuffHdr->nOffset + pBuffHdr->nFilledLen + 3;
    MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_MEDIUM,
                "Mux ExtraData pTmp[%p] + 3 pBuffHdr->pBuffer[%p] nFilledLen[%ld] nOffset[%ld]" ,
                pTmp,pBuffHdr->pBuffer,
                pBuffHdr->nFilledLen,
                pBuffHdr->nOffset);

    /**-------------------------------------------------------------------------
     Clear HDCP info to receive new info if any coming as extradata
    ----------------------------------------------------------------------------
    */
    OMX_MUX_MEM_SET(&m_HDCPInfo, 0, sizeof(m_HDCPInfo));

    /**-------------------------------------------------------------------------
     Clear FmtPvtInfo info to receive new info if any coming as extradata
    ----------------------------------------------------------------------------
    */
    OMX_MUX_MEM_SET(&m_FmtPvtInfo, 0 ,sizeof(m_FmtPvtInfo));

    pExtra = (OMX_OTHER_EXTRADATATYPE *) ((reinterpret_cast<long>( pTmp)) & ~3);

    if(((size_t)(pBuffHdr->pBuffer + pBuffHdr->nAllocLen -
         (OMX_U8*)pExtra)  <  sizeof(OMX_OTHER_EXTRADATATYPE))
       || (((reinterpret_cast<long>(&pExtra->data[0])) + pExtra->nDataSize - 1)
             - reinterpret_cast<long>(pBuffHdr->pBuffer)) >  pBuffHdr->nAllocLen)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                      "Mux ExtraData flag Set, but no data!!!");
        return OMX_ErrorNone;
    }
    while(pExtra->eType != OMX_ExtraDataNone)
    {
        /**---------------------------------------------------------------------
          If there is multi slice info and the codec is AVC we can use this
          to replace start codes with NAL lengths as required in 3gp files.
          This is not of any use for MPEG4 as of now.
        ------------------------------------------------------------------------
        */
        if(pExtra->eType == (OMX_EXTRADATATYPE)QOMX_ExtraDataVideoMultiSliceInfo)
        {
            if(pExtra->nSize > sizeof(OMX_OTHER_EXTRADATATYPE) &&
                VIDEO_COMPRESSION_FORMAT == OMX_VIDEO_CodingAVC )
            {
                OMX_FileMux_ReplaceAVCStartCodes(pBuffHdr, pExtra);
            }
            break;
        }

        if(pExtra->eType == (OMX_EXTRADATATYPE)QOMX_ExtraDataHDCPEncryptionInfo)
        {
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
            "OMX_FileMux_ProcessExtraData pExtra->nSize[%ld] ExtraDataStruct Size:%d",
            pExtra->nSize,sizeof(OMX_OTHER_EXTRADATATYPE));
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,"nSize[%ld] nDataSize[%ld]",
            pExtra->nSize, pExtra->nDataSize);
          if(pExtra->nSize > sizeof(OMX_OTHER_EXTRADATATYPE))
          {
            m_HDCPInfo.nExtraSize = pExtra->nDataSize;
            m_HDCPInfo.nExtraOffset = (OMX_U32)(
            reinterpret_cast<long>(pExtra) + sizeof(OMX_OTHER_EXTRADATATYPE)-4
              - reinterpret_cast<long>(pBuffHdr->pBuffer));

            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
              "OMX_FileMux_ProcessExtraData nDataSize[%ld] nExtraOffset[%ld]",
              m_HDCPInfo.nExtraSize,m_HDCPInfo.nExtraOffset);

            m_HDCPInfo.pExtra = (OMX_U8*)(
            reinterpret_cast<long>(pExtra) + sizeof(OMX_OTHER_EXTRADATATYPE) -4);
          }
        }
        if(pExtra->eType == OMX_ExtraDataMax)
        {
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
            "OMX_FileMux_ProcessExtraData pExtra->nSize[%ld] ExtraDataStruct Size:%d",
            pExtra->nSize,sizeof(OMX_OTHER_EXTRADATATYPE));
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,"nSize[%ld] nDataSize[%ld]",
            pExtra->nSize, pExtra->nDataSize);
          if(pExtra->nSize > sizeof(OMX_OTHER_EXTRADATATYPE))
          {
            m_FmtPvtInfo.nExtraSize = pExtra->nDataSize;
            m_FmtPvtInfo.nExtraOffset = (OMX_U32)(
            reinterpret_cast<long>(pExtra) + sizeof(OMX_OTHER_EXTRADATATYPE)-4
             - reinterpret_cast<long>(pBuffHdr->pBuffer));

            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
              "OMX_FileMux_ProcessExtraData nDataSize[%ld] nExtraOffset[%ld]",
              m_FmtPvtInfo.nExtraSize,m_FmtPvtInfo.nExtraOffset);

            m_FmtPvtInfo.pExtra = (OMX_U8*)(
              pExtra + sizeof(OMX_OTHER_EXTRADATATYPE) -4);
          }
        }

        pExtra = (OMX_OTHER_EXTRADATATYPE *)
                 ((reinterpret_cast<OMX_U8 *>(pExtra)) + pExtra->nSize);

      if(((size_t)(pBuffHdr->pBuffer + pBuffHdr->nAllocLen -
            (OMX_U8*)pExtra)  <   sizeof(OMX_OTHER_EXTRADATATYPE))
         || (((reinterpret_cast<long>((&pExtra->data[0])) + pExtra->nDataSize - 1) -
             reinterpret_cast<long>(pBuffHdr->pBuffer)) > pBuffHdr->nAllocLen))
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                          "Mux ExtraData overflows buffer boundary!!!");
            return OMX_ErrorNone;
        }
    }
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_ReplaceAVCStartCodes

         DESCRIPTION:
*//**       @brief         Replace NAL startcodes with NAL size for h264.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         OMX_BUFFERHEADERTYPE

*//*     RETURN VALUE:
*//**       @return
                           OMX_ErrorNone or appropriate error


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_ReplaceAVCStartCodes
(
    OMX_BUFFERHEADERTYPE    *pBuffHdr,
    OMX_OTHER_EXTRADATATYPE *pExtra
)
{

    OMX_U32 nnumsliceentries;
    OMX_U32 nnumslices;
    OMX_U32 *psliceinfoptr;
    ExtraDataPtr tempPtr;
    OMX_U8  *pdataptr = pBuffHdr->pBuffer + pBuffHdr->nOffset;
    OMX_U32 nslicesize = 0;
    OMX_U32 nfilledlen = pBuffHdr->nFilledLen;


    if(pExtra->eType != (OMX_EXTRADATATYPE)QOMX_ExtraDataVideoMultiSliceInfo)
    {
        /**---------------------------------------------------------------------
          For replacing start codes extra data MUST be slice info.
        ------------------------------------------------------------------------
        */
        return OMX_ErrorUndefined;
    }
    tempPtr.dataPtr = &pExtra->data[0];
    psliceinfoptr = (OMX_U32*)(tempPtr.dataPtr);
    nnumsliceentries = *psliceinfoptr++;
    nnumslices = nnumsliceentries;


    /**-------------------------------------------------------------------------
     Replace all the start codes with the corresponding size. Make boundary
     checks all the while.
    ----------------------------------------------------------------------------
    */
    while(nnumslices--)
    {
        pdataptr = pBuffHdr->pBuffer + *psliceinfoptr++;
        nslicesize = *psliceinfoptr++;
        /**---------------------------------------------------------------------
          Replace start code with NAL length in big endian
        ------------------------------------------------------------------------
        */
        MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_ERROR,
            "Sliceinfo pdataptr = %p, slice offset = %d nslicesize = %ld !!!",
             pdataptr, (pdataptr - pBuffHdr->pBuffer), nslicesize);
        if(nslicesize <= nfilledlen &&
          (pdataptr + 3) < (pBuffHdr->pBuffer + pBuffHdr->nAllocLen))
        {
            if(*(OMX_U32*)pdataptr != 0x01000000)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                        "Sliceinfo not matching with data!!!");
                return OMX_ErrorUndefined;
            }
            pdataptr[0] = (OMX_U8)((nslicesize - 4) >> 24);
            pdataptr[1] = (OMX_U8)(((nslicesize - 4) >> 16) & 0xff);
            pdataptr[2] = (OMX_U8)(((nslicesize - 4) >> 8 ) & 0xff);
            pdataptr[3] = (OMX_U8)(((nslicesize - 4)     ) & 0xff);
        }
        else
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                        "Sliceoffset beyond buffer!!!");
            return OMX_ErrorUndefined;
        }
    }
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_AVSyncFindNumAudioPacketsToInsert

         DESCRIPTION:
*//**       @brief         This function finds the number of auido packets
                           of silence to insert.

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         nPortIndex

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*============================================================================
*/
OMX_U32 OMX_FileMux::OMX_FileMux_AVSyncFindNumAudioPacketsToInsert
(
    OMX_U32  nPortIndex
)
{
    (void) nPortIndex;
    MUX_stream_create_params_type *pstreams =
             (MUX_stream_create_params_type*) pFileMuxStreams;

    OMX_U32 nframedelta = pstreams[MMI_AUDIO_STREAM_NUM].sample_delta;
    OMX_U32 ntimescale  = pstreams[MMI_AUDIO_STREAM_NUM].media_timescale;

    /**-------------------------------------------------------------------------
      Calculate the number of silent frames that would be required to
      compensate for delay.

      Num of frames required = total AV time diff / time per frame.

    ----------------------------------------------------------------------------
    */

    if(!ntimescale)
    {
        if((AUDIO_COMPRESSION_FORMAT) == OMX_AUDIO_CodingAAC)
        {
            ntimescale = (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                    sFormatSpecificInfo.sAACInfo.nBitRate;

        }
        else
        {
            ntimescale = VOCODER_SAMPLING_RATE;
        }
        /**---------------------------------------------------------------------
           It is a big risk to make approximations. If client has not set
           all properties. Lets return as 0
        ------------------------------------------------------------------------
        */
        if(!ntimescale)
        {
            return 0;
        }
    }

    OMX_U32 nframedeltams = (nframedelta * 1000)/ ntimescale;

    if(!nframedeltams)
    {
        return 0;
    }

    /**-------------------------------------------------------------------------
       lets round up as the occurance of audio earlier than corresponding
       video is more annoying than the opposite case.
    ----------------------------------------------------------------------------
    */
    OMX_U32 nframestoinsert = (OMX_U32) ((sAVSyncInfo.nAVTimeDiff /1000 +
                                                          (nframedeltams >> 1))
                                                          /nframedeltams);
    /**-------------------------------------------------------------------------
       Keep an upper cap for max AV sync duration.
    ----------------------------------------------------------------------------
    */
    nframestoinsert = MIN((MAX_AVSYNC_SILENCE_DURATION/nframedeltams),
                           nframestoinsert);

    return nframestoinsert;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_AVSyncInsertSilentAudioPackets

         DESCRIPTION:
*//**       @brief         Insert the desired amount of silent packets for doing
                           AVSync.

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         nPortIndex

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*============================================================================
*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_AVSyncInsertSilentAudioPackets
(
    OMX_U32  nportindex,
    OMX_U32  npacketinsert
)
{
    (void)nportindex;
    OMX_U8 *paudiobuffer                       = NULL;
    OMX_U32 naudiobufferdata                   = 0;
    MUX_STATUS nretstatus                      = MUX_FAIL;
    OMX_U32  ndelta                            = 0;
    MUX_sample_info_type          *psampleinfo = NULL;
    MUX_stream_create_params_type *pstreams    =
             (MUX_stream_create_params_type*) pFileMuxStreams;
    /**-------------------------------------------------------------------------
      Each audio format has its own silence packet. Lets put them into the clip
    ----------------------------------------------------------------------------
    */
    switch(AUDIO_COMPRESSION_FORMAT)
    {
        case OMX_AUDIO_CodingAAC:
        {
            /**-----------------------------------------------------------------
              Each sample rate channel mode has a silent frame associated
            --------------------------------------------------------------------
            */
            OMX_U32 nsamplerate  =
                          (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                                  sFormatSpecificInfo.sAACInfo.nSampleRate;
            OMX_AUDIO_CHANNELMODETYPE nchannelmode =
                          (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                                  sFormatSpecificInfo.sAACInfo.eChannelMode;

            if(nchannelmode != OMX_AUDIO_ChannelModeStereo &&
                nchannelmode != OMX_AUDIO_ChannelModeMono)
            {
                /**-------------------------------------------------------------
                  Only Stereo and mono supported
                ----------------------------------------------------------------
                */
                return OMX_ErrorNone;
            }
            switch(nsamplerate)

            {
                case  96000: /*TBD*/
                case  88200: /*TBD*/
                case  64000: /*TBD*/
                    break;
                case  48000:
                    if( nchannelmode == OMX_AUDIO_ChannelModeStereo)
                    {
                        paudiobuffer = AAC48Kstereosilence;
                        naudiobufferdata = sizeof(AAC48Kstereosilence);
                    }
                    else if(nchannelmode == OMX_AUDIO_ChannelModeMono)
                    {
                        paudiobuffer = AAC48Kmonosilence;
                        naudiobufferdata = sizeof(AAC48Kmonosilence);
                    }
                    break;
                case  44100:
                    if( nchannelmode == OMX_AUDIO_ChannelModeStereo)
                    {
                        paudiobuffer = AAC44Kstereosilence;
                        naudiobufferdata = sizeof(AAC44Kstereosilence);
                    }
                    else if(nchannelmode == OMX_AUDIO_ChannelModeMono)
                    {
                        paudiobuffer = AAC44Kmonosilence;
                        naudiobufferdata = sizeof(AAC44Kmonosilence);
                    }
                    break;
                case  32000:
                    if( nchannelmode == OMX_AUDIO_ChannelModeStereo)
                    {
                        paudiobuffer = AAC32Kstereosilence;
                        naudiobufferdata = sizeof(AAC32Kstereosilence);
                    }
                    else if(nchannelmode == OMX_AUDIO_ChannelModeMono)
                    {
                        paudiobuffer = AAC32Kmonosilence;
                        naudiobufferdata = sizeof(AAC32Kmonosilence);
                    }
                    break;
                case  24000:
                    if( nchannelmode == OMX_AUDIO_ChannelModeStereo)
                    {
                        paudiobuffer = AAC24Kstereosilence;
                        naudiobufferdata = sizeof(AAC24Kstereosilence);
                    }
                    else if(nchannelmode == OMX_AUDIO_ChannelModeMono)
                    {
                        paudiobuffer = AAC24Kmonosilence;
                        naudiobufferdata = sizeof(AAC24Kmonosilence);
                    }
                    break;
                case  22050:
                    if( nchannelmode == OMX_AUDIO_ChannelModeStereo)
                    {
                        paudiobuffer = AAC22Kstereosilence;
                        naudiobufferdata = sizeof(AAC22Kstereosilence);
                    }
                    else if(nchannelmode == OMX_AUDIO_ChannelModeMono)
                    {
                        paudiobuffer = AAC22Kmonosilence;
                        naudiobufferdata = sizeof(AAC22Kmonosilence);
                    }
                    break;
                case  16000:
                    if( nchannelmode == OMX_AUDIO_ChannelModeStereo)
                    {
                        paudiobuffer = AAC16Kstereosilence;
                        naudiobufferdata = sizeof(AAC16Kstereosilence);
                    }
                    else if(nchannelmode == OMX_AUDIO_ChannelModeMono)
                    {
                        paudiobuffer = AAC16Kmonosilence;
                        naudiobufferdata = sizeof(AAC16Kmonosilence);
                    }
                    break;
                case  12000:
                    if( nchannelmode == OMX_AUDIO_ChannelModeStereo)
                    {
                        paudiobuffer = AAC12Kstereosilence;
                        naudiobufferdata = sizeof(AAC12Kstereosilence);
                    }
                    else if(nchannelmode == OMX_AUDIO_ChannelModeMono)
                    {
                        paudiobuffer = AAC12Kmonosilence;
                        naudiobufferdata = sizeof(AAC12Kmonosilence);
                    }
                    break;
                case  11025:
                    if( nchannelmode == OMX_AUDIO_ChannelModeStereo)
                    {
                        paudiobuffer = AAC11Kstereosilence;
                        naudiobufferdata = sizeof(AAC11Kstereosilence);
                    }
                    else if(nchannelmode == OMX_AUDIO_ChannelModeMono)
                    {
                        paudiobuffer = AAC11Kmonosilence;
                        naudiobufferdata = sizeof(AAC11Kmonosilence);
                    }
                    break;
                case  8000 :
                    if( nchannelmode == OMX_AUDIO_ChannelModeStereo)
                    {
                        paudiobuffer = AAC8Kstereosilence;
                        naudiobufferdata = sizeof(AAC8Kstereosilence);
                    }
                    else if(nchannelmode == OMX_AUDIO_ChannelModeMono)
                    {
                        paudiobuffer = AAC8Kmonosilence;
                        naudiobufferdata = sizeof(AAC8Kmonosilence);
                    }
                    break;
                default:
                    break;
            }

            ndelta = pstreams[MMI_AUDIO_STREAM_NUM].sample_delta;
            break;
        }


        /**---------------------------------------------------------------------
          AMR has a one byte indicator for silence
        ------------------------------------------------------------------------
        */
        case OMX_AUDIO_CodingAMR:
            paudiobuffer = AMR_SILENT_FRAME_DATA;
            naudiobufferdata = sizeof(AMR_SILENT_FRAME_DATA);
            ndelta = pstreams[MMI_AUDIO_STREAM_NUM].sample_delta;
            break;


        /**---------------------------------------------------------------------
          Inserting a NULL byte with a valid delta produces desired
          amountof silence for QCELP and EVRC
        ------------------------------------------------------------------------
        */
        case OMX_AUDIO_CodingEVRC:
        case OMX_AUDIO_CodingQCELP13:
        case OMX_AUDIO_CodingQCELP8:
            paudiobuffer = QCELP_EVRC_SILENT_FRAME_DATA;
            naudiobufferdata = 1;
            ndelta = pstreams[MMI_AUDIO_STREAM_NUM].sample_delta;
            break;
/*	case OMX_AUDIO_CodingPCM:
	    paudiobuffer = PCM48KHzsilence;
            naudiobufferdata = sizeof(PCM48KHzsilence);
            ndelta = pstreams[MMI_AUDIO_STREAM_NUM].sample_delta;
            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux_AVSyncInsertSilentAudioPackets Inserting silence PCMs =ndelta,naudiobufferdata =%d %d", ndelta,naudiobufferdata);
*/
        default:
            break;
    }
    /**-------------------------------------------------------------------------
      If we have a valid silence frame we can queue it to filemux layer.
      Else notify AVsync is complete
    ----------------------------------------------------------------------------
    */
    if(paudiobuffer && naudiobufferdata)
    {
        for(OMX_U32 i = 0; i < npacketinsert; i++)
        {
            psampleinfo = (MUX_sample_info_type*)
                            OMX_FILEMUX_MALLOC(sizeof(MUX_sample_info_type));
            if(!psampleinfo)
            {
                return OMX_ErrorInsufficientResources;
            }

            memset(psampleinfo,0,sizeof(MUX_sample_info_type));

            psampleinfo->size = (uint32)naudiobufferdata;
            psampleinfo->delta = (uint32)ndelta;
            psampleinfo->sync = true;
            psampleinfo->offset = 0;

            nretstatus = pFileMux->MUX_Process_Sample(
                             MMI_AUDIO_STREAM_NUM,
                             1,
                             psampleinfo,
                             paudiobuffer,
                             (void*)NULL);

            if(nretstatus != MUX_SUCCESS)
            {
                break;
            }
            /**-----------------------------------------------------------------
                Update total duration with silence frames added
            --------------------------------------------------------------------
            */
            sAVSyncInfo.nCurrentAudioTime +=
                    (psampleinfo->delta * 1000000 +
                    (pstreams[MMI_AUDIO_STREAM_NUM].media_timescale >> 1) )  /
                     pstreams[MMI_AUDIO_STREAM_NUM].media_timescale;
MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux_AVSyncInsertSilentAudioPackets Changing audio start time to =%lld",  sAVSyncInfo.nCurrentAudioTime);

        }
    }
    return OMX_ErrorNone;

}


/*==============================================================================

         FUNCTION:         OMX_FileMux_SetMediaParams

         DESCRIPTION:
*//**       @brief         Set the codec specific params for audio and video.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ErrorNone or appropriate error


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_ERRORTYPE OMX_FileMux::OMX_FileMux_SetMediaParams
(
    OMX_INDEXTYPE eMediaIndex,
    void *pSrcStruct,
    void *pDstStruct
)
{
    if(NULL == pSrcStruct || NULL == pDstStruct)
    {
        return OMX_ErrorUndefined;
    }
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux_SetMediaParams eMediaIndex = %d", eMediaIndex);
    switch((OMX_U32)eMediaIndex)
    {
        case  QOMX_FilemuxIndexParamAudioAmrWbPlus:
        {
            QOMX_AUDIO_PARAM_AMRWBPLUSTYPE *pSrc =
                           (QOMX_AUDIO_PARAM_AMRWBPLUSTYPE*)pSrcStruct;
            QOMX_AUDIO_PARAM_AMRWBPLUSTYPE *pDst =
                           (QOMX_AUDIO_PARAM_AMRWBPLUSTYPE*)pDstStruct;
            pDst->eAMRBandMode    = pSrc->eAMRBandMode;
            pDst->eAMRDTXMode     = pSrc->eAMRDTXMode;
            pDst->eAMRFrameFormat = pSrc->eAMRFrameFormat;
            pDst->nBitRate        = pSrc->nBitRate;
            pDst->nChannels       = pSrc->nChannels;
            pDst->nSampleRate     = pSrc->nSampleRate;
            return OMX_ErrorNone;
        }
        case OMX_IndexParamAudioAmr:
        {
            OMX_AUDIO_PARAM_AMRTYPE *pSrc = (OMX_AUDIO_PARAM_AMRTYPE*)pSrcStruct;
            OMX_AUDIO_PARAM_AMRTYPE *pDst = (OMX_AUDIO_PARAM_AMRTYPE*)pDstStruct;
            pDst->eAMRBandMode    = pSrc->eAMRBandMode;
            pDst->eAMRDTXMode     = pSrc->eAMRDTXMode;
            pDst->eAMRFrameFormat = pSrc->eAMRFrameFormat;
            pDst->nBitRate        = pSrc->nBitRate;
            pDst->nChannels       = pSrc->nChannels;
            return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioPcm:
        {
            OMX_AUDIO_PARAM_PCMMODETYPE *pSrc =
                                        (OMX_AUDIO_PARAM_PCMMODETYPE*)pSrcStruct;
            OMX_AUDIO_PARAM_PCMMODETYPE *pDst =
                                        (OMX_AUDIO_PARAM_PCMMODETYPE*)pDstStruct;

            pDst->bInterleaved = pSrc->bInterleaved;
            memcpy(&pDst->eChannelMapping,
                   pSrc->eChannelMapping,
                   sizeof(OMX_AUDIO_CHANNELTYPE) * OMX_AUDIO_MAXCHANNELS);
            pDst->eEndian       = pSrc->eEndian;
            pDst->eNumData      = pSrc->eNumData;
            pDst->ePCMMode      = pSrc->ePCMMode;
            pDst->nBitPerSample = pSrc->nBitPerSample;
            pDst->nChannels     = pSrc->nChannels;
            pDst->nSamplingRate = pSrc->nSamplingRate;
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "OMX_FileMux_SetMediaParams PCM = %ld", pDst->nSamplingRate);
            return OMX_ErrorNone;
        }
        case OMX_IndexParamAudioQcelp13:
        {
           OMX_AUDIO_PARAM_QCELP13TYPE *pSrc =
                                         (OMX_AUDIO_PARAM_QCELP13TYPE*)pSrcStruct;
           OMX_AUDIO_PARAM_QCELP13TYPE *pDst =
                                         (OMX_AUDIO_PARAM_QCELP13TYPE*)pDstStruct;

           pDst->eCDMARate       = pSrc->eCDMARate;
           pDst->nChannels       = pSrc->nChannels;
           pDst->nMaxBitRate     = pSrc->nMaxBitRate;
           pDst->nMinBitRate     = pSrc->nMinBitRate;
           return OMX_ErrorNone;
        }

        case OMX_IndexParamAudioEvrc:
        case QOMX_FilemuxIndexParamAudioEvrcb:
        case QOMX_FilemuxIndexParamAudioEvrcwb:
        {
            OMX_AUDIO_PARAM_EVRCTYPE *pSrc =
                         (OMX_AUDIO_PARAM_EVRCTYPE*)pSrcStruct;

            OMX_AUDIO_PARAM_EVRCTYPE *pDst =
                         (OMX_AUDIO_PARAM_EVRCTYPE*)pDstStruct;

            pDst->bHiPassFilter    = pSrc->bHiPassFilter;
            pDst->bNoiseSuppressor = pSrc->bNoiseSuppressor;
            pDst->bPostFilter      = pSrc->bPostFilter;
            pDst->bRATE_REDUCon    = pSrc->bRATE_REDUCon;
            pDst->eCDMARate        = pSrc->eCDMARate;
            pDst->nChannels        = pSrc->nChannels;
            pDst->nMaxBitRate      = pSrc->nMaxBitRate;
            pDst->nMinBitRate      = pSrc->nMinBitRate;
            return OMX_ErrorNone;
        }


        case OMX_IndexParamAudioAac:
        {
            OMX_AUDIO_PARAM_AACPROFILETYPE *pSrc =
                                   (OMX_AUDIO_PARAM_AACPROFILETYPE*)pSrcStruct;
            OMX_AUDIO_PARAM_AACPROFILETYPE *pDst =
                                   (OMX_AUDIO_PARAM_AACPROFILETYPE*)pDstStruct;

            pDst->eAACProfile      = pSrc->eAACProfile;
            pDst->eAACStreamFormat = pSrc->eAACStreamFormat;
            pDst->eChannelMode     = pSrc->eChannelMode;
            pDst->nAACERtools      = pSrc->nAACERtools;
            pDst->nAACtools        = pSrc->nAACtools;
            pDst->nAudioBandWidth  = pSrc->nAudioBandWidth;
            pDst->nBitRate         = pSrc->nBitRate;
            pDst->nChannels        = pSrc->nChannels;
            pDst->nFrameLength     = pSrc->nFrameLength;
            pDst->nSampleRate      = pSrc->nSampleRate;
            return OMX_ErrorNone;
        }

        case OMX_IndexParamVideoAvc:
        {
            OMX_VIDEO_PARAM_AVCTYPE *pSrc =
                                        (OMX_VIDEO_PARAM_AVCTYPE*)pSrcStruct;
            OMX_VIDEO_PARAM_AVCTYPE *pDst =
                                        (OMX_VIDEO_PARAM_AVCTYPE*)pDstStruct;

            /* TBD can we use memcpy, only thing is portIndex size and version
            will be overwritten*/
            pDst->bconstIpred               = pSrc->bconstIpred;
            pDst->bDirect8x8Inference       = pSrc->bDirect8x8Inference;
            pDst->bDirectSpatialTemporal    = pSrc->bDirectSpatialTemporal;
            pDst->bEnableASO                = pSrc->bEnableASO;
            pDst->bEnableFMO                = pSrc->bEnableFMO;
            pDst->bEnableRS                 = pSrc->bEnableRS;
            pDst->bEnableUEP                = pSrc->bEnableUEP;
            pDst->bEntropyCodingCABAC       = pSrc->bEntropyCodingCABAC;
            pDst->bFrameMBsOnly             = pSrc->bFrameMBsOnly;
            pDst->bMBAFF                    = pSrc->bMBAFF;
            pDst->bUseHadamard              = pSrc->bUseHadamard;
            pDst->bWeightedPPrediction      = pSrc->bWeightedPPrediction;
            pDst->eLevel                    = pSrc->eLevel;
            pDst->eLoopFilterMode           = pSrc->eLoopFilterMode;
            pDst->eProfile                  = pSrc->eProfile;
            pDst->nAllowedPictureTypes      = pSrc->nAllowedPictureTypes;
            pDst->nBFrames                  = pSrc->nBFrames;
            pDst->nCabacInitIdc             = pSrc->nCabacInitIdc;
            pDst->nPFrames                  = pSrc->nPFrames;
            pDst->nRefFrames                = pSrc->nRefFrames;
            pDst->nRefIdx10ActiveMinus1     = pSrc->nRefIdx10ActiveMinus1;
            pDst->nRefIdx11ActiveMinus1     = pSrc->nRefIdx11ActiveMinus1;
            pDst->nSliceHeaderSpacing       = pSrc->nSliceHeaderSpacing;
            pDst->nWeightedBipredicitonMode = pSrc->nWeightedBipredicitonMode;
            return OMX_ErrorNone;
        }


        case OMX_IndexParamVideoH263:
        {
            OMX_VIDEO_PARAM_H263TYPE *pSrc =
                                       (OMX_VIDEO_PARAM_H263TYPE*)pSrcStruct;
            OMX_VIDEO_PARAM_H263TYPE *pDst =
                                       (OMX_VIDEO_PARAM_H263TYPE*)pDstStruct;

            pDst->bForceRoundingTypeToZero = pSrc->bForceRoundingTypeToZero;
            pDst->bPLUSPTYPEAllowed        = pSrc->bPLUSPTYPEAllowed;
            pDst->eLevel                   = pSrc->eLevel;
            pDst->eProfile                 = pSrc->eProfile;
            pDst->nAllowedPictureTypes     = pSrc->nAllowedPictureTypes;
            pDst->nBFrames                 = pSrc->nBFrames;
            pDst->nGOBHeaderInterval       = pSrc->nGOBHeaderInterval;
            pDst->nPFrames                 = pSrc->nPFrames;
            pDst->nPictureHeaderRepetition = pSrc->nPictureHeaderRepetition;

            return OMX_ErrorNone;
        }


        case QOMX_IndexParamAudioAc3:
        {
            QOMX_AUDIO_PARAM_AC3TYPE *pSrc =
                                       (QOMX_AUDIO_PARAM_AC3TYPE*)pSrcStruct;
            QOMX_AUDIO_PARAM_AC3TYPE *pDst =
                                       (QOMX_AUDIO_PARAM_AC3TYPE*)pDstStruct;

            pDst->bCompressionOn           = pSrc->bCompressionOn;
            pDst->bDelaySurroundChannels   = pSrc->bDelaySurroundChannels;
            pDst->bLfeOn                   = pSrc->bLfeOn;
            pDst->eChannelConfig           = pSrc->eChannelConfig;
            pDst->eFormat                  = pSrc->eFormat;
            pDst->nBitRate                 = pSrc->nBitRate;
            pDst->nChannels                = pSrc->nChannels;
            pDst->nProgramID               = pSrc->nProgramID;
            pDst->nSamplingRate            = pSrc->nSamplingRate;

            return OMX_ErrorNone;
        }


        case OMX_IndexParamVideoMpeg4:
        {
            OMX_VIDEO_PARAM_MPEG4TYPE *pSrc =
                                      (OMX_VIDEO_PARAM_MPEG4TYPE*)pSrcStruct;
            OMX_VIDEO_PARAM_MPEG4TYPE *pDst =
                                      (OMX_VIDEO_PARAM_MPEG4TYPE*)pDstStruct;

            pDst->bACPred              = pSrc->bACPred;
            pDst->bGov                 = pSrc->bGov;
            pDst->bReversibleVLC       = pSrc->bReversibleVLC;
            pDst->bSVH                 = pSrc->bSVH;
            pDst->eLevel               = pSrc->eLevel;
            pDst->eProfile             = pSrc->eProfile;
            pDst->nAllowedPictureTypes = pSrc->nAllowedPictureTypes;
            pDst->nBFrames             = pSrc->nBFrames;
            pDst->nHeaderExtension     = pSrc->nHeaderExtension;
            pDst->nIDCVLCThreshold     = pSrc->nIDCVLCThreshold;
            pDst->nMaxPacketSize       = pSrc->nMaxPacketSize;
            pDst->nPFrames             = pSrc->nPFrames;
            pDst->nSliceHeaderSpacing  = pSrc->nSliceHeaderSpacing;
            pDst->nTimeIncRes          = pSrc->nTimeIncRes;
            return OMX_ErrorNone;
        }

        default:
            return OMX_ErrorUnsupportedIndex;
    }
}



/*==============================================================================

         FUNCTION:         OMX_FileMux_ValidateFileFormat

         DESCRIPTION:
*//**       @brief         This function validates audio video codec support
                           with the file brand set.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_BOOL OMX_FileMux::OMX_FileMux_ValidateFileFormat(
        QOMX_CONTAINER_FORMATTYPE fileFormat,
        OMX_AUDIO_CODINGTYPE audioCoding,
        OMX_VIDEO_CODINGTYPE videoCoding
)
{
    OMX_BOOL err = OMX_TRUE;

    OMX_BOOL bVideoCompatible =
         (OMX_VIDEO_CodingUnused == videoCoding) ? OMX_TRUE : OMX_FALSE;
    OMX_BOOL bAudioCompatible =
         (OMX_AUDIO_CodingUnused == audioCoding) ? OMX_TRUE : OMX_FALSE;
    /**-------------------------------------------------------------------------
       When it comes to codec combinations used with varios file formats we
       need to see if the client has followed standards.
       For eg. .mp4 ideally is the MPEG4 part-12/14 and can carry only mpeg
       codecs. Whereas 3gp can carry GSM codecs like AMR and h263
       .mp4- mpeg4,avc,aac
       .3gp- .mp4 + h263 + amr
       .3g2,skm,k3g = .3gp + qcp,evrc
       .amc - folows kddi MM spec. MPEG4,aac,qcp
    ----------------------------------------------------------------------------
    */
    switch (fileFormat)
    {
        case QOMX_FORMAT_3GP:
            if ((!bVideoCompatible) &&
                ((OMX_VIDEO_CodingH263 == videoCoding) ||
                 (OMX_VIDEO_CodingAVC == videoCoding) ||
                 (OMX_VIDEO_CodingMPEG4 == videoCoding)))
            {
                bVideoCompatible = OMX_TRUE;
            }
            if ((!bAudioCompatible) &&
                ((OMX_AUDIO_CodingAMR == audioCoding) ||
                 (OMX_AUDIO_CodingAAC == audioCoding)))
            {
                bAudioCompatible = OMX_TRUE;
            }
            break;
        case QOMX_FORMAT_3G2:
        case QOMX_FORMAT_SKM:
        case QOMX_FORMAT_K3G:
            if ((!bAudioCompatible) &&
                ((OMX_AUDIO_CodingAMR == audioCoding) ||
                 (OMX_AUDIO_CodingEVRC == audioCoding)))
            {
                bAudioCompatible = OMX_TRUE;
            }

        case QOMX_FORMAT_MP4:
            if ((!bVideoCompatible) && (OMX_VIDEO_CodingAVC == videoCoding))
            {
                bVideoCompatible = OMX_TRUE;
            }

        case QOMX_FORMAT_AMC:
            if ((!bAudioCompatible) &&
                ((OMX_AUDIO_CodingQCELP13 == audioCoding) ||
                (OMX_AUDIO_CodingAAC  == audioCoding)))
            {
                bAudioCompatible = OMX_TRUE;
            }
            if ((!bVideoCompatible) &&
                          ((OMX_VIDEO_CodingH263 == videoCoding) ||
                                     (OMX_VIDEO_CodingMPEG4 == videoCoding)))
            {
                bVideoCompatible = OMX_TRUE;
            }
        default:
            break;
    }

    err = (bAudioCompatible && bVideoCompatible) ? OMX_TRUE : OMX_FALSE;
    return (err);
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_ConvertOMXBrandToFileMuxBrand

         DESCRIPTION:
*//**       @brief         This function converts file brand enums from OMX type
                           to file mux type.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           Mux_brand_type


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
int OMX_FileMux::OMX_FileMux_ConvertOMXBrandToFileMuxBrand()
{

   switch (nFileBrand)
    {
        case QOMX_FORMAT_3G2:
        {
            /**-----------------------------------------------------------------
               If Fragment duration is set need to record fragment 3gp. TODO
            --------------------------------------------------------------------
            */
            if (nFragmentDuration)
            {
                return (int)MUX_BRAND_FRAG_3G2;
            }
            else
                return (int)MUX_BRAND_3G2;
        }
        case QOMX_FORMAT_3GP:
            return (int)MUX_BRAND_3GP;

        case QOMX_FORMAT_SKM:
            return (int)MUX_BRAND_SKM;

        case QOMX_FORMAT_K3G:
            return (int)MUX_BRAND_K3G;

        case QOMX_FORMAT_MP4:
            return (int)MUX_BRAND_MP4;

        case QOMX_FORMAT_AMC:
            return (int)MUX_BRAND_AMC;

        case QOMX_FORMAT_AAC:
            /**-----------------------------------------------------------------
               ADIF and ADTS are only supported for AAC.
            --------------------------------------------------------------------
            */
            if((arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                sFormatSpecificInfo.sAACInfo.eAACStreamFormat ==
                                   OMX_AUDIO_AACStreamFormatMP4ADTS)
            {
                return (int)MUX_BRAND_ADTS;
            }
            else if((arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                sFormatSpecificInfo.sAACInfo.eAACStreamFormat ==
                                      OMX_AUDIO_AACStreamFormatADIF)
            {
                return (int)MUX_BRAND_ADIF; //TBD
            }
            else
                return (int)MUX_BRAND_INVALID;

        case QOMX_FORMAT_EVRC:
            return (int)MUX_BRAND_EVRC; //TBD

        case QOMX_FORMAT_AMR:
            /**-----------------------------------------------------------------
                   Detect AMR NB/WB.
            --------------------------------------------------------------------
            */
            if((arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                sFormatSpecificInfo.sAMRWBInfo.eAMRBandMode <=
                                         OMX_AUDIO_AMRBandModeNB7)
            {
                return (int)MUX_BRAND_AMRNB;
            }
            else
            {
                return (int)MUX_BRAND_AMRNB;//TBD
            }
        case QOMX_FORMAT_QCP:
        {
            if(AUDIO_COMPRESSION_FORMAT == OMX_AUDIO_CodingEVRC)
            {
                return (int)MUX_BRAND_EVRC;
            }
            OMX_AUDIO_PARAM_QCELP13TYPE *pQCELPInfo;
            pQCELPInfo = &(arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                                             sFormatSpecificInfo.sQCELPInfo;
            /**-----------------------------------------------------------------
                   Identify QCP fixed full/half or var full/half
            --------------------------------------------------------------------
            */
            if(pQCELPInfo->eCDMARate == OMX_AUDIO_CDMARateFull)
            {
                if(MUX_QCP_RATE_FULL == pQCELPInfo->nMaxBitRate &&
                   MUX_QCP_RATE_FULL == pQCELPInfo->nMinBitRate)
                {
                    return (int)MUX_BRAND_QCELP13K_FIXED_FULL_RATE;
                }
                else
                {
                    return (int)MUX_BRAND_QCELP13K_VAR_FULL_RATE;
                }
            }
            else if(pQCELPInfo->eCDMARate == OMX_AUDIO_CDMARateHalf)
            {
                if(MUX_QCP_RATE_HALF == pQCELPInfo->nMaxBitRate &&
                   MUX_QCP_RATE_HALF == pQCELPInfo->nMinBitRate)
                {
                    return (int)MUX_BRAND_QCELP13K_FIXED_HALF_RATE;
                }
                else
                {
                    return (int)MUX_BRAND_QCELP13K_VAR_FULL_RATE;
                }
            }
            else
                return MUX_BRAND_INVALID;
        }

        case QOMX_FORMAT_WAVE:
          {
            MUX_brand_type brand_type;

            if((arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                sFormatSpecificInfo.sPCMInfo.ePCMMode == OMX_AUDIO_PCMModeALaw)
            {
              brand_type = MUX_BRAND_PCMALaw;
            }
            else if((arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                sFormatSpecificInfo.sPCMInfo.ePCMMode == OMX_AUDIO_PCMModeMULaw)
            {
              brand_type =MUX_BRAND_PCMMULaw;
            }
            else
            {
              brand_type = MUX_BRAND_PCMLINEAR;
            }
            return (int)brand_type;
          }

        case QOMX_FORMATMPEG_TS:
            return (int)MUX_BRAND_MP2TS;

        default:
            return (int)MUX_BRAND_INVALID;
   }

}

/*==============================================================================

         FUNCTION:         OMX_FileMux_DriverCallback

         DESCRIPTION:
*//**       @brief         Registered callback with File Mux layer
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
void OMX_FileMux::OMX_FileMux_DriverCallback
(
    int   status,                    /**< Status from FileMux layer below */
    void *pClientData,               /**< MMI interface instance data     */
    void *pData,                     /**< Sample info that is processed   */
    void *pBuffer                    /**< Buffer to be released           */
)

{
    OMX_FileMux *pThis = (OMX_FileMux*)pClientData;

    if(!pThis)
    {
        /**---------------------------------------------------------------------
          We cant do anything much here. So lets return
        ------------------------------------------------------------------------
        */
        return;
    }

    pThis->OMX_FileMux_ProcessFileMuxCb(status,
                                        pClientData,
                                        pData,
                                        pBuffer);
    return;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_ProcessFileMuxCb

         DESCRIPTION:
*//**       @brief         Process callback from FileMux driverlayer
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

void OMX_FileMux::OMX_FileMux_ProcessFileMuxCb
(
    int   status,
    void *pClientData,
    void *pData,
    void *pBufferHdr
)
{
    (void) pClientData;
    MUX_sample_info_type  *pSampleInfo;
    unsigned int   nEvtCode = 0;
    OMX_ERRORTYPE   nEvtStatus = OMX_ErrorNone;
    unsigned int   nPayloadLen = 0;
    void          *pEvtData = NULL;


    if(status == CLOSE_MUX_COMPLETE || status == CLOSE_MUX_FAIL ||
        status == FLUSH_COMPLETED || status == FLUSH_FAILED ||
        (arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO)->bDisableRequested)
    {
        /**---------------------------------------------------------------------
          Before returning for flush or stop we need to return any buffer
          that we are holding in MMI layer.
        ------------------------------------------------------------------------
        */
        if(pTmpVidBufferHdr)
        {
            OMX_FileMux_ProcessFileMuxCb(PROCESS_SAMPLE_FLUSH,
                                      this,
                                      NULL,
                                      pTmpVidBufferHdr);
        }
        pTmpVidBufferHdr = NULL;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                  "FileMux callback. Flush cached buffer!!!");
    }

    switch (status)
    {
        case PROCESS_SAMPLE_COMPLETE:
        case PROCESS_SAMPLE_FAIL:
        case PROCESS_HEADER_COMPLETE:
        case PROCESS_HEADER_FAIL:
        case SPACE_LIMIT_REACHED:
        case WRITE_FAILED:
        {
            pSampleInfo = (MUX_sample_info_type*)pData;
            OMX_FILEMUX_FREEIF(pData);

            if(!pBufferHdr)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                  "FileMux callback. Flush cached buffer!!!");
                /**-------------------------------------------------------------
                   This is an internally generated process sample call
                   No need to ack to client
                ----------------------------------------------------------------
                */
                return;
            }
            if (pFileMux)
            {
                uint64 nTime = 0;
                pFileMux->MUX_Get_Current_PTS(&nTime);
                ((OMX_BUFFERHEADERTYPE*)pBufferHdr)->nTimeStamp = nTime;
            }
            /**-----------------------------------------------------------------
               Let client know we have completely consumed the buffer.
            --------------------------------------------------------------------
            */

            if(status == PROCESS_SAMPLE_COMPLETE)
            {
                ((OMX_BUFFERHEADERTYPE*)pBufferHdr)->nFilledLen = 0;
                if(((OMX_BUFFERHEADERTYPE*)pBufferHdr)->nInputPortIndex
                                                    == OMX_MUX_INDEX_PORT_VIDEO)
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                   "OMX_FileMux_ProcessFileMuxCb Process Video frame complete");
                }
                else if(((OMX_BUFFERHEADERTYPE*)pBufferHdr)->nInputPortIndex
                                                    == OMX_MUX_INDEX_PORT_AUDIO)
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                   "OMX_FileMux_ProcessFileMuxCb Process Audio frame complete");
                }
                m_pCallbacks->EmptyBufferDone(m_hSelf, m_pAppData,
                                    (OMX_BUFFERHEADERTYPE*)pBufferHdr);
            }

            if(status == PROCESS_HEADER_COMPLETE)
            {
                ((OMX_BUFFERHEADERTYPE*)pBufferHdr)->nFilledLen = 0;

                if(((OMX_BUFFERHEADERTYPE*)pBufferHdr)->nInputPortIndex
                                                    == OMX_MUX_INDEX_PORT_VIDEO)
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                   "OMX_FileMux_ProcessFileMuxCb Process Video Hdr complete");
                }
                else if(((OMX_BUFFERHEADERTYPE*)pBufferHdr)->nInputPortIndex
                                                    == OMX_MUX_INDEX_PORT_AUDIO)
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                   "OMX_FileMux_ProcessFileMuxCb Process Audio Hdr complete");
                }
                m_pCallbacks->EmptyBufferDone(m_hSelf, m_pAppData,
                                      (OMX_BUFFERHEADERTYPE*)pBufferHdr);
            }

            if((status == PROCESS_SAMPLE_FAIL) || (status == PROCESS_HEADER_FAIL))
            {
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                  "OMX_FileMux::OMX_FileMux_ProcessFileMuxCb process sample failed %d", status);
                m_pCallbacks->EventHandler(m_hSelf, m_pAppData
                    ,OMX_EventError,
                    OMX_ErrorInsufficientResources,
                    0, NULL);
                m_pCallbacks->EmptyBufferDone(m_hSelf, m_pAppData,
                    (OMX_BUFFERHEADERTYPE*)pBufferHdr);
            }

            if((status == SPACE_LIMIT_REACHED) || (status == WRITE_FAILED))
            {
                if(status == WRITE_FAILED)
                {
                    m_pCallbacks->EventHandler(m_hSelf, m_pAppData
                                               ,OMX_EventError,
                                               OMX_ErrorInsufficientResources,
                                               0, NULL);

                    m_pCallbacks->EmptyBufferDone(m_hSelf, m_pAppData,
                                         (OMX_BUFFERHEADERTYPE*)pBufferHdr);

                }
                else if(!bOutputLimitReached)
                {
                    /**---------------------------------------------------------
                     Once notify the client
                    ------------------------------------------------------------
                    */
                    bOutputLimitReached = OMX_TRUE;

                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                         " OMX_FileMux::OMX_FileMux_ProcessFileMuxCb output limit reached");

                    m_pCallbacks->EventHandler(m_hSelf, m_pAppData
                                               ,OMX_EventError,
                                               QOMX_ErrorStorageLimitReached,
                                               0, NULL);

                    m_pCallbacks->EmptyBufferDone(m_hSelf, m_pAppData,
                                         (OMX_BUFFERHEADERTYPE*) pBufferHdr);

                }

                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                 "OMX_FileMux::OMX_FileMux_ProcessFileMuxCb %d", status);
            }
            break;
        }
        case PROCESS_SAMPLE_OUTDATED:
        {
            //((OMX_BUFFERHEADERTYPE*)pBufferHdr)->nFilledLen = 0;
            if(((OMX_BUFFERHEADERTYPE*)pBufferHdr)->nInputPortIndex
                                                == OMX_MUX_INDEX_PORT_VIDEO)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
               "OMX_FileMux_ProcessFileMuxCb Process Video frame complete");
            }
            else if(((OMX_BUFFERHEADERTYPE*)pBufferHdr)->nInputPortIndex
                                                == OMX_MUX_INDEX_PORT_AUDIO)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
               "OMX_FileMux_ProcessFileMuxCb Process Audio frame complete");
            }
            m_pCallbacks->EmptyBufferDone(m_hSelf, m_pAppData,
                                (OMX_BUFFERHEADERTYPE*)pBufferHdr);
            break;
        }
        case CLOSE_MUX_COMPLETE:
        case CLOSE_MUX_FAIL:
        {
            OMX_FileMux_PrintStatistics();

            nEvtStatus  = (status == CLOSE_MUX_COMPLETE) ?
                                   OMX_ErrorNone : OMX_ErrorUndefined;
            if(m_eState == OMX_StateExecuting &&
                (m_eTargetState == OMX_StateInvalid ||
                 m_eTargetState == OMX_StateIdle))
            {
                m_eState = m_eTargetState;
                MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR,
                  "OMX_FileMux::OMX_FileMux_ProcessFileMuxCb state changed from current %d, to next = %d",
                (int32)m_eState, (int32)m_eTargetState);
                m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                           OMX_EventCmdComplete,
                                           OMX_CommandStateSet,
                                           m_eTargetState, NULL);
            }

            bMuxOpen = OMX_FALSE;
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                "FileMux callback. Close complete, status = %x!!!", nEvtStatus);
            break;
        }

        case FLUSH_COMPLETED:
        case FLUSH_FAILED:
        {
            MUX_flush_info_type *pFlushInfo = (MUX_flush_info_type*)pData;

            if(!pData)
            {
                 m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                           OMX_EventCmdComplete,
                                           OMX_CommandFlush,
                                           OMX_MUX_INDEX_PORT_AUDIO, NULL);
                 m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                           OMX_EventCmdComplete,
                                           OMX_CommandFlush,
                                           OMX_MUX_INDEX_PORT_VIDEO, NULL);
            }
            else
            {
                 m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                           OMX_EventCmdComplete,
                                           OMX_CommandFlush,
                                           pFlushInfo->stream_id, NULL);
            }

            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                "FileMux callback. Flush complete, status = %x!!!", nEvtStatus);
            break;
        }

        case PAUSE_COMPLETED:
        case PAUSE_FAILED:
        {
            if(m_eTargetState == OMX_StatePause)
            {
                m_pCallbacks->EventHandler(m_hSelf, m_pAppData,
                                           OMX_EventCmdComplete,
                                           OMX_CommandStateSet,
                                           m_eTargetState, NULL);
                m_eState = m_eTargetState;
            }
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                "FileMux callback. Pause complete, status = %x!!!", nEvtStatus);
            break;
        }
        case MUX_STATISTICS:
        {
#if 0
            FileMuxStatistics *pStatData = (FileMuxStatistics *)pData;
                    /**---------------------------------------------------------
                     Once notify the client
                    ------------------------------------------------------------
                    */
            MMI_ExtSpecificMsgType sErrMsg;
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
               "ProcessFileMuxCb Notify MUX_STATISTICS available");

            nRecStatistics.nRecordedTime = pStatData->recordedTime;
            nRecStatistics.nTimeCanRecord= pStatData->timeCanRecord;
            nRecStatistics.nSpaceConsumed = pStatData->spaceConsumed;
            nRecStatistics.nSpaceLeft = pStatData->spaceLeft;

            sErrMsg.eEvent = (OMX_EVENTTYPE)OMX_EventIndexSettingChanged;
            sErrMsg.nData1  = NULL;
            sErrMsg.nData2 = NULL;
            sErrMsg.pEventData = 0;
#endif
        /*    sClientEvtNofity.pfnEvtHdlr(MMI_EVT_QOMX_EXT_SPECIFIC,
                        OMX_ErrorNone,
                        sizeof(MMI_ExtSpecificMsgType),
                        &sErrMsg,
                        sClientEvtNofity.pClientData);*/
            return;
        }
        case PROCESS_SAMPLE_FLUSH:
        {
            pSampleInfo = (MUX_sample_info_type*)pData;
            OMX_FILEMUX_FREEIF(pSampleInfo);
            m_pCallbacks->EmptyBufferDone(m_hSelf, m_pAppData,
                                 (OMX_BUFFERHEADERTYPE*) pBufferHdr);

        }
        default:
            break;
    }
    return;

}

/*==============================================================================

         FUNCTION:         OMX_FileMux_PopulateFileMuxParams

         DESCRIPTION:
*//**       @brief         This function populates the structures
                            required to initialize FileMux Interface
*//**

@par     DEPENDENCIES:
                           This function can be called only after all set
                           parameters have been successfully performed.
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           FALSE - on failure \n
                           TRUE - on success \n


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_BOOL OMX_FileMux::OMX_FileMux_PopulateFileMuxParams
(
    void
)
{
#if 1
    MUX_create_params_type *pParams =
            (MUX_create_params_type*) pFileMuxParams;

    MUX_stream_create_params_type *pStreams =
             (MUX_stream_create_params_type*) pFileMuxStreams;

    OMX_BOOL bVideoPresent = IS_VIDEO_PORT_USED;

    OMX_BOOL bAudioPresent = IS_AUDIO_PORT_USED;


    OMX_U32 nAudioIndex = 0;
    OMX_U32 nVideoIndex = bAudioPresent? 1:0;

    OMX_MUX_MEM_SET(pParams, 0, sizeof(MUX_create_params_type));

    OMX_MUX_MEM_SET(pStreams, 0,
                       OMX_MUX_MAX_STREAMS * sizeof(MUX_stream_create_params_type));


    pParams->drm_distribution_rights = 0;
    pParams->enable_fixupdata = FALSE;
    pParams->file_duration_limit = (uint32)nFileDurationLimit;
    pParams->file_size_limit = (uint32)nFileSizeLimit;
    pParams->force_stsz_table = FALSE;
    pParams->include_drm = FALSE;
    pParams->include_user_data = TRUE;
    pParams->movie_size_warning_imminent_threshold = LIMIT_IMMINENT_THRESHOLD;
    pParams->movie_size_warning_near_threshold = LIMIT_NEAR_THRESHOLD;
    pParams->movie_timescale = 1000; // Set below
    pParams->num_streams =(bVideoPresent ? 1 : 0) + (bAudioPresent ? 1 : 0);
    pParams->output_unit_size = OUTPUT_UNIT_SIZE;
    pParams->streams = pStreams;
    pParams->version_major = 0;
    pParams->version_minor = 0;
    pParams->stream_bitrate = (uint32)(GET_VIDEO_BITRATE);

    pParams->encrypt_param.streamEncrypted = nEncryptTypeConfigParameters.nStreamEncrypted;
	if( nEncryptTypeConfigParameters.nType == QOMX_ENCRYPT_TYPE_HDCP)
	{
      pParams->encrypt_param.type = MUX_ENCRYPT_TYPE_HDCP;
    }
    else
    {
      pParams->encrypt_param.type = MUX_ENCRYPT_TYPE_INVALID;
    }
    pParams->encrypt_param.nEncryptVersion = (uint32)nEncryptTypeConfigParameters.nEncryptVersion;

    /**-------------------------------------------------------------------------
       Initialize all Audio / Video stream specific parameters here.
    ----------------------------------------------------------------------------
    */
    if(
    ! OMX_FileMux_PopulateVideoCommonParams()         ||
    ! OMX_FileMux_PopulateVideoCodecSpecificParams()  ||
    ! OMX_FileMux_PopulateAudioCommonParams()         ||
    ! OMX_FileMux_PopulateAudioCodecSpecificParams()
    )
    {
        return OMX_FALSE;
    }


    /**-------------------------------------------------------------------------
                  File format specific modifications goes here.
    ---------------------------------------------------------------------------
    */
    switch (nFileBrand)
    {
    case QOMX_FORMAT_MP4:
        pParams->movie_timescale = 1000;
        pParams->major_brand = MP4_MAJOR_BRAND;
        pParams->num_compat_brands = (uint32)(sizeof (MP4_COMPAT_BRANDS)
                                      / sizeof (MP4_COMPAT_BRANDS [0]));
        pParams->compat_brands = &MP4_COMPAT_BRANDS [0];
        break;

    case QOMX_FORMAT_AMC:
        pParams->movie_timescale = 90000;  /* AMC requirement */
        pParams->major_brand = AMC_MAJOR_BRAND;
        pParams->num_compat_brands = (uint32)(sizeof (AMC_COMPAT_BRANDS)
                                    / sizeof (AMC_COMPAT_BRANDS [0]));
        pParams->compat_brands = &AMC_COMPAT_BRANDS [0];
        /**---------------------------------------------------------------------
           AMC requirement
        ------------------------------------------------------------------------
        */
        pStreams[nVideoIndex].priority = 16;
        pStreams[nVideoIndex].media_timescale = 90000;
        pStreams[nVideoIndex].handler = "";
        pStreams[nAudioIndex].priority = 16;
        pStreams[nAudioIndex].handler = "";

        //pParams->video_repeat_header = TRUE;
        break;

    case QOMX_FORMAT_3GP:
        pParams->movie_timescale = 1000;
        pParams->major_brand = _3GP_MAJOR_BRAND;
        pStreams[nVideoIndex].media_timescale = 1000;
        pParams->num_compat_brands = (uint32)(sizeof (_3GP_COMPAT_BRANDS)
            / sizeof (_3GP_COMPAT_BRANDS [0]));
        pParams->compat_brands = &_3GP_COMPAT_BRANDS [0];
        break;

    case QOMX_FORMAT_3G2:
        pParams->movie_timescale = 1000;
        pParams->major_brand = _3G2_MAJOR_BRAND;
        pParams->num_compat_brands = (uint32)(sizeof (_3G2_COMPAT_BRANDS)
            / sizeof (_3G2_COMPAT_BRANDS [0]));
        pParams->compat_brands = &_3G2_COMPAT_BRANDS [0];
        if(0 != nFragmentDuration)
        {
            pParams->movie_timescale = 90000;  /* spec requirement */
            pParams->fragment_size =
                         (uint32)(nFragmentDuration * pParams->movie_timescale);
            if (pStreams[nAudioIndex].subinfo.audio.format ==
            MUX_STREAM_AUDIO_AMR)
            {
                pParams->major_brand = FRAG_3G2_MAJOR_BRAND;
                pParams->num_compat_brands = (uint32)(sizeof (FRAG_3G2_COMPAT_BRANDS)
                    / sizeof (FRAG_3G2_COMPAT_BRANDS [0]));
                pParams->compat_brands = &FRAG_3G2_COMPAT_BRANDS [0];
            }
            else
            {
                pParams->major_brand = FRAG_3G2_MAJOR_BRAND_NOT_AMR;
                pParams->num_compat_brands
                    =(uint32)( sizeof (FRAG_3G2_COMPAT_BRANDS_NOT_AMR)
                    / sizeof (FRAG_3G2_COMPAT_BRANDS_NOT_AMR [0]));
                pParams->compat_brands = &FRAG_3G2_COMPAT_BRANDS_NOT_AMR [0];
            }
            if (bVideoPresent)
            {
                /**---------------------------------------------------------*//*
                          Spec Requirement
                ----------------------------------------------------------------
                */
                pStreams[nVideoIndex].priority = 16;
                pStreams[nVideoIndex].media_timescale = 90000;
                pStreams[nVideoIndex].handler = "";
            }
            if (bAudioPresent)
            {
                /**---------------------------------------------------------*//*
                          Spec Requirement
                ----------------------------------------------------------------
                */
                pStreams[nAudioIndex].priority = 16;
                pStreams[nAudioIndex].handler = "";
            }
        }
        break;

    case QOMX_FORMAT_K3G:
        pParams->movie_timescale = 1000;
        pParams->major_brand = K3G_MAJOR_BRAND;
        pParams->num_compat_brands = (uint32)(sizeof (K3G_COMPAT_BRANDS)
            / sizeof (K3G_COMPAT_BRANDS [0]));
        pParams->compat_brands = &K3G_COMPAT_BRANDS [0];
        break;

    case QOMX_FORMAT_SKM:
        pParams->movie_timescale = 1000;
        pParams->major_brand = SKM_MAJOR_BRAND;
        pParams->force_stsz_table = TRUE;
        if (bVideoPresent)
        {
            pStreams[nVideoIndex].handler = "vide";
        }
        if (bAudioPresent)
        {
            pStreams[nAudioIndex].handler = "soun";
        }

        if (pStreams[nAudioIndex].subinfo.audio.format ==
            MUX_STREAM_AUDIO_AMR)
        {
            pStreams[nAudioIndex].frames_per_sample = 10;
            pStreams[nAudioIndex].max_table_stores =
            /**-----------------------------------------------------------------
                  First we update number of times chunks table can be pushed
                  until max recording duration
            --------------------------------------------------------------------
            */
                           ((MMI_MAX_STREAM_DURATION * 1000) /
                           DESIRED_INTERLACE_RATE)     /
                           pStreams[nAudioIndex].max_chunks
                                           +
            /**-----------------------------------------------------------------
                  First we update number of times samples table can be pushed
                  until max recording duration
            --------------------------------------------------------------------
            */
                                     MMI_MAX_STREAM_DURATION *
                           pStreams[nAudioIndex].max_samples_rate/
                           pStreams[nAudioIndex].frames_per_sample  /
                           pStreams[nAudioIndex].max_samples ;//TBD

        }

        /**---------------------------------------------------------------------
            NOTE: the SKT VOD spec 2.3 (draft2) specifies '512' for the
            version, but there are 32-bits.  So we will guess they mean
            it to be a NULL-terminated ASCII character string.
        ------------------------------------------------------------------------
        */
        pParams->version_major = 0x3531;  /* '5', '1' */
        pParams->version_minor = 0x3200;  /* '2', '\0' */

        pParams->num_compat_brands = (uint32)(sizeof (SKM_COMPAT_BRANDS)
            / sizeof (SKM_COMPAT_BRANDS [0]));
        pParams->compat_brands = &SKM_COMPAT_BRANDS [0];
        break;

    case QOMX_FORMAT_AAC:
        /**---------------------------------------------------------------------
          NEED TO ADD SWITCH FOR FORMAT ONCE THE EXTN IS DEFINED.
        ------------------------------------------------------------------------
        */
        OMX_MUX_MEM_SET(pParams, 0, sizeof(MUX_create_params_type));

        OMX_MUX_MEM_SET(pStreams, 0,
                       OMX_MUX_MAX_STREAMS * sizeof(MUX_stream_create_params_type));
        pStreams[MMI_AUDIO_STREAM_NUM].sample_delta = (uint32)(
                   (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                            sFormatSpecificInfo.sAACInfo.nFrameLength);
        pStreams[MMI_AUDIO_STREAM_NUM].media_timescale = (uint32)(
                   (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                            sFormatSpecificInfo.sAACInfo.nSampleRate);
        pParams->sampling_rate_index =
                                  GET_AAC_SAMPLING_RATE_INDEX((
                         (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                                    sFormatSpecificInfo.sAACInfo.nSampleRate));
        pParams->num_channels =
                            (uint8)(arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                                        sFormatSpecificInfo.sAACInfo.nChannels;
        pParams->stream_bitrate = (uint32)((arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                                        sFormatSpecificInfo.sAACInfo.nBitRate);
        break;

    case QOMX_FORMAT_AMR:
        OMX_MUX_MEM_SET(pParams, 0, sizeof(MUX_create_params_type));

        OMX_MUX_MEM_SET(pStreams, 0,
                       OMX_MUX_MAX_STREAMS * sizeof(MUX_stream_create_params_type));
        pStreams[MMI_AUDIO_STREAM_NUM].sample_delta = 0;
        pStreams[MMI_AUDIO_STREAM_NUM].media_timescale = VOCODER_SAMPLING_RATE;
        pStreams[MMI_AUDIO_STREAM_NUM].sample_size = 32;
        break;

    case QOMX_FORMAT_WAVE:
        OMX_MUX_MEM_SET(pParams, 0, sizeof(MUX_create_params_type));

        OMX_MUX_MEM_SET(pStreams, 0,
                       OMX_MUX_MAX_STREAMS * sizeof(MUX_stream_create_params_type));

        pStreams[MMI_AUDIO_STREAM_NUM].sample_delta = 0;
        pStreams[MMI_AUDIO_STREAM_NUM].media_timescale = VOCODER_SAMPLING_RATE;
        pStreams[MMI_AUDIO_STREAM_NUM].sample_size = 32;

        pParams->bits_per_sample =
                   (uint16)(arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                                    sFormatSpecificInfo.sPCMInfo.nBitPerSample;

        pParams->num_channels    =
            (uint8)(arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                                        sFormatSpecificInfo.sPCMInfo.nChannels;

        pParams->sampling_rate = (uint32)((arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                                   sFormatSpecificInfo.sPCMInfo.nSamplingRate);
        pParams->block_align   = (uint16)
                          ((pParams->bits_per_sample+7)/(uint16)8*pParams->num_channels);
        break;

    case QOMX_FORMAT_EVRC:
        /**---------------------------------------------------------------------
              NOthing to DO here format specific. Just memset to 0
        ------------------------------------------------------------------------
       */
        OMX_MUX_MEM_SET(pParams, 0, sizeof(MUX_create_params_type));

        OMX_MUX_MEM_SET(pStreams, 0,
                       OMX_MUX_MAX_STREAMS * sizeof(MUX_stream_create_params_type));

        break;


    case QOMX_FORMAT_QCP:
        /**---------------------------------------------------------------------
              NOthing to DO here format specific. Just memset to 0
        ------------------------------------------------------------------------
       */
        OMX_MUX_MEM_SET(pParams, 0, sizeof(MUX_create_params_type));

        OMX_MUX_MEM_SET(pStreams, 0,
                       OMX_MUX_MAX_STREAMS * sizeof(MUX_stream_create_params_type));

        break;
    case QOMX_FORMATMPEG_TS:
        if((AUDIO_COMPRESSION_FORMAT) == OMX_AUDIO_CodingAAC)
        {
            if((arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                    sFormatSpecificInfo.sAACInfo.eAACStreamFormat ==
                       OMX_AUDIO_AACStreamFormatMP4ADTS)
            {
                pStreams[OMX_MUX_INDEX_PORT_AUDIO].
                    subinfo.audio.audio_params.frames_per_sample = 1;

            }
            else
            {
                pStreams[OMX_MUX_INDEX_PORT_AUDIO].
                    subinfo.audio.audio_params.frames_per_sample = 0;//Unknown
            }
        }
        break;
    default:
        return OMX_FALSE;
    }
#endif
    return OMX_TRUE;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_PopulateVideoCommonParams

         DESCRIPTION:
*//**       @brief         Populate video codec independent params required for
                           initializing FileMux
*//**

@par     DEPENDENCIES:
                           This function can be called only after all set
                           parameters have been successfully performed.
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_BOOL OMX_FileMux::OMX_FileMux_PopulateVideoCommonParams
(
    void
)
{
    MUX_stream_create_params_type *pStreams =
             (MUX_stream_create_params_type*) pFileMuxStreams;
    MUX_create_params_type *pParams =
                    (MUX_create_params_type*) pFileMuxParams;

    OMX_U32  nSpaceLimitThreshold  =
                    pParams->movie_size_warning_near_threshold;

    OMX_BOOL bVideoPresent = IS_VIDEO_PORT_USED;

    OMX_BOOL bAudioPresent = IS_AUDIO_PORT_USED;
    /**-------------------------------------------------------------------------
           If audio not present video params goes first
    ----------------------------------------------------------------------------
    */
    OMX_U32 nIndex = bAudioPresent ? MMI_VIDEO_STREAM_NUM :
                                          MMI_AUDIO_STREAM_NUM;



    if (OMX_TRUE == bVideoPresent)
    {
        OMX_VIDEO_PORTDEFINITIONTYPE *pVideoPortInfo;
        pVideoPortInfo = &(arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO)
                                              ->sPortDef.format.video;

        pStreams[nIndex].type = MUX_STREAM_VIDEO;



        pStreams[nIndex].subinfo.video.width  =
                                        (uint16)pVideoPortInfo->nFrameWidth;
        pStreams[nIndex].subinfo.video.height =
                                        (uint16)pVideoPortInfo->nFrameHeight;
        pStreams[nIndex].subinfo.video.frame_rate =(uint16)(
                                                 GET_VIDEO_FRAMERATE);

        pStreams[nIndex].width = (((uint32) pVideoPortInfo->nFrameWidth)<< 16);
        pStreams[nIndex].height= (((uint32) pVideoPortInfo->nFrameHeight)<< 16);

        pStreams[nIndex].handler = "vide";
        pStreams[nIndex].media_timescale = 90000;

        pStreams[nIndex].interlace = (uint32)nIndex;
        if (/*!nInterlaceDuration*/ 1
            && ((arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)
                  ->sPortDef.format.video.eCompressionFormat !=
                                                OMX_VIDEO_CodingUnused))
        {
            /* interlace video with audio, with video as master */
            pStreams[nIndex].interlace = MMI_AUDIO_STREAM_NUM;
        }
        pStreams[nIndex].interlace_rate = MAX
                     (pStreams[nIndex].media_timescale
                         * DESIRED_INTERLACE_RATE / 1000, 1);

        pStreams[nIndex].chunk_size =
                                 (uint32)(OPTIMAL_CHUNK_SIZE(GET_VIDEO_BITRATE));
        pStreams[nIndex].buffer_size = (uint32)(
                                 OPTIMAL_CHUNK_SIZE(GET_VIDEO_BITRATE)
                                 * OPTIMAL_STREAM_BUF_SIZE_FACTOR);

        pStreams[nIndex].max_header   = MAX_HEADER_SIZE;
        pStreams[nIndex].max_footer   = MAX_FOOTER_SIZE;
        pStreams[nIndex].inter_frames = TRUE;

        if (/*nInterlaceDuration*/ 1)
        {
            pStreams[nIndex].chunks_out_near_threshold
                = pParams->movie_size_warning_near_threshold * 1000
                  / DESIRED_INTERLACE_RATE;

            pStreams[nIndex].chunks_out_imminent_threshold
                = pParams->movie_size_warning_imminent_threshold * 1000
                  / DESIRED_INTERLACE_RATE;
        }
        else
        {
            pStreams[nIndex].chunks_out_near_threshold
                = (pParams->movie_size_warning_near_threshold
                   + OPTIMAL_CHUNK_DURATION - 1) / OPTIMAL_CHUNK_DURATION;

            pStreams[nIndex].chunks_out_imminent_threshold
                = (pParams->movie_size_warning_imminent_threshold
                   + OPTIMAL_CHUNK_DURATION - 1) / OPTIMAL_CHUNK_DURATION;
        }


        pStreams[nIndex].samples_out_near_threshold
                            = (uint32)(pParams->movie_size_warning_near_threshold
                              * GET_VIDEO_FRAMERATE);

        pStreams[nIndex].samples_out_imminent_threshold
                            = (uint32)(pParams->movie_size_warning_imminent_threshold
                             * GET_VIDEO_FRAMERATE);



        pStreams[nIndex].max_samples  = (uint32)MAX(MMI_MAX_SAMPLES_PER_STORE,
                                            nSpaceLimitThreshold *
                                            GET_VIDEO_FRAMERATE);

        if(nFragmentDuration)
        {
            pStreams[nIndex].max_chunks = (uint32)(MAX(
                    nFragmentDuration + nSpaceLimitThreshold
                ,
                  ((nFragmentDuration + nSpaceLimitThreshold) *
                    (GET_VIDEO_BITRATE >> 3) /
                    pStreams[nIndex].chunk_size) * 2
                  ) * FRAGMENT_TABLE_SIZE_FACTOR /100);
        }
        else
        {
            pStreams[nIndex].max_chunks = (uint32)MAX(
                                          DEFAULT_CHUNKS_TABLE_SIZE,
                                         (nSpaceLimitThreshold * 1000) /
                                          DESIRED_INTERLACE_RATE + 1);

        }


        pStreams[nIndex].max_samples_rate = (uint16)GET_VIDEO_FRAMERATE;


        pStreams[nIndex].max_table_stores = (uint32)(MMI_MAX_STREAM_DURATION *
                                            (GET_VIDEO_BITRATE >> 3) /
                                            pStreams[nIndex].chunk_size /
                                            pStreams[nIndex].max_chunks
                                                       +
                                           ((MMI_MAX_STREAM_DURATION *
                                            (uint16)(GET_VIDEO_FRAMERATE))/
                                            pStreams[nIndex].max_samples) *3);


        pStreams[nIndex].stsc_reset_rate = STSC_ALGO_RESET_RATE;

    }
    return OMX_TRUE;
}




/*==============================================================================

         FUNCTION:         OMX_FileMux_PopulateVideoCodecSpecificParams

         DESCRIPTION:
*//**       @brief         Populate video codec specific params required for
                           initializing FileMux
*//**

@par     DEPENDENCIES:
                           This function can be called only after all set
                           parameters have been successfully performed.
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
OMX_BOOL OMX_FileMux::OMX_FileMux_PopulateVideoCodecSpecificParams
(
    void
)

{
    MUX_stream_create_params_type *pStreams =
             (MUX_stream_create_params_type*) pFileMuxStreams;

    OMX_FileMux_MediaFormatConfigType *pVideoFormatParams =
        &(arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO)->sFormatSpecificInfo;

    OMX_VIDEO_PORTDEFINITIONTYPE *pVideoPortInfo =
        &(arrPortConfig + OMX_MUX_INDEX_PORT_VIDEO)->sPortDef.format.video;

    OMX_BOOL bVideoPresent = IS_VIDEO_PORT_USED;

    OMX_BOOL bAudioPresent = IS_AUDIO_PORT_USED;
    /**-------------------------------------------------------------------------
           If audio not present video params goes first
    ----------------------------------------------------------------------------
    */
    OMX_U32 nIndex = bAudioPresent ? MMI_VIDEO_STREAM_NUM :
                                          MMI_AUDIO_STREAM_NUM;

    if(!bVideoPresent)
    {
        return OMX_TRUE;
    }
    switch(pVideoPortInfo->eCompressionFormat)
    {
    case OMX_VIDEO_CodingAVC:
    {
          pStreams[nIndex].subinfo.video.format = MUX_STREAM_VIDEO_H264;
          sMuxStats.pVideoCodec = (OMX_U8*)"OMX_VIDEO_CodingAVC";

          if(pVideoFormatParams->sAVCInfo.eProfile <=
                                                 OMX_VIDEO_AVCProfileHigh444)
          {
              /**---------------------------------------------------------------
                TODO add higher profiles.
              ------------------------------------------------------------------
              */
              pStreams [nIndex].subinfo.video.profile =
                  AVC_Profile_Table[
                  GET_AVC_PROFILE_INDEX(pVideoFormatParams->sAVCInfo.eProfile)];
          }

          if(pVideoFormatParams->sAVCInfo.eLevel <= OMX_VIDEO_AVCLevel51)
          {
              pStreams [nIndex].subinfo.video.level =
                     AVC_Level_Table[
                      GET_AVC_LEVEL_INDEX(pVideoFormatParams->sAVCInfo.eLevel)];
          }
          if(pVideoFormatParams->sAVCInfo.eLevel == OMX_VIDEO_AVCLevel1b)
          {
             pStreams [nIndex].subinfo.video.profile_comp =
                                                     AVC_L1b_PROFILE_COM;//TBD;
          }
          else
          {
             pStreams [nIndex].subinfo.video.profile_comp =
                                                 AVC_DEFAULT_PROFILE_COM;//TBD;
          }
          break;

    }
    case OMX_VIDEO_CodingH263:
    {
          pStreams[nIndex].subinfo.video.format = MUX_STREAM_VIDEO_H263;

          pStreams[nIndex].subinfo.video.profile = 0;
          sMuxStats.pVideoCodec = (OMX_U8*)"OMX_VIDEO_CodingH263";

          if(pVideoFormatParams->sH263Info.eLevel <= OMX_VIDEO_H263Level70)
          {
              if(pVideoFormatParams->sH263Info.eLevel
                  != 0)
              {
                  pStreams [nIndex].subinfo.video.level =
                    H263_Level_Table
                        [GET_H263_LEVEL_INDEX(pVideoFormatParams->sH263Info.eLevel)];
              }
              else
              {
                  pStreams [nIndex].subinfo.video.level =
                       (uint8)(OMX_FileMux_GetH263Level(
                                     pStreams[nIndex].subinfo.video.width,
                                     pStreams[nIndex].subinfo.video.height,
                                     GET_VIDEO_FRAMERATE,
                                     GET_VIDEO_BITRATE,
                                     pVideoFormatParams->sH263Info.eProfile));
              }

          }

          break;
    }

    case OMX_VIDEO_CodingUnused:
          sMuxStats.pVideoCodec = (OMX_U8*)"OMX_VIDEO_CodingUnused";
          break;

    case OMX_VIDEO_CodingMPEG4:
          sMuxStats.pVideoCodec = (OMX_U8*)"OMX_VIDEO_CodingMPEG4";
          pStreams[nIndex].subinfo.video.format = MUX_STREAM_VIDEO_MPEG4;
          break;

    default:
          return OMX_FALSE;
        /**---------------------------------------------------------------------
                   Cannot come here.
        ------------------------------------------------------------------------
        */
    }
    return OMX_TRUE;
}


/*==============================================================================

         FUNCTION:         OMX_FileMux_PopulateAudioCommonParams

         DESCRIPTION:
*//**       @brief         Populate audio codec independent parameters
*//**

@par     DEPENDENCIES:
                           This function can be called only after all set
                           parameters have been successfully performed.
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

OMX_BOOL OMX_FileMux::OMX_FileMux_PopulateAudioCommonParams(void)
{

    MUX_stream_create_params_type *pStreams =
             (MUX_stream_create_params_type*) pFileMuxStreams;
    MUX_create_params_type *pParams =
                    (MUX_create_params_type*) pFileMuxParams;
    OMX_BOOL bAudioPresent = IS_AUDIO_PORT_USED;
    OMX_U32  nSpaceLimitThreshold  =
                    pParams->movie_size_warning_near_threshold;
    OMX_U32  nTempBufSize = 0;
    /* Set up audio stream creation parameters. */
    if (OMX_TRUE == bAudioPresent)
    {
        pStreams[MMI_AUDIO_STREAM_NUM].interlace = MMI_AUDIO_STREAM_NUM;
        pStreams[MMI_AUDIO_STREAM_NUM].type = MUX_STREAM_AUDIO;
        pStreams[MMI_AUDIO_STREAM_NUM].handler = "soun";
        pStreams[MMI_AUDIO_STREAM_NUM].media_timescale =
                                                   VOCODER_SAMPLING_RATE;
        /**---------------------------------------------------------------------
         Audio being an interlaced stream we need to have some cushion
         in buffer size. Its taken as 300%. 200% for making it usable inparallel
         when one chunk is precessed and 100% cushion.
        ------------------------------------------------------------------------
        */
        nTempBufSize = 3 * MIN(DEAFULT_INTERLACE_PERIOD_MS,
                                   OPTIMAL_CHUNK_DURATION) *
                                   OMX_FileMux_GetAudioBitrate()/8;

        pStreams[MMI_AUDIO_STREAM_NUM].buffer_size = (uint32)
                                 MAX(nTempBufSize , MIN_AUDIO_MUX_BUFFER_SIZE);

        pStreams[MMI_AUDIO_STREAM_NUM].max_header = QCP_FILE_HEADER_SIZE;
        pStreams[MMI_AUDIO_STREAM_NUM].sample_delta
                     = VOCODER_SAMPLING_RATE / VOCODER_PACKET_RATE;

        pStreams[MMI_AUDIO_STREAM_NUM].max_samples =
                                        MMI_MAX_SAMPLES_PER_STORE;

        pStreams[MMI_AUDIO_STREAM_NUM].chunk_size = (uint32)
                                       (OMX_FileMux_GetAudioBitrate()/8) *
                                        MIN(DEAFULT_INTERLACE_PERIOD_MS,
                                                OPTIMAL_CHUNK_DURATION);



        pStreams[MMI_AUDIO_STREAM_NUM].max_chunks = (uint32)MAX(
                                          DEFAULT_CHUNKS_TABLE_SIZE,
                                         (nSpaceLimitThreshold * 1000) /
                                          DESIRED_INTERLACE_RATE + 1);

        pStreams[MMI_AUDIO_STREAM_NUM].max_samples_rate = 50;

        pStreams[MMI_AUDIO_STREAM_NUM].max_table_stores =
                            /**-------------------------------------------------
                              Number of times chunks table may be pused to temp
                              file
                            ----------------------------------------------------
                            */
                            ((MMI_MAX_STREAM_DURATION * 1000) /
                            DESIRED_INTERLACE_RATE)     /
                            pStreams[MMI_AUDIO_STREAM_NUM].max_chunks
                                                     +
                            /**-------------------------------------------------
                              Number of times sample table may be pused to temp
                              file
                            ----------------------------------------------------
                            */
                            MMI_MAX_STREAM_DURATION *
                            pStreams[MMI_AUDIO_STREAM_NUM].max_samples_rate/
                            pStreams[MMI_AUDIO_STREAM_NUM].max_samples ;


        pStreams[MMI_AUDIO_STREAM_NUM].stsc_reset_rate   =
                                                    STSC_ALGO_RESET_RATE;
        pStreams[MMI_AUDIO_STREAM_NUM].frames_per_sample = 1;


    }
    return OMX_TRUE;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_PopulateAudioCodecSpecificParams

         DESCRIPTION:
*//**       @brief         Initialize audio codec specific params required
                           for FileMux
*//**

@par     DEPENDENCIES:
                           This function can be called only after all set
                           parameters have been successfully performed.
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

OMX_BOOL OMX_FileMux::OMX_FileMux_PopulateAudioCodecSpecificParams
(
    void
)
{
    MUX_stream_create_params_type *pStreams =
             (MUX_stream_create_params_type*) pFileMuxStreams;
    MUX_create_params_type *pParams =
                    (MUX_create_params_type*) pFileMuxParams;

    OMX_AUDIO_CODINGTYPE eAudioFormat;

    OMX_FileMux_MediaFormatConfigType *pAudioConfig;

    OMX_BOOL bAudioPresent = IS_AUDIO_PORT_USED;

    eAudioFormat = AUDIO_COMPRESSION_FORMAT;

    pAudioConfig = &(arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)
                               ->sFormatSpecificInfo;


    if(!bAudioPresent)
    {
        return OMX_TRUE;
    }

    switch ((OMX_U32)eAudioFormat)
    {
    case OMX_AUDIO_CodingAC3:
        /**---------------------------------------------------------------------
            AC3 specific configuration for Muxer
        ------------------------------------------------------------------------
        */
        pStreams[MMI_AUDIO_STREAM_NUM].subinfo.audio.format =
                                          MUX_STREAM_AUDIO_AC3;
        pStreams[MMI_AUDIO_STREAM_NUM].media_timescale =
                   pAudioConfig->sAC3Info.nSamplingRate;
        pStreams[MMI_AUDIO_STREAM_NUM].max_header = 2;

        pStreams[MMI_AUDIO_STREAM_NUM].sample_delta = AC3_SAMPLES_PER_FRAME;

        pStreams[MMI_AUDIO_STREAM_NUM].max_samples_rate
                                      = 50;
        pStreams[MMI_AUDIO_STREAM_NUM].sample_size = 0;
        pStreams[MMI_AUDIO_STREAM_NUM].subinfo.audio.sampling_frequency
                                  = pAudioConfig->sAC3Info.nSamplingRate;

        sMuxStats.pAudioCodec = (OMX_U8*)"OMX_AUDIO_CodingAC3";

        break;
    case OMX_AUDIO_CodingQCELP13:
        /**---------------------------------------------------------------------
            QCELP specific configuration for Muxer
        ------------------------------------------------------------------------
        */
        if(pAudioConfig->sQCELPInfo.eCDMARate == OMX_AUDIO_CDMARateHalf)
        {
            pStreams[MMI_AUDIO_STREAM_NUM].subinfo.audio.format =
                                          MUX_STREAM_AUDIO_QCELP13K_HALF;
        }
        else if ((pAudioConfig->sQCELPInfo.eCDMARate == OMX_AUDIO_CDMARateFull))
        {
            pStreams[MMI_AUDIO_STREAM_NUM].subinfo.audio.format =
                                          MUX_STREAM_AUDIO_QCELP13K_FULL;
        }

        if (nFileBrand == QOMX_FORMAT_K3G)
        {
            pStreams[MMI_AUDIO_STREAM_NUM].subinfo.audio.format
                                   = MUX_STREAM_AUDIO_PUREVOICE;
        }

        /**---------------------------------------------------------------------
        *   We are expecting variable as of now. So set 0
        *-----------------------------------------------------------------------
        */
        pStreams[MMI_AUDIO_STREAM_NUM].sample_size = 0;
        pStreams[MMI_AUDIO_STREAM_NUM].sample_delta = VOCODER_SAMPLING_RATE /
                                                       VOCODER_PACKET_RATE;
        sMuxStats.pAudioCodec = (OMX_U8*)"OMX_AUDIO_CodingQCELP13";

        break;

    case OMX_AUDIO_CodingEVRC:
        pStreams[MMI_AUDIO_STREAM_NUM].subinfo.audio.format =
                                          MUX_STREAM_AUDIO_EVRC;
        pStreams[MMI_AUDIO_STREAM_NUM].sample_size = 0;
        pStreams[MMI_AUDIO_STREAM_NUM].sample_delta = VOCODER_SAMPLING_RATE /
                                                       VOCODER_PACKET_RATE;

        sMuxStats.pAudioCodec = (OMX_U8*)"OMX_AUDIO_CodingEVRC";
        break;

    case OMX_AUDIO_CodingAMR:
        /**---------------------------------------------------------------------
            AMR specific configuration for Muxer
        ------------------------------------------------------------------------
        */
        if(pAudioConfig->sAMRWBInfo.eAMRBandMode > OMX_AUDIO_AMRBandModeWB8)
        {
            pStreams[MMI_AUDIO_STREAM_NUM].subinfo.audio.format =
                                                   MUX_STREAM_AUDIO_AMR_WB_PLUS;
        }
        else if(pAudioConfig->sAMRWBInfo.eAMRBandMode >OMX_AUDIO_AMRBandModeNB7)
        {
            pStreams[MMI_AUDIO_STREAM_NUM].subinfo.audio.format =
                                                   MUX_STREAM_AUDIO_AMR_WB;
        }
        else
        {
            pStreams[MMI_AUDIO_STREAM_NUM].subinfo.audio.format =
                                                      MUX_STREAM_AUDIO_AMR;
        }

        pStreams[MMI_AUDIO_STREAM_NUM].sample_size = 0;
        pStreams[MMI_AUDIO_STREAM_NUM].sample_delta = VOCODER_SAMPLING_RATE /
                                                       VOCODER_PACKET_RATE;
        sMuxStats.pAudioCodec = (OMX_U8*)"OMX_AUDIO_CodingAMR";

        break;

    case OMX_AUDIO_CodingAAC:
        OMX_U32 nAudioObjectType;
        OMX_U32 nSampFreqIndex;
        OMX_U32 nChannelConfig;
        /**---------------------------------------------------------------------
            AAC specific configuration for Muxer
        ------------------------------------------------------------------------
        */
        pStreams[MMI_AUDIO_STREAM_NUM].subinfo.audio.format =
                                          MUX_STREAM_AUDIO_MPEG4_AAC;
        pStreams[MMI_AUDIO_STREAM_NUM].media_timescale =
                   (uint32)pAudioConfig->sAACInfo.nSampleRate;
        pStreams[MMI_AUDIO_STREAM_NUM].max_header = 2;
        pStreams[MMI_AUDIO_STREAM_NUM].sample_delta = (uint32)
                               (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                            sFormatSpecificInfo.sAACInfo.nFrameLength;
        if(pStreams[MMI_AUDIO_STREAM_NUM].sample_delta == 0)
        {
            pStreams[MMI_AUDIO_STREAM_NUM].sample_delta = AAC_SAMPLES_PER_FRAME;
        }

        pStreams[MMI_AUDIO_STREAM_NUM].max_samples_rate
                                      = 50;
        pStreams[MMI_AUDIO_STREAM_NUM].sample_size = 0;
        pStreams[MMI_AUDIO_STREAM_NUM].subinfo.audio.sampling_frequency
                                  = (uint32)pAudioConfig->sAACInfo.nSampleRate;

        nAudioObjectType = pAudioConfig->sAACInfo.eAACProfile;
        nChannelConfig   = pAudioConfig->sAACInfo.eChannelMode ==
                           OMX_AUDIO_ChannelModeMono? 1 : 2;
        nSampFreqIndex   =
           GET_AAC_SAMPLING_RATE_INDEX(pAudioConfig->sAACInfo.nSampleRate);

        nAACStreamHeader [0]
            = (OMX_U8)((nAudioObjectType << 3)
               | ((nSampFreqIndex & 0x0E) >> 1));
        nAACStreamHeader [1]
            = (OMX_U8)(((nSampFreqIndex & 0x01) << 7)
               | (nChannelConfig << 3));

        sMuxStats.pAudioCodec = (OMX_U8*)"OMX_AUDIO_CodingAAC";

        break;

    case QOMX_AUDIO_CodingEVRCB:
        sMuxStats.pAudioCodec = (OMX_U8*)"QOMX_AUDIO_CodingEVRCB";
        break;
    case QOMX_AUDIO_CodingEVRCWB:
        sMuxStats.pAudioCodec = (OMX_U8*)"QOMX_AUDIO_CodingEVRCWB";
        break;
    case OMX_AUDIO_CodingG711:
        sMuxStats.pAudioCodec = (OMX_U8*)"QOMX_AUDIO_CodingG711";
        break;
    case OMX_AUDIO_CodingPCM:
        sMuxStats.pAudioCodec = (OMX_U8*)"OMX_AUDIO_CodingPCM";
        pStreams[MMI_AUDIO_STREAM_NUM].subinfo.audio.format =
                                                   MUX_STREAM_AUDIO_PCM;
        pParams->bits_per_sample =
               (uint16)(arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                                sFormatSpecificInfo.sPCMInfo.nBitPerSample;
        pStreams[MMI_AUDIO_STREAM_NUM].media_timescale =
                  (uint32) pAudioConfig->sPCMInfo.nSamplingRate;
        pParams->num_channels    =
             (uint8)(arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                                    sFormatSpecificInfo.sPCMInfo.nChannels;

        pParams->sampling_rate =(uint32)(arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                                   sFormatSpecificInfo.sPCMInfo.nSamplingRate;
        pStreams[MMI_AUDIO_STREAM_NUM].subinfo.audio.sampling_frequency
                                  =(uint32) (arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)->
                                   sFormatSpecificInfo.sPCMInfo.nSamplingRate;
        pParams->block_align   =
                        (uint16)((pParams->bits_per_sample+7)/8*pParams->num_channels);
        if(nFileBrand == QOMX_FORMATMPEG_TS)
        {
            //DVD LPCM with 480 samples per frame
            pStreams[MMI_AUDIO_STREAM_NUM].sample_delta = 480;
        }
        break;
    case OMX_AUDIO_CodingUnused:
        sMuxStats.pAudioCodec = (OMX_U8*)"OMX_AUDIO_CodingUnused";
        /**---------------------------------------------------------------------
            Coding Unused. Nothing to fill
        ------------------------------------------------------------------------
        */
        break;

    default:
        return OMX_FALSE;
    }
    return OMX_TRUE;
}

/*==============================================================================

         FUNCTION:         OMX_FileMux_GetH263Level

         DESCRIPTION:
*//**       @brief         Returns the H263 level from settings
*//**

@par     DEPENDENCIES:
                           This function uses fps bitrate and resolution to
                           get the level information.
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_U32 bitrate


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

OMX_U32 OMX_FileMux::OMX_FileMux_GetH263Level
(
    OMX_U32 width,
    OMX_U32 height,
    OMX_U32 framePerSec,
    OMX_U32 bitrate,
    OMX_U32 profile

)
{
    #define QCIF_SIZE     176 * 144
    #define SQCIF_SIZE    128 * 96
    #define CIF_SIZE      352 * 288
    #define CIF_SIZE_SUB  352 * 240
    #define CUSTOM_SIZE1      720 * 288
    #define CUSTOM_SIZE1_SUB  720 * 240

    OMX_U32 imageSize = width * height;
    OMX_U32 level = 10;

    /**-------------------------------------------------------------------------
      This is an implementation of specification
      X.4 Levels of performance capability in
      t-REC-h263-2005-01

    ----------------------------------------------------------------------------
    */

    if(imageSize   >  CUSTOM_SIZE1     ||
       (framePerSec >= 60  &&
        imageSize >  CIF_SIZE)         ||
       (imageSize > CUSTOM_SIZE1_SUB &&
        framePerSec >= 50)             ||
       bitrate     > 8192000)
    {
        level = 70;
    }

    else if(imageSize   >  CIF_SIZE ||
       framePerSec >= 60            ||
       (imageSize > CIF_SIZE_SUB &&
        framePerSec >= 50)          ||
       bitrate     > 4096000)
    {
        level = 60;
    }

    else if(
       framePerSec >= 30            ||
       bitrate     > 2048000        ||
       (imageSize != CIF_SIZE &&
        imageSize != QCIF_SIZE &&
        imageSize != SQCIF_SIZE))
    {
        level = 50;
        if(bitrate <= 128000 &&
           framePerSec < 15 &&
           profile != 0     &&
           profile != 2     &&
           imageSize <= QCIF_SIZE)
        {
            /**-----------------------------------------------------------------
              Only level 45 can have non standard size below level 50.
              But it has other constrains like <=128k, <15 fps etc and also
              profile should not be 0 or 2.
            --------------------------------------------------------------------
            */
            level = 45;
        }
    }


    else if(
       bitrate     > 384000 )
    {
        level = 40;
    }


    else if(
       bitrate     > 128000 ||
       (imageSize == CIF_SIZE &&
        framePerSec >= 15))
    {
        level = 30;
    }

    else if(
       framePerSec >=  15             ||
       bitrate     > 64000 )
    {
        level = 20;
    }
    else
    {
        level = 10;
    }

    return level;

}

/*==============================================================================

         FUNCTION:         OMX_FileMux_GetAudioBitrate

         DESCRIPTION:
*//**       @brief         Returns the audio bitrate based on settings
*//**

@par     DEPENDENCIES:
                           This function can be called only after all set
                           parameters have been successfully performed.
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_U32 bitrate


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

OMX_U32 OMX_FileMux::OMX_FileMux_GetAudioBitrate
(
    void
)
{

    OMX_AUDIO_CODINGTYPE eAudioFormat;

    OMX_FileMux_MediaFormatConfigType *pAudioConfig;

    eAudioFormat = AUDIO_COMPRESSION_FORMAT;

    pAudioConfig = &(arrPortConfig + OMX_MUX_INDEX_PORT_AUDIO)
                               ->sFormatSpecificInfo;
    switch (eAudioFormat)
    {
    case OMX_AUDIO_CodingQCELP13:
        /**---------------------------------------------------------------------
            QCELP bitrate:
            We should consider the maximum bitrate i.e the fixed versions of
            these versions to estimate the mamory requirement.
        ------------------------------------------------------------------------
        */
        if(pAudioConfig->sQCELPInfo.eCDMARate == OMX_AUDIO_CDMARateHalf)
        {
            return VOCODER_BITRATE_13K_FULL;
        }
        else if ((pAudioConfig->sQCELPInfo.eCDMARate == OMX_AUDIO_CDMARateFull))
        {
            return VOCODER_BITRATE_13K_HALF;
        }


    case OMX_AUDIO_CodingEVRC:
        if(pAudioConfig->sEVRCInfo.eCDMARate == OMX_AUDIO_CDMARateHalf)
        {
            return VOCODER_BITRATE_EVRC;
        }
        else if ((pAudioConfig->sEVRCInfo.eCDMARate == OMX_AUDIO_CDMARateFull))
        {
            return VOCODER_BITRATE_EVRC;
        }
        break;

    case OMX_AUDIO_CodingAMR:
       return ((pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeNB7)
             ? VOCODER_PACKET_SIZE_AMR_1220 * VOCODER_PACKET_RATE * 8
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeNB6)
             ? VOCODER_PACKET_SIZE_AMR_1020 * VOCODER_PACKET_RATE * 8
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeNB5)
             ? VOCODER_PACKET_SIZE_AMR_0795 * VOCODER_PACKET_RATE * 8
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeNB4)
             ? VOCODER_PACKET_SIZE_AMR_0740 * VOCODER_PACKET_RATE * 8
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeNB3)
             ? VOCODER_PACKET_SIZE_AMR_0670 * VOCODER_PACKET_RATE * 8
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeNB2)
             ? VOCODER_PACKET_SIZE_AMR_0590 * VOCODER_PACKET_RATE * 8
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeNB1)
             ? VOCODER_PACKET_SIZE_AMR_0515 * VOCODER_PACKET_RATE * 8
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeNB0)
             ? VOCODER_PACKET_SIZE_AMR_0475 * VOCODER_PACKET_RATE * 8
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == (OMX_AUDIO_AMRBANDMODETYPE)QOMX_AUDIO_AMRBandModeWB15)
             ? 4000
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == (OMX_AUDIO_AMRBANDMODETYPE)QOMX_AUDIO_AMRBandModeWB14)
             ? 24000
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode ==(OMX_AUDIO_AMRBANDMODETYPE) QOMX_AUDIO_AMRBandModeWB13)
             ? 24000
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == (OMX_AUDIO_AMRBANDMODETYPE)QOMX_AUDIO_AMRBandModeWB12)
             ? 24000
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == (OMX_AUDIO_AMRBANDMODETYPE)QOMX_AUDIO_AMRBandModeWB11)
             ? 18000
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == (OMX_AUDIO_AMRBANDMODETYPE)QOMX_AUDIO_AMRBandModeWB10)
             ? 13600
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == (OMX_AUDIO_AMRBANDMODETYPE)QOMX_AUDIO_AMRBandModeWB9)
             ? 4000
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeWB8)
             ? 23850
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeWB7)
             ? 23050
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeWB6)
             ? 19850
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeWB5)
             ? 18250
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeWB4)
             ? 15850
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeWB3)
             ? 14250
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeWB2)
             ? 12650
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeWB1)
             ? 8850
             : (pAudioConfig->sAMRWBInfo.eAMRBandMode == OMX_AUDIO_AMRBandModeWB0)
             ? 6600
             : 23850 /*Maximum AMR bitrate*/);

    case OMX_AUDIO_CodingAAC:
        return (pAudioConfig->sAACInfo.nBitRate ?
                pAudioConfig->sAACInfo.nBitRate
                 : 343000);

    case OMX_AUDIO_CodingPCM:
    case OMX_AUDIO_CodingG711:
        return (OMX_U32)0;

    case OMX_AUDIO_CodingUnused:
        /**---------------------------------------------------------------------
            Coding Unused. Nothing to fill
        ------------------------------------------------------------------------
        */
        return (OMX_U32)0;

    default:
        return (OMX_U32)48000;
    }
    return OMX_TRUE;
}



/*==============================================================================

         FUNCTION:         OMX_FileMux_PrintStatistics

         DESCRIPTION:
*//**       @brief         This function validates audio video codec support
                           with the file brand set.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
void OMX_FileMux::OMX_FileMux_PrintStatistics()
{
    MUX_stream_create_params_type *pStreams =
             (MUX_stream_create_params_type*) pFileMuxStreams;

    OMX_U8  sFileName[FS_FILENAME_MAX_LENGTH_P];

    /**-------------------------------------------------------------------------
           If audio not present video params goes first
    ----------------------------------------------------------------------------
    */
    OMX_U32 nAudioIndex = 0;

    OMX_FileMux_CopyString(sFileName, pContentURI, FS_FILENAME_MAX_LENGTH_P);

    MM_MSG_PRIO(MM_STATISTICS, MM_PRIO_HIGH,
        "--------------------------------------------------------------");
    MM_MSG_PRIO(MM_STATISTICS, MM_PRIO_HIGH,
        "<<<<        FileMux Component Recording Statistics        >>>>");
    MM_MSG_PRIO(MM_STATISTICS, MM_PRIO_HIGH,
        "--------------------------------------------------------------");

    MM_MSG_SPRINTF_PRIO_1( MM_STATISTICS, MM_PRIO_HIGH,
        "Mux Stats:  File Name                      = %s", sFileName);

    if(sMuxStats.pRole)
    MM_MSG_SPRINTF_PRIO_1(MM_STATISTICS, MM_PRIO_HIGH,
        "Mux Stats:  File Format                    = %s", sMuxStats.pRole );

    if(IS_AUDIO_PORT_USED)
    {
        if(sMuxStats.pAudioCodec)
        MM_MSG_SPRINTF_PRIO_1(MM_STATISTICS, MM_PRIO_HIGH,
        "Mux Stats:  AudioCodec                     = %s",
        sMuxStats.pAudioCodec );

        if(pStreams)
        MM_MSG_PRIO1(MM_STATISTICS, MM_PRIO_HIGH,
        "Mux Stats:  AudioSampleRate                = %d",
        pStreams[nAudioIndex].subinfo.audio.sampling_frequency );

        if(pStreams)
        MM_MSG_PRIO1(MM_STATISTICS, MM_PRIO_HIGH,
        "Mux Stats:  Audio Num Channels             = %d",
        pStreams[nAudioIndex].subinfo.audio.num_channels );

        MM_MSG_PRIO1(MM_STATISTICS, MM_PRIO_HIGH,
        "Mux Stats:  Audio Duration                 = %ld",
        sMuxStats.nAudioDuration / 1000
        );

        MM_MSG_PRIO1(MM_STATISTICS, MM_PRIO_HIGH,
        "Mux Stats:  Audio Frames Dropped           = %ld",
        sMuxStats.nNumAudFramesDropped
        );

        MM_MSG_PRIO1(MM_STATISTICS, MM_PRIO_HIGH,
        "Mux Stats:  Audio Silence Frames Inserted  = %ld",
        sMuxStats.nSilentFramesInserted
        );

        if(sMuxStats.nAudioDuration)
        MM_MSG_PRIO1(MM_STATISTICS, MM_PRIO_HIGH,
        "Mux Stats:  Audio Average Bitrate          = %ld",
        (OMX_U32)(((OMX_U64)sMuxStats.nAudioBytes * 8 * 1000000) /
                             sMuxStats.nAudioDuration)
        );
    }

    if(IS_VIDEO_PORT_USED)
    {
        if(sMuxStats.pVideoCodec)
        MM_MSG_SPRINTF_PRIO_1(MM_STATISTICS, MM_PRIO_HIGH,
        "Mux Stats:  VideoCodec                     = %s",
        sMuxStats.pVideoCodec );

        MM_MSG_PRIO1(MM_STATISTICS, MM_PRIO_HIGH,
        "Mux Stats:  Video Duration                 = %ld",
        sMuxStats.nVideoDuration / 1000
        );

        MM_MSG_PRIO1(MM_STATISTICS, MM_PRIO_HIGH,
        "Mux Stats:  Video Frames Dropped           = %ld",
        sMuxStats.nNumVidFramesDropped
        );

        if(sMuxStats.nVideoDuration)
        MM_MSG_PRIO1(MM_STATISTICS, MM_PRIO_HIGH,
        "Mux Stats:  Video Average Bitrate          = %llu",
        (((OMX_U64)sMuxStats.nVideoBytes * 8 * (OMX_U64)1000000.0) /
                             sMuxStats.nVideoDuration)
        );

        if(sMuxStats.nVideoDuration)
        MM_MSG_PRIO1(MM_STATISTICS, MM_PRIO_HIGH,
        "Mux Stats:  Video Average FPS              = %llu",
        (((OMX_U64)sMuxStats.nNumVidFramesWritten *(OMX_U32) 1000000.0)/
                        sMuxStats.nVideoDuration)
        );
    }
    return;
}







