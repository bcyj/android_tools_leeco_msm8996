#ifndef __P2P_EXPORTS_H__
#define __P2P_EXPORTS_H__

/*============================================================================
                           P2PExports.h

DESCRIPTION:  Function definitions for the P2P lib (libqcp2p).

Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------------*/
#define STUB_VERSION "0.0.0.0"

/**
  Define the max number of TX channels supported by the library.
*/
#define QC_US_P2P_MAX_TX_CHANNELS       4

/**
 Maximum number of users that the P2P library can track
 */
#define QC_US_P2P_MAX_USERS             8

/**
 * Distance value meaning "user not present"
 */
#define QC_US_P2P_DISTANCE_NOT_PRESENT  -1

/**
  P2P library return status
*/
  typedef enum
  {
    QC_US_P2P_LIB_STATUS_SUCCESS     =  0,   /* Success */
    QC_US_P2P_LIB_STATUS_FAILURE     =  1,   /* General failure */
    QC_US_P2P_LIB_STATUS_BAD_PARAMS  =  2,   /* Bad parameters */
  } QcUsP2PLibStatusType;

// P2P Application type
typedef enum
{
  QC_US_P2P_APP_POS = 0,             // Presence and Positioning
  QC_US_P2P_APP_DATA_TRANSFER = 1,   // Data Transfer
} QcUsP2PAppType;

// P2P Positioning algorithm type
typedef enum
{
  QC_US_P2P_POS_ALG_FDMA = 0,  // FDMA
  QC_US_P2P_POS_ALG_CDMA = 1,  // CDMA
  QC_US_P2P_POS_ALG_TDMA = 2,  // TDMA
} QcUsP2PPosAlgorithmType;

// P2P Data algorithm type
typedef enum
{
  QC_US_P2P_DATA_ALG_GENERAL = 0,  // Currently there is a single algorithm for data transfer
} QcUsP2PDataAlgorithmType;

// P2P library mode
typedef enum
{
  QC_US_P2P_LIB_MODE_NORMAL = 0,  // Normal mode
  QC_US_P2P_LIB_MODE_SOCKET = 1,  // Send samples to socket for MATLAB real-time
} QcUsP2PLibraryMode;

// P2P Rx Pattern mode
typedef enum
{
  QC_US_P2P_RX_PATTERN_MODE_NORMAL = 0,  // Normal mode - Rx Pattern generated according to configuration
  QC_US_P2P_RX_PATTERN_MODE_FILE = 1,    // Rx Pattern read from file (to be used for testing purposes)
} QcUsP2PRxPatternMode;

//  Configurations for the P2P algorithm.
typedef struct
{
  // number of TX channels (microphones). Should be the same as usf_tx_port_count
    // parameter.
  uint16_t  numChannels;

    // x-position of microphones (only first numChannels entries are valid)
    // in 1/10th mm relative to top upper left corner of phone
    int32_t    micPositionsX[QC_US_P2P_MAX_TX_CHANNELS];

    // y-position of microphones (only first numChannels entries are valid)
    // in 1/10th mm relative to top upper left corner of phone
    int32_t    micPositionsY[QC_US_P2P_MAX_TX_CHANNELS];

    // z-position of microphones (only first numChannels entries are valid)
    // in 1/10th mm relative to top upper left corner of phone
    int32_t    micPositionsZ[QC_US_P2P_MAX_TX_CHANNELS];

  // P2P application type
  uint16_t  appType;

  // P2P Positioning/Presence algorithm type
  uint16_t  posAlgType;

  // P2P Data Transfer algorithm type
  uint16_t  dataAlgType;

  // Number of users to be tracked for presence/position
  uint16_t    numUsers;

  // user index that this device should use, or -1 for dynamic user ID selection
  int16_t   userIdx;

  // number of samples per single channel of audio TX data
  // this determines the amount of samples the library expects to get in each
  // frame (service will deliver samplesPerFrame * numChannels samples)
  uint32_t  samplesPerFrame;

    // Peak-To-Average ratio (linear) in the correlation, above which presence is declared
  uint16_t  p2aThreshold;

  // LOS (Line Of Sight) window length to search for the LOS peak left of the maximal correlation peak
  // Given as a ratio relative to the frame length (which is also the correlation length)
  // The LOS Window length in samples is given by losWindowLenRatio/65536 * samplesPerFrame
  uint16_t  losWindowLenRatio;

  // Peak thereshold above which a correlation peak (within the LOS window) is considered a candidate for LOS peak
  // Given as a ratio relative to the maximal correlation peak value (CorrMaxVal)
  // A correlation peak value CorrPeakVal is considered a candidate for LOS peak if CorrPeakVal^2 > losPeakThreasholdRatio/65536 * CorrMaxVal^2
  uint16_t  losPeakThreasholdRatio;
  // define sequence index for each user (used internally by algorithm)
  uint16_t  sequenceIdx[QC_US_P2P_MAX_USERS];

  // first non-zero bin (signal spectrum control)
  uint16_t firstBin;

  // last non-zero bin (signal spectrum control)
  uint16_t lastBin;

  // Resampler frequency in PPM
  int32_t  resamplerFreqPpm;

  // FFT size for data transfer
  uint32_t fftSize;

  // Library mode
  uint16_t  libraryMode;

  // Rx Pattern mode
  uint16_t  rxPatternMode;

} P2PCfg;

/**
 Definition of the data that we send in TX configuration, using
 the transparent data part of the configuration structure. We
 need to know the complete transparent data because we allow
 user to override most configuration parameters with values from
 p2p config file IMPORTANT: the definition of this structure
 must be in sync with the same definition in LPASS headers
 */
typedef struct
{
    // skip factor. Number of frames to skip when delivering TX data
    // skip (value - 1) frames (1=no skip, 2=skip 1 frame after each frame,
    // 3=skip 2 frames after each frame, ...)
    // Currently only skip=1 is supported by LPASS, this parameter is provided
    // for compatibility with other calculators.
    uint16_t    skipFactor;

    // grouping factor. The number of frames to group and deliver together
    // in each call to ual_read.
    // Currently only group factor=1 is supported by LPASS, this parameter
    // is provided for compatibility with other calculators.
    uint16_t    groupingFactor;

    // output type, determines the type of output coming from LPASS
    // (events only, raw data or both)
    uint16_t    outputType;

    // the rest of the configuration is the same as the P2PCfg structure
    // that we pass to the library
    P2PCfg      libConfig;
} P2PExternalCfg;

/**
 Result for a single user
 */
typedef struct
{
  int16_t angle;
  int16_t distance;  // -1 denotes not present
} P2PUserStatus;

/**
 Result of frame processing
 */
typedef enum
{
  QC_US_P2P_RESULT_IDLE             =  0,   /* No status for frame */
  QC_US_P2P_RESULT_STATUS_UPDATED   =  1,   /* Status available for frame */
} QcUsP2PResult;

/**
 Request from library to service
 */
typedef enum
{
  QC_US_P2P_REQUEST_NONE                =  0,   /* No request */
  QC_US_P2P_REQUEST_UPDATE_RX           =  1,   /* Update Rx pattern but don't start RX */
  QC_US_P2P_REQUEST_START_RX            =  2,   /* Start RX */
  QC_US_P2P_REQUEST_STOP_RX             =  3,   /* Stop RX */
  QC_US_P2P_REQUEST_UPDATE_AND_START_RX =  4,   /* Update Rx pattern and start RX */
} QcUsP2PRequest;

/**
 The output of the P2P algorithm.
 This structure is reported with any call to P2P library
 */
typedef struct
{
    // result of frame processing (QcUsP2PResult)
    uint16_t result;

    // request from library to P2P service (QcUsP2PRequest)
    uint16_t request;

    // presence status for each user
  P2PUserStatus usersStatus[QC_US_P2P_MAX_USERS];
} P2POutput;

/*----------------------------------------------------------------------------
  Function Declarations
----------------------------------------------------------------------------*/

/*==============================================================================
  FUNCTION:  QcUsP2PLibGetSizes
===============================================================================*/
/*
 * Returns the sizes of buffers required by the P2P algorithm.
 *
 * @param pConfig[in]              The configuration of the algorithm to be used.
 * @param pWorkspaceSize[out]      The memory required by P2P algorithm in bytes.
 * @param pPatternSizeSamples[out] The pattern size to be used in sampels (16 bit).
 *
 * @return 0 for success, 1 for failure.
 */
int QcUsP2PLibGetSizes(P2PCfg const *pConfig,
                       uint32_t *pWorkspaceSize,
                       uint32_t *pPatternSizeSamples);

/*==============================================================================
  FUNCTION:  QcUsP2PLibInit
==============================================================================*/
/*
 * Initializes the P2P algorithm.
 *
 * @param pConfig[in]       The configuration of the algorithm.
 * @param pWorkspace[in]    Scratch buffer to be used by the algorithm.
 * @param workspaceSize[in] Size of the scratch buffer.
 *
 * @return 0 for success, 1 for failure.
 */
int QcUsP2PLibInit(P2PCfg const *pConfig,
                   int8_t *pWorkspace,
                   uint32_t workspaceSize);

/*==============================================================================
  FUNCTION:  QcUsP2PLibGetPattern
==============================================================================*/
/*
 * Returns the pattern that would be used by the algorithm.
 *
 * @param pPattern[in]   Buffer to which the pattern will be written.
 * @param patternSize[in] Size of given buffer in samples (16 bit).
 *
 * @return 0 for success, 1 for failure.
 */
int QcUsP2PLibGetPattern(int16_t *pPattern,
                         uint32_t patternSize);

/*==============================================================================
  FUNCTION:  QcUsP2PLibEngine
==============================================================================*/
/*
 * P2P algorithm to be called for each US frame.
 *
 * @param pSamplesBuffer[in] The buffer of samples to be used by the algorithm
 * @param pP2P[out]      The algorithm's decision
 *
 * @return 0 for success, 1 for failure.
 */
int QcUsP2PLibEngine(int16_t const *pSamplesBuffer,
                     P2POutput *pP2P);

#ifdef __cplusplus
}
#endif

#endif // P2P_EXPORTS_H
