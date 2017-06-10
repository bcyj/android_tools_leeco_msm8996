/*
 * Copyright (c) 2011-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qti.factory.Flashlight;

import static com.qti.factory.Values.HasFlashlightFile;

import java.io.FileOutputStream;
import java.util.HashMap;
import java.util.Map;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.os.Bundle;
import android.util.Log;

import com.qti.factory.R;
import com.qti.factory.Utils;
import com.qti.factory.Values;
import com.qti.factory.Framework.MainApp;

public class Flashlight extends Activity {

	private static Context mContext;
	String TAG = "Flashlight";
	Camera mycam = null;
	Parameters camerPara = null;
	final byte[] ON = { '2', '5', '5' };
	final byte[] OFF = { '0' };
	private static String deviceNode = "/sys/class/leds/flashlight/brightness";

	@Override
	public void finish() {

		super.finish();
	}

	private void init(Context context) {

		setResult(RESULT_CANCELED);
		mContext = context;

		int index = getIntent().getIntExtra(Values.KEY_SERVICE_INDEX, -1);
		if (index >= 0) {

			Map<String, ?> item = (Map<String, ?>) MainApp.getInstance().mItemList
					.get(index);
			HashMap<String, String> paraMap = (HashMap<String, String>) item
					.get("parameter");
			String device = paraMap.get("path");
			if (device != null)
				deviceNode = device;
		}

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
	protected void onDestroy() {

		if (!HasFlashlightFile) {
			camerPara.setFlashMode(Parameters.FLASH_MODE_OFF);
			mycam.setParameters(camerPara);
			mycam.release();
		} else {
			enableDevice(deviceNode, false);
		}

		super.onDestroy();
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);

		init(getApplicationContext());

		if (!HasFlashlightFile) {
			mycam = Camera.open();
			camerPara = mycam.getParameters();
			camerPara.setFlashMode(Parameters.FLASH_MODE_TORCH);
			mycam.setParameters(camerPara);
		} else {

			enableDevice(deviceNode, true);

		}
		showDialog(Flashlight.this);

	}

	void showDialog(final Flashlight fl) {

		new AlertDialog.Builder(fl)
				.setTitle(getString(R.string.flashlight_confirm))
				.setPositiveButton(getString(R.string.yes),
						new DialogInterface.OnClickListener() {

							public void onClick(DialogInterface dialog,
									int which) {

								setResult(RESULT_OK);
								Utils.writeCurMessage(mContext, TAG, "Pass");
								finish();
							}
						})
				.setNegativeButton(getString(R.string.no),
						new DialogInterface.OnClickListener() {

							public void onClick(DialogInterface dialog,
									int which) {

								setResult(RESULT_CANCELED);
								Utils.writeCurMessage(mContext, TAG, "Failed");
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
