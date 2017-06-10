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

package com.android.wifi.cmcc;

import android.content.Context;
import android.content.DialogInterface;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.provider.Settings;
import android.view.View;
import android.widget.CheckBox;
import android.widget.TextView;
import android.widget.Toast;
import android.util.Log;
import com.android.wifi.cmcc.R;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

public class ChangeDataConnectionDialog extends AlertActivity implements
        DialogInterface.OnClickListener {

    private static final String WIFI_BROWSER_INTERACTION_REMIND_TYPE =
            "wifi_browser_interaction_remind";
    private static final int TYPE_OK = 1;
    private static final int TYPE_CANCEL = 0;

    private Context mContext;
    private CheckBox mCb;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = getApplicationContext();
        // Set up the "dialog"
        final AlertController.AlertParams p = mAlertParams;
        p.mIconId = android.R.drawable.ic_dialog_alert;
        p.mTitle = getString(R.string.wifi_failover_gprs_title);
        p.mView = createView();
        p.mPositiveButtonText = getString(android.R.string.ok);
        p.mPositiveButtonListener = this;
        p.mNegativeButtonText = getString(android.R.string.cancel);
        p.mNegativeButtonListener = this;
        setupAlert();
    }

    private View createView() {
        View view = getLayoutInflater().inflate(R.layout.change_data_connection_dialog, null);
        TextView contentView = (TextView) view.findViewById(R.id.setContent);
        contentView.setText(getString(R.string.wifi_failover_gprs_summary));
        mCb = (CheckBox) view.findViewById(R.id.setPrimary);
        return view;
    }

    public void onClick(DialogInterface dialog, int which) {
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:

                Utils.setMobileDataEnabled(mContext, true);

                if (mCb.isChecked() == true) {
                    Settings.System.putInt(mContext.getContentResolver(),
                            WIFI_BROWSER_INTERACTION_REMIND_TYPE,
                            TYPE_CANCEL);
                }

                finish();
                break;

            case DialogInterface.BUTTON_NEGATIVE:

                Utils.setMobileDataEnabled(mContext, false);

                if (mCb.isChecked() == true) {
                    Settings.System.putInt(mContext.getContentResolver(),
                            WIFI_BROWSER_INTERACTION_REMIND_TYPE,
                            TYPE_CANCEL);
                }

                finish();
                break;
        }
    }
}
