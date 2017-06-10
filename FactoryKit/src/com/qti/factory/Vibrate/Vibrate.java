/*
 * Copyright (c) 2011-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qti.factory.Vibrate;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Vibrator;
import android.util.Log;

import com.qti.factory.R;
import com.qti.factory.Utils;

public class Vibrate extends Activity {

	private Handler mHandler = new Handler();
	private final long VIBRATOR_ON_TIME = 1000;
	private final long VIBRATOR_OFF_TIME = 500;
	String TAG = "Vibrate";
	private static Context mContext;
	Vibrator mVibrator = null;
	long[] pattern = { VIBRATOR_OFF_TIME, VIBRATOR_ON_TIME };

	@Override
	public void finish() {

		super.finish();
	}

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		mContext = this;

		setContentView(R.layout.vibrate);
		mVibrator = (Vibrator) getSystemService(VIBRATOR_SERVICE);
		showDialog();

	}

	@Override
	protected void onPause() {

		mHandler.removeCallbacks(mRunnable);
		mVibrator.cancel();
		super.onPause();
	}

	@Override
	protected void onResume() {

		mHandler.postDelayed(mRunnable, 0);
		super.onResume();
	}

	private Runnable mRunnable = new Runnable() {

		public void run() {

			mHandler.removeCallbacks(mRunnable);
			mVibrator.vibrate(pattern, 0);
		}
	};

	private void showDialog() {

		new AlertDialog.Builder(this).setTitle(R.string.vibrate_confirm)
				.setPositiveButton

				(R.string.pass, new DialogInterface.OnClickListener() {

					public void onClick(DialogInterface dialoginterface, int i) {

						setResult(RESULT_OK);
						Utils.writeCurMessage(mContext, TAG, "Pass");
						finish();
					}
				}).setNegativeButton

				(R.string.fail, new DialogInterface.OnClickListener() {

					public void onClick(DialogInterface dialoginterface, int i) {

						setResult(RESULT_CANCELED);
						Utils.writeCurMessage(mContext, TAG, "Failed");
						finish();
					}
				}).setCancelable(false).show();

	}

	@SuppressWarnings("unused")
	private void loge(Object e) {

		Thread mThread = Thread.currentThread();
		StackTraceElement[] mStackTrace = mThread.getStackTrace();
		String mMethodName = mStackTrace[3].getMethodName();
		e = "[" + mMethodName + "] " + e;
		Log.e(TAG, e + "");
	}

	@SuppressWarnings("unused")
	private void logd(Object s) {

		Thread mThread = Thread.currentThread();
		StackTraceElement[] mStackTrace = mThread.getStackTrace();
		String mMethodName = mStackTrace[3].getMethodName();

		s = "[" + mMethodName + "] " + s;
		Log.d(TAG, s + "");
	}

}
