/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.carrierconfigure;

import android.app.Activity;
import android.app.Fragment;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.qti.accesscache.ICarrierAccessCacheService;
import com.qualcomm.qti.carrierconfigure.Actions.ActionCallback;
import com.qualcomm.qti.carrierconfigure.Actions.SwitchCarrierTask;
import com.qualcomm.qti.carrierconfigure.Actions.UpdateNVItemsTask;
import com.qualcomm.qti.carrierconfigure.Carrier.EmptyPathException;
import com.qualcomm.qti.carrierconfigure.Carrier.NullServiceException;
import com.qualcomm.qti.carrierconfigure.Carrier.SwitchData;
import com.qualcomm.qti.carrierconfigure.Utils.MyAlertDialog;
import com.qualcomm.qti.carrierconfigure.Utils.MyNoticeDialog;
import com.qualcomm.qti.carrierconfigure.Utils.WaitDialog;
import com.qualcomm.qti.loadcarrier.ILoadCarrierService;

import java.util.ArrayList;

public class TriggerStartFragment extends Fragment implements OnClickListener,
        MyAlertDialog.OnAlertDialogButtonClick, MyNoticeDialog.OnNoticeDialogButtonClick,
        ActionCallback {
    private static final String TAG = "TriggerStartFragment";

    private static final String ARG_CARRIER_LIST = "carrier_list";

    private static final String RE_MNC = "(\\d{2,3})?";
    private static final String SEP_COMMA = ",";

    private static final int RETRY_DELAY = 500;
    private static final int RETRY_COUNT = 10;

    private static final int MSG_START_SWITCH = 0;
    private static final int MSG_SWITCH_ERROR = 1;

    private static final int UPDATE_ROW_NV_ITEMS_FINISH = 1;

    private int mMagicValue;
    private ArrayList<Carrier> mCarriers;

    private int mSubMask = 0;
    private String mConfigId = null;

    private LinearLayout mCarriersContainer;

    private int mRetryTimes = 0;
    private Carrier mSelectedCarrier;
    private Carrier mSwitchToDefaultCarrier = null;

    private ILoadCarrierService mService = null;
    private ICarrierAccessCacheService mAccessCacheService = null;
    private ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            if (Utils.DEBUG) Log.i(TAG, "Service Connected to " + name.getShortClassName());
            if (name.getShortClassName().equals(".LoadCarrierService")) {
                mService = ILoadCarrierService.Stub.asInterface(service);
            } else if (name.getShortClassName().equals(".CarrierAccessCacheService")) {
                mAccessCacheService = ICarrierAccessCacheService.Stub.asInterface(service);
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            if (name.getShortClassName().equals(".LoadCarrierService")) {
                mService = null;
            } else if (name.getShortClassName().equals(".CarrierAccessCacheService")) {
                mAccessCacheService = null;
            }
        }
    };

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == MSG_START_SWITCH) {
                if (Utils.DEBUG) Log.d(TAG, "Start switch! For user press yes.");
                // Show the "Please wait ..." dialog.
                WaitDialog wait = WaitDialog.newInstance();
                wait.show(getFragmentManager(), WaitDialog.TAG_LABEL);

                // Before start the switch action, need update the NV Items first. And after
                // the update action finished, will start the switch action.
                updateNVItems();
            } else if (msg.what == MSG_SWITCH_ERROR) {
                // There is some error, prompt one toast to alert the user.
                WaitDialog wait = (WaitDialog) getFragmentManager()
                        .findFragmentByTag(WaitDialog.TAG_LABEL);
                wait.dismiss();
                Toast.makeText(getActivity(), R.string.alert_switch_error, Toast.LENGTH_LONG)
                        .show();
            }
        }
    };

    public TriggerStartFragment() {
    }

    public static TriggerStartFragment newInstance(ArrayList<Carrier> carriers) {
        final TriggerStartFragment fragment = new TriggerStartFragment();

        final Bundle args = new Bundle(1);
        args.putParcelableArrayList(ARG_CARRIER_LIST, carriers);
        fragment.setArguments(args);

        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Try to bind the service.
        if (mServiceConnection != null) {
            if (mService == null) {
                // Bind the service to get the carriers stored in SD card.
                Intent intent = new Intent(ILoadCarrierService.class.getName());
                intent.setPackage(ILoadCarrierService.class.getPackage().getName());
                getActivity().bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);
            }
            if (mAccessCacheService == null) {
                // Bind the service to access cache dir.
                Intent intent = new Intent(ICarrierAccessCacheService.class.getName());
                intent.setPackage(ICarrierAccessCacheService.class.getPackage().getName());
                getActivity().bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);
            }
        }

        mCarriers = getArguments().getParcelableArrayList(ARG_CARRIER_LIST);
        mMagicValue = mCarriers.hashCode();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle bundle) {
        View rootView = inflater.inflate(R.layout.trigger_start_fragment, container, false);

        mCarriersContainer = (LinearLayout) rootView.findViewById(R.id.carriers_container_view);
        rootView.findViewById(R.id.btn_no).setOnClickListener(this);
        rootView.findViewById(R.id.btn_ask).setOnClickListener(this);

        TextView message = (TextView) rootView.findViewById(R.id.trigger_message);
        message.setText(isUninstallMode() ? R.string.trigger_uninstall_message
                : R.string.trigger_message);

        buildCarriersView(mCarriersContainer);

        return rootView;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        if (mServiceConnection != null) {
            getActivity().unbindService(mServiceConnection);
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_no:
                updateROWNVItems();
                break;
            case R.id.btn_ask:
                getActivity().setResult(Activity.RESULT_OK);
                getActivity().finish();
                break;
            default:
                if (v.getId() >= mMagicValue
                        && v.getId() < mMagicValue + mCarriers.size()) {
                    mSelectedCarrier = (Carrier) v.getTag();
                    if (Utils.DEBUG) Log.d(TAG, "Click the carrier " + mSelectedCarrier);
                    findMatchedSubscriptionId(mSelectedCarrier);
                    Log.d(TAG, "mSubMask=" + mSubMask + " mConfigId=" + mConfigId);
                    // Show the dialog to alert the user.
                    MyAlertDialog dialog = MyAlertDialog.newInstance(
                            this, R.string.alert_switch_title, R.string.alert_switch_text);
                    dialog.show(getFragmentManager(), MyAlertDialog.TAG_LABEL);
                }
                break;
        }
    }

    private void findMatchedSubscriptionId(Carrier carrier) {
        mConfigId = carrier.mName;
        mSubMask = 0;
        if (matchedByICC(carrier) || matchedByMCCMNCGID(carrier) || matchedByMCCMNC(carrier)
                || matchedBySPN(carrier) || matchedByMCC(carrier)) {
            Log.d(TAG, "Match success.");
        }

        // means matched by Dependency
        if (mSubMask == 0) {
            TelephonyManager telephonyManager = (TelephonyManager) getActivity().getSystemService(
                    Context.TELEPHONY_SERVICE);
            int phoneCount = telephonyManager.getPhoneCount();
            for (int i = 0; i < phoneCount; i++) {
                String mccMnc = telephonyManager.getSimOperator(SubscriptionManager.getSubId(i)[0]);
                if (!TextUtils.isEmpty(mccMnc)) {
                    mSubMask = mSubMask | ( 0x0001 << i);
                }
            }
        }
    }

    private boolean matchedByICC(Carrier carrier) {
        String carrierICC = carrier.getICC();
        if (TextUtils.isEmpty(carrierICC)) {
            return false;
        }

        boolean matched = false;
        TelephonyManager telephonyManager = (TelephonyManager) getActivity().getSystemService(
                Context.TELEPHONY_SERVICE);
        int phoneCount = telephonyManager.getPhoneCount();
        for (int i = 0; i < phoneCount; i++) {
            String icc = telephonyManager.getIccOperatorNumeric(SubscriptionManager.getSubId(i)[0]);
            if (!TextUtils.isEmpty(icc) && (carrierICC.equals(icc))) {
                Log.d(TAG, "slot " + i + " matched by ICC");
                mSubMask = mSubMask | ( 0x0001 << i);
                matched = true;
            }
        }
        return matched;
    }

    private boolean matchedByMCCMNCGID(Carrier carrier) {
        String carrierGID = carrier.getGID();
        String carrierMCCMNCString = carrier.getMCCMNC();
        if (TextUtils.isEmpty(carrierGID) || TextUtils.isEmpty(carrierMCCMNCString)) {
            return false;
        }

        boolean matched = false;
        String[] carrierMCCMNCs = carrierMCCMNCString.split(SEP_COMMA);
        TelephonyManager telephonyManager = (TelephonyManager) getActivity().getSystemService(
                Context.TELEPHONY_SERVICE);
        int phoneCount = telephonyManager.getPhoneCount();
        for (int i = 0; i < phoneCount; i++) {
            String gid = telephonyManager.getGroupIdLevel1(SubscriptionManager.getSubId(i)[0]);
            if (!TextUtils.isEmpty(gid) && (carrierGID.equals(gid))) {
                String mccmnc = telephonyManager.getSimOperator(SubscriptionManager.getSubId(i)[0]);
                if (!TextUtils.isEmpty(mccmnc)) {
                    for (String carrierMCCMNC : carrierMCCMNCs) {
                        if (mccmnc.equals(carrierMCCMNC)) {
                            Log.d(TAG, "slot " + i + " matched by GIDMCCMNC");
                            mSubMask = mSubMask | ( 0x0001 << i);
                            matched = true;
                            break;
                        }
                    }
                }
            }
        }
        return matched;
    }

    private boolean matchedByMCCMNC(Carrier carrier) {
        String carrierMCCMNCString = carrier.getMCCMNC();
        if (TextUtils.isEmpty(carrierMCCMNCString)) {
            return false;
        }

        boolean matched = false;
        String[] carrierMCCMNCs = carrierMCCMNCString.split(SEP_COMMA);
        TelephonyManager telephonyManager = (TelephonyManager) getActivity().getSystemService(
                Context.TELEPHONY_SERVICE);
        int phoneCount = telephonyManager.getPhoneCount();
        for (int i = 0; i < phoneCount; i++) {
            String mccmnc = telephonyManager.getSimOperator(SubscriptionManager.getSubId(i)[0]);
            if (!TextUtils.isEmpty(mccmnc)) {
                for (String carrierMCCMNC : carrierMCCMNCs) {
                    if (mccmnc.equals(carrierMCCMNC)) {
                        Log.d(TAG, "slot " + i + " matched by MCCMNC");
                        mSubMask = mSubMask | ( 0x0001 << i);
                        matched = true;
                        break;
                    }
                }
            }
        }
        return matched;
    }

    private boolean matchedBySPN(Carrier carrier) {
        String carrierSPN = carrier.getSPN();
        if (TextUtils.isEmpty(carrierSPN)) {
            return false;
        }

        boolean matched = false;
        TelephonyManager telephonyManager = (TelephonyManager) getActivity().getSystemService(
                Context.TELEPHONY_SERVICE);
        int phoneCount = telephonyManager.getPhoneCount();
        for (int i = 0; i < phoneCount; i++) {
            String spn = telephonyManager.getSimOperatorNameForSubscription(
                    SubscriptionManager.getSubId(i)[0]);
            if (!TextUtils.isEmpty(spn) && (carrierSPN.equals(spn))) {
                Log.d(TAG, "slot " + i + " matched by SPN");
                mSubMask = mSubMask | ( 0x0001 << i);
                matched = true;
            }
        }
        return matched;
    }

    private boolean matchedByMCC(Carrier carrier) {
        String carrierMCCMNCString = carrier.getMCCMNC();
        if (TextUtils.isEmpty(carrierMCCMNCString)) {
            return false;
        }

        boolean matched = false;
        TelephonyManager telephonyManager = (TelephonyManager) getActivity().getSystemService(
                Context.TELEPHONY_SERVICE);
        int phoneCount = telephonyManager.getPhoneCount();
        String[] carrierMCCMNCs = carrierMCCMNCString.split(SEP_COMMA);
        for (int i = 0; i < phoneCount; i++) {
            String mccmnc = telephonyManager.getSimOperator(SubscriptionManager.getSubId(i)[0]);
            if (!TextUtils.isEmpty(mccmnc)) {
                String mcc = mccmnc.substring(0, 3);
                for (String carrierMCCMNC : carrierMCCMNCs) {
                    if (mcc.equals(carrierMCCMNC)) {
                        Log.d(TAG, "slot " + i + " matched by MCC");
                        mSubMask = mSubMask | ( 0x0001 << i);
                        matched = true;
                        break;
                    }
                }
            }
        }
        return matched;
    }

    @Override
    public void onAlertDialogButtonClick(int which) {
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:
                // For user press OK, then try to switch the carrier.
                mHandler.sendEmptyMessage(MSG_START_SWITCH);
                break;
        }
    }

    @Override
    public void onNoticeDialogButtonClick(int dialogId) {
        switch (dialogId) {
            case UPDATE_ROW_NV_ITEMS_FINISH:
                getActivity().setResult(Activity.RESULT_OK);
                getActivity().finish();
                break;
            default:
                break;
        }
    }

    @Override
    public void onActionFinished(int requestCode) {
        switch (requestCode) {
            case Actions.REQUEST_UPDATE_NV_ITEMS:
                // After the update NV items action finished, start the switch action.
                startSwitchAction();
                break;
            case Actions.REQUEST_UPDATE_ROW_NV_ITEMS:
                // Save the carriers as not triggered.
                Carrier.saveAsNotTriggeredCarrier(mCarriers);
                // Pop up a notification dialog to let user reboot manually.
                MyNoticeDialog dialog = MyNoticeDialog.newInstance(this,
                        UPDATE_ROW_NV_ITEMS_FINISH, R.string.notice_row_title,
                        R.string.notice_row_text);
                dialog.show(getFragmentManager(), MyNoticeDialog.TAG_LABEL);
                break;
            case Actions.REQUEST_SWITCH_CARRIER:
                getActivity().setResult(Activity.RESULT_OK);
                getActivity().finish();
                break;
        }
    }

    @Override
    public void onActionError(int requestCode, int resultCode, Exception ex) {
        switch (requestCode) {
            case Actions.REQUEST_UPDATE_NV_ITEMS:
                // If the update action failed, just go on to start switch action for now.
                startSwitchAction();
                break;
            case Actions.REQUEST_UPDATE_ROW_NV_ITEMS:
                if (resultCode == Actions.RESULT_NO_MBN) {
                    Carrier.saveAsNotTriggeredCarrier(mCarriers);
                    getActivity().setResult(Activity.RESULT_OK);
                    getActivity().finish();
                }
                break;
            default:
                Log.e(TAG, "Get the request[ " + requestCode + "] error, ex is " + ex.getMessage());
                mHandler.sendEmptyMessage(MSG_SWITCH_ERROR);
                break;
        }
    }

    private void buildCarriersView(LinearLayout parentView) {
        ArrayList<String> carriers = Carrier.getCurrentCarriers();
        String currentCarrierName = Carrier.getCurrentCarriersName(carriers);

        travelCarriers(parentView, travelCarriers(parentView, currentCarrierName));
    }

    private boolean isUninstallMode() {
        Carrier currentCarrier = Carrier.getCurrentCarrierInstance();
        if (currentCarrier != null) {
            String currentCarrierDependency = currentCarrier.getDependency();
            if (!TextUtils.isEmpty(currentCarrierDependency)) {
                for (Carrier carrier : mCarriers) {
                    if (carrier.mName.equals(currentCarrierDependency)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    private String travelCarriers(LinearLayout parentView, String fullName) {
        if (fullName == null) return null;
        boolean needScanAgain = !Carrier.getBaseCarrierName(fullName).equals("Default");
        for (int i = 0; i < mCarriers.size(); i++) {
            Carrier carrier = mCarriers.get(i);
            if (!carrier.getBaseCarrierName().equals(Carrier.getSwitchCarrierName(fullName)))
                    continue;
            if (!needScanAgain && mSwitchToDefaultCarrier != null && carrier.mName.equals("Default"))
                    continue;

            // Init the button view.
            Button btn = new Button(getActivity());
            btn.setText(getCarrierDisplay(carrier));
            btn.setTextAppearance(getActivity(), android.R.attr.textAppearanceMedium);
            btn.setTag(carrier);
            btn.setId(mMagicValue + i);
            btn.setOnClickListener(this);
            if (carrier.mName.endsWith("2Default"))
                btn.setVisibility(View.GONE);
            else {
                btn.setEnabled(!(carrier.mName.equalsIgnoreCase(Carrier.getCurrentCarriersName(
                        Carrier.getCurrentCarriers()))));
            }

            // Add this button to parent view.
            parentView.addView(btn,
                    new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));

            if (!needScanAgain && carrier.getSwitchCarrierName().equals("Default")
                     && !"Default".equals(fullName)) {
                needScanAgain = true;
                mSwitchToDefaultCarrier = carrier;
            }
        }
        if (needScanAgain && mSwitchToDefaultCarrier != null)
            return mSwitchToDefaultCarrier.getSwitchCarrierName();
        else return null;

    }

    private String getCarrierDisplay(Carrier carrier) {
        if (carrier == null) return null;

        String target = carrier.getTarget();
        String brand =  carrier.getBrand();

        String display = TextUtils.isEmpty(brand) ? carrier.getTopCarrierTitle() : brand;
        if (!TextUtils.isEmpty(target)) {
            display = display + " - " + target;
        }
        return display;
    }

    private void startSwitchAction() {
        try {
            SwitchData data = mSelectedCarrier.getSwitchData(mService);
            SwitchCarrierTask switchTask = new SwitchCarrierTask(getActivity(),
                    Actions.REQUEST_SWITCH_CARRIER, this, mAccessCacheService);
            if (mSwitchToDefaultCarrier != null && !mSelectedCarrier.getBaseCarrierName()
                    .equals(Carrier.getCurrentCarriersName(Carrier.getCurrentCarriers())))
                switchTask.execute(mSwitchToDefaultCarrier.getSwitchData(mService), data);
            else switchTask.execute(data);
        } catch (NullServiceException ex) {
            Log.e(TAG, "Catch the NullServiceException: " + ex.getMessage());
            if (mRetryTimes <= RETRY_COUNT) {
                mRetryTimes = mRetryTimes + 1;
                mHandler.sendEmptyMessageDelayed(MSG_START_SWITCH, RETRY_DELAY);
            } else {
                Log.e(TAG, "Already couldn't get the service, please check the status.");
                mHandler.sendEmptyMessage(MSG_SWITCH_ERROR);
            }
        } catch (EmptyPathException ex) {
            Log.e(TAG, "Catch the EmptyPathException: " + ex.getMessage());
            // There is some error when we get the switch intent.
            // Send the switch error message, .
            mHandler.sendEmptyMessage(MSG_SWITCH_ERROR);
        }
    }

    private void updateNVItems() {
        // Get the intent to start the action service to handle the switch action.
        UpdateNVItemsTask updateTask = new UpdateNVItemsTask(getActivity(),
                Actions.REQUEST_UPDATE_NV_ITEMS, this, mSubMask, mConfigId);
        updateTask.execute(mSelectedCarrier);
    }

    private void updateROWNVItems() {
        Carrier targetCarrier = null;
        for (Carrier carrier : mCarriers) {
            if (!carrier.mName.endsWith(Utils.CARRIER_TO_DEFAULT_NAME)) {
                Log.d(TAG, "Update ROW NV items, target Carrier : " + carrier.mName);
                targetCarrier = carrier;
                break;
            }
        }
        if (targetCarrier != null) {
            findMatchedSubscriptionId(targetCarrier);
            UpdateNVItemsTask updateTask = new UpdateNVItemsTask(getActivity(),
                    Actions.REQUEST_UPDATE_ROW_NV_ITEMS, this, mSubMask, mConfigId);
            updateTask.execute(targetCarrier);
        } else {
            Carrier.saveAsNotTriggeredCarrier(mCarriers);
            getActivity().setResult(Activity.RESULT_OK);
            getActivity().finish();
        }
    }
}
