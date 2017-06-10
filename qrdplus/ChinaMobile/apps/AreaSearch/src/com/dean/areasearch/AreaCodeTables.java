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

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.InputStream;

import android.content.Context;
import android.net.Uri;
import android.provider.BaseColumns;


public final class AreaCodeTables
{
    public static final String AUTHORITY = "customareasearch";

    // This class cannot be instantiated
    private AreaCodeTables() {}

    static void InitDB(Context context) {
        final String[] filenames = { "acnumber.db", "version.txt" };
        for (int i = 0; i < filenames.length; i++) {
            String filename = filenames[i];
            FileInputStream fis = null;
            boolean bFound = true;
            try {
                fis = context.openFileInput(filename);
                fis.close();
                fis = null;
            } catch (FileNotFoundException e) {
                bFound = false;
            } catch (Exception ex) {
            }
            if (fis != null) {
                try {
                    fis.close();
                } catch (Exception ex) {
                    ex.printStackTrace();
                }
            }
            if (!bFound) {
                FileOutputStream fos = null;
                InputStream is = null;
                try {
                    fos = context.openFileOutput(filename, Context.MODE_PRIVATE);
                    is = context.getAssets().open(filename);
                    byte[] buff = new byte[1024];
                    int count = 0;
                    while ((count = is.read(buff)) != -1) {
                        fos.write(buff, 0, count);
                        fos.flush();
                    }
                    fos.close();
                    fos = null;
                    is.close();
                    is = null;
                } catch(Exception e) {
                    e.printStackTrace();
                }
                if (fos != null) {
                    try {
                        fos.close();
                    } catch(Exception ex) {
                    }
                }
                if (is != null) {
                    try {
                        is.close();
                    } catch(Exception ex) {
                    }
                }
            }
        }
    }

    /**
     * Custom code area table
     */
    public static final class AreaCodeTable implements BaseColumns
    {
        // This class cannot be instantiated
        private AreaCodeTable() {}

        /**
         * The content:// style URL for this table
         */
        public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY + "/areacodes");
        public static final Uri NATIVE_URI = Uri.parse("content://nativeareasearch");
        public static final Uri NATIVE_INTER_URI = Uri.parse("content://nativeareasearch/international");
        public static final Uri NATIVE_DOMES_URI = Uri.parse("content://nativeareasearch/area_info");
        /**
         * 1 area code
         * <P>Type: TEXT</P>
         */
        public static final String CODE = "code";

        /**
         * 2 city name
         * <P>Type: TEXT</P>
         */
        public static final String CITY_NAME = "cityname";



        /**
         * 3 modified timestamp
         * <P>Type: INTEGER (long from System.curentTimeMillis())</P>
         */
        public static final String MODIFIED_DATE = "modified";

        /**
         * The default sort order for this table
         */
        public static final String DEFAULT_SORT_ORDER = "modified DESC";
    }

}
