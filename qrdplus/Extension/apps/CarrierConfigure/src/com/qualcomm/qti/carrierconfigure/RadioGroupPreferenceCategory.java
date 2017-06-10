/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.carrierconfigure;

import android.content.Context;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.util.AttributeSet;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;

public class RadioGroupPreferenceCategory extends PreferenceCategory {

    private String mCheckedPreferenceKey;
    private HashMap<String, RadioButtonPreference> mRadioPreferenceMap;

    public RadioGroupPreferenceCategory(Context context) {
        this(context, null);
    }

    public RadioGroupPreferenceCategory(Context context, AttributeSet attrs) {
        this(context, attrs, com.android.internal.R.attr.preferenceCategoryStyle);
    }

    public RadioGroupPreferenceCategory(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        mRadioPreferenceMap = new HashMap<String, RadioButtonPreference>();
        mCheckedPreferenceKey = null;
    }

    @Override
    public boolean addPreference(Preference preference) {
        if (preference instanceof RadioButtonPreference) {
            mRadioPreferenceMap.put(preference.getKey(), (RadioButtonPreference) preference);
            return super.addPreference(preference);
        } else {
            throw new IllegalArgumentException("Do not accept this preference: " + preference);
        }
    }

    @Override
    public boolean removePreference(Preference preference) {
        if (preference instanceof RadioButtonPreference) {
            mRadioPreferenceMap.remove(preference.getKey());
            return super.removePreference(preference);
        } else {
            throw new IllegalArgumentException("Couldn't find this preference: " + preference);
        }
    }

    @Override
    public void removeAll() {
        mRadioPreferenceMap.clear();
        super.removeAll();
    }

    public void setCheckedPreference(String preferenceKey) {
        mCheckedPreferenceKey = preferenceKey;
        updateGroupCheckedStatus();
    }

    public String getCheckedPreferenceKey() {
        return mCheckedPreferenceKey;
    }

    public RadioButtonPreference getCheckedPreference() {
        return mRadioPreferenceMap.get(mCheckedPreferenceKey);
    }

    private void updateGroupCheckedStatus() {
        Iterator<Entry<String, RadioButtonPreference>> iterator =
                mRadioPreferenceMap.entrySet().iterator();
        while (iterator.hasNext()) {
            Entry<String, RadioButtonPreference> entry = iterator.next();
            if (mCheckedPreferenceKey == null) return;
            entry.getValue().setChecked(mCheckedPreferenceKey.equals(entry.getKey()));
        }
    }
}
