/*============================================================================
@file QSensorEventListener.java

@brief
Call-back API for threshold JNI.

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qualcomm.sensors.qsensortest;

import android.annotation.SuppressLint;
import android.hardware.SensorEvent;
import android.hardware.TriggerEvent;
import android.hardware.TriggerEventListener;

@SuppressLint("NewApi")
public class QSensorEventListener extends TriggerEventListener {
   public void onSensorChanged(SensorEvent event) {
   }

   @Override
   public void onTrigger(TriggerEvent event) {}
}
