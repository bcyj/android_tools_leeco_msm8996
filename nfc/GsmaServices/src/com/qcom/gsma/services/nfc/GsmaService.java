/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qcom.gsma.services.nfc;

import java.util.HashMap;
import com.gsma.services.nfc.*;
import com.gsma.services.nfc.IGsmaService;
import com.gsma.services.nfc.IGsmaServiceCallbacks;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;

import android.os.Message;
import android.os.Messenger;
import android.os.IBinder;
import android.os.RemoteException;
import android.app.Service;
import android.nfc.NfcAdapter;
import android.util.Log;
import java.util.ArrayList;
import java.util.List;
import com.android.qcom.nfc_extras.*;

public class GsmaService extends Service {

    private static final String TAG = "GsmaNfcService";
    final static boolean DBG = true;
    static NfcAdapter sNfcAdapter = null;
    private BroadcastReceiver mNfcAdapterEventReceiver;
    private Context mContext;
    static HashMap<Context, NfcQcomAdapter> sNfcQcomAdapterMap = new HashMap();

    static NfcQcomAdapter getNfcQcomAdapter (Context context) {
        return sNfcQcomAdapterMap.get(context);
    }

    static final String ENABLE_NFCC = "ENABLE_NFCC";
    static final String ENABLE_CE_MODE = "ENABLE_CE_MODE";
    static final String DISABLE_CE_MODE = "DISABLE_CE_MODE";

    static final int MSG_RESULT_SUCCESS = 1;
    static final int MSG_RESULT_FAILED = 0;
    static final int MSG_RESULT_ENABLE_NFCC = 1;
    static final int MSG_RESULT_ENABLE_CE_MODE = 2;
    static final int MSG_RESULT_DISABLE_CE_MODE = 3;

    static HashMap<String, Messenger> sClientMessengerMap = new HashMap();
    String pack = null;

    private AdapterCallbacks mAdapterCallbacks;

    static HashMap<Context, IGsmaServiceCallbacks> sIGsmaServiceCallbacksMap = new HashMap();

    private void sendMsg(Messenger clientMessenger, int msgId, int result) {
        Message msg = Message.obtain(null, msgId, result, 0);
        try {
            clientMessenger.send(msg);
        } catch (RemoteException e) {
            Log.e(TAG, "clientMessenger.send() failed: " + e.getMessage());
        }
    }

    private void registerNfcAdapterEvent(String action, Messenger clientMessenger) {
        if (DBG) Log.d(TAG, "register NFC Adapter event for action:" + action);

        sClientMessengerMap.put(action, clientMessenger);

        if (mNfcAdapterEventReceiver != null) {
            return;
        }

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(NfcAdapter.ACTION_ADAPTER_STATE_CHANGED);
        //intentFilter.addAction(NfcAdapter.ACTION_ENABLE_NFC_ADAPTER_FAILED);
        intentFilter.addAction("com.android.qcom.nfc_extras.action.ENABLE_NFC_ADAPTER_FAILED");
        //intentFilter.addAction(NfcAdapter.ACTION_CARD_EMUALTION_ENABLED);
        intentFilter.addAction("com.android.qcom.nfc_extras.action.CARD_EMUALTION_ENABLED");
        //intentFilter.addAction(NfcAdapter.ACTION_ENABLE_CARD_EMULATION_FAILED);
        intentFilter.addAction("com.android.qcom.nfc_extras.action.ENABLE_CARD_EMULATION_FAILED");
        //intentFilter.addAction(NfcAdapter.ACTION_CARD_EMUALTION_DISABLED);
        intentFilter.addAction("com.android.qcom.nfc_extras.action.CARD_EMUALTION_DISABLED");
        //intentFilter.addAction(NfcAdapter.ACTION_DISABLE_CARD_EMUALTION_FAILED);
        intentFilter.addAction("com.android.qcom.nfc_extras.action.DISABLE_CARD_EMUALTION_FAILED");

        mNfcAdapterEventReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (DBG) Log.d(TAG, "got intent " + action);
                if (action.equals(NfcAdapter.ACTION_ADAPTER_STATE_CHANGED)) {
                    int isEnabled;
                    int adapterState = intent.getIntExtra(NfcAdapter.EXTRA_ADAPTER_STATE, 1);
                    if (adapterState == NfcAdapter.STATE_TURNING_ON) {
                        if (DBG) Log.d(TAG, "NfcAdapter.STATE_TURNING_ON");
                        // wait for STATE_ON
                        return;
                    } else if (adapterState == NfcAdapter.STATE_ON) {
                        if (DBG) Log.d(TAG, "NfcAdapter.STATE_ON");
                        isEnabled = 1;
                    } else {
                        isEnabled = 0;
                    }

                    Messenger clientMessenger = sClientMessengerMap.remove(ENABLE_NFCC);
                    if (clientMessenger != null) {
                        sendMsg(clientMessenger, MSG_RESULT_ENABLE_NFCC, MSG_RESULT_SUCCESS);
                    }
                } else if (action.equals("com.android.qcom.nfc_extras.action.ENABLE_NFC_ADAPTER_FAILED")) {
                    if (DBG) Log.d(TAG, "Failed to enable NFC Controller");

                    Messenger clientMessenger = sClientMessengerMap.remove(ENABLE_NFCC);
                    if (clientMessenger != null) {
                        sendMsg(clientMessenger, MSG_RESULT_ENABLE_NFCC, MSG_RESULT_FAILED);
                    }
                } else if (action.equals("com.android.qcom.nfc_extras.action.CARD_EMUALTION_ENABLED")) {
                    if (DBG) Log.d(TAG, "Card Emulation Mode is enabled");

                    Messenger clientMessenger = sClientMessengerMap.remove(ENABLE_CE_MODE);
                    if (clientMessenger != null) {
                        sendMsg(clientMessenger, MSG_RESULT_ENABLE_CE_MODE, MSG_RESULT_SUCCESS);
                    }
                } else if (action.equals("com.android.qcom.nfc_extras.action.CARD_EMUALTION_DISABLED")) {
                    if (DBG) Log.d(TAG, "Card Emulation Mode is disabled");

                    Messenger clientMessenger = sClientMessengerMap.remove(DISABLE_CE_MODE);
                    if (clientMessenger != null) {
                        sendMsg(clientMessenger, MSG_RESULT_DISABLE_CE_MODE, MSG_RESULT_SUCCESS);
                    }
                } else if (action.equals("com.android.qcom.nfc_extras.action.ENABLE_CARD_EMULATION_FAILED")) {
                    if (DBG) Log.d(TAG, "Enable Card Emulation Mode failed");

                    Messenger clientMessenger = sClientMessengerMap.remove(ENABLE_CE_MODE);
                    if (clientMessenger != null) {
                        sendMsg(clientMessenger, MSG_RESULT_ENABLE_CE_MODE, MSG_RESULT_FAILED);
                    }
                } else if (action.equals("com.android.qcom.nfc_extras.action.DISABLE_CARD_EMUALTION_FAILED")) {
                    if (DBG) Log.d(TAG, "Disable Card Emulation Mode failed");

                    Messenger clientMessenger = sClientMessengerMap.remove(DISABLE_CE_MODE);
                    if (clientMessenger != null) {
                        sendMsg(clientMessenger, MSG_RESULT_DISABLE_CE_MODE, MSG_RESULT_FAILED);
                    }
                }

                // if no client is waiting
                if (sClientMessengerMap.isEmpty()) {
                    unregisterNfcAdapterEvent();
                } else if (DBG) {
                    Log.d(TAG, "sClientMessengerMap.size() = " + sClientMessengerMap.size());
                }
            }
        };
        mContext.registerReceiver(mNfcAdapterEventReceiver, intentFilter);
    }

    private void unregisterNfcAdapterEvent() {
        if (DBG) Log.d(TAG, "unregister NFC Adapter event");
        mContext.unregisterReceiver(mNfcAdapterEventReceiver);
        mNfcAdapterEventReceiver = null;
    }

    private class AdapterCallbacks implements NfcQcomAdapter.Callbacks {
        public void onGetOffHostService(boolean isLast, String description, String seName, int bannerResId,
                                        List<String> dynamicAidGroupDescriptions,
                                        List<android.nfc.cardemulation.AidGroup> dynamicAidGroups) {

            if (DBG) Log.d(TAG, "onGetOffHostService() " + isLast + ", " + description + ", "
                                 + seName + ", " + String.valueOf(bannerResId));

            IGsmaServiceCallbacks callbacks = sIGsmaServiceCallbacksMap.get(mContext);
            if (callbacks != null) {
                try {
                    callbacks.onGetOffHostService(isLast, description, seName, bannerResId,
                                                  dynamicAidGroupDescriptions, dynamicAidGroups);
                } catch (Exception e) {
                    Log.e(TAG, "onGetOffHostService() " + e.getMessage());
                }
            }
        }
    }

    private IGsmaService.Stub mGsmaServiceBinder = new IGsmaService.Stub() {
        @Override
        public boolean isNfccEnabled() {
            if (DBG) Log.d(TAG, "isNfccEnabled()");

            if (sNfcAdapter != null) {
                return sNfcAdapter.isEnabled();
            } else {
                return false;
            }
        }

        @Override
        public boolean enableNfcc(Messenger clientMessenger) {
            if (DBG) Log.d(TAG, "enableNfcc()");

            if (sNfcAdapter != null) {
                try {
                    NfcQcomAdapter nfcQcomAdapter = getNfcQcomAdapter (getApplicationContext());
                    if (nfcQcomAdapter == null) {
                        if (DBG) Log.d(TAG, "cannot get NfcQcomAdapter");
                        return false;
                    }

                    // NfcQcomAdapter will ask user if user wants to enable NFC or not
                    if (nfcQcomAdapter.enableNfcController() == false) {
                        return false;
                    } else {
                        // store client messenger to notify when NFC controller enabling is finished
                        // wait for intent
                        if (sClientMessengerMap.get(ENABLE_NFCC) == null) {
                            registerNfcAdapterEvent(ENABLE_NFCC, clientMessenger);
                            return true;
                        } else {
                            Log.e(TAG, "Enabling NFCC is in progress");
                            return false;
                        }
                    }
                } catch (Exception e) {
                    Log.e(TAG, "enableNfcc() : " + e.getMessage());
                    return false;
                }
            } else {
                return false;
            }
        }

        @Override
        public boolean isCardEmulationEnabled() {
            if (DBG) Log.d(TAG, "isCardEmulationEnabled()");

            if ((sNfcAdapter != null)&&(sNfcAdapter.isEnabled())) {
                NfcQcomAdapter nfcQcomAdapter = getNfcQcomAdapter (getApplicationContext());
                if (nfcQcomAdapter == null) {
                    if (DBG) Log.d(TAG, "cannot get NfcQcomAdapter");
                    return false;
                }

                return (nfcQcomAdapter.isCardEmulationEnabled());
            } else {
                return false;
            }
        }

        @Override
        public boolean enableCardEmulationMode(Messenger clientMessenger) {
            if (DBG) Log.d(TAG, "enableCardEmulationMode()");

            try {
                NfcQcomAdapter nfcQcomAdapter = getNfcQcomAdapter (getApplicationContext());
                if (nfcQcomAdapter == null) {
                    if (DBG) Log.d(TAG, "cannot get NfcQcomAdapter");
                    return false;
                }

                if (nfcQcomAdapter.enableCardEmulationMode() == false) {
                    return false;
                } else {
                    // store client messenger to notify when NFC controller enabling is finished
                    // wait for intent
                    if (sClientMessengerMap.get(ENABLE_CE_MODE) == null) {
                        registerNfcAdapterEvent(ENABLE_CE_MODE, clientMessenger);
                        return true;
                    } else {
                        Log.e(TAG, "Enabling NFCC is in progress");
                        return false;
                    }
                }
            } catch (Exception e) {
                // SecurityException - if the application is not allowed to use this API
                throw new SecurityException("application is not allowed");
            }
        }

        @Override
        public boolean disableCardEmulationMode(Messenger clientMessenger) {
            if (DBG) Log.d(TAG, "disableCardEmulationMode()");

            try {
                NfcQcomAdapter nfcQcomAdapter = getNfcQcomAdapter (getApplicationContext());
                if (nfcQcomAdapter == null) {
                    if (DBG) Log.d(TAG, "cannot get NfcQcomAdapter");
                    return false;
                }

                if (nfcQcomAdapter.disableCardEmulationMode() == false) {
                    return false;
                } else {
                    // store client messenger to notify when NFC controller enabling is finished
                    // wait for intent
                    if (sClientMessengerMap.get(DISABLE_CE_MODE) == null) {
                        registerNfcAdapterEvent(DISABLE_CE_MODE, clientMessenger);
                        return true;
                    } else {
                        Log.e(TAG, "Enabling NFCC is in progress");
                        return false;
                    }
                }
            } catch (Exception e) {
                // SecurityException - if the application is not allowed to use this API
                throw new SecurityException("application is not allowed");
            }
        }

        @Override
        public String getActiveSecureElement() {
            if (DBG) Log.d(TAG, "getActiveSecureElement()");
            try {
                NfcQcomAdapter nfcQcomAdapter = getNfcQcomAdapter (getApplicationContext());
                if (nfcQcomAdapter == null) {
                    if (DBG) Log.d(TAG, "cannot get NfcQcomAdapter");
                    return null;
                }
                return nfcQcomAdapter.getActiveSecureElement();
            } catch (Exception e) {
                Log.e(TAG, "getActiveSecureElement() : " + e.getMessage());
            }
            return null;
        }

        @Override
        public void setActiveSecureElement(String SEName) {
            if (DBG) Log.d(TAG, "setActiveSecureElement() " + SEName);
            try {
                NfcQcomAdapter nfcQcomAdapter = getNfcQcomAdapter (getApplicationContext());
                if (nfcQcomAdapter == null) {
                    if (DBG) Log.d(TAG, "cannot get NfcQcomAdapter");
                    return;
                }
                nfcQcomAdapter.setActiveSecureElement(SEName);
            } catch (Exception e) {
                Log.e(TAG, "setActiveSecureElement() : " + e.getMessage());
                // SecurityException - if the application is not allowed to use this API
                throw new SecurityException("application is not allowed");
            }
        }

        @Override
        public void mgetPname(String packageN) {
            pack = packageN;
        }

        @Override
        public void enableMultiReception(String SEName) {
            if (DBG) Log.d(TAG, "enableMultiReception() " + SEName);
            try {
                NfcQcomAdapter nfcQcomAdapter = getNfcQcomAdapter (getApplicationContext());
                if (nfcQcomAdapter == null) {
                    if (DBG) Log.d(TAG, "cannot get NfcQcomAdapter");
                    return;
                }
                nfcQcomAdapter.GsmaPack(pack);
                nfcQcomAdapter.enableMultiReception(SEName);
            } catch (Exception e) {
                Log.e(TAG, "enableMultiReception() : " + e.getMessage());
                // SecurityException - if the application is not allowed to use this API
                throw new SecurityException("application is not allowed");
            }
        }

        /*
        * removes aids from aidGroups that are not selectable for the given secure eleement.
        * @ param seName the secure element to query for selectability
        * @ param aidGroups the aids that we are checking and removing if not selectable
        */
        private synchronized void filterOutAidsNotInSecureElement (String seName,
                                               List<android.nfc.cardemulation.AidGroup> aidGroups) {
            if (DBG) Log.d(TAG, "filterOutAidsNotInSecureElement() " + seName);
            List<android.nfc.cardemulation.AidGroup> selectableAidGroups
                                              = new ArrayList<android.nfc.cardemulation.AidGroup>();
            final List<String> selectableAidList = new ArrayList<String>();
            final Object lock = new Object();
            final String ACTION_CHECK_AID = "org.simalliance.openmobileapi.service.ACTION_CHECK_AID";
            final String EXTRA_AIDS = "org.simalliance.openmobileapi.service.EXTRA_AIDS";
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(ACTION_CHECK_AID);

            // An AidGroup has a list of aids. Each AidGroup will be sent to the SmartCardService
            // for filtering by broadcast intent. The filtered lists will be returned and parsed
            // in below broadcast reciever.
            // These requests are serialized by the lock object.
            BroadcastReceiver receiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    String action = intent.getAction();

                    // Smartcard Service will return AIDs those are selectable
                    if (action.equals(ACTION_CHECK_AID)){
                        synchronized (lock) {
                            String aidStringsWithComma = intent.getStringExtra(EXTRA_AIDS);
                            //if (DBG) Log.d(TAG, "filterOutAidsNotInSecureElement(): selectable AIDs:"
                            //                    + aidStringsWithComma);
                            String[] aidStrings;
                            if (aidStringsWithComma != null) {
                                aidStrings = aidStringsWithComma.split(",");
                                for (int i = 0; (aidStrings != null)&&(i < aidStrings.length); i++) {
                                    selectableAidList.add(aidStrings[i]);
                                }
                            }

                            lock.notifyAll();
                            if (DBG) Log.d(TAG, "filterOutAidsNotInSecureElement(): Notifying All" );
                        }
                    }
                }
            };

            mContext.registerReceiver(receiver, intentFilter);

            for (android.nfc.cardemulation.AidGroup aidGroup: aidGroups) {
                selectableAidList.clear();
                StringBuilder aids = new StringBuilder();
                for (String aid: aidGroup.getAids()) {
                    if (aids.length() > 0)
                        aids.append(",");
                    aids.append(aid);
                }
                //if (DBG) Log.d(TAG, "filterOutAidsNotInSecureElement(): checking " + seName +
                //                    ", " + aids.toString());

                Intent reqIntent = new Intent();
                reqIntent.setAction("org.simalliance.openmobileapi.service.ACTION_CHECK_AID");
                reqIntent.putExtra("org.simalliance.openmobileapi.service.EXTRA_SE_NAME", seName);
                reqIntent.putExtra("org.simalliance.openmobileapi.service.EXTRA_AIDS", aids.toString());
                reqIntent.setPackage("org.simalliance.openmobileapi.service");
                mContext.sendBroadcast(reqIntent);

                try {
                    synchronized (lock) {
                        lock.wait(5000);
                        if (selectableAidList.size() > 0) {
                            // create new AID Group with only selectable AIDs
                            android.nfc.cardemulation.AidGroup selectableAidGroup
                                = new android.nfc.cardemulation.AidGroup(selectableAidList,
                                                                         aidGroup.getCategory());
                            selectableAidGroups.add(selectableAidGroup);
                        }
                    }
                } catch (Exception e) {
                    Log.e(TAG, "filterOutAidsNotInSecureElement(): " + e.getMessage());
                }
            }

            mContext.unregisterReceiver(receiver);

            // replace AID Group with only selectable AIDs
            aidGroups.clear();
            for (android.nfc.cardemulation.AidGroup aidGroup: selectableAidGroups) {
                aidGroups.add(aidGroup);
            }
        }

        @Override
        public boolean commitOffHostService(String packageName, String seName, String description,
                                            int bannerResId, int uid, List<String> aidGroupDescriptions,
                                            List<android.nfc.cardemulation.AidGroup> aidGroups) {
            if (DBG) Log.d(TAG, "commitOffHostService() " + packageName + ", " + seName);

            if ((sNfcAdapter != null)&&(sNfcAdapter.isEnabled())) {
                NfcQcomAdapter adapter = getNfcQcomAdapter (getApplicationContext());
                if (adapter == null) {
                    if (DBG) Log.d(TAG, "cannot get NfcQcomAdapter");
                    return false;
                }

                // filter out AIDs which are not selectable
                filterOutAidsNotInSecureElement (seName, aidGroups);

                return (adapter.commitOffHostService(packageName, seName, description,
                                                     bannerResId, uid, aidGroupDescriptions, aidGroups));
            } else {
                return false;
            }
        }

        @Override
        public boolean deleteOffHostService(String packageName, String seName) {
            if (DBG) Log.d(TAG, "deleteOffHostService() " + packageName + ", " + seName);

            if ((sNfcAdapter != null)&&(sNfcAdapter.isEnabled())) {
                NfcQcomAdapter adapter = getNfcQcomAdapter (getApplicationContext());
                if (adapter == null) {
                    if (DBG) Log.d(TAG, "cannot get NfcQcomAdapter");
                    return false;
                }

                return (adapter.deleteOffHostService(packageName, seName));
            } else {
                return false;
            }
        }

        @Override
        public boolean getOffHostServices(String packageName, IGsmaServiceCallbacks callbacks) {
            if (DBG) Log.d(TAG, "getOffHostServices() " + packageName);

            if ((sNfcAdapter != null)&&(sNfcAdapter.isEnabled())) {
                NfcQcomAdapter adapter = getNfcQcomAdapter (getApplicationContext());
                if (adapter == null) {
                    if (DBG) Log.d(TAG, "cannot get NfcQcomAdapter");
                } else {
                    sIGsmaServiceCallbacksMap.put(mContext, callbacks);
                    return (adapter.getOffHostServices(packageName, mAdapterCallbacks));
                }
            }

            if (DBG) Log.d(TAG, "getOffHostServices() cannot get adapter or NFC Service is not enabled");

            try {
                callbacks.onGetOffHostService(true, null, null, 0, null, null);
            } catch (Exception e) {
                Log.e(TAG, "getOffHostServices() " + e.getMessage());
            }
            return false;
        }
    };

    @Override
    public void onCreate() {
        Log.d(TAG,"service created");

        mAdapterCallbacks = new AdapterCallbacks();

        mContext = getApplicationContext();
        if (sNfcAdapter == null)
            sNfcAdapter = NfcAdapter.getDefaultAdapter(mContext);

        Log.d(TAG,"NfcAdapter acquired");
        new Thread(){
            public void run() {
                for(int tries =0; tries<3; tries++) {
                    try {
                        NfcQcomAdapter nfcQcomAdapter = sNfcQcomAdapterMap.get(mContext);
                        if (nfcQcomAdapter == null) {
                            nfcQcomAdapter = NfcQcomAdapter.getNfcQcomAdapter(mContext);
                            sNfcQcomAdapterMap.put(mContext, nfcQcomAdapter);
                        }
                        Log.d(TAG,"NfcQcomAdapter acquired");
                        return;
                    } catch (UnsupportedOperationException e) {
                        String errorMsg = "GSMA service gracefully failing to acquire NfcQcomAdapter at boot. try" + tries;
                        Log.e(TAG, errorMsg);
                        new Throwable(TAG + ": " + errorMsg, e);
                        e.printStackTrace();
                    }
                    try {
                        if(DBG) Log.d(TAG,"Waiting for QcomAdapter");

                        wait(5000);
                    } catch (Exception e) {
                        if(DBG) Log.d(TAG,"Interupted while waiting for QcomAdapter. by" + e);
                    }
                }
            }
        }.start();
    }

    @Override
    public IBinder onBind(Intent intent) {
        if (IGsmaService.class.getName().equals(intent.getAction())) {
            return mGsmaServiceBinder;
        }
    return null;
    }
}
