/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/**
 * This header contains only the DTA variables.
 */

#ifndef DTA_FLAG_H
#define DTA_FLAG_H

/**
 * This value should only be read in libnfc-nci. NativeNfcManager sets this
 * through NFA API
 */
extern int dta_Pattern_Number;

/**
 * Name of pattern file in data partition
 */
#define PATTERN_FILE_NAME "/data/nfc/pattern"

/**
 * log10(fabs(LONG_MAX)) + 1 + 1 <= 15
 */
#define MAX_PATTERN_LENGTH 15

/**
 * Checks whether the DTA mode is enabled.
 *
 * @return true if the DTA mode is enabled, false otherwise.
 */
static BOOLEAN in_dta_mode() {
  return dta_Pattern_Number >= 0;
}

static BOOLEAN in_llcp_or_snep_dta_mode() {
  /**
   * NOTE: 0xFFFF pattern is not defined by NFC Forum. It is a custom pattern
   * number indicating that the stack should be configured for LLCP or SNEP testing
   * 2.1 specification of DTA defines pattern numbers for LLCP testing, but they
   * are not supported as this implementation conforms only to DTA spec v2.0
   */
  return dta_Pattern_Number >= 4608;
}
static BOOLEAN in_llcp_snl() {
  return dta_Pattern_Number == 4736;
}
#endif // DTA_FLAG_H
