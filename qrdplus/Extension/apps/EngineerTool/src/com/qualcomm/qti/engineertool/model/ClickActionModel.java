/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool.model;

import android.content.Context;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;

public class ClickActionModel implements Parcelable {
    private static final String TAG = "ClickActionModel";

    private Parcelable mClickAction;

    public static int getClickActionModel(Context context, ClickActionModel model,
            XmlPullParser xml, int curEventType) {
        if (model == null || curEventType != XmlPullParser.START_TAG) {
            return curEventType;
        }

        String curName = xml.getName();

        Parcelable clickAction = null;
        try {
            while (curEventType != XmlPullParser.END_DOCUMENT) {
                if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_LIST.equals(xml.getName())) {
                    clickAction = new ListModel();
                    curEventType = ListModel.getListModel(context, (ListModel) clickAction, xml,
                            curEventType);
                } else if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_DIALOG.equals(xml.getName())) {
                    clickAction = new DialogModel();
                    curEventType = DialogModel.getDialogModel(context, (DialogModel) clickAction,
                            xml, curEventType);
                } else if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_INTENT.equals(xml.getName())) {
                    clickAction = new IntentModel();
                    curEventType = IntentModel.getIntentModel(
                            (IntentModel) clickAction, xml, curEventType);
                } else if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_DO.equals(xml.getName())) {
                    clickAction = new DoOpsModel();
                    curEventType = DoOpsModel.getDoOpsModel(
                            (DoOpsModel) clickAction, xml, curEventType);
                }

                if (curEventType == XmlPullParser.END_TAG && curName.equals(xml.getName())) {
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

        model.setClickAction(clickAction);
        return curEventType;
    }

    public static final ClassLoaderCreator<ClickActionModel> CREATOR =
            new ClassLoaderCreator<ClickActionModel>() {
        @Override
        public ClickActionModel createFromParcel(Parcel in) {
            return new ClickActionModel(in, null);
        }
        @Override
        public ClickActionModel createFromParcel(Parcel in, ClassLoader loader) {
            return new ClickActionModel(in, loader);
        }
        @Override
        public ClickActionModel[] newArray(int size) {
            return new ClickActionModel[size];
        }
    };

    public ClickActionModel() {
    }

    public ClickActionModel(Parcel in, ClassLoader loader) {
        this.mClickAction = in.readParcelable(loader);
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeParcelable(mClickAction, 0);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public void setClickAction(Parcelable clickAction) {
        mClickAction = clickAction;
    }

    public Parcelable getClickAction() {
        return mClickAction;
    }
}
