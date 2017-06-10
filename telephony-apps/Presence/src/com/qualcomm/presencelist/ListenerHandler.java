/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

package com.qualcomm.presencelist;
import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import com.qualcomm.presencelist.MainActivity.ContactArrayAdapter;
import com.qualcomm.qcrilhook.PresenceMsgParser;
import com.qualcomm.qcrilhook.PresenceOemHook;

import com.qualcomm.qcrilhook.PresenceOemHook.PresenceUnsolIndication;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;


public class ListenerHandler extends Handler {

    private static String TAG = "ListenerHandler";
    Context mContext;

    public ListenerHandler(Context context, Looper looper) {
        super (looper);
        mContext = context;
    }

    @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            Log.d(TAG, "Thread=" + Thread.currentThread().getName() + " received "
                    + msg);

            Object obj = PresenceOemHook.handleMessage(msg);
            if (obj == null) {
                Log.d(TAG, "Notify update with NULL data, ignore it");
                msg.what = 0; //special mesg
                uiThreadHandler.sendEmptyMessage(msg.what);
                return;
            }

            PresenceUnsolIndication ind = (PresenceUnsolIndication) obj;

            switch (ind.oemHookMesgId) {

                case PresenceOemHook.QCRILHOOK_PRESENCE_IMS_UNSOL_NOTIFY_UPDATE:
                {
                    ArrayList<PresenceMsgParser.ContactInfo> parsedContactList =
                        (ArrayList<PresenceMsgParser.ContactInfo>) ind.obj;
                    updateStatus(parsedContactList);
                    Message updateUiMesg = uiThreadHandler
                        .obtainMessage(
                                PresenceOemHook.QCRILHOOK_PRESENCE_IMS_UNSOL_NOTIFY_UPDATE,
                            parsedContactList);
                    uiThreadHandler.sendMessage(updateUiMesg);
                }
                break;

                case PresenceOemHook.QCRILHOOK_PRESENCE_IMS_UNSOL_ENABLER_STATE:
                {
                    int enabler_state = (Integer) ind.obj;

                    Message updateUiMesg = uiThreadHandler
                        .obtainMessage(
                                PresenceOemHook.QCRILHOOK_PRESENCE_IMS_UNSOL_ENABLER_STATE,
                                enabler_state);
                    uiThreadHandler.sendMessage(updateUiMesg);
                    break;
               }

                case PresenceOemHook.QCRILHOOK_PRESENCE_IMS_UNSOL_NOTIFY_XML_UPDATE:
                {
                    String xml = (String) ind.obj;

                    Log.d(TAG, "Notify xml "+xml);
                    Utility.appendToXMLFile(mContext, "NOTIFY IND with XML content.\n");
                    Utility.appendToXMLFile(mContext, xml);
                    Utility.sendLogMesg(mContext, "NOTIFY IND with XML content.");
                    Utility.sendLogMesg(mContext, xml);

                    Message updateUiMesg = uiThreadHandler
                        .obtainMessage(
                                PresenceOemHook.QCRILHOOK_PRESENCE_IMS_UNSOL_NOTIFY_XML_UPDATE);
                    uiThreadHandler.sendMessage(updateUiMesg);
                }
                break;

                case PresenceOemHook.QCRILHOOK_PRESENCE_IMS_UNSOL_PUBLISH_TRIGGER:
                {
                    Integer val = (Integer) ind.obj;

                    Message updateUiMesg = uiThreadHandler
                        .obtainMessage(PresenceOemHook
                                .QCRILHOOK_PRESENCE_IMS_UNSOL_PUBLISH_TRIGGER, val);
                    uiThreadHandler.sendMessage(updateUiMesg);

                        switch(val) {
                            case AppGlobalState.IMSP_ETAG_EXPIRED:
                            {
                                Log.d(TAG, "Publish trigger for IMSP_ETAG_EXPIRED");
                                Log.d(TAG, "Schedule publish.");
                                invokePublish();
                                break;

                            }
                            case AppGlobalState.IMSP_RAT_CHANGE_LTE:
                            {
                                Log.d(TAG, "Publish trigger for IMSP_RAT_CHANGE_LTE");

                                Log.d(TAG, "Schedule publish.");
                                invokePublish();

                                break;

                            }
                            case AppGlobalState.IMSP_RAT_CHANGE_EHRPD:
                            {
                                Log.d(TAG, "Publish trigger for IMSP_RAT_CHANGE_EHRPD");
                                Log.d(TAG, "Schedule publish.");
                                invokePublish();
                                break;

                            }
                            case AppGlobalState.IMSP_AIRPLANE_MODE:
                            {
                                Log.d(TAG, "Publish trigger for IMSP_AIRPLANE_MODE");
                                Log.d(TAG, "Schedule publish.");
                                invokePublish();
                                break;
                            }
                            default :
                                Log.d(TAG, "Publish trigger for unknown value="+val);
                        }
                        break;
                    }
                default :
                    Log.e(TAG, "Unknown unsol indication mesg.");
            }
        }



    private void invokePublish() {
        if (Utility.getNotifyFmt(mContext) == AppGlobalState.NOTIFY_FMT_STRUCT) {
            Utility.sendLogMesg(mContext, "Trigger publish ind recvd,"+
                    " Publish fmt=STRUCT based.");
            new PublishTask().execute();
        } else {
            Utility.sendLogMesg(mContext, "Trigger publish ind recvd,"+
                    " Publish fmt=XML based.");
            new PublishXMLTask().execute();
        }
    }



    private void updateStatus(
            ArrayList<PresenceMsgParser.ContactInfo> parsedContactList) {

        ArrayList contacts = AppGlobalState.getContacts();

        for(PresenceMsgParser.ContactInfo c : parsedContactList) {
            String uriValue = c.mResourceUri;

            if (uriValue == null) {
                Log.e(TAG, "ContactUri is null, dont update on UI. c="+c);
                continue ;
            }

            int i= getIndex(contacts, uriValue);

            if (i < 0) {
                Log.e(TAG, "Contact="+uriValue+
                        " does not exist in phone book, dont update.");
                continue;
            }

            Contact temp = (Contact) contacts.get(i);
            temp.setBasicStatus((c.mPublishStatus == 1)? "Open": "Closed");
            temp.setAvailability((c.mPublishStatus == 1)?1:0);
            temp.setAudio((c.mIsAudioSupported)?"True":"False");
            temp.setVideo((c.mIsVideoSupported)?"True":"False");
            temp.setTimeStamp(c.mTimeStamp);
            temp.setNote((c.mIsVolteContact)?("VoLTE Contact"):("Not VoLTE contact"));

            temp.setListContactUri(c.listHeaderInfo.mListContactUri);
            temp.setListName(c.listHeaderInfo.mListName);
            temp.setListVersion(c.listHeaderInfo.mListVersion);
            temp.setListFullState(c.listHeaderInfo.mListFullState);
            temp.setResourceUri(c.mResourceUri);
            temp.setIsVolteContact(Boolean.toString(c.mIsVolteContact));
            temp.setResourceId(c.mResourceId);
            temp.setResourceState(c.mResourceState);
            temp.setResourceReason(c.mResourceReason);
            temp.setResourceCid(c.mResourceCid);
            temp.setContactUri(c.mContactUri);
            temp.setDescription(c.mDescription);
            temp.setVersion(c.mVersion);
            temp.setServiceId(c.mServiceId);
            temp.setAudioCapabilities(c.mAudioCapabilities);
            temp.setVideoCapabilities(c.mVideoCapabilities);

            Utility.rescheduleSubcribeTimer(i, temp);
        }
    }

    private int getIndex(ArrayList<Contact> contacts, String uriValue) {

        String phone = getPhoneFromUri(uriValue);

        Log.d(TAG, "getIndex() phone from uri="+phone);
        int i =0;
        for(Contact c: contacts){
            if (c.getPhone().equals(phone)) {
                Log.d(TAG,"Found phone="+phone+" at index ="+i);
                return i;
            }
            i++;
        }
        return -1;
    }

    private String getPhoneFromUri(String uriValue) {
        int startIndex = uriValue.indexOf(":", 0);
        int endIndex = uriValue.indexOf("@", startIndex);

        if(startIndex == -1 || endIndex == -1) { //URI does not have : or @
            return uriValue;
        } else {
            return uriValue.substring(startIndex+1, endIndex);
        }
    }

    Handler uiThreadHandler = new Handler(Looper.getMainLooper()) {
        public void handleMessage(Message msg ) {
            super.handleMessage(msg);

            Log.d(TAG, "Thread="+Thread.currentThread().getName()+" received "+msg);

            ContactArrayAdapter<Contact> adapter = AppGlobalState.getMainListAdapter();

            switch(msg.what) {
                case 0:
                {
                    Toast.makeText(mContext, "NOTIFY UPDATE with No data Received",
                            Toast.LENGTH_SHORT).show();
                    removeProgressBar();
                    break;
                }

                case PresenceOemHook.QCRILHOOK_PRESENCE_IMS_UNSOL_PUBLISH_TRIGGER:
                {
                    int val = (Integer)msg.obj;

                    switch(val)
                    {
                        case AppGlobalState.IMSP_ETAG_EXPIRED:
                        {
                            Toast.makeText(mContext,
                                    "Publish trigger for IMSP_ETAG_EXPIRED",
                                    Toast.LENGTH_SHORT).show();
                            break;

                        }
                        case AppGlobalState.IMSP_RAT_CHANGE_LTE:
                        {

                            Toast.makeText(mContext,
                                    "Publish trigger for IMSP_RAT_CHANGE_LTE",
                                    Toast.LENGTH_SHORT).show();
                            break;

                        }
                        case AppGlobalState.IMSP_RAT_CHANGE_EHRPD:
                        {
                            Toast.makeText(mContext,
                                    "Publish trigger for IMSP_RAT_CHANGE_EHRPD",
                                    Toast.LENGTH_SHORT).show();
                            break;

                        }
                        case AppGlobalState.IMSP_AIRPLANE_MODE:
                        {

                            Toast.makeText(mContext,
                                    "Publish trigger for IMSP_AIRPLANE_MODE",
                                    Toast.LENGTH_SHORT).show();
                            break;
                        }
                        default :
                        Toast.makeText(mContext, "Publish trigger for unknow val="+
                                val,Toast.LENGTH_SHORT).show();
                    }
                    break;
                }

                case PresenceOemHook.QCRILHOOK_PRESENCE_IMS_UNSOL_NOTIFY_UPDATE :
                {
                    ArrayList<PresenceMsgParser.ContactInfo> parsedContactList =
                        (ArrayList<PresenceMsgParser.ContactInfo>) msg.obj;
                    int num = parsedContactList.size();
                    if(num > 1) {
                        Toast.makeText(mContext, "NOTIFY UPDATE Received for  "+
                                num+" Contacts.",Toast.LENGTH_SHORT).show();
                    } else {
                        String phone = parsedContactList.get(0).mContactUri;
                        Toast.makeText(mContext, "NOTIFY UPDATE Received for "+
                                phone,Toast.LENGTH_SHORT).show();
                    }

                    adapter.notifyDataSetChanged();
                    removeProgressBar();

                    triggerUpdateOnDisplayedContactInfo();

                    break;
                }

                case PresenceOemHook.QCRILHOOK_PRESENCE_IMS_UNSOL_ENABLER_STATE:
                {
                    int enabler_state = (Integer)msg.obj;
                    Toast.makeText(mContext, "IMS Enabler State Result ="+
                            PresenceOemHook.IMS_ENABLER_RESPONSE[enabler_state],
                            Toast.LENGTH_SHORT).show();
                    break;
                }

                case PresenceOemHook.QCRILHOOK_PRESENCE_IMS_UNSOL_NOTIFY_XML_UPDATE:
                {
                    removeProgressBar();
                    Toast.makeText(mContext, "NOTIFY XML UPDATE Received",
                            Toast.LENGTH_SHORT).show();
                    break;
                }

                default:
                Toast.makeText(mContext, "Unknown mesg "+msg.what +" recieved.",
                        Toast.LENGTH_SHORT).show();

            }
        }

        private void triggerUpdateOnDisplayedContactInfo() {
            ContactInfo contactInfo = AppGlobalState.getContactInfo();
            if(contactInfo != null) {
                contactInfo.populateForm(contactInfo.getIndexOfDisplayedContact());
            }

        }

        private void removeProgressBar() {
            if (AppGlobalState.getProgressDialog() != null ) {
                // only in case of subscribe polling request
                AppGlobalState.getProgressDialog().dismiss();
            }
        }
    };
}
