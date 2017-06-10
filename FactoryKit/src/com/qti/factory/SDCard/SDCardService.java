/*
 * Copyright (c) 2013-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qti.factory.SDCard;

import java.util.Map;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;

import com.qti.factory.Utils;
import com.qti.factory.Values;
import com.qti.factory.Framework.MainApp;

public class SDCardService extends Service {

	private final static String TAG = "SDCardService";
	private static boolean result = false;
	private static Context mContext = null;
	private static int index = -1;
	private final static Handler mHandler = new Handler() {
		public void dispatchMessage(android.os.Message msg) {
			boolean res = (Boolean) msg.obj;

			if (res)
				pass();
			else
				fail(null);
		};
	};

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		if (intent == null)
			return -1;
		index = intent.getIntExtra(Values.KEY_SERVICE_INDEX, -1);
		if (index < 0)
			return -1;

		init(getApplicationContext());
		start();
		// finish();

		return super.onStartCommand(intent, flags, startId);
	}

	private void init(Context context) {
		mContext = context;
		result = false;
	}

	private void start() {
		logd("");
		exec("cat /proc/partitions");
	}

	void exec(final String para) {

		new Thread() {

			public void run() {
				try {
					logd(para);
					String data = Utils.readFile("/proc/partitions");

					logd(data);
					Message message = new Message();
					if (data.contains("mmcblk1"))
						message.obj = true;
					else
						message.obj = false;
					message.setTarget(mHandler);
					message.sendToTarget();

				} catch (Exception e) {
					logd(e);
				}

			}
		}.start();

	}

	public static void finish(boolean writeResult) {

		if (writeResult) {
			Map<String, String> item = (Map<String, String>) MainApp
					.getInstance().mItemList.get(index);
			if (result) {
				item.put("result", Utils.RESULT_PASS);
				Utils.saveStringValue(mContext, item.get("title"),
						Utils.RESULT_PASS);
				Utils.writeCurMessage(TAG, Utils.RESULT_PASS);
			} else {
				item.put("result", Utils.RESULT_FAIL);
				Utils.saveStringValue(mContext, item.get("title"),
						Utils.RESULT_FAIL);
				Utils.writeCurMessage(TAG, Utils.RESULT_FAIL);
			}
			mContext.sendBroadcast(new Intent(Values.BROADCAST_UPDATE_MAINVIEW));
		}

	};

	public static void finish() {
		finish(true);
	}

	@Override
	public void onDestroy() {
		logd("");
		finish(false);
		super.onDestroy();
	}

	static void fail(Object msg) {
		loge(msg);
		result = false;
		finish();
	}

	static void pass() {

		result = true;
		finish();
	}

	private void logd(Object s) {

		if (Values.SERVICE_LOG) {
			Thread mThread = Thread.currentThread();
			StackTraceElement[] mStackTrace = mThread.getStackTrace();
			String mMethodName = mStackTrace[3].getMethodName();

			s = "[" + mMethodName + "] " + s;
			Log.d(TAG, s + "");
		}
	}

	private static void loge(Object e) {

		if (e == null)
			return;
		Thread mThread = Thread.currentThread();
		StackTraceElement[] mStackTrace = mThread.getStackTrace();
		String mMethodName = mStackTrace[3].getMethodName();
		e = "[" + mMethodName + "] " + e;
		Log.e(TAG, e + "");
	}

	@Override
	public IBinder onBind(Intent arg0) {
		return null;
	}

}
