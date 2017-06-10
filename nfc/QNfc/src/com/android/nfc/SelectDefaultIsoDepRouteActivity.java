/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2012 The Android Open Source Project
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

package com.android.nfc;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.os.Bundle;
import android.content.Context;
import com.android.nfc.R;
import android.util.Log;
import java.util.ArrayList;


public class SelectDefaultIsoDepRouteActivity extends Activity {
    private static final String TAG = "nfc:SelectDefaultIsoDepRouteActivity";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final Context appContext = getApplicationContext();
        AlertDialog.Builder builder = new AlertDialog.Builder(this, AlertDialog.THEME_HOLO_DARK);
        Intent launchIntent = getIntent();
        String currentRoute = launchIntent.getStringExtra("com.android.nfc_extras.extra.SE_NAME");
        String seListWithComma = launchIntent.getStringExtra("com.android.nfc_extras.extra.SE_NAME_LIST");

        if (currentRoute == null) {
            Log.d(TAG, "current default route is not valid");
            currentRoute = new String("TBD");
        }
        if (seListWithComma == null) {
            Log.d(TAG, "no secure element found");
            SelectDefaultIsoDepRouteActivity.this.finish();
            return;
        }
        String[] seList = seListWithComma.split(",");
        if ((seList == null)||(seList.length == 0)) {
            Log.d(TAG, "no secure element found");
            SelectDefaultIsoDepRouteActivity.this.finish();
            return;
        }

        final String[] routeList = new String[seList.length];
        int currentRouteIndex = -1;
        for (int i = 0; (seList != null)&&(i < seList.length); i++) {
            routeList[i] = seList[i];
            if (seList[i].equals(currentRoute))
                currentRouteIndex = i;
        }

        final ArrayList<String> selectedRoute = new ArrayList<String>();
        if (currentRouteIndex != -1) {
            selectedRoute.add(routeList[currentRouteIndex]);
        }

        builder.setTitle("Select Default ISO-DEP route")
               .setCancelable(true)
               .setSingleChoiceItems(routeList, currentRouteIndex, new DialogInterface.OnClickListener() {
                   public void onClick(DialogInterface dialog, int id) {
                       selectedRoute.clear();
                       selectedRoute.add(routeList[id]);
                   }
               })
               .setPositiveButton("OK",
                       new DialogInterface.OnClickListener() {
                   public void onClick(DialogInterface dialog, int id) {
                       if (selectedRoute.size() > 0 ) {
                           Intent intent = new Intent("com.android.nfc.action.SET_DEFAULT_ISO_DEP_ROUTE");
                           intent.putExtra("com.android.nfc_extras.extra.SE_NAME", selectedRoute.get(0));
                           appContext.sendBroadcast(intent);
                       }
                       SelectDefaultIsoDepRouteActivity.this.finish();
                   }
               })
               .setNegativeButton("Cancel",
                       new DialogInterface.OnClickListener() {
                   public void onClick(DialogInterface dialog, int id) {
                       Log.d(TAG, "Cancelled");
                       SelectDefaultIsoDepRouteActivity.this.finish();
                   }
               })
               ;
        AlertDialog alert = builder.create();
        alert.show();
    }
}

