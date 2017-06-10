/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.gesture;

import java.util.ArrayList;
import java.util.List;

import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ResolveInfo;
import android.database.Cursor;

public class GestureAction {

    public static final String EXTRA_ACTION = "action";

    public static final String EXTRA_GESTURE = "gesture";

    private final String mPackageName;
    private final String mClassName;
    private boolean mEnable;

    private static final String SPLIT_COMPONENT_NAME = "_";

    public GestureAction(String packageName, String className, boolean enable) {
        mPackageName = packageName;
        mClassName = className;
        mEnable = enable;
    }

    public GestureAction(Cursor c) {
        this(c.getString(c.getColumnIndex(GesturesStore.COLUMN_GESTURE_PACKAGE_NAME)),
                c.getString(c.getColumnIndex(GesturesStore.COLUMN_GESTURE_CLASS_NAME)),
                c.getInt(c.getColumnIndex(GesturesStore.COLUMN_GESTURE_ENABLE)) == 1);
    }

    public String getPackageName() {
        return mPackageName;
    }

    public String getClassName() {
        return mClassName;
    }

    public boolean isEnabled() {
        return mEnable;
    }

    public void setEnable(boolean enable) {
        mEnable = enable;
    }

    public String getActionName(Context context) {
        Intent intent = new Intent();
        intent.setClassName(mPackageName, mClassName);
        List<ResolveInfo> infos = context.getPackageManager().queryIntentActivities(intent, 0);
        if (infos != null && !infos.isEmpty()) {
            return (String) infos.get(0).loadLabel(context.getPackageManager());
        }
        return null;
    }

    public boolean isValid(Context context) {
        Intent intent = new Intent();
        intent.setClassName(mPackageName, mClassName);
        List<ResolveInfo> infos = context.getPackageManager().queryIntentActivities(intent, 0);
        return infos != null && !infos.isEmpty();
    }

    public static List<GestureAction> listAvailableActions(Context context) {
        List<GestureAction> actions = new ArrayList<GestureAction>();
        Cursor c = null;
        try {
            c = GesturesStore.getInstance().query(GesturesStore.TABLE_GESTURES, null, null, null,
                    null);
            GestureAction gestureAction = null;
            while (c != null && c.moveToNext()) {
                if ((gestureAction = new GestureAction(c)).isValid(context)) {
                    actions.add(gestureAction);
                }
            }
        } finally {
            if (c != null) {
                c.close();
            }
        }
        return actions;
    }

    public static List<GestureAction> listEnabledActions(Context context) {
        List<GestureAction> actions = new ArrayList<GestureAction>();
        Cursor c = null;
        try {
            c = GesturesStore.getInstance().query(GesturesStore.TABLE_GESTURES, null,
                    GesturesStore.COLUMN_GESTURE_ENABLE + "=?", new String[] {
                        "1"
                    }, null);
            GestureAction gestureAction = null;
            while (c != null && c.moveToNext()) {
                if ((gestureAction = new GestureAction(c)).isValid(context)) {
                    actions.add(gestureAction);
                }
            }
        } finally {
            if (c != null) {
                c.close();
            }
        }
        return actions;
    }

    public String getKey() {
        return mPackageName + SPLIT_COMPONENT_NAME + mClassName;
    }

    public static String[] getComponentName(String action) {
        if (action == null || action.indexOf(SPLIT_COMPONENT_NAME) == -1) {
            return null;
        }
        return action.split(SPLIT_COMPONENT_NAME);
    }

    public static GestureAction queryAction(String actionKey) {
        String[] componentName = getComponentName(actionKey);
        if (componentName == null) {
            return null;
        }
        Cursor c = null;
        try {
            c = GesturesStore.getInstance().query(
                    GesturesStore.TABLE_GESTURES,
                    null,
                    GesturesStore.COLUMN_GESTURE_PACKAGE_NAME + "=? and "
                            + GesturesStore.COLUMN_GESTURE_CLASS_NAME + "=?", new String[] {
                            componentName[0], componentName[1]
                    }, null);
            if (c != null && c.moveToNext()) {
                return new GestureAction(c);
            }
        } finally {
            if (c != null) {
                c.close();
            }
        }
        return null;
    }

    public boolean save() {
        ContentValues values = new ContentValues();
        values.put(GesturesStore.COLUMN_GESTURE_PACKAGE_NAME, mPackageName);
        values.put(GesturesStore.COLUMN_GESTURE_CLASS_NAME, mClassName);
        values.put(GesturesStore.COLUMN_GESTURE_ENABLE, mEnable);
        if (GesturesStore.getInstance().update(
                GesturesStore.TABLE_GESTURES,
                values,
                GesturesStore.COLUMN_GESTURE_PACKAGE_NAME + "=? and "
                        + GesturesStore.COLUMN_GESTURE_CLASS_NAME + "=?", new String[] {
                        mPackageName, mClassName
                }) > 0) {
            return true;
        }
        return GesturesStore.getInstance().insert(GesturesStore.TABLE_GESTURES, values) != -1;
    }
}
