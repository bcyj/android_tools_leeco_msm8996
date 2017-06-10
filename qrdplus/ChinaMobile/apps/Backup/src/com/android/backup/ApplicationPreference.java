/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.backup;

import com.android.backup.R;

import android.content.Context;
import android.preference.Preference;
import android.preference.CheckBoxPreference;
import android.util.TypedValue;
import android.view.View;
import android.widget.ImageView;
import android.widget.CheckedTextView;
import android.widget.TextView;
import android.util.Log;
import android.view.ViewGroup;

public class ApplicationPreference extends Preference {
    private static final String TAG = "ApplicationPreference";

    private static int sDimAlpha = Integer.MIN_VALUE;
    private String mAppName = null;
    private AppInfo mAppInfo = null;
    private boolean mIsChecked = false;
    CheckedTextView mCheckView = null;

    public ApplicationPreference(Context context, AppInfo app) {
        super(context);

        setLayoutResource(R.layout.preference_application);
        mAppInfo = app;
        mAppName = mAppInfo.appName;
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        TextView txtview = (TextView) view.findViewById(R.id.app_name);
        Log.d(TAG, "ApplicationPreference name: " + mAppName);
        txtview.setText(mAppName);
        mCheckView = (CheckedTextView) view.findViewById(R.id.check);
        mCheckView.setChecked(mIsChecked);
    }

    @Override
    public int compareTo(Preference another) {
        return 1;
    }

    public AppInfo getCachedAppInfo() {
        return mAppInfo;
    }

    public boolean isChecked() {
        Log.d(TAG, "isChecked mIsChecked: " + mIsChecked);
        return mIsChecked;
    }

    public View getView(View convertView, ViewGroup parent) {
        if (convertView == null) {
            convertView = onCreateView(parent);
        }
        onBindView(convertView);
        return convertView;
    }

    public void setChecked() {
        mIsChecked = !mIsChecked;
        Log.d(TAG, "setChecked mIsChecked: " + mIsChecked);
        mCheckView.setChecked(mIsChecked);
    }

    public void setChecked(boolean checkOrnot) {
        Log.d(TAG, "Setchecked:" + (checkOrnot == true ? "true" : "false"));
        mCheckView.setChecked(checkOrnot);
    }
}
