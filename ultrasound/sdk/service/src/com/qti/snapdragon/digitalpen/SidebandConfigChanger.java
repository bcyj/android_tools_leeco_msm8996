/*===========================================================================
                           ConfigChanger.java

Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/
package com.qti.snapdragon.digitalpen;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import android.content.Intent;
import android.os.Bundle;

import com.qti.snapdragon.digitalpen.util.DigitalPenConfig;

public class SidebandConfigChanger {

    private DigitalPenConfig config;

    public SidebandConfigChanger(DigitalPenConfig config) {
        this.config = config;
    }

    private interface ConfigCommand {
        void execute(Bundle b, String key, DigitalPenConfig config);
    }

    // TODO: switch to using reflection
    private static class ConfigField {

        private String fieldName;
        private ConfigCommand doCommand;

        public ConfigField(String fieldName, ConfigCommand doCommand) {
            this.fieldName = fieldName;
            this.doCommand = doCommand;
        }

        public void checkAndDoCommand(Bundle b, DigitalPenConfig config) {
            String key = "com.qti.snapdragon.digitalpen." + fieldName;
            if (b.containsKey(key)) {
                doCommand.execute(b, key, config);
            }
        }
    }

    protected static final String TAG = "LoadConfigIntentHandler";

    static List<ConfigField> configFieldHandlers = new ArrayList<ConfigField>();

    static {
        configFieldHandlers.add(new ConfigField("OffScreenMode", new ConfigCommand() {

            @Override
            public void execute(Bundle b, String key, DigitalPenConfig config) {
                String newMode = b.getString(key);
                config.setOffScreenMode(Byte.parseByte(newMode));
            }
        }));

        configFieldHandlers.add(new ConfigField("UseSmarterStand", new ConfigCommand() {

            @Override
            public void execute(Bundle b, String key, DigitalPenConfig config) {
                String enable = b.getString(key);
                config.setSmarterStand(Boolean.parseBoolean(enable));
            }
        }));

        configFieldHandlers.add(new ConfigField("OffScreenPlane", new ConfigCommand() {

            @Override
            public void execute(Bundle b, String key, DigitalPenConfig config) {
                float[] coords = b.getFloatArray(key);
                if (coords.length != 9) {
                    throw new IllegalArgumentException("For key " + key + ", expected " +
                            "array of length 9, received length " + coords.length);
                }
                float[] origin = Arrays.copyOfRange(coords, 0, 3);
                float[] endX = Arrays.copyOfRange(coords, 3, 6);
                float[] endY = Arrays.copyOfRange(coords, 6, 9);
                config.setOffScreenPlane(origin, endX, endY);
            }
        }));

        configFieldHandlers.add(new ConfigField("EraserMode", new ConfigCommand() {

            @Override
            public void execute(Bundle b, String key, DigitalPenConfig config) {
                String newMode = b.getString(key);
                config.setEraseButtonBehavior(Byte.parseByte(newMode));
            }
        }));

        configFieldHandlers.add(new ConfigField("EraserButtonIndex", new ConfigCommand() {

            @Override
            public void execute(Bundle b, String key, DigitalPenConfig config) {
                String newIndex = b.getString(key);
                config.setEraseButtonIndex(Integer.parseInt(newIndex));
            }
        }));
    }

    public DigitalPenConfig processIntent(Intent intent) {
        if (intent.getAction() != "com.qti.snapdragon.digitalpen.LOAD_CONFIG") {
            throw new IllegalArgumentException("Unexpected action: " + intent.getAction());
        }
        Bundle bundle = intent.getExtras();
        for (ConfigField field : configFieldHandlers) {
            field.checkAndDoCommand(bundle, config);
        }
        return config;
    }

}
