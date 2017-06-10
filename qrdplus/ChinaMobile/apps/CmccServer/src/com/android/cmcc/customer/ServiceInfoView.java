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
package com.android.cmcc.customer;

import com.android.cmcc.R;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.os.Build;

import android.os.SystemProperties;

public class ServiceInfoView extends Activity {

    public static final int SERCICE_GUIDE_TYPE = 1;
    public static final int MONTERNET_SMS_TYPE = 2;
    public static final int MONTERNET_MMS_TYPE = 3;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.service_info_view);
        TextView localTextView = (TextView) findViewById(R.id.textview_normal);
        Intent localIntent = getIntent();
        int i = localIntent.getExtras().getInt("type");
        Log.i("**InfoView**", "" + i);
        if (localIntent != null) {
            switch (i) {
            case SERCICE_GUIDE_TYPE: {
                setTitle(R.string.service_guide);
                StringBuffer sb = new StringBuffer();
                String device = getString(R.string.service_guide_device);
                sb.append(device);
                String deviceName = SystemProperties.get("ro.product.model",
                        "");//need to write
                sb.append(deviceName);
                sb.append("\n");
                String serviceNum = getString(R.string.service_guide_servicenumber);
                sb.append(serviceNum);

                String serviceNumName = "";//need to write

                sb.append(serviceNumName);
                sb.append("\n");
                String guideWebsite = getString(R.string.service_guide_website);
                sb.append(guideWebsite);
                String guideWebsiteName = "";//need to write
                sb.append(guideWebsiteName);
                localTextView.setText(sb.toString());
                break;
            }
            case MONTERNET_SMS_TYPE: {
                setTitle(R.string.mymonternet_sms);
                localTextView.setText(R.string.monternet_sms_txt);
                break;
            }
            case MONTERNET_MMS_TYPE: {
                setTitle(R.string.mymonternet_mms);
                localTextView.setText(R.string.monternet_mms_txt);
                break;
            }
            }

        }
    }
}
