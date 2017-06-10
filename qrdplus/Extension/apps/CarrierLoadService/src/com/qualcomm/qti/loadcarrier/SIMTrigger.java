/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.loadcarrier;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.PhoneProxy;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.IccCardStatus.CardState;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.UiccController;

public class SIMTrigger extends Trigger {

    private static String TAG = "SIMTrigger";

    // The flag to track use MCC or target value as the additional value to find
    // carrier.
    private static final int FLAG_BRAND_AND_MCC = 1;
    private static final int FLAG_BRAND_AND_TARGET = 2;

    private int mPhoneCount = -1;
    private UiccCard[] mCards = null;

    // If there are cards insert, it used to save these cards info.
    private ArrayList<CardInfo> mUiccCards = new ArrayList<CardInfo>();

    SIMTrigger(Context context, OnCompleteMatchListener listener) {
        super(context);

        mHierarchies = "Operator";
        mListener = (OnCompleteMatchListener) listener;
        mPhoneCount = TelephonyManager.getDefault().getPhoneCount();
        mCards = new UiccCard[mPhoneCount];
        if (Utils.DEBUG) Log.i(TAG, "mPhoneCount =" + mPhoneCount);

    }

    private void updateCardState(int cardIndex) {
        if (Utils.DEBUG) Log.i(TAG, "Update the card " + cardIndex + " state.");

        UiccCard uiccCard = null;
        Phone[] phones = PhoneFactory.getPhones();
        PhoneBase phone = (PhoneBase) ((PhoneProxy) phones[cardIndex]).getActivePhone();
        if (phone.mCi.getRadioState().isOn()) {
            uiccCard = UiccController.getInstance().getUiccCard(cardIndex);
            mCards[cardIndex] = uiccCard;
        }
    }

    private boolean allCardsInfoAvailable() {
        for (int i = 0; i < mPhoneCount; i++) {
            if (mCards[i] == null) return false;
        }
        return true;
    }

    private static final int MSG_ALL_CARDS_INFO_AVAILABLE = 1;
    private static final int MSG_ICC_CARD_CHANGED = 2;
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_ALL_CARDS_INFO_AVAILABLE:
                if (Utils.DEBUG)
                    Log.d(TAG, "Handler the event MSG_ALL_CARDS_INFO_AVAILABLE.");
                mHandler.removeMessages(MSG_ALL_CARDS_INFO_AVAILABLE);
                // Unregister this message.
                UiccController.getInstance().unregisterForIccChanged(mHandler);
                mContext.unregisterReceiver(mSIMStateReceiver);

                if (!findMatchedCarriers() && existCards()) {
                    if (findCarriersByName(Utils.SPECIAL_ROW_PACKAGE_NAME)) {
                        // If no matched carriers, try to find the ROW package.
                        Log.d(TAG, "Find the special package ROW.");
                    } else if (findPresetCarriersByName(Utils.SPECIAL_ROW_PACKAGE_NAME)) {
                        // Try to find the preset ROW package.
                        Log.d(TAG, "Find the preset ROW package." + getMatchedCarriersNumber());
                    } else if (findCarriersByName(Utils.getCurrentCarriersDependency())) {
                        // If the ROW package does not exist, find the dependent carrier.
                        Log.d(TAG, "Find the carriers by dependency.");
                    }
                }
                mIsActive = true;
                mListener.onCompleteMatch();
                break;
            case MSG_ICC_CARD_CHANGED:
                Log.i(TAG, "Handle the msg: MSG_ICC_CARD_CHANGED");

                AsyncResult asyncResult = (AsyncResult) msg.obj;
                if (asyncResult != null && asyncResult.result != null) {
                    updateCardState((Integer) asyncResult.result);
                } else {
                    Log.w(TAG, "The message's obj is null or the result is null.");
                    for (int i = 0; i < mPhoneCount; i++) {
                        updateCardState(i);
                    }
                }

                mHandler.removeMessages(MSG_ALL_CARDS_INFO_AVAILABLE);
                if (existCards()) {
                    // Send the message as all cards info available.
                    mHandler.sendMessage(
                            mHandler.obtainMessage(MSG_ALL_CARDS_INFO_AVAILABLE));
                }

                break;
            }
        }
    };

    private boolean findCarriersByName(String carrierName){
        if (Utils.DEBUG)
            Log.i(TAG, "Try to find carrier by name. name=" + carrierName);

        if (TextUtils.isEmpty(carrierName)) {
            return false;
        }

        ArrayList<HashMap<String, String>> filterItems = new ArrayList<HashMap<String, String>>();
        HashMap<String, String> filterItem = new HashMap<String, String>();
        filterItem.put(Utils.FILTER_ITEM_NAME, carrierName);
        filterItems.add(filterItem);
        return findCarriersByFilterItems(filterItems);
    }

    private boolean findPresetCarriersByName(String carrierName) {
        if (Utils.DEBUG)
            Log.i(TAG, "Try to find preset carrier by name. name=" + carrierName);

        if (TextUtils.isEmpty(carrierName)) {
            return false;
        }

        ArrayList<HashMap<String, String>> filterItems = new ArrayList<HashMap<String, String>>();
        HashMap<String, String> filterItem = new HashMap<String, String>();
        filterItem.put(Utils.FILTER_ITEM_NAME, carrierName);
        filterItems.add(filterItem);
        return findPresetCarriersByFilterItems(filterItems);
    }

    private boolean findMatchedCarriers() {
        if (Utils.DEBUG)
            Log.i(TAG, "Try to find the matched carriers.");

        // 1. Get the cards status.
        if (existCards()) {
            // 2.1 There is the card insert, try to use the card's info to find
            // the carrier.
            // 3.1 Try to use the brand + MCC to find the carrier.
            // 3.2 Try to use the ICCID to find the carrier.
            // 3.3 Try to use the MCC+MCN+GID to find the carrier.
            // 3.4 Try to use the MCC+MCN to find the carrier.
            // 3.5 Try to use the SPN to find the carrier.
            // 3.6 Try to use the MCC only to find the carrier. It should only match one carrier.
            if (findCarriersByBrand(FLAG_BRAND_AND_MCC)) {
                // Already find the carrier list, return true.
                Log.d(TAG, "Find the carriers by brand and MCC.");
                return true;
            } else if (triggerByICCEnabled() && findCarriersByICC()) {
                // Already find the carrier list, return true.
                Log.d(TAG, "Find the carriers by ICC.");
                return true;
            } else if (findCarriersByMCCMNCGID()) {
                // Already find the carrier list, return true.
                Log.d(TAG, "Find the carriers by MCC, MNC and GID.");
                return true;
            } else if (findCarriersByMCCMNC()) {
                // Already find the carrier list, return true.
                Log.d(TAG, "Find the carriers by MCC and MNC.");
                return true;
            } else if (triggerBySPNEnabled() && findCarriersBySPN()) {
                // Already find the carrier list, return true.
                Log.d(TAG, "Find the carriers by SPN.");
                return true;
            } else if (findCarriersByMCC()) {
                // Already find the carrier. Check if match only one carrier.
                Log.d(TAG, "Find the carriers by MCC.");
                return true;
            }
        } else {
            // 2.2 There isn't any card insert, try to use brand and target to
            // find the carrier.
            return findCarriersByBrand(FLAG_BRAND_AND_TARGET);
        }

        return false;
    }

    private boolean existCards() {
        if (Utils.DEBUG)
            Log.i(TAG, "Try to check if exist the cards.");

        // Clear the cards info.
        mUiccCards.clear();

        for (int i = 0; i < mPhoneCount; i++) {
            // Get the UiccCard.
            UiccCard uiccCard = mCards[i];
            if (uiccCard != null && uiccCard.getCardState() == CardState.CARDSTATE_PRESENT) {
                if (Utils.DEBUG) Log.d(TAG, "The " + i + " card exist.");

                if (uiccCard.getNumApplications() > 1) {
                    // TODO: If the has more than one application, what should
                    // we do?
                    Log.w(TAG, "The " + i
                            + " sub has more than one application.");
                }

                UiccCardApplication app = uiccCard.getApplicationIndex(0);
                IccRecords iccRecords = app.getIccRecords();
                String iccId = iccRecords.getIccId();
                String mccmnc = iccRecords.getOperatorNumeric();
                String spn = iccRecords.getServiceProviderName();
                String gid = TelephonyManager.getDefault().getGroupIdLevel1(i);
                // Sometimes, if the card is inserted, but not accept, so these
                // value will
                // null. Then we think there isn't card.
                // TODO: If the card has been locked by pin, then we couldn't
                // get the MCCMNC,
                // but the ICC could be get. Then what should we do?
                if (!TextUtils.isEmpty(iccId) && !TextUtils.isEmpty(mccmnc)
                        ) {
                    CardInfo info = new CardInfo(iccId, mccmnc, spn, gid);
                    mUiccCards.add(info);
                }
            }
        }

        return !mUiccCards.isEmpty();
    }

    private boolean findCarriersByBrand(int additionalFlag) {
        if (Utils.DEBUG)
            Log.i(TAG, "Try to find carrier by brand.");

        String defaultBrand = mContext.getString(R.string.trigger_brand);
        String brand = Utils.getValue(Utils.PROP_KEY_TRIGGER_BRAND,
                defaultBrand);

        if (TextUtils.isEmpty(brand)) {
            if (Utils.DEBUG)
                Log.d(TAG,
                        "Try to find carrier by brand, but the brand is empty.");
            return false;
        }

        HashMap<String, String> filterItem = new HashMap<String, String>();
        switch (additionalFlag) {
        case FLAG_BRAND_AND_MCC:
            if (mUiccCards.isEmpty())
                return false;

            // Get the MCC list.
            StringBuilder mcc = new StringBuilder();
            mcc.append(mUiccCards.get(0)._mcc);
            for (int i = 1; i < mUiccCards.size(); i++) {
                mcc.append(",").append(mUiccCards.get(i)._mcc);
            }

            // Build carrier filter item.
            filterItem.put(Utils.FILTER_ITEM_BRAND, brand);
            filterItem.put(Utils.FILTER_ITEM_MCC, mcc.toString());
            break;
        case FLAG_BRAND_AND_TARGET:
            String defaultTarget = mContext.getString(R.string.trigger_target);
            String target = Utils.getValue(Utils.PROP_KEY_TRIGGER_TARGET,
                    defaultTarget);
            if (TextUtils.isEmpty(target)) {
                if (Utils.DEBUG) {
                    Log.d(TAG,
                            "Try to find carrier by target, but the target is empty.");
                }
                return false;
            }

            // Build carrier filter item.
            filterItem.put(Utils.FILTER_ITEM_BRAND, brand);
            filterItem.put(Utils.FILTER_ITEM_TARGET, target);
            break;
        }

        // Find the matched carriers from SD card and system vendor.
        ArrayList<HashMap<String, String>> filterItems = new ArrayList<HashMap<String, String>>();
        filterItems.add(filterItem);
        return findCarriersByFilterItems(filterItems);
    }

    private boolean findCarriersByICC() {
        if (Utils.DEBUG)
            Log.i(TAG, "Try to find carrier by ICC.");

        // Build the filter items by ICC.
        ArrayList<HashMap<String, String>> filterItems = new ArrayList<HashMap<String, String>>();
        for (CardInfo info : mUiccCards) {
            HashMap<String, String> filterItem = new HashMap<String, String>();
            filterItem.put(Utils.FILTER_ITEM_ICC, info._icc);
            filterItems.add(filterItem);
        }
        return findCarriersByFilterItems(filterItems);
    }

    private boolean findCarriersByMCCMNC() {
        if (Utils.DEBUG)
            Log.i(TAG, "Try to find carrier by MCCMNC.");

        // Build the filter items by MCC and MNC.
        ArrayList<HashMap<String, String>> filterItems = new ArrayList<HashMap<String, String>>();
        for (CardInfo info : mUiccCards) {
            HashMap<String, String> filterItem = new HashMap<String, String>();
            filterItem.put(Utils.FILTER_ITEM_MCCMNC, info._mcc + info._mnc);
            filterItems.add(filterItem);
        }
        return findCarriersByFilterItems(filterItems);
    }

    private boolean findCarriersByMCCMNCGID() {
        if (Utils.DEBUG)
            Log.i(TAG, "Try to find carrier by MCCMNC and GID.");

        // Build the filter items by MCC, MNC and GID.
        ArrayList<HashMap<String, String>> filterItems = new ArrayList<HashMap<String, String>>();
        for (CardInfo info : mUiccCards) {
            HashMap<String, String> filterItem = new HashMap<String, String>();
            filterItem.put(Utils.FILTER_ITEM_MCCMNC, info._mcc + info._mnc);
            filterItem.put(Utils.FILTER_ITEM_GID, info._gid);
            filterItems.add(filterItem);
        }
        return findCarriersByFilterItems(filterItems);
    }

    private boolean findCarriersBySPN() {
        if (Utils.DEBUG)
            Log.i(TAG, "Try to find carrier by SPN.");

        // Build the filter items by SPN.
        ArrayList<HashMap<String, String>> filterItems = new ArrayList<HashMap<String, String>>();
        for (CardInfo info : mUiccCards) {
            if (TextUtils.isEmpty(info._spn)) {
                if (Utils.DEBUG) Log.d(TAG, "the SPN is empty.");
                continue;
            }

            HashMap<String, String> filterItem = new HashMap<String, String>();
            filterItem.put(Utils.FILTER_ITEM_SPN, info._spn);
            filterItems.add(filterItem);
        }
        if (filterItems.size() < 1) return false;
        return findCarriersByFilterItems(filterItems);
    }

    private boolean findCarriersByMCC() {
        if (Utils.DEBUG)
            Log.i(TAG, "Try to find carrier by MCC only.");

        HashMap<String, String> matchedCarriers = new HashMap<String, String>();
        // Build the filter items by MCC only. Each SIM card should only match one carrier.
        for (CardInfo info : mUiccCards) {
            ArrayList<HashMap<String, String>> filterItems =
                    new ArrayList<HashMap<String, String>>();
            HashMap<String, String> filterItem = new HashMap<String, String>();
            filterItem.put(Utils.FILTER_ITEM_MCC, info._mcc);
            filterItems.add(filterItem);
            if (findCarriersByFilterItems(filterItems)) {
                Log.d(TAG, "MCC=" + info._mcc);
                if (getMatchedCarriersNumber() == 1) {
                    Log.d(TAG, "Matched only one carrier as expected.");
                    matchedCarriers.putAll(mMatchedCarriers);
                } else {
                    Log.d(TAG, "Matched more than one carrier. Count=" + mMatchedCarriers.size());
                }
            }
        }
        // Copy the result to mMatchedCarriers.
        mMatchedCarriers.clear();
        if (!matchedCarriers.isEmpty()) {
            mMatchedCarriers.putAll(matchedCarriers);
            return true;
        }
        return false;
    }

    private boolean triggerByICCEnabled() {
        return mContext.getResources()
                .getBoolean(R.bool.trigger_by_icc_enabled);
    }

    private boolean triggerBySPNEnabled() {
        return mContext.getResources()
                .getBoolean(R.bool.trigger_by_spn_enabled);
    }

    private class CardInfo {
        public final String _icc;
        public final String _mcc;
        public final String _mnc;
        public final String _spn;
        public final String _gid;

        public CardInfo(String icc, String operatorNumeric, String spn, String gid) {
            _icc = icc;
            _mcc = getMCC(operatorNumeric);
            _mnc = getMNC(operatorNumeric);
            _spn = spn;
            _gid = gid;
            if (Utils.DEBUG)
                Log.i(TAG, "New Card, " + this.toString());
        }

        private String getMCC(String operatorNumeric) {
            return operatorNumeric.substring(0, 3);
        }

        private String getMNC(String operatorNumeric) {
            return operatorNumeric.substring(3);
        }

        @Override
        public String toString() {
            return "Info: ICC=" + _icc + " MCC=" + _mcc + " MNC=" + _mnc
                    + " SPN=" + _spn + " GID=" + _gid;
        }
    }

    private BroadcastReceiver mSIMStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (Utils.DEBUG) Log.i(TAG, "Receive the action: " + action);

            if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(action)) {
                String stateExtra = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                if (Utils.DEBUG) Log.i(TAG, "ACTION_SIM_STATE_CHANGED: " + stateExtra);
                if (stateExtra.equals(IccCardConstants.INTENT_VALUE_ICC_IMSI)
                        || stateExtra.equals(IccCardConstants.INTENT_VALUE_ICC_LOADED)) {
                    if (existCards()) {
                        if (Utils.DEBUG) Log.i(TAG, "existCards true");
                        mHandler.sendMessage(
                                mHandler.obtainMessage(MSG_ALL_CARDS_INFO_AVAILABLE));
                    } else {
                        if (Utils.DEBUG) Log.i(TAG, "existCards false");
                    }
                }
            }
       }
    };

    @Override
    public void startTrigger() {
        UiccController controller = UiccController.getInstance();
        controller.registerForIccChanged(mHandler, MSG_ICC_CARD_CHANGED, null);
        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        mContext.registerReceiver(mSIMStateReceiver, filter);
    }

    @Override
    public String getHierarchies() {
        return mHierarchies;
    }

    @Override
    public boolean isActive() {
        return mIsActive;
    }

    @Override
    public HashMap<String, String> getMatchedCarries() {
        return mMatchedCarriers;
    }
}
