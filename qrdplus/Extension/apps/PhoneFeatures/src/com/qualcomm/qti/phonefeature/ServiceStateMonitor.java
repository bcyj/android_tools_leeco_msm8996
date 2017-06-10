/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.phonefeature;

import android.os.Handler;
import android.os.Registrant;
import android.os.RegistrantList;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

public class ServiceStateMonitor extends Handler {

    private static final String TAG = "ServiceStateMonitor";

    private static class ServiceStateInfo {

        private int state;
        private Boolean roaming;

        private static ServiceStateInfo from(ServiceState serviceState) {
            ServiceStateInfo service = new ServiceStateInfo();
            service.state = serviceState.getState();
            if (serviceState.getState() == ServiceState.STATE_IN_SERVICE) {
                service.roaming = serviceState.getRoaming();
            }
            return service;
        }

        private boolean updateState(ServiceState serviceState) {
            if (state != serviceState.getState()) {
                state = serviceState.getState();
                return true;
            }
            return false;
        }

        private boolean updateRoaming(ServiceState serviceState) {
            // roaming state is only available when service state is IN_SERVICE
            if (serviceState.getState() != ServiceState.STATE_IN_SERVICE) {
                return false;
            }
            if (roaming == null || serviceState.getRoaming() != roaming) {
                roaming = serviceState.getRoaming();
                return true;
            }
            return false;
        }

        public String toString() {
            return "[ServiceState] state=" + state + ", roaming=" + roaming;
        }
    }

    private ServiceStateInfo[] mServiceStates = new ServiceStateInfo[Constants.PHONE_COUNT];
    private PhoneStateListener[] mPhoneStateListeners =
            new PhoneStateListener[Constants.PHONE_COUNT];

    private class StateListener extends PhoneStateListener {

        private final int mSlot;

        public StateListener(int slot, int subId) {
            super(subId);
            mSlot = slot;
        }

        public StateListener(int slot) {
            mSlot = slot;
        }

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            super.onServiceStateChanged(serviceState);
            boolean serviceChanged = false;
            if (mServiceStates[mSlot].updateState(serviceState)) {
                serviceChanged = true;
                mServiceStateRegistrants.notifyResult(mSlot);
            }
            if (mServiceStates[mSlot].updateRoaming(serviceState)) {
                serviceChanged = true;
                mRoamingStateRegistrants.notifyResult(mSlot);
            }
            if (serviceChanged) {
                Log.d(TAG, "slot" + mSlot + " changed, " + mServiceStates[mSlot]);
            }
        }
    }

    private RegistrantList mRoamingStateRegistrants = new RegistrantList();
    private RegistrantList mServiceStateRegistrants = new RegistrantList();

    public void registerRoamingStateChanged(Handler handler, int what, Object obj) {
        Registrant r = new Registrant(handler, what, obj);
        synchronized (mRoamingStateRegistrants) {
            mRoamingStateRegistrants.add(r);
        }
        for (int index = 0; index < Constants.PHONE_COUNT; index++) {
            if (mServiceStates[index].roaming != null) {
                r.notifyResult(index);
            }
        }
    }

    public void registerServiceStateChanged(Handler handler, int what, Object obj) {
        Registrant r = new Registrant(handler, what, obj);
        synchronized (mServiceStateRegistrants) {
            mServiceStateRegistrants.add(r);
        }
        for (int index = 0; index < Constants.PHONE_COUNT; index++) {
            r.notifyResult(index);
        }
    }

    public void unregisterRoamingStateChanged(Handler handler) {
        synchronized (mRoamingStateRegistrants) {
            mRoamingStateRegistrants.remove(handler);
        }
    }

    public void unregisterServiceStateChanged(Handler handler) {
        synchronized (mServiceStateRegistrants) {
            mServiceStateRegistrants.remove(handler);
        }
    }

    public ServiceStateMonitor() {
        for (int index = 0; index < Constants.PHONE_COUNT; index++) {
            mServiceStates[index] = ServiceStateInfo.from(AppGlobals.getInstance().mPhones[index]
                    .getServiceState());
            int[] subId = SubscriptionManager.getSubId(index);
            if (subId != null) {
                mPhoneStateListeners[index] = new StateListener(index, subId[0]);
            } else {
                mPhoneStateListeners[index] = new StateListener(index);
            }
            TelephonyManager.getDefault().listen(mPhoneStateListeners[index],
                    PhoneStateListener.LISTEN_SERVICE_STATE);
        }
    }

    public boolean isRoaming(int sub) {
        return mServiceStates[sub] != null && mServiceStates[sub].roaming != null
                && mServiceStates[sub].roaming;
    }

    public void dispose() {
        for (int index = 0; index < Constants.PHONE_COUNT; index++) {
            TelephonyManager.getDefault().listen(mPhoneStateListeners[index],
                    PhoneStateListener.LISTEN_NONE);
        }
    }
}
