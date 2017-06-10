/*============================================================================
@file ThresholdInstanceID.java

@brief
Value-type class to store the instace ID returned from threshold algorithm.

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qualcomm.sensors.sensortest;

public class ThresholdInstanceID {
   private int instanceID;
   public ThresholdInstanceID(int instanceID) {
      this.instanceID = instanceID;
   }
   public int instanceID() { return this.instanceID; }
}
