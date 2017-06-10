/*
 * Copyright (c) 2011-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qti.factory.TouchPanelKey;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;
import android.os.SystemProperties;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import com.qti.factory.Utils;
import com.qti.factory.Values;

public class TouchPanelKey extends Activity {

	String TAG = "TouchPanelKey";
	final static int MENU = 1;
	final static int HOME = 2;
	final static int SEARCH = 3;
	final static int BACK = 4;
	Panel mPanel;
	WindowManager mWindowManager;
	int[] MHB_KEYMODE = { MENU, HOME, BACK };// MHB means: MENU, HOME, BACK
	int[] DEFAULT_KEYMODE = { MENU, HOME, SEARCH, BACK };
	int[] MHBS_KEYMODE = { MENU, HOME, BACK, SEARCH };
	int[] HMBS_KEYMODE = { HOME, MENU, BACK, SEARCH };
	int[] MB_KEYMODE = { MENU, BACK };
	int[] keyMode;
	int keyTestIndex;

	@Override
	public void finish() {

		super.finish();
	}

	private void init() {
		// get KeyPad mode
		keyTestIndex = 0;
		String hwPlatform = SystemProperties.get(Values.PROP_HW_PLATFORM);
		if (Values.PRODUCT_MSM8226.equals(hwPlatform))
			keyMode = MHB_KEYMODE;
		else if (Values.PRODUCT_MSM8610.equals(hwPlatform))
			keyMode = MHB_KEYMODE;
		else if (Values.PRODUCT_MSM8X25_SKU5.equals(hwPlatform))
			keyMode = MHBS_KEYMODE;
		else if (Values.PRODUCT_MSM8X25Q_SKUD.equals(hwPlatform))
			keyMode = HMBS_KEYMODE;
		else
			keyMode = DEFAULT_KEYMODE;

		mPanel = new Panel(getApplicationContext());
		mWindowManager = (WindowManager) getSystemService(Context.WINDOW_SERVICE);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(mPanel);
	}

	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		init();
	}

	class Panel extends View {

		Paint mPaint = new Paint();

		public Panel(Context context) {

			super(context);
		}

		public void onAttachedToWindow() {
			super.onAttachedToWindow();
		}

		public void onDraw(Canvas canvas) {
			logd("onDraw");
			super.onDraw(canvas);
			// get panel size
			DisplayMetrics mDisplayMetrics = new DisplayMetrics();
			getWindowManager().getDefaultDisplay().getMetrics(mDisplayMetrics);
			int heightPix = mDisplayMetrics.heightPixels, widthPix = mDisplayMetrics.widthPixels;
			mPaint.setTextSize(widthPix / 20);
			mPaint.setColor(Color.MAGENTA);
			try {
				getWindow().setType(WindowManager.LayoutParams.TYPE_KEYGUARD);
			} catch (Exception e) {
				loge(e);
			}
			if (keyTestIndex >= keyMode.length)
				return;
			switch (keyMode[keyTestIndex]) {
			case MENU:
				canvas.drawText("Please Press <MENU KEY>", 0, heightPix
						- widthPix / 20, mPaint);
				break;
			case HOME:
				canvas.drawText("Please Press <HOME KEY>", 0, heightPix
						- widthPix / 20, mPaint);
				break;
			case SEARCH:
				canvas.drawText("Please Press <SEARCH KEY>", 0, heightPix
						- widthPix / 20, mPaint);
				break;
			case BACK:
				canvas.drawText("Please Press <BACK KEY>", 0, heightPix
						- widthPix / 20, mPaint);
				break;
			default:
				break;
			}
		}

	}

	public boolean onKeyUp(int keyCode, KeyEvent msg) {

		logd(keyCode);
		if (keyCode == KeyEvent.KEYCODE_BACK && keyMode[keyTestIndex] != BACK) {
			setResult(RESULT_CANCELED);
			Utils.writeCurMessage(this, TAG, "Failed");
			finish();
		}
		switch (keyMode[keyTestIndex]) {
		case MENU:
			if (keyCode == KeyEvent.KEYCODE_MENU)
				keyTestIndex++;
			break;
		case HOME:
			if (keyCode == KeyEvent.KEYCODE_HOME)
				keyTestIndex++;
			break;
		case SEARCH:
			if (keyCode == KeyEvent.KEYCODE_SEARCH)
				keyTestIndex++;
			break;
		case BACK:
			if (keyCode == KeyEvent.KEYCODE_BACK)
				keyTestIndex++;
		}
		if (keyTestIndex == keyMode.length) {
			setResult(RESULT_OK);
			Utils.writeCurMessage(this, TAG, "Pass");
			finish();
		}
		mPanel.invalidate();
		return true;
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
