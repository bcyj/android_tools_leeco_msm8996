/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

package com.qualcomm.presencelist;

import java.util.ArrayList;

import com.qualcomm.presencelist.R;
import com.qualcomm.presencelist.MainActivity.ContactArrayAdapter;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

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
    Button audioButton;
    Button unSubscribeButton;

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

        checkPreconditions();

        AppGlobalState.setContactInfo(this);

        mListenerHandler = AppGlobalState.getListenerHandler();

        initContactInfoResIds();

        populateForm(contactIndex);

        initButtons(contactIndex);
    }

    private void checkPreconditions() {
        if(Utility.isContactExcluded(mContactIndex)) {
            showErrorContactExcluded();
        } else if (Utility.isContactOptedOut(mContactIndex)) {
            showErrorContactOptedOut();
        }
    }

    private void showErrorContactOptedOut() {
        showDialog("This contact did not participate in Volte 2012 services.");
    }


    private void showErrorContactExcluded() {
        showDialog("This contact is special number and"+
                " hence excluded from presence contact list");
    }

    private void showDialog(String msg) {
        DialogInterface.OnClickListener dialogClickListener =
            new DialogInterface.OnClickListener() {
            @Override
                public void onClick(DialogInterface dialog, int which) {
                    switch (which){
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
        ArrayList<Contact> contacts =AppGlobalState.getContacts();
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

        icon = (ImageView)findViewById(R.id.icon);
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

        videoButton = (Button)findViewById(R.id.StartVideocall);
        audioButton = (Button)findViewById(R.id.StartAudiocall);
        unSubscribeButton = (Button)findViewById(R.id.UnSubscribe);
    }

    private void initButtons(int contactIndex) {

        initSubscribePollingButton(contactIndex);
        initSubscribeSimpleButton(contactIndex);
        initUnSubscribeButton(contactIndex);
        initDoneButton();
    }

    private void initUnSubscribeButton(int contactIndex) {
        Button unSubscribeButton = (Button)findViewById(R.id.UnSubscribe);
        handleUnSubscribeButtonClick(unSubscribeButton, contactIndex);
    }

    private void handleUnSubscribeButtonClick(Button unSubscribeButton,
            final int contactIndex) {
        unSubscribeButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                mUnSubscribeTask = (UnSubscribeTask) new UnSubscribeTask(
                    mContext,
                    contactIndex).execute();
            }
        });
    }

    private void initSubscribeSimpleButton(int contactIndex) {
        Button subscribeSimpleButton = (Button)findViewById(
                R.id.SubscribeSimple);
        handleSubscribeSimpleButtonClick(subscribeSimpleButton,
                contactIndex);
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
        Button subscribePollingButton = (Button)findViewById(
                R.id.SubscribePolling);
        handleSubscribePollingButtonClick(subscribePollingButton,
                contactIndex);
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
        if(Utility.getNotifyFmt(mContext) == AppGlobalState.NOTIFY_FMT_STRUCT) {
            Utility.sendLogMesg(mContext, "Subscribe simple fmt=STRUCT based.");
            mSubscribePollingTask = new SubscribeSimpleTask(
                    mContext,
                    contactIndex, isBackground).execute();
        } else {
            Utility.sendLogMesg(mContext, "Subscribe simple fmt=XML based.");
            mSubscribePollingTask = new SubscribeSimpleXMLTask(
                    mContext,
                    contactIndex, isBackground).execute();
        }


    }

    private void invokeSubscribePolling(int contactIndex) {
        if(Utility.getNotifyFmt(mContext) == AppGlobalState.NOTIFY_FMT_STRUCT) {
            Utility.sendLogMesg(mContext, "Subscribe polling fmt=STRUCT based.");
            mSubscribePollingTask = new SubscribePollingTask(
                    mContext,
                    contactIndex).execute();
        } else {
            Utility.sendLogMesg(mContext, "Subscribe polling fmt=XML based.");
            mSubscribePollingTask = new SubscribePollingXMLTask(
                    mContext,
                    contactIndex).execute();
        }
    }


    private void initDoneButton() {
        Button doneButton = (Button)findViewById(R.id.Done);
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
        i.setClassName("com.qualcomm.presencelist",
                "com.qualcomm.presencelist.SubscribeFilter");
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
        if(i.hasExtra("ContactIndex") == true) {
            int index = i.getIntExtra("ContactIndex", 0);
            return index;
        } else return 0;
    }

    public int getIndexOfDisplayedContact() {
        return mContactIndex;
    }

    public void populateForm(int index) {
        Log.d(TAG,"populateForm");
        Contact c;

        ArrayList<Contact> contacts =AppGlobalState.getContacts();
        c = contacts.get(index);

        updateIcon(c);
        name.setText(c.getName());
        phone.setText("Phone: " + c.getPhone());
        isSelected.setChecked(c.isMultiSelected());

        publishStatus.setText("BasicStatus: " + c.getBasicStatus());

        timestamp.setText("Timestamp: " + c.getTimeStamp());
        isAudioSupported.setText("Audio: " + c.getAudio());
        isVideoSupported.setText("Video: " + c.getVideo());

        listContactUri.setText("ListContactUri: " +c.getListContactUri());
        listName.setText("ListName: " +c.getListName());
        listVersion.setText("ListVersion: " + c.getListVersion());
        listFullState.setText("ListFullState: " + c.getListFullState());
        resourceUri.setText("ResourceUri: " + c.getResourceUri());
        isVolteContact.setText("IsVolteContact: " + c.getIsVolteContact());
        resourceId.setText("ResourceId: " + c.getResourceId());
        resourceState.setText("ResourceState: " + c.getResourceState());
        resourceReason.setText("ResourceReason: " + c.getResourceReason());
        resourceCid.setText("ResourceCid: " + c.getResourceCid());
        contactUri.setText("ContactUri: " + c.getContactUri());
        description.setText("Description: " + c.getDescription());
        version.setText("Version: " + c.getVersion());
        serviceId.setText("ServiceId: " + c.getServiceId());
        audioCapabilities.setText("AudioCapabilities: " + c.getAudioCapabilities());
        videoCapabilities.setText("VideoCapabilities: " + c.getVideoCapabilities());

        updateCallButtons(c);
    }

    private void updateIcon(Contact c) {
        switch(c.getAvailability()) {
            case 0:
                icon.setImageResource(getResources().getIdentifier(
                            "icon", "drawable",  getPackageName()));
                break;

            case 1:
                icon.setImageResource(getResources().getIdentifier(
                            "online", "drawable",  getPackageName()));
                break;
        }
    }

    private String prepareCompleteUri(String phone) {
        SharedPreferences setting = Utility.getSharedPrefHandle(mContext,
                    AppGlobalState.IMS_PRESENCE_MY_INFO);
        String uri1Value = setting.getString(mContext.getString(R.string.uri1text), "");
        String uri2Value = setting.getString(mContext.getString(R.string.uri2text), "");

        phone = ""+(uri1Value!= null?uri1Value:"")+phone;
        phone = phone+(uri2Value !=null?uri2Value:"");

        return phone;
    }

    private void triggerVTCall(String number) {
        String uri = prepareCompleteUri(number);
        Log.d(TAG,"Starting VT call to: "+uri);
        Intent intent = new Intent(Intent.ACTION_CALL, Uri.parse(uri));
        intent.putExtra("android.phone.extra.CALL_DOMAIN", 2); //RIL_CALL_DOMAIN_PS
        intent.putExtra("android.phone.extra.CALL_TYPE", 3);  // RIL_CALL_TYPE_VT
        startActivity(intent);
    }

    private void triggerHDCall(String number) {
        String uri = prepareCompleteUri(number);
        Log.d(TAG,"Starting HD call to: "+uri);
        Intent intent = new Intent(Intent.ACTION_CALL, Uri.parse(uri));
        intent.putExtra("android.phone.extra.CALL_DOMAIN", 2); //RIL_CALL_DOMAIN_PS
        intent.putExtra("android.phone.extra.CALL_TYPE", 0);  //RIL_CALL_TYPE_VOICE

        startActivity(intent);
    }

    private void updateCallButtons(final Contact c) {
        SharedPreferences setting = Utility.getSharedPrefHandle(mContext,
                AppGlobalState.IMS_PRESENCE_MY_INFO);

        String myInfoVideoSupported = setting.getString(this.getString(
                    R.string.isVideoSupportedtext), "false");
        String myInfoAudioSupported = setting.getString(this.getString(
                    R.string.isAudioSupportedtext), "false");

        String vtSupport = c.getVideo();
        String audioSupport = c.getAudio();

        Log.d(TAG, "peer vtSupport="+vtSupport);
        Log.d(TAG, "My video support="+myInfoVideoSupported);
        Log.d(TAG, "peer audioSupport="+audioSupport);
        Log.d(TAG, "My audio support="+myInfoAudioSupported);

        audioButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "Set audio button for "+c.getPhone().trim());
                triggerHDCall(c.getPhone().trim());
            }
        });

        videoButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "Set video button for "+c.getPhone().trim());
                triggerVTCall(c.getPhone().trim());
            }
        });

        if (c.getBasicStatus().equalsIgnoreCase(
                    getString(R.string.bs_open_text))) {

            if (getString(R.string.sc_supported_text).equals(vtSupport)
                    || "Yes".equals(vtSupport)) {
                // Video supported .
                videoButton.setEnabled(true);

                Log.d(TAG, "Turn on Video button");

                if (myInfoVideoSupported.equalsIgnoreCase("false")) {
                    Log.d(TAG,
                            "Turn off video button, my device do not support");
                    videoButton.setEnabled(false);
                }
            } else {
                Log.d(TAG,
                        "Turn off video button, peer device does not support");
                videoButton.setEnabled(false);
            }

            if (getString(R.string.sc_supported_text).equals(audioSupport)
                    || "Yes".equals(audioSupport)) {
                // audio supported .
                audioButton.setEnabled(true);

                Log.d(TAG, "Turn on Audio button");
                if (myInfoAudioSupported.equalsIgnoreCase("false")
                        || myInfoAudioSupported.equalsIgnoreCase("No")) {
                    Log.d(TAG,
                            "Turn off audio button, my device do not support");
                    audioButton.setEnabled(false);
                }
            } else {
                Log.d(TAG,
                        "Turn off audio button, peer device does not support");
                audioButton.setEnabled(false);
            }
        } else if (c.getBasicStatus().equalsIgnoreCase(
                    getString(R.string.bs_closed_text))) {
            Log.d(TAG, "Turn off audio and video button, Basic status is \"CLOSE\" for peer");
            audioButton.setEnabled(false);
            videoButton.setEnabled(false);
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
        if(hasFocus == true) {
            Log.d(TAG, "contactInfo: onWindowFocusChanged(True)");
            populateForm(mContactIndex);
        }
        super.onWindowFocusChanged(hasFocus);
    }
}
