/*****************************************************************************
  Copyright (C) 2011,2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 ******************************************************************************/

package com.qti.diagservices;

import android.os.Handler;
import android.os.Bundle;
import android.os.Message;
import com.qti.diagservices.DiagCommand;
import android.util.Log;

public class DiagJNIInterface {
    private static final String TAG = "DiagJNIInterface";
    static Handler h = null;
    static{
        try {
            Log.e(TAG, "Trying to load libDiagService");
            System.loadLibrary("DiagService");
        }
        catch (UnsatisfiedLinkError ule) {
            Log.e(TAG, "WARNING: Could not load libDiagService");
        }
    }

    public static void eventHandler(String s) {
        if(h == null) {
            Log.w(TAG, "Handle is empty");
            return;
        }
        Log.i(TAG, "Going to send message");
        Bundle b = new Bundle();
        b.putString("diag_command", s);
        Message m = Message.obtain();
        m.setData(b);
        m.setTarget(h);
        m.sendToTarget();
    }

    public void setHandler(Handler h) {
        this.h = h;
    }

    public static native void getNextCommand();
}
