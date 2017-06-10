/* @file SecureUILib.h
 * @brief
 * This file contains the interfaces to use the Secure UI Library
 */

/*===========================================================================
 * Copyright(c) 2013-2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

#pragma once

#include <QSEEComAPI.h>

#ifdef __cplusplus
  extern "C" {
#endif


  /**
   * @brief Get the shared memory buffer size
   *
   * This function will return the minimum size required for the shared memory buffer.
   * The returned size is the one required by the Secure UI Lib only, therefore the user
   * must set the shared buffer size to be at least as large as the returned value
   * (i.e. it can and should be larger if required by the specific app).
   *
   * @param buffer_size[out] - set to hold the required size (in bytes)
   *
   * @return 0 on success, errno otherwise.
   *
   * @note This function is not thread safe.
   */

int32_t GetSharedMemorySize(uint32_t* buffer_size);

  /**
   * @brief Get the secure indicator
   *
   * This function will retrieve the secure indicator and will save it
   * ready-to-use in TrustZone, so it is available for any eligible TUI app who
   * would like to display it.
   *
   * @param handle[in] - handle for the active connection with the secure application
   *
   * @return 0 on success, errno otherwise.
   *
   * @note This function is not thread safe.
   */

int32_t GetSecureIndicator(struct QSEECom_handle *handle);

/**
 * @brief Start the Secure Touch.
 *
 * This call will block until the Secure Touch session is concluded. If the
 * caller wants to abort the session asynchronously, it must pass a file
 * descriptor which will be polled for data availability: when data will be
 * seen available on this fd, this will be interpreted as an abort request.
 *
 * @param fd[in] - file descriptor that will be waited on to request the service
 *                 to be early aborted.
 * @param handle[in] - handle for the active connection with the secure application
 *
 * @return 0 on success, errno otherwise.
 *
 * @note This function is not thread safe.
 */

int32_t UseSecureTouch(int fd, struct QSEECom_handle *handle);

#ifdef __cplusplus
  }
#endif
