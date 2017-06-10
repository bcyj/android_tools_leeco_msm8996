/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.xdivert;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.preference.PreferenceManager;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.internal.telephony.CallManager;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyIntents;

import java.util.List;



public class XDivertUtility extends XDivert {
    static final String LOG_TAG = "XDivertUtility";
    private static final int SIM_RECORDS_LOADED = 1;
    private static final int EVENT_SUBSCRIPTION_DEACTIVATED = 2;
    static final int CALL_FORWARD_XDIVERT = 22;

    private static final String SIM_IMSI = "sim_imsi_key";
    private static final String SIM_NUMBER = "sim_number_key";

    public static final String LINE1_NUMBERS = "Line1Numbers";
    private static final String XDIVERT_STATUS = "xdivert_status_key";

    private static final int PHONE_ID1 = 0;
    private static final int PHONE_ID2 = 1;

    private Context mContext;
    private Phone mPhone;
    //private PhoneGlobals mApp;
    protected static XDivertUtility sMe;
     protected NotificationManager mNotificationManager;
    private BroadcastReceiver mReceiver;

    private String[] mImsiFromSim;
    private String[] mStoredImsi;
    private String[] mLineNumber;

    private int mNumPhones = 0;
    private boolean[] mHasImsiChanged;

    public XDivertUtility() {
        sMe = this;
    }

    static XDivertUtility init(Context context) {
        synchronized (XDivertUtility.class) {
             Log.d(LOG_TAG, "init...");
            if (sMe == null) {
                sMe = new XDivertUtility(context);
            } else {
                Log.w(LOG_TAG, "init() called multiple times!  sInstance = " + sMe);
            }
            return sMe;
        }
    }

    private XDivertUtility(Context context) {
        Log.d(LOG_TAG, "onCreate()...");

        mPhone = PhoneFactory.getDefaultPhone();
        mContext = context;

        mNumPhones = TelephonyManager.getDefault().getPhoneCount();
        mNotificationManager =
                (NotificationManager) mContext.getSystemService(Context.NOTIFICATION_SERVICE);
        mImsiFromSim = new String[mNumPhones];
        mStoredImsi = new String[mNumPhones];
        mLineNumber = new String[mNumPhones];
        mHasImsiChanged = new boolean[mNumPhones];

        for (int i = 0; i < mNumPhones; i++) {
            mPhone = PhoneFactory.getPhone(i);
            // register for SIM_RECORDS_LOADED
            mPhone.registerForSimRecordsLoaded(mHandler, SIM_RECORDS_LOADED, i);
            mHasImsiChanged[i] = true;
        }
    }

    static XDivertUtility getInstance() {
        return sMe;
    }

    /**
     * Receiver for intent broadcasts the XDivertUtility cares about.
     */
    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.v(LOG_TAG,"onReceive Action intent recieved:"+action);
        int defaultPhoneId = SubscriptionManager.getPhoneId(SubscriptionManager.getDefaultSubId());
        //gets the slot information ( "0" or "1")
        int slot = intent.getIntExtra(PhoneConstants.SLOT_KEY, defaultPhoneId);
        if (action.equals(TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED)) {
            Phone phone = PhoneFactory.getPhone(slot);
            phone.unregisterForSimRecordsLoaded(mHandler);
            phone.registerForSimRecordsLoaded(mHandler, SIM_RECORDS_LOADED, slot);
        } else if (action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
            if(!isAllSubActive()) {
                onSubscriptionDeactivated();
            }
        }
    }


    private boolean isSlotActive(int slotId) {
        boolean slotActive = false;
        List<SubscriptionInfo> activeSubList =
                 SubscriptionManager.from(mContext).getActiveSubscriptionInfoList();
        if (activeSubList != null) {
            for (SubscriptionInfo subScriptionInfo : activeSubList) {
                if (subScriptionInfo.getSimSlotIndex() == slotId) {
                    slotActive = true;
                     break;
                }
            }
        }
        return slotActive;
    }

    private boolean isAllSubActive() {
        int numPhones = TelephonyManager.getDefault().getPhoneCount();
        for (int i = 0; i < numPhones; i++) {
            if (!isSlotActive(i)) return false;
        }
        return true;
    }

    Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            PhoneConstants.State phoneState;
            AsyncResult ar;
            switch (msg.what) {
                case SIM_RECORDS_LOADED:
                    ar = (AsyncResult)msg.obj;
                    boolean status = false;

                    if (ar.exception != null) {
                        break;
                    }
                    int phoneId = (Integer)ar.userObj;
                    Log.d(LOG_TAG, "phoneId = " + phoneId);
                    // Get the Imsi value from the SIM records. Retrieve the stored Imsi
                    // value from the shared preference. If both are same, then read the
                    // stored phone number from the shared preference, else prompt the
                    // user to enter them.
                    int[] subID = SubscriptionManager.getSubId(phoneId);
                    mImsiFromSim[phoneId] = TelephonyManager.getDefault()
                            .getSubscriberId(subID[0]);
                    mStoredImsi[phoneId] = getSimImsi(phoneId);
                    Log.d(LOG_TAG, "SIM_RECORDS_LOADED mImsiFromSim = " +
                            mImsiFromSim[phoneId] + "mStoredImsi = " +
                            mStoredImsi[phoneId]);
                    if ((mStoredImsi[phoneId] == null) || ((mImsiFromSim[phoneId] != null)
                            && (!mImsiFromSim[phoneId].equals(mStoredImsi[phoneId])))) {
                        // Imsi from SIM does not match the stored Imsi.
                        // Hence reset the values.
                        setXDivertStatus(false);
                        setSimImsi(mImsiFromSim[phoneId], phoneId);
                        storeNumber(null, phoneId);
                    } else if ((mStoredImsi[phoneId] != null) &&
                            (mImsiFromSim[phoneId] != null) &&
                            mImsiFromSim[phoneId].equals(mStoredImsi[phoneId])) {
                        // Imsi from SIM matches the stored Imsi so get the stored lineNumbers
                        mLineNumber[phoneId] = getNumber(phoneId);
                        mHasImsiChanged[phoneId] = false;
                        Log.d(LOG_TAG, "Stored Line Number = " + mLineNumber[phoneId]);
                    }

                    // Only if Imsi has not changed, query for XDivert status from shared pref
                    // and update the notification bar.
                    if ((!mHasImsiChanged[PHONE_ID1]) && (!mHasImsiChanged[PHONE_ID2])) {
                        status = getXDivertStatus();
                        onXDivertChanged(status);
                    }
                    break;
                case EVENT_SUBSCRIPTION_DEACTIVATED:
                    Log.d(LOG_TAG, "EVENT_SUBSCRIPTION_DEACTIVATED");
                    onSubscriptionDeactivated();
                    break;
                default:
                    super.handleMessage(msg);
            }
        }
    };

    protected boolean checkImsiReady() {
        for (int i = 0; i < mNumPhones; i++) {
            mStoredImsi[i] = getSimImsi(i);
            int[] subId = SubscriptionManager.getSubId(i);
            mImsiFromSim[i] = TelephonyManager.getDefault().getSubscriberId(subId[0]);
            // if imsi is not yet read, then above api returns ""
            if ((mImsiFromSim[i] == null)  || (mImsiFromSim[i] == "")) {
                return false;
            } else if ((mStoredImsi[i] == null) || ((mImsiFromSim[i] != null)
                    && (!mImsiFromSim[i].equals(mStoredImsi[i])))) {
                // Imsi from SIM does not match the stored Imsi.
                // Hence reset the values.
                setXDivertStatus(false);
                setSimImsi(mImsiFromSim[i], i);
                storeNumber(null, i);
                mHasImsiChanged[i] = true;
            }
        }
        return true;
    }

    // returns the stored Line Numbers
    public String[] getLineNumbers() {
        return mLineNumber;
    }

    // returns the stored Line Numbers
    public String[] setLineNumbers(String[] lineNumbers) {
        return mLineNumber = lineNumbers;
    }

    // returns the stored Imsi from shared preference
    protected String getSimImsi(int subscription) {
        Log.d(LOG_TAG, "getSimImsi sub = " + subscription);
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(
                mContext);
        return sp.getString(SIM_IMSI + subscription, null);
    }

    // saves the Imsi to shared preference
    protected void setSimImsi(String imsi, int subscription) {
        Log.d(LOG_TAG, "setSimImsi imsi = " + imsi + "sub = " + subscription);
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(
                mContext);
        SharedPreferences.Editor editor = sp.edit();
        editor.putString(SIM_IMSI + subscription, imsi);
        editor.apply();
    }

    // On Subscription deactivation, clear the Xdivert icon from
    // notification bar
    private void onSubscriptionDeactivated() {
        onXDivertChanged(false);
    }

    // returns the stored Line Numbers from shared preference
    protected String getNumber(int subscription) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(
                mContext);
        return sp.getString(SIM_NUMBER + subscription, null);
    }

    // saves the Line Numbers to shared preference
    protected void storeNumber(String number, int subscription) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(
                mContext);
        SharedPreferences.Editor editor = sp.edit();
        editor.putString(SIM_NUMBER + subscription, number);
        editor.apply();

        // Update the lineNumber which will be passed to XDivertPhoneNumbers
        // to populate the number from next time.
        mLineNumber[subscription] = number;
    }
    protected void onXDivertChanged(boolean visible) {
         Log.d(LOG_TAG, "onXDivertChanged(): " + visible);
        updateXDivert(visible);
    }

    // Gets the XDivert Status from shared preference.
    protected boolean getXDivertStatus() {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(
                mContext);
        boolean status = sp.getBoolean(XDIVERT_STATUS, false);
        Log.d(LOG_TAG, "getXDivertStatus status = " + status);
        return status;
    }

    // Sets the XDivert Status to shared preference.
    protected void setXDivertStatus(boolean status) {
        Log.d(LOG_TAG, "setXDivertStatus status = " + status);
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(
                mContext);
        SharedPreferences.Editor editor = sp.edit();
        editor.putBoolean(XDIVERT_STATUS, status);
        editor.apply();
    }
    /**
     * Updates the XDivert indicator notification.
     *
     * @param visible true if XDivert is enabled.
     */
    /* package */ void updateXDivert(boolean visible) {
        Log.d(LOG_TAG, "updateXDivert: " + visible);
        if (visible) {
            Intent intent = new Intent(Intent.ACTION_MAIN);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.setClassName("com.android.phone",
                    "com.android.phone.CallFeaturesSetting");
            int resId = R.drawable.stat_sys_phone_call_forward_xdivert;
            Notification notification = new Notification(
                    resId,  // icon
                    null, // tickerText
                    System.currentTimeMillis()
                    );
            notification.setLatestEventInfo(
                    mContext, // context
                    mContext.getString(R.string.xdivert_title), // expandedTitle
                    mContext.getString(R.string.sum_xdivert_enabled), // expandedText
                    PendingIntent.getActivity(mContext, 0, intent, 0)); // contentIntent

            mNotificationManager.notify(CALL_FORWARD_XDIVERT, notification);
        } else {
            mNotificationManager.cancel(CALL_FORWARD_XDIVERT);
        }
    }

}
