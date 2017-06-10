/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.qti.services.secureui;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ActivityInfo;
import android.graphics.Point;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;
import android.view.Window;
import android.view.WindowManager;
import android.view.Display;
import android.view.Surface;

public class OrientationActivity extends Activity {
  public static final String TAG = "SecUIOrientation";
  public static final String ACTION_CLOSE = "com.qualcomm.qti.services.secureui.ACTION_CLOSE";
  private static boolean isRegistered = false;
  private CloseReceiver orientationCloseReceiver;
  private int sys_rotation;

  /** Called when activity is first created **/
  @Override
  public void onCreate(Bundle savedInstanceState) {
    Log.d(TAG, "onCreate+");
    super.onCreate(savedInstanceState);

    if (isRegistered == false) {
      Log.d(TAG, "Register broadcast receiver");
      IntentFilter filter = new IntentFilter();
      filter.addAction(ACTION_CLOSE);
      filter.addAction(Intent.ACTION_SCREEN_OFF);
      orientationCloseReceiver = new CloseReceiver();
      registerReceiver(orientationCloseReceiver, filter);
      isRegistered = true;
    }

    // Requesting to turn the title OFF
    requestWindowFeature(Window.FEATURE_NO_TITLE);
    // Making it full screen
    getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

    // Check whether liquid or fluid
    Display display = getWindowManager().getDefaultDisplay();
    Point size = new Point();
    display.getRealSize(size);
    int rotation = display.getRotation();
    Log.d(TAG, "Screen size: x=" + size.x + " y=" + size.y + ". Rotation: " + rotation);
    try {
      sys_rotation = Settings.System.getInt(getContentResolver(), Settings.System.ACCELEROMETER_ROTATION);
      Settings.System.putInt(getContentResolver(), Settings.System.ACCELEROMETER_ROTATION, 0);
    } catch (Settings.SettingNotFoundException ex) {
      Log.w(TAG, "Orientation setting not found");
    }
    if (size.x > size.y) {
      // landscape
      if (rotation == Surface.ROTATION_0) {
        // tablet landscape
        Log.w(TAG, "Tablet landscape");
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
      } else if (rotation == Surface.ROTATION_180) {
        // tablet reverse landscape
        // should set this as SCREEN_ORIENTATION_REVERSE_LANDSCAPE
        Log.w(TAG, "Tablet reverse landscape");
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
      } else {
        // phone
        Log.w(TAG, "Phone");
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
      }
    } else {
      // portrait
      if (rotation == Surface.ROTATION_90) {
        // tablet portrait
        // should set this as SCREEN_ORIENTATION_PORTRAIT
        Log.w(TAG, "Tablet portrait");
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
      } else if (rotation == Surface.ROTATION_270) {
        // tablet reverse portrait
        // should set this as SCREEN_ORIENTATION_REVERSE_PORTRAIT
        Log.w(TAG, "Tablet reverse portrait");
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
      } else {
        // phone
        Log.w(TAG, "Phone");
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
      }
    }
    rotation = display.getRotation();
    switch (rotation) {
      case Surface.ROTATION_0: SecureUIService.setRotation(0x00); break;
      case Surface.ROTATION_90: SecureUIService.setRotation(0x10); break;
      case Surface.ROTATION_180: SecureUIService.setRotation(0x20); break;
      case Surface.ROTATION_270: SecureUIService.setRotation(0x30); break;
    }
    Log.d(TAG, "Final rotation: " + rotation);

    Log.d(TAG, "onCreate-");
  }

  @Override
  public void onNewIntent(Intent intent) {
    super.onNewIntent(intent);
    if(isRegistered == false) {
      Log.d(TAG, "Register broadcast receiver");
      IntentFilter filter = new IntentFilter();
      filter.addAction(ACTION_CLOSE);
      filter.addAction(Intent.ACTION_SCREEN_OFF);
      orientationCloseReceiver = new CloseReceiver();
      registerReceiver(orientationCloseReceiver, filter);
      isRegistered = true;
    }
  }

  @Override
  public void onPause() {
    Log.d(TAG, "onPause+");
    super.onPause();
    if (isRegistered == true) {
      Log.d(TAG, "Unregister broadcast receiver");
      unregisterReceiver(orientationCloseReceiver);
      isRegistered = false;
    }
    this.finish();
    Log.d(TAG, "onPause-");
  }

  @Override
  public void onResume() {
    Log.d(TAG, "onResume+");
    super.onResume();
    if(isRegistered == false) {
      Log.d(TAG, "Register broadcast receiver");
      IntentFilter filter = new IntentFilter();
      filter.addAction(ACTION_CLOSE);
      filter.addAction(Intent.ACTION_SCREEN_OFF);
      orientationCloseReceiver = new CloseReceiver();
      registerReceiver(orientationCloseReceiver, filter);
      isRegistered = true;
    }
    Log.d(TAG, "onResume-");
  }

  @Override
  public void onDestroy() {
    Log.d(TAG, "onDestroy+");
    super.onDestroy();
    Log.d(TAG, "Restoring screen rotation");
    Settings.System.putInt(getContentResolver(), Settings.System.ACCELEROMETER_ROTATION, sys_rotation);
    setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
    Log.d(TAG, "onDestroy-");
  }

  class CloseReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
      if (intent.getAction().equals(ACTION_CLOSE) ||
            intent.getAction().equals(Intent.ACTION_SCREEN_OFF)) {
              Log.d(TAG, "Intent received = "+ intent.getAction());
        OrientationActivity.this.finish();
      }
    }
  }
}
