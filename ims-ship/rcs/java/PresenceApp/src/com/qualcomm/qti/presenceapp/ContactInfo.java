/*************************************************************************
 Copyright (c)2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
*************************************************************************/

package com.qualcomm.qti.presenceapp;

import java.util.ArrayList;

import com.qualcomm.qti.presenceapp.MainActivity.ContactArrayAdapter;
import com.qualcomm.qti.presenceapp.R;
import com.qualcomm.qti.rcsimssettings.QrcsImsSettingsIntType;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.RemoteException;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.Toast;

public class ContactInfo extends Activity {
    static final String TAG = "ContactInfo";

    TextView name, phone;
    TextView listContactUri,
            listName,
            listVersion,
            listFullState,
            resourceUri,
            isVolteContact,
            publishStatus,
            resourceId,
            resourceState,
            resourceReason,
            resourceCid,
            contactUri,
            description,
            version,
            serviceId,
            isAudioSupported,
            audioCapabilities,
            isVideoSupported,
            videoCapabilities,
            timestamp;

    CheckBox isSelected;

    ImageView icon;
    Button videoButton;
    static Button audioButton;
    Button unSubscribeButton;
    public static Button subscribePollingButton;
    Button capabilityPollingButton;
    public static boolean networkTypeEHRPD = false;
    public static boolean networkTypeLTE = false;
    public static boolean vopsEnabled = true;
    public static boolean firstPublish = true;
    public static ContactInfo contactInfoObject;
    public static boolean inContactInfoScreen = false;

    int mContactIndex;
    AsyncTask mSubscribePollingTask;
    AsyncTask mSubscribeSimpleTask;
    AsyncTask mUnSubscribeTask;
    Context mContext = this;
    ListenerHandler mListenerHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.contactinfo);

        Intent startUpIntent = this.getIntent();
        int contactIndex = getContactIndexFromIntent(startUpIntent);
        mContactIndex = contactIndex;
        contactInfoObject = this;
        inContactInfoScreen = true;

        try {
            if(MainActivity.imsSettingService!= null)
            {
                QrcsImsSettingsIntType client_handle = new QrcsImsSettingsIntType();
                client_handle.setQrcsImsSettingsIntType(AppGlobalState.getQrcsImsSettingsclienthandle());
                MainActivity.imsSettingService
                        .QrcsImsSettings_GetQipcallConfig(client_handle);
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }catch (Exception e) {
            e.printStackTrace();
        }

        checkPreconditions();

        AppGlobalState.setContactInfo(this);

        mListenerHandler = AppGlobalState.getListenerHandler();

        initContactInfoResIds();

        sendAvailabilityFetch(contactIndex);

        populateForm(contactIndex);

        initButtons(contactIndex);
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.d("PRESENCE_UI", "ONPAUSE");
        inContactInfoScreen = false;
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d("PRESENCE_UI", "ON RESUME");
        inContactInfoScreen = true;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        if (AppGlobalState.getOperatorMode() == AppGlobalState.ATT_MODE)
        {
            MenuInflater inflater = getMenuInflater();
            inflater.inflate(R.menu.contact_info_menu, menu);
            return super.onCreateOptionsMenu(menu);
        }
        else
        {
            return false;
        }

    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {

        switch (item.getItemId()) {
            case R.id.seturi:
                Intent setUri = new Intent(ContactInfo.this, UpdateContactURI.class);
                setUri.putExtra("INDEX", mContactIndex);
                startActivity(setUri);
                break;
        }

        return super.onOptionsItemSelected(item);
    }

    private void checkPreconditions() {
        if (Utility.isContactExcluded(mContactIndex)) {
            showErrorContactExcluded();
        } else if (Utility.isContactOptedOut(mContactIndex)) {
            showErrorContactOptedOut();
        }
    }

    private void showErrorContactOptedOut() {
        showDialog("This contact did not participate in Volte 2012 services.");
    }

    private void showErrorContactExcluded() {
        showDialog("This contact is special number and" +
                " hence excluded from presence contact list");
    }

    private void showDialog(String msg) {
        DialogInterface.OnClickListener dialogClickListener =
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        switch (which) {
                            case DialogInterface.BUTTON_POSITIVE:
                                finish();
                                break;
                        }
                    }
                };

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage(msg).setPositiveButton("Ok",
                dialogClickListener).setCancelable(false).show();
    }

    boolean getCheckBoxStatus() {
        return isSelected.isChecked();
    }

    void updateMultiSelection() {
        Contact c;
        ArrayList<Contact> contacts = AppGlobalState.getContacts();
        c = contacts.get(mContactIndex);

        c.setMultiSelected(getCheckBoxStatus());
    }

    @Override
    public void onBackPressed() {
        ContactArrayAdapter<Contact> adapter =
                AppGlobalState.getMainListAdapter();
        adapter.notifyDataSetChanged();

        updateMultiSelection();
        super.onBackPressed();
    }

    private void initContactInfoResIds() {

        icon = (ImageView) findViewById(R.id.icon);
        name = (TextView) findViewById(R.id.name);
        isSelected = (CheckBox) findViewById(R.id.isSelected);

        phone = (TextView) findViewById(R.id.phone);
        publishStatus = (TextView) findViewById(R.id.basicStatus);
        timestamp = (TextView) findViewById(R.id.Timestamp);
        isAudioSupported = (TextView) findViewById(R.id.Audio);
        isVideoSupported = (TextView) findViewById(R.id.Video);
        listContactUri = (TextView) findViewById(R.id.listContactUri);
        listName = (TextView) findViewById(R.id.listName);
        listVersion = (TextView) findViewById(R.id.listVersion);
        listFullState = (TextView) findViewById(R.id.listFullState);
        resourceUri = (TextView) findViewById(R.id.resourceUri);
        isVolteContact = (TextView) findViewById(R.id.isVolteContact);
        resourceId = (TextView) findViewById(R.id.resourceId);
        resourceState = (TextView) findViewById(R.id.resourceState);
        resourceReason = (TextView) findViewById(R.id.resourceReason);
        resourceCid = (TextView) findViewById(R.id.resourceCid);
        contactUri = (TextView) findViewById(R.id.contactUri);
        description = (TextView) findViewById(R.id.description);
        version = (TextView) findViewById(R.id.version);
        serviceId = (TextView) findViewById(R.id.serviceId);
        audioCapabilities = (TextView) findViewById(R.id.audioCapabilities);
        videoCapabilities = (TextView) findViewById(R.id.videoCapabilities);

        videoButton = (Button) findViewById(R.id.StartVideocall);
        audioButton = (Button) findViewById(R.id.StartAudiocall);
        unSubscribeButton = (Button) findViewById(R.id.UnSubscribe);
        subscribePollingButton = (Button) findViewById(R.id.AvailabilityFetch);
        capabilityPollingButton = (Button) findViewById(R.id.capabilityPolling);
    }

    private void initButtons(int contactIndex) {

        initSubscribePollingButton(contactIndex);
        initcapabilityPollingButton(contactIndex);
        initDoneButton();
    }

    private void initcapabilityPollingButton(int contactIndex) {
        Button capabilityPollingButton = (Button) findViewById(R.id.capabilityPolling);
        if(AppGlobalState.getOperatorMode() == AppGlobalState.VZW_MODE)
        {
            handleSubscribeSimpleButtonClick(capabilityPollingButton, contactIndex);
        }
        else
        {
            TableRow row = (TableRow)findViewById(R.id.TableRow02);
            if(capabilityPollingButton != null)
            {
                row.removeView(capabilityPollingButton);
            }
        }

    }

    private void handleSubscribeSimpleButtonClick(Button subscribePollingButton,
            final int contactIndex) {

        subscribePollingButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                AppGlobalState.setPendingSubscriptionUri(contactIndex);
                invokeSubscribeSimple(contactIndex, false);
            }
        });
    }

    private void initSubscribePollingButton(int contactIndex) {
        Button subscribePollingButton = (Button) findViewById(R.id.AvailabilityFetch);

        handleSubscribePollingButtonClick(subscribePollingButton, contactIndex);
    }

    public static void enableDisableSubscribePollingButton(boolean enabled) {
        if (subscribePollingButton != null) {
            subscribePollingButton.setEnabled(!enabled);
        }
    }

    private void handleSubscribePollingButtonClick(Button subscribePollingButton,
            final int contactIndex) {

        subscribePollingButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                AppGlobalState.setPendingSubscriptionUri(contactIndex);
                invokeSubscribePolling(contactIndex);
            }
        });
    }

    private void invokeSubscribeSimple(int contactIndex, boolean isBackground) {
        if (Utility.getNotifyFmt(mContext) == AppGlobalState.NOTIFY_FMT_STRUCT) {
            Utility.sendLogMesg(mContext, "Subscribe simple fmt=STRUCT based.");
            mSubscribePollingTask = new CapabilityPollingTask(
                    mContext,
                    contactIndex, isBackground).execute();
        }
    }

    private void invokeSubscribePolling(int contactIndex) {
        if (Utility.getNotifyFmt(mContext) == AppGlobalState.NOTIFY_FMT_STRUCT) {
            Utility.sendLogMesg(mContext, "Subscribe polling fmt=STRUCT based.");
            mSubscribePollingTask = new AvailabilityFetchTask(
                    mContext,
                    contactIndex).execute();
        }
    }

    private void initDoneButton() {
        Button doneButton = (Button) findViewById(R.id.Done);
        handleDoneButtonClick(doneButton);
    }

    private void handleDoneButtonClick(Button doneButton) {
        doneButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                ContactArrayAdapter<Contact> adapter = AppGlobalState
                        .getMainListAdapter();
                adapter.notifyDataSetChanged();

                updateMultiSelection();
                finish();
            }
        });

    }

    private Intent getStartIntentForSetFilterActivity(int contactIndex) {
        Intent i = new Intent();
        i.setClassName("com.qualcomm.qti.presenceapp",
                "com.qualcomm.qti.presenceapp.SubscribeFilter");
        i.putExtra("ContactIndex", contactIndex);
        return i;
    }

    private void setButtonClickToStartActivity(Button setFilterButton,
            final Intent i) {
        setFilterButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                startActivity(i);
            }
        });
    }

    private int getContactIndexFromIntent(Intent i) {
        if (i.hasExtra("ContactIndex") == true) {
            int index = i.getIntExtra("ContactIndex", 0);
            return index;
        } else
            return 0;
    }

    public int getIndexOfDisplayedContact() {
        return mContactIndex;
    }

    public void sendAvailabilityFetch(int index) {
        Log.d(TAG, "In sendAvailabilityFetch");
        final Contact c;

        ArrayList<Contact> contacts = AppGlobalState.getContacts();
        c = contacts.get(index);
        if (c.getNote().equals("VoLTE Contact")) {
            Log.d(TAG,
                    "Capability Polling is success as Contact is VOLTE... NOT SENDING AUTO AF");
        } else
        {
            Log.d(TAG, "Capability Polling is success as Contact is NOT VOLTE... So do no send AF ");
        }

        isSelected.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {
                // TODO Auto-generated method stub

                if (isSelected.isChecked())
                {
                    c.setMultiSelected(true);
                    Log.d("PRESENCE_UI", "CHECKED");
                }
                else {
                    c.setMultiSelected(false);
                    Log.d("PRESENCE_UI", "UNCHECKED");
                }
            }
        });

    }

    public void populateForm(int index) {
        Log.d(TAG, "populateForm");
        Contact c;

        ArrayList<Contact> contacts = AppGlobalState.getContacts();
        c = contacts.get(index);

        updateIcon(c);
        name.setText(c.getName());
        phone.setText("Phone: " + c.getPhone());
        isSelected.setChecked(c.isMultiSelected());
        Log.d(TAG, "c.isMultiSelected()" + c.isMultiSelected());

        publishStatus.setText("BasicStatus: " + c.getBasicStatus());

        timestamp.setText("Timestamp: " + c.getTimeStamp());
        isAudioSupported.setText("Audio: " + c.getAudio());
        isVideoSupported.setText("Video: " + c.getVideo());

        listContactUri.setText("ListContactUri: " + c.getListContactUri());
        listName.setText("ListName: " + c.getListName());
        listVersion.setText("ListVersion: " + c.getListVersion());
        listFullState.setText("ListFullState: " + c.getListFullState());
        resourceUri.setText("ResourceUri: " + c.getResourceUri());
        isVolteContact.setText("IsVolteContact: " + c.getIsVolteContact());
        resourceId.setText("ResourceId: " + c.getResourceId());
        resourceState.setText("ResourceState: " + c.getResourceState());
        resourceReason.setText("ResourceReason: " + c.getResourceReason());
        resourceCid.setText("ResourceCid: " + c.getResourceCid());
        contactUri.setText("ContactUri: " + c.getContactUri());
        description.setText("Chat: " + c.getDescription());
        version.setText("File Transfer: " + c.getVersion());
        serviceId.setText("ServiceId: " + c.getServiceId());
        audioCapabilities.setText("AudioCapabilities: " + c.getAudioCapabilities());
        videoCapabilities.setText("VideoCapabilities: " + c.getVideoCapabilities());

        if (c.getNote().equals("VoLTE Contact") && networkTypeLTE && vopsEnabled) {
            Log.d(TAG, "Capability Polling is success as Contact is  VOLTE So enable AF button");
            subscribePollingButton.setEnabled(true);
        }
        else if (c.getDescription().equals("true") && (networkTypeLTE || networkTypeEHRPD)) {
            Log.d(TAG, "Capability Polling is success as Contact is  IM So enable AF button");
            subscribePollingButton.setEnabled(true);
        }
        else if (c.getVersion().equals("true") && (networkTypeLTE || networkTypeEHRPD)) {
            Log.d(TAG, "Capability Polling is success as Contact is  FT So enable AF button");
            subscribePollingButton.setEnabled(true);
        } else
        {
            Log.d(TAG,
                    "Capability Polling is success as Contact is NOT VOLTE, IM ,FT... So disable Availability Fetch button ");
            subscribePollingButton.setEnabled(false);
        }

        if (AppGlobalState.getOperatorMode() == AppGlobalState.ATT_MODE)
        {
            Log.d(TAG, "AppGlobalState.ATT_MODE");
            subscribePollingButton.setEnabled(true);
        }

        updateCallButtons(c);

    }

    private void updateIcon(Contact c) {
        switch (c.getAvailability()) {
            case 0:
                icon.setImageResource(getResources().getIdentifier(
                        "icon", "drawable", getPackageName()));
                break;

            case 1:
                icon.setImageResource(getResources().getIdentifier(
                        "online", "drawable", getPackageName()));
                break;
        }
    }

    private String prepareCompleteUri(String phone) {
        SharedPreferences setting = Utility.getSharedPrefHandle(mContext,
                AppGlobalState.IMS_PRESENCE_MY_INFO);
        String uri1Value = setting.getString(mContext.getString(R.string.uri1text), "");
        String uri2Value = setting.getString(mContext.getString(R.string.uri2text), "");

        phone = "" + (uri1Value != null ? uri1Value : "") + phone;
        phone = phone + (uri2Value != null ? uri2Value : "");

        return phone;
    }

    private void triggerVTCall(String number) {
        try
        {
        String uri = prepareCompleteUri(number);
        Log.d(TAG, "Starting VT call to: " + uri);
        Intent intent = new Intent(Intent.ACTION_CALL, Uri.parse(uri));
        intent.putExtra("android.phone.extra.CALL_DOMAIN", 2); // RIL_CALL_DOMAIN_PS
        intent.putExtra("android.phone.extra.CALL_TYPE", 3); // RIL_CALL_TYPE_VT
        startActivity(intent);
        }catch (Exception e) {
            // TODO: handle exception
            Log.d(TAG, "triggerVTCall Exception "+e);
            Toast.makeText(this, "URI IS NOT SET", Toast.LENGTH_SHORT).show();
        }
    }

    private void triggerHDCall(String number) {
        try
        {
        String uri = prepareCompleteUri(number);
        Log.d(TAG, "Starting HD call to: " + uri);
        Intent intent = new Intent(Intent.ACTION_CALL, Uri.parse(uri));
        intent.putExtra("android.phone.extra.CALL_DOMAIN", 2); // RIL_CALL_DOMAIN_PS
        intent.putExtra("android.phone.extra.CALL_TYPE", 0); // RIL_CALL_TYPE_VOICE

        startActivity(intent);
        }catch (Exception e) {
            // TODO: handle exception
            Log.d(TAG, "triggerHDCall Exception "+e);
            Toast.makeText(this, "URI IS NOT SET", Toast.LENGTH_SHORT).show();
        }
    }

    private void updateCallButtons(final Contact c) {


        SharedPreferences setting = Utility.getSharedPrefHandle(mContext,
                AppGlobalState.IMS_PRESENCE_MY_INFO);

        String vtSupport = c.getVideo();
        String audioSupport = c.getAudio();

        Log.d(TAG, "peer vtSupport=" + vtSupport);

        Log.d(TAG, "peer audioSupport=" + audioSupport);


        audioButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                Log.d(TAG, "Set audio button for " + c.getPhone().trim());
                triggerHDCall(c.getPhone().trim());
            }
        });

        videoButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                Log.d(TAG, "Set video button for " + c.getPhone().trim());
                triggerVTCall(c.getPhone().trim());
            }
        });

        Log.d(TAG, "getBasicStatus = " + c.getBasicStatus());

        Log.d("PRESENCE_UI", " updateCallButtons... networkTypeLTE =" + networkTypeLTE);

        Log.d("PRESENCE_UI", " updateCallButtons... vopsEnabled =" + vopsEnabled);


        if (c.getBasicStatus().equalsIgnoreCase(
                getString(R.string.bs_open_text))) {

            if ((getString(R.string.sc_supported_text).equals(vtSupport)
                    || "Yes".equals(vtSupport)) && Settings.isMobile_data_enabled) {
                // Video supported .
                videoButton.setEnabled(true);

                Log.d(TAG, "Turn on Video button and Mobile data is enabled");

                if (!Settings.isVt_calling_enabled) {

                    Log.d(TAG,
                            "Turn off video button, my device do not support");
                    videoButton.setEnabled(false);
                }
            } else {
                Log.d(TAG,
                        "Turn off video button, peer device does not support or Mobile data is disabled");
                videoButton.setEnabled(false);
            }
        } else if (c.getBasicStatus().equalsIgnoreCase(
                getString(R.string.bs_closed_text))) {
            Log.d(TAG, "Turn off audio and video button, Basic status is \"CLOSE\" for peer");

            videoButton.setEnabled(false);
        }

        if (networkTypeLTE && vopsEnabled)
        {
            audioButton.setEnabled(true);
            Log.d("PRESENCE_UI", "Settings.isCapability_discovery_enable_valid "+Settings.isCapability_discovery_enable_valid);
            Log.d("PRESENCE_UI", "Settings.capabilityDiscoveryEnable "+Settings.capabilityDiscoveryEnable);
            if(Settings.isMobile_data_enabled && Settings.isCapability_discovery_enable_valid == true && Settings.capabilityDiscoveryEnable == false)
            {
                Log.d("PRESENCE_UI", "VideoEnabled");
                videoButton.setEnabled(true);
            }

        } else {
            Log.d("PRESENCE_UI",
                    " Disable HD call and VT in EHRPD... networkTypeLTE =" + networkTypeLTE);
            Log.d("PRESENCE_UI",
                    " Disable HD call and VT in EHRPD... vopsEnabled =" + vopsEnabled);
            audioButton.setEnabled(false);
            videoButton.setEnabled(false);
        }

        if(AppGlobalState.getOperatorMode() == AppGlobalState.ATT_MODE)
        {
            Log.d("PRESENCE_UI", "AppGlobalState.ATT_MODE");
            if(networkTypeLTE && vopsEnabled && Settings.volteUserOptInStatus)
            {
                if(c != null && c.getAudioCapabilities() != null && c.getAudioCapabilities().equals("true"))
                {
                    audioButton.setEnabled(true);
                }
                else
                {
                    audioButton.setEnabled(false);
                }
                if(c != null && c.getVideoCapabilities() != null && c.getVideoCapabilities().equals("true"))
                {
                    videoButton.setEnabled(true);
                }
                else
                {
                    videoButton.setEnabled(false);
                }
            }
            else
            {
                audioButton.setEnabled(false);
                videoButton.setEnabled(false);
            }
        }
    }

    @Override
    protected void onDestroy() {
        AppGlobalState.setContactInfo(null);
        super.onDestroy();
    }

    @Override
    protected void onStop() {
        AppGlobalState.setContactInfo(null);
        super.onStop();
        finish();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        if (hasFocus == true) {
            Log.d(TAG, "contactInfo: onWindowFocusChanged(True)");
            populateForm(mContactIndex);
        }
        super.onWindowFocusChanged(hasFocus);
    }
}
