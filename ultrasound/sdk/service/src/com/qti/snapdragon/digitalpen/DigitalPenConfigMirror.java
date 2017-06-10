/*===========================================================================
                            DigitalPenConfigMirror.java

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.

Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.snapdragon.digitalpen;

import java.lang.reflect.Field;

import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;

import com.qti.snapdragon.digitalpen.util.DigitalPenConfig;

/**
 * This class mirrors the DigitalPenConfig in fields only. Its purpose is to
 * allow persistence through the simple xml framework, but to shield the SDK
 * (where DigitalPenConfig is necessarily included) from needing that framework.
 * <p>
 * Hiding the implementation in this way imposes a maintenance risk: adding or
 * changing fields in DigitalPenConfig that need to persist must be mirrored
 * here.
 *
 * @hide
 */
@Root(name = "GlobalSettings")
public class DigitalPenConfigMirror {

    static public enum CopyDirection {
        MIRROR_TO_TRUE,
        TRUE_TO_MIRROR
    }

    private static class OffScreen {
        @Element(name = "Mode")
        public byte mode;

        @Element(name = "SmarterStand")
        public SmarterStand smarterStand = new SmarterStand();

        @Element(name = "Origin")
        public float[] origin = new float[3];

        @Element(name = "EndX")
        public float[] endX = new float[3];

        @Element(name = "EndY")
        public float[] endY = new float[3];

        @Element(name = "PortraitSide")
        public byte portraitSide;
    };

    private static class SmarterStand {
        @Element(name = "Enable")
        public boolean enable;

        @Element(name = "SupportedOffScreenOrientation")
        public int supportedOffScreenOrientation;
    };

    private static class Hover {
        @Element(name = "MaxRange")
        public int maxRange; // in mm

        @Element(name = "Enable")
        public boolean enable;

        @Element(name = "ShowIcon")
        public boolean showIcon;
    };

    private static class EraseButton {
        @Element(name = "Index")
        public int index;

        @Element(name = "Mode")
        public byte mode;
    }

    @Element(name = "TouchRange")
    private int mTouchRange; // in mm

    @Element(name = "PowerSaveMode")
    private byte mPowerSaveMode;

    @Element(name = "OffScreen")
    private OffScreen mOffScreen = new OffScreen();

    @Element(name = "OnScreenHover")
    private Hover mOnScreenHover = new Hover();

    @Element(name = "OffScreenHover")
    private Hover mOffScreenHover = new Hover();

    @Element(name = "EraseButton")
    private EraseButton mEraseButton = new EraseButton();

    @Element(name = "StopPenOnScreenOff")
    private boolean mStopPenOnScreenOff;

    @Element(name = "StartPenOnBoot")
    private boolean mStartPenOnBoot;

    public void copyPersistedFields(DigitalPenConfig trueObj, CopyDirection dir) {
        try {
            doCopyPersistedFields(this, trueObj, dir);
        } catch (Exception e) {
            throw new RuntimeException("Problem persisting fields in direction: " + dir, e);
        }
    }

    private static void doCopyPersistedFields(Object mirrorObj, Object trueObj, CopyDirection dir)
            throws NoSuchFieldException, IllegalAccessException, IllegalArgumentException {
        Field[] mirrorFields = mirrorObj.getClass().getDeclaredFields();
        for (Field mirrorField : mirrorFields) {
            if (mirrorField.getAnnotation(Element.class) != null) {
                String fieldName = mirrorField.getName();
                Field trueField = trueObj.getClass().getDeclaredField(fieldName);
                mirrorField.setAccessible(true);
                trueField.setAccessible(true);
                if (mirrorField.getType().getFields().length == 0) {
                    // no children
                    if (trueField.getType() != mirrorField.getType()) {
                        throw new IllegalArgumentException("Type of field '"
                                + mirrorField.getName()
                                + "' differs");
                    }
                    if (dir == CopyDirection.MIRROR_TO_TRUE) {
                        trueField.set(trueObj, mirrorField.get(mirrorObj));
                    } else {
                        mirrorField.set(mirrorObj, trueField.get(trueObj));
                    }
                } else {
                    // recurse through children
                    doCopyPersistedFields(mirrorField.get(mirrorObj), trueField.get(trueObj), dir);
                }
            }
        }
    }
}
