/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.android.nfc;

import android.content.SharedPreferences;
import qcom.nfc.IQNfcSecureElementManager;
import qcom.nfc.IQNfcSecureElementManagerCallbacks;

import com.android.nfc.DeviceHost;
import com.android.nfc.NfcService.ApplyRoutingTask;
import com.android.nfc.QNfcService;
import com.android.nfc.NfcService;

import android.app.Application;
import android.util.Log;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.Message;
import android.os.Handler;
import android.os.IBinder;
import android.os.UserHandle;
import android.os.SystemProperties;

import com.android.nfc.NfceeAccessControl;
import com.android.nfc.dhimpl.NativeNfcManager;
import com.android.nfc.dhimpl.NativeNfcSecureElement;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.pm.ServiceInfo;
import android.content.BroadcastReceiver;
import android.net.Uri;
import android.nfc.NfcAdapter;
import android.nfc.cardemulation.AidGroup;
import android.app.IActivityManager;
import android.app.ActivityManagerNative;
import android.content.ComponentName;
import com.android.nfc.cardemulation.MultiSeManager;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;
import android.nfc.cardemulation.ApduServiceInfo;
import android.nfc.cardemulation.AidGroup;

public class QSecureElementManager extends IQNfcSecureElementManager.Stub {
    private static final String TAG = "nfc:QSecureElementManager";
    private static final boolean DBG = false;

    private DeviceHost mDeviceHost;
    private MyCallback mHandler;
    private NfceeAccessControl mNfceeAccessControl;
    final IActivityManager mIActivityManager;
    private Context mContext;
    private static SharedPreferences mPrefs;
    private static SharedPreferences.Editor mPrefsEditor;
    private NfcService mNfcService;
    boolean mIsSentUnicastReception = false;
    public int mEeRoutingState;
    public boolean mNfceeRouteEnabled;
    final HashMap<String, Boolean> mMultiReceptionMap = new HashMap<String, Boolean>();
    private final static Boolean multiReceptionMode = Boolean.TRUE;
    private final static Boolean unicastReceptionMode = Boolean.FALSE;
    private final Boolean defaultTransactionEventReceptionMode;
    private final boolean mIsHceCapable;

    private Object mWaitOMACheckCert = null;
    private boolean mHasOMACert = false;
    private static QSecureElementManager sService = null;
    private static MultiSeManager mMultiSeManager = null;

    //support open/close/trancieve
    private NativeNfcSecureElement mSecureElement;
    private OpenSecureElement mOpenEe;  // null when EE closed
    private PowerManager.WakeLock mEeWakeLock;

    private static final class eeErrorCodes {
        // Return values from NfcEe.open() - these are 1:1 mapped
        // to the thrown EE_EXCEPTION_ exceptions in nfc-extras.
        static final int EE_ERROR_IO = -1;
        static final int EE_ERROR_ALREADY_OPEN = -2;
        static final int EE_ERROR_INIT = -3;
        static final int EE_ERROR_LISTEN_MODE = -4;
        static final int EE_ERROR_EXT_FIELD = -5;
        static final int EE_ERROR_NFC_DISABLED = -6;
    }

    // Copied from com.android.nfc_extras to avoid library dependency
    // nfc_extras is deprecated we can controll these.
    static final int ROUTE_OFF = 1;
    static final int ROUTE_ON_WHEN_SCREEN_ON_UNLOCKED = 2;
    static final int ROUTE_ON_WHEN_SCREEN_ON =3;
    static final int ROUTE_ON_ALLWAYS= 4;

    static final String PREF_NFC_ON = "nfc_on";
    static final boolean NFC_ON_DEFAULT = true;
    static final String PREF_NDEF_PUSH_ON = "ndef_push_on";
    static final boolean NDEF_PUSH_ON_DEFAULT = true;
    static final String PREF_FIRST_BEAM = "first_beam";
    static final String PREF_FIRST_BOOT = "first_boot";
    static final String PREF_AIRPLANE_OVERRIDE = "airplane_override";
    static final boolean SE_BROADCASTS_WITH_HCE = true;
    static final String PREF_EE_ROUTING_STATE= "ee_routing_state";
    static final String PREF_ACTIVE_SECURE_ELEMENT= "active_secure_element";


    public static final String ACTION_AID_SELECTED =
        "com.android.nfc_extras.action.AID_SELECTED";
    public static final String EXTRA_AID = "com.android.nfc_extras.extra.AID";
    public static final String EXTRA_DATA = "com.android.nfc_extras.extra.DATA";
    public static final String EXTRA_SE_NAME = "com.android.nfc_extras.extra.SE_NAME";
    public static final String EXTRA_SE_NAME_LIST = "com.android.nfc_extras.extra.SE_NAME_LIST";

    public static final String ACTION_LLCP_UP =
            "com.android.nfc.action.LLCP_UP";

    public static final String ACTION_LLCP_DOWN =
            "com.android.nfc.action.LLCP_DOWN";

    public static final String ACTION_APDU_RECEIVED =
        "com.android.nfc_extras.action.APDU_RECEIVED";
    public static final String EXTRA_APDU_BYTES =
        "com.android.nfc_extras.extra.APDU_BYTES";

    public static final String ACTION_EMV_CARD_REMOVAL =
        "com.android.nfc_extras.action.EMV_CARD_REMOVAL";

    public static final String ACTION_MIFARE_ACCESS_DETECTED =
        "com.android.nfc_extras.action.MIFARE_ACCESS_DETECTED";
    public static final String EXTRA_MIFARE_BLOCK =
        "com.android.nfc_extras.extra.MIFARE_BLOCK";

    public static final String ACTION_SE_LISTEN_ACTIVATED =
            "com.android.nfc_extras.action.SE_LISTEN_ACTIVATED";
    public static final String ACTION_SE_LISTEN_DEACTIVATED =
            "com.android.nfc_extras.action.SE_LISTEN_DEACTIVATED";


    private static final String prefsName = "QSecureElementManager_prefs";

    private static final String displayNameForDeviceHost = "Device Host";

    public QSecureElementManager(Application nfcApplication, DeviceHost host, NfcService nfcService) {
        sService = this;
        String mIsisConfig = SystemProperties.get("persist.nfc.smartcard.isis");
        Log.d(TAG,"persist.nfc.smartcard.isis = " + mIsisConfig);
        if(mIsisConfig==null || mIsisConfig.equals("none") || mIsisConfig.equals("")) {
            Log.d(TAG, "unicast mode set");
            defaultTransactionEventReceptionMode = unicastReceptionMode;
        } else {
            Log.d(TAG, "multicast mode set");
            defaultTransactionEventReceptionMode = multiReceptionMode;
        }

        mContext = nfcApplication;
        mSecureElement = new NativeNfcSecureElement(mContext);
        mDeviceHost = host;
        mPrefs = mContext.getSharedPreferences(prefsName, Context.MODE_PRIVATE);
        mPrefsEditor = mPrefs.edit();

        mNfceeAccessControl = new NfceeAccessControl(mContext);
        mNfcService = nfcService;
        mHandler = new MyCallback();
        mIActivityManager = ActivityManagerNative.getDefault();

        PowerManager mPowerManager = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
        mEeWakeLock = mPowerManager.newWakeLock(
                PowerManager.PARTIAL_WAKE_LOCK, "NfcService:mEeWakeLock");

        PackageManager pm = mContext.getPackageManager();
        mIsHceCapable = pm.hasSystemFeature(PackageManager.FEATURE_NFC_HOST_CARD_EMULATION);

        IntentFilter intentFilterSetDefaultIsoDepRoute = new IntentFilter();
        intentFilterSetDefaultIsoDepRoute.addAction("com.android.nfc.action.GET_DEFAULT_ISO_DEP_ROUTE");
        intentFilterSetDefaultIsoDepRoute.addAction("com.android.nfc.action.SET_DEFAULT_ISO_DEP_ROUTE");

        mContext.registerReceiver(setDefaultIsoDepRouteReceiver,
                                  intentFilterSetDefaultIsoDepRoute,
                                  "android.permission.NFC_ENABLE", null);

        mMultiSeManager = MultiSeManager.getInstance();
    }

    private final BroadcastReceiver setDefaultIsoDepRouteReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals("com.android.nfc.action.GET_DEFAULT_ISO_DEP_ROUTE")){
                sendMessage(mHandler.MSG_GET_DEFAULT_ISO_DEP_ROUTE, null);
            }
            else if (action.equals("com.android.nfc.action.SET_DEFAULT_ISO_DEP_ROUTE")){
                String seName = intent.getStringExtra(EXTRA_SE_NAME);
                sendMessage(mHandler.MSG_SET_DEFAULT_ISO_DEP_ROUTE, seName);
            }
        }
    };

    public QNfcService.HandlerCallback getCallback()
    {
        return mHandler;
    }

    // Destination (eSE1/eSE2) of APDU gate per application process
    final HashMap<String, String> mSEToOpenAPDUGateMap = new HashMap<String, String>();

    /** resources kept while secure element is open */
    private class OpenSecureElement implements IBinder.DeathRecipient {
        public int pid;  // pid that opened SE
        // binder handle used for DeathReceipient. Must keep
        // a reference to this, otherwise it can get GC'd and
        // the binder stub code might create a different BinderProxy
        // for the same remote IBinder, causing mismatched
        // link()/unlink()
        public IBinder binder;
        public int handle; // low-level handle
        public OpenSecureElement(int pid, int handle, IBinder binder) {
            this.pid = pid;
            this.handle = handle;
            this.binder = binder;
        }
        @Override
        public void binderDied() {
            synchronized (mNfcService) {
                Log.i(TAG, "Tracked app " + pid + " died");
                pid = -1;
                try {
                    _nfcEeClose(-1, binder);
                } catch (IOException e) { /* already closed */ }
            }
        }
        @Override
        public String toString() {
            return new StringBuilder('@').append(Integer.toHexString(hashCode())).append("[pid=")
                    .append(pid).append(" handle=").append(handle).append("]").toString();
        }
    }

    private void sendSeBroadcast(Intent intent) {
        mNfcService.sendNfcEeAccessProtectedBroadcast(intent);
    }

    final class MyCallback extends QNfcService.HandlerCallback {

        static final int MSG_NDEF_TAG = 0;
        static final int MSG_CARD_EMULATION = 1;
        static final int MSG_LLCP_LINK_ACTIVATION = 2;
        static final int MSG_LLCP_LINK_DEACTIVATED = 3;
        static final int MSG_TARGET_DESELECTED = 4;
        static final int MSG_MOCK_NDEF = 7;
        static final int MSG_SE_APDU_RECEIVED = 10;
        static final int MSG_SE_EMV_CARD_REMOVAL = 11;
        static final int MSG_SE_MIFARE_ACCESS = 12;
        static final int MSG_SE_LISTEN_ACTIVATED = 13;
        static final int MSG_SE_LISTEN_DEACTIVATED = 14;
        static final int MSG_LLCP_LINK_FIRST_PACKET = 15;
        static final int MSG_ROUTE_AID = 16;
        static final int MSG_UNROUTE_AID = 17;
        static final int MSG_COMMIT_ROUTING = 18;
        static final int MSG_SE_DELIVER_INTENT = 19;
        static final int MSG_ENABLE_NFC_ADAPTER = 20;
        static final int MSG_UPDATE_CARD_EMULATION_ROUTE = 21;
        static final int MSG_ENABLE_CARD_EMULATION_MODE = 22;
        static final int MSG_DISABLE_CARD_EMULATION_MODE = 23;
        static final int MSG_SET_ACTIVE_SECURE_ELEMENT = 24;
        static final int MSG_SET_CLF_AID_FILTER_LIST = 25;
        static final int MSG_ENABLE_CLF_AID_FILTER_COND = 26;
        static final int MSG_DISABLE_CLF_AID_FILTER_COND = 27;
        static final int MSG_NOTIFY_RF_INTF_DEACTIVATED = 28;
        static final int MSG_GET_DEFAULT_ISO_DEP_ROUTE = 29;
        static final int MSG_SET_DEFAULT_ISO_DEP_ROUTE = 30;

        private void applyRouting(boolean force) {
            mNfcService.applyRouting(force);
        }

        protected void handleMessage(Message msg){
            if(msg == null) {
                Log.e(TAG,"SecureElementManager callback handler discarding null message");
                return;
            }
            switch(msg.what) {

                case MSG_CARD_EMULATION:
                    if (DBG) Log.d(TAG, "Card Emulation message");
                    /* Tell the host-emu manager an AID has been selected on
                     * a secure element.
                     */

                    if (mNfcService.mCardEmulationManager != null) {
                        mNfcService.mCardEmulationManager.onOffHostAidSelected();
                    }
                    byte[] dataBuf = (byte[]) msg.obj;
                    byte[] aid = null;
                    byte[] param = null;
                    byte[] nfceeId = null;
                    // NFCEE ID(1 byte) + length of AID(1 byte) + AID + length of param(1 byte) + PARAM
                    nfceeId = new byte[1];
                    nfceeId[0] = dataBuf[0];

                    int lengthOfAid = dataBuf[1];
                    int lengthOfParam = 0;
                    if (dataBuf.length > 2 + lengthOfAid)
                        lengthOfParam = dataBuf[2 + lengthOfAid];

                    if (lengthOfAid > 0) {
                        aid = new byte[lengthOfAid];
                        for (int i = 0; i < lengthOfAid; i++)
                            aid[i] = dataBuf[2 + i];
                    }

                    if (lengthOfParam > 0) {
                        param = new byte[lengthOfParam];
                        for (int i = 0; i < lengthOfParam; i++)
                            param[i] = dataBuf[2 + lengthOfAid + i + 1];
                    }

                    /* Send broadcast */
                    Intent aidIntent = new Intent();
                    aidIntent.setAction(ACTION_AID_SELECTED);
                    if (lengthOfAid > 0)
                        aidIntent.putExtra(EXTRA_AID, aid);
                    if (lengthOfParam > 0)
                        aidIntent.putExtra(EXTRA_DATA, param);
                    if (nfceeId != null) {
                        String seName = mDeviceHost.getSecureElementName(nfceeId[0]);
                        if (seName != null)
                            aidIntent.putExtra(EXTRA_SE_NAME, seName);
                    }

                    if (DBG) Log.d(TAG, "Broadcasting " + ACTION_AID_SELECTED);

                    // unicast reception for GSMA
                    mIsSentUnicastReception = false;

                    sendSeBroadcast(aidIntent);
                    break;

                case MSG_SE_EMV_CARD_REMOVAL:
                    if (DBG) Log.d(TAG, "Card Removal message");
                    /* Send broadcast */
                    Intent cardRemovalIntent = new Intent();
                    cardRemovalIntent.setAction(ACTION_EMV_CARD_REMOVAL);
                    if (DBG) Log.d(TAG, "Broadcasting " + ACTION_EMV_CARD_REMOVAL);
                    sendSeBroadcast(cardRemovalIntent);
                    break;

                case MSG_SE_APDU_RECEIVED:
                    if (DBG) Log.d(TAG, "APDU Received message");
                    byte[] apduBytes = (byte[]) msg.obj;
                    /* Send broadcast */
                    Intent apduReceivedIntent = new Intent();
                    apduReceivedIntent.setAction(ACTION_APDU_RECEIVED);
                    if (apduBytes != null && apduBytes.length > 0) {
                        apduReceivedIntent.putExtra(EXTRA_APDU_BYTES, apduBytes);
                    }
                    if (DBG) Log.d(TAG, "Broadcasting " + ACTION_APDU_RECEIVED);
                    sendSeBroadcast(apduReceivedIntent);
                    break;

                case MSG_SE_MIFARE_ACCESS:
                    if (DBG) Log.d(TAG, "MIFARE access message");
                    /* Send broadcast */
                    byte[] mifareCmd = (byte[]) msg.obj;
                    Intent mifareAccessIntent = new Intent();
                    mifareAccessIntent.setAction(ACTION_MIFARE_ACCESS_DETECTED);
                    if (mifareCmd != null && mifareCmd.length > 1) {
                        int mifareBlock = mifareCmd[1] & 0xff;
                        if (DBG) Log.d(TAG, "Mifare Block=" + mifareBlock);
                        mifareAccessIntent.putExtra(EXTRA_MIFARE_BLOCK, mifareBlock);
                    }
                    if (DBG) Log.d(TAG, "Broadcasting " + ACTION_MIFARE_ACCESS_DETECTED);
                    sendSeBroadcast(mifareAccessIntent);
                    break;

                case MSG_TARGET_DESELECTED:
                    /* Broadcast Intent Target Deselected */
                    if (DBG) Log.d(TAG, "Target Deselected");
                    Intent intent = new Intent();
  //* needed                  intent.setAction(NativeNfcManager.INTERNAL_TARGET_DESELECTED_ACTION);
                    if (DBG) Log.d(TAG, "Broadcasting Intent");
  //* needed_simple                mNfcService.mContext.sendOrderedBroadcast(intent, NFC_PERM);
                    break;

                case MSG_SE_LISTEN_ACTIVATED: {
                    if (DBG) Log.d(TAG, "SE LISTEN MODE ACTIVATED");
                    Intent listenModeActivated = new Intent();
                    listenModeActivated.setAction(ACTION_SE_LISTEN_ACTIVATED);
                    sendSeBroadcast(listenModeActivated);
                    break;
                }

                case MSG_SE_LISTEN_DEACTIVATED: {
                    if (DBG) Log.d(TAG, "SE LISTEN MODE DEACTIVATED");
                    Intent listenModeDeactivated = new Intent();
                    listenModeDeactivated.setAction(ACTION_SE_LISTEN_DEACTIVATED);
                    sendSeBroadcast(listenModeDeactivated);
                    break;
                }

                case MSG_SE_DELIVER_INTENT: {
                    //if (DBG) Log.d(TAG, "SE DELIVER INTENT");
                    Intent seIntent = (Intent) msg.obj;

                    String action = seIntent.getAction();
                    if (action.equals("com.gsma.services.nfc.action.TRANSACTION_EVENT")) {
                        byte[] byteAid = seIntent.getByteArrayExtra("com.android.nfc_extras.extra.AID");
                        byte[] data = seIntent.getByteArrayExtra("com.android.nfc_extras.extra.DATA");
                        String seName = seIntent.getStringExtra("com.android.nfc_extras.extra.SE_NAME");
                        StringBuffer strAid = new StringBuffer();
                        for (int i = 0; i < byteAid.length; i++) {
                            String hex = Integer.toHexString(0xFF & byteAid[i]);
                            if (hex.length() == 1)
                                strAid.append('0');
                            strAid.append(hex);
                        }
                        Intent gsmaIntent = new Intent();
                        gsmaIntent.setAction("com.gsma.services.nfc.action.TRANSACTION_EVENT");
                        if (byteAid != null)
                            gsmaIntent.putExtra("com.gsma.services.nfc.extra.AID", byteAid);
                        if (data != null)
                            gsmaIntent.putExtra("com.gsma.services.nfc.extra.DATA", data);

                        //"nfc://secure:0/<seName>/<strAid>"
                        String url = new String ("nfc://secure:0/" + seName + "/" + strAid);
                        gsmaIntent.setData(Uri.parse(url));
                        gsmaIntent.addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
                        gsmaIntent.setPackage(seIntent.getPackage());

                        Boolean receptionMode = mMultiReceptionMap.get(seName);
                        if (receptionMode == null)
                            receptionMode = defaultTransactionEventReceptionMode;

                        if (receptionMode == multiReceptionMode) {
                            // if multicast reception for GSMA
                            mNfcService.mContext.sendBroadcast(gsmaIntent);
                        } else {
                            // if unicast reception for GSMA
                            try {
                                if (mIsSentUnicastReception == false) {
                                    //start gsma
                                    gsmaIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                                    mContext.startActivity(gsmaIntent);
                                    mIsSentUnicastReception = true;
                                }
                            } catch (Exception e) {
                                if (DBG) Log.d(TAG, "Exception: " + e.getMessage());
                            }
                        }
                    } else {
                        mNfcService.mContext.sendBroadcast(seIntent);
                    }
                    break;
                }

                case MSG_ENABLE_NFC_ADAPTER: {
                    if (DBG) Log.d(TAG, "MSG_ENABLE_NFC_ADAPTER");
                    boolean isSuccess = true;
                    try {
                        if (msg.arg1 == 1) { // user confirmed enabling
                            mNfcService.mNfcAdapter.enable();
                        } else {
                            isSuccess = false;
                        }
                    } catch (RemoteException e) {
                        Log.e(TAG, "failed to enable NfcAdapter");
                        isSuccess = false;
                    }
                    if (isSuccess == false) {
                        String pkg = (String)msg.obj;
                        Intent nfcEnableIntent = new Intent();
//need                        nfcEnableIntent.setAction(NfcAdapter.ACTION_ENABLE_NFC_ADAPTER_FAILED);
                        nfcEnableIntent.setPackage(pkg);
                        mNfcService.mContext.sendBroadcast(nfcEnableIntent);
                    }
                    break;
                }

                case MSG_UPDATE_CARD_EMULATION_ROUTE: {
                    if (DBG) Log.d(TAG, "MSG_UPDATE_CARD_EMULATION_ROUTE");

                    mEeRoutingState = (int)msg.arg1;
                    mPrefsEditor.putInt(PREF_EE_ROUTING_STATE, mEeRoutingState);
                    mPrefsEditor.apply();
                    break;
                }

                case MSG_ENABLE_CARD_EMULATION_MODE: {
                    if (DBG) Log.d(TAG, "MSG_ENABLE_CARD_EMULATION_MODE");

                    mEeRoutingState = mPrefs.getInt(PREF_EE_ROUTING_STATE, mDeviceHost.getEeRoutingState());
                    mEeRoutingState = mPrefs.getInt(PREF_EE_ROUTING_STATE, 0);
                    ApplyRoutingTask applyRoutingTask = mNfcService.new ApplyRoutingTask();
                    applyRoutingTask.execute();
                    try {
                        // Block until route is set
                        applyRoutingTask.get();
                    } catch (Exception e) {
                        Log.e(TAG, "failed to enable card emulation mode");
                    }

                    String pkg = (String)msg.obj;
                    Intent cardEmulationEnableIntent = new Intent();
                    if (mNfceeRouteEnabled == true) {
//needed                        cardEmulationEnableIntent.setAction(NfcAdapter.ACTION_CARD_EMUALTION_ENABLED);
                    } else {
//needed                        cardEmulationEnableIntent.setAction(NfcAdapter.ACTION_ENABLE_CARD_EMUALTION_FAILED);
                    }
                    cardEmulationEnableIntent.setPackage(pkg);
                    mNfcService.mContext.sendBroadcast(cardEmulationEnableIntent);
                    break;
                }

                case MSG_DISABLE_CARD_EMULATION_MODE: {
                    if (DBG) Log.d(TAG, "MSG_DISABLE_CARD_EMULATION_MODE");

                    mEeRoutingState = ROUTE_OFF;
                    ApplyRoutingTask applyRoutingTask = mNfcService.new ApplyRoutingTask();
                    applyRoutingTask.execute();
                    try {
                        // Block until route is set
                        applyRoutingTask.get();
                    } catch (Exception e) {
                        Log.e(TAG, "failed to disable card emulation mode");
                    }

                    String pkg = (String)msg.obj;
                    Intent cardEmulationEnableIntent = new Intent();
//need o update with screen state                    if (mNfceeRouteEnabled == false) {
//move define somewhere                        cardEmulationEnableIntent.setAction(NfcAdapter.ACTION_CARD_EMUALTION_DISABLED);

//need                    } else {
//need                        cardEmulationEnableIntent.setAction(NfcAdapter.ACTION_DISABLE_CARD_EMUALTION_FAILED);
//need                    }
                    cardEmulationEnableIntent.setPackage(pkg);
                    mNfcService.mContext.sendBroadcast(cardEmulationEnableIntent);
                    break;
                }
                case MSG_SET_ACTIVE_SECURE_ELEMENT: {
                    if (DBG) Log.d(TAG, "MSG_SET_ACTIVE_SECURE_ELEMENT");

                    String seName = (String)msg.obj;
                    if (!mNfcService.mActiveSecureElement.equals(seName)) {
                        mNfcService.mActiveSecureElement = seName;
                        mPrefsEditor.putString(PREF_ACTIVE_SECURE_ELEMENT, mNfcService.mActiveSecureElement);
                        mPrefsEditor.apply();

                        applyRouting(false);
                    }
                    break;
                }

                case MSG_SET_CLF_AID_FILTER_LIST: {
                    byte[] filterList = (byte [])msg.obj;

                    if (DBG) Log.d(TAG, "MSG_SET_CLF_AID_FILTER_LIST");
                    mDeviceHost.setClfAidFilterList(filterList);
                    mDeviceHost.updateHostPresence(mNfcService.mHciUiState, mNfcService.mNfccUiState);
                    break;
                }

                case MSG_ENABLE_CLF_AID_FILTER_COND: {
                    byte filterConditionTag = (byte)msg.arg1;

                    if (DBG) Log.d(TAG, "MSG_ENABLE_CLF_AID_FILTER_COND");
                    mDeviceHost.enableClfAidFilterCondition(filterConditionTag);
                    mDeviceHost.updateHostPresence(mNfcService.mHciUiState, mNfcService.mNfccUiState);
                    break;
                }

                case MSG_DISABLE_CLF_AID_FILTER_COND: {
                    byte filterConditionTag = (byte)msg.arg1;

                    if (DBG) Log.d(TAG, "MSG_DISABLE_CLF_AID_FILTER_COND");
                    mDeviceHost.disableClfAidFilterCondition(filterConditionTag);
                    mDeviceHost.updateHostPresence(mNfcService.mHciUiState, mNfcService.mNfccUiState);
                    break;
                }

                case MSG_NOTIFY_RF_INTF_DEACTIVATED: {
                    if (DBG) Log.d(TAG, "MSG_NOTIFY_RF_INTF_DEACTIVATED");
                    mDeviceHost.notifyApduGateRfIntfDeactivated();
                    break;
                }

                case MSG_GET_DEFAULT_ISO_DEP_ROUTE: {
                    if (DBG) Log.d(TAG, "MSG_GET_DEFAULT_ISO_DEP_ROUTE");

                    String selectionListWithComma;
                    String seListWithComma = mDeviceHost.getSecureElementList();
                    if (seListWithComma != null) {
                        selectionListWithComma = displayNameForDeviceHost + "," + seListWithComma;
                    } else {
                        selectionListWithComma = new String(displayNameForDeviceHost);
                    }

                    String currentDefaultRoute = mDeviceHost.getDefaultRoute();
                    if (currentDefaultRoute == null) {
                        currentDefaultRoute = new String("TBD");
                    } else if (currentDefaultRoute.equals("DH")) {
                        currentDefaultRoute = new String(displayNameForDeviceHost);
                    }

                    if (DBG) Log.d(TAG, "currentDefaultRoute = " + currentDefaultRoute);
                    if (DBG) Log.d(TAG, "selectionListWithComma = " + selectionListWithComma);

                    Intent dialogIntent = new Intent(mContext, SelectDefaultIsoDepRouteActivity.class);
                    dialogIntent.putExtra(EXTRA_SE_NAME, currentDefaultRoute);
                    dialogIntent.putExtra(EXTRA_SE_NAME_LIST, selectionListWithComma);
                    dialogIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    mContext.startActivity(dialogIntent);
                    break;
                }

                case MSG_SET_DEFAULT_ISO_DEP_ROUTE: {
                    if (DBG) Log.d(TAG, "MSG_SET_DEFAULT_ISO_DEP_ROUTE");

                    String seName = (String)msg.obj;
                    if (seName.equals(displayNameForDeviceHost)) {
                        mNfcService.setDefaultRoute("DH");
                    } else {
                        mNfcService.setDefaultRoute(seName);
                    }
                    break;
                }

                default:
                    Log.e(TAG, "Unknown message received");
                    break;
            }
        }
    }

    void sendMessage(int what, Object obj) {
        Message msg = new Message();
        msg.what = what;
        msg.obj = obj;
        mHandler.sendMessage(msg);
    }

    public void onCardEmulationAidSelected(byte[] dataBuf) {
        if(DBG) Log.d(TAG, "onCardEmulationAidSelected()");
        if(!mIsHceCapable || SE_BROADCASTS_WITH_HCE)
            sendMessage(mHandler.MSG_CARD_EMULATION, dataBuf);
    }

    public void onRfInterfaceDeactivated () {
        if(DBG) Log.d(TAG, "onRfInterfaceDeactivated()");
        sendMessage(mHandler.MSG_NOTIFY_RF_INTF_DEACTIVATED, null);
    }

    private void enableAfterUserConfirm (String pkgName) {
        if (DBG) Log.d(TAG, "enableAfterUserConfirm()");

        Intent dialogIntent = new Intent(mContext, ConfirmNfcEnableActivity.class);
        dialogIntent.putExtra("com.android.nfc.action.EXTRA_PKG", pkgName);
        dialogIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        mContext.startActivity(dialogIntent);
    }

    private boolean checkCertificatesFromSim (String pkg, String seName) {
        if (DBG) Log.d(TAG, "checkCertificatesFromSim() " + pkg + ", " + seName);

        Intent checkCertificateIntent = new Intent();
        checkCertificateIntent.setAction("org.simalliance.openmobileapi.service.ACTION_CHECK_CERT");
        checkCertificateIntent.setPackage("org.simalliance.openmobileapi.service");
        checkCertificateIntent.putExtra("org.simalliance.openmobileapi.service.EXTRA_SE_NAME", seName);
        checkCertificateIntent.putExtra("org.simalliance.openmobileapi.service.EXTRA_PKG", pkg);
        mContext.sendBroadcast(checkCertificateIntent);

        mWaitOMACheckCert = new Object();
        mHasOMACert = false;
        try {
            synchronized (mWaitOMACheckCert) {
                mWaitOMACheckCert.wait(1000); //add timeout ms
            }
        } catch (InterruptedException e) {
            // Should not happen; fall-through to abort.
            Log.w(TAG, "interruped.");
        }
        mWaitOMACheckCert = null;

        if (mHasOMACert) {
            return true;
        } else {
            return false;
        }
    }

    private boolean checkX509CertificatesFromSim (String pkg, String seName) {
        if (DBG) Log.d(TAG, "checkX509CertificatesFromSim() " + pkg + ", " + seName);

        Intent checkCertificateIntent = new Intent();
        checkCertificateIntent.setAction("org.simalliance.openmobileapi.service.ACTION_CHECK_X509");
        checkCertificateIntent.setPackage("org.simalliance.openmobileapi.service");
        checkCertificateIntent.putExtra("org.simalliance.openmobileapi.service.EXTRA_SE_NAME", seName);
        checkCertificateIntent.putExtra("org.simalliance.openmobileapi.service.EXTRA_PKG", pkg);
        mContext.sendBroadcast(checkCertificateIntent);

        mWaitOMACheckCert = new Object();
        mHasOMACert = false;
        try {
            synchronized (mWaitOMACheckCert) {
                mWaitOMACheckCert.wait(1000); //add timeout ms
            }
        } catch (InterruptedException e) {
            // Should not happen; fall-through to abort.
            Log.w(TAG, "interruped.");
        }
        mWaitOMACheckCert = null;

        if (mHasOMACert) {
            return true;
        } else {
            return false;
        }
   }

    @Override
    public byte[] getLMRT(String pkg) throws RemoteException {
        if (DBG) Log.d(TAG, "getLMRT() called");
        return mDeviceHost.getLMRT();
    }

    @Override
    public void enableNfcController(String pkg) throws RemoteException {
        if (DBG) Log.d(TAG, "enableNfcController() " + pkg);

        //first ask user if enable NFC or not
        enableAfterUserConfirm (pkg);
    }

    public boolean multiSeRegisterAid(List<String> aid, ComponentName paymentService,
                                      List<String> seName, List<String> priority,
                                      List<String> powerState) {
        Log.d(TAG, "multiSeRegisterAid()");
        if (mMultiSeManager != null) {
            mMultiSeManager.multiSeRegisterAid(aid, paymentService,
                                               seName, priority, powerState);
            return true;
        }
        return false;
    }
    public boolean multiSeControlCmd(byte[] ppse_rsp, int op_code) {
        Log.d(TAG, "multiSeControlCmd()");
        mDeviceHost.multiSeControlCmd(ppse_rsp,op_code);
        return true;
    }
    @Override
    public boolean updateCardEmulationRoute(String pkg, int route)
            throws RemoteException {
        if (DBG) Log.d(TAG, "updateCardEmulationRoute() " + pkg + " route = " + route);

        //NfcService.this.enforceNfceeAdminPerm(mContext);
        NfcPermissions.enforceAdminPermissions(mContext);
        Message msg = new Message();
        msg.what = mHandler.MSG_UPDATE_CARD_EMULATION_ROUTE;
        msg.arg1 = route;
        mHandler.sendMessage(msg);
        return true;
    }

    @Override
    public boolean isCardEmulationEnabled(String pkg) throws RemoteException {
        if (DBG) Log.d(TAG, "isCardEmulationEnabled() " + pkg);
        if (mEeRoutingState == ROUTE_OFF) {
            return false;
        } else {
            return true;
        }
    }

    @Override
    public boolean enableCardEmulationMode(String pkg) throws RemoteException {
        if (DBG) Log.d(TAG, "enableCardEmulationMode() " + pkg);

        if (mNfcService.mActiveSecureElement.startsWith("SIM")) {
            if (checkCertificatesFromSim (pkg, mNfcService.mActiveSecureElement) == false) {
                throw new SecurityException("No cerficates from " + mNfcService.mActiveSecureElement);
            }
        } else {
            //doesn't reference pkg...
           // NfcPermissions.enforceNfceeAdminPerm(mContext);
            NfcPermissions.enforceAdminPermissions(mContext);
        }
        Message msg = new Message();
        msg.what=mHandler.MSG_ENABLE_CARD_EMULATION_MODE;
        msg.obj = pkg;
        mHandler.sendMessage(msg);
        return true;
    }

    @Override
    public boolean disableCardEmulationMode(String pkg) throws RemoteException {
        if (DBG) Log.d(TAG, "disableCardEmulationMode() " + pkg);

        if (mNfcService.mActiveSecureElement.startsWith("SIM")) {
            if (checkCertificatesFromSim (pkg, mNfcService.mActiveSecureElement) == false) {
                throw new SecurityException("No cerficates from " + mNfcService.mActiveSecureElement);
            }
        } else {
            //NfcService.this.enforceNfceeAdminPerm(pkg);
            NfcPermissions.enforceAdminPermissions(mContext);
        }

        sendMessage(mHandler.MSG_DISABLE_CARD_EMULATION_MODE, pkg);
        return true;
    }

    @Override
    public String getActiveSecureElement(String pkg) throws RemoteException {
        if (DBG) Log.d(TAG, "getActiveSecureElement() " + pkg);
        NfcPermissions.enforceAdminPermissions(mContext);
        return mNfcService.mActiveSecureElement;
    }

    @Override
    public void setActiveSecureElement(String pkg, String seName)
            throws RemoteException {
        if (DBG) Log.d(TAG, "setActiveSecureElement() " + pkg + " " + seName);

        if (seName.startsWith("SIM")) {
            if (checkCertificatesFromSim (pkg, seName) == false) {
                throw new SecurityException("No cerficates from " + seName);
            }
        } else {
//            NfcService.this.enforceNfceeAdminPerm(pkg);
            NfcPermissions.enforceAdminPermissions(mContext);
        }
        sendMessage(mHandler.MSG_SET_ACTIVE_SECURE_ELEMENT, seName);
    }

    
    @Override
    public void enableMultiReception(String pkg, String seName)
            throws RemoteException {
        if (DBG) Log.d(TAG, "enableMultiReception() " + pkg + " " + seName);

        if (seName.startsWith("SIM")) {
            if (checkX509CertificatesFromSim (pkg, seName) == false) {
                throw new SecurityException("No cerficates from " + seName);
            }
        } else {
            // NfcService.this.enforceNfceeAdminPerm(pkg);
            NfcPermissions.enforceAdminPermissions(mContext);
        }

        mMultiReceptionMap.remove(seName);
        mMultiReceptionMap.put(seName, Boolean.TRUE);
    }

    @Override
    public boolean isSeEnabled(String pkg, String seName)
            throws RemoteException {
        if (DBG) Log.d(TAG, "isSeEnabled() seName = " + seName);

        String seListWithComma = mDeviceHost.getSecureElementList();
        if (seListWithComma != null) {
            String[] seList = seListWithComma.split(",");
            for (int i = 0; (seList != null)&&(i < seList.length); i++) {
                if (DBG) Log.d(TAG, "seList[" + i + "] = " + seList[i]);
                if (seList[i].equals(seName))
                    return true;
            }
        }
        return false;
    }

    @Override
    public void deliverSeIntent(String pkg, Intent seIntent)
            throws RemoteException {
        //if (DBG) Log.d(TAG, "deliverSeIntent() " + pkg + " " + intent.getAction());
        //NfcService.this.enforceNfceeAdminPerm(pkg);
        NfcPermissions.enforceAdminPermissions(mContext);    
        sendMessage(mHandler.MSG_SE_DELIVER_INTENT, seIntent);    
    }

    @Override
    public void notifyCheckCertResult(String pkg, boolean success)
            throws RemoteException {
        if (DBG) Log.d(TAG, "notifyCheckCertResult() " + pkg + ", success=" + success);

        //NfcService.this.enforceNfceeAdminPerm(pkg);
        NfcPermissions.enforceAdminPermissions(mContext);
        synchronized (mWaitOMACheckCert) {
            if (mWaitOMACheckCert != null) {
                if (success) {
                    mHasOMACert = true;
                } else {
                    mHasOMACert = false;
                }
                mWaitOMACheckCert.notify();
            }
        }
    }

    @Override
    public void selectSEToOpenApduGate(String pkg, String seName)
            throws RemoteException {

        if (DBG) Log.d(TAG, "selectSEToOpenApduGate() " + pkg + ", seName=" + seName);


        if (!seName.startsWith("eSE")) {
            Log.d(TAG, "selectSEToOpenApduGate() invalid secure element for APDU Gate");
            return;
        }

        mSEToOpenAPDUGateMap.put(String.valueOf(getCallingPid()), seName);
    }

    @Override
    public boolean setClfAidFilterList(byte[] filterList) {
        //mContext.enforceCallingOrSelfPermission(NFC_PERM, NFC_PERM_ERROR);

        if (filterList == null) {
            Log.d(TAG, "setClfAidFilterList() filterList should not be null");
            return false;
        }

        if (DBG) Log.d(TAG, "setClfAidFilterList() " + filterList.length + "bytes");
        sendMessage(mHandler.MSG_SET_CLF_AID_FILTER_LIST, filterList);
        return true;
    }

    @Override
    public boolean enableClfAidFilterCondition(byte filterConditionTag) {
        //mContext.enforceCallingOrSelfPermission(NFC_PERM, NFC_PERM_ERROR);

        if (DBG) Log.d(TAG, "enableClfAidFilterCondition() tag = " + filterConditionTag);

        Message msg = new Message();
        msg.what = mHandler.MSG_ENABLE_CLF_AID_FILTER_COND;
        msg.arg1 = filterConditionTag;
        mHandler.sendMessage(msg);

        return true;
    }

    @Override
    public boolean disableClfAidFilterCondition(byte filterConditionTag) {
        //mContext.enforceCallingOrSelfPermission(NFC_PERM, NFC_PERM_ERROR);

        if (DBG) Log.d(TAG, "disableClfAidFilterCondition() tag = " + filterConditionTag);

        Message msg = new Message();
        msg.what = mHandler.MSG_DISABLE_CLF_AID_FILTER_COND;
        msg.arg1 = filterConditionTag;
        mHandler.sendMessage(msg);

        return true;
    }

    @Override
    public boolean commitOffHostService(String packageName, String seName, String description,
                                        int bannerResId, int uid, List<String> aidGroupDescriptions,
                                        List<AidGroup> aidGroups) {
        if (DBG) Log.d(TAG, "commitOffHostService() " + packageName + ", " + seName);

        if (mMultiSeManager != null) {
            mMultiSeManager.multiSeCommit(packageName, seName, description, bannerResId, uid,
                                           aidGroupDescriptions, aidGroups);
        }
        return true;
    }

    @Override
    public boolean deleteOffHostService(String packageName, String seName) {
        if (DBG) Log.d(TAG, "deleteOffHostService() " + packageName + ", " + seName);

        if (mMultiSeManager != null) {
            mMultiSeManager.deleteOffHostService(packageName, seName);
        }
        return true;
    }

    @Override
    public boolean getOffHostServices(String packageName, IQNfcSecureElementManagerCallbacks callbacks) {
        if (DBG) Log.d(TAG, "getOffHostServices() " + packageName);

        try {
            if (mMultiSeManager != null) {
                MultiSeManager.AppPackage appPackage = mMultiSeManager.getOffHostServiceForPackage(packageName);
                if (appPackage != null) {
                    for (Map.Entry<String, MultiSeManager.OffHostService> offHostServiceEntry : appPackage.mOffHostServices.entrySet()) {
                        String seName = offHostServiceEntry.getKey();
                        final MultiSeManager.OffHostService offHostService = offHostServiceEntry.getValue();
                        ArrayList<String> groupDescription = new ArrayList<String>();
                        for (AidGroup aidGroup : offHostService.mApduServiceInfo.getAidGroups()) {
                            groupDescription.add(offHostService.mAidGroupDescriptions.get(aidGroup.getCategory()));
                        }
                        callbacks.onGetOffHostService (false,
                                                       offHostService.mApduServiceInfo.getDescription(),
                                                       seName,
                                                       offHostService.mBannerId,
                                                       groupDescription,
                                                       offHostService.mApduServiceInfo.getAidGroups());
                    }
                }
            } else {
                Log.e(TAG, "getOffHostServices() MultiSeManager is not initialized");
            }
            callbacks.onGetOffHostService (true, null, null, 0, null, null);
        } catch(Exception e) {
            Log.e(TAG, "getOffHostServices() " + e.getMessage());
        }
        return true;
    }

    private Bundle writeNoException() {
        Bundle p = new Bundle();
        p.putInt("e", 0);
        return p;
    }

    private Bundle writeEeException(int exceptionType, String message) {
        Bundle p = new Bundle();
        p.putInt("e", exceptionType);
        p.putString("m", message);
        return p;
    }

    private void restoreHostPresence(){
        int hciUiState;
        int nfccUiState;

        Log.d(TAG, "restoreHostPresence()");
        if (mNfcService.mScreenState == ScreenStateHelper.SCREEN_STATE_ON_UNLOCKED) {
            hciUiState = mNfcService.HCI_UI_STATE_AVAILABLE;
            nfccUiState = mNfcService.NFCC_UI_STATE_UNLOCKED;
        } else if (mNfcService.mScreenState == ScreenStateHelper.SCREEN_STATE_ON_LOCKED){
            hciUiState = mNfcService.HCI_UI_STATE_LOCKED_NOT_NOTIFIABLE;
            nfccUiState = mNfcService.NFCC_UI_STATE_LOCKED; // HCE can be supported in screen locked
        } else {
            hciUiState = mNfcService.HCI_UI_STATE_LOCKED_NOT_NOTIFIABLE;
            nfccUiState = mNfcService.NFCC_UI_STATE_OFF;
        }
        mDeviceHost.updateHostPresence(hciUiState, nfccUiState);
    }

    @Override
    public Bundle open(String pkg, IBinder b) throws RemoteException {
        //NfcService.this.enforceNfceeAdminPerm(pkg);
        NfcPermissions.enforceAdminPermissions(mContext);
        //maybe should also do whitelist checking using package name

        if (mNfcService.mScreenState <= ScreenStateHelper.SCREEN_STATE_ON_LOCKED){
            int hciUiState = mNfcService.HCI_UI_STATE_LOCKED_NOT_NOTIFIABLE;
            int nfccUiState = mNfcService.NFCC_UI_STATE_UNLOCKED;
            mDeviceHost.updateHostPresence(hciUiState, nfccUiState);
        }

        Bundle result;
        int handle = _open(b);
        if (handle < 0) {
            result = writeEeException(handle, "NFCEE open exception.");
            restoreHostPresence();
        } else {
            result = writeNoException();
        }
        return result;
    }



    @Override
    public Bundle transceive(String pkg, byte[] data_in) throws RemoteException {
        // NfcService.this.enforceNfceeAdminPerm(pkg);
        NfcPermissions.enforceAdminPermissions(mContext);

        Bundle result;
        byte[] out;
        try {
            out = _transceive(data_in);
            result = writeNoException();
            result.putByteArray("out", out);
        } catch (IOException e) {
            result = writeEeException(eeErrorCodes.EE_ERROR_IO, e.getMessage());
        }
        return result;
    }

    private byte[] _transceive(byte[] data) throws IOException {
        synchronized(mNfcService) {
            if (!mNfcService.isNfcEnabled()) {
                throw new IOException("NFC is not enabled");
            }
            if (mOpenEe == null) {
                throw new IOException("NFC EE is not open");
            }
            if (getCallingPid() != mOpenEe.pid) {
                throw new SecurityException("Wrong PID");
            }
        }

        return doTransceive(mOpenEe.handle, data);
    }

    int doOpenSecureElementConnection(int nfceeId) {
        mEeWakeLock.acquire();
        try {
            return mSecureElement.doOpenSecureElementConnection(nfceeId);
        } finally {
            mEeWakeLock.release();
        }
    }

    byte[] doTransceive(int handle, byte[] cmd) {
        mEeWakeLock.acquire();
        try {
            return doTransceiveNoLock(handle, cmd);
        } finally {
            mEeWakeLock.release();
        }
    }

    byte[] doTransceiveNoLock(int handle, byte[] cmd) {
        return mSecureElement.doTransceive(handle, cmd);
    }

    void doDisconnect(int handle) {
        mEeWakeLock.acquire();
        try {
            mSecureElement.doDisconnect(handle);
        } finally {
            mEeWakeLock.release();
        }
    }

    private int _open(IBinder b) {
        synchronized(mNfcService) {
            if (!mNfcService.isNfcEnabled()) {
                return eeErrorCodes.EE_ERROR_NFC_DISABLED;
            }
            if (mNfcService.mInProvisionMode) {
                // Deny access to the NFCEE as long as the device is being setup
                return eeErrorCodes.EE_ERROR_IO;
            }
            if (mOpenEe != null) {
                return eeErrorCodes.EE_ERROR_ALREADY_OPEN;
            }

            String seName = mSEToOpenAPDUGateMap.get(String.valueOf(getCallingPid()));
            if (seName == null) // if it's not set then use eSE1 as defualt
                seName = new String("eSE1");

            int nfceeId = mDeviceHost.getNfceeId(seName);

            int handle = doOpenSecureElementConnection(nfceeId);
            if (handle < 0) {
                return handle;
            }
            //mDeviceHost.setTimeout(TagTechnology.ISO_DEP, 30000);

            mOpenEe = new OpenSecureElement(getCallingPid(), handle, b);
            try {
                b.linkToDeath(mOpenEe, 0);
            } catch (RemoteException e) {
                mOpenEe.binderDied();
            }

            // Add the calling package to the list of packages that have accessed
            // the secure element.
//            for (String packageName : mContext.getPackageManager().getPackagesForUid(getCallingUid())) {
//                mSePackages.add(packageName);
//            }

            return handle;
       }
    }

    @Override
    public Bundle close(String pkg, IBinder binder) throws RemoteException {
//        NfcService.this.enforceNfceeAdminPerm(pkg);
        NfcPermissions.enforceAdminPermissions(mContext);
        Bundle result;
        try {
            _nfcEeClose(getCallingPid(), binder);
            result = writeNoException();
        } catch (IOException e) {
            result = writeEeException(eeErrorCodes.EE_ERROR_IO, e.getMessage());
        }
        return result;
    }

    void _nfcEeClose(int callingPid, IBinder binder) throws IOException {
        // Blocks until a pending open() or transceive() times out.
        //TODO: This is incorrect behavior - the close should interrupt pending
        // operations. However this is not supported by current hardware.

        synchronized (mNfcService) {
            mSEToOpenAPDUGateMap.remove(String.valueOf(callingPid));

            if (!mNfcService.isNfcEnabledOrShuttingDown()) {
                throw new IOException("NFC adapter is disabled");
            }
            if (mOpenEe == null) {
                throw new IOException("NFC EE closed");
            }
            if (callingPid != -1 && callingPid != mOpenEe.pid) {
                throw new SecurityException("Wrong PID");
            }
            if (mOpenEe.binder != binder) {
                throw new SecurityException("Wrong binder handle");
            }

            binder.unlinkToDeath(mOpenEe, 0);
            //mDeviceHost.resetTimeouts();
            doDisconnect(mOpenEe.handle);
            mOpenEe = null;

            if (mDeviceHost.isRfInterfaceActivated() == false) {
                mNfcService.applyRouting(true);
            }
        }
    }

    public void updateDefaultOffHostRoute (String seName) {
        if (DBG) Log.d(TAG, "updateDefaultOffHostRoute() " + seName);
        sendMessage(mHandler.MSG_SET_ACTIVE_SECURE_ELEMENT, seName);
    }

    public static QSecureElementManager getInstance() {
        if (DBG) Log.d(TAG, "getInstance");
        return sService;
    }
}
