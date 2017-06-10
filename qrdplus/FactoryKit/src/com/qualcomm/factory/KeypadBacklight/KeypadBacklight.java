/*
 * Copyright (c) 2011-2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.factory.KeypadBacklight;

import java.io.FileOutputStream;
import java.util.HashMap;
import java.util.Map;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;

import com.qualcomm.factory.R;
import com.qualcomm.factory.Utilities;
import com.qualcomm.factory.Values;
import com.qualcomm.factory.Framework.MainApp;

public class KeypadBacklight extends Activity {

    private static Context mContext;
    String TAG = "KeypadBacklight";
    final byte[] ON = { '2', '5', '5' };
    final byte[] OFF = { '0' };
    private static String deviceNode = "/sys/class/leds/button-backlight/brightness";

    @Override
    public void finish() {

        super.finish();
    }
    
    private void init(Context context) {
        
        setResult(RESULT_CANCELED);
        mContext = context;
        
        int index = getIntent().getIntExtra(Values.KEY_SERVICE_INDEX, -1);
        if (index >= 0) {
            
            Map<String, ?> item = (Map<String, ?>) MainApp.getInstance().mItemList.get(index);
            HashMap<String, String> paraMap = (HashMap<String, String>) item.get("parameter");
            String device = paraMap.get("device");
            if (device != null)
                deviceNode = "/sys/class/leds/" + device + "/brightness";

        }

    }

    @Override
    protected void onDestroy() {
        
        enableDevice(deviceNode, false);
        super.onDestroy();
    }
    
    void enableDevice(String fileNode, boolean enable) {
        FileOutputStream fileOutputStream;
        try {
            
            fileOutputStream = new FileOutputStream(fileNode);
            if (enable)
                fileOutputStream.write(ON);
            else
                fileOutputStream.write(OFF);
            fileOutputStream.close();
            
        } catch (Exception e) {
            loge(e);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        
        init(getApplicationContext());
        enableDevice(deviceNode, true);
        showDialog(KeypadBacklight.this);

    }

    void showDialog(final KeypadBacklight fl) {

        new AlertDialog.Builder(fl).setTitle(getString(R.string.keypadbacklight_confirm))
                .setPositiveButton(getString(R.string.yes), new DialogInterface.OnClickListener() {

                    public void onClick(DialogInterface dialog, int which) {
                        
                        setResult(RESULT_OK);
                        Utilities.writeCurMessage(mContext, TAG, "Pass");
                        finish();
                    }
                }).setNegativeButton(getString(R.string.no), new DialogInterface.OnClickListener() {

                    public void onClick(DialogInterface dialog, int which) {

                        setResult(RESULT_CANCELED);
                        Utilities.writeCurMessage(mContext, TAG, "Failed");
                        finish();
                    }
                }).setCancelable(false).show();
    }

    private void loge(Object e) {
        
        if (e == null)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();
        e = "[" + mMethodName + "] " + e;
        Log.e(TAG, e + "");
    }

    private void logd(Object s) {
        
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.d(TAG, s + "");
    }
}
