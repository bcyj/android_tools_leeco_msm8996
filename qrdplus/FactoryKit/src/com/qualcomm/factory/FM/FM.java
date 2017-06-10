/*
 * Copyright (c) 2011-2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.factory.FM;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.media.AudioManager;
import android.media.AudioSystem;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.factory.R;
import com.qualcomm.factory.Utilities;

public class FM extends Activity {
    
    static String TAG = "FM";
    Button searchButton, passButton, failButton;
    TextView mTextView;
    AudioManager mAudioManager = null;
    FmManager mFmManager = null;
    Context mContext = null;
    Handler mHandler = new Handler() {
        public void handleMessage(android.os.Message msg) {
            if (msg.what == 0) {
                mTextView.setText(new Float(mFmManager.getFrequency() / 1000f).toString() + "MHZ");
            }
        };
    };
    
    @Override
    public void finish() {
        
        mFmManager.closeFM();
        AudioSystem.setDeviceConnectionState(AudioSystem.DEVICE_OUT_WIRED_HEADSET,
                AudioSystem.DEVICE_STATE_UNAVAILABLE, "");
        AudioSystem.setForceUse(AudioSystem.FOR_MEDIA, AudioSystem.FORCE_NONE);
        super.finish();
    }
    
    void getService() {
        
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        mFmManager = new FmManager(mContext, mHandler);
    }
    
    void bindView() {
        
        searchButton = (Button) findViewById(R.id.fm_search);
        passButton = (Button) findViewById(R.id.fm_pass);
        failButton = (Button) findViewById(R.id.fm_fail);
        mTextView = (TextView) findViewById(R.id.fm_frequency);
        mTextView.setText("87.5" + "MHZ");
        
        searchButton.setOnClickListener(new OnClickListener() {
            
            public void onClick(View v) {
                
                if (mAudioManager.isWiredHeadsetOn()) {
                    setButtonClickable(true);
                    mFmManager.searchUP();
                } else {
                    setButtonClickable(false);
                    showWarningDialog(getString(R.string.insert_headset));
                }
            }
        });
        
        passButton.setOnClickListener(new OnClickListener() {
            
            public void onClick(View v) {
                
                pass();
            }
        });
        
        failButton.setOnClickListener(new OnClickListener() {
            
            public void onClick(View v) {
                
                fail(null);
            }
        });
        
    }
    
    public void setAudio() {
        
        // Force headset's check
        AudioSystem.setDeviceConnectionState(AudioSystem.DEVICE_OUT_WIRED_HEADSET, AudioSystem.DEVICE_STATE_AVAILABLE,
                "");
        AudioSystem.setForceUse(AudioSystem.FOR_MEDIA, AudioSystem.FORCE_WIRED_ACCESSORY);
        mAudioManager.requestAudioFocus(null, AudioManager.STREAM_MUSIC, AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
        mAudioManager.setMode(AudioManager.MODE_NORMAL);
        float ratio = 0.3f;
        
        mAudioManager.setStreamVolume(AudioManager.STREAM_ALARM,
                (int) (ratio * mAudioManager.getStreamMaxVolume(AudioManager.STREAM_ALARM)), 0);
        mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC,
                (int) (ratio * mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC)), 0);
        mAudioManager.setStreamVolume(AudioManager.STREAM_VOICE_CALL,
                (int) (ratio * mAudioManager.getStreamMaxVolume(AudioManager.STREAM_VOICE_CALL)), 0);
        mAudioManager.setStreamVolume(AudioManager.STREAM_DTMF,
                (int) (ratio * mAudioManager.getStreamMaxVolume(AudioManager.STREAM_DTMF)), 0);
        mAudioManager.setStreamVolume(AudioManager.STREAM_NOTIFICATION,
                (int) (ratio * mAudioManager.getStreamMaxVolume(AudioManager.STREAM_NOTIFICATION)), 0);
        mAudioManager.setStreamVolume(AudioManager.STREAM_RING,
                (int) (ratio * mAudioManager.getStreamMaxVolume(AudioManager.STREAM_RING)), 0);
        mAudioManager.setStreamVolume(AudioManager.STREAM_SYSTEM,
                (int) (ratio * mAudioManager.getStreamMaxVolume(AudioManager.STREAM_SYSTEM)), 0);
    }
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        
        super.onCreate(savedInstanceState);
        mContext = this;
        
        setContentView(R.layout.fm);
        getService();
        setAudio();
        bindView();
        
        if (!mAudioManager.isWiredHeadsetOn()) {
            setButtonClickable(false);
            showWarningDialog(getString(R.string.insert_headset));
        }
        mAudioManager.setMode(AudioManager.MODE_NORMAL);
        mFmManager.openFM();
        
    }
    
    void showWarningDialog(String title) {
        
        new AlertDialog.Builder(mContext).setTitle(title)
                .setPositiveButton(getString(R.string.ok), new DialogInterface.OnClickListener() {
                    
                    public void onClick(DialogInterface dialog, int which) {
                        
                    }
                }).setCancelable(false).show();
        
    }
    
    void setButtonClickable(boolean cmd) {
        
        passButton.setClickable(cmd);
        passButton.setFocusable(cmd);
        failButton.setClickable(cmd);
        failButton.setFocusable(cmd);
        
    }
    
    @Override
    protected void onDestroy() {
        
        super.onDestroy();
    }
    
    void fail(Object msg) {
        
        loge(msg);
        toast(msg);
        setResult(RESULT_CANCELED);
        Utilities.writeCurMessage(mContext, TAG, "Failed");
        finish();
    }
    
    void pass() {
        
        setResult(RESULT_OK);
        Utilities.writeCurMessage(mContext, TAG, "Pass");
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
