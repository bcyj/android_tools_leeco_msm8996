/*
 * ====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *
 * XTRA-T WiFi Replacement Network Location Provider
 *
 * GENERAL DESCRIPTION This is implementation of public status for NLP.
 *
 * Copyright (c) 2012-2013 Qualcomm Atheros, Inc. All Rights Reserved. Qualcomm
 * Atheros Confidential and Proprietary.
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * =============================================================================
 */
package com.qualcomm.location.xtwifi;

import android.location.LocationProvider;
import android.os.SystemClock;

public class NlpPublicStatus
{
  private int m_publicStatus;
  private long m_publicStatusUpdateTime;

  public NlpPublicStatus()
  {
    setStatus(LocationProvider.TEMPORARILY_UNAVAILABLE);
  }

  public synchronized void setStatus(int status)
  {
    m_publicStatus = status;
    m_publicStatusUpdateTime = SystemClock.elapsedRealtime();
  }

  public synchronized int getStatus()
  {
    return m_publicStatus;
  }

  public synchronized long getStatusUpdateTime()
  {
    return m_publicStatusUpdateTime;
  }
}
