/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

package com.qualcomm.location.pvd;

import android.os.Parcel;
import android.os.Parcelable;

public enum PVDStatus implements Parcelable {
    PVD_STATUS_UNKNOWN,
    PVD_STATUS_DETECTED,
    PVD_STATUS_UNDETECTED,
    PVD_STATUS_BUSY,
    PVD_STATUS_GENERAL_FAILURE,
    // After providing following statuses to listeners, PVDService will drop the listeners
    // Listeners should re-register if they want to use PVDService.
    PVD_STATUS_WIFI_DISABLED,
    PVD_STATUS_NO_EULA;

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(ordinal());
    }

    public static final Creator <PVDStatus> CREATOR
        = new Creator <PVDStatus> () {

        @Override
        public PVDStatus createFromParcel(Parcel source) {
            return PVDStatus.values()[source.readInt()];
        }

        @Override
        public PVDStatus[] newArray(int size) {
            return new PVDStatus[size];
        }

    };

}
