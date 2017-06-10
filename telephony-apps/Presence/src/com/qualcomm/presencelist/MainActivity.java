/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

package com.qualcomm.presencelist;


import java.util.ArrayList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import com.qualcomm.presencelist.R;
import com.qualcomm.qcrilhook.PresenceOemHook;

import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.HandlerThread;
import android.os.Looper;
import android.provider.ContactsContract;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

public class MainActivity extends ListActivity {
    private final static String TAG = "MainActivity";

    public Context mContext;
    ArrayList<Contact> mContacts;

    ContactArrayAdapter<Contact> mAdapter;
    ListenerHandler mListenerHandler;

    AsyncTask mPublishTask;
    AsyncTask mSubscribePollingTask;
    AsyncTask mImsEnablerTask;
    AsyncTask mUnPublishTask;

    public class ContactArrayAdapter<T> extends ArrayAdapter<T> {

        int layoutRes;
        ArrayList<Contact> contacts;

        public ContactArrayAdapter(Context context, int resource,
                int textViewResourceId, List<T> objects) {
            super(context, resource, textViewResourceId, objects);
            layoutRes = resource;
            contacts = (ArrayList<Contact>) objects;
        }

        @Override
            public View getView(int position, View convertView, ViewGroup parent) {
                LinearLayout rowView;

                if(convertView == null) {
                    rowView = new LinearLayout(getContext());

                    String inflater=Context.LAYOUT_INFLATER_SERVICE;
                    LayoutInflater vi=(LayoutInflater)getContext()
                        .getSystemService(inflater);
                    vi.inflate(layoutRes,rowView,true);
                } else {
                    rowView = (LinearLayout) convertView;
                }

                TextView nameView = (TextView) rowView.findViewById(R.id.name);
                TextView phoneView = (TextView) rowView.findViewById(R.id.phone);
                TextView noteView = (TextView) rowView.findViewById(R.id.Note);
                ImageView bmp = (ImageView) rowView.findViewById(R.id.icon);
                ImageView isMultiSelectedIcon = (ImageView) rowView
                    .findViewById(R.id.is_selected_icon);
                ImageView isExcludedIcon = (ImageView) rowView
                    .findViewById(R.id.is_excluded_icon);



                nameView.setText(contacts.get(position).getName());
                phoneView.setText(contacts.get(position).getPhone());
                noteView.setText(contacts.get(position).getNote());

                updateIcon(bmp, position);
                updateExcludedIcon(isExcludedIcon, false);
                if(contacts.get(position).isContactExcluded()) {
                    updateExcludedIcon(isExcludedIcon, true);
                    updateMultiSelectedIcon(isMultiSelectedIcon,false);

                } else {
                    updateMultiSelectedIcon(isMultiSelectedIcon,
                            contacts.get(position).isMultiSelected());

                }

                return rowView;
            }

        private void updateExcludedIcon(ImageView isExcludedIcon,
                boolean visible) {
            if(visible) {
                isExcludedIcon.setVisibility(View.VISIBLE);
            } else {
                isExcludedIcon.setVisibility(View.INVISIBLE);
            }


        }

        private void updateMultiSelectedIcon(ImageView icon,
                boolean multiSelected) {
            if(multiSelected) {
                icon.setVisibility(View.VISIBLE);
            } else {
                icon.setVisibility(View.INVISIBLE);
            }
        }

        private void updateIcon(ImageView bmp, int position) {
            switch(contacts.get(position).getAvailability()) {
                case 0:
                    bmp.setImageResource(getResources().getIdentifier("icon",
                                "drawable",  getPackageName()));
                    break;

                case 1:
                    bmp.setImageResource(getResources().getIdentifier("online",
                                "drawable",  getPackageName()));
                    break;

                case 2:
                    bmp.setImageResource(getResources().getIdentifier("busy",
                                "drawable",  getPackageName()));
                    break;

                default:
                    bmp.setImageResource(getResources().getIdentifier("icon",
                                "drawable",  getPackageName()));
            }
        }
    }

    private SharedPreferences getSharedPrefHandle(String imsPresencePref) {
        SharedPreferences settings = getSharedPreferences(imsPresencePref, 0);

        return settings;
    }


    @Override
        protected void onRestart() {
            Log.d(TAG, "MainActivity onRestart hit");
            super.onRestart();
        }


    @Override
        protected void onResume() {
            Log.d(TAG, "MainActivity onResume hit");
            super.onResume();
        }


    /** Called when the activity is first created. */
    @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            mContext = this;

            Utility.initLiveLogging(mContext);
            Utility.initNotifyXMLFile(mContext);

            Log.d(TAG, "MainActivity onCreate hit");
            sendLogMesg(mContext, "******************************");
            sendLogMesg(mContext, "Presence Application Starting");


            int dummyContactCount = getIntent().getIntExtra("COUNT", 0);
            Log.d(TAG, "dummyContactCount="+dummyContactCount);
            if (dummyContactCount == 0) {
                mContacts  = getContactsFromDB();
            } else {
                mContacts = getDummyContacts(dummyContactCount);
            }

            markExcludedContacts();


            AppGlobalState.setMainActivityContext(mContext);

            if (AppGlobalState.getTimerManager() == null) {
                Timer timerManager = new Timer();
                AppGlobalState.setTimerManager(timerManager);
            }

            AppGlobalState.setContacts(mContacts);

            linkArrayWithDisplayedList(mContacts);

            createListeningThread();

            SharedPreferences setting = getSharedPrefHandle(
                    AppGlobalState.IMS_PRESENCE_MY_INFO);
            AppGlobalState.setMyInfoSettingHandle(setting);


            PresenceOemHook p = PresenceOemHook.getInstance(
                    mContext, AppGlobalState.getListenerHandler());
            AppGlobalState.setPresenceOemHook(p);




            showVolteParticipation();

        }



    private void markExcludedContacts() {
        Utility.prepareExcludedContactList();
        for(Contact c: mContacts) {
            if( Utility.isContactExcluded(c)) {
                c.setContactExcluded();
            }
        }

    }

    @Override
        protected void onPause() {
            Log.d(TAG, "MainActivity onPause()");

            super.onPause();
        }

    @Override
        protected void onStop() {
            Log.d(TAG, "MainActivity onStop()");
            if(false) {
                // UT Only.
                for(Contact c: AppGlobalState.getContacts()) {
                    TimerTask  t= c.getSubscribeTimerTask();

                    if( t != null ) {
                        t.cancel();
                    }
                }
            }
            super.onStop();
        }

    private ArrayList<Contact> getDummyContacts(int dummyContactCount) {
        ArrayList<Contact> contacts = new ArrayList();
        for(int i =0;i<dummyContactCount; i++) {
            Contact c = new Contact("Test."+i, "555"+i, 0, "");
            contacts.add(c);
        }
        return contacts;
    }

    private void showVolteParticipation() {

        DialogInterface.OnClickListener dialogClickListener = new DialogInterface
            .OnClickListener() {

            @Override
                public void onClick(DialogInterface dialog, int which) {
                    switch (which){
                        case DialogInterface.BUTTON_POSITIVE:
                            //Yes button clicked
                            break;

                        case DialogInterface.BUTTON_NEGATIVE:
                            finish();
                            break;
                    }
                }

        };

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("Do you want to participate in Vzw VoLTE 2012 services?")
            .setPositiveButton("Yes", dialogClickListener)
            .setNegativeButton("No", dialogClickListener)
            .setCancelable(false)
            .show();

    }

    public void onDestroy() {
        Log.d(TAG, "onDestroy hit");
        //clean up oem hook
        PresenceOemHook p = AppGlobalState.getPresenceOemHook();
        p.dispose();
        AppGlobalState.setPresenceOemHook(null);

        stopTimers();

        Utility.closeLiveLoggingFile();
        Utility.closeNotifyXMLFile();

        super.onDestroy();

    }

    private void stopTimers() {
        Log.d(TAG, "Stop timers");
        for (Contact c : AppGlobalState.getContacts()) {
            TimerTask t = c.getSubscribeTimerTask();

            if (t != null) {
                t.cancel();
            }
        }

    }

    private void createListeningThread() {

        if (AppGlobalState.getListenerLooper() == null) {
            HandlerThread listenerThread = new HandlerThread("Listener",
                    android.os.Process.THREAD_PRIORITY_BACKGROUND);

            listenerThread.start();
            Looper listenerLooper = listenerThread.getLooper();
            mListenerHandler = new ListenerHandler(mContext, listenerLooper);

            AppGlobalState.setListenerHandler(mListenerHandler);
            AppGlobalState.setListenerLooper(listenerLooper);

        }
    }

    @Override
        public boolean onCreateOptionsMenu(Menu menu) {
            super.onCreateOptionsMenu(menu);
            getMenuInflater().inflate(R.menu.menu, menu);
            return true;
        }

    @Override
        public boolean onOptionsItemSelected(MenuItem item) {
            switch (item.getItemId()) {

                case R.id.ims_enabler:
                    sendLogMesg(mContext, "Menu option IMS Enabler selected.");
                    mImsEnablerTask =  new ImsEnablerTask(mContext).execute();
                    return true;

                case R.id.publish_rich:
                    sendLogMesg(mContext, "Menu option Publish Rich selected.");
                    invokePublish();
                    return true;

                case R.id.listsubscribepolling:
                     invokeListSubscribePolling();

                    return true;

                case R.id.listsubscribesimple:
                    invokeListSubscribeSimple();
                    return true;

                case R.id.unpublish:
                    sendLogMesg(mContext, "Menu option UnPublish selected.");
                    invokeUnPublish();

                    return true;

                case R.id.myinfo:
                    sendLogMesg(mContext, "Menu option MyInfo selected.");
                    startMyInfoActivity();
                    return true;

                case R.id.settings:
                    sendLogMesg(mContext, "Menu option Settings selected.");
                    startSettingsActivity();
                    return true;

                case R.id.select_all:
                    sendLogMesg(mContext, "Menu option SelectAll selected.");
                    selectAllContacts(true);
                    return true;

                case R.id.select_none:
                    sendLogMesg(mContext, "Menu option SelectNone selected.");
                    selectAllContacts(false);
                    return true;

                case R.id.live_logging:
                    startLiveLoggingActivity();
                    return true;
            }
            return false; //should never happen
        }

    private void invokeUnPublish() {
        if(Utility.getNotifyFmt(mContext) == AppGlobalState.NOTIFY_FMT_STRUCT) {
            sendLogMesg(mContext, "UnPublish fmt=STRUCT based.");
            mUnPublishTask =  new UnPublishTask(mContext).execute();
        } else {
            sendLogMesg(mContext, "UnPublish fmt=XML based.");
            mUnPublishTask =  new UnPublishXMLTask(mContext).execute();
        }

    }

    private void invokePublish() {

        if(Utility.getNotifyFmt(mContext) == AppGlobalState.NOTIFY_FMT_STRUCT) {
            sendLogMesg(mContext, "Publish fmt=STRUCT based.");
            mPublishTask = new PublishTask().execute();
        } else {
            sendLogMesg(mContext, "Publish fmt=XML based.");
            mPublishTask = new PublishXMLTask().execute();
        }

    }

    private void invokeListSubscribePolling() {
        if(Utility.getNotifyFmt(mContext) == AppGlobalState.NOTIFY_FMT_STRUCT) {
            sendLogMesg(mContext, "List subscribe polling fmt=STRUCT based.");
            mSubscribePollingTask =  new ListSubscribePollingTask(mContext).execute();
        } else {
            sendLogMesg(mContext, "List subscribe polling fmt=XML based.");
            mSubscribePollingTask = new SubscribePollingXMLTask(
                    mContext,
                    0).execute();
        }
    }

    private void invokeListSubscribeSimple() {
        if(Utility.getNotifyFmt(mContext) == AppGlobalState.NOTIFY_FMT_STRUCT) {
            sendLogMesg(mContext, "List subscribe simple fmt=STRUCT based.");
            mSubscribePollingTask =  new ListSubscribeSimpleTask(mContext).execute();
        } else {
            sendLogMesg(mContext, "List subscribe simple fmt=XML based.");
            mSubscribePollingTask = new SubscribePollingXMLTask(
                    mContext,
                    0).execute();
        }
    }

    private void selectAllContacts(boolean flag) {
        for(Contact c :AppGlobalState.getContacts()){
            if(Utility.isContactExcluded(c)) {
                c.setContactExcluded();
                c.setMultiSelected(false);
            } else {
                c.setMultiSelected(flag);
            }
        }
        AppGlobalState.getMainListAdapter().notifyDataSetChanged();
    }

    private void sendLogMesg(Context c, String string) {

        Utility.sendLogMesg(c, string);

    }

    private void startPublishActivity() {
        Intent i = new Intent();
        i.setClassName("com.qualcomm.presencelist",
                "com.qualcomm.presencelist.Publish");
        startActivity(i);
    }

    private void startMyInfoActivity() {
        Intent i = new Intent();
        i.setClassName("com.qualcomm.presencelist",
                "com.qualcomm.presencelist.MyInfo");
        startActivity(i);
    }

    private void startParActivity() {
        Intent i = new Intent();
        i.setClassName("com.qualcomm.presencelist",
                "com.qualcomm.presencelist.Par");
        startActivity(i);
    }

    private void startSettingsActivity() {
        Intent i = new Intent();
        i.setClassName("com.qualcomm.presencelist",
                "com.qualcomm.presencelist.Settings");
        startActivity(i);

    }

    private void dumpContacts(ArrayList<Contact> contacts) {
        for(Contact c : contacts) {
            Log.d(TAG, "Contact="+c);
        }
    }

    private void startLiveLoggingActivity() {
        Intent i = new Intent();
        i.setClassName("com.qualcomm.presencelist",
                "com.qualcomm.presencelist.LiveLoggingActivity");
        startActivity(i);

    }

    private void linkArrayWithDisplayedList(ArrayList<Contact> contacts) {
        mAdapter = new ContactArrayAdapter<Contact>(this,
                R.layout.custom, R.id.name,contacts);

        setListAdapter(mAdapter);

        AppGlobalState.setMainListAdapter(mAdapter);
    }

    @Override
        protected void onListItemClick(ListView l, View v,
                int position, long id) {
            super.onListItemClick(l, v, position, id);
            Log.d(TAG, "selected="+position);
            Log.d(TAG, "Contact="+mContacts.get(position));
            startContactInfoActivity(position);
        }

    private void startContactInfoActivity(int index) {
        Intent i = new Intent();
        i.setClassName("com.qualcomm.presencelist",
                "com.qualcomm.presencelist.ContactInfo");
        i.putExtra("ContactIndex", index);

        sendLogMesg(mContext, "Starting ContactInfo");

        startActivity(i);
    }

    private ArrayList<Contact> getContactsFromDB() {
        ArrayList<Contact> contacts = new ArrayList<Contact>();

        /* 1. Use the contacts.content_uri to get the id of the contacts,
         * 2. then query data.content_uri to get name and phone number.
         * 3. Update the arrayList contacts.
         */
        /*TODO: Get a better way to get name and phone number from phoneDB.
         *
         */


        Cursor contactsCursor = getContactsContentCursor();

        for (int i=0;contactsCursor.moveToNext();i++) {
            String id = getContactIdFromCursor(contactsCursor);

            Cursor dataCursor = getDataCursorRelatedToId(id);
            //DatabaseUtils.dumpCursor(dataCursor);

            populateContactDataFromCursor(contacts,dataCursor );

            dataCursor.close();
        }

        return contacts;
    }

    private String getContactIdFromCursor(Cursor contactsCursor) {
        String id = contactsCursor.getString(contactsCursor
                .getColumnIndex(ContactsContract.Contacts._ID));
        return id;
    }

    private void populateContactDataFromCursor(ArrayList<Contact> contacts,
            Cursor dataCursor) {
        int nameIdx = dataCursor
            .getColumnIndex(ContactsContract.Data.DISPLAY_NAME);
        int phoneIdx = dataCursor
            .getColumnIndex(ContactsContract.CommonDataKinds.Phone.NUMBER);

        if (dataCursor.moveToFirst()) {
            // Extract the name.
            String name = dataCursor.getString(nameIdx);
            // Extract the phone number.
            String rawNumber = dataCursor.getString(phoneIdx);
            String number = getNormalizedNumber(rawNumber);

            contacts.add(new Contact(name, number, 0,"<Not Subscribed>"));
        }

    }

    private String getNormalizedNumber(String rawNumber) {
        Log.d(TAG, "RawNumber from phonebook= " + rawNumber);
        /* remove "(", ")", "-" and "." from the number.
         * Only integers or + is allowed.
         */
        int len = rawNumber.length();
        String out = new String();

        if(rawNumber.charAt(0) == '+') {
            out = "+";
        }

        for(int i =0; i<len;i++) {
            char c = rawNumber.charAt(i);
            if (Character.isDigit(c)) {
                out += c;
            }
        }
        Log.d(TAG, "Normalized number= " + out);
        return out;
    }

    private Cursor getDataCursorRelatedToId(String id) {
        String where = ContactsContract.Data.CONTACT_ID + " = " + id;


        Cursor dataCursor = getContentResolver().query(
                ContactsContract.Data.CONTENT_URI, null, where, null, null);
        return dataCursor;
    }

    private Cursor getContactsContentCursor() {
        Uri phoneBookContentUri = ContactsContract.Contacts.CONTENT_URI;
        String recordsWithPhoneNumberOnly = ContactsContract.Contacts.HAS_PHONE_NUMBER
            + "='1'";

        Cursor contactsCursor = managedQuery(phoneBookContentUri, null,
                recordsWithPhoneNumberOnly, null, null);
        return contactsCursor;
    }

}
