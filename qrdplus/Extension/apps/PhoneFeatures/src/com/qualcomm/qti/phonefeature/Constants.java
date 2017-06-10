/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.phonefeature;

import android.content.Context;
import android.os.Bundle;
import android.os.Message;
import android.os.Parcelable;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.util.NativeTextHelper;

import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.UiccController;

public class Constants {

    public static final int PHONE_COUNT = TelephonyManager.getDefault().getPhoneCount();
    public static final boolean MULTI_MODE = TelephonyManager.getDefault().isMultiSimEnabled();

    public static final int NW_ACQ_ORDER_LTE_TDS_GSM = 1;
    public static final int NW_ACQ_ORDER_LTE_UMTS_GSM = 3;

    public static final int NW_BAND_LTE_FULL = 1;
    public static final int NW_BAND_LTE_TDD = 2;
    public static final int NW_BAND_LTE_FDD = 3;
    public static final int NW_BAND_LTE_NV = NW_BAND_LTE_FULL;
    public static final int NW_BAND_LTE_DEFAULT = SystemProperties.getInt("persist.radio.lte_band",
            NW_BAND_LTE_NV);

    public static final String SETTING_ACQ = "network_acq";
    public static final String SETTING_DEFAULT_PREF_NETWORK_MODE =
            "preferred_network_mode_default";
    public static final String SETTING_NETWORK_BAND = "network_band";
    public static final String SETTING_PREF_NETWORK_BAND = "network_band_preferred";

    public static void saveIntSetting(Context context, int slot, String setting, int value) {
        if (PHONE_COUNT > 1) {
            TelephonyManager.putIntAtIndex(context.getContentResolver(), setting, slot, value);
        } else {
            Settings.Global.putInt(context.getContentResolver(), setting, value);
        }
    }

    public static String getSimName(Context context, int slot) {
        SubscriptionInfo subInfo = SubscriptionManager.from(context).getActiveSubscriptionInfo(
                SubscriptionManager.getSubId(slot)[0]);
        String simName = subInfo == null ? null : subInfo.getDisplayName().toString();
        simName = NativeTextHelper.getInternalLocalString(context, simName,
                R.array.origin_carrier_alias_names, R.array.locale_carrier_alias_names);
        if (simName != null) {
            return simName;
        } else {
            return context.getResources().getString(R.string.sim_card_number_title, slot + 1);
        }
    }

    public static String getIccOperatorNumeric(int slot) {
        int dataRat = AppGlobals.getInstance().mPhones[slot].getServiceState()
                .getRilDataRadioTechnology();
        if (dataRat == ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN) {
            return null;
        }
        int appFamily = UiccController.getFamilyFromRadioTechnology(dataRat);
        IccRecords iccRecords = UiccController.getInstance().getIccRecords(slot, appFamily);
        if (iccRecords == null) {
            return null;
        }
        return iccRecords.getOperatorNumeric();
    }

    public static void response(Bundle bundle, Message callback) {
        if (callback == null) {
            Log.w(AppGlobals.TAG, "failed to response, callback is null!");
            return;
        }
        if (callback.obj != null && callback.obj instanceof Parcelable) {
            if (bundle == null) {
                bundle = new Bundle();
            }
            bundle.putParcelable(FeatureProvider.EXTRA_USEROBJ, (Parcelable) callback.obj);
        }
        callback.obj = bundle;
        if (callback.replyTo != null) {
            try {
                callback.replyTo.send(callback);
            } catch (RemoteException e) {
                Log.w(AppGlobals.TAG, "failed to response result", e);
            }
        } else if (callback.getTarget() != null) {
            callback.sendToTarget();
        } else {
            Log.w(AppGlobals.TAG, "can't response the result, replyTo and target are all null!");
        }
    }
}
