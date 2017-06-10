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

package com.android.dm;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.ListView;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.Context;

public class DmApp extends Activity {

    private ListView mListView;
    private String[] mlistItem;
    private Toast mToast;
    private Context mContext;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        mListView = (ListView) findViewById(R.id.app_list);

        // init list item content
        mlistItem = getResources().getStringArray(R.array.app_list);

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, R.layout.text_view, mlistItem);
        mListView.setAdapter(adapter);
        mListView.setOnItemClickListener(mItemClickListenter);

        mContext = this;
    }

    private void ShowMessage(CharSequence msg)
    {
        if (mToast == null)
            mToast = Toast.makeText(this, msg, Toast.LENGTH_LONG);
        mToast.setText(msg);
        mToast.show();
    }

    private OnItemClickListener mItemClickListenter = new OnItemClickListener()
    {
        public void onItemClick(AdapterView<?> parent, View view, int position, long id)
        {
            DmService selfReg = DmService.getInstance();
            switch (position)
            {
                case 0:
                    // display current software version info
                    ShowMessage(selfReg.getSoftwareVersion());
                    break;

                case 1:
                    // search avalable software update package and download it
                    if (selfReg.isSelfRegOk())
                    {
                    }
                    else
                    {
                        ShowMessage(mContext.getString(R.string.havent_selfregist_tip));
                    }
                    break;

                case 2:
                    // intall software update package immediatly
                    ShowMessage(mContext.getString(R.string.no_update_package_tip));
                    break;

                default:
                    break;
            }
        }
    };

}
