/*
 * Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.dm;

import android.app.Activity;
import android.os.Bundle;
import android.view.Gravity;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.Toast;
import android.content.SharedPreferences;
import android.content.Context;
import android.util.Log;
import android.content.Intent;
import com.android.dm.vdmc.Vdmc;
//import com.android.dm.vdmc.VdmcFumoHandler;
//import com.android.dm.vdmc.MyConfirmation;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.graphics.Color;
//import com.redbend.vdm.*;
import android.app.Notification;
import android.app.NotificationManager;
import android.os.Handler;
import java.lang.Runnable;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.app.KeyguardManager;
import android.app.KeyguardManager.KeyguardLock;
import com.android.dm.DMNativeMethod;
import android.text.format.DateFormat;
import android.media.AudioManager;

public class DmAlertDialog extends Activity {
    private String TAG = DmReceiver.DM_TAG + "DmAlertDialog: ";
    private Context mContext = null;
    private static DmAlertDialog mInstance = null;
    private NotificationManager mNotificationMgr = null;
    private int mDialogId = Vdmc.DM_NULL_DIALOG;

    private WakeLock mWakelock = null;
    private KeyguardLock mKeyguardLock = null;
    private boolean isScreenLock = false;
    private boolean isTimerActive = false;

    private AlertDialog builder = null;

    private Handler handler = new Handler();
    private Runnable runnable = new Runnable() {

        public void run() {
            stopTimer(); // stop timer

            // close alert dialog
            Log.d(TAG, "run() : mDialogId = " + mDialogId);
            switch (mDialogId)
            {
                case Vdmc.DM_NIA_INFO_DIALOG:
                    // DmJniInterface.startPppConnect();//start dial up
                    // DMNativeMethod.JVDM_notifyNIASessionProceed();
                    notifyNIASessionProceed();

                    builder.dismiss();

                    finish();
                    DestroyAlertDialog();
                    break;

                case Vdmc.DM_NIA_CONFIRM_DIALOG:
                    DMNativeMethod.JMMIDM_ExitDM();
                    // Vdmc.getInstance().stopVDM();
                    builder.dismiss();
                    finish();
                    DestroyAlertDialog();
                    break;

                case Vdmc.DM_ALERT_CONFIRM_DIALOG:
                    handleTimeoutEvent();
                    // MyConfirmation.getInstance().handleTimeoutEvent();
                    // //cancel dm session
                    break;

                case Vdmc.DM_DATACONNECT_DIALOG:
                    Toast.makeText(DmAlertDialog.this,
                            getString(R.string.dataconnect_query_timeout), Toast.LENGTH_SHORT)
                            .show();
                    if (builder!=null) {
                        if(builder instanceof DmTipDialog)
                            ((DmTipDialog) builder).superDismiss();
                        else
                            builder.dismiss();
                    }
                    break;

                default:
                    Log.d(TAG, "run() : mDialogId is invalid ");
                    break;
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // setContentView(R.layout.main);
        // getWindow().setBackgroundDrawable(null);
        mContext = this;
        mInstance = this;
        mNotificationMgr = (NotificationManager) mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);

        /*
         * //forbid sleep int flag =
         * WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON |
         * WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON |
         * WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED |
         * WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD;
         * getWindow().setFlags(flag, flag);
         * getWindow().setContentView(R.layout.alert_dialog);
         */

        Intent intent = getIntent();

        int id = intent.getIntExtra("dialogId", Vdmc.DM_NULL_DIALOG);
        String msg = intent.getStringExtra("message");
        int timeout = intent.getIntExtra("timeout", 60); // default 1min
        int slotid = intent.getIntExtra("slotid", 0);

        Log.d(TAG, "OnCreate: mContext = " + mContext);

        Log.d(TAG, "OnCreate: id = " + id);
        Log.d(TAG, "OnCreate: msg = " + msg);
        Log.d(TAG, "OnCreate: timeout = " + timeout);
        Log.d(TAG, "OnCreate: slotid = " + slotid);

        CreateAlertDialog(id, msg, timeout, slotid);
        mDialogId = id;
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        super.onDestroy();
        stopTimer(); // if switch to horizontal display, the window will auto
                     // close by system, and create new window, need stop timer
        if (mNotificationMgr != null)
        {
            mNotificationMgr.cancel(100);
            mNotificationMgr = null;
        }
        /*
         * mContext = null; mInstance = null; mNotificationMgr.cancel(100);
         * mNotificationMgr = null; stopTimer(); releaseUnlock();
         */
    }

    @Override
    public void onStart() {
        super.onStart();
        getUnlock();
    }

    @Override
    public void onStop() {
        super.onStop();
        releaseUnlock();
    }

    // do some reset operation
    // finish() is asynchronous function, onDestroy() sometimes comes too
    // late,will result error
    private void DestroyAlertDialog() {
        Log.d(TAG, "DestroyAlertDialog !!!!!!!!!!!!!!!!!!!!!!!!");
        mContext = null;
        mInstance = null;
        if (mNotificationMgr != null) {
            mNotificationMgr.cancel(100);
            mNotificationMgr = null;
        }

        stopTimer();
        releaseUnlock();
    }

    public static DmAlertDialog getInstance()
    {
        return mInstance;
    }

    public void finishDialog()
    {
        finish();
        DestroyAlertDialog();
    }

    private synchronized void getUnlock() {
        // acquire wake lock
        if (mWakelock == null || !mWakelock.isHeld()) {
            PowerManager pm = (PowerManager) mContext.getSystemService(
                    Context.POWER_SERVICE);
            mWakelock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK
                    | PowerManager.ACQUIRE_CAUSES_WAKEUP
                    | PowerManager.ON_AFTER_RELEASE, "SimpleTimer");
            // no need for reference count : getUnlock just called in oncreate
            // and so we just need concern the timing for releaseUnlock
            mWakelock.setReferenceCounted(false);
            mWakelock.acquire();
            Log.d(TAG, "getUnlock: acquire Wakelock");
        }

        // unlock screen
        KeyguardManager km = (KeyguardManager) mContext.getSystemService(Context.KEYGUARD_SERVICE);
        mKeyguardLock = km.newKeyguardLock(TAG);
        if (km.inKeyguardRestrictedInputMode()) {
            Log.d(TAG, "getUnlock: disable keyguard");
            mKeyguardLock.disableKeyguard();
            isScreenLock = true;
        } else {
            isScreenLock = false;
        }
    }

    private synchronized void releaseUnlock() {
        // release screen
        if (isScreenLock) {
            Log.d(TAG, "releaseUnlock: reenable Keyguard");
            mKeyguardLock.reenableKeyguard();
            isScreenLock = false;
        }
        // release wake lock
        if (mWakelock!=null && mWakelock.isHeld()) {
            Log.d(TAG, "releaseUnlock: release Wakelock");
            mWakelock.release();
        }
    }

    private void playAlertSound()
    {
        Log.d(TAG, "playAlertSound: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        Notification n = new Notification();

        n.flags |= Notification.FLAG_ONLY_ALERT_ONCE;
        n.defaults = Notification.DEFAULT_SOUND;

        AudioManager audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        boolean nowSilent = audioManager.getRingerMode() != AudioManager.RINGER_MODE_NORMAL;
        if (nowSilent) {
            n.defaults |= Notification.DEFAULT_VIBRATE;
        }

        mNotificationMgr.notify(100, n);
    }

    private void startTimer(int timeout) // timeout : s
    {
        Log.d(TAG, "startTimer: timeout = " + timeout);

        // convert timout to ms
        isTimerActive = handler.postDelayed(runnable, timeout * 1000);// after
                                                                      // timeout's
                                                                      // operate
                                                                      // runnable
    }

    private void stopTimer()
    {
        Log.d(TAG, "stopTimer: isTimerActive = " + isTimerActive);
        if (isTimerActive)
        {
            handler.removeCallbacks(runnable);
            isTimerActive = false;
        }
    }

    private void CreateAlertDialog(int id, String message, int timeout, int slotid)
    {
        Log.d(TAG, "CreateAlertDialog: id = " + id);
        switch (id)
        {
            case Vdmc.DM_NULL_DIALOG:
                Log.d(TAG, "CreateAlertDialog: DM_NULL_DIALOG");
                break;

            case Vdmc.DM_NIA_INFO_DIALOG:
                String notify_msg = Vdmc.getAppContext().getResources()
                        .getString(R.string.notify_msg);
                Log.d(TAG, "CreateAlertDialog: DM_NIA_INFO_DIALOG");
                createNiaInformDialog(notify_msg, timeout);
                break;

            case Vdmc.DM_NIA_CONFIRM_DIALOG:
                String notify_confirm_msg = Vdmc.getAppContext().getResources()
                        .getString(R.string.notify_confirm_msg);
                Log.d(TAG, "CreateAlertDialog: DM_NIA_CONFIRM_DIALOG");
                createNiaConfirmDialog(notify_confirm_msg, timeout);
                break;

            case Vdmc.DM_ALERT_CONFIRM_DIALOG:
                Log.d(TAG, "CreateAlertDialog: DM_ALERT_CONFIRM_DIALOG");
                createAlertConfirmDialog(message, timeout);
                break;

            case Vdmc.DM_CONFIRM_DOWNLOAD_DIALOG:
                Log.d(TAG, "CreateAlertDialog: DM_CONFIRM_DOWNLOAD_DIALOG");
                // Vdmc.getFumoHandler().createDialog(mContext,
                // VdmcFumoHandler.CONFIRM_DOWNLOAD).show();
                break;

            case Vdmc.DM_CONFIRM_UPDATE_DIALOG:
                Log.d(TAG, "CreateAlertDialog: DM_CONFIRM_UPDATE_DIALOG");
                // Vdmc.getFumoHandler().createDialog(mContext,
                // VdmcFumoHandler.CONFIRM_UPDATE).show();
                break;

            case Vdmc.DM_SIMULATE_UPDATE_DIALOG:
                Log.d(TAG, "CreateAlertDialog: DM_CONFIRM_UPDATE_DIALOG");
                // Vdmc.getFumoHandler().createDialog(mContext,
                // VdmcFumoHandler.SIMULATE_UPDATE).show();
                break;

            case Vdmc.DM_PROGRESS_DIALOG:
                Log.d(TAG, "CreateAlertDialog: DM_PROGRESS_DIALOG");
                break;

            case Vdmc.DM_SELFREGIST_DIALOG:
                Log.d(TAG, "CreateAlertDialog: DM_SELFREGIST_DIALOG");
                createAlertSelfRegistConfirmDialog(message, timeout, slotid);
                break;
            case Vdmc.DM_DATACONNECT_DIALOG:
                Log.d(TAG, "CreateAlertDialog: DM_DATACONNECT_DIALOG");
                createAlertDataConnectConfirmDialog(message, timeout);
                break;
            default:
                break;
        }
    }

    private void createNiaInformDialog(String message, int timeout)
    {
        String softkey_ok = getResources().getString(R.string.menu_ok);

        playAlertSound();

        builder = new AlertDialog.Builder(mContext)
                .setTitle("")
                .setMessage(message)
                .setPositiveButton(softkey_ok, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        Log.d(TAG, "createNiaInformDialog: onClick OK, notifyNIASessionProceed");
                        finish();
                        DestroyAlertDialog();
                        // start dial up
                        // DmJniInterface.startPppConnect();
                        // DMNativeMethod.JVDM_notifyNIASessionProceed();
                        notifyNIASessionProceed();

                    }
                })

                .setOnCancelListener(new DialogInterface.OnCancelListener() {
                    public void onCancel(DialogInterface dialog) {
                        Log.d(TAG, "createNiaInformDialog: onCancel, notifyNIASessionProceed");
                        finish();
                        DestroyAlertDialog();
                        // start dial up
                        // DmJniInterface.startPppConnect();
                        // DMNativeMethod.JVDM_notifyNIASessionProceed();
                        notifyNIASessionProceed();
                    }
                })

                .show();

        startTimer(timeout);
    }

    private void createNiaConfirmDialog(String message, int timeout)
    {
        String softkey_yes = getResources().getString(R.string.menu_yes);
        String softkey_no = getResources().getString(R.string.menu_no);

        playAlertSound();

        builder = new AlertDialog.Builder(mContext)
                .setTitle("")
                .setMessage(message)
                .setPositiveButton(softkey_yes, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        Log.d(TAG, "createNiaConfirmDialog: onClick Yes, notifyNIASessionProceed");
                        finish();
                        DestroyAlertDialog();
                        // start dial up
                        // DMNativeMethod.JVDM_notifyNIASessionProceed();
                        notifyNIASessionProceed();
                        // DmService.getInstance().startPppConnect();
                    }
                })
                .setNegativeButton(softkey_no, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        Log.d(TAG, "createNiaConfirmDialog: onClick No, cancel dm session!");
                        DMNativeMethod.JMMIDM_ExitDM();
                        // Vdmc.getInstance().stopVDM();
                        finish();
                        DestroyAlertDialog();
                    }
                })
                .setOnCancelListener(new DialogInterface.OnCancelListener() {
                    public void onCancel(DialogInterface dialog) {
                        Log.d(TAG, "createNiaConfirmDialog: onCancel, default cancel dm session!");
                        // Vdmc.getInstance().stopVDM();
                        DMNativeMethod.JMMIDM_ExitDM();
                        finish();
                        DestroyAlertDialog();
                    }
                })

                .show();

        startTimer(timeout);
    }

    private class DmTipDialog extends AlertDialog implements DialogInterface.OnClickListener,
            DmTipDialogListener, CompoundButton.OnCheckedChangeListener {
        protected DmTipDialog(Context context) {
            super(context);
        }

        protected DmTipDialog(Context context, int theme) {
            super(context, theme);
        }

        private DmTipDialogListener reporter;
        private boolean needVerify;
        private boolean showAlways = false;
        boolean needAlways = false;
        private String tip;
        private String verifyTip;
        private String alwaysTip;
        private DmTipDialog dg = null;

        public DmTipDialog(Context context, DmTipDialogListener listener, String firstTip,
                String secondTip) {
            this(context, listener, firstTip, secondTip, true);
        }

        public DmTipDialog(Context context, DmTipDialogListener listener, String firstTip,
                String secondTip, String always) {
            this(context, listener, firstTip, secondTip, true);
            showAlways = true;
            alwaysTip = always;
        }

        private DmTipDialog(Context context, DmTipDialogListener listener, String firstTip,
                String secondTip, boolean verified) {
            super(context, 0);
            tip = firstTip;
            verifyTip = secondTip;
            reporter = listener;
            needVerify = verified;
            if (needVerify)
                setMessage(tip);
            else
                setMessage(verifyTip);
            setTitle(R.string.notify_msg);
            setButton(DialogInterface.BUTTON_POSITIVE, getString(R.string.menu_ok), this);
            setButton(DialogInterface.BUTTON_NEGATIVE, getString(R.string.menu_cancel), this);
            setCancelable(false);
            setCanceledOnTouchOutside(false);
        }

        public void accept() {
            if (reporter != null)
                reporter.onAccept(this);
            if (needVerify) {
                super.dismiss();
            }
        }

        public void refuse() {
            if (reporter != null)
                reporter.onRefuse(this);
            if (needVerify) {
                super.dismiss();
            }
        }

        public void onAccept(DmTipDialog dlg) {
            accept();
        }

        public void onRefuse(DmTipDialog dlg) {
            refuse();
        }

        public void onClick(DialogInterface dialog, int which) {
            switch (which) {
                case DialogInterface.BUTTON_POSITIVE:
                    if (needVerify) {
                        ((DmTipDialog) dialog).accept();
                    } else {
                        ((DmTipDialog) dialog).refuse();
                    }
                    break;
                case DialogInterface.BUTTON_NEGATIVE:
                    if (needVerify) {
                        dg = new DmTipDialog(mContext, this, tip, verifyTip, false);
                        dg.show();
                    }
                    break;
            }
        }

        public void dismiss() {
            if (!needVerify)
                super.dismiss();
        }

        public void superDismiss() {
            if (needVerify) {
                super.dismiss();
                if (dg != null)
                    dg.dismiss();
                needAlways = false;
                refuse();
            }
        }

        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            needAlways = isChecked;
        }
    }

    interface DmTipDialogListener {
        public void onAccept(DmTipDialog dlg);

        public void onRefuse(DmTipDialog dlg);
    }

    public DmTipDialog showTip(String firstTip, String secondTip, String alwayTip,
            DmTipDialogListener listener) {
        DmTipDialog dialog;
        if (alwayTip != null) {
            dialog = new DmTipDialog(mContext, listener, firstTip, secondTip, alwayTip);
            LinearLayout layout = new LinearLayout(mContext);
            LinearLayout.LayoutParams param = new LinearLayout.LayoutParams(
                    ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT);
            param.gravity = Gravity.CENTER;
            layout.setLayoutParams(param);
            CheckBox box = new CheckBox(mContext);
            box.setText(alwayTip);
            box.setLayoutParams(param);
            box.setTextColor(Color.WHITE);
            box.setOnCheckedChangeListener(dialog);
            box.setChecked(false);
            layout.addView(box);
            layout.setOrientation(LinearLayout.VERTICAL);
            dialog.setView(layout);
        } else
            dialog = new DmTipDialog(mContext, listener, firstTip, secondTip);
        dialog.show();
        return dialog;
    }

    private void createAlertSelfRegistConfirmDialog(String message, int timeout,
            final int slotid) {
        int splitIdx = message.indexOf("|");
        if (splitIdx < 0)
            return;
        String firstTip = message.substring(0, splitIdx);
        String secondTip = message.substring(splitIdx + 1);
        showTip(firstTip, secondTip, null, new DmTipDialogListener() {
            public void onAccept(DmTipDialog dlg) {
                if (DmService.isServiceStart()) {
                    DmService.getInstance().postAgreedSelfRegist(true, slotid);
                } else {
                    Intent intent = new Intent("com.android.dm.SelfReg");
                    intent.putExtra("com.android.dm.notifySelfReg", true);
                    intent.putExtra("com.android.dm.notifySelfReg.agree", true);
                    intent.putExtra("com.android.dm.notifySelfReg.sub", slotid);
                    startService(intent);
                }
                finish();
                DestroyAlertDialog();
            }

            public void onRefuse(DmTipDialog dlg) {
                if (DmService.isServiceStart()) {
                    DmService.getInstance().postAgreedSelfRegist(false, slotid);
                } else {
                    Intent intent = new Intent("com.android.dm.SelfReg");
                    intent.putExtra("com.android.dm.notifySelfReg", true);
                    intent.putExtra("com.android.dm.notifySelfReg.agree", false);
                    intent.putExtra("com.android.dm.notifySelfReg.sub", slotid);
                    startService(intent);
                }
                finish();
                DestroyAlertDialog();
            }
        });
    }

    private void createAlertDataConnectConfirmDialog(String message, int timeout) {
        int splitIdx = message.indexOf("|");
        if (splitIdx < 0)
            return;
        String firstTip = message.substring(0, splitIdx);
        String secondTip = message.substring(splitIdx + 1);
        splitIdx = secondTip.indexOf("|");
        if (splitIdx < 0)
            return;
        String alwaysTip = secondTip.substring(splitIdx + 1);
        secondTip = secondTip.substring(0, splitIdx);
        startTimer(timeout);
        DmTipDialog dlg = showTip(firstTip, secondTip, alwaysTip, new DmTipDialogListener() {
            public void onAccept(DmTipDialog dg) {
                if (DmService.isServiceStart()) {
                    DmService.getInstance().postAgreedDataConnect(true, dg.needAlways);
                } else {
                    Intent intent = new Intent("com.android.dm.SelfReg");
                    intent.putExtra("com.android.dm.notifyDataCon", true);
                    intent.putExtra("com.android.dm.notifyDataCon.agree", true);
                    intent.putExtra("com.android.dm.notifyDataCon.always", dg.needAlways);
                    startService(intent);
                }
                finish();
                DestroyAlertDialog();
            }

            public void onRefuse(DmTipDialog dg) {
                if (DmService.isServiceStart()) {
                    DmService.getInstance().postAgreedDataConnect(false, dg.needAlways);
                } else {
                    Intent intent = new Intent("com.android.dm.SelfReg");
                    intent.putExtra("com.android.dm.notifyDataCon", true);
                    intent.putExtra("com.android.dm.notifyDataCon.agree", false);
                    intent.putExtra("com.android.dm.notifyDataCon.always", dg.needAlways);
                    startService(intent);
                }
                finish();
                DestroyAlertDialog();
            }
        });
        builder = dlg;
    }

    private void createAlertConfirmDialog(String message, int timeout)
    {
        playAlertSound();
        startTimer(timeout);
        String title = getResources().getString(R.string.notify_confirm_msg);
        String softkey_yes = getResources().getString(R.string.menu_yes);
        String softkey_no = getResources().getString(R.string.menu_no);
        createAlertConfirmDialog(mContext,
                title,
                message,
                softkey_yes,
                softkey_no,
                timeout);
    }

    private void createAlertConfirmDialog(Context context,
            String title,
            String message,
            String leftSfk,
            String rightSfk,
            int timeout)
    {
        builder = new AlertDialog.Builder(context)
                .setTitle(title)
                .setMessage(message)

                .setPositiveButton(leftSfk, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        Log.d("MyConfirmation ==> ",
                                "createAlertConfirmDialog: onClick Yes, notifyConfirmationResult true");
                        // observer.notifyConfirmationResult(true);
                        DMNativeMethod.Jhs_dm_mmi_confirmationQuerycb(true);
                        DmAlertDialog.getInstance().finishDialog();
                    }
                })
                .setNegativeButton(rightSfk, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        Log.d("MyConfirmation ==> ",
                                "createAlertConfirmDialog: onClick Yes, notifyConfirmationResult false");
                        DMNativeMethod.Jhs_dm_mmi_confirmationQuerycb(false);
                        // observer.notifyConfirmationResult(false);//
                        // dm_mmi_confirmationQuerycb
                        DmAlertDialog.getInstance().finishDialog();
                    }
                })
                .setOnCancelListener(new DialogInterface.OnCancelListener() {
                    public void onCancel(DialogInterface dialog) {
                        Log.d("MyConfirmation ==> ",
                                "createAlertConfirmDialog: onClick Yes, notifyCancelEvent");
                        // observer.notifyCancelEvent();
                        DMNativeMethod.Jhs_dm_mmi_confirmationQuerycb(false);
                        DmAlertDialog.getInstance().finishDialog();
                    }
                })

                .show();
    }

    private void handleTimeoutEvent()
    {
        Log.d("MyConfirmation ==> ", "handleTimeoutEvent: alert window timeout!!!!!!!!!");
        // observer.notifyCancelEvent();
        // DmAlertDialog.getInstance().finishDialog();
        DMNativeMethod.Jhs_dm_mmi_confirmationQuerycb(false);
        if (builder!=null) {
            if(builder instanceof DmTipDialog)
                ((DmTipDialog) builder).superDismiss();
            else
                builder.dismiss();
        }
        DmAlertDialog.getInstance().finishDialog();
    }

    private void notifyNIASessionProceed() {
        Thread t = new Thread() {
            public void run() {
                Log.d(TAG, "Thread notifyNIASessionProceed ");
                Log.d(TAG,
                        "##### DIAL START TIME = "
                                + DateFormat.format("yyyy-MM-dd kk:mm:ss",
                                        System.currentTimeMillis()));
                DMNativeMethod.JVDM_notifyNIASessionProceed();
            }
        };
        t.start();
    }

}
