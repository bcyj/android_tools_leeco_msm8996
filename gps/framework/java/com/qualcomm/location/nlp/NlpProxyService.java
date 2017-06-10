/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.

  Not a Contribution, Apache license notifications and
  license are retained for attribution purposes only.
=============================================================================*/

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

package com.qualcomm.location.nlp;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

public class NlpProxyService extends Service {
    private NlpProxyProvider mProvider;

    @Override
    public IBinder onBind(Intent intent) {
        if (mProvider == null) {
            mProvider = new NlpProxyProvider(this);
        }
        return mProvider.getBinder();
    }

    @Override
    public boolean onUnbind(Intent intent) {
        // make sure to stop performing work
        if (mProvider != null) {
            mProvider.cleanUp();
        }
        mProvider = null;
        return false;
    }

    @Override
    public void onDestroy() {
        // make sure to stop performing work
        if (mProvider != null) {
            mProvider.cleanUp();
        }
        mProvider = null;
    }
}
