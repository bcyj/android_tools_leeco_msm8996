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

package com.android.backup;

import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

public class AppInfo implements Parcelable {
    public String appName = "";
    public String packageName = "";
    public String versionName = "";
    public String sourcedir = "";
    public String publicsourcedir = "";
    public int versionCode = 0;

    // public Drawable appIcon=null;
    public void print()
    {
        Log.d("app", "Name:" + appName + " Package:" + packageName);
        Log.d("app", "Name:" + appName + " versionName:" + versionName);
        Log.d("app", "Name:" + appName + " versionCode:" + versionCode);
        Log.d("app", "Name:" + appName + " sourcedir:" + sourcedir);
        Log.d("app", "Name:" + appName + " publicsourcedir:" + publicsourcedir);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(appName);
        dest.writeString(packageName);
        dest.writeString(versionName);
        dest.writeString(sourcedir);
        dest.writeString(publicsourcedir);
        dest.writeInt(versionCode);
    }

    public static final Parcelable.Creator<AppInfo> CREATOR = new Creator<AppInfo>() {

        @Override
        public AppInfo[] newArray(int size) {
            return new AppInfo[size];
        }

        @Override
        public AppInfo createFromParcel(Parcel source) {
            AppInfo appInfo = new AppInfo();
            appInfo.appName = source.readString();
            appInfo.packageName = source.readString();
            appInfo.versionName = source.readString();
            appInfo.sourcedir = source.readString();
            appInfo.publicsourcedir = source.readString();
            appInfo.versionCode = source.readInt();
            return appInfo;
        }
    };
}
