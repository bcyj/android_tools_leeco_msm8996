/*===========================================================================
                           ServiceNotifications.java

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.

Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.snapdragon.digitalpen;

import android.R;
import android.app.Notification;
import android.app.Notification.Builder;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;

import com.qti.snapdragon.digitalpen.util.DigitalPenEvent;

public class ServiceNotifications {
    private enum PenNotification {
        ACTIVE(0x10, "Digital pen is active", R.drawable.digital_pen_active),
        BLOCKED_2_MICS(0x20, "2 digital pen microphones are blocked",
                R.drawable.digital_pen_blocked_mic_yellow),
        BLOCKED_3_OR_MORE_MICS(0x20, "3 or more digital pen microphones are blocked",
                R.drawable.digital_pen_blocked_mic_red),
        LOW_BATTERY(0x30, "Digital pen battery is low", R.drawable.digital_pen_low_battery),
        POSITIONING_PROBLEM(
                0x40,
                "There is a problem determining the location of the pen. Environmental issues or blocked microphones might cause this",
                R.drawable.digital_pen_positioning_problem),
        BACKGROUND_LISTENER(0x50, "Background application is recording off-screen pen strokes",
                R.drawable.digital_pen_background_side_channel);

        final int id; // arbitrary number; blocked mics share ID, and so does
                      // bad writing scenarios.
        final String title;
        final int icon;
        Builder builder;

        private PenNotification(int id, String title, int icon) {
            this.id = id;
            this.title = title;
            this.icon = icon;
        }
    }

    private NotificationManager mNM;
    private boolean mIsSpur = false;
    private boolean mIsBadScenario = false;
    private Context mContext;
    private boolean mPenEnabled;
    private int mCurrentPowerState = DigitalPenEvent.POWER_STATE_OFF;

    public ServiceNotifications(Context context) {
        mContext = context;
        mNM = (NotificationManager) mContext.getSystemService(Context.NOTIFICATION_SERVICE);
        // The PendingIntent to launch our activity if the user selects this
        // notification
        // TODO: OEM can add intent for their own Digital Pen Settings app
        Intent intent = new Intent(mContext, DigitalPenService.class);
        PendingIntent contentIntent = PendingIntent.getActivity(mContext, 0,
                intent, 0);
        for (PenNotification pn : PenNotification.values()) {

            pn.builder = new Notification.Builder(mContext)
                    .setSmallIcon(pn.icon)
                    .setContentTitle(pn.title)
                    .setContentIntent(contentIntent);
        }
    }

    /**
     * Show a notification while this service is running.
     */
    protected void showNotification(ServiceNotifications.PenNotification penNotification) {
        long oldId = Binder.clearCallingIdentity();
        doNotify(penNotification.id, penNotification.builder.build());
        Binder.restoreCallingIdentity(oldId);
    }

    protected void doNotify(int id, Notification notification) {
        mNM.notify(id, notification);
    }

    /**
     * Called when pen is enabled to notify user
     */
    public void notifyPenEnabled() {
        mPenEnabled = true;
        showNotification(PenNotification.ACTIVE);
    }

    /**
     * Called when pen is disabled to notify user
     */
    public void notifyPenDisabled()
    {
        mPenEnabled = false;
        long oldId = Binder.clearCallingIdentity();
        cancelAllNotifications();
        Binder.restoreCallingIdentity(oldId);
        removeBatteryLevelInformation();
    }

    private void cancelAllNotifications() {
        // Note: NotificationManager.cancelAll has been experimentally shown to
        // be unsynchronized with sending notifications, meaning it will miss
        // canceling "in-flight" notifications.

        // This specifically cancels every notification that can be sent by ID.
        for (PenNotification notification : PenNotification.values()) {
            mNM.cancel(notification.id);
        }
    }

    /**
     * Remove a notification.
     */
    private void removeNotification(ServiceNotifications.PenNotification notification)
    {
        long oldId = Binder.clearCallingIdentity();
        mNM.cancel(notification.id);
        Binder.restoreCallingIdentity(oldId);
    }

    public void sendEvent(int eventType, int[] params) {
        // Show or remove blocked microphone notifications
        if (DigitalPenEvent.TYPE_MIC_BLOCKED == eventType) {
            final int numMicsBlocked = params[0];
            switch (numMicsBlocked) {
                case 0:
                case 1:
                    // Since BLOCKED_2_MICS and BLOCKED_3_OR_MORE_MICS have the
                    // same notification id, removing the BLOCKED_2_MICS
                    // notification also removes the BLOCKED_3_OR_MORE_MICS
                    // notification
                    removeNotification(PenNotification.BLOCKED_2_MICS);

                    break;
                case 2:
                    showNotification(PenNotification.BLOCKED_2_MICS);
                    break;
                default:
                    showNotification(PenNotification.BLOCKED_3_OR_MORE_MICS);
                    break;
            }
        }

        // Show or remove low pen battery notification
        if (DigitalPenEvent.TYPE_PEN_BATTERY_STATE == eventType) {
            switch (params[0]) {
                case DigitalPenEvent.BATTERY_OK:
                    removeNotification(PenNotification.LOW_BATTERY);
                    break;
                case DigitalPenEvent.BATTERY_LOW:
                    showNotification(PenNotification.LOW_BATTERY);
                    break;
            }
            int batteryLevel = params[1];
            if (batteryLevel == DigitalPenEvent.BATTERY_LEVEL_UNAVAILABLE
                    || mCurrentPowerState != DigitalPenEvent.POWER_STATE_ACTIVE) {
                removeBatteryLevelInformation();
            } else {
                updateBatteryLevelInformation(batteryLevel);
            }

        }

        // Remove battery level notifiation if not in ACTIVE power state
        if (DigitalPenEvent.TYPE_POWER_STATE_CHANGED == eventType) {
            mCurrentPowerState = params[0];
            if (params[0] != DigitalPenEvent.POWER_STATE_ACTIVE) {
                removeBatteryLevelInformation();
            }
        }

        // Show or remove spurs notification
        if (DigitalPenEvent.TYPE_SPUR_STATE == eventType) {
            switch (params[0]) {
                case DigitalPenEvent.SPURS_OK:
                    mIsSpur = false;
                    if (!mIsBadScenario) {
                        removeNotification(PenNotification.POSITIONING_PROBLEM);
                    }
                    break;
                case DigitalPenEvent.SPURS_EXIST:
                    mIsSpur = true;
                    showNotification(PenNotification.POSITIONING_PROBLEM);
                    break;
            }
        }

        // Show or remove bad writing scenarios notifications
        if (DigitalPenEvent.TYPE_BAD_SCENARIO == eventType) {
            final int scenario = params[0];
            switch (scenario) {
                case DigitalPenEvent.BAD_SCENARIO_OK:
                    mIsBadScenario = false;
                    if (!mIsSpur) {
                        removeNotification(PenNotification.POSITIONING_PROBLEM);
                    }
                    break;
                case DigitalPenEvent.BAD_SCENARIO_MULTILATERAION:
                case DigitalPenEvent.BAD_SCENARIO_BETWEEN_TWO_MICS:
                    mIsBadScenario = true;
                    showNotification(PenNotification.POSITIONING_PROBLEM);
                    break;
            }
        }
    }

    private void updateBatteryLevelInformation(int batteryLevel) {
        String batteryString = "Battery level: " + batteryLevel + "%";
        PenNotification.ACTIVE.builder.setContentText(batteryString);
        if (mPenEnabled) {
            showNotification(PenNotification.ACTIVE);
        }
    }

    private void removeBatteryLevelInformation() {
        PenNotification.ACTIVE.builder.setContentText(null);
        if (mPenEnabled) {
            showNotification(PenNotification.ACTIVE);
        }
    }

    public void backgroundListenerEnabled(boolean enabled) {
        if (enabled) {
            showNotification(PenNotification.BACKGROUND_LISTENER);
        } else {
            removeNotification(PenNotification.BACKGROUND_LISTENER);
        }
    }
}
