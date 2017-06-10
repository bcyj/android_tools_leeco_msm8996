/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (C) 2013 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/


package com.qualcomm.fastdormancy;

import android.app.Service;
import android.os.IBinder;
import android.content.Intent;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.content.Context;
import android.net.TrafficStats;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.content.BroadcastReceiver;
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.IntentFilter;
import android.os.PowerManager;
import com.qualcomm.qcrilhook.QcRilHook;

public class FastDormancyService extends Service {
    static final String TAG = "FastDormancyService";
    static final boolean DEBUG = false;
    static final int MSG_ENTER_DORMANCY = 1;
    static final int MSG_DETECT_DATA_ACTIVITY = 2;
    static final int DISPLAY_ON_TIMER = 15 * 1000;
    static final int DISPLAY_OFF_TIMER = 3 * 1000;
    static final int POLLING_TIME = 1 * 1000;
    static final int DORMANT_MODE = 1;
    static final int AGGRESSIVE_DORMANT_MODE = 2;
    static final int DATA_ACTIVE_MODE = 3;
    private int  mFDset = DORMANT_MODE;
    private boolean mIsScreenOn = true;
    private boolean mSynPacketTrigger = false;
    private static int ALARM_TIME = DISPLAY_ON_TIMER;
    static final String PROPERTY_FD_SCROFF_TIMER = "persist.fd.scroff.timer";
    static final String PROPERTY_FD_SCRON_TIMER = "persist.fd.scron.timer";
    static final String PROPERTY_TX_MASK_ENABLE = "persist.data.tcp_rst_drop";
    static final String PACKET_MAX_COUNT = "persist.data.pkt_max_count";
    static final String FD_ENABLE_NETWORKS = "persist.fd.enable.networks";
    PendingIntent mPendingIntent;
    AlarmManager mAlarmManager;
    PowerManager mPowerManager;
    PowerManager.WakeLock mDormancyWL;
    private TelephonyManager mTelephonyManager;
    private int mDataState = -1;
    private int mDataActivity = -1;
    private int mNetworkType = -1;
    private int mFdEnableNetwork = -1;

    private Handler myHandler = new Handler(){
        public void handleMessage(Message msg){
            switch(msg.what){
                case MSG_ENTER_DORMANCY:
                    enterDormancy();
                    break;
                case MSG_DETECT_DATA_ACTIVITY:
                    detectMobileDataActivity(false, false);
                    break;
                default:
                    Log.e(TAG,"unexpected event "+msg.what);
                    break;
            }
        }
    };

    private class MobileDataStatistics {
        private long tx;
        private long rx;
        private long txTcp;
        private long rxTcp;
        private long txBytes;
        private long rxBytes;

        private MobileDataStatistics() {
            tx = -1;
            rx = -1;
            txTcp = -1;
            rxTcp = -1;
            txBytes = -1;
            rxBytes = -1;
        }

        private MobileDataStatistics(MobileDataStatistics s) {
            tx = s.tx;
            rx = s.rx;
            txTcp = s.txTcp;
            rxTcp = s.rxTcp;
            txBytes = s.txBytes;
            rxBytes = s.rxBytes;
        }

        private void update(){
            tx = TrafficStats.getMobileTxPackets();
            rx = TrafficStats.getMobileRxPackets();
            txTcp = TrafficStats.getMobileTcpTxPackets();
            rxTcp = TrafficStats.getMobileTcpRxPackets();
            txBytes = TrafficStats.getMobileTxBytes();
            rxBytes = TrafficStats.getMobileRxBytes();
        }
    }

    private QcRilHook mQcRilHook;
    private MobileDataStatistics mDataUsage = new MobileDataStatistics();

    private void scheduleDelayedMsg() {
        myHandler.sendMessageDelayed(
            myHandler.obtainMessage(MSG_DETECT_DATA_ACTIVITY), POLLING_TIME);
    }

    private void scheduleAlarm(int alarmTime) {
        mAlarmManager.cancel(mPendingIntent);
        mAlarmManager.set(AlarmManager.RTC_WAKEUP, System.currentTimeMillis()
            + alarmTime , mPendingIntent);
    }

    private synchronized void detectMobileDataActivity(boolean isAlarmExpired,
            boolean ScrStateChanged) {
        long sent, received, sentTcp, receivedTcp, sentTxBytes, receivedRxBytes;
        sent = received = sentTcp = receivedTcp = sentTxBytes = receivedRxBytes = -1;

        Log.d(TAG, "detect mobile data activity! isAlarmExpired: "
                + isAlarmExpired + "  mFDset: " + mFDset );
        myHandler.removeMessages(MSG_DETECT_DATA_ACTIVITY);
        if (mFDset != DORMANT_MODE) {
            MobileDataStatistics prev = new MobileDataStatistics(mDataUsage);
            mDataUsage.update();
            sent = mDataUsage.tx - prev.tx;
            received = mDataUsage.rx - prev.rx;
            Log.d(TAG,"sent:" + sent + " received:" + received);
            if (!mIsScreenOn && (mFDset == AGGRESSIVE_DORMANT_MODE)) {
                sentTcp = mDataUsage.txTcp - prev.txTcp;
                sentTxBytes = mDataUsage.txBytes - prev.txBytes;
                receivedTcp = mDataUsage.rxTcp - prev.rxTcp;
                receivedRxBytes = mDataUsage.rxBytes - prev.rxBytes;
                int maxPktCnt = SystemProperties.getInt(PACKET_MAX_COUNT, 20);
                if (DEBUG) Log.d(TAG,"sentTcp:" + sentTcp + " receivedTcp:" + receivedTcp +
                    " sentTxBytes:" + sentTxBytes + " receivedRxBytes:" + receivedRxBytes);
                if (SystemProperties.getBoolean(PROPERTY_TX_MASK_ENABLE, false)) {
                    if (sent == 0 && (received <= receivedTcp) && receivedTcp > 0 &&
                        received < maxPktCnt && (receivedRxBytes / receivedTcp) <= 60) {
                        mSynPacketTrigger = true;
                    }
                } else {
                    Log.d(TAG, "rst packet drop feature is not enabled");
                }
            }
        }
        if ((mFDset != DORMANT_MODE) && ((sent == 0 && received == 0) || mSynPacketTrigger)) {
            if (isAlarmExpired && (mSynPacketTrigger || (mFDset == DATA_ACTIVE_MODE))) {
                Log.d(TAG, "enter dormancy");
                mDormancyWL.acquire();
                if (DEBUG) Log.d(TAG,"acquired WakeLock:" + mDormancyWL);
                if (mSynPacketTrigger) mSynPacketTrigger = false;
                myHandler.sendMessage(myHandler.obtainMessage(MSG_ENTER_DORMANCY));
            } else {
                if (DEBUG) Log.d(TAG, "schedule timer:" + POLLING_TIME + "ms");
                if (ScrStateChanged) scheduleAlarm(ALARM_TIME);
                if (!mSynPacketTrigger && isAlarmExpired) {
                    scheduleAlarm(ALARM_TIME - POLLING_TIME);
                }
                mFDset = DATA_ACTIVE_MODE;
                scheduleDelayedMsg();
            }
        } else {
            if (mFDset == DORMANT_MODE && !mIsScreenOn) {
                Log.d(TAG, "Alarm:" + POLLING_TIME);
                scheduleAlarm(POLLING_TIME);
                mFDset = AGGRESSIVE_DORMANT_MODE;
            } else {
                Log.d(TAG, "scheduling timer:" + POLLING_TIME +
                    "ms Alarm:" + ALARM_TIME+ "ms");
                scheduleDelayedMsg();
                scheduleAlarm(ALARM_TIME);
                mFDset = DATA_ACTIVE_MODE;
            }
        }
        if ((mFdEnableNetwork & (1 << mNetworkType)) == 0) {
            myHandler.removeMessages(MSG_DETECT_DATA_ACTIVITY);
            mAlarmManager.cancel(mPendingIntent);
            mFDset = DORMANT_MODE;
        }
    }

    private void broadcastReceiverInit(){
        BroadcastReceiver mAlarmReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context c, Intent i) {
            mDormancyWL.acquire();
            detectMobileDataActivity(true, false);
            mDormancyWL.release();
            }
        };
        registerReceiver(mAlarmReceiver, new IntentFilter("FDAlarmExpiry") );
        mPendingIntent = PendingIntent.getBroadcast( this, 0, new Intent("FDAlarmExpiry"),0 );
        mAlarmManager = (AlarmManager)(this.getSystemService( Context.ALARM_SERVICE ));
    }

    private void screenStateNotification(){
        BroadcastReceiver mScreenStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context c, Intent intent) {
            if (intent.getAction().equals(Intent.ACTION_SCREEN_OFF)) {
                mIsScreenOn = false;
                ALARM_TIME = SystemProperties.getInt(
                        PROPERTY_FD_SCROFF_TIMER, DISPLAY_OFF_TIMER);
            } else if (intent.getAction().equals(Intent.ACTION_SCREEN_ON)) {
                mIsScreenOn = true;
                ALARM_TIME = SystemProperties.getInt(
                        PROPERTY_FD_SCRON_TIMER, DISPLAY_ON_TIMER);
            }
            if (DEBUG) Log.d(TAG, "Screen state changed. ALARM_TIME" + ALARM_TIME + "ms");
            if (mFDset != DORMANT_MODE) {
                detectMobileDataActivity(false, true);
            }
            }
        };
        IntentFilter filter = new IntentFilter(Intent.ACTION_SCREEN_ON);
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        registerReceiver(mScreenStateReceiver, filter);
    }

    private void startDataActivityCheck() {
        if (DEBUG) Log.d(TAG,"startDataActivityCheck()");
        if ((mDataState == TelephonyManager.DATA_CONNECTED) && (mFDset ==
            DORMANT_MODE) && ((mFdEnableNetwork & (1 << mNetworkType)) != 0)
            && (mDataActivity < TelephonyManager.DATA_ACTIVITY_DORMANT)) {
            detectMobileDataActivity(false, false);
        }
    }

    private void enterDormancy(){
        mQcRilHook.qcRilGoDormant("");
        mFDset = DORMANT_MODE;
        mDormancyWL.release();
        if (DEBUG) Log.d(TAG,"released WakeLock:" + mDormancyWL);
    }

    @Override
    public IBinder onBind(Intent intent){
        return null;
    }

    @Override
    public void onCreate(){
        if (DEBUG) Log.d(TAG,"onCreate");
        mQcRilHook = new QcRilHook(getApplicationContext());
        mPowerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
        mDormancyWL = mPowerManager.newWakeLock(
                PowerManager.PARTIAL_WAKE_LOCK, TAG);
        mTelephonyManager = (TelephonyManager)getApplicationContext().getSystemService
                (Context.TELEPHONY_SERVICE);
        mTelephonyManager.listen(mPhoneStateListener,
                PhoneStateListener.LISTEN_DATA_CONNECTION_STATE
                        | PhoneStateListener.LISTEN_DATA_ACTIVITY);
        broadcastReceiverInit();
        screenStateNotification();
        mIsScreenOn = mPowerManager.isScreenOn();
        if (!mIsScreenOn) {
            ALARM_TIME = DISPLAY_OFF_TIMER;
        }
        if (DEBUG) Log.d(TAG,"isScreenOn:"+ mIsScreenOn+ "ALARM_TIME:" + ALARM_TIME);
    }

    PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onDataConnectionStateChanged(int state, int networkType) {
            if (DEBUG) {
                Log.d(TAG, "onDataConnectionStateChanged: state=" + state
                        + " type=" + networkType);
            }
            mDataState = state;
            mNetworkType = networkType;
            /* if property is not set, FD is enabled for 3GPP only.
            0x28708 - 101000011100001000 - each bit coresponds to one network
            NETWORK_TYPE_UMTS = 3;
            NETWORK_TYPE_HSDPA = 8;
            NETWORK_TYPE_HSUPA = 9;
            NETWORK_TYPE_HSPA = 10;
            NETWORK_TYPE_HSPAP = 15;
            NETWORK_TYPE_TD_SCDMA = 17;
            */
            mFdEnableNetwork = SystemProperties.getInt(FD_ENABLE_NETWORKS, 0x28708);
            if (((mFdEnableNetwork & (1 << mNetworkType)) == 0) && (mFDset != DORMANT_MODE)) {
                myHandler.removeMessages(MSG_DETECT_DATA_ACTIVITY);
                mAlarmManager.cancel(mPendingIntent);
                mFDset = DORMANT_MODE;
            }
        }

        @Override
        public void onDataActivity(int direction) {
            if (DEBUG) {
                Log.d(TAG, "onDataActivity: direction=" + direction);
            }
            mDataActivity = direction;
            startDataActivityCheck();
        }
    };
}
