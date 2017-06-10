/*===========================================================================
                           ConfigManager.java

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.

Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.snapdragon.digitalpen;

import android.os.Bundle;

import com.qti.snapdragon.digitalpen.util.DigitalPenConfig;
import com.qti.snapdragon.sdk.digitalpen.impl.AppInterfaceKeys;

public class ConfigManager {
    public interface ConfigChangedListener {
        void onConfigChanged(DigitalPenConfig newConfig, State state, boolean stateChanged);
    }

    /**
     * See below for state definitions.
     * <p>
     * In LEGACY, the global config should be applied on daemon directly.
     * <p>
     * In APP, the global state overlayed by the app settings should form the
     * config.
     * <p>
     * In LEGACY_WITH_SIDE_CHANNEL, there's a background off-screen side-channel
     * listener registered. Config is global global overlayed by minimum app
     * settings -- just do to with off-screen mapping and mode.
     * <p>
     * Actual implementation of the overlay code is in helper class
     * AppSettingsAdapter.
     */
    public enum State {
        /** no app, no side-channel */
        LEGACY,

        /**
         * app no longer foreground, but background side-channel registered
         */
        LEGACY_WITH_SIDE_CHANNEL,

        /** foreground app has DigitalPenManager w/ settings applied */
        APP,

    }

    private DigitalPenConfig globalConfig;
    private ConfigChangedListener listener;
    private Bundle appSettings = new Bundle();
    private State state;
    private Bundle backgroundSettings;

    public ConfigManager(DigitalPenConfig globalConfig, ConfigChangedListener listener) {
        this.globalConfig = globalConfig;
        this.listener = listener;
        state = State.LEGACY;
    }

    public void applyAppSettings(Bundle settings) {
        appSettings.clear();
        appSettings.putAll(settings);

        Bundle overlay;
        State newState;
        if (appHasBackgroundListener()) {
            // This is actually a command to go background with listener.
            backgroundSettings = AppSettingsAdapter.copyForBackgroundOffScreenListener(settings);
            overlay = backgroundSettings;
            newState = State.LEGACY_WITH_SIDE_CHANNEL;
        } else {
            overlay = appSettings;
            newState = State.APP;
        }
        updateListener(overlaySettingsOnGlobalConfig(overlay), newState);
    }

    public void appNowBackground() {
        releaseApp();
    }

    public DigitalPenConfig getCurrentConfig() {
        return overlaySettingsOnGlobalConfig(appSettings);
    }

    public DigitalPenConfig getGlobalConfig() {
        return AppSettingsAdapter.copyOfConfig(globalConfig);
    }

    public void releaseApp() {
        updateListener(globalConfig, State.LEGACY);
    }

    public void setGlobalConfig(DigitalPenConfig newGlobalConfig) {
        globalConfig = AppSettingsAdapter.copyOfConfig(newGlobalConfig);
        DigitalPenConfig newConfig;
        switch (state) {
            case APP:
                newConfig = AppSettingsAdapter.applyAppSettings(globalConfig, appSettings);
                break;
            case LEGACY:
                newConfig = globalConfig;
                break;
            case LEGACY_WITH_SIDE_CHANNEL:
                newConfig = AppSettingsAdapter.applyAppSettings(globalConfig, backgroundSettings);
                break;
            default:
                throw new RuntimeException("Programming error: state missing");
        }
        updateListener(newConfig, state);
    }

    private boolean appHasBackgroundListener() {
        return appSettings.getBoolean(AppInterfaceKeys.OFF_SCREEN_BACKGROUND_LISTENER, false);
    }

    private DigitalPenConfig overlaySettingsOnGlobalConfig(Bundle settings) {
        return AppSettingsAdapter.applyAppSettings(globalConfig, settings);
    }

    private void updateListener(DigitalPenConfig config, State newState) {
        boolean stateChanged = state != newState;
        state = newState;
        listener.onConfigChanged(AppSettingsAdapter.copyOfConfig(config), newState, stateChanged);
    }

}
