/*
 * Copyright (c) 2011-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.factory.PSensor;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.qti.factory.R;
import com.qti.factory.Utils;

public class PSensor extends Activity {

	private SensorManager mSensorManager = null;
	private Sensor mPSensor = null;
	private PSensorListener mPSensorListener;
	String resultString = Utils.RESULT_FAIL;
	ImageTextView mImageTextView;
	TextView mPsTextView;
	TextView mCmdText;
	Button cancel;
	private final static int VALUE_INIT = 10;

	private final static int STAUS_COVERED = 0;
	private final static int STAUS_UNCOVERED = 5;
	private final static int STAUS_CHANGING = -1;
	private final static int STAUS_INIT = 10;

	private final static int TO_COVER = 0;
	private final static int TO_UNCOVER = 5;
	private final static int TO_KEEP = 10;

	private int psensorValue = VALUE_INIT;
	private int command = TO_COVER;
	private int status = STAUS_INIT;

	private final long INIT_TIME = 500;
	private final long TIME_OUT = 7000;
	private boolean initFlag = false;
	private int step = 0;

	String TAG = "PSensor";
	String PS_FILE = "/sys/bus/i2c/devices/1-0039/ps_data";
	private final static int SENSOR_TYPE = Sensor.TYPE_PROXIMITY;

	TimeOutTimer mTimeOutTimer = new TimeOutTimer(7000, 7000);

	class TimeOutTimer extends CountDownTimer {

		public boolean isRunning = false;

		public TimeOutTimer(long millisInFuture, long countDownInterval) {

			super(millisInFuture, countDownInterval);
			isRunning = false;
		}

		@Override
		public void onTick(long arg0) {

		}

		@Override
		public void onFinish() {

			isRunning = false;
			fail(getString(R.string.time_out));
		}
	};

	CountDownTimer mInitTimer = new CountDownTimer(INIT_TIME, INIT_TIME) {

		@Override
		public void onTick(long arg0) {

		}

		@Override
		public void onFinish() {

			step = 1;
			initFlag = true;
			command = TO_COVER;
			mCmdText.setText(getString(R.string.psensor_to_cover));
			mCmdText.setTextColor(Color.GREEN);
			mTimeOutTimer.start();
			mTimeOutTimer.isRunning = true;
		}
	};

	CommandCoverTimer mCommandCoverTimer = new CommandCoverTimer(700, 700);

	class CommandCoverTimer extends CountDownTimer {

		public boolean isRunning = false;

		public CommandCoverTimer(long millisInFuture, long countDownInterval) {

			super(millisInFuture, countDownInterval);
			isRunning = false;
		}

		@Override
		public void onTick(long arg0) {

		}

		@Override
		public void onFinish() {

			logd("CommandCoverTimer Finish");
			step = 2;
			isRunning = false;
			command = TO_UNCOVER;
			mCmdText.setText(getString(R.string.psensor_to_uncover));
			mCmdText.setTextColor(Color.RED);
			mTimeOutTimer.start();
			mTimeOutTimer.isRunning = true;
		}

	}

	CommandUncoverTimer mCommandUncoverTimer = new CommandUncoverTimer(700, 700);

	class CommandUncoverTimer extends CountDownTimer {

		public boolean isRunning = false;

		public CommandUncoverTimer(long millisInFuture, long countDownInterval) {

			super(millisInFuture, countDownInterval);
			isRunning = false;
		}

		@Override
		public void onTick(long arg0) {

		}

		@Override
		public void onFinish() {

			logd("CommandUncoverTimer Finish");
			isRunning = false;
			mCmdText.setTextColor(Color.BLACK);
			mCmdText.setText(getString(R.string.pass));
			pass();
		}

	}

	@Override
	public void finish() {

		Utils.writeCurMessage(this, TAG, resultString);
		if (mCommandCoverTimer.isRunning)
			mCommandCoverTimer.cancel();
		if (mCommandUncoverTimer.isRunning)
			mCommandUncoverTimer.cancel();
		if (mTimeOutTimer.isRunning)
			mTimeOutTimer.cancel();
		try {

			mSensorManager.unregisterListener(mPSensorListener, mPSensor);
		} catch (Exception e) {
		}
		super.finish();
	}

	void bindView() {

		mImageTextView = (ImageTextView) findViewById(R.id.psensor_result);
		mPsTextView = (TextView) findViewById(R.id.psenor_ps);
		mCmdText = (TextView) findViewById(R.id.psenor_hint);
		// Button cancel = (Button) findViewById(R.id.psensor_cancel);
		// cancel.setOnClickListener(new View.OnClickListener() {
		//
		// public void onClick(View v) {
		//
		// fail(null);
		// }
		// });
	}

	void getService() {

		mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
		if (mSensorManager == null) {
			fail(getString(R.string.service_get_fail));
		}

		mPSensor = mSensorManager.getDefaultSensor(SENSOR_TYPE);
		if (mPSensor == null) {
			fail(getString(R.string.sensor_get_fail));
		}

		mPSensorListener = new PSensorListener(this);
		if (!mSensorManager.registerListener(mPSensorListener, mPSensor,
				SensorManager.SENSOR_DELAY_FASTEST)) {
			fail(getString(R.string.sensor_register_fail));
		}
	}

	void updateStatusView(int status, String psData) {

		if ("".equals(psData))
			mPsTextView.setVisibility(View.GONE);
		else {
			mPsTextView.setVisibility(View.VISIBLE);
			mPsTextView.setText("PS= " + psData);
		}

		switch (status) {
		case STAUS_COVERED:
			mImageTextView.setImageResource(R.drawable.on);
			mImageTextView.setText("Status" + " : "
					+ getString(R.string.psensor_covered));
			break;
		case STAUS_INIT:
		case STAUS_UNCOVERED:
			mImageTextView.setImageResource(R.drawable.off);
			mImageTextView.setText("Status" + " : "
					+ getString(R.string.psensor_uncovered));
			break;
		case STAUS_CHANGING:
			mImageTextView.setImageResource(R.drawable.changing);
			mImageTextView.setText("Status" + " : "
					+ getString(R.string.psensor_verifying));
			break;
		default:
			break;
		}

	}

	@Override
	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		setContentView(R.layout.psensor);

		bindView();
		getService();

		init();
		mInitTimer.start();
	}

	private void init() {

		psensorValue = VALUE_INIT;
		mCmdText.setText(getString(R.string.initialize));
		updateStatusView(VALUE_INIT, "");
	}

	public class PSensorListener implements SensorEventListener {

		public PSensorListener(Context context) {

			super();
		}

		public void onSensorChanged(SensorEvent event) {

			// PSensor event.value has 3 equal value. Value only can be 1 and 0
			synchronized (this) {
				if (event.sensor.getType() == SENSOR_TYPE) {

					psensorValue = (int) event.values[0];
					status = psensorValue;

					logd(event.values.length + ":" + event.values[0] + " "
							+ event.values[0] + " " + event.values[0] + " ");

					if (initFlag) {
						String ps = Utils.readFile(PS_FILE);
						updateStatusView(status, ps);
						logd(mCommandCoverTimer.isRunning + " " + status + " "
								+ command);
						if (mCommandCoverTimer.isRunning
								|| mCommandUncoverTimer.isRunning)
							if ((status == STAUS_UNCOVERED && command == TO_COVER)
									|| (status == STAUS_COVERED && command == TO_UNCOVER))
								fail(getString(R.string.psensor_sensor_unstable_fail));
					}

					switch (step) {
					case 1:
						if (!mCommandCoverTimer.isRunning) {
							mCommandCoverTimer.start();
							mCommandCoverTimer.isRunning = true;
							if (mTimeOutTimer.isRunning) {
								mTimeOutTimer.cancel();
								mTimeOutTimer.isRunning = false;
							}

						}
						break;
					case 2:
						if (!mCommandUncoverTimer.isRunning) {
							mCommandUncoverTimer.start();
							mCommandUncoverTimer.isRunning = true;
							if (mTimeOutTimer.isRunning) {
								mTimeOutTimer.cancel();
								mTimeOutTimer.isRunning = false;
							}
						}
					}

					// 1(no covered)->0(covered)
				}
			}
		}

		public void onAccuracyChanged(Sensor arg0, int arg1) {

		}
	}

	void fail(Object msg) {

		loge(msg);
		toast(msg);
		setResult(RESULT_CANCELED);
		resultString = Utils.RESULT_FAIL;
		finish();
	}

	void pass() {

		// toast(getString(R.string.test_pass));
		setResult(RESULT_OK);
		resultString = Utils.RESULT_PASS;
		finish();

	}

	@Override
	protected void onDestroy() {

		super.onDestroy();

		if (mSensorManager == null || mPSensorListener == null
				|| mPSensor == null)
			return;
		mSensorManager.unregisterListener(mPSensorListener, mPSensor);
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
