#ifndef __ACDB_DEPRECATE_API_H__
#define __ACDB_DEPRECATE_API_H__
/*===========================================================================
    @file   acdb_deprecate.h

    This file contains the public deprecated interface to the Audio Calibration
    Database (ACDB) module.

    The public deprcated interface of the Audio Calibration Database (ACDB) module,
    providing calibration storage for the audio and the voice path. The ACDB
    module is coupled with Qualcomm Audio Calibration Tool (QACT) v2.x.x.
    Qualcomm recommends to always use the latest version of QACT for
    necessary fixes and compatibility.

                    Copyright (c) 2010-2012 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal2/common/8k_PL/acdb/rel/4.2/inc/acdb_deprecate.h#1 $ */

/*===========================================================================

EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------

07/29/11   ernanl  1. Introduced first version of MSM8960/MDM9615 deprecated ACDB API.

===========================================================================*/

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#ifdef ACDB_SIM_ENV
    #include "mmdefs.h"
#endif

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

/* ================================ Deprecated PID ==============================*/
/**
   @def ACDB_PID_ADIE_CODEC_PATH_TIMPANI_V2
   @brief This is the Parameter ID identifying the format of the Adie Codec
          path profile structure.
   @see This structure is associate with Adie codec path 
*/
#define ACDB_PID_ADIE_CODEC_PATH_TIMPANI_V2                       0x0001122C
/**
struct _AcdbAdieCodecACDBHeader
{
  uint32_t                        size;
  uint32_t                        version;
  AdieCodecPathId                 ePathID;
  AdieCodecDataPortIDType         eDataPortID;
  AdieCodecDevicePortIDType       eDevPortID[ADIE_CODEC_MAX_ATTACHED_DEVICES];
  AdieCodecPathFormatType         ePathFormat;
  AdieCodecPathType		          ePathType;
  AdieCodecPathFrequencyPlan      iPathFreq;
  AdieCodecPathOSR                iPathOSR;
  AdieCodecPathUnitType           ePathUnit;
  uint32_t                        numActions;
} AcdbAdieCodecACDBHeader;

struct _AcdbAdieCodecDbActionUnit {
	uint32_t  nActionType; 	
	uint32_t  nActionEntry;
} AcdbAdieCodecDbActionUnit;

struct _AcdbAdieCodecDbProfile {
	 AcdbAdieCodecACDBHeader nHeader;
	 AcdbAdieCodecDbActionUnit nActionUnit [numActions];
} AcdbAdieCodecDbProfile;
*/

/**
   @def ACDB_PID_CODEC_DATA_TIMPANI
   @brief This is the Parameter ID identifying the format of the ANC config
          data profile structure.
   @see This structure is associate with ANC config data 
*/
#define ACDB_PID_CODEC_DATA_TIMPANI                     0x000111F7
/**
//ANC Config data structure
typedef struct
{
  DALBOOL                       bDefaultEnbl; 			
  DALBOOL                       bANC_FeedBackEnbl;		
  DALBOOL				           bANC_LRMixEnbl;
  AdieCodecANCChannelType       eANC_LRMixOutputChannel; // Only used when bANC_LRMixEnbl
  int32                         iANC_FFCoeff[15];   // IIR coeffs: B[0-7] A[8-14] -- Q2.13 signed
  int32                         iANC_FBCoeff[13];   // IIR coeffs: B[0-6] A[7-12] -- Q2.13 signed
  uint32                        iANC_FF_Shift;      // 4 bit
  uint32                        iANC_FB_Shift;      // 4 bit
  int32                         iANC_FF_LPFCoeff[5];    // LPF coeffs: B[0-1] A[2-4] -- Q2.13 signed
  int32                         iANC_FB_LPFCoeff[5];    // LPF coeffs: B[0-1] A[2-4] -- Q2.13 signed
  uint32                        iANC_AGCTargetAmplitude;
  AdieCodecDbANCChnlConfigType  ancChnlConfig[ADIE_CODEC_ANC_CHANNEL_NUM];
} AdieCodecDbANCConfigType;

// AdieCodecDbANCChnlConfigType
typedef struct
{
  AdieCodecDevicePortIDType     eANC_InputDevice;
  DALBOOL                       bANC_InputLRSwap;      // If set swap L input with R input
  int32                         iANC_Gain;           // Gain dB * 2
  int32                         iANC_Gain_Default;   // Gain dB * 2
} AdieCodecDbANCChnlConfigType;
*/

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_DEVICE_INFO Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_DEVICE_INFO, ...)
   @brief API to query for the device information for a single device.

   This command will obtain the detailed device information describing
   the device properties of a given device id.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_DEVICE_INFO.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDeviceInfoCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDeviceInfoCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbDeviceInfoCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_DEVICE_INFO             0x00011113

/**
   @struct   AcdbDeviceInfoCmdType
   @brief This is a query command structure for the device info.

   @param nDeviceId: The Device ID to lookup
   @param nParamId: The parameter id identifying the payload format
                    contained in the buffer.
   @param nBufferLength: The length of the buffer. This should be
                    large enough to hold the entire device info structure
                    identified by nParamId.
   @param nBufferPointer: A virtual memory pointer pointing to the memory
                    region ACDB will copy the device info into.

   This is a query command structure for the device info.
*/
typedef struct _AcdbDeviceInfoCmdType {
   uint32_t nDeviceId;
   uint32_t nParamId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbDeviceInfoCmdType;

/**
   @def ACDB_DEVICE_INFO_PARAM
   @brief This is the Parameter ID identifying the format of the device
          information structure.
   @see AcdbDeviceInfoType
*/
#define ACDB_DEVICE_INFO_PARAM                           0x0001112E

/**
   @struct   AcdbDeviceInfoType
   @brief This is the device information structure containing device metadata
        for the audio driver.

   @param ulSampleRateMask: The sample rate mask describing what sample rates
                     are possible for this device. This mask will
                     contain bits on or off depending on whether
                     the device will support the samplerate at that
                     rate. The acceptable values are:
                        (0x00000001)  8000 hz
                        (0x00000002) 11025 hz
                        (0x00000004) 12000 hz
                        (0x00000008) 16000 hz
                        (0x00000010) 22050 hz
                        (0x00000020) 24000 hz
                        (0x00000040) 32000 hz
                        (0x00000080) 44100 hz
                        (0x00000100) 48000 hz
   @param ulOverSamplerateId: The over-sample rate the codec will be run at.
                           (0xFFFFFFFF) Information is not applicable
                           (0x00000000) 0 hz
                           (0x00000001) 64 hz
                           (0x00000002) 128 hz
                           (0x00000003) 256 hz
   @param ulI2sMClkMode: Special mode for I2S devices allowing for clock choice:
                           (0xFFFFFFFF) Information is not applicable
                           (0x00000000): MCLK Master 
                           (0x00000001): MCLK Slave
   @param ulChannelConfig: The channel configuration of the device. Valid 
                           choices are:
                              (0x00000001) MONO
                              (0x00000002) STEREO
                              (0x00000004) 4-channels
                              (0x00000006) 6-channels
                              (0x00000008) 8-channels
   @param ulDirectionMask: The direction of the device. Valid values are:
                              (0xFFFFFFFF) Information is not applicable
                              (0x00000001) RX
                              (0x00000002) TX
                              (0x00000004) AUXPGA
   @param ulCodecPathId: The codec path id associated with this device.
   @param ulAfePortId: This is the AFE port id that this device is to be
                       associated with. The acceptable values are:
                     (0xFFFFFFFF) Information is not applicable
                     (0x00000000) Primary I2S RX
                     (0x00000001) Primary I2S TX
                     (0x00000002) PCM RX
                     (0x00000003) PCM TX
                     (0x00000004) Secondary I2S RX
                     (0x00000005) Secondary I2S TX
                     (0x00000006) MI2S RX
                     (0x00000007) Reserved
                     (0x00000008) HDMI RX
                     (0x00000009) Reserved
                     (0x0000000A) Reserved
                     (0x0000000B) Digital Quad Mic TX
   @param ulAfeChannelMode: I2S channel mode definition is dependent on ulAfePortId.
                            General:
                               (0xFFFFFFFF) Information is not applicable
                            For Primary, Secondary and Multi I2S ports:
                               (0x00000001) MI2S Channel Mode 0 (sd0)
                               (0x00000002) MI2S Channel Mode 1 (sd1)
                               (0x00000003) MI2S Channel Mode 2 (sd2)
                               (0x00000004) MI2S Channel Mode 3 (sd3)
                               (0x00000005) MI2S Channel Mode 4 (sd0/sd1)
                               (0x00000006) MI2S Channel Mode 5 (sd2/sd3)
                               (0x00000007) MI2S Channel Mode 6 (6 ch)
                               (0x00000008) MI2S Channel Mode 7 (8 ch)
                            For PCM ports:
                               (0x00000000) Alaw with no padding
                               (0x00000001) mulaw with no padding
                               (0x00000002) linear with no padding
                               (0x00000003) Alaw with padding
                               (0x00000004) mulaw with padding
                               (0x00000005) linear with padding
                            For Digital Mic ports:
                               (0x00000000) Reserved
                               (0x00000001) Mode Left 0
                               (0x00000002) Mode Right 0
                               (0x00000003) Mode Left 1
                               (0x00000004) Mode Right 1
                               (0x00000005) Mode Stereo 0
                               (0x00000006) Mode Stereo 2
                               (0x00000007) Mode Quad
                            For HDMI ports:
                               (0x00000000) Stereo
                               (0x00000001) 3.1 speakers
                               (0x00000002) 5.1 speakers
                               (0x00000003) 7.1 speakers 
   @param ulAfeBytesPerSampleMask: The BPS mask describes at what bit-rate the
                                AFE can operate at. Valid choices are:
                               (0x00000001) 16 bit PCM
                              (0x00000002) 24 bit PCM
                              (0x00000004) 32 bit PCM
   @param ulAfePortConfig: Afe port configuration information.
                            General
                                  (0xFFFFFFFF) Information is not applicable
                            For I2S ports:
                                  (0x00000000) Mono both channels
                                  (0x00000001) Mono right channel only
                                  (0x00000002) Mono left channel only
                                  (0x00000003) Stereo 
                            For AUXPCM ports:
                                  (0x00000000) 8  Bits per frame
                                  (0x00000001) 16 Bits per frame
                                  (0x00000002) 32 Bits per frame
                                  (0x00000003) 64 Bits per frame
                                  (0x00000004) 128 Bits per frame
                                  (0x00000005) 256 Bits per frame    
                                  (0x00000010) PCM mode bitmask (0 - PCM mode, 1 - AUX mode)
                                  (0x00000100) PCM Ctrl data OE bitmask ( 0 - disable, 1 - enable)
                            For HDMI ports:
                                  (0x00000000) linear
                                  (0x00000001) non-linear
                            For Digital Mic ports:
                                  (0x00000000) Reserved
   @param ulAfeSyncSrcSelect: AFE Sync source selection
                              General:
                                 (0xFFFFFFFF) Information is not applicable
                              For I2S ports:
                                 (0x00000000) Word Select Source from external
                                 (0x00000001) Word Select Source from internal
                              For PCM ports:
                                 (0x00000000) PCM Sync source from external
                                 (0x00000001) PCM Sync source from internal
                              For HDMI and Digi Mic ports:
                                 (0x00000000) Reserved
   @param ulVoiceTopologyId: Topology of the voice COPP
                        The acceptable values are as follows.
                        For Tx Devices:
                           0x00010F70 NULL
                           0x00010F71 SingleMicEcns
                           0x00010F72 DualMicFluence
                           0x00010F73 SingleMicQuartet
                           0x00010F74 DualMicQuartet
                           0x00010F75 QuadMicQuartet
                        For Rx Devices:
                           0x00010F76 NULL
                           0x00010F77 DEFAULT_VOICE
   @param ulAudioTopologyId: Topology of the audio COPP
                        The acceptable values are as follows.
                        For Tx Devices:
                           0x00011164 DEFAULT_AUDIO
                        For Rx Devices:
                           0x00011165 DEFAULT_AUDIO
   This is the device information structure containing device metadata for
   the audio driver. This structure will be identified by the device info
   parameter id of 'ACDB_DEVICE_INFO_PARAM'.
   */
typedef struct _AcdbDeviceInfoType {
   uint32_t ulSampleRateMask;
   uint32_t ulOverSamplerateId;
   uint32_t ulI2sMClkMode;
   uint32_t ulChannelConfig;
   uint32_t ulDirectionMask;
   uint32_t ulCodecPathId;
   uint32_t ulAfePortId;
   uint32_t ulAfeChannelMode;
   uint32_t ulAfeBytesPerSampleMask;
   uint32_t ulAfePortConfig;
   uint32_t ulAfeSyncSrcSelect;
   uint32_t ulVoiceTopologyId;
   uint32_t ulAudioTopologyId;
} AcdbDeviceInfoType;


#define ACDB_DEVICE_INFO_PARAM2                 0x000111FB
/**
   @struct   AcdbDeviceInfo2Type
   @brief This is the device information structure containing device metadata
        for the audio driver.

   @param ulSampleRateMask: The sample rate mask describing what sample rates
                     are possible for this device. This mask will
                     contain bits on or off depending on whether
                     the device will support the samplerate at that
                     rate. The acceptable values are:
                        (0x00000001)  8000 hz
                        (0x00000002) 11025 hz
                        (0x00000004) 12000 hz
                        (0x00000008) 16000 hz
                        (0x00000010) 22050 hz
                        (0x00000020) 24000 hz
                        (0x00000040) 32000 hz
                        (0x00000080) 44100 hz
                        (0x00000100) 48000 hz
   @param ulOverSamplerateId: The over-sample rate the codec will be run at.
                           (0xFFFFFFFF) Information is not applicable
                           (0x00000001) 64 hz
                           (0x00000002) 128 hz
                           (0x00000003) 256 hz
   @param ulI2sMClkMode: Special mode for I2S devices allowing for clock choice:
                           (0xFFFFFFFF) Information is not applicable
                           (0x00000000): MCLK Master 
                           (0x00000001): MCLK Slave
   @param ulChannelConfig: The channel configuration of the device. Valid 
                           choices are:
                              (0x00000001) MONO
                              (0x00000002) STEREO
                              (0x00000004) 4-channels
                              (0x00000006) 6-channels
                              (0x00000008) 8-channels
   @param ulDirectionMask: The direction of the device. Valid values are:
                              (0xFFFFFFFF) Information is not applicable
                              (0x00000001) RX
                              (0x00000002) TX
                              (0x00000004) AUXPGA
   @param ulCodecPathId: The codec path id associated with this device.
   @param ulCodecPathId2: The codec path id associated with this device.
   @param ulAfePortId: This is the AFE port id that this device is to be
                       associated with. The acceptable values are:
                     (0xFFFFFFFF) Information is not applicable
                     (0x00000000) Primary I2S RX
                     (0x00000001) Primary I2S TX
                     (0x00000002) PCM RX
                     (0x00000003) PCM TX
                     (0x00000004) Secondary I2S RX
                     (0x00000005) Secondary I2S TX
                     (0x00000006) MI2S RX
                     (0x00000007) Reserved
                     (0x00000008) HDMI RX
                     (0x00000009) Reserved
                     (0x0000000A) Reserved
                     (0x0000000B) Digital Quad Mic TX
   @param ulAfeChannelMode: I2S channel mode definition is dependent on ulAfePortId.
                            General:
                               (0xFFFFFFFF) Information is not applicable
                            For Primary, Secondary and Multi I2S ports:
                               (0x00000001) MI2S Channel Mode 0 (sd0)
                               (0x00000002) MI2S Channel Mode 1 (sd1)
                               (0x00000003) MI2S Channel Mode 2 (sd2)
                               (0x00000004) MI2S Channel Mode 3 (sd3)
                               (0x00000005) MI2S Channel Mode 4 (sd0/sd1)
                               (0x00000006) MI2S Channel Mode 5 (sd2/sd3)
                               (0x00000007) MI2S Channel Mode 6 (6 ch)
                               (0x00000008) MI2S Channel Mode 7 (8 ch)
                            For PCM ports:
                               (0x00000000) Alaw with no padding
                               (0x00000001) mulaw with no padding
                               (0x00000002) linear with no padding
                               (0x00000003) Alaw with padding
                               (0x00000004) mulaw with padding
                               (0x00000005) linear with padding
                            For Digital Mic ports:
                               (0x00000000) Reserved
                               (0x00000001) Mode Left 0
                               (0x00000002) Mode Right 0
                               (0x00000003) Mode Left 1
                               (0x00000004) Mode Right 1
                               (0x00000005) Mode Stereo 0
                               (0x00000006) Mode Stereo 2
                               (0x00000007) Mode Quad
                            For HDMI ports:
                               (0x00000000) Stereo
                               (0x00000001) 3.1 speakers
                               (0x00000002) 5.1 speakers
                               (0x00000003) 7.1 speakers 
   @param ulAfeBytesPerSampleMask: The BPS mask describes at what bit-rate the
                                AFE can operate at. Valid choices are:
                              (0x00000001) 16 bit PCM
                              (0x00000002) 24 bit PCM
                              (0x00000004) 32 bit PCM
   @param ulAfePortConfig: Afe port configuration information.
                            General:
                                  (0xFFFFFFFF) Information is not applicable
                            For I2S ports:
                                  (0x00000000) Mono both channels
                                  (0x00000001) Mono right channel only
                                  (0x00000002) Mono left channel only
                                  (0x00000003) Stereo 
                            For AUXPCM ports:
                                  (0x00000000) 8  Bits per frame
                                  (0x00000001) 16 Bits per frame
                                  (0x00000002) 32 Bits per frame
                                  (0x00000003) 64 Bits per frame
                                  (0x00000004) 128 Bits per frame
                                  (0x00000005) 256 Bits per frame    
                                  (0x00000010) PCM mode bitmask (0 - PCM mode, 1 - AUX mode)
                                  (0x00000100) PCM Ctrl data OE bitmask ( 0 - disable, 1 - enable)
                            For HDMI ports:
                                  (0x00000000) linear
                                  (0x00000001) non-linear
                            For Digital Mic ports:
                                  (0x00000000) Reserved
   @param ulAfeSyncSrcSelect: AFE Sync source selection
                              General:
                                 (0xFFFFFFFF) Information is not applicable
                              For I2S ports:
                                 (0x00000000) Word Select Source from external
                                 (0x00000001) Word Select Source from internal
                              For PCM ports:
                                 (0x00000000) PCM Sync source from external
                                 (0x00000001) PCM Sync source from internal
                              For HDMI and Digi Mic ports:
                                 (0x00000000) Reserved
   @param ulVoiceTopologyId: Topology of the voice COPP
                        The acceptable values are as follows.
                        For Tx Devices:
                           0x00010F70 NULL
                           0x00010F71 SingleMicEcns
                           0x00010F72 DualMicFluence
                           0x00010F73 SingleMicQuartet
                           0x00010F74 DualMicQuartet
                           0x00010F75 QuadMicQuartet
                        For Rx Devices:
                           0x00010F76 NULL
                           0x00010F77 DEFAULT_VOICE
   @param ulAudioTopologyId: Topology of the audio COPP
                        The acceptable values are as follows.
                        For Tx Devices:
                           0x00011164 DEFAULT_AUDIO
                        For Rx Devices:
                           0x00011165 DEFAULT_AUDIO
   This is the device information structure containing device metadata for
   the audio driver. This structure will be identified by the device info
   parameter id of 'ACDB_DEVICE_INFO_PARAM2'.
   */
typedef struct _AcdbDeviceInfo2Type {
   uint32_t ulSampleRateMask;
   uint32_t ulOverSamplerateId;
   uint32_t ulI2sMClkMode;
   uint32_t ulChannelConfig;
   uint32_t ulDirectionMask;
   uint32_t ulCodecPathId;
   uint32_t ulCodecPathId2;
   uint32_t ulAfePortId;
   uint32_t ulAfeChannelMode;
   uint32_t ulAfeBytesPerSampleMask;
   uint32_t ulAfePortConfig;
   uint32_t ulAfeSyncSrcSelect;
   uint32_t ulVoiceTopologyId;
   uint32_t ulAudioTopologyId;
} AcdbDeviceInfo2Type;

#define ACDB_DEVICE_INFO_PARAM3                 0x00011215
#define ACDB_DEVICE_NAME_LEN                    100
/**
   @struct   AcdbDeviceInfo3Type
   @brief This is the device information structure containing device metadata
        for the audio driver.

   @param ulSampleRateMask: The sample rate mask describing what sample rates
                     are possible for this device. This mask will
                     contain bits on or off depending on whether
                     the device will support the samplerate at that
                     rate. The acceptable values are:
                        (0x00000001)  8000 hz
                        (0x00000002) 11025 hz
                        (0x00000004) 12000 hz
                        (0x00000008) 16000 hz
                        (0x00000010) 22050 hz
                        (0x00000020) 24000 hz
                        (0x00000040) 32000 hz
                        (0x00000080) 44100 hz
                        (0x00000100) 48000 hz
   @param ulOverSamplerateId: The over-sample rate the codec will be run at.
                           (0xFFFFFFFF) Information is not applicable
                           (0x00000001) 64 hz
                           (0x00000002) 128 hz
                           (0x00000003) 256 hz
   @param ulI2sMClkMode: Special mode for I2S devices allowing for clock choice:
                           (0xFFFFFFFF) Information is not applicable
                           (0x00000000): MCLK Master 
                           (0x00000001): MCLK Slave
   @param ulChannelConfig: The channel configuration of the device. Valid 
                           choices are:
                              (0x00000001) MONO
                              (0x00000002) STEREO
                              (0x00000004) 4-channels
                              (0x00000006) 6-channels
                              (0x00000008) 8-channels
   @param ulDirectionMask: The direction of the device. Valid values are:
                              (0xFFFFFFFF) Information is not applicable
                              (0x00000001) RX
                              (0x00000002) TX
                              (0x00000004) AUXPGA
   @param ulCodecPathId: The codec path id associated with this device.
   @param ulCodecPathId2: The codec path id associated with this device.
   @param ulAfePortId: This is the AFE port id that this device is to be
                       associated with. The acceptable values are:
                     (0xFFFFFFFF) Information is not applicable
                     (0x00000000) Primary I2S RX
                     (0x00000001) Primary I2S TX
                     (0x00000002) PCM RX
                     (0x00000003) PCM TX
                     (0x00000004) Secondary I2S RX
                     (0x00000005) Secondary I2S TX
                     (0x00000006) MI2S RX
                     (0x00000007) Reserved
                     (0x00000008) HDMI RX
                     (0x00000009) Reserved
                     (0x0000000A) Reserved
                     (0x0000000B) Digital Quad Mic TX
   @param ulAfeChannelMode: I2S channel mode definition is dependent on ulAfePortId.
                            General:
                               (0xFFFFFFFF) Information is not applicable
                            For Primary, Secondary and Multi I2S ports:
                               (0x00000001) MI2S Channel Mode 0 (sd0)
                               (0x00000002) MI2S Channel Mode 1 (sd1)
                               (0x00000003) MI2S Channel Mode 2 (sd2)
                               (0x00000004) MI2S Channel Mode 3 (sd3)
                               (0x00000005) MI2S Channel Mode 4 (sd0/sd1)
                               (0x00000006) MI2S Channel Mode 5 (sd2/sd3)
                               (0x00000007) MI2S Channel Mode 6 (6 ch)
                               (0x00000008) MI2S Channel Mode 7 (8 ch)
                            For PCM ports:
                               (0x00000000) Alaw with no padding
                               (0x00000001) mulaw with no padding
                               (0x00000002) linear with no padding
                               (0x00000003) Alaw with padding
                               (0x00000004) mulaw with padding
                               (0x00000005) linear with padding
                            For Digital Mic ports:
                               (0x00000000) Reserved
                               (0x00000001) Mode Left 0
                               (0x00000002) Mode Right 0
                               (0x00000003) Mode Left 1
                               (0x00000004) Mode Right 1
                               (0x00000005) Mode Stereo 0
                               (0x00000006) Mode Stereo 2
                               (0x00000007) Mode Quad
                            For HDMI ports:
                               (0x00000000) Stereo
                               (0x00000001) 3.1 speakers
                               (0x00000002) 5.1 speakers
                               (0x00000003) 7.1 speakers 
   @param ulAfeBytesPerSampleMask: The BPS mask describes at what bit-rate the
                                AFE can operate at. Valid choices are:
                              (0x00000001) 16 bit PCM
                              (0x00000002) 24 bit PCM
                              (0x00000004) 32 bit PCM
   @param ulAfePortConfig: Afe port configuration information.
                            General:
                                  (0xFFFFFFFF) Information is not applicable
                            For I2S ports:
                                  (0x00000000) Mono both channels
                                  (0x00000001) Mono right channel only
                                  (0x00000002) Mono left channel only
                                  (0x00000003) Stereo 
                            For AUXPCM ports:
                                  (0x00000000) 8  Bits per frame
                                  (0x00000001) 16 Bits per frame
                                  (0x00000002) 32 Bits per frame
                                  (0x00000003) 64 Bits per frame
                                  (0x00000004) 128 Bits per frame
                                  (0x00000005) 256 Bits per frame    
                                  (0x00000010) PCM mode bitmask (0 - PCM mode, 1 - AUX mode)
                                  (0x00000100) PCM Ctrl data OE bitmask ( 0 - disable, 1 - enable)
                            For HDMI ports:
                                  (0x00000000) linear
                                  (0x00000001) non-linear
                            For Digital Mic ports:
                                  (0x00000000) Reserved
   @param ulAfeSyncSrcSelect: AFE Sync source selection
                              General:
                                 (0xFFFFFFFF) Information is not applicable
                              For I2S ports:
                                 (0x00000000) Word Select Source from external
                                 (0x00000001) Word Select Source from internal
                              For PCM ports:
                                 (0x00000000) PCM Sync source from external
                                 (0x00000001) PCM Sync source from internal
                              For HDMI and Digi Mic ports:
                                 (0x00000000) Reserved
   @param ulVoiceTopologyId: Topology of the voice COPP
                        The acceptable values are as follows.
                        For Tx Devices:
                           0x00010F70 NULL
                           0x00010F71 SingleMicEcns
                           0x00010F72 DualMicFluence
                           0x00010F73 SingleMicQuartet
                           0x00010F74 DualMicQuartet
                           0x00010F75 QuadMicQuartet
                        For Rx Devices:
                           0x00010F76 NULL
                           0x00010F77 DEFAULT_VOICE
   @param ulTimingMode: Device timing mode
                        The accpetable values are as follow
                        0xFFFFFFFF: Not available information
                        This Device
                        0x00000000: operate rate faster than real time
                        This Device
                        0x00000001: operate rate at system timer rate
   @param ulIsVirtualDevice: Identify this device is a virtual device or not
                        The accpetable values are as follow
                        This Device:
                        0x00000000: is not virtual device
                        This Device:
                        0x00000001: is virtual device
   @param ulDeviceName: The buffer is fixed length of 100 bytes. The buffer will store a 
                        PASCAL-like string where the length field is the first two bytes. 
                        So the maximum length of the device name is 98 unit code characters
                        long.

                        The structure for this PASCAL string will look like the following:
                        struct DeviceName 
                        {
                           uint16 length;
                           uint8 aDeviceName [98];
                        }

   This is the device information structure containing device metadata for
   the audio driver. This structure will be identified by the device info
   parameter id of 'ACDB_DEVICE_INFO_PARAM3'.
   */
typedef struct _AcdbDeviceInfo3Type {
   uint32_t ulSampleRateMask;
   uint32_t ulOverSamplerateId;
   uint32_t ulI2sMClkMode;
   uint32_t ulChannelConfig;
   uint32_t ulDirectionMask;
   uint32_t ulCodecPathId;
   uint32_t ulCodecPathId2;
   uint32_t ulAfePortId;
   uint32_t ulAfeChannelMode;
   uint32_t ulAfeBytesPerSampleMask;
   uint32_t ulAfePortConfig;
   uint32_t ulAfeSyncSrcSelect;
   uint32_t ulVoiceTopologyId;
   uint32_t ulTimingMode;
   uint32_t ulIsVirtualDevice;
   uint8_t ulDeviceName[ACDB_DEVICE_NAME_LEN];
} AcdbDeviceInfo3Type;

#define ACDB_DEVICE_INFO_PARAM4                 0x00011284
/**
   @struct   AcdbDeviceInfo3Type
   @brief This is the device information structure containing device metadata
        for the audio driver.

   @param ulSampleRateMask: The sample rate mask describing what sample rates
                     are possible for this device. This mask will
                     contain bits on or off depending on whether
                     the device will support the samplerate at that
                     rate. The acceptable values are:
                        (0x00000001)  8000 hz
                        (0x00000002) 11025 hz
                        (0x00000004) 12000 hz
                        (0x00000008) 16000 hz
                        (0x00000010) 22050 hz
                        (0x00000020) 24000 hz
                        (0x00000040) 32000 hz
                        (0x00000080) 44100 hz
                        (0x00000100) 48000 hz
   @param ulOverSamplerateId: The over-sample rate the codec will be run at.
                           (0xFFFFFFFF) Information is not applicable
                           (0x00000001) 64 hz
                           (0x00000002) 128 hz
                           (0x00000003) 256 hz
   @param ulI2sMClkMode: Special mode for I2S devices allowing for clock choice:
                           (0xFFFFFFFF) Information is not applicable
                           (0x00000000): MCLK Master 
                           (0x00000001): MCLK Slave
   @param ulChannelConfig: The channel configuration of the device. Valid 
                           choices are:
                              (0x00000001) MONO
                              (0x00000002) STEREO
                              (0x00000004) 4-channels
                              (0x00000006) 6-channels
                              (0x00000008) 8-channels
   @param ulDirectionMask: The direction of the device. Valid values are:
                              (0xFFFFFFFF) Information is not applicable
                              (0x00000001) RX
                              (0x00000002) TX
                              (0x00000004) AUXPGA
   @param ulCodecPathId: The codec path id associated with this device.
   @param ulCodecPathId2: The codec path id associated with this device.
   @param ulAfePortId: This is the AFE port id that this device is to be
                       associated with. The acceptable values are:
                     (0xFFFFFFFF) Information is not applicable
                     (0x00000000) Primary I2S RX
                     (0x00000001) Primary I2S TX
                     (0x00000002) PCM RX
                     (0x00000003) PCM TX
                     (0x00000004) Secondary I2S RX
                     (0x00000005) Secondary I2S TX
                     (0x00000006) MI2S RX
                     (0x00000007) Reserved
                     (0x00000008) HDMI RX
                     (0x00000009) Reserved
                     (0x0000000A) Reserved
                     (0x0000000B) Digital Quad Mic TX
   @param ulAfeChannelMode: I2S channel mode definition is dependent on ulAfePortId.
                            General:
                               (0xFFFFFFFF) Information is not applicable
                            For Primary, Secondary and Multi I2S ports:
                               (0x00000001) MI2S Channel Mode 0 (sd0)
                               (0x00000002) MI2S Channel Mode 1 (sd1)
                               (0x00000003) MI2S Channel Mode 2 (sd2)
                               (0x00000004) MI2S Channel Mode 3 (sd3)
                               (0x00000005) MI2S Channel Mode 4 (sd0/sd1)
                               (0x00000006) MI2S Channel Mode 5 (sd2/sd3)
                               (0x00000007) MI2S Channel Mode 6 (6 ch)
                               (0x00000008) MI2S Channel Mode 7 (8 ch)
                            For PCM ports:
                               (0x00000000) Alaw with no padding
                               (0x00000001) mulaw with no padding
                               (0x00000002) linear with no padding
                               (0x00000003) Alaw with padding
                               (0x00000004) mulaw with padding
                               (0x00000005) linear with padding
                            For Digital Mic ports:
                               (0x00000000) Reserved
                               (0x00000001) Mode Left 0
                               (0x00000002) Mode Right 0
                               (0x00000003) Mode Left 1
                               (0x00000004) Mode Right 1
                               (0x00000005) Mode Stereo 0
                               (0x00000006) Mode Stereo 2
                               (0x00000007) Mode Quad
                            For HDMI ports:
                               (0x00000000) Stereo
                               (0x00000001) 3.1 speakers
                               (0x00000002) 5.1 speakers
                               (0x00000003) 7.1 speakers 
   @param ulAfeBytesPerSampleMask: The BPS mask describes at what bit-rate the
                                AFE can operate at. Valid choices are:
                              (0x00000001) 16 bit PCM
                              (0x00000002) 24 bit PCM
                              (0x00000004) 32 bit PCM
   @param ulAfePortConfig: Afe port configuration information.
                            General:
                                  (0xFFFFFFFF) Information is not applicable
                            For I2S ports:
                                  (0x00000000) Mono both channels
                                  (0x00000001) Mono right channel only
                                  (0x00000002) Mono left channel only
                                  (0x00000003) Stereo 
                            For AUXPCM ports:
                                  (0x00000000) 8  Bits per frame
                                  (0x00000001) 16 Bits per frame
                                  (0x00000002) 32 Bits per frame
                                  (0x00000003) 64 Bits per frame
                                  (0x00000004) 128 Bits per frame
                                  (0x00000005) 256 Bits per frame    
                                  (0x00000010) PCM mode bitmask (0 - PCM mode, 1 - AUX mode)
                                  (0x00000100) PCM Ctrl data OE bitmask ( 0 - disable, 1 - enable)
                            For HDMI ports:
                                  (0x00000000) linear
                                  (0x00000001) non-linear
                            For Digital Mic ports:
                                  (0x00000000) Reserved
   @param ulAfeSyncSrcSelect: AFE Sync source selection
                              General:
                                 (0xFFFFFFFF) Information is not applicable
                              For I2S ports:
                                 (0x00000000) Word Select Source from external
                                 (0x00000001) Word Select Source from internal
                              For PCM ports:
                                 (0x00000000) PCM Sync source from external
                                 (0x00000001) PCM Sync source from internal
                              For HDMI and Digi Mic ports:
                                 (0x00000000) Reserved
   @param ulVoiceTopologyId: Topology of the voice COPP
                        The acceptable values are as follows.
                        For Tx Devices:
                           0x00010F70 NULL
                           0x00010F71 SingleMicEcns
                           0x00010F72 DualMicFluence
                           0x00010F73 SingleMicQuartet
                           0x00010F74 DualMicQuartet
                           0x00010F75 QuadMicQuartet
                        For Rx Devices:
                           0x00010F76 NULL
                           0x00010F77 DEFAULT_VOICE
   @param ulTimingMode: Device timing mode
                        The accpetable values are as follow
                        0xFFFFFFFF: Not available information
                        This Device
                        0x00000000: operate rate faster than real time
                        This Device
                        0x00000001: operate rate at system timer rate
   @param ulIsVirtualDevice: Identify this device is a virtual device or not
                        The accpetable values are as follow
                        This Device:
                        0x00000000: is not virtual device
                        This Device:
                        0x00000001: is virtual device
   @param ulDeviceName: The buffer is fixed length of 100 bytes. The buffer will store a 
                        PASCAL-like string where the length field is the first two bytes. 
                        So the maximum length of the device name is 98 unit code characters
                        long.

                        The structure for this PASCAL string will look like the following:
                        struct DeviceName 
                        {
                           uint16 length;
                           uint8 aDeviceName [98];
                        }
   @param ulDeviceType: Identify this device type
                        The accpetable values are as follow
                        This Device:
                        0x00000000: is real/physical device
                        This Device:
                        0x00000001: is virtual device
                        This Device:
                        0x00000002: is proxy device

   This is the device information structure containing device metadata for
   the audio driver. This structure will be identified by the device info
   parameter id of 'ACDB_DEVICE_INFO_PARAM4'.
   */
typedef struct _AcdbDeviceInfo4Type {
   uint32_t ulSampleRateMask;
   uint32_t ulOverSamplerateId;
   uint32_t ulI2sMClkMode;
   uint32_t ulChannelConfig;
   uint32_t ulDirectionMask;
   uint32_t ulCodecPathId;
   uint32_t ulCodecPathId2;
   uint32_t ulAfePortId;
   uint32_t ulAfeChannelMode;
   uint32_t ulAfeBytesPerSampleMask;
   uint32_t ulAfePortConfig;
   uint32_t ulAfeSyncSrcSelect;
   uint32_t ulVoiceTopologyId;
   uint32_t ulTimingMode;
   uint32_t ulIsVirtualDevice;   
   uint8_t ulDeviceName[ACDB_DEVICE_NAME_LEN];
   uint32_t ulDeviceType;
} AcdbDeviceInfo4Type;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL, ...)
   @brief API to query for a audio volume table, along with it's gain-dependent
         calibration.

   This command will obtain the specific volume table and gain-dependent
   calibration tables queried for.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAudProcVolTblCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAudProcVolTblCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbAudProcGainDepVolTblRspType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbAudProcGainDepVolTblRspType).

   @see  acdb_ioctl
   @see  AcdbAudProcVolTblCmdType
   @see  AcdbAudProcGainDepVolTblRspType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL              0x000111B7

/**
   @struct   AcdbAudProcVolTblCmdType
   @brief This is a query command structure for the gain-dependent volume tables.

   @param nDeviceId: The Device ID
   @param nApplicationType: The application type ID
   @param nCoppBufferLength: The length of the calibration buffer. This should be
                         large enough to hold the entire COPP gain dependent
                         calibration table.
   @param nCoppBufferPointer: A virtual memory pointer pointing to the memory region 
                         ACDB will copy the gain-dependent calibration table into. 
                         The format of the table will match the expected table
                         format in the QDSP6 API. In this case, the table format
                         will be the repetition of this structure:

                         struct AudProcVolTableEntry {
                         uint32_t nModuleId;
                         uint32_t nParamId;
                         uint16_t nParamSize; //multiple of 4
                         uint16_t nReserved; // Must be 0
                         uint8 aParamData [multiple of 4)];
                         }
                         struct AudProcVolTable {
                         uint32_t nTableSize;
                         AudProcVolTableEntry aGainDepTables [];
                         }
                         struct AudProcGainDepVolTable {
                         uint32_t nStepCount;
                         AudProcVolTable aVolumeTable [nStepCount];
                         }

   @param nPoppBufferLength: The length of the calibration buffer. This should be
                         large enough to hold the entire POPP gain dependent
                         calibration table.
   @param nPoppBufferPtr: A virtual memory pointer pointing to the memory region 
                         ACDB will copy the gain-dependent calibration table into. 
                         The format of the table will match the expected table
                         format in the QDSP6 API. In this case, the table format
                         will be the repetition of this structure:
                         
                         struct AudProcVolTableEntry {
                         uint32_t nModuleId;
                         uint32_t nParamId;
                         uint16_t nParamSize //multiple of 4;
                         uint16_t nReserved; // Must be 0
                         uint8 aParamData [multiple of 4];
                         }
                         struct AudProcVolTable {
                         uint32_t nTableSize;
                         AudProcVolTableEntry aGainDepTables [];
                         }
                         struct AudProcGainDepVolTable {
                         uint32_t nStepCount;
                         AudProcVolTable aVolumeTable [nStepCount];
                         }
                           
   This is a query command structure for the gain-dependent audio volume tables. The
   audio volume tables have to be split into two parts to allow the flexibility of
   adjusting modules on the POPP and the COPP independently.
*/
typedef struct _AcdbAudProcVolTblCmdType {
   uint32_t nDeviceId;
   uint32_t nApplicationType;
   uint32_t nCoppBufferLength;
   uint8_t *nCoppBufferPointer;
   uint32_t nPoppBufferLength;
   uint8_t *nPoppBufferPointer;
} AcdbAudProcVolTblCmdType;

#endif /* __ACDB_DEPRECATE_API_H__ */


