/*
 * Copyright (c) 2011-2012, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */
package com.android.profile;

import static com.android.profile.ProfileConst.LOG;
import android.content.Context;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.provider.Settings;
import android.util.Log;

public class ProfileVolumePlayer {

    private final static String TAG = "ProfileVolumePlayer";
    private Context mContext;
    private AudioManager mAudioManager;
    private MediaPlayer mMediaPlayer;

    private Uri mUri = Settings.System.DEFAULT_RINGTONE_URI;
    private int mStreamType = AudioManager.STREAM_RING;
    private int mOriginalVolume = 0;
    private int mStreamVolume = 0;

    ProfileVolumePlayer(Context context, int streamType) {

        logd("");
        mContext = context;
        mAudioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
        mStreamType = streamType;
        mOriginalVolume = mAudioManager.getStreamVolume(mStreamType);

    }

    ProfileVolumePlayer(Context context) {

        mContext = context;
        mAudioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
        mOriginalVolume = mAudioManager.getStreamVolume(mStreamType);
    }

    public void setUri(Uri uri) {

        mUri = uri;
    }

    public void setStreamVolume(int streamVolume) {

        mStreamVolume = streamVolume;
        mAudioManager.setStreamVolume(mStreamType, mStreamVolume,
                AudioManager.FLAG_REMOVE_SOUND_AND_VIBRATE);
    }

    public void setStreamType(int streamType) {

        mStreamType = streamType;
    }

    public void preparePlayer() {

        if (mMediaPlayer == null)
            mMediaPlayer = new MediaPlayer();
        if (mUri != null && mMediaPlayer != null && !mMediaPlayer.isPlaying()) {
            try {
                mMediaPlayer.setDataSource(mContext, mUri);
                mMediaPlayer.setAudioStreamType(mStreamType);
                mMediaPlayer.prepare();
            } catch (Exception e) {
                loge(e);
            }
        }
    }

    public void play() {

        if (mMediaPlayer == null)
            preparePlayer();
        if (mMediaPlayer != null && mStreamVolume != 0 && mOriginalVolume != 0) {
            logd("Play Volume=" + mStreamVolume);
            mMediaPlayer.start();
        }
    }

    public void releasePlayer() {

        logd("");
        if (mMediaPlayer != null) {
            mMediaPlayer.reset();
            mMediaPlayer.release();
            mMediaPlayer = null;
        }
        revertOriginalVolume();
    }

    private void revertOriginalVolume() {

        mAudioManager.setStreamVolume(mStreamType, mOriginalVolume,
                AudioManager.FLAG_REMOVE_SOUND_AND_VIBRATE);
    }

    private static void logd(Object s) {

        if (!LOG)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.d(TAG, s + "");
    }

    private static void loge(Object e) {

        if (!LOG)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();
        e = "[" + mMethodName + "] " + e;
        Log.e(TAG, e + "");
    }

}
