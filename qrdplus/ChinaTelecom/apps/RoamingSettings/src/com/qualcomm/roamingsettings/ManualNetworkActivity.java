/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.roamingsettings;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.OperatorInfo;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.qualcomm.qcrilhook.IQcRilHook;
import com.qualcomm.qcrilhook.QcRilHook;
import com.qualcomm.qcrilhook.QcRilHookCallback;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ManualNetworkActivity extends Activity implements OnClickListener {
    private static final String TAG = "CTIRManualNetworkActivity";

    // Events definition
    private static final int EVENT_GET_NETWORK_TYPE = 0;
    private static final int EVENT_SET_NETWORK_TYPE = 1;
    private static final int EVENT_CDMA_ENABLED = 2;
    private static final int EVENT_NETWORK_RETRY_SCAN = 3;
    private static final int EVENT_NETWORK_SCAN_COMPLETED = 4;
    private static final int EVENT_NETWORK_SELECTION_DONE = 5;
    private static final int EVENT_AUTO_SELECT_DONE = 6;
    private static final int EVENT_REFRESH_NETWORK = 7;

    private static final int EVENT_INVALID = 10;
    private static final int EVENT_DATA_DISABLED = 11;
    private static final int EVENT_AVOID_CURRENT_NETWORK = 12;
    private static final int EVENT_AVOID_CURRENT_NETWORK_COMPLETED = 13;
    private static final int EVENT_CLEAR_AVOIDANCE_LIST = 14;
    private static final int EVENT_CLEAR_AVOIDANCE_LIST_COMPLETED = 15;
    private static final int EVENT_GET_AVOIDANCE_LIST = 16;
    private static final int EVENT_GET_AVOIDANCE_LIST_COMPLETED = 17;
    private static final int EVENT_UNABLE_TO_DISABLE_DATA = 18;
    private static final int EVENT_REMOVE_3GPP2_RAT = 19;
    private static final int EVENT_ADD_3GPP2_RAT = 20;
    private static final int EVENT_SET_MODE_PREF = 21;

    private static final int SCAN_RETRY_COUNT = 5;
    private static final int MESSAGE_DELAY_MILLIS = 2000;
    private static final int BACK_KEY_CMD = 99;

    private Button mGsmOpt;
    private TextView mGsmTitle;
    private TextView mEffectTip;
    private TextView mGsmLoading;
    private ListView mGsmListView;
    private LinearLayout mSubGsmLayout;
    private ProgressDialog mProgressDialog;

    //map of network controls to the network data.
    private HashMap<Integer, OperatorInfo> mNetworkMap;
    private List<Map<String, Object>> mMapList = null;
    private Phone mPhone = null;

    private int mSubscription;
    private static int mNetworkMode = 0;
    private boolean mIsMultiSimEnabled;
    private String mNetworkName;
    //GSM Manual network
//    private String preNOGSMManual;
//    private String postNOGSMManual;
    //CDMA AVOID CUR NWK
    private String preNOCDMAManual;
    private String postNOCDMAManual;

    private CdmaConfig mCdmaConfig = null;
    private boolean registerServiceState = false;
    private boolean avoidCurNwk = false;
    private String initStateNumeric;
    private int initVoiceState;
    private boolean AvoidShowFailure = false;

    private boolean CDMAManualShowSuccess = false;
    private boolean CDMAManualShowFailure = false;
    private static int sNetworkScanCount = 0;
    private static boolean isManual = false;

    /**
     * Local handler to receive the network query compete callback from the RIL.
     */
    Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar;

            switch (msg.what) {
            case EVENT_REMOVE_3GPP2_RAT:
                Log.v(TAG, "handler EVENT_REMOVE_3GPP2_RAT");
                ar = (AsyncResult) msg.obj;
                int prefMode;
                if (ar.exception == null) {
                    prefMode = ((int[]) ar.result)[0];
                } else {
                    // There is some exception when get the network type.
                    Log.w(TAG, "GET_MODE_PREF Exception: " + ar.exception);

                    // Try to use the system prop to get the network type.
                    try {
                        prefMode = TelephonyManager.getIntAtIndex(
                                mPhone.getContext().getContentResolver(),
                                Settings.Global.PREFERRED_NETWORK_MODE,
                                mSubscription);
                    } catch (SettingNotFoundException e) {
                        Log.e(TAG, "Catch the SettingNotFoundException:" + e);
                        break;
                    }
                }
                Message removeRat = mHandler
                        .obtainMessage(EVENT_SET_MODE_PREF);
                removeRat.arg1 = mSubscription;
                if (prefMode == Phone.NT_MODE_GLOBAL) {
                    removeRat.arg2 = Phone.NT_MODE_WCDMA_PREF;
                    mPhone.setPreferredNetworkType(Phone.NT_MODE_WCDMA_PREF, removeRat);
                } else if (prefMode == Phone.NT_MODE_LTE_CDMA_EVDO_GSM_WCDMA) {
                    removeRat.arg2 = Phone.NT_MODE_LTE_GSM_WCDMA;
                    mPhone.setPreferredNetworkType(Phone.NT_MODE_LTE_GSM_WCDMA, removeRat);
                }
                break;

            case EVENT_ADD_3GPP2_RAT:
                Log.v(TAG, "handler EVENT_ADD_3GPP2_RAT");
                ar = (AsyncResult) msg.obj;
                int prefNetworkMode;
                if (ar.exception == null) {
                    prefNetworkMode = ((int[]) ar.result)[0];
                } else {
                    // There is some exception when get the network type.
                    Log.w(TAG, "GET_MODE_PREF Exception: " + ar.exception);

                    // Try to use the system prop to get the network type.
                    try {
                        prefNetworkMode = TelephonyManager.getIntAtIndex(
                                mPhone.getContext().getContentResolver(),
                                Settings.Global.PREFERRED_NETWORK_MODE,
                                mSubscription);
                    } catch (SettingNotFoundException e) {
                        Log.e(TAG, "Catch the SettingNotFoundException:" + e);
                        break;
                    }
                }
                Message addRat = mHandler
                        .obtainMessage(EVENT_SET_MODE_PREF);
                addRat.arg1 = mSubscription;
                if (prefNetworkMode == Phone.NT_MODE_WCDMA_PREF) {
                    addRat.arg2 = Phone.NT_MODE_GLOBAL;
                    mPhone.setPreferredNetworkType(Phone.NT_MODE_GLOBAL, addRat);
                } else if (prefNetworkMode == Phone.NT_MODE_LTE_GSM_WCDMA) {
                    addRat.arg2 = Phone.NT_MODE_LTE_CDMA_EVDO_GSM_WCDMA;
                    mPhone.setPreferredNetworkType(Phone.NT_MODE_LTE_CDMA_EVDO_GSM_WCDMA, addRat);
                }
                break;

            case EVENT_SET_MODE_PREF:
                Log.v(TAG, "handler EVENT_SET_MODE_PREF");
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Log.v(TAG, "SET_MODE_PREF success, networkMode: " + msg.arg2);
                } else {
                    Log.w(TAG, "SET_MODE_PREF Exception: " + ar.exception);
                }
                break;

            case EVENT_GET_NETWORK_TYPE:
                Log.v(TAG, "handler EVENT_GET_NETWORK_TYPE");
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    int networkMode = ((int[]) ar.result)[0];
                    if (networkMode >= Phone.NT_MODE_WCDMA_PREF
                            && networkMode <= Phone.NT_MODE_LTE_WCDMA) {
                        mNetworkMode = getDisplayedNetworkMode(networkMode);
                    } else {
                        // The network is not an accept value, we will set the
                        // network to default.
                        Log.w(TAG, "The network(" + networkMode
                            + ") is not an accept value, we will set the network to default.");
                        Message setMsg = mHandler
                                .obtainMessage(EVENT_SET_NETWORK_TYPE);
                        setMsg.arg1 = mSubscription;
                        setMsg.arg2 = Phone.NT_MODE_GLOBAL;
                        mPhone.setPreferredNetworkType(Phone.NT_MODE_GLOBAL, setMsg);
                    }
                } else {
                    // There is some exception when get the network type.
                    Log.w(TAG, "GET_NETWORK_TYPE Exception: " + ar.exception);

                    // Try to use the system prop to get the network type.
                    try {
                        int networkMode = TelephonyManager.getIntAtIndex(
                                mPhone.getContext().getContentResolver(),
                                Settings.Global.PREFERRED_NETWORK_MODE,
                                mSubscription);
                        mNetworkMode = getDisplayedNetworkMode(networkMode);
                    } catch (SettingNotFoundException e) {
                        Log.e(TAG, "Catch the SettingNotFoundException:" + e);
                    }
                }
                break;

            case EVENT_SET_NETWORK_TYPE:
                Log.v(TAG, "handler EVENT_SET_NETWORK_TYPE");
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Log.v(TAG, "SET_NETWORK_TYPE success, networkMode: " + msg.arg2);
                    mNetworkMode = msg.arg2;
                    if (mNetworkMode == Phone.NT_MODE_GSM_ONLY
                        || mSubscription > PhoneConstants.SUB1) {
                        Log.v(TAG, "networMode = GSM or sub > " + PhoneConstants.SUB1);
                        TelephonyManager.putIntAtIndex(
                                mPhone.getContext().getContentResolver(),
                                Settings.Global.PREFERRED_NETWORK_MODE,
                                mSubscription, mNetworkMode);
                    }
                    selectNetworkMode(mNetworkMode);
                } else {
                    Log.w(TAG, "SET_NETWORK_TYPE Exception: " + ar.exception);

                    // re get the network type.
                    Message get = mHandler.obtainMessage(EVENT_GET_NETWORK_TYPE);
                    get.arg1 = mSubscription;
                    mPhone.getPreferredNetworkType(get);
                }
                break;

            case EVENT_NETWORK_RETRY_SCAN:
                mPhone.getAvailableNetworks(
                        mHandler.obtainMessage(EVENT_NETWORK_SCAN_COMPLETED));
                break;

            case EVENT_NETWORK_SCAN_COMPLETED:
                Log.v(TAG, "EVENT_NETWORK_SCAN_COMPLETED ");
                loadNetworkList((AsyncResult) msg.obj);
                break;

            case EVENT_NETWORK_SELECTION_DONE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception != null) {
                    Log.e(TAG, "CTIR EVENT_NETWORK_SELECTION_DONE: fail! will search" +
                            " network automatic");
                    cancelProgressDialog();
                    String msgStr = getString(R.string.network_select_failure_info,
                            mNetworkName);
                    mPhone.getPreferredNetworkType(mHandler.obtainMessage(EVENT_ADD_3GPP2_RAT));
                    showNetworkSelectResultDialog(true, msgStr);
                } else {
                    if (mSubscription > PhoneConstants.SUB1) {
                        SharedPreferences userInfo = getSharedPreferences("user_info", 0);
                        userInfo.edit().putBoolean("manual_gsm", true).commit();
                    }
                    mPhone.getAvailableNetworks(
                            mHandler.obtainMessage(EVENT_REFRESH_NETWORK));
                }
                break;

            case EVENT_AUTO_SELECT_DONE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception != null) {
                    Log.w(TAG, "automatic network selection Exception: " + ar.exception);
                    cancelProgressDialog();
                    String msgStr = getString(R.string.network_auto_failure_info);
                    showNetworkSelectResultDialog(false, msgStr);
                } else {
                    Log.v(TAG, "automatic network selection: succeeded!");
                    if (mSubscription > PhoneConstants.SUB1) {
                        SharedPreferences userInfo = getSharedPreferences("user_info", 0);
                        userInfo.edit().putBoolean("manual_gsm", false).commit();
                    }
                    mPhone.getAvailableNetworks(
                            mHandler.obtainMessage(EVENT_REFRESH_NETWORK));
                }
                break;

            case EVENT_REFRESH_NETWORK:
                Log.v(TAG, "handler EVENT_REFRESH_NETWORK");
                refreshNetworkList((AsyncResult) msg.obj);
                break;

            case EVENT_DATA_DISABLED:
                mCdmaConfig.handleDataDisabled(msg);
                break;

            case EVENT_UNABLE_TO_DISABLE_DATA:
                mCdmaConfig.setNextReqEvent(EVENT_INVALID);
                Toast.makeText(getApplicationContext(), R.string.unable_to_disable_data,
                        Toast.LENGTH_SHORT).show();
                break;

            case EVENT_AVOID_CURRENT_NETWORK:
                mCdmaConfig.handleAvoidCurNwk(msg);
                break;

            case EVENT_AVOID_CURRENT_NETWORK_COMPLETED:
                Log.d(TAG, "Cdma Avoid Current Network Command Completed.");
                break;

            case EVENT_CLEAR_AVOIDANCE_LIST:
                mCdmaConfig.handleClearAvoidanceList(msg);
                break;

            case EVENT_CLEAR_AVOIDANCE_LIST_COMPLETED:
                Log.d(TAG, "Cdma Clear Avoidandce List Command Completed.");
                break;

            case EVENT_CDMA_ENABLED:
                Log.d(TAG, "will enable CDMA mode, current networkMode: " + mNetworkMode);
                mCdmaConfig = new CdmaConfig(getApplicationContext());
                if (mNetworkMode == Phone.NT_MODE_GSM_ONLY) {
                    mCdmaConfig.registerPhoneStateListener();
                }
                break;

            default:
                Log.d(TAG, "Do nothing.");
                break;
            }
        }
    };

    @Override
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        setContentView(R.layout.manual_network);

        mSubscription = getIntent()
                .getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, PhoneConstants.SUB1);
        mIsMultiSimEnabled = TelephonyManager.getDefault().isMultiSimEnabled();
        if (mIsMultiSimEnabled) {
            mPhone = PhoneFactory.getPhone(mSubscription);
        } else {
            mPhone = PhoneFactory.getDefaultPhone();
        }

        if (mSubscription == PhoneConstants.SUB1) {
            // init the network mode
            try {
                if (mPhone != null) {
                    mNetworkMode = TelephonyManager.getIntAtIndex(mPhone
                            .getContext().getContentResolver(),
                            Settings.Global.PREFERRED_NETWORK_MODE, mSubscription);
                    mNetworkMode = getDisplayedNetworkMode(mNetworkMode);
                }
            } catch (SettingNotFoundException e) {
                Log.e(TAG, "Catch the SettingNotFoundException:" + e);
            }
        }

        initManualNetworkView();
    }

    private void initManualNetworkView() {
        String slotNumber = getResources().getStringArray(
                R.array.slot_number)[mSubscription];
        String titleName = getString(R.string.slot_name, slotNumber);
        titleName += getString(R.string.manual_select_network);
        setTitle(titleName);

        mGsmOpt = (Button) findViewById(R.id.bt_gsm_opt);
        mGsmOpt.setOnClickListener(this);
        mGsmTitle = (TextView) findViewById(R.id.tv_gsm_title);
        mEffectTip = (TextView) findViewById(R.id.tv_current_effect);
        mGsmLoading = (TextView) findViewById(R.id.tv_gsm_loading);
        mGsmListView = (ListView) findViewById(R.id.lv_gsm_list);
        mSubGsmLayout = (LinearLayout) findViewById(R.id.layout_sub_gsm);

        mNetworkMap = new HashMap<Integer, OperatorInfo>();
        mMapList = new ArrayList<Map<String, Object>>();
        mGsmListView.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> listview, View view, int index,
                    long lIndex) {
//                mNetworkName = listview.getItemAtPosition(index).toString();
                isManual = true;
                mNetworkName = getNetworkTitle(mNetworkMap.get(index));
                Log.d(TAG, "select item name string: " + mNetworkName);
                String msgStr = getString(R.string.register_on_network, mNetworkName);
                showProgressDailog(msgStr);

                Message msg = mHandler.obtainMessage(EVENT_NETWORK_SELECTION_DONE);
                mPhone.selectNetworkManually(mNetworkMap.get(index), msg);
            }
        });

        mPhone.getPreferredNetworkType(mHandler.obtainMessage(EVENT_REMOVE_3GPP2_RAT));
        if (mSubscription > PhoneConstants.SUB1) {
            Log.e(TAG, "gsm network query when startup");
            mSubGsmLayout.setVisibility(View.VISIBLE);
            mGsmTitle.setVisibility(View.VISIBLE);
            mGsmOpt.setVisibility(View.GONE);
            mEffectTip.setVisibility(View.GONE);

            mPhone.getAvailableNetworks(
                    mHandler.obtainMessage(EVENT_NETWORK_SCAN_COMPLETED));

            showProgressDailog(getString(R.string.searching_gsm_network_title));
        }
    }

    @Override
    public void onBackPressed() {
        if (mSubscription == PhoneConstants.SUB1) {
            mNetworkMode = Phone.NT_MODE_GLOBAL;
            TelephonyManager.putIntAtIndex(
                   mPhone.getContext().getContentResolver(),
                   Settings.Global.PREFERRED_NETWORK_MODE,
                   mSubscription, mNetworkMode);
        }
        if (mCdmaConfig != null) {
            Log.v(TAG, "mCdmaConfig != null, unRegisterPhoneStateListener");
            mCdmaConfig.unRegisterPhoneStateListener();
            mCdmaConfig = null;
        }
        super.onBackPressed();
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onPause() {
        cancelProgressDialog();
        super.onPause();
    }

    private void selectNetworkAutomatic() {
        Log.v(TAG, "automatic select network");
        isManual = false;
        showProgressDailog(getString(R.string.register_automatically));
        Message msg = mHandler.obtainMessage(EVENT_AUTO_SELECT_DONE);
        mPhone.setNetworkSelectionModeAutomatic(msg);
    }


    private void selectNetworkMode(int modeType) {
        Log.v(TAG, "select network mode: " + modeType);

        if (modeType == Phone.NT_MODE_GSM_ONLY) {
            mSubGsmLayout.setVisibility(View.VISIBLE);
            mGsmListView.setVisibility(View.GONE);
            mPhone.getAvailableNetworks(
                    mHandler.obtainMessage(EVENT_NETWORK_SCAN_COMPLETED));
            showProgressDailog(getString(R.string.searching_gsm_network_title));
        }
    }

    private void loadNetworkList(AsyncResult ar) {
        if (ar == null) {
            Log.v(TAG, "AsyncResult is null.");
            mGsmLoading.setText(R.string.empty_networks_list);
            cancelProgressDialog();
            return;
        }

        if (ar.exception != null) {
            Log.e(TAG, "while querying available networks: " + sNetworkScanCount);
            if (sNetworkScanCount++ < SCAN_RETRY_COUNT) {
                mHandler.sendMessageDelayed(mHandler.obtainMessage(EVENT_NETWORK_RETRY_SCAN),
                        MESSAGE_DELAY_MILLIS);
            } else {
                Log.e(TAG, "error while querying available networks, finish");
                mGsmLoading.setText(R.string.empty_networks_list);
                cancelProgressDialog();
                sNetworkScanCount = 0;
            }
        } else {
            Log.v(TAG, "now show gsm network list");
            cancelProgressDialog();
            sNetworkScanCount = 0;
            ArrayList<OperatorInfo> networkList = (ArrayList<OperatorInfo>) ar.result;
            if (networkList != null) {
                mNetworkMap.clear();
                mMapList.clear();
                int index = 0;

                for (OperatorInfo ni : networkList) {
                    String itemTitle = getNetworkTitle(ni);
                    Log.e(TAG, "Found GSM Network = " + itemTitle + " Numeric = "
                            + ni.getOperatorNumeric() + " State: " + ni.getState());
                    mNetworkMap.put(index++, ni);
                    Map<String, Object> data = new HashMap<String, Object>();
                    data.put("gsm_text", itemTitle);
                    if (ni.getState() == OperatorInfo.State.CURRENT) {
                        data.put("gsm_icon", R.drawable.btn_check_on_roaming);
                    }
                    mMapList.add(data);
                }
                SimpleAdapter adapter = new SimpleAdapter(ManualNetworkActivity.this,
                        mMapList, R.layout.gsm_list_item, new String[]{"gsm_text", "gsm_icon"},
                        new int[]{R.id.gsmtext, R.id.gsmicon});
                mSubGsmLayout.setVisibility(View.VISIBLE);
                mGsmListView.setVisibility(View.VISIBLE);
                mGsmListView.setAdapter(adapter);

                if (mSubscription == PhoneConstants.SUB1) {
                    selectNetworkAutomatic();
                }
            } else {
                Log.v(TAG, "empty networks list");
                mGsmLoading.setText(R.string.empty_networks_list);
            }
        }
    }

    private void refreshNetworkList(AsyncResult ar) {
        cancelProgressDialog();
        if (ar.exception != null) {
            Log.e(TAG, "refresh network list failure");
        } else {
            Log.v(TAG, "refresh network list success");
            ArrayList<OperatorInfo> networkList = (ArrayList<OperatorInfo>) ar.result;
            if (networkList != null) {
                mNetworkMap.clear();
                mMapList.clear();
                int index = 0;

                for (OperatorInfo ni : networkList) {
                    String itemTitle = getNetworkTitle(ni);
                    Log.v(TAG, "refresh GSM Network = " + itemTitle + " Numeric = "
                            + ni.getOperatorNumeric() + " State: " + ni.getState());
                    mNetworkMap.put(index++, ni);
                    Map<String, Object> data = new HashMap<String, Object>();
                    data.put("gsm_text", itemTitle);
                    if (isManual) {
                        if (mNetworkName.equals(itemTitle)) {
                            data.put("gsm_icon", R.drawable.btn_check_on_roaming);
                        }
                    } else {
                        if (ni.getState() == OperatorInfo.State.CURRENT) {
                            data.put("gsm_icon", R.drawable.btn_check_on_roaming);
                        }
                    }
                    mMapList.add(data);
                }
                SimpleAdapter adapter = new SimpleAdapter(ManualNetworkActivity.this,
                        mMapList, R.layout.gsm_list_item, new String[]{"gsm_text", "gsm_icon"},
                        new int[]{R.id.gsmtext, R.id.gsmicon});
                mGsmListView.setAdapter(adapter);

                String msgStr;
                if (isManual) {
                    msgStr = getString(R.string.network_select_success_info, mNetworkName);
                } else {
                    String manualNetwork = getNetworkString(
                            TelephonyManager.getDefault()
                            .getNetworkOperatorName(mSubscription),
                            R.array.original_carrier_names, R.array.local_carrier_names);
                    msgStr = getString(R.string.network_select_success_info, manualNetwork);
                }
                showNetworkSelectResultDialog(false, msgStr);
            } else {
                Log.v(TAG, "empty networks list");
                mGsmLoading.setText(R.string.empty_networks_list);
            }
        }
    }

    private String getNetworkTitle(OperatorInfo ni) {
        if (!TextUtils.isEmpty(ni.getOperatorAlphaLong())) {
            return ni.getOperatorAlphaLong();
        } else if (!TextUtils.isEmpty(ni.getOperatorAlphaShort())) {
            return ni.getOperatorAlphaShort();
        } else {
            return ni.getOperatorNumeric();
        }
    }

    @Override
    public void onClick(View v) {

        int newNetworkMode = mNetworkMode;
        if (mNetworkMode != Phone.NT_MODE_GSM_ONLY) {
            Log.v(TAG, "set mNetworkMode = CDMA" +
                    " if current networkMode was CDMA/GLOBAL");
            mNetworkMode = Phone.NT_MODE_CDMA;
        }

        switch (v.getId()) {
        case R.id.bt_gsm_opt:
            Log.v(TAG, "do GSM Manual search");
            newNetworkMode = Phone.NT_MODE_GSM_ONLY;
            break;
        }

        Log.v(TAG, "will change network type pre = "+ mNetworkMode + " new = "
                + newNetworkMode);
        if (newNetworkMode != mNetworkMode) {
            Message msg = mHandler.obtainMessage(EVENT_SET_NETWORK_TYPE);
            msg.arg1 = mSubscription;
            msg.arg2 = newNetworkMode;
            mPhone.setPreferredNetworkType(newNetworkMode, msg);
        }
    }

    /**
     * We need to transform the network mode for there only three items to show.
     *
     * @param networkMode the actual network mode
     * @return the network mode will be displayed
     */
    private int getDisplayedNetworkMode(int networkMode) {
        switch (networkMode) {
        case Phone.NT_MODE_GLOBAL:
        case Phone.NT_MODE_LTE_CDMA_EVDO_GSM_WCDMA:
            return Phone.NT_MODE_GLOBAL;
        case Phone.NT_MODE_WCDMA_PREF:
        case Phone.NT_MODE_GSM_ONLY:
        case Phone.NT_MODE_WCDMA_ONLY:
        case Phone.NT_MODE_GSM_UMTS:
        case Phone.NT_MODE_LTE_GSM_WCDMA:
        case Phone.NT_MODE_LTE_WCDMA:
            return Phone.NT_MODE_GSM_ONLY;
        case Phone.NT_MODE_CDMA:
        case Phone.NT_MODE_CDMA_NO_EVDO:
        case Phone.NT_MODE_EVDO_NO_CDMA:
        case Phone.NT_MODE_LTE_CDMA_AND_EVDO:
        case Phone.NT_MODE_LTE_ONLY:
        default:
            return Phone.NT_MODE_CDMA;
        }
    }

    private final String getNetworkString(String originalString,
            int originNamesId, int localNamesId) {
        String[] origNames = getResources().getStringArray(originNamesId);
        String[] localNames = getResources().getStringArray(localNamesId);
        for (int i = 0; i < origNames.length; i++) {
            if (origNames[i].equalsIgnoreCase(originalString)) {
                return getString(getResources().getIdentifier(localNames[i],
                        "string", "android"));
            }
        }
        return originalString;
    }

    private class CdmaConfig {
        private int mNextReqEvent = EVENT_INVALID;
        private boolean mIsQcRilHookReady = false;
        private boolean mIsRegisterLinstener = false;
        private int mListenNetworkMode = 0;

        private Context mContext;
        private QcRilHook mQcRilHook;
        private TelephonyManager mTelephonyManager;

        public CdmaConfig(Context context) {
            mContext = context;
            mQcRilHook = new QcRilHook(mContext, mQcrilHookCb);
        }

        public void registerPhoneStateListener() {
            if (mIsRegisterLinstener) {
                return;
            }
            mIsRegisterLinstener = true;
            registerServiceState = true;
            mTelephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
            if (mTelephonyManager != null) {
                mTelephonyManager.listen(mPhoneStateListener,
                        PhoneStateListener.LISTEN_SERVICE_STATE);
            }
        }

        public void unRegisterPhoneStateListener() {
            mIsRegisterLinstener = false;
            mTelephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
            if (mTelephonyManager != null) {
                mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
            }
        }

        private QcRilHookCallback mQcrilHookCb = new QcRilHookCallback() {
            public void onQcRilHookReady() {
                mIsQcRilHookReady = true;
            }
            public void onQcRilHookDisconnected() {
                mIsQcRilHookReady = false;
            }
        };

        /**
         * Called when the "Avoid the current network" option is selected
         */
        private void selectAvoidCurNetwork() {
            new AlertDialog.Builder(ManualNetworkActivity.this)
                    .setTitle(R.string.pref_cdma_choose_title)
                    .setMessage(R.string.confirm_avoid)
                    .setNegativeButton(R.string.cancel_btn, null)
                    .setPositiveButton(R.string.ok_btn,
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    mHandler.sendMessage(mHandler
                                            .obtainMessage(EVENT_AVOID_CURRENT_NETWORK));
                                }
                            }).show();
        }

        /**
         * Called when the "Clear the avoidance network list" option is selected
         */
        private void clearNetworkAvoidList() {
            new AlertDialog.Builder(ManualNetworkActivity.this)
                    .setTitle(R.string.pref_cdma_choose_title)
                    .setMessage(R.string.confirm_clear)
                    .setNegativeButton(R.string.cancel_btn, null)
                    .setPositiveButton(R.string.ok_btn,
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    mHandler.sendMessage(mHandler
                                            .obtainMessage(EVENT_CLEAR_AVOIDANCE_LIST));
                                }
                            }).show();
        }

        /**
         * Called when the "View the avoidance network list" option is selected
         */
        private void viewNetworkAvoidList() {
            mHandler.sendMessage(mHandler.obtainMessage(EVENT_GET_AVOIDANCE_LIST));
        }

        /**
         * Process a Data Disabled event
         *
         * @param msg : Message
         */
        private void handleDataDisabled(Message msg) {
            if (getNextReqEvent() != EVENT_INVALID) {
                Log.d(TAG, "Data disconnected. Ready to send the next request.");
                mHandler.sendMessage(mHandler.obtainMessage(getNextReqEvent()));

                setNextReqEvent(EVENT_INVALID);
            }
        }

        /**
         * Process a Avoid Current Network Event
         *
         * @param msg : Message
         */
        private void handleAvoidCurNwk(Message msg) {
            Log.d(TAG, "Sending RIL OEM HOOK Cdma Avoid Current Network message.");
             avoidCurNwk = true;
            int resStrId = 0;
            boolean result = false;
            if (!mIsQcRilHookReady) {
                // return if the QcRilHook isn't ready
                return;
            }
            int preNTCDMAManual = TelephonyManager.getDefault()
                    .getNetworkType(PhoneConstants.SUB1);
            int prePTCDMAManual = TelephonyManager.getDefault()
                    .getPhoneType(PhoneConstants.SUB1);

            Log.e(TAG, "preCDMA Avoid current = " + preNTCDMAManual
                    + " pre PT = " + prePTCDMAManual + " pre NO = " + preNOCDMAManual);
            result = mQcRilHook.qcRilCdmaAvoidCurNwk();
            if (result == false) {
                Log.e(TAG, "qcRilCdmaAvoidCurNwk command failed.");
                showNetworkSelectResultDialog(false,
                        getString(R.string.avoid_cur_nwk_failed));
            } else {
               showNetworkSelectResultDialog(false,
                        getString(R.string.avoid_cur_nwk_succeeded));
            }
            int postNTCDMAManual = TelephonyManager.getDefault()
                    .getNetworkType(PhoneConstants.SUB1);
            int postPTCDMAManual = TelephonyManager.getDefault()
                    .getPhoneType(PhoneConstants.SUB1);
            postNOCDMAManual = TelephonyManager.getDefault()
                    .getNetworkOperatorForSubscription(PhoneConstants.SUB1);
            Log.e(TAG, "postCDMAManul NT = " + postNTCDMAManual
                    + " post PT = " + postPTCDMAManual + " post NO = " + postNOCDMAManual);

            mHandler.sendMessage(mHandler
                    .obtainMessage(EVENT_AVOID_CURRENT_NETWORK_COMPLETED));
        }

        /**
         * Process a Clear Avoidance Network List Event
         *
         * @param msg : Message
         */
        private void handleClearAvoidanceList(Message msg) {
            Log.d(TAG, "Sending RIL OEM HOOK Cdma Clear Avoidance List message.");

            boolean result = false;
            if (!mIsQcRilHookReady) {
                // return if the QcRilHook isn't ready
                return;
            }
            result = mQcRilHook.qcRilCdmaClearAvoidanceList();
            if (result == false) {
                Log.e(TAG, "qcRilCdmaClearAvoidanceList command failed.");
                Toast.makeText(mContext, R.string.clear_nwk_list_failed,
                        Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(mContext, R.string.clear_nwk_list_succeeded,
                        Toast.LENGTH_SHORT).show();
            }

            mHandler.sendMessage(mHandler
                    .obtainMessage(EVENT_CLEAR_AVOIDANCE_LIST_COMPLETED));
        }

        /**
         * Process a Get Avoidance Network List Event
         *
         * @param msg : Message
         */
        private void handleGetAvoidanceList(Message msg) {
            Log.d(TAG, "Sending RIL OEM HOOK Cdma Get Avoidance List message.");

            byte[] result = null;
            if (!mIsQcRilHookReady) {
                // return if the QcRilHook isn't ready
                return;
            }
            result = mQcRilHook.qcRilCdmaGetAvoidanceList();
            if (result == null) {
                Log.e(TAG, "qcRilCdmaGetAvoidanceList command failed.");
            }

            mHandler.sendMessage(mHandler.obtainMessage(
                    EVENT_GET_AVOIDANCE_LIST_COMPLETED, (Object) result));
        }

        /**
         * Set which request event to be sent after the data is disabled.
         *
         * @param event : int
         */
        synchronized private void setNextReqEvent(int event) {
            mNextReqEvent = event;
        }

        /**
         * Get which request event to be sent after the data is disabled.
         *
         * @return int
         */
        synchronized private int getNextReqEvent() {
            return mNextReqEvent;
        }

        /**
         * We are listening to this because we want to disable the following option
         * when phone is not in service.
         *     - Avoid the current network
         */
        PhoneStateListener mPhoneStateListener = new PhoneStateListener(PhoneConstants.SUB1) {

            @Override
            public void onServiceStateChanged(ServiceState state) {
                if (mNetworkMode == Phone.NT_MODE_GSM_ONLY) {
                    Log.v(TAG, "serviceState, networkMode = GSM");
                    return;
                }
                String NumericSSC = state.getOperatorNumeric();
                int VoiceState = state.getVoiceRegState();
                String OperatorName = state.getOperatorAlphaShort();
                String RadioTech = state.rilRadioTechnologyToString(
                        state.getRilDataRadioTechnology());
                int SSCPT = TelephonyManager.getDefault().getPhoneType(PhoneConstants.SUB1);
                boolean isGsm = state.isGsm(state.getRilDataRadioTechnology());
                Log.e(TAG, "==onSSC== VoiceRegState = " + VoiceState + " Numeric= "
                        + NumericSSC + " RadioTech= " + RadioTech  + " isGsm= " + isGsm
                        + " SSCPT= " + SSCPT + " Name = " + OperatorName
                        + " preNOCDMAManual =" + preNOCDMAManual);
                if (registerServiceState){
                    registerServiceState = false;
                    initStateNumeric = NumericSSC;
                    initVoiceState = VoiceState;
                    Log.e(TAG, "return at the first register == init onServiceStateChanged" +
                            " == VoiceRegState = " + initVoiceState + " OperatorNumeric= "
                            + initStateNumeric);
                    return;
                }

                final String CHINA_TELECOM = "46003";
                int resStrId = 0;

                if (NumericSSC != null) {
                    Log.e(TAG, "CDMA Manual [preNO.equals(postNO)] = "
                            + NumericSSC.equals(preNOCDMAManual) + " CDMAManualShowSuccess = "
                            + CDMAManualShowSuccess);
                    if ((VoiceState == 0) && !(NumericSSC.equals(preNOCDMAManual))
                            && !CDMAManualShowSuccess && (SSCPT == 2) ) {
                        CDMAManualShowSuccess = true;
                        String networkStr = getNetworkString(
                                TelephonyManager.getDefault()
                                .getNetworkOperatorName(mSubscription),
                                R.array.original_carrier_names, R.array.local_carrier_names);

                        // can not get network operator name
                        if (TextUtils.isEmpty(networkStr)) {
                            Log.e(TAG, "return in the SSC cuz the networkStr = null" );
                            return;
                        }

                        resStrId = R.string.network_select_success_info;
                        if (networkStr.startsWith(CHINA_TELECOM)) {
                            networkStr = getString(R.string.carrier_names);
                        }
                        String msgStr = getString(resStrId, networkStr);
                        showNetworkSelectResultDialog(false, msgStr);
                    }
                }

                //show failure when CDMA Manual search failure to camp on CDMA network
                int newNetworkMode = getDisplayedNetworkMode(state.getVoiceNetworkType());
                Log.e(TAG, "onSSC newNetworkMode = " + newNetworkMode);
                if ((VoiceState == 1) && (SSCPT == 1) && !CDMAManualShowFailure ) {
                    CDMAManualShowFailure = true;
                    String networkStr = "CDMA";
                    // can not get network operator name
                    if (TextUtils.isEmpty(networkStr)) {
                        Log.e(TAG, "return in the SSC cuz the networkStr = null" );
                        return;
                    }

                    resStrId = R.string.network_select_failure_info;
                    String msgStr = getString(resStrId, networkStr);
                    Log.v(TAG, "show CDMA failure dialog");
                    showNetworkSelectResultDialog(false, msgStr);
                }
                cancelProgressDialog();
            }
        };
    }

    private void showProgressDailog(String msgStr) {
        Activity activity = ManualNetworkActivity.this;
        while (activity.getParent() != null) {
            activity = activity.getParent();
        }
        mProgressDialog = new ProgressDialog(activity);
        mProgressDialog.setMessage(msgStr);
        mProgressDialog.setCancelable(false);
        mProgressDialog.show();
    }

    private void cancelProgressDialog() {
        if (mProgressDialog != null) {
            mProgressDialog.dismiss();
            mProgressDialog = null;
        }
    }

    private void showNetworkSelectResultDialog(final boolean isAuto, String msgStr) {
        AlertDialog.Builder builder = new AlertDialog.Builder(ManualNetworkActivity.this);
        builder.setMessage(msgStr);
        builder.setPositiveButton(R.string.result_btn,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        if (isAuto) {
                            selectNetworkAutomatic();
                        }
                    }
                });
        builder.setCancelable(false).create().show();
    }
}
