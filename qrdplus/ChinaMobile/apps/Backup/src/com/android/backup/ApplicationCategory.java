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

import android.content.Context;
import android.preference.PreferenceCategory;
import android.util.AttributeSet;
import android.view.View;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Map;
import android.preference.Preference;

import android.util.Log;

public class ApplicationCategory extends PreferenceCategory {

    private boolean mProgress = false;
    private int mPreferenceSize = 0;
    private ArrayList<ApplicationPreference> mAppList;
    private View mView = null;
    private int iTitleResId = -1;

    public ApplicationCategory(Context context, AttributeSet attrs) {
        super(context, attrs);
        setLayoutResource(R.layout.preference_category);
        mPreferenceSize = 0;
        mAppList = new ArrayList<ApplicationPreference>();
    }

    @Override
    public void onBindView(View view) {
        super.onBindView(view);
        mView = view;
        if (iTitleResId != -1) {
            setTitleContent(iTitleResId);
        }
    }

    @Override
    protected boolean onPrepareAddPreference(Preference preference) {
        Log.d("ApplicationCategory", "onPrepareAddPreference: mPreferenceSize: " + mPreferenceSize);
        mPreferenceSize++;
        mAppList.add((ApplicationPreference) preference);
        return true;
    }

    public int size() {
        return mPreferenceSize;
    }

    public ApplicationPreference get(int pos) {
        if (mAppList != null) {
            return mAppList.get(pos);
        }
        return null;
    }

    public void clearAppList() {
        Log.d("ApplicationCategory", "clearAppList:");
        if (mAppList != null) {
            mAppList.clear();
        }
        mAppList = null;
    }

    public void setTitleContent(int resId) {
        if (mView != null) {
            TextView view = (TextView) (mView.findViewById(R.id.title));
            view.setText(resId);
        }
        iTitleResId = resId;
    }
}
