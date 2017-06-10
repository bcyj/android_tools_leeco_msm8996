/*
 * Copyright (c) 2011-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qti.factory.Gyroscope;

import android.app.Activity;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.qti.factory.R;
import com.qti.factory.Utils;

public class Gyroscope extends Activity {

	private SensorManager GyroscopeManager = null;
	private Sensor mGyroscope = null;
	private GyroscopeListener mGyroscopeListener;
	TextView mTextView;
	Button cancelButton;
	private final static String INIT_VALUE = "";
	private static String value = INIT_VALUE;
	private static String pre_value = INIT_VALUE;
	private final int MIN_COUNT = 10;
	String TAG = "Gyroscope";
	private final static int SENSOR_TYPE = Sensor.TYPE_GYROSCOPE;
	private final static int SENSOR_DELAY = SensorManager.SENSOR_DELAY_FASTEST;

	@Override
	public void finish() {

		try {

			GyroscopeManager.unregisterListener(mGyroscopeListener, mGyroscope);
		} catch (Exception e) {
		}
		super.finish();
	}

	void bindView() {

		mTextView = (TextView) findViewById(R.id.gyroscope_result);
		cancelButton = (Button) findViewById(R.id.gyroscope_cancel);
		cancelButton.setOnClickListener(new View.OnClickListener() {

			public void onClick(View v) {

				fail(null);
			}
		});
	}

	void getService() {

		GyroscopeManager = (SensorManager) getSystemService(SENSOR_SERVICE);
		if (GyroscopeManager == null) {
			fail(getString(R.string.service_get_fail));
		}

		mGyroscope = GyroscopeManager.getDefaultSensor(SENSOR_TYPE);
		if (mGyroscope == null) {
			fail(getString(R.string.sensor_get_fail));
		}

		mGyroscopeListener = new GyroscopeListener(this);
		if (!GyroscopeManager.registerListener(mGyroscopeListener, mGyroscope,
				SENSOR_DELAY)) {
			fail(getString(R.string.sensor_register_fail));
		}
	}

	void updateView(Object s) {

		mTextView.setText(TAG + " : " + s);
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		setContentView(R.layout.gyroscope);

		bindView();
		getService();

		updateView(value);

	}

	void fail(Object msg) {

		loge(msg);
		toast(msg);
		setResult(RESULT_CANCELED);
		Utils.writeCurMessage(this, TAG, "Failed");
		finish();
	}

	void pass() {

		// toast(getString(R.string.test_pass));
		setResult(RESULT_OK);
		Utils.writeCurMessage(this, TAG, "Pass");
		finish();

	}

	@Override
	protected void onDestroy() {

		super.onDestroy();

		if (GyroscopeManager == null || mGyroscopeListener == null
				|| mGyroscope == null)
			return;
		GyroscopeManager.unregisterListener(mGyroscopeListener, mGyroscope);
	}

	public class GyroscopeListener implements SensorEventListener {

		private int count = 0;

		public GyroscopeListener(Context context) {

			super();
		}

		public void onSensorChanged(SensorEvent event) {

			// Gyroscope event.value has 3 equal value.
			synchronized (this) {
				if (event.sensor.getType() == SENSOR_TYPE) {
					logd(event.values.length + ":" + event.values[0] + " "
							+ event.values[0] + " " + event.values[0] + " ");
					String value = "(" + event.values[0] + ", "
							+ event.values[1] + ", " + event.values[2] + ")";
					updateView(value);
					if (value != pre_value)
						count++;
					if (count >= MIN_COUNT)
						pass();
					pre_value = value;
				}
			}
		}

		public void onAccuracyChanged(Sensor arg0, int arg1) {

		}
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
