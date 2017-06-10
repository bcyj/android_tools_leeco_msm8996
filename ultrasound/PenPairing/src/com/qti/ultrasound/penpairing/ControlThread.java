/******************************************************************************
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2014 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/
package com.qti.ultrasound.penpairing;

import android.os.Handler;
import android.os.Message;
import android.util.Log;

/**
 * This class defines the thread which waits for the socket
 * thread to finish in no more than a timeout. If timeout has
 * passed and the socket thread still runs - it is interrupted.
 */
public class ControlThread extends Thread {

    private int mTimeoutMsec;

    private final Thread mControlledThread;

    // Background threads use this Handler to post messages to
    // the main application thread
    private final Handler mHandler;

    private final DaemonWrapper mDaemon;

    public ControlThread(Thread thread, DaemonWrapper daemon, Handler handler) {
        mControlledThread = thread;
        mDaemon = daemon;
        mHandler = handler;
        mTimeoutMsec = 120000; // default is 2 minutes.
    }

    public ControlThread(Thread thread, DaemonWrapper daemon, Handler handler, int timeout) {
        this(thread, daemon, handler);
        mTimeoutMsec = timeout;
    }

    @Override
    public void run() {
        try {
            mControlledThread.join(mTimeoutMsec);
        } catch (InterruptedException e) {
            Log.e(this.toString(), "Control thread was interrupted while waiting for socket thread");
        }

        if (mControlledThread.isAlive()) {
            mControlledThread.interrupt();
            mDaemon.stop();
            Message msg = Message.obtain();
            msg.what = SemiAutomaticActivity.ALERT_FAIL_MESSAGE;
            msg.obj = SemiAutomaticActivity.TIMEOUT_MESSAGE;
            mHandler.sendMessage(msg);
        }
    }
}
