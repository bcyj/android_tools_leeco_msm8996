/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.universaldownload;

import android.content.Context;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.TextView;

public class UpdateListItem extends LinearLayout {

    private final TextView mUpdateTitle;
    private final TextView mUpdateVersion;

    public UpdateListItem(Context context) {
        super(context);
        this.inflate(context, R.layout.update_list_item, this);
        mUpdateTitle = (TextView) findViewById(R.id.update_item_title);
        mUpdateVersion = (TextView) findViewById(R.id.update_item_new_ver);
    }

    public void setUpdateTitle(String title) {
        mUpdateTitle.setText(title);
    }

    public void setUpdateVersion(String version) {
        mUpdateVersion.setText(version);
    }

    public void setChecked(boolean checked) {
        CheckBox checkBox = (CheckBox) findViewById(R.id.checkBox);
        checkBox.setVisibility(View.VISIBLE);
        checkBox.setChecked(checked);
    }

    public void setOncheckChangeListener(CompoundButton.OnCheckedChangeListener listener) {
        CheckBox checkBox = (CheckBox) findViewById(R.id.checkBox);
        checkBox.setOnCheckedChangeListener(listener);
    }
}
