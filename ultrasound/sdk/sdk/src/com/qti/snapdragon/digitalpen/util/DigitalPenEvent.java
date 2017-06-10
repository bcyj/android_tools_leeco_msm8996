/*===========================================================================
                           DigitalPenEvent.java

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.

Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
package com.qti.snapdragon.digitalpen.util;

import java.util.HashMap;
import java.util.Map;
import android.os.Parcelable;
import android.os.Parcel;

/**
 * This class is a "container" for the DigitalPen events parameters,
 * for each event we store its type, and all the parameters associated with it.
 * following is a list of all events and their parameters:
 *   <p>1) {@link #TYPE_POWER_STATE_CHANGED}:
 *       The power state of the DigitalPen daemon has changed. The digital pen
 *       standby modes are based on the fact that the digital pen stops
 *       transmitting any signal after being idle for a pre-configured number of
 *       seconds.
 *       States are:
 *         {@link #POWER_STATE_ACTIVE},
 *         {@link #POWER_STATE_STANDBY},
 *         {@link #POWER_STATE_IDLE},
 *         {@link #POWER_STATE_OFF}
 *       * Associated parameters:
 *         a) {@link #PARAM_CURRENT_POWER_STATE}  - The current power state
 *                                                  after the change.
 *         b) {@link #PARAM_PREVIOUS_POWER_STATE} - The previous power state
 *                                                  before the change.
 *   <p>2) {@link #TYPE_INTERNAL_DAEMON_ERROR}:
 *       Sent when the daemon stops unexpectedly, This could happen for
 *       several reasons. Among those are crashes, driver errors, forced
 *       shutdown by system, etc...
 *       * Associated parameters:
 *         a) {@link #PARAM_INTERNAL_ERROR_NUMBER} - The number of the error.
 *   <p>3) {@link #TYPE_MIC_BLOCKED}:
 *       Sent when one of the microphones used by the DigitalPen
 *       framework is currently being blocked by the user, the user
 *       might accidently put his/her hand on the microphone, which
 *       makes the microphone blocked and thus we stop receiving
 *       data from that microphone, which results in making the
 *       DigitalPen framework stop working properly.
 *       * Associated parameters:
 *         a) {@link #PARAM_NUMBER_MIC_BLOCKED} - The number of the blocked
 *                                                microphone. 0 when no
 *                                                microphone is blocked.
 *   <p>4) {@link #TYPE_PEN_DRAW_SCREEN_CHANGED}:
 *       Sent when the pen changes to a different drawing screen.
 *       Current screen areas are:
 *         {@link #SCREEN_NON_SUPPORTED}, An non supported area.
 *         {@link #SCREEN_ON}           , The device's screen area.
 *         {@link #SCREEN_OFF}          , A pre-defined area outside of the
 *                                        device.
 *       * Associated parameters:
 *         a) {@link #PARAM_CURRENT_PEN_DRAW_SCREEN} -  The current pen draw
 *                                                     screen after the change.
 *         b) {@link #PARAM_PREVIOUS_PEN_DRAW_SCREEN} - The previous pen draw
 *                                                     screen before the change.
 *   <p>5) {@link #TYPE_PEN_BATTERY_STATE}:
 *       Sent when the pen battery is changed from OK to low and
 *       vice versa.
 *       * Associated parameters:
 *         a) {@link #PARAM_PEN_BATTERY_STATE} - The pen battery
 *         state. States are:
 *           {@link #BATTERY_OK},
 *           {@link #BATTERY_LOW}
 *         b) {@link #PARAM_PEN_BATTERY_LEVEL} - The pen battery
 *         level. {@link #BATTERY_LEVEL_UNAVAILABLE} if no estimate,
 *         otherwise from 0 to 100, with 0 being empty and 100 being full.
 *
 *  <p><b>Use getEventType() to get the type of the event and
 *  getParameterValue(int key) to get a parameter's value</b>
 *
 * @hide
 */
public class DigitalPenEvent implements Parcelable {

    // Event types
    /**
     * The power state of the DigitalPen daemon has changed. The digital pen
     * standby modes are based on the fact that the digital pen stops transmitting
     * any signal after being idle for a pre-configured number of seconds.
     */
    public static final int TYPE_POWER_STATE_CHANGED     = 0;
    /**
     * Sent when the daemon stops unexpectedly, This could happen for several
     * reasons. Among those are crashes, driver errors, forced shutdown by system,
     * etc...
     */
    public static final int TYPE_INTERNAL_DAEMON_ERROR   = 1;
    /**
     * Sent when one of the microphones used by the DigitalPen framework is
     * currently being blocked by the user, the user might accidently put his/her
     * hand on the microphone, which makes the microphone blocked and thus we stop
     * receiving data from that microphone, which results in making the DigitalPen
     * framework stop working properly.
     */
    public static final int TYPE_MIC_BLOCKED             = 2;

    /**
     * Sent when the pen battery state is changed.
     */
    public static final int TYPE_PEN_BATTERY_STATE       = 4;

    /**
     * Sent when the microphones spur state is changed.
     */
    public static final int TYPE_SPUR_STATE              = 8;

    /**
     * Sent when there is a bad writing scenario.
     */
    public static final int TYPE_BAD_SCENARIO            = 16;

    /**
     * Sent on attach to notify of version
     */
    public static final int TYPE_VERSION                 = 32;

    /**
     * Sent on attach to notify of side channel capability
     */
    public static final int TYPE_SIDE_CHANNEL_CAPABLE    = 64;

    // POWER_STATE_CHANGED parameters
    /** The current power state after the change. */
    public static final int PARAM_CURRENT_POWER_STATE    = 0;
    /** The previous power state before the change. */
    public static final int PARAM_PREVIOUS_POWER_STATE   = 1;

    // INTERNAL_DAEMON_ERROR parameters
    /** The number of the error. */
    public static final int PARAM_INTERNAL_ERROR_NUMBER  = 10;

    // MIC_BLOCKED parameters
    /** Blocked microphones state. */
    public static final int PARAM_MIC_BLOCKED        = 20;

    // PEN_DRAW_SCREEN_CHANGED parameters
    /** The current pen draw screen after the change. */
    public static final int PARAM_CURRENT_PEN_DRAW_SCREEN  = 30;
    /** The previous pen draw screen before the change. */
    public static final int PARAM_PREVIOUS_PEN_DRAW_SCREEN = 31;

    // PEN_BATTERY_STATE parameters
    /** The pen battery state. */
    public static final int PARAM_PEN_BATTERY_STATE  = 40;
    public static final int PARAM_PEN_BATTERY_LEVEL = 41;

    // SPUR_STATE parameters
    /** The spur state. */
    public static final int PARAM_SPUR_STATE  = 50;

    // BAD_SCENARIO parameters
    /** The bad scenario. */
    public static final int PARAM_BAD_SCENARIO  = 60;

    // FRAMEWORK_VERSION parameters
    public static final int PARAM_VERSION_MAJOR = 70;
    public static final int PARAM_VERSION_MINOR = 71;

    // SIDE_CHANNEL_CAPABLE parameters
    public static final int PARAM_SIDE_CHANNEL_CAPABLE = 80;

    // POWER_STATE_CHANGED defines
    /**
     * Framework is working in normal mode.
     */
    public static final int POWER_STATE_ACTIVE  = 0;
    /**
     * Medium power mode with high UI responsiveness when the pen resumes
     * operation.
     */
    public static final int POWER_STATE_STANDBY = 1;
    /**
     * Low power mode with lower UI responsiveness when the pen resumes operation.
     */
    public static final int POWER_STATE_IDLE    = 2;
    /**
     * Framework is turned off.
     */
    public static final int POWER_STATE_OFF     = 3;

    // PEN_DRAW_SCREEN_CHANGED defines
    /**
     * Pen is not in any of the supported screens.
     */
    public static final int SCREEN_NON_SUPPORTED  = 0;
    /**
     * Pen is in the pre-defined "on screen" area. Usually this area is the device
     * touch-screen area.
     */
    public static final int SCREEN_ON             = 1;
    /**
     * Pen is in the pre-defined "off screen" area.
     */
    public static final int SCREEN_OFF            = 2;

    // PEN_BATTERY_STATE defines
    /**
     * Pen battery is OK.
     */
    public static final int BATTERY_OK  = 0;
    /**
     * Pen battery is low.
     */
    public static final int BATTERY_LOW = 1;
    /**
     * Battery level estimate not available
     */
    public static final int BATTERY_LEVEL_UNAVAILABLE = -1;

    // SPUR_STATE defines
    /**
     * No spurs.
     */
    public static final int SPURS_OK  = 0;
    /**
     * Spurs exist.
     */
    public static final int SPURS_EXIST = 1;

    // BAD_SCENARIO defines
    /**
     * Not a bad scenario.
     */
    public static final int BAD_SCENARIO_OK  = 0;
    /**
     * Too many microphones are blocked to perform multilateration.
     */
    public static final int BAD_SCENARIO_MULTILATERAION = 1;
    /**
     * Writing between the only two available microphones.
     */
    public static final int BAD_SCENARIO_BETWEEN_TWO_MICS = 2;

    private int mEventType;
    private Map<Integer, Integer> mParams = new HashMap<Integer, Integer>();

    public DigitalPenEvent() {
        // Empty, All default to 0
    }

    /**
     * Takes as arguments the event type and the parameters, and returns a
     * new event according to the input given.
     * @param eventType The type of the event to create.
     * @param params The corresponding parameters for the event (parameters change
     * according to type).
     */
    public DigitalPenEvent(int eventType, int[] params) {
        mEventType = eventType;
        // Fill in parameters according to the event type
        switch (eventType) {
            case TYPE_POWER_STATE_CHANGED:
                checkNumParams(params, 2);
                setParameter(PARAM_CURRENT_POWER_STATE, params[0]);
                setParameter(PARAM_PREVIOUS_POWER_STATE, params[1]);
                break;
            case TYPE_INTERNAL_DAEMON_ERROR:
                checkNumParams(params, 1);
                setParameter(PARAM_INTERNAL_ERROR_NUMBER, params[0]);
                break;
            case TYPE_MIC_BLOCKED:
                checkNumParams(params, 1);
                setParameter(PARAM_MIC_BLOCKED, params[0]);
                break;
            case TYPE_PEN_BATTERY_STATE:
                checkNumParams(params, 1);
                setParameter(PARAM_PEN_BATTERY_STATE, params[0]);
                setParameter(PARAM_PEN_BATTERY_LEVEL, params[1]);
                break;
            case TYPE_SPUR_STATE:
                checkNumParams(params, 1);
                setParameter(PARAM_SPUR_STATE, params[0]);
                break;
            case TYPE_BAD_SCENARIO:
                checkNumParams(params, 1);
                setParameter(PARAM_BAD_SCENARIO, params[0]);
                break;
            case TYPE_VERSION:
                checkNumParams(params, 2);
                setParameter(PARAM_VERSION_MAJOR, params[0]);
                setParameter(PARAM_VERSION_MINOR, params[1]);
                break;
            case TYPE_SIDE_CHANNEL_CAPABLE:
                checkNumParams(params, 1);
                setParameter(PARAM_SIDE_CHANNEL_CAPABLE, params[0]);
                break;
            default:
                throw new IllegalArgumentException("Unknown event type: " + eventType);
        }
    }

    private void checkNumParams(int[] params, int minParams) {
        if (params.length < minParams) {
            throw new IllegalArgumentException("Parameter array size is incorrect");
        }
    }

    /** @return The type of this event */
    public int getEventType() {
        return mEventType;
    }

    /**
     * Sets the parameter.
     * @param key The parameter.
     * @param value The value to give for the parameter.
     */
    private void setParameter(int key, int value) {
        mParams.put(key, value);
    }

    /** @return the value of the requested parameter */
    public int getParameterValue(int key) {
        if (!mParams.containsKey(key)) {
            return -1; // Default value to return when the parameter is not included
        }
        return mParams.get(key);
    }

    // -- Parcelable methods --
    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mEventType);
        // Write the hash size to the parcel
        out.writeInt(mParams.size());
        // Write all (key,value) pairs
        for (Map.Entry<Integer, Integer> entry : mParams.entrySet()) {
            out.writeInt(entry.getKey());
            out.writeInt(entry.getValue());
        }
    }

    public static final Parcelable.Creator<DigitalPenEvent> CREATOR
    = new Parcelable.Creator<DigitalPenEvent>() {
        public DigitalPenEvent createFromParcel(Parcel in) {
            return new DigitalPenEvent(in);
        }

        public DigitalPenEvent[] newArray(int size) {
            return new DigitalPenEvent[size];
        }
    };

    private DigitalPenEvent(Parcel in) {
        mEventType  = in.readInt();
        // Read size of hash from parcel
        int n = in.readInt();
        while (n-- > 0) {
            // Fill the Hashmap from the parcel (n elements of key,value pair)
            int key   = in.readInt();
            int value = in.readInt();
            setParameter(key, value);
        }
    }
}
