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

public class DialogModel implements Parcelable {
    private static final String TAG = "DialogModel";

    private StringRes mTitle;
    private StringRes mMessage;
    private int mIconResId;
    private DialogButton mPositiveButton;
    private DialogButton mNeutralButton;
    private DialogButton mNegativeButton;

    private ViewModel mView;

    public static int getDialogModel(Context context, DialogModel model, XmlPullParser xml,
            int curEventType) {
        if (model == null
                || curEventType != XmlPullParser.START_TAG
                || !XmlUtils.TAG_DIALOG.equals(xml.getName())) {
            return curEventType;
        }

        // Get the title, message, icon.
        StringRes title = XmlUtils.getAttributeAsStringRes(xml, XmlUtils.ATTR_TITLE);
        model.setTitle(title);

        StringRes message = XmlUtils.getAttributeAsStringRes(xml, XmlUtils.ATTR_MESSAGE);
        model.setMessage(message);

        int icon = XmlUtils.getAttributeResourceValue(xml, XmlUtils.ATTR_ICON);
        model.setIconResId(icon);

        try {
            while (curEventType != XmlPullParser.END_DOCUMENT) {
                if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_POSITIVE.equals(xml.getName())) {
                    DialogButton positive = new DialogButton();
                    curEventType = DialogButton.getDialogButton(
                            context, positive, xml, curEventType);
                    model.setPositiveButton(positive);
                } else if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_NEUTRAL.equals(xml.getName())) {
                    DialogButton neutral = new DialogButton();
                    curEventType = DialogButton.getDialogButton(
                            context, neutral, xml, curEventType);
                    model.setNeutralButton(neutral);
                } else if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_NEGATIVE.equals(xml.getName())) {
                    DialogButton negative = new DialogButton();
                    curEventType = DialogButton.getDialogButton(
                            context, negative, xml, curEventType);
                    model.setNegativeButton(negative);
                } else if (curEventType == XmlPullParser.START_TAG
                        && XmlUtils.TAG_VIEW.equals(xml.getName())) {
                    ViewModel view = new ViewModel();
                    curEventType = ViewModel.getViewModel(view, xml, curEventType);
                    model.setView(view);
                }

                if (curEventType == XmlPullParser.END_TAG
                        && XmlUtils.TAG_DIALOG.equals(xml.getName())) {
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

    public static final ClassLoaderCreator<DialogModel> CREATOR =
            new ClassLoaderCreator<DialogModel>() {
        @Override
        public DialogModel createFromParcel(Parcel in) {
            return new DialogModel(in, null);
        }
        @Override
        public DialogModel createFromParcel(Parcel in, ClassLoader loader) {
            return new DialogModel(in, loader);
        }
        @Override
        public DialogModel[] newArray(int size) {
            return new DialogModel[size];
        }
    };

    public DialogModel() {
        super();
    }

    public DialogModel(Parcel in, ClassLoader loader) {
        this.mTitle = in.readParcelable(loader);
        this.mMessage = in.readParcelable(loader);
        this.mIconResId = in.readInt();
        this.mPositiveButton = in.readParcelable(loader);
        this.mNeutralButton = in.readParcelable(loader);
        this.mNegativeButton = in.readParcelable(loader);
        this.mView = in.readParcelable(loader);
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeParcelable(mTitle, 0);
        dest.writeParcelable(mMessage, 0);
        dest.writeInt(mIconResId);
        dest.writeParcelable(mPositiveButton, 0);
        dest.writeParcelable(mNeutralButton, 0);
        dest.writeParcelable(mNegativeButton, 0);
        dest.writeParcelable(mView, 0);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public void setTitle(StringRes title) {
        mTitle = title;
    }

    public void setMessage(StringRes message) {
        mMessage = message;
    }

    public void setIconResId(int resId) {
        mIconResId = resId;
    }

    public void setPositiveButton(DialogButton button) {
        mPositiveButton = button;
    }

    public void setNeutralButton(DialogButton button) {
        mNeutralButton = button;
    }

    public void setNegativeButton(DialogButton button) {
        mNegativeButton = button;
    }

    public void setView(ViewModel view) {
        mView = view;
    }

    public String getTitle(Context context) {
        return mTitle == null ? null : mTitle.getString(context);
    }

    public String getMessage(Context context) {
        return mMessage == null ? null : mMessage.getString(context);
    }

    public int getIconResId() {
        return mIconResId;
    }

    public DialogButton getPositiveButton() {
        return mPositiveButton;
    }

    public DialogButton getNeutralButton() {
        return mNeutralButton;
    }

    public DialogButton getNegativeButton() {
        return mNegativeButton;
    }

    public ViewModel getView() {
        return mView;
    }

    public static class DialogButton implements Parcelable {
        private StringRes mLabel;
        private OperationModel mOperation;

        private ClickActionModel mClickAction;

        public static int getDialogButton(Context context, DialogButton button, XmlPullParser xml,
                int curEventType) {
            if (button == null || curEventType != XmlPullParser.START_TAG) {
                return curEventType;
            }

            String curName = xml.getName();

            // Get the label and operation.
            StringRes label = XmlUtils.getAttributeAsStringRes(xml, XmlUtils.ATTR_LABEL);
            button.setLabel(label);

            String function = xml.getAttributeValue(null, XmlUtils.ATTR_FUNCTION);
            if (!TextUtils.isEmpty(function)) {
                String params = xml.getAttributeValue(null, XmlUtils.ATTR_PARAMS);
                int delay = XmlUtils.getAttributeValueAsInt(xml, XmlUtils.ATTR_DELAY_MILLIS);
                button.setOperation(new OperationModel(function, params, delay));
            }

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

            button.setClickAction(clickAction);
            return curEventType;
        }

        public static final ClassLoaderCreator<DialogButton> CREATOR =
                new ClassLoaderCreator<DialogButton>() {
            @Override
            public DialogButton createFromParcel(Parcel in) {
                return new DialogButton(in, null);
            }
            @Override
            public DialogButton createFromParcel(Parcel in, ClassLoader loader) {
                return new DialogButton(in, loader);
            }
            @Override
            public DialogButton[] newArray(int size) {
                return new DialogButton[size];
            }
        };

        public DialogButton() {
            super();
        }

        public DialogButton(Parcel in, ClassLoader loader) {
            this.mLabel = in.readParcelable(loader);
            this.mOperation = in.readParcelable(loader);
            this.mClickAction = in.readParcelable(loader);
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeParcelable(mLabel, 0);
            dest.writeParcelable(mOperation, 0);
            dest.writeParcelable(mClickAction, 0);
        }

        @Override
        public int describeContents() {
            return 0;
        }

        public void setLabel(StringRes label) {
            mLabel = label;
        }

        public void setOperation(OperationModel operationModel) {
            mOperation = operationModel;
        }

        public void setClickAction(ClickActionModel clickAction) {
            mClickAction = clickAction;
        }

        public String getLabel(Context context) {
            return mLabel == null ? null : mLabel.getString(context);
        }

        public String getFunction() {
            return mOperation == null ? null : mOperation.getFunction();
        }

        public String getParams() {
            return mOperation == null ? null : mOperation.getParams();
        }

        public int getDelayMillis() {
            return mOperation == null ? -1 : mOperation.getDelayMillis();
        }

        public Parcelable getClickAction() {
            return mClickAction == null ? null : mClickAction.getClickAction();
        }
    }

}
