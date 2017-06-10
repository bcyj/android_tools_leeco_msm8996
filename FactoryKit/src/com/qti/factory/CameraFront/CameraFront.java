/*
 * Copyright (c) 2011-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qti.factory.CameraFront;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.graphics.PixelFormat;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.ShutterCallback;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.Toast;

import com.qti.factory.R;
import com.qti.factory.Utils;
import com.qti.factory.Values;

public class CameraFront extends Activity implements SurfaceHolder.Callback {

	private Camera mCamera = null;
	private Button takeButton, passButton, failButton;
	private SurfaceView mSurfaceView;
	private SurfaceHolder mSurfaceHolder;
	private String resultString = Utils.RESULT_FAIL;
	final static String TAG = "CameraFront";
	private boolean useAutoFocus = false;
	private boolean firstTime = false;
	private static Context mContext = null;
	String i2C_CMD = "i2cdetect -y -r 6 0x42 0x42";
	private static boolean WORKROUND = false;

	@Override
	public void finish() {

		if (!WORKROUND)
			stopCamera();
		Utils.writeCurMessage(TAG, resultString);
		logd(resultString);
		super.finish();
	}

	void init() {
		mContext = getApplicationContext();
		firstTime = true;
		int index = getIntent().getIntExtra(Values.KEY_SERVICE_INDEX, -1);
		if (index > 0)
			useAutoFocus = Utils.getBoolPara(index, "AutoFocus", false);
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

	@Override
	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		init();
		if (WORKROUND) {
			exec(i2C_CMD);
		} else {

			getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
					WindowManager.LayoutParams.FLAG_FULLSCREEN);
			requestWindowFeature(Window.FEATURE_NO_TITLE);
			setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
			setContentView(R.layout.camera_front);
			setResult(RESULT_CANCELED);
			/* SurfaceHolder set */
			mSurfaceView = (SurfaceView) findViewById(R.id.mSurfaceView);
			mSurfaceHolder = mSurfaceView.getHolder();
			mSurfaceHolder.addCallback(CameraFront.this);
			mSurfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

			bindView();
		}

	}

	void bindView() {

		takeButton = (Button) findViewById(R.id.take_picture);
		passButton = (Button) findViewById(R.id.camera_pass);
		failButton = (Button) findViewById(R.id.camera_fail);
		takeButton.setOnClickListener(new Button.OnClickListener() {

			public void onClick(View arg0) {

				takeButton.setVisibility(View.GONE);
				try {
					if (mCamera != null) {
						if (useAutoFocus)
							mCamera.autoFocus(new AutoFocusCallback());
						else
							takePicture();
					} else {
						finish();
					}
				} catch (Exception e) {
					fail(getString(R.string.autofocus_fail));
					loge(e);
				}
			}
		});

		passButton.setOnClickListener(new Button.OnClickListener() {

			public void onClick(View arg0) {

				setResult(RESULT_OK);
				Utils.writeCurMessage(mContext, TAG, "Pass");
				finish();
			}
		});
		failButton.setOnClickListener(new Button.OnClickListener() {

			public void onClick(View arg0) {

				setResult(RESULT_CANCELED);
				Utils.writeCurMessage(mContext, TAG, "Failed");
				finish();
			}
		});

	}

	public void surfaceCreated(SurfaceHolder surfaceholder) {
		logd("surfaceCreated");
		int oritationAdjust = 0;
		// oritationAdjust = 180;
		try {
			mCamera = Camera.open(Camera.CameraInfo.CAMERA_FACING_FRONT);
			mCamera.setDisplayOrientation(oritationAdjust);
		} catch (Exception exception) {
			toast(getString(R.string.cameraback_fail_open));
			mCamera = null;
		}

		if (mCamera == null) {
			fail(null);
		} else {
			try {
				mCamera.setPreviewDisplay(mSurfaceHolder);
			} catch (IOException exception) {
				mCamera.release();
				mCamera = null;
				finish();
			}
		}
	}

	public void surfaceChanged(SurfaceHolder surfaceholder, int format, int w,
			int h) {

		logd("surfaceChanged");
		startCamera();
	}

	public void surfaceDestroyed(SurfaceHolder surfaceholder) {

		logd("surfaceDestroyed");
		stopCamera();
	}

	private void takePicture() {

		logd("takePicture");
		if (mCamera != null) {
			try {
				mCamera.takePicture(mShutterCallback, rawPictureCallback,
						rawPictureCallback, jpegCallback);
			} catch (Exception e) {
				loge(e);
				finish();
			}
		} else {
			loge("Camera null");
			finish();
		}
	}

	private ShutterCallback mShutterCallback = new ShutterCallback() {

		public void onShutter() {

			logd("onShutter");
			try {
				takeButton.setVisibility(View.GONE);
				passButton.setVisibility(View.VISIBLE);
				failButton.setVisibility(View.VISIBLE);
			} catch (Exception e) {
				loge(e);
			}
		}
	};

	private PictureCallback rawPictureCallback = new PictureCallback() {

		public void onPictureTaken(byte[] _data, Camera _camera) {
			logd("rawPictureCallback onPictureTaken");
			try {
				takeButton.setVisibility(View.GONE);
				passButton.setVisibility(View.VISIBLE);
				failButton.setVisibility(View.VISIBLE);
			} catch (Exception e) {
				loge(e);
			}
		}
	};

	private PictureCallback jpegCallback = new PictureCallback() {

		public void onPictureTaken(byte[] _data, Camera _camera) {
			logd("jpegCallback onPictureTaken");
			try {
				takeButton.setVisibility(View.GONE);
				passButton.setVisibility(View.VISIBLE);
				failButton.setVisibility(View.VISIBLE);
			} catch (Exception e) {
				loge(e);
			}
		}
	};

	public final class AutoFocusCallback implements
			android.hardware.Camera.AutoFocusCallback {

		public void onAutoFocus(boolean focused, Camera camera) {

			if (focused) {
				takePicture();
			} else
				fail(getString(R.string.autofocus_fail));
		}
	};

	private void startCamera() {

		if (mCamera != null) {
			try {
				Camera.Parameters parameters = mCamera.getParameters();
				parameters.setPictureFormat(PixelFormat.JPEG);
				// parameters.setFlashMode(Camera.Parameters.FLASH_MODE_ON);
				// parameters.setRotation(CameraInfo.CAMERA_FACING_FRONT);
				mCamera.setParameters(parameters);
				mCamera.startPreview();
			} catch (Exception e) {
				loge(e);
			}
		}

	}

	private void stopCamera() {

		if (mCamera != null) {
			try {
				if (mCamera.previewEnabled())
					mCamera.stopPreview();
				mCamera.release();
			} catch (Exception e) {
				e.printStackTrace();
			}
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

	private void logd(Object d) {

		Thread mThread = Thread.currentThread();
		StackTraceElement[] mStackTrace = mThread.getStackTrace();
		String mMethodName = mStackTrace[3].getMethodName();

		d = "[" + mMethodName + "] " + d;
		Log.d(TAG, d + "");
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

	public void toast(Object s) {

		if (s == null)
			return;
		Toast.makeText(this, s + "", Toast.LENGTH_SHORT).show();
	}
}
