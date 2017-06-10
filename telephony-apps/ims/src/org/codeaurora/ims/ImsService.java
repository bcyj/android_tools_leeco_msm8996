/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package org.codeaurora.ims;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.ims.ImsReasonInfo;
import com.android.ims.ImsServiceClass;
import com.android.ims.ImsCallProfile;
import com.android.ims.internal.IImsCallSession;
import com.android.ims.internal.IImsCallSessionListener;
import com.android.ims.internal.IImsRegistrationListener;
import com.android.ims.internal.IImsEcbm;
import com.android.ims.internal.IImsService;
import com.android.ims.internal.IImsUt;
import com.android.ims.internal.IImsConfig;
import com.android.ims.internal.ImsCallSession;
import com.android.internal.telephony.ModemStackController;
import com.android.internal.telephony.ModemStackController.ModemCapabilityInfo;
import com.qualcomm.ims.vt.ImsVideoGlobals;

import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.ServiceManager;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.util.Log;

public class ImsService extends Service {
    private static final String LOG_TAG = "ImsService";
    private static final int MAX_SUBSCRIPTIONS = 1;
    private ImsServiceSub mServiceSub[];
    // service id --> table
    private Map<Integer, ImsServiceSub> mServiceSubMap = new HashMap<Integer, ImsServiceSub>();

    private Handler mHandler;
    private ModemStackController mModemStackController;
    protected static final int EVENT_MODEM_STACK_READY = 1;
    protected int mImsPhoneId = 0;

    /**
     * Utility for getting number of subscriptions
     * @return int containing maximum number
     */
    private int getNumSubscriptions() {
        return MAX_SUBSCRIPTIONS; //One for now - plugin with msim util later
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d (LOG_TAG, "ImsService created!");
        mServiceSub = new ImsServiceSub[getNumSubscriptions()];
        for (int i = 0; i < getNumSubscriptions(); i++) {
            mServiceSub[i] = new ImsServiceSub(i + 1, this);
        }
        ServiceManager.addService("ims", mBinder);
        Intent intent = new Intent(ImsManager.ACTION_IMS_SERVICE_UP);
        intent.addFlags(Intent.FLAG_RECEIVER_REPLACE_PENDING);
        if (TelephonyManager.getDefault().isMultiSimEnabled()) {
            intent.putExtra(ImsManager.EXTRA_PHONEID, mImsPhoneId);
            this.sendStickyBroadcast(intent);
        } else {
            this.sendStickyBroadcast(intent);
        }
        final int defaultSub = 1;
        ImsVideoGlobals.init(mServiceSub[defaultSub-1], this);

        initSubscriptionStatus();
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(LOG_TAG, "Returning mBinder for ImsService binding.");
        return mBinder;
    }

    @Override
    public void onDestroy() {
        Log.d(LOG_TAG, "Ims Service Destroyed Successfully...");
        super.onDestroy();
    }

    private void createHandler() {
        mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                ImsService.this.handleMessage(msg);
            }
        };
    }

    void handleMessage(Message msg) {
        switch ( msg.what ) {
            case EVENT_MODEM_STACK_READY:
                Log.d(LOG_TAG, "Received event: EVENT_MODEM_STACK_READY");
                handleModemStackReady(msg);
                break;
            default:
                Log.d (LOG_TAG, "Unknown msg!");
                break;
        }
    }

    /* Method to initialize the Phone Id */
    private void initSubscriptionStatus() {
        if (TelephonyManager.getDefault().isMultiSimEnabled()) {
            /* Initialize the handler */
            createHandler();
            /* Initialize the modemStackController */
            mModemStackController = ModemStackController.getInstance();
            mModemStackController.registerForStackReady(mHandler, EVENT_MODEM_STACK_READY, null);
            Log.d(LOG_TAG, "initSubscriptionStatus: registered for EVENT_MODEM_STACK_READY");
        } else {
            mModemStackController = null;
            mHandler = null;
            Log.d(LOG_TAG, "initSubscriptionStatus: Not multi-sim...");
        }
    }

    /**
     * Method to identify the RAT mask for Multimode support.
     * We are checking for LTE or UMTS support for IMS and CSVT.
     * If L or W is supported on one sub, the other sub will be
     * G only sub.
     */
    private boolean isMultiModeSupported(int nRatMask) {
        int nMmMask = ((1 << ServiceState.RIL_RADIO_TECHNOLOGY_LTE) |
                (1 << ServiceState.RIL_RADIO_TECHNOLOGY_UMTS));
        return ((nRatMask & nMmMask) == 0) ? false : true;
    }

    /* Method to handle phone id change event */
    private void handleModemStackReady(Message msg) {
        if (!mModemStackController.isStackReady()) {
            Log.d(LOG_TAG, "handleModemStackReady: Stack is Not Ready. EXIT!!!");
            return;
        }

        int mNumPhones = TelephonyManager.getDefault().getPhoneCount();
        boolean mPhoneChanged = false;
        Log.d(LOG_TAG, "handleModemStackReady: NumPhones:" + mNumPhones +
                " Ims PhoneID:" + mImsPhoneId);

        for (int i = 0; i < mNumPhones; i++) {
            ModemCapabilityInfo mModemCapInfo = mModemStackController.getModemRatCapsForPhoneId(i);
            Log.d(LOG_TAG, "handleModemStackReady: Phone:" + i + " Info:" + mModemCapInfo);
            // Since only 1 MM Modem is possible, checking Modem capabilities is enough.
            if (mModemCapInfo != null && isMultiModeSupported(mModemCapInfo.getSupportedRatBitMask()))
            {
                if (mImsPhoneId != i) {
                    Log.d(LOG_TAG, "handleModemStackReady: Phone:" + mImsPhoneId + " changed to " + i);
                    mImsPhoneId = i;
                    mPhoneChanged = true;
                } else {
                    Log.d(LOG_TAG, "handleModemStackReady: Phone:" + mImsPhoneId + " UNchanged");
                }
                break;
            }
        }

        /* Indicate the phone change information */
        if (mPhoneChanged) {
            /* Change the socket communication */
            final int defaultSub = 1;
            mServiceSub[defaultSub-1].registerForPhoneId(mImsPhoneId);

            /* Shut-down the existing ims phone */
            for (int i = 0; i < mNumPhones; i++) {
                if(mImsPhoneId != i) {
                    Intent intent = new Intent(ImsManager.ACTION_IMS_SERVICE_DOWN);
                    intent.putExtra(ImsManager.EXTRA_PHONEID, i);
                    this.sendBroadcast(new Intent(intent));
                }
            }

            /* Create the new IMS phone */
            Intent intent = new Intent(ImsManager.ACTION_IMS_SERVICE_UP);
            intent.addFlags(Intent.FLAG_RECEIVER_REPLACE_PENDING);
            intent.putExtra(ImsManager.EXTRA_PHONEID, mImsPhoneId);
            this.sendStickyBroadcast(new Intent(intent));
        }
    }

    /*
     * Implement the methods of the IImsService interface in this stub
     */
    private final IImsService.Stub mBinder = new IImsService.Stub() {

        /**
         * Opens the IMS service for making calls and/or receiving generic IMS calls.
         * The caller may make subsequent calls through {@link #makeCall}.
         * The IMS service will register the device to the operator's network with the credentials
         * (from ISIM) periodically in order to receive calls from the operator's network.
         * When the IMS service receives a new call, it will send out an intent with
         * the provided action string.
         * The intent contains a call ID extra {@link getCallId} and it can be used to take a call.
         *
         * @param serviceClass a service class specified in {@link ImsServiceClass}
         *      For VoLTE service, it MUST be a {@link ImsServiceClass#MMTEL}.
         * @param incomingCallPendingIntent When an incoming call is received,
         *        the IMS service will call {@link PendingIntent#send(Context, int, Intent)} to
         *        send back the intent to the caller with {@link #INCOMING_CALL_RESULT_CODE}
         *        as the result code and the intent to fill in the call ID; It cannot be null
         * @param listener To listen to IMS registration events; It cannot be null
         * @return identifier (greater than 0) for the specified service
         * @see #getCallId
         * @see #getServiceId
         */
        public int open(int phoneId, int serviceClass, PendingIntent incomingCallIntent,
                IImsRegistrationListener listener) {
           return openForSub(1, serviceClass, incomingCallIntent, listener);
        }

        /**
         * Opens the IMS service for making calls and/or receiving generic IMS calls.
         * The caller may make subsequent calls through {@link #makeCall}.
         * The IMS service will register the device to the operator's network with the credentials
         * (from ISIM) periodically in order to receive calls from the operator's network.
         * When the IMS service receives a new call, it will send out an intent with
         * the provided action string.
         * The intent contains a call ID extra {@link getCallId} and it can be used to take a call.
         *
         * @param subscription The SIM subscription for multi-sim scenarios
         * @param serviceClass a service class specified in {@link ImsServiceClass}
         *      For VoLTE service, it MUST be a {@link ImsServiceClass#MMTEL}.
         * @param incomingCallPendingIntent When an incoming call is received,
         *        the IMS service will call {@link PendingIntent#send(Context, int, Intent)} to
         *        send back the intent to the caller with {@link #INCOMING_CALL_RESULT_CODE}
         *        as the result code and the intent to fill in the call ID; It cannot be null
         * @param listener To listen to IMS registration events; It cannot be null
         * @return identifier (greater than 0) for the specified service
         * @see #getCallId
         * @see #getServiceId
         */
        public int openForSub(int subscription, int serviceClass, PendingIntent incomingCallIntent,
                IImsRegistrationListener listener) {
            //TODO: Check valid subscription using framework hooks for multi sim
            int serviceId = mServiceSub[subscription - 1].getServiceId(serviceClass,
                    incomingCallIntent,
                    listener);
            if (serviceId > 0) {
                mServiceSubMap.put(new Integer(serviceId), mServiceSub[subscription - 1]);
            }
            // TODO: (ims-vt) This needds to run on main thread.
            ImsVideoGlobals.getInstance().setActiveSub(mServiceSub[subscription - 1]);
            mServiceSub[subscription - 1].setPhoneId(mImsPhoneId);
            Log.d (LOG_TAG, "Open returns serviceId " + serviceId);
            return serviceId;
        }

        /**
         * Closes the specified service ({@link ImsServiceClass}) not to make/receive calls.
         * All the resources that were allocated to the service are also released.
         *
         * @param serviceId a service id to be closed which is obtained from {@link ImsManager#open}
         */
        public void close(int serviceId) {
            //TODO
        }

        /**
         * Checks if the IMS service has successfully registered to the IMS network
         * with the specified service & call type.
         *
         * @param serviceId a service id which is obtained from {@link ImsManager#open}
         * @param serviceType a service type that is specified in {@link ImsCallProfile}
         *        {@link ImsCallProfile#SERVICE_TYPE_NORMAL}
         *        {@link ImsCallProfile#SERVICE_TYPE_EMERGENCY}
         * @param callType a call type that is specified in {@link ImsCallProfile}
         *        {@link ImsCallProfile#CALL_TYPE_VOLTE_N_VIDEO}
         *        {@link ImsCallProfile#CALL_TYPE_VOLTE}
         *        {@link ImsCallProfile#CALL_TYPE_VT}
         *        {@link ImsCallProfile#CALL_TYPE_VS}
         * @return true if the specified service id is connected to the IMS network;
         *        false otherwise
         */
        public boolean isConnected(int serviceId, int serviceType, int callType) {
            return true; //TODO:
        }

        /**
         * Checks if the specified IMS service is opened.
         *
         * @param serviceId a service id which is obtained from {@link ImsManager#open}
         * @return true if the specified service id is opened; false otherwise
         */
        public boolean isOpened(int serviceId) {
            return true; //TODO:
        }

        /**
         * Set the registration listener for the client associated with service id
         * @param serviceId - service ID obtained through open
         * @param listener - registration listener
         */
        public void setRegistrationListener(int serviceId, IImsRegistrationListener listener) {
            ImsServiceSub service = mServiceSubMap.get(new Integer(serviceId));
            if (service == null) {
                Log.e (LOG_TAG, "Invalid ServiceId ");
                return;
            }
            service.setRegistrationListener(serviceId, listener);
        }

        /**
         * Creates a {@link ImsCallProfile} from the service capabilities & IMS registration state.
         *
         * @param serviceId a service id which is obtained from {@link ImsManager#open}
         * @param serviceType a service type that is specified in {@link ImsCallProfile}
         *        {@link ImsCallProfile#SERVICE_TYPE_NONE}
         *        {@link ImsCallProfile#SERVICE_TYPE_NORMAL}
         *        {@link ImsCallProfile#SERVICE_TYPE_EMERGENCY}
         * @param callType a call type that is specified in {@link ImsCallProfile}
         *        {@link ImsCallProfile#CALL_TYPE_VOLTE}
         *        {@link ImsCallProfile#CALL_TYPE_VT}
         *        {@link ImsCallProfile#CALL_TYPE_VT_TX}
         *        {@link ImsCallProfile#CALL_TYPE_VT_RX}
         *        {@link ImsCallProfile#CALL_TYPE_VT_NODIR}
         *        {@link ImsCallProfile#CALL_TYPE_VS}
         *        {@link ImsCallProfile#CALL_TYPE_VS_TX}
         *        {@link ImsCallProfile#CALL_TYPE_VS_RX}
         * @return a {@link ImsCallProfile} object
         */
        public ImsCallProfile createCallProfile(int serviceId, int serviceType, int callType) {
            ImsServiceSub service = mServiceSubMap.get(new Integer(serviceId));
            if (service == null) {
                Log.e (LOG_TAG, "Invalid ServiceId ");
                return null;
            }
            return service.createCallProfile(serviceId, serviceType, callType);
        }

        /**
         * Creates a {@link ImsCallSession} with the specified call profile.
         * Use other methods, if applicable, instead of interacting with
         * {@link ImsCallSession} directly.
         *
         * @param serviceId a service id which is obtained from {@link ImsManager#open}
         * @param profile a call profile to make the call
         */
        public IImsCallSession createCallSession(int serviceId, ImsCallProfile profile,
                IImsCallSessionListener listener) {
            ImsServiceSub service = mServiceSubMap.get(new Integer(serviceId));
            if (service == null) {
                Log.e (LOG_TAG, "Invalid ServiceId ");
                return null;
            }
            return service.createCallSession(serviceId, profile, listener);
        }

        /**
         * Retrieves the call session associated with a pending call.
         *
         * @param serviceId a service id which is obtained from {@link ImsManager#open}
         * @param profile a call profile to make the call
         */
        public IImsCallSession getPendingCallSession(int serviceId, String callId) {
            ImsServiceSub service = mServiceSubMap.get(new Integer(serviceId));
            if (service == null || callId == null) {
                Log.e (LOG_TAG, "Invalid arguments " + service + " " + callId);
                return null;
            }
            return service.getPendingSession(serviceId, callId);
        }

        /**
         * Ut interface for the supplementary service configuration.
         */
        public IImsUt getUtInterface(int serviceId) {
            ImsServiceSub service = mServiceSubMap.get(new Integer(serviceId));
            if (service == null) {
                Log.e (LOG_TAG, "Invalid argument " + service);
                return null;
            }
            return service.getUtInterface();
        }

        /**
        * Config interface for IMS Configuration
        */
        public IImsConfig getConfigInterface(int phoneId) {
            int default_subscription = 1;
            return mServiceSub[default_subscription - 1].getConfigInterface();
        }

        /**
         * Used for turning on IMS when its in OFF state.
         */
        public void turnOnIms(int phoneId) {
            int default_subscription = 1;
            mServiceSub[default_subscription - 1].turnOnIms();
        }

        /**
         * Used for turning off IMS when its in ON state. When IMS is OFF, device will behave as
         * CSFB'ed.
         */
        public void turnOffIms(int phoneId) {
            int default_subscription = 1;
            mServiceSub[default_subscription - 1].turnOffIms();
        }

        /**
         * ECBM interface for Emergency callback notifications
         */
        public IImsEcbm getEcbmInterface(int serviceId) {
            ImsServiceSub service = mServiceSubMap.get(new Integer(serviceId));
            if (service == null) {
                Log.e(LOG_TAG, "getEcbmInterface: Invalid argument " + service);
                return null;
            }
            return service.getEcbmInterface();
        }

        public void setUiTTYMode(int serviceId, int uiTtyMode, Message onComplete) {
            ImsServiceSub service = mServiceSubMap.get(new Integer(serviceId));
            if (service == null) {
                Log.e (LOG_TAG, "Invalid arguments " + serviceId);
                return;
            }
            service.setUiTTYMode(serviceId, uiTtyMode, onComplete);
        }
    };
}
