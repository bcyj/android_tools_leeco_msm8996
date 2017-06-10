/*============================================================================
@file SuspendMonitor.java

@brief
Monitor apps suspend, and generate an audible beep if/when the device comes
out of suspend.

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qualcomm.sensors.qsensortest;

import java.util.Timer;
import java.util.TimerTask;

import android.annotation.SuppressLint;
import android.content.Context;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.PowerManager;

public class SuspendMonitor {
  private long currentTime;
  private PowerManager pm;
  private PowerManager.WakeLock wl;
  private Timer timer;
  private Uri notificationRing;
  private Ringtone ringTone;
  private boolean active;

  public static final int MONITOR_TIMER_MS = 500;
  public static final int MAX_TIMER_SLOP = 20;

  public SuspendMonitor(){
    this.active = false;
    this.notificationRing = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);
    this.pm = (PowerManager)TabControl.getContext().getSystemService(Context.POWER_SERVICE);
    this.wl = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,"beep");
    this.ringTone = RingtoneManager.getRingtone(TabControl.getContext(), notificationRing);
  }

  @SuppressLint("Wakelock")
  public void start(){
    this.active = true;
    this.currentTime = System.currentTimeMillis();
    this.timer = new Timer();
    timer.scheduleAtFixedRate(new TimerTask() {
      @Override
      public void run() {
        long newTime = System.currentTimeMillis();
        long diff = newTime - SuspendMonitor.this.currentTime;
        SuspendMonitor.this.currentTime = newTime;
        if(diff > MAX_TIMER_SLOP + MONITOR_TIMER_MS) {
          wl.acquire();
          SuspendMonitor.this.ringTone.play();
          wl.release();
        }
      }
    }, 0, MONITOR_TIMER_MS);
  }

  public void stop(){
    this.active = false;
    this.timer.cancel();
  }

  public boolean active(){ return this.active; }
}