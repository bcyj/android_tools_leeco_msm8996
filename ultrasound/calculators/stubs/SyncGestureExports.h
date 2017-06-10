#ifndef __SYNC_GESTURE_EXPORTS_H__
#define __SYNC_GESTURE_EXPORTS_H__

/*============================================================================
                           SyncGestureExports.h

DESCRIPTION:  Function definitions for the Sync Gesture lib (libqcsyncgesture).

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
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
  Gesture library return status
*/
typedef enum
{
  QC_US_GESTURE_LIB_STATUS_SUCCESS     =  0,   /* Success */
  QC_US_GESTURE_LIB_STATUS_FAILURE     =  1,   /* General failure */
  QC_US_GESTURE_LIB_STATUS_BAD_PARAMS  =  2,   /* Bad parameters */
} QcUsGestureLibStatusType;

/**
  Gesture library outcome type
*/
typedef enum
{
  QC_US_GESTURE_LIB_RESULT_NOT_READY =  -1,  /* Not ready */
  QC_US_GESTURE_LIB_RESULT_IDLE      =  0,   /* No gesture detected */
  QC_US_GESTURE_LIB_RESULT_SELECT    =  1,   /* Select detected */
  QC_US_GESTURE_LIB_RESULT_LEFT      =  2,   /* Left detected */
  QC_US_GESTURE_LIB_RESULT_RIGHT     =  3,   /* Right detected */
  QC_US_GESTURE_LIB_RESULT_UP        =  4,   /* Up detected */
  QC_US_GESTURE_LIB_RESULT_DOWN      =  5,   /* Down detected */
  QC_US_GESTURE_LIB_RESULT_REPEAT    =  6,   /* Please repeat */
} QcUsGestureLibResultType;

/**
  Configurations for the gesture algorithm.
*/
typedef struct
{
  uint32_t TxTransparentDataSize;
  char TxTransparentData[];
} GestureCfg;

/**
  The output of the gesture algorithm.
*/
typedef struct
{
  QcUsGestureLibResultType gesture;
  int velocity;
} GestureOutput;

/*----------------------------------------------------------------------------
  Function Declarations
----------------------------------------------------------------------------*/
/*==============================================================================
  FUNCTION:  QcUsGestureLibGetSizes
==============================================================================*/
/**
 * Returns the sizes of buffers required by the gesture algorithm.
 *
 * @param pConfig[in]              The configuration of the algorithm to be used.
 * @param pWorkspaceSize[out]      The memory required by gesture algorithm in bytes.
 * @param pPatternSizeSamples[out] The pattern size to be used in samples (16 bit).
 *
 * @return 0 for success, 1 for failure.
 */
extern int QcUsGestureLibGetSizes(GestureCfg const *pConfig,
                                  uint32_t *pWorkspaceSize,
                                  uint32_t *pPatternSizeSamples);

/*==============================================================================
  FUNCTION:  QcUsGestureLibInit
==============================================================================*/
/**
 * Initializes the gesture algorithm.
 *
 * @param pConfig[in]       The configuration of the algorithm.
 * @param pWorkspace[in]    Scratch buffer to be used by the algorithm.
 * @param workspaceSize[in] Size of the scratch buffer.
 *
 * @return 0 for success, 1 for failure.
 */
extern int QcUsGestureLibInit(GestureCfg const *pConfig,
                              int8_t *pWorkspace,
                              uint32_t workspaceSize);

/*==============================================================================
  FUNCTION:  QcUsGestureLibGetPattern
==============================================================================*/
/**
 * Returns the pattern that would be used by the algorithm.
 *
 * @param pPattern[in]   Buffer to which the pattern will be written.
 * @param patternSize[in] Size of given buffer in samples (16 bit).
 *
 * @return 0 for success, 1 for failure.
 */
extern int QcUsGestureLibGetPattern(int16_t *pPattern,
                                    uint32_t patternSize);

/*==============================================================================
  FUNCTION:  QcUsGestureLibEngine
==============================================================================*/
/**
 * Gesture algorithm to be called for each US frame.
 *
 * @param pSamplesBuffer[in] The buffer of samples to be used by the algorithm
 * @param pGesture[out]      The algorithm's decision
 *
 * @return 0 for success, 1 for failure.
 */
extern int QcUsGestureLibEngine(int16_t const *pSamplesBuffer,
                                GestureOutput *pGesture);

/*==============================================================================
  FUNCTION:  QcUsGestureLibIsBusy
==============================================================================*/
/**
 * Check if the library is currently busy processing frames.
 * The LPASS gesture sub-service uses this as a hint to stop giving the library
 * buffers to prcess while it is busy. That way the library will have much less
 * "stale" buffers in the queue when it exits the busy loop.
 * It is safe to call this function from any thread, but only call it after library is
 * initialized.
 *
 * @param pIsBusy [out] Fill with busy flag, 0=not busy 1=busy
 *
 * @return 0 for success, 1 for failure.
 */
extern int QcUsGestureLibIsBusy(int* pIsBusy);

/*==============================================================================
  FUNCTION:  QcUsGestureLibSetDynamicConfig
==============================================================================*/
/**
 * Sets dynamic configuration to the gesture library according to the
 * application configuration.
 *
 * @param pDynamicConfig [in] config buffer
 * @param dynamicConfigSize [in] config buffer length in integers
 *
 * @return 0 for success, 1 for failure.
 */
extern int QcUsGestureLibSetDynamicConfig(int* pDynamicConfig,
                                          uint32_t dynamicConfigSize);

#ifdef __cplusplus
}
#endif

#endif //__SYNC_GESTURE_EXPORTS_H__
