/*
 * Copyright (c) 2011-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.factory.LightSensor;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;

import android.app.Activity;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.qti.factory.R;
import com.qti.factory.Utils;

public class LightSensor extends Activity {
	private SensorManager LightSensorManager = null;
	private Sensor mLightSensor = null;
	private LightSensorListener mLightSensorListener;
	TextView mTextView;
	TextView mAlsTextView;
	Button cancelButton;
	private final static String INIT_VALUE = "";
	private static String value = INIT_VALUE;
	private static String pre_value = INIT_VALUE;
	private final int MIN_COUNT = 4;
	String TAG = "LightSensor";
	String i2C_CMD = "i2cdetect -y -r 0 0x48 0x48";
	private static boolean WORKROUND = false;
	private final static int SENSOR_TYPE = Sensor.TYPE_LIGHT;
	private final static int SENSOR_DELAY = SensorManager.SENSOR_DELAY_FASTEST;

	@Override
	public void finish() {

		if (!WORKROUND)
			try {
				LightSensorManager.unregisterListener(mLightSensorListener,
						mLightSensor);
			} catch (Exception e) {
			}
		super.finish();
	}

	void exec(final String para) {

		new Thread() {

			public void run() {
				try {
					logd(para);

					Process mProcess;
					String paras[] = para.split(" ");
					for (int i = 0; i < paras.length; i++)
						logd(i + ":" + paras[i]);
					mProcess = Runtime.getRuntime().exec(paras);
					mProcess.waitFor();

					InputStream inStream = mProcess.getInputStream();
					InputStreamReader inReader = new InputStreamReader(inStream);
					BufferedReader inBuffer = new BufferedReader(inReader);
					String s;
					String data = "";
					while ((s = inBuffer.readLine()) != null) {
						data += s + "\n";
					}
					logd(data);
					int result = mProcess.exitValue();
					logd("ExitValue=" + result);
					Message message = new Message();
					if (data.contains("--"))
						message.obj = false;
					else
						message.obj = true;
					message.setTarget(mHandler);
					message.sendToTarget();

				} catch (Exception e) {
					logd(e);
				}

			}
		}.start();

	}

	Handler mHandler = new Handler() {
		public void dispatchMessage(android.os.Message msg) {
			boolean res = (Boolean) msg.obj;
			if (res)
				pass();
			else
				fail(null);

		};
	};

	void bindView() {

		mTextView = (TextView) findViewById(R.id.lightsensor_result);
		mAlsTextView = (TextView) findViewById(R.id.lightsensor_als);
		cancelButton = (Button) findViewById(R.id.lightsensor_cancel);
		cancelButton.setOnClickListener(new View.OnClickListener() {

			public void onClick(View v) {

				fail(null);
			}
		});
	}

	void getService() {

		LightSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
		if (LightSensorManager == null) {
			fail(getString(R.string.service_get_fail));
		}

		mLightSensor = LightSensorManager.getDefaultSensor(SENSOR_TYPE);
		if (mLightSensor == null) {
			fail(getString(R.string.sensor_get_fail));
		}

		mLightSensorListener = new LightSensorListener(this);
		if (!LightSensorManager.registerListener(mLightSensorListener,
				mLightSensor, SENSOR_DELAY)) {
			fail(getString(R.string.sensor_register_fail));
		}
	}

	void updateView(Object s, String alsData) {

		mTextView.setText(TAG + " : " + s);

		if ("".equals(alsData))
			mAlsTextView.setVisibility(View.GONE);
		else {
			mAlsTextView.setVisibility(View.VISIBLE);
			mAlsTextView.setText("ALS= " + alsData);
		}
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		if (WORKROUND) {
			exec(i2C_CMD);
		} else {
			setContentView(R.layout.lightsensor);
			bindView();
			getService();
			init();
			updateView(value, "");
		}
	}

	private void init() {
		value = INIT_VALUE;
		pre_value = INIT_VALUE;
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

		if (LightSensorManager == null || mLightSensorListener == null
				|| mLightSensor == null)
			return;
		LightSensorManager.unregisterListener(mLightSensorListener,
				mLightSensor);
	}

	public class LightSensorListener implements SensorEventListener {

		private int count = 0;

		public LightSensorListener(Context context) {

			super();
		}

		public void onSensorChanged(SensorEvent event) {

			// LightSensor event.value has 3 equal value.
			synchronized (this) {
				if (event.sensor.getType() == SENSOR_TYPE) {
					logd(event.values.length + ":" + event.values[0] + " "
							+ event.values[0] + " " + event.values[0] + " ");
					String value = "(" + event.values[0] + ", "
							+ event.values[1] + ", " + event.values[2] + ")";
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
