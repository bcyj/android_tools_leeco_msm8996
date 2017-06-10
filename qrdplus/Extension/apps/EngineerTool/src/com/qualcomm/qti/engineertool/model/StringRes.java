/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool.model;

import android.content.Context;
import android.os.Parcel;
import android.os.Parcelable;

public class StringRes implements Parcelable {

    private int mResId;
    private String mString;

    public static final Creator<StringRes> CREATOR = new Creator<StringRes>() {
        @Override
        public StringRes createFromParcel(Parcel in) {
            return new StringRes(in);
        }
        @Override
        public StringRes[] newArray(int size) {
            return new StringRes[size];
        }
    };

    public StringRes(int resId, String str) {
        mResId = resId;
        mString = str;
    }

    public StringRes(Parcel in) {
        this.mResId = in.readInt();
        this.mString = in.readString();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mResId);
        dest.writeString(mString);
    }

    public String getString(Context context) {
        if (context != null && mResId > 0) {
            return context.getResources().getString(mResId);
        }
        return mString;
    }
}
