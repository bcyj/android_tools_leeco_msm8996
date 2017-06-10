/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.services.secureui;

import com.qualcomm.wfd.service.ISessionManagerService;
import com.qualcomm.wfd.WfdEnums.SessionState;
import com.qualcomm.wfd.WfdEnums;
import com.qualcomm.wfd.service.IWfdActionListener;

import android.app.KeyguardManager;
import android.content.Intent;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.RemoteException;
import android.util.Log;

/**
 * Listener for WFD Service notifications.
 * Issue the pause and resume commands, and wait for the pause to have taken
 * effect before allowing the caller to proceed.
 * */
public class WfdListener extends IWfdActionListener.Stub {

  public static final String TAG = "SecUISvc.WfdListener";

  private ISessionManagerService mService;
  private boolean paused = false;
  private boolean pausing = false;
  private final Object LOCK = new Object();

  private native int pause(int doPause);

  WfdListener(ISessionManagerService service) {
    super();
    mService = service;
  }

  public boolean wfdActive() {
    int state = SessionState.INVALID.ordinal();
    Log.d(TAG, "wfdActive: " + state);
    try {
      state = mService.getStatus().state;
    } catch (RemoteException e) {
      Log.e(TAG, "Remote exception", e);
    }
    Log.d(TAG, "WFD state: " + state);
    return (state == SessionState.PLAY.ordinal());
  }

  public boolean pause() {
    Log.d(TAG, "pause");
    boolean bRet = true;
    if (wfdActive()) {
      do {
        pausing = true;
        pause(1);
        try {
          // allow the blank frame to be ready
          Thread.sleep(1000);
          mService.pause();
          synchronized(LOCK) {
            LOCK.wait();
          }
        } catch (InterruptedException e) {
          Log.e(TAG, "InterruptedException", e);
          bRet = false;
          break;
        } catch (RemoteException e) {
          Log.e(TAG, "Remote exception", e);
          bRet = false;
          break;
        }

        paused = true;
      } while (false);
    }
    return bRet;
  }

  public void resume() {
    if (paused) {
      try {
        if (mService.getStatus().state == SessionState.PAUSE.ordinal()) {
          // we don't resume unless the state has changed meanwhile
          pause(0);
          mService.play();
        }
        paused = false;
      } catch (RemoteException e) {
        Log.e(TAG, "Remote exception", e);
      }
    }
  }


  @Override
  public void onStateUpdate(int newState, int sessionId) throws RemoteException {
    WfdEnums.SessionState state = WfdEnums.SessionState.values()[newState];
    switch (state) {
      case INITIALIZED:
        Log.d(TAG, "WfdEnums.SessionState==INITIALIZED");
        break;
      case INVALID:
        Log.d(TAG, "WfdEnums.SessionState==INVALID");
        break;
      case IDLE:
        Log.d(TAG, "WfdEnums.SessionState==IDLE");
        break;
      case PLAY:
        Log.d(TAG, "WfdEnums.SessionState==PLAY");
        break;
      case PAUSE:
        Log.d(TAG, "WfdEnums.SessionState==PAUSE");
        if (pausing) {
          synchronized(LOCK) {
            LOCK.notifyAll();
          }
          pausing = false;
        }
        break;
      case STANDBY:
        Log.d(TAG, "WfdEnums.SessionState = STANDBY");
        break;
      case ESTABLISHED:
        Log.d(TAG, "WfdEnums.SessionState==ESTABLISHED");
        break;
      case TEARDOWN:
        Log.d(TAG, "WfdEnums.SessionState==TEARDOWN");
        break;
    }
  }

  @Override
  public void notifyEvent(int event, int sessionId) throws RemoteException {
      if (event == WfdEnums.WfdEvent.UIBC_ENABLED.ordinal()) {
          Log.d(TAG, "notifyEvent- UIBC enabled callback");
      } else if (event == WfdEnums.WfdEvent.UIBC_DISABLED.ordinal()) {
          Log.d(TAG, "notifyEvent- UIBC disabled callback");
      }else if (event == WfdEnums.WfdEvent.START_SESSION_FAIL.ordinal()) {
          Log.d(TAG, "notifyEvent- START_SESSION_FAIL");
      } else {
          Log.d(TAG, "notifyEvent- unrecognized event: " + event);
      }
  }

  @Override
  public void notify(Bundle b, int sessionId) throws RemoteException {
      // TODO Auto-generated method stub
  }

}
