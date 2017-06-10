#ifndef __ACDB_API_H__
#define __ACDB_API_H__
/*===========================================================================
    @file   acdb.h

    This file contains the public interface to the Audio Calibration Database
    (ACDB) module.

    The public interface of the Audio Calibration Database (ACDB) module,
    providing calibration storage for the audio and the voice path. The ACDB
    module is coupled with Qualcomm Audio Calibration Tool (QACT) v2.x.x.
    Qualcomm recommends to always use the latest version of QACT for
    necessary fixes and compatibility.

                    Copyright (c) 2010-2012 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal2/common/8k_PL/acdb/rel/4.2/inc/acdb.h#1 $ */

/*===========================================================================

EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------

06/06/11   ernanl  1. Introduced first version of MSM8960/MDM9615 ACDB API.
                   2. introduce new 8960/MDM9615 device info structure
                   3. deprecate QACT apis
14/05/12   kpavan  1. New interface for APQ MDM device Mapping.
                   2. New interface to get TX RX Device pair for Recording

01/06/12   kpavan  1. New interface to get RX device pair for AUDIO EC TX device.           
04/06/12   kpavan  1. Include files marcro changes. 
30/07/12   aboppay 1. New interface to get ADIE Sidetone table data.
===========================================================================*/

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */


#ifdef ACDB_SIM_ENV
    #include "mmdefs.h"
#else
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <fcntl.h>
    #include <sys/ioctl.h>
    #include <sys/errno.h>
    #include <inttypes.h>
    #include <linux/msm_audio.h>
#endif
#include "acdb_deprecate.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

/** @enum AcdbReturnCodes 
 *  These are the return codes to expect from the ACDB commands. Each command
 *  will return a specific set of these return codes. Please refer to the
 *  documentation for each command for the return codes to be expected for
 *  each command.
 */
typedef enum AcdbReturnCodes {
   ACDB_SUCCESS = 0,                   /**< Defines general success */
   ACDB_ERROR = -1,                    /**< Defines general failure */
   ACDB_BADPARM = -2,                  /**< Describes a bad parameter was given */
   ACDB_BADSTATE = -3,                 /**< Describes ACDB is in a bad state */
   ACDB_DEVICENOTFOUND = -4,           /**< Describes given device id was not found in DB */
   ACDB_SAMPLERATENOTFOUND = -5,       /**< Describes given samplerate id was not found in DB */
   ACDB_NETWORKNOTFOUND = -6,          /**< Describes given network id was not found in DB */
   ACDB_MODULENOTFOUND = -7,           /**< Describes given module id was not found in DB */
   ACDB_PARMNOTFOUND = -8,             /**< Describes given parameter id was not found in DB */
   ACDB_APPLICATIONTYPENOTFOUND = -9,  /**< Describes given application type id was not found in DB */
   ACDB_CODECPATHNOTFOUND = -10,       /**< Describes given codec path id was not found in DB */
   ACDB_CODECSAMPLERATENOTFOUND = -11, /**< Describes given codec samplerate id was not found in DB */
   ACDB_INSUFFICIENTMEMORY = -12,      /**< Describes an error where insufficient memory was allocated for this operation */
   ACDB_NOTSUPPORTED = -13,            /**< Describes an error where a command is not supported */
   ACDB_NULLPOINTER = -14,             /**< Describes an error where the a null pointer is encountered*/
   ACDB_MISMATCH_TARGET_VERSION = -15, /**< Occurs only at init when Target version in AcdbData and Acdb are mismatched */
   ACDB_MISMATCH_DATA_SW_VERSION = -16,/**< Occurs only at init when AcdbData Data Structure version in AcdbData and Acdb are mismatched */
   ACDB_VOLUME_STEP_NOT_FOUND = -17,   /**< Occurs when volume step is not available */
}AcdbReturnCodesType;

/** @enum AcdbNetworkIds 
 *  These are the network ids expected to query the voice table.
 */
typedef enum AcdbNetworkIds {
   ACDB_NETWORK_ID_CDMA  = 0x00011134, /**< CDMA Network */
   ACDB_NETWORK_ID_GSM   = 0x00011135, /**< GSM Network */
   ACDB_NETWORK_ID_WCDMA = 0x00011136, /**< WCDMA Network */
   ACDB_NETWORK_ID_VOIP = 0x0001128F,  /**< VOIP Network */
   ACDB_NETWORK_ID_VOLTE = 0x0001132E, /**< LTE Network*/
}AcdbNetworkIdsType;

/** @enum AcdbVocProcSampleRateIds 
 *  These are the VocProc samplerate ids expected to query the voice table.
 */
typedef enum AcdbVocProcSampleRateIds {
   ACDB_VOCPROC_ID_NARROWBAND_WITHOUT_WIDEVOICE = 0x00011137,  /**< To be used if the vocproc is operating in a narrowband network */
   ACDB_VOCPROC_ID_NARROWBAND_WITH_WIDEVOICE    = 0x00011138,  /**< To be used if the vocproc is operating in a narrowband network */
   ACDB_VOCPROC_ID_WIDEBAND                     = 0x00011139,  /**< To be used if the vocproc is operating in a wideband network */
}AcdbVocProcSampleRateIdsType;

/** @enum AcdbApplicationTypeIds 
 *  These are the Application Type ids expected to query the audio table.
 */
typedef enum AcdbApplicationTypeIds {
   ACDB_APPTYPE_GENERAL_PLAYBACK  = 0x00011130,  /**< To query for general audio playback calibration for RX audio devices */
   ACDB_APPTYPE_SYSTEM_SOUNDS     = 0x00011131,  /**< To query for system sounds calibration for RX audio devices */
   ACDB_APPTYPE_GENERAL_RECORDING = 0x00011132,  /**< To query for general audio recording calibration for TX audio devices */
   ACDB_APPTYPE_VOICE_RECOGNITION = 0x00011133,  /**< To query for voice recognition calibration for TX audio devices */
}AcdbApplicationTypeIdsType;

/** @enum AcdbSampleRateMask 
 *  These are the possible sample rates for a specific device.
 */
typedef enum AcdbSampleRateMask {
   ACDB_SAMPLERATE_8000Hz  = 0x0001,       /**< Used if the AFE supports running at  8000 hz */
   ACDB_SAMPLERATE_11025Hz = 0x0002,       /**< Used if the AFE supports running at 11025 hz */
   ACDB_SAMPLERATE_12000Hz = 0x0004,       /**< Used if the AFE supports running at 12000 hz */
   ACDB_SAMPLERATE_16000Hz = 0x0008,       /**< Used if the AFE supports running at 16000 hz */
   ACDB_SAMPLERATE_22050Hz = 0x0010,       /**< Used if the AFE supports running at 22050 hz */
   ACDB_SAMPLERATE_24000Hz = 0x0020,       /**< Used if the AFE supports running at 24000 hz */
   ACDB_SAMPLERATE_32000Hz = 0x0040,       /**< Used if the AFE supports running at 32000 hz */
   ACDB_SAMPLERATE_44100Hz = 0x0080,       /**< Used if the AFE supports running at 44100 hz */
   ACDB_SAMPLERATE_48000Hz = 0x0100,       /**< Used if the AFE supports running at 48000 hz */
   ACDB_SAMPLERATE_96000Hz  = 0x0200,       /**< Used if the AFE supports running at 96000 hz */
   ACDB_SAMPLERATE_192000Hz = 0x0400,       /**< Used if the AFE supports running at 192000 hz */
}AcdbSampleRateMaskType;

/** @enum AcdbOverSampleRateIds 
 *  These are the possible over sample rates for a specific codec
 */
typedef enum AcdbOverSampleRateIds {
   ACDB_OVERSAMPLERATE_0    = 0x0000,        /**< Used if the codec supports running at 0 OSR */
   ACDB_OVERSAMPLERATE_64   = 0x0001,        /**< Used if the codec supports running at a  64 OSR */
   ACDB_OVERSAMPLERATE_128  = 0x0002,        /**< Used if the codec supports running at a 128 OSR */
   ACDB_OVERSAMPLERATE_256  = 0x0003,        /**< Used if the codec supports running at a 256 OSR */
}AcdbOverSampleRateIdsType;

/** @enum AcdbChannelConfig
 *  These are the possible channel configuration choices.
 */
typedef enum AcdbChannelConfig {
   ACDB_CHANNEL_NONE       = 0x00,        /**< Channel = 0 */
   ACDB_CHANNEL_MONO       = 0x01,        /**< MONO */
   ACDB_CHANNEL_STEREO     = 0x02,        /**< STEREO */
   ACDB_CHANNEL_3CHANNELS  = 0x03,        /**< 3-channels */
   ACDB_CHANNEL_4CHANNELS  = 0x04,        /**< 4-channels */
   ACDB_CHANNEL_6CHANNELS  = 0x06,        /**< 6-channels */
   ACDB_CHANNEL_8CHANNELS  = 0x08,        /**< 8-channels */
}AcdbChannelConfigType;

/** @enum AcdbDirectionMask 
 *  These are the possible directions to expect in the ACDB Device info's ulDirectionMask field.
 */
typedef enum AcdbDirectionMask {
   ACDB_DIRECTION_RX       = 0x01,           /**< Determines if a device is an RX device */
   ACDB_DIRECTION_TX       = 0x02,           /**< Determines if a device is an TX device */
   ACDB_DIRECTION_AUXPGA   = 0x04,           /**< Determines if a device is an AUXPGA device */
}AcdbDirectionMaskType;

/** @enum AcdbI2sMSMode
 *  These are the possible I2S clock configuration choices.
 */
typedef enum AcdbI2sMClkMode {
   ACDB_I2SMSMODE_MLCK_MSTR    = 0x0,        /**< MCLK Master */
   ACDB_I2SMSMODE_MLCK_SLAVE   = 0x01,       /**< MCLK Master */
}AcdbI2sMClkModeType;

/** @enum AcdbAfePortId 
 *  These are the AFE port ids that a device can be associated with.
 */
typedef enum AcdbAfePortId {
   ACDB_AFEPORT_PRIMARY_I2S_RX      = 0x00000000, /**< Primary I2S RX */
   ACDB_AFEPORT_PRIMARY_I2S_TX      = 0x00000001, /**< Primary I2S TX */
   ACDB_AFEPORT_PCM_RX              = 0x00000002, /**< PCM RX */
   ACDB_AFEPORT_PCM_TX              = 0x00000003, /**< PCM TX */
   ACDB_AFEPORT_SECONDARY_I2S_RX    = 0x00000004, /**< Secondary I2S RX */
   ACDB_AFEPORT_SECONDARY_I2S_TX    = 0x00000005, /**< Secondary I2S TX */
   ACDB_AFEPORT_MI2S_RX             = 0x00000006, /**< MI2S RX */
   ACDB_AFEPORT_MI2S_TX             = 0x00000007, /**< MI2S TX */
   ACDB_AFEPORT_HDMI_RX             = 0x00000008, /**< HDMI RX */
   ACDB_AFEPORT_DIGITAL_MIC_TX      = 0x0000000B, /**< Digital Quad Mic TX */
   ACDB_AFEPORT_PSEUDOPORT_01       = 0x00008001, /**< PSEUDOPORT_01 */
   ACDB_AFEPORT_PSEUDOPORT_02       = 0x00008002, /**< PSEUDOPORT_02 */
   ACDB_AFEPORT_PSEUDOPORT_03       = 0x00008003, /**< PSEUDOPORT_03 */
   ACDB_AFEPORT_VOICE_RECORD_TX     = 0x00008004, /**< VOICE_RECORD_TX */
   ACDB_AFEPORT_VOICE_PLAYBACK_TX   = 0x00008005, /**< VOICE_PLAYBACK_TX */
}AcdbAfePortIdType;

/** @enum AcdbAfeChannelModeI2S 
 *  These are the AFE channel modes available for Primary, Secondary and 
    Multi I2S ports.
 */
typedef enum AcdbAfeChannelModeI2S {
   ACDB_CHMODE_I2S_MODE_0      = 0x00000001, /**< sd0 */
   ACDB_CHMODE_I2S_MODE_1      = 0x00000002, /**< sd1 */
   ACDB_CHMODE_I2S_MODE_2      = 0x00000003, /**< sd2 */
   ACDB_CHMODE_I2S_MODE_3      = 0x00000004, /**< sd3 */
   ACDB_CHMODE_I2S_MODE_4      = 0x00000005, /**< sd0/sd1 */
   ACDB_CHMODE_I2S_MODE_5      = 0x00000006, /**< sd2/sd3 */
   ACDB_CHMODE_I2S_MODE_6      = 0x00000007, /**< 6 ch */
   ACDB_CHMODE_I2S_MODE_7      = 0x00000008, /**< 8 ch */
}AcdbAfeChannelModeI2SType;

/** @enum AcdbAfeChannelModePCM
 *  These are the AFE channel modes available for PCM ports.
 */
typedef enum AcdbAfeChannelModePCM {
   ACDB_CHMODE_PCM_ALAW_NO_PAD         = 0x00000000, /**< Alaw with no padding */
   ACDB_CHMODE_PCM_MULAW_NO_PAD        = 0x00000001, /**< mulaw with no padding */
   ACDB_CHMODE_PCM_LINEAR_NO_PAD       = 0x00000002, /**< linear with no padding */
   ACDB_CHMODE_PCM_ALAW_WITH_PAD       = 0x00000003, /**< Alaw with padding */
   ACDB_CHMODE_PCM_MULAW_WITH_PAD      = 0x00000004, /**< mulaw with padding */
   ACDB_CHMODE_PCM_LINEAR_WITH_PAD     = 0x00000005, /**< linear with padding */
}AcdbAfeChannelModePCMType;

/** @enum AcdbAfeChannelModeDigMic
 *  These are the AFE channel modes available for Digital Mic ports.
 */
typedef enum AcdbAfeChannelModeDigMic {
   ACDB_CHMODE_DIGMIC_MODE_RESERVED       = 0x00000000, /**< Reserved */
   ACDB_CHMODE_DIGMIC_MODE_LEFT_0         = 0x00000001, /**< Mode Left 0 */
   ACDB_CHMODE_DIGMIC_MODE_RIGHT_0        = 0x00000002, /**< Mode Right 0 */
   ACDB_CHMODE_DIGMIC_MODE_LEFT_1         = 0x00000003, /**< Mode Left 1 */
   ACDB_CHMODE_DIGMIC_MODE_RIGHT_1        = 0x00000004, /**< Mode Right 1 */
   ACDB_CHMODE_DIGMIC_MODE_STEREO_0       = 0x00000005, /**< Mode Stereo 0 */
   ACDB_CHMODE_DIGMIC_MODE_STEREO_2       = 0x00000006, /**< Mode Stereo 2 */
   ACDB_CHMODE_DIGMIC_MODE_QUAD           = 0x00000007  /**< Mode Quad */
}AcdbAfeChannelModeDigMicType;

/** @enum AcdbAfeChannelModeHDMI
 *  These are the AFE channel modes available for HDMI ports.
 */
typedef enum AcdbAfeChannelModeHDMI {
   ACDB_CHMODE_HDMI_STEREO    = 0x00000000, /**< Stereo */
   ACDB_CHMODE_HDMI_3_1       = 0x00000001, /**< 3.1 speakers */
   ACDB_CHMODE_HDMI_5_1       = 0x00000002, /**< 5.1 speakers */
   ACDB_CHMODE_HDMI_7_1       = 0x00000003, /**< 7.1 speakers */
}AcdbAfeChannelModeHDMIType;

/** @enum AcdbAfeBytesPerSample
 *  These are choices for the AFE bytes per sample.
 */
typedef enum AcdbAfeBytesPerSample {
   ACDB_BPS_16BIT_PCM   = 0x00000001, /**< 16 bit PCM */
   ACDB_BPS_24BIT_PCM   = 0x00000002, /**< 24 bit PCM */
   ACDB_BPS_32BIT_PCM   = 0x00000004, /**< 32 bit PCM */
}AcdbAfeBytesPerSampleType;

/** @enum AcdbAfePortConfigI2S
 *  These are the AFE port config choices for I2S ports.
 */
typedef enum AcdbAfePortConfigI2S {
   ACDB_PORTCFG_I2S_MONO_BOTH_CHANNELS          = 0x00000000, /**< Mono on both channels */
   ACDB_PORTCFG_I2S_MONO_RIGHT_CHANNEL_ONLY     = 0x00000001, /**< Mono right channel only */
   ACDB_PORTCFG_I2S_MONO_LEFT_CHANNEL_ONLY      = 0x00000002, /**< Mono left channel only */
   ACDB_PORTCFG_I2S_STEREO                      = 0x00000010, /**< Stereo */
}AcdbAfePortConfigI2SType;

/** @enum AcdbAfePortConfigHDMI
 *  These are the AFE port config choices for HDMI ports.
 */
typedef enum AcdbAfePortConfigHDMI {
   ACDB_PORTCFG_HDMI_LINEAR      = 0x00000000, /**< linear */
   ACDB_PORTCFG_HDMI_NONLINEAR   = 0x00000001, /**< non-linear */
}AcdbAfePortConfigHDMIType;

/** @enum AcdbAfePortConfigAUXPCM
 *  These are the AFE port config choices for AUXPCM ports.
 */
typedef enum AcdbAfePortConfigAUXPCM {
   ACDB_PORTCFG_AUXPCM_8_BITS_PER_FRAME             = 0x00000000, /**< 8  Bits per frame */
   ACDB_PORTCFG_AUXPCM_16_BITS_PER_FRAME            = 0x00000001, /**< 16  Bits per frame */
   ACDB_PORTCFG_AUXPCM_32_BITS_PER_FRAME            = 0x00000002, /**< 32  Bits per frame */
   ACDB_PORTCFG_AUXPCM_64_BITS_PER_FRAME            = 0x00000003, /**< 64  Bits per frame */
   ACDB_PORTCFG_AUXPCM_128_BITS_PER_FRAME           = 0x00000004, /**< 128  Bits per frame */
   ACDB_PORTCFG_AUXPCM_256_BITS_PER_FRAME           = 0x00000005, /**< 256  Bits per frame */
   ACDB_PORTCFG_AUXPCM_PCM_MODE_BITMASK             = 0x00000010, /**< PCM mode bitmask(0 - PCM Mode, 1 - AUX mode */
   ACDB_PORTCFG_AUXPCM_PCM_CTRL_DATA_OE_BITMASK     = 0x00000100, /**< PCM Ctrl data OE bitmask ( 0 - disable, 1 - enable) */
}AcdbAfePortConfigAUXPCMType;

/** @enum AcdbAfeSyncSrcI2S
 *  These are the choices for I2S ports
  */
typedef enum AcdbAfeSyncSrcI2S{
   ACDB_AFE_SYNC_SRC_I2S_EXTERNAL = 0x00000000, /**< Word Select Source from external */
   ACDB_AFE_SYNC_SRC_I2S_INTERNAL = 0x00000001, /**< Word Select Source from internal */
}AcdbAfeSyncSrcI2SType;

/** @enum AcdbAfeSyncSrcPCM
 *  These are the choices for PCM ports
  */
typedef enum AcdbAfeSyncSrcPCM{
   ACDB_AFE_SYNC_SRC_PCM_EXTERNAL = 0x00000000, /**< PCM Sync source from external */
   ACDB_AFE_SYNC_SRC_PCM_INTERNAL = 0x00000001, /**< PCM Sync source from internal */
}AcdbAfeSyncSrcPCMType;

/** @enum AcdbVoiceTxCOPPTopology
 *  These are the choices for TX voice COPP topologies.
 */
typedef enum AcdbVoiceTxCOPPTopology {
   ACDB_TX_VOICE_COPP_TOPOLOGY_NULL                 = 0x00010F70, /**< NULL */
   ACDB_TX_VOICE_COPP_TOPOLOGY_SINGLE_MIC_ECNS      = 0x00010F71, /**< SingleMicEcns */
   ACDB_TX_VOICE_COPP_TOPOLOGY_DUAL_MIC_FLUENCE     = 0x00010F72, /**< DualMicFluence */
   ACDB_TX_VOICE_COPP_TOPOLOGY_SINGLE_MIC_QUARTET   = 0x00010F73, /**< SingleMicQuartet */
   ACDB_TX_VOICE_COPP_TOPOLOGY_DUAL_MIC_QUARTET     = 0x00010F74, /**< DualMicQuartet */
   ACDB_TX_VOICE_COPP_TOPOLOGY_QUAD_MIC_QUARTET     = 0x00010F75, /**< QuadMicQuartet */
}AcdbVoiceTxCOPPTopologyType;

/** @enum AcdbVoiceRxCOPPTopology
 *  These are the choices for RX voice COPP topologies.
 */
typedef enum AcdbVoiceRxCOPPTopology {
   ACDB_RX_VOICE_COPP_TOPOLOGY_NULL                 = 0x00010F76, /**< NULL */
   ACDB_RX_VOICE_COPP_TOPOLOGY_DEFAULT_VOICE        = 0x00010F77, /**< Default Voice */
}AcdbVoiceRxCOPPTopologyType;

/** @enum AcdbAudioTxCOPPTopology
 *  These are the choices for TX audio COPP topologies.
 */
typedef enum AcdbAudioTxCOPPTopology {
   ACDB_TX_AUDIO_COPP_TOPOLOGY_DEFAULT_AUDIO        = 0x00011164, /**< Default Audio */
}AcdbAudioTxCOPPTopologyType;

/** @enum AcdbAudioRxCOPPTopology
 *  These are the choices for RX audio COPP topologies.
 */
typedef enum AcdbAudioRxCOPPTopology {
   ACDB_RX_AUDIO_COPP_TOPOLOGY_DEFAULT_AUDIO        = 0x00011165, /**< Default Audio */
}AcdbAudioRxCOPPTopologyType;

/** @enum AcdbTimingMode
 * Timing mode is to identify how the device clock operate at.
 */
typedef enum AcdbTimingMode {
   ACDB_TIMINGMODE_FTRT = 0x00000000,              /** timing operate faster than real time */
   ACDB_TIMINGMODE_SYSTEM_TIMER = 0x00000001,      /** timing operate at system timer rate */
} AcdbTimingModeType;

/** @enum AcdbIsVirtualDevice
 * These are the choices to identify the associated device is a virtual device or not.
 */
typedef enum AcdbIsVirtualDevice {
   ACDB_IS_NOT_VIRTUAL_DEVICE = 0x00000000,        /** This device is not a virtual device */
   ACDB_IS_VIRTUAL_DEVICE =0x00000001,             /** This device is a virtual device */
   ACDB_IS_USB_DEVICE = 0x00000002,                /** This device is a USB device */
} AcdbIsVirtualDeviceType;

/**
   @struct   AcdbQueryResponseType
   @brief This is a generic query command response structure.

   @param nBytesUsedInBuffer: Describes bytes that were used in the output buffer
                              initially provided in the command structure.

   This is a generic query command response structure.
*/
typedef struct _AcdbQueryResponseType {
   uint32_t nBytesUsedInBuffer;
} AcdbQueryResponseType;

/**
   @struct   AcdbAudProcGainDepVolTblRspType
   @brief This is the response structure for the gain-dependent audio volume table query.

   @param nBytesUsedPoppBuffer: The size of the gain-dependent audproc POPP volume 
                                calibration table stored in buffer provided in the input 
                                structure.
   @param nBytesUsedCoppBuffer: The size of the gain-dependent audproc COPP volume 
                                calibration table stored in buffer provided in the input 
                                structure.
   
   This is the response structure for the gain-dependent audio volume table query.
*/
typedef struct _AcdbAudProcGainDepVolTblRspType {
   uint32_t nBytesUsedPoppBuffer;
   uint32_t nBytesUsedCoppBuffer;
} AcdbAudProcGainDepVolTblRspType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_INITIALIZE Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_INITIALIZE, ...)
   @brief API to initialize the ACDB SW.

   This command will initialize the ACDB SW. By default, ACDB contains a set of
   statically linked calibration data contained in acdb_default_data.c. During
   initialize ACDB will check if a data override file exists on the file system.
   If the data override file exists on the file system, ACDB will read in the
   entire contents of the override data file and store the difference in the
   heap. Please note that if the data override file is substantially different than
   the default file, ACDB may consume quite a bit of RAM.

   @param[in] nCommandId:
         The command id is ACDB_CMD_INITIALIZE.
   @param[in] pCommandStruct:
         There is no input structure so this should be NULL.
   @param[in] nCommandStructLength:
         There is no input structure so this should be 0.
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_FAILURE: If any error is encountered during course of
                           executing the reset.
         ACDB_MISMATCH_TARGET_VERSION:
            This occurs if the target version in the Acdb SW is different than
            the target version in the AcdbData SW. This is a safety error and
            will be raised if there is a mismatch to alert the client that the
            AcdbData file is from a different build and may need to be upgraded
            or have it's data merged into the AcdbData expected by this build.
         ACDB_MISMATCH_DATA_SW_VERSION:
            This occurs if the AcdbData data structure version in the AcdbData SW
            is different than what is expected in the ACDB SW. This is a safety
            error and will be raised if there is a mismatch to protect the client
            from an impending runtime error that is about to happen. This will
            be a rare problem.
*/
#define ACDB_CMD_INITIALIZE               0x0001112F

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_ACDB_VERSION Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_ACDB_VERSION, ...)
   @brief API to query for the ACDB SW version.

   This command will obtain the ACDB SW version. Clients are to use this version
   to determine how best to interact with the ACDB SW module.

   Rules governing the versioning of the API interface:
      Major: Incremented whenever there is a major adjustment to the ACDB that
             will affect the ACDB clients. This includes but not limited to
             backward incompatible API changes or drastic changes in memory
             usage.
      Minor: Incremented whenever there is a minor adjustment to the ACDB that
             will not affect existing ACDB clients. This incldues but not
             limited to backward compatible API changes or new capabilties or
             new calibration structures.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_ACDB_VERSION.
   @param[in] pCommandStruct:
         There is no input structure so this should be NULL.
   @param[in] nCommandStructLength:
         There is no input structure so this should be 0.
   @param[out] pResponseStruct:
         This should be a pointer to AcdbModuleVersionType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbModuleVersionType).

   @see  acdb_ioctl
   @see  AcdbModuleVersionType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_MISMATCH_TARGET_VERSION:
                       If command and queried data target version mismatch,
                       return ACDB_MISMATCH_TARGET_VERSION and executing reset.
         ACDB_MISMATCH_DATA_SW_VERSION:
                       If command and queried data SW version mismatch,
                       return ACDB_MISMATCH_DATA_SW_VERSION and executing reset.
*/
#define ACDB_CMD_GET_ACDB_VERSION            0x0001110F

/**
   @struct   AcdbModuleVersionType
   @brief This is a response structure for the get ACDB module version command.
   
   @param major: The major version describing the capabilities of the ACDB API.
   @param minor: The minor version describing the capabilities of the ACDB API.

   This is the response structure for the ACDB module version command
   enabling the client to understand the ACDB APIs and capabilities.
*/
typedef struct _AcdbModuleVersionType {
   uint16_t major;
   uint16_t minor;
} AcdbModuleVersionType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_TARGET_VERSION Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_TARGET_VERSION, ...)
   @brief API to query for the ACDB Target Version.

   This command will obtain the ACDB Target version. The target version is a
   unique 32-bit GUID intended to identify which chipset ACDB is currently
   representing.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_TARGET_VERSION.
   @param[in] pCommandStruct:
         There is no input structure so this should be NULL.
   @param[in] nCommandStructLength:
         There is no input structure so this should be 0.
   @param[out] pResponseStruct:
         This should be a pointer to AcdbTargetVersionType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbTargetVersionType).

   @see  acdb_ioctl
   @see  AcdbTargetVersionType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_TARGET_VERSION            0x00011110

/**
   @struct   AcdbTargetVersionType
   @brief This is a response structure for the get ACDB module version command.
   
   @param targetversion: The target version

   This is the response structure for the ACDB module version command
   enabling the client to understand the ACDB APIs and capabilities.
*/
typedef struct _AcdbTargetVersionType {
   uint32_t targetversion;
} AcdbTargetVersionType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_RESET Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_RESET, ...)
   @brief API to reset the ACDB SW back to the default state.

   This command will release all heap memory used by the data override data
   structure. This operation will effectively put ACDB back into its default
   state only containing the data that was compiled in.

   @param[in] nCommandId:
         The command id is ACDB_CMD_RESET.
   @param[in] pCommandStruct:
         There is no input structure so this should be NULL.
   @param[in] nCommandStructLength:
         There is no input structure so this should be 0.
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
*/
#define ACDB_CMD_RESET                       0x00011111

/* ---------------------------------------------------------------------------
 * ACDB_CMD_ESTIMATE_MEMORY_USE Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_ESTIMATE_MEMORY_USE, ...)
   @brief Calculates the memory usage used by the ACDB data structure.

   This command will iterate over the ACDB database and sum up the memory
   used by the data structures. There are two classificition of data
   structures. Organizational refers to the memory required to organize
   the calibration data into a queryable constructs. Data refers to the
   memory required to store the calibration data.

   @param[in] nCommandId:
         The command id is ACDB_CMD_ESTIMATE_MEMORY_USE.
   @param[in] pCommandStruct:
         There is no input structure so this should be NULL.
   @param[in] nCommandStructLength:
         There is no input structure so this should be 0.
   @param[out] pResponseStruct:
         This should be a pointer to AcdbMemoryUsageType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbMemoryUsageType).

   @see  acdb_ioctl
   @see  AcdbMemoryUsageType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_ESTIMATE_MEMORY_USE         0x00011112

/**
   @struct   AcdbMemoryUsageType
   @brief This is a response structure for the estimate memory usage command.
   
   @param org_ROM: The total bytes used by the organizational data structure
                   in ROM or the DEFAULT ACDB file.
   @param data_ROM: The total bytes used by the calibration parameter data
                   structure in ROM or the DEFAULT ACDB file.
   @param org_RAM: The total bytes used by the organizational data structure
                   in RAM.
   @param data_RAM: The total bytes used by the calibration parameter data
                   structure in RAM.

   This is the response structure for the estimate memory usage command
   enabling the client to query for the current memory usage of the ACDB
   module.
*/
typedef struct _AcdbMemoryUsageType {
   uint32_t org_ROM;
   uint32_t data_ROM;
   uint32_t org_RAM;
   uint32_t data_RAM;
} AcdbMemoryUsageType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_DEVICE_CAPABILITIES Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_DEVICE_CAPABILITIES, ...)
   @brief API to query for the device capabilities of all devices.

   This command will contain a list of every device id stored in ACDB and
   their capabilities.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_DEVICE_CAPABILITIES.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDeviceCapabilitiesCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDeviceCapabilitiesCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbDeviceCapabilitiesCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_DEVICE_CAPABILITIES     0x00011114

/**
   @struct   AcdbDeviceCapabilitiesCmdType
   @brief This is a query command structure for the device capabilities.

   This is a query command structure for the device capabilities. The format
   of the buffer returned will look like this:


   @param nParamId: The parameter id identifying the payload format
                    contained in the buffer.
   @param nBufferLength: The length of the buffer. This should be
                    large enough to hold the entire device info structure
                    identified by nParamId.
   @param nBufferPointer: A virtual memory pointer pointing to the memory
                    region ACDB will copy the device info into.

   @see AcdbDeviceCapabilityType

*/
typedef struct _AcdbDeviceCapabilitiesCmdType {
   uint32_t nParamId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbDeviceCapabilitiesCmdType;

/**
   @def ACDB_DEVICE_CAPABILITY_PARAM
   @brief This is the Parameter ID identifying the format of the device
          capabilities structure.

   The format of the device capabilities parameter is a packed structure:
      struct AcdbDeviceCapabilityType {
         uint32_t nDeviceId;
         uint32_t ulSampleRateMask;
         uint32_t ulBytesPerSampleMask;
         };
      struct DeviceCapabilities {
         uint32_t nDeviceCount;
         AcdbDeviceCapabilityType entries [nDeviceCount];
      };
   This structure will require parsing as it's packed and there is no
   padding bytes contained in the structure. C compilers will compile
   the above structure in an padded fashion.

   @see AcdbDeviceCapabilityType
*/
#define ACDB_DEVICE_CAPABILITY_PARAM                     0x0001117E

/**
   @struct   AcdbDeviceCapabilityType
   @brief This is a response structure for the device capabilities.

   @param nDeviceId:
      The device ID the two next capability information is associated with.
   @param ulSampleRateMask:
      The sample rate mask (please see device info structure for bit meanings).
   @param ulBytesPerSampleMask:
      The bytes per samples mask (please see device info structure for bit meanings).
*/
typedef struct AcdbDeviceCapabilityType {
   uint32_t nDeviceId;
   uint32_t ulSampleRateMask;
   uint32_t ulBytesPerSampleMask;
}AcdbDeviceCapabilityType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_IS_DEVICE_PAIRED Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_IS_DEVICE_PAIRED, ...)
   @brief API to query to determine whether two device ids are paired.

   This command will determine whether the given device ids are paired.

   @param[in] nCommandId:
         The command id is ACDB_CMD_IS_DEVICE_PAIRED.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDevicePairType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDevicePairType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDevicePairingResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDevicePairingResponseType).

   @see  acdb_ioctl
   @see  AcdbDevicePairType
   @see  AcdbDevicePairingResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_IS_DEVICE_PAIRED            0x00011115

/**
   @struct   AcdbDevicePairType
   @brief This is a query command structure for determing valid device pairs.

   @param nTxDeviceId: The TX Device ID
   @param nRxDeviceId: The RX Device ID

   This is a query command structure for determing valid device pairs. This
   information is helpful to query prior to attempting to obtain calibration
   for voice calls. This command will let those interested in understanding
   what paired devices are acceptable for use in a voice call.
*/
typedef struct _AcdbDevicePairType {
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
} AcdbDevicePairType;

/**
   @struct   AcdbDevicePairingResponseType
   @brief Identifies whether the queried device pairing is valid.

   @param nIsDevicePairValid:
         This identifies whether the queried device pair is valid. Expected
         values are:
            (0x00000000) Pair is valid.
            (0x00000001) Pair is not valid.
*/
typedef struct _AcdbDevicePairingResponseType {
   uint32_t ulIsDevicePairValid;
} AcdbDevicePairingResponseType;

/**
   @def ACDB_DEVICE_PAIR_PARAM
*/
#define ACDB_DEVICE_PAIR_PARAM                           0x00011184

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_VOCPROC_COMMON_DATA and ACDB_CMD_SET_VOCPROC_COMMON_DATA
 *    Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_VOCPROC_COMMON_DATA, ...)
   @brief API to query for individual calibration data.

   This command will obtain the specific calibration data queried for VocProc 
   common.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_VOCPROC_COMMON_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbVocProcCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbVocProcCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbVocProcCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_VOCPROC_COMMON_DATA     0x00011116

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_VOCPROC_COMMON_DATA, ...)
   @brief API to change individual calibration data in database.

   This command will allow the overriding of VocProc common calibration data 
   stored in the database.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_VOCPROC_COMMON_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbVocProcCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbVocProcCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbVocProcCmdType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_VOCPROC_COMMON_DATA     0x00011117

/**
   @struct   AcdbVocProcCmdType
   @brief This is a query command structure allowing for individual get/set
          for calibration data in the vocproc data tables.

   @param nTxDeviceId: The TX Device ID
   @param nRxDeviceId: The RX Device ID
   @param nTxDeviceSampleRateId: Operating sample rate for the TX device
   @param nRxDeviceSampleRateId: Operating sample rate for the RX device
   @param nNetworkId: The Network ID
   @param nVocProcSampleRateId: The VocProc Stream Sample Rate ID
   @param nModuleId: The Module ID
   @param nParamId: The Parameter ID of the Module to query for.
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the parameters identified by the
                    nParamId identifier.
   @param nBufferPointer: A virtual memory pointer pointing to a memory region
                          containing the payload (or receiving the payload) 
                          identified by nParamId.

   This is a query command structure allowing for individual get/set for
   calibration data in the vocproc data tables.
*/
typedef struct _AcdbVocProcCmdType {
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
   uint32_t nTxDeviceSampleRateId;
   uint32_t nRxDeviceSampleRateId;
   uint32_t nNetworkId;
   uint32_t nVocProcSampleRateId;
   uint32_t nModuleId;
   uint32_t nParamId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbVocProcCmdType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_VOCPROC_COMMON_TABLE and ACDB_CMD_SET_VOCPROC_COMMON_TABLE
 *    Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_VOCPROC_COMMON_TABLE, ...)
   @brief API to query for an entire calibration table in the QDSP6 table format.

   This command will obtain the specific calibration data queried for VocProc 
   Common Table

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_VOCPROC_COMMON_TABLE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbVocProcTableCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbVocProcTableCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbVocProcTableCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_VOCPROC_COMMON_TABLE    0x00011118

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_VOCPROC_COMMON_TABLE, ...)
   @brief API to change a set of calibration data in database.

   This command will allow the overriding of VocProc common calibration data stored 
   in the database.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_VOCPROC_COMMON_TABLE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbVocProcTableCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbVocProcTableCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbVocProcTableCmdType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_VOCPROC_COMMON_TABLE    0x00011119

/**
   @struct   AcdbVocProcTableCmdType
   @brief This is a query command structure allowing for multiple get/set
          of calibration data in the vocproc data tables.

   @param nTxDeviceId: The TX Device ID
   @param nRxDeviceId: The RX Device ID
   @param nTxDeviceSampleRateId: Operating sample rate for the TX device
   @param nRxDeviceSampleRateId: Operating sample rate for the RX device
   @param nNetworkId: The Network ID
   @param nVocProcSampleRateId: The VocProc Stream Sample Rate ID
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the entire table.
   @param nBufferPointer: A virtual memory pointer pointing to the memory
                    region ACDB will copy the table into.
   
   This is a query command structure allowing for multiple get/set of
   calibration data in the vocproc data tables. The format of the table will
   match the expected table format in the QDSP6 API. In this case, the
   table format will be the repetition of this structure:
      struct VocProcTableEntry {
         uint32_t nModuleId;
         uint32_t nParamId;
         uint16_t nParamSize; //multiple of 4
         uint16_t nReserved; // Must be 0
         uint8 aParamData [multiple of 4)];
     }
*/
typedef struct _AcdbVocProcTableCmdType {
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
   uint32_t nTxDeviceSampleRateId;
   uint32_t nRxDeviceSampleRateId;
   uint32_t nNetworkId;
   uint32_t nVocProcSampleRateId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbVocProcTableCmdType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_VOCPROC_STREAM_DATA and ACDB_CMD_SET_VOCPROC_STREAM_DATA
 *    Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_VOCPROC_STREAM_DATA, ...)
   @brief API to query for individual calibration data.

   This command will obtain the specific VocProc Stream calibration data 
   queried for.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_VOCPROC_COMMON_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbVocStrmCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbVocStrmCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbVocStrmCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_VOCPROC_STREAM_DATA     0x0001111A

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_VOCPROC_STREAM_DATA, ...)
   @brief API to change individual calibration data in database.

   This command will allow the overriding of VocProc Stream calibration 
   data stored in the database.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_VOCPROC_STREAM_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbVocStrmCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbVocStrmCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbVocStrmCmdType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_VOCPROC_STREAM_DATA     0x0001111B

/**
   @struct   AcdbVocStrmCmdType
   @brief This is a query command structure allowing for individual get/set
          for calibration data in the vocstrm data tables.

   @param nNetworkId: The Network ID
   @param nVocProcSampleRateId: The VocProc sample rate ID
   @param nModuleId: The Module ID
   @param nParamId: The Parameter ID of the Module to query for.
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the parameters identified by the
                    nParamId identifier.
   @param nBufferPointer: A virtual memory pointer pointing to a memory region
                          containing the payload (or receiving the payload) 
                          identified by nParamId.

   This is a query command structure allowing for individual get/set for
   calibration data in the vocstrm data tables.
*/
typedef struct _AcdbVocStrmCmdType {
   uint32_t nNetworkId;
   uint32_t nVocProcSampleRateId;
   uint32_t nModuleId;
   uint32_t nParamId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbVocStrmCmdType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_VOCPROC_STREAM_TABLE and ACDB_CMD_SET_VOCPROC_STREAM_TABLE
 *    Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_VOCPROC_STREAM_TABLE, ...)
   @brief API to query for an entire calibration table in the QDSP6 table format.

   This command will obtain the specific calibration data queried for VocProc 
   Stream Table

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_VOCPROC_COMMON_TABLE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbVocStrmTableCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbVocStrmTableCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbVocStrmTableCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_VOCPROC_STREAM_TABLE    0x0001111C

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_VOCPROC_STREAM_TABLE, ...)
   @brief API to change a set of calibration data in database.

   This command will allow the overriding of VocProc Stream Table calibration 
   data stored in the database.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_VOCPROC_STREAM_TABLE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbVocStrmTableCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbVocStrmTableCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbVocStrmTableCmdType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_VOCPROC_STREAM_TABLE    0x0001111D

/**
   @struct   AcdbVocStrmTableCmdType
   @brief This is a query command structure allowing for multiple get/set
          of calibration data in the vocstrm data tables.

   @param nNetworkId: The Network ID
   @param nVocProcSampleRateId: The VocProc sample rate ID
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the entire table.
   @param nBufferPointer: A virtual memory pointer pointing to the memory
                    region ACDB will copy the table into.

   This is a query command structure allowing for multiple get/set of
   calibration data in the vocstrm data tables. The format of the table will
   match the expected table format in the QDSP6 API. In this case, the
   table format will be the repetition of this structure:
      struct VocStrmTableEntry {
         uint32_t nModuleId;
         uint32_t nParamId;
         uint16_t nParamSize; //multiple of 4
         uint16_t nReserved; // Must be 0
         uint8 aParamData [multiple of 4)];
     }
*/
typedef struct _AcdbVocStrmTableCmdType {
   uint32_t nNetworkId;
   uint32_t nVocProcSampleRateId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbVocStrmTableCmdType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_AUDPROC_COMMON_DATA and ACDB_CMD_SET_AUDPROC_COMMON_DATA
 *    Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_AUDPROC_COMMON_DATA, ...)
   @brief API to query for individual calibration data.

   This command will obtain the specific calibration data queried for AudProc 
   Common data.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AUDPROC_COMMON_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAudProcCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAudProcCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbAudProcCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_AUDPROC_COMMON_DATA     0x0001111E

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_AUDPROC_COMMON_DATA, ...)
   @brief API to change individual calibration data in database.

   This command will allow the overriding of AudProc comon calibration 
   data stored in the database.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_AUDPROC_COMMON_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAudProcCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAudProcCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbAudProcCmdType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_AUDPROC_COMMON_DATA     0x0001111F

/**
   @struct   AcdbAudProcCmdType
   @brief This is a query command structure for the audproc data tables.

   @param nDeviceId: The Device ID
   @param nDeviceSampleRateId: The Device actual Sample Rate operate at
   @param nAppType: The Application Type ID
   @param nModuleId: The Module ID   
   @param nParamId: The Parameter ID of the Module to query for.
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the parameters identified by the
                    nParamId identifier.
   @param nBufferPointer: A virtual memory pointer pointing to a memory region
                          containing the payload (or receiving the payload) 
                          identified by nParamId.

   This is a query command structure for the audproc data tables.
*/
typedef struct _AcdbAudProcCmdType {
   uint32_t nDeviceId;
   uint32_t nDeviceSampleRateId;
   uint32_t nApplicationType;
   uint32_t nModuleId;
   uint32_t nParamId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbAudProcCmdType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_AUDPROC_COMMON_TABLE and ACDB_CMD_SET_AUDPROC_COMMON_TABLE
 *    Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_AUDPROC_COMMON_TABLE, ...)
   @brief API to query for an entire calibration table in the QDSP6 table format.

   This command will obtain the specific calibration data queried for AudProc 
   common Table

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AUDPROC_COMMON_TABLE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAudProcTableCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAudProcTableCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbAudProcTableCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_AUDPROC_COMMON_TABLE    0x00011120

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_AUDPROC_COMMON_TABLE, ...)
   @brief API to change a set of calibration data in database.

   This command will allow the overriding of AudProc common table calibration 
   data stored in the database

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_AUDPROC_COMMON_TABLE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAudProcTableCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAudProcTableCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbAudProcTableCmdType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_AUDPROC_COMMON_TABLE    0x00011121

/**
   @struct   AcdbAudProcTableCmdType
   @brief This is a query command structure allowing for multiple get/set
          of calibration data in the audproc data tables.

   @param nDeviceId: The Device ID
   @param nDeviceSampleRateId: The Device actual Sample Rate operate at
   @param nAppType: The Application Type ID
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the entire table.
   @param nBufferPointer: A virtual memory pointer pointing to the memory
                    region ACDB will copy the table into.

   This is a query command structure allowing for multiple get/set of
   calibration data in the audproc data tables. The format of the table will
   match the expected table format in the QDSP6 API. 
   In this case, the table format will be the repetition of this structure:
      struct AudProcTableEntry {
         uint32_t nModuleId;
         uint32_t nParamId;
         uint16_t nParamSize; //multiple of 4
         uint16_t nReserved; // Must be 0
         uint8 aParamData [multiple of 4];
     }
*/
typedef struct _AcdbAudProcTableCmdType {
   uint32_t nDeviceId;
   uint32_t nDeviceSampleRateId;
   uint32_t nApplicationType;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbAudProcTableCmdType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_AUDPROC_STREAM_DATA and ACDB_CMD_SET_AUDPROC_STREAM_DATA
 *    Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_AUDPROC_STREAM_DATA, ...)
   @brief API to query for individual calibration data.

   This command will obtain the specific calibration data queried for AudProc
   Stream data.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AUDPROC_STREAM_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAudStrmCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAudStrmCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbAudStrmCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_AUDPROC_STREAM_DATA     0x00011122

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_AUDPROC_STREAM_DATA, ...)
   @brief API to change individual calibration data in database.

   This command will allow the overriding of AudProc Stream calibration data
   stored in the database.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_AUDPROC_STREAM_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAudStrmCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAudStrmCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbAudStrmCmdType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_AUDPROC_STREAM_DATA      0x00011123

/**
   @struct   AcdbAudStrmCmdType
   @brief This is a query command structure for the audstrm data tables.
   
   @param nDeviceId: The Device ID
   @param nApplicationTypeId: The Application Type ID
   @param nModuleId: The Module ID
   @param nParamId: The Parameter ID of the Module to query for.
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the parameters identified by the
                    nParamId identifier.
   @param nBufferPointer: A virtual memory pointer pointing to a memory region
                          containing the payload (or receiving the payload) 
                          identified by nParamId.

   This is a query command structure for the audstrm data tables.
*/
typedef struct _AcdbAudStrmCmdType {
   uint32_t nDeviceId;
   uint32_t nApplicationTypeId;
   uint32_t nModuleId;
   uint32_t nParamId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbAudStrmCmdType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_AUDPROC_STREAM_TABLE and ACDB_CMD_SET_AUDPROC_STREAM_TABLE
 *    Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_AUDPROC_STREAM_TABLE, ...)
   @brief API to query for an entire calibration table in the QDSP6 table format.

   This command will obtain the specific calibration data queried for AudProc 
   Stream Table

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AUDPROC_STREAM_TABLE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAudStrmTableCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAudStrmTableCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbAudStrmTableCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_AUDPROC_STREAM_TABLE    0x00011124

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_AUDPROC_STREAM_TABLE, ...)
   @brief API to change a set of calibration data in database.

   This command will allow the overriding of AudProc Stream Table calibration 
   data stored in the database.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_AUDPROC_STREAM_TABLE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAudStrmTableCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAudStrmTableCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbAudStrmTableCmdType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_AUDPROC_STREAM_TABLE    0x00011125

/**
   @struct   AcdbAudStrmTableCmdType
   @brief This is a query command structure allowing for multiple get/set
          of calibration data in the audstrm data tables.

   @param nDeviceId: The Device ID
   @param nApplicationTypeId: The Application Type ID
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the entire table.
   @param nBufferPointer: A virtual memory pointer pointing to the memory
                    region ACDB will copy the table into.

   This is a query command structure allowing for multiple get/set of
   calibration data in the audstrm data tables. The format of the table will
   match the expected table format in the QDSP6 API. In this case, the
   table format will be the repetition of this structure:
      struct AudStrmTableEntry {
         uint32_t nModuleId;
         uint32_t nParamId;
         uint16_t nParamSize; //multiple of 4
         uint16_t nReserved; // Must be 0
         uint8 aParamData [multiple of 4];
     }
*/
typedef struct _AcdbAudStrmTableCmdType {
   uint32_t nDeviceId;
   uint32_t nApplicationTypeId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbAudStrmTableCmdType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_VOL_TABLE_STEP_SIZE
 *    Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_VOL_TABLE_STEP_SIZE, ...)
   @brief API to query for a audio volume table step size.

   This command will obtain the generic AudProc and VocProc volume table step size 
   queried for.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_VOL_TABLE_STEP_SIZE.
   @param[in] pCommandStruct:
         There is no iutput command structure so this should be NULL.
   @param[in] nCommandStructLength:
         There is no iutput structure so this should be 0.
   @param[out] pResponseStruct: 
         Pointer to the struct AcdbVolTblStepSizeRspType
   @param[out] nResponseStructLength: 
         Length of the AcdbVolTblStepSizeRspType

   @see  acdb_ioctl
   @see  AcdbVolTblStepSizeRspType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_VOL_TABLE_STEP_SIZE                        0x0001112C
 
/**
   @struct   AcdbVolTblStepSizeRspType
   @brief This is a query response structure for the RX volume table step size.

   @param[out] VocProcVolTblStepSize: 
         Number of step in VocProc volume table
   @param[out] AudProcVolTblStepSize: 
         Number of step in AudProc volume table.
   
   This is a response structure for the RX volume table step size. Number
   of volume step size represent the calibration volume table contains number of data 
   in the VocProc and AudProc volume table.
*/

typedef struct _AcdbVolTblStepSizeRspType {
   uint32_t VocProcVolTblStepSize;
   uint32_t AudProcVolTblStepSize;
} AcdbVolTblStepSizeRspType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL, ...)
   @brief API to query for a voice volume table, along with it's gain-dependent
         calibration.

   This command will obtain the specific volume table and gain-dependent
   calibration tables queried for.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbVocProcVolTblCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbVocProcVolTblCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbVocProcVolTblCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL              0x000111B1

/**
   @struct   AcdbVocProcVolTblCmdType
   @brief This is a query command structure for the RX VocProc volume table.

   @param nTxDeviceId: The TX Device ID
   @param nRxDeviceId: The RX Device ID
   @param nNetworkId: The Network ID
   @param nVocProcSampleRateId: The Voc Proc Sample Rate ID
   @param nBufferLength: The length of the calibration buffer. This should be
                         large enough to hold the entire gain-dependent
                         calibration table.
   @param nBufferPointer: A virtual memory pointer pointing to the memory
                          region ACDB will copy the gain-dependent calibration
                          table into. The format of the table will match the 
                          expected table format in the QDSP6 API. In this case,
                          the table format will be this structure:
 
                          struct VocProcVolTableEntry {
                          uint32_t nModuleId;
                          uint32_t nParamId;
                          uint16_t nParamSize; //multiple of 4
                          uint16_t nReserved; // Must be 0
                          uint8 aParamData [multiple of 4];
                          }
                          struct VocProcVolTable {
                          uint32_t nTableSize;
                          VocProcVolTableEntry aGainDepTables [];
                          }
                          struct VocProcGainDepVolTable {
                          uint32_t nStepCount;
                          VocProcVolTable aVolumeTable [nStepCount];
                          }

   This is a query command structure for the RX VocProc volume table. Each
   volume step may contain zero or more calibration structures that should
   also be applied along with the volume step.
*/
typedef struct _AcdbVocProcVolTblCmdType {
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
   uint32_t nNetworkId;
   uint32_t nVocProcSampleRateId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbVocProcVolTblCmdType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL_STEP_DATA and
 * ACDB_CMD_SET_VOCPROC_GAIN_DEP_VOLTBL_STEP_DATA
 * Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL_STEP_DATA, ...)
   @brief API to query for a specific calibration entry contained in the
          gain-dependent calibration table.

   This command will obtain the specific calibration entry contianed in the
   gain-dependent calibration table.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL_STEP_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbVocProcVolTblDataCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbVocProcVolTblDataCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbVocProcVolTblDataCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL_STEP_DATA              0x000111B5

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_VOCPROC_GAIN_DEP_VOLTBL_STEP_DATA, ...)
   @brief API to modify a specific calibration entry contained in the
          gain-dependent calibration table.

   This command will modify a specific calibration entry contianed in the
   gain-dependent calibration table.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_VOCPROC_GAIN_DEP_VOLTBL_STEP_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbVocProcVolTblDataCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbVocProcVolTblDataCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbVocProcVolTblDataCmdType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_VOCPROC_GAIN_DEP_VOLTBL_STEP_DATA              0x000111B6

/**
   @struct   AcdbVocProcVolTblDataCmdType
   @brief This is a query command structure for the voice volume table
          gain-dependent calibration data.

   @param nTxDeviceId: The TX Device ID
   @param nRxDeviceId: The RX Device ID
   @param nNetworkId: The Network ID
   @param nVocProcSampleRateId: The Voc Proc Sample Rate ID
   @param nVolumeIndex: The volume index
   @param nModuleId: The Module ID
   @param nParamId: The Parameter ID of the Module to query for.
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the parameters identified by the
                    nParamId identifier.
   @param nBufferPointer: A virtual memory pointer pointing to a memory region
                          containing the payload (or receiving the payload) 
                          identified by nParamId.
   
   This is a query command structure for the voice volume table gain-dependent calibration data.
*/
typedef struct _AcdbVocProcVolTblDataCmdType {
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
   uint32_t nNetworkId;
   uint32_t nVocProcSampleRateId;
   uint32_t nVolumeIndex;
   uint32_t nModuleId;
   uint32_t nParamId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbVocProcVolTblDataCmdType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_COPP and
 * ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_POPP
 * Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_COPP, ...)
   @brief API to query for the audio volume  gain-dependent calibration.

   This command will obtain the specific volume table step's gain-dependent
   COPP calibration table.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_COPP.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAudProcVolTblStepCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAudProcVolTblStepCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbAudProcGainDepVolTblStepCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_COPP              0x000111B9

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_POPP, ...)
   @brief API to query for the audio volume  gain-dependent calibration.

   This command will obtain the specific volume table step's gain-dependent
   POPP calibration table.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_POPP.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAudProcGainDepVolTblStepCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAudProcGainDepVolTblStepCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbAudProcGainDepVolTblStepCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_POPP              0x000111BA
/**
   @struct   AcdbAudProcGainDepVolTblStepCmdType
   @brief This is a query command structure for the audio volume tables.

   @param nDeviceId: The Device ID
   @param nApplicationType: The Application Type ID
   @param nVolumeIndex: The volume index.
   @param nBufferLength: The length of the calibration buffer. This should be
                         large enough to hold the entire gain dependent
                         calibration table.
   @param nBufferPointer: A virtual memory pointer pointing to the memory region 
                         ACDB will copy the gain-dependent calibration table into. 
                         The format of the table will match the expected table
                         format in the QDSP6 API. In this case, the table format
                         will be the repetition of this structure:
                           struct AudProcVolTableEntry {
                              uint32_t nModuleId;
                              uint32_t nParamId;
                              uint16_t nParamSize; //multiple of 4
                              uint16_t nReserved; // Must be 0
                              uint8 aParamData [multiple of 4];
                           }
   
   This is a query command structure for the gain-dependent audio calibration
   table.
*/
typedef struct _AcdbAudProcGainDepVolTblStepCmdType {
   uint32_t nDeviceId;
   uint32_t nApplicationType;
   uint32_t nVolumeIndex;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbAudProcGainDepVolTblStepCmdType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_DATA and
 * ACDB_CMD_SET_AUDPROC_GAIN_DEP_VOLTBL_STEP_DATA
 * Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_DATA, ...)
   @brief API to query for a specific calibration entry contained in the
          gain-dependent AudProc calibration table.

   This command will obtain the specific calibration entry contianed in the
   gain-dependent AudProc calibration table.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAudProcVolTblDataCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAudProcVolTblDataCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbAudProcVolTblDataCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_DATA              0x000111BC

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_AUDPROC_GAIN_DEP_VOLTBL_STEP_DATA, ...)
   @brief API to modify a specific calibration entry contained in the
          gain-dependent AudProc calibration table.

   This command will modify a specific calibration entry contianed in the
   gain-dependent VocProc calibration table.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_AUDPROC_GAIN_DEP_VOLTBL_STEP_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAudProcVolTblDataCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAudProcVolTblDataCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbAudProcVolTblDataCmdType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_AUDPROC_GAIN_DEP_VOLTBL_STEP_DATA              0x000111BD

/**
   @struct   AcdbAudProcVolTblDataCmdType
   @brief This is a query command structure for the audio volume table
          gain-dependent calibration data.

   @param nDeviceId: The Device ID
   @param nApplicationType: The Application Type ID
   @param nVolumeIndex: The volume Index
   @param nModuleId: The Module ID
   @param nParamId: The Parameter ID of the Module to query for.
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the parameters identified by the
                    nParamId identifier.
   @param nBufferPointer: A virtual memory pointer pointing to a memory region
                          containing the payload (or receiving the payload) 
                          identified by nParamId.
   
   This is a query command structure for the audio volume table gain-dependent calibration data.
*/
typedef struct _AcdbAudProcVolTblDataCmdType {
   uint32_t nDeviceId;
   uint32_t nApplicationType;
   uint32_t nVolumeIndex;
   uint32_t nModuleId;
   uint32_t nParamId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbAudProcVolTblDataCmdType;

/**
   @brief This ID is to use a specific calibration module Id for AudProc volume
          Mono gain.
*/
#define ACDB_MID_AUDIO_VOLUME_MONO                                        0x000111D2
/**
   @brief This ID is to use for a specific calibration module Id for AudProc volume
          Stereo gain.
*/
#define ACDB_MID_AUDIO_VOLUME_STEREO                                      0x000111D3
/**
   @brief This ID is to use for a specific calibration param Id for AudProc volume
          Mono gain.
*/
#define ACDB_PID_AUDIO_VOLUME_MONO                                        0x000111D5
/**
   @brief This ID is to use for a specific calibration param Id for AudProc volume
          Stereo gain.
*/
#define ACDB_PID_AUDIO_VOLUME_STEREO                                      0x000111D4

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_ADIE_CODEC_PATH_PROFILE Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_ADIE_CODEC_PATH_PROFILE, ...)
   @brief API to query for ADIE path profile.

   This command will query for ADIE path profile calibration data.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_ADIE_CODEC_PATH_PROFILE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAdiePathProfileType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAdiePathProfileType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbAdiePathProfileCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_ADIE_CODEC_PATH_PROFILE                        0x000111F2

/* ---------------------------------------------------------------------------
 * ACDB_CMD_SET_ADIE_CODEC_PATH_PROFILE Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_ADIE_CODEC_PATH_PROFILE, ...)
   @brief API to override command for ADIE path profile.

   This command will override ADIE path profile calibration data in ACDB.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_ADIE_CODEC_PATH_PROFILE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAdiePathProfileType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAdiePathProfileType).
   @param[out] pResponseStruct:
         This should be a pointer to NULL.
   @param[in] nResponseStructLength:
         This should equal to 0.

   @see  acdb_ioctl
   @see  AcdbAdiePathProfileCmdType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_ADIE_CODEC_PATH_PROFILE                        0x000111F3

/**
   @struct   AcdbAdiePathProfileCmdType
   @brief This is a query command structure for query the Adie path profile 
   
   @param ulCodecPathId: The codec path ID
   @param nFrequencyId: Frequency index ID
   @param nOversamplerateId: The Device actual Over sample rate operate at
   @param nParamId: Parameter ID control the data structure versioning
   @param nBufferLength: The length of the calibration buffer. This should be
                         large enough to hold the parameters identified by the
                         nParamId identifier.
   @param nBufferPointer: A virtual memory pointer pointing to a memory region
                          containing the payload (or receiving the payload) 
                          identified by nParamId.

   This is a query command structure for query the Adie path profile 
*/
typedef struct _AcdbAdiePathProfileCmdStruct {
   uint32_t ulCodecPathId;
   uint32_t nFrequencyId;
   uint32_t nOversamplerateId;
   uint32_t nParamId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;   
} AcdbAdiePathProfileCmdType;

/**
   @def ACDB_PID_ADIE_CODEC_PATH_TABLA
   @brief This is the Parameter ID identifying the format of the Adie Codec
          path profile structure.
   @see This structure is associate with Adie codec path 
*/
#define ACDB_PID_ADIE_CODEC_PATH_TABLA                           0x000113BB
/**
struct _AcdbAdieTABLACodecACDBHeader
{
   uint32_t size;
   uint32_t version;
   uint32_t ePathID;
   uint32_t ePathFormat;
   uint32_t ePathType;
   uint32_t ePathFreq;
   uint32_t numActions;
   uint32_t eAnalogPortID[8];
   uint32_t ePreferredPortMask[8];
} AcdbAdieTABLACodecACDBHeader;

struct _AdieTABLACodecDbActionUnit
{
   uint32_t command;
   uint32_t data;
} AdieTABLACodecDbActionUnit;

struct _AcdbAdieTABLACodecDbProfile 
{
   AcdbAdieTABLACodecACDBHeader header;
   AdieTABLACodecDbActionUnit action[numActions];
} AcdbAdieTABLACodecDbProfile;
*/

/**
   @def ACDB_PID_ADIE_CODEC_PATH_WCD9304
   @brief This is the Parameter ID identifying the format of the Adie Codec
          path profile structure.
   @see This structure is associate with Adie codec path 
*/
#define ACDB_PID_ADIE_CODEC_PATH_WCD9304                         0x00011313
/**
typedef AcdbAdieTABLACodecACDBHeader AcdbAdieWCD9304CodecACDBHeader;
typedef AdieTABLACodecDbActionUnit AdieWCD9304CodecDbActionUnit;
typedef AcdbAdieTABLACodecDbProfile AcdbAdieWCD9304CodecDbProfile;
*/

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_ANC_TX_DEVICE Declarations and Documentation
 * --------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_ANC_TX_DEVICE, ...)
   @brief API to query for TX device Id with corresponding ANC device paired RX device Id.

   This command will query for a TX device Id with given RX device Id.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_ANC_TX_DEVICE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAncDevicePairCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAncDevicePairCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbAncDevicePairRspType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbAncDevicePairRspType).

   @see  acdb_ioctl
   @see  AcdbAncDevicePairCmdType
   @see  AcdbAncDevicePairRspType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_ANC_TX_DEVICE                       0x000111DE

/**
   @struct   AcdbAncDevicePairCmdType
   @brief This is a query command structure for the ANC device paired
          TX device.

   @param nRxDeviceId: The RX Device ID

   This is a query command structure for the ANC device pair TX device
*/
typedef struct _AcdbAncDevicePairCmdType {
   uint32_t nRxDeviceId;
} AcdbAncDevicePairCmdType;

/**
   @struct   AcdbAncDevicePairRspType

   @brief This is a response structure for querying ANC device paired TX device.
          if RX device not found,TX device will be returned as 0x00000000.
   @param nTxAncDeviceId: The TX ANC Device ID

   This is a query command structure for the ANC device pair TX device, if RX 
   device not found,TX device will be returned as 0x00000000.
*/
typedef struct _AcdbAncDevicePairRspType {
   uint32_t nTxAncDeviceId;
} AcdbAncDevicePairRspType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_ANC_SETTING Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_ANC_SETTING, ...)
   @brief API to query for a list of ANC device setting.

   This command will query for ANC setting calibration data.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_ANC_SETTING.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbANCSettingType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbANCSettingType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbANCSettingCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_ANC_SETTING                       0x000111F5

/* ---------------------------------------------------------------------------
 * ACDB_CMD_SET_ANC_SETTING Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_ANC_SETTING, ...)
   @brief API to query for a list of ANC device setting.

   This command will set for ANC setting calibration data.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_ANC_SETTING.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbANCSettingType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbANCSettingType).
   @param[out] pResponseStruct:
         This should be a pointer to NULL.
   @param[in] nResponseStructLength:
         This should equal to 0.

   @see  acdb_ioctl
   @see  AcdbANCSettingCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_ANC_SETTING                       0x000111F6

/**
   @struct   AcdbANCSettingCmdType
   @brief This is a query command structure for query the Adie ANC Config
          data.
   
   @param nRxDeviceId: The Rx Device ID
   @param nFrequencyId: Frequency index ID
   @param nOversamplerateId: The device actual Over sample rate operate at
   @param nParamId: Parameter ID control the data structure versioning
   @param nBufferLength: The length of the calibration buffer. This should be
                         large enough to hold the parameters identified by the
                         nParamId identifier.
   @param nBufferPointer: A virtual memory pointer pointing to a memory region
                          containing the payload (or receiving the payload) 
                          identified by nParamId.

   This is a query command structure for query the Adie ANC Config data. 
*/
typedef struct _AcdbANCSettingCmdStruct {
   uint32_t nRxDeviceId;
   uint32_t nFrequencyId;
   uint32_t nOversamplerateId;
   uint32_t nParamId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbANCSettingCmdType;

#define ACDB_PID_CODEC_DATA_TABLA                     0x000112A0
/**
//ANC TABLA Config data structure format
typedef struct
{
  uint8_t default_enable
  uint8_t anc_feedback_enable;
  uint8_t anc_lr_mix_enable
  uint8_t smlpf_enable;
  uint8_t lr_swap;
  uint8_t ff_in_enable;
  uint8_t hybrid_enable;
  uint8_t ff_out_enable;
  uint8_t dcflt_enable; 
  uint8_t adaptive;      
  uint8_t padding[3];   
  uint8_t anc_lr_mix_output_channel;
  uint8_t anc_ff_shift;   //range: 0 ¨C 15
  uint8_t anc_fb_shift;   //range: 0 ¨C 15
  int32_t anc_ff_coeff[15];              
  int32_t anc_gain;                      
  int32_t anc_ff_lpf_coeff[2];           
  int32_t anc_fb_coeff[13];              
  int32_t anc_gain_default;              
  int32_t anc_fb_lpf_coeff[2];           
  uint32_t input_device;  
  uint32_t smlpf_shift; 
  uint32_t dcflt_shift  
} wcd_adie_codec_db_anc_config;
*/

/**
   @def ACDB_PID_CODEC_ANC_DATA_WCD9304
   @brief This is the Parameter ID identifying the format of the Adie ANC
           Config data structure for WCD9304.
   
   @see AcdbANCSettingCmdType
*/

#define ACDB_PID_CODEC_ANC_DATA_WCD9304               ACDB_PID_CODEC_DATA_TABLA

/**
//ANC WCD9304 Config data structure format
typedef wcd_adie_codec_db_anc_config wcd9304_adie_codec_db_anc_config;
*/

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID Declarations and Documentation
 * --------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDB_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID, ...)
   @brief API to query for audio COPP topology ID

   This command will obtain AudProc common COPP topology ID queried for.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGetAudProcTopIdCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbGetAudProcTopIdCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbGetTopologyIdRspType.
   @param[out] nResponseStructLength:
         This should equal sizeof (AcdbGetTopologyIdRspType).

   @see  acdb_ioctl
   @see  AcdbGetAudProcTopIdCmdType
   @see  AcdbGetTopologyIdRspType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID                 0x0001122D

/**
   @struct  AcdbGetAudProcTopIdCmdType
   @brief This is a query command structure for query audio COPP topology ID.
   
   @param nDeviceId: Device ID.
   @param nApplicationType: Application type ID.

   This is a query command structure for query audio COPP topology ID.
*/
typedef struct _AcdbGetAudProcTopIdCmdType {
   uint32_t nDeviceId;
   uint32_t nApplicationType;
} AcdbGetAudProcTopIdCmdType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDB_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID, ...)
   @brief API to query for audio POPP topology ID

   This command will obtain AudProc stream POPP topology ID queried for.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGetAudProcStrmTopIdCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbGetAudProcStrmTopIdCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbGetTopologyIdRspType.
   @param[out] nResponseStructLength:
         This should equal sizeof (AcdbGetTopologyIdRspType).

   @see  acdb_ioctl
   @see  AcdbGetAudProcStrmTopIdCmdType
   @see  AcdbGetTopologyIdRspType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID                 0x0001122E

/**
   @struct  AcdbGetAudProcStrmTopIdCmdType
   @brief This is a query command structure for query audio COPP topology ID.
   
   @param nApplicationType: Application type ID.

   This is a query command structure for query audio COPP topology ID.
*/
typedef struct _AcdbGetAudProcStrmTopIdCmdType {
   uint32_t nApplicationType;
} AcdbGetAudProcStrmTopIdCmdType;

/**
   @struct  AcdbGetTopologyIdRspType
   @brief This is a response structure for query topology ID.
   
   @param nTopologyId: Topology ID.

   This is a query response structure for query topology ID.
*/
typedef struct _AcdbGetTopologyIdRspType {
   uint32_t nTopologyId;
} AcdbGetTopologyIdRspType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_AFE_DATA and ACDB_CMD_SET_AFE_DATA
 * Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_AFE_DATA, ...)
   @brief API to query for individual calibration data.

   This command will obtain the specific AFE calibration data 
   queried for.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AFE_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAfeDataCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAfeDataCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbAfeDataCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_AFE_DATA                                     0x0001124E

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_AFE_DATA, ...)
   @brief API to change individual calibration data in database.

   This command will allow the overriding of AFE calibration 
   data stored in the database.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_AFE_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAfeDataCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAfeDataCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbAfeDataCmdType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_AFE_DATA                                      0x0001124F

/**
   @def ACDB_MID_SIDETONE
   @brief This is the module identifyier. It identifies what module should be
          query for.
*/
#define ACDB_MID_SIDETONE                                          0x0001270E

/**
   @def ACDB_PID_SIDETONE
   @brief This ID is to use for a specific calibration param Id assocaite
          with sidetone data structure.
*/
#define ACDB_PID_SIDETONE                                          0x0001270F

/**
   @struct   AcdbAfeDataCmdType
   @brief This is a query command structure allowing for individual get/set
          for calibration data in the AFE data tables.

   @param nTxDeviceId: The Tx path Device ID
   @param nRxDeviceId: The Rx path Device ID
   @param nModuleId: The Module ID
   @param nParamId: The Parameter ID of the Module to query for.
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the parameters identified by the
                    nParamId identifier.
   @param nBufferPointer: A virtual memory pointer pointing to a memory region
                          containing the payload (or receiving the payload) 
                          identified by nParamId.
  
   This is a query command structure allowing for multiple get/set of
   calibration data in the AFE data tables. The format of the data is described
   in below.
   
   struct AFESideToneStruct {
      uint16_t nEnableFlag;
      uint16_t nGain; //Linear gain
   }
*/
typedef struct _AcdbAfeDataCmdType {
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
   uint32_t nModuleId;
   uint32_t nParamId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbAfeDataCmdType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_GLBTBL_DATA and ACDB_CMD_SET_GLBTBL_DATA
 *    Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_GLBTBL_DATA, ...)
   @brief API to query for individual calibration data.

   This command will obtain the specific global table calibration data 
   queried for.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_GLBTBL_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGblTblCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbGblTblCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioct
   @see  AcdbGblTblCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_GLBTBL_DATA                                     0x0001126B

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_GLBTBL_DATA, ...)
   @brief API to change individual calibration data in database.

   This command will allow the overriding global table calibration 
   data stored in the database.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_GLBTBL_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGblTblCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbGblTblCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbGblTblCmdType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_GLBTBL_DATA                                     0x0001126C

/**
   @struct   AcdbGblTblCmdType
   @brief This is a query command structure allowing for individual get/set
          for calibration data in the Global cal tables.

   @param nModuleId: The Module ID
   @param nParamId: The Parameter ID of the Module to query for.
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the parameters identified by the
                    nParamId identifier.
   @param nBufferPointer: A virtual memory pointer pointing to a memory region
                          containing the payload (or receiving the payload) 
                          identified by nParamId.
  
   This is a query command structure allowing for multiple get/set of
   calibration data in the global calibration table. The format of the data is described
   in below.

   The return buffer formatted is determined by the parameter ID as one of key to access
   to ACDB.
   //Existing Pause/resume volume stepping structure is formatted as
   struct VolumeStepStruct {
      uint32_t nEnableFlag; //Enable flag
      uint32_t nPeriod; //duration
      uint32_t nStep;   //number of step
      uint32_t ramping_curve; //Ramping curve
   }
*/
typedef struct _AcdbGblTblCmdType {
   uint32_t nModuleId;
   uint32_t nParamId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbGblTblCmdType;

/**
   @def ACDB_MID_PAUSE_RESUME_VOLUME_CONTROL
   @brief This is the module identifyier. It identifies what module should be
          query for.
*/
#define ACDB_MID_PAUSE_RESUME_VOLUME_CONTROL                      0x0001127B

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_COMMON_DEVICE_INFO Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_COMMON_DEVICE_INFO, ...)
   @brief API to query for the common device information for a single device.

   This command will obtain the common device information describing
   the device properties of a given device id.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_DEVICE_INFO.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDeviceInfoCmnCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDeviceInfoCmnCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbDeviceInfoCmnCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_COMMON_DEVICE_INFO                           0x00011290

/**
   @struct   AcdbDeviceInfoCmnCmdType
   @brief This is a query command structure for the device info.

   @param nDeviceId: The Device ID to lookup
   @param nBufferLength: The length of the buffer. This should be
                    large enough to hold the entire device info structure
                    identified by nParamId.
   @param nBufferPointer: A virtual memory pointer pointing to the memory
                    region ACDB will copy the device info into.

   This is a query command structure for the device info.

   @def ACDB_COMMON_DEVICE_INFO_HDR_PARAM
   @brief This is the Parameter ID identifying the format of the common device
          information structure.
   @see AcdbCommonDeviceInfoType
*/
#define ACDB_COMMON_DEVICE_INFO_HDR_PARAM                          0x000112A7
/**   
   This is the common device information return structure containing common device
          metadata for the audio driver.

   struct _AcdbDeviceInfoCommonRspType
   {
      uint32_t headerVersion: The version of the common device info type structure format
                              ACDB_COMMON_DEVICE_INFO_HDR_PARAM - 0x000112A7
                              - First Common device info header version ID
      uint8_t payload[payloadsize]; - payloadsize is identified by the common device info 
                                      structure size.
   } AcdbDeviceInfoCommonRspType

   struct {
            uint32_t ulDeviceType: The device type.
                                   (0x00011296) - ACDB_DEVICE_TYPE_I2S_PARAM
                                                 :I2S type ParamId
                                   (0x00011297) - ACDB_DEVICE_TYPE_PCM_PARAM
                                                 :PCM type ParamId
                                   (0x00011298) - ACDB_DEVICE_TYPE_SLIMBUS_PARAM
                                                 :Slimbus type ParamId
                                   (0x00011299) - ACDB_DEVICE_TYPE_DIGITAL_MIC_PARAM
                                                 :DigitalMic Type ParamId
                                   (0x0001129A) - ACDB_DEVICE_TYPE_HDMI_PARAM
                                                 :HDMI type ParamId
                                   (0x0001129B) - ACDB_DEVICE_TYPE_VIRTUAL_PARAM
                                                 :Virtual Device Type ParamId
                                   (0x0001129C) - ACDB_DEVICE_TYPE_INT_BTFM_PARAM
                                                 :Internal B/T and F/M config type ParamId
                                   (0x0001129D) - ACDB_DEVICE_TYPE_RT_PROXY_PARAM
                                                 :RT Proxy device type ParamId
           }
*/
typedef struct _AcdbDeviceInfoCmnCmdType {
   uint32_t nDeviceId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbDeviceInfoCmnCmdType;

/**
   @def ACDB_DEVICE_TYPE_I2S_PARAM
   @brief This is the Parameter ID identifying the device type - I2S device type
*/
#define ACDB_DEVICE_TYPE_I2S_PARAM                           0x00011296
/**
   @def ACDB_DEVICE_TYPE_PCM_PARAM
   @brief This is the Parameter ID identifying the device type - PCM device type
*/
#define ACDB_DEVICE_TYPE_PCM_PARAM                           0x00011297
/**
   @def ACDB_DEVICE_TYPE_SLIMBUS_PARAM
   @brief This is the Parameter ID identifying the device type - Slimbus device type
*/
#define ACDB_DEVICE_TYPE_SLIMBUS_PARAM                       0x00011298
/**
   @def ACDB_DEVICE_TYPE_DIGITAL_MIC_PARAM
   @brief This is the Parameter ID identifying the device type - Digital Mic device type
*/
#define ACDB_DEVICE_TYPE_DIGITAL_MIC_PARAM                   0x00011299
/**
   @def ACDB_DEVICE_TYPE_HDMI_PARAM
   @brief This is the Parameter ID identifying the device type - HDMI device type
*/
#define ACDB_DEVICE_TYPE_HDMI_PARAM                          0x0001129A
/**
   @def ACDB_DEVICE_TYPE_VIRTUAL_PARAM
   @brief This is the Parameter ID identifying the device type - Virtual device type
*/
#define ACDB_DEVICE_TYPE_VIRTUAL_PARAM                       0x0001129B
/**
   @def ACDB_DEVICE_TYPE_INT_BTFM_PARAM
   @brief This is the Parameter ID identifying the device type - Internal B/T and F/M device type
*/
#define ACDB_DEVICE_TYPE_INT_BTFM_PARAM                      0x0001129C
/**
   @def ACDB_DEVICE_TYPE_RT_PROXY_PARAM
   @brief This is the Parameter ID identifying the device type - RT proxy device type
*/
#define ACDB_DEVICE_TYPE_RT_PROXY_PARAM                      0x0001129D
/**
   @def ACDB_DEVICE_TYPE_SIMPLE_PARAM 
   @brief This is the Parameter ID identifying the device type - 
          Simple device type
*/
#define ACDB_DEVICE_TYPE_SIMPLE_PARAM                      0x00011328

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_TARGET_SPECIFIC_DEVICE_INFO Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_TARGET_SPECIFIC_DEVICE_INFO, ...)
   @brief API to query for the target specific device information for a single device.

   This command will obtain the target specific device information describing
   the device properties of a given device id.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_TARGET_SPECIFIC_DEVICE_INFO.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDeviceInfoTargetSpecificCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDeviceInfoTargetSpecificCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbDeviceInfoTargetSpecificCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_TARGET_SPECIFIC_DEVICE_INFO               0x00011291

/**
   @struct   AcdbDeviceInfoTargetSpecificCmdType
   @brief This is a query command structure for the target specific device info.

   @param nDeviceId: The Device ID
   @param nDeviceType: The Device Type ID query from common device info
   @param nBufferLength: The length of the buffer. This should be
                    large enough to hold the entire device info structure
                    identified by nParamId.
   @param nBufferPointer: A virtual memory pointer pointing to the memory
                    region ACDB will copy the device info into.
   
   The return buffer formatted is determined by the parameter ID as one of key to access
   to ACDB.
   struct _AcdbDeviceInfoTargetSpecificRspType
   {
      uint16_t	ulPayloadMajorVersion;
      uint16_t	ulPayloadMinorVersion;
      uint8_t payload[payloadsize]; - payloadsize is identified by the target specific device structure size.
   } AcdbDeviceInfoTargetSpecificRspType

   /// I2S Device Type
   @def ulPayloadMajorVersion = 0x0001
        ulPayloadMinorVersion = 0x0000
   @brief This is the Parameter ID identifying the format of the I2S device
          information structure.
   struct 
   {
      uint32_t	ulCodecPathId;
      uint32_t	ulCodecPathId2;
      uint32_t	ulAfePortId;
      uint32_t	ulAfeChannelMode;
      uint32_t	ulAfeMonoStereo;           //Mono(0), mono_right (1), mono_left (2) , stereo(3)
      uint32_t	ulAfeWordSyncSrcSelect;	   //Word select source external (0), Word select source internal (1)
      uint32_t	ulBitsPerSampleMask;
      uint32_t	ulChannelConfig;
      uint32_t	ulI2SMClkMode;
      uint32_t	ulOversampleRateMask;
      uint32_t	ulSampleRateMask;
      uint32_t ulDirectionMask;
   }

   /// PCM Device Type
   @def ulPayloadMajorVersion = 0x0001
        ulPayloadMinorVersion = 0x0000
   @brief This is the Parameter ID identifying the format of the PCM device
          information structure.
   struct 
   {
      uint32_t	ulAfePortId;
      uint32_t	ulAfeAuxMode; 	            //PCM mode (0) AUX_PCM mode (1)
      uint32_t	ulAfeSyncSrcSelect;        //Src_external(0) src_internal (1)		
      uint32_t	ulAfeFrameSetting;	      // bits width per frame: 8 bits  (0), 16 bits (1) 32 bits (2) 64bits (3) 128bits(4) 256 bits(5)
      uint32_t	ulAfeQuantType;            // Quant type: A-law no padding (0), Mu-law no padding (1), linear no padding (2) 
                                          // A-law padding (3) Mu-law padding (4) linear padding (5)
      uint32_t	ulAfeSlotNo;               // slot number: valid data 0 - 31
      uint32_t	ulAfeDataOutEnableCtrl;    // Output Data Enable Control: tri-state disable (0) tri-state enable (1)
      uint32_t ulBitsPerSampleMask;
      uint32_t ulChannelConfig;
      uint32_t	ulSampleRateMask;
      uint32_t	ulDirectionMask;
   }

   /// Slimbus Device Type
   @def ulPayloadMajorVersion = 0x0001
        ulPayloadMinorVersion = 0x0000
   @brief This is the Parameter ID identifying the format of the SlimBus device
          information structure.
   struct 
   {     
      uint32_t	ulCodecPathId;
      uint32_t	ulBitsPerSampleMask;
      uint32_t ulChannelConfig
      uint32_t	ulSampleRateMask;
      uint32_t	ulDirectionMask;
   }

   @def ulPayloadMajorVersion = 0x0001
        ulPayloadMinorVersion = 0x0001
   @brief This is the Parameter ID identifying the format of the SlimBus device
          information structure.
   struct 
   {     
      uint32_t	ulCodecPathId;
      uint32_t	ulBitsPerSampleMask;
      uint32_t ulChannelConfig
      uint32_t	ulSampleRateMask;
      uint32_t	ulDirectionMask;
      uint32_t ulAfePortId;
   }

   /// Digital Mic Device Type
   @def ulPayloadMajorVersion = 0x0001
        ulPayloadMinorVersion = 0x0000
   @brief This is the Parameter ID identifying the format of the digital mic device
          information structure.
   struct 
   {
      uint32_t	ulAfePortId;
      uint32_t	ulAfeChannelMode; 	
      uint32_t	ulBitsPerSampleMask;
      uint32_t ulChannelConfig;
      uint32_t	ulMasterClkMode;
      uint32_t	ulOversampleRateMask;
      uint32_t	ulSampleRateMask;
      uint32_t ulDirectionMask;
   }

   /// HDMI Device Type
   @def ulPayloadMajorVersion = 0x0001
        ulPayloadMinorVersion = 0x0000
   @brief This is the Parameter ID identifying the format of the HDMI device
          information structure.
   struct 
   {
      uint32_t	ulAfePortId;
      uint32_t	ulAfeChannelMode;
      uint32_t	ulAfeDataType              //linear data type (0), non-linear data type (1)	
      uint32_t	ulBitsPerSampleMask;	      //This is not specific to Afe interface.
      uint32_t ulChannelConfig;
      uint32_t ulSampleRateMask;
      uint32_t ulDirectionMask;
   }

   /// Virtual Device Type
   @def ulPayloadMajorVersion = 0x0001
        ulPayloadMinorVersion = 0x0000
   @brief This is the Parameter ID identifying the format of the virtual device
          information structure.
   struct 
   {
      uint32_t    ulAfePortId;                  
      uint32_t    ulBitsPerSampleMask;
      uint32_t    ulChannelConfig;
      uint32_t    ulAfeTimingMode;          //TIMING_MODE_FTRT             0x0 -> timing operate faster than real time
                                            //TIMING_MODE_TIMER            0x1 -> timing operate at system timer rate
      uint32_t    ulSampleRateMask;
      uint32_t    ulDirectionMask;
   }

   /// INT BTFM Device Type
   @def ulPayloadMajorVersion = 0x0001
        ulPayloadMinorVersion = 0x0000
   @brief This is the Parameter ID identifying the format of the INT BTFM device
          information structure.
   struct 
   {
      uint32_t	ulAfePortId;
      uint32_t	ulBitsPerSampleMask;
      uint32_t	ulChannelConfig;
      uint32_t	ulSampleRateMask;
      uint32_t	ulDirectionMask;
   }

   /// RT Proxy Device Type
   @def ulPayloadMajorVersion = 0x0001
        ulPayloadMinorVersion = 0x0000
   @brief This is the Parameter ID identifying the format of the rt proxy device
          information structure.
   struct 
   {
      uint32_t ulSampleRateMask;
      uint32_t ulChannelConfig;
      uint32_t ulDirectionMask;
      uint32_t ulAfePortId;
      uint32_t ulAfeBitsPerSampleMask;
   }

   This is a query command structure for the target specific device info.
*/
typedef struct _AcdbDeviceInfoTargetSpecificCmdType {
   uint32_t nDeviceId;
   uint32_t nDeviceType;
   uint32_t nBufferLength;
   uint8_t *pBufferPointer;
} AcdbDeviceInfoTargetSpecificCmdType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_VOCPROC_COMMON_TOPOLOGY_ID Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDB_CMD_GET_VOCPROC_COMMON_TOPOLOGY_ID, ...)
   @brief API to query for voice COPP topology ID

   This command will obtain VocProc common COPP topology ID queried for.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_VOCPROC_COMMON_TOPOLOGY_ID.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGetVocProcTopIdCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbGetVocProcTopIdCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbGetTopologyIdRspType.
   @param[out] nResponseStructLength:
         This should equal sizeof (AcdbGetTopologyIdRspType).

   @see  acdb_ioctl
   @see  AcdbGetVocProcTopIdCmdType
   @see  AcdbGetTopologyIdRspType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_VOCPROC_COMMON_TOPOLOGY_ID                 0x00011292

/**
   @struct  AcdbGetVocProcTopIdCmdType
   @brief This is a query command structure for query VocProc COPP and POPP topology ID.
   
   @param nDeviceId: Device ID.

   This is a query command structure for query VocProc COPP and POPP topology ID.
*/
typedef struct _AcdbGetVocProcTopIdCmdType {
   uint32_t nDeviceId;
} AcdbGetVocProcTopIdCmdType;

/**
   @brief This ID is to use for a specific calibration module Id for MBHC.
*/
#define ACDB_MID_MBHC											0x000112E5
/**
   @brief This ID is to use for a specific calibration param Id GENERAL_CONFIG in MBHC module.
   ACDB_CMD_GET_GLBTBL_DATA IOCTL call should be used to get the data for this PID
   The output of ACDB_CMD_GET_GLBTBL_DATA IOCTL call for this PID results in the below structure format.
	struct gen_config
	{
		uint8_t	MBHC_Tldoh;
		uint8_t	MBHC_TbgFastSettle;
		uint8_t	MBHC_TshutDownPlugRem;
		uint8_t	MBHC_NSA;
		uint8_t	MBHC_Navg;
		uint8_t	MBHC_VmicBiasL;
		uint8_t	MBHC_VmicBias;
		uint8_t	reserve;
		uint16_t 	MBHC_SettleWait   ;
		uint16_t 	TmicbiasRampup;
		uint16_t 	TmicbiasRampdown;
		uint16_t 	MBHC_TsupplyBringup;
	};
*/
#define ACDB_PID_GENERAL_CONFIG                                 0x000112E6
/**
   @brief This ID is to use for a specific calibration param Id PLUG_REMOVAL_DETECTION	in MBHC module.
   ACDB_CMD_GET_GLBTBL_DATA IOCTL call should be used to get the data for this PID
   The output of ACDB_CMD_GET_GLBTBL_DATA IOCTL call for this PID results in the below structure format.
    struct removal_detect
	{
		uint32_t 	MBHC_Current_MIC_MBHC_PIDmicCurrent;
		uint32_t 	MBHC_Current_HPH_MBHC_PIDhphCurrent;
		uint16_t 	MBHC_TmicPid;
		uint16_t 	MBHC_TinsCompl;
		uint16_t 	MBHC_TinsRetry;
		uint16_t 	MBHC_VremovalDelta;
		uint8_t	MBHC_micbiasSlowRamp ;
		uint8_t	reserve ;
		uint8_t	reserve1;
		uint8_t	reserve2;
	};
*/
#define ACDB_PID_PLUG_REMOVAL_DETECTION							0x000112E7
/**
   @brief This ID is to use for a specific calibration param Id PLUG_TYPE_DETECTION in MBHC module.
   ACDB_CMD_GET_GLBTBL_DATA IOCTL call should be used to get the data for this PID
   The output of ACDB_CMD_GET_GLBTBL_DATA IOCTL call for this PID results in the below structure format.
    struct type_detection
	{
		uint8_t	MBHC_AVdetect;
		uint8_t	MBHC_MonoDetect;
		uint8_t	MBHC_NinsTries;
		uint8_t	reserve;
		int16_t	MBHC_VnoMic;
		int16_t	MBHC_VavMin;
		int16_t	MBHC_VavMax;
		int16_t	MBHC_VHSMin;
		int16_t	MBHC_VHSMax;
		uint16_t	reserve2;
	};

*/
#define ACDB_PID_PLUG_TYPE_DETECTION							0x000112E8
/**
   @brief This ID is to use for a specific calibration param Id BUTTON_PRESS_DETECTION	in MBHC module.
   ACDB_CMD_GET_GLBTBL_DATA IOCTL call should be used to get the data for this PID
   The output of ACDB_CMD_GET_GLBTBL_DATA IOCTL call for this PID results in the below structure format.
    struct press_detection
	{
		int8_t 	MBHC_coeff[8];
		uint8_t	MBHC_Ncoeff;
		uint8_t	MBHC_Nmeas;
		uint8_t	MBHC_NSC;
		uint8_t	MBHC_NbutMeas;
		uint8_t	MBHC_NbutCon;
		uint8_t	MBHC_NumBut;
		uint8_t	reserve;
		uint8_t	reserve2;
		uint16_t	MBHC_Tpoll;
		uint16_t	MBHC_TBounceWait;
		uint16_t	MBHC_RelTimeOut;
		int16_t 	MBHC_VButPressDeltaSTA;
		int16_t 	MBHC_VButPressDeltaCIC;
		uint16_t 	Tbutton0TimeOut;
		int16_t 	MBHC_VButLow[MBHC_NumBut];
		int16_t 	MBHC_VButHigh[MBHC_NumBut];
		uint8_t	MBHC_Nready[MBHC_NbutCon] ;
		uint8_t	MBHC_NCIC[MBHC_NbutCon];
		uint8_t	MBHC_Gain[MBHC_NbutCon];
	};
*/
#define ACDB_PID_BUTTON_PRESS_DETECTION							0x000112E9
/**
   @brief This ID is to use for a specific calibration param Id IMPEDANCE_DETECTION in MBHC module.
   ACDB_CMD_GET_GLBTBL_DATA IOCTL call should be used to get the data for this PID
   The output of ACDB_CMD_GET_GLBTBL_DATA IOCTL call for this PID results in the below structure format.
    struct impedance_detection
	{
		int8_t	MBHC_HeadSetImpDet;
		uint8_t 	MBHC_NumRload;
		int8_t	MBHC_HPHkeepon;
		uint8_t 	MBHC_RepeatRloadCalc;
		uint16_t 	MBHC_DacRampTime;
		uint16_t 	MBHC_RhphHigh;
		uint16_t 	MBHC_Rhphlow;
		uint16_t 	MBHC_rload[MBHC_NumRload];
		uint16_t 	MBHC_alpha[MBHC_NumRload];
		uint16_t 	MBHC_beta[3];
	};
*/
#define ACDB_PID_IMPEDANCE_DETECTION							0x000112EA

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_AFE_COMMON_TABLE and ACDB_CMD_SET_AFE_COMMON_COMMON_TABLE
 *    Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_AFE_COMMON_TABLE, ...)
   @brief API to query for an entire calibration table in the QDSP6 table format.

   This command will obtain the specific calibration data queried for AFE 
   Common Table

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AFE_COMMON_TABLE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAfeCommonTableCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAfeCommonTableCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbAfeCommonTableCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_AFE_COMMON_TABLE    0x000112EF 

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_AFE_COMMON_TABLE, ...)
   @brief API to change a set of calibration data in database.

   This command will allow the overriding of AFE common calibration data stored 
   in the database.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_AFE_COMMON_TABLE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAfeCommonTableCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAfeCommonTableCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbAfeCommonTableCmdType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_AFE_COMMON_TABLE    0x000112F0  

/**
   @struct   AcdbAfeCommonTableCmdType
   @brief This is a query command structure allowing for multiple get/set
          of calibration data in the vocproc data tables.

   @param nDeviceId: The TX/RX Device ID
   @param nSampleRateId: The AFE Sample Rate ID
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the entire table.
   @param nBufferPointer: A virtual memory pointer pointing to the memory
                    region ACDB will copy the table into.
   
   This is a query command structure allowing for multiple get/set of
   calibration data in the afe common data tables. The format of the table will
   match the expected table format in the QDSP6 API. In this case, the
   table format will be the repetition of this structure:
      struct AfeCommonTableEntry {
         uint32_t nModuleId;
         uint32_t nParamId;
         uint16_t nParamSize; //multiple of 4
         uint16_t nReserved; // Must be 0
         uint8 aParamData [multiple of 4)];
     }
*/
typedef struct _AcdbAfeCommonTableCmdType {
   uint32_t nDeviceId;
   uint32_t nSampleRateId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbAfeCommonTableCmdType ;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_AFE_COMMON_DATA and ACDB_CMD_SET_AFE_COMMON_DATA
 *    Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_AFE_COMMON_DATA, ...)
   @brief API to query for individual calibration data.

   This command will obtain the specific AFE calibration data 
   queried for.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AFE_COMMON_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAfeCmnDataCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAfeCmnDataCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbAfeCmnDataCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_AFE_COMMON_DATA                                     0x000112FD

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_AFE_COMMON_DATA, ...)
   @brief API to change individual calibration data in database.

   This command will allow the overriding of AFE calibration 
   data stored in the database.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_AFE_COMMON_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAfeCmnDataCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAfeCmnDataCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbAfeCmnDataCmdType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_AFE_COMMON_DATA                                      0x000112FE
/**
   @struct   AcdbAfeCmnDataCmdType
   @brief This is a query command structure allowing for individual get/set
          for calibration data in the AFECmn data tables.

   @param nDeviceId: The Device ID
   @param nAfeSampleRateId: The Afe sample rate id
   @param nModuleId: The Module ID
   @param nParamId: The Parameter ID of the Module to query for.
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the parameters identified by the
                    nParamId identifier.
   @param nBufferPointer: A virtual memory pointer pointing to a memory region
                          containing the payload (or receiving the payload) 
                          identified by nParamId.
  
*/
typedef struct _AcdbAfeCmnDataCmdType {
   uint32_t nDeviceId;
   uint32_t nAfeSampleRateId;
   uint32_t nModuleId;
   uint32_t nParamId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbAfeCmnDataCmdType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_AFE_TOPOLOGY_LIST Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_AFE_TOPOLOGY_LIST, ...)
   @brief API to query for a list of device pairs.

   This command will query for all possible device pairs.

   return structure will look like
   struct AfeTopologyList {
      uint32_t nCount;
      struct AfeTopItem {
         uint32_t nDeviceID;
         uint32_t nAfeTopologyID;
      } AfeTopItem[nCount];
   };

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AFE_TOPOLOGY_LIST.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGeneralInfoCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbGeneralInfoCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbGeneralInfoCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_AFE_TOPOLOGY_LIST                   0x00011300
/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_COMPATIBLE_REMOTE_DEVICE_ID Declarations and Documentation
 * --------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDB_CMD_GET_COMPATIBLE_REMOTE_DEVICE_ID, ...)
   @brief API to query for compatible Device ID on Remote Proc, for a given Device ID on Local proc.

   This command will obtain compatible Device ID on the Remote proc queried, for a given Device ID on local proc.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_COMPATIBLE_REMOTE_DEVICE_ID.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGetRmtCompDevIdCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbGetRmtCompDevIdCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbGetRmtCompDevIdRspType.
   @param[out] nResponseStructLength:
         This should equal sizeof (AcdbGetRmtCompDevIdRspType).

   @see  acdb_ioctl
   @see  AcdbGetRmtCompDevIdCmdType
   @see  AcdbGetRmtCompDevIdRspType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the data not exists for the given device id.
*/
#define ACDB_CMD_GET_COMPATIBLE_REMOTE_DEVICE_ID                 0x00011331

/**
   @struct  AcdbGetRmtCompDevIdCmdType
   @brief This is a query command structure for query audio COPP topology ID.
   
   @param nNativeDeviceId: Device ID on the local proc.

   This is a query command structure for query ACDB_CMD_GET_COMPATIBLE_REMOTE_DEVICE_ID.
*/
typedef struct _AcdbGetRmtCompDevIdCmdType {
   uint32_t nNativeDeviceId;
} AcdbGetRmtCompDevIdCmdType;

/**
   @struct  AcdbGetRmtCompDevIdRspType
   @brief This is a response structure for query ACDB_CMD_GET_COMPATIBLE_REMOTE_DEVICE_ID.
   
   @param nRmtDeviceId: Device ID on remote proc.

   This is a query response structure for query ACDB_CMD_GET_COMPATIBLE_REMOTE_DEVICE_ID.
*/
typedef struct _AcdbGetRmtCompDevIdRspType {
   uint32_t nRmtDeviceId;
} AcdbGetRmtCompDevIdRspType;
/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_AUDIO_RECORD_RX_DEVICE_LIST Declarations and Documentation
 * --------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_AUDIO_RECORD_RX_DEVICE_LIST, ...)
   @brief API to query for a list of device pairs.

   This command will query for a list of valid RX device Ids with given TX device Id for audio record use case using EC.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AUDIO_RECORD_RX_DEVICE_LIST.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAudioRecRxListCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAudioRecRxListCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbAudioRecRxListRspType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbAudioRecRxListRspType).

   @see  acdb_ioctl
   @see  AcdbAudioRecRxListCmdType
   @see  AcdbAudioRecRxListRspType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
		 ACDB_PARMNOTFOUND: When the rx devices are not found for a give tx device
*/
#define ACDB_CMD_GET_AUDIO_RECORD_RX_DEVICE_LIST                   0x0001134D

/**
   @struct   AcdbAudioRecRxListCmdType
   @brief This is a query command structure for the list of Rx devices paired
          with the given TX device.

   @param nTxDeviceId: The TX Device ID

   This is a query command structure for the Rx devices list for a given TX device
*/
typedef struct _AcdbAudioRecRxListCmdType {
   uint32_t nTxDeviceId;
} AcdbAudioRecRxListCmdType;

/**
   @struct   AcdbAudioRecRxListRspType

   @brief This is a response command structure for the list of Rx devices paired
          with the given TX device.
   @param nNoOfRxDevs: The no of Rx devices paired with the given input Tx Device
					   If the given Tx device is not paired with any of the rx devcies
					   the nNoOfRxDevs value will be returned 0.
   @param pRxDevs:	   points to the array of Rx devices of type uint32_t each.

*/
typedef struct _AcdbAudioRecRxListRspType {
   uint32_t nNoOfRxDevs;
   uint32_t *pRxDevs;
} AcdbAudioRecRxListRspType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_ADIE_SIDETONE_TABLE
 *    Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_ADIE_SIDETONE_TABLE, ...)
   @brief API to query for an entire calibration table in the format explained below.

   This command will obtain the specific calibration data queried for AdieSidetone 
   Table

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_ADIE_SIDETONE_TABLE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAdieSidetoneTableCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAdieSidetoneTableCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbAdieSidetoneTableCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_ADIE_SIDETONE_TABLE    0x00012A0B

/**
   @struct   AcdbAdieSidetoneTableCmdType
   @brief This is a query command structure allowing for multiple get/set
          of calibration data in the adieSidetone data tables.

   @param nTxDeviceId: The tx Device ID
   @param nRxDeviceId: The rx Device ID
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the entire table.
   @param nBufferPointer: A virtual memory pointer pointing to the memory
                    region ACDB will copy the table into.

   This is a query command structure allowing for multiple get/set of
   calibration data in the adieSidetone data tables. The format of the table 
   will be the repetition of this structure:
   struct AdieSideToneEntry {
         uint32_t nModuleId;
         uint32_t nParamId;
         uint16_t nParamSize; //multiple of 4
         uint16_t nReserved; // Must be 0
         uint8 aParamData [multiple of 4)];
     }
*/
typedef struct _AcdbAdieSidetoneTableCmdType {
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbAdieSidetoneTableCmdType;


/* ---------------------------------------------------------------------------
 * Function Declarations and Documentation
 *--------------------------------------------------------------------------- */

/*===========================================================================

FUNCTION acdb_ioctl

DESCRIPTION
   Main entry function to the ACDB. This entry function will take any
   supported ACDB IOCTL and provide the appropriate response.

   @param[in] nCommandId
         Command id to execute on the Audio Calibration Database. The
         pCommandStruct and the pResponseStruct should match the expected
         structures for that command. If not, the command will fail.
   @param[in] pCommandStruct
         Pointer to the command structure.
   @param[in] nCommandStructLength
         The size of command structure.
   @param[out] pResponseStruct
         Pointer to the response structure.
   @param[in] nResponseStructLength
         The size of the response structure.

   Supported commands are:
      ACDB_CMD_INITIALIZE
      ACDB_CMD_GET_ACDB_VERSION
      ACDB_CMD_GET_TARGET_VERSION
      ACDB_CMD_RESET
      ACDB_CMD_ESTIMATE_MEMORY_USE
      ACDB_CMD_GET_DEVICE_CAPABILITIES
      ACDB_CMD_IS_DEVICE_PAIRED
      ACDB_CMD_GET_VOCPROC_COMMON_DATA
      ACDB_CMD_SET_VOCPROC_COMMON_DATA
      ACDB_CMD_GET_VOCPROC_COMMON_TABLE
      ACDB_CMD_SET_VOCPROC_COMMON_TABLE
      ACDB_CMD_GET_VOCPROC_STREAM_DATA
      ACDB_CMD_SET_VOCPROC_STREAM_DATA
      ACDB_CMD_GET_VOCPROC_STREAM_TABLE
      ACDB_CMD_SET_VOCPROC_STREAM_TABLE
      ACDB_CMD_GET_AUDPROC_COMMON_DATA
      ACDB_CMD_SET_AUDPROC_COMMON_DATA
      ACDB_CMD_GET_AUDPROC_COMMON_TABLE
      ACDB_CMD_SET_AUDPROC_COMMON_TABLE
      ACDB_CMD_GET_AUDPROC_STREAM_DATA
      ACDB_CMD_SET_AUDPROC_STREAM_DATA
      ACDB_CMD_GET_AUDPROC_STREAM_TABLE
      ACDB_CMD_SET_AUDPROC_STREAM_TABLE
      ACDB_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL
      ACDB_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL_STEP_DATA
      ACDB_CMD_SET_VOCPROC_GAIN_DEP_VOLTBL_STEP_DATA
      ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_POPP
      ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_COPP
      ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_DATA
      ACDB_CMD_SET_AUDPROC_GAIN_DEP_VOLTBL_STEP_DATA
      ACDB_CMD_GET_VOL_TABLE_STEP_SIZE
      ACDB_CMD_GET_ANC_TX_DEVICE
      ACDB_CMD_GET_ADIE_CODEC_PATH_PROFILE
      ACDB_CMD_SET_ADIE_CODEC_PATH_PROFILE
      ACDB_CMD_GET_ANC_SETTING
      ACDB_CMD_SET_ANC_SETTING
      ACDB_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID
      ACDB_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID
      ACDB_CMD_GET_AFE_DATA
      ACDB_CMD_SET_AFE_DATA
      ACDB_CMD_GET_GLBTBL_DATA
      ACDB_CMD_SET_GLBTBL_DATA
      ACDB_CMD_GET_COMMON_DEVICE_INFO
      ACDB_CMD_GET_TARGET_SPECIFIC_DEVICE_INFO
      ACDB_CMD_GET_VOCPROC_COMMON_TOPOLOGY_ID
      ACDB_CMD_GET_ADIE_SIDETONE_TABLE

   Please see individual command documentation for more details on the
   expectations of the command.

DEPENDENCIES
   None

RETURN VALUE
   The result of the call as defined by the command.

SIDE EFFECTS
   None.

===========================================================================*/
int32_t acdb_ioctl (uint32_t nCommandId,
                    const uint8_t *pCommandStruct,
                    uint32_t nCommandStructLength,
                    uint8_t *pResponseStruct,
                    uint32_t nResponseStructLength);

#endif /* __ACDB_API_H__ */


