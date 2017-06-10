/******************************************************************************
 * @file    CommandHandler.java
 * ---------------------------------------------------------------------------
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 ******************************************************************************/
package com.qti.antitheftdemo;

import android.app.admin.DevicePolicyManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.res.Resources;
import android.telephony.SmsManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import java.util.Random;

public class CommandHandler {
    private final static Object lock = new Object();
    // Expected message text
    private final static String MSG_FOUND = "#Found";
    private final static String MSG_LOCK = "#Lock";
    private final static String MSG_WIPE = "#Wipe";

    // Command Codes
    private final static int COMMAND_INVALID = 0;
    private final static int COMMAND_FOUND = 1;
    private final static int COMMAND_LOCK = 2;
    private final static int COMMAND_WIPE = 3;

    private static void handleCommand(Context context, String senderNumber,
            int command) {
        synchronized (lock) {
            String message;
            String pin;
            Resources res = context.getResources();
            if (Debug.DEBUG)
                Log.d(Debug.TAG, "Got command: " + command);
            if (context == null) {
                Log.e(Debug.TAG,
                        "Called with empty context. Returning from here!");
                return;
            }
            DevicePolicyManager devicePolicyManager = (DevicePolicyManager) context
                .getSystemService(Context.DEVICE_POLICY_SERVICE);
            ComponentName deviceAdmin =
                new ComponentName(context, DeviceManagerAdminReceiver.class);

            if (!isDeviceAdminActive(devicePolicyManager, deviceAdmin)) {
                message = res.getString(R.string.message_dev_admin_not_enabled);
                sendMessage(senderNumber, message);
                return;
            }

            switch (command) {
                case COMMAND_FOUND:
                    devicePolicyManager.resetPassword("", 0);
                    message = res.getString(R.string.message_dev_unlock);
                    sendMessage(senderNumber, message);
                    break;
                case COMMAND_LOCK:
                    int min = 99999;
                    int max = 99999999;

                    Random r = new Random();
                    int iPin = r.nextInt(max - min + 1) + min;

                    pin = "" + iPin;
                    devicePolicyManager.lockNow();
                    devicePolicyManager.resetPassword(pin, 0);
                    message = String.format(res.getString(R.string.message_new_pin), pin);
                    sendMessage(senderNumber, message);
                    break;
                case COMMAND_WIPE:
                    message = res.getString(R.string.message_wipe);
                    sendMessage(senderNumber, message);
                    devicePolicyManager.wipeData(0);
                    break;
                default:
                    break;
            }
        }
    }

    private static boolean isDeviceAdminActive(DevicePolicyManager devicePolicyManager,
            ComponentName deviceAdmin) {
        if(devicePolicyManager != null && devicePolicyManager.isAdminActive(deviceAdmin)) {
            return true;
        }
        return false;
    }


    public static boolean checkMessage(Context context, final String senderNumber,
            final String message) {
        if (context == null) {
            Log.e(Debug.TAG, "Context is null. Returning!");
            return false;
        }
        if (senderNumber == null) {
            Log.e(Debug.TAG, "Sender number is null. Returning!");
            return false;
        }

        String friendNumber = SettingsEditor.getFriendNumber(context);
        String confirmationCode = SettingsEditor.getConfirmationCode(context);
        if (friendNumber == null) {
            Log.e(Debug.TAG, "Friend Number not set. Returning!");
            return false;
        }
        if (confirmationCode == null) {
            Log.e(Debug.TAG, "Confirmation code not set. Returning!");
            return false;
        }
        if (senderNumber.contains(friendNumber) == false) {
            if (Debug.DEBUG)
                Log.d(Debug.TAG, "Ignoring text from unknown number.");
            return false;
        }

        if (Debug.DEBUG)
            Log.d(Debug.TAG, "\tSender Number: " + senderNumber
                    + "\n\tmessage: " + message);

        int command = COMMAND_INVALID;
        if (MSG_FOUND.equalsIgnoreCase(message))
            command = COMMAND_FOUND;
        else if (MSG_LOCK.equalsIgnoreCase(message))
            command = COMMAND_LOCK;
        else if (MSG_WIPE.equalsIgnoreCase(message))
            command = COMMAND_WIPE;

        if (command != COMMAND_INVALID) {
            handleCommand(context, senderNumber, command);
            return true;
        }

        return false;
    }

    private static void sendMessage(String senderNumber, String message) {
        SmsManager.getDefault().sendTextMessage(senderNumber, null, message,
                null, null);
    }

    public static void checkSimChange(Context context) {
        if (context == null) {
            Log.e(Debug.TAG, "Context is null. Returning!");
            return;
        }
        synchronized (lock) {
            TelephonyManager telephonyMgr = (TelephonyManager) context
                .getSystemService(Context.TELEPHONY_SERVICE);
            String newSubscriberId = telephonyMgr.getSubscriberId();

            if (newSubscriberId == null)
                return;

            String oldSubscriberId = SettingsEditor.getSubscriberId(context);
            if (newSubscriberId.equals(oldSubscriberId)) {
                if (Debug.DEBUG)
                    Log.d(Debug.TAG, "No change in sim.");
                return;
            }

            if (Debug.DEBUG)
                Log.d(Debug.TAG, "New Sim detected.");

            String newNumber = telephonyMgr.getLine1Number();
            if (newNumber == null) {
                Log.e(Debug.TAG, "Cann't read phone number! waiting...");
                try {
                    //wait for 10 seconds.
                    Thread.sleep(10000);
                    Log.e(Debug.TAG, "Done waiting.");
                } catch (InterruptedException e ){
                    Log.e(Debug.TAG, "Sleep is interrupted.");
                }
            }

            Resources res = context.getResources();
            String message = res.getString(R.string.message_new_number);
            String friendNumber = SettingsEditor.getFriendNumber(context);
            if (friendNumber != null)
                sendMessage(friendNumber, message);

            SettingsEditor.setSubscriberId(context, newSubscriberId);
        }
    }
}
