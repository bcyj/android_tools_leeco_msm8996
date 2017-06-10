/*
 * Copyright (c) 2011-2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.factory.TriColorLed;

import java.io.FileOutputStream;
import java.util.HashMap;
import java.util.Map;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.SystemProperties;
import android.util.Log;
import android.widget.TextView;

import com.qualcomm.factory.R;
import com.qualcomm.factory.Utilities;
import com.qualcomm.factory.Values;
import com.qualcomm.factory.Framework.MainApp;

public class TriColorLed extends Activity {

    private static final String TAG = "TriColorLed";
    private static Context mContext;
    private final int RED = 0;
    private final int GREEN = 1;
    private final int BLUE = 2;
    private final int INIT_COLOR_NUM = 3;
    private int color = RED;
    final byte[] LIGHT_ON = { '2', '5', '5' };
    final byte[] LIGHT_OFF = { '0' };
    String RED_LED_DEV = "/sys/class/leds/red/brightness";
    String GREEN_LED_DEV = "/sys/class/leds/green/brightness";
    String BLUE_LED_DEV = "/sys/class/leds/blue/brightness";
    // final String RED_LED_DEV = "/sys/class/leds/red/brightness";
    // final String GREEN_LED_DEV = "/sys/class/leds/green/brightness";
    
    private int colorNum = INIT_COLOR_NUM;
    CountDownTimer mCountDownTimer = new CountDownTimer((colorNum - 1) * 1000 + 200, 1000) {

        public void onTick(long arg0) {
            logd("");
            setColor(color++);
        }

        public void onFinish() {

            logd("");
            showDialog();
            setColor(-1);
        }
    };

    @Override
    public void finish() {

        super.finish();
    }

    private void init(Context context) {
        
        setResult(RESULT_CANCELED);
        mContext = context;
        colorNum = INIT_COLOR_NUM;
        
        int index = getIntent().getIntExtra(Values.KEY_SERVICE_INDEX, -1);
        if (index >= 0) {
            
            Map<String, ?> item = (Map<String, ?>) MainApp.getInstance().mItemList.get(index);
            HashMap<String, String> paraMap = (HashMap<String, String>) item.get("parameter");
            String red = paraMap.get("red");
            String green = paraMap.get("green");
            String blue = paraMap.get("blue");
            if (red != null)
                RED_LED_DEV = "/sys/class/leds/" + red + "/brightness";
            else
                colorNum--;
            if (green != null)
                GREEN_LED_DEV = "/sys/class/leds/" + green + "/brightness";
            else
                colorNum--;
            if (blue != null)
                BLUE_LED_DEV = "/sys/class/leds/" + blue + "/brightness";
            else
                colorNum--;
        }

        setContentView(R.layout.tricolor_led);
        TextView textView = (TextView) findViewById(R.id.led_hint);
        if (colorNum == 3)
            textView.setText(R.string.led_tri_text);
        else if (colorNum == 2)
            textView.setText(R.string.led_dual_text);

        color = RED;
        mCountDownTimer.start();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        init(getApplicationContext());

    }

    @Override
    protected void onDestroy() {

        mCountDownTimer.cancel();
        super.onDestroy();
    }

    private void showDialog() {

        new AlertDialog.Builder(TriColorLed.this).setMessage(R.string.led_confirm)
                .setPositiveButton(R.string.yes, passListener).setNegativeButton(R.string.no, failListener).show();
    }

    OnClickListener passListener = new DialogInterface.OnClickListener() {

        public void onClick(DialogInterface dialog, int which) {

            setResult(RESULT_OK);
            Utilities.writeCurMessage(mContext, TAG, "Pass");
            finish();
        }
    };

    OnClickListener failListener = new DialogInterface.OnClickListener() {

        public void onClick(DialogInterface dialog, int which) {

            setResult(RESULT_CANCELED);
            Utilities.writeCurMessage(mContext, TAG, "Failed");
            finish();
        }
    };

    private void setColor(int color) {

        logd("set:" + color);
        boolean red = false, green = false, blue = false;
        switch (color) {
        case RED:
            red = true;
            break;
        case GREEN:
            green = true;
            break;
        case BLUE:
            blue = true;
            break;
        default:
            break;
        }
        try {
            FileOutputStream fRed = new FileOutputStream(RED_LED_DEV);
            fRed.write(red ? LIGHT_ON : LIGHT_OFF);
            fRed.close();
            FileOutputStream fGreen = new FileOutputStream(GREEN_LED_DEV);
            fGreen.write(green ? LIGHT_ON : LIGHT_OFF);
            fGreen.close();
            FileOutputStream fBlue = new FileOutputStream(BLUE_LED_DEV);
            fBlue.write(blue ? LIGHT_ON : LIGHT_OFF);
            fBlue.close();

        } catch (Exception e) {
            loge(e);
        }

    }

    void logd(Object d) {

        Log.d(TAG, "" + d);
    }

    void loge(Object e) {

        Log.e(TAG, "" + e);
    }
}
