/*************************************************************************
 Copyright (c)2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
*************************************************************************/

package com.qualcomm.qti.presenceapp;

import com.qualcomm.qti.presenceapp.R;
import com.qualcomm.qti.rcsimssettings.QrcsImsSettingsError;
import com.qualcomm.qti.rcsimssettings.QrcsImsSettingsIntType;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.opengl.Visibility;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

public class Settings extends Activity {
    final String TAG = "Settings";

    int FIELD_TAG = 0;
    int FIELD_VALUE = 1;
    Context mContext;
    AsyncTask mNotifyFormatTask;
    AsyncTask mEventReportTask;
    public static Settings settingsObject;
    public static Context settingsContext;
    public static boolean inSettingScreen = false;
    EditText availabilityCacheView;
    CheckBox videoSetting;
    TextView dataStatusViewTitle;
    TextView dataStatusView;
    public static Context context;
    AsyncTask mPublishTask;

    public static class EventReport {
        int publishTriggerInd;
        int notifyInd;
        int enablerInd;

        @Override
        public String toString() {
            return "EventReport [publishTriggerInd=" + publishTriggerInd
                    + ", notifyInd=" + notifyInd + ", enablerInd=" + enablerInd
                    + "]";
        }

        public EventReport(int publishTriggerInd, int notifyInd, int enablerInd) {
            this.publishTriggerInd = publishTriggerInd;
            this.notifyInd = notifyInd;
            this.enablerInd = enablerInd;
        }

        public EventReport() {
            // TODO Auto-generated constructor stub
        }
    }

    public static int video_on_off;
    public static int availabilityCacheExpiration;
    public static int capabilityCacheExpiration;
    public static boolean capabilityDiscoveryEnable;
    public static int capabilityPollInterval;
    public static int listSubscriptionExpiryTimer;
    public static int maxSubscriptionListEntries;
    public static int minimumPublishInterval;
    public static int publishExpiryTimer;
    public static int publishExtendedExpiryTimer;
    public static boolean volteUserOptInStatus;
    public static boolean isAvailability_cache_expiration_valid;
    public static boolean isCapabilites_cache_expiration_valid;
    public static boolean isCapability_discovery_enable_valid;
    public static boolean isCapability_poll_interval_valid;
    public static boolean isCapability_poll_list_subscription_expiry_timer_valid;
    public static boolean isMax_subcription_list_entries_valid;
    public static boolean isMinimum_publish_interval_valid;
    public static boolean isPublish_expiry_timer_valid;
    public static boolean isPublish_extended_expiry_timer_valid;
    public static boolean isVolte_user_opted_in_status_valid;
    public static boolean isVt_calling_enabled_valid;
    public static boolean isVt_calling_enabled;
    public static boolean isMobile_data_enabled;
    public static boolean isMobile_data_enabled_valid;


    private boolean isVtCheckBoxStateChange;
    private boolean isFtCheckBoxStateChange;
    private boolean isChatCheckBoxStateChange;
    private boolean isPresenceCheckBoxStateChange;

    int formMap[][] = {

            {
                    R.string.set_notify_fmt_text, R.id.notify_fmt_Spinner
            },

            {
                    R.string.availability_cache_exp_text,
                    R.id.availability_cache_exp_value
            },
            {
                    R.string.capability_cache_exp_text,
                    R.id.capability_cache_exp_value
            },
            {
                    R.string.capability_poll_interval_text,
                    R.id.capability_poll_interval_value
            },
            {
                    R.string.capability_poll_list_exp_text,
                    R.id.capability_poll_list_exp_value
            },
            {
                    R.string.source_throttle_publish_text,
                    R.id.source_throttle_publish_value
            },
            {
                    R.string.subscribe_retry_text, R.id.subscribe_retry_value
            },
            {
                    R.string.publish_retry_text, R.id.publish_retry_value
            },
            {
                    R.string.max_list_entries_text, R.id.max_list_entries_value
            },

    };

    class SettingsMainThreadHandler extends Handler {

        final static int SET_NOTIFY_FORMAT_RESPONSE = 0;
        final static int SET_EVENT_REPORT_RESPONSE = 1;

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case SET_NOTIFY_FORMAT_RESPONSE: {
                    String val = (String) msg.obj;
                    Log.d(TAG,
                            "Received msg in Setting activity main thread. msg.obj="
                                    + val);
                    updateNotifyFmtValue(val);
                    break;
                }
                case SET_EVENT_REPORT_RESPONSE: {
                    EventReport rep = (EventReport) msg.obj;
                    Log.d(TAG, rep.toString());
                    break;
                }

                default:
                    Log.e(TAG, "Unknown msg");
            }

            super.handleMessage(msg);
        }

        private void updateNotifyFmtValue(String val) {
            Spinner spinner = (Spinner) findViewById(R.id.notify_fmt_Spinner);
            ArrayAdapter adapter = (ArrayAdapter) spinner.getAdapter();
            int pos = adapter.getPosition(val);
            spinner.setSelection(pos);
        }
    }

    private void sendLogMesg(Context c, String string) {

        Utility.sendLogMesg(c, string);

    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        settingsObject = this;
        settingsContext = getApplicationContext();
        mContext = this;
        inSettingScreen = true;
        this.setContentView(R.layout.settings);
        availabilityCacheView = (EditText) findViewById(R.id.availability_cache_exp_value);

        try {
            if(MainActivity.imsSettingService != null)
            {
                QrcsImsSettingsIntType client_handle = new QrcsImsSettingsIntType();
                client_handle.setQrcsImsSettingsIntType(AppGlobalState.getQrcsImsSettingsclienthandle());
                MainActivity.imsSettingService
                    .QrcsImsSettings_GetQipcallConfig(client_handle);
                MainActivity.imsSettingService
                .QrcsImsSettings_GetPresenceConfig(client_handle);
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        catch (Exception e) {
            e.printStackTrace();
        }

        isVtCheckBoxStateChange = false;
        isChatCheckBoxStateChange = false;
        isFtCheckBoxStateChange = false;
        isPresenceCheckBoxStateChange = false;

        dataStatusViewTitle = (TextView) findViewById(R.id.data_textview_title);
        dataStatusView = (TextView) findViewById(R.id.data_textview);
        if(AppGlobalState.getOperatorMode() == AppGlobalState.ATT_MODE)
        {
            Log.d(TAG, "AppGlobalState.ATT_MODE");
            try
            {
                dataStatusViewTitle.setText("");
                dataStatusViewTitle.setHeight(0);
                dataStatusViewTitle.setVisibility(View.INVISIBLE);
                dataStatusView.setText("");
                dataStatusView.setHeight(0);
                dataStatusView.setVisibility(View.INVISIBLE);
            }
            catch(Exception e)
            {
                e.printStackTrace();
            }
        }

        videoSetting = (CheckBox) findViewById(R.id.vt_checkbox);
        handleVtCheckBox(videoSetting);
        if(AppGlobalState.getOperatorMode() == AppGlobalState.ATT_MODE)
        {
            Log.d(TAG, "AppGlobalState.ATT_MODE");
            videoSetting.setChecked(true);
            videoSetting.setEnabled(false);
        }

        SettingsMainThreadHandler settingsHandler = new SettingsMainThreadHandler();
        AppGlobalState.setSettingsHandler(settingsHandler);

        initButtons();
        populateInitialValues();
    }

    @Override
    protected void onStart() {
        super.onStart();
        inSettingScreen = true;
    }

    public void updateDataSetting() {

        CheckBox ftSetting = (CheckBox) findViewById(R.id.ft_checkbox);
        CheckBox chatSetting = (CheckBox) findViewById(R.id.chat_checkbox);

        if(AppGlobalState.getOperatorMode() == AppGlobalState.VZW_MODE)
        {
            Log.d("PRESENCE_UI", "Inside updateDataSetting isMobile_data_enabled =  "
                    + isMobile_data_enabled);
            if (isMobile_data_enabled) {
                dataStatusView.setText("Mobile Data : Enabled");
                ftSetting.setEnabled(true);
                chatSetting.setEnabled(true);
            }
            else {
                dataStatusView.setText("Mobile Data : Disabled");
                ftSetting.setEnabled(false);
                chatSetting.setEnabled(false);
            }
        }
        else
        {
            Log.d(TAG, "AppGlobalState.ATT_MODE");
            try
            {
                LinearLayout checkBoxLayout = (LinearLayout)findViewById(R.id.checkbox_layout);
                if(ftSetting != null)
                {
                    checkBoxLayout.removeView(ftSetting);
                }
                if(chatSetting != null)
                {
                    checkBoxLayout.removeView(chatSetting);
                }
                dataStatusViewTitle.setText("");
                dataStatusViewTitle.setHeight(0);
                dataStatusViewTitle.setVisibility(View.INVISIBLE);
                dataStatusView.setText("");
                dataStatusView.setHeight(0);
                dataStatusView.setVisibility(View.INVISIBLE);
            }
            catch(Exception e)
            {
                e.printStackTrace();
            }
        }
    }

    public void updateVideoSetting() {

        videoSetting = (CheckBox) findViewById(R.id.vt_checkbox);
        if(AppGlobalState.getOperatorMode() == AppGlobalState.VZW_MODE)
        {
            if (isVt_calling_enabled && !isMobile_data_enabled) {
                Log.d("PRESENCE_UI", "Data is disabled ... video was checked, video = "
                        + isVt_calling_enabled);
                videoSetting.setChecked(true);
                videoSetting.setEnabled(false);
            }
            else if (isVt_calling_enabled && isMobile_data_enabled) {
                Log.d("PRESENCE_UI", "Data is enabled ... video was checked , video = "
                        + isVt_calling_enabled);
                videoSetting.setChecked(true);
                videoSetting.setEnabled(true);
            }
            else if (!isVt_calling_enabled && !isMobile_data_enabled)
            {
                Log.d("PRESENCE_UI", " Data is disabled ...video was unchecked, video = "
                        + isVt_calling_enabled);
                videoSetting.setChecked(false);
                videoSetting.setEnabled(false);
            }
            else if (!isVt_calling_enabled && isMobile_data_enabled)
            {
                Log.d("PRESENCE_UI", "Data is enabled ... video was unchecked, video = "
                        + isVt_calling_enabled);
                videoSetting.setChecked(false);
                videoSetting.setEnabled(true);
            }
        }
        else
        {
            Log.d(TAG, "AppGlobalState.ATT_MODE");
            videoSetting.setChecked(true);
            videoSetting.setEnabled(false);
        }
    }

    public void SavePreference(Context ctx, String key, int value) {

        SharedPreferences preferences = ctx.getSharedPreferences("presencedata",
                MODE_PRIVATE);

        SharedPreferences.Editor editor = preferences.edit();
        editor.putInt(key, value);
        Log.d("PRESENCE_UI", "Saving ....Data in preference " + value);
        editor.commit();
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.d("PRESENCE_UI", "ONPAUSE");
        inSettingScreen = false;
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d("PRESENCE_UI", "ON RESUME");
        inSettingScreen = true;
    }

    @Override
    protected void onStop() {
        super.onStop();
        finish();
    }

    private void initButtons() {

        if(AppGlobalState.getOperatorMode() == AppGlobalState.ATT_MODE)
        {
            CheckBox ftSetting = (CheckBox) findViewById(R.id.ft_checkbox);
            CheckBox chatSetting = (CheckBox) findViewById(R.id.chat_checkbox);
            LinearLayout checkBoxLayout = (LinearLayout)findViewById(R.id.checkbox_layout);
            if(ftSetting != null)
            {
                checkBoxLayout.removeView(ftSetting);
            }
            if(chatSetting != null)
            {
                checkBoxLayout.removeView(chatSetting);
            }
        }
        else
        {
            CheckBox ftSetting = (CheckBox) findViewById(R.id.ft_checkbox);
            CheckBox chatSetting = (CheckBox) findViewById(R.id.chat_checkbox);
            handleFtCheckBox(ftSetting);
            handleChatCheckBox(chatSetting);
        }

        CheckBox presenceSetting = (CheckBox) findViewById(R.id.presence_checkbox);
        handlePresenceCheckBox(presenceSetting);

        Button okButton = (Button) findViewById(R.id.ok);
        handleOkButtonClick(okButton);

        Button cancelButton = (Button) findViewById(R.id.cancel);
        handleCancelButtonClick(cancelButton);

        Button setNotifyFmtButton = (Button) findViewById(R.id.set_notify_fmt_button);
        handleSetNotifyButtonClick(setNotifyFmtButton);
        Button getNotifyFmtButton = (Button) findViewById(R.id.get_notify_current_fmt_button);
        handleGetNotifyButtonClick(getNotifyFmtButton);

    }

    private void handleSetNotifyButtonClick(Button setNotifyFmtButton) {
        setNotifyFmtButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                SharedPreferences.Editor editor = getSharedPrefEditor(
                        AppGlobalState.IMS_PRESENCE_SETTINGS);
                storeFormValuesToSharedPreferences(editor);
            }
        });
    }

    private void handleGetNotifyButtonClick(Button getNotifyFmtButton) {
        getNotifyFmtButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {

            }
        });
    }


 private void handleVtCheckBox(CheckBox vtCheckBox) {

     vtCheckBox.setOnClickListener(new OnClickListener() {

         public void onClick(View v) {

             if(isVtCheckBoxStateChange)
             {
                 isVtCheckBoxStateChange = false;
                 Log.d("PRESENCE_UI", "Setting : handleVtCheckBox : "+isVtCheckBoxStateChange);
             }
             else
             {
                 isVtCheckBoxStateChange = true;
                 Log.d("PRESENCE_UI", "Setting : handleVtCheckBox : "+isVtCheckBoxStateChange);
             }
         }
     });
 }

 private void handleFtCheckBox(CheckBox ftCheckBox) {
        ftCheckBox.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {

                if(isFtCheckBoxStateChange)
                {
                    isFtCheckBoxStateChange = false;
                }
                else
                {
                    isFtCheckBoxStateChange = true;
                }
            }
        });
    }

    private void handleChatCheckBox(CheckBox chatCheckBox) {
        chatCheckBox.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {

                if(isChatCheckBoxStateChange)
                {
                    isChatCheckBoxStateChange = false;
                }
                else
                {
                    isChatCheckBoxStateChange = true;
                }
            }
        });
    }

    private void handlePresenceCheckBox(CheckBox chatCheckBox) {

        try
        {
            if(AppGlobalState.getOperatorMode() == AppGlobalState.ATT_MODE)
            {
                Log.d(TAG, "AppGlobalState.ATT_MODE");
                chatCheckBox.setChecked(volteUserOptInStatus);
                chatCheckBox.setEnabled(true);
                chatCheckBox.setVisibility(View.VISIBLE);
                chatCheckBox.setOnClickListener(new OnClickListener() {

                    @Override
                    public void onClick(View v) {
                        if(isPresenceCheckBoxStateChange)
                        {
                            isPresenceCheckBoxStateChange = false;
                        }
                        else
                        {
                            isPresenceCheckBoxStateChange = true;
                        }
                    }
                });
            }
            else
            {
                LinearLayout checkBoxLayout = (LinearLayout)findViewById(R.id.checkbox_layout);
                checkBoxLayout.removeView(chatCheckBox);
            }
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }
    }


    private void handleVtImFtCheckBox()
    {
        CheckBox videoCheckBox = (CheckBox) findViewById(R.id.vt_checkbox);
        CheckBox ftCheckBox = (CheckBox) findViewById(R.id.ft_checkbox);
        CheckBox chatCheckBox = (CheckBox) findViewById(R.id.chat_checkbox);
        CheckBox presenceCheckBox = (CheckBox) findViewById(R.id.presence_checkbox);

        if(isFtCheckBoxStateChange || isChatCheckBoxStateChange)
        {
            SharedPreferences preferences = mContext.getSharedPreferences(
                    "presencedata", mContext.MODE_PRIVATE);
            SharedPreferences.Editor editor = preferences.edit();

            if(isFtCheckBoxStateChange)
            {
                if (ftCheckBox.isChecked() == true)
                {
                    Log.d("PRESENCE_UI", "FT is Checked");
                    editor.putBoolean("FT_KEY", true);
                } else {
                    Log.d("PRESENCE_UI", "FT is UnChecked");
                    editor.putBoolean("FT_KEY", false);
                }
            }
            if(isChatCheckBoxStateChange)
            {
                if (chatCheckBox.isChecked() == true)
                {
                    Log.d("PRESENCE_UI", "Chat is Checked");
                    editor.putBoolean("CHAT_KEY", true);
                } else {
                    Log.d("PRESENCE_UI", "Chat is UnChecked");
                    editor.putBoolean("CHAT_KEY", false);
                }
            }

            editor.commit();

            Log.d("PRESENCE_UI", "Trigerring publish for FT or IM change ");
            sendLogMesg(mContext,"File transfer or Chat selected ...Menu option Publish Rich selected.");
        }

        if(isVtCheckBoxStateChange)
        {
            ContactInfo.firstPublish = true;
            if (videoCheckBox.isChecked() == true)
            {
                Log.d("PRESENCE_UI", "User selected Video ");

                try
                {
                    QrcsImsSettingsIntType client_handle = new QrcsImsSettingsIntType();
                    client_handle.setQrcsImsSettingsIntType(AppGlobalState.getQrcsImsSettingsclienthandle());
                    MainActivity.qipcall_config.setVt_calling_enabled(true);
                    MainActivity.qipcall_config.setVt_calling_enabled_valid(true);
                    Log.d("PRESENCE_UI", "Before QrcsImsSettings_SetQipcallConfig");
                    if(MainActivity.imsSettingService != null)
                    {
                        QrcsImsSettingsError settingError = MainActivity.imsSettingService.QrcsImsSettings_SetQipcallConfig(
                                client_handle, MainActivity.qipcall_config);
                        if(settingError.getQrcsImsSettingsError() == 0)
                        {
                            isVt_calling_enabled = true;
                            Log.d("PRESENCE_UI", "if QrcsImsSettings_SetQipcallConfig success");
                        }
                        else
                        {
                            videoCheckBox.setChecked(false);
                            Log.d("PRESENCE_UI", "if QrcsImsSettings_SetQipcallConfig failed");
                               Toast.makeText(mContext, "Video Setting Failed", Toast.LENGTH_SHORT).show();
                        }
                        Log.d("PRESENCE_UI", "After QrcsImsSettings_SetQipcallConfig");
                    }
                } catch (RemoteException e) {
                    e.printStackTrace();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
            else {

                Log.d("PRESENCE_UI", "User deselected Video ");
                try
                {
                    QrcsImsSettingsIntType client_handle = new QrcsImsSettingsIntType();
                    client_handle.setQrcsImsSettingsIntType(AppGlobalState.getQrcsImsSettingsclienthandle());
                    MainActivity.qipcall_config.setVt_calling_enabled(false);
                    MainActivity.qipcall_config.setVt_calling_enabled_valid(true);
                    Log.d("PRESENCE_UI", "Before QrcsImsSettings_SetQipcallConfig");
                    if(MainActivity.imsSettingService != null)
                    {
                        QrcsImsSettingsError settingError = MainActivity.imsSettingService.QrcsImsSettings_SetQipcallConfig(
                                client_handle, MainActivity.qipcall_config);
                        if(settingError.getQrcsImsSettingsError() == 0)
                        {
                            isVt_calling_enabled = false;
                            Log.d("PRESENCE_UI", "if QrcsImsSettings_SetQipcallConfig success");
                        }
                        else
                        {
                            videoCheckBox.setChecked(true);
                            Log.d("PRESENCE_UI", "QrcsImsSettings_SetQipcallConfig failed");
                               Toast.makeText(mContext, "Video Setting Failed", Toast.LENGTH_SHORT).show();
                        }
                    }
                    Log.d("PRESENCE_UI", "After QrcsImsSettings_SetQipcallConfig");
                } catch (RemoteException e) {
                    e.printStackTrace();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }

        if(isVtCheckBoxStateChange || isFtCheckBoxStateChange || isChatCheckBoxStateChange)
        {
            if(!isVtCheckBoxStateChange)
            {
                if (Utility.getNotifyFmt(mContext) == AppGlobalState.NOTIFY_FMT_STRUCT)
                {
                    Log.d("PRESENCE_UI", "Before publish for Chat, im and ft ");
                    sendLogMesg(mContext, "Publish fmt=STRUCT based.");
                    mPublishTask = new PublishTask().execute();
                    Log.d("PRESENCE_UI", "After publish for Chat, im and ft ");
                }
            }
            else
            {
                Log.d("PRESENCE_UI", "VT Capability changed");
            }
        }
        else
        {
            Log.d("PRESENCE_UI", "No Capability change for Video, im and ft ");
            Toast.makeText(mContext, "No Capability change", Toast.LENGTH_SHORT).show();
        }

        if(isPresenceCheckBoxStateChange)
        {
            try
            {
                Log.d(TAG, "presence check box state is changed");
                if(MainActivity.imsSettingService != null && AppGlobalState.getQrcsImsSettingsclienthandle() != 0)
                {
                    Log.d("PRESENCE_UI", "before QrcsImsSettings_SetPresenceConfig ");
                    QrcsImsSettingsIntType client_handle = new QrcsImsSettingsIntType();
                    client_handle.setQrcsImsSettingsIntType(AppGlobalState.getQrcsImsSettingsclienthandle());
                    if(presenceCheckBox.isChecked())
                    {
                        MainActivity.presence_config.setVolte_user_opted_in_status_valid(true);
                        MainActivity.presence_config.setVolte_user_opted_in_status(true);
                        QrcsImsSettingsError settingError = MainActivity.imsSettingService.QrcsImsSettings_SetPresenceConfig(
                                client_handle, MainActivity.presence_config);
                        if(settingError.getQrcsImsSettingsError() == 0)
                        {
                            volteUserOptInStatus = true;
                        }
                    }
                    else
                    {
                        MainActivity.presence_config.setVolte_user_opted_in_status_valid(true);
                        MainActivity.presence_config.setVolte_user_opted_in_status(false);
                        QrcsImsSettingsError settingError = MainActivity.imsSettingService.QrcsImsSettings_SetPresenceConfig(
                                client_handle, MainActivity.presence_config);
                        if(settingError.getQrcsImsSettingsError() == 0)
                        {
                            volteUserOptInStatus = false;
                        }
                    }
                    Log.d("PRESENCE_UI", "After QrcsImsSettings_SetPresenceConfig ");
                }
            }
            catch(RemoteException e)
            {
                e.printStackTrace();
            }
            catch(Exception e)
            {
                e.printStackTrace();
            }
        }
    }

    private void handleOkButtonClick(Button cancelButton) {
        cancelButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                handleVtImFtCheckBox();
                sendFormValues();
                finish();
            }
        });
    }

    private void handleCancelButtonClick(Button cancelButton) {
        cancelButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                isVtCheckBoxStateChange = false;
                isChatCheckBoxStateChange = false;
                isFtCheckBoxStateChange = false;
                finish();
            }
        });
    }

    public void updateSettingValues() {

        if (isAvailability_cache_expiration_valid)
        {
            Log.d("PRESENCE_UI", "isAvailability_cache_expiration_valid View  "
                    + findViewById(R.id.availability_cache_exp_value));
            EditText availabilityCacheView = (EditText) findViewById(R.id.availability_cache_exp_value);
            Log.d("PRESENCE_UI", "isAvailability_cache_expiration_valid View  "
                    + availabilityCacheView);
            availabilityCacheView.setText("" + availabilityCacheExpiration);
            availabilityCacheView.invalidate();
        }

        if (isCapabilites_cache_expiration_valid) {
            Log.d("PRESENCE_UI", "isCapabilites_cache_expiration_valid ");
            EditText capabilityCacheView = (EditText) findViewById(R.id.capability_cache_exp_value);
            capabilityCacheView.setText("" + capabilityCacheExpiration);
            capabilityCacheView.invalidate();

        }

        if (isCapability_poll_interval_valid) {
            Log.d("PRESENCE_UI", "isCapability_poll_interval_valid ");
            EditText capabilityPollView = (EditText) findViewById(R.id.capability_poll_interval_value);
            capabilityPollView.setText("" + capabilityPollInterval);
            capabilityPollView.invalidate();
        }

        if (isCapability_poll_list_subscription_expiry_timer_valid) {
            Log.d("PRESENCE_UI", "isCapability_poll_list_subscription_expiry_timer_valid ");
            EditText listSubscriptionExpiryTimerView = (EditText) findViewById(R.id.capability_poll_list_exp_value);
            listSubscriptionExpiryTimerView.setText("" + listSubscriptionExpiryTimer);
            listSubscriptionExpiryTimerView.invalidate();

        }

        if (isMinimum_publish_interval_valid) {
            Log.d("PRESENCE_UI", "isMinimum_publish_interval_valid ");
            EditText sourceThrottlelView = (EditText) findViewById(R.id.source_throttle_publish_value);
            sourceThrottlelView.setText("" + minimumPublishInterval);
            sourceThrottlelView.invalidate();
        }

        if (isMax_subcription_list_entries_valid) {
            Log.d("PRESENCE_UI", "isMax_subcription_list_entries_valid ");
            EditText maxSubscriptionListEntriesView = (EditText) findViewById(R.id.max_list_entries_value);
            maxSubscriptionListEntriesView.setText("" + maxSubscriptionListEntries);
            maxSubscriptionListEntriesView.invalidate();

        }

    }

    private void populateInitialValues() {
        SharedPreferences setting = getSharedPrefHandle(
                AppGlobalState.IMS_PRESENCE_SETTINGS);

        for (int i = 0; i < 4; i++) {

            String temp = getString(formMap[i][FIELD_TAG]);
            Log.d("PRESENCE_UI", temp);
            String uriValue = setting.getString(
                    getString(formMap[i][FIELD_TAG]), "");

            if (formMap[i][FIELD_VALUE] == R.id.notify_fmt_Spinner) {

                Spinner spinner = (Spinner) findViewById(formMap[i][FIELD_VALUE]);
                ArrayAdapter adapter = (ArrayAdapter) spinner.getAdapter();
                int pos = adapter.getPosition(uriValue);
                spinner.setSelection(pos);

            } else {
                Log.d("PRESENCE_UI", "DO NOTHING");
            }
        }

        SharedPreferences preferences = mContext.getSharedPreferences(
                "presencedata", Context.MODE_PRIVATE);
        boolean ftSupported = preferences.getBoolean("FT_KEY", false);
        boolean chatSupported = preferences.getBoolean("CHAT_KEY", false);
        Log.d("PRESENCE_UI", "ftSupported =" + ftSupported + "chatSupported =" + chatSupported);

        if(AppGlobalState.getOperatorMode() == AppGlobalState.VZW_MODE)
        {
            CheckBox ftSetting = (CheckBox) findViewById(R.id.ft_checkbox);
            ftSetting.setChecked(ftSupported);
            CheckBox chatSetting = (CheckBox) findViewById(R.id.chat_checkbox);
            chatSetting.setChecked(chatSupported);
        }
    }

    private SharedPreferences getSharedPrefHandle(String imsPresencePref) {
        SharedPreferences settings = getSharedPreferences(imsPresencePref, 0);

        return settings;
    }

    private Editor getSharedPrefEditor(String imsPresencePref) {
        SharedPreferences settings = getSharedPrefHandle(imsPresencePref);
        SharedPreferences.Editor editor = settings.edit();
        return editor;
    }

    private void storeFormValuesToSharedPreferences(Editor editor) {
        for (int i = 0; i < 4; i++) {
            if (formMap[i][FIELD_VALUE] == R.id.notify_fmt_Spinner) {

                Spinner spinner = (Spinner) findViewById(formMap[i][FIELD_VALUE]);
                editor.putString(getString(formMap[i][FIELD_TAG]), spinner
                        .getSelectedItem().toString());

            }
            else {
                Log.d("PRESENCE_UI", "DO NOTHING");
            }
        }
        editor.commit();
    }

    private void sendFormValues() {

        Log.d("PRESENCE_UI", "sendFormValues ");

        try {
            TextView availabilityCacheExpiry = (TextView) findViewById(R.id.availability_cache_exp_value);
            if (availabilityCacheExpiry.getText().length() > 0) {

                MainActivity.presence_config
                        .setAvailability_cache_expiration_valid(true);
                MainActivity.presence_config
                        .setAvailability_cache_expiration(Integer
                                .parseInt(availabilityCacheExpiry.getText()
                                        .toString()));
                Log.d("PRESENCE_UI",
                        "availabilityCacheExpiry "
                                + Integer.parseInt(availabilityCacheExpiry
                                        .getText().toString()));
            } else {
                MainActivity.presence_config
                        .setAvailability_cache_expiration_valid(false);
            }

            TextView capabilityCacheExpiry = (TextView) findViewById(R.id.capability_cache_exp_value);
            if (capabilityCacheExpiry.getText().length() > 0) {
                MainActivity.presence_config
                        .setCapabilites_cache_expiration_valid(true);
                MainActivity.presence_config
                        .setCapabilites_cache_expiration(Integer
                                .parseInt(capabilityCacheExpiry.getText()
                                        .toString()));
                Log.d("PRESENCE_UI",
                        "capabilityCacheExpiry "
                                + Integer.parseInt(capabilityCacheExpiry
                                        .getText().toString()));

            } else {
                MainActivity.presence_config
                        .setCapabilites_cache_expiration_valid(false);
            }

            TextView capabilityPollInterval = (TextView) findViewById(R.id.capability_poll_interval_value);
            if (capabilityPollInterval.getText().length() > 0) {
                MainActivity.presence_config
                        .setCapability_poll_interval_valid(true);
                MainActivity.presence_config
                        .setCapability_poll_interval(Integer
                                .parseInt(capabilityPollInterval.getText()
                                        .toString()));
                Log.d("PRESENCE_UI",
                        "capabilityPollInterval "
                                + Integer.parseInt(capabilityPollInterval
                                        .getText().toString()));
            } else {
                MainActivity.presence_config
                        .setCapability_poll_interval_valid(false);

            }

            TextView listSubscriptionExpiryTimer = (TextView) findViewById(R.id.capability_poll_list_exp_value);
            if (listSubscriptionExpiryTimer.getText().length() > 0) {
                MainActivity.presence_config
                        .setCapability_poll_list_subscription_expiry_timer_valid(true);
                MainActivity.presence_config
                        .setCapability_poll_list_subscription_expiry_timer(Integer
                                .parseInt(listSubscriptionExpiryTimer.getText()
                                        .toString()));
                Log.d("PRESENCE_UI",
                        "listSubscriptionExpiryTimer "
                                + Integer.parseInt(listSubscriptionExpiryTimer
                                        .getText().toString()));
            } else {
                MainActivity.presence_config
                        .setCapability_poll_list_subscription_expiry_timer_valid(true);
            }

            TextView maxSubscriptionListEntries = (TextView) findViewById(R.id.max_list_entries_value);
            if (maxSubscriptionListEntries.getText().length() > 0) {
                MainActivity.presence_config
                        .setMax_subcription_list_entries_valid(true);
                MainActivity.presence_config
                        .setMax_subcription_list_entries(Integer
                                .parseInt(maxSubscriptionListEntries.getText()
                                        .toString()));
                Settings.maxSubscriptionListEntries = Integer.parseInt(maxSubscriptionListEntries
                        .getText().toString());
                Log.d("PRESENCE_UI",
                        "Settings.maxSubscriptionListEntries "
                                + Integer.parseInt(maxSubscriptionListEntries
                                        .getText().toString()));
                Log.d("PRESENCE_UI",
                        "maxSubscriptionListEntries "
                                + Integer.parseInt(maxSubscriptionListEntries
                                        .getText().toString()));
            } else {
                MainActivity.presence_config
                        .setMax_subcription_list_entries_valid(false);
            }

            TextView minPublishInterval = (TextView) findViewById(R.id.source_throttle_publish_value);
            if (minPublishInterval.getText().length() > 0) {
                MainActivity.presence_config
                        .setMinimum_publish_interval_valid(true);
                MainActivity.presence_config.setMinimum_publish_interval(Integer
                        .parseInt(minPublishInterval.getText().toString()));
                Log.d("PRESENCE_UI",
                        "minPublishInterval "
                                + Integer.parseInt(minPublishInterval
                                        .getText().toString()));
            } else {
                MainActivity.presence_config
                        .setMinimum_publish_interval_valid(false);
            }

            Log.d("PRESENCE_UI", "Before QrcsImsSettings_SetPresenceConfig ");
            if(MainActivity.imsSettingService != null)
            {
                QrcsImsSettingsIntType client_handle = new QrcsImsSettingsIntType();
                client_handle.setQrcsImsSettingsIntType(AppGlobalState.getQrcsImsSettingsclienthandle());
                MainActivity.imsSettingService.QrcsImsSettings_SetPresenceConfig(
                        client_handle, MainActivity.presence_config);
                Log.d("PRESENCE_UI", "After QrcsImsSettings_SetPresenceConfig ");
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

}
