/**
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 */

package com.qti.snapdragon.sdk.digitalpen;

import android.hardware.display.DisplayManager;

import com.qti.snapdragon.digitalpen.IDigitalPenDataCallback;
import com.qti.snapdragon.digitalpen.IDigitalPenEventCallback;
import com.qti.snapdragon.digitalpen.IDigitalPenService;
import com.qti.snapdragon.digitalpen.util.DigitalPenData;
import com.qti.snapdragon.digitalpen.util.DigitalPenEvent;

import android.app.Activity;
import android.app.Application;
import android.app.Application.ActivityLifecycleCallbacks;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.os.Build;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;
import android.view.Display;
import java.util.Arrays;

import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.BatteryStateChangedListener.BatteryState;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.PowerStateChangedListener.PowerState;
import com.qti.snapdragon.sdk.digitalpen.impl.AppInterfaceKeys;
import com.qti.snapdragon.sdk.digitalpen.impl.EventInterface;
import com.qti.snapdragon.sdk.digitalpen.impl.EventInterface.EventType;

/**
 * The Snapdragon DigitalPenManager communicates settings to the Snapdragon
 * Digital Pen service on the device and has methods for registering listeners
 * for events and side-channel data from the digital pen.
 * <p>
 * <ul>
 * <li>The minimum Android API level is <b>17</b>.
 * <li>Not all devices will have Snapdragon Digital Pen functionality. Query
 * <code>{@link #isFeatureSupported(Feature)}</code> for
 * <code>{@link Feature#BASIC_DIGITAL_PEN_SERVICES}</code> to verify.
 * </ul>
 * <p>
 * Settings are changed at run-time by acquiring a <code>{@link Settings}</code>
 * object, modifying it then calling <code>{@link Settings#apply()}</code>.
 * <p>
 * Multiple applications may use the Snapdragon Digital Pen SDK. Settings are
 * reset to system default when the application is paused, and restored when the
 * application resumes.
 * <p>
 * <p>
 * <b>Development Flow</b>
 * <p>
 * <ul>
 * <li>Query for capability of device. Call
 * <code>{@link #isFeatureSupported(Feature)}</code> for
 * <code>{@link Feature#BASIC_DIGITAL_PEN_SERVICES}</code> to check if the
 * device has the Snapdragon Digital Pen feature enabled.
 * <li>Create a new DigitalPenManager object.
 * <li>Call <code>{@link #getSettings()}</code> to get the settings interface.
 * <li>Change the desired setting, e.g.,
 * {@link Settings#setOffScreenMode(DigitalPenManager.OffScreenMode)}
 * <li>Apply settings changes with {@link Settings#apply()}
 * </ul>
 * <p>
 * <b>Usage</b>
 *
 * <pre class="language-java">
 * // in your class's activity...
 * protected void onCreate(Bundle savedInstanceState) {
 *     // boilerplate ...
 *     super.onCreate(savedInstanceState);
 *     setContentView(R.layout.activity_main);
 *
 *     // To use the Snapdragon Digital Pen service...
 *     DigitalPenManager mgr = new DigitalPenManager(getApplication());
 *     Settings settings = mgr.getSettings();
 *     settings.setOffScreenMode(OffScreenMode.EXTEND)
 *             .setOnScreenHoverEnabled()
 *             .apply();
 * }
 * </pre>
 */
public class DigitalPenManager {

    static private class PenNotSupportedException extends Exception {
        /**
         *
         */
        private static final long serialVersionUID = 1L;

        public PenNotSupportedException(String msg) {
            super(msg);
        }
    }

    private static final String TAG = "DigitalPenManager";
    private static final String DIGITAL_PEN_SYSTEM_SERVICE_NAME = "DigitalPen";
    private static final String VERSION = "135.0";

    // Query service version and capabilities when service is bound
    private static boolean isSideChannelSupported = false;
    private static boolean capabilitySet = false;
    private static String serviceVersion = "<unknown>";

    private static void queryServiceProperties(IDigitalPenService service) {
        if (capabilitySet == false) {
            IDigitalPenEventCallback cb = new IDigitalPenEventCallback.Stub() {
                @Override
                public void onDigitalPenPropEvent(DigitalPenEvent event) throws RemoteException {
                    capabilitySet = true;
                    if (event.getEventType() == DigitalPenEvent.TYPE_SIDE_CHANNEL_CAPABLE) {
                        isSideChannelSupported = event
                                .getParameterValue(DigitalPenEvent.PARAM_SIDE_CHANNEL_CAPABLE) == 1;
                    }
                    if (event.getEventType() == DigitalPenEvent.TYPE_VERSION) {
                        int major = event.getParameterValue(DigitalPenEvent.PARAM_VERSION_MAJOR);
                        int minor = event.getParameterValue(DigitalPenEvent.PARAM_VERSION_MINOR);
                        serviceVersion = "" + major + "." + minor;
                    }
                }
            };

            // service should call back when registering
            try {
                if (!service.registerEventCallback(cb)) {
                    Log.e(TAG, "Error registering event callback");
                }
                if (!service.unregisterEventCallback(cb)) {
                    Log.e(TAG, "Error unregistering event callback");
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * The Settings class is the interface to settings in the Snapdragon Digital
     * Pen service.
     * <p>
     * To change settings using the Settings class:
     * <ul>
     * <li>Call <code>{@link DigitalPenManager#getSettings()}</code> to get a
     * Settings object.
     * <li>Change any of the desired settings.
     * <li>Apply settings changes with {@link #apply()}
     * </ul>
     * Only the last settings is applied if the same settings is changed
     * multiple times before calling {@link #apply()}.
     * <p>
     * To read settings using the Settings class, call any of the accessor
     * functions in the class.
     * <p>
     * Note: The settings values read will be "local" settings. That is, if you
     * change a setting and query it without calling {@link #apply()} first, the
     * "local" setting will be returned which doesn't necessarily match that
     * setting in the service.
     */
    public class Settings {

        Settings() {
            // package-local ctor; only DigitalPenManager should make Settings
        }

        /**
         * Apply all Settings with the Snapdragon Digital Pen service.
         */
        public boolean apply() {
            return applySettings();
        }

        /**
         * Set how the off-screen area will be handled.
         *
         * @param mode new off-screen mode.
         */
        public Settings setOffScreenMode(OffScreenMode mode) {
            if (mode == null) {
                throw new IllegalArgumentException("OffScreenMode shouldn't be null");
            }
            appSettings.putSerializable(AppInterfaceKeys.OFF_SCREEN_MODE, mode);
            return this;
        }

        /**
         * The calling app will receive input events when the pen is hovering
         * above the on-screen area within a default of
         * {@link #DEFAULT_MAX_HOVER_DISTANCE} in the Z axis.
         */
        public Settings setOnScreenHoverEnabled() {
            appSettings.putBoolean(AppInterfaceKeys.ON_SCREEN_HOVER_ENABLED, true);
            appSettings.putInt(AppInterfaceKeys.ON_SCREEN_HOVER_MAX_DISTANCE,
                    DigitalPenManager.DEFAULT_MAX_HOVER_DISTANCE);
            return this;
        }

        /**
         * The calling app will receive input events when the pen is hovering
         * above the on-screen area within the maxDistance limit in the Z axis.
         *
         * @param maxDistance the distance limit in mm
         */
        public Settings setOnScreenHoverEnabled(int maxDistance) {
            appSettings.putInt(AppInterfaceKeys.ON_SCREEN_HOVER_MAX_DISTANCE, maxDistance);
            appSettings.putBoolean(AppInterfaceKeys.ON_SCREEN_HOVER_ENABLED, true);
            return this;
        }

        /**
         * The calling app will not receive pen hover events when the pen is
         * over the on-screen area.
         */
        public Settings setOnScreenHoverDisabled() {
            appSettings.putBoolean(AppInterfaceKeys.ON_SCREEN_HOVER_ENABLED, false);
            appSettings.putInt(AppInterfaceKeys.ON_SCREEN_HOVER_MAX_DISTANCE, 0);
            return this;
        }

        /**
         * Returns whether hover is enabled over the on-screen area.
         */
        public boolean isOnScreenHoverEnabled() {
            return appSettings.getBoolean(AppInterfaceKeys.ON_SCREEN_HOVER_ENABLED);
        }

        /**
         * Returns the maximum distance for on-screen hover events. The default
         * is {@link DigitalPenManager#DEFAULT_MAX_HOVER_DISTANCE}.
         *
         * @return the current hovering maximum distance in mm.
         */
        public int getOnScreenHoverMaxDistance() {
            return appSettings.getInt(AppInterfaceKeys.ON_SCREEN_HOVER_MAX_DISTANCE, -1);
        }

        /**
         * Allows the app to ignore the current global eraser button setting,
         * which can switch the device to the eraser tool if enabled.
         */
        public Settings setEraserBypass() {
            appSettings.putBoolean(AppInterfaceKeys.ERASER_BYPASS, true);
            return this;
        }

        /**
         * Stops bypassing the global eraser button setting. The behavior of the
         * pen buttons, in respect to enabling the eraser tool type, is
         * determined by the global setting of the eraser.
         */
        public Settings setEraserBypassDisabled() {
            appSettings.putBoolean(AppInterfaceKeys.ERASER_BYPASS, false);
            return this;
        }

        /**
         * Returns whether the system's eraser setting is bypassed.
         *
         * @return true if the eraser is bypassed, false if it is being left to
         *         the system default.
         */
        public boolean isEraserBypassed() {
            return appSettings.getBoolean(AppInterfaceKeys.ERASER_BYPASS);
        }

        /**
         * Returns the current off-screen mode.
         *
         * @return the current off-screen mode.
         */
        public OffScreenMode getOffScreenMode() {
            return (OffScreenMode) appSettings.getSerializable(AppInterfaceKeys.OFF_SCREEN_MODE);
        }

        /**
         * Returns whether hover is enabled over the off-screen area.
         *
         * @return the current hover setting, or false if in incorrect
         *         off-screen mode
         */
        public boolean isOffScreenHoverEnabled() {
            return appSettings.getBoolean(AppInterfaceKeys.OFF_SCREEN_HOVER_ENABLED);
        }

        /**
         * The calling app will receive input events when the pen is hovering
         * above the off-screen area within {@link #DEFAULT_MAX_HOVER_DISTANCE}
         * in the Z axis.
         */
        public Settings setOffScreenHoverEnabled() {
            appSettings.putBoolean(AppInterfaceKeys.OFF_SCREEN_HOVER_ENABLED, true);
            appSettings.putInt(AppInterfaceKeys.OFF_SCREEN_HOVER_MAX_DISTANCE,
                    DigitalPenManager.DEFAULT_MAX_HOVER_DISTANCE);
            return this;
        }

        /**
         * The calling app will not receive pen hover events when the pen is
         * over the off-screen area.
         */
        public Settings setOffScreenHoverDisabled() {
            appSettings.putBoolean(AppInterfaceKeys.OFF_SCREEN_HOVER_ENABLED, false);
            appSettings.putInt(AppInterfaceKeys.OFF_SCREEN_HOVER_MAX_DISTANCE, -1);
            return this;
        }

        /**
         * The calling app will receive input events when the pen is hovering
         * above the off-screen area within the maxDistance limit in the Z axis.
         *
         * @param maxDistance the distance limit in mm
         */
        public Settings setOffScreenHoverEnabled(int maxDistance) {
            appSettings.putBoolean(AppInterfaceKeys.OFF_SCREEN_HOVER_ENABLED, true);
            appSettings.putInt(AppInterfaceKeys.OFF_SCREEN_HOVER_MAX_DISTANCE, maxDistance);
            return this;
        }

        /**
         * Returns the maximum distance for off-screen hover events. The default
         * is {@link DigitalPenManager#DEFAULT_MAX_HOVER_DISTANCE}.
         *
         * @return the current hovering maximum distance in mm
         */
        public int getOffScreenHoverMaxDistance() {
            return appSettings.getInt(AppInterfaceKeys.OFF_SCREEN_HOVER_MAX_DISTANCE);
        }

        /**
         * Clears the current settings, resetting them to defaults.
         * <p>
         * Like with other settings, this doesn't take effect in the service
         * until {@link #apply()} is called.
         */
        public Settings clear() {
            appSettings.clear();
            appSettings.putAll(DigitalPenManager.DEFAULT_APP_CONFIG);
            return this;
        }

        /**
         * Sets the mapping for the given area. Platform must support
         * {@link Feature#SIDE_CHANNEL}.
         * <p>
         * Note: {@link Area#ALL} does not support
         * {@link SideChannelMapping#SCALED}.
         * <p>
         *
         * @param area the area
         */
        public Settings setSideChannelMapping(Area area, SideChannelMapping mapping) {
            if (area == null) {
                throw new IllegalArgumentException("Area shouldn't be null");
            }

            if (mapping == null) {
                throw new IllegalArgumentException("SideChannelMapping shouldn't be null");
            }

            if (!DigitalPenManager.isFeatureSupported(Feature.SIDE_CHANNEL)) {
                Log.w(TAG, "Feature " + Feature.SIDE_CHANNEL + " is not supported; ignoring");
                return this;
            }
            switch (mapping) {
                case DISABLED:
                    appSettings.remove(area.getMappingKey());
                    break;
                case RAW:
                    appSettings.putSerializable(area.getMappingKey(), mapping);
                    break;
                case SCALED:
                    if (area == Area.ALL) {
                        throw new IllegalArgumentException("Only " + SideChannelMapping.RAW
                                + " or " + SideChannelMapping.DISABLED + " allowed for " + Area.ALL);
                    }
                    appSettings.putSerializable(area.getMappingKey(), mapping);
                default:
                    break;
            }

            return this;
        }

        /**
         * Gets the mapping for the given area. Platform must support
         * {@link Feature#SIDE_CHANNEL}.
         *
         * @param area the area
         * @return the current side channel mapping
         */
        public SideChannelMapping getSideChannelMapping(Area area) {
            if (area == null) {
                throw new IllegalArgumentException("Area shouldn't be null");
            }

            if (!DigitalPenManager.isFeatureSupported(Feature.SIDE_CHANNEL)) {
                Log.w(TAG, "Feature " + Feature.SIDE_CHANNEL + " is not supported; ignoring");
                return null;
            }
            if (DigitalPenManager.this.appSettings.containsKey(area.getMappingKey())) {
                return (SideChannelMapping) DigitalPenManager.this.appSettings.getSerializable(area
                        .getMappingKey());
            } else {
                return DigitalPenManager.DEFAULT_SIDE_CHANNEL_MAPPING;
            }
        }

    };

    /**
     * The listener interface for receiving Battery State Changed events, which
     * indicate the battery is low or OK. Clients interested in this information
     * should add a listener with
     * {@link DigitalPenManager#setBatteryStateChangedListener(BatteryStateChangedListener)}
     */
    public interface BatteryStateChangedListener {

        /**
         * The enum BatteryState, indicating battery state.
         */
        public enum BatteryState {
            /**
             * Battery is OK.
             */
            OK,
            /**
             * Battery is low.
             */
            LOW
        }

        /**
         * Called when the battery state changes.
         *
         * @param state the current battery state.
         */
        void onBatteryStateChanged(BatteryState state);

    }

    /**
     * The listener interface for receiving Power State Changed events, which
     * indicate the framework has changed power state. Clients interested in
     * this information should add a listener with
     * {@link DigitalPenManager#setPowerStateChangedListener(PowerStateChangedListener)}
     */
    public interface PowerStateChangedListener {

        /**
         * The enum PowerState, which indicates the state of the framework.
         */
        static enum PowerState {
            /**
             * Framework is working in normal mode.
             */
            ACTIVE,
            /**
             * Medium power mode with high UI responsiveness when the pen
             * resumes operation.
             */
            STANDBY,
            /**
             * Low power mode with lower UI responsiveness when the pen resumes
             * operation.
             */
            IDLE,
            /**
             * Framework is turned off.
             */
            OFF

        }

        /**
         * Called when the power state of the framework has changed.
         *
         * @param currentState the current state of the framework.
         * @param lastState the prior state of the framework.
         */
        void onPowerStateChanged(PowerState currentState, PowerState lastState);

    }

    /**
     * The listener interface for receiving micBlocked events, which indicate
     * that the number of blocked microphones has changed. Clients interested in
     * this information should add a listener with
     * {@link DigitalPenManager#setMicBlockedListener(MicBlockedListener)}.
     */
    public interface MicBlockedListener {

        /**
         * Called when the number of blocked microphones changes
         *
         * @param numBlockedMics the number of blocked microphones
         */
        void onMicBlocked(int numBlockedMics);

    }

    /**
     * The listener interface for receiving notification that a side-channel
     * listener has been canceled. Clients interested in this information should
     * add a listener with
     * {@link DigitalPenManager#setBackgroundSideChannelCanceledListener(BackgroundSideChannelCanceledListener)}
     * .
     */
    public interface BackgroundSideChannelCanceledListener {

        /**
         * Called when a background side-channel listener is canceled.
         *
         * @param areaCanceled the area for which the listener was canceled
         */
        void onBackgroundSideChannelCanceled(Area areaCanceled);

    }

    private static final String DIGITAL_PEN_OFF_SCREEN_DISPLAY_NAME = "Digital Pen off-screen display";

    /** The default maximum hover distance in mm. */
    public static final int DEFAULT_MAX_HOVER_DISTANCE = 400;

    private static final SideChannelMapping DEFAULT_SIDE_CHANNEL_MAPPING = SideChannelMapping.RAW;

    /**
     * The features that can be queried through
     * {@link DigitalPenManager#isFeatureSupported(Feature)}
     */
    public enum Feature {

        /** Basic pen services are supported. */
        BASIC_DIGITAL_PEN_SERVICES,

        /**
         * Digital pen is currently enabled by user. If not enabled, application
         * may wish to start the settings activity with
         * <code>startIntent(new Intent({@link DigitalPenManager#ACTION_DIGITAL_PEN_SETTINGS}))</code>
         *
         * @see PenEnabledChecker
         */
        DIGITAL_PEN_ENABLED,

        /**
         * Side-channel methods are supported.
         *
         * @see DigitalPenManager#registerSideChannelEventListener(Area,
         *      OnSideChannelDataListener)
         * @see DigitalPenManager#unregisterSideChannelEventListener(Area)
         * @see Settings#setSideChannelMapping(DigitalPenManager.Area,
         *      DigitalPenManager.SideChannelMapping)
         * @see Settings#getSideChannelMapping(DigitalPenManager.Area)
         */
        SIDE_CHANNEL,
    }

    /**
     * This data is returned through listeners upon receiving pen events. This
     * side-channel data differs from the Android <code>MotionEvent</code> in
     * that it contains more information (e.g. tilt) and bypasses the android
     * input framework.
     * <p>
     * The position and tilt fields depend on the {@link SideChannelMapping}
     * parameter, set when calling
     * {@link DigitalPenManager#registerSideChannelEventListener(Area, OnSideChannelDataListener)}
     * <p>
     * When using {@link SideChannelMapping#RAW}:
     * <ul>
     * <li>Position fields are 0.01mm units
     * <li>Tilt fields describe a normalized 3D vector in Q30 format
     * </ul>
     * When using {@link SideChannelMapping#SCALED}
     * <ul>
     * <li>Position fields are in logical units from 0 to
     * {@link #MAX_LOGICAL_UNIT}
     * <li>Tilt fields xTilt and yTilt are deflection from the Z axis in
     * degrees. zTilt is always 0.
     * </ul>
     *
     * @see OnSideChannelDataListener
     * @see DigitalPenManager#registerSideChannelEventListener(Area,
     *      OnSideChannelDataListener)
     * @see android.view.MotionEvent
     */
    public static class SideChannelData {
        /** The maximum value for a logical unit in an axis (e.g., xPos) */
        static public int MAX_LOGICAL_UNIT = 65000;

        /**
         * X axis for the pen position in 0.01mm for
         * {@link SideChannelMapping#RAW}, or logical units from 0 to
         * {@link #MAX_LOGICAL_UNIT} for {@link SideChannelMapping#SCALED}
         */
        public int xPos;

        /**
         * Y axis for the pen position in 0.01mm for
         * {@link SideChannelMapping#RAW}, or logical units from 0 to
         * {@link #MAX_LOGICAL_UNIT} for {@link SideChannelMapping#SCALED}
         */
        public int yPos;

        /**
         * Z axis for the pen position in 0.01mm for
         * {@link SideChannelMapping#RAW}, or logical units from 0 to
         * {@link #MAX_LOGICAL_UNIT} for {@link SideChannelMapping#SCALED}
         */
        public int zPos;

        /**
         * X projection of the normalized pen vector in Q30 format for
         * {@link SideChannelMapping#RAW}, or tilt of the pen between X and Z
         * axis in degrees for {@link SideChannelMapping#SCALED}
         */
        public int xTilt;

        /**
         * Y projection of the normalized pen vector in Q30 format for
         * {@link SideChannelMapping#RAW}, or tilt of the pen between Y and Z
         * axis in degrees for {@link SideChannelMapping#SCALED}
         */
        public int yTilt;

        /**
         * Z projection of the normalized pen vector in Q30 format for
         * {@link SideChannelMapping#RAW}, or 0 for
         * {@link SideChannelMapping#SCALED}
         */
        public int zTilt;

        /** Pen pressure from 0 (none) to 2047 (max) */
        public int pressure;

        /** True if pen is down, false if pen is up (hovering) */
        public boolean isDown;

        /** Array of button states; 0: not pressed, 1: pressed */
        public int[] sideButtonsState;

        @Override
        public String toString() {
            return "SideChannelData ("
                    + "x=" + xPos
                    + ",y=" + yPos
                    + ",z=" + zPos
                    + ",xTilt=" + xTilt
                    + ",yTilt=" + yTilt
                    + ",zTilt=" + zTilt
                    + ",isDown=" + isDown
                    + ",sideButtonsState=" + Arrays.toString(sideButtonsState)
                    + ",pressure=" + pressure
                    + ")";
        }

    }

    /**
     * Used when registering listeners for side-channel data, i.e., data points
     * that bypass the Android input framework.
     *
     * @see DigitalPenManager#registerSideChannelEventListener
     * @see SideChannelData
     */
    public interface OnSideChannelDataListener {
        /**
         * Called each time the Snapdragon Digital Pen service generates a data
         * point.
         *
         * @param data the side-channel data point generated by the pen
         */
        void onDigitalPenData(SideChannelData data);
    }

    /** How the off-screen mode will operate */
    public enum OffScreenMode {
        /**
         * Off-screen input events will be generated on the virtual
         * <code>Presentation</code> to the side of the screen. See the Extended
         * Offscreen sample for an example of how to get this handle.
         * <p>
         * Additionally enables side-channel events via
         * {@link DigitalPenManager#registerSideChannelEventListener(Area, OnSideChannelDataListener)}
         * with {@link Area#OFF_SCREEN}.
         *
         * @see android.app.Presentation
         */
        EXTEND,

        /**
         * Off-screen input events will be mapped to the device's display, as if
         * the device screen was duplicated.
         * <p>
         * Additionally enables side-channel events via
         * {@link DigitalPenManager#registerSideChannelEventListener(Area, OnSideChannelDataListener)}
         * with {@link Area#OFF_SCREEN}.
         */
        DUPLICATE,

        /**
         * Input events will not be generated from off-screen pen activity, nor
         * will side-channel listeners to {@link Area#OFF_SCREEN} be called.
         */
        DISABLED
    }

    /**
     * Describes areas around the device. Note that {@link Area#ALL} refers to
     * area not otherwise registered, so for example if {@link Area#ON_SCREEN}
     * is registered, it will "punch through" {@link Area#ALL} and the on-screen
     * listener will be called instead of the all listener for points on-screen.
     *
     * @see DigitalPenManager#registerSideChannelEventListener
     * @see DigitalPenManager#unregisterSideChannelEventListener
     */
    public enum Area {
        /** Refers to the off-screen area to the side of the device */
        OFF_SCREEN(AppInterfaceKeys.OFF_SCREEN_MAPPING),

        /** Refers to the device's screen */
        ON_SCREEN(AppInterfaceKeys.ON_SCREEN_MAPPING),

        /** Refers to all areas not otherwise currently registered */
        ALL(AppInterfaceKeys.ALL_MAPPING),

        /**
         * Like {@link Area#OFF_SCREEN}, refers to the off-screen area to the
         * side of the device. However, this area's listener will continue to be
         * called once this application is in the background.
         * <p>
         * If another application also tries to use the DigitalPenManager while
         * this application is in background, this application's listener will
         * be canceled.
         *
         * @see DigitalPenManager#setBackgroundSideChannelCanceledListener(BackgroundSideChannelCanceledListener)
         */
        OFF_SCREEN_BACKGROUND(AppInterfaceKeys.OFF_SCREEN_MAPPING);

        private final String mappingKey;

        private Area(String mappingKey) {
            this.mappingKey = mappingKey;
        }

        String getMappingKey() {
            return mappingKey;
        }
    }

    /**
     * Describes how the X/Y/Z coordinates in the side-channel callback are
     * represented
     */
    public enum SideChannelMapping {

        /** Side-channel callbacks are disabled */
        DISABLED,

        /** Units are 0.01mm distance from device origin, regardless of display */
        RAW,

        /**
         * Units are scaled onto the display area, from 0 to
         * {@link SideChannelData#MAX_LOGICAL_UNIT}
         */

        SCALED
    }

    private static final OnSideChannelDataListener NULL_DATA_LISTENER = new OnSideChannelDataListener() {

        @Override
        public void onDigitalPenData(SideChannelData data) {
        }
    };

    private ActivityLifecycleHandler activityLifecycleHandler;
    private static IDigitalPenService service = null;
    private OnSideChannelDataListener allAreaDataListener = NULL_DATA_LISTENER;
    private OnSideChannelDataListener onScreenAreaDataListener = NULL_DATA_LISTENER;
    private OnSideChannelDataListener offScreenAreaDataListener = NULL_DATA_LISTENER;

    synchronized protected void handleDataCallback(DigitalPenData data) {
        SideChannelData returnedData = new SideChannelData();
        returnedData.xPos = data.getX();
        returnedData.yPos = data.getY();
        returnedData.zPos = data.getZ();
        returnedData.xTilt = data.getXTilt();
        returnedData.yTilt = data.getYTilt();
        returnedData.zTilt = data.getZTilt();
        returnedData.isDown = data.getPenState();
        returnedData.sideButtonsState = data.getSideButtonsState();
        returnedData.pressure = data.getPressure();
        OnSideChannelDataListener listener = NULL_DATA_LISTENER;
        switch (data.getRegion()) {
            case DigitalPenData.REGION_ALL:
                listener = allAreaDataListener;
                break;
            case DigitalPenData.REGION_ON_SCREEN:
                listener = onScreenAreaDataListener;
                break;
            case DigitalPenData.REGION_OFF_SCREEN:
                listener = offScreenAreaDataListener;
                break;
            default:
                Log.e(TAG, "Got data point with unknown region: " + data.getRegion());
                break;
        }
        listener.onDigitalPenData(returnedData);
    }

    static private final Bundle DEFAULT_APP_CONFIG;

    static {
        DEFAULT_APP_CONFIG = new Bundle();
        DEFAULT_APP_CONFIG.putBoolean(AppInterfaceKeys.ON_SCREEN_HOVER_ENABLED, true);
        DEFAULT_APP_CONFIG.putBoolean(AppInterfaceKeys.OFF_SCREEN_HOVER_ENABLED, true);
        DEFAULT_APP_CONFIG.putInt(AppInterfaceKeys.ON_SCREEN_HOVER_MAX_DISTANCE,
                DEFAULT_MAX_HOVER_DISTANCE);
        DEFAULT_APP_CONFIG.putInt(AppInterfaceKeys.OFF_SCREEN_HOVER_MAX_DISTANCE,
                DEFAULT_MAX_HOVER_DISTANCE);
        DEFAULT_APP_CONFIG.putSerializable(AppInterfaceKeys.OFF_SCREEN_MODE,
                OffScreenMode.DISABLED);
        DEFAULT_APP_CONFIG.putBoolean(AppInterfaceKeys.ERASER_BYPASS, false);
    }

    private final Bundle appSettings = new Bundle(DEFAULT_APP_CONFIG);

    private Bundle cachedAppSettings = new Bundle();

    private Context appContext;

    private BroadcastReceiver penEventReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            EventType type = EventInterface.getEventType(intent);
            int[] params = EventInterface.getEventParams(intent);
            switch (type) {
                case BATTERY_STATE:
                    cachedBatteryState = EventInterface.batteryStateFromParam(params[0]);
                    batteryStateChangedListener.onBatteryStateChanged(cachedBatteryState);
                    break;
                case MIC_BLOCKED:
                    cachedMicsBlocked = params[0];
                    micBlockedListener.onMicBlocked(params[0]);
                    break;
                case POWER_STATE_CHANGED:
                    cachedCurrentPowerState = EventInterface.powerStateFromParam(params[0]);
                    cachedLastPowerState = EventInterface.powerStateFromParam(params[1]);
                    powerStateChangedListener.onPowerStateChanged(
                            cachedCurrentPowerState,
                            cachedLastPowerState);
                    break;
                case BACKGROUND_SIDE_CHANNEL_CANCELED:
                    // currently only off-screen side-channel is possible
                    backgroundSideChannelCanceledListener
                            .onBackgroundSideChannelCanceled(Area.OFF_SCREEN_BACKGROUND);
                    break;
                case UNKNOWN:
                    break;
                default:
                    break;
            }
        }
    };

    private int cachedMicsBlocked = 0;

    static private MicBlockedListener NULL_MIC_BLOCKED_LISTENER = new MicBlockedListener() {

        @Override
        public void onMicBlocked(int numBlockedMics) {
        }
    };
    private MicBlockedListener micBlockedListener = NULL_MIC_BLOCKED_LISTENER;

    static private PowerStateChangedListener NULL_POWER_STATE_CHANGED_LISTENER = new PowerStateChangedListener() {

        @Override
        public void onPowerStateChanged(PowerState currentState, PowerState lastState) {
        }
    };

    private PowerStateChangedListener powerStateChangedListener = NULL_POWER_STATE_CHANGED_LISTENER;

    private PowerState cachedCurrentPowerState = PowerState.OFF;

    private PowerState cachedLastPowerState = PowerState.OFF;

    private static final BatteryStateChangedListener NULL_BATTERY_STATE_CHANGED_LISTENER = new BatteryStateChangedListener() {

        @Override
        public void onBatteryStateChanged(BatteryState state) {
        }
    };

    public static final String ACTION_DIGITAL_PEN_SETTINGS = "com.qti.snapdragon.sdk.digitalpen.SETTINGS";

    private BatteryState cachedBatteryState = BatteryState.OK;

    private BatteryStateChangedListener batteryStateChangedListener = NULL_BATTERY_STATE_CHANGED_LISTENER;

    private static final BackgroundSideChannelCanceledListener NULL_BACKGROUND_SIDE_CHANNEL_CANCELED_LISTENER = new BackgroundSideChannelCanceledListener() {

        @Override
        public void onBackgroundSideChannelCanceled(Area areaCanceled) {
        }

    };

    private BackgroundSideChannelCanceledListener backgroundSideChannelCanceledListener = NULL_BACKGROUND_SIDE_CHANNEL_CANCELED_LISTENER;

    private boolean isBackgroundListenerRegistered;

    private Application registeredApplication;

    private boolean released = false;

    /**
     * Creates a new DigitalPenManager instance.
     *
     * @param application the application handle
     */
    public DigitalPenManager(Application application) {
        if (application == null) {
            throw new IllegalArgumentException("Application shouldn't be null");
        }
        init(application);
    }

    /*
     * This method exists for testing purposes only
     */
    protected DigitalPenManager() {
        // do not call init; deriving class must call init explicitly
    }

    /**
     * Returns true if the feature is available and false if it is not.
     *
     * @param feature the {@link Feature} being checked.
     * @return true if feature is supported
     */
    public static boolean isFeatureSupported(Feature feature) {
        if (feature == null) {
            throw new IllegalArgumentException("Feature shouldn't be null");
        }
        switch (feature) {
            case BASIC_DIGITAL_PEN_SERVICES:
                return isBasicServiceSupported();
            case DIGITAL_PEN_ENABLED:
                return checkPenEnabled();
            case SIDE_CHANNEL:
                return checkSideChannelSupported();
            default:
                return false;
        }
    }

    private static boolean isBasicServiceSupported() {
        return isBasicServiceSupported(DIGITAL_PEN_SYSTEM_SERVICE_NAME);
    }

    private static boolean isBasicServiceSupported(String serviceName) {
        if (Build.FINGERPRINT.contains("generic")) {
            Log.d(TAG, "Detected emulator due to \"generic\" as part of build fingerprint");
            return false;
        }
        Resources systemResources = Resources.getSystem();
        if (!systemResources.getBoolean(com.android.internal.R.bool.config_digitalPenCapable)) {
            Log.d(TAG, "System not configured to be digital pen capable");
            return false;
        }
        try {
            bindService(serviceName);
        } catch (PenNotSupportedException e) {
            Log.d(TAG, "Couldn't connect to digital pen service");
            return false;
        }
        return true;
    }

    private static boolean checkPenEnabled() {
        try {
            return bindService().isEnabled();
        } catch (RemoteException e) {
            e.printStackTrace();
            return false;
        } catch (PenNotSupportedException e) {
            throw new RuntimeException(e);
        }
    }

    private static boolean checkSideChannelSupported() {
        try {
            bindService(); // fills-out isSideChannelSupported
        } catch (PenNotSupportedException e) {
            throw new RuntimeException(e);
        }
        return isSideChannelSupported;
    }

    private class ActivityLifecycleHandler implements ActivityLifecycleCallbacks {

        @Override
        public void onActivityCreated(Activity activity, Bundle savedInstanceState) {
        }

        @Override
        public void onActivityStarted(Activity activity) {
        }

        @Override
        public void onActivityResumed(Activity activity) {
            applyCachedAppSettings();
            attachCallbacksToService();
        }

        @Override
        public void onActivityPaused(Activity activity) {
            if (isBackgroundListenerRegistered) {
                Bundle backgroundSettings = new Bundle(appSettings);
                backgroundSettings
                        .putBoolean(AppInterfaceKeys.OFF_SCREEN_BACKGROUND_LISTENER, true);
                try {
                    service.applyAppSettings(backgroundSettings);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            } else {
                if (!releaseAppFromService()) {
                    Log.w(TAG, "onActivityPaused: Problem releasing the application from service ");
                }
            }
        }

        @Override
        public void onActivityStopped(Activity activity) {
        }

        @Override
        public void onActivitySaveInstanceState(Activity activity, Bundle outState) {
        }

        @Override
        public void onActivityDestroyed(Activity activity) {
            releaseApplication();
        }
    };

    protected void init(Application application) {
        init(application, null);
    }

    protected void init(Application application, String serviceName) {
        // Detect if supported and attach to service.
        // The "serviceName" flavor is to substitute a different service for
        // testing
        boolean isSupported = (serviceName == null ? isBasicServiceSupported()
                : isBasicServiceSupported(serviceName));
        if (!isSupported) {
            throw new RuntimeException("Basic digital pen services not supported");
        }
        try {
            service = getServiceInterface();
        } catch (PenNotSupportedException e) {
            throw new RuntimeException(e);
        }

        // Currently the settings are app-wide; different activities within an
        // application with different settings needs will have to manage
        // their own settings
        activityLifecycleHandler = new ActivityLifecycleHandler();
        registeredApplication = application;
        registeredApplication.registerActivityLifecycleCallbacks(activityLifecycleHandler);
        appContext = registeredApplication.getApplicationContext();

        registeredApplication.registerReceiver(penEventReceiver, new IntentFilter(
                EventInterface.INTENT_ACTION));

        attachCallbacksToService();

        // Applying default settings immediately makes settings coherent with
        // daemon state.
        applySettings();
    }

    /**
     * Release the connection with the Snapdragon Digital Pen service. The
     * digital pen settings will revert to the system default.
     */
    public boolean releaseApplication() {
        if (!released) {
            released = true;
            registeredApplication.unregisterActivityLifecycleCallbacks(activityLifecycleHandler);
            registeredApplication.unregisterReceiver(penEventReceiver);
            return releaseAppFromService();
        } else {
            Log.w(TAG, "Application is already released");
            return false;
        }
    }

    private boolean releaseAppFromService() {
        try {
            service.releaseActivity();
            return true;
        } catch (RemoteException e) {
            e.printStackTrace();
            return false;
        }
    }

    /**
     * Returns the {@link Settings} interface which is used to modify and query
     * settings.
     *
     * @return the settings interface
     */
    public Settings getSettings() {
        return new Settings();
    }

    private boolean applySettings() {
        try {
            if (!service.applyAppSettings(appSettings)) {
                return false;
            }
            cachedAppSettings.clear();
            cachedAppSettings.putAll(appSettings);
            return true;
        } catch (Exception e) {
            Log.e(TAG, "applyAppSettings failed!");
            e.printStackTrace();
            return false;
        }
    }

    private void applyCachedAppSettings() {
        try {
            if (!service.applyAppSettings(cachedAppSettings)) {
            }
        } catch (Exception e) {
            Log.e(TAG, "applyCachedAppSettings failed!");
            e.printStackTrace();
        }
    }

    protected IDigitalPenService getServiceInterface() throws PenNotSupportedException {
        if (service != null) {
            return service;
        }
        IDigitalPenService boundService = bindService();

        service = boundService;
        return service;
    }

    static private IDigitalPenService bindService() throws PenNotSupportedException {
        return bindService(DIGITAL_PEN_SYSTEM_SERVICE_NAME);
    }

    static private IDigitalPenService bindService(String serviceName)
            throws PenNotSupportedException {
        IDigitalPenService boundService = IDigitalPenService.Stub.asInterface(ServiceManager
                .getService(serviceName));
        if (null == boundService) {
            throw new PenNotSupportedException("Could not connect to Digital Pen service");
        }
        queryServiceProperties(boundService);

        return boundService;
    }

    private void attachCallbacksToService() {
        try {
            service.registerDataCallback(new IDigitalPenDataCallback.Stub() {

                @Override
                public void onDigitalPenPropData(DigitalPenData data) throws RemoteException {
                    handleDataCallback(data);
                }

            });
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    /**
     * Register side channel event listener. This sets the callback for when
     * side channel events are received, but doesn't enable the side channel. To
     * enable the side channel itself, set its mapping with
     * {@link Settings#setSideChannelMapping(DigitalPenManager.Area, DigitalPenManager.SideChannelMapping)}
     *
     * @param area the area to listen for side-channel events
     * @param listener the listener that will be called for each event
     */
    public void registerSideChannelEventListener(Area area, OnSideChannelDataListener listener) {
        if (area == null) {
            throw new IllegalArgumentException("Area shouldn't be null");
        }

        if (listener == null) {
            throw new IllegalArgumentException("OnSideChannelDataListener shouldn't be null");
        }

        switch (area) {
            case ALL:
                allAreaDataListener = listener;
                break;
            case OFF_SCREEN_BACKGROUND:
                isBackgroundListenerRegistered = true;
                offScreenAreaDataListener = listener;
                break;
            case OFF_SCREEN:
                isBackgroundListenerRegistered = false;
                offScreenAreaDataListener = listener;
                break;
            case ON_SCREEN:
                onScreenAreaDataListener = listener;
                break;
            default:
                break;
        }
    }

    /**
     * Unregister side channel event listener. Platform must support
     * {@link Feature#SIDE_CHANNEL}. This only unregisters the listener. To
     * disable the side-channel, call
     * {@link Settings#setSideChannelMapping(DigitalPenManager.Area, DigitalPenManager.SideChannelMapping)}
     * with {@link SideChannelMapping#DISABLED}.
     *
     * @param area the area from which to unregister the listener.
     */
    public void unregisterSideChannelEventListener(Area area) {
        if (area == null) {
            throw new IllegalArgumentException("Area shouldn't be null");
        }

        switch (area) {
            case ALL:
                allAreaDataListener = NULL_DATA_LISTENER;
                break;
            case OFF_SCREEN_BACKGROUND:
                if (!isBackgroundListenerRegistered) {
                    Log.w(TAG, "Ignoring request to unregister " + Area.OFF_SCREEN_BACKGROUND
                            + " because it is not registered");
                    return;
                }
                isBackgroundListenerRegistered = false;
                offScreenAreaDataListener = NULL_DATA_LISTENER;
                break;
            case OFF_SCREEN:
                if (isBackgroundListenerRegistered) {
                    Log.w(TAG, "Ignoring request to unregister " + Area.OFF_SCREEN + " because "
                            + Area.OFF_SCREEN_BACKGROUND + " is registered");
                    return;
                }
                offScreenAreaDataListener = NULL_DATA_LISTENER;
                break;
            case ON_SCREEN:
                onScreenAreaDataListener = NULL_DATA_LISTENER;
                break;
            default:
                break;
        }
    }

    /**
     * Returns the Snapdragon Digital Pen off-screen Display handle. This is the
     * virtual display that's available for user input when selecting
     * {@link OffScreenMode#EXTEND}.
     *
     * @return the off-screen display
     */
    public Display getOffScreenDisplay() {
        DisplayManager displayMgr = (DisplayManager) appContext
                .getSystemService(Context.DISPLAY_SERVICE);
        Display[] displays = displayMgr.getDisplays();
        Display offscreenDisplay = null;
        for (Display display : displays) {
            String displayName = display.getName();
            if (displayName.equals(DIGITAL_PEN_OFF_SCREEN_DISPLAY_NAME)) {
                offscreenDisplay = display;
                break;
            }
        }

        if (offscreenDisplay == null) {
            Log.e(TAG, "No off-screen display found");
        }
        return offscreenDisplay;
    }

    /**
     * Sets the mic blocked listener. This is called whenever the number of
     * microphones detected to be blocked changes.
     *
     * @param listener the new mic blocked listener, or null to unregister.
     */
    public void setMicBlockedListener(MicBlockedListener listener) {
        if (listener == null) {
            micBlockedListener = NULL_MIC_BLOCKED_LISTENER;
        } else {
            micBlockedListener = listener;
            listener.onMicBlocked(cachedMicsBlocked);
        }
    }

    /**
     * Sets the listener for Power State Changed events. This is called whenever
     * the power state of the framework changes.
     *
     * @param listener the power state changed listener, or null to unregister.
     */
    public void setPowerStateChangedListener(PowerStateChangedListener listener) {
        if (listener == null) {
            powerStateChangedListener = NULL_POWER_STATE_CHANGED_LISTENER;
        } else {
            powerStateChangedListener = listener;
            listener.onPowerStateChanged(cachedCurrentPowerState, cachedLastPowerState);
        }
    }

    /**
     * Sets the listener for Battery State Changed events. This is called
     * whenever the battery state changes.
     *
     * @param listener the battery state changed listener, or null to
     *            unregister.
     */
    public void setBatteryStateChangedListener(BatteryStateChangedListener listener) {
        if (listener == null) {
            batteryStateChangedListener = NULL_BATTERY_STATE_CHANGED_LISTENER;
        } else {
            batteryStateChangedListener = listener;
            listener.onBatteryStateChanged(cachedBatteryState);
        }
    }

    /**
     * Sets the listener for when background side-channel listeners are
     * canceled. This can happen if another application tries to use the
     * DigitalPenManager while this application is listening in background.
     *
     * @param listener the background side channel canceled listener, or null to
     *            unregister.
     */
    public void setBackgroundSideChannelCanceledListener(
            BackgroundSideChannelCanceledListener listener) {
        if (listener == null) {
            backgroundSideChannelCanceledListener = NULL_BACKGROUND_SIDE_CHANNEL_CANCELED_LISTENER;
        } else {
            backgroundSideChannelCanceledListener = listener;
        }
    }

    /**
     * Causes DigitalPenManager to log the current version information for the
     * Snapdragon DigitalPenManager and Snapdragon Digital Pen service via
     * logcat.
     */
    public static void logVersions() {
        try {
            bindService();
        } catch (PenNotSupportedException e) {
            Log.e(TAG, "Cannot connect to digital pen service");
            // fall through to log versions; DigitalPenManager version still
            // valid, service version will be <unknown>
        }

        Log.i(TAG, "versions: DigitalPenManager v" +
                VERSION +
                ", Service v" +
                serviceVersion);
    }

}
