/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.universaldownload;

import android.os.Parcel;
import android.os.Parcelable;

public class UpdateInfo implements Parcelable {
    private String mCapability;
    private String mVersion;
    private String mDataAddr;
    private String md5;
    UpdateInfo(){
    }

    public void setCapibility(String capibility) {
        this.mCapability = capibility;
    }

    public void setVersion(String version) {
        this.mVersion = version;
    }

    public void setAddr(String addr) {
        this.mDataAddr = addr;
    }

    public void setMd5(String md5) {
        this.md5 = md5;
    }

    public String getCapability() {
        return mCapability;
    }

    public String getVersion() {
        return mVersion;
    }

    public String getAddr() {
        return mDataAddr;
    }

    public String getMd5() {
        return md5;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel parcel, int flags) {
        parcel.writeString(mCapability);
        parcel.writeString(mVersion);
        parcel.writeString(mDataAddr);
        parcel.writeString(md5);
    }

    public static final Creator<UpdateInfo> CREATOR = new Creator<UpdateInfo>() {
        @Override
        public UpdateInfo createFromParcel(Parcel parcel) {
            UpdateInfo updateInfo = new UpdateInfo();
            updateInfo.setCapibility(parcel.readString());
            updateInfo.setVersion(parcel.readString());
            updateInfo.setAddr(parcel.readString());
            updateInfo.setMd5(parcel.readString());
            return updateInfo;
        }

        @Override
        public UpdateInfo[] newArray(int size) {
            return new UpdateInfo[size];
        }
    };
}
