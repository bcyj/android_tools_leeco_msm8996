/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.timerswitch;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.PowerManager;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.view.Window;
import android.view.WindowManager;

import com.android.timerswitch.R;
import com.android.timerswitch.utils.Log;

public class ShutdownActivity extends Activity {
    public static final String TAG = ShutdownActivity.class.getSimpleName();

    public static CountDownTimer mCountDownTimer = null;
    private static TelephonyManager mTelephonyManager;
    private String mMessage;
    private long mSecondsCountdown;
    private PowerManager.WakeLock mWakeLock;
    private static final int POWER_OFF_DIALOG = 1;
    private static final long COUNT_SECONDS = 20;
    private static final long COUNT_DOWN_INTERVAL = 1000;
    private static final String SWITCH_OFF_MSG = "message";
    private static final String TIMER_LEFT_TIME = "lefttime";
    private static boolean mRenewDlgFlag = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG + ":onCreate");
        super.onCreate(savedInstanceState);
        acquireWakeLock();

        mSecondsCountdown = COUNT_SECONDS;
        if (null != savedInstanceState) {
            mSecondsCountdown = savedInstanceState.getLong(TIMER_LEFT_TIME);
            mMessage = savedInstanceState.getString(SWITCH_OFF_MSG);
        }
        mTelephonyManager = (TelephonyManager)getSystemService(Context.TELEPHONY_SERVICE);
        mCountDownTimer = new MyCountDownTimer(mSecondsCountdown * COUNT_DOWN_INTERVAL,
                COUNT_DOWN_INTERVAL);
        mCountDownTimer.start();
        mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_CALL_STATE);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putLong(TIMER_LEFT_TIME, mSecondsCountdown);
        outState.putString(SWITCH_OFF_MSG, mMessage);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        cancelCountDownTimer();
        if (!isFinishing()) {
            dismissDialogSafely(POWER_OFF_DIALOG);
        }
        releaseWakeLock();
    }

    private void acquireWakeLock() {
        if (mWakeLock == null) {
            PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
            mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
        }
        mWakeLock.acquire();
    }

    private void releaseWakeLock() {
        if (mWakeLock != null) {
            mWakeLock.release();
            mWakeLock = null;
        }
    }

    @Override
    protected void onPrepareDialog(int id, Dialog dialog, Bundle args) {
        ((AlertDialog) dialog).setMessage(mMessage);
        super.onPrepareDialog(id, dialog);
    }

    private void cancelCountDownTimer() {
        if (mCountDownTimer != null) {
            mCountDownTimer.cancel();
            mCountDownTimer = null;
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        AlertDialog switchOffDialog = new AlertDialog.Builder(this)
                .setCancelable(false)
                .setTitle(R.string.switch_off_dialog_title)
                .setMessage(mMessage)
                .setPositiveButton(R.string.switch_off_dialog_ok,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                cancelCountDownTimer();
                                startShutDown();
                            }
                        })
                .setNegativeButton(R.string.switch_off_dialog_cancel,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                mTelephonyManager.listen(mPhoneStateListener,
                                        PhoneStateListener.LISTEN_NONE);
                                dialog.cancel();
                                cancelCountDownTimer();
                                ShutdownActivity.this.finish();
                            }
                        }).create();
        switchOffDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
        return switchOffDialog;
    }

    private void startShutDown() {
        mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
        Intent intent = new Intent(Intent.ACTION_REQUEST_SHUTDOWN);
        intent.putExtra(Intent.EXTRA_KEY_CONFIRM, false);
        intent.setFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }

    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onCallStateChanged(int state, String ignored) {
            Log.d(TAG + "onCallStateChanged:" + state);
            if (state != TelephonyManager.CALL_STATE_IDLE && mTelephonyManager != null) {
                mRenewDlgFlag = true;
                if (!ShutdownActivity.this.isFinishing()) {
                    ShutdownActivity.this.setVisible(false);
                    ShutdownActivity.this.cancelCountDownTimer();
                    dismissDialogSafely(POWER_OFF_DIALOG);
                }
            } else {
                if (mRenewDlgFlag) {
                    mSecondsCountdown = COUNT_SECONDS;
                    mCountDownTimer = new MyCountDownTimer(mSecondsCountdown * COUNT_DOWN_INTERVAL,
                            COUNT_DOWN_INTERVAL);
                    mCountDownTimer.start();
                    mRenewDlgFlag = false;
                }
            }
        }
    };

    private void dismissDialogSafely(int id) {
        try {
            dismissDialog(id);
        } catch (IllegalArgumentException e) {
        }
    }
    public class MyCountDownTimer extends CountDownTimer {

        public MyCountDownTimer(long millisInFuture, long countDownInterval) {
            super(millisInFuture, countDownInterval);
        }

        public void onTick(long millisUntilFinished) {
            mSecondsCountdown = millisUntilFinished / COUNT_DOWN_INTERVAL;
            mMessage = (mSecondsCountdown > 1) ?
                    getString(R.string.switch_off_message, mSecondsCountdown) :
                    getString(R.string.switch_off_message_second, mSecondsCountdown);
               if (!isFinishing()) {
                   showDialog(POWER_OFF_DIALOG);
               }
        }

        public void onFinish() {
            startShutDown();
            mCountDownTimer = null;
            ShutdownActivity.this.finish();
        }
    }
}
