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

public class ListModel implements Parcelable {
    private static final String TAG = "ListModel";

    public static final int MENU_NONE = 0;
    public static final int MENU_LOAD = 1;
    public static final int MENU_APPLY = 2;
    public static final int MENU_EXPORT = 4;

    public static final int TYPE_GROUP_LIST = 1;
    public static final int TYPE_CHECK_LIST = 2;

    private static final String MENU_FLAG_LOAD = "load";
    private static final String MENU_FLAG_APPLY = "apply";
    private static final String MENU_FLAG_EXPORT = "export";

    private int mMenuType;
    private StringRes mTitle;

    private int mListType;
    private Parcelable[] mList;

    public static int getListModel(Context context, ListModel model, XmlPullParser xml,
            int curEventType) {
        if (model == null
                || curEventType != XmlPullParser.START_TAG
                || !XmlUtils.TAG_LIST.equals(xml.getName())) {
            return curEventType;
        }

        String menu = xml.getAttributeValue(null, XmlUtils.ATTR_MENU);
        if (TextUtils.isEmpty(menu)) {
            model.setMenuType(MENU_NONE);
        } else {
            int type = 0;
            if (menu.contains(MENU_FLAG_LOAD)) {
                type = type + MENU_LOAD;
            }
            if (menu.contains(MENU_FLAG_APPLY)) {
                type = type + MENU_APPLY;
            }
            if (menu.contains(MENU_FLAG_EXPORT)) {
                type = type + MENU_EXPORT;
            }
            model.setMenuType(type);
        }

        StringRes title = XmlUtils.getAttributeAsStringRes(xml, XmlUtils.ATTR_TITLE);
        model.setTitle(title);

        // Parse the list content, get mListType and mList.
        ArrayList<Parcelable> list = new ArrayList<Parcelable>();
        curEventType = parserList(context, model, xml, XmlUtils.TAG_LIST, curEventType, list);

        if (list.size() > 0) {
            Parcelable[] parcelableList = new Parcelable[list.size()];
            list.toArray(parcelableList);
            model.setList(parcelableList);
        }
        return curEventType;
    }

    private static int parserList(Context context, ListModel model, XmlPullParser xml,
            String endTag, int curEventType, ArrayList<Parcelable> list) {
        try {
            while (curEventType != XmlPullParser.END_DOCUMENT) {
                if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_INCLUDE.equals(xml.getName())) {
                    int newXmlResId = XmlUtils.getAttributeResourceValue(xml, XmlUtils.ATTR_XML);
                    if (newXmlResId > 0) {
                        XmlResourceParser newXml = context.getResources().getXml(newXmlResId);
                        curEventType = parserList(context, model, newXml, XmlUtils.TAG_INCLUDE,
                                curEventType, list);
                    }
                } else if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_GROUP.equals(xml.getName())) {
                    model.setListType(TYPE_GROUP_LIST);
                    GroupModel group = new GroupModel();
                    curEventType = GroupModel.getGroupModel(context, group, xml, curEventType);
                    list.add(group);
                } else if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_CHECKITEM.equals(xml.getName())) {
                    model.setListType(TYPE_CHECK_LIST);
                    CheckListItemModel checkListItem = new CheckListItemModel();
                    curEventType = CheckListItemModel.getCheckListItemModel(context,
                            checkListItem, xml, curEventType);
                    list.add(checkListItem);
                } else if (curEventType == XmlPullParser.START_TAG) {
                    Log.w(TAG, "Do not support this content(" + xml.getName() + ") now.");
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

    public static final ClassLoaderCreator<ListModel> CREATOR =
            new ClassLoaderCreator<ListModel>() {
        @Override
        public ListModel createFromParcel(Parcel in) {
            return new ListModel(in, null);
        }
        @Override
        public ListModel createFromParcel(Parcel in, ClassLoader loader) {
            return new ListModel(in, loader);
        }
        @Override
        public ListModel[] newArray(int size) {
            return new ListModel[size];
        }
    };

    public ListModel() { }

    public ListModel(Parcel in, ClassLoader loader) {
        this.mMenuType = in.readInt();
        this.mTitle = in.readParcelable(loader);
        this.mListType = in.readInt();
        this.mList = in.readParcelableArray(loader);
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mMenuType);
        dest.writeParcelable(mTitle, 0);
        dest.writeInt(mListType);
        dest.writeParcelableArray(mList, 0);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public void setMenuType(int type) {
        mMenuType = type;
    }

    public void setTitle(StringRes title) {
        mTitle = title;
    }

    public void setListType(int type) {
        if (type != TYPE_GROUP_LIST && type != TYPE_CHECK_LIST) {
            throw new IllegalArgumentException("Do not support type.");
        }
        mListType = type;
    }

    public void setList(Parcelable[] list) {
        mList = list;
    }

    public int getMenuType() {
        return mMenuType;
    }

    public String getTitle(Context context) {
        return mTitle == null ? null : mTitle.getString(context);
    }

    public int getListViewType() {
        return mListType;
    }

    public Parcelable[] getList() {
        if (mList == null) return null;

        if (mListType == TYPE_GROUP_LIST) {
            GroupModel[] groupList = new GroupModel[mList.length];
            for (int i = 0; i < mList.length; i++) {
                groupList[i] = (GroupModel) mList[i];
            }
            return groupList;
        } else if (mListType == TYPE_CHECK_LIST) {
            CheckListItemModel[] checkListItemList = new CheckListItemModel[mList.length];
            for (int i = 0; i < mList.length; i++) {
                checkListItemList[i] = (CheckListItemModel) mList[i];
            }
            return checkListItemList;
        }

        return null;
    }
}
