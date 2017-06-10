/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.phonefeature;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneProxy;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppState;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppType;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.PersoSubState;
import com.android.internal.telephony.uicc.IccCardStatus.CardState;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.UiccController;

public class CardStateMonitor extends Handler {

    private static final String TAG = "CardStateMonitor";
    private final static boolean DEBUG = true;

    private RegistrantList mCardInfoAvailableRegistrants = new RegistrantList();
    private RegistrantList mAllCardsInfoAvailableRegistrants = new RegistrantList();
    private RegistrantList mCardActivatedRegistrants = new RegistrantList();
    private RegistrantList mAllCardsActivatedRegistrants = new RegistrantList();
    private RegistrantList mAllCardsDectivatedRegistrants = new RegistrantList();
    private RegistrantList mCardDeactivatedRegistrants = new RegistrantList();
    private RegistrantList mPersoStateChangedRegistrants = new RegistrantList();

    private static final int EVENT_ICC_CHANGED = 1;
    private static final int EVENT_ICCID_LOAD_DONE = 2;

    static class CardInfo {
        boolean mLoadingIcc;
        String mIccId;
        String mAppState;
        String mCardState;
        String mPersoState;

        boolean isCardStateEquals(String cardState) {
            return TextUtils.equals(mCardState, cardState);
        }

        boolean isAppStateEquals(String appState) {
            return TextUtils.equals(mAppState, appState);
        }

        boolean isPersoStateEquals(String persoState) {
            return TextUtils.equals(mPersoState, persoState);
        }

        boolean isCardAvailable() {
            return !isCardStateEquals(null)
                    && !(isCardStateEquals(CardState.CARDSTATE_PRESENT.toString()) && TextUtils
                            .isEmpty(mIccId));
        }

        boolean isPersoLocked() {
            if (mPersoState == null) {
                return false;
            }
            switch (PersoSubState.valueOf(mPersoState)) {
                case PERSOSUBSTATE_UNKNOWN:
                case PERSOSUBSTATE_IN_PROGRESS:
                case PERSOSUBSTATE_READY:
                    return false;
                default:
                    return true;
            }
        }

        private void reset() {
            mLoadingIcc = false;
            mIccId = null;
            mAppState = null;
            mCardState = null;
            mPersoState = null;
        }
    }

    private static boolean mIsShutDownInProgress;
    private CardInfo[] mCards = new CardInfo[Constants.PHONE_COUNT];
    private Context mContext;
    private BroadcastReceiver receiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            if (TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED.equals(intent.getAction())) {
                int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, 0);
                UiccCard uiccCard = getUiccCard(phoneId);
                logd("radio changed on slot" + phoneId + ", state is "
                        + (uiccCard == null ? "NULL" : uiccCard.getCardState()));
                notifyCardStateChangedfNeed(phoneId, uiccCard);
            } else if (Intent.ACTION_SHUTDOWN.equals(intent.getAction()) &&
                    !intent.getBooleanExtra(Intent.EXTRA_SHUTDOWN_USERSPACE_ONLY, false)) {
                logd("ACTION_SHUTDOWN Received");
                mIsShutDownInProgress = true;
            }
        }
    };

    public CardStateMonitor(Context context) {
        mContext = context;
        for (int index = 0; index < Constants.PHONE_COUNT; index++) {
            mCards[index] = new CardInfo();
        }
        UiccController.getInstance().registerForIccChanged(this, EVENT_ICC_CHANGED, null);
        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED);
        filter.addAction(Intent.ACTION_SHUTDOWN);
        mContext.registerReceiver(receiver, filter);
    }

    public void dispose() {
        mContext.unregisterReceiver(receiver);
        UiccController.getInstance().unregisterForIccChanged(this);
    }

    public void registerPersoStateChanged(Handler handler, int what, Object obj) {
        Registrant r = new Registrant(handler, what, obj);
        synchronized (mPersoStateChangedRegistrants) {
            mPersoStateChangedRegistrants.add(r);
        }
    }

    public void registerCardActivated(Handler handler, int what, Object obj) {
        Registrant r = new Registrant(handler, what, obj);
        synchronized (mCardActivatedRegistrants) {
            mCardActivatedRegistrants.add(r);
        }
        for (int index = 0; index < Constants.PHONE_COUNT; index++) {
            if (mCards[index].isAppStateEquals(AppState.APPSTATE_READY.toString())) {
                r.notifyResult(index);
            }
        }
    }

    public void registerAllCardsActivated(Handler handler, int what, Object obj) {
        Registrant r = new Registrant(handler, what, obj);
        synchronized (mAllCardsActivatedRegistrants) {
            mAllCardsActivatedRegistrants.add(r);
        }
        for (int index = 0; index < Constants.PHONE_COUNT; index++) {
            if (!mCards[index].isAppStateEquals(AppState.APPSTATE_READY.toString())) {
                return;
            }
        }
        r.notifyRegistrant();
    }

    public void registerAllCardsDectivated(Handler handler, int what, Object obj) {
        Registrant r = new Registrant(handler, what, obj);
        synchronized (mAllCardsDectivatedRegistrants) {
            mAllCardsDectivatedRegistrants.add(r);
        }
    }

    public void registerCardDeactivated(Handler handler, int what, Object obj) {
        Registrant r = new Registrant(handler, what, obj);
        synchronized (mCardDeactivatedRegistrants) {
            mCardDeactivatedRegistrants.add(r);
        }
    }

    public void registerCardInfoAvailable(Handler handler, int what, Object obj) {
        Registrant r = new Registrant(handler, what, obj);
        synchronized (mCardInfoAvailableRegistrants) {
            mCardInfoAvailableRegistrants.add(r);
        }
        for (int index = 0; index < Constants.PHONE_COUNT; index++) {
            if (mCards[index].isCardAvailable()) {
                r.notifyResult(index);
            }
        }
    }

    public void registerAllCardsInfoAvailable(Handler handler, int what, Object obj) {
        Registrant r = new Registrant(handler, what, obj);
        synchronized (mAllCardsInfoAvailableRegistrants) {
            mAllCardsInfoAvailableRegistrants.add(r);
        }
        for (int index = 0; index < Constants.PHONE_COUNT; index++) {
            if (!mCards[index].isCardAvailable()) {
                return;
            }
        }
        r.notifyRegistrant();
    }

    public void unregisterPersoStateChanged(Handler handler) {
        synchronized (mPersoStateChangedRegistrants) {
            mPersoStateChangedRegistrants.remove(handler);
        }
    }

    public void unregisterCardActivated(Handler handler) {
        synchronized (mCardActivatedRegistrants) {
            mCardActivatedRegistrants.remove(handler);
        }
    }

    public void unregisterAllCardsActivated(Handler handler) {
        synchronized (mAllCardsActivatedRegistrants) {
            mAllCardsActivatedRegistrants.remove(handler);
        }
    }

    public void unregisterAllCardsDectivated(Handler handler) {
        synchronized (mAllCardsActivatedRegistrants) {
            mAllCardsDectivatedRegistrants.remove(handler);
        }
    }

    public void unregisterCardDeactivated(Handler handler) {
        synchronized (mCardDeactivatedRegistrants) {
            mCardDeactivatedRegistrants.remove(handler);
        }
    }

    public void unregisterCardInfoAvailable(Handler handler) {
        synchronized (mCardInfoAvailableRegistrants) {
            mCardInfoAvailableRegistrants.remove(handler);
        }
    }

    public void unregisterAllCardsInfoAvailable(Handler handler) {
        synchronized (mAllCardsInfoAvailableRegistrants) {
            mAllCardsInfoAvailableRegistrants.remove(handler);
        }
    }

    public CardInfo getCardInfo(int cardIndex) {
        return mCards[cardIndex];
    }

    public String getIccId(int cardIndex) {
        return mCards[cardIndex].mIccId;
    }

    public static UiccCard getUiccCard(int cardIndex) {
        UiccCard uiccCard = null;
        PhoneBase phone = (PhoneBase) ((PhoneProxy) AppGlobals.getInstance().mPhones[cardIndex])
                .getActivePhone();
        if (mIsShutDownInProgress
                || Settings.Global.getInt(phone.getContext().getContentResolver(),
                        Settings.Global.AIRPLANE_MODE_ON, 0) == 1) {
            return null;
        }
        if (phone.mCi.getRadioState().isOn()) {
            uiccCard = UiccController.getInstance().getUiccCard(cardIndex);
        }
        return uiccCard;
    }

    @Override
    public void handleMessage(Message msg) {
        switch (msg.what) {
            case EVENT_ICC_CHANGED:
                logd("on EVENT_ICC_CHANGED");
                onIccChanged((AsyncResult) msg.obj);
                break;
            case EVENT_ICCID_LOAD_DONE:
                logd("on EVENT_ICCID_LOAD_DONE");
                onIccIdLoaded((AsyncResult) msg.obj);
                break;
        }
    }

    private void onIccIdLoaded(AsyncResult iccIdResult) {
        byte[] data = (byte[]) iccIdResult.result;
        int cardIndex = (Integer) iccIdResult.userObj;
        String iccId = null;
        if (iccIdResult.exception != null) {
            logd("Exception in GET ICCID," + iccIdResult.exception);
        } else {
            iccId = IccUtils.bcdToString(data, 0, data.length);
            logd("get iccid on card" + cardIndex + ", id=" + iccId);
        }
        mCards[cardIndex].mLoadingIcc = false;
        if (!TextUtils.isEmpty(iccId)) {
            mCards[cardIndex].mIccId = iccId;
            mCards[cardIndex].mCardState = CardState.CARDSTATE_PRESENT.toString();
            mCardInfoAvailableRegistrants.notifyResult(cardIndex);
            notifyAllCardsAvailableIfNeed();
        }
    }

    private void onIccChanged(AsyncResult iccChangedResult) {
        if (iccChangedResult == null || iccChangedResult.result == null) {
            for (int index = 0; index < Constants.PHONE_COUNT; index++) {
                updateCardState(index);
            }
        } else {
            updateCardState((Integer) iccChangedResult.result);
        }
    }

    private void updateCardState(int sub) {
        UiccCard uiccCard = getUiccCard(sub);
        logd("ICC changed on sub" + sub + ", state is "
                + (uiccCard == null ? "NULL" : uiccCard.getCardState()));
        notifyCardAvailableIfNeed(sub, uiccCard);
        notifyCardStateChangedfNeed(sub, uiccCard);
    }

    private void loadIccId(int sub, UiccCard uiccCard) {
        mCards[sub].mLoadingIcc = true;
        boolean request = false;
        UiccCardApplication validApp = null;
        int numApps = uiccCard.getNumApplications();
        for (int i = 0; i < numApps; i++) {
            UiccCardApplication app = uiccCard.getApplicationIndex(i);
            if (app != null && app.getType() != AppType.APPTYPE_UNKNOWN) {
                validApp = app;
                break;
            }
        }
        if (validApp != null) {
            IccFileHandler fileHandler = validApp.getIccFileHandler();
            if (fileHandler != null) {
                fileHandler.loadEFTransparent(IccConstants.EF_ICCID,
                        obtainMessage(EVENT_ICCID_LOAD_DONE, sub));
                request = true;
            }
        }
        if (!request) {
            mCards[sub].mLoadingIcc = false;
        }
    }

    private void notifyCardAvailableIfNeed(int sub, UiccCard uiccCard) {
        if (uiccCard != null) {
            if (!mCards[sub].isCardStateEquals(uiccCard.getCardState().toString())) {
                if (CardState.CARDSTATE_PRESENT == uiccCard.getCardState()) {
                    if (!mCards[sub].mLoadingIcc) {
                        loadIccId(sub, uiccCard);
                    }
                } else {
                    if (CardState.CARDSTATE_ABSENT == uiccCard.getCardState()) {
                        mCards[sub].mIccId = null;
                    }
                    mCards[sub].mCardState = uiccCard.getCardState().toString();
                    mCardInfoAvailableRegistrants.notifyResult(sub);
                    notifyAllCardsAvailableIfNeed();
                }
            }
        } else {
            // card is null, means card info is inavailable or the device is in
            // APM, need to reset all card info, otherwise no change will be
            // detected when card info is available again!
            mCards[sub].reset();
        }
    }

    private void notifyCardStateChangedfNeed(int sub, UiccCard uiccCard) {
        String appState = null;
        String persoState = null;
        if (uiccCard != null
                && CardState.CARDSTATE_PRESENT == uiccCard.getCardState()) {
            int phoneType = TelephonyManager.getPhoneType(sub);
            UiccCardApplication app = null;
            if (phoneType == TelephonyManager.PHONE_TYPE_GSM) {
                app = uiccCard.getApplication(UiccController.APP_FAM_3GPP);
            } else if (phoneType == TelephonyManager.PHONE_TYPE_CDMA) {
                app = uiccCard.getApplication(UiccController.APP_FAM_3GPP2);
            }
            if (app != null) {
                appState = app.getState().toString();
                if (AppState.APPSTATE_SUBSCRIPTION_PERSO == app.getState()) {
                    persoState = app.getPersoSubState().toString();
                }
            } else {
                appState = AppState.APPSTATE_DETECTED.toString();
            }
            logd("phoneType:" + phoneType + ", appState:" + appState + ", perso:" + persoState);
        }
        if (!mCards[sub].isAppStateEquals(appState)) {
            String beforeState = mCards[sub].mAppState;
            mCards[sub].mAppState = appState;
            if (AppState.APPSTATE_READY.toString().equals(appState)) {
                mCardActivatedRegistrants.notifyResult(sub);
                notifyAllCardsActivatedIfNeed();
            } else if (AppState.APPSTATE_DETECTED.toString().equals(appState)
                    && beforeState != null) {
                mCardDeactivatedRegistrants.notifyResult(sub);
                notifyAllCardsDectivatedIfNeed();
            }
        }
        if (!mCards[sub].isPersoStateEquals(persoState)) {
            mCards[sub].mPersoState = persoState;
            mPersoStateChangedRegistrants.notifyResult(sub);
        }
    }

    private void notifyAllCardsActivatedIfNeed() {
        for (int index = 0; index < Constants.PHONE_COUNT; index++) {
            if (!mCards[index].isAppStateEquals(AppState.APPSTATE_READY.toString())) {
                return;
            }
        }
        mAllCardsActivatedRegistrants.notifyRegistrants();
    }

    private void notifyAllCardsDectivatedIfNeed() {
        for (int index = 0; index < Constants.PHONE_COUNT; index++) {
            if (!mCards[index].isAppStateEquals(AppState.APPSTATE_DETECTED.toString())) {
                return;
            }
        }
        mAllCardsDectivatedRegistrants.notifyRegistrants();
    }

    private void notifyAllCardsAvailableIfNeed() {
        for (int index = 0; index < Constants.PHONE_COUNT; index++) {
            if (!mCards[index].isCardAvailable()) {
                return;
            }
        }
        mAllCardsInfoAvailableRegistrants.notifyRegistrants();
    }

    static void logd(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }
}
