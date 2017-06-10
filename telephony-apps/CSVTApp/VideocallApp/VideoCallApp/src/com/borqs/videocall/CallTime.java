/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */
package com.borqs.videocall;

import android.os.Handler;
import android.os.SystemClock;
import android.util.Log;


public class CallTime extends Handler {
    private static final String TAG = "VT/CallTime";
    private static final boolean DBG = false;

    private long mBaseTime = -1;
    private boolean mTimerRunning = false;
    final private long mInterval = 1000;  //update every second
	final private long mIntervalRound = mInterval / 2;
    private PeriodicTimerCallback mTimerCallback;
    private OnTickListener mListener;

    interface OnTickListener {
        void onTickForCallTimeElapsed(long timeElapsed);
    }

    public CallTime( OnTickListener listener) {
        mListener = listener;
        mTimerCallback = new PeriodicTimerCallback();
    }

    void startTimer(){
	log("start timer, basetime:" + mBaseTime);
        mBaseTime = SystemClock.uptimeMillis();
	long nextReport = mBaseTime + mInterval;
	mTimerRunning = true;
	postAtTime(mTimerCallback, nextReport);
    }

    /* package */
    private void periodicUpdateTimer() {

	if( mTimerRunning == false){
		if (DBG) log("timer not start yet, fake case, just return.");
		return;
	}

	long now = SystemClock.uptimeMillis();
	long elapsed = ( now - mBaseTime + mIntervalRound)/mInterval;

	mListener.onTickForCallTimeElapsed( elapsed);

	long nextReport = mBaseTime + ( elapsed + 1)*mInterval;
	postAtTime(mTimerCallback, nextReport);
	if(DBG)log("now: " + now + " nextReport:" + nextReport);

    }

    void cancelTimer() {
        if (DBG) log("cancelTimer()......");
        mTimerRunning = false;
        removeCallbacks(mTimerCallback);
    }

    private static void log(String msg) {
        if (MyLog.DEBUG) MyLog.d(TAG, "[CallTime] " + msg);
    }

    private class PeriodicTimerCallback implements Runnable {
        PeriodicTimerCallback() {
        }

        public void run() {
            periodicUpdateTimer();
        }
    }

	/* return call during in format: 'long' */
	long getCallDuration() {
		if (MyLog.DEBUG) MyLog.d(TAG, "getCallDuration...");
		if(!mTimerRunning)
			return 0;
		long DisconnectTime = SystemClock.uptimeMillis();
		return ( DisconnectTime - mBaseTime)/1000;
	}
}
