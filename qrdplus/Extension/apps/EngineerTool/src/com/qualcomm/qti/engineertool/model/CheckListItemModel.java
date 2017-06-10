/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool.model;

import android.content.Context;
import android.os.Parcel;
import android.text.TextUtils;
import android.util.Log;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.util.HashMap;

public class CheckListItemModel extends ListItemModel {
    private static final String TAG = "CheckListItemModel";

    public static final int EXPECT_TYPE_EQUAL = 1;
    public static final int EXPECT_TYPE_LESS = 2;
    public static final int EXPECT_TYPE_MORE = 4;

    private static final String SEP = "*";

    private OperationModel mOperation;
    private String mCurResult;
    private String mExpectResult;
    private int mExpectType;

    private HashMap<String, String> mSummary = new HashMap<String, String>();;

    private boolean mNeedUpdateStatus = false;

    public static int getCheckListItemModel(Context context, CheckListItemModel model,
            XmlPullParser xml, int curEventType) {
        if (model == null
                || curEventType != XmlPullParser.START_TAG
                || !XmlUtils.TAG_CHECKITEM.equals(xml.getName())) {
            return curEventType;
        }

        String function = xml.getAttributeValue(null, XmlUtils.ATTR_FUNCTION);
        if (!TextUtils.isEmpty(function)) {
            String params = xml.getAttributeValue(null, XmlUtils.ATTR_PARAMS);
            model.setOperation(new OperationModel(function, params));
        }

        String expectResult = xml.getAttributeValue(null, XmlUtils.ATTR_EXPECT);
        if (!TextUtils.isEmpty(expectResult)) {
            model.setExpectResult(expectResult);
        }

        int definedExpType = XmlUtils.getAttributeValueAsInt(xml, XmlUtils.ATTR_EXPECT_TYPE);
        int expectType = EXPECT_TYPE_EQUAL;
        if (definedExpType > 0 && (definedExpType & EXPECT_TYPE_LESS) != 0) {
            expectType = expectType | EXPECT_TYPE_LESS;
        }
        if (definedExpType > 0 && (definedExpType & EXPECT_TYPE_MORE) != 0) {
            expectType = expectType | EXPECT_TYPE_MORE;
        }
        model.setExpectType(expectType);

        int valuesArrayResId =
                XmlUtils.getAttributeResourceValue(xml, XmlUtils.ATTR_SUMMARY_VALUES);
        int labelsArrayResId =
                XmlUtils.getAttributeResourceValue(xml, XmlUtils.ATTR_SUMMARY_LABELS);

        String[] values = null;
        if (valuesArrayResId > 0) {
            values = context.getResources().getStringArray(valuesArrayResId);
        } else {
            String valuesString = xml.getAttributeValue(null, XmlUtils.ATTR_SUMMARY_VALUES);
            if (!TextUtils.isEmpty(valuesString)) {
                values = valuesString.split(SEP);
            }
        }
        String[] labels = null;
        if (labelsArrayResId > 0) {
            labels = context.getResources().getStringArray(labelsArrayResId);
        } else {
            String labelsString = xml.getAttributeValue(null, XmlUtils.ATTR_SUMMARY_LABELS);
            if (!TextUtils.isEmpty(labelsString)) {
                labels = labelsString.split(SEP);
            }
        }

        model.setSummary(values, labels);

        curEventType = ListItemModel.getListItemModel(context, model, xml, curEventType);
        try {
            while (curEventType != XmlPullParser.END_DOCUMENT) {
                if (curEventType == XmlPullParser.END_TAG
                        && XmlUtils.TAG_CHECKITEM.equals(xml.getName())) {
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
        return curEventType;
    }

    public static final ClassLoaderCreator<CheckListItemModel> CREATOR =
            new ClassLoaderCreator<CheckListItemModel>() {
        @Override
        public CheckListItemModel createFromParcel(Parcel in) {
            return new CheckListItemModel(in, null);
        }
        @Override
        public CheckListItemModel createFromParcel(Parcel in, ClassLoader loader) {
            return new CheckListItemModel(in, loader);
        }
        @Override
        public CheckListItemModel[] newArray(int size) {
            return new CheckListItemModel[size];
        }
    };

    public CheckListItemModel() {
        super();
    }

    public CheckListItemModel(Parcel in, ClassLoader loader) {
        super(in, loader);
        this.mOperation = in.readParcelable(loader);
        this.mCurResult = in.readString();
        this.mExpectResult = in.readString();
        this.mExpectType = in.readInt();
        in.readMap(this.mSummary, null);
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        super.writeToParcel(dest, flags);
        dest.writeParcelable(mOperation, 0);
        dest.writeString(mCurResult);
        dest.writeString(mExpectResult);
        dest.writeInt(mExpectType);
        dest.writeMap(mSummary);
    }

    public void setOperation(OperationModel operationModel) {
        mOperation = operationModel;
    }

    public void setCurrentResult(String curResult) {
        mCurResult = curResult;
    }

    public void setExpectResult(String res) {
        mExpectResult = res;
    }

    public void setExpectType(int type) {
        mExpectType = type;
    }

    public void setSummary(String[] values, String[] labels) {
        if (values == null || labels == null || values.length != labels.length) {
            return;
        }

        mSummary.clear();
        for (int i = 0; i < values.length; i++) {
            mSummary.put(values[i], labels[i]);
        }
    }

    public void setUpdateStatus(boolean need) {
        mNeedUpdateStatus = need;
    }

    public String getFunction() {
        return mOperation == null ? null : mOperation.getFunction();
    }

    public String getParams() {
        return mOperation == null ? null : mOperation.getParams();
    }

    public String getCurResult() {
        return mCurResult;
    }

    public String getExpectResult() {
        return mExpectResult;
    }

    public int getExpextType() {
        return mExpectType;
    }

    public String getSummaryLabel(String value) {
        return mSummary.get(value);
    }

    public boolean needUpdate() {
        return mNeedUpdateStatus;
    }
}
