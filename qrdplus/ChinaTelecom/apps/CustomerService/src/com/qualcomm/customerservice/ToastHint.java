/*
 * Copyright (c) 2012-2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
* for attribution purposes only.
*
/* Copyright (C) 2008 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

package com.qualcomm.customerservice;

import java.util.Timer;
import java.util.TimerTask;

import com.qualcomm.customerservice.R;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.PixelFormat;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.TextView;

public class ToastHint {
    private static final String TAG = "ToastHint";
    private Context mContext;
    private View mView = null;
    private View mNextView = null;
    private Timer mTimer = new Timer();
    private static final int HIDE = 1;

    private WindowManager.LayoutParams mParams = new WindowManager.LayoutParams();
    private WindowManager mWM = null;
    private static ToastHint mToastHint = null;
    private final Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case HIDE:
                hide();
            }
        }
    };

    private ToastHint(Context context) {
        mContext = context;
        mWM = (WindowManager) mContext.getApplicationContext()
                .getSystemService(Context.WINDOW_SERVICE);
        mParams = new WindowManager.LayoutParams();
        mParams.height = WindowManager.LayoutParams.WRAP_CONTENT;
        mParams.width = WindowManager.LayoutParams.WRAP_CONTENT;
        mParams.flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
        mParams.format = PixelFormat.TRANSLUCENT;
        mParams.type = WindowManager.LayoutParams.TYPE_TOAST;
    }

    public static ToastHint makeText(Context context, CharSequence text) {
        if (mToastHint == null)
            mToastHint = new ToastHint(context);

        LayoutInflater inflate = (LayoutInflater) context
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View v = inflate.inflate(R.layout.on_screen_hint, null);
        TextView tv = (TextView) v.findViewById(R.id.message);
        tv.setText(text);
        mToastHint.mNextView = v;
        mToastHint.mContext = context;
        return mToastHint;
    }

    public static ToastHint makeText(Context context, int resId)
            throws Resources.NotFoundException {
        return makeText(context, context.getResources().getText(resId));
    }

    /**
     * Show the view on the screen.
     */
    public synchronized void show() {
        Log.d(TAG, "show");
        mTimer.cancel();
        mTimer = new Timer();
        // remove the old view if necessary
        hide();

        mView = mNextView;
        mParams.gravity = Gravity.CENTER_HORIZONTAL | Gravity.BOTTOM;
        mParams.x = 0;
        mParams.y = 120;
        if (mView.getParent() != null) {
            mWM.removeView(mView);
        }
        mWM.addView(mView, mParams);
        timerTask();
    }

    /**
     * Close the view if it's showing.
     */
    private void hide() {
        Log.d(TAG, "hide");
        if (mView != null) {
            // note: checking parent() just to make sure the view has
            // been added... i have seen cases where we get here when
            // the view isn't yet added, so let's try not to crash.
            if (mView.getParent() != null) {
                mWM.removeView(mView);
            }
            mView = null;
        }
    }

    public void timerTask() {
        mTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                Log.d(TAG, "timer task");
                mHandler.sendEmptyMessage(HIDE);
            }
        }, 2000);
    }
}
