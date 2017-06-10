/*
 * Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
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

import static com.android.internal.telephony.TelephonyIntents.SECRET_CODE_ACTION;

import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.telephony.SmsMessage;
import android.os.Bundle;
import android.telephony.TelephonyManager;

import android.content.ServiceConnection;
import android.content.ComponentName;
import android.os.IBinder;
import android.provider.Settings;
import android.widget.Toast;
import android.app.Activity; // add 2012.12.28 for TS723G-365

public class DmReceiver extends BroadcastReceiver {

    protected static final String DM_TAG = "DM ==> ";
    private String TAG = DM_TAG + "DmReceiver: ";
    private static final String WAP_PUSH_RECEIVED_ACTION =
            "android.provider.Telephony.WAP_PUSH_RECEIVED";
    private static final String DATA_SMS_RECEIVED_ACTION =
            "android.intent.action.DATA_SMS_RECEIVED";
    private static final String WALLPAPER_CHANGED_ACTION =
            "android.intent.action.WALLPAPER_CHANGED";
    private static final String SMS_INIT_COMPLETE_ACTION =
            "android.intent.action.sms_init_complete";
    private static final String START_DM_DEBUG_MENU_ACTION =
            "com.android.dm.StartDmDebugMenu";

    private static final String DM_MIME_TYPE1 = "application/vnd.syncml.dm+wbxml";
    private static final String DM_MIME_TYPE2 = "application/vnd.syncml.dm+xml";
    private static final String DM_MIME_TYPE3 = "application/vnd.syncml.notification";

    private static final String DM_START = "3636";

    static final String DM_AUTOBOOT_SETTING = "dm_selfregist_autoboot";
    static final int DM_AUTOBOOT_SETTING_ENABLE = 1;
    private boolean bSettingEnable = false;

    @Override
    public void onReceive(Context context, Intent intent) {

        String action = intent.getAction();

        Log.d(TAG, "onReceive, action is " + action);

        if (action.equals(Intent.ACTION_BOOT_COMPLETED)) {
            Log.d(TAG, "onReceive, ACTION_BOOT_COMPLETED");
            int enable = Settings.Global.getInt(context.getContentResolver(), DM_AUTOBOOT_SETTING,
                    DM_AUTOBOOT_SETTING_ENABLE);
            if (enable == DM_AUTOBOOT_SETTING_ENABLE) {
                bSettingEnable = true;
                Intent selfRegService = new Intent("com.android.dm.SelfReg");
                selfRegService.setComponent(new ComponentName("com.android.dm",
                        "com.android.dm.DmService"));
                context.startService(selfRegService);
            }
        }
        else if (SMS_INIT_COMPLETE_ACTION.equals(action))
        {
            Log.d(TAG, "onReceive, SMS_INIT_COMPLETE_ACTION");
            int enable = Settings.Global.getInt(context.getContentResolver(), DM_AUTOBOOT_SETTING,
                    DM_AUTOBOOT_SETTING_ENABLE);
            if (enable == DM_AUTOBOOT_SETTING_ENABLE) {
                Intent selfRegService = new Intent("com.android.dm.SelfReg");
                selfRegService.putExtra("smsinit", true);
                selfRegService.setComponent(new ComponentName("com.android.dm",
                        "com.android.dm.DmService"));
                context.startService(selfRegService);
            }
        }
        else if (DATA_SMS_RECEIVED_ACTION.equals(action))
        {
            Log.d(TAG, "onReceive, DATA_SMS_RECEIVED_ACTION");

            processDataSms(context, intent);
        }
        else if (WAP_PUSH_RECEIVED_ACTION.equals(action))
        {
            Log.d(TAG, "onReceive, WAP_PUSH_RECEIVED_ACTION");
            int enable = Settings.Global.getInt(context.getContentResolver(), DM_AUTOBOOT_SETTING,
                    DM_AUTOBOOT_SETTING_ENABLE);
            if (enable == DM_AUTOBOOT_SETTING_ENABLE || bSettingEnable) {
                processWapPush(context, intent);
            }
        }
        else if (START_DM_DEBUG_MENU_ACTION.equals(action))
        {
            Log.d(TAG, "onReceive, START_DM_DEBUG_MENU_ACTION");

            Intent intent2 = new Intent();
            intent2.setClassName(context, "com.android.dm.DmDebugMenu");
            intent2.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(intent2);
        }
        /* add 2012.12.28 for TS723G-365 begin */
        else if (DmService.SMS_ACTION_SENT.equals(action))
        {
            Log.v(TAG, "onReceive, DmService.SMS_ACTION_SENT");
            processSmsSent(context, intent);
        }
        else if (DmService.SMS_ACTION_DELIVER.equals(action))
        {
            Log.v(TAG, "onReceive, DmService.SMS_ACTION_DELIVER");
            processSmsDeliver(context, intent);
        }
        else if (SECRET_CODE_ACTION.equals(intent.getAction())) {
            String host = intent.getData() != null ? intent.getData().getHost() : null;
            if (DM_START.equals(host)) {
                Intent i = new Intent();
                i.setClassName("com.android.dm", "com.android.dm.DmDebugMenu");
                i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                try {
                    context.startActivity(i);
                } catch (ActivityNotFoundException e) {
                    Log.i(TAG, "no activity to handle handleDmCode");
                }
            }
        }
        /* add 2012.12.28 for TS723G-365 end */
        else
        {
            Log.d(TAG, "onReceive, DM need not process!");
        }
    }

    // judge if the address is dm self registe sms address
    private boolean isDmSmsAddress(Context context, String addr)
    {
        boolean result = false;
        String selfRegSmsAddr = DmService.getInstance().getSmsAddr();

        if (addr.equals(selfRegSmsAddr))
        {
            result = true;
        }
        Log.d(TAG, "isDmSmsAddress : return " + result);
        return result;
    }

    // parse reply message content
    private boolean parseReplyMsgContent(Context context, String msgBody)
    {
        if (!DmService.isServiceStart()) return false;
        boolean result = true; // is processed message
        String imsi = DmService.getInstance().getSubscriberId();
        String imei = DmService.getInstance().getImei();
        String okReply = "IMEI:" + imei + "/1";
        String failReply = "IMEI:" + imei + "/0";
        DmService dmService = DmService.getInstance();

        Log.d(TAG, "parseReplyMsgContent: msgBody is " + msgBody);
        Log.d(TAG, "parseReplyMsgContent: okReply is " + okReply);
        Log.d(TAG, "parseReplyMsgContent: failReply is " + failReply);

        if (msgBody.equals(okReply))
        {
            Log.d(TAG, "parseReplyMsgContent: self registe successfully!");

            // save current card imsi to file
            dmService.saveImsi(context, imsi);
            // set self registe state to success
            dmService.setSelfRegState(context, true);
            dmService.stopListeningServiceState();
            if (dmService.isDebugMode())
            {
                ShowMessage(context, context.getString(R.string.self_registe_ok_tip));
            }
            // DmService.getContext().stopService(new
            // Intent("com.android.dm.SelfReg"));
        }
        else if (msgBody.equals(failReply))
        {
            // self registe fail, need not save imsi
            Log.d(TAG, "parseReplyMsgContent: self registe fail!");
            // set self registe state to fail
            dmService.setSelfRegState(context, false);
            dmService.stopListeningServiceState();
            if (dmService.isDebugMode())
            {
                ShowMessage(context, context.getString(R.string.self_registe_fail_tip));
            }
            // DmService.getContext().stopService(new
            // Intent("com.android.dm.SelfReg"));
        }
        else
        {
            Log.d(TAG, "parseReplyMsgContent: not for dm!");
            result = false;
        }
        return result;
    }

    // process data message content
    private boolean processDataSms(Context context, Intent intent)
    {
        if (!DmService.isServiceStart()) return false;
        Bundle bundle = intent.getExtras();
        SmsMessage[] msgs = null;
        String str = "";
        boolean result = false;

        if (bundle != null)
        {
            Log.d(TAG, "processDataSms: bundle is not null");
            // ---retrieve the SMS message received---
            Object[] pdus = (Object[]) bundle.get("pdus");
            msgs = new SmsMessage[pdus.length];
            for (int i = 0; i < msgs.length; i++) {
                msgs[i] = SmsMessage.createFromPdu((byte[]) pdus[i]);

                str = msgs[i].getOriginatingAddress();
                Log.d(TAG, "getOriginatingAddress : " + str);
                if (isDmSmsAddress(context, str))
                {
                    str = msgs[i].getMessageBody();
                    Log.d(TAG, "getMessageBody : " + str);
                    if ((str == null) || str.equals(""))
                    {
                        Log.d(TAG, "processDataSms: message body is null");
                        break;
                    }
                    if (parseReplyMsgContent(context, str))
                    {
                        // if have processed dm self registe reply message,
                        // break this cycle
                        Log.d(TAG, "processDataSms: have processed dm reply message");
                        result = true;
                        break;
                    }
                }
            }
        }
        else
        {
            Log.d(TAG, "processDataSms: bundle is null");
        }

        return result;
    }

    // process wap push message
    private boolean processWapPush(Context context, Intent intent)
    {
        if (!DmService.isServiceStart()) return false;
        boolean result = false;
        String type = intent.getType(); // data type
        String addr = intent.getStringExtra("address");
        String dmSmsAddr = DmService.getInstance().getSmsAddr();

        Log.d(TAG, "processWapPush: enter!");
        Log.d(TAG, "processWapPush: data type ==> " + type);
        Log.d(TAG, "processWapPush: from ==> " + addr);

        if ((DM_MIME_TYPE1.equals(type)) || (DM_MIME_TYPE2.equals(type)) || (DM_MIME_TYPE3
                .equals(type)))
        {
            Log.d(TAG, "processWapPush: is for dm");

            if (DmService.getInstance().isDebugMode())
            {
                ShowMessage(context, context.getString(R.string.receive_dm_push_message_tip));
            }

            byte[] header = intent.getByteArrayExtra("header");
            byte[] data = intent.getByteArrayExtra("data");
            int i;
            for (i = 0; i < header.length; i++)
            {
                Log.d(TAG, "header[" + i + "] = " + header[i]);
            }

            Log.d(TAG, "data.length = " + data.length);
            for (i = 0; i < data.length; i++)
            {
                Log.d(TAG, "data[" + i + "] = " + data[i]);
            }
            Intent vdmIntent = new Intent("com.android.dm.NIA");
            Bundle extras = new Bundle();
            extras.putByteArray("msg_body", data);
            extras.putString("msg_org", dmSmsAddr);
            vdmIntent.putExtras(extras);
            vdmIntent.setComponent(new ComponentName("com.android.dm",
                    "com.android.dm.DmService"));
            context.startService(vdmIntent);
        }

        return result;
    }

    /* add 2012.12.28 for TS723G-365 begin */
    private boolean processSmsSent(Context context, Intent intent) {
        if (!DmService.isServiceStart()) return false;
        boolean result = false;
        int mResultCode;
        DmService dmService = DmService.getInstance();
        String imsi = DmService.getInstance().getSubscriberId();

        mResultCode = getResultCode();
        Log.v(TAG, "processSmsSent mResultCode:" + mResultCode);
        if (mResultCode == Activity.RESULT_OK) {
            Log.d(TAG, "processSmsSent: send message successfully");
            /*
             * if (dmService.isDebugMode()) { ShowMessage(context,
             * "Send message successful!"); }
             */
            // add 2013.2.22 begin for limit self registe message max send
            // counts
            if (DmService.getInstance().getSmsDeliverReportSwitch())
            {
                Log.d(TAG,
                        "processSmsSent: smsdeliverreportswitch is open, need listen to deliver report");
                int count = dmService.getMessageSendCount(context);
                if (count > DmService.MAX_SEND_COUNT)
                {
                    Log.d(TAG,
                            "sendMsgBody: send more than 10 times, if message send success, consider self registe success!");
                    Log.d(TAG, "processSmsSent: self registe successfully!");

                    // save current card imsi to file
                    dmService.saveImsi(context, imsi);
                    // set self registe state to success
                    dmService.setSelfRegState(context, true);
                    dmService.stopListeningServiceState();
                    if (dmService.isDebugMode()) {
                        ShowMessage(context, context.getString(R.string.self_registe_ok_tip));
                    }
                }
            } else {

                // save current card imsi to file
                Log.d(TAG,
                        "processSmsSent: smsdeliverreportswitch is close, need not listen to deliver report");
                dmService.saveImsi(context, imsi);
                // set self registe state to success
                dmService.setSelfRegState(context, true);
                dmService.stopListeningServiceState();
                if (dmService.isDebugMode()) {
                    ShowMessage(context, context.getString(R.string.self_registe_ok_tip));
                }
            }

            return true;
        } else {
            Log.d(TAG, "processSmsSent: self registe fail!");
            // set self registe state to fail
            dmService.setSelfRegState(context, false);
            dmService.stopListeningServiceState();
            if (dmService.isDebugMode()) {
                ShowMessage(context, context.getString(R.string.self_registe_fail_tip));
            }

            return false;
        }
    }

    private boolean processSmsDeliver(Context context, Intent intent) {
        if (!DmService.isServiceStart()) return false;
        if (!DmService.getInstance().getSmsDeliverReportSwitch())
        {
            return false;
        }
        if (DmService.getInstance().getMessageSendCount(context) > DmService.MAX_SEND_COUNT)
        {
            Log.d(TAG,
                    "sendMsgBody: send more than 10 times, there is no need to wait SMS deliver!!!!!");
            return false;
        }
        boolean result = false;
        DmService dmService = DmService.getInstance();
        String imsi = DmService.getInstance().getSubscriberId();

        // enter this function, meens server have received selfregiste message
        Log.d(TAG, "processSmsDeliver: self registe successfully!");

        // save current card imsi to file
        dmService.saveImsi(context, imsi);
        // set self registe state to success
        dmService.setSelfRegState(context, true);
        dmService.stopListeningServiceState();
        if (dmService.isDebugMode()) {
            ShowMessage(context, context.getString(R.string.receive_deliver_report_tip));
        }

        return true;
    }

    private Toast mToast;

    private void ShowMessage(Context context, CharSequence msg)
    {
        if (mToast == null)
            mToast = Toast.makeText(context, msg, Toast.LENGTH_LONG);
        mToast.setText(msg);
        mToast.show();
    }

}
