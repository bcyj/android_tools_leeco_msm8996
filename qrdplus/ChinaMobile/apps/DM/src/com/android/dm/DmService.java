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

import android.os.Debug;
import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.telephony.SmsManager;
import android.telephony.SubscriptionManager;
import android.os.IBinder;
import android.util.Log;
import android.view.ViewGroup.LayoutParams;
import android.view.ViewGroup.MarginLayoutParams;
import android.widget.TextView;
import android.os.Build;
import android.os.SystemProperties;
//import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Vector;
import android.os.Handler;
import android.os.Message;

import com.android.dm.vdmc.Vdmc;
import com.android.dm.vdmc.MyTreeIoHandler;
//import com.redbend.vdm.NIAMsgHandler.UIMode;
import android.app.AlertDialog;
import android.content.DialogInterface;

import android.provider.Settings;
import android.provider.Telephony;
import android.content.ContentValues;
import android.database.Cursor;
import android.graphics.Color;

import com.android.internal.telephony.TelephonyProperties;

import com.android.dm.transaction.DMTransaction;
import android.app.PendingIntent; // add 2012.12.28 for TS723G-365

public class DmService extends Service {
    private String TAG = DmReceiver.DM_TAG + "DmService: ";
    private static final String mLastImsiFile = "lastImsi.txt";
    private static TelephonyManager mTelephonyManager;
    private static boolean mMSimEnabled = false;
    private static boolean mIsHaveSendMsg = false; // if have send self registe
                                                   // message
    private static boolean mIsSelfRegOk = false; // if self registe ok
    private static boolean mSelfRegSwitch = true; // true: open self registe
                                                  // function false:close self
                                                  // registe function
    private static boolean mSmsDeliverReportSwitch = true;

    private static boolean mSmsInitComplete = false;
    private static boolean mSimInitComplete = false;

    private static final String PREFERENCE_NAME = "LastImsi";
    private static int MODE = MODE_PRIVATE;
    // private static DmService mInstance = null;
    private static DmService mInstance = null;
    // to justify whether service is started
    private static boolean mInited = false;
    private static Context mContext = null;
    private static int pppstatus = PhoneConstants.APN_TYPE_NOT_AVAILABLE;

    private static String mSmsAddr;
    private static String mSmsPort;
    private static String mServerAddr;
    private static String mApn = null;
    private static String mApnTemp = null;
    private static String mProxyTemp = null;
    private static String mProxyPortTemp = null;
    private static String mManufactory;
    private static String mModel;
    private static String mSoftVer;
    private static String mImeiStr;
    private static boolean mIsDebugMode;
    private static boolean mIsRealNetParam; // if current is read net parameter
    private static boolean mIsHaveInit = false;

    private static final String DM_CONFIG = "DmConfig";
    private static final String ITEM_DEBUG_MODE = "DebugMode";
    private static final String ITEM_REAL_PARAM = "RealParam";
    private static final String ITEM_MANUFACTORY = "Manufactory";
    private static final String ITEM_MODEL = "Model";
    private static final String ITEM_SOFTVER = "SoftVer";
    private static final String ITEM_IMEI = "IMEI";
    private static final String ITEM_SERVER_ADDR = "ServerAddr";
    private static final String ITEM_SMS_ADDR = "SmsAddr";
    private static final String ITEM_SMS_PORT = "SmsPort";
    private static final String ITEM_APN = "APN";
    private static final String ITEM_PROXY = "Proxy";
    private static final String ITEM_PROXY_PORT = "ProxyPort";
    private static final String ITEM_SELFREG_SWITCH = "SelfRegSwitch";
    private static final String ITEM_SMSDELIVERREPORT_SWITCH = "SmsDeliverReportSwitch";
    private static final String ITEM_SERVER_NONCE = "ServerNonce";
    private static final String ITEM_CLIENT_NONCE = "ClientNonce";

    private static final String ITEM_MESSAGE_SEND_COUNT = "SendCount"; // add
                                                                       // for
                                                                       // record
                                                                       // self
                                                                       // registe
                                                                       // message
                                                                       // send
                                                                       // count
    private static final String ITEM_SELFREG_CHOICE = "SelfRegTipChoice";
    private static final String ITEM_DATACON_CHOICE = "DataConnectTipChoice";
    private static final String ITEM_CARD_CHOICE = "CardChoice";
    public static final int SELFREG_CHOICE_INIT = 0;
    public static final int SELFREG_CHOICE_AGREE = 1;
    public static final int SELFREG_CHOICE_DISAGREE = 2;
    public static final int DATACON_CHOICE_INIT = 0;
    public static final int DATACON_CHOICE_AGREE = 1;
    public static final int DATACON_CHOICE_DISAGREE = 2;
    public static final int DATACON_CHOICE_ALWAYS = 4;

    public static final String APN_CMDM = "cmdm";
    public static final String APN_CMWAP = "cmwap";
    public static final String APN_CMNET = "cmnet";

    // Real net parameter
    private static final String REAL_SERVER_ADDR = "http://dm.monternet.com:7001";
    private static final String REAL_SMS_ADDR = "10654040";
    private static final String REAL_SMS_PORT = "16998";
    private static final String REAL_APN = APN_CMDM;

    // Lab net parameter
    private static final String LAB_SERVER_ADDR = "http://218.206.176.97:7001";
    // private static final String LAB_SERVER_ADDR = "http://218.206.176.97";
    private static final String LAB_SMS_ADDR = "1065840409";
    private static final String LAB_SMS_PORT = "16998";
    private static final String LAB_APN = APN_CMWAP;
    private DmJniInterface mDmInterface;
    private DMTransaction mDmTransaction;
    private static DmNetwork mNetwork = null;
    private static MyTreeIoHandler mTreeIoHandler = null;
    private Handler mHandler;
    final public static String SMS_ACTION_SENT = "com.android.mms.transaction.MESSAGE_SENT_DM";
    final public static String SMS_ACTION_DELIVER = "com.android.mms.transaction.MESSAGE_DELIVER_DM";
    final public static int MAX_SEND_COUNT = 10; // limit self registe message
                                                 // max send counts
    private static Object dataConSync = new Object();
    private static final int queryDialogTimeout = 5 * 60;
    private static final int queryDialogTimeoutMs = queryDialogTimeout * 1000;
    private static final int notifyid_forselfregist = 123;
    private static final int notifyid_fordataconnect = 122;

    private Vector<Integer> mSimArray = new Vector<Integer>();
    private Thread mDelayThread = null;
    private static final int SIM_INIT_MAX_DELAY = 20 * 1000;
    private static final int SIM_INIT_MIN_DELAY = 10 * 1000;
    private boolean isSimInitDelay = false;
    private Object mNotifySimInited = new Object();
    private int mPhoneId = 0;

    private class DmPhoneStateListener extends PhoneStateListener {

        private int mSubscription = 0;
        private int mSlotId = 0;

        public DmPhoneStateListener() {
            super();
        }

        public DmPhoneStateListener(int slotId, int subscription) {
            super(subscription);
            mSubscription = subscription;
            mSlotId = slotId;
        }

        private void sendSelfRegMsgDelay() {
            if (mSimArray.contains(new Integer(mSlotId))) {
                return;
            }
            mSimArray.add(new Integer(mSlotId));
            if (mDelayThread == null) {
                mDelayThread = new Thread(){
                    public void run() {
                        try {
                            synchronized(mNotifySimInited) {
                                mNotifySimInited.wait(SIM_INIT_MAX_DELAY);
                            }
                            if (isSimInitDelay) Thread.sleep(SIM_INIT_MIN_DELAY);
                        } catch (Exception e) {
                            Log.e(TAG, "Delay Thread for selfreg stopped by " + e);
                        }
                        int preferedSlot = selectPreferedSim();
                        Log.d(TAG, "select slot:"+preferedSlot);
                        if (preferedSlot != -1) {
                            mPhoneId = preferedSlot;
                            sendSelfRegMsg(preferedSlot);
                        }
                    }
                };
                mDelayThread.start();
            } else {
                // The second card inited, no need to start delay thread
                int phoneCnt = mTelephonyManager.getPhoneCount();
                if (mSimArray.size() >= phoneCnt) {
                    // All the sim card are initialized.
                    try {
                        synchronized(mNotifySimInited) {
                            mNotifySimInited.notify();
                            isSimInitDelay = true;
                        }
                    } catch (Exception e) {
                        Log.e(TAG, "Notify Error:" + e);
                    }
                }
            }
        }

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            Log.d(TAG, "onServiceStateChanged: current state = " + serviceState.getState());

            // judge if network is ready
            if (ServiceState.STATE_IN_SERVICE == serviceState.getState())
            {
                Log.d(TAG, "onServiceStateChanged: STATE_IN_SERVICE");

                // sim card is ready
                int simState = getSimState(mSlotId);
                if (TelephonyManager.SIM_STATE_READY == simState)
                {
                    // judge if cmcc sim card
                    String cmccStr1 = "46000";
                    String cmccStr2 = "46002";
                    String cmccStr3 = "46007";
                    String curOper = getNetworkOperator(mSubscription);
                    // String curOper1 = mTelephonyManager.getSimOperator();

                    Log.d(TAG, "onServiceStateChanged: SIM_STATE_READY");
                    Log.d(TAG, "onServiceStateChanged: getNetworkOperator = " + curOper);

                    if (curOper.equals(cmccStr1) || curOper.equals(cmccStr2)
                            || curOper.equals(cmccStr3))
                    {
                        Log.d(TAG, "onServiceStateChanged: is cmcc card!");

                        // send self registe message
                        Log.d(TAG, "------------------------------ mSimInitComplete OK mSim:"
                                + mMSimEnabled + " sub:" + mSubscription + " phoneid:" + mSlotId);
                        mSimInitComplete = true;
                        if (null != mInstance) {
                            int enable = Settings.Global.getInt(DmService.getInstance()
                                    .getContentResolver(), DmReceiver.DM_AUTOBOOT_SETTING,
                                    DmReceiver.DM_AUTOBOOT_SETTING_ENABLE);
                            if (enable == DmReceiver.DM_AUTOBOOT_SETTING_ENABLE) {
                                if (!mMSimEnabled) {
                                    sendSelfRegMsg(0);
                                } else {
                                    sendSelfRegMsgDelay();
                                }
                            }
                        }
                    }
                    else
                    {
                        Log.d(TAG, "onServiceStateChanged: not cmcc card!");
                        stopListeningServiceState(mSlotId);
                        // mContext.stopService(new
                        // Intent("com.android.dm.SelfReg"));
                    }
                }
                else
                {
                    Log.d(TAG,
                            "onServiceStateChanged: sim state = " + simState);
                }
            }
        }
    }

    private PhoneStateListener mPhoneStateListener = new DmPhoneStateListener();
    private PhoneStateListener[] mMSimPhoneStateListener = null;

    private class DMHandler extends Handler {

        public void handleMessage(Message msg) {

        }

    };

    @Override
    public void onCreate() {
        // Debug.startMethodTracing("Tracelog");
        mTelephonyManager = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        if (mTelephonyManager.isMultiSimEnabled()) {
            mMSimEnabled = true;
        } else {
            mMSimEnabled = false;
        }
        mContext = getBaseContext();
        initParam();
        mInstance = this;
        Log.d(TAG, "OnCreate: mInstance = " + mInstance);

        mHandler = new DMHandler();

        mDmInterface = new DmJniInterface(mContext);
        DMNativeMethod.JsaveDmJniInterfaceObject(mDmInterface);

        mDmTransaction = new DMTransaction(mContext, mHandler);
        DMNativeMethod.JsaveDmTransactionObject(mDmTransaction);

        if (mNetwork == null) {
            mNetwork = new DmNetwork(mContext);
        } else {
        }
        if (mTreeIoHandler == null) {
            mTreeIoHandler = new MyTreeIoHandler(mContext);
        } else {
        }
        // Start listening to service state
        if (getSelfRegSwitch())
        {
            setIsHaveSendSelfRegMsg(mContext, false, getSlotId());
            Log.d(TAG, "+++++setIsHaveSendSelfRegMsg(mContext, false)++++");
            if(mMSimEnabled) {
                int phoneCnt = mTelephonyManager.getPhoneCount();
                mMSimPhoneStateListener = new PhoneStateListener[phoneCnt];
                for (int i = 0; i < phoneCnt; ++i) {
                    int[] subIds = SubscriptionManager.getSubId(i);
                    if (subIds != null && subIds.length > 0) {
                        mMSimPhoneStateListener[i] = new DmPhoneStateListener(i,subIds[0]);
                        mTelephonyManager.listen(mMSimPhoneStateListener[i],
                                PhoneStateListener.LISTEN_SERVICE_STATE);
                    }
                }
            } else {
                mTelephonyManager.listen(mPhoneStateListener,
                        PhoneStateListener.LISTEN_SERVICE_STATE);
            }
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        // Debug.startMethodTracing("TraceDMlog");
        if (intent == null)
        {
            return Service.START_NOT_STICKY;
        }
        if (intent.getAction().equals("com.android.dm.SelfReg"))
        {
            Log.d(TAG, "onStart: com.android.dm.SelfReg");
            Log.d(TAG, "onStartCommand" + intent.getBooleanExtra("smsinit", false));
            Log.d(TAG, "onStartCommand notify reg=" + intent.getBooleanExtra(
                    "com.android.dm.notifySelfReg", false) + " con=" + intent.getBooleanExtra(
                    "com.android.dm.notifyDataCon", false));

            if (intent.getBooleanExtra("com.android.dm.notifySelfReg", false)) {
                postAgreedSelfRegist(intent.getBooleanExtra("com.android.dm.notifySelfReg.agree",
                        false), intent.getIntExtra("com.android.dm.notifySelfReg.sub", 0));
            } else if (intent.getBooleanExtra("com.android.dm.notifyDataCon", false)) {
                postAgreedDataConnect(intent.getBooleanExtra("com.android.dm.notifyDataCon.agree",
                        false), intent.getBooleanExtra("com.android.dm.notifyDataCon.always", false));
            } else if (intent.getBooleanExtra("smsinit", false)) {
                Log.d(TAG, "++++++++++++++++++++++ mSmsInitComplete OK ");
                mSmsInitComplete = true;
                if(!mMSimEnabled)
                    sendSelfRegMsg(0);
            }
        }
        else if (intent.getAction().equals("com.android.dm.NIA"))
        {
            Log.d(TAG, "onStart: com.android.dm.NIA");

            initConnectParam(); // insure dm connect network param is properly
                                // set

            byte[] body = intent.getByteArrayExtra("msg_body");
            String origin = intent.getStringExtra("msg_org");

            Log.d(TAG, "onStart: mInstance = " + mInstance);
            Log.d(TAG, "onStart: mContext = " + mContext);
            Log.d(TAG, "onStart: this = " + this);
            Vdmc.getInstance().startVDM(mContext, Vdmc.SessionType.DM_SESSION_SERVER, body, origin);
        }
        return Service.START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        // Stop listening for service state
        stopListeningServiceState();
        Log.d(TAG, "onDestroy: DmService is killed!");
        mInstance = null;
        mContext = null;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Deprecated
    public void onStart(Intent intent, int startId) {
        Log.d(TAG, "onStart: intent = " + intent + ", startId = " + startId);

    }

    public static boolean isServiceStart() {
        return (mInstance != null);
    }

    public static DmService getInstance()
    {
        if (null == mInstance)
        {
            return new DmService();
            // Log.d("DM ==> DmService: ",
            // "getInstance: new DmService() , mInstance = " + mInstance);
        }

        return mInstance;
    }

    public DmJniInterface getDmInterface()
    {
        return mDmInterface;
    }

    public static Context getContext()
    {
        // Log.d("DM ==> DmService: ", "getContext: mContext = " + mContext);
        return mContext;
    }

    // Stop listening for service state of current subScription
    public void stopListeningServiceState(int slotId)
    {
        if(mMSimEnabled && slotId < mMSimPhoneStateListener.length) {
            mTelephonyManager.listen(mMSimPhoneStateListener[slotId], 0);
        } else {
            if (null != mTelephonyManager) {
                mTelephonyManager.listen(mPhoneStateListener, 0);
            }
        }
    }

    // Stop listening for service state
    public void stopListeningServiceState()
    {
        if(mMSimEnabled) {
            for (int i = 0;i<mMSimPhoneStateListener.length;++i) {
                mTelephonyManager.listen(mMSimPhoneStateListener[i], 0);
            }
        } else {
            if (null != mTelephonyManager) {
                mTelephonyManager.listen(mPhoneStateListener, 0);
            }
        }
    }

    // init dm relative parameter
    private void initParam()
    {
        Log.d(TAG, "++++initParam+++++");
        SharedPreferences sharedPreferences;
        sharedPreferences = getSharedPreferences(DM_CONFIG, MODE);

        // init self registe switch
        if ((SystemProperties.get("ro.cmcc.protocol.test", "0").equals("1"))
                || (SystemProperties.get("ro.cta.test", "0").equals("1")))
        {
            // close self registe function
            setSelfRegSwitch(mContext, false);
        }
        else
        {
            // default is open
            mSelfRegSwitch = sharedPreferences.getBoolean(ITEM_SELFREG_SWITCH, true);
        }

        // init debug mode
        mIsDebugMode = sharedPreferences.getBoolean(ITEM_DEBUG_MODE, false);

        Log.d(TAG, "mIsDebugMode=" + mIsDebugMode);

        if (mIsDebugMode)
        {
            // init manufacture
            mManufactory = sharedPreferences.getString(ITEM_MANUFACTORY, Build.MANUFACTURER);

            // init model
            mModel = sharedPreferences.getString(ITEM_MODEL, Build.MODEL);

            // init software version
            String softVer = SystemProperties.get("ro.software.version", Build.UNKNOWN);
            mSoftVer = sharedPreferences.getString(ITEM_SOFTVER, softVer);

            // init imei
            mImeiStr = sharedPreferences.getString(
                    ITEM_IMEI,
                    mMSimEnabled ? mTelephonyManager.getDeviceId(getSlotId())
                            : mTelephonyManager.getDeviceId());
        }
        else
        {
            // init manufacture
            mManufactory = SystemProperties.get("ro.product.manufacturer", Build.UNKNOWN);

            Log.d(TAG, "name = " + SystemProperties.get("ro.product.name"));

            // init model
            mModel = SystemProperties.get("ro.product.model", Build.UNKNOWN);

            // init software version
            mSoftVer = SystemProperties.get("ro.software.version", Build.UNKNOWN);

            // init imei
            mImeiStr = mMSimEnabled ? mTelephonyManager.getDeviceId(getSlotId())
                    : mTelephonyManager.getDeviceId();
        }

        Log.d(TAG, "mManufactory = " + mManufactory);
        Log.d(TAG, "mModel = " + mModel);
        Log.d(TAG, "mSoftVer = " + mSoftVer);
        Log.d(TAG, "mImeiStr = " + mImeiStr);

        // according to cmcc test flag to decide current server relative
        // parameters
        // if (SystemProperties.get("ro.hisense.cmcc.test", "0").equals("1"))

        if (SystemProperties.get("ro.cmcc.test", "0").equals("1"))
        {
            setRealNetParam(mContext, false, false);
        }
        else
        {
            // init if use real net parameter
            mIsRealNetParam = sharedPreferences.getBoolean(ITEM_REAL_PARAM, true);
        }

        // init server address/sms address/sms port
        if (mIsRealNetParam)
        {
            mServerAddr = sharedPreferences.getString(ITEM_SERVER_ADDR, REAL_SERVER_ADDR);
            mSmsAddr = sharedPreferences.getString(ITEM_SMS_ADDR, REAL_SMS_ADDR);
            mSmsPort = sharedPreferences.getString(ITEM_SMS_PORT, REAL_SMS_PORT);
        }
        else
        {
            mServerAddr = sharedPreferences.getString(ITEM_SERVER_ADDR, LAB_SERVER_ADDR);
            mSmsAddr = sharedPreferences.getString(ITEM_SMS_ADDR, LAB_SMS_ADDR);
            mSmsPort = sharedPreferences.getString(ITEM_SMS_PORT, LAB_SMS_PORT);
        }

        // init apn/proxy/port
        initConnectParam();
        mInited = true;
    }

    // init dm connect network param,include apn/proxy/port
    private void initConnectParam()
    {
        if (mIsRealNetParam)
        {
            // mApn = sharedPreferences.getString(ITEM_APN, REAL_APN);
            mApn = getInitApn(mContext);
            if ((mApn == null) || mApn.equals("")) // add "" judgement for
                                                   // apns-conf.xml have
                                                   // modified CMCC DM apn to ""
                                                   // 2012.11.12
            {
                mApn = REAL_APN;
                setAPN(mContext, mApn);
            }
        }
        else
        {
            // mApn = sharedPreferences.getString(ITEM_APN, LAB_APN);
            mApn = getInitApn(mContext);
            if (mApn == null || mApn.equals("") || mApn.equals(REAL_APN)) // add
                                                                          // ""
                                                                          // judgement
                                                                          // for
                                                                          // apns-conf.xml
                                                                          // have
                                                                          // modified
                                                                          // CMCC
                                                                          // DM
                                                                          // apn
                                                                          // to
                                                                          // ""
                                                                          // 2012.11.12
            {
                mApn = LAB_APN;
                setAPN(mContext, mApn);
            }
        }
        if (mApn.equals(APN_CMWAP))
        {
            // cmwap apn, need set proxy and proxy port
            String str = null;
            str = getProxy(mContext);
            if (str == null || !str.equals("10.0.0.172"))
            {
                setProxy(mContext, "10.0.0.172");
            }
            str = getProxyPort(mContext);
            if (str == null || !str.equals("80"))
            {
                setProxyPort(mContext, "80");
            }
        }
        else
        {
            // cmnet or cmdm apn, no need set proxy and proxy port
            if (getProxy(mContext) != null)
            {
                setProxy(mContext, null);
            }
            if (getProxyPort(mContext) != null)
            {
                setProxyPort(mContext, null);
            }
        }
    }

    // get last successful self registe card imsi
    private String getLastImsi()
    {
        SharedPreferences sharedPreferences = getSharedPreferences(PREFERENCE_NAME, MODE);
        String imsi = sharedPreferences.getString("IMSI", "");
        Log.d(TAG, "getLastImsi : imsi = " + imsi);
        return imsi;
    }

    // save current self registe success card imsi
    protected boolean saveImsi(Context context, String imsi)
    {
        Log.d(TAG, "saveImsi: imsi = " + imsi);
        SharedPreferences sharedPreferences = context.getSharedPreferences(PREFERENCE_NAME, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString("IMSI", imsi);
        editor.commit();
        return true;
    }

    // get current self registe state
    protected boolean isSelfRegOk()
    {
        SharedPreferences sharedPreferences = getSharedPreferences(PREFERENCE_NAME, MODE);
        mIsSelfRegOk = sharedPreferences.getBoolean("IsSelfRegOk", false);
        Log.d(TAG, "isSelfRegOk: return " + mIsSelfRegOk);
        return mIsSelfRegOk;
    }

    // set current self registe state
    protected void setSelfRegState(Context context, boolean isSuccess)
    {
        Log.d(TAG, "setSelfRegState: to " + isSuccess);
        mIsSelfRegOk = isSuccess;
        SharedPreferences sharedPreferences = context.getSharedPreferences(PREFERENCE_NAME, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putBoolean("IsSelfRegOk", isSuccess);
        editor.commit();
    }

    private void checkSimcardStatus(int slotId) {
        int[] subIds = SubscriptionManager.getSubId(slotId);
        String curImsi = null;
        if (subIds != null && subIds.length > 0)
            curImsi = getImsiBySubscriberId(subIds[0]);
        String lastImsi = getLastRequestImsi();
        if (curImsi != null && curImsi.length() != 0) {
            setLastRequestImsi(curImsi);
            if (lastImsi.length() != 0
                    && lastImsi.compareTo(curImsi) != 0
                    && !mMSimEnabled) {
                setSelfRegChoice(SELFREG_CHOICE_INIT,slotId);
                setDataConnectChoice(DATACON_CHOICE_INIT,slotId);
            }
        }
    }

    // judge if simcard change since last successful self registe
    private boolean isSimcardChange(int slotId)
    {
        boolean result = false;
        int[] subIds = SubscriptionManager.getSubId(slotId);
        String curImsi = null;
        if (subIds != null && subIds.length > 0)
            curImsi = getImsiBySubscriberId(subIds[0]);
        String lastImsi = getLastImsi();

        Log.d(TAG, "isSimcardChange: curImsi = " + curImsi);
        Log.d(TAG, "isSimcardChange: lastImsi = " + lastImsi);

        // NOTE: if string compare is ok , should use memcmp etc.
        if (curImsi == null)
        {
            Log.d(TAG, "isSimcardChange: Error !!! curImsi is null! ");
            result = false; // if can't get imsi, no need to send selfreg
                            // message
        }
        else
        {
            if (curImsi.equals(lastImsi))
            {
                result = false;
            }
            else
            {
                result = true;
            }
        }
        Log.d(TAG, "isSimcardChange: result = " + result);

        return result;
    }

    // judge if is need self registe
    protected boolean isNeedSelfReg(int slotId)
    {
        boolean result = false;
        if (isSimcardChange(slotId))
        {
            result = true;
        }
        Log.d(TAG, "isNeedSelfReg: " + result);
        return result;
    }

    // judge is have send self registe message
    protected boolean isHaveSendSelfRegMsg()
    {
        SharedPreferences sharedPreferences = getSharedPreferences(PREFERENCE_NAME, MODE);
        mIsHaveSendMsg = sharedPreferences.getBoolean("IsHaveSendSelfRegMsg", false);
        Log.d(TAG, "isHaveSendSelfRegMsg: return " + mIsHaveSendMsg);
        return mIsHaveSendMsg;
    }

    private void setIsHaveSendSelfRegMsg(Context context, boolean isHaveSend, int slotId)
    {
        Log.d(TAG, "setIsHaveSendSelfRegMsg: to " + isHaveSend);
        mIsHaveSendMsg = isHaveSend;
        mPhoneId = slotId;
        SharedPreferences sharedPreferences = context.getSharedPreferences(PREFERENCE_NAME, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putBoolean("IsHaveSendSelfRegMsg", isHaveSend);
        editor.commit();
    }

    // get self registe switch
    protected boolean getSelfRegSwitch()
    {
        Log.d(TAG, "getSelfRegSwitch: " + mSelfRegSwitch);
        return mSelfRegSwitch;
    }

    // set self registe switch
    protected void setSelfRegSwitch(Context context, boolean isOpen)
    {
        Log.d(TAG, "setSelfRegSwitch: " + isOpen);
        mSelfRegSwitch = isOpen;

        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putBoolean(ITEM_SELFREG_SWITCH, isOpen);
        editor.commit();
    }

    // ===========================================//
    protected boolean getSmsDeliverReportSwitch()
    {
        Log.d(TAG, "getSelfRegSwitch: " + mSmsDeliverReportSwitch);
        return mSmsDeliverReportSwitch;
    }

    protected void setSmsDeliverReportSwitch(Context context, boolean isOpen)
    {
        Log.d(TAG, "setSmsDeliverReportSwitch: " + isOpen);
        mSmsDeliverReportSwitch = isOpen;

        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putBoolean(ITEM_SMSDELIVERREPORT_SWITCH, isOpen);
        editor.commit();
    }

    // ===========================================//
    public boolean isDebugMode()
    {
        Log.d(TAG, "isDebugMode: " + mIsDebugMode);
        return mIsDebugMode;
    }

    protected void setDebugMode(Context context, boolean isDebugMode)
    {
        Log.d(TAG, "setDebugMode: " + isDebugMode);
        mIsDebugMode = isDebugMode;

        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putBoolean(ITEM_DEBUG_MODE, isDebugMode);
        editor.commit();
    }

    protected boolean isRealNetParam()
    {
        Log.d(TAG, "isRealParam: " + mIsRealNetParam);
        return mIsRealNetParam;
    }

    protected void setRealNetParam(Context context, boolean isRealParam, boolean isSetApn)
    {
        Log.d(TAG, "setRealNetParam: " + isRealParam);
        mIsRealNetParam = isRealParam;

        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putBoolean(ITEM_REAL_PARAM, isRealParam);
        editor.commit();

        if (mIsRealNetParam)
        {
            setServerAddr(context, REAL_SERVER_ADDR);
            setSmsAddr(context, REAL_SMS_ADDR);
            setSmsPort(context, REAL_SMS_PORT);
            if (isSetApn)
            {
                setAPN(context, REAL_APN);
                DmService.getInstance().setProxy(mContext, null);
                DmService.getInstance().setProxyPort(mContext, null);
            }
        }
        else
        {
            setServerAddr(context, LAB_SERVER_ADDR);
            setSmsAddr(context, LAB_SMS_ADDR);
            setSmsPort(context, LAB_SMS_PORT);
            if (isSetApn)
            {
                setAPN(context, LAB_APN);
                DmService.getInstance().setProxy(mContext, "10.0.0.172");
                DmService.getInstance().setProxyPort(mContext, "80");
            }
        }

    }

    public String getServerAddr()
    {
        Log.d(TAG, "getServerAddr: " + mServerAddr);
        return mServerAddr;
    }

    protected void setServerAddr(Context context, String serverAddr)
    {
        Log.d(TAG, "setServerAddr: " + serverAddr);
        mServerAddr = serverAddr;

        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(ITEM_SERVER_ADDR, serverAddr);
        editor.commit();
    }

    protected String getSmsAddr()
    {
        Log.d(TAG, "getSmsAddr: " + mSmsAddr);
        return mSmsAddr;
    }

    protected void setSmsAddr(Context context, String smsAddr)
    {
        Log.d(TAG, "setSmsAddr: " + smsAddr);
        mSmsAddr = smsAddr;

        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(ITEM_SMS_ADDR, smsAddr);
        editor.commit();
    }

    protected String getSmsPort()
    {
        Log.d(TAG, "getSmsPort: " + mSmsPort);
        return mSmsPort;
    }

    protected void setSmsPort(Context context, String smsPort)
    {
        Log.d(TAG, "setSmsPort: " + smsPort);
        mSmsPort = smsPort;

        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(ITEM_SMS_PORT, smsPort);
        editor.commit();
    }

    private String getInitApn(Context context)
    {
        String str = getInfoByIccOperator(context, Telephony.Carriers.APN);
        Log.d(TAG, "getInitApn: " + str);
        return str;
    }

    public String getSavedAPN()
    {
        Log.d(TAG, "getSavedAPN: " + mApnTemp);
        return mApnTemp;
    }

    public String getAPN()
    {
        Log.d(TAG, "getAPN: " + mApn);
        return mApn;
    }

    public void setAPN(Context context, String apn)
    {
        Log.d(TAG, "setAPN: " + apn);

        // SharedPreferences sharedPreferences =
        // context.getSharedPreferences(DM_CONFIG, MODE);
        // SharedPreferences.Editor editor = sharedPreferences.edit();
        // editor.putString(ITEM_APN, apn);
        // editor.commit();
        if (!Vdmc.getInstance().isVDMRunning())
        {
            mApn = apn;
            setInfoByIccOperator(context, Telephony.Carriers.APN, apn);
            mApnTemp = null;
        }
        else
        {
            mApnTemp = apn;
            Log.d(TAG, "setAPN: dm session is running, save value temporarily!");
        }
    }

    public String getSavedProxy()
    {
        Log.d(TAG, "getSavedProxy: " + mProxyTemp);
        return mProxyTemp;
    }

    public String getProxy(Context context)
    {
        // SharedPreferences sharedPreferences =
        // context.getSharedPreferences(DM_CONFIG, MODE);
        // SharedPreferences.Editor editor = sharedPreferences.edit();
        // String str = sharedPreferences.getString(ITEM_PROXY, "10.0.0.172");
        String str = getInfoByIccOperator(context, Telephony.Carriers.PROXY);
        Log.d(TAG, "getProxy: " + str);
        return str;
    }

    public void setProxy(Context context, String proxy)
    {
        Log.d(TAG, "setProxy: " + proxy);

        // SharedPreferences sharedPreferences =
        // context.getSharedPreferences(DM_CONFIG, MODE);
        // SharedPreferences.Editor editor = sharedPreferences.edit();
        // editor.putString(ITEM_PROXY, proxy);
        // editor.commit();

        if (!Vdmc.getInstance().isVDMRunning())
        {
            if (proxy != null) {
                setInfoByIccOperator(context, Telephony.Carriers.PROXY, proxy);
            }
            mProxyTemp = null;
        }
        else
        {
            mProxyTemp = proxy;
            Log.d(TAG, "setProxy: dm session is running, save value temporarily!");
        }
    }

    public String getSavedProxyPort()
    {
        Log.d(TAG, "getSavedProxyPort: " + mProxyPortTemp);
        return mProxyPortTemp;
    }

    public String getProxyPort(Context context)
    {
        // SharedPreferences sharedPreferences =
        // context.getSharedPreferences(DM_CONFIG, MODE);
        // SharedPreferences.Editor editor = sharedPreferences.edit();
        // String str = sharedPreferences.getString(ITEM_PROXY_PORT, "80");
        String str = getInfoByIccOperator(context, Telephony.Carriers.PORT);
        Log.d(TAG, "getProxyPort: " + str);
        return str;
    }

    public void setProxyPort(Context context, String port)
    {
        Log.d(TAG, "setProxyPort: " + port);

        // SharedPreferences sharedPreferences =
        // context.getSharedPreferences(DM_CONFIG, MODE);
        // SharedPreferences.Editor editor = sharedPreferences.edit();
        // editor.putString(ITEM_PROXY_PORT, port);
        // editor.commit();

        if (!Vdmc.getInstance().isVDMRunning())
        {
            setInfoByIccOperator(context, Telephony.Carriers.PORT, port);
            mProxyPortTemp = null;
        }
        else
        {
            mProxyPortTemp = port;
            Log.d(TAG, "setProxyPort: dm session is running, save value temporarily!");
        }
    }

    public void getServerNonce(Context context, byte[] data)
    {
        ByteBuffer buf = null;
        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        String str = sharedPreferences.getString(ITEM_SERVER_NONCE, "ffff");
        Log.d(TAG, "getServerNonce:= " + str);
        if (data == null) {
            Log.d(TAG, "read: data is null!");
            return;
        }
        buf = ByteBuffer.wrap(data);
        Log.d(TAG, "read: buf = " + buf);
        buf.put(str.getBytes());

    }

    public void setServerNonce(Context context, byte[] data)
    {
        String ServerNonce = new String(data);
        Log.d(TAG, "setServerNonce:=" + ServerNonce);

        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(ITEM_SERVER_NONCE, ServerNonce);
        editor.commit();
    }

    public void getClientNonce(Context context, byte[] data)
    {
        ByteBuffer buf = null;
        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        String str = sharedPreferences.getString(ITEM_CLIENT_NONCE, "ffff");
        Log.d(TAG, "getClientNonce= " + str);

        if (data == null) {
            Log.d(TAG, "read: data is null!");
            return;
        }
        buf = ByteBuffer.wrap(data);
        Log.d(TAG, "read: buf = " + buf);
        buf.put(str.getBytes());

    }

    public void setClientNonce(Context context, byte[] data)
    {
        String ClientNonce = new String(data);
        Log.d(TAG, "setClientNonce= " + ClientNonce);

        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(ITEM_CLIENT_NONCE, ClientNonce);
        editor.commit();
    }

    public String getManufactory()
    {
        if ( mInited ) {
            if (mManufactory.equals("HISENSE"))
                mManufactory = "Hisense";

            Log.d(TAG, "getManufactory: " + mManufactory);
            return mManufactory;
        } else {
            String manufactory = SystemProperties.get(
                "ro.product.manufacturer", Build.UNKNOWN);
            if (manufactory.equals("HISENSE"))
                manufactory = "Hisense";
            Log.d(TAG, "getCurrentManufactory: " + manufactory);
            return manufactory;
        }
    }

    protected void setManufactory(Context context, String manufactory)
    {
        Log.d(TAG, "setManufactory: " + manufactory);
        mManufactory = manufactory;

        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(ITEM_MANUFACTORY, manufactory);
        editor.commit();
    }

    public String getModel()
    {
        if ( mInited ) {
            Log.d(TAG, "getModel: " + mModel);
            return mModel;
        } else {
            String model = SystemProperties.get(
                "ro.product.model", Build.UNKNOWN);
            Log.d(TAG, "getCurrentModel: " + model);
            return model;
        }
    }

    protected void setModel(Context context, String model)
    {
        Log.d(TAG, "setModel: " + model);
        mModel = model;

        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(ITEM_MODEL, model);
        editor.commit();
    }

    public String getImei()
    {
        if ( mInited ) {
            Log.d(TAG, "getImei: " + mImeiStr);
            return mImeiStr;
        } else {
            String imei = mMSimEnabled ? ((TelephonyManager) getSystemService(
                    Context.TELEPHONY_SERVICE)).getDeviceId(mPhoneId)
                    : ((TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE))
                            .getDeviceId();
            Log.d(TAG, "getCurrentImei: " + imei);
            return imei;
        }
    }

    protected void setImei(Context context, String imei)
    {
        Log.d(TAG, "setImei: " + imei);
        mImeiStr = imei;

        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(ITEM_IMEI, imei);
        editor.commit();
    }

    public String getSoftwareVersion()
    {
        if ( mInited ) {
            Log.d(TAG, "getSoftwareVersion: " + mSoftVer);
            return mSoftVer;
        } else {
            String softVer = SystemProperties.get("ro.software.version",
                Build.UNKNOWN);
            Log.d(TAG, "getCurrentSoftwareVersion: " + softVer);
            return softVer;
        }
    }

    protected void setSoftwareVersion(Context context, String softVer)
    {
        Log.d(TAG, "setSoftwareVersion: " + softVer);
        mSoftVer = softVer;

        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(ITEM_SOFTVER, softVer);
        editor.commit();
    }

    // send message body
    private void sendMsgBody(int slotId)
    {
        Log.d(TAG, "++++sendMsgBody++++");
        int[] subIds = SubscriptionManager.getSubId(slotId);
        int subscription = ((subIds != null) && (subIds.length > 0)) ? subIds[0] : 0;
        String destAddr = getSmsAddr();
        short destPort = 0;
        try {
            destPort = (short) Integer.parseInt(getSmsPort());
        } catch (Exception e) {
            Log.e(TAG, "sendMsgBody exception:" + e);
        }
        short srcPort = destPort;
        byte[] data; // sms body byte stream
        String smsBody; // sms body string format
        String imei = getImei();
        String softVer = getSoftwareVersion();
        String manStr = getManufactory();
        String modStr = getModel();

        Log.d(TAG, " Enter!");

        // smsbody: IMEI:860206000003972/Hisense/TS7032/TI7.2.01.22.00
        smsBody = "IMEI:" + imei + "/" + manStr + "/" + modStr + "/" + softVer;
        Log.d(TAG, "sendMsgBody: " + smsBody);
        data = smsBody.getBytes();
        for (int i = 0; i < smsBody.length(); i++)
        {
            Log.d(TAG, "sendMsgBody: ============= data[" + i + "] = " + data[i] + "\n");
        }

        Log.d(TAG, "sendMsgBody: dest addr = " + destAddr);
        Log.d(TAG, "sendMsgBody: dest port = " + destPort);

        PendingIntent sentIntent;
        Intent intent = new Intent(SMS_ACTION_SENT);
        sentIntent = PendingIntent.getBroadcast(mContext, 0, intent, 0);

        PendingIntent deliverIntent;
        intent = new Intent(SMS_ACTION_DELIVER);
        deliverIntent = PendingIntent.getBroadcast(mContext, 0, intent, 0);
        String scAddr = "";
        SmsManager smsManager = SmsManager.getSmsManagerForSubscriptionId(subscription);
        smsManager.sendDataMessage(destAddr, scAddr, destPort, srcPort, data, sentIntent,
                deliverIntent);

        int count = getMessageSendCount(mContext);
        count++;
        setMessageSendCount(mContext, count); // record self registe message
                                              // send count

    }

    // Send self registe message
    private void sendSelfRegMsg(int slotId)
    {
        if (!getSelfRegSwitch())
        {
            Log.d(TAG,
                    "sendSelfRegMsg: self registe switch is closed, no need send self registe message!");
            stopListeningServiceState();
            return;
        }

        Log.d(TAG, "++++sendSelfRegMsg++mSmsInitComplete+++++" + mSmsInitComplete);
        Log.d(TAG, "+++sendSelfRegMsg+++mSimInitComplete+++++" + mSimInitComplete);
        // if (!mSmsInitComplete || !mSimInitComplete)
        if (!mSimInitComplete)
        {
            Log.d(TAG, "sendSelfRegMsg: not init complete!");
            return;
        }
        Log.d(TAG, "++++++mImeiStr+++++" + mImeiStr);
        if (mMSimEnabled) {
            if (mIsDebugMode) {
                mImeiStr = getSharedPreferences(DM_CONFIG, MODE).getString(ITEM_IMEI,
                        mTelephonyManager.getDeviceId(slotId));
            } else {
                mImeiStr = mTelephonyManager.getDeviceId(slotId);
            }
        }
        if (isHaveSendSelfRegMsg())
        {
            Log.d(TAG, "sendSelfRegMsg: have send self registe message!");
            stopListeningServiceState();
            return;
        }

        Log.d(TAG, "sendSelfRegMsg: Enter!");
        if (isNeedSelfReg(slotId))
        {
            showSelfRegistDialog(slotId);
            // Debug.stopMethodTracing();

        }
        else
        {
            setSelfRegState(mContext, true);
            stopListeningServiceState();
        }
    }

    // Send self registe message for debug mode
    protected void sendSelfRegMsgForDebug()
    {
        // send message directly under debug mode
        Log.d(TAG, "sendSelfRegMsgForDebug: Enter!");
        showSelfRegistDialog(mPhoneId);
    }

    void showSelfRegistDialog(int slotId) {
        if (mContext != null) {
            checkSimcardStatus(slotId);
            Intent intent = new Intent(mContext, DmAlertDialog.class);
            int intentFlags = Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP;
            intent.setFlags(intentFlags);
            intent.putExtra("dialogId", Vdmc.DM_SELFREGIST_DIALOG);
            intent.putExtra("message", getString(R.string.selfregist_tip) + "|"
                    + getString(R.string.selfregist_second_tip));
            intent.putExtra("timeout", Integer.MAX_VALUE);
            intent.putExtra("slotid", slotId);
            int selfRegChoice = getSelfRegChoice(slotId);
            if (SELFREG_CHOICE_INIT == selfRegChoice
                    || SELFREG_CHOICE_AGREE == (SELFREG_CHOICE_AGREE & selfRegChoice)) {
                showNotification(intent, getString(R.string.selfregist_notify_title),
                    getString(R.string.selfregist_notify_message), notifyid_forselfregist);
                mContext.startActivity(intent);
            } else {
                if (SELFREG_CHOICE_DISAGREE == (SELFREG_CHOICE_DISAGREE & selfRegChoice)) {
                    postAgreedSelfRegist(false,slotId);
                } else {
                    showNotification(intent, getString(R.string.selfregist_notify_title),
                    getString(R.string.selfregist_notify_message), notifyid_forselfregist);
                    setSelfRegChoice(SELFREG_CHOICE_INIT,slotId);
                    mContext.startActivity(intent);
                }
            }
        } else {
            sendMsgBody(slotId);
            setIsHaveSendSelfRegMsg(mContext, true, slotId);
        }

    }

    void postAgreedSelfRegist(boolean agreed, int slotId) {
        if (agreed) {
            mPhoneId = slotId;
            sendMsgBody(slotId);
            setIsHaveSendSelfRegMsg(mContext, true, slotId);
        }
        int selfRegChoice = getSelfRegChoice(slotId);
        int currentChoice = agreed ? SELFREG_CHOICE_AGREE : SELFREG_CHOICE_DISAGREE;
        if (selfRegChoice != currentChoice) {
            setSelfRegChoice(currentChoice,slotId);
        }
        hideNotification(notifyid_forselfregist);
    }

    boolean showDataConnectDialog() {
        int dataConnectChoice = (mContext != null) ? getDataConnectChoice(mPhoneId)
                : DATACON_CHOICE_INIT;
        if (DATACON_CHOICE_ALWAYS == (DATACON_CHOICE_ALWAYS & dataConnectChoice)) {
            if (DATACON_CHOICE_AGREE == (DATACON_CHOICE_AGREE & dataConnectChoice))
                return true;
            else if (DATACON_CHOICE_DISAGREE == (DATACON_CHOICE_DISAGREE & dataConnectChoice))
                return false;
            else
                setDataConnectChoice(DATACON_CHOICE_INIT,mPhoneId);
        } else {
            setDataConnectChoice(DATACON_CHOICE_INIT,mPhoneId);
        }
        synchronized (dataConSync) {
            try {
                if (mContext != null) {
                    Intent intent = new Intent(mContext, DmAlertDialog.class);
                    int intentFlags = Intent.FLAG_ACTIVITY_NEW_TASK
                            | Intent.FLAG_ACTIVITY_CLEAR_TOP;
                    intent.setFlags(intentFlags);
                    intent.putExtra("dialogId", Vdmc.DM_DATACONNECT_DIALOG);
                    intent.putExtra("message", getString(R.string.dataconnect_tip)
                            + "|" + getString(R.string.dataconnect_second_tip)
                            + "|" + getString(R.string.dataconnect_always_tip));
                    intent.putExtra("timeout", queryDialogTimeout);
                    mContext.startActivity(intent);
                    showNotification(intent, getString(R.string.dataconnect_notify_title),
                            getString(R.string.dataconnect_notify_message),
                            notifyid_fordataconnect);
                } else {
                    return false;
                }
                dataConSync.wait(queryDialogTimeoutMs);
                dataConnectChoice = getDataConnectChoice(mPhoneId);
                if (DATACON_CHOICE_AGREE == (DATACON_CHOICE_AGREE & dataConnectChoice))
                    return true;
                else
                    return false;
            } catch (Exception ex) {
                return false;
            }
        }
    }

    void postAgreedDataConnect(boolean agreed, boolean always) {
        synchronized (dataConSync) {
            try {
                int dataConnectChoice = 0;
                if (agreed)
                    dataConnectChoice |= DATACON_CHOICE_AGREE;
                else
                    dataConnectChoice |= DATACON_CHOICE_DISAGREE;
                if(always)
                    dataConnectChoice |= DATACON_CHOICE_ALWAYS;
                setDataConnectChoice(dataConnectChoice,mPhoneId);
                dataConSync.notifyAll();
            } catch (Exception ex) {

            }
        }
        hideNotification(notifyid_fordataconnect);
    }

    private void showNotification(Intent intent, String title, String message, int id) {
        PendingIntent contentIntent = PendingIntent.getActivity(this, id, intent, PendingIntent.FLAG_CANCEL_CURRENT);
        Notification notif = new Notification(R.drawable.icon, "China Mobile",
                System.currentTimeMillis());
        notif.flags = Notification.FLAG_ONGOING_EVENT;
        notif.setLatestEventInfo(this, title, message, contentIntent);
        NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        nm.notify(id, notif);
    }

    private void hideNotification(int id) {
        NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        nm.cancel(id);
    }

    // add for get self registe message send count
    public int getMessageSendCount(Context context)
    {
        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        int count = sharedPreferences.getInt(ITEM_MESSAGE_SEND_COUNT, 0);
        Log.d(TAG, "getMessageSendCount: count = " + count);
        return count;
    }

    // add for set self registe message send count
    protected void setMessageSendCount(Context context, int count)
    {
        Log.d(TAG, "setMessageSendCount: count = " + count);

        SharedPreferences sharedPreferences = context.getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putInt(ITEM_MESSAGE_SEND_COUNT, count);
        editor.commit();
    }

    public String getCardChoice(int slotId) {
        SharedPreferences sharedPreferences = getSharedPreferences(DM_CONFIG, MODE);
        return sharedPreferences.getString(ITEM_CARD_CHOICE + slotId, "");
    }

    public void setCardChoice(int slotId, String imsi) {
        SharedPreferences sharedPreferences = getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(ITEM_CARD_CHOICE + slotId, imsi);
        editor.commit();
    }

    public int getSelfRegChoice(int slotId) {
        SharedPreferences sharedPreferences = getSharedPreferences(DM_CONFIG, MODE);
        if(!mMSimEnabled) {
            return sharedPreferences.getInt(ITEM_SELFREG_CHOICE, SELFREG_CHOICE_INIT);
        } else {
            return sharedPreferences.getInt(ITEM_SELFREG_CHOICE + slotId, SELFREG_CHOICE_INIT);
        }
    }

    private void setSelfRegChoice(int choice,int slotId) {
        SharedPreferences sharedPreferences = getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        if(!mMSimEnabled) {
            editor.putInt(ITEM_SELFREG_CHOICE, choice);
        } else {
            editor.putInt(ITEM_SELFREG_CHOICE + slotId, choice);
            int[] subIds = SubscriptionManager.getSubId(slotId);
            String imsi = null;
            if (subIds != null && subIds.length > 0)
                imsi = mTelephonyManager.getSubscriberId(subIds[0]);
            if (imsi != null)
                editor.putString(ITEM_CARD_CHOICE + slotId, imsi);
        }
        editor.commit();
        Log.d(TAG, "save choice("+choice+") slot:"+slotId);
    }

    public int getDataConnectChoice(int slotId) {
        SharedPreferences sharedPreferences = getSharedPreferences(DM_CONFIG, MODE);
        if (!mMSimEnabled) {
            return sharedPreferences.getInt(ITEM_DATACON_CHOICE, DATACON_CHOICE_INIT);
        } else {
            return sharedPreferences.getInt(ITEM_DATACON_CHOICE + slotId, DATACON_CHOICE_INIT);
        }
    }

    private void setDataConnectChoice(int choice,int slotId) {
        SharedPreferences sharedPreferences = getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        if (!mMSimEnabled) {
            editor.putInt(ITEM_DATACON_CHOICE, choice);
        } else {
            editor.putInt(ITEM_DATACON_CHOICE + slotId, choice);
        }
        editor.commit();
    }

    private String getLastRequestImsi() {
        SharedPreferences sharedPreferences = getSharedPreferences(PREFERENCE_NAME, MODE);
        String imsi = sharedPreferences.getString("IMSI_Request", "");
        return imsi;
    }

    private void setLastRequestImsi(String imsi) {
        SharedPreferences sharedPreferences = getSharedPreferences(PREFERENCE_NAME, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString("IMSI_Request", imsi);
        editor.commit();
    }

    public int getSlotId() {
        if (mMSimEnabled) {
            return mPhoneId;
        } else {
            return 0;
        }
    }

    String getSubscriberId() {
        if (mMSimEnabled) {
            int[] subIds = SubscriptionManager.getSubId(mPhoneId);
            if (subIds != null && subIds.length > 0)
                return mTelephonyManager.getSubscriberId(subIds[0]);
            else
                return "";
        } else {
            return mTelephonyManager.getSubscriberId();
        }
    }

    Vector<Integer> getVacantMSimCfg() {
        Vector<Integer> vacant = new Vector<Integer>();
        int phoneCnt = mTelephonyManager.getPhoneCount();
        for (int i = 0; i < phoneCnt; i++) {
            if (!mSimArray.contains(Integer.valueOf(i))) {
                vacant.add(Integer.valueOf(i));
            }
        }
        return vacant;
    }

    HashMap<String, int[]> getCurrentMSimConfig() {
        int phoneCnt = mTelephonyManager.getPhoneCount();
        HashMap<String, int[]> map = new HashMap<String, int[]>();
        for (int i = 0; i < phoneCnt; i++) {
            String imsi = getCardChoice(i);
            if (imsi.length() == 0)
                continue;
            int sr = getSelfRegChoice(i);
            int dc = getDataConnectChoice(i);
            int[] cfg = new int[]{i,sr,dc};
            map.put(imsi, cfg);
            Log.d(TAG,"cfg=> slot:"+i+"imsi:"+imsi+" sr:"+sr+" dc:"+dc);
        }
        return map;
    }

    HashMap<String, int[]> getAvailableSim(HashMap<String, int[]> curCfg,
            SharedPreferences.Editor modify) {
        HashMap<String, int[]> availSim = new HashMap<String, int[]>();
        for (Integer id : mSimArray) {
            int slotId = id.intValue();
            int[] subIds = SubscriptionManager.getSubId(slotId);
            if (subIds != null && subIds.length > 0) {
                String imsi = mTelephonyManager.getSubscriberId(subIds[0]);
                int[] cfg;
                if (curCfg.containsKey(imsi)) {
                    cfg = curCfg.remove(imsi);
                    if(cfg[1]!=SELFREG_CHOICE_DISAGREE) {
                        cfg[0] = slotId;
                        availSim.put(imsi,cfg);
                    }
                } else {
                    cfg = new int[]{slotId,SELFREG_CHOICE_INIT,DATACON_CHOICE_INIT};
                    availSim.put(imsi, cfg);
                }
                modify.putString(ITEM_CARD_CHOICE + slotId, imsi);
                modify.putInt(ITEM_SELFREG_CHOICE + slotId, cfg[1]);
                modify.putInt(ITEM_DATACON_CHOICE + slotId, cfg[2]);
                Log.d(TAG,"now=> slot:"+cfg[0]+" imsi:"+imsi+" sr:"+cfg[1]+" dc:"+cfg[2]);
            }
        }
        return availSim;
    }

    void storeLeftMSimCfg(HashMap<String, int[]> leftCfg, Vector<Integer> vacant,
            SharedPreferences.Editor modify) {
        for (String imsi : leftCfg.keySet()) {
            try {
                Integer id = vacant.remove(0);
                int[] cfg = leftCfg.get(imsi);
                int slotId = id.intValue();
                modify.putString(ITEM_CARD_CHOICE + slotId, imsi);
                modify.putInt(ITEM_SELFREG_CHOICE + slotId, cfg[1]);
                modify.putInt(ITEM_DATACON_CHOICE + slotId, cfg[2]);
            } catch (Exception e) {
                Log.d(TAG, "Vacancy is used up");
                break;
            }
        }
    }

    int selectPreferedSim() {
        int phoneCnt = mTelephonyManager.getPhoneCount();
        String last_request_imsi = getLastRequestImsi();
        String last_imsi = getLastImsi();
        Vector<Integer> vacant = getVacantMSimCfg();
        HashMap<String, int[]> map = getCurrentMSimConfig();
        SharedPreferences sharedPreferences = getSharedPreferences(DM_CONFIG, MODE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        HashMap<String, int[]> availSim = getAvailableSim(map, editor);
        storeLeftMSimCfg(map, vacant, editor);
        editor.commit();
        // if availsim is equal to last imsi, then choose the last sim.
        if (availSim.containsKey(last_imsi)) {
            Log.d(TAG,"last imsi selected:"+last_imsi+" slot:"+availSim.get(last_imsi)[0]);
            return availSim.get(last_imsi)[0];
        }
        // if availsim contains sim agreed by user, then choose it.
        for (String imsi : availSim.keySet()) {
            int[] cfg = availSim.get(imsi);
            if (cfg != null && cfg[0] < phoneCnt && cfg[0] >= 0
                    && cfg[1] == SELFREG_CHOICE_AGREE)
                return cfg[0];
        }
        // if availsim is equal to last request, then choose the last request.
        if (availSim.containsKey(last_request_imsi)) {
            int slotId = availSim.get(last_request_imsi)[0];
            Log.d(TAG,"last request imsi selected:"+last_request_imsi+" slot:"+slotId);
            return slotId;
        }
        if (availSim.keySet().size()==0) {
            return -1;
        } else {
            // choose the suitable sim
            int availSlot = -1;
            for (String imsi : availSim.keySet()) {
                int[] cfg = availSim.get(imsi);
                if (cfg != null && cfg[0] < phoneCnt && cfg[0] >= 0) {
                    if (availSlot == -1 || availSlot > cfg[0])
                        availSlot = cfg[0];
                }
            }
            return availSlot;
        }
    }

    int getSimState(int slotid) {
        return mMSimEnabled ? mTelephonyManager.getSimState(slotid)
                : mTelephonyManager.getSimState();
    }

    String getNetworkOperator(int subscription) {
        return mMSimEnabled ? mTelephonyManager
                .getNetworkOperatorForSubscription(subscription) : mTelephonyManager
                .getNetworkOperator();
    }

    String getImsiBySubscriberId(int subscription) {
        return mMSimEnabled ? mTelephonyManager.getSubscriberId(subscription)
                : mTelephonyManager.getSubscriberId();
    }

    private String getInfoByIccOperator(Context context, String columnName)
    {
        String str = null;
        String numeric = android.os.SystemProperties.get(
                TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "");
        String[] operArr = numeric != null ? numeric.split(",") : null;
        if (operArr != null && operArr.length > 0) {
            for (String oper : operArr) {
                final String selection = "name = 'CMCC DM' and numeric=\""
                        + oper + "\"";
                Cursor cursor = context.getContentResolver().query(
                        Telephony.Carriers.CONTENT_URI,
                        null, selection, null, null);
                if (cursor == null) continue;
                if (cursor.getCount() > 0 && cursor.moveToFirst()) {
                    str = cursor.getString(cursor.getColumnIndexOrThrow(
                            columnName));
                }
                cursor.close();
                if (str != null && str.length() > 0) break;
            }
        }
        return str;
    }

    private void setInfoByIccOperator(Context context, String columnName, String value)
    {
        String numeric = android.os.SystemProperties.get(
                TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "");
        String[] operArr = numeric != null ? numeric.split(",") : null;
        if (operArr != null && operArr.length > 0) {
            ContentValues values = new ContentValues();
            values.put(columnName, value);
            for (String oper : operArr) {
                final String selection = "name = 'CMCC DM' and numeric=\""
                        + oper + "\"";
                int c = values.size() > 0 ? context.getContentResolver().update(
                        Telephony.Carriers.CONTENT_URI,
                        values, selection, null) : 0;
                Log.d(TAG, "setInfoByIccOperator: column = " + columnName +
                        "values.size() = " + values.size());
                Log.d(TAG, "setInfoByIccOperator: update count = " + c);
            }
        }
    }
}
