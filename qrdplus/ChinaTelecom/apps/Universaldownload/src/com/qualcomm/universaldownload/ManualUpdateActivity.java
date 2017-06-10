/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
*/
package com.qualcomm.universaldownload;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.LinearLayout;

import java.util.List;

public class ManualUpdateActivity extends Activity {

    private DownloadSharePreference mPref;
    private View.OnClickListener mUpdateOnclick = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            Intent intent = new Intent(ManualUpdateActivity.this, UpdateListActivity.class);
            startActivity(intent);
        }
    };
    private View.OnClickListener mCancelOnclick = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            finish();
        }
    };

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.manualupdate);
        Button cancel = (Button) this.findViewById(R.id.btn_cancel);
        Button update = (Button) this.findViewById(R.id.btn_update);
        cancel.setOnClickListener(mCancelOnclick);
        update.setOnClickListener(mUpdateOnclick);
        mPref = DownloadSharePreference.Instance(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        loadItem();
    }

    private void loadItem() {
        List<UpdateItem> items = mPref.getUpdateItems();
        LinearLayout updateItems = (LinearLayout) findViewById(R.id.update_item);
        updateItems.removeAllViews();
        for(final UpdateItem item : items) {
            UpdateListItem itemView = new UpdateListItem(this);
            itemView.setUpdateTitle(getCapabilityLabel(item.getCapabilityName()));
            String current_version = this.getString(R.string.current_version);
            if (item.getVersion() == null || item.getVersion().equals("")) {
                current_version += "0.0";
            } else {
                current_version += item.getVersion();
            }
            itemView.setUpdateVersion(current_version);
            itemView.setChecked(item.isChecked());
            itemView.setOncheckChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                    mPref.checkCapability(item.getCapabilityName(), b);
                }
            });
            updateItems.addView(itemView);
        }
    }

    private String getCapabilityLabel(String name) {
        String[] updateLabels = getResources().getStringArray(R.array.pref_update_items_label);
        for(int i = 0; i < updateLabels.length; i++) {
            if(updateLabels[i++].equals(name)) {
                return  updateLabels[i];
            }
        }
        return null;
    }
}
