/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.loadcarrier;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;

import android.content.Context;

public abstract class Trigger {
    private static final String CARRIER2DEFAULT_OTA_ZIP_FILE_NAME = "2Default.ota.zip";

    // The path used to find the carriers.
    protected String mCustomerPath = null;
    protected String mSysVendorPath = null;
    protected String mSdcardPath = null;
    protected String mSecSdcardPath = null;

    protected boolean mIsActive = false;
    protected String mHierarchies = "";

    protected Context mContext = null;
    protected HashMap<String, String> mMatchedCarriers = new HashMap<String, String>();

    interface OnCompleteMatchListener {
        void onCompleteMatch();
    }

    protected OnCompleteMatchListener mListener;

    Trigger(Context context) {
        mContext = context;
        // Get the path for find the carriers.
        String defPath = context.getString(R.string.trigger_path);
        mCustomerPath = Utils.getValue(Utils.PROP_KEY_TRIGGER_PATH, defPath);
        mSysVendorPath = Utils.getPath(Utils.FLAG_SYSTEM_VENDOR);
        mSdcardPath = Utils.getPath(Utils.FLAG_STORAGE_EXTERNAL);
        mSecSdcardPath = Utils.getPath(Utils.FLAG_STORAGE_SECONDARY);
    }

    abstract public void startTrigger();

    abstract public String getHierarchies();

    abstract public boolean isActive();

    abstract public HashMap<String, String> getMatchedCarries();

    protected boolean findCarriersByFilterItems(
            ArrayList<HashMap<String, String>> filterItems) {
        // Try to find the matched carrier, clear it first.
        mMatchedCarriers.clear();

        if (filterItems == null || filterItems.size() < 1)
            return false;

        //Utils.getCarrierList(mMatchedCarriers, mCustomerPath, false,
        //        filterItems);
        //Utils.getCarrierList(mMatchedCarriers, mSysVendorPath, false,
        //        filterItems);
        //Utils.getCarrierList(mMatchedCarriers, mSdcardPath, true, filterItems);
        Utils.getCarrierList(mMatchedCarriers, mSecSdcardPath, true,
                filterItems);

        if (mMatchedCarriers.isEmpty()) {
            return false;
        } else if (mMatchedCarriers.size() == 1) {
            Iterator<Entry<String, String>> iterator = mMatchedCarriers.entrySet().iterator();
            Entry<String, String> carrier = iterator.next();
            String path = carrier.getKey();
            if (path.endsWith(CARRIER2DEFAULT_OTA_ZIP_FILE_NAME)) {
                return false;
            }
        }

        return true;
    }

    protected boolean findPresetCarriersByFilterItems(
            ArrayList<HashMap<String, String>> filterItems) {
        // Try to find the matched carrier, clear it first.
        mMatchedCarriers.clear();

        if (filterItems == null || filterItems.size() < 1)
            return false;

        Utils.getCarrierList(mMatchedCarriers, mSysVendorPath, false, filterItems);

        if (mMatchedCarriers.isEmpty()) {
            return false;
        }

        return true;
    }

    protected int getMatchedCarriersNumber() {
        int matchedCarriersNumber = 0;
        Iterator<Entry<String, String>> iterator = mMatchedCarriers.entrySet().iterator();
        while (iterator.hasNext()) {
            Entry<String, String> carrier = iterator.next();
            String path = carrier.getKey();
            if (!path.endsWith(CARRIER2DEFAULT_OTA_ZIP_FILE_NAME)){
                matchedCarriersNumber++;
            }
        }
        return matchedCarriersNumber;
    }
}
