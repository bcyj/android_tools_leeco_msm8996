/**
 * Copyright (c) 2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 **/

package com.qualcomm.qti.modemtestmode;

import android.app.AlertDialog;
import android.app.Service;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.ComponentName;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.text.TextUtils;
import android.util.Log;
import android.view.WindowManager;
import com.android.internal.telephony.TelephonyProperties;

import com.qualcomm.qti.modemtestmode.IMbnTestService;

public class MbnSystemService extends Service {
    private static final String TAG = "MbnSystemService";
    private static final int EVENT_QCRIL_HOOK_READY = 1;
    private static final int EVENT_SERVICE_BINDED = 2;

    private MbnMetaInfo mMbnMetaInfo;
    private String mModemMode = null;
    private IMbnTestService mMbnTestService;
    private Object mLock = new Object();
    private boolean mChecked = false;

    private ServiceConnection mMbnServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            synchronized(mLock) {
                mMbnTestService = (IMbnTestService) IMbnTestService.Stub.asInterface(service);
                mHandler.obtainMessage(EVENT_SERVICE_BINDED).sendToTarget();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mMbnTestService = null;
        }

    };

    private Handler mHandler = new Handler() {

        @Override
        public void dispatchMessage(Message msg) {
            switch (msg.what) {
            case EVENT_QCRIL_HOOK_READY:
                log("EVENT_QCRIL_HOOK_READY");
                MbnAppGlobals.getInstance().unregisterQcRilHookReady(mHandler);
                String mbnId = MbnAppGlobals.getInstance().getMbnConfig(0);
                String metaInfo = MbnAppGlobals.getInstance().getMetaInfoForConfig(mbnId);
                log("Mbn ID:" + mbnId + " Meta Info:" + metaInfo);
                if (TextUtils.isEmpty(metaInfo)) {
                    log("strict check, will ignore");
                    return;
                }
                mMbnMetaInfo = new MbnMetaInfo(MbnSystemService.this, mbnId, metaInfo);
                synchronized(mLock) {
                    if (mMbnTestService != null && mChecked == false) {
                        handleApnMultiMode();
                    }
                }
                break;

            case EVENT_SERVICE_BINDED:
                log("EVENT_SERVICE_BINDED");
                synchronized(mLock) {
                    if (mMbnTestService != null && mMbnMetaInfo != null && mChecked == false) {
                        handleApnMultiMode();
                    }
                }
                break;

            default:
                log("Unexpected event:" + msg.what);
                break;
            }
        }
    };

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        bindService(new Intent(getApplicationContext(), MbnTestService.class),
                mMbnServiceConnection, Context.BIND_AUTO_CREATE);
        MbnAppGlobals.getInstance().registerQcRilHookReady(mHandler,
                EVENT_QCRIL_HOOK_READY, null);
        log("Service is started");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unbindService(mMbnServiceConnection);
        MbnAppGlobals.getInstance().unregisterQcRilHookReady(mHandler);
    }

    private void handleApnConfiguration() {
        boolean isCommercial = mMbnMetaInfo.getMetaInfo().toLowerCase().contains(
                MbnTestUtils.COMMERCIAL_MBN);
        String mbnType = MbnTestUtils.getPropertyValue(MbnTestUtils.PROPERTY_MBN_TYPE);
        boolean isPreferredCommercial = true;

        if (!TextUtils.isEmpty(mbnType) && mbnType.equals(MbnTestUtils.TEST_MBN)) {
            isPreferredCommercial = false;
        }

        log("Activated commercial:" + isCommercial + " Preference commercial:" +
                isPreferredCommercial + " Mbn type:" + mbnType);

        String carrier = mMbnMetaInfo.getCarrier().toLowerCase();
        if (isCommercial && !isPreferredCommercial) {
            // activated mbn is Commerical, preferred is not commercial
            try {
                mMbnTestService.setProperty(MbnTestUtils.PROPERTY_MBN_TYPE,
                        MbnTestUtils.COMMERCIAL_MBN);
                MbnTestUtils.setupData(MbnSystemService.this, true);
            } catch (RemoteException e) {
            }
            //log("will restore default apn");
            //new ApnHandlerHelper(MbnSystemService.this, carrier).restoreDefaultAPN();
        } else if (!isCommercial && isPreferredCommercial) {
            // activated mbn is non-commercial, preferred is commercial (after factory-reset)
            try {
                mMbnTestService.setProperty(MbnTestUtils.PROPERTY_MBN_TYPE, MbnTestUtils.TEST_MBN);
                MbnTestUtils.setupData(MbnSystemService.this, false);
            } catch (RemoteException e) {
            }
            //log("will restore labtest apn");
            //new ApnHandlerHelper(MbnSystemService.this, carrier).restoreLabTestAPN();
        } else {
            log("Do nothing for apn");
        }
    }

    private boolean isModeMatched() {
        boolean matched = true;
        String telMode = MbnTestUtils.getPropertyValue(
                TelephonyProperties.PROPERTY_MULTI_SIM_CONFIG);
        String mbnId = mMbnMetaInfo.getMbnId();
        String metaInfo = mMbnMetaInfo.getMetaInfo();

        log("telMode:" + telMode + " mbnId:" + mbnId + " metaInfo:" + metaInfo);

        // TODO one of mbn id or meta info can be NULL?
        if (TextUtils.isEmpty(MbnTestUtils.getMultiSimMode(metaInfo))) {
            log("Fail to find MultiMode from Meta Info");
            return true;
        }

        if (telMode == null) {
            telMode = ""; //in case fail get multi mode situation.
        }

        // Can't use toLower
        if (telMode.equals(MbnTestUtils.getMultiSimMode(metaInfo))) {
            return true;
        }

        mModemMode = MbnTestUtils.getMultiSimMode(metaInfo);
        return false;
    }

    private void showMismatchDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        AlertDialog alertDialog = builder
                .setMessage(R.string.mismacth_hit)
                .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialogInterface, int i) {
                        try {
                            mMbnTestService.setProperty(TelephonyProperties.
                                    PROPERTY_MULTI_SIM_CONFIG, mModemMode);
                        } catch (RemoteException e) {
                        }
                        MbnTestUtils.rebootSystem(MbnSystemService.this);
                    }
                })
                .setCancelable(false)
                .create();
        alertDialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_DIALOG);
        alertDialog.show();
    }

    private void handleApnMultiMode() {
        mChecked = true;
        handleApnConfiguration();
        if (!isModeMatched()) {
            log("Multi-Sim Mode unmatched. Alerting...");
            showMismatchDialog();
        }
    }

    private void log(String msg) {
        Log.d(TAG, "MbnTest_ " + msg);
    }
}
