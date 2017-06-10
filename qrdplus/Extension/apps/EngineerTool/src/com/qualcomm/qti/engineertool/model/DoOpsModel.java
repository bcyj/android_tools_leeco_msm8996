/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool.model;

import android.os.Parcel;
import android.os.Parcelable;
import android.text.TextUtils;
import android.util.Log;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.util.ArrayList;

public class DoOpsModel implements Parcelable {
    private static final String TAG = "DoOpsModel";

    private OperationModel[] mOperations;

    public static int getDoOpsModel(DoOpsModel model, XmlPullParser xml, int curEventType) {
        if (model == null
                || curEventType != XmlPullParser.START_TAG
                || !XmlUtils.TAG_DO.equals(xml.getName())) {
            return curEventType;
        }

        ArrayList<OperationModel> list = new ArrayList<OperationModel>();
        try {
            while (curEventType != XmlPullParser.END_DOCUMENT) {
                if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_OP.equals(xml.getName())) {
                    String function = xml.getAttributeValue(null, XmlUtils.ATTR_FUNCTION);
                    if (!TextUtils.isEmpty(function)) {
                        String params = xml.getAttributeValue(null, XmlUtils.ATTR_FUNCTION);
                        list.add(new OperationModel(function, params));
                    }
                }

                if (curEventType == XmlPullParser.END_TAG
                        && XmlUtils.TAG_DO.equals(xml.getName())) {
                    // Parse finished.
                    break;
                }

                curEventType = xml.next();
            }
        } catch (XmlPullParserException e) {
            Log.w(TAG, "Parse xml failed, XmlPullParserException: " + e.getMessage());
        } catch (IOException e) {
            Log.w(TAG, "Parse xml failed, IOException: " + e.getMessage());
        }

        if (list.size() > 0) {
            OperationModel[] ops = new OperationModel[list.size()];
            list.toArray(ops);
            model.setOperations(ops);
        }
        return curEventType;
    }

    public static final ClassLoaderCreator<DoOpsModel> CREATOR =
            new ClassLoaderCreator<DoOpsModel>() {
        @Override
        public DoOpsModel createFromParcel(Parcel in) {
            return new DoOpsModel(in, null);
        }
        @Override
        public DoOpsModel createFromParcel(Parcel in, ClassLoader loader) {
            return new DoOpsModel(in, loader);
        }
        @Override
        public DoOpsModel[] newArray(int size) {
            return new DoOpsModel[size];
        }
    };

    public DoOpsModel() {}

    public DoOpsModel(Parcel in, ClassLoader loader) {
        this.mOperations = (OperationModel[]) in.readParcelableArray(loader);
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeParcelableArray(mOperations, 0);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public void setOperations(OperationModel[] ops) {
        mOperations = ops;
    }

    public OperationModel[] getOperations() {
        return mOperations;
    }
}
