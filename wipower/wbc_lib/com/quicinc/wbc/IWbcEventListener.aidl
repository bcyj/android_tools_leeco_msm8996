/*=========================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  =========================================================================*/

package com.quicinc.wbc;

oneway interface IWbcEventListener {
    void onWbcEventUpdate(int what, int arg1, int arg2);
}
