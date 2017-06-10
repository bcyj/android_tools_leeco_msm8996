/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool.model;

import android.os.Parcel;
import android.os.Parcelable;

public class OperationModel implements Parcelable {

    private String mFunction;
    private String mParams;
    private int mDelayMillis;

    public static final Creator<OperationModel> CREATOR = new Creator<OperationModel>() {
        @Override
        public OperationModel createFromParcel(Parcel in) {
            return new OperationModel(in);
        }
        @Override
        public OperationModel[] newArray(int size) {
            return new OperationModel[size];
        }
    };

    public OperationModel(String fun, String params) {
        this(fun, params, -1);
    }

    public OperationModel(String fun, String params, int delayMillis) {
        mFunction = fun;
        mParams = params;
        mDelayMillis = delayMillis;
    }

    public OperationModel(Parcel in) {
        this.mFunction = in.readString();
        this.mParams = in.readString();
        this.mDelayMillis = in.readInt();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(mFunction);
        dest.writeString(mParams);
        dest.writeInt(mDelayMillis);
    }

    public String getFunction() {
        return mFunction;
    }

    public String getParams() {
        return mParams;
    }

    public int getDelayMillis() {
        return mDelayMillis;
    }
}
