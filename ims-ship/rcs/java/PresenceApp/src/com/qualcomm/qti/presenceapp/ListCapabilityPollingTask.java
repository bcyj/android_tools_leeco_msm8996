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

public class ListCapabilityPollingTask extends AsyncTask<Void, Integer, Integer> {
    final String TAG = "ListSubscribeSimpleTask";

    public static String prefix = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
            + "<resource-lists xmlns=\"urn:ietf:params:xml:ns:resource-lists\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
            + "<list name=\"dummy-rfc5367\">";
    public static String suffix = "</list>" + "</resource-lists>";
    Context mContext;
    ProgressDialog dialog;

    public ListCapabilityPollingTask() {
    }

    public ListCapabilityPollingTask(Context appContext) {

        mContext = appContext;
    }

    @Override
    protected void onPreExecute() {
        super.onPreExecute();

        initProgressBar();
    }

    private void initProgressBar() {
        dialog = new ProgressDialog(mContext);
        dialog.setMessage("API Request in progress...");
        dialog.setCancelable(false);

        AppGlobalState.setProgressDialog(dialog);

        dialog.show();

    }

    @Override
    protected Integer doInBackground(Void... params) {
        Log.d(TAG, "doInBackground(), Thread=" + Thread.currentThread().getName());

        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        ArrayList<String> uriList = getSelectedContactsList();

        if (uriList.size() == 0) {
            Log.d(TAG, "None of the contact selected for list subscription");
            return 0;
        }

        return sendListSubscribePollingRequest(uriList);
    }

    private ArrayList<String> getSelectedContactsList() {
        ArrayList<Contact> contacts = AppGlobalState.getContacts();
        ArrayList<String> uriList = new ArrayList<String>();
        for (Contact c : contacts) {
            if (c.isMultiSelected()) {
                String phone = c.getPhone();
                if (AppGlobalState.getOperatorMode() == AppGlobalState.VZW_MODE)
                {
                    uriList.add(prepareCompleteUri(phone));
                }
                else
                {
                    uriList.add(prepareCompleteUriForAtt(c));
                }
            }
        }
        return uriList;
    }



    private String prepareCompleteUri(String phone) {
        // Modified to enable sip uri. need to update back to tell uri.
        phone = "tel:" + phone;
        return phone;
    }

    private String prepareCompleteUriForAtt(Contact contact) {
        // Modified to enable sip uri. need to update back to tell uri.
        String phone = contact.getContactUri();

        if((phone == null) || (phone != null && phone.length() == 0))
        {
            phone = contact.getResourceUri();

        }
        if(phone != null && phone.length() > 0)
        {
            if(phone.equals("<Not Subscribed>"))
            {
                if(!contact.getIndividualContactURI().equals(""))
                {
                    phone = contact.getIndividualContactURI();
                }
                else
                {
                    phone = "tel:" + contact.getPhone();
                }
            }
        }
        else
        {
            if(!contact.getIndividualContactURI().equals(""))
            {
                phone = contact.getIndividualContactURI();
            }
            else
            {
                phone = "tel:" + contact.getPhone();
            }
        }

        return phone;
    }

    private Integer sendListSubscribePollingRequest(ArrayList<String> contactList) {
        Log.d(TAG, "sendListSubscribePollingRequest for " + contactList);

        String [] stringContactList = null;

        int standardXmlLength = Settings.maxSubscriptionListEntries;
        Log.d("PRESENCE_UI", "standardXmlLength =  " + standardXmlLength);
        int noOfIterations;
        if(standardXmlLength > 0)
        {
            noOfIterations = contactList.size() / standardXmlLength;
        }
        else
        {
            noOfIterations = contactList.size();
        }

        Log.d("PRESENCE_UI", "noOfIterations =  " + noOfIterations);
        int contactPosition = 0;
        for (int count = 0; count <= noOfIterations; count++) {
            Log.d("PRESENCE_UI", "count =  " + count);
            if (count == noOfIterations)
            {
                standardXmlLength = contactList.size() % standardXmlLength;
                Log.d("PRESENCE_UI", "standardXmlLength =  " + standardXmlLength);
            }
            stringContactList = new String[standardXmlLength];

            for (int i = 0; i < standardXmlLength; i++) {

                Log.d("PRESENCE_UI", "contactPosition =  " + contactPosition);
                stringContactList[i] = contactList.get(contactPosition);
                Log.d("PRESENCE_UI", "stringContactList[i] =  " + stringContactList[i]);
                contactPosition++;
            }
            if (count != noOfIterations) {

                Log.d("PRESENCE_UI", "Before QPresService_GetContactListCap Inside");
                try {
                if(AppGlobalState.getPresenceService() != null)
                {
                    Log.d("PRESENCE_UI", "ListCapabilityPollingTask : sendListSubscribePollingRequest : AppGlobalState.getPresenceSerrviceHandle() "+AppGlobalState.getPresenceSerrviceHandle());
                    Log.d("PRESENCE_UI", "ListCapabilityPollingTask : sendListSubscribePollingRequest : stringContactList "+stringContactList.toString());
                    RequestInfo requestinfoObject = new RequestInfo();
                    requestinfoObject.URI = new String[stringContactList.length];
                    for(int i =0; i < stringContactList.length; i++ )
                    {
                        requestinfoObject.URI[i] = stringContactList[i];
                    }
                    requestinfoObject.userData = AppGlobalState.getpUserDataValue();
                    AppGlobalState.requestinfo.add(requestinfoObject);
                    StatusCode status = AppGlobalState.getPresenceService().QPresService_GetContactListCap(AppGlobalState.getPresenceSerrviceHandle(), stringContactList, AppGlobalState.getpUserData());
                    Log.d("PRESENCE_UI", "ListCapabilityPollingTask : sendListSubscribePollingRequest : status.getStatusCode() "+status.getStatusCode());
                }
                else
                {
                    Log.d("PRESENCE_UI", "ListCapabilityPollingTask : sendListSubscribePollingRequest : AppGlobalState.getPresenceService =  NULL");
                    return -1;
                }
            } catch (RemoteException e) {
                e.printStackTrace();
                return -2;
            } catch (Exception e) {
                e.printStackTrace();
                return -3;
            }
                Log.d("PRESENCE_UI", "After QPresService_GetContactListCap");
            }

        }
        if(stringContactList != null)
        {
            Log.d("PRESENCE_UI", "stringContactList " + stringContactList.length);
            if (stringContactList.length <= 0)
            {
                return 0;
            }
            else
            {
                Log.d("PRESENCE_UI", "Before RETURN QPresService_GetContactListCap");
                try {
                if(AppGlobalState.getPresenceService() != null)
                {
                    Log.d("PRESENCE_UI", "ListCapabilityPollingTask : sendListSubscribePollingRequest : AppGlobalState.getPresenceSerrviceHandle() "+AppGlobalState.getPresenceSerrviceHandle());
                    Log.d("PRESENCE_UI", "ListCapabilityPollingTask : sendListSubscribePollingRequest : stringContactList "+stringContactList.toString());
                    RequestInfo requestinfoObject = new RequestInfo();
                    requestinfoObject.URI = new String[stringContactList.length];
                    for(int i =0; i < stringContactList.length; i++ )
                    {
                        requestinfoObject.URI[i] = stringContactList[i];
                    }
                    requestinfoObject.userData = AppGlobalState.getpUserDataValue();
                    AppGlobalState.requestinfo.add(requestinfoObject);
                    StatusCode status = AppGlobalState.getPresenceService().QPresService_GetContactListCap(AppGlobalState.getPresenceSerrviceHandle(), stringContactList, AppGlobalState.getpUserData());
                    Log.d("PRESENCE_UI", "ListCapabilityPollingTask : sendListSubscribePollingRequest : status.getStatusCode() "+status.getStatusCode());
                    return status.getStatusCode().ordinal();
                }
                else
                {
                    Log.d("PRESENCE_UI", "ListCapabilityPollingTask : sendListSubscribePollingRequest : AppGlobalState.getPresenceService =  NULL");
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
        }
        else
        {
        return -4;
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

        if (getSelectedContactsList().size() == 0) {
            Toast.makeText(mContext, "None of the contact selected for list subscription.",
                    Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(mContext, "List Subscribe Polling Result =" +
                    result, Toast.LENGTH_SHORT).show();
        }
    }
}
