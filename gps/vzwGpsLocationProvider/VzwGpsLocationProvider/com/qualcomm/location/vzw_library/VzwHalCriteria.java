/******************************************************************************
  @file    VzwHalCriteria.java
  @brief   criteria for the location fix

  DESCRIPTION

  VZW extention of the standard Criteria

  ---------------------------------------------------------------------------
  Copyright (C) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.location.vzw_library;

import android.os.Parcel;
import android.os.Parcelable;
import android.location.Criteria;

public class VzwHalCriteria extends Criteria {

    private int mMode;
    private int mPreferredHorizontalAccuracy;
    private int mPreferredVerticalAccuracy;
    private int mMaximumResponseTime;
    private int mHintNextFixArriveInSec;
    private int mHintNextFixMode;
    private int mHintNextFixHorizontalAccuracy;
    private int mHybridMode;

    private void setToDefault () {
        mMode = IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_BASED;

        // default to 50 meters
        mPreferredHorizontalAccuracy = 50;
        mPreferredVerticalAccuracy = 50;

        // default to 30 sec
        mMaximumResponseTime = 30;

        // default to single shot, no hint for the next fix
        // note if this is not -1, we should have mode and accuracy info, too
        mHintNextFixArriveInSec = -1;
        mHintNextFixMode = mMode;
        mHintNextFixHorizontalAccuracy = mPreferredHorizontalAccuracy;

        // None
        mHybridMode = 0;
    }

    public VzwHalCriteria () {
        setToDefault ();
    }

    public VzwHalCriteria (VzwHalCriteria criteria) {
        mMode = criteria.mMode;
        mPreferredHorizontalAccuracy = criteria.mPreferredHorizontalAccuracy;
        mPreferredVerticalAccuracy = criteria.mPreferredVerticalAccuracy;
        mMaximumResponseTime = criteria.mMaximumResponseTime;
        mHintNextFixArriveInSec = criteria.mHintNextFixArriveInSec;
        mHintNextFixMode = criteria.mHintNextFixMode;
        mHintNextFixHorizontalAccuracy = criteria.mHintNextFixHorizontalAccuracy;
    }

    public VzwHalCriteria (Criteria criteria) {
        super(criteria);
        setToDefault ();
        switch(criteria.getAccuracy())
        {
        case ACCURACY_FINE:
                mPreferredHorizontalAccuracy = 50; // 50-meter is the normal idea of fine location
                break;
        case ACCURACY_COARSE:
                mPreferredHorizontalAccuracy = 2000; // arbitrarily set to 2000-meter
                break;
        default:
                // unknown, probably new, accuracy setting. do nothing. Leave it to default value
        }
    }

    public int getFixMode () {
        return mMode;
    }

    public void setFixMode (int Mode) {
        switch(Mode) {
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_BASED:
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_STANDALONE:
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_ASSISTED:
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_AFLT:
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_SPEED:
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_ACCURACY:
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_DATA:
            mMode = Mode;
            break;
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_CID:
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_ECID:
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_WIFI_MSA:
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_WIFI_MSB:
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_HYBRID:
            // the above mode are not currently supported but in the
            // vzw spec, while the requirement is not clear.
        default:
            throw new IllegalArgumentException("Mode not recognized");
        }
    }

    public int getHintNextFixArriveInSec () {
        return mHintNextFixArriveInSec;
    }

    public void setHintNextFixArriveInSec (int interval) {

        if(interval == -1)
        {
            // means: no hint
            mHintNextFixArriveInSec = -1;
        }
        else if((interval >= 0) && (interval <= 255)) {
            // next fix will come in 0-255 seconds
            mHintNextFixArriveInSec = interval;
        }
        else
        {
            throw new IllegalArgumentException("Time (sec) between fixes must be in -1, or [0..255]");
        }
    }

    // desired max horizontal error in meters
    public int getPreferredHorizontalAccuracy () {
        return mPreferredHorizontalAccuracy;
    }

    public void setPreferredHorizontalAccuracy (int ErrorMeter) {
        if (ErrorMeter <= 0 )
        {
            throw new IllegalArgumentException("Horizontal accuracy must be greater than 0");
        }
        else
        {
            mPreferredHorizontalAccuracy = ErrorMeter;
        }
    }

    // desired max vertical error in meters
    public int getPreferredVerticalAccuracy () {
        return mPreferredVerticalAccuracy;
    }

    public void setPreferredVerticalAccuracy (int ErrorMeter) {
        if (ErrorMeter <= 0 )
        {
            throw new IllegalArgumentException("Vertical accuracy must be greater than 0");
        }
        else
        {
            mPreferredVerticalAccuracy = ErrorMeter;
        }
    }

    // desired horizontal error in meters, for the next fix req
    public int getHintNextFixHorizontalAccuracy () {
        return mHintNextFixHorizontalAccuracy;
    }

    public void setHintNextFixHorizontalAccuracy (int ErrorMeter) {
        if (ErrorMeter <= 0 )
        {
            throw new IllegalArgumentException("Hint for horizontal accuracy must be greater than 0");
        }
        else
        {
            mHintNextFixHorizontalAccuracy = ErrorMeter;
        }
    }

    // desired max vertical error in meters
    public int getHintNextFixMode () {
        return mHintNextFixMode;
    }
    // set mode for the next fix
    public void setHintNextFixMode (int mode) {
        switch(mode) {
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_BASED:
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_STANDALONE:
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_ASSISTED:
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_AFLT:
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_SPEED:
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_ACCURACY:
        case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_DATA:
            mHintNextFixMode = mode;
            break;
        default:
            throw new IllegalArgumentException("Mode not recognized");
        }
    }

    public int getMaximumResponseTime () {
        return mMaximumResponseTime;
    }

    public void setMaximumResponseTime (int TimeSec) {
        if((TimeSec >= 0) && (TimeSec <= 255)) {
            // location engine should give up in 0-255 seconds
            mMaximumResponseTime = TimeSec;
        }
        else
        {
            throw new IllegalArgumentException("Timeout (sec) must be in [0..255]");
        }
    }

    public static final Parcelable.Creator<VzwHalCriteria> CREATOR =
        new Parcelable.Creator<VzwHalCriteria>() {
        public VzwHalCriteria createFromParcel(Parcel in) {
            Criteria super_c = (Criteria)in.readParcelable(null);
            VzwHalCriteria c = new VzwHalCriteria(super_c);
            c.mMode = in.readInt();
            c.mPreferredHorizontalAccuracy = in.readInt();
            c.mPreferredVerticalAccuracy = in.readInt();
            c.mMaximumResponseTime = in.readInt();
            c.mHintNextFixArriveInSec = in.readInt();
            c.mHintNextFixMode = in.readInt();
            c.mHintNextFixHorizontalAccuracy = in.readInt();
            c.mHybridMode = in.readInt();
            return c;
        }

        public VzwHalCriteria[] newArray(int size) {
            return new VzwHalCriteria[size];
        }
    };

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel parcel, int flags) {
        parcel.writeParcelable((Criteria)this, flags);
        parcel.writeInt(mMode);
        parcel.writeInt(mPreferredHorizontalAccuracy);
        parcel.writeInt(mPreferredVerticalAccuracy);
        parcel.writeInt(mMaximumResponseTime);
        parcel.writeInt(mHintNextFixArriveInSec);
        parcel.writeInt(mHintNextFixMode);
        parcel.writeInt(mHintNextFixHorizontalAccuracy);
        parcel.writeInt(mHybridMode);
    }

    public void setHybridMode (int hybridMode) {
        mHybridMode = hybridMode;
    }

    public int getHybridMode () {
        return mHybridMode;
    }
}
