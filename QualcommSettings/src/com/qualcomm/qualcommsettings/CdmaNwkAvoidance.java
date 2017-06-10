/******************************************************************************
 * @file    CdmaNwkAvoidance.java
 * @brief   Provides the settings for the Manual system avoidance for CDMA
 *          network feature
 *
 *
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2013 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *  ******************************************************************************/
package com.qualcomm.qualcommsettings;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.telephony.TelephonyManager;
import android.telephony.PhoneNumberUtils;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;

import com.android.internal.telephony.ITelephony;
import com.qualcomm.qcrilhook.QcRilHook;
import com.qualcomm.qcrilhook.QcRilHookCallback;

public class CdmaNwkAvoidance extends PreferenceActivity implements
        Preference.OnPreferenceClickListener {

    private static final String TAG = "CdmaNwkAvoidance";
    private static final String PHONE_SERVICE = "phone";
    public static final String QCOM_SETTINGS = "qualcomm_settings";
    public static final String AVOID_CUR_NWK = "avoid_cur_nwk";
    public static final String VIEW_NWK_LIST = "view_nwk_list";
    public static final String CLEAR_NWK_LIST = "clear_nwk_list";

    // Events definition
    private static final int EVENT_INVALID = 0;
    private static final int EVENT_DATA_DISABLED = 1;
    private static final int EVENT_AVOID_CURRENT_NETWORK = 2;
    private static final int EVENT_AVOID_CURRENT_NETWORK_COMPLETED = 3;
    private static final int EVENT_CLEAR_AVOIDANCE_LIST = 4;
    private static final int EVENT_CLEAR_AVOIDANCE_LIST_COMPLETED = 5;
    private static final int EVENT_GET_AVOIDANCE_LIST = 6;
    private static final int EVENT_GET_AVOIDANCE_LIST_COMPLETED = 7;
    private static final int EVENT_UNABLE_TO_DISABLE_DATA = 8;

    private static int mNextReqEvent = EVENT_INVALID;
    private static boolean mInEmergencyCall = false;
    private static int mServiceState = ServiceState.STATE_IN_SERVICE;
    private static int mCallState = TelephonyManager.CALL_STATE_IDLE;

    // Currently in C+G combo, the CDMA sub is always 0
    private static final int CDMA_SUB = 0;

    private QcRilHook mQcRilHook;
    private static boolean mIsQcRilHookReady = false;
    private Context mContext;

    // UI components
    private Preference mAvoidCurNwk;
    private Preference mViewNwkList;
    private Preference mClearNwkList;

    private ProgressDialog mpDialogDisableData;

    private ITelephony mPhone;
    private TelephonyManager mTelephonyManager;

    private static CdmaNwkAvoidance mThis;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mThis = this;
        mContext = getApplicationContext();
        mQcRilHook = new QcRilHook(mContext, mQcrilHookCb);
        addPreferencesFromResource(R.xml.cdma_nwk_avoidance);

        mAvoidCurNwk = (Preference) findPreference(AVOID_CUR_NWK);
        mAvoidCurNwk.setOnPreferenceClickListener(this);
        mViewNwkList = (Preference) findPreference(VIEW_NWK_LIST);
        mViewNwkList.setOnPreferenceClickListener(this);
        mClearNwkList = (Preference) findPreference(CLEAR_NWK_LIST);
        mClearNwkList.setOnPreferenceClickListener(this);

        mpDialogDisableData = new ProgressDialog(CdmaNwkAvoidance.this);
        mpDialogDisableData.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        mpDialogDisableData.setMessage(getResources().getString(
                R.string.waiting_disable_data));
        mpDialogDisableData.setIndeterminate(false);
        mpDialogDisableData.setCancelable(false);

        mPhone = ITelephony.Stub.asInterface(ServiceManager
                .getService(PHONE_SERVICE));
        mTelephonyManager = (TelephonyManager)getSystemService(TELEPHONY_SERVICE);
        if (mPhone == null || mTelephonyManager == null) {
            Log.e(TAG, "Unable to get Phone Services. mPhone = " + mPhone +
                    " mTelephonyManager = " + mTelephonyManager);
            this.finish();
        }
        mTelephonyManager.listen(mPhoneStateListener,
                PhoneStateListener.LISTEN_SERVICE_STATE | PhoneStateListener.LISTEN_CALL_STATE);
    }

    @Override
    protected void onResume() {
       super.onResume();
       updatePreference();
    }

    private QcRilHookCallback mQcrilHookCb = new QcRilHookCallback() {
        public void onQcRilHookReady() {
            mIsQcRilHookReady = true;
        }
        public void onQcRilHookDisconnected() {
            mIsQcRilHookReady = false;
        }
    };

    @Override
    public boolean onPreferenceClick(Preference preference) {
        if (preference == mAvoidCurNwk) {
            onAvoidCurNwkClicked();
        } else if (preference == mViewNwkList) {
            onViewNwkListClicked();
        } else if (preference == mClearNwkList) {
            onClearNwkListClicked();
        }
        return true;
    }

    /**
     * Called when the "Avoid the current network" option is selected
     */
    private void onAvoidCurNwkClicked() {
        new AlertDialog.Builder(CdmaNwkAvoidance.this)
                .setTitle(R.string.avoid_cur_nwk_title)
                .setMessage(R.string.confirm_avoid)
                .setNegativeButton(R.string.cancel_btn, null)
                .setPositiveButton(R.string.ok_btn,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,
                                    int which) {
                                mHandler.sendMessage(mHandler
                                            .obtainMessage(EVENT_AVOID_CURRENT_NETWORK));
                            }
                        }).show();
    }

    /**
     * Called when the "Clear the avoidance network list" option is selected
     */
    private void onClearNwkListClicked() {
        new AlertDialog.Builder(CdmaNwkAvoidance.this)
                .setTitle(R.string.avoid_cur_nwk_title)
                .setMessage(R.string.confirm_clear)
                .setNegativeButton(R.string.cancel_btn, null)
                .setPositiveButton(R.string.ok_btn,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,
                                    int which) {
                                mHandler.sendMessage(mHandler
                                            .obtainMessage(EVENT_CLEAR_AVOIDANCE_LIST));
                            }
                        }).show();
    }

    /**
     * Called when the "View the avoidance network list" option is selected
     */
    private void onViewNwkListClicked() {
        mHandler.sendMessage(mHandler.obtainMessage(EVENT_GET_AVOIDANCE_LIST));
    }

    /**
     * Handler
     */
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "Handler received event: " + msg.what);
            switch (msg.what) {
                case EVENT_DATA_DISABLED:
                    handleDataDisabled(msg);
                    break;
                case EVENT_UNABLE_TO_DISABLE_DATA:
                    setNextReqEvent(EVENT_INVALID);
                    mpDialogDisableData.cancel();
                    Toast.makeText(getApplicationContext(), R.string.unable_to_disable_data,
                            Toast.LENGTH_SHORT).show();
                    break;
                case EVENT_AVOID_CURRENT_NETWORK:
                    handleAvoidCurNwk(msg);
                    break;
                case EVENT_AVOID_CURRENT_NETWORK_COMPLETED:
                    Log.d(TAG, "Cdma Avoid Current Network Command Completed.");
                    break;
                case EVENT_CLEAR_AVOIDANCE_LIST:
                    handleClearAvoidanceList(msg);
                    break;
                case EVENT_CLEAR_AVOIDANCE_LIST_COMPLETED:
                    Log.d(TAG, "Cdma Clear Avoidandce List Command Completed.");
                    break;
                case EVENT_GET_AVOIDANCE_LIST:
                    handleGetAvoidanceList(msg);
                    break;
                case EVENT_GET_AVOIDANCE_LIST_COMPLETED:
                    if (msg.obj != null) {
                        Intent intent = new Intent(CdmaNwkAvoidance.this, AvoidanceList.class);
                        Bundle bundle = new Bundle();
                        bundle.putByteArray("avoidance_list", (byte[])msg.obj);
                        intent.putExtras(bundle);
                        startActivity(intent);
                    }
                    break;
                default:
                    // We shouldn't be here!
                    Log.e(TAG, "Unknown message type=" + msg.what);
                    super.handleMessage(msg);
            }
        }
    };

    /**
     * Process a Data Disabled event
     *
     * @param msg : Message
     */
    private void handleDataDisabled(Message msg) {
        if (mpDialogDisableData.isShowing()) {
            mpDialogDisableData.cancel();
        }

        if (getNextReqEvent() != EVENT_INVALID) {
            Log.d(TAG, "Data disconnected. Ready to send the next request.");
            mHandler.sendMessage(mHandler.obtainMessage(getNextReqEvent()));

            setNextReqEvent(EVENT_INVALID);
        }
    }

    /**
     * Process a Avoid Current Network Event
     *
     * @param msg : Message
     */
    private void handleAvoidCurNwk(Message msg) {
        Log.d(TAG, "Sending RIL OEM HOOK Cdma Avoid Current Network message.");

        boolean result = false;
        if (!mIsQcRilHookReady) {
            // return if the QcRilHook isn't ready
            return;
        }
        result = mQcRilHook.qcRilCdmaAvoidCurNwk();
        if (result == false) {
            Log.e(TAG, "qcRilCdmaAvoidCurNwk command failed.");
            Toast.makeText(getApplicationContext(), R.string.avoid_cur_nwk_failed,
                    Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(getApplicationContext(), R.string.avoid_cur_nwk_succeeded,
                    Toast.LENGTH_SHORT).show();
        }

        mHandler.sendMessage(mHandler
                .obtainMessage(EVENT_AVOID_CURRENT_NETWORK_COMPLETED));
    }

    /**
     * Process a Clear Avoidance Network List Event
     *
     * @param msg : Message
     */
    private void handleClearAvoidanceList(Message msg) {
        Log.d(TAG, "Sending RIL OEM HOOK Cdma Clear Avoidance List message.");

        boolean result = false;
        if (!mIsQcRilHookReady) {
            // return if the QcRilHook isn't ready
            return;
        }
        result = mQcRilHook.qcRilCdmaClearAvoidanceList();
        if (result == false) {
            Log.e(TAG, "qcRilCdmaClearAvoidanceList command failed.");
            Toast.makeText(getApplicationContext(), R.string.clear_nwk_list_failed,
                    Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(getApplicationContext(), R.string.clear_nwk_list_succeeded,
                    Toast.LENGTH_SHORT).show();
        }

        mHandler.sendMessage(mHandler
                .obtainMessage(EVENT_CLEAR_AVOIDANCE_LIST_COMPLETED));
    }

    /**
     * Process a Get Avoidance Network List Event
     *
     * @param msg : Message
     */
    private void handleGetAvoidanceList(Message msg) {
        Log.d(TAG, "Sending RIL OEM HOOK Cdma Get Avoidance List message.");

        byte[] result = null;
        if (!mIsQcRilHookReady) {
            // return if the QcRilHook isn't ready
            return;
        }
        result = mQcRilHook.qcRilCdmaGetAvoidanceList();
        if (result == null) {
            Log.e(TAG, "qcRilCdmaGetAvoidanceList command failed.");
            // TODO: tell user it was unable to get the avoidance network list.
        }

        mHandler.sendMessage(mHandler.obtainMessage(
                EVENT_GET_AVOIDANCE_LIST_COMPLETED, (Object)result));
    }

    /**
     * Set which request event to be sent after the data is disabled.
     *
     * @param event : int
     */
    synchronized private void setNextReqEvent(int event) {
        mNextReqEvent = event;
    }

    /**
     * Get which request event to be sent after the data is disabled.
     *
     * @return int
     */
    synchronized private int getNextReqEvent() {
        return mNextReqEvent;
    }

    /**
     * Check whether it is a CDMA phone.
     *
     * @return boolean
     */
    private boolean isCdmaPhone() {
        boolean ret = false;
        try {

            if (mPhone.getActivePhoneType() !=
                    TelephonyManager.PHONE_TYPE_CDMA) {
                Log.d(TAG, "No CDMA phone");
            } else {
                ret = true;
            }
        } catch (RemoteException e) {
            Log.w(TAG, "getActivePhoneType() failed", e);
        }

        return ret;
    }

    /**
     * We enable the Avoid the current network option only when
     *      - Phone is in service
     *      - Call state is IDLE
     *      - Not in an emergency call
     */
    public void updatePreference() {
        Log.d(TAG, "Service State: " + mServiceState + " Call State: " + mCallState +
                " In an Emergency Call: " + mInEmergencyCall);
        if (mServiceState == ServiceState.STATE_IN_SERVICE &&
                (mCallState == TelephonyManager.CALL_STATE_IDLE ||
                !mInEmergencyCall) && isCdmaPhone()) {
            Log.d(TAG, "Enable the Avoid the current network option.");
            mAvoidCurNwk.setEnabled(true);
        } else {
            Log.d(TAG, "Disable the Avoid the current network option.");
            mAvoidCurNwk.setEnabled(false);
        }
    }

    /**
     * We are listening to this because we want to disable the following option
     * when phone is not in service.
     *     - Avoid the current network
     */
    PhoneStateListener mPhoneStateListener = new PhoneStateListener(CDMA_SUB) {

        @Override
        public void onServiceStateChanged(ServiceState state) {

            mServiceState = state.getState();
            // Calling updatePreference to enable/disable the Avoid the current network option
            updatePreference();
        }

        @Override
        public void onCallStateChanged(int state, String incomingNumber) {

            mCallState = state;
            // Calling updatePreference to enable/disable the Avoid the current network option
            updatePreference();
        }
    };

    /**
     * We are monitoring the outgoing calls because we want to disable the following option
     * when phone is making a MO emergency call.
     *     - Avoid the current network
     */
    public static class OutgoingCallReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            Bundle bundle = intent.getExtras();
            if(bundle == null)
                return;

            String phonenumber = intent.getStringExtra(Intent.EXTRA_PHONE_NUMBER);
            mInEmergencyCall = PhoneNumberUtils.isEmergencyNumber(phonenumber);
            // Calling updatePreference to enable/disable the Avoid the current network option
            if (mThis != null)
                mThis.updatePreference();
        }
    }
}
