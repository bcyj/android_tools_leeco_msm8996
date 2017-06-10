/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

package com.android.backup;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class HelpActivity extends Activity {

    private TextView mTextContent;

    public static final String HELP_TYPE_KEY = "help_type_key";
    public static final int TYPE_SETTING = 0;
    public static final int TYPE_BACKUP = 1;
    public static final int TYPE_RESTORE = 2;

    private int mHelpType;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.help_activity);

        mTextContent = (TextView) findViewById(R.id.tv_content);
        mHelpType = getIntent().getIntExtra(HELP_TYPE_KEY, TYPE_SETTING);
        UpdateHelpContent();
    }

    private void UpdateHelpContent() {
        if (mHelpType == TYPE_SETTING) {
            if (BackupUtils.isSDSupported()) {
                if (BackupUtils.isSDMounted()) {
                    mTextContent.setText(R.string.no_sdcard);
                } else {
                    mTextContent.setText(R.string.no_sdcard);
                }
            } else {
                mTextContent.setText(R.string.not_support_sdcard);
            }
        } else if (mHelpType == TYPE_BACKUP) {
            if (BackupUtils.isSDSupported()) {
                if (BackupUtils.isSDMounted()) {
                    mTextContent.setText(R.string.no_sdcard_backup);
                } else {
                    mTextContent.setText(R.string.no_sdcard_backup);
                }
            } else {
                mTextContent.setText(R.string.not_support_sdcard_backup);
            }
        } else {
            if (BackupUtils.isSDSupported()) {
                if (BackupUtils.isSDMounted()) {
                    mTextContent.setText(R.string.no_sdcard_restore);
                } else {
                    mTextContent.setText(R.string.no_sdcard_restore);
                }
            } else {
                mTextContent.setText(R.string.not_support_sdcard_restore);
            }
        }
    }

}
