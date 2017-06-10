/*===========================================================================
                           EventInterface.java

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.snapdragon.sdk.digitalpen.impl;

import java.util.HashMap;

import com.qti.snapdragon.digitalpen.util.DigitalPenEvent;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.BatteryStateChangedListener.BatteryState;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.PowerStateChangedListener.PowerState;

import android.content.Intent;
import android.util.Log;

public class EventInterface {

    private static final String INTENT_EXTRA_KEY_PARAMS = "params";
    public static final String INTENT_EXTRA_KEY_TYPE = "type";
    public static final String INTENT_ACTION = "com.qti.snapdragon.digitalpen.ACTION_EVENT";
    private static final String TAG = "DigitalPenEventInterface";

    // Broadcast event types from service; won't be in a DigitalPenEvent:
    // (values must differ from those in usf_epos_defs.h)
    public static int EVENT_TYPE_BACKGROUND_SIDE_CHANNEL_CANCELED = 1000;

    public enum EventType {
        UNKNOWN,
        POWER_STATE_CHANGED,
        MIC_BLOCKED,
        BATTERY_STATE,
        SPUR_STATE,
        BAD_SCENARIO,
        BACKGROUND_SIDE_CHANNEL_CANCELED,

    }

    static private HashMap<Integer, EventType> eventMap;

    static {
        eventMap = new HashMap<Integer, EventType>();
        eventMap.put(DigitalPenEvent.TYPE_POWER_STATE_CHANGED, EventType.POWER_STATE_CHANGED);
        eventMap.put(DigitalPenEvent.TYPE_MIC_BLOCKED, EventType.MIC_BLOCKED);
        eventMap.put(DigitalPenEvent.TYPE_PEN_BATTERY_STATE, EventType.BATTERY_STATE);
        eventMap.put(DigitalPenEvent.TYPE_SPUR_STATE, EventType.SPUR_STATE);
        eventMap.put(DigitalPenEvent.TYPE_BAD_SCENARIO, EventType.BAD_SCENARIO);
        eventMap.put(EVENT_TYPE_BACKGROUND_SIDE_CHANNEL_CANCELED,
                EventType.BACKGROUND_SIDE_CHANNEL_CANCELED);
    }

    static public EventType getEventType(Intent intent) {
        int eventCode = intent.getIntExtra(INTENT_EXTRA_KEY_TYPE, -1);
        EventType type = eventMap.get(eventCode);
        if (type == null) {
            return EventType.UNKNOWN;
        }
        return type;
    }

    public static int[] getEventParams(Intent intent) {
        int[] params = intent.getIntArrayExtra(INTENT_EXTRA_KEY_PARAMS);
        return params;
    }

    public static Intent makeEventIntent(int eventType, int[] params) {
        Intent eventIntent = new Intent(INTENT_ACTION);
        eventIntent.putExtra(INTENT_EXTRA_KEY_TYPE, eventType);
        eventIntent.putExtra(INTENT_EXTRA_KEY_PARAMS, params);
        return eventIntent;
    }

    public static PowerState powerStateFromParam(int param) {

        switch (param) {
            case 0:
                return PowerState.ACTIVE;
            case 1:
                return PowerState.STANDBY;
            case 2:
                return PowerState.IDLE;
            case 3:
                return PowerState.OFF;
            default:
                Log.w(TAG, "Unexpected power state: " + param);
                return null;
        }
    }

    public static BatteryState batteryStateFromParam(int param) {
        return param == 0 ? BatteryState.OK : BatteryState.LOW;
    }

}
