/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.loadcarrier;

import android.app.Service;
import android.content.Intent;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;

public class TriggerService extends Service implements TriggerManager.OnTriggerCompleteListener {
    private static final String TAG = "TriggerService";

    // Save the matched carriers info.
    private HashMap<String, HashMap<String, String>> mMatchedCarriers
            = new HashMap<String, HashMap<String, String>>();

    // The action and extra used to start the trigger activity.
    private static final String ACTION_TRIGGER = "com.qualcomm.qti.carrierconfigure.trigger";
    private static final String EXTRA_PATH_LIST = "trigger_path_list";

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        if (Utils.DEBUG)
            Log.i(TAG, "Start the TriggerService to find the carriers.");

        for (Trigger trigger : TriggerManager.getInstance(this).getActiveTriggerList()) {
            trigger.startTrigger();
        }
    }

    private Intent buildStartTriggerIntent() {
        Intent startTriggerIntent = new Intent();
        startTriggerIntent.setAction(ACTION_TRIGGER);

        Bundle bundle = new Bundle();
        ArrayList<String> pathList = new ArrayList<String>();
        Iterator<Entry<String, HashMap<String, String>>> iterator = mMatchedCarriers
                .entrySet().iterator();
        while (iterator.hasNext()) {
            Entry<String, HashMap<String, String>> entry = iterator.next();
            Iterator<Entry<String, String>> iterator_carrier =
                    entry.getValue().entrySet().iterator();
            while (iterator_carrier.hasNext()) {
                Entry<String, String> entry_carrier = iterator_carrier.next();
                String path = entry_carrier.getKey();
                String contents = entry_carrier.getValue();
                bundle.putString(path, contents);
                pathList.add(path);
            }
        }
        bundle.putStringArrayList(EXTRA_PATH_LIST, pathList);

        startTriggerIntent.putExtras(bundle);
        return startTriggerIntent;
    }

    @Override
    public void onTriggerComplete() {
        if (Utils.DEBUG) Log.d(TAG,"onTriggerComplete() ");
        // Find all the matched carriers.
        for (Trigger trigger : TriggerManager.getInstance(this).getActiveTriggerList()) {
            if (trigger.isActive()) {
                if (Utils.DEBUG) Log.v(TAG, "hierarchy = " + trigger.getHierarchies());
                mMatchedCarriers.put(trigger.getHierarchies(),
                        trigger.getMatchedCarries());
            }
        }

        // Send the intent to activity to handle these carriers.
        sendBroadcast(buildStartTriggerIntent());
        Log.d(TAG, "ACTION_TRIGGER intent has been sent. Time="+System.currentTimeMillis());
        // Stop this service.
        stopSelf();
    }

}
