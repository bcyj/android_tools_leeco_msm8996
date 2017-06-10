/*
 * Copyright(c) 2013 Qualcomm Technologies, Inc.All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
  extern "C" {
#endif

/**
 * Start a Secure Touch session with the TouchScreen driver.
 * This function MUST be called BEFORE any resource locking has taken place in
 * TrustZone. Failing to do so will cause device instability.
 *
 *
 * @returns 0     on success.
 * @returns EBUSY if the Secure Touch is already enabled
 * @returns EIO   in case of errors related to power management.
 *
 */
int32_t stStartSession(void);

/**
 * Terminate a Secure Touch session.
 * This function MUST be called AFTER any resource locked in TrustZone has been
 * released. After this call the TouchScreen driver is free to process any
 * further touch event internally. No more events will be notified to the
 * callback passed in stStartSession.
 *
 * @param[in] force - 1 to force termination, even if not controlled by current
 *                    application.
 *
 * @returns 0      on success.
 * @returns ENODEV if Secure Touch is not enabled.
 */
int32_t stTerminateSession(uint32_t force);

/**
 * Wait for the next event to be available from the Touch Screen Driver. A return
 * value of EINVAL has to be interpreted as a request from the driver to terminate
 * the Secure Touch session, to allow the driver to process incumbent power
 * management events.
 *
 * @param[in] abortFD - File descriptor polled for early abort requests.
 * @param[in] timeout - Timeout in ms. Negative for infinite, 0 for no timeout
 *
 * @returns 0      on success.
 * @returns EBADF  if Secure Touch is not enabled
 * @returns EINVAL if the driver needs the Secure Touch service to be turned off
 *                 to process power management events.
 * @returns ETIMEDOUT if the call has timed out
 * @returns ECONNABORTED If the caller has requested an early abort
 */
int32_t stWaitForEvent(int32_t abortFd, int32_t timeout);

#ifdef __cplusplus
  }
#endif
