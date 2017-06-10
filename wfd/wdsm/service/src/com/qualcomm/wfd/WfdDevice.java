/* ==============================================================================
 * WfdDevice.java
 *
 * Data structure for WFD capable device
 *
 * Copyright (c) 2012 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 ============================================================================== */

package com.qualcomm.wfd;

import com.qualcomm.wfd.WfdEnums.WFDDeviceType;

import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;

public class WfdDevice implements Parcelable {

    public int deviceType;

    public String macAddress;

    public String deviceName;

    public String ipAddress;

    public int rtspPort;

    public int decoderLatency;

    public boolean isAvailableForSession;

    public int preferredConnectivity;

    public String addressOfAP;

    public int coupledSinkStatus;

    public Bundle capabilities; // Map of String => Parcelable

    public static final Parcelable.Creator<WfdDevice> CREATOR = new Parcelable.Creator<WfdDevice>() {
        public WfdDevice createFromParcel(Parcel in) {
            return new WfdDevice(in);
        }

        public WfdDevice[] newArray(int size) {
            return new WfdDevice[size];
        }
    };

    public WfdDevice() {
        deviceType = WFDDeviceType.UNKNOWN.getCode();
        rtspPort = -1;
        capabilities = new Bundle();
    }

    public WfdDevice(Parcel in) {
        readFromParcel(in);
    }

    private void readFromParcel(Parcel in) {
        deviceType = in.readInt();
        macAddress = in.readString();
        deviceName = in.readString();
        ipAddress = in.readString();
        rtspPort = in.readInt();
        decoderLatency = in.readInt();
        isAvailableForSession = in.readByte() != 0;
        preferredConnectivity = in.readInt();
        addressOfAP = in.readString();
        coupledSinkStatus = in.readInt();
        capabilities = in.readBundle();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(deviceType);
        dest.writeString(macAddress);
        dest.writeString(deviceName);
        dest.writeString(ipAddress);
        dest.writeInt(rtspPort);
        dest.writeInt(decoderLatency);
        dest.writeByte((byte) (isAvailableForSession ? 1 : 0));
        dest.writeInt(preferredConnectivity);
        dest.writeString(addressOfAP);
        dest.writeInt(coupledSinkStatus);
        dest.writeBundle(capabilities);
    }

}
