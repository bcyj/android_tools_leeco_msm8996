/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.roamingsettings;

import java.util.List;
import java.util.Collections;
import java.util.Comparator;

import android.app.ActionBar.LayoutParams;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.database.ContentObserver;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionInfo;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.webkit.WebView.FindListener;
import android.widget.Button;
import android.widget.TextView;


import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.PhoneProxy;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.cdma.CDMAPhone;
import com.android.internal.telephony.SubscriptionController;

public class RoamingSettingsFragmet extends Fragment implements OnClickListener {
    private static final String TAG = "RoamingSettingsFragmet";
    private static final boolean DEBUG = true;

    private static final int EVENT_SET_SUBSCRIPTION_DONE = 0;
    private static final int EVENT_AUTO_SELECT_DONE = 1;

    // The keys for arguments of dialog.
    private static final String SUB = PhoneConstants.SUBSCRIPTION_KEY;
    private final int MAX_SUBSCRIPTIONS = TelephonyManager.getDefault().getPhoneCount();
    private static final String TITLE_RES_ID = "title_res_id";
    private static final String MSG_RES_ID = "msg_res_id";

    private static final String ATTENTION_DIALOG_TAG = "attention";

    // The invalid sub index for we will use it if deactivate the card.
    private static final int SUBSCRIPTION_INDEX_INVALID = 99999;

    private static final String INIT_MCC = "000"; // Init

    // The operator numeric of China and Macao.
    private static final String CHINA_MCC = "460"; // China
    private static final String MACAO_MCC = "455"; // Macao

    private static final String CHINA_TELECOM = "46003";
    private static final String UNKNOWN_DATA = "00000";

    private View mViewRoot = null;
    private Button mActiveCard;
    private Button mNetworkInfo;
    private Button mManualSetup;
    private Button mRoamingHotline;
    private TextView mIntroInfo;

    private SharedPreferences mUserInfo;

    private Drawable mOffChecked;
    private Drawable mOnChecked;

    private boolean mIsActiveCardEnable = false;
    private boolean mIsDataRoamingEnable = false;
    private boolean mIsManualSetupEnable = false;

    private Phone mPhone = null;
    private SubscriptionController mSubMgr = null;

    private int mSubscription;
    private int mNetworkMode;
    private int mPhoneCount;

    private boolean mIsActiveStore[] = new boolean[MAX_SUBSCRIPTIONS];
    private boolean mIsForceStop;
    private boolean mIsMultiSimEnabled;

    private String mNumericPLMN = "null";

    private boolean mIsActiveSubInProgress = false;
    private boolean mIsDeactiveSubInProgress = false;

    private static final int DEFAULT_SUBSCRIPTION = 0;

    public class SimStateChangedReceiver extends BroadcastReceiver {
        private static final String TAG = "RoamingSettingsApp";

        private String mCurrentNumber = "";

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            int phoneId = 0;
            int subId = 0;
            if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(action)
                    || TelephonyIntents.ACTION_SERVICE_STATE_CHANGED.equals(action)) {
                phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, 0);
                subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, 0);
                String simStatus = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                mCurrentNumber = TelephonyManager.getDefault()
                       .getNetworkOperatorForSubscription(subId);
                if (phoneId != mSubscription) {
                    return;
                }
                if (!mCurrentNumber.equals(mNumericPLMN) && !mCurrentNumber.equals(UNKNOWN_DATA)) {
                    updateRoamingSetttingsView();
                }
                // if handle active or deactive in process we process result.
                if (mIsActiveSubInProgress
                        && (IccCardConstants.INTENT_VALUE_ICC_READY.equals(simStatus) || IccCardConstants.INTENT_VALUE_ICC_LOCKED
                                .equals(simStatus))) {
                    // SUB is activated
                    handleSetSubscriptionDone();
                    mIsActiveSubInProgress = false;
                } else if (mIsDeactiveSubInProgress
                        && (IccCardConstants.INTENT_VALUE_ICC_UNKNOWN.equals(simStatus) || IccCardConstants.INTENT_VALUE_ICC_NOT_READY
                                .equals(simStatus))) {
                    // SUB is Deactivated
                    handleSetSubscriptionDone();
                    mIsDeactiveSubInProgress = false;
                }

            }
        }
    }
    private SimStateChangedReceiver mReceiver = null;

    // We used this handle to deal with the msg of set/get network type and
    // enable/disable icc card.
    public Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            if (DEBUG)
                Log.i(TAG, "handle the message, what=" + msg.what);

            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
            case EVENT_AUTO_SELECT_DONE:
                if (mIsForceStop != true) {
                    MyProgressDialog progress = (MyProgressDialog) getFragmentManager()
                            .findFragmentByTag("progress");
                    if (progress != null) {
                        progress.dismiss();
                    }
                }

                int msgId;
                if (ar.exception != null) {
                    Log.v(TAG, "automatic network selection: failed!");
                    setManualSetupChecked(true);
                    msgId = R.string.network_auto_failure_info;
                } else {
                    Log.v(TAG, "automatic network selection: succeeded!");
                    setManualSetupChecked(false);
                    mUserInfo.edit().putBoolean("manual_gsm", false).commit();
                    msgId = R.string.network_auto_success_info;
                }

                showConnectResultDialog(getString(msgId));
                break;
            }
        }
    };

    @Override
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);

        mOffChecked = getResources().getDrawable(R.drawable.btn_check_off_roaming);
        mOnChecked = getResources().getDrawable(R.drawable.btn_check_on_roaming);

        mSubscription = DEFAULT_SUBSCRIPTION;
        mIsMultiSimEnabled = TelephonyManager.getDefault().isMultiSimEnabled();
        if (mIsMultiSimEnabled) {
            mPhone = PhoneFactory.getPhone(mSubscription);
            mSubMgr = SubscriptionController.getInstance();
        } else {
            mPhone = PhoneFactory.getDefaultPhone();
        }

        mUserInfo = getActivity().getSharedPreferences("user_info", 0);
        mIsManualSetupEnable = mUserInfo.getBoolean("manual_gsm", false);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        // Unregister handler and remove all message in handler.
        mHandler.removeMessages(EVENT_AUTO_SELECT_DONE);
        super.onDestroy();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        mViewRoot = inflater.inflate(R.layout.roaming_settings, null);
        mActiveCard = (Button) mViewRoot.findViewById(R.id.bt_active_card);
        mActiveCard.setOnClickListener(this);
        mNetworkInfo = (Button) mViewRoot.findViewById(R.id.bt_network_info);
        mNetworkInfo.setOnClickListener(this);
        mManualSetup = (Button) mViewRoot.findViewById(R.id.bt_manual_network_select);
        mManualSetup.setOnClickListener(this);
        mRoamingHotline = (Button) mViewRoot.findViewById(R.id.bt_hotline);
        mRoamingHotline.setOnClickListener(this);
        mIntroInfo = (TextView) mViewRoot.findViewById(R.id.tv_intro_info);

        // Update view
        updateRoamingSetttingsView();

        // Hide hot line and info view when single card
        if (!mIsMultiSimEnabled) {
            mActiveCard.setVisibility(View.GONE);
            mIntroInfo.setVisibility(View.GONE);
        }

        return mViewRoot;
    }

    @Override
    public void onResume() {
        super.onResume();

        mIsForceStop = false;
        if (mSubscription > PhoneConstants.SUB1) {
            boolean enable = mUserInfo.getBoolean("manual_gsm", false);
            setManualSetupChecked(enable);
        }

        // register the receiver.
        mReceiver = new SimStateChangedReceiver();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        intentFilter.addAction(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED);
        getActivity().registerReceiver(mReceiver, intentFilter);
    }

    @Override
    public void onPause() {
        MyProgressDialog progress = (MyProgressDialog) getFragmentManager()
                .findFragmentByTag("progress");
        if (progress != null) {
            mIsForceStop = true;
        }

        if (mReceiver != null) {
            getActivity().unregisterReceiver(mReceiver);
            mReceiver = null;
        }
        super.onPause();
    }

    public void onSubscriptionChanged(int sub) {
        mSubscription = sub;
        if (mViewRoot == null) {
            Log.v(TAG, "the first startup");
        } else {
            if (mSubscription == PhoneConstants.SUB1) {
                mManualSetup.setCompoundDrawablesWithIntrinsicBounds(
                        null, null, null, null);
                mRoamingHotline.setVisibility(View.VISIBLE);
                mIntroInfo.setVisibility(View.VISIBLE);
            } else {
                setManualSetupChecked(mIsManualSetupEnable);
                mRoamingHotline.setVisibility(View.GONE);
                mIntroInfo.setVisibility(View.GONE);
            }

            // if the phone count > 1, we need get the phone by subscription.
            if (mIsMultiSimEnabled) {
                mPhone = PhoneFactory.getPhone(mSubscription);
            } else {
                mPhone = PhoneFactory.getDefaultPhone();
            }

            updateRoamingSetttingsView();
        }
    }

    private void updateRoamingSetttingsView() {
        boolean isActive = false;
        boolean hasIccCard = false;

        hasIccCard = TelephonyManager.getDefault().hasIccCard();
        if (hasIccCard) {
            if (!mIsMultiSimEnabled) {
                isActive = true;
            } else {
                isActive = isSubActive(mSubscription);
            }
        }

        if (mSubscription == PhoneConstants.SUB1) {
            boolean enable = false;
            boolean isChinaTelecom = true;
            boolean isDomestic = true;

            if (hasIccCard) {
                enable = hasIccCard;
                // Get China Telecom code and CN MO
                mNumericPLMN = TelephonyManager.getDefault().getNetworkOperatorForSubscription(
                        CurrentNetworkStatus.getSubIdFromSlotId(mSubscription));
                Log.v(TAG, "Get PLMN name: " + mNumericPLMN);
                //mNumericSPN = mPhone.getIccCard().getIccRecords().getOperatorNumeric();
                if (mNumericPLMN != null && !mNumericPLMN.isEmpty()) {
                    //isChinaTelecom = mNumericSPN.startsWith(CHINA_TELECOM);
                    if (mNumericPLMN.startsWith(CHINA_MCC) || mNumericPLMN.startsWith(MACAO_MCC)
                            || mNumericPLMN.startsWith(INIT_MCC)) {
                        isDomestic = true;
                    } else {
                        isDomestic = false;
                    }
                }
                Log.v(TAG, "ChinaTelecom is: " + isChinaTelecom + " , Domestic is: " + isDomestic);

                if (isActive) {
                    enable = isChinaTelecom;
                } else {
                    enable = isActive;
                }
            }

            mActiveCard.setEnabled(hasIccCard);
            Log.d(TAG, "enabled:" + enable);
            mNetworkInfo.setEnabled(enable);
            mManualSetup.setEnabled(enable && !isDomestic);
            mIntroInfo.setEnabled(enable);
            mRoamingHotline.setEnabled(enable);
        } else {
            mActiveCard.setEnabled(TelephonyManager.getDefault().hasIccCard(mSubscription));
            mNetworkInfo.setEnabled(isActive);
            mManualSetup.setEnabled(isActive);
        }

        if (mIsMultiSimEnabled ) {
            setActiveCardChecked(isSubActive(mSubscription)
                    && hasIccCard);
        }

    }

    private boolean onActiveCardCheck() {
        mIsActiveCardEnable = mIsActiveCardEnable ? false : true;
        Drawable drawable = mIsActiveCardEnable ? mOnChecked : mOffChecked;
        mActiveCard.setCompoundDrawablesWithIntrinsicBounds(
                null, null, drawable, null);
        return mIsActiveCardEnable;
    }

    private boolean onManualSetupCheck() {
        mIsManualSetupEnable = mIsManualSetupEnable ? false : true;
        Drawable drawable = mIsManualSetupEnable ? mOnChecked : mOffChecked;
        mManualSetup.setCompoundDrawablesWithIntrinsicBounds(
                null, null, drawable, null);
        return mIsManualSetupEnable;
    }

    private void setActiveCardChecked(boolean checked) {
        mIsActiveCardEnable = checked;
        Drawable drawable = mIsActiveCardEnable ? mOnChecked : mOffChecked;
        mActiveCard.setCompoundDrawablesWithIntrinsicBounds(
                null, null, drawable, null);
    }

    private void setManualSetupChecked(boolean checked) {
        mIsManualSetupEnable = checked;
        Drawable drawable = mIsManualSetupEnable ? mOnChecked : mOffChecked;
        mManualSetup.setCompoundDrawablesWithIntrinsicBounds(
                null, null, drawable, null);
    }

    @Override
    public void onClick(View v) {
        Intent intent;
        switch (v.getId()) {
        case R.id.bt_active_card:
            final boolean activeCardChecked = onActiveCardCheck();
            // can not disable both SIM
            //TODO:DEP FW API
            if (!activeCardChecked && getActiveSubscriptionsCount() <= 1) {
                AttentionDialog
                        .newInstance(mSubscription, R.string.attention_dialog_title,
                                getString(R.string.disable_both_sim_error_msg), true, false,
                                null, null).show(getFragmentManager(), ATTENTION_DIALOG_TAG);
                // reset check status.
                setActiveCardChecked(!activeCardChecked);
                return;
            }
            if (mIsMultiSimEnabled) {
                // We will always prompt the dialog to alert the user which
                // action will be done.
                AttentionDialog.OnPositiveClickListener plistener =
                        new AttentionDialog.OnPositiveClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog) {
                        // Set the subcription's state
                        setSubState(activeCardChecked);

                        // Show the progress dialog
                        int msgResId = activeCardChecked ? R.string.progress_msg_enable_card
                                : R.string.progress_msg_disable_card;
                        MyProgressDialog.OnCancleClickListener clistener =
                                new MyProgressDialog.OnCancleClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog) {
                            dialog.dismiss();
                            setActiveCardChecked(!activeCardChecked);
                            }
                        };
                        MyProgressDialog progress = MyProgressDialog
                                .newInstance(mSubscription, getString(msgResId), clistener);

                        progress.show(getFragmentManager(), "progress");

                        dialog.dismiss();
                    }
                };
                AttentionDialog.OnNegativeClickListener nlistener =
                        new AttentionDialog.OnNegativeClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog) {
                        dialog.dismiss();
                        // reset the value.
                        setActiveCardChecked(!activeCardChecked);
                    }
                };
                int msgResId = activeCardChecked ? R.string.alert_msg_enable_card
                        : R.string.alert_msg_disable_card;
                AttentionDialog dialog = AttentionDialog.newInstance(
                        mSubscription, R.string.slot_name, getString(msgResId), true, true,
                        plistener, nlistener);
                dialog.show(getFragmentManager(), "alert");
            }
            break;
        case R.id.bt_network_info:
            intent = new Intent(getActivity(), CurrentNetworkStatus.class);
            intent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubscription);
            startActivity(intent);
            break;

        case R.id.bt_manual_network_select:
            if (mSubscription == PhoneConstants.SUB1) {
                intent = new Intent(getActivity(), ManualNetworkActivity.class);
                intent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubscription);
                startActivity(intent);
            } else {
                final boolean manualSetupChecked = onManualSetupCheck();
                if (manualSetupChecked) {
                    intent = new Intent(getActivity(), ManualNetworkActivity.class);
                    intent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubscription);
                    startActivity(intent);
                } else {
                    selectNetworkAutomatic();
                }
            }
            break;

        case R.id.bt_hotline:
            intent = new Intent();
            intent.setAction(Intent.ACTION_CALL);
            intent.setData(Uri.parse("tel:" + "+8618918910000"));
            startActivity(intent);
            break;
        }
    }

    private boolean isPhoneInCall() {
        boolean phoneInCall = false;
        if (mIsMultiSimEnabled) {
            for (int i = 0; i < mPhoneCount; i++) {
                if (TelephonyManager.getDefault().getCallState(i)
                        != TelephonyManager.CALL_STATE_IDLE) {
                    phoneInCall = true;
                    break;
                }
            }
        } else {
            if (TelephonyManager.getDefault().getCallState()
                    != TelephonyManager.CALL_STATE_IDLE) {
                phoneInCall = true;
            }
        }
        return phoneInCall;
    }

    /**
     * Set the subscription's state.
     *
     * @param enabled
     *            if enable the sub, set it as true.
     */
    private void setSubState(boolean enabled) {
        if (enabled) {
            SubscriptionManager.activateSubId(
                    CurrentNetworkStatus.getSubIdFromSlotId(mSubscription));
            mIsActiveSubInProgress = true;
        } else {
            SubscriptionManager.deactivateSubId(
                    CurrentNetworkStatus.getSubIdFromSlotId(mSubscription));
            mIsDeactiveSubInProgress = true;
        }
    }

    private void handleSetSubscriptionDone() {
        updateRoamingSetttingsView();
        if (mIsForceStop != true) {
            MyProgressDialog progress = (MyProgressDialog) getFragmentManager().findFragmentByTag(
                    "progress");
            if (progress != null) {
                progress.dismiss();
            }
        }
        showConnectResultDialog(getString(R.string.operation_success));
    }

    private boolean isSubActive(int slotId) {
        return (SubscriptionManager.getSubState(CurrentNetworkStatus.getSubIdFromSlotId(slotId))
                == SubscriptionManager.ACTIVE);
    }

    private int getActiveSubscriptionsCount() {
        int activeSubInfoCount = 0;
        List<SubscriptionInfo> subInfoLists = getActiveSubInfoList(this.getActivity());
        if (subInfoLists != null) {
            for (SubscriptionInfo subInfo : subInfoLists) {
                if (subInfo.getStatus() == SubscriptionManager.ACTIVE)
                    activeSubInfoCount++;
            }
        }
        return activeSubInfoCount;
    }

    public static List<SubscriptionInfo> getActiveSubInfoList(Context context) {
        List<SubscriptionInfo> subInfoLists = SubscriptionManager.from(context)
                .getActiveSubscriptionInfoList();
        if (subInfoLists != null) {
            Collections.sort(subInfoLists, new Comparator<SubscriptionInfo>() {
                @Override
                public int compare(SubscriptionInfo arg0, SubscriptionInfo arg1) {
                    return arg0.getSimSlotIndex()- arg1.getSimSlotIndex();
                }
            });
        }
        return subInfoLists;
    }

    private void selectNetworkAutomatic() {
        MyProgressDialog progress = MyProgressDialog.newInstance(mSubscription,
                getString(R.string.register_automatically), null);
        progress.show(getFragmentManager(), "progress");

        Message msg = mHandler.obtainMessage(EVENT_AUTO_SELECT_DONE);
        mPhone.setNetworkSelectionModeAutomatic(msg);
    }

    private void showConnectResultDialog(String msgStr) {
        if (mIsForceStop != true) {
            AttentionDialog.OnPositiveClickListener listener =
                    new AttentionDialog.OnPositiveClickListener() {
                @Override
                public void onClick(DialogInterface dialog) {
                    dialog.dismiss();
                }
            };

            AttentionDialog alert = AttentionDialog.newInstance(
                    mSubscription, R.string.slot_name,
                    msgStr, true, false, listener, null);
            alert.show(getFragmentManager(), "alert");
        }
    }

    /**
     * Request user confirmation before settings
     */
    public static class AttentionDialog extends DialogFragment {
        private OnPositiveClickListener mPositiveListener;
        private OnNegativeClickListener mNegativeListener;
        private boolean mShowTitle;
        private boolean mShowCancelButton;

        public static AttentionDialog newInstance(int sub, int titleResId,
                String msgStr, boolean showTitle, boolean showCancelButton,
                OnPositiveClickListener pLis, OnNegativeClickListener nLis) {
            AttentionDialog dialog = new AttentionDialog();
            dialog.mPositiveListener = pLis;
            dialog.mNegativeListener = nLis;
            dialog.mShowTitle = showTitle;
            dialog.mShowCancelButton = showCancelButton;

            final Bundle args = new Bundle();
            args.putInt(SUB, sub);
            args.putInt(TITLE_RES_ID, titleResId);
            args.putString(MSG_RES_ID, msgStr);
            dialog.setArguments(args);

            return dialog;
        }

        @Override
        public Dialog onCreateDialog(Bundle bundle) {
            final int titleResId = getArguments().getInt(TITLE_RES_ID);
            final String msgRes = getArguments().getString(MSG_RES_ID);
            final int sub = getArguments().getInt(SUB);

            String slotNumber = getResources().getStringArray(
                    R.array.slot_number)[sub];
            AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
            builder.setMessage(msgRes)
                    .setPositiveButton(android.R.string.ok,
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog,
                                        int which) {
                                    if (mPositiveListener != null) {
                                        mPositiveListener.onClick(dialog);
                                    }
                                }
                            }).setCancelable(true);
            if (mShowTitle) {
                builder.setIcon(android.R.drawable.ic_dialog_alert)
                .setTitle(getString(titleResId, slotNumber));
            }
            if (mShowCancelButton) {
                builder.setNegativeButton(android.R.string.cancel,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog,
                                    int which) {
                                if (mNegativeListener != null) {
                                    mNegativeListener.onClick(dialog);
                                }
                            }
                        });
            }

            return builder.create();
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
            if (mNegativeListener != null) {
                mNegativeListener.onClick(dialog);
            }
        }

        static abstract class OnPositiveClickListener {
            public abstract void onClick(DialogInterface dialog);
        }

        static abstract class OnNegativeClickListener {
            public abstract void onClick(DialogInterface dialog);
        }
    }

    /**
     * This progress dialog will be shown if user want to active or deactivate
     * the icc card.
     */
    public static class MyProgressDialog extends DialogFragment {

        private OnCancleClickListener mCancleListener;

        public static MyProgressDialog newInstance(int sub, String msgStr,
                OnCancleClickListener cLis) {
            MyProgressDialog dialog = new MyProgressDialog();

            dialog.mCancleListener = cLis;
            final Bundle args = new Bundle();
            args.putInt(SUB, sub);
            args.putString(MSG_RES_ID, msgStr);
            dialog.setArguments(args);

            return dialog;
        }

        @Override
        public Dialog onCreateDialog(Bundle bundle) {
            final String msgStr = getArguments().getString(MSG_RES_ID);
            final int sub = getArguments().getInt(SUB);

            final ProgressDialog progress = new ProgressDialog(getActivity(),
                    ProgressDialog.STYLE_SPINNER);
            String slotNumber = getResources().getStringArray(
                    R.array.slot_number)[sub];
            progress.setTitle(getString(R.string.slot_name, slotNumber));
            progress.setMessage(msgStr);
            progress.setCancelable(false);
            progress.setCanceledOnTouchOutside(false);

            return progress;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
            if (mCancleListener != null) {
                mCancleListener.onClick(dialog);
            }

        }

        static abstract class OnCancleClickListener {
            public abstract void onClick(DialogInterface dialog);
        }

    }

}
