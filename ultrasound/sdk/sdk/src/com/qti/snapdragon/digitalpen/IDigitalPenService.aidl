/*===========================================================================
                           IDigitalPenService

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.

Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
package com.qti.snapdragon.digitalpen;

import android.os.Bundle;
import com.qti.snapdragon.digitalpen.util.DigitalPenConfig;
import com.qti.snapdragon.digitalpen.IDigitalPenDataCallback;
import com.qti.snapdragon.digitalpen.IDigitalPenEventCallback;
import codeaurora.ultrasound.IDigitalPenDimensionsCallback;

/**
 * IDigitalPenService interface:
 *   This interface defines functions to communicate with the Digital Pen
 *   service.
 *
 * @hide
 */
interface IDigitalPenService {
  /*
   * Register to notifications on off-screen area dimension changes
   */
  void registerOffScreenDimensionsCallback(IDigitalPenDimensionsCallback callback);
  /*
   * OEM INTERFACE METHODS
   */
  /*
   * Enable the Digital Pen feature
   */
  boolean enable();
  /*
   * Disable the Digital Pen feature
   */
  boolean disable();
  /*
   * Checks whether the Digital Pen feature is enabled or not
   */
  boolean isEnabled();
  /*
   * Set the global configuration for the digital pen
   */
  boolean setGlobalConfig(in DigitalPenConfig globalConfig);
  /*
   * Get the configuration for the digital pen
   */
  DigitalPenConfig getConfig();
  /*
   * Registers a callback function for data
   */
  boolean registerDataCallback(IDigitalPenDataCallback cb);
  /*
   * Registers a callback function for event
   */
  boolean registerEventCallback(IDigitalPenEventCallback cb);
  /*
   * Unregisters a registered callback function for data
   */
  boolean unregisterDataCallback(IDigitalPenDataCallback cb);
  /*
   * Unregisters a registered callback function for event
   */
  boolean unregisterEventCallback(IDigitalPenEventCallback cb);

  /*
   * APPLICATION INTERFACE METHODS
   */
  /*
   * Applies the given application settings
   */
  boolean applyAppSettings(in Bundle settings);
  /**
   * Releases the current activity configuration.
   * The digital pen configuration will revert to the system default.
   */
  boolean releaseActivity();
}
