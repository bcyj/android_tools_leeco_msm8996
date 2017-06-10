/*
 * Copyright (c) 2011-2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.factory.Keypad;

import java.util.HashMap;
import java.util.Map;

import android.R.integer;
import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.SystemProperties;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

import com.qualcomm.factory.R;
import com.qualcomm.factory.Utilities;
import com.qualcomm.factory.Values;
import com.qualcomm.factory.Framework.MainApp;

public class Keypad extends Activity {

    private static final String TAG = "Keypad";
    private static String resultString = Utilities.RESULT_FAIL;
    private static Context mContext;
    private int itemIndex = -1;
    
    private final int[] KEYMODE0 = { KeyEvent.KEYCODE_VOLUME_UP, KeyEvent.KEYCODE_VOLUME_DOWN };
    private final int[] KEYMODE1 = { KeyEvent.KEYCODE_VOLUME_UP, KeyEvent.KEYCODE_VOLUME_DOWN, KeyEvent.KEYCODE_CAMERA };
    private final int[] KEYMODE2 = { KeyEvent.KEYCODE_VOLUME_UP, KeyEvent.KEYCODE_VOLUME_DOWN, KeyEvent.KEYCODE_CAMERA,
            KeyEvent.KEYCODE_FOCUS };
    private final int[][] KEYMODE = { KEYMODE0, KEYMODE1, KEYMODE2 };

    int[] keyMode;
    HashMap<Integer, Boolean> keyStatusHashMap = new HashMap<Integer, Boolean>();

    @Override
    public void finish() {
        Utilities.writeCurMessage(this, TAG, resultString);
        super.finish();
    }

    @Override
    protected void onPause() {

        super.onPause();
    }

    private void init(Context context) {
        mContext = context;
        resultString = Utilities.RESULT_FAIL;
        setContentView(R.layout.keypad);

        // get keymode
        itemIndex = getIntent().getIntExtra(Values.KEY_SERVICE_INDEX, -1);
        int keymodeIndex = Utilities.getIntPara(itemIndex, "KeyMode", 0);
        
        keyMode = KEYMODE[keymodeIndex];
        for (int i = 0; i < keyMode.length; i++) {
            keyStatusHashMap.put(keyMode[i], false);
        }
        
        // hide some keys according to keymode on board
        TextView focusView = (TextView) findViewById(R.id.focus);
        TextView camView = (TextView) findViewById(R.id.camera);
        if (keymodeIndex == 0) {
            focusView.setVisibility(View.GONE);
            camView.setVisibility(View.GONE);
        } else if (keymodeIndex == 1)
            focusView.setVisibility(View.GONE);
    }
    
    private boolean allKeyPassed() {
        for (int i = 0; i < keyMode.length; i++) {
            if (!keyStatusHashMap.get(keyMode[i]))
                return false;
        }
        return true;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        init(getApplicationContext());
        
        Button pass = (Button) findViewById(R.id.pass);
        pass.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {

                pass();
            }
        });

        Button fail = (Button) findViewById(R.id.fail);
        fail.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {

                fail(null);
            }
        });

    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {

        TextView keyText = null;
        logd(keyCode);
        keyStatusHashMap.put(keyCode, true);
        switch (keyCode) {
        case KeyEvent.KEYCODE_VOLUME_UP:
            keyText = (TextView) findViewById(R.id.volume_up);
            break;
        case KeyEvent.KEYCODE_VOLUME_DOWN:
            keyText = (TextView) findViewById(R.id.volume_down);

            break;

        case KeyEvent.KEYCODE_FOCUS:
            keyText = (TextView) findViewById(R.id.focus);

            break;
        case KeyEvent.KEYCODE_CAMERA:
            keyText = (TextView) findViewById(R.id.camera);
            break;
        }

        if (null != keyText) {
            keyText.setBackgroundResource(R.color.green);
        }
        if (allKeyPassed())
            pass();
        return true;
    }

    void fail(Object msg) {

        loge(msg);
        setResult(RESULT_CANCELED);
        resultString = Utilities.RESULT_FAIL;
        finish();
    }

    void pass() {

        setResult(RESULT_OK);
        resultString = Utilities.RESULT_PASS;
        finish();
    }

    void logd(Object d) {

        Log.d(TAG, "" + d);
    }

    void loge(Object e) {

        Log.e(TAG, "" + e);
    }

}
