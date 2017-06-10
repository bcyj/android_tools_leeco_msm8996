/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
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

package com.wdstechnology.android.kryten;

import java.util.Random;
import com.wdstechnology.android.kryten.sax.WapProvisioningDocContentHandler;
import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;

public class ProvisioningNotification {

    private static int NOTIFICATION_ID = 1;

    public static void createNotification(Context context, String sec, String mac, byte[] document,
            String from) {

        String tickerText = context.getString(R.string.provisioning_received_ticker);
        String title = context.getString(R.string.provisioning_received_title);
        String message = context.getString(R.string.provisioning_received_message);

        NotificationManager manager = (NotificationManager) context
                .getSystemService(Context.NOTIFICATION_SERVICE);

        Notification notification = new Notification();
        notification.when = System.currentTimeMillis();
        notification.tickerText = tickerText;
        notification.icon = R.drawable.stat_notify_configuration;

        notification.setLatestEventInfo(context, title, message, ConfigurationMessageActivity
                .createPendingValidationActivity(context, sec, mac, document, from));
        notification.defaults |= Notification.DEFAULT_ALL;
        manager.notify(NOTIFICATION_ID, notification);

    }

    public static void clearNotification(Context context) {
        NotificationManager manager = (NotificationManager) context
                .getSystemService(Context.NOTIFICATION_SERVICE);
        manager.cancel(NOTIFICATION_ID);
    }

}
