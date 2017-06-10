/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package org.codeaurora.ims;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import com.android.ims.ImsCallProfile;
import com.android.ims.ImsConfig;
import com.android.ims.ImsReasonInfo;
import com.android.ims.ImsServiceClass;
import com.android.ims.internal.IImsCallSessionListener;
import com.android.ims.internal.IImsRegistrationListener;
import com.android.ims.internal.IImsEcbm;
import com.android.ims.internal.IImsUtListener;
import com.android.ims.internal.ImsCallSession;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.DriverCall;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.PhoneProxy;

import android.os.Handler;
import android.os.Message;
import android.telephony.ServiceState;
import android.util.Log;
import android.app.PendingIntent;
import android.content.Context;
import android.os.AsyncResult;
import android.os.Looper;
import android.os.RegistrantList;
import android.os.RemoteException;
import android.telephony.TelephonyManager;

public class ImsServiceSub {
    private static final String LOG_TAG = "ImsServiceSub";
    private boolean DBG = true;
    protected ImsSenderRxr mCi = null; /* Commands Interface */
    // service class --> table
    private Map<Integer, ImsServiceClassTracker> mTrackerTable =
        new HashMap<Integer, ImsServiceClassTracker>();
    // service id --> tracker table
    private Map<Integer, ImsServiceClassTracker> mServiceIdTable =
        new HashMap<Integer, ImsServiceClassTracker>();
    private Handler mHandler;
    private Context mContext;
    private ImsEcbmImpl mImsEcbmImpl;
    private ImsConfigImpl mImsConfigImpl;
    private ServiceStatus mServiceStatus[] = null;
    private CommandsInterface mRilCommandsInterface = null;
    private int mPhoneId = -1;
    // VOIP, VT_TX, VT_RX, VT, UT
    private static final int SERVICE_TYPE_MAX = 5;
    private final int MAX_FEATURES_SUPPORTED = 3;

    private final int EVENT_CALL_STATE_CHANGE = 1;
    private final int EVENT_IMS_STATE_CHANGED = 2;
    private final int EVENT_IMS_STATE_DONE = 3;
    private final int EVENT_SRV_STATUS_UPDATE = 4;
    private final int EVENT_GET_SRV_STATUS = 5;
    private final int EVENT_SET_SRV_STATUS = 6;
    private final int EVENT_GET_CURRENT_CALLS = 7;
    private final int EVENT_SUPP_SRV_UPDATE = 8;
    private final int EVENT_SET_IMS_STATE = 9;
    private final int EVENT_TTY_STATE_CHANGED = 10;
    private final int EVENT_REFRESH_CONF_INFO = 11;
    //Event that gets triggered for intra RAT HandOver's
    private final int EVENT_HANDOVER_STATE_CHANGED = 12;
    private final int EVENT_CALL_MODIFY = 13;
    private final int EVENT_MWI = 14;
    private final int EVENT_SRVCC_STATE_CHANGED = 15;

    //TODO:Remove and use the same value when ImsReasonInfo change is merged.
    private static final int CODE_SERVICE_CLASS_NOT_SUPPORTED = -2;
    protected int mSub = -1;
    int[] mEnabledFeatures = {
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN
    };
    int[] mDisabledFeatures = {
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN
    };

    private Mwi mMwiResponse;

    //Constructor
    public ImsServiceSub(int sub, Context context) {
        mSub = sub;
        mContext = context;
        mCi = new ImsSenderRxr(mContext);
        mHandler = new ImsServiceSubHandler();
        mCi.registerForImsNetworkStateChanged(mHandler, EVENT_IMS_STATE_CHANGED,
                null);
        // Query for registration state in case we have missed the UNSOL
    //    mCi.getImsRegistrationState(mHandler.obtainMessage(EVENT_IMS_STATE_DONE));
        mCi.registerForSrvStatusUpdate(mHandler, EVENT_SRV_STATUS_UPDATE, null);
        mCi.registerForCallStateChanged(mHandler, EVENT_CALL_STATE_CHANGE, null);
        mCi.registerForRefreshConfInfo(mHandler, EVENT_REFRESH_CONF_INFO, null);
        mCi.registerForHandoverStatusChanged(mHandler, EVENT_HANDOVER_STATE_CHANGED, null);

        mImsEcbmImpl = new ImsEcbmImpl(mCi);

        //Initialize the UT interface associated with the sub.
        ImsUtImpl.createUtInterface(mCi);

        //Initialize the IMS Config interface associated with the sub.
        mImsConfigImpl =  new ImsConfigImpl(mCi);

        // For listening to incoming (MT) Hold/Resume UNSOLs.
        mCi.setOnSuppServiceNotification(mHandler, EVENT_SUPP_SRV_UPDATE, null);

        // For listening to MWI UNSOLs.
        mCi.registerForMwi(mHandler, EVENT_MWI, null);
        mMwiResponse = new Mwi();

        // For listening to TTY mode change UNSOL.
        mCi.registerForTtyStatusChanged(mHandler, EVENT_TTY_STATE_CHANGED, null);

        mCi.registerForModifyCall(mHandler, EVENT_CALL_MODIFY, null);
        initServiceStatus();
        mCi.queryServiceStatus(mHandler.obtainMessage(EVENT_GET_SRV_STATUS));
    }

    /* Method to initialize the Service related objects */
    private void initServiceStatus() {
        mServiceStatus = new ServiceStatus[SERVICE_TYPE_MAX];
        for (int i = 0; i < SERVICE_TYPE_MAX; i++) {
            mServiceStatus[i] = new ServiceStatus();
            /*
             * By default, the assumption is the service is enabled on LTE - when RIL and modem
             * changes to update the availability of service on power up this will be removed
             */
            mServiceStatus[i].accessTechStatus = new ServiceStatus.StatusForAccessTech[1];
            mServiceStatus[i].accessTechStatus[0] = new ServiceStatus.StatusForAccessTech();
            mServiceStatus[i].accessTechStatus[0].networkMode = ImsQmiIF.RADIO_TECH_LTE;
            mServiceStatus[i].accessTechStatus[0].status = ImsQmiIF.STATUS_NOT_SUPPORTED;
            mServiceStatus[i].accessTechStatus[0].registered = ImsQmiIF.Registration.NOT_REGISTERED;
            mServiceStatus[i].status = ImsQmiIF.STATUS_NOT_SUPPORTED;
        }
    }

    /**
     * Creates/updates the tracker object for the service class
     * @param serviceClass a service class specified in {@link ImsServiceClass}
     *      For VoLTE service, it MUST be a {@link ImsServiceClass#MMTEL}.
     * @param incomingCallPendingIntent When an incoming call is received,
     *        the IMS service will call {@link PendingIntent#send(Context, int, Intent)} to
     *        send back the intent to the caller with {@link #INCOMING_CALL_RESULT_CODE}
     *        as the result code and the intent to fill in the call ID; It cannot be null
     * @param listener To listen to IMS registration events; It cannot be null
     * @return Unique identifier
     */
    public int getServiceId(int serviceClass, PendingIntent intent,
            IImsRegistrationListener listener) {
        int serviceId = 0; // O is not used - boundary value between error and correct value
        if(serviceClass != ImsServiceClass.MMTEL &&
                serviceClass != ImsServiceClass.CSVT) {
            serviceId = CODE_SERVICE_CLASS_NOT_SUPPORTED;
        } else {
            ImsServiceClassTracker tracker = mTrackerTable.get(new Integer(serviceClass));
            if (tracker == null) {
                tracker = new ImsServiceClassTracker(serviceClass, intent, listener, mCi, mContext);
                tracker.updateVtCapability(isVTGloballySupported());
                mTrackerTable.put(new Integer(serviceClass), tracker);
                mServiceIdTable.put(new Integer(tracker.getServiceId()), tracker);
            } else {
                tracker.mIncomingCallIntent = intent;
                tracker.mRegListener = listener;
            }
            createFeatureCapabilityCallBackThread(listener);
            serviceId = tracker.getServiceId();
        }
        Log.d(LOG_TAG, "getServiceId returns " + serviceId);
        mCi.getImsRegistrationState(mHandler.obtainMessage(EVENT_IMS_STATE_DONE));
        return serviceId;
    }

    /**
     * Create a call profile for the call type
     * @param serviceId
     * @param serviceType
     * @param callType
     * @return ImsCallProfile object if successful or null.
     */
    public ImsCallProfile createCallProfile(int serviceId, int serviceType, int callType) {
        ImsCallProfile profile = null;
        ImsServiceClassTracker tracker = mServiceIdTable.get(new Integer(serviceId));
        if (tracker == null) {
            Log.e(LOG_TAG, " Invalid service ID - cannot create profile");
        } else {
            //TODO: Check if IMS is registered
            //TODO: Check if callType is supported i.e UNSOL_SRV_STATUS_UPDATE
            profile = new ImsCallProfile(serviceType, callType);
        }
        return profile;
    }

    /**
     * Create a call session
     * @param serviceId
     * @param profile
     * @param listener
     * @return IImsCallSession object or null on failure
     */
    public ImsCallSessionImpl createCallSession(int serviceId, ImsCallProfile profile,
            IImsCallSessionListener listener) {
        ImsCallSessionImpl session = null;
        ImsServiceClassTracker tracker = mServiceIdTable.get(new Integer(serviceId));
        if (tracker == null) {
            Log.e(LOG_TAG, "Invalid service Id - cannot create Call Session " + serviceId);
        } else {
            session = tracker.createCallSession(profile, listener);
        }
        return session;
    }

    /**
     * Get a pending call's session
     * @param serviceId
     * @param callId
     * @return IImsCallSession associated with pending call or null on failure
     */
    public ImsCallSessionImpl getPendingSession(int serviceId, String callId) {
        ImsCallSessionImpl session = null;
        ImsServiceClassTracker tracker = mServiceIdTable.get(new Integer(serviceId));
        if(tracker == null) {
            Log.e(LOG_TAG, "Invalid service Id - cannot get pending session " + serviceId);
        } else {
            session = tracker.getCallSession(callId);
        }
        return session;
    }

    public void setPhoneId(int phoneId) {
        Log.d(LOG_TAG, "setPhoneId old Phone Id = " + mPhoneId + ", new Phone Id = " + phoneId);
        if (mPhoneId != phoneId) {
            PhoneProxy phoneProxy = (PhoneProxy) PhoneFactory.getPhone(phoneId);
            if (mRilCommandsInterface != null) {
                mRilCommandsInterface.unregisterForSrvccStateChanged(mHandler);
            }
            PhoneBase phoneBase = (PhoneBase) phoneProxy.getActivePhone();
            mRilCommandsInterface = phoneBase.mCi;
            mRilCommandsInterface.registerForSrvccStateChanged(mHandler,
                    EVENT_SRVCC_STATE_CHANGED, null);
            mPhoneId = phoneId;
        }
    }

    public void setUiTTYMode(int serviceId, int uiTtyMode, Message onComplete) {
        mCi.setUiTTYMode(uiTtyMode, onComplete);
    }

    public void registerForPhoneId(int phoneId) {
        mCi.registerForPhoneId(phoneId);
    }

    /**
     * Get the UT interface handle.
     * @return IImsUt interface handle.
     */
    public ImsUtImpl getUtInterface() {
        return ImsUtImpl.getUtInterface();
    }

    /**
     * Get the Config interface handle.
     * @return IImsConfig interface handle.
     */
    public ImsConfigImpl getConfigInterface() {
        return mImsConfigImpl;
    }

    /**
     * Set the registration listener
     * @param serviceId - service ID obtained through open
     * @param listener - registration listener
     */
    public void setRegistrationListener(int serviceId, IImsRegistrationListener listener) {
        ImsServiceClassTracker tracker = mServiceIdTable.get(new Integer(serviceId));
        if(tracker == null) {
            Log.e(LOG_TAG, "Invalid service Id - cannot set reg listener " + serviceId);
        } else {
            tracker.mRegListener = listener;
        }
    }

    /**
     * Get the ECBM interface handle.
     * @return IImsEcbm interface handle.
     */
    public IImsEcbm getEcbmInterface() {
        return mImsEcbmImpl;
    }

    /**
     * Local utility to start a new thread and then call feature capability call back
     * @param tracker object that contains handle to feature capability call back
     */
    private void createFeatureCapabilityCallBackThread(final IImsRegistrationListener listener) {
        final Runnable r = new Runnable() {
            @Override
            public void run() {
                try {
                    listener.registrationFeatureCapabilityChanged(ImsServiceClass.MMTEL,
                            mEnabledFeatures, mDisabledFeatures);
                } catch (Throwable t) {
                    Log.e(LOG_TAG, t + " " + "createFeatureCapabilityCallBackThread()");
                }
            }
        };
        Thread t = new Thread(r, LOG_TAG + "createFeatureCapabilityCallBackThread");
        t.start();
    }

    private boolean isSrvTypeValid(int type) {
        // checking UT condition separately
        if (type == ImsQmiIF.CALL_TYPE_UT) {
            return true;
        } else {
            return ((type < ImsQmiIF.CALL_TYPE_VOICE) || (type > ImsQmiIF.CALL_TYPE_VT)) ? false
                    : true;
        }
    }

    private void createVoiceMessageUpdateCallbackThread(final IImsRegistrationListener listener,
                                                        final int count) {
        final Runnable r = new Runnable() {
            @Override
            public void run() {
                try {
                    listener.voiceMessageCountUpdate(count);
                } catch (Throwable t) {
                    Log.e(LOG_TAG, t + " " + "createVoiceMessageUpdateCallbackThread()");
                }
            }
        };
        Thread t = new Thread(r, LOG_TAG + "createVoiceMessageUpdateCallbackThread");
        t.start();
    }

    private void resetFeatures() {
        for (int i = 0; i < mEnabledFeatures.length; i++) {
            mEnabledFeatures[i] = ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN;
            mDisabledFeatures[i] = ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN;
        }
    }

    private void handleSrvStatusUpdate(ArrayList<ServiceStatus> updateList) {
        boolean isVtEnabled = false;
        resetFeatures();
        for (ServiceStatus update : updateList) {
            if (DBG)
                Log.d(LOG_TAG, "type = " + update.type + " status = " + update.status
                        + " isValid = " + update.isValid);
            if (update != null && update.isValid && isSrvTypeValid(update.type)) {
                ServiceStatus srvSt = null;
                if (update.type == ImsQmiIF.CALL_TYPE_UT) {
                    srvSt = mServiceStatus[SERVICE_TYPE_MAX -1];
                } else {
                    srvSt = mServiceStatus[update.type];
                }
                srvSt.isValid = update.isValid;
                srvSt.type = update.type;
                if (update.userdata != null) {
                    srvSt.userdata = new byte[update.userdata.length];
                    srvSt.userdata = Arrays.copyOf(update.userdata, update.userdata.length);
                }
                if (update.accessTechStatus != null && update.accessTechStatus.length > 0) {
                    srvSt.accessTechStatus = new ServiceStatus.StatusForAccessTech[update.
                            accessTechStatus.length];
                    if (DBG)
                        Log.d(LOG_TAG, "Call Type " + srvSt.type + "has num updates = "
                                + update.accessTechStatus.length);
                    ServiceStatus.StatusForAccessTech[] actSt = srvSt.accessTechStatus;

                    for (int i = 0; i < update.accessTechStatus.length; i++) {
                        ServiceStatus.StatusForAccessTech actUpdate =
                                update.accessTechStatus[i];
                        if (DBG)
                            Log.d(LOG_TAG, "StatusForAccessTech networkMode = "
                                    + actUpdate.networkMode
                                    + " registered = " + actUpdate.registered
                                    + " status = " + actUpdate.status
                                    + " restrictCause = " + actUpdate.restrictCause);
                        actSt[i] = new ServiceStatus.StatusForAccessTech();
                        actSt[i].networkMode = actUpdate.networkMode;
                        actSt[i].registered = actUpdate.registered;
                        if (actUpdate.status == ImsQmiIF.STATUS_ENABLED &&
                                actUpdate.restrictCause != CallDetails.CALL_RESTRICT_CAUSE_NONE) {
                            actSt[i].status = ImsQmiIF.STATUS_PARTIALLY_ENABLED;
                        } else {
                            actSt[i].status = actUpdate.status;
                        }
                        srvSt.status = actSt[i].status;
                        actSt[i].restrictCause = actUpdate.restrictCause;
                        int feature = ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN;
                        boolean modeWifi = actSt[i].networkMode == ImsQmiIF.RADIO_TECH_WIFI ||
                                actSt[i].networkMode == ImsQmiIF.RADIO_TECH_IWLAN;
                        boolean modeLte = actSt[i].networkMode == ImsQmiIF.RADIO_TECH_ANY ||
                                actSt[i].networkMode == ImsQmiIF.RADIO_TECH_LTE;
                        if (modeWifi || modeLte) {
                            if (update.type == ImsQmiIF.CALL_TYPE_VOICE) {
                                feature = modeLte ?
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE :
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI;
                            } else if (update.type == ImsQmiIF.CALL_TYPE_UT) {
                                 feature = modeLte ?
                                         ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_LTE:
                                         ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_WIFI;
                            } else { // VT_TX, VT_RX, VT
                                feature = modeLte ?
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE :
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI;
                            }
                            if (actSt[i].status == ImsQmiIF.STATUS_ENABLED ||
                                    actSt[i].status == ImsQmiIF.STATUS_PARTIALLY_ENABLED) {
                                mEnabledFeatures[feature] = feature;
                                mDisabledFeatures[feature] =
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN;
                                if (feature ==
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE ||
                                        feature ==
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI) {
                                    isVtEnabled = true;
                                }
                                if (DBG)
                                    Log.d(LOG_TAG, "enabledFeature = " + feature);
                            } else if (actSt[i].status == ImsQmiIF.STATUS_DISABLED ||
                                    actSt[i].status == ImsQmiIF.STATUS_NOT_SUPPORTED) {
                                // VT is enabled if VT_TX, or VT_RX, or VT is enabled
                                if (!(isVtEnabled == true && (feature ==
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE ||
                                        feature ==
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI))) {
                                    mDisabledFeatures[feature] = feature;
                                    mEnabledFeatures[feature] =
                                            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN;
                                    if (DBG)
                                        Log.d(LOG_TAG, "disabledFeature = " + feature);
                                }
                            }
                        }
                    }
                }
            }
        }
        for (Map.Entry<Integer, ImsServiceClassTracker> e : mTrackerTable.entrySet()) {
            createFeatureCapabilityCallBackThread(e.getValue().mRegListener);
        }
        ImsServiceClassTracker tracker = mTrackerTable.get(ImsServiceClass.MMTEL);
        if (tracker != null) {
            tracker.updateVtCapability(isVTGloballySupported());
        } else {
            loge("handleSrvStatusUpdate tracker is null so not updating global VT capability");
        }
    }

    public boolean isVTGloballySupported() {
        return mEnabledFeatures[ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE]
                    == ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE
                || mEnabledFeatures[ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI]
                    == ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI;
    }

    /**
     * Used for turning on IMS when its in OFF state.
     */
    public void turnOnIms() {
        mCi.sendImsRegistrationState(ImsQmiIF.Registration.REGISTERED,
                mHandler.obtainMessage(EVENT_SET_IMS_STATE));
    }

    /**
     * Used for turning off IMS when its in ON state. When IMS is OFF, device will behave as
     * CSFB'ed.
     */
    public void turnOffIms() {
        mCi.sendImsRegistrationState(ImsQmiIF.Registration.NOT_REGISTERED,
                mHandler.obtainMessage(EVENT_SET_IMS_STATE));
    }

    /**
     * Check if the exception is due to Radio not being available
     * @param e
     * @return true, if exception is due to RADIO_NOT_AVAILABLE
     */
    private boolean isImsExceptionRadioNotAvailable(Throwable e) {
        return e != null
                && e instanceof RuntimeException
                && ((RuntimeException) e).getMessage().equals(
                        ImsSenderRxr.errorIdToString(ImsQmiIF.E_RADIO_NOT_AVAILABLE));
    }

    /**
     * Handle the calls returned as part of UNSOL_CALL_STATE_CHANGED
     * @param ar - the AsyncResult object that contains the call list information
     */
    private void handleCalls(AsyncResult ar) {
        ArrayList<DriverCallIms> callList;
        Log.d(LOG_TAG, ">handleCalls");
        Map<Integer, DriverCallIms> dcList = new HashMap<Integer, DriverCallIms>();

        if (ar.exception == null) {
            callList = (ArrayList<DriverCallIms>) ar.result;
        } else if (isImsExceptionRadioNotAvailable(ar.exception)) {
            // just a dummy empty ArrayList to cause the loop
            // to hang up all the calls
            callList = new ArrayList<DriverCallIms> ();
        } else {
            // Radio probably wasn't ready--try again in a bit
            // But don't keep polling if the channel is closed
            return;
        }

        ArrayList<DriverCallIms> mmTelList = new ArrayList<DriverCallIms> ();
        ArrayList<DriverCallIms> csvtList = new ArrayList<DriverCallIms> ();

        if (callList != null) {
            for (DriverCallIms dc: callList) {
                Log.d(LOG_TAG, "handleCalls: dc = " + dc);
                if (dc != null && dc.callDetails != null &&
                        dc.callDetails.call_type == CallDetails.CALL_TYPE_VT &&
                        (dc.callDetails.call_domain == CallDetails.CALL_DOMAIN_CS)) {
                    csvtList.add(dc);
                } else {
                    //Setting Mpty bit false because Confrence UI is not supported in incoming state
                    //UI will transition to conference call when Mpty bit is updated in active state
                    if (dc.isMT && dc.isMpty && ((dc.state == DriverCallIms.State.INCOMING) ||
                            (dc.state == DriverCallIms.State.WAITING))) {
                        Log.d(LOG_TAG, "Setting the multi party flag to false");
                        dc.isMpty = false;
                    }
                    mmTelList.add(dc);
                }
            }
        }

        /** TODO: Right now assume all calls are for MMTEL - in future
         * when RCS support is added DriverCallIms should contain information for what
         * service class the call is for - may be have it in CallDetails
         */
        ImsServiceClassTracker tracker = mTrackerTable.get(new Integer(ImsServiceClass.MMTEL));
        if (tracker == null) {
            /**
             * TODO: Right not there is no response back from telephony to RIL
             * for these scenarios, Client did not register for service class but a call
             * for the service class was received. Some possible ways are
             *  1) send a reject for an incoming call
             *  2) send a hangup for an active call
             */
            if (mmTelList.size() > 0) {
                Log.e (LOG_TAG, "Call for non-registered service class MMTEL");
            }
        } else {
            // service class will be filtered here and only calls with service class MMTEL will go to tracker
            tracker.handleCalls((ArrayList<DriverCallIms>) mmTelList);
        }

        tracker = mTrackerTable.get(new Integer(ImsServiceClass.CSVT));
        if (tracker == null) {
            if (csvtList.size() > 0) {
                Log.e (LOG_TAG, "Call for non-registered service class CSVT");
            }
        } else {
            // Only CSVT calls will go to CSVT tracker
            tracker.handleCalls((ArrayList<DriverCallIms>) csvtList);
        }
    }

    /**
     * Handle intra RAT Handovers as part of UNSOL_RESPONSE_HANDOVER
     * @param ar - the AsyncResult object that contains handover information
     */
    private void handleHandover(AsyncResult ar) {
        Log.d(LOG_TAG, "handleHandover");
        ImsQmiIF.Handover handover = null;
        if (ar.exception == null) {
            handover = (ImsQmiIF.Handover) ar.result;

            ImsServiceClassTracker tracker = mTrackerTable.get(new Integer(ImsServiceClass.MMTEL));
            if (tracker != null) {
                tracker.handleHandover(handover);
            }
            else {
                Log.e (LOG_TAG, "Message for non-registered service class");
                return;
            }
        }
        else {
            Log.e(LOG_TAG, "AsyncResult exception in handleHandover- " + ar.exception);
        }

    }

    /**
     * Handle the call state changes for incoming (MT) Hold/Resume as part of
     * the UNSOL_SUPP_SVC_NOTIFICATION message.
     * @param ar - the AsyncResult object that contains the call list information
     */
    private void handleSuppSvc(AsyncResult ar) {
        Log.d(LOG_TAG, "handleSuppSvc");
        ImsQmiIF.SuppSvcNotification supp_svc_unsol = null;
        if (ar.exception == null) {
            supp_svc_unsol = (ImsQmiIF.SuppSvcNotification) ar.result;

            // TODO: RCS support, later. Also refer to TODO items in handleCalls method above.
            ImsServiceClassTracker tracker = mTrackerTable.get(new Integer(ImsServiceClass.MMTEL));
            if (tracker != null) {
                tracker.handleSuppSvcUnsol(supp_svc_unsol);
            }
            else {
                Log.e (LOG_TAG, "Message for non-registered service class");
                return;
            }
        }
        else {
            Log.e(LOG_TAG, "AsyncResult exception in handleSuppSvc.");
        }
    }

    /**
     * Handle the TTY mode changes as part of the UNSOL_TTY_NOTIFICATION message.
     * @param ar - the AsyncResult object that contains new TTY mode.
     */
    private void handleTtyModeChange(AsyncResult ar) {
        Log.d(LOG_TAG, "handleTtyModeChange");
        if (ar != null && ar.result != null && ar.exception == null) {
            int[] mode = (int[])ar.result;
            Log.e(LOG_TAG, "Received EVENT_TTY_STATE_CHANGED mode= " + mode[0]);

            // TODO: RCS support, later. Also refer to TODO items in handleCalls method above.
            ImsServiceClassTracker tracker = mTrackerTable.get(new Integer(ImsServiceClass.MMTEL));
            if (tracker != null) {
                tracker.handleTtyModeChangeUnsol(mode[0]);
            } else {
                Log.e (LOG_TAG, "Message for non-registered service class");
                return;
            }
        } else {
            Log.e(LOG_TAG, "Error EVENT_TTY_STATE_CHANGED AsyncResult ar= " + ar);
        }
     }

    /**
     * Gets a call session with give media id.
     * @param mediaId Media id of the session to be searched.
     * @return Call session with {@code mediaId}
     */
    public List<ImsCallSessionImpl> getCallSessionByState(DriverCallIms.State state) {
        ImsServiceClassTracker tracker = mTrackerTable.get(ImsServiceClass.MMTEL);
        return tracker == null ? Collections.EMPTY_LIST : tracker.getCallSessionByState(state);
    }

    /**
     * Gets a call session with give media id.
     * @param mediaId Media id of the session to be searched.
     * @return Call session with {@code mediaId}
     */
    public ImsCallSessionImpl findSessionByMediaId(int mediaId) {
        ImsServiceClassTracker tracker = mTrackerTable.get(ImsServiceClass.MMTEL);
        return tracker == null ? null : tracker.findSessionByMediaId(mediaId);
    }

    /**
     * @return Subscription id.
     */
    public int getSubscription() {
        return mSub;
    }

    /**
     * Registers call list listener.
     * Note: Only {@code ImsServiceClass.MMTEL} is supported.
     * @param listener Listener to registered
     * @see ICallListListener
     */
    public void addListener(ICallListListener listener) {
        ImsServiceClassTracker tracker = mTrackerTable.get(ImsServiceClass.MMTEL);
        if (tracker != null) {
            tracker.addListener(listener);
        } else {
            loge("ImsServiceClassTracker not found.");
        }
    }

    /**
     * Handles Conference refresh Info notified through UNSOL_REFRESH_CONF_INFO message
     * @param ar - the AsyncResult object that contains the refresh Info information
     */
    public void handleRefreshConfInfo(AsyncResult ar) {
        Log.d(LOG_TAG, "handleRefreshConfInfo");
        if ((ar != null) && (ar.exception == null) && (ar.result != null)) {
            ImsQmiIF.ConfInfo confInfo = (ImsQmiIF.ConfInfo) ar.result;
            ImsServiceClassTracker tracker = mTrackerTable.get(new Integer(ImsServiceClass.MMTEL));
            if (tracker != null) {
                tracker.handleRefreshConfInfo(confInfo);
            } else {
                Log.i(LOG_TAG, "Message for non-registered service class");
                return;
            }
        } else {
            if (ar != null) {
                Log.e(LOG_TAG, "Failed @handleRefreshConfInfo --> " + ar.exception
                        + "with result = " + ar.result);
            } else {
                Log.e(LOG_TAG, "Failed @handleRefreshConfInfo --> AsyncResult is null");
            }
        }
    }

    /**
     * Unregisters call list listener.
     * Note: Only {@code ImsServiceClass.MMTEL} is supported.
     * @param listener Listener to unregistered
     * @see ICallListListener
     */
    public void removeListener(ICallListListener listener) {
        ImsServiceClassTracker tracker = mTrackerTable.get(ImsServiceClass.MMTEL);
        if (tracker != null) {
            tracker.removeListener(listener);
        } else {
            loge("ImsServiceClassTracker not found.");
        }
    }

    private void handleModifyCallRequest(CallModify cm) {
        Log.d(LOG_TAG, "handleCallModifyRequest(" + cm + ")");

        ImsServiceClassTracker tracker = mTrackerTable.get(ImsServiceClass.MMTEL);
        tracker.handleModifyCallRequest(cm);
    }

    //Handler for the events on response from ImsSenderRxr
    private class ImsServiceSubHandler extends Handler {
        ImsServiceSubHandler() {
            super();
        }

        /**
         * Local utility to start a new thread and then call registration call back
         * @param tracker object that contains handle to registration call back
         * @param registered
         */
        private void createRegCallBackThread(final IImsRegistrationListener listener,
                final int registrationState, final ImsReasonInfo imsReasonInfo,
                final int imsRadioTech) {
            final Runnable r = new Runnable() {
                @Override
                public void run() {
                    try {
                        switch (registrationState) {
                            case ImsQmiIF.Registration.REGISTERED:
                                listener.registrationConnected(imsRadioTech);
                                break;
                            case ImsQmiIF.Registration.NOT_REGISTERED:
                                listener.registrationDisconnected(imsReasonInfo);
                                break;
                            case ImsQmiIF.Registration.REGISTERING:
                                listener.registrationProgressing(imsRadioTech);
                                break;
                        }
                    } catch (Throwable t) {
                        Log.e(LOG_TAG, t + " " + "createRegCallBackThread()");
                    }
                }
            };
            Thread t = new Thread(r, LOG_TAG + "RegCallbackThread");
            t.start();
        }

        private void handleImsStateChanged(AsyncResult ar) {
            log("handleImsStateChanged");
            int errorCode = ImsReasonInfo.CODE_UNSPECIFIED;
            String errorMessage = null;
            int regState = ImsQmiIF.Registration.NOT_REGISTERED;
            int imsRat = ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN;
            if (ar != null && ar.exception == null && ar.result instanceof ImsQmiIF.Registration) {
                ImsQmiIF.Registration registration = (ImsQmiIF.Registration) ar.result;

                errorCode = registration.hasErrorCode() ? registration.getErrorCode()
                        : ImsReasonInfo.CODE_UNSPECIFIED;
                errorMessage = registration.hasErrorMessage() ? registration
                        .getErrorMessage() : null;
                regState = registration.hasState() ? registration.getState()
                        : ImsQmiIF.Registration.NOT_REGISTERED;
                imsRat = getRilRadioTech(registration);
            } else {
                loge("handleImsStateChanged error");
            }

            ImsReasonInfo imsReasonInfo = new ImsReasonInfo(
                    ImsReasonInfo.CODE_REGISTRATION_ERROR,
                    errorCode, errorMessage);
            for (Map.Entry<Integer, ImsServiceClassTracker> e : mTrackerTable.entrySet()) {
                createRegCallBackThread(e.getValue().mRegListener, regState, imsReasonInfo, imsRat);
            }
        }

        private int getRilRadioTech(ImsQmiIF.Registration registration) {
            if (!registration.hasRadioTech()) {
                return ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN;
            }

            int imsRat;
            switch (registration.getRadioTech()) {
                case ImsQmiIF.RADIO_TECH_LTE:
                    imsRat = ServiceState.RIL_RADIO_TECHNOLOGY_LTE;
                break;
                case ImsQmiIF.RADIO_TECH_WIFI:
                case ImsQmiIF.RADIO_TECH_IWLAN:
                    imsRat = ServiceState.RIL_RADIO_TECHNOLOGY_IWLAN;
                break;
                default:
                   imsRat = ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN;
            }

            return imsRat;
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d (LOG_TAG, "Message received: what = " + msg.what);
            AsyncResult ar;

            switch (msg.what) {
                case EVENT_IMS_STATE_CHANGED: //intentional fall through
                case EVENT_IMS_STATE_DONE:
                    ar = (AsyncResult) msg.obj;
                    handleImsStateChanged(ar);
                    break;
                case EVENT_SET_IMS_STATE:
                    ar = (AsyncResult) msg.obj;
                    if ( ar.exception != null ) {
                        Log.d(LOG_TAG, "Request turn on/off IMS failed");
                    }
                    break;
                case EVENT_SRV_STATUS_UPDATE:
                    Log.d(LOG_TAG, "Received event: EVENT_SRV_STATUS_UPDATE");
                    AsyncResult arResult = (AsyncResult) msg.obj;
                    if (arResult.exception == null && arResult.result != null) {
                        ArrayList<ServiceStatus> responseArray =
                                (ArrayList<ServiceStatus>) arResult.result;
                        handleSrvStatusUpdate(responseArray);
                    } else {
                        Log.e(LOG_TAG, "IMS Service Status Update failed!");
                        initServiceStatus();
                    }
                    break;
                case EVENT_GET_SRV_STATUS:
                    Log.d(LOG_TAG, "Received event: EVENT_GET_STATUS_UPDATE");
                    AsyncResult arResultSrv = (AsyncResult) msg.obj;
                    if (arResultSrv.exception == null && arResultSrv.result != null) {
                        ArrayList<ServiceStatus> responseArray =
                                (ArrayList<ServiceStatus>) arResultSrv.result;
                        handleSrvStatusUpdate(responseArray);
                    } else {
                        Log.e(LOG_TAG, "IMS Service Status Update failed!");
                        initServiceStatus();
                    }
                    break;
                case EVENT_SET_SRV_STATUS:
                    //TODO:
                    break;
                case EVENT_CALL_STATE_CHANGE:
                    ar = (AsyncResult) msg.obj;
                    handleCalls(ar);
                    break;
                case EVENT_GET_CURRENT_CALLS:
                    ar = (AsyncResult) msg.obj;
                    handleCalls(ar);
                    break;
                case EVENT_SUPP_SRV_UPDATE:
                    ar = (AsyncResult) msg.obj;
                    handleSuppSvc(ar);
                    break;
                case EVENT_TTY_STATE_CHANGED:
                    ar = (AsyncResult) msg.obj;
                    handleTtyModeChange(ar);
                    break;
                case EVENT_REFRESH_CONF_INFO:
                    ar = (AsyncResult) msg.obj;
                    handleRefreshConfInfo(ar);
                    break;
                case EVENT_HANDOVER_STATE_CHANGED:
                    ar = (AsyncResult) msg.obj;
                    handleHandover(ar);
                    break;
                case EVENT_SRVCC_STATE_CHANGED:
                    ar = (AsyncResult) msg.obj;
                    ImsServiceClassTracker tracker = mTrackerTable.get(ImsServiceClass.MMTEL);
                    if (tracker != null && ar.exception == null) {
                        tracker.calculateOverallSrvccState((int[]) ar.result);
                    } else {
                        loge("Error EVENT_SRVCC_STATE_CHANGED tracker is null or srvcc exception "
                                + ar.exception);
                    }
                    break;
                case EVENT_CALL_MODIFY:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null && ar.result != null && ar.exception == null) {
                        handleModifyCallRequest((CallModify) ar.result);
                    } else {
                        Log.e(LOG_TAG, "Error EVENT_MODIFY_CALL AsyncResult ar= " + ar);
                    }
                    break;
                case EVENT_MWI:
                    handleMwiNotification(msg);
                    break;
                default:
                    Log.d(LOG_TAG, "Unknown message = " + msg.what);
            }
        }
    }

    private static void log(String msg) {
        Log.d(LOG_TAG, msg);
    }

    private static void loge(String msg) {
        Log.e(LOG_TAG, msg);
    }

    private void handleMwiNotification(Message msg) {
        if ((msg != null) && (msg.obj != null)) {
            AsyncResult arMwiUpdate = (AsyncResult) msg.obj;
            if (arMwiUpdate.exception == null) {
                if (arMwiUpdate.result != null) {
                    ImsQmiIF.Mwi mwiIF = (ImsQmiIF.Mwi) arMwiUpdate.result;
                    int summaryCount = mwiIF.getMwiMsgSummaryCount();
                    mMwiResponse.mwiMsgSummary = new Mwi.MwiMessageSummary[summaryCount];
                    Log.d(LOG_TAG,"handleMwiNotification summaryCount = " + summaryCount);

                    for (int i = 0; i < summaryCount; i++) {
                        mMwiResponse.mwiMsgSummary[i] = new Mwi.MwiMessageSummary();
                        mMwiResponse.mwiMsgSummary[i].mMessageType =
                                mwiIF.getMwiMsgSummary(i).getMessageType();
                        mMwiResponse.mwiMsgSummary[i].mNewMessage =
                                mwiIF.getMwiMsgSummary(i).getNewMessage();
                        mMwiResponse.mwiMsgSummary[i].mOldMessage =
                                mwiIF.getMwiMsgSummary(i).getOldMessage();
                        mMwiResponse.mwiMsgSummary[i].mNewUrgent =
                                mwiIF.getMwiMsgSummary(i).getNewUrgent();
                        mMwiResponse.mwiMsgSummary[i].mOldUrgent =
                                mwiIF.getMwiMsgSummary(i).getOldUrgent();
                        Log.d(LOG_TAG,"Message Summary = "
                                + mMwiResponse.summaryToString(mMwiResponse.mwiMsgSummary[i]));
                    }

                    int detailsCount = mwiIF.getMwiMsgDetailCount();
                    Log.d(LOG_TAG,"handleMwiNotification detailsCount = " + detailsCount);
                    mMwiResponse.mwiMsgDetails = new Mwi.MwiMessageDetails[detailsCount];
                    for (int i = 0; i < detailsCount; i++) {
                        mMwiResponse.mwiMsgDetails[i] = new Mwi.MwiMessageDetails();
                        mMwiResponse.mwiMsgDetails[i].mToAddress =
                                mwiIF.getMwiMsgDetail(i).getToAddress();
                        mMwiResponse.mwiMsgDetails[i].mFromAddress =
                                mwiIF.getMwiMsgDetail(i).getFromAddress();
                        mMwiResponse.mwiMsgDetails[i].mSubject =
                                mwiIF.getMwiMsgDetail(i).getSubject();
                        mMwiResponse.mwiMsgDetails[i].mDate = mwiIF.getMwiMsgDetail(i).getDate();
                        mMwiResponse.mwiMsgDetails[i].mPriority =
                                mwiIF.getMwiMsgDetail(i).getPriority();
                        mMwiResponse.mwiMsgDetails[i].mMessageId =
                                mwiIF.getMwiMsgDetail(i).getMessageId();
                        mMwiResponse.mwiMsgDetails[i].mMessageType =
                                mwiIF.getMwiMsgDetail(i).getMessageType();
                        Log.d(LOG_TAG,"Message Details = "
                                + mMwiResponse.detailsToString(mMwiResponse.mwiMsgDetails[i]));
                    }
                    updateVoiceMail();
                } else {
                    Log.e(LOG_TAG,"handleMwiNotification arMwiUpdate.result null");
                }
            } else {
                Log.e(LOG_TAG,"handleMwiNotification arMwiUpdate exception");
            }
        } else {
            Log.e(LOG_TAG,"handleMwiNotification msg null");
        }
    }

    private void updateVoiceMail() {
        int count = 0;
        for (int i = 0; i < mMwiResponse.mwiMsgSummary.length; i++) {
            if (mMwiResponse.mwiMsgSummary[i].mMessageType == ImsQmiIF.MWI_MSG_VOICE) {
                count = count + mMwiResponse.mwiMsgSummary[i].mNewMessage
                        + mMwiResponse.mwiMsgSummary[i].mNewUrgent;
                break;
            }
        }
        // Holds the Voice mail count
        // .i.e.MwiMessageSummary.NewMessage + MwiMessageSummary.NewUrgent
        Log.d(LOG_TAG,"updateVoiceMail count = " + count);

        for (Map.Entry<Integer, ImsServiceClassTracker> e : mTrackerTable.entrySet()) {
            createVoiceMessageUpdateCallbackThread(e.getValue().mRegListener, count);
        }
    }
}
