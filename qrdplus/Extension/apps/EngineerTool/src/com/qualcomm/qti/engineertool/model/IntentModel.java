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

public class IntentModel implements Parcelable {
    private static final String TAG = "IntentModel";

    public static final int TYPE_ACTIVITY = 1;
    public static final int TYPE_SERVICE = 2;
    public static final int TYPE_BROADCAST = 3;

    private static final String ACTIVITY = "activity";
    private static final String SERVICE = "service";
    private static final String BROADCAST = "broadcast";

    private int mType;
    private String mPackageName;
    private String mClassName;
    private String mAction;

    private Parcelable[] mExtras;

    public static int getIntentModel(IntentModel model, XmlPullParser xml, int curEventType) {
        if (model == null
                || curEventType != XmlPullParser.START_TAG
                || !XmlUtils.TAG_INTENT.equals(xml.getName())) {
            return curEventType;
        }

        String type = xml.getAttributeValue(null, XmlUtils.ATTR_TYPE);
        if (ACTIVITY.equals(type)) {
            model.setType(TYPE_ACTIVITY);
        } else if (SERVICE.equals(type)) {
            model.setType(TYPE_SERVICE);
        } else if (BROADCAST.equals(type)) {
            model.setType(TYPE_BROADCAST);
        } else {
            // Set the default type as activity.
            model.setType(TYPE_ACTIVITY);
        }

        String packageName = xml.getAttributeValue(null, XmlUtils.ATTR_PACKAGE);
        if (!TextUtils.isEmpty(packageName)) {
            model.setPackageName(packageName);
        }

        String className = xml.getAttributeValue(null, XmlUtils.ATTR_CLASS);
        if (!TextUtils.isEmpty(className)) {
            model.setClassName(className);
        }

        String action = xml.getAttributeValue(null, XmlUtils.ATTR_ACTION);
        if (!TextUtils.isEmpty(action)) {
            model.setAction(action);
        }

        ArrayList<Extra> list = new ArrayList<Extra>();
        try {
            while (curEventType != XmlPullParser.END_DOCUMENT) {
                if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_EXTRA.equals(xml.getName())) {
                    String key = xml.getAttributeValue(null, XmlUtils.ATTR_KEY);
                    String value = xml.getAttributeValue(null, XmlUtils.ATTR_VALUE);
                    if (!TextUtils.isEmpty(key) && !TextUtils.isEmpty(value)) {
                        list.add(new Extra(key, value));
                    }
                }

                if (curEventType == XmlPullParser.END_TAG
                        && XmlUtils.TAG_INTENT.equals(xml.getName())) {
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
            Extra[] extras = new Extra[list.size()];
            list.toArray(extras);
            model.setExtras(extras);
        }
        return curEventType;
    }

    public static final ClassLoaderCreator<IntentModel> CREATOR =
            new ClassLoaderCreator<IntentModel>() {
        @Override
        public IntentModel createFromParcel(Parcel in) {
            return new IntentModel(in, null);
        }
        @Override
        public IntentModel createFromParcel(Parcel in, ClassLoader loader) {
            return new IntentModel(in, loader);
        }
        @Override
        public IntentModel[] newArray(int size) {
            return new IntentModel[size];
        }
    };

    public IntentModel() {
    }

    public IntentModel(Parcel in, ClassLoader loader) {
        this.mType = in.readInt();
        this.mPackageName = in.readString();
        this.mClassName = in.readString();
        this.mAction = in.readString();
        this.mExtras = in.readParcelableArray(loader);
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mType);
        dest.writeString(mPackageName);
        dest.writeString(mClassName);
        dest.writeString(mAction);
        dest.writeParcelableArray(mExtras, 0);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public void setType(int type) {
        switch (type) {
            case TYPE_ACTIVITY:
            case TYPE_SERVICE:
            case TYPE_BROADCAST:
                mType = type;
                return;
            default:
                mType = TYPE_ACTIVITY;
                return;
        }
    }

    public void setPackageName(String packageName) {
        mPackageName = packageName;
    }

    public void setClassName(String className) {
        mClassName = className;
    }

    public void setAction(String action) {
        mAction = action;
    }

    public void setExtras(Extra[] extras) {
        mExtras = extras;
    }

    public int getType() {
        return mType;
    }

    public String getPackageName() {
        return mPackageName;
    }

    public String getClassName() {
        return mClassName;
    }

    public String getAction() {
        return mAction;
    }

    public Extra[] getExtras() {
        if (mExtras == null) return null;

        Extra[] extras = new Extra[mExtras.length];
        for (int i = 0; i < mExtras.length; i++) {
            extras[i] = (Extra) mExtras[i];
        }
        return extras;
    }

    public static class Extra implements Parcelable {
        private String mKey;
        private String mValue;

        public static final Creator<Extra> CREATOR = new Creator<Extra>() {
            public Extra createFromParcel(Parcel in) {
                return new Extra(in);
            }

            public Extra[] newArray(int size) {
                return new Extra[size];
            }
        };

        public Extra(String key, String value) {
            this.mKey = key;
            this.mValue = value;
        }

        public Extra(Parcel in) {
            this.mKey = in.readString();
            this.mValue = in.readString();
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(mKey);
            dest.writeString(mValue);
        }

        @Override
        public int describeContents() {
            return 0;
        }

        public String getKey() {
            return mKey;
        }

        public String getValue() {
            return mValue;
        }
    }
}
