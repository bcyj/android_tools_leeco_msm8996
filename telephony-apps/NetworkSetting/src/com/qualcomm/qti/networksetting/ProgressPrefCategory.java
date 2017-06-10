/**
 * Copyright (c) 2013-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.networksetting;

import android.content.Context;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.util.AttributeSet;
import android.view.View;

public class ProgressPrefCategory extends PreferenceCategory {

    private Preference mNoResultFoundPreference;
    private boolean mNoResultFoundAdded;
    private boolean mProgress = false;
    private String mEmptyTextRes;

    public ProgressPrefCategory(Context context, AttributeSet attrs) {
        super(context, attrs);
        setLayoutResource(R.layout.progress_category);
        mEmptyTextRes = context.getString(attrs.getAttributeResourceValue(null, "empty_text", 0));
        setTitle(attrs.getAttributeResourceValue(null, "title", 0));
    }

    @Override
    public void onBindView(View view) {
        super.onBindView(view);
        final View progressBar = view.findViewById(R.id.scanning_progress);

        boolean noResultFound = (getPreferenceCount() == 0 ||
                (getPreferenceCount() == 1 && getPreference(0) == mNoResultFoundPreference));
        progressBar.setVisibility(mProgress ? View.VISIBLE : View.GONE);

        if (mProgress || !noResultFound) {
            if (mNoResultFoundAdded) {
                removePreference(mNoResultFoundPreference);
                mNoResultFoundAdded = false;
            }
        } else {
            if (!mNoResultFoundAdded) {
                if (mNoResultFoundPreference == null) {
                    mNoResultFoundPreference = new Preference(getContext());
                    mNoResultFoundPreference.setLayoutResource(R.layout.category_empty_list);
                    mNoResultFoundPreference.setTitle(mEmptyTextRes);
                    mNoResultFoundPreference.setSelectable(false);
                }
                addPreference(mNoResultFoundPreference);
                mNoResultFoundAdded = true;
            }
        }
    }

    public void setProgress(boolean progressOn) {
        mProgress = progressOn;
        notifyChanged();
    }
}
