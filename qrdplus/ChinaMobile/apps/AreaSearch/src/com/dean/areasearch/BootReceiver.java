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
package com.dean.areasearch;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
public class BootReceiver extends BroadcastReceiver {
    static SharedPreferences pref;
    Context ctx;
    //set an alarm to check the database server
    //need to add first boot value
    @Override
    public void onReceive(Context context, Intent intent) {
        // TODO Auto-generated method stub
        ctx=context;
        pref = context.getSharedPreferences("pref", context.MODE_PRIVATE);
        if (!pref.contains("FIRST_RUN")) {
            SharedPreferences.Editor editor = pref.edit();
            editor.putInt("currentversion", 1);
            editor.putInt("selected_id1", 0);
            editor.putInt("selected_id2", 0);
            editor.apply();
            (new Thread(new CopyDatabase())).start();
        }
    }
    public class CopyDatabase implements Runnable {

        @Override
        public void run() {
            // TODO Auto-generated method stub
            AreaCodeTables.InitDB(ctx);
            if (!pref.contains("FIRST_RUN")) {
                SharedPreferences.Editor editor = pref.edit();
                editor.putBoolean("FIRST_RUN", false);
                editor.apply();
            }
        }
    }
}
