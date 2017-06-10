/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

 /*
 * Copyright (C) 2013 The Android Open Source Project
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
 * limitations under the License
 */

package com.borqs.videocall;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.view.View;

import com.borqs.videocall.widget.multiwaveview.GlowPadView;
//import com.android.services.telephony.common.CallDetails;

/**
 *
 */
public class GlowPadWrapper extends GlowPadView implements GlowPadView.OnTriggerListener {

    // Parameters for the GlowPadView "ping" animation; see triggerPing().
    private static final int PING_MESSAGE_WHAT = 101;
    private static final boolean ENABLE_PING_AUTO_REPEAT = true;
    private static final long PING_REPEAT_DELAY_MS = 1200;

    private final Handler mPingHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case PING_MESSAGE_WHAT:
                    triggerPing();
                    break;
            }
        }
    };

    private VideoCallScreen mScreen;
	private VideoCallApp mApp;
    private AnswerListener mAnswerListener;
    private boolean mPingEnabled = true;
    private boolean mTargetTriggered = false;

    public GlowPadWrapper(Context context) {
        super(context);
        Log.d(this, "class created " + this + " ");
    }

    public GlowPadWrapper(Context context, AttributeSet attrs) {
        super(context, attrs);
        Log.d(this, "class created " + this);
    }

	public void setGlowPadWrapper(VideoCallScreen screen, VideoCallApp app) {
		 mScreen = screen;
		 mApp = app;
	 }

    @Override
    protected void onFinishInflate() {
        Log.d(this, "onFinishInflate()");
        super.onFinishInflate();
        setOnTriggerListener(this);
    }

    public void startPing() {
        Log.d(this, "startPing");
        mPingEnabled = true;
        triggerPing();
    }

    public void stopPing() {
        Log.d(this, "stopPing");
        mPingEnabled = false;
        mPingHandler.removeMessages(PING_MESSAGE_WHAT);
    }

    private void triggerPing() {
        Log.d(this, "triggerPing(): " + mPingEnabled + " " + this);
        if (mPingEnabled && !mPingHandler.hasMessages(PING_MESSAGE_WHAT)) {
            ping();

            if (ENABLE_PING_AUTO_REPEAT) {
                mPingHandler.sendEmptyMessageDelayed(PING_MESSAGE_WHAT, PING_REPEAT_DELAY_MS);
            }
        }
    }

    @Override
    public void onGrabbed(View v, int handle) {
        Log.d(this, "onGrabbed()");
        stopPing();
    }

    @Override
    public void onReleased(View v, int handle) {
        Log.d(this, "onReleased()");
        if (mTargetTriggered) {
            mTargetTriggered = false;
        } else {
            startPing();
        }
    }
/*
    private int toCallType(int resId) {
        int callType = CallDetails.CALL_TYPE_VOICE;
        switch (resId) {
            case R.drawable.ic_lockscreen_answer_video:
                callType = CallDetails.CALL_TYPE_VT;
                break;
            case R.drawable.ic_lockscreen_answer_tx_video:
                callType = CallDetails.CALL_TYPE_VT_TX;
                break;
            case R.drawable.ic_lockscreen_answer_rx_video:
                callType = CallDetails.CALL_TYPE_VT_RX;
                break;
            case R.drawable.ic_lockscreen_answer:
                callType = CallDetails.CALL_TYPE_VOICE;
                break;
            default:
                Log.wtf(this, "Unknown resource id, resId=" + resId);
                break;
        }
        return callType;
    }
*/
    @Override
    public void onTrigger(View v, int target) {
        Log.d(this, "onTrigger()");
		setGlowPadWrapper(mScreen,mApp);
        final int resId = getResourceIdForTarget(target);
        switch (resId) {
            case R.drawable.ic_lockscreen_answer_video:
            case R.drawable.ic_lockscreen_answer_tx_video:
            case R.drawable.ic_lockscreen_answer_rx_video:
            case R.drawable.ic_lockscreen_answer:
               // mAnswerListener.onAnswer(toCallType(resId));
				//mAnswerListener.onAnswer(mScreen,mApp);
			   mAnswerListener.onAnswer();
                mTargetTriggered = true;
                break;
            case R.drawable.ic_lockscreen_decline:
                mAnswerListener.onDecline();
                mTargetTriggered = true;
                break;
            case R.drawable.ic_lockscreen_text:
                mAnswerListener.onText();
                mTargetTriggered = true;
                break;
            default:
                // Code should never reach here.
                Log.e(this, "Trigger detected on unhandled resource. Skipping.");
        }
    }

    @Override
    public void onGrabbedStateChange(View v, int handle) {

    }

    @Override
    public void onFinishFinalAnimation() {

    }

    public void setAnswerListener(AnswerListener listener) {
        mAnswerListener = listener;
    }

    public interface AnswerListener {
//        void onAnswer(int callType);
		//void onAnswer(VideoCallScreen mScreen,VideoCallApp app);
		void onAnswer();

        void onDecline();
        void onText();
    }
}
