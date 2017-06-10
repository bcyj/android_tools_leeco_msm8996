/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool.model;

import android.content.Context;
import android.os.Parcel;
import android.os.Parcelable;

import org.xmlpull.v1.XmlPullParser;

public class TwoLineModel implements Parcelable {

    protected StringRes mLabel;
    protected StringRes mSummary;

    public static int getTwoLineModel(TwoLineModel model, XmlPullParser xml, int curEventType) {
        if (model == null || curEventType != XmlPullParser.START_TAG) {
            return curEventType;
        }

        StringRes label = XmlUtils.getAttributeAsStringRes(xml, XmlUtils.ATTR_LABEL);
        model.setLabel(label);

        StringRes summary = XmlUtils.getAttributeAsStringRes(xml, XmlUtils.ATTR_SUMMARY);
        model.setSummary(summary);

        return curEventType;
    }

    public static final ClassLoaderCreator<TwoLineModel> CREATOR =
            new ClassLoaderCreator<TwoLineModel>() {
        @Override
        public TwoLineModel createFromParcel(Parcel in) {
            return new TwoLineModel(in, null);
        }
        @Override
        public TwoLineModel createFromParcel(Parcel in, ClassLoader loader) {
            return new TwoLineModel(in, loader);
        }
        @Override
        public TwoLineModel[] newArray(int size) {
            return new TwoLineModel[size];
        }
    };

    public TwoLineModel() {
    }

    public TwoLineModel(Parcel in, ClassLoader loader) {
        this.mLabel = in.readParcelable(loader);
        this.mSummary = in.readParcelable(loader);
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeParcelable(mLabel, 0);
        dest.writeParcelable(mSummary, 0);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public void setLabel(StringRes label) {
        mLabel = label;
    }

    public void setSummary(StringRes summary) {
        mSummary = summary;
    }

    public String getLabel(Context context) {
        return mLabel == null ? null : mLabel.getString(context);
    }

    public String getSummary(Context context) {
        return mSummary == null ? null : mSummary.getString(context);
    }
}
