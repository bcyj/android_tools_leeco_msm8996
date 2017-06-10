/*************************************************************************
 Copyright (c)2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
*************************************************************************/

package com.qualcomm.qti.presenceapp;

import java.util.ArrayList;

import com.qualcomm.qti.rcsservice.StatusCode;
import android.app.ProgressDialog;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Looper;
import android.os.RemoteException;
import android.util.Log;
import android.widget.Toast;

public class AvailabilityFetchTask extends AsyncTask<Void, Integer, Integer> {
    final String TAG = "SubscribePollingTask";
    static final int AVAILABILITY_FETCH_NOT_SENT = 5;

    int mContactIndex;
    Context mContext;
    ProgressDialog dialog;

    AvailabilityFetchTask me;

    public AvailabilityFetchTask() {
    }

    public AvailabilityFetchTask(Context appContext, int contactIndex) {

        mContext = appContext;
        mContactIndex = contactIndex;
        me = this;
    }

    @Override
    protected void onPreExecute() {
        super.onPreExecute();

        initProgressBar();
    }

    private void initProgressBar() {
        dialog = new ProgressDialog(mContext);
        dialog.setMessage("API Request in progress...");
        dialog.setCancelable(true);

        AppGlobalState.setProgressDialog(dialog);

        dialog.show();

    }

    @Override
    protected Integer doInBackground(Void... params) {
        Log.d(TAG, "doInBackground(), Thread=" + Thread.currentThread().getName());

        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        ArrayList<Contact> contacts = AppGlobalState.getContacts();
        String phone = contacts.get(mContactIndex).getPhone();
        Log.d("PRESENCE_UI", "phone" + phone);

        String uri = contacts.get(mContactIndex).getContactUri();
        Log.d("PRESENCE_UI", "Contact URI" + uri);

        if (uri.length() == 0)
        {
            Log.d("PRESENCE_UI",
                    "getContactUri is empty, So use getResourceUri for Availability fetch");
            uri = contacts.get(mContactIndex).getResourceUri();
        }

        if (uri.length() > 0) {
            ArrayList<String> contactList = new ArrayList<String>();
            if (AppGlobalState.getOperatorMode() == AppGlobalState.ATT_MODE && uri.equals("<Not Subscribed>"))
            {
                if(!contacts.get(mContactIndex).getIndividualContactURI().equals(""))
                {
                    uri = contacts.get(mContactIndex).getIndividualContactURI();
                }
                else
                {
                    uri = "tel:"+phone;
                }
                Log.d(TAG, "ATT Not Subscribe uri");
            }
            contactList.add(uri);

            return sendSubscribePollingRequest(contactList);
        } else {
            return AVAILABILITY_FETCH_NOT_SENT;
        }
    }

    private Integer sendSubscribePollingRequest(ArrayList<String> contactList) {
        Log.d(TAG, "aks2 sendSubscribePollingRequest for " + contactList.get(0));

        try {
            if(AppGlobalState.getPresenceService()!=null)
            {
                Log.d("PRESENCE_UI", "aks2 AvailabilityFetchTask : sendSubscribePollingRequest : AppGlobalState.getPresenceSerrviceHandle() "+AppGlobalState.getPresenceSerrviceHandle());
                Log.d("PRESENCE_UI", "AvailabilityFetchTask : sendSubscribePollingRequest : contactList.get(0) "+contactList.get(0));
                RequestInfo requestinfoObject = new RequestInfo();
                requestinfoObject.URI = new String[1];
                requestinfoObject.URI[0] = contactList.get(0);
                requestinfoObject.userData = AppGlobalState.getpUserDataValue();
                AppGlobalState.requestinfo.add(requestinfoObject);
                StatusCode status = AppGlobalState.getPresenceService().QPresService_GetContactCap(AppGlobalState.getPresenceSerrviceHandle(), contactList.get(0), AppGlobalState.getpUserData());
                Log.d("PRESENCE_UI", "AvailabilityFetchTask : sendSubscribePollingRequest : status "+status.getStatusCode().toString());
            return status.getStatusCode().ordinal();
            }
            else
            {
                Log.d("PRESENCE_UI", "AvailabilityFetchTask : sendSubscribePollingRequest : return o");
                return -1;
            }
        } catch (RemoteException e) {
            e.printStackTrace();
            return -2;
        } catch (Exception e) {
            e.printStackTrace();
            return -3;
        }
    }

    @Override
    protected void onProgressUpdate(Integer... values) {
        super.onProgressUpdate(values);
    }

    @Override
    protected void onPostExecute(Integer result) {
        super.onPostExecute(result);
        Log.d(TAG, "onPostExecute(), Thread=" + Thread.currentThread().getName());

        dialog.dismiss();
        if (result != 0) { // failure
            // dialog.dismiss();
        } else { // success
            updateContactSubscriptionFlag();
            Utility.rescheduleSubcribeTimer(mContactIndex,
                    AppGlobalState.getContacts().get(mContactIndex));
        }

        if (result == AVAILABILITY_FETCH_NOT_SENT)
        {
            Toast.makeText(mContext, "Availability fetch not send due to empty URI",
                    Toast.LENGTH_SHORT).show();
        } else {

            Toast.makeText(mContext, "Subscribe Polling Result =" +
                    result, Toast.LENGTH_SHORT).show();
        }
    }

    private void updateContactSubscriptionFlag() {

        ArrayList<Contact> contacts = AppGlobalState.getContacts();

        Contact temp = contacts.get(mContactIndex);
        temp.setSubscriptionOnFlag(false);
    }
}
