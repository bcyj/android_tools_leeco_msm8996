/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.SparseBooleanArray;
import android.view.View;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;

import com.suntek.mway.rcs.nativeui.R;

public class ComplainAccountActivity extends Activity {

    private String[] reasonItems;
    private String accoutId;
    private TextView reasonEdit;
    private EditText desEdit;
    private String reasonStr;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.complain_account);
        reasonItems = getResources().getStringArray(
                R.array.complain_reason_detail);
        accoutId = getIntent().getStringExtra("accountId");
        reasonEdit = (TextView) findViewById(R.id.complain_reason);
        desEdit = (EditText) findViewById(R.id.complain_description);

        reasonEdit.setClickable(true);
        reasonEdit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new AlertDialog.Builder(ComplainAccountActivity.this)
                        .setTitle(R.string.reason_detail)
                        .setMultiChoiceItems(
                                reasonItems,
                                new boolean[] { false, false, false, false,
                                        false, false }, null)
                        .setPositiveButton(getString(R.string.send_comfirm_ok),
                                new DialogInterface.OnClickListener() {

                                    @Override
                                    public void onClick(DialogInterface dialog,
                                            int which) {
                                        AlertDialog alertDialog = (AlertDialog) dialog;
                                        ListView listView = alertDialog
                                                .getListView();
                                        SparseBooleanArray selects = listView
                                                .getCheckedItemPositions();
                                        int size = selects.size();
                                        for (int i = 0; i < size; i++) {
                                            if (selects.valueAt(i)) {
                                                reasonStr += reasonItems[i]
                                                        + " ";
                                                reasonEdit.setText(reasonStr);
                                            }
                                        }

                                    }
                                })
                        .setNegativeButton(
                                getString(R.string.send_comfirm_cancel), null)
                        .create().show();
            }
        });
    }
}
