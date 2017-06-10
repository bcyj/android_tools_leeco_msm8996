/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.loadcarrier;

import java.util.ArrayList;
import java.util.HashMap;

import android.content.Context;
import android.os.Build;
import android.util.Log;

public class ModelTrigger extends Trigger {

    private final static String TAG = "ModelTrigger";

    ModelTrigger(Context context, OnCompleteMatchListener listener) {
        super(context);
        mHierarchies = "Model";
        mListener = listener;
    }

    @Override
    public void startTrigger() {
        findCarriersByModel();
        mIsActive = true;
        mListener.onCompleteMatch();
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

    private boolean findCarriersByModel() {
        if (Utils.DEBUG)
            Log.i(TAG, "Try to find carrier by Model.");

        // Build the filter items by ICC.
        ArrayList<HashMap<String, String>> filterItems = new ArrayList<HashMap<String, String>>();
        HashMap<String, String> filterItem = new HashMap<String, String>();
        filterItem.put(Utils.FILTER_ITEM_MODEL, Build.MODEL);
        if (Utils.DEBUG) Log.i(TAG, "hierarchy = " + getHierarchies() +"model = " + Build.MODEL);
        filterItems.add(filterItem);
        return findCarriersByFilterItems(filterItems);
    }

}
