#ifndef SECURE_INDICATOR_API_H
#define SECURE_INDICATOR_API_H

/** @file SecureIndicatorAPI.h
 * @brief
 * This file contains the definitions of the constants, data structures
 * and interfaces to the Secure Indicator Module
 */

/*===========================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

					EDIT HISTORY FOR FILE

when       who     what, where, why
--------   ---     ----------------------------------------------------------
08/14/14   rz      Added GetDimensions API + updated error codes
05/20/14   rz      Initial Version

===========================================================================*/

/*----------------------------------------------------------------------------
 *Type Declarations
 *--------------------------------------------------------------------------*/

#include "comdef.h"

/* Error codes */
typedef enum {
    TUI_SI_SUCCESS                        = 0,
    TUI_SI_ERROR                          = -1,
    TUI_SI_INIT_ERR                       = -2,
    TUI_SI_TEARDOWN_ERR                   = -3,
    TUI_SI_QSEE_APP_NOT_INITIALIZED       = -4,
    TUI_SI_INVALID_INPUT_ERR              = -5,
    TUI_SI_IS_PROVISIONED_ERR             = -6,
    TUI_SI_STORE_INDICATOR_ERR            = -7,
    TUI_SI_INVALID_PNG_ERR                = -8,
    TUI_SI_FS_FILE_NOT_REMOVED_ERR        = -9,
    TUI_SI_PROVIDE_INDICATOR_ERR          = -10,
    TUI_SI_INDICATOR_NOT_PROVISIONED_ERR  = -11,
    TUI_SI_APP_NOT_APPROVED_ERR           = -12,
    TUI_SI_GET_DIMENSIONS_ERR             = -13,
    TUI_SI_REMOVE_INDICATOR_ERR           = -14,
    TUI_SI_ERR_SIZE                       = 0x7FFFFFFF
} tui_si_status_t;

#define MAX_IMAGE_SIZE              250*1024  //250KB

/*----------------------------------------------------------------------------
 *Function Declarations and Documentation
 *--------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/**
  @brief Initialize the Secure Indicator Module
  The function loads the Secure Indicator module into TrustZone

  @return
  0        - Success
  non 0    - Failure

  @dependencies
  Must be called prior to invoking any of the other Secure Indicator APIs

  @side effects
  None
*/
tui_si_status_t SecureIndicator_Init();


/**
  @brief Terminate the Secure Indicator Module
  The function terminates the operation of the Secure Indicator module in TrustZone

  @return
  0        - Success
  non 0    - Failure

  @dependencies
  None

  @side effects
  None
*/
tui_si_status_t SecureIndicator_Teardown();

/**
  @brief Check if the secure indicator is already provisioned

  @param[out] result  - set to true/false

  @return
  0        - Success
  non 0    - Failure

  @dependencies
  None

  @side effects
  None
*/
tui_si_status_t SecureIndicator_IsIndicatorProvisioned(boolean* result);


/**
  @brief Store the secure indicator
  The function copies the image from the given FS path and saves it securely in TrustZone.
  After the image was successfully saved into TrustZone, it will be removed from the FS.
  The size of the image must not exceed MAX_IMAGE_SIZE.

  @param[in] imagePath  - path to the image that will be stored as the secure indicator

  @return
  0        - Success
  non 0    - Failure

  @dependencies
  None

  @side effects
  On success, image will be deleted from the FS
*/
tui_si_status_t SecureIndicator_StoreSecureIndicator(const char* imagePath);

/**
  @brief Provide the secure indicator dimensions
  The function provides the max allowed dimensions (in pixels) for the secure indicator.
  The returned parameters are the max total size in pixels, and the max width in pixels.

  @param[out] maxSizeInPixels  - set to hold the max size in pixels
  @param[out] maxWidthInPixels  - set to hold the max width in pixels

  @return
  0        - Success
  non 0    - Failure

  @dependencies
  None

  @side effects
  None
*/
tui_si_status_t SecureIndicator_GetSecureIndicatorDimensions(uint32* maxSizeInPixels, uint32* maxWidthInPixels);

/**
@brief Remove the secure indicator from TrustZone

@return
0        - Success
non 0    - Failure

@dependencies
None

@side effects
On success, image will be deleted from TrustZone
*/
tui_si_status_t SecureIndicator_RemoveIndicator();

#ifdef __cplusplus
}
#endif

#endif /* SECURE_INDICATOR_API_H */
