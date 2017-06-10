/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.salestracker;

import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.Semaphore;
import java.util.zip.CRC32;

import android.app.Activity;
import android.app.PendingIntent;
import android.app.Service;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.SystemProperties;
import android.preference.PreferenceManager;
import android.telephony.CellLocation;
import android.telephony.SmsManager;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.gsm.GsmCellLocation;
import android.util.Log;

public class SalesTrackerService extends Service {
    private static final String TAG = "SalesTracker::Service";

    public static final String ACTION_START_SERVICE = "com.android.salestracker.action.START_SERVICE";
    public static final String EXTRA_TRIGGER_ACTION = "extra_trigger_action";

    public static final String ACTION_SENT_RESULT = "com.android.salestracker.action.SENT_RESULT";
    public static final String ACTION_DELIVERY_RESULT = "com.android.salestracker.action.DELIVERY_RESULT";
    public static final String EXTRA_RETRY_TOKEN = "extra_retry_token";
    public static final String EXTRA_REFERENCE_NUM = "extra_reference_num";
    public static final String EXTRA_RESULT_CODE = "extra_result_code";

    public static final String FLAG_REG_RESULT = "flag_reg_result";
    public static final int REG_UNKNOWN = -1;
    public static final int REG_FAIL = 0;
    public static final int REG_SUCCESS = 1;

    // TODO Make sure the following:
    // What is the Bangladesh Number?
    // Remove dev & test phone number.
    private static final String[][] MCC_DEST_ADDRESS = {
        /* MCC,     destination address */
        { "413",   "+94114339003" },    /* SL Number */
        { "470",   "+8807464" },        /* Bangladesh Number */
        { "404",   "+919212230707" },   /* India Number */
        { "405",   "+919212230707" },   /* India Number */
    };

    private static final int FIRST_TIME_DELAY = 10 * 60 * 1000;
    private static final int RETRY_DELAY = 3 * 60 * 1000;
    // For CDMA messages, sometimes it will get failed if waiting a short
    // time to send message after in service.
    // I think it is because the network is not stable at that time.
    // It also needs to wait the cell location changed, but its changing is
    // very quickly than network change to stable.
    private static final int NETWORK_STABLE_DELAY = 10 * 60 * 1000;
    private static final int MAX_RETRY_TOKEN = 2;

    private static final String HEAD_SMS_IDENTIFIER = "REG";
    private static final String HEAD_SMS_VERSION = "01";

    private static final int EVENT_TIME_EXPIRED = 0;

    private PhoneStateListener[] listener = null;
    private ServiceState[] mServiceState = null;
    private CellLocation[] mCellLocation = null;
    private boolean mAcceptServiceState = false;
    private static final Object sServiceStateLock = new Object();

    private boolean[] mDeliverySuccess = null;
    private int mRetryToken = 0;
    public static final Semaphore sRegResultSemaphore = new Semaphore(1, true);

    private String mLastestAddress;

    @Override
    public void onCreate() {
        Log.d(TAG, "service creating");
        TelephonyManager telephonyManager = (TelephonyManager) getSystemService(
                Context.TELEPHONY_SERVICE);
        int phoneCount = telephonyManager.getPhoneCount();
        Log.d(TAG, "phone count " + phoneCount);

        mServiceState = new ServiceState[phoneCount];
        for (int i = 0; i < phoneCount; i++) {
            mServiceState[i] = new ServiceState();
        }

        mCellLocation = new CellLocation[phoneCount];
        for (int i = 0; i < phoneCount; i++) {
            int phoneType = telephonyManager.getCurrentPhoneType(i);
            if (phoneType == TelephonyManager.PHONE_TYPE_GSM) {
                mCellLocation[i] = new GsmCellLocation();
            } else if (phoneType == TelephonyManager.PHONE_TYPE_CDMA) {
                mCellLocation[i] = new CdmaCellLocation();
            } else {
                mCellLocation[i] = null;
            }
        }

        listener = new PhoneStateListener[phoneCount];
        for (long i = 0; i < phoneCount; i++) {
            if (SubscriptionManager.getSubId((int)i)[0] < 0) {
                continue;
            }
            listener[(int)i] = new PhoneStateListener(SubscriptionManager.getSubId((int)i)[0]) {
                @Override
                public void onServiceStateChanged(ServiceState serviceState) {
                    Log.d(TAG, "slot " + SubscriptionManager.getSlotId(mSubId) + " (sub "
                            + mSubId+ ") on service state changed: " + serviceState);
                    int phoneType = TelephonyManager.PHONE_TYPE_NONE;
                    synchronized (sServiceStateLock) {
                        mServiceState[SubscriptionManager.getSlotId(mSubId)] = serviceState;

                        Log.d(TAG, "accept service state " + mAcceptServiceState);
                        if (!mAcceptServiceState) {
                            return;
                        }

                        if (serviceState.getState() != ServiceState.STATE_IN_SERVICE) {
                            return;
                        }

                        TelephonyManager telephonyManager = (TelephonyManager) getSystemService(
                                Context.TELEPHONY_SERVICE);
                        phoneType = telephonyManager.getCurrentPhoneType(mSubId);
                        Log.d(TAG, "phone type " + phoneType);
                        if (phoneType != TelephonyManager.PHONE_TYPE_GSM &&
                                phoneType != TelephonyManager.PHONE_TYPE_CDMA) {
                            return;
                        }

                        mAcceptServiceState = false;
                    }

                    scheduleRegistrationTask(NETWORK_STABLE_DELAY, (int) mSubId);
                }

                @Override
                public void onCellLocationChanged(CellLocation location) {
                    Log.d(TAG, "slot " + SubscriptionManager.getSlotId(mSubId) + " (sub "
                            + mSubId + ") on cell location changed: " + location);
                    mCellLocation[SubscriptionManager.getSlotId(mSubId)] = location;
                }
            };
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        int retValue = START_NOT_STICKY;
        if (intent == null) {
            return retValue;
        }
        if (!ACTION_START_SERVICE.equals(intent.getAction())) {
            return retValue;
        }

        String triggerAction = intent.getStringExtra(EXTRA_TRIGGER_ACTION);
        Log.d(TAG, "received trigger action: " +
                (triggerAction == null ? "null" : triggerAction));

        if (Intent.ACTION_BOOT_COMPLETED.equals(triggerAction)) {
            TelephonyManager telephonyManager = (TelephonyManager) getSystemService(
                    Context.TELEPHONY_SERVICE);
            for (int i = 0; i < listener.length; i++) {
                if (listener[i] == null){
                    continue;
                }
                telephonyManager.listen(listener[i],
                        PhoneStateListener.LISTEN_SERVICE_STATE |
                        PhoneStateListener.LISTEN_CELL_LOCATION);
            }

            scheduleRegistrationTask(FIRST_TIME_DELAY, 0);
        }
        else if (ACTION_SENT_RESULT.equals(triggerAction)) {
            handleSentResultIntent(intent);
        }
        else if (ACTION_DELIVERY_RESULT.equals(triggerAction)) {
            handleDeliveryResultIntent(intent);
        }

        return retValue;
    }

    @Override
    public void onDestroy() {
        listener = null;
        mServiceState = null;
        mCellLocation = null;
        mDeliverySuccess = null;
    }

    private void handleSentResultIntent(Intent intent) {
        int retryToken = intent.getIntExtra(EXTRA_RETRY_TOKEN, -1);
        Log.d(TAG, "incoming retry token " + retryToken +
                ", current retry token " + mRetryToken);
        if (retryToken != mRetryToken) {
            sRegResultSemaphore.release();
            return;
        }

        int resultCode = intent.getIntExtra(EXTRA_RESULT_CODE, 0);
        Log.d(TAG, "result code " + resultCode);
        if (resultCode == Activity.RESULT_OK) {
            sRegResultSemaphore.release();
        }
        else {
            checkRetryRegistration();
        }
    }

    private void handleDeliveryResultIntent(Intent intent) {
        int retryToken = intent.getIntExtra(EXTRA_RETRY_TOKEN, -1);
        Log.d(TAG, "incoming retry token " + retryToken +
                ", current retry token " + mRetryToken);
        if (retryToken != mRetryToken) {
            sRegResultSemaphore.release();
            return;
        }

        int refNum = intent.getIntExtra(EXTRA_REFERENCE_NUM, -1);
        Log.d(TAG, "reference number " + refNum);
        if (refNum == -1) {
            sRegResultSemaphore.release();
            return;
        }

        int resultCode = intent.getIntExtra(EXTRA_RESULT_CODE, 0);
        Log.d(TAG, "result code " + resultCode);
        if (resultCode == Activity.RESULT_OK) {

            mDeliverySuccess[refNum] = true;
            boolean allSuccess = true;
            for (int i = 0; i < mDeliverySuccess.length; i++) {
                if (mDeliverySuccess[i] == false) {
                    allSuccess = false;
                    break;
                }
            }

            Log.d(TAG, "all delivery success " + allSuccess);
            if (allSuccess) {
                Log.d(TAG, "set registration result to " + REG_SUCCESS);
                setRegistrationResult(this, REG_SUCCESS);
                sRegResultSemaphore.release();

                finishRegistration(true);
            }
            else {
                sRegResultSemaphore.release();
            }
        }
        else {
            checkRetryRegistration();
        }
    }

    private void checkRetryRegistration() {
        mRetryToken++;
        if (mRetryToken <= MAX_RETRY_TOKEN) {
            sRegResultSemaphore.release();

            if (mRetryToken == MAX_RETRY_TOKEN) {
                scheduleRegistrationTask(RETRY_DELAY, 1);
            } else {
                scheduleRegistrationTask(RETRY_DELAY, 0);
            }
        }
        else {
            Log.d(TAG, "set registration result to " + REG_FAIL);
            setRegistrationResult(this, REG_FAIL);
            sRegResultSemaphore.release();

            finishRegistration(false);
        }
    }

    private void finishRegistration(boolean success) {
        Log.d(TAG, "registration finished, success " + success);
        TelephonyManager telephonyManager = (TelephonyManager) getSystemService(
                Context.TELEPHONY_SERVICE);
        for (int i = 0; i < listener.length; i++) {
            telephonyManager.listen(listener[i], PhoneStateListener.LISTEN_NONE);
        }

        if (success) {
            disableReceiver(this);
            Intent intent = new Intent(this, RegistrationPromptActivity.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
        }

        Log.d(TAG, "service will stop-self");
        stopSelf();
    }

    private void performRegistration(int subscription, int phoneType) {
        Log.d(TAG, "perform registration on sub " + subscription + " phone type " + phoneType);

        String regMsg = null;
        if (phoneType == TelephonyManager.PHONE_TYPE_GSM) {
            regMsg = constructGsmTextRegMessage(subscription, phoneType);
        }
        else if (phoneType == TelephonyManager.PHONE_TYPE_CDMA) {
            regMsg = constructCdmaTextRegMessage(subscription, phoneType);
        }

        Log.d(TAG, "construct reg-msg success " + (regMsg != null));
        if (regMsg == null) {
            try {
                sRegResultSemaphore.acquire();
            } catch (InterruptedException e) {
                Log.e(TAG, "acquire exception: " + e);
            }

            checkRetryRegistration();
            return;
        }

        String destAddress = getDestinationAddress(subscription);
        Log.d(TAG, "destination address " + (destAddress == null ? "null" : destAddress));
        if (destAddress == null) {
            try {
                sRegResultSemaphore.acquire();
            } catch (InterruptedException e) {
                Log.e(TAG, "acquire exception: " + e);
            }

            checkRetryRegistration();
            return;
        }

        // TODO Make sure to send TEXT or DATA message.
        sendTextRegMessage(regMsg, destAddress, subscription, phoneType);
    }

    private void sendTextRegMessage(String regMsg, String destinationAddress, int subscription, int phoneType) {
        mLastestAddress = destinationAddress;
        ArrayList<String> sendMessages = SmsManager.getSmsManagerForSubscriptionId(
                subscription).divideMessage(regMsg);
        Log.d(TAG, "prepare to send reg-msg, size " + sendMessages.size() + ", retry token " + mRetryToken);
        for (String dividedMsg : sendMessages) {
            Log.d(TAG, "divided message [" + dividedMsg + "]");
        }

        ArrayList<PendingIntent> sentIntents = new ArrayList<PendingIntent>(sendMessages.size());
        for (int i = 0; i < sendMessages.size(); i++) {
            // The requestCode must be different between each other.
            // If not different, the result intent extra will be the same for every message part.
            // The Flag must be FLAG_UPDATE_CURRENT or the extra will always be the one in the first
            // time getting broadcast.
            sentIntents.add(PendingIntent.getBroadcast(this, i,
                    new Intent(ACTION_SENT_RESULT)
                            .putExtra(EXTRA_RETRY_TOKEN, mRetryToken)
                            .putExtra(EXTRA_REFERENCE_NUM, i),
                    PendingIntent.FLAG_UPDATE_CURRENT));
        }

        ArrayList<PendingIntent> deliveryIntents = new ArrayList<PendingIntent>(sendMessages.size());
        for (int i = 0; i < sendMessages.size(); i++) {
            deliveryIntents.add(PendingIntent.getBroadcast(this, i,
                    new Intent(ACTION_DELIVERY_RESULT)
                            .putExtra(EXTRA_RETRY_TOKEN, mRetryToken)
                            .putExtra(EXTRA_REFERENCE_NUM, i),
                    PendingIntent.FLAG_UPDATE_CURRENT));
        }

        mDeliverySuccess = new boolean[sendMessages.size()];
        for (int i = 0; i < mDeliverySuccess.length; i++) {
            mDeliverySuccess[i] = false;
        }

        if (phoneType == TelephonyManager.PHONE_TYPE_CDMA) {
            // CDMA messages will only request status report at the last message part.
            // So set the rest parts of delivery status to true.
            for (int i = 0; i < mDeliverySuccess.length - 1; i++) {
                mDeliverySuccess[i] = true;
            }
        }

        SmsManager.getSmsManagerForSubscriptionId(SubscriptionManager.getSubId(subscription)[0])
                .sendMultipartTextMessage(
                destinationAddress,
                null,
                sendMessages,
                sentIntents,
                deliveryIntents
                );

        Log.d(TAG, "the registration message has been sent");

        // Delete the registration messages for not displaying in Message application
        ContentResolver contentResolver = getContentResolver();
        if (contentResolver != null) {
            Cursor cursor = contentResolver.query(
                    Uri.parse("content://sms/"), new String[] { "_id" }, "address = ?",
                    new String[] { mLastestAddress },"date DESC");
            int messageNum = 0;
            while ((cursor.moveToNext()) && (messageNum < sendMessages.size())) {
                long messageId = cursor.getLong(0);
                contentResolver.delete(Uri.parse("content://sms/" + messageId), null, null);
                Log.d(TAG, "Delete SMS message ID:" + messageId);
                messageNum++;
            }
            cursor.close();
        }
    }

    private String getDestinationAddress(int subscription) {
        // TODO Make sure the following code block is removed. It's only used for dev & test.
        String destAddress = SystemProperties.get("persist.salestracker.destAddr");
        if (!destAddress.isEmpty()) {
            return destAddress;
        }

        TelephonyManager telephonyManager = (TelephonyManager) getSystemService(
                Context.TELEPHONY_SERVICE);
        String simOperator = telephonyManager.getSimOperator(
                SubscriptionManager.getSubId(subscription)[0]);
        Log.d(TAG, "sim operator " + (simOperator == null ? "null" : simOperator));
        if (simOperator == null || simOperator.isEmpty()) {
            return null;
        }
        String mcc = simOperator.substring(0, 3);

        for (int i = 0; i < MCC_DEST_ADDRESS.length; i++) {
            if (MCC_DEST_ADDRESS[i][0].equals(mcc)) {
                if (!MCC_DEST_ADDRESS[i][1].isEmpty()) {
                    return MCC_DEST_ADDRESS[i][1];
                }
            }
        }
        return null;
    }

    private int[] getAvailableService(int preferredSub) {
        TelephonyManager telephonyManager = (TelephonyManager) getSystemService(
                Context.TELEPHONY_SERVICE);

        int i = preferredSub;
        while (true) {
            if (mServiceState[i].getState() == ServiceState.STATE_IN_SERVICE) {
                int phoneType = telephonyManager.getCurrentPhoneType(i);
                if (phoneType == TelephonyManager.PHONE_TYPE_GSM ||
                        phoneType == TelephonyManager.PHONE_TYPE_CDMA) {

                    return new int[] {i, phoneType};
                }
            }

            if (preferredSub == 0) {
                i++;
                if (i >= mServiceState.length) break;
            } else {
                i--;
                if (i < 0) break;
            }
        }
        return null;
    }

    private void scheduleRegistrationTask(long delay, int preferredSub) {
        Log.d(TAG, "schedule registration task, delay " + delay + ", preferred sub " + preferredSub);
        mTask = new RegistrationTask(preferredSub);
        mTimer.schedule(mTask, delay);
    }

    private final Timer mTimer = new Timer();

    private TimerTask mTask = null;

    private class RegistrationTask extends TimerTask {
        private int mPreferredSub = 0;

        public RegistrationTask(int preferredSub) {
            mPreferredSub = preferredSub;
        }

        @Override
        public void run() {
            Message msg = mHandler.obtainMessage(EVENT_TIME_EXPIRED, mPreferredSub);
            msg.sendToTarget();
        }
    }

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case EVENT_TIME_EXPIRED:
                int[] availableService = null;
                int preferredSub = (Integer)msg.obj;
                Log.d(TAG, "time expired, preferred sub " + preferredSub);
                synchronized (sServiceStateLock) {
                    availableService = getAvailableService(preferredSub);

                    if (availableService == null) {
                        Log.d(TAG, "there is no available service");
                        mAcceptServiceState = true;
                    }
                }

                if (availableService != null) {
                    Log.d(TAG, "available service on sub " + availableService[0] +
                            " phone type " + availableService[1]);
                    performRegistration(availableService[0], availableService[1]);
                }
                break;
            }
        }
    };

    private String constructGsmTextRegMessage(int subscription, int phoneType) {
        StringBuilder regMsg = new StringBuilder();

        // SMS identifier
        regMsg.append(HEAD_SMS_IDENTIFIER);
        regMsg.append(":");

        // SMS version
        regMsg.append(HEAD_SMS_VERSION);
        regMsg.append(":");

        // 01 MCC MNC
        regMsg.append("01");
        String opNum = mServiceState[subscription].getOperatorNumeric();
        Log.d(TAG, "mcc mnc " + (opNum == null ? "null" : opNum));
        if (opNum == null) {
            return null;
        }
        regMsg.append(opNum);
        regMsg.append(":");

        // 02 Cell ID, 03 Lac ID
        if (mCellLocation[subscription] == null) {
            Log.d(TAG, "cell location is null");
            return null;
        }
        if (mCellLocation[subscription] instanceof GsmCellLocation) {
            int cellId = ((GsmCellLocation) mCellLocation[subscription]).getCid();
            Log.d(TAG, "cell ID " + cellId);
            if (cellId == -1) {
                return null;
            }
            regMsg.append("02");
            regMsg.append(cellId);
            regMsg.append(":");

            int lacId = ((GsmCellLocation) mCellLocation[subscription]).getLac();
            Log.d(TAG, "lac ID " + lacId);
            if (lacId == -1) {
                return null;
            }
            regMsg.append("03");
            regMsg.append(lacId);
            regMsg.append(":");
        }
        else {
            Log.d(TAG, "not a gsm cell location");
            return null;
        }

        // TODO make sure the following:
        // Which is the Model No.?
        // Which is the Hardware No.?
        // Which is the Software No.?
        // What is the checksum algorithm? The checksum string must be two characters.
        // Besides which Charset should be used in the algorithm?
        // For CDMA messages it also needs to make sure the same things above.

        // 04 Model No.
        regMsg.append("04");
        Log.d(TAG, "model no  GSM. " + Build.MODEL.replaceAll(" ",""));
        regMsg.append(Build.MODEL.replaceAll(" ",""));
        regMsg.append(":");

        // 05 IMEI
        regMsg.append("05");
        TelephonyManager telephonyManager = (TelephonyManager) getSystemService(
                Context.TELEPHONY_SERVICE);
        int phoneCount = telephonyManager.getPhoneCount();
        for (int i = 0; i < phoneCount; i++) {
            String deviceId = telephonyManager.getDeviceId(i);
            Log.d(TAG, "IMEI_" + i + ": " + (deviceId == null ? "null" : deviceId));
            if (deviceId == null) {
                return null;
            }
            regMsg.append(deviceId);
            if (i < phoneCount - 1) {
                regMsg.append(",");
            }
        }
        regMsg.append(":");

        // 06 Hardware No.
        regMsg.append("06");
        //Log.d(TAG, "hardware no. " + Build.SERIAL);
        //regMsg.append(Build.SERIAL);
        Log.d(TAG, "hardware no. " + SystemProperties.get("ro.hardware.custom_version"));
        regMsg.append(SystemProperties.get("ro.hardware.custom_version"));
        regMsg.append(":");

        // 07 Software No.
        regMsg.append("07");
        //Log.d(TAG, "software no. " + Build.VERSION.RELEASE);
        //regMsg.append(Build.VERSION.RELEASE);
        Log.d(TAG, "software no. " + SystemProperties.get("ro.product.sw.version"));
        regMsg.append(SystemProperties.get("ro.product.sw.version"));
        regMsg.append(":");

        // checksum
        String checkSum = getRemainderCheckSum(regMsg.toString());
        regMsg.append(checkSum);

        String sendMsg = regMsg.toString();
        Log.d(TAG, "construct GSM msg done: " + sendMsg);
        return sendMsg;
    }

    private String constructCdmaTextRegMessage(int subscription, int phoneType) {
        StringBuilder regMsg = new StringBuilder();

        // SMS identifier
        regMsg.append(HEAD_SMS_IDENTIFIER);
        regMsg.append(":");

        // SMS version
        regMsg.append(HEAD_SMS_VERSION);
        regMsg.append(":");

        // 01 SID NID
        regMsg.append("01");
        int systemId = mServiceState[subscription].getSystemId();
        Log.d(TAG, "system ID " + systemId);
        if (systemId == -1) {
            return null;
        }
        regMsg.append(systemId);
        regMsg.append(",");
        int networkId = mServiceState[subscription].getNetworkId();
        Log.d(TAG, "network ID " + networkId);
        if (networkId == -1) {
            return null;
        }
        regMsg.append(networkId);
        regMsg.append(":");

        // 02 Latitude, 03 Longitude
        if (mCellLocation[subscription] == null) {
            Log.d(TAG, "cell location is null");
            return null;
        }
        if (mCellLocation[subscription] instanceof CdmaCellLocation) {
            int latitude = ((CdmaCellLocation) mCellLocation[subscription]).getBaseStationLatitude();
            Log.d(TAG, "latitude " + latitude);
            if (latitude == CdmaCellLocation.INVALID_LAT_LONG) {
                return null;
            }
            regMsg.append("02");
            regMsg.append(latitude);
            regMsg.append(":");

            int longitude = ((CdmaCellLocation) mCellLocation[subscription]).getBaseStationLongitude();
            Log.d(TAG, "longitude " + longitude);
            if (longitude == CdmaCellLocation.INVALID_LAT_LONG) {
                return null;
            }
            regMsg.append("03");
            regMsg.append(longitude);
            regMsg.append(":");
        }
        else {
            Log.d(TAG, "not a cdma cell location");
            return null;
        }

        // 04 Model No.
        regMsg.append("04");
        Log.d(TAG, "model no.CDMA " + Build.MODEL.replaceAll(" ",""));
        regMsg.append(Build.MODEL.replaceAll(" ",""));
        regMsg.append(":");

        // 05 MEID
        regMsg.append("05");
        TelephonyManager telephonyManager = (TelephonyManager) getSystemService(
                Context.TELEPHONY_SERVICE);
        int phoneCount = telephonyManager.getPhoneCount();
        for (int i = 0; i < phoneCount; i++) {
            String deviceId = telephonyManager.getDeviceId(i);
            Log.d(TAG, "MEID_" + i + ": " + (deviceId == null ? "null" : deviceId));
            if (deviceId == null) {
                return null;
            }
            regMsg.append(deviceId);
            if (i < phoneCount - 1) {
                regMsg.append(",");
            }
        }
        regMsg.append(":");

        // 06 Hardware No.
        regMsg.append("06");
        //Log.d(TAG, "hardware no. " + Build.SERIAL);
        //regMsg.append(Build.SERIAL);
        Log.d(TAG, "hardware no. " + SystemProperties.get("ro.hardware.custom_version"));
        regMsg.append(SystemProperties.get("ro.hardware.custom_version"));
        regMsg.append(":");

        // 07 Software No.
        regMsg.append("07");
        //Log.d(TAG, "software no. " + Build.VERSION.RELEASE);
        //regMsg.append(Build.VERSION.RELEASE);
        Log.d(TAG, "software no. " + SystemProperties.get("ro.product.sw.version"));
        regMsg.append(SystemProperties.get("ro.product.sw.version"));
        regMsg.append(":");

        // checksum
        String checkSum = getRemainderCheckSum(regMsg.toString());
        regMsg.append(checkSum);

        String sendMsg = regMsg.toString();
        Log.d(TAG, "construct CDMA msg finished: " + sendMsg);
        return sendMsg;
    }

    /**
     * Use CRC32 to generate checksum.
     * Note the return String is not two characters.
     */
    private String getCRC32CheckSum(String input) {
        Log.d(TAG, "checksum input: " + input);
        CRC32 crc = new CRC32();
        crc.update(input.getBytes(Charset.forName("US-ASCII")));

        Log.d(TAG, "checksum value: " + crc.getValue());
        return String.valueOf(crc.getValue());
    }

    /**
     * Take a summation short(two bytes) by short to generate checksum.
     * The return String is two characters.
     */
    private String getSumCheckSum(String input) {
        Log.d(TAG, "checksum input: " + input);
        byte[] bytes = input.getBytes(Charset.forName("US-ASCII"));

        short sum = 0;
        for (int i = 0; i < bytes.length;) {
            if (i + 1 < bytes.length) {
                sum += (short) (bytes[i] << 8 + bytes[i + 1]);
                i += 2;
            }
            else {
                sum += (short) (bytes[i] << 8);
                i++;
            }
        }

        byte[] outBytes = new byte[2];
        outBytes[0] = (byte) (sum >> 8);
        outBytes[1] = (byte) sum;
        Log.d(TAG, "checksum value: " + outBytes[0] + " " + outBytes[1]);
        String output = new String(outBytes, Charset.forName("US-ASCII"));
        return output;
    }

    private String getRemainderCheckSum(String input) {
        Log.d(TAG, "checksum input: " + input);
        byte[] bytes = input.getBytes(Charset.forName("US-ASCII"));

        int sum = 0;
        int remainder =0;
        for (int i = 0; i < bytes.length;) {
                sum += (int) (bytes[i]);
                //Log.d(TAG, "checksum byte: " + (int) (bytes[i]));
                //Log.d(TAG, "checksum sum: " + sum);
                i++;
        }

        remainder = sum%255;
        Log.d(TAG, "checksum value: " + remainder);
        //Integer.toHexString(remainder);
        String output = Integer.toHexString(remainder);
        Log.d(TAG, "checksum remainder: " + output);
        return output;
    }


    public static int getRegistrationResult(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        int regResult = prefs.getInt(FLAG_REG_RESULT,
                REG_UNKNOWN);
        return regResult;
    }

    public static void setRegistrationResult(Context context, int regResult) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        prefs.edit()
                .putInt(FLAG_REG_RESULT, regResult)
                .commit();
    }

    public static void disableReceiver(Context context) {
        ComponentName component = new ComponentName(context, SalesTrackerReceiver.class);
        PackageManager packageManager = context.getPackageManager();
        packageManager.setComponentEnabledSetting(component,
                PackageManager.COMPONENT_ENABLED_STATE_DISABLED, PackageManager.DONT_KILL_APP);
    }
}
