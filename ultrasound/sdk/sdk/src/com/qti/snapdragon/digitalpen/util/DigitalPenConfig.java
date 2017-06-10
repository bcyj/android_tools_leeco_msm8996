/*===========================================================================
                           DigitalPenConfig.java

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.

Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.snapdragon.digitalpen.util;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;

import android.os.Parcelable;
import android.os.Parcel;

/**
 * This class is a "container" for the configuration parameters, its function
 * are getters/setters for the following parameters:
 * <p>
 * 1) On screen: If on screen is enabled, touch events while the pen is located
 * in the area of the device screen will be sent.
 * <p>
 * 2) Off screen: If off screen is enabled, touch events while the pen is
 * located in the area of the outer screen will be sent
 * <p>
 * 3) 3D: If the 3D is enabled, the user will start getting hover events.
 * <p>
 * 4) Tool type: The type of the touch events that will be sent. For now the
 * following types could be chosen: a) {@link #DP_TOOL_TYPE_STYLUS} b)
 * {@link #DP_TOOL_TYPE_STYLUS_POINTER}
 * <p>
 * 5) Off screen mode: For pen-aware apps, how to address off-screen area a)
 * {@link #DP_OFF_SCREEN_MODE_DISABLED} b) {@link #DP_OFF_SCREEN_MODE_EXTEND} c)
 * {@link #DP_OFF_SCREEN_MODE_DUPLICATE}
 * <p>
 * <b>The configuration parameters only effect events sent by the Android input
 * module, it won't effect events sent to the services.</b>
 *
 * @hide
 */
public class DigitalPenConfig implements Parcelable {
    // TODO: OEM sends portrait side, but service should send 3x3 plane to
    // daemon
    // OFF_SCREEN_MODE defines
    /**
     * Off-screen is disabled.
     */
    public static final byte DP_OFF_SCREEN_MODE_DISABLED = 0;

    /**
     * Off-screen is extended onto a separate logical display
     */
    public static final byte DP_OFF_SCREEN_MODE_EXTEND = 1;

    /**
     * Off-screen is a duplication or mirror of on-screen
     */
    public static final byte DP_OFF_SCREEN_MODE_DUPLICATE = 2;

    // TODO: comments

    public static final byte DP_COORD_DESTINATION_MOTION_EVENT = 0;

    public static final byte DP_COORD_DESTINATION_SOCKET = 1;

    public static final byte DP_COORD_DESTINATION_BOTH = 2;

    public static final byte DP_ERASE_MODE_HOLD = 0;

    public static final byte DP_ERASE_MODE_TOGGLE = 1;

    public static final byte DP_POWER_PROFILE_OPTIMIZE_ACCURACY = 0;

    public static final byte DP_POWER_PROFILE_OPTIMIZE_POWER = 1;

    public static final byte DP_PORTRAIT_SIDE_LEFT = 0;

    public static final byte DP_PORTRAIT_SIDE_RIGHT = 1;

    public static final byte DP_SMARTER_STAND_ORIENTATION_LANDSCAPE = 0;

    public static final byte DP_SMARTER_STAND_ORIENTATION_PORTRAIT = 1;

    public static final byte DP_SMARTER_STAND_ORIENTATION_LANDSCAPE_REVERSED = 2;

    public static final byte DP_SMARTER_STAND_ORIENTATION_PORTRAIT_REVERSED = 3;

    private static class OffScreen {
        public byte mode;

        public SmarterStand smarterStand = new SmarterStand();

        public float[] origin = new float[3];

        public float[] endX = new float[3];

        public float[] endY = new float[3];

        public byte portraitSide;
    };

    private static class SmarterStand {
        public boolean enable;

        public int supportedOffScreenOrientation;
    };

    private static class Hover {
        public int maxRange; // in mm

        public boolean enable;

        public boolean showIcon;
    };

    private class CoordReport {
        public byte destination;

        public boolean isMapped;
    }

    private static class EraseButton {
        public int index;

        public byte mode;
    }

    private static final int MAX_CONFIG_MSG_SIZE = 1024;

    private int mTouchRange; // in mm

    private byte mPowerSaveMode;

    private OffScreen mOffScreen = new OffScreen();

    private Hover mOnScreenHover = new Hover();

    private Hover mOffScreenHover = new Hover();

    private final CoordReport mOnScreenCoordReport = new CoordReport();

    private final CoordReport mOffScreenCoordReport = new CoordReport();

    private EraseButton mEraseButton = new EraseButton();

    private boolean mSendAllDataEventsToSideChannel;

    private boolean mStopPenOnScreenOff;

    private boolean mStartPenOnBoot;

    // TODO: remove old binary enable/disable methods
    // TODO: document new methods

    /**
     * Sets the off-screen mode
     *
     * @param <code>mode</code> the off-screen mode to set.
     */
    public DigitalPenConfig setOffScreenMode(byte mode) {
        switch (mode) {
            case DP_OFF_SCREEN_MODE_DISABLED:
            case DP_OFF_SCREEN_MODE_DUPLICATE:
            case DP_OFF_SCREEN_MODE_EXTEND:
                mOffScreen.mode = mode;
                break;
            default:
                throw new IllegalArgumentException("Bad off-screen mode: " + mode);
        }
        return this;
    }

    /**
     * Gets the off-screen mode.
     *
     * @return The off-screen mode
     */
    public byte getOffScreenMode() {
        return mOffScreen.mode;
    }

    public int getTouchRange() {
        return mTouchRange;
    }

    public DigitalPenConfig setTouchRange(int i) {
        // TODO: validation?
        mTouchRange = i;
        return this;
    }

    public byte getPowerSave() {
        return mPowerSaveMode;
    }

    public DigitalPenConfig setPowerSave(byte mode) {
        // TODO: validation?
        mPowerSaveMode = mode;
        return this;
    }

    /**
     * Returns a new configuration object initialized with the default
     * configuration on the device.
     */
    public DigitalPenConfig() {
        // Default configuration (Hard-coded, think about moving to config file)
        mTouchRange = 35;
        mPowerSaveMode = DP_POWER_PROFILE_OPTIMIZE_ACCURACY;
        mOnScreenHover.maxRange = 400;
        mOnScreenHover.enable = true;
        mOnScreenHover.showIcon = true;
        mOffScreenHover.maxRange = 400;
        mOffScreenHover.enable = true;
        mOffScreenHover.showIcon = true;
        mOffScreen.mode = DP_OFF_SCREEN_MODE_DUPLICATE;
        mOffScreen.smarterStand.enable = true;
        mOffScreen.smarterStand.supportedOffScreenOrientation = DP_SMARTER_STAND_ORIENTATION_LANDSCAPE;
        mOffScreen.origin = new float[] {
                -12.4f, 332.76f, -10.0f
        };
        mOffScreen.endX = new float[] {
                243.92f, 332.76f, -10.0f
        };
        mOffScreen.endY = new float[] {
                -12.4f, 188.58f, -10.0f
        };
        mOffScreen.portraitSide = DP_PORTRAIT_SIDE_RIGHT;
        mOnScreenCoordReport.destination = DP_COORD_DESTINATION_MOTION_EVENT;
        mOnScreenCoordReport.isMapped = false;
        mOffScreenCoordReport.destination = DP_COORD_DESTINATION_MOTION_EVENT;
        mOffScreenCoordReport.isMapped = false;
        mEraseButton.index = 1;
        mEraseButton.mode = DP_ERASE_MODE_TOGGLE;
        mSendAllDataEventsToSideChannel = false;
        mStopPenOnScreenOff = false;
        mStartPenOnBoot = false;
    }

    // -- Parcelable functions --
    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeByte(mOffScreen.mode);
        out.writeInt(mTouchRange);
        out.writeByte(mPowerSaveMode);
        out.writeInt(mOnScreenHover.maxRange);
        out.writeInt(mOnScreenHover.enable ? 1 : 0);
        out.writeInt(mOnScreenHover.showIcon ? 1 : 0);
        out.writeInt(mOffScreenHover.maxRange);
        out.writeInt(mOffScreenHover.enable ? 1 : 0);
        out.writeInt(mOffScreenHover.showIcon ? 1 : 0);
        out.writeInt(mOffScreen.smarterStand.enable ? 1 : 0);
        out.writeInt(mOffScreen.smarterStand.supportedOffScreenOrientation);
        out.writeFloatArray(mOffScreen.origin);
        out.writeFloatArray(mOffScreen.endX);
        out.writeFloatArray(mOffScreen.endY);
        out.writeByte(mOnScreenCoordReport.destination);
        out.writeInt(mOnScreenCoordReport.isMapped ? 1 : 0);
        out.writeByte(mOffScreenCoordReport.destination);
        out.writeInt(mOffScreenCoordReport.isMapped ? 1 : 0);
        out.writeInt(mEraseButton.index);
        out.writeByte(mEraseButton.mode);
        out.writeInt(mSendAllDataEventsToSideChannel ? 1 : 0);
        out.writeInt(mStopPenOnScreenOff ? 1 : 0);
        out.writeInt(mStartPenOnBoot ? 1 : 0);
    }

    public static final Parcelable.Creator<DigitalPenConfig> CREATOR = new Parcelable.Creator<DigitalPenConfig>() {
        @Override
        public DigitalPenConfig createFromParcel(Parcel in) {
            return new DigitalPenConfig(in);
        }

        @Override
        public DigitalPenConfig[] newArray(int size) {
            return new DigitalPenConfig[size];
        }
    };

    private DigitalPenConfig(Parcel in) {
        mOffScreen.mode = in.readByte();
        mTouchRange = in.readInt();
        mPowerSaveMode = in.readByte();
        mOnScreenHover.maxRange = in.readInt();
        mOnScreenHover.enable = (in.readInt() == 1);
        mOnScreenHover.showIcon = (in.readInt() == 1);
        mOffScreenHover.maxRange = in.readInt();
        mOffScreenHover.enable = (in.readInt() == 1);
        mOffScreenHover.showIcon = (in.readInt() == 1);
        mOffScreen.smarterStand.enable = (in.readInt() == 1);
        mOffScreen.smarterStand.supportedOffScreenOrientation = in.readInt();
        in.readFloatArray(mOffScreen.origin);
        in.readFloatArray(mOffScreen.endX);
        in.readFloatArray(mOffScreen.endY);
        mOnScreenCoordReport.destination = in.readByte();
        mOnScreenCoordReport.isMapped = (in.readInt() == 1);
        mOffScreenCoordReport.destination = in.readByte();
        mOffScreenCoordReport.isMapped = (in.readInt() == 1);
        mEraseButton.index = in.readInt();
        mEraseButton.mode = in.readByte();
        mSendAllDataEventsToSideChannel = (in.readInt() == 1);
        mStopPenOnScreenOff = (in.readInt() == 1);
        mStartPenOnBoot = (in.readInt() == 1);
    }

    public DigitalPenConfig setOnScreenHoverMaxRange(int maxRange) {
        mOnScreenHover.maxRange = maxRange;
        return this;
    }

    public int getOnScreenHoverMaxRange() {
        return mOnScreenHover.maxRange;
    }

    public DigitalPenConfig setOnScreenHoverEnable(boolean enable) {
        mOnScreenHover.enable = enable;
        return this;
    }

    public boolean isOnScreenHoverEnabled() {
        return mOnScreenHover.enable;
    }

    public DigitalPenConfig setShowOnScreenHoverIcon(boolean show) {
        mOnScreenHover.showIcon = show;
        return this;
    }

    public boolean isShowingOnScreenHoverIcon() {
        return mOnScreenHover.showIcon;
    }

    public void setOffScreenHoverMaxRange(int maxRange) {
        mOffScreenHover.maxRange = maxRange;
    }

    public int getOffScreenHoverMaxRange() {
        return mOffScreenHover.maxRange;
    }

    public void setOffScreenHoverEnable(boolean enable) {
        mOffScreenHover.enable = enable;
    }

    public boolean isOffScreenHoverEnabled() {
        return mOffScreenHover.enable;
    }

    public void setShowOffScreenHoverIcon(boolean show) {
        mOffScreenHover.showIcon = show;
    }

    public boolean isShowingOffScreenHoverIcon() {
        return mOffScreenHover.showIcon;
    }

    public DigitalPenConfig setSmarterStand(boolean enable) {
        mOffScreen.smarterStand.enable = enable;
        return this;
    }

    public boolean isSmarterStandEnabled() {
        return mOffScreen.smarterStand.enable;
    }

    public void setSmarterStandSupportedOrientation(int supportedOrientation) {
        mOffScreen.smarterStand.supportedOffScreenOrientation = supportedOrientation;
    }

    public int getSmarterStandSupportedOrientation() {
        return mOffScreen.smarterStand.supportedOffScreenOrientation;
    }

    public DigitalPenConfig setOffScreenPlane(float[] origin, float[] endX, float[] endY) {
        mOffScreen.origin = Arrays.copyOf(origin, 3);
        mOffScreen.endX = Arrays.copyOf(endX, 3);
        mOffScreen.endY = Arrays.copyOf(endY, 3);
        return this;
    }

    public void getOffScreenPlane(float[] origin, float[] endX, float[] endY) {
        for (int i = 0; i < 3; ++i) {
            origin[i] = mOffScreen.origin[i];
            endX[i] = mOffScreen.endX[i];
            endY[i] = mOffScreen.endY[i];
        }
    }

    public DigitalPenConfig setOnScreenCoordReporting(byte destination, boolean isMapped) {
        mOnScreenCoordReport.destination = destination;
        mOnScreenCoordReport.isMapped = isMapped;
        return this;
    }

    public DigitalPenConfig setOffScreenCoordReporting(byte destination, boolean isMapped) {
        mOffScreenCoordReport.destination = destination;
        mOffScreenCoordReport.isMapped = isMapped;
        return this;

    }

    public byte getOnScreenCoordReportDestination() {
        return mOnScreenCoordReport.destination;
    }

    public byte getOffScreenCoordReportDestination() {
        return mOffScreenCoordReport.destination;
    }

    public boolean getOnScreenCoordReportIsMapped() {
        return mOnScreenCoordReport.isMapped;
    }

    public boolean getOffScreenCoordReportIsMapped() {
        return mOffScreenCoordReport.isMapped;
    }

    public DigitalPenConfig setSendAllDataEventsToSideChannel(boolean enable) {
        mSendAllDataEventsToSideChannel = enable;
        return this;
    }

    public boolean getSendAllDataEventsToSideChannel() {
        return mSendAllDataEventsToSideChannel;
    }

    public DigitalPenConfig setStopPenOnScreenOff(boolean enable) {
        mStopPenOnScreenOff = enable;
        return this;
    }

    public boolean isStopPenOnScreenOffEnabled() {
        return mStopPenOnScreenOff;
    }

    public DigitalPenConfig setStartPenOnBoot(boolean enable) {
        mStartPenOnBoot = enable;
        return this;
    }

    public boolean isStartPenOnBootEnabled() {
        return mStartPenOnBoot;
    }

    public DigitalPenConfig setEraseButtonIndex(int index) {
        mEraseButton.index = index;
        return this;
    }

    public int getEraseButtonIndex() {
        return mEraseButton.index;
    }

    public DigitalPenConfig setEraseButtonBehavior(byte mode) {
        switch (mode) {
            case DP_ERASE_MODE_HOLD:
            case DP_ERASE_MODE_TOGGLE:
                mEraseButton.mode = mode;
                break;
            default:
                throw new IllegalArgumentException("Bad eraser-button mode: " + mode);
        }
        return this;
    }

    public byte getEraseButtonMode() {
        return mEraseButton.mode;
    }

    public DigitalPenConfig setOffScreenPortraitSide(byte code) {
        // TODO: validate
        mOffScreen.portraitSide = code;
        return this;
    }

    public byte getOffSceenPortraitSide() {
        return mOffScreen.portraitSide;
    }

    public byte[] marshalForDaemon() {
        // Make the daemon change configuration
        ByteBuffer msg = ByteBuffer.allocate(MAX_CONFIG_MSG_SIZE);
        msg.order(ByteOrder.LITTLE_ENDIAN);

        float[] origin = new float[3];
        float[] endX = new float[3];
        float[] endY = new float[3];
        getOffScreenPlane(origin, endX, endY);
        msg.put(getOffScreenMode()).putInt(getTouchRange()).put(getPowerSave())
                .putInt(getOnScreenHoverMaxRange()).put((byte)(isOnScreenHoverEnabled() ? 1 : 0))
                .put((byte)(isShowingOnScreenHoverIcon() ? 1 : 0))
                .putInt(getOffScreenHoverMaxRange()).put((byte)(isOffScreenHoverEnabled() ? 1 : 0))
                .put((byte)(isShowingOffScreenHoverIcon() ? 1 : 0))
                .put((byte)(isSmarterStandEnabled() ? 1 : 0)).putFloat(origin[0])
                .putFloat(origin[1]).putFloat(origin[2]).putFloat(endX[0]).putFloat(endX[1])
                .putFloat(endX[2]).putFloat(endY[0]).putFloat(endY[1]).putFloat(endY[2])
                .put(getOnScreenCoordReportDestination())
                .put((byte)(getOnScreenCoordReportIsMapped() ? 1 : 0))
                .put(getOffScreenCoordReportDestination())
                .put((byte)(getOffScreenCoordReportIsMapped() ? 1 : 0))
                .putInt(getEraseButtonIndex()).put(getEraseButtonMode())
                .put((byte)(getSendAllDataEventsToSideChannel() ? 1 : 0));

        // return trimmed message
        return Arrays.copyOf(msg.array(), msg.position());
    }

}
