/*
 * Copyright (c) 2011-2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.factory.TransmitterReceiver;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.media.AudioManager;
import android.media.AudioSystem;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.factory.R;
import com.qualcomm.factory.Utilities;

public class TransmitterReceiver extends Activity {

    String mAudiofilePath;
    static String TAG = "TransmitterReceiver";
    MediaRecorder mMediaRecorder = new MediaRecorder();
    boolean isRecording = false;
    Button recordButton = null;
    Button stopButton = null;
    AudioManager mAudioManager;
    Context mContext;

    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.transmitter_receiver);

        mContext = this;
        isRecording = false;

        getService();
        bindView();

        // if (mAudioManager.isWiredHeadsetOn())
        // showWarningDialog(getString(R.string.remove_headset));
        setAudio();
    }

    @Override
    public void finish() {
        mAudioManager.setMode(AudioManager.MODE_NORMAL);
        // AudioSystem.setForceUse(AudioSystem.FOR_MEDIA,
        // AudioSystem.FORCE_NONE);
        super.finish();
    }

    void record() throws IllegalStateException, IOException, InterruptedException {

        mMediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
        mMediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.THREE_GPP);
        mMediaRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_WB);
        mMediaRecorder.setOutputFile(this.getCacheDir().getAbsolutePath() + "/test.amr");
        mAudiofilePath = this.getCacheDir().getAbsolutePath() + "/test.amr";
        mMediaRecorder.prepare();
        mMediaRecorder.start();
    }

    void replay() throws IllegalArgumentException, IllegalStateException, IOException {
        final TextView mTextView = (TextView) findViewById(R.id.transmitter_receiver_hint);
        mTextView.setText(getString(R.string.transmitter_receiver_playing));
        // mAudioManager.setMode(AudioManager.MODE_IN_CALL);
        // Replaying sound right now by record();
        stopButton.setClickable(false);
        File file = new File(mAudiofilePath);
        FileInputStream mFileInputStream = new FileInputStream(file);
        final MediaPlayer mMediaPlayer = new MediaPlayer();

        mMediaPlayer.reset();
        mMediaPlayer.setDataSource(mFileInputStream.getFD());
        mMediaPlayer.prepare();
        mMediaPlayer.start();

        mMediaPlayer.setOnCompletionListener(new OnCompletionListener() {

            public void onCompletion(MediaPlayer mPlayer) {

                mPlayer.stop();
                mPlayer.reset();
                mPlayer.release();
                File file = new File(mAudiofilePath);
                file.delete();
                
                final TextView mTextView = (TextView) findViewById(R.id.transmitter_receiver_hint);
                mTextView.setText(getString(R.string.transmitter_receiver_replay_end));
                showConfirmDialog();

            }
        });

    }

    void showWarningDialog(String title) {
        
        new AlertDialog.Builder(mContext).setTitle(title)
                .setPositiveButton(getString(R.string.ok), new DialogInterface.OnClickListener() {

                    public void onClick(DialogInterface dialog, int which) {

                    }
                }).setCancelable(false).show();

    }

    void showConfirmDialog() {
        
        new AlertDialog.Builder(mContext).setTitle(getString(R.string.transmitter_receiver_confirm))
                .setPositiveButton(getString(R.string.yes), new DialogInterface.OnClickListener() {
                    
                    public void onClick(DialogInterface dialog, int which) {
                        
                        pass();
                    }
                }).setNegativeButton(getString(R.string.no), new DialogInterface.OnClickListener() {
                    
                    public void onClick(DialogInterface dialog, int which) {
                        
                        fail(null);
                    }
                }).setCancelable(false).show();
    }

    public void setAudio() {

        mAudioManager.setMode(AudioManager.MODE_IN_COMMUNICATION);
        //mAudioManager.setMode(AudioManager.MODE_IN_CALL);
        // AudioSystem.setForceUse(AudioSystem.FOR_MEDIA,
        // AudioSystem.FORCE_HEADPHONES);
        float ratio = 1f;

        mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC,
                (int) (ratio * mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC)), 0);
        mAudioManager.setStreamVolume(AudioManager.STREAM_VOICE_CALL,
                (int) (ratio * mAudioManager.getStreamMaxVolume(AudioManager.STREAM_VOICE_CALL)), 0);
        mAudioManager.setStreamVolume(AudioManager.STREAM_RING,
                (int) (ratio * mAudioManager.getStreamMaxVolume(AudioManager.STREAM_RING)), 0);
        mAudioManager.setStreamVolume(AudioManager.STREAM_SYSTEM,
                (int) (ratio * mAudioManager.getStreamMaxVolume(AudioManager.STREAM_SYSTEM)), 0);

    }

    void bindView() {

        recordButton = (Button) findViewById(R.id.transmitter_receiver_start);
        stopButton = (Button) findViewById(R.id.transmitter_receiver_stop);
        final TextView mTextView = (TextView) findViewById(R.id.transmitter_receiver_hint);
        mTextView.setText(getString(R.string.transmitter_receiver_to_record));

        recordButton.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {

                if (!mAudioManager.isWiredHeadsetOn()) {

                    mTextView.setText(getString(R.string.transmitter_receiver_recording));
                    try {
                        recordButton.setClickable(false);
                        record();
                        isRecording = true;

                    } catch (Exception e) {
                        loge(e);
                    }
                } else
                    showWarningDialog(getString(R.string.remove_headset));

            }
        });

        stopButton.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {

                if (isRecording) {
                    mMediaRecorder.stop();
                    mMediaRecorder.reset();
                    mMediaRecorder.release();

                    try {
                        replay();
                    } catch (Exception e) {
                        loge(e);
                    }
                } else
                    showWarningDialog(getString(R.string.transmitter_receiver_record_first));
            }
        });
    }

    void getService() {

        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
    }

    void fail(Object msg) {

        loge(msg);
        toast(msg);
        setResult(RESULT_CANCELED);
        Utilities.writeCurMessage(this, TAG, "Failed");
        finish();
    }

    void pass() {

        setResult(RESULT_OK);
        Utilities.writeCurMessage(this, TAG, "Pass");
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
