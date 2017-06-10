/*===========================================================================
                           ConfigManager.java

Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.snapdragon.digitalpen;

import com.qti.snapdragon.digitalpen.util.DigitalPenConfig;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.OffScreenMode;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.SideChannelMapping;
import com.qti.snapdragon.sdk.digitalpen.impl.AppInterfaceKeys;

import android.os.Bundle;
import android.os.Parcel;
import android.util.Log;

/**
 * This is a helper class that applies application settings on top of an
 * existing ("global") DigitalPenConfig.
 */
public class AppSettingsAdapter {

    private static final String TAG = "AppSettingsManager";

    private AppSettingsAdapter() {
        // only static methods; shouldn't be constructed
    }

    /**
     * Returns a copy of a configuration based on the global config but modified
     * to apply the application settings.
     *
     * @param globalConfig the global configuration that forms basis of the
     *            settings
     * @param appSettings the application settings to apply on top of
     *            globalConfig
     * @return a copy of a configuration with appSettings applied to the
     *         globalConfig
     */
    static public DigitalPenConfig applyAppSettings(DigitalPenConfig globalConfig,
            Bundle appSettings) {
        DigitalPenConfig currentConfig = copyOfConfig(globalConfig);

        // TODO: dispatch table or reflection
        // TODO: validate keys in separate step since they should all exist
        if (appSettings.containsKey(AppInterfaceKeys.OFF_SCREEN_MODE)) {
            DigitalPenManager.OffScreenMode mode = (OffScreenMode) appSettings
                    .getSerializable(AppInterfaceKeys.OFF_SCREEN_MODE);
            byte configMode;
            switch (mode) {
                case DISABLED:
                    configMode = DigitalPenConfig.DP_OFF_SCREEN_MODE_DISABLED;
                    break;
                case DUPLICATE:
                    configMode = DigitalPenConfig.DP_OFF_SCREEN_MODE_DUPLICATE;
                    break;
                case EXTEND:
                    configMode = DigitalPenConfig.DP_OFF_SCREEN_MODE_EXTEND;
                    break;
                default:
                    throw new RuntimeException("Unknown mode: " + mode);
            }
            currentConfig.setOffScreenMode(configMode);
        }

        if (appSettings.containsKey(AppInterfaceKeys.ON_SCREEN_HOVER_ENABLED)) {
            currentConfig.setOnScreenHoverEnable(appSettings
                    .getBoolean(AppInterfaceKeys.ON_SCREEN_HOVER_ENABLED));
        }

        if (appSettings.containsKey(AppInterfaceKeys.ON_SCREEN_HOVER_MAX_DISTANCE)) {
            currentConfig.setOnScreenHoverMaxRange(appSettings
                    .getInt(AppInterfaceKeys.ON_SCREEN_HOVER_MAX_DISTANCE));
        }

        if (appSettings.containsKey(AppInterfaceKeys.OFF_SCREEN_HOVER_ENABLED)) {
            currentConfig.setOffScreenHoverEnable(appSettings
                    .getBoolean(AppInterfaceKeys.OFF_SCREEN_HOVER_ENABLED));
        }

        if (appSettings.containsKey(AppInterfaceKeys.OFF_SCREEN_HOVER_MAX_DISTANCE)) {
            currentConfig.setOffScreenHoverMaxRange(appSettings
                    .getInt(AppInterfaceKeys.OFF_SCREEN_HOVER_MAX_DISTANCE));
        }

        if (appSettings.containsKey(AppInterfaceKeys.ERASER_BYPASS)) {
            if (appSettings.getBoolean(AppInterfaceKeys.ERASER_BYPASS)) {
                currentConfig.setEraseButtonIndex(-1);
            }
        }

        if (appSettings.containsKey(AppInterfaceKeys.INPUT_TYPE)) {
            // TODO: implement INPUT_TYPE
            Log.w(TAG, "Implementation of Input Type for Digital Pen is incomplete");
        }

        // on-screen mapping
        if (appSettings.containsKey(AppInterfaceKeys.ON_SCREEN_MAPPING)) {
            boolean isMapped =
                    SideChannelMapping.SCALED == (SideChannelMapping) appSettings
                            .getSerializable(AppInterfaceKeys.ON_SCREEN_MAPPING);
            currentConfig.setOnScreenCoordReporting(DigitalPenConfig.DP_COORD_DESTINATION_BOTH,
                    isMapped);
        }

        // all mapping
        if (appSettings.containsKey(AppInterfaceKeys.ALL_MAPPING)) {
            currentConfig.setSendAllDataEventsToSideChannel(true);
        }

        // off-screen mapping
        if (appSettings.containsKey(AppInterfaceKeys.OFF_SCREEN_MAPPING)) {
            boolean isMapped =
                    SideChannelMapping.SCALED == (SideChannelMapping) appSettings
                            .getSerializable(AppInterfaceKeys.OFF_SCREEN_MAPPING);
            byte destination;
            if (appSettings.getBoolean(AppInterfaceKeys.OFF_SCREEN_BACKGROUND_LISTENER, false)) {
                // background listener -- special configuration
                destination = DigitalPenConfig.DP_COORD_DESTINATION_SOCKET;
                currentConfig.setOffScreenMode(DigitalPenConfig.DP_OFF_SCREEN_MODE_EXTEND);
            } else {
                byte offScreenMode = currentConfig.getOffScreenMode();
                if (offScreenMode == DigitalPenConfig.DP_OFF_SCREEN_MODE_EXTEND
                        || offScreenMode == DigitalPenConfig.DP_OFF_SCREEN_MODE_DUPLICATE) {
                    destination = DigitalPenConfig.DP_COORD_DESTINATION_BOTH;
                } else {
                    destination = DigitalPenConfig.DP_COORD_DESTINATION_SOCKET;
                }
            }
            currentConfig.setOffScreenCoordReporting(destination, isMapped);
        }
        return currentConfig;
    }

    // parcels & unparcels object in lieu of writing clone or copy ctor
    /**
     * Copy of global config.
     *
     * @return the digital pen config
     */
    static public DigitalPenConfig copyOfConfig(DigitalPenConfig globalConfig) {
        Parcel parcel = Parcel.obtain();
        globalConfig.writeToParcel(parcel, 0);
        parcel.setDataPosition(0);
        DigitalPenConfig currentConfig = DigitalPenConfig.CREATOR.createFromParcel(parcel);
        parcel.recycle();
        return currentConfig;
    }

    // create empty bundle with just listener, off-screen mode = extend, and
    // copy of off-screen mapping
    public static Bundle copyForBackgroundOffScreenListener(Bundle baseSettings) {
        Bundle settings = new Bundle();
        settings.putBoolean(AppInterfaceKeys.OFF_SCREEN_BACKGROUND_LISTENER, true);
        settings.putSerializable(AppInterfaceKeys.OFF_SCREEN_MODE, OffScreenMode.EXTEND);
        settings.putSerializable(AppInterfaceKeys.OFF_SCREEN_MAPPING,
                baseSettings.getSerializable(AppInterfaceKeys.OFF_SCREEN_MAPPING));
        return settings;
    }
}
