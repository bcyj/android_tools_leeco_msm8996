/*
 *Copyright (c) 2014 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.internal.telephony;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import com.android.internal.telephony.Phone;
import com.qualcomm.qti.ims.intermediate.ICallInterface;
import com.qualcomm.qti.ims.intermediate.ICallInterfaceListener;

public class ImsCallProxy implements ICallInterface {

    private static String LOG_TAG = "ImsCallProxy";
    private Context mContext;
    private static ImsCallProxy mInstance;
    private IImsCallServiceListener mVTListener = new IImsCallServiceListener.Stub() {

        @Override
        public void imsRegStateChanged(int regstate) throws RemoteException {
            Log.d(LOG_TAG, "imsRegStateChanged  regstate=" + regstate);
            mListener.imsRegStateChanged(regstate);
        }

        @Override
        public void imsRegStateChangeReqFailed() throws RemoteException {
            Log.d(LOG_TAG, "imsRegStateChangeReqFailed");
            mListener.imsRegStateChangeReqFailed();
        }

        @Override
        public void onNewRingingConnection(String address, int callType)
                throws RemoteException {
            Log.d(LOG_TAG, "onNewRingingConnection  address=" + address
                    + ";callType=" + callType);
            mListener.onNewRingingConnection(address, callType);
        }

        @Override
        public void onPhoneStateChanged(int state) throws RemoteException {
            Log.d(LOG_TAG, "onPhoneStateChanged  state=" + state);
            mListener.onPhoneStateChanged(state);
        }

        @Override
        public void onDisconnect(int status) throws RemoteException {
            Log.d(LOG_TAG, "onDisconnect  status=" + status);
            mListener.onDisconnect(status);
        }

        @Override
        public void onAvpUpgradeFailure(String error) {
            Log.d(LOG_TAG, "onAvpUpgradeFailure  error=" + error);
            mListener.onAvpUpgradeFailure(error);

        }

        @Override
        public void onRemoteModifyCallRequest(int callType)
                throws RemoteException {
            Log.d(LOG_TAG, "onRemoteModifyCallRequest  callType=" + callType);
            mListener.onRemoteModifyCallRequest(callType);
        }

        @Override
        public void onModifyCallDone(boolean success, String result)
                throws RemoteException {
            Log.d(LOG_TAG, "onModifyCallDone  success=" + success);
            mListener.onModifyCallDone(success, result);
        }
    };
    private IImsCallService mImsCallService;

    private ICallInterfaceListener mListener;

    private ImsCallProxy(Context context) {
        mContext = context;
    }

    public static ImsCallProxy getInstance(Context context) {
        if (mInstance == null) {
            mInstance = new ImsCallProxy(context);
        }
        return mInstance;
    }

    public void createSdkService() {
        try {
            // send intent to start ims service n get phone from ims service
            boolean bound = mContext.bindService(new Intent(
                    "com.qualcomm.qti.ims.internal.telephony.imscallservice"),
                    ImsCallServiceConnection, Context.BIND_AUTO_CREATE);
            Log.d(LOG_TAG, "IMSService bound request : " + bound);
        } catch (NoClassDefFoundError e) {
            Log.w(LOG_TAG, "Ignoring IMS class not found exception " + e);
        }
    }

    private ServiceConnection ImsCallServiceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder service) {
            // Get handle to IImsService.Stub.asInterface(service);
            mImsCallService = IImsCallService.Stub.asInterface(service);
            Log.d(LOG_TAG, "Ims SDK Service Connected" + mImsCallService);
            try {
                mImsCallService.registerCallback(mVTListener);
            } catch (Exception e) {

            }
        }

        public void onServiceDisconnected(ComponentName arg0) {
            Log.w(LOG_TAG, "Ims SDK Service onServiceDisconnected");
        }
    };

    @Override
    public int dial(String number, int callType) {
        try {
            return mImsCallService.dial(number, callType);
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
            return 2;
        }
    }

    @Override
    public boolean acceptCall(int callType) {
        try {
            return mImsCallService.acceptCall(callType);
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
            return false;
        }
    }

    @Override
    public boolean hangup() {
        try {
            return mImsCallService.hangup();
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
            return false;
        }
    }

    @Override
    public void registerCallback(ICallInterfaceListener listener) {
        mListener = listener;
    }

    @Override
    public int getActiveCallType() {
        try {
            return mImsCallService.getActiveCallType();
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
            return -1;
        }
    }

    @Override
    public void setRegistrationState(int imsRegState) {
        try {
            mImsCallService.setRegistrationState(imsRegState);
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
        }
    }

    @Override
    public void changeConnectionType(int callType) {
        try {
            mImsCallService.changeConnectionType(callType);
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
        }
    }

    @Override
    public void acceptConnectionTypeChange(boolean accept) {
        try {
            mImsCallService.acceptConnectionTypeChange(accept);
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
        }
    }

    @Override
    public void addParticipant(String dialString, boolean isConference) {
        try {
            mImsCallService.addParticipant(dialString, 0,
                    Phone.CALL_TYPE_UNKNOWN,
                    new String[] { Phone.EXTRAS_IS_CONFERENCE_URI + "="
                            + Boolean.toString(isConference) });
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
        }
    }

    @Override
    public boolean isImsPhoneActive() {
        try {
            return mImsCallService.isImsPhoneActive();
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
            return false;
        }
    }

    @Override
    public boolean isImsPhoneIdle() {
        try {
            return mImsCallService.isImsPhoneIdle();
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
            return true;
        }
    }

    @Override
    public void mergeCalls() {
        try {
            mImsCallService.mergeCalls();
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
        }
    }

    @Override
    public void setMute(boolean muted) {
        try {
            mImsCallService.setMute(muted);
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
        }

    }

    @Override
    public void startDtmf(char c) {
        try {
            mImsCallService.startDtmf(c);
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
        }
    }

    @Override
    public void switchHoldingAndActive() {
        try {
            mImsCallService.switchHoldingAndActive();
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
        }
    }

    @Override
    public String[] getUriListConf() {
        try {
            return mImsCallService.getUriListinConf();
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
            return null;
        }
    }

    @Override
    public void hangupUri(String uri) {
        try {
            mImsCallService.hangupUri(uri);
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
        }
    }

    @Override
    public void endConference() {
        try {
            mImsCallService.endConference();
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
        }
    }

    @Override
    public int getRegistrationState() {
        try {
            return mImsCallService.getRegistrationState();
        } catch (RemoteException e) {
            Log.e(LOG_TAG, e.getMessage());
            return 2;
        }
    }

}
