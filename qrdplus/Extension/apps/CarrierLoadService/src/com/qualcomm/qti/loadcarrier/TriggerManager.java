/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.loadcarrier;

import java.util.ArrayList;

import android.content.Context;
import android.util.Log;

public class TriggerManager implements Trigger.OnCompleteMatchListener{

    private static String TAG = "TriggerManager";
    private static TriggerManager sInstance;
    private int mTriggerCount = 0;
    private ArrayList<Trigger> mTriggerList = new ArrayList<Trigger>();

    interface OnTriggerCompleteListener {
        public void onTriggerComplete();
    }
    OnTriggerCompleteListener mListener = null;

    public static TriggerManager getInstance(TriggerService context) {
        if (sInstance == null) {
            sInstance = new TriggerManager(context);
        }
        return sInstance;
    }

    public TriggerManager(TriggerService context) {
        mListener = (OnTriggerCompleteListener) context;
        registTrigger(new SIMTrigger(context, this));
        registTrigger(new ModelTrigger(context, this));
    }

    public int getRegisterTriggerCount() {
        return mTriggerCount;
    }

    public ArrayList<Trigger> getActiveTriggerList() {
        return mTriggerList;
    }

    public void registTrigger(Trigger trigger) {
        if (trigger == null) {
            Log.d(TAG, "registerTrigger is null!");
            return;
        }
        if (!mTriggerList.contains(trigger)) {
            mTriggerList.add(trigger);
            mTriggerCount++;
        }
    }

    public void unRegisterTrigger(Trigger trigger) {
        if (trigger == null) {
            Log.d(TAG, "unRegisterTrigger is null!");
            return;
        }
        if (mTriggerList.contains(trigger)) {
            mTriggerList.remove(trigger);
            mTriggerCount--;
        }
    }

    public boolean isRegistTrigger(Trigger trigger) {
        if (trigger == null)
            return false;
        return mTriggerList.contains(trigger);
    }

    public void clear() {
        mTriggerList.clear();
        mTriggerCount = 0;
        sInstance = null;
    }

    @Override
    public void onCompleteMatch() {

        boolean completeMatch = true;
        for (Trigger trigger: getActiveTriggerList()) {
            if (!trigger.isActive()) {
                completeMatch = false;
                break;
            }
        }
        if (Utils.DEBUG) Log.v(TAG, "onCompleteMatch() complteMatch = "+completeMatch);
        if (completeMatch && mListener != null) {
            mListener.onTriggerComplete();
        }
    }
}
