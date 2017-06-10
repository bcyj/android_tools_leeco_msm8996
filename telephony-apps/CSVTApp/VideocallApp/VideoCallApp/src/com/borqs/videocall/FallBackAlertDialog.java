/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

import android.os.Handler;
//import android.app.AlddertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;


import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
public class FallBackAlertDialog extends AlertActivity implements DialogInterface.OnClickListener {

    public static final String VIDEOCALL_FALL_BACK_SETTING = "videocall_fallback_setting";
    Handler mAppHandler;
    private String mPhoneUrl = null;

    public static final String ACTION_DISMISS_WAITING_FALLBACK_DIALOG =
		"com.borqs.videocall.WaitingFallBackDialog.ACTION_DISMISS_WAITING_FALLBACK_DIALOG";

    private static final String TAG = "VTFallBackAlertDg";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mAppHandler = VideoCallApp.getInstance().mHandler;
        String msg = getIntent().getStringExtra(VideoCallApp.EXTRA_FALLBACK_MESSAGE);
        mPhoneUrl = getIntent().getStringExtra(VideoCallApp.INTENT_EXTRA_PHONE_URL);

        setupBroadcastReceiver();

       // Set up the "dialog"
        final AlertController.AlertParams p = mAlertParams;
        //p.mIconId = com.android.internal.R.drawable.ic_dialog_usb;
        p.mTitle = getString( R.string.fallback_alert_label);
        p.mMessage = msg;
        //p.mPositiveButtonText = getString(com.android.internal.R.string.usb_storage_button_mount);
        //p.mPositiveButtonListener = this;
        p.mPositiveButtonText = getText( R.string.dlg_fallback_yes);
        p.mNegativeButtonText = getText( R.string.dlg_fallback_no);

        p.mPositiveButtonListener = this;
        p.mNegativeButtonListener = this;
        setupAlert();
    }
    public void onDestroy()
    {
        super.onDestroy();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    /**
     * //Cancel
     */

    public void onClick(DialogInterface dialog, int which) {
	if (MyLog.DEBUG) MyLog.d(TAG, "onClick...which: " + which);
	switch (which) {
		case DialogInterface.BUTTON_NEGATIVE:
			break;
		case DialogInterface.BUTTON_POSITIVE:
			VTCallUtils.launch2GCall(mPhoneUrl, VideoCallApp.getInstance());
			break;
		default:
			break;
		}
	dismiss();
    }

    /**
	 * Receiver to close dialog
	 */
	private BroadcastReceiver mReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			if (MyLog.DEBUG) MyLog.d(TAG, "close dialog, onreceive");
			dismiss();
		}
	};

	private void setupBroadcastReceiver() {
		IntentFilter filter = new IntentFilter();
		// register power down event
		filter.addAction(ACTION_DISMISS_WAITING_FALLBACK_DIALOG);
		VideoCallApp.getInstance().registerReceiver(mReceiver, filter);
	}
}
