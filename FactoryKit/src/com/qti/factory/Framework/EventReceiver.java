/*
 * Copyright (c) 2011-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qti.factory.Framework;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.util.Log;

import com.android.internal.telephony.TelephonyIntents;
import com.qti.factory.Utils;
import com.qti.factory.Values;

public class EventReceiver extends BroadcastReceiver {
	private static final String TAG = "EventReceiver";

	@Override
	public void onReceive(final Context context, Intent intent) {
		String action = intent.getAction();
		logd(action);
		if (Values.FACTORY_MODE) {
			if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
				if (Values.FACTORY_MODE)
					Utils.createShortcut(context, Framework.class);
				// configSoundEffects(context, false);
			} else if (TelephonyIntents.SECRET_CODE_ACTION.equals(action)) {
				Intent logkitIntent = new Intent(context, Framework.class);
				context.startService(logkitIntent);
			}
		}
	}

	private void configSoundEffects(Context context, boolean enable) {
		AudioManager mAudioManager = (AudioManager) context
				.getSystemService(Context.AUDIO_SERVICE);
		if (enable)
			mAudioManager.loadSoundEffects();
		else
			mAudioManager.unloadSoundEffects();
	}

	private static void logd(Object s) {

		Thread mThread = Thread.currentThread();
		StackTraceElement[] mStackTrace = mThread.getStackTrace();
		String mMethodName = mStackTrace[3].getMethodName();

		s = "[" + mMethodName + "] " + s;
		Log.d(TAG, s + "");
	}

	private static void loge(Object e) {

		Thread mThread = Thread.currentThread();
		StackTraceElement[] mStackTrace = mThread.getStackTrace();
		String mMethodName = mStackTrace[3].getMethodName();

		e = "[" + mMethodName + "] " + e;
		Log.e(TAG, e + "");
	}
}
