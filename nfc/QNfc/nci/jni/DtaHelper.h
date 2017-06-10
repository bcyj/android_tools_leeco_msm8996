/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

/**
 * A helper module that contains DTA-related variables and functions.
 */

#pragma once

#include <stdint.h>

namespace dta {

bool nfcdepListenLoopbackOn = false;

int responseWaitTime = 0;

// Just for testing, MUST BE FALSE for releases!
const bool simulateLowerTester = false;
// Just for testing, MUST BE FALSE for releases!
const bool echoPacketsBack2LT = false;

const uint8_t waitCommand[] = {
    0xFF, 0xFF, 0xFF, 0x01, 0x03
};

/*
const uint8_t waitCommand[] = {
    0x00, 0xFF, 0xFF, 0x01, 0x03
};*/


const uint8_t patternNumberCommand[] = {
    0xFF, 0x00, 0x00, 0x00
};

const UINT16 llcp_link_rwt[15] =  /* RWT = (302us)*2**WT; 302us = 256*16/fc; fc = 13.56MHz */
{
       1, /* WT=0,     302us */
       1, /* WT=1,     604us */
       2, /* WT=2,    1208us */
       3, /* WT=3,     2.4ms */
       5, /* WT=4,     4.8ms */
      10, /* WT=5,     9.7ms */
      20, /* WT=6,    19.3ms */
      39, /* WT=7,    38.7ms */
      78, /* WT=8,    77.3ms */
     155, /* WT=9,   154.6ms */
     310, /* WT=10,  309.2ms */
     619, /* WT=11,  618.5ms */
    1237, /* WT=12, 1237.0ms */
    2474, /* WT=13, 2474.0ms */
    4948, /* WT=14, 4948.0ms */
};

/**
 * Sets the response wait time.
 *
 * @param activated The activation struct.
 */
void setResponseWaitTime(UINT8 rwt) {
  responseWaitTime = llcp_link_rwt[ rwt ];
  ALOGD ("%s: [DTA] response wait time=%d", __FUNCTION__, responseWaitTime);
}

/**
 * Checks whether the specified buffer contains the WAIT command.
 *
 * @param buf The byte buffer.
 * @param bufLen The length of the buffer.
 *
 * @return true in the positive case, false otherwise.
 */
bool isWaitCommand(uint8_t* buf, uint32_t bufLen) {

  bool izWait = true;

  uint32_t i = 0;
  for ( ; i < sizeof(waitCommand) && i < bufLen; ++i) {
    if (buf[i] != waitCommand[i]) {
      izWait = false;
      break;
    }
  }

  if (i != sizeof(waitCommand)) {
    izWait = false;
  }

  if (izWait) {
    ALOGD ("%s: [DTA] [NFC-DEP LISTEN LB]: WAIT COMMAND DETECTED", __FUNCTION__);
  }

  return izWait;
}

/**
 * Checks whether the specified buffer contains the pattern number command.
 *
 * @param buf The byte buffer.
 * @param bufLen The length of the buffer.
 *
 * @return the pattern number or -1 if the pattern number is not found from the buffer.
 */
int checkPatternNumber(uint8_t* buf, uint32_t bufLen) {

  bool commandFound = true;

  uint32_t i = 0;
  for ( ; i < sizeof(patternNumberCommand) && i < bufLen; ++i) {
    if (buf[i] != patternNumberCommand[i]) {
      commandFound = false;
      break;
    }
  }

  if (i != sizeof(patternNumberCommand)) {
    commandFound = false;
  }

  if (commandFound) {
    ALOGD ("%s: [DTA] [NFC-DEP LISTEN LB]: PATTERN NUMBER COMMAND DETECTED", __FUNCTION__);

    if (i + 1 < bufLen) {
      int patternNumber = 0;
      patternNumber = 0xFF & buf[i];
      patternNumber <<= 8;
      patternNumber |= 0xFF & buf[++i];
      ALOGD ("%s: [DTA] [NFC-DEP LISTEN LB]: PATTERN NUMBER=%d", __FUNCTION__, patternNumber);
      return patternNumber;
    }

    ALOGE ("%s: [DTA] [NFC-DEP LISTEN LB]: PATTERN NUMBER buffer too short!", __FUNCTION__);
  }

  return -1;
}

/**
 * Handles the received application data while the DTA is in NFC-DEP loop-back listen mode.
 *
 * @param buf The byte buffer.
 * @param bufLen The length of the buffer.
 */
static SyncEvent waitEvent;
void nfcDepLoopBackInListenMode(uint8_t* buf, uint32_t bufLen) {

  uint16_t presence_chk_delay = 750;
  // Is wait command?
  ALOGD ("%s: [DTA] echoPacketsBack2LT =%d", __FUNCTION__, echoPacketsBack2LT);
  if (isWaitCommand(buf, bufLen)) {
    ALOGD ("%s: [DTA] response wait time=%d", __FUNCTION__, responseWaitTime);
    //SyncEvent waitEvent;
    SyncEventGuard g(waitEvent);
    waitEvent.wait (responseWaitTime);
    if (!echoPacketsBack2LT) {
      ALOGD ("%s: [DTA] echoPacketsBack2LT =%d", __FUNCTION__, echoPacketsBack2LT);
    }
  }

  int patternNumber = checkPatternNumber(buf, bufLen);
  if (patternNumber != -1) {
    nfcManager_dta_set_pattern_number(0, 0, patternNumber);
  }
  ALOGD ("%s: [DTA] NFA_SendRawFrame = %d", __FUNCTION__, presence_chk_delay);
  // Echo data back to LT:
  NFA_SendRawFrame(buf, bufLen, presence_chk_delay);
}

}
