/* ==============================================================================
 * WfdStatus.java
 *
 * Copyright (c) 2012 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 ============================================================================== */

package com.qualcomm.wfd;

import android.os.Parcel;
import android.os.Parcelable;

public class WfdStatus implements Parcelable {

    public int state;

    public int sessionId;

    public WfdDevice connectedDevice;

    public WfdStatus() {
        state = WfdEnums.SessionState.INVALID.ordinal();
        sessionId = -1;
        connectedDevice = null;
    }

    public static final Creator<WfdStatus> CREATOR = new Creator<WfdStatus>() {

        @Override
        public WfdStatus createFromParcel(Parcel source) {
            WfdStatus ret = new WfdStatus();
            ret.state = source.readInt();
            ret.sessionId = source.readInt();
            ret.connectedDevice = (WfdDevice) source.readValue(WfdDevice.class.getClassLoader());
            return ret;
        }

        @Override
        public WfdStatus[] newArray(int size) {
            return new WfdStatus[size];
        }
    };

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(state);
        dest.writeInt(sessionId);
        dest.writeValue(connectedDevice);
    }

}
