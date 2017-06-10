/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 *
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.borqs.videocall;

import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.os.Vibrator;
import android.util.Log;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;

/**
 * Ringer manager for the Phone app.
 */
public class Ringer {

	private static final boolean DBG = true;

	private static final int PLAY_RING_ONCE = 1;
	private static final int PLAY_RING_LOOP = 2;
	private static final int STOP_RING = 3;

	private static final int VIBRATE_LENGTH = 1000; // ms
	private static final int PAUSE_LENGTH = 1000; // ms

	private static final String TAG = "Ringer";

	// Uri for the ringtone.
	Uri mCustomRingtoneUri;

	Ringtone mRingtone;
	//Vibrator mVibrator = new Vibrator();
	Vibrator mVibrator  = null;// (Vibrator)getSystemService(VIBRATOR_SERVICE);
	volatile boolean mContinueVibrating;
	VibratorThread mVibratorThread;
    PlayRingThread mPlayRingThread;
	Context mContext;
	private Worker mRingThread;
	private Handler mRingHandler;
	private boolean mRingPending;
	private long mFirstRingEventTime = -1;
	private long mFirstRingStartTime = -1;

	Ringer(Context context) {
		mContext = context;
         mVibrator = (Vibrator)mContext.getSystemService(Context.VIBRATOR_SERVICE);
	}

	/**
	 * @return true if we're playing a ringtone and/or vibrating to indicate
	 *         that there's an incoming call. ("Ringing" here is used in the
	 *         general sense. If you literally need to know if we're playing a
	 *         ringtone or vibrating, use isRingtonePlaying() or isVibrating()
	 *         instead.)
	 *
	 * @see isVibrating
	 * @see isRingtonePlaying
	 */
	boolean isRinging() {
		synchronized (this) {
			return (isRingtonePlaying() || isVibrating());
		}
	}

	/**
	 * @return true if the ringtone is playing
	 * @see isVibrating
	 * @see isRinging
	 */
	private boolean isRingtonePlaying() {
		synchronized (this) {
			return (mRingtone != null && mRingtone.isPlaying())
					|| (mRingHandler != null && (mRingHandler
							.hasMessages(PLAY_RING_ONCE) || mRingHandler
							.hasMessages(PLAY_RING_LOOP)));
		}
	}

	/**
	 * @return true if we're vibrating in response to an incoming call
	 * @see isVibrating
	 * @see isRinging
	 */
	private boolean isVibrating() {
		synchronized (this) {
			return (mVibratorThread != null);
		}
	}

	/**
	 * Starts the ringtone and/or vibrator Note, now this func will play
	 * ringtone repeatly by using MediaPlayer.setLoop()
	 */
	void ring() {
		synchronized (this) {
			if (MyLog.DEBUG) MyLog.d(TAG, "ring");
			if (shouldVibrate() && mVibratorThread == null) {
				if (MyLog.DEBUG) MyLog.d(TAG, "should vibrate");
				mContinueVibrating = true;
				mVibratorThread = new VibratorThread();
				mVibratorThread.start();
			}
			AudioManager audioManager = (AudioManager) mContext
					.getSystemService(Context.AUDIO_SERVICE);

			if (audioManager.getStreamVolume(AudioManager.STREAM_RING) == 0) {
				if (MyLog.DEBUG) MyLog.d(TAG, "Volume is 0");
				return;
			}

			if (MyLog.DEBUG) MyLog.d(TAG, "isRingtoneplaying? " + isRinging() + " mringpending? " + mRingPending);
			if (!isRingtonePlaying() && !mRingPending) {
				if(mPlayRingThread==null)
				{
			    mPlayRingThread = new PlayRingThread();
				}
				makeLooper();
				mRingHandler.removeCallbacksAndMessages(null);
				mRingPending = true;
				// now do not control the repeat play here, instead use
				// MediaPlayer.setLoop
				if (false) {
					if (mFirstRingEventTime < 0) {
						mFirstRingEventTime = SystemClock.elapsedRealtime();
						mRingHandler.sendEmptyMessage(PLAY_RING_ONCE);
					} else {
						// For repeat rings, figure out by how much to delay
						// the ring so that it happens the correct amount of
						// time after the previous ring
						if (mFirstRingStartTime > 0) {
							// Delay subsequent rings by the delta between event
							// and play time of the first ring

							mRingHandler.sendEmptyMessageDelayed(
									PLAY_RING_ONCE, mFirstRingStartTime
											- mFirstRingEventTime);
						} else {
							// We've gotten two ring events so far, but the ring
							// still hasn't started. Reset the event time to the
							// time of this event to maintain correct spacing.
							mFirstRingEventTime = SystemClock.elapsedRealtime();
						}
					}
				}
				if (MyLog.DEBUG) MyLog.d(TAG, "sendemptymessage, play_ring_loop");
				mRingHandler.sendEmptyMessage(PLAY_RING_LOOP);
			} else {
				if (MyLog.DEBUG) MyLog.d(TAG, "tmp else");
			}
		}
	}

	boolean shouldVibrate() {
           AudioManager audioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
           int ringerMode = audioManager.getRingerMode();
           if (getVibrateWhenRinging(mContext)) {
              return ringerMode != AudioManager.RINGER_MODE_SILENT;
           } else {
              return ringerMode == AudioManager.RINGER_MODE_VIBRATE;
           }
	}

    /**
     * Obtain the setting for "vibrate when ringing" setting.
     *
     * Watch out: if the setting is missing in the device, this will try obtaining the old
     * "vibrate on ring" setting from AudioManager, and save the previous setting to the new one.
     */
    public static boolean getVibrateWhenRinging(Context context) {
        Vibrator vibrator = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
        if (vibrator == null || !vibrator.hasVibrator()) {
            return false;
        }
        return Settings.System.getInt(context.getContentResolver(),
                Settings.System.VIBRATE_WHEN_RINGING, 0) != 0;
    }


	/**
	 * Stops the ringtone and/or vibrator if any of these are actually
	 * ringing/vibrating.
	 */
	void stopRing() {
		if (MyLog.DEBUG) MyLog.d(TAG, "stopRing()...");
		synchronized (this) {
			if (mRingHandler != null) {
				mRingHandler.removeCallbacksAndMessages(null);
				Message msg = mRingHandler.obtainMessage(STOP_RING);
				msg.obj = mRingtone;
				mRingHandler.sendMessage(msg);
				mRingThread = null;
				mRingHandler = null;
				if(mRingtone!=null)
				{
				mRingtone.stop();
				mRingtone = null;
				}
				mFirstRingEventTime = -1;
				mFirstRingStartTime = -1;
				mRingPending = false;
			}
			if (mVibratorThread != null) {
				mContinueVibrating = false;
				mVibratorThread = null;
            }
            if(mPlayRingThread != null){
                mPlayRingThread.interrupt();
                mPlayRingThread = null;
            }
			// Also immediately cancel any vibration in progress.
			if (shouldVibrate()) {
				if (MyLog.DEBUG) MyLog.d(TAG, "cancel vibrator");
				mVibrator.cancel();
			}
		}
	}

    private class PlayRingThread extends Thread {
        private long mDelay = 0 ;
        public void setDelay(long delayed){
            mDelay=delayed;
        }
        public void run() {
         try{
		 Thread.sleep(1000); //Wait for videocallscreen launching
Log.e(TAG,"VideoCallApp.INTENT_ACTION_STARTRING_VTCALL="+VideoCallApp.INTENT_ACTION_STARTRING_VTCALL);
		 Intent i = new Intent(VideoCallApp.INTENT_ACTION_STARTRING_VTCALL);
			 VideoCallApp.getInstance().getApplicationContext().sendBroadcast(i);
            while (true) {
                if(isRingtonePlaying()){
                        Thread.sleep(1000);
                } else {
                    if(mRingtone != null)
                        mRingtone.play();
                }
            }
         }
       catch(InterruptedException ie){
                if (MyLog.DEBUG) MyLog.d(TAG, "Interrupted to stop ring");
            }

        }
    }

	private class VibratorThread extends Thread {
		public void run() {
			try
			{
				Thread.sleep(2000);//Wait for videocallscreen launching
Log.e(TAG,"VideoCallApp.INTENT_ACTION_STARTRING_VTCALL="+VideoCallApp.INTENT_ACTION_STARTRING_VTCALL);
				Intent i = new Intent(VideoCallApp.INTENT_ACTION_STARTRING_VTCALL);
				VideoCallApp.getInstance().getApplicationContext().sendBroadcast(i);
				while (mContinueVibrating) {
					mVibrator.vibrate(VIBRATE_LENGTH);
					SystemClock.sleep(VIBRATE_LENGTH + PAUSE_LENGTH);
				}
			}
			 catch(InterruptedException ie){
	                if (MyLog.DEBUG) MyLog.d(TAG, "Interrupted to stop Vibrator");
	            }
		}
	}

	private class Worker implements Runnable {
		private final Object mLock = new Object();
		private Looper mLooper;

		Worker(String name) {
			Thread t = new Thread(null, this, name);
			t.start();
			synchronized (mLock) {
				while (mLooper == null) {
					try {
						mLock.wait();
					} catch (InterruptedException ex) {
					}
				}
			}
		}

		public Looper getLooper() {
			return mLooper;
		}

		public void run() {
			synchronized (mLock) {
				Looper.prepare();
				mLooper = Looper.myLooper();
				mLock.notifyAll();
			}
			if (MyLog.DEBUG) MyLog.d(TAG, "continue to loop");
			Looper.loop();
		}

		public void quit() {
			mLooper.quit();
		}
	}

	/**
	 * set the ringtone uri in preparation for ringtone creation in
	 * makeLooper(). This uri is defaulted to the phone-wide default ringtone.
	 */
	void setCustomRingtoneUri(Uri uri) {
		if (uri != null) {
			mCustomRingtoneUri = uri;
		}
	}

	private void makeLooper() {
		if (MyLog.DEBUG) MyLog.d(TAG, "makeLooper...");
		if (mRingThread == null) {
			mRingThread = new Worker("ringer");
			mRingHandler = new Handler(mRingThread.getLooper()) {
				@Override
				public void handleMessage(Message msg) {
					if (MyLog.DEBUG) MyLog.d(TAG, "handler message: " + msg);
					Ringtone r = null;
					switch (msg.what) {
					case PLAY_RING_ONCE:
						if (MyLog.DEBUG) MyLog.d(TAG, "handle PLAY_RING_ONCE");
					case PLAY_RING_LOOP:
						if (MyLog.DEBUG) MyLog.d(TAG, "PLAY_RING_LOOP");
						if (mRingtone == null && !hasMessages(STOP_RING)) {
							// create the ringtone with the uri
							r = RingtoneManager.getRingtone(mContext,
									mCustomRingtoneUri);
							synchronized (Ringer.this) {
								if (!hasMessages(STOP_RING)) {
									mRingtone = r;
								}
							}
						}
						if (msg.what == PLAY_RING_ONCE) {
							if (mRingtone != null && !hasMessages(STOP_RING)) {
								mRingtone.play();
								synchronized (Ringer.this) {
									mRingPending = false;
									if (mFirstRingStartTime < 0) {
										mFirstRingStartTime = SystemClock
												.elapsedRealtime();
									}
								}
							}
						} else {
							if (mRingtone != null && !hasMessages(STOP_RING)) {
								if (MyLog.DEBUG) MyLog.d(TAG, "play ringtone, mRingtone: " + mRingtone);
								//mRingtone.play();
								if(!mPlayRingThread.isAlive())
                                mPlayRingThread.start();
								synchronized (Ringer.this) {
									mRingPending = false;
								}
							}
						}
						break;

					case STOP_RING:
						if (MyLog.DEBUG) MyLog.d(TAG, "quit looper");
                        if (null != getLooper()) {
                            if(mPlayRingThread != null){
                                mPlayRingThread.interrupt();
                                mPlayRingThread = null;
                            }
                            getLooper().quit();
                        }
						break;
					}
				}
			};
		}
	}
}
