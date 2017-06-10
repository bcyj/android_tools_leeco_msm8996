/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 */
 /*
 * Copyright (C) 2007 The Android Open Source Project
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

package com.qti.digitalpensdkdemos;

import android.app.ListActivity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import java.text.Collator;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.qti.snapdragon.sdk.digitalpen.PenEnabledChecker;

public class DigitalPenSDKDemosActivity extends ListActivity {

    private final Map<String, String> mDescMap = new HashMap<String, String>();

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        createDemosDescriptionsMap();

        setListAdapter(new ArrayAdapter<Map<String, Object>>(this,
                android.R.layout.simple_list_item_2, android.R.id.text1, getData()) {
            @Override
            public View getView(int position, View convertView, ViewGroup parent) {
                View view = super.getView(position, convertView, parent);
                TextView text1 = (TextView)view.findViewById(android.R.id.text1);
                TextView text2 = (TextView)view.findViewById(android.R.id.text2);

                String title = (String)getItem(position).get("title");
                String desc = mDescMap.get(title);

                text1.setText(title);
                text2.setText(desc);

                return view;
            }
        });
    }

    private void createDemosDescriptionsMap() {
        mDescMap.put(getString(R.string.activity_extended_offscreen_name),
                "Demonstrates how the extended offscreen touch and hover are working");
        mDescMap.put(getString(R.string.activity_background_side_channel_name),
                "Demonstrates how to use the background side-channel while screen is disabled");
        mDescMap.put(getString(R.string.activity_finger_stylus_name),
                "Demonstrates the difference between the stylus and finger tool types");
        mDescMap.put(getString(R.string.activity_tilt_name),
                "Demonstrates the tilt feature on the Digital Pen");
        mDescMap.put(getString(R.string.activity_hovering_side_button_name),
                "Demonstrates the configurable hovering feature and the SDP's side-button");
        mDescMap.put(getString(R.string.activity_eraser_name),
                "Demonstrates the eraser and the eraser-bypass features");
    }

    protected List<Map<String, Object>> getData() {
        List<Map<String, Object>> myData = new ArrayList<Map<String, Object>>();

        Intent mainIntent = new Intent(Intent.ACTION_MAIN, null);
        mainIntent.addCategory("com.qti.digitalpensdkdemos.category.SAMPLE_CODE");

        PackageManager pm = getPackageManager();
        List<ResolveInfo> list = pm.queryIntentActivities(mainIntent, 0);

        if (null == list)
            return myData;

        for (int i = 0; i < list.size(); i++) {
            ResolveInfo info = list.get(i);
            CharSequence labelSeq = info.loadLabel(pm);
            String label = labelSeq != null ? labelSeq.toString() : info.activityInfo.name;

            addItem(myData,
                    label,
                    activityIntent(info.activityInfo.applicationInfo.packageName,
                            info.activityInfo.name));
        }

        Collections.sort(myData, sDisplayNameComparator);

        return myData;
    }

    protected Intent activityIntent(String pkg, String componentName) {
        Intent result = new Intent();
        result.setClassName(pkg, componentName);
        return result;
    }

    private final static Comparator<Map<String, Object>> sDisplayNameComparator = new Comparator<Map<String, Object>>() {
        private final Collator collator = Collator.getInstance();

        @Override
        public int compare(Map<String, Object> map1, Map<String, Object> map2) {
            return collator.compare(map1.get("title"), map2.get("title"));
        }
    };

    protected void addItem(List<Map<String, Object>> data, String name, Intent intent) {
        Map<String, Object> temp = new HashMap<String, Object>();
        temp.put("title", name);
        temp.put("intent", intent);
        data.add(temp);
    }

    @Override
    @SuppressWarnings("unchecked")
    protected void onListItemClick(ListView l, View v, int position, long id) {
        Map<String, Object> map = (Map<String, Object>)l.getItemAtPosition(position);

        Intent intent = (Intent)map.get("intent");
        startActivity(intent);
    }

    protected void onResume() {
        super.onResume();
        PenEnabledChecker.checkEnabledAndLaunchSettings(this);
    }
}
