/**
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.phonefeature;

import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface.OnDismissListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.net.ConnectivityManager;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.preference.PreferenceManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.Html;
import android.text.TextUtils;
import android.util.Log;
import android.util.NativeTextHelper;
import android.view.Gravity;
import android.view.WindowManager;
import android.widget.TextView;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppState;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppType;
import com.android.internal.telephony.uicc.IccCardStatus.CardState;
import com.android.internal.telephony.uicc.UiccCard;

public class SimDetector extends Handler implements OnClickListener, OnDismissListener {

    private static final String TAG = "SimDetector";
    private static final boolean DEBUG = true;

    private static final String PREF_KEY_SUBSCRIPTION = "subscription";
    private static final String PREF_KEY_PRIMARY_SUB_ENABLED = "primary_sub_enabled";

    private static final int TEXTVIEW_PADDING = 30;
    private static final int DRAWABLE_PADDING = 10;
    private static final int TEXT_SIZE = 18;

    private static final int MSG_ALL_CARDS_AVAILABLE = 1;
    private static final int MSG_CONFIG_LTE_DONE = 3;
    private static final int MSG_SUBSCRIPTION_ACTIVATED = 4;
    private static final int EVENT_PERSO_STATE_CHANGED = 6;

    private boolean mAllCardsChanged = true;
    private boolean mAllCardsAbsent = false;
    private boolean mCardChanged = false;
    private boolean mRestoreDdsToLTE = false;
    private final PrimarySubPolicy mPrimarySubPolicy;

    private AlertDialog mNoSIMDialog = null;
    private AlertDialog mSIMChangedDialog = null;
    private AlertDialog mNotUiccCardDialog = null;
    private AlertDialog mPersoLockedDialog = null;

    private final Context mContext;

    private CardStateMonitor mCardStateMonitor;

    public SimDetector(Context context) {
        mContext = context;
        mPrimarySubPolicy = PrimarySubPolicy.getInstance(mContext);

        IntentFilter filter = new IntentFilter(Intent.ACTION_LOCALE_CHANGED);
        mContext.registerReceiver(mLocaleChangedReceiver, filter);
        mCardStateMonitor = AppGlobals.getInstance().mCardMonitor;

        mCardStateMonitor.registerAllCardsInfoAvailable(this,
                MSG_ALL_CARDS_AVAILABLE, null);
        mCardStateMonitor.registerPersoStateChanged(this, EVENT_PERSO_STATE_CHANGED,
                null);
        mCardStateMonitor
                .registerCardActivated(this, MSG_SUBSCRIPTION_ACTIVATED, null);
        validPersoLock();
    }

    // Update the contents of dialog when the locale is changed
    private BroadcastReceiver mLocaleChangedReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            logd("Recieved EVENT ACTION_LOCALE_CHANGED");
            if (mNoSIMDialog != null && mNoSIMDialog.isShowing()) {
                logd("Update NoSIM dialog");
                mNoSIMDialog.dismiss();
                alertNoSIM();
            }
            if (mSIMChangedDialog != null && mSIMChangedDialog.isShowing()) {
                logd("Update SIMChanged dialog");
                mSIMChangedDialog.dismiss();
                alertSIMChanged();
            }
            if (mNotUiccCardDialog != null && mNotUiccCardDialog.isShowing()) {
                logd("Update NotUiccCard dialog");
                mNotUiccCardDialog.dismiss();
                alertNotUiccCard();
            }
            if (mPersoLockedDialog != null && mPersoLockedDialog.isShowing()) {
                logd("Update PersoLocked dialog");
                mPersoLockedDialog.dismiss();
                alertPersoLocked();
            }
        }
    };

    private void loadStates() {
        mAllCardsChanged = true;
        mCardChanged = false;
        mRestoreDdsToLTE = false;
        for (int i = 0; i < Constants.PHONE_COUNT; i++) {
            if (!isCardInfoChanged(i)) {
                mAllCardsChanged = false;
            } else {
                mCardChanged = true;
            }
        }
        mAllCardsAbsent = isAllCardsAbsent();
    }

    @Override
    public void handleMessage(Message msg) {
        switch (msg.what) {
            case MSG_ALL_CARDS_AVAILABLE:
                logd("on EVENT MSG_ALL_CARDS_AVAILABLE");
                onAllCardsAvailable(msg);
                break;
            case MSG_CONFIG_LTE_DONE:
                logd("on EVENT MSG_CONFIG_LTE_DONE");
                onConfigLteDone(msg);
                break;
            case EVENT_PERSO_STATE_CHANGED:
                logd("on EVENT_PERSO_STATE_CHANGED");
                onPersoStateChanged(msg);
                break;
            case MSG_SUBSCRIPTION_ACTIVATED:
                logd("on MSG_SUBSCRIPTION_ACTIVATED");
                onSubscriptionReady(msg);
                break;
        }
    }

    protected void checkUiccCard() {
        UiccCard uiccCard = CardStateMonitor.getUiccCard(0);
        String spn = IINList.getDefault(mContext)
                .getSpn(mCardStateMonitor.getIccId(0));
        if (spn != null) {
            spn = NativeTextHelper.getInternalLocalString(mContext, spn,
                    R.array.origin_carrier_names, R.array.locale_carrier_names);
        }
        if (mContext.getResources().getString(R.string.china_telecom).equals(spn)
                && uiccCard != null && uiccCard.getCardState() == CardState.CARDSTATE_PRESENT) {
            boolean hasUiccApp = uiccCard.isApplicationOnIcc(AppType.APPTYPE_CSIM)
                    || uiccCard.isApplicationOnIcc(AppType.APPTYPE_USIM)
                    || uiccCard.isApplicationOnIcc(AppType.APPTYPE_ISIM);
            if (!hasUiccApp) {
                alertNotUiccCard();
            }
        }
    }

    private void alertNotUiccCard() {
        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
        mNotUiccCardDialog = builder.setTitle(R.string.sim_info)
                .setMessage(R.string.alert_not_ct_uicc_card)
                .setNegativeButton(R.string.close, this).create();
        mNotUiccCardDialog.getWindow().setType(
                WindowManager.LayoutParams.TYPE_SYSTEM_DIALOG);
        mNotUiccCardDialog.setOnDismissListener(this);
        mNotUiccCardDialog.show();
    }

    private String getSIMInfo() {

        ConnectivityManager connService = (ConnectivityManager) mContext
                .getSystemService(Context.CONNECTIVITY_SERVICE);

        String mobileDataState = connService.getMobileDataEnabled() ? mContext
                .getString(R.string.mobile_data_on) : mContext.getString(R.string.mobile_data_off);

        String html = mContext.getString(R.string.new_sim_detected) + "<br>";
        // show SIM card info
        for (int index = 0; index < Constants.PHONE_COUNT; index++) {
            html += Constants.getSimName(mContext, index) + ":" + getSimCardInfo(index) + "<br>";
        }

        // show data status
        html += mContext.getString(R.string.default_sim_setting) + "<br>"
                + mContext.getString(R.string.mobile_data) + mobileDataState;

        return html;
    }

    protected void alertSIMChanged() {
        AlertDialog.Builder builder = new AlertDialog.Builder(mContext).setTitle(R.string.sim_info)
                .setMessage(Html.fromHtml(getSIMInfo())).setNegativeButton(R.string.close, this);
        if (Constants.MULTI_MODE) {
            builder.setPositiveButton(R.string.change, this);
        }
        mSIMChangedDialog = builder.create();
        mSIMChangedDialog.getWindow().setType(
                WindowManager.LayoutParams.TYPE_SYSTEM_DIALOG);
        mSIMChangedDialog.setOnDismissListener(this);
        mSIMChangedDialog.show();
    }

    private void saveSubscriptions() {
        for (int i = 0; i < Constants.PHONE_COUNT; i++) {
            String iccId = mCardStateMonitor.getIccId(i);
            if (!TextUtils.isEmpty(iccId)) {
                PreferenceManager.getDefaultSharedPreferences(mContext).edit()
                        .putString(PREF_KEY_SUBSCRIPTION + i, iccId).commit();
            }
        }
    }

    protected boolean isCardInfoChanged(int sub) {
        String iccId = mCardStateMonitor.getIccId(sub);
        String iccIdInSP = PreferenceManager.getDefaultSharedPreferences(mContext).getString(
                PREF_KEY_SUBSCRIPTION + sub, null);
        logd("sub" + sub + " icc id=" + iccId + ", icc id in sp=" + iccIdInSP);
        return !TextUtils.isEmpty(iccId) && !iccId.equals(iccIdInSP);
    }

    private boolean isAllCardsAbsent() {
        for (int i = 0; i < Constants.PHONE_COUNT; i++) {
            UiccCard uiccCard = CardStateMonitor.getUiccCard(i);
            if (uiccCard == null || uiccCard.getCardState() != CardState.CARDSTATE_ABSENT) {
                logd("card state on sub" + i + " not absent");
                return false;
            }
        }
        logd("all cards absent");
        return true;
    }

    private void alertPersoLocked() {
        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
        mPersoLockedDialog = builder.setTitle(R.string.sim_info)
                .setMessage(R.string.alert_not_ct_uicc_card)
                .setNegativeButton(R.string.close, this)
                .create();
        mPersoLockedDialog.getWindow().setType(
                WindowManager.LayoutParams.TYPE_SYSTEM_ERROR);
        mPersoLockedDialog.setOnDismissListener(this);
        mPersoLockedDialog.show();
    }

    private void alertNoSIM() {
        if (mNoSIMDialog != null) {
            mNoSIMDialog.dismiss();
            mNoSIMDialog = null;
        }

        TextView tv = new TextView(mContext);
        tv.setPadding(TEXTVIEW_PADDING,
                TEXTVIEW_PADDING - DRAWABLE_PADDING, TEXTVIEW_PADDING,
                TEXTVIEW_PADDING);
        tv.setText(R.string.no_sim_message);
        tv.setTextSize(TEXT_SIZE);
        tv.setGravity(Gravity.CENTER);
        tv.setCompoundDrawablesWithIntrinsicBounds(0,
                android.R.drawable.ic_dialog_alert, 0, 0);
        tv.setCompoundDrawablePadding(DRAWABLE_PADDING);

        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
        mNoSIMDialog = builder.setView(tv).setCancelable(true)
                .create();
        mNoSIMDialog.getWindow().setType(
                WindowManager.LayoutParams.TYPE_SYSTEM_DIALOG);
        mNoSIMDialog.setOnDismissListener(this);
        mNoSIMDialog.show();
    }

    public void onClick(DialogInterface dialog, int which) {
        switch (which) {
            case DialogInterface.BUTTON_NEGATIVE:
                // do nothing.
                break;
            case DialogInterface.BUTTON_POSITIVE:
                // call dual settings;
                Intent intent = new Intent("com.android.settings.sim.SIM_SUB_INFO_SETTINGS");
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                        | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
                try {
                    mContext.startActivity(intent);
                } catch (ActivityNotFoundException e) {
                    logd("can not start activity " + intent);
                }
                break;
        }
    }

    static void logd(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }

    public void dispose() {
        mCardStateMonitor.unregisterAllCardsInfoAvailable(this);
        mCardStateMonitor.unregisterPersoStateChanged(this);
        mCardStateMonitor.unregisterCardActivated(this);
        mContext.unregisterReceiver(mLocaleChangedReceiver);
    }

    private void configPrimarySub() {
        if (mAllCardsAbsent) {
            logd("all cards are absent!");
            return;
        }
        int sub = -1;
        if (!mPrimarySubPolicy.isPrimarySetable()) {
            logd("lte is not setable in any sub!");
        } else {
            int preferredLte = mPrimarySubPolicy.getPrefPrimarySub();
            int currentLte = mPrimarySubPolicy.getPrimarySub();
            logd("preferred lte sub is " + preferredLte);
            logd("current lte sub is " + currentLte);
            logd("is card changed? " + mCardChanged);
            if (preferredLte == -1 && (mCardChanged || currentLte == -1)) {
                sub = 0;
            } else if (preferredLte != -1 && (mCardChanged || currentLte != preferredLte)) {
                sub = preferredLte;
            }
            if (sub != -1 && currentLte == sub) {
                // card changed, but lte sub is correct, just notify
                obtainMessage(MSG_CONFIG_LTE_DONE).sendToTarget();
                return;
            } else if (sub == -1) {
                // card not changed and lte sub is correct, do nothing
                return;
            }
        }
        mPrimarySubPolicy.setPrimarySub(sub, obtainMessage(MSG_CONFIG_LTE_DONE));
    }

    private boolean isAutoConfigMode() {
        return mPrimarySubPolicy.isPrimaryEnabled() && mPrimarySubPolicy.isPrimarySetable()
                && mPrimarySubPolicy.getPrefPrimarySub() != -1;
    }

    private boolean isManualConfigMode() {
        return mPrimarySubPolicy.isPrimaryEnabled() && mPrimarySubPolicy.isPrimarySetable()
                && mPrimarySubPolicy.getPrefPrimarySub() == -1;
    }

    private String getSimCardInfo(int slot) {
        UiccCard uiccCard = CardStateMonitor.getUiccCard(slot);
        if (uiccCard != null && uiccCard.getCardState() == CardState.CARDSTATE_ABSENT) {
            return mContext.getString(R.string.sim_absent);
        } else {
            String carrierName = TelephonyManager.getDefault().getSimOperatorNameForSubscription(
                    SubscriptionManager.getSubId(slot)[0]);
            carrierName = NativeTextHelper.getInternalLocalString(mContext, carrierName,
                    R.array.origin_carrier_names, R.array.locale_carrier_names);
            if (TextUtils.isEmpty(carrierName) || TextUtils.isDigitsOnly(carrierName)) {
                String iccId = mCardStateMonitor.getIccId(slot);
                String spn = IINList.getDefault(mContext).getSpn(iccId);
                if (spn != null) {
                    carrierName = NativeTextHelper.getInternalLocalString(mContext, spn,
                            R.array.origin_carrier_names, R.array.locale_carrier_names);
                } else {
                    carrierName = mContext.getString(R.string.sim_unknown);
                }
            }
            if (isAutoConfigMode() && slot == mPrimarySubPolicy.getPrimarySub()) {
                if (uiccCard.isApplicationOnIcc(AppType.APPTYPE_USIM)) {
                    return carrierName + "(4G)";
                } else {
                    return carrierName + "(3G)";
                }
            } else {
                return carrierName;
            }
        }
    }

    private void onAllCardsAvailable(Message msg) {
        // reset states and load again by new card info
        loadStates();
        // close the dialog of no SIM if one SIM detected
        if (!mAllCardsAbsent && mNoSIMDialog != null) {
            mNoSIMDialog.dismiss();
            mNoSIMDialog = null;
        }
        Resources r = mContext.getResources();
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(mContext);
        if (r.getBoolean(R.bool.feature_no_sim) && mAllCardsAbsent) {
            alertNoSIM();
        } else if (mPrimarySubPolicy.isPrimaryEnabled()) {
            logd("primary sub feature is enabled!");
            sp.edit().putBoolean(PREF_KEY_PRIMARY_SUB_ENABLED, true).commit();
            configPrimarySub();
        } else {
            if (r.getBoolean(R.bool.feature_new_sim) && mCardChanged) {
                alertSIMChanged();
            }
            boolean primarySubBefore = sp.getBoolean(PREF_KEY_PRIMARY_SUB_ENABLED, false);
            logd("primary sub feature was disabled? " + primarySubBefore);
            if (primarySubBefore) {
                sp.edit().putBoolean(PREF_KEY_PRIMARY_SUB_ENABLED, false).commit();
                new PrefNetworkRequest(mContext, 0, r.getInteger(R.integer.default_lte_network),
                        null).loop();
            }
        }
        if (r.getBoolean(R.bool.feature_check_uicc)) {
            checkUiccCard();
        }
        saveSubscriptions();
    }

    private void onSubscriptionReady(Message msg) {
        int slotId = (Integer) ((AsyncResult) msg.obj).result;
        if (SubscriptionManager.getSlotId(PhoneFactory.getDataSubscription()) == slotId) {
            return;
        }
        Resources r = mContext.getResources();
        if (mRestoreDdsToLTE) {
            if (slotId == mPrimarySubPolicy.getPrimarySub()) {
                logd("restore dds to primary card");
                SubscriptionManager.from(mContext).setDefaultDataSubId(
                        SubscriptionManager.getSubId(slotId)[0]);
                mRestoreDdsToLTE = false;
            }
        } else if (slotId == PhoneConstants.SUB1 && r.getBoolean(R.bool.config_restore_dds)) {
            boolean nondsda = TelephonyManager.getDefault()
                    .getMultiSimConfiguration() != TelephonyManager.MultiSimVariants.DSDA;
            // restore the dds to sub1 when in non dsda mode and all SIM
            // cards changed.
            if (mAllCardsChanged || nondsda) {
                logd("restore dds to sub1");
                SubscriptionManager.from(mContext).setDefaultDataSubId(SubscriptionManager
                        .getSubId(PhoneConstants.SUB1)[0]);
            }
        }
    }

    protected void onPersoStateChanged(Message msg) {
        validPersoLock((Integer) ((AsyncResult) msg.obj).result);
    }

    protected void validPersoLock(int slot) {
        CardStateMonitor.CardInfo cardInfo = mCardStateMonitor.getCardInfo(slot);
        if (cardInfo.isPersoLocked()
                && (mPersoLockedDialog == null || !mPersoLockedDialog.isShowing())) {
            alertPersoLocked();
        }
    }

    protected void validPersoLock() {
        for (int i = 0; i < Constants.PHONE_COUNT; i++) {
            validPersoLock(i);
        }
    }

    protected void onConfigLteDone(Message msg) {
        int current = mPrimarySubPolicy.getPrimarySub();
        if (current != -1) {
            boolean ready = mCardStateMonitor.getCardInfo(current)
                    .isAppStateEquals(AppState.APPSTATE_READY.toString());
            if (ready && SubscriptionManager.getSlotId(PhoneFactory.
                    getDataSubscription()) != current) {
                SubscriptionManager.from(mContext).setDefaultDataSubId(
                        SubscriptionManager.getSubId(current)[0]);
                mRestoreDdsToLTE = false;
            } else {
                mRestoreDdsToLTE = true;
            }
        }
        if (isAutoConfigMode()) {
            alertSIMChanged();
        } else if (isManualConfigMode()) {
            Intent intent = new Intent(mContext, PrimarySubSetting.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
            try {
                mContext.startActivity(intent);
            } catch (ActivityNotFoundException e) {
                logd("can not start activity " + intent);
            }
        }
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        if (mNoSIMDialog == (AlertDialog) dialog) {
            mNoSIMDialog = null;
        } else if (mSIMChangedDialog == (AlertDialog) dialog) {
            mSIMChangedDialog = null;
        } else if (mNotUiccCardDialog == (AlertDialog) dialog) {
            mNotUiccCardDialog = null;
        } else if (mPersoLockedDialog == (AlertDialog) dialog) {
            mPersoLockedDialog = null;
        }
    }
}
