/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.phonefeature;

import java.util.HashMap;
import java.util.Map;

import android.content.Context;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings.SettingNotFoundException;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.uicc.UiccCard;

public class PrimarySubPolicy {

    private static final String TAG = "primary_policy";
    private static final boolean DEBUG = true;

    private static PrimarySubPolicy sInstance;

    private final Context mContext;
    private final int[] mPrimaryNetworks;

    private PrimarySubPolicy(Context context) {
        mContext = context;
        mPrimaryNetworks = mContext.getResources().getIntArray(R.array.lte_networks);
    }

    private void logd(String message) {
        if (DEBUG) {
            Log.d(TAG, message);
        }
    }

    static PrimarySubPolicy getInstance(Context context) {
        synchronized (PrimarySubPolicy.class) {
            if (sInstance == null) {
                sInstance = new PrimarySubPolicy(context);
            }
        }
        return sInstance;
    }

    public int getPrefPrimarySub() {
        return getPriority(retrievePriorities(),
                IINList.getDefault(mContext).getHighestPriority());
    }

    public boolean isPrimarySetable() {
        Map<Integer, Integer> priorities = retrievePriorities();
        int unsetableCount = getCount(priorities, -1);
        return unsetableCount < priorities.size();
    }

    public boolean isPrimaryEnabled() {
        boolean DSDS = TelephonyManager.MultiSimVariants.DSDS == TelephonyManager
                .getDefault().getMultiSimConfiguration();
        return SystemProperties.getBoolean("persist.radio.primarysub", false) && DSDS;
    }

    public int getPrimarySub() {
        for (int index = 0; index < Constants.PHONE_COUNT; index++) {
            int current = getPreferredNetwork(index);
            for (int network : mPrimaryNetworks) {
                if (current == network) {
                    return index;
                }
            }
        }
        return -1;
    }

    private Map<Integer, Integer> retrievePriorities() {
        Map<Integer, Integer> priorities = new HashMap<Integer, Integer>();
        for (int index = 0; index < Constants.PHONE_COUNT; index++) {
            String iccId = AppGlobals.getInstance().mCardMonitor.getIccId(index);
            UiccCard uiccCard = CardStateMonitor.getUiccCard(index);
            priorities.put(index, IINList.getDefault(mContext).getIINPriority(iccId, uiccCard));
        }
        return priorities;
    }

    public boolean setPrimarySub(int subscription, Message callback) {
        int network = -1;
        if (subscription >= 0 && subscription < Constants.PHONE_COUNT) {
            String iccId = AppGlobals.getInstance().mCardMonitor.getIccId(subscription);
            UiccCard uiccCard = CardStateMonitor.getUiccCard(subscription);
            network = IINList.getDefault(mContext).getIINPrefNetwork(iccId, uiccCard);
            if (network == -1) {
                return false;
            }
        }
        logd("set primary card for sub" + subscription + ", network=" + network);
        new PrefNetworkRequest(mContext, subscription, network, callback).loop();
        return true;
    }

    private int getPriority(Map<Integer, Integer> priorities, Integer higherPriority) {
        int count = getCount(priorities, higherPriority);
        if (count == 1) {
            return getKey(priorities, higherPriority);
        } else if (count > 1) {
            return -1;
        } else if (higherPriority > 0) {
            return getPriority(priorities, --higherPriority);
        } else {
            return -1;
        }
    }

    private int getCount(Map<Integer, Integer> priorities, int priority) {
        int count = 0;
        for (Integer key : priorities.keySet()) {
            if (priorities.get(key) == priority) {
                count++;
            }
        }
        return count;
    }

    private Integer getKey(Map<Integer, Integer> map, int priority) {
        for (Integer key : map.keySet()) {
            if (map.get(key) == priority) {
                return key;
            }
        }
        return null;
    }

    private int getPreferredNetwork(int sub) {
        int nwMode = -1;
        try {
            nwMode = TelephonyManager.getIntAtIndex(mContext.getContentResolver(),
                    android.provider.Settings.Global.PREFERRED_NETWORK_MODE, sub);
        } catch (SettingNotFoundException snfe) {
        }
        return nwMode;
    }
}
