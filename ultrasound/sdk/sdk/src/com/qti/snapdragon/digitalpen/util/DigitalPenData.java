/*===========================================================================
                           DigitalPenData.java

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.

Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
package com.qti.snapdragon.digitalpen.util;

import java.util.Arrays;
import android.os.Parcelable;
import android.os.Parcel;

/**
 * This class is a "container" for the data parameters, its
 * function are getters for the following parameters:
 * <p>1) X Axis:   X axis for the pen position in 0.01mm
 * <p>2) Y Axis:   Y axis for the pen position in 0.01mm
 * <p>3) Z Axis:   Z axis for the pen position in 0.01mm
 * <p>4) X Tilt:   X coordinate of the pen tilt
 * <p>5) Y Tilt:   Y coordinate of the pen tilt
 * <p>6) Z Tilt:   Z coordinate of the pen tilt
 * <p>7) Pressure: The amount of pressure when the pen is
 *              pressed (0 if pen is not pressed)
 * <p>8) PenState: up/down
 * <p>9) SidebuttonsState: up/down
 * <p>10) Region: REGION_ALL, REGION_ON_SCREEN or
 * REGION_OFF_SCREEN
 *
 * @hide
 */
public class DigitalPenData implements Parcelable {
    public final static int REGION_ALL = 0;
    public final static int REGION_ON_SCREEN = 1;
    public final static int REGION_OFF_SCREEN = 2;
    private int    mX;
    private int    mY;
    private int    mZ;
    private int    mXTilt;
    private int    mYTilt;
    private int    mZTilt;
    private int    mPressure;
    private boolean mPenState;
    private int[] mSideButtonsState;
    private int    mRegion;
    private final static int NUM_OF_SIDE_BUTTONS = 3;

    public DigitalPenData() {
        // Empty, all default to 0.
    }

    public DigitalPenData(
            int    x,
            int    y,
            int    z,
            int    xTilt,
            int    yTilt,
            int    zTilt,
            int    pressure,
            boolean penState,
            int[] sideButtonsState,
            int    region
            ) {
        mX                = x;
        mY                = y;
        mZ                = z;
        mXTilt            = xTilt;
        mYTilt            = yTilt;
        mZTilt            = zTilt;
        mPressure         = pressure;
        mPenState         = penState;
        mSideButtonsState = Arrays.copyOf(sideButtonsState, NUM_OF_SIDE_BUTTONS);
        mRegion           = region;
    }
    /** @return X coordinate of this event. The number is in 0.01mm coordinates.*/
    public int    getX()            { return mX; }
    /** @return Y coordinate of this event. The number is in 0.01mm coordinates.*/
    public int    getY()            { return mY; }
    /** @return Z coordinate of this event. The number is in 0.01mm coordinates.*/
    public int    getZ()            { return mZ; }
    /**
     * @return The X coordinate of a normalized vector which
     * represents the tilt vector of this event.
     */
    public int    getXTilt()        { return mXTilt; }
    /**
     * @return The Y coordinate of a normalized vector which
     * represents the tilt vector of this event.
     */
    public int    getYTilt()        { return mYTilt; }
    /**
     * @return The Z coordinate of a normalized vector which
     * represents the tilt vector of this event.
     */
    public int    getZTilt()        { return mZTilt; }
    /**
     * @return The current pressure of this event. Value of the pressure is
     * between 0-255.
     */
    public int    getPressure()     { return mPressure; }
    /**
     * @return Pen state of this event. false when pen is up, and true when pen is
     * down.
     */
    public boolean getPenState()     { return mPenState; }
    /**
     * @return Side buttons state of this event. false
     * when side button is up, and true when side button is down.
     */
    public int[] getSideButtonsState()     { return mSideButtonsState; }
    /**
     * @return Region of the point: REGION_ALL, REGION_ON_SCREEN or REGION_OFF_SCREEN
     */
    public int getRegion()     { return mRegion; }

    // -- Parcelable functions --
    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mX);
        out.writeInt(mY);
        out.writeInt(mZ);
        out.writeInt(mXTilt);
        out.writeInt(mYTilt);
        out.writeInt(mZTilt);
        out.writeInt(mPressure);
        out.writeInt(mPenState? 1 : 0);
        out.writeIntArray(mSideButtonsState);
        out.writeInt(mRegion);
    }

    public static final Parcelable.Creator<DigitalPenData> CREATOR
    = new Parcelable.Creator<DigitalPenData>() {
        public DigitalPenData createFromParcel(Parcel in) {
            return new DigitalPenData(in);
        }

        public DigitalPenData[] newArray(int size) {
            return new DigitalPenData[size];
        }
    };

    private DigitalPenData(Parcel in) {
        mX                = in.readInt();
        mY                = in.readInt();
        mZ                = in.readInt();
        mXTilt            = in.readInt();
        mYTilt            = in.readInt();
        mZTilt            = in.readInt();
        mPressure         = in.readInt();
        mPenState         = (in.readInt() == 1);
        mSideButtonsState = new int[NUM_OF_SIDE_BUTTONS];
        in.readIntArray(mSideButtonsState);
        mRegion           = in.readInt();
    }
}
