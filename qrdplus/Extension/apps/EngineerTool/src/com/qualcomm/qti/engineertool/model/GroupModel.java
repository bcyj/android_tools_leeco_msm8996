/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool.model;

import android.content.Context;
import android.content.res.XmlResourceParser;
import android.os.Parcel;
import android.os.Parcelable;
import android.text.TextUtils;
import android.util.Log;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.util.ArrayList;

public class GroupModel extends TwoLineModel {
    private static final String TAG = "GroupModel";

    private String mCarrier;
    private Parcelable[] mChildList;

    public static int getGroupModel(Context context, GroupModel model, XmlPullParser xml,
            int curEventType) {
        if (model == null
                || curEventType != XmlPullParser.START_TAG
                || !XmlUtils.TAG_GROUP.equals(xml.getName())) {
            return curEventType;
        }

        String carrier = xml.getAttributeValue(null, XmlUtils.ATTR_CARRIER);
        if (!TextUtils.isEmpty(carrier)) {
            model.setCarrier(carrier);
        }

        curEventType = TwoLineModel.getTwoLineModel(model, xml, curEventType);

        ArrayList<ListItemModel> list = new ArrayList<ListItemModel>();
        curEventType = parserGroup(context, model, xml, XmlUtils.TAG_GROUP, curEventType, list);

        if (list.size() > 0) {
            ListItemModel[] listItemModels = new ListItemModel[list.size()];
            list.toArray(listItemModels);
            model.setChildList(listItemModels);
        }
        return curEventType;
    }

    private static int parserGroup(Context context, GroupModel model, XmlPullParser xml,
            String endTag, int curEventType, ArrayList<ListItemModel> list) {
        try {
            while (curEventType != XmlPullParser.END_DOCUMENT) {
                if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_INCLUDE.equals(xml.getName())) {
                    int newXmlResId = XmlUtils.getAttributeResourceValue(xml, XmlUtils.ATTR_XML);
                    if (newXmlResId > 0) {
                        XmlResourceParser newXml = context.getResources().getXml(newXmlResId);
                        curEventType = parserGroup(context, model, newXml, XmlUtils.TAG_INCLUDE,
                                curEventType, list);
                    }
                } else if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_CHILD.equals(xml.getName())) {
                    ListItemModel child = new ListItemModel();
                    curEventType = ListItemModel.getListItemModel(context, child, xml,
                            curEventType);
                    list.add(child);
                }

                if (curEventType == XmlPullParser.END_TAG && endTag.equals(xml.getName())) {
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

    public static final ClassLoaderCreator<GroupModel> CREATOR =
            new ClassLoaderCreator<GroupModel>() {
        @Override
        public GroupModel createFromParcel(Parcel in) {
            return new GroupModel(in, null);
        }
        @Override
        public GroupModel createFromParcel(Parcel in, ClassLoader loader) {
            return new GroupModel(in, loader);
        }
        @Override
        public GroupModel[] newArray(int size) {
            return new GroupModel[size];
        }
    };

    public GroupModel() {
        super();
    }

    public GroupModel(Parcel in, ClassLoader loader) {
        super(in, loader);
        mCarrier = in.readString();
        mChildList = in.readParcelableArray(loader);
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        super.writeToParcel(dest, flags);
        dest.writeString(mCarrier);
        dest.writeParcelableArray(mChildList, 0);
    }

    public void setCarrier(String carrier) {
        mCarrier = carrier;
    }

    public void setChildList(ListItemModel[] childList) {
        mChildList = childList;
    }

    public String getCarrier() {
        return mCarrier;
    }

    public int getChildrenCount() {
        return mChildList == null ? 0 : mChildList.length;
    }

    public ListItemModel getChild(int childPosition) {
        return mChildList == null ? null : (ListItemModel) mChildList[childPosition];
    }
}
