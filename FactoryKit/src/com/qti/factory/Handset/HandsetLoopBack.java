/*
 * Copyright (c) 2011-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qti.factory.Handset;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioSystem;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

import com.qti.factory.R;
import com.qti.factory.Utils;

public class HandsetLoopBack extends Activity {

	static String TAG = "HandsetLoopBack";
	Button passButton;
	Button failButton;
	AudioManager mAudioManager;

	boolean mRunning = false;
	Context mContext;

	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		logd("");
		setContentView(R.layout.handset_loopback);
		init();
		setAudio();

		if (!mAudioManager.isWiredHeadsetOn()) {
			startLoopBackThread();
		} else {
			registerReceiver(broadcastReceiver, new IntentFilter(
					Intent.ACTION_HEADSET_PLUG));
			showWarningDialog(getString(R.string.remove_headset));
		}
	}

	@Override
	public void finish() {
		mAudioManager.setMode(AudioManager.MODE_NORMAL);
		AudioSystem.setForceUse(AudioSystem.FOR_COMMUNICATION,
				AudioSystem.FORCE_NONE);
		mRunning = false;
		super.finish();
	}

	void showWarningDialog(String title) {

		new AlertDialog.Builder(this)
				.setTitle(title)
				.setPositiveButton(getString(R.string.ok),
						new DialogInterface.OnClickListener() {

							public void onClick(DialogInterface dialog,
									int which) {

							}
						}).setCancelable(false).show();
	}

	public void setAudio() {

		mAudioManager.setMode(AudioManager.MODE_IN_COMMUNICATION);
		// mAudioManager.setSpeakerphoneOn(false);
		AudioSystem.setForceUse(AudioSystem.FOR_COMMUNICATION,
				AudioSystem.FORCE_HEADPHONES);

		float ratio = 1f;

		mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC,
				(int) (ratio * mAudioManager
						.getStreamMaxVolume(AudioManager.STREAM_MUSIC)), 0);
		mAudioManager
				.setStreamVolume(
						AudioManager.STREAM_VOICE_CALL,
						(int) (ratio * mAudioManager
								.getStreamMaxVolume(AudioManager.STREAM_VOICE_CALL)),
						0);
		mAudioManager.setStreamVolume(AudioManager.STREAM_RING,
				(int) (ratio * mAudioManager
						.getStreamMaxVolume(AudioManager.STREAM_RING)), 0);
		mAudioManager.setStreamVolume(AudioManager.STREAM_SYSTEM,
				(int) (ratio * mAudioManager
						.getStreamMaxVolume(AudioManager.STREAM_SYSTEM)), 0);

	}

	Runnable runnable = new Runnable() {

		final static int SAMPLE_RATE = 8000;

		public void run() {

			logd("LoopBack started");
			mRunning = true;

			int bufferSize = AudioRecord
					.getMinBufferSize(SAMPLE_RATE, AudioFormat.CHANNEL_IN_MONO,
							AudioFormat.ENCODING_PCM_16BIT);
			bufferSize = Math.max(bufferSize, AudioTrack.getMinBufferSize(
					SAMPLE_RATE, AudioFormat.CHANNEL_IN_MONO,
					AudioFormat.ENCODING_PCM_16BIT));

			AudioRecord audioRecord = new AudioRecord(
					MediaRecorder.AudioSource.MIC, SAMPLE_RATE,
					AudioFormat.CHANNEL_IN_MONO,
					AudioFormat.ENCODING_PCM_16BIT, bufferSize);

			AudioTrack audioTrack = new AudioTrack(
					AudioManager.STREAM_VOICE_CALL, SAMPLE_RATE,
					AudioFormat.CHANNEL_OUT_MONO,
					AudioFormat.ENCODING_PCM_16BIT, bufferSize,
					AudioTrack.MODE_STREAM);

			audioTrack.setPlaybackRate(SAMPLE_RATE);

			audioRecord.startRecording();
			audioTrack.play();

			byte[] buffer = new byte[bufferSize];
			while (mRunning) {
				int readSize = audioRecord.read(buffer, 0, bufferSize);
				if (readSize > 0)
					audioTrack.write(buffer, 0, readSize);
			}
			audioRecord.stop();
			audioRecord.release();
			audioTrack.stop();
			audioTrack.release();
		};
	};

	void init() {

		mContext = getApplicationContext();
		mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);

		passButton = (Button) findViewById(R.id.pass);
		failButton = (Button) findViewById(R.id.fail);

		passButton.setOnClickListener(new OnClickListener() {

			public void onClick(View v) {

				pass();
			}
		});

		failButton.setOnClickListener(new OnClickListener() {

			public void onClick(View v) {
				fail("");

			}
		});

	}

	BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context arg0, Intent intent) {
			String action = intent.getAction();
			logd(action);
			if (Intent.ACTION_HEADSET_PLUG.equals(action)) {
				logd(intent.getIntExtra("state", 0));
				if (intent.getIntExtra("state", 0) == 0) {
					startLoopBackThread();
					unregisterReceiver(this);
				}
			}

		}
	};

	void startLoopBackThread() {
		if (!mRunning)
			new Thread(runnable).start();
	}

	void fail(Object msg) {

		if (msg != null) {
			loge(msg);
			toast(msg);
		}
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

	@SuppressWarnings("unused")
	private void logd(Object s) {

		Thread mThread = Thread.currentThread();
		StackTraceElement[] mStackTrace = mThread.getStackTrace();
		String mMethodName = mStackTrace[3].getMethodName();

		s = "[" + mMethodName + "] " + s;
		Log.d(TAG, s + "");
	}

}
