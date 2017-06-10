/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.carrierconfigure;

import android.content.Context;
import android.preference.TwoStatePreference;
import android.util.AttributeSet;
import android.view.View;
import android.widget.Checkable;
import android.widget.RadioButton;

public class RadioButtonPreference extends TwoStatePreference {

    private Object mObject;

    public RadioButtonPreference(Context context) {
        this(context, null);
    }

    public RadioButtonPreference(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public RadioButtonPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);

        // Reset the layout.
        setLayoutResource(R.layout.preference_item);
        setWidgetLayoutResource(R.layout.preference_widget_radio);
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);

        // Set the RadioButton's checked value.
        RadioButton radio = (RadioButton) view.findViewById(R.id.radio_button);
        if (radio != null && radio instanceof Checkable) {
            radio.setChecked(isChecked());
        }
    }

    public void setTag(Object o) {
        mObject = o;
    }

    public Object getTag() {
        return mObject;
    }
}
