/*****************************************************************************
  Copyright (C) 2011,2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 ******************************************************************************/

package com.qti.diagservices;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;
import android.os.PowerManager;
import android.content.Context;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

public class QTIDiagServices extends Service {
    DiagJNIInterface dji;
    Handler callbackHandler;
    private static final String TAG = "QTIDiagServices";

    @Override
        public IBinder onBind(Intent intent) {
            return null;
        }

    @Override
        public void onCreate() {
            callbackHandler = new Handler() {
                public void handleMessage(Message msg) {
                    String s;
                    Bundle b = msg.getData();
                    s= b.getString("diag_command");
                    if (s.equals("reboot")) {
                        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
                        pm.reboot(null);
                    } else {
                        Log.w(TAG, "Cannot handle command");
                    }
                }
            };
            dji = new DiagJNIInterface();
            dji.setHandler(callbackHandler);
        }

    @Override
        public void onDestroy() {
        }

    @Override
        public void onStart(Intent intent, int startid) {
        }
}
