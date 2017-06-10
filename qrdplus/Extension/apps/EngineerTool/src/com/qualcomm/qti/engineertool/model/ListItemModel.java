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

public class ListItemModel extends TwoLineModel {
    private static final String TAG = "ListItemModel";

    protected ClickActionModel mClickAction;

    public static int getListItemModel(Context context, ListItemModel model, XmlPullParser xml,
            int curEventType) {
        if (model == null
                || curEventType != XmlPullParser.START_TAG) {
            return curEventType;
        }

        // Get the current xml tag's name.
        String curName = xml.getName();

        curEventType = TwoLineModel.getTwoLineModel(model, xml, curEventType);

        ClickActionModel clickAction = null;
        try {
            while (curEventType != XmlPullParser.END_DOCUMENT) {
                if (curEventType == XmlPullParser.START_TAG) {
                    clickAction = new ClickActionModel();
                    curEventType = ClickActionModel.getClickActionModel(context,
                            clickAction, xml, curEventType);
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

        model.setActionModel(clickAction);
        return curEventType;
    }

    public static final ClassLoaderCreator<ListItemModel> CREATOR =
            new ClassLoaderCreator<ListItemModel>() {
        @Override
        public ListItemModel createFromParcel(Parcel in) {
            return new ListItemModel(in, null);
        }
        @Override
        public ListItemModel createFromParcel(Parcel in, ClassLoader loader) {
            return new ListItemModel(in, loader);
        }
        @Override
        public ListItemModel[] newArray(int size) {
            return new ListItemModel[size];
        }
    };

    public ListItemModel() {
        super();
    }

    public ListItemModel(Parcel in, ClassLoader loader) {
        super(in, loader);
        this.mClickAction = in.readParcelable(loader);
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        super.writeToParcel(dest, flags);
        dest.writeParcelable(mClickAction, 0);
    }

    public void setActionModel(ClickActionModel action) {
        mClickAction = action;
    }

    public Parcelable getClickAction() {
        return mClickAction == null ? null : mClickAction.getClickAction();
    }
}
