/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.services.secureui;

import com.qualcomm.wfd.service.ISessionManagerService;
import com.qualcomm.wfd.service.IWfdActionListener;
import com.qualcomm.wfd.WfdEnums;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

/**
 * Receiver for WFD intents.
 * It relies on the WFDListener to interact with the WFD Service.
 * WFDListener is created at runtime when WFD is enabled.
 * */
public class WfdReceiver extends BroadcastReceiver {
  private static final String TAG = "SecUISvc.WfdReceiver";
  private static boolean mServiceBound = false;
  private WfdListener wfdListener = null;

  WfdReceiver(boolean wfdActive) {
    super();
    Log.d(TAG, "WfdReceiver(), wfdActive=" + wfdActive);
    if (wfdActive) {
      Log.d(TAG, "Creating service, binding to WFD");
      Intent serviceIntent = new Intent("com.qualcomm.wfd.service.WfdService");
      PackageManager pm = SecureUIService.context.getPackageManager();
      ResolveInfo resolveInfo = pm.resolveService(serviceIntent, 0);
      if (resolveInfo == null) {
        Log.e(TAG, "Failed to locate WFD service");
        return;
      }
      ComponentName name = new ComponentName(resolveInfo.serviceInfo.applicationInfo.packageName, resolveInfo.serviceInfo.name);
      Log.d(TAG, "service: " + name);
      serviceIntent.setComponent(name);
      if (!SecureUIService.context.bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE)) {
        Log.e(TAG,"Failed to connect to Provider service");
      } else {
        Log.d(TAG, "WfdService binding has started");
      }
    }
  }

  public boolean pause() {
    Log.d(TAG, "pause");
    if (wfdListener != null) {
      Log.d(TAG, "We have a listener, forwarding");
      return wfdListener.pause();
    }
    return true;
  }

  public void resume() {
    Log.d(TAG, "resume");
    if (wfdListener != null) {
      wfdListener.resume();
    }
  }

  @Override
  public void onReceive(Context context, Intent intent) {
    Log.d(TAG, "onReceive intent: " + intent.toUri(0));
    String action = intent.getAction();
    if (action.equals(WfdEnums.ACTION_WIFI_DISPLAY_ENABLED) && !mServiceBound) {
      Log.d(TAG, "Resolve intent");
      Intent serviceIntent = new Intent("com.qualcomm.wfd.service.WfdService");
      PackageManager pm = SecureUIService.context.getPackageManager();
      ResolveInfo resolveInfo = pm.resolveService(serviceIntent, 0);
      if (resolveInfo == null) {
        Log.e(TAG, "Failed to locate WFD service");
        return;
      }
      ComponentName name = new ComponentName(resolveInfo.serviceInfo.applicationInfo.packageName, resolveInfo.serviceInfo.name);
      Log.d(TAG, "service: " + name);
      serviceIntent.setComponent(name);
      Log.d(TAG, "Creating service, binding to WFD");
      if (!context.bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE)) {
        Log.e(TAG,"Failed to connect to Provider service");
      } else {
        Log.d(TAG, "WfdService binding has started");
      }
    } else if (action.equals(WfdEnums.ACTION_WIFI_DISPLAY_DISABLED) && mServiceBound) {
        context.unbindService(mConnection);
        Log.d(TAG, "WfdService has stopped");
        mServiceBound = false;
        wfdListener = null;
    }
  }

  protected ServiceConnection mConnection = new ServiceConnection() {
    public void onServiceConnected(ComponentName className, IBinder service) {
      Log.d(TAG, "Connection object created");
      if (mServiceBound)
        return;
      ISessionManagerService wfdService = ISessionManagerService.Stub.asInterface(service);
      wfdListener = new WfdListener(wfdService);
      try {
        wfdService.init(wfdListener, null);
        mServiceBound = true;
      } catch (RemoteException e) {
        Log.e(TAG, "Remote exception", e);
      }
    }

    public void onServiceDisconnected(ComponentName className) {
      Log.d(TAG, "Remote service disconnected");
      mServiceBound = false;
      wfdListener = null;
    }
  };
}
