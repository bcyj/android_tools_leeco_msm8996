/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.roamingsettings;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.provider.Settings;
import android.telephony.CellLocation;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.gsm.GsmCellLocation;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.UiccController;

/**
 * Display the following information # Battery Strength : TODO # Uptime # Awake
 * Time # XMPP/buzz/tickle status :TODO
 */
public class CurrentNetworkStatus extends PreferenceActivity {
    private static final String TAG = "CurrentSimStatus";
    private static final boolean DEBUG = true;
    private static final String BUNDLE_KEY_SUBSCRIPTION =
            PhoneConstants.SUBSCRIPTION_KEY;
    private static final String SMSC = "smsc";
    private static final String SUB = "sub";
    private static final String COMMAND_GET_SMSC = "com.android.smsc.cmd.get";
    private static final String NOTIFY_SMSC_UPDATE =
        "com.android.smsc.notify.update";
    private static final String KEY_OPERATOR_NAME = "operator_name";
    private static final String KEY_MCC_MNC = "mcc_mnc";
    private static final String KEY_SID_NID = "sid_nid";
    private static final String KEY_CELL_ID = "cell_id";
    private static final String KEY_NETWORK_TYPE = "network_type";
    private static final String KEY_SIGNAL_STRENGTH = "signal_strength";
    private static final String KEY_PRL_VERSION = "prl_version";
    private static final String KEY_PNL_VERSION = "pnl_version";
    private static final String KEY_SMS_SERVICE_CENTER = "sms_service_center";
    private static final int GET_SMSC_RESULT = 0;
    private static final int GET_MESSAGE_EF_PRL = 1;
    // Add phone feature uri
    private static final Uri URI_PHONE_FEATURE = Uri
            .parse("content://com.qualcomm.qti.phonefeature.FEATURE_PROVIDER");
    private static final String METHOD_GET_SMSC = "get_smsc";
    public static final String EXTRA_SMSC = "smsc";
    public static final String EXTRA_CALLBACK = "callback";
    public static final String EXTRA_USEROBJ = "userobj";

    private static final int INVALID_SUBSCRIPTION = -1;

    //True PRL ID is x bytes data
    private static final int NUM_BYTES_PRL_ID = 4;

    private TelephonyManager mTMgr = null;
    private PhoneStateListener mPhoneStateListener = null;
    private Resources mRes = null;

    private static String mUnknown = null;

    private SignalStrength mSignalStrength = null;
    private ServiceState mServiceState = null;
    private BroadcastReceiver mReceiver = null;
    private Phone mPhone = null;

    private int mSlotId;
    private boolean mIsMultiSimEnabled;
    private Handler mHandler = new SmscHandler();
    private final class SmscHandler extends Handler {

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                 case GET_SMSC_RESULT:
                    Bundle bundle = (Bundle) msg.obj;
                    if (bundle == null){
                        return;
                    }
                    Bundle userParams = (Bundle)bundle.getParcelable(EXTRA_USEROBJ);
                    if (userParams == null) {
                        Log.d(TAG, "userParams = null");
                        return;
                    }
                    int sub = userParams.getInt(PhoneConstants.SLOT_KEY, -1);
                    if (sub != -1) {
                        bundle.putInt(PhoneConstants.SLOT_KEY, sub);
                        updateSmsc(bundle);
                    }
                    break;

                case GET_MESSAGE_EF_PRL:
                    boolean ret = handleGetEFPrl(msg);
                    if (ret = false) {
                        getDefaultPrlVersion();
                    }
                    break;

                default:
                    break;
            }
        }
    }

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mSlotId = getIntent().getIntExtra(BUNDLE_KEY_SUBSCRIPTION,
                INVALID_SUBSCRIPTION);
        if (mSlotId == INVALID_SUBSCRIPTION) {
            finish();
        }
        if (DEBUG) {
            Log.d(TAG, "mSubscription:" + mSlotId);
        }

        addPreferencesFromResource(R.xml.current_sim_status);

        mIsMultiSimEnabled = TelephonyManager.getDefault().isMultiSimEnabled();
        mTMgr = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        if (mIsMultiSimEnabled) {
            mPhone = PhoneFactory.getPhone(mSlotId);
        } else {
            mPhone = PhoneFactory.getDefaultPhone();
        }

        mPhoneStateListener = getPhoneStateListener(getSubIdFromSlotId(mSlotId));

        mRes = getResources();
        if (mUnknown == null) {
            mUnknown = mRes.getString(R.string.device_info_default);
        }

        findPreference(KEY_MCC_MNC).setTitle(
                getString(R.string.status_mcc) + ","
                        + getString(R.string.status_mnc));

        if (mSlotId == PhoneConstants.SUB1) {
            if (isCdmaMode()) {
                findPreference(KEY_SID_NID).setTitle(
                        getString(R.string.status_sid) + ","
                                + getString(R.string.status_nid));
                getPreferenceScreen().removePreference(
                        findPreference(KEY_SMS_SERVICE_CENTER));
            } else {
                getPreferenceScreen().removePreference(findPreference(KEY_SID_NID));
            }
            updateSummery();
        } else {
            getPreferenceScreen().removePreference(findPreference(KEY_SID_NID));
            getPreferenceScreen().removePreference(findPreference(KEY_CELL_ID));
            getPreferenceScreen().removePreference(findPreference(KEY_PRL_VERSION));
            getPreferenceScreen().removePreference(findPreference(KEY_PNL_VERSION));
            boolean airplaneModeOn = Settings.System.getInt(
                getContentResolver(), Settings.System.AIRPLANE_MODE_ON, 0) != 0;
            updateServiceCenterState(airplaneModeOn);
        }

        registerReceiver();
    }

    private void updateSummery() {
        String pnlVersionSummery = SystemProperties.get("persist.env.phone.pnlversion", "0");
        if (isCdmaMode()) {
            findPreference(KEY_PNL_VERSION).setTitle("GSM(WCDMA)");
            pnlVersionSummery = mRes.getString(R.string.status_pnl_version)
                    + ": " + pnlVersionSummery;
            getPrlVersion();
        } else {
            getDefaultPrlVersion();
        }
        boolean airplaneModeOn = Settings.System.getInt(
                getContentResolver(), Settings.System.AIRPLANE_MODE_ON, 0) != 0;
        updateServiceCenterState(airplaneModeOn);
        setSummery(KEY_PNL_VERSION, pnlVersionSummery);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mPhoneStateListener != null) {
            mTMgr.listen(mPhoneStateListener, PhoneStateListener.LISTEN_SERVICE_STATE
                    | PhoneStateListener.LISTEN_SIGNAL_STRENGTHS
                    | PhoneStateListener.LISTEN_DATA_CONNECTION_STATE);
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        if (mPhoneStateListener != null) {
            mTMgr.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
        }
    }

    @Override
    protected void onDestroy() {
        if (mReceiver != null) {
            unregisterReceiver(mReceiver);
        }
        super.onDestroy();
    }

    private PhoneStateListener getPhoneStateListener(final int subId) {
        PhoneStateListener phoneStateListener = new PhoneStateListener(subId) {
            @Override
            public void onSignalStrengthsChanged(SignalStrength signalStrength) {
                mSignalStrength = signalStrength;
                if (DEBUG) {
                    Log.d(TAG, "mSignalStrength:" + mSignalStrength);
                }
                updateSignalStrength(subId);
            }

            @Override
            public void onServiceStateChanged(ServiceState state) {
                mServiceState = state;
                if (DEBUG) {
                    Log.d(TAG, "mServiceState:" + mServiceState);
                }
                updateServiceState(subId);
                updateNetworkType(subId);
            }

            @Override
            public void onDataConnectionStateChanged(int state) {

            }
        };
        return phoneStateListener;
    }

    private void setSummery(String preference, String text) {
        if (TextUtils.isEmpty(text)) {
            text = mUnknown;
        }
        // some preferences may be missing
        if (findPreference(preference) != null) {
            findPreference(preference).setSummary(text);
        }
    }

    private String getLocaleString(String networkName) {
        Log.i(TAG, networkName);
        networkName = android.util.NativeTextHelper.getInternalLocalString(this,
                networkName,
                R.array.original_carrier_names, R.array.local_carrier_names);
        return networkName;
    }

    private void updateServiceState(int subId) {
        String display = mUnknown;
        if (mServiceState != null) {
            String operatorNameSummery;
            if (mIsMultiSimEnabled) {
                operatorNameSummery = mTMgr.getNetworkOperatorName(subId);
            } else {
                operatorNameSummery = mTMgr.getNetworkOperatorName();
            }
            if (operatorNameSummery == null) {
                operatorNameSummery = mUnknown;
            }
            setSummery(KEY_OPERATOR_NAME, getLocaleString(operatorNameSummery));

            String mccMncSummery = mUnknown;
            String operatorNumeric = mServiceState.getOperatorNumeric();
            if (!TextUtils.isEmpty(operatorNumeric)) {
                if (DEBUG) {
                    Log.d(TAG, "operatorNumeric:" + operatorNumeric);
                }
                mccMncSummery = operatorNumeric.substring(0, 3) + ","
                        + operatorNumeric.substring(3, operatorNumeric.length());
                new StringBuilder(operatorNumeric).insert(4, ",").toString();
            }
            setSummery(KEY_MCC_MNC, mccMncSummery);

            if (mSlotId == PhoneConstants.SUB1) {
                String sidNidSummery = mRes.getString(R.string.device_info_not_available);
                boolean airplaneModeOn = Settings.System.getInt(
                        getContentResolver(), Settings.System.AIRPLANE_MODE_ON, 0) != 0;
                if (isCdmaMode() && !airplaneModeOn) {
                    int sid = mServiceState.getSystemId();
                    int nid = mServiceState.getNetworkId();
                    if (sid > 0 && nid > 0) {
                        sidNidSummery = sid + "," + nid;
                    } else {
                        sidNidSummery = mUnknown;
                    }
                }
                setSummery(KEY_SID_NID, sidNidSummery);

                int cellId = -1;
                String cellIdSummery = mUnknown;
                CellLocation loc = mPhone.getCellLocation();
                if (loc != null) {
                    if (loc instanceof GsmCellLocation) {
                        cellId = ((GsmCellLocation)loc).getCid();
                        cellIdSummery = String.valueOf(cellId);
                    } else if (loc instanceof CdmaCellLocation) {
                        cellId = ((CdmaCellLocation)loc).getBaseStationId();
                        cellIdSummery = String.valueOf(cellId);
                    }
                }
                setSummery(KEY_CELL_ID, cellIdSummery);
            }
        }
    }

    void updateSignalStrength(int subId) {
        // not loaded in some versions of the code (e.g., zaku)
        int signalDbm = 0;
        int signalAsu = 0;
        if (mSignalStrength != null) {
            int state = mServiceState.getState();
            Resources r = getResources();
            String mSigStrengthSummery = null;
            if ((ServiceState.STATE_OUT_OF_SERVICE == state)
                    || (ServiceState.STATE_POWER_OFF == state)) {
                mSigStrengthSummery = "0";
            } else {
                signalDbm = mSignalStrength.getDbm();
                if (-1 == signalDbm) signalDbm = 0;

                signalAsu = mSignalStrength.getAsuLevel();
                if (-1 == signalAsu) signalAsu = 0;

                mSigStrengthSummery = String.valueOf(signalDbm) + " "
                        + r.getString(R.string.radioInfo_display_dbm) + "   "
                        + String.valueOf(signalAsu) + " "
                        + r.getString(R.string.radioInfo_display_asu);

                if (TelephonyManager.getNetworkClass(getDataNetworkType(subId))
                        == TelephonyManager.NETWORK_CLASS_4_G) {
                    int signalLteAsu = mSignalStrength.getLteSignalStrength();
                    if (-1 == signalLteAsu) {
                        signalLteAsu = 0;
                    }
                    mSigStrengthSummery += "\n" + String.valueOf(-113 + 2 * signalLteAsu) + " "
                            + r.getString(R.string.radioInfo_display_dbm) + "   "
                            + String.valueOf(signalLteAsu) + " "
                            + r.getString(R.string.radioInfo_display_asu);
                }
            }
            setSummery(KEY_SIGNAL_STRENGTH, mSigStrengthSummery);
        }
    }

    private String covertRadioTypeToString(int networkType){
        String networkTypeName = null;
        switch (networkType) {
            case TelephonyManager.NETWORK_TYPE_GPRS:
            case TelephonyManager.NETWORK_TYPE_EDGE:
            case TelephonyManager.NETWORK_TYPE_GSM:
            case TelephonyManager.NETWORK_TYPE_TD_SCDMA:
                networkTypeName = "GSM";
                break;
            case TelephonyManager.NETWORK_TYPE_UMTS:
                networkTypeName = "WCDMA";
                break;
            case TelephonyManager.NETWORK_TYPE_HSDPA:
            case TelephonyManager.NETWORK_TYPE_HSUPA:
            case TelephonyManager.NETWORK_TYPE_HSPA:
            case TelephonyManager.NETWORK_TYPE_HSPAP:
                networkTypeName = "HSPA";
                break;
            case TelephonyManager.NETWORK_TYPE_CDMA:
            case TelephonyManager.NETWORK_TYPE_EVDO_0:
            case TelephonyManager.NETWORK_TYPE_EVDO_A:
            case TelephonyManager.NETWORK_TYPE_EVDO_B:
                networkTypeName = "CDMA EVDO";
                break;
            case TelephonyManager.NETWORK_TYPE_LTE:
                networkTypeName = "LTE";
                break;
            case TelephonyManager.NETWORK_TYPE_EHRPD:
                networkTypeName = "EHRPD";
                break;
            case TelephonyManager.NETWORK_TYPE_1xRTT:
                networkTypeName = "CDMA 1x";
                break;
            default:
                networkTypeName = mRes.getString(R.string.device_info_default);
                break;
        }

        return networkTypeName;
    }

    private int getDataNetworkType(int subId) {
        int networkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        if (mIsMultiSimEnabled) {
            networkType = mTMgr.getDataNetworkType(subId);
        } else {
            networkType = mTMgr.getDataNetworkType();
        }
        return networkType;
    }

    private int getVoiceNetworkType(int subId) {
        int networkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        if (mIsMultiSimEnabled) {
            networkType = mTMgr.getVoiceNetworkType(subId);
        } else {
            networkType = mTMgr.getVoiceNetworkType();
        }
        return networkType;
    }

    private void updateNetworkType(int subId) {
        // Whether EDGE, UMTS, etc...
        int networkType = getDataNetworkType(subId);
        int voiceType = getVoiceNetworkType(subId);
        int networkClass = TelephonyManager.getNetworkClass(networkType);
        String networkSummery = covertRadioTypeToString(voiceType)
                + (networkClass != TelephonyManager.NETWORK_CLASS_4_G ? ""
                        : (", " + covertRadioTypeToString(networkType)));
        if (DEBUG) {
            Log.d(TAG, "networkSummery:" + networkSummery);
        }
        setSummery(KEY_NETWORK_TYPE, networkSummery);
    }

    private void registerReceiver() {
        if (mReceiver != null) {
            return;
        }
        mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)) {
                    // set the default as the airplane mode is off
                    boolean on = intent.getBooleanExtra("state", false);
                    updateServiceCenterState(on);
                } else if (NOTIFY_SMSC_UPDATE.equals(action)) {
                    String summary = intent.getStringExtra(SMSC);
                    setSummery(KEY_SMS_SERVICE_CENTER, summary);
                }
            }
        };

        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        filter.addAction(NOTIFY_SMSC_UPDATE);
        registerReceiver(mReceiver, filter);

    }

    private void updateServiceCenterState(boolean airplaneModeOn) {
        if (DEBUG) {
            Log.d(TAG, "updateServiceCenterState :" + mSlotId);
        }
        if (airplaneModeOn) {
            setSummery(KEY_SMS_SERVICE_CENTER, mUnknown);
            return;
        }

        final Message callback = mHandler.obtainMessage(GET_SMSC_RESULT);
        Bundle userParams = new Bundle();
        userParams.putInt(PhoneConstants.SLOT_KEY, mSlotId);
        callback.obj = userParams;
        getSmscFromSub(this, mSlotId, callback);
    }

    public static boolean isPhoneFeatureEnabled(Context context) {
        return context.getContentResolver().acquireProvider(URI_PHONE_FEATURE) != null;
    }

    private static Bundle callBinder(Context context, String method, Bundle extras) {
        if (!isPhoneFeatureEnabled(context)) {
            return null;
        }
        return context.getContentResolver().call(URI_PHONE_FEATURE, method, null, extras);
    }

    public static void getSmscFromSub(Context context, int sub, Message callback) {
        if (callback == null) {
            return;
        }
        Log.d(TAG, "Get: sub = " + sub);
        callback.replyTo = new Messenger(callback.getTarget());
        Bundle params = new Bundle();
        params.putInt(PhoneConstants.SLOT_KEY, sub);
        params.putParcelable(EXTRA_CALLBACK, callback);
        callBinder(context, METHOD_GET_SMSC, params);
    }

    private  void updateSmsc(Bundle bundle) {
        int sub = bundle.getInt(PhoneConstants.SLOT_KEY, -1);
        if (sub != -1) {
            String summary = bundle.getString(EXTRA_SMSC, null);
            Log.d(TAG, "Update SMSC: sub= " + sub + " SMSC= " + summary);
            int end = summary.lastIndexOf("\"");
            setSummery(KEY_SMS_SERVICE_CENTER, summary.substring(1, end));
        }
    }

    public static int getSubIdFromSlotId(int slotId) {
        Log.d(TAG, "getSubIdFromSlotId slotId:" + slotId);
        int[] subId = SubscriptionManager.getSubId(slotId);
        Log.d(TAG, "getSubIdFromSlotId subId = " + subId[0]);
        return subId[0];
    }

    public void getPrlVersion() {
        UiccController controller = UiccController.getInstance();
        if (controller != null) {
            IccFileHandler fh = controller.getIccFileHandler(mSlotId,
                    UiccController.APP_FAM_3GPP2);
            if (fh != null) {
                fh.loadEFTransparent(IccConstants.EF_CSIM_PRL, NUM_BYTES_PRL_ID,
                    mHandler.obtainMessage(GET_MESSAGE_EF_PRL));
                return;
            }
        }

        getDefaultPrlVersion();
    }

    private boolean handleGetEFPrl(Message msg) {
        if (msg == null) {
            Log.d(TAG, "handleGetEFPrl, msg is null");
            return false;
        }
        AsyncResult ar = (AsyncResult) msg.obj;
        if (ar == null) {
            Log.d(TAG, "handleGetEFPrl, ar is null");
            return false;
        }
        byte[] data = (byte[]) ar.result;
        if (ar.exception == null) {
            if (data.length > NUM_BYTES_PRL_ID - 1) {
                int prlId = ((data[2] & 0xFF) << 8) | (data[3] & 0xFF);
                setSummery(KEY_PRL_VERSION, String.valueOf(prlId));

                SharedPreferences.Editor editor = getPreferenceManager()
                        .getSharedPreferences().edit();
                editor.putString(KEY_PRL_VERSION, String.valueOf(prlId));
                editor.commit();
                return true;
            }
        }

        return false;
    }

    private void getDefaultPrlVersion() {
        SharedPreferences sp = getPreferenceManager().getSharedPreferences();
        String prlVersion = mRes.getString(R.string.device_info_not_available);
        prlVersion = sp.getString(KEY_PRL_VERSION, prlVersion);
        Log.d(TAG, "prlVersionSummery gsm is :" + prlVersion);
        setSummery(KEY_PRL_VERSION, prlVersion);
    }

    private boolean isCdmaMode() {
        return mPhone.getPhoneType() == TelephonyManager.PHONE_TYPE_CDMA;
    }
}
