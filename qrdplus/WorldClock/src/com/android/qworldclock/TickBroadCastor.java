/*
 * Copyright (c) 2011, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 */
package com.android.qworldclock;

import java.util.ArrayList;

import android.os.Handler;
import android.os.Message;

public class TickBroadCastor {
    private final ArrayList mListener;
    private static TickBroadCastor sInstance = null;
    private boolean mStarted;

    private final static int MSG_UPDATE = 2;
    private final static int UPATE_DURATION = 1000; // update digital clock every second

    private TickBroadCastor() {
        mListener = new ArrayList();
        mStarted = false;
    }
    public static TickBroadCastor getInstance() {
        if(null == sInstance) {
            sInstance = new TickBroadCastor();
        }
        return sInstance;
    }

    /**
     * Add listener to list.
     * @param listener
     * @return true if the listener haven't been added into the list;
     */
    public boolean addListener(TickListener listener) {
        if(mListener.indexOf(listener) >= 0) {
            return false; // Already exists
        }
        mListener.add(listener);
        return true;
    }

    /**
     * Remove listener from list.
     * @param listener
     * @return true if the listener exists in the list
     */
    public boolean removeListener(TickListener listener) {
        int index = mListener.indexOf(listener);
        if( index < 0) {
            return false; // NOT exists
        }
        mListener.remove(index);
        return true;
    }

    public void start() {
        if(!mStarted) {
            mHandler.sendMessage(mHandler.obtainMessage(MSG_UPDATE));
        }
    }

    public void stop() {
        mHandler.removeMessages(MSG_UPDATE);
    }

    private void updateTime() {
        for(int i = 0; i < mListener.size(); ++i) {
            TickListener listener = (TickListener) mListener.get(i);
            listener.updateTime();
        }
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch(msg.what) {
            case MSG_UPDATE:  // Update the view each second
                sendMessageDelayed(obtainMessage(MSG_UPDATE), UPATE_DURATION);
                updateTime();
                break;
            default:
                break;
            }
        }
    };
}
