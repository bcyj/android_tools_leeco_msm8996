/*
 * Copyright (c) 2011-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qti.factory.LCM;

import java.io.InputStream;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.PowerManager;
import android.util.Log;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.widget.LinearLayout;

import com.qti.factory.R;
import com.qti.factory.Utils;

public class LCM extends Activity {

	private Handler mHandler = new Handler();
	private int brightnessState = 0, imgSeq = 0;
	private float mBrightness = 1.0f;
	float x = 0, y = 0;
	WindowManager.LayoutParams mLayoutParams;

	private boolean ifLocked = false;
	private PowerManager.WakeLock mWakeLock;
	private PowerManager mPowerManager;
	private LinearLayout mLinearLayout;

	String TAG = "LCM";
	private static Context mContext;

	private int[] mTestImg = { R.drawable.lcm_red, R.drawable.lcm_green,
			R.drawable.lcm_blue, R.drawable.lcm_black_white_lump };
	private final boolean DO_LUM_TEST = false;

	// , R.drawable.lcm_stripe
	@Override
	public void finish() {

		super.finish();
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		mContext = this;

		requestWindowFeature(Window.FEATURE_NO_TITLE);

		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(R.layout.lcm);

		mLayoutParams = getWindow().getAttributes();
		mLayoutParams.screenBrightness = 1;
		getWindow().setAttributes(mLayoutParams);

		mLinearLayout = (LinearLayout) findViewById(R.id.myLinearLayout1);
		mPowerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
		mWakeLock = mPowerManager.newWakeLock(
				PowerManager.SCREEN_BRIGHT_WAKE_LOCK, "BackLight");
		if (DO_LUM_TEST)
			start();
		else {
			setBackgroud(0);
		}
	}

	public void start() {

		mHandler.removeCallbacks(mRunnable);
		mHandler.postDelayed(mRunnable, 0);
	}

	private Runnable mRunnable = new Runnable() {

		public void run() {

			if (DO_LUM_TEST) {
				if (brightnessState == 0) {
					mBrightness = 0.01f;
					brightnessState = 1;

				} else {
					mBrightness = 1.0f;
					brightnessState = 0;
				}
			}

			mLayoutParams = getWindow().getAttributes();
			mLayoutParams.screenBrightness = mBrightness;
			getWindow().setAttributes(mLayoutParams);

			mHandler.postDelayed(mRunnable, 1200);
			System.gc();
		}
	};

	private void setBackgroud(int index) {
		if (index >= mTestImg.length)
			return;

		try {
			BitmapFactory.Options option = new BitmapFactory.Options();
			option.inPreferredConfig = Config.ARGB_8888;
			option.inPurgeable = true;
			option.inInputShareable = true;
			InputStream mInputSream = getResources().openRawResource(
					mTestImg[index]);
			Bitmap bitmap = BitmapFactory.decodeStream(mInputSream, null,
					option);
			BitmapDrawable bitmapDrawable = new BitmapDrawable(bitmap);
			mLinearLayout.setBackgroundDrawable(bitmapDrawable);

		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {

		final int mAction = event.getAction();
		if ((mAction == MotionEvent.ACTION_UP)) {
			if (imgSeq < mTestImg.length) {
				imgSeq++;
				setBackgroud(imgSeq);
			}
		}
		if (imgSeq >= mTestImg.length) {
			imgSeq = mTestImg.length - 1;
			showDialog();
		} else
			setBackgroud(imgSeq);
		return true;
	}

	private void showDialog() {

		new AlertDialog.Builder(this)

				.setTitle(R.string.lcm_confirm)

				.setPositiveButton(R.string.pass,
						new DialogInterface.OnClickListener() {

							public void onClick(
									DialogInterface dialoginterface, int i) {

								mHandler.removeCallbacks(mRunnable);

								setResult(RESULT_OK);
								Utils.writeCurMessage(mContext, TAG, "Pass");
								finish();
							}
						})

				.setNegativeButton(R.string.fail,
						new DialogInterface.OnClickListener() {

							public void onClick(
									DialogInterface dialoginterface, int i) {

								mHandler.removeCallbacks(mRunnable);
								setResult(RESULT_CANCELED);
								Utils.writeCurMessage(mContext, TAG, "Failed");
								finish();
							}
						}).setCancelable(false).show();
	}

	@Override
	protected void onResume() {

		wakeLock();
		super.onResume();
	}

	@Override
	protected void onPause() {

		wakeUnlock();
		super.onPause();

		mHandler.removeCallbacks(mRunnable);
		setResult(RESULT_CANCELED);

	}

	private void wakeLock() {

		if (!ifLocked) {
			ifLocked = true;
			mWakeLock.acquire();
		}
	}

	private void wakeUnlock() {

		if (ifLocked) {
			mWakeLock.release();
			ifLocked = false;
		}
	}

	void logd(Object d) {

		Log.d(TAG, "" + d);
	}

	void loge(Object e) {

		Log.e(TAG, "" + e);
	}

}
