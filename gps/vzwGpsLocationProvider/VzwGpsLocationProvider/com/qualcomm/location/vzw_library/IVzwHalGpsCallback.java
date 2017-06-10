/******************************************************************************
  @file    IVzwHalGpsCallback.java
  @brief   call back interface for VZW GPS Location Provider

  DESCRIPTION

  clients of VZW GPS Location Provider must implement this interface to receive callbacks

  ---------------------------------------------------------------------------
  Copyright (C) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.location.vzw_library;

public interface IVzwHalGpsCallback {
    public void ReportLocation(VzwHalLocation location);

    public void ReportSvStatus(VzwHalSvInfo svSvInfo);

    public void ReportGpsStatus(int statusCode);

    public void ReportEngineStatus(int statusCode);
}
