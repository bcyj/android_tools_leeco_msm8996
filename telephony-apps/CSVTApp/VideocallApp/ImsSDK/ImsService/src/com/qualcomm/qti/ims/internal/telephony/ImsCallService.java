/*
 *Copyright (c) 2014 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.internal.telephony;

import java.util.Map;

import org.codeaurora.ims.IImsService;

import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.util.Log;

import com.android.internal.telephony.CallManager;
import com.android.services.telephony.common.Call;

public class ImsCallService extends Service implements IImsNotifier {

    private static final String LOG_TAG = "ImsCallService";

    private CallManager mCM;
    public IImsService mImsService;

    final private RemoteCallbackList<IImsCallServiceListener> mListeners = new RemoteCallbackList<IImsCallServiceListener>();

    private int confconnectId = -1;

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        mCM = CallManager.getInstance();
        PhoneGlobals pg = new PhoneGlobals(this);
        pg.onCreate();
        createImsService();
        return START_STICKY;
    }

    public void createImsService() {
        if (CallManager.isCallOnImsEnabled()) {
            try {
                // send intent to start ims service n get phone from ims service
                boolean bound = bindService(new Intent(
                        "org.codeaurora.ims.IImsService"),
                        ImsServiceConnection, Context.BIND_AUTO_CREATE);
                Log.d(LOG_TAG, "IMSService bound request : " + bound);
            } catch (NoClassDefFoundError e) {
                Log.w(LOG_TAG, "Ignoring IMS class not found exception " + e);
            }
        }
    }

    private ServiceConnection ImsServiceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder service) {
            // Get handle to IImsService.Stub.asInterface(service);
            mImsService = IImsService.Stub.asInterface(service);
            Log.d(LOG_TAG, "Ims Service Connected" + mImsService);
            PhoneGlobals.getInstance().setImsService(mImsService);
        }

        public void onServiceDisconnected(ComponentName arg0) {
            Log.w(LOG_TAG, "Ims Service onServiceDisconnected");
        }
    };

    /*
     * (non-Javadoc)
     *
     * @see
     * com.qualcomm.qti.ims.internal.telephony.IImsNotifier#notifyImsRegStateChanged
     * (int)
     */
    @Override
    public void notifyImsRegStateChanged(final int regstate) {
        notifyListeners(new INotifyEvent() {
            @Override
            public void onNotify(IImsCallServiceListener l)
                    throws RemoteException {
                l.imsRegStateChanged(regstate);
            }
        });
    }

    /*
     * (non-Javadoc)
     *
     * @see com.qualcomm.qti.ims.internal.telephony.IImsNotifier#
     * notifyImsRegStateChangeReqFailed()
     */
    @Override
    public void notifyImsRegStateChangeReqFailed() {
        notifyListeners(new INotifyEvent() {
            @Override
            public void onNotify(IImsCallServiceListener l)
                    throws RemoteException {
                l.imsRegStateChangeReqFailed();
            }
        });
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.qualcomm.qti.ims.internal.telephony.IImsNotifier#notifyNewRingingConnection
     * (java.lang.String, int)
     */
    @Override
    public void notifyNewRingingConnection(final String address,
            final int callType) {
        notifyListeners(new INotifyEvent() {
            @Override
            public void onNotify(IImsCallServiceListener l)
                    throws RemoteException {
                l.onNewRingingConnection(address, callType);
            }
        });
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.qualcomm.qti.ims.internal.telephony.IImsNotifier#notifyImsPhoneStateChanged
     * (int)
     */
    @Override
    public void notifyImsPhoneStateChanged(final int state) {
        notifyListeners(new INotifyEvent() {
            @Override
            public void onNotify(IImsCallServiceListener l)
                    throws RemoteException {
                if (l != null) {
                    l.onPhoneStateChanged(state);
                } else {
                    Log.d(LOG_TAG, "onNotify listener=null");
                }
            }
        });
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.qualcomm.qti.ims.internal.telephony.IImsNotifier#notifyImsDisconnect(int)
     */
    @Override
    public void notifyImsDisconnect(final int state) {
        notifyListeners(new INotifyEvent() {
            @Override
            public void onNotify(IImsCallServiceListener l)
                    throws RemoteException {
                l.onDisconnect(state);
            }
        });
    }

    /*
     * (non-Javadoc)
     *
     * @see com.qualcomm.qti.ims.internal.telephony.IImsNotifier#
     * notifyImsRemoteModifyCallRequest(int)
     */
    @Override
    public void notifyImsRemoteModifyCallRequest(final int callType) {
        notifyListeners(new INotifyEvent() {
            @Override
            public void onNotify(IImsCallServiceListener l)
                    throws RemoteException {
                l.onRemoteModifyCallRequest(callType);
            }
        });
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.qualcomm.qti.ims.internal.telephony.IImsNotifier#notifyImsModifyCallDon
     * (boolean, java.lang.String)
     */
    @Override
    public void notifyImsModifyCallDone(final boolean success,
            final String result) {
        notifyListeners(new INotifyEvent() {
            @Override
            public void onNotify(IImsCallServiceListener l)
                    throws RemoteException {
                l.onModifyCallDone(success, result);
            }
        });
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.qualcomm.qti.ims.internal.telephony.IImsNotifier#notifyImsAvpUpgradeFailure
     * (java.lang.String)
     */
    @Override
    public void notifyImsAvpUpgradeFailure(final String error) {
        notifyListeners(new INotifyEvent() {
            @Override
            public void onNotify(IImsCallServiceListener l)
                    throws RemoteException {
                l.onAvpUpgradeFailure(error);
            }
        });
    }

    private interface INotifyEvent {
        void onNotify(IImsCallServiceListener l) throws RemoteException;
    }

    void notifyListeners(INotifyEvent n) {
        synchronized (this) {
            mListeners.beginBroadcast();
            final int size = mListeners.getRegisteredCallbackCount();
            for (int i = 0; i < size; ++i) {
                try {
                    n.onNotify(mListeners.getBroadcastItem(i));
                } catch (RemoteException e) {
                    Log.e(LOG_TAG, "Broadcast failed. idx: " + i + " Ex: " + e);
                }
            }
            mListeners.finishBroadcast();
        }
    }

    @Override
    public IBinder onBind(Intent arg0) {
        mCM = CallManager.getInstance();
        PhoneGlobals pg = new PhoneGlobals(this);
        pg.onCreate();
        pg.setIImsNotifier(this);
        createImsService();

        return mBinder;
    }

    private final IImsCallService.Stub mBinder = new IImsCallService.Stub() {

        @Override
        public int dial(String number, int callType) throws RemoteException {
            return PhoneUtils.placeImsCall(ImsCallService.this, number,
                    callType);

        }

        @Override
        public void registerCallback(IImsCallServiceListener imsServListener)
                throws RemoteException {
            if (imsServListener == null) {
                Log.e(LOG_TAG, "Listener registration failed. Listener is null");
                return;
            }
            synchronized (this) {
                mListeners.register(imsServListener);
            }
        }

        @Override
        public void unregisterCallback(IImsCallServiceListener imsServListener)
                throws RemoteException {
            if (imsServListener == null) {
                Log.e(LOG_TAG,
                        "Listener unregistration failed. Listener is null");
                return;
            }
            synchronized (this) {
                mListeners.unregister(imsServListener);
            }
        }

        @Override
        public void setRegistrationState(int imsRegState)
                throws RemoteException {
            mImsService.setRegistrationState(imsRegState);

        }

        @Override
        public int getRegistrationState() throws RemoteException {
            // TODO Auto-generated method stub
            return mImsService.getRegistrationState();
        }

        @Override
        public void hangupUri(String userUri) throws RemoteException {
            PhoneUtils.hangupWithReason(-1, userUri, true,
                    Call.DisconnectCause.NORMAL.ordinal(), "", mCM);

        }

        @Override
        public void hangupWithReason(int connectionId, String userUri,
                String confUri, boolean mpty, int failCause, String errorInfo)
                throws RemoteException {
            PhoneUtils.hangupWithReason(connectionId, userUri, mpty, failCause,
                    errorInfo, mCM);
        }

        @Override
        public String[] getCallDetailsExtrasinCall(int callId)
                throws RemoteException {
            return PhoneUtils.getCallDetailsExtrasinCall(callId, mCM);
        }

        @Override
        public String getImsDisconnectCauseInfo(int callId)
                throws RemoteException {
            // TODO Auto-generated method stub
            return PhoneUtils.getImsDisconnectCauseInfo(callId);
        }

        @Override
        public String[] getUriListinConf() throws RemoteException {
            return PhoneUtils.getUriListinConf();
        }

        @Override
        public boolean isVTModifyAllowed() throws RemoteException {
            return PhoneUtils.isVTModifyAllowed();
        }

        @Override
        public boolean getProposedConnectionFailed(int connIndex)
                throws RemoteException {
            return PhoneUtils.getProposedConnectionFailed(connIndex);
        }

        @Override
        public boolean isAddParticipantAllowed() throws RemoteException {
            return PhoneUtils.isAddParticipantAllowed();
        }

        @Override
        public void addParticipant(String dialString, int clir, int callType,
                String[] extra) throws RemoteException {
            PhoneUtils.addParticipant(dialString, clir, callType, extra);
        }

        @Override
        public boolean acceptCall(int callType) throws RemoteException {
            PhoneUtils.answerCall(callType);
            return false;
        }

        @Override
        public void changeConnectionType(int callType) throws RemoteException {
            PhoneGlobals.getInstance().changeConnectionType(callType);
        }

        @Override
        public boolean startDtmf(char c) throws RemoteException {
            return mCM.startDtmf(c);
        }

        @Override
        public void acceptConnectionTypeChange(boolean accept)
                throws RemoteException {
            PhoneGlobals.getInstance().acceptConnectionType(accept);

        }

        @Override
        public int getActiveCallType() throws RemoteException {
            try {
                return PhoneUtils.getImsPhone(mCM).getCallType(
                        mCM.getActiveFgCall());
            } catch (Exception e) {
                Log.e(LOG_TAG, e.getMessage());
                return -1;
            }
        }

        @Override
        public boolean hangup() throws RemoteException {
            return PhoneUtils.hangup(mCM);
        }

        @Override
        public void mergeCalls() throws RemoteException {
            PhoneUtils.mergeCalls();
        }

        @Override
        public boolean isImsPhoneActive() throws RemoteException {
            return mCM.isImsPhoneActive();
        }

        @Override
        public boolean isImsPhoneIdle() throws RemoteException {
            return PhoneUtils.isImsPhoneIdle();
        }

        @Override
        public void setMute(boolean muteState) {
            PhoneUtils.setMute(muteState);
        }

        @Override
        public void endConference() {
            PhoneUtils.endConference();
        }

        @Override
        public void switchHoldingAndActive() {
            PhoneUtils.switchHoldingAndActive(mCM.getFirstActiveBgCall());
        }
    };
}
