/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;


import android.app.Activity;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.provider.Settings;
import android.util.Log;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneStateIntentReceiver;
import com.android.internal.telephony.PhoneConstants;
import com.borqs.videocall.MyLog;


public class WaitingFallBackDialog extends AlertActivity implements
        CallTime.OnTickListener, DialogInterface.OnClickListener
{
	private final String TAG = "VT/WaitFallBackDg";
    private CallTime mCallTime;
	private static final int PHONE_CHANGED = 1;

	private long WATING_TIME = 15; //second

	private PhoneStateIntentReceiver mPsir;
	AlertController.AlertParams mWaitingAlertParams;
	AlertController mWaitingAlertController;
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		if (MyLog.DEBUG) MyLog.d(TAG, "onCreate...");
		super.onCreate(savedInstanceState);

		//setContentView(R.layout.waiting_fallback_dialog);
        mCallTime = new CallTime( this);
		mPsir = new PhoneStateIntentReceiver(this, mPhoneHandler);
        mPsir.notifyPhoneCallState(PHONE_CHANGED);
        mPsir.registerIntent();

        // Set up the "dialog"
        mWaitingAlertParams = mAlertParams;
        mWaitingAlertController = mAlert;
        // mWaitingAlertParams.mIconId = com.android.internal.R.drawable.ic_dialog_usb;
        mWaitingAlertParams.mTitle = getString( R.string.waiting_fallback_label);

        StringBuilder str = new StringBuilder();
		str.append(this.getString(R.string.wait_voice_call));
		mWaitingAlertController.setMessage(str.toString());

        //p.mPositiveButtonText = getString(com.android.internal.R.string.usb_storage_button_mount);
        //p.mPositiveButtonListener = this;
        mWaitingAlertParams.mNegativeButtonText = getString( R.string.cancel_fallback_setting);
        mWaitingAlertParams.mNegativeButtonListener = this;
        setupAlert();

        mCallTime.startTimer();
	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
        mCallTime.cancelTimer();
		mPsir.unregisterIntent();
		super.onDestroy();
	}

    public void onTickForCallTimeElapsed(long timeElapsed) {
        if (timeElapsed == 0) {
            return;
        } else {
            updateTimerDisp( --WATING_TIME);
        }
    }

	private Handler mPhoneHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case PHONE_CHANGED:
                    if (MyLog.DEBUG) MyLog.d(TAG, "get message PHONE_CHANGED");
                    PhoneConstants.State state = mPsir.getPhoneState();
                    if (state == PhoneConstants.State.RINGING) {
			if (MyLog.DEBUG) MyLog.d(TAG, "end waiting by incomming call");
                        finish();
                    }
                    break;
                default:
                    break;
            }
        }
    };

	void updateTimerDisp (long count) {

		StringBuilder str = new StringBuilder();
		str.append(this.getString(R.string.wait_voice_call));
		str.append("" + count);
		str.append(this.getString(R.string.seconds));

		mWaitingAlertController.setMessage(str.toString());

        if( count <=0)
        {
		finish();
        }


	}

	public void onClick(DialogInterface arg0, int arg1) {
		// TODO Auto-generated method stub

	}
}
