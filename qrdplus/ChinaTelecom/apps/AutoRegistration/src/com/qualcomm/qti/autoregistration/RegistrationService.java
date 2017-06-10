/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.autoregistration;

import com.android.internal.telephony.PhoneConstants;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.preference.PreferenceManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

public class RegistrationService extends Service {

    private static final String TAG = "RegistrationService";
    private static final String BOOT_COMPLETE_FLAG = "boot_complete";
    private static final String MANUAL_REGISTRATION_FLAG = "manual";
    private static final boolean DBG = true;

    private static final long DELAY_REQUEST_AFTER_POWER_ON = 60 * 1000;
    private static final long INTERVAL_RESCHEDUAL = 60 * 60 * 1000;
    private static final int MAX_REQUEST_TIMES = 3;

    private static final String PREF_ICCID_ON_SUCCESS = "sim_iccid";
    private static final String PREF_REQUEST_COUNT = "register_request_count";

    public static final String ACTION_AUTO_REGISTERATION = "com.qualcomm.action.AUTO_REGISTRATION";

    private SharedPreferences mSharedPreferences;
    private AlarmManager mAlarmManager;

    @Override
    public void onCreate() {
        super.onCreate();
        mSharedPreferences = PreferenceManager.getDefaultSharedPreferences(this);
        mAlarmManager = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent != null && intent.getBooleanExtra(MANUAL_REGISTRATION_FLAG, false)) {
            onRegistrationRquestManually();
        } else {
            onRegistrationRquest(intent != null
                    && intent.getBooleanExtra(BOOT_COMPLETE_FLAG, false));
        }
        return super.onStartCommand(intent, flags, startId);
    }

    private void onRegistrationRquestManually() {
        new RegistrationTask(this) {
            @Override
            public void onResult(boolean registered, String resultDesc) {
                toast(resultDesc);
            }
        };
    }

    private void onRegistrationRquest(boolean bootComplete) {
        if (bootComplete) {
            mSharedPreferences.edit().putInt(PREF_REQUEST_COUNT, 0).commit();
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    onRegistrationRquest(false);
                }
            }, DELAY_REQUEST_AFTER_POWER_ON);
            return;
        }

        // MT in APM or SIM state not ready will not register.

        if (!isAnySimCardReady() || TelephonyManager.getDefault().isNetworkRoaming()) {
            Toast.makeText(this, R.string.register_failed, Toast.LENGTH_LONG).show();
            if (DBG) {
                Log.d(TAG, "Any SIM is not ready or in Roaming state, not to register");
            }
            return;
        }

        if (!isDDSInSub1()) {
            Toast.makeText(this, R.string.register_failed, Toast.LENGTH_LONG).show();
            return;
        }

        String iccIdText = getIccIdText();
        if (DBG) {
            Log.d(TAG, "Iccid: " + iccIdText);
        }

        if (isIccIdChanged(iccIdText)
                || iccIdText.equals(mSharedPreferences.getString(PREF_ICCID_ON_SUCCESS, null))) {
            Toast.makeText(this, R.string.already_registered, Toast.LENGTH_LONG).show();
            if (DBG) {
                Log.d(TAG, "Registered subs, Ignore");
            }
            return;
        }

        new RegistrationTask(this) {
            @Override
            public void onResult(boolean registered, String resultDesc) {
                if (!registered) {
                    scheduleNextIfNeed();
                } else {
                    mSharedPreferences.edit().putString(PREF_ICCID_ON_SUCCESS, getIccIdText())
                            .commit();
                    if (DBG) {
                        Log.d(TAG, "Register Done!");
                    }
                }
                toast(resultDesc);
            }
        };
    }

    private boolean isDDSInSub1() {
        // Without wifi but dds in SIM2 will not register.
        ConnectivityManager connMgr =
                (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo networkInfo = connMgr.getActiveNetworkInfo();
        if (networkInfo != null && networkInfo.isConnected()) {
            if (TelephonyManager.getDefault().isMultiSimEnabled()) {
                if (networkInfo.getType() != ConnectivityManager.TYPE_WIFI
                        && SubscriptionManager
                        .getPhoneId(SubscriptionManager.getDefaultDataSubId())
                            == PhoneConstants.SUB2) {
                    if (DBG) {
                        Log.d(TAG, "DDS now in SIM2 without wifi, not to register");
                    }
                    return false;
                }
            }
        }
        return true;
    }

    private boolean isAnySimCardReady() {
        int numPhones = TelephonyManager.getDefault().getPhoneCount();
        for (int index = 0; index < numPhones; index++) {
            if (TelephonyManager.getDefault().getSimState(index)
                    == TelephonyManager.SIM_STATE_READY) {
                return true;
            }
        }
        return false;
    }

    private boolean isIccIdChanged(String iccId) {
        boolean flag = true;
        if (iccId.contains(",")) {
            String[] strArray = TextUtils.split(iccId, ",");
            for (String str : strArray) {
                flag &= TextUtils.isEmpty(str.trim());
            }
        } else {
            flag = TextUtils.isEmpty(iccId.trim());
        }
        return flag;
    }

    private String getIccIdText() {
        String iccId = null;
        if (TelephonyManager.getDefault().isMultiSimEnabled()) {
            int phoneCount = TelephonyManager.getDefault().getPhoneCount();
            for (int index = 0; index < phoneCount; index++) {
                String id = TelephonyManager.getDefault().getSimSerialNumber(
                        SubscriptionManager.getSubId(index)[0]);
                if (id == null) {
                    id = " ";
                }
                if (iccId == null) {
                    iccId = id;
                } else {
                    iccId += ("," + id);
                }
            }
        } else {
            String id = TelephonyManager.getDefault().getSimSerialNumber();
            iccId = (null == id) ? " " : id;
        }
        return iccId;
    }

    protected void scheduleNextIfNeed() {
        int reuqestCount = mSharedPreferences.getInt(PREF_REQUEST_COUNT, 0) + 1;
        mSharedPreferences.edit().putInt(PREF_REQUEST_COUNT, reuqestCount).commit();
        if (reuqestCount < MAX_REQUEST_TIMES) {
            PendingIntent intent = PendingIntent.getBroadcast(this, 0, new Intent(
                    ACTION_AUTO_REGISTERATION), 0);
            mAlarmManager.set(AlarmManager.RTC_WAKEUP,
                    System.currentTimeMillis() + INTERVAL_RESCHEDUAL, intent);
        }
    }

    protected void toast(final String resultDesc) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                String resultInfo = null;
                if (TextUtils.isEmpty(resultDesc)) {
                    resultInfo = RegistrationService.this.getResources().getString(
                            R.string.register_failed);
                } else {
                    resultInfo = getLocalString(resultDesc);
                }
                Toast.makeText(RegistrationService.this, resultInfo, Toast.LENGTH_LONG).show();
            }
        });
    }

    private String getLocalString(String originalResult) {
        return (android.util.NativeTextHelper.getInternalLocalString(RegistrationService.this,
                originalResult,
                R.array.original_registry_results, R.array.local_registry_results));
    }

}
