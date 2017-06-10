/*
 * Copyright (c) 2011-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qti.factory.FuelGaugeCalibration;

import java.io.IOException;

import android.app.Activity;
import android.app.ProgressDialog;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;

import com.qti.factory.R;
import com.qti.factory.Utils;

public class FuelGaugeCalibration extends Activity {

	final String TAG = "FuelGaugeCalibration";
	TextView mTextView;

	@Override
	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		setContentView(R.layout.fuelgauge);
		bindView();
		exec();
	}

	@Override
	public void finish() {

		super.finish();
	}

	@Override
	protected void onDestroy() {

		super.onDestroy();
	}

	Process proc = null;

	boolean fgCalibration() throws InterruptedException {

		try {
			Process process;
			// String operation[] = {"system/bin/mkdir","sdcard/zzzzz"};
			String operation[] = { "system/bin/fuel_test" };
			process = Runtime.getRuntime().exec(operation);
			process.waitFor();
			int result = process.exitValue();
			logd("exitValue=" + result);
			if (result == 255 || result == -1)
				return false;
			else
				return true;

		} catch (IOException e) {
			loge(e);
		}
		return false;
	}

	void exec() {
		logd("");
		final ProgressDialog mProgressDialog;
		mProgressDialog = new ProgressDialog(this);
		mProgressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
		mProgressDialog.setCancelable(false);
		mProgressDialog.setMessage(getString(R.string.fuelgauge_processing));
		mProgressDialog.show();

		new Thread() {

			public void run() {

				try {
					boolean result = fgCalibration();
					mProgressDialog.dismiss();
					if (result)
						pass();
					else
						fail(null);

				} catch (Exception e) {
					loge(e);
					fail(e);
				} finally {
				}
				super.run();

			}
		}.start();

	}

	void bindView() {

		mTextView = (TextView) findViewById(R.id.fuelgauge_hint_text);
		mTextView.setText(getString(R.string.fuelgauge_hint));
	}

	void fail(Object msg) {

		loge(msg);
		toast(msg);
		setResult(RESULT_CANCELED);
		Utils.writeCurMessage(this, TAG, "Failed");
		finish();
	}

	void pass() {

		setResult(RESULT_OK);
		Utils.writeCurMessage(this, TAG, "Pass");
		finish();
	}

	public void toast(Object s) {

		if (s == null)
			return;
		Toast.makeText(this, s + "", Toast.LENGTH_SHORT).show();
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
