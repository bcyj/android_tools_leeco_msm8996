/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
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

package com.android.qcom.nfc_extras;

import java.util.HashMap;
import java.util.List;

import android.content.Context;
import android.os.Binder;
import android.os.IBinder;
import android.os.Process;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.content.Intent;
import android.util.Log;
import java.util.List;

import android.nfc.NfcAdapter;
import qcom.nfc.IQNfcSecureElementManager;
import qcom.nfc.IQNfcSecureElementManagerCallbacks;

import android.content.ComponentName;
import android.content.ServiceConnection;
import android.os.Bundle;

public final class NfcQcomAdapter {
    private static final String TAG = "NfcQcomAdapter";

    static IQNfcSecureElementManager sService;
    static HashMap<Context, NfcQcomAdapter> sNfcQcomAdapterMap = new HashMap();
    static HashMap<Context, Callbacks> sCallbacksMap = new HashMap();
    final static boolean DBG = true;
    private String packageName;
    final Context mContext;

    NfcQcomAdapter(Context context) {
        if (DBG) Log.d(TAG, "NfcQcomAdapter()");
        mContext = context;
    }

    private static MyServiceConnection mConnection = new MyServiceConnection();

    private static class MyServiceConnection implements ServiceConnection {

        private boolean connected = false;
        private Object lock = new Object();

        @Override
        public void onServiceConnected(ComponentName name, IBinder binder) {
            connected = true;

            if (DBG) Log.d(TAG, "onServiceConnected" );
            synchronized (lock) {
                sService = IQNfcSecureElementManager.Stub.asInterface(binder);

                lock.notifyAll();
                if (DBG) Log.d(TAG, "Notifying All" );
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            connected = false;
            sService = null;
        }

        public void waitUntilConnected() throws InterruptedException {
            if (!connected) {
                if (DBG) Log.d(TAG, "waiting untill connected");
                synchronized (lock) {
                    lock.wait();
                }
            }
            if (DBG) Log.d(TAG, "connected");
        }
    }

    private static void getServiceInterface(Context context) {
        if (DBG) Log.d(TAG, "getServiceInterface()");
        Intent intent = new Intent(IQNfcSecureElementManager.class.getName());
        intent.setClassName ("com.android.nfc", "com.android.nfc.SecureElementService");
        context.bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
        try {
            mConnection.waitUntilConnected();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public static synchronized NfcQcomAdapter getNfcQcomAdapter(Context context) {
        if (DBG) Log.d(TAG, "getNfcQcomAdapter()");
        if (context == null) {
            throw new IllegalArgumentException("context cannot be null");
        }
        if (sService == null) {
            getServiceInterface(context);
            if (sService == null) {
                Log.e(TAG, "could not retrieve NFC service");
                throw new UnsupportedOperationException();
            }
        }
        NfcQcomAdapter nfcQcomAdapter = sNfcQcomAdapterMap.get(context);
        if (nfcQcomAdapter == null) {
            nfcQcomAdapter = new NfcQcomAdapter(context);
            sNfcQcomAdapterMap.put(context, nfcQcomAdapter);
        }
        return nfcQcomAdapter;
    }

    /**
     * NFC service dead - attempt best effort recovery
     */
    private void attemptDeadServiceRecovery(Exception e) {
        Log.e(TAG, "NFC Qcom Adapter Service is dead - attempting to recover");

        NfcAdapter nfcAdapter = NfcAdapter.getNfcAdapter(mContext);
        if (nfcAdapter != null)
            nfcAdapter.attemptDeadServiceRecovery(e);

        getServiceInterface(mContext);
        if (sService == null) {
            Log.e(TAG, "could not retrieve NFC service during service recovery");
            // nothing more can be done now, sService is still stale, we'll hit
            // this recovery path again later
            return;
        }
    }

    public Bundle open(IBinder b) throws RemoteException {
        if (DBG) Log.d(TAG, "enableNfcController()");
        try {
            if (sService != null) {
                return sService.open("", b);
            } else {
                throw new RemoteException("Service not initialized");
            }
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
            throw new RemoteException("Service not initialized");
        }
    }
    public Bundle close(IBinder b) throws RemoteException {
        if (DBG) Log.d(TAG, "close NfcController()");
        try {
            if (sService != null) {
                return sService.close("", b);
            } else {
                throw new RemoteException("Service can not be closed");
            }
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
            throw new RemoteException("Service can not be closed");
        }
    }
    public Bundle transceive(byte[] b) throws RemoteException {
        if (DBG) Log.d(TAG, "transceive NfcController()");
        try {
            if (sService != null) {
                return sService.transceive(null,b);
            } else {
                throw new RemoteException("Service can not be transceive");
            }
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
            throw new RemoteException("Service can not be transceive");
        }
    }

    public byte[] getLMRT() throws RemoteException {
        if (DBG) Log.d(TAG, "getLMRT to be sent to controller");
        try {
            if (sService != null) {
                return sService.getLMRT(mContext.getPackageName());
            } else {
                throw new RemoteException("Service can not get LMRT");
            }
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
            throw new RemoteException("Service can not get LMRT");
        }
    }

    public boolean multiSeRegisterAid(List<String> aid, ComponentName paymentService,
                                      List<String> seName, List<String> priority,
                                      List<String> powerState) throws RemoteException {

        if (DBG) Log.d(TAG, "multiSeRegisterAid NfcController()");
        try {
            if (sService != null) {
                return sService.multiSeRegisterAid(aid, paymentService,
                                                   seName, priority, powerState);
            } else {
                throw new RemoteException("Service can not be multiSeRegisterAid ");
            }
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
            throw new RemoteException("Service can not be mseTransmit");
        }
    }
    public boolean enableNfcController() {
        if (DBG) Log.d(TAG, "enableNfcController()");
        try {
            if (sService != null) {
                sService.enableNfcController(mContext.getPackageName());
                return true;
            } else {
                return false;
            }
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
            return false;
        }
    }

    public boolean isCardEmulationEnabled() {
        if (DBG) Log.d(TAG, "isCardEmulationEnabled()");
        try {
            if (sService != null) {
                return sService.isCardEmulationEnabled(mContext.getPackageName());
            } else {
                return false;
            }
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
            return false;
        }
    }

    public boolean updateCardEmulationRoute(int route) {
        if (DBG) Log.d(TAG, "updateCardEmulationRoute() : route = " + route);
        try {
            if (sService != null) {
                return sService.updateCardEmulationRoute(mContext.getPackageName(), route);
            } else {
                return false;
            }
        } catch (SecurityException e) {
            throw new SecurityException("application is not allowed");
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
            return false;
        }
    }

    public boolean enableCardEmulationMode() {
        if (DBG) Log.d(TAG, "enableCardEmulationMode()");
        try {
            if (sService != null) {
                return sService.enableCardEmulationMode(mContext.getPackageName());
            } else {
                return false;
            }
        } catch (SecurityException e) {
            throw new SecurityException("application is not allowed");
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
            return false;
        }
    }

    public boolean disableCardEmulationMode() {
        if (DBG) Log.d(TAG, "disableCardEmulationMode()");
        try {
            if (sService != null) {
                return sService.disableCardEmulationMode(mContext.getPackageName());
            } else {
                return false;
            }
        } catch (SecurityException e) {
            throw new SecurityException("application is not allowed");
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
            return false;
        }
    }

    public boolean isSeEnabled(String seName) {
        if (DBG) Log.d(TAG, "isSeEnabled() " + seName);
        try {
            if (sService != null)
                return sService.isSeEnabled(mContext.getPackageName(), seName);
            else
                return false;
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
            return false;
        }
    }

    public void deliverSeIntent(Intent seIntent) {
        //if (DBG) Log.d(TAG, "deliverSeIntent()");
        if (Binder.getCallingUid() != Process.NFC_UID)
            throw new SecurityException("Only SmartcardService may use deliverSeIntent()");

        try {
            if (sService != null)
                sService.deliverSeIntent(mContext.getPackageName(), seIntent);
            else
                return;
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
        }
    }

    public void notifyCheckCertResult(boolean success) {
        if (DBG) Log.d(TAG, "notifyCheckCertResult()");
        if (Binder.getCallingUid() != Process.NFC_UID)
            throw new SecurityException("Only SmartcardService may use notifyCheckCertResult()");

        try {
            if (sService != null)
                sService.notifyCheckCertResult(mContext.getPackageName(), success);
            else
                return;
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
        }
    }

    public void selectSEToOpenApduGate(String seName) {
        if (DBG) Log.d(TAG, "selectSEToOpenApduGate() " + seName);

        try {
            if (sService != null)
                sService.selectSEToOpenApduGate(mContext.getPackageName(), seName);
            else
                return;
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
        }
    }

    public String getActiveSecureElement() {
        if (DBG) Log.d(TAG, "getActiveSecureElement()");

        try {
            if (sService != null)
                return sService.getActiveSecureElement(mContext.getPackageName());
            else
                return null;
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
            return null;
        }
    }

    public void GsmaPack (String packname){
        Log.d(TAG, "GsmaPack " + packname);
        packageName =  packname ;
    }

    public void setActiveSecureElement(String SEName) {
        if (DBG) Log.d(TAG, "setActiveSecureElement() " + SEName);

        try {
            if (sService != null)
                sService.setActiveSecureElement(mContext.getPackageName(), SEName);
            else
                return;
        } catch (SecurityException e) {
            throw new SecurityException("application is not allowed");
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
        }
    }

    public void enableMultiReception(String SEName) {
        if (DBG) Log.d(TAG, "enableMultiReception() " + SEName);

        try {
            if (sService != null) {
                if (mContext.getPackageName().equals("com.qcom.gsma.services.nfc") ){
                    sService.enableMultiReception(packageName, SEName);
                } else {
                    sService.enableMultiReception(mContext.getPackageName(), SEName);
                }
           } else
                return;
        } catch (SecurityException e) {
            throw new SecurityException("application is not allowed");
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
        }
    }

    public boolean commitOffHostService(String packageName, String seName, String description,
                                        int bannerResId, int uid, List<String> aidGroupDescriptions,
                                        List<android.nfc.cardemulation.AidGroup> aidGroups) {
        if (DBG) Log.d(TAG, "commitOffHostService()");

        try {
            if (sService != null) {
                sService.commitOffHostService(packageName, seName, description,
                                              bannerResId, uid, aidGroupDescriptions, aidGroups);
                return true;
            } else {
                return false;
            }
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
            return false;
        }
    }

    public boolean deleteOffHostService(String packageName, String seName) {
        if (DBG) Log.d(TAG, "deleteOffHostService()");

        try {
            if (sService != null) {
                sService.deleteOffHostService(packageName, seName);
                return true;
            } else {
                return false;
            }
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
            return false;
        }
    }

    public interface Callbacks {
        public abstract void onGetOffHostService (boolean isLast, String description, String seName, int bannerResId,
                                                  List<String> dynamicAidGroupDescriptions,
                                                  List<android.nfc.cardemulation.AidGroup> dynamicAidGroups);
    }

    private IQNfcSecureElementManagerCallbacks mIQNfcSecureElementManagerCallbacks = new IQNfcSecureElementManagerCallbacks.Stub() {
        public void onGetOffHostService (boolean isLast, String description, String seName, int bannerResId,
                                         List<String> dynamicAidGroupDescriptions,
                                         List<android.nfc.cardemulation.AidGroup> dynamicAidGroups) {
            if (DBG) Log.d(TAG, "onGetOffHostService() " + isLast + ", " + description + ", " + seName);
            Callbacks callbacks = sCallbacksMap.get(mContext);
            if (callbacks != null) {
                callbacks.onGetOffHostService (isLast, description, seName, bannerResId,
                                               dynamicAidGroupDescriptions, dynamicAidGroups);
            }
        }
    };

    public boolean getOffHostServices(String packageName, Callbacks callbacks) {
        if (DBG) Log.d(TAG, "getOffHostServices()");

        try {
            if (sService != null) {
                sCallbacksMap.put(mContext, callbacks);
                sService.getOffHostServices (packageName, mIQNfcSecureElementManagerCallbacks);
                return true;
            } else {
                return false;
            }
        } catch (RemoteException e) {
            attemptDeadServiceRecovery(e);
            return false;
        }
    }
}


