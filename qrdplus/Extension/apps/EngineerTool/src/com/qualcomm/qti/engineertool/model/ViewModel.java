/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool.model;

import android.content.Context;
import android.os.Parcel;
import android.os.Parcelable;
import android.text.TextUtils;
import android.util.Log;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.util.ArrayList;

public class ViewModel implements Parcelable {
    private static final String TAG = "ViewModel";

    public static final int TYPE_EDIT          = 1;
    public static final int TYPE_SWITCH        = 2;
    public static final int TYPE_SPINNER       = 3;

    public static final int INFO_GRAVITY_TOP = 1;
    public static final int INFO_GRAVITY_BOTTOM = 2;

    private static final String GRAVITY_TOP = "top";
    private static final String GRAVITY_BOTTOM = "bottom";

    private int mType;
    private StringRes mInfo;
    private int mInfoGravity;

    private EditContent mContent;

    public static int getViewModel(ViewModel model, XmlPullParser xml, int curEventType) {
        if (model == null
                || curEventType != XmlPullParser.START_TAG
                || !XmlUtils.TAG_VIEW.equals(xml.getName())) {
            return curEventType;
        }

        StringRes info = XmlUtils.getAttributeAsStringRes(xml, XmlUtils.ATTR_INFO);
        model.setInfo(info);

        String gravity = xml.getAttributeValue(null, XmlUtils.ATTR_INFO_GRAVITY);
        if (GRAVITY_BOTTOM.equals(gravity)) {
            model.setInfoGravity(INFO_GRAVITY_BOTTOM);
        } else {
            if (!GRAVITY_TOP.equals(gravity)) {
                Log.w(TAG, "Do not support this gravity: " + gravity + ", set as top.");
            }
            model.setInfoGravity(INFO_GRAVITY_TOP);
        }

        int viewType = 0;
        EditContent content = null;
        try {
            while (curEventType != XmlPullParser.END_DOCUMENT) {
                if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_EDIT.equals(xml.getName())) {
                    viewType = TYPE_EDIT;
                    content = new EditModel();
                    curEventType = EditModel.getEditModel(
                            (EditModel) content, xml, curEventType);
                } else if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_SWITCH.equals(xml.getName())) {
                    viewType = TYPE_SWITCH;
                    content = new SwitchModel();
                    curEventType = SwitchModel.getSwitchModel(
                            (SwitchModel) content, xml, curEventType);
                } else if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_SPINNER.equals(xml.getName())) {
                    viewType = TYPE_SPINNER;
                    content = new SpinnerModel();
                    curEventType = SpinnerModel.getSpinnerModel(
                            (SpinnerModel) content, xml, curEventType);
                }

                if (curEventType == XmlPullParser.END_TAG
                        && XmlUtils.TAG_VIEW.equals(xml.getName())) {
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

        model.setType(viewType);
        model.setContent(content);
        return curEventType;
    }

    public static final ClassLoaderCreator<ViewModel> CREATOR =
            new ClassLoaderCreator<ViewModel>() {
        @Override
        public ViewModel createFromParcel(Parcel in) {
            return new ViewModel(in, null);
        }
        @Override
        public ViewModel createFromParcel(Parcel in, ClassLoader loader) {
            return new ViewModel(in, loader);
        }
        @Override
        public ViewModel[] newArray(int size) {
            return new ViewModel[size];
        }
    };

    public ViewModel() {
    }

    public ViewModel(Parcel in, ClassLoader loader) {
        this.mType = in.readInt();
        this.mInfo = in.readParcelable(loader);
        this.mInfoGravity = in.readInt();
        this.mContent = in.readParcelable(loader);
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mType);
        dest.writeParcelable(mInfo, 0);
        dest.writeInt(mInfoGravity);
        dest.writeParcelable(mContent, 0);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public void setInfo(StringRes info) {
        mInfo = info;
    }

    public void setInfoGravity(int gravity) {
        if (gravity != INFO_GRAVITY_TOP && gravity != INFO_GRAVITY_BOTTOM) {
            Log.w(TAG, "Do not support this gravity " + gravity + ", set as top.");
            mInfoGravity = INFO_GRAVITY_TOP;
        } else {
            mInfoGravity = gravity;
        }
    }

    public void setType(int type) {
        if (type != TYPE_EDIT && type != TYPE_SWITCH && type != TYPE_SPINNER) {
            Log.w(TAG, "Do not support this type " + type);
            return;
        }
        mType = type;
    }

    public void setContent(EditContent content) {
        mContent = content;
    }

    public String getInfo(Context context) {
        return mInfo == null ? null : mInfo.getString(context);
    }

    public int getInfoGravityType() {
        return mInfoGravity;
    }

    public int getContentType() {
        return mType;
    }

    public EditContent getContent() {
        return mContent;
    }

    public static class EditContent implements Parcelable {
        private OperationModel mOperation;
        private String mCurValue;
        private String mNewValue;

        public static int getEditContent(EditContent content, XmlPullParser xml,
                int curEventType) {
            if (content == null || curEventType != XmlPullParser.START_TAG) {
                return curEventType;
            }

            String function = xml.getAttributeValue(null, XmlUtils.ATTR_FUNCTION);
            String params = xml.getAttributeValue(null, XmlUtils.ATTR_PARAMS);
            content.setOperation(new OperationModel(function, params));

            return curEventType;
        }

        public static final ClassLoaderCreator<EditContent> CREATOR =
                new ClassLoaderCreator<EditContent>() {
            @Override
            public EditContent createFromParcel(Parcel in) {
                return new EditContent(in, null);
            }
            @Override
            public EditContent createFromParcel(Parcel in, ClassLoader loader) {
                return new EditContent(in, loader);
            }
            @Override
            public EditContent[] newArray(int size) {
                return new EditContent[size];
            }
        };

        public EditContent() {}

        public EditContent(Parcel in, ClassLoader loader) {
            this.mCurValue = in.readString();
            this.mNewValue = in.readString();
            this.mOperation = in.readParcelable(loader);
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(mCurValue);
            dest.writeString(mNewValue);
            dest.writeParcelable(mOperation, 0);
        }

        @Override
        public int describeContents() {
            return 0;
        }

        public void setOperation(OperationModel op) {
            mOperation = op;
        }

        public void setCurValue(String curValue) {
            mCurValue = curValue;
        }

        public void setNewValue(String newValue) {
            mNewValue = newValue;
        }

        public void resetValues() {
            mCurValue = null;
            mNewValue = null;
        }

        public String getFunction() {
            return mOperation == null ? null : mOperation.getFunction();
        }

        public String getParams() {
            return mOperation == null ? null : mOperation.getParams();
        }

        public String getCurValue() {
            return mCurValue;
        }

        public String getNewValue() {
            return mNewValue;
        }
    }

    public static class EditModel extends EditContent {
        private StringRes mHint;

        public static int getEditModel(EditModel model, XmlPullParser xml, int curEventType) {
            if (model == null
                    || curEventType != XmlPullParser.START_TAG
                    || !XmlUtils.TAG_EDIT.equals(xml.getName())) {
                return curEventType;
            }

            StringRes hint = XmlUtils.getAttributeAsStringRes(xml, XmlUtils.ATTR_HINT);
            model.setHint(hint);

            curEventType = EditContent.getEditContent(model, xml, curEventType);
            try {
                while (curEventType != XmlPullParser.END_DOCUMENT) {
                    if (curEventType == XmlPullParser.END_TAG
                            && XmlUtils.TAG_EDIT.equals(xml.getName())) {
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

        public static final ClassLoaderCreator<EditModel> CREATOR =
                new ClassLoaderCreator<EditModel>() {
            @Override
            public EditModel createFromParcel(Parcel in) {
                return new EditModel(in, null);
            }
            @Override
            public EditModel createFromParcel(Parcel in, ClassLoader loader) {
                return new EditModel(in, loader);
            }
            @Override
            public EditModel[] newArray(int size) {
                return new EditModel[size];
            }
        };

        public EditModel() {
            super();
        }

        public EditModel(Parcel in, ClassLoader loader) {
            super(in, loader);
            this.mHint = in.readParcelable(loader);
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            super.writeToParcel(dest, flags);
            dest.writeParcelable(mHint, 0);
        }

        public void setHint(StringRes hint) {
            mHint = hint;
        }

        public String getHint(Context context) {
            return mHint == null ? null : mHint.getString(context);
        }
    }

    public static class SwitchModel extends EditContent {
        private StringRes mLabel;
        private String mCheckedValue;
        private String mUnCheckedValue;

        public static int getSwitchModel(SwitchModel model, XmlPullParser xml, int curEventType) {
            if (model == null
                    || curEventType != XmlPullParser.START_TAG
                    || !XmlUtils.TAG_SWITCH.equals(xml.getName())) {
                return curEventType;
            }

            StringRes label = XmlUtils.getAttributeAsStringRes(xml, XmlUtils.ATTR_LABEL);
            model.setLabel(label);

            curEventType = EditContent.getEditContent(model, xml, curEventType);
            try {
                while (curEventType != XmlPullParser.END_DOCUMENT) {
                    if (curEventType == XmlPullParser.START_TAG
                            && XmlUtils.TAG_SET.equals(xml.getName())) {
                        String checked = xml.getAttributeValue(null, XmlUtils.ATTR_CHECKED);
                        String value = xml.getAttributeValue(null, XmlUtils.ATTR_VALUE);
                        model.setValue(Boolean.parseBoolean(checked), value);
                    }

                    if (curEventType == XmlPullParser.END_TAG
                            && XmlUtils.TAG_SWITCH.equals(xml.getName())) {
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

        public static final ClassLoaderCreator<SwitchModel> CREATOR =
                new ClassLoaderCreator<SwitchModel>() {
            @Override
            public SwitchModel createFromParcel(Parcel in) {
                return new SwitchModel(in, null);
            }
            @Override
            public SwitchModel createFromParcel(Parcel in, ClassLoader loader) {
                return new SwitchModel(in, loader);
            }
            @Override
            public SwitchModel[] newArray(int size) {
                return new SwitchModel[size];
            }
        };

        public SwitchModel() {
            super();
        }

        public SwitchModel(Parcel in, ClassLoader loader) {
            super(in, loader);
            this.mLabel = in.readParcelable(loader);
            this.mCheckedValue = in.readString();
            this.mUnCheckedValue = in.readString();
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            super.writeToParcel(dest, flags);
            dest.writeParcelable(mLabel, 0);
            dest.writeString(mCheckedValue);
            dest.writeString(mUnCheckedValue);
        }

        public void setLabel(StringRes label) {
            mLabel = label;
        }

        public void setValue(boolean checked, String value) {
            if (checked) {
                mCheckedValue = value;
            } else {
                mUnCheckedValue = value;
            }
        }

        public void setNewValue(boolean checked) {
            setNewValue(checked ? mCheckedValue : mUnCheckedValue);
        }

        public String getLabel(Context context) {
            return mLabel == null ? null : mLabel.getString(context);
        }

        public boolean getChecked(String value) {
            return value.equals(mCheckedValue);
        }
    }

    public static class SpinnerModel extends EditContent {
        private Parcelable[] mContents;

        public static int getSpinnerModel(SpinnerModel model, XmlPullParser xml,
                int curEventType) {
            if (model == null
                    || curEventType != XmlPullParser.START_TAG
                    || !XmlUtils.TAG_SPINNER.equals(xml.getName())) {
                return curEventType;
            }

            curEventType = EditContent.getEditContent(model, xml, curEventType);

            ArrayList<SpinnerContent> list = new ArrayList<SpinnerContent>();
            try {
                while (curEventType != XmlPullParser.END_DOCUMENT) {
                    if (curEventType == XmlPullParser.START_TAG
                            && XmlUtils.TAG_SET.equals(xml.getName())) {
                        StringRes label = XmlUtils.getAttributeAsStringRes(
                                xml, XmlUtils.ATTR_LABEL);
                        String value = xml.getAttributeValue(null, XmlUtils.ATTR_VALUE);
                        if (label != null && !TextUtils.isEmpty(value)) {
                            SpinnerContent content = new SpinnerContent(label, value);
                            list.add(content);
                        }
                    }

                    if (curEventType == XmlPullParser.END_TAG
                            && XmlUtils.TAG_SPINNER.equals(xml.getName())) {
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
                SpinnerContent[] contents = new SpinnerContent[list.size()];
                list.toArray(contents);
                model.setSpinnerContents(contents);
            }

            return curEventType;
        }

        public static final ClassLoaderCreator<SpinnerModel> CREATOR =
                new ClassLoaderCreator<SpinnerModel>() {
            @Override
            public SpinnerModel createFromParcel(Parcel in) {
                return new SpinnerModel(in, null);
            }
            @Override
            public SpinnerModel createFromParcel(Parcel in, ClassLoader loader) {
                return new SpinnerModel(in, loader);
            }
            @Override
            public SpinnerModel[] newArray(int size) {
                return new SpinnerModel[size];
            }
        };

        public SpinnerModel() {
            super();
        }

        public SpinnerModel(Parcel in, ClassLoader loader) {
            super(in, loader);
            this.mContents = in.readParcelableArray(loader);
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            super.writeToParcel(dest, flags);
            dest.writeParcelableArray(mContents, 0);
        }

        public void setSpinnerContents(SpinnerContent[] contents) {
            mContents = contents;
        }

        public SpinnerContent[] getSpinnerContents() {
            if (mContents == null) return null;

            SpinnerContent[] contents = new SpinnerContent[mContents.length];
            for (int i = 0; i < mContents.length; i++) {
                contents[i] = (SpinnerContent) mContents[i];
            }
            return contents;
        }

        public static class SpinnerContent implements Parcelable {
            private String mValue;
            private StringRes mLabel;

            public static final ClassLoaderCreator<SpinnerContent> CREATOR =
                    new ClassLoaderCreator<SpinnerContent>() {
                @Override
                public SpinnerContent createFromParcel(Parcel in) {
                    return new SpinnerContent(in, null);
                }
                @Override
                public SpinnerContent createFromParcel(Parcel in, ClassLoader loader) {
                    return new SpinnerContent(in, loader);
                }
                @Override
                public SpinnerContent[] newArray(int size) {
                    return new SpinnerContent[size];
                }

            };

            public SpinnerContent(StringRes label, String value) {
                this.mValue = value;
                this.mLabel = label;
            }

            public SpinnerContent(Parcel in, ClassLoader loader) {
                this.mValue = in.readString();
                this.mLabel = in.readParcelable(loader);
            }

            @Override
            public void writeToParcel(Parcel dest, int flags) {
                dest.writeString(mValue);
                dest.writeParcelable(mLabel, 0);
            }

            @Override
            public int describeContents() {
                return 0;
            }

            public String getLabel(Context context) {
                return mLabel == null ? null : mLabel.getString(context);
            }

            public String getValue() {
                return mValue;
            }
        }
    }
}
