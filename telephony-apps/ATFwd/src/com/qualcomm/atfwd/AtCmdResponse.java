/*******************************************************************************
    Copyright (c) 2010,2011 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

package com.qualcomm.atfwd;

import android.os.Parcel;
import android.os.Parcelable;

public class AtCmdResponse implements Parcelable {

    public static final int RESULT_ERROR = 0;
    public static final int RESULT_OK = 1;
    public static final int RESULT_OTHER = 2;

    private int mResult;
    private String mResponse;

    public int getResult() {
        return mResult;
    }

    public void setResult(int mResult) {
        this.mResult = mResult;
    }

    public String getResponse() {
        return mResponse;
    }

    public void setResponse(String mResponse) {
        this.mResponse = mResponse;
    }

    AtCmdResponse(int result, String response) {
        init(result, response);
    }

    AtCmdResponse(Parcel p, int flags) {
        int result = p.readInt();
        String response = p.readString();
        init(result, response);
    }

    private void init(int result, String response) {
        mResult = result;
        mResponse = response;
    }
    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mResult);
        dest.writeString(mResponse);
    }

    public static Parcelable.Creator<AtCmdResponse> CREATOR =
        new Parcelable.Creator<AtCmdResponse>() {

        public AtCmdResponse createFromParcel(Parcel source) {
            int result;
            String response;

            result = source.readInt();
            response = source.readString();
            return new AtCmdResponse(result, response);
        }

        public AtCmdResponse[] newArray(int size) {
            return new AtCmdResponse[size];
        }

    };

}
