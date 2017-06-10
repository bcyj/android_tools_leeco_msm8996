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

package com.android.dm;

import android.app.Activity;
import android.content.ComponentName;
import android.os.Bundle;
import android.view.View;
import android.widget.ListView;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;
import android.content.SharedPreferences;
import android.content.Context;
import android.util.Log;
import android.content.Intent;
import com.android.dm.vdmc.Vdmc;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.BroadcastReceiver;

public class DmDebugMenu extends Activity {
    private String TAG = DmReceiver.DM_TAG + "DmDebugMenu: ";
    private ListView mListView;
    private String[] mlistItem;
    private Toast mToast;
    private Context mContext;
    protected static final int ITEM_DEBUG_MODE = 0;
    protected static final int ITEM_CLEAN_SR_FLAG = 1;
    protected static final int ITEM_SEND_SR_MSG = 2;
    protected static final int ITEM_SWITCH_SERVER = 3;
    protected static final int ITEM_MANUFACTORY = 4;
    protected static final int ITEM_MODEL = 5;
    protected static final int ITEM_SW_VER = 6;
    protected static final int ITEM_IMEI = 7;
    protected static final int ITEM_APN = 8;
    protected static final int ITEM_SERVER_ADDR = 9;
    protected static final int ITEM_SMS_ADDR = 10;
    protected static final int ITEM_SMS_PORT = 11;
    // protected static final int ITEM_TRIGGER_NULL_SESSION = 12;
    protected static final int ITEM_SMSDELIVERREPORT_SWITCH = 12;
    protected static final int ITEM_START_SERVICE = 13;
    protected static final int ITEM_DM_STATE = 14;
    protected static final int ITEM_SELFREG_SWITCH = 15;
    protected static final int ITEM_SENDMESSAGE_COUNT = 16;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        mListView = (ListView) findViewById(R.id.app_list);

        // init list item content
        mlistItem = getResources().getStringArray(R.array.debug_list);

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, R.layout.text_view, mlistItem);
        mListView.setAdapter(adapter);
        mListView.setOnItemClickListener(mItemClickListenter);

        mContext = this;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    private void ShowMessage(CharSequence msg)
    {
        if (mToast == null)
            mToast = Toast.makeText(this, msg, Toast.LENGTH_LONG);
        mToast.setText(msg);
        mToast.show();
    }

    private OnItemClickListener mItemClickListenter = new OnItemClickListener()
    {
        public void onItemClick(AdapterView<?> parent, View view, int position, long id)
        {
            Intent intent;

            Log.d(TAG, "mItemClickListenter : position = " + position);
            switch (position)
            {
                case ITEM_DEBUG_MODE:
                    // Debug mode
                    if (DmService.getInstance().isDebugMode())
                    {
                        DmService.getInstance().setDebugMode(mContext, false);
                        ShowMessage(mContext.getString(R.string.debug_mode_closed_tip));
                    }
                    else
                    {
                        DmService.getInstance().setDebugMode(mContext, true);
                        ShowMessage(mContext.getString(R.string.debug_mode_opened_tip));
                    }

                    break;

                case ITEM_CLEAN_SR_FLAG:
                    // Clean selfregiste flag
                    DmService.getInstance().saveImsi(mContext, "");
                    DmService.getInstance().setSelfRegState(mContext, false);
                    ShowMessage(mContext.getString(R.string.clean_finished_tip));
                    break;

                case ITEM_SEND_SR_MSG:
                    // Send selfregiste message
                    if (DmService.getInstance().isDebugMode())
                    {
                        DmService.getInstance().sendSelfRegMsgForDebug();
                        ShowMessage(mContext.getString(R.string.selfregiste_message_send_tip));
                    }
                    else
                    {
                        ShowMessage(mContext.getString(R.string.open_debug_mode_tip));
                    }
                    break;

                case ITEM_SWITCH_SERVER:
                    // Switch server
                    boolean isReal = DmService.getInstance().isRealNetParam();
                    if (isReal)
                    {
                        DmService.getInstance().setRealNetParam(mContext, false, true);
                        ShowMessage(mContext.getString(R.string.switch_lab_param_tip));
                    }
                    else
                    {
                        DmService.getInstance().setRealNetParam(mContext, true, true);
                        ShowMessage(mContext.getString(R.string.switch_real_param_tip));
                    }
                    break;

                case ITEM_MANUFACTORY:
                    // Manufactory
                    intent = new Intent(mContext, DmEditItem.class);
                    intent.putExtra("EditType", mlistItem[ITEM_MANUFACTORY]);
                    intent.putExtra("EditContent", DmService.getInstance().getManufactory());
                    startActivity(intent);
                    break;

                case ITEM_MODEL:
                    // Model
                    intent = new Intent(mContext, DmEditItem.class);
                    intent.putExtra("EditType", mlistItem[ITEM_MODEL]);
                    intent.putExtra("EditContent", DmService.getInstance().getModel());
                    startActivity(intent);
                    break;

                case ITEM_SW_VER:
                    // Software version
                    intent = new Intent(mContext, DmEditItem.class);
                    intent.putExtra("EditType", mlistItem[ITEM_SW_VER]);
                    intent.putExtra("EditContent", DmService.getInstance().getSoftwareVersion());
                    startActivity(intent);
                    break;

                case ITEM_IMEI:
                    // IMEI
                    intent = new Intent(mContext, DmEditItem.class);
                    intent.putExtra("EditType", mlistItem[ITEM_IMEI]);
                    intent.putExtra("EditContent", DmService.getInstance().getImei());
                    startActivity(intent);
                    break;

                case ITEM_APN:
                    // APN
                    intent = new Intent(mContext, DmEditItem.class);
                    intent.putExtra("EditType", mlistItem[ITEM_APN]);
                    intent.putExtra("EditContent", DmService.getInstance().getAPN());
                    startActivity(intent);
                    break;

                case ITEM_SERVER_ADDR:
                    // Server address
                    intent = new Intent(mContext, DmEditItem.class);
                    intent.putExtra("EditType", mlistItem[ITEM_SERVER_ADDR]);
                    intent.putExtra("EditContent", DmService.getInstance().getServerAddr());
                    startActivity(intent);
                    break;

                case ITEM_SMS_ADDR:
                    // Sms gateway address
                    intent = new Intent(mContext, DmEditItem.class);
                    intent.putExtra("EditType", mlistItem[ITEM_SMS_ADDR]);
                    intent.putExtra("EditContent", DmService.getInstance().getSmsAddr());
                    startActivity(intent);
                    break;

                case ITEM_SMS_PORT:
                    // Sms gateway port
                    intent = new Intent(mContext, DmEditItem.class);
                    intent.putExtra("EditType", mlistItem[ITEM_SMS_PORT]);
                    intent.putExtra("EditContent", DmService.getInstance().getSmsPort());
                    startActivity(intent);
                    break;

                // case ITEM_TRIGGER_NULL_SESSION:
                // Trigger null session
                /*
                 * if (Vdmc.getInstance().isVDMRunning()) {
                 * ShowMessage("vDM engine is running, stop it first,please wait..."
                 * ); Vdmc.getInstance().stopVDM(); } else {
                 * ShowMessage("Starting vDM engine, please wait...");
                 * Vdmc.getInstance().startVDM(mContext,
                 * Vdmc.SessionType.DM_SESSION_USER, null, null); }
                 */
                // break;

                case ITEM_START_SERVICE:
                    Log.d(TAG, "start service with intent com.android.dm.SelfReg");
                    intent = new Intent("com.android.dm.SelfReg");
                    intent.setComponent(new ComponentName("com.android.dm",
                            "com.android.dm.DmService"));
                    startService(intent);
                    ShowMessage(mContext.getString(R.string.dm_service_started_tip));
                    break;

                case ITEM_DM_STATE:
                    showDmState();
                    break;

                case ITEM_SELFREG_SWITCH:
                    if (DmService.getInstance().getSelfRegSwitch())
                    {
                        DmService.getInstance().setSelfRegSwitch(mContext, false);
                        ShowMessage(mContext.getString(R.string.selfreg_switch_closed_tip));
                    }
                    else
                    {
                        DmService.getInstance().setSelfRegSwitch(mContext, true);
                        ShowMessage(mContext.getString(R.string.selfreg_switch_opened_tip));
                    }

                    break;

                case ITEM_SMSDELIVERREPORT_SWITCH:
                    if (DmService.getInstance().getSmsDeliverReportSwitch())
                    {
                        DmService.getInstance().setSmsDeliverReportSwitch(mContext, false);
                        ShowMessage(mContext.getString(R.string.smsdeliverreport_closed_tip));
                    }
                    else
                    {
                        DmService.getInstance().setSmsDeliverReportSwitch(mContext, true);
                        ShowMessage(mContext.getString(R.string.smsdeliverreport_opened_tip));
                    }

                    break;
                // =========guowen add for debug====================//
                case ITEM_SENDMESSAGE_COUNT:
                    Log.d(TAG, "=****=the count is=****= "
                            + DmService.getInstance().getMessageSendCount(mContext));
                    intent = new Intent(mContext, DmEditItem.class);
                    intent.putExtra("EditType", mlistItem[ITEM_SENDMESSAGE_COUNT]);
                    intent.putExtra("EditContent",
                            String.valueOf(DmService.getInstance().getMessageSendCount(mContext)));
                    startActivity(intent);
                    break;
                // ================end============================//
                default:
                    break;
            }
        }
    };

    private void showDmState()
    {
        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
        String message = new String();
        message = getString(R.string.debug_mode_dialog)
                + getString(DmService.getInstance().isDebugMode()
                ? R.string.true_dialog : R.string.false_dialog);
        message = message + "\n" + getString(R.string.send_message_dialog)
                + getString(DmService.getInstance().isHaveSendSelfRegMsg()
                ? R.string.true_dialog : R.string.false_dialog);
        message = message + "\n" + getString(R.string.session_running_dialog)
                + getString(DmService.getInstance().isSelfRegOk()
                ? R.string.true_dialog : R.string.false_dialog);
        message = message + "\n" + getString(R.string.last_session_state_dialog)
                + getString(Vdmc.getInstance().isVDMRunning()
                ? R.string.true_dialog : R.string.false_dialog);
        message = message + "\n" + getString(R.string.last_error_dialog)
                + (Vdmc.getLastSessionState().equals("NULL")
                ? getString(R.string.null_dialog) : Vdmc.getLastSessionState());
        message = message + "\n" + getString(R.string.last_error_dialog) + Vdmc.getLastError();

        builder.setTitle("")
                .setMessage(message)
                .setPositiveButton(getString(R.string.menu_ok),
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        Log.d(TAG, "showDmState: onClick OK");
                        dialog.dismiss();
                    }
                })

                .setOnCancelListener(new DialogInterface.OnCancelListener() {
                    public void onCancel(DialogInterface dialog) {
                        Log.d(TAG, "showDmState: onCancel");
                        dialog.dismiss();
                    }
                })

                .show();
    }
}
