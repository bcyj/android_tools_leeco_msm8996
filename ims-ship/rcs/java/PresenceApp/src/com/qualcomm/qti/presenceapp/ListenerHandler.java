/*************************************************************************
 Copyright (c)2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
*************************************************************************/

package com.qualcomm.qti.presenceapp;


import java.util.ArrayList;


import com.qualcomm.qti.presence.IQPresListener;
import com.qualcomm.qti.presence.PresCmdStatus;
import com.qualcomm.qti.presence.PresPublishTriggerType;
import com.qualcomm.qti.presence.PresResInfo;
import com.qualcomm.qti.presence.PresRlmiInfo;
import com.qualcomm.qti.presence.PresSipResponse;
import com.qualcomm.qti.presence.PresTupleInfo;
import com.qualcomm.qti.presenceapp.MainActivity.ContactArrayAdapter;
import com.qualcomm.qti.rcsimssettings.QrcsImsSettingsIntType;
import com.qualcomm.qti.rcsservice.QRCSInt;
import com.qualcomm.qti.rcsservice.QRCSString;
import com.qualcomm.qti.rcsservice.StatusCode;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.os.RemoteException;
import android.util.Log;
import android.widget.Toast;

@SuppressLint("HandlerLeak")
public class ListenerHandler extends Handler
{

    private static String TAG = "ListenerHandler";
    Context mContext;
    private final short PRESENCE_IMS_UNSOL_PUBLISH_TRIGGER = 1;
    private final short PRESENCE_IMS_UNSOL_NOTIFY_UPDATE = 2;
    private final short PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_AVAILABILITY = 3;
    private final short PRESENCE_IMS_UNSOL_ENABLER_STATE = 4;
    private final short PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_CMDSTATUS = 5;
    private final short PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_SIPRESPONSE = 6;
    private final short PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_SIPRESPONSE404 = 7;


   public ListenerHandler(Context context, Looper looper) {
        super(looper);
        mContext = context;
    }

    @Override
    public void handleMessage(Message msg) {
        super.handleMessage(msg);

        Log.d(TAG, "Thread=" + Thread.currentThread().getName() + " received "
                + msg);
    }

    public IQPresListener presListener = new IQPresListener.Stub() {

        public boolean onTransact(int code, Parcel data, Parcel reply, int flags) throws RemoteException
        {
            try
            {
                    return super.onTransact(code, data, reply, flags);
            }
            catch (RuntimeException e)
            {
                Log.w("ListenerHandler", "Unexpected remote exception", e);
                throw e;
            }
        }

        public void IQPresListener_SipResponseReceived(int pPresListenerHandle,
                PresSipResponse pSipResponse) throws RemoteException {

            Log.d("PRESENCE_UI", "aks2 ListenerHandler : SipResponseReceived : pPresListenerHandle "+pPresListenerHandle);
            Log.d("PRESENCE_UI", "ListenerHandler : SipResponseReceived : pSipResponse.getCmdId() "+pSipResponse.getCmdId().getCmdId().toString());
            Log.d("PRESENCE_UI", "ListenerHandler : SipResponseReceived : pSipResponse.getReasonPhrase() "+pSipResponse.getReasonPhrase());
            Log.d("PRESENCE_UI", "ListenerHandler : SipResponseReceived : pSipResponse.getsRequestID() "+pSipResponse.getRequestID());
            Log.d("PRESENCE_UI", "ListenerHandler : SipResponseReceived : pSipResponse.getsSipResponseCode() "+pSipResponse.getSipResponseCode());
            if(pSipResponse.getSipResponseCode() != 200)
            {
            switch (pSipResponse.getCmdId().getCmdId()) {
            case QRCS_PRES_CMD_PUBLISHMYCAP:
            {
                    if(pSipResponse.getSipResponseCode() == 999)
                    {
                        Message updateUiMesgSipPub = uiThreadHandler
                        .obtainMessage(
                                PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_SIPRESPONSE,
                                "Publish ignored - No capability change");
                        uiThreadHandler.sendMessage(updateUiMesgSipPub);
                    }
                    else
                    {
                Message updateUiMesgSipPub = uiThreadHandler
                .obtainMessage(
                        PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_SIPRESPONSE,
                        "Publish Sip Response Code "+pSipResponse.getSipResponseCode());
                uiThreadHandler.sendMessage(updateUiMesgSipPub);
                    }
                break;
            }
            case QRCS_PRES_CMD_GETCONTACTCAP:
            {
                if ((AppGlobalState.getOperatorMode() == AppGlobalState.VZW_MODE && pSipResponse
                        .getSipResponseCode() == 404)
                        || (AppGlobalState.getOperatorMode() == AppGlobalState.ATT_MODE && pSipResponse
                                .getSipResponseCode() == 489))
                {
                    updateStatus(null, null, null, null , pSipResponse.getRequestID());
                    Log.d("PRESENCE_UI", "ListenerHandler : CMDStatus : index 11112222223");
                    Message updateUiMesgSipContactCap = uiThreadHandler
                    .obtainMessage(
                    PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_SIPRESPONSE404,
                    "Availability Sip Response Code "+pSipResponse.getSipResponseCode());
                    uiThreadHandler.sendMessage(updateUiMesgSipContactCap);
                    Log.d("PRESENCE_UI", "ListenerHandler : CMDStatus : index 34444444445");
                }
                else
                {
                    Message updateUiMesgSipContactCap = uiThreadHandler
                    .obtainMessage(
                    PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_SIPRESPONSE,
                    "Availability Sip Response Code "+pSipResponse.getSipResponseCode());
                    uiThreadHandler.sendMessage(updateUiMesgSipContactCap);
                }
                break;
            }
            case QRCS_PRES_CMD_GETCONTACTLISTCAP:
            {
                if ((AppGlobalState.getOperatorMode() == AppGlobalState.VZW_MODE && pSipResponse
                        .getSipResponseCode() == 404)
                        || (AppGlobalState.getOperatorMode() == AppGlobalState.ATT_MODE && pSipResponse
                                .getSipResponseCode() == 489))
                {
                    updateStatus(null, null, null, null, pSipResponse.getRequestID());
                    Log.d("PRESENCE_UI", "ListenerHandler : CMDStatus : index 111111111111");
                    Message updateUiMesgSipContactListCap = uiThreadHandler
                    .obtainMessage(
                    PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_SIPRESPONSE404,
                    "Polling Sip Response Code "+pSipResponse.getSipResponseCode());
                    uiThreadHandler.sendMessage(updateUiMesgSipContactListCap);
                    Log.d("PRESENCE_UI", "ListenerHandler : CMDStatus : index 22222222222222");
                }
                else
                {
                    Message updateUiMesgSipContactListCap = uiThreadHandler
                    .obtainMessage(
                    PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_SIPRESPONSE,
                    "Polling Sip Response Code "+pSipResponse.getSipResponseCode());
                    uiThreadHandler.sendMessage(updateUiMesgSipContactListCap);
                }
                break;
            }
            case QRCS_PRES_CMD_SETNEWFEATURETAG:
            {
                Message updateUiMesgSipNewTag = uiThreadHandler
                .obtainMessage(
                PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_SIPRESPONSE,
                "New Tag Sip Response Code "+pSipResponse.getSipResponseCode());
                uiThreadHandler.sendMessage(updateUiMesgSipNewTag);
                break;
            }
            default:
                Log.d(TAG, "CMD ID for unknown value=" + pSipResponse.getCmdId().getCmdId().toString());
            }
            }
        }

        public void IQPresListener_ServiceUnAvailable(int pPresListenerHandle,
                StatusCode statusCode) throws RemoteException {

            Log.d("PRESENCE_UI", "aks2 ListenerHandler : ServiceUnAvailable : pPresListenerHandle "+pPresListenerHandle);
            Log.d("PRESENCE_UI", "ListenerHandler : ServiceUnAvailable : statusCode "+statusCode.getStatusCode().toString());
            AppGlobalState.setImsEnablerState("UN-INITIALIZED");
            Message updateUiMesg = uiThreadHandler
                    .obtainMessage(
                            PRESENCE_IMS_UNSOL_ENABLER_STATE,
                            "UN-INITIALIZED");
            uiThreadHandler.sendMessage(updateUiMesg);
        }

        public void IQPresListener_ServiceAvailable(int pPresListenerHandle,
                StatusCode statusCode) throws RemoteException {

            Log.d("PRESENCE_UI", "aks2 ListenerHandler : ServiceAvailable : pPresListenerHandle "+pPresListenerHandle);
            Log.d("PRESENCE_UI", "ListenerHandler : ServiceAvailable : statusCode "+statusCode.getStatusCode().toString());
            AppGlobalState.setImsEnablerState("REGISTERED");
            Message updateUiMesg = uiThreadHandler
                    .obtainMessage(
                            PRESENCE_IMS_UNSOL_ENABLER_STATE,
                            "REGISTERED");
            uiThreadHandler.sendMessage(updateUiMesg);
        }

        public void IQPresListener_PublishTriggering(int pPresListenerHandle,
                final PresPublishTriggerType publishTrigger) throws RemoteException {

            Log.d("PRESENCE_UI", "aks2 ListenerHandler : PublishTriggering : pPresListenerHandle "
                    + pPresListenerHandle);
            Log.d("PRESENCE_UI",
                    "ListenerHandler : PublishTriggering : publishTrigger.getPublishTrigeerType() "
                            + publishTrigger.getPublishTrigeerType().toString());
            Log.d("PRESENCE_UI",
                    "ListenerHandler : PublishTriggering : publishTrigger.getPublishTrigeerType() "
                            + publishTrigger.getPublishTrigeerType().ordinal());

            Runnable show_UI = new Runnable()
            {
                public void run()
                {
                    try
                    {
                        SharedPreferences preferences = mContext.getSharedPreferences(
                                "ImsPresencePrefMyInfo", Context.MODE_PRIVATE);
                        SharedPreferences.Editor editor = preferences.edit();

                        switch (publishTrigger.getPublishTrigeerType()) {
                            case QRCS_PRES_PUBLISH_TRIGGER_ETAG_EXPIRED: {
                                Log.d(TAG, "Publish trigger for IMSP_ETAG_EXPIRED");
                                Log.d(TAG, "Schedule publish.");
                                invokePublish();
                                break;

                            }
                            case QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_LTE_VOPS_ENABLED: {
                                Log.d(TAG,
                                        "Publish trigger for QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_LTE_VOPS_ENABLED");
                                Log.d(TAG, "Status is OPEN");
                                ContactInfo.networkTypeLTE = true;
                                ContactInfo.vopsEnabled = true;
                                ContactInfo.networkTypeEHRPD = false;
                                editor.putString("Basic Status", "Open");
                                editor.commit();
                                Log.d(TAG, "Schedule publish.");
                                invokePublish();

                                break;

                            }
                            case QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_LTE_VOPS_DISABLED: {
                                Log.d(TAG,
                                        "Publish trigger for QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_LTE_VOPS_DISABLED");
                                Log.d(TAG, "Status is CLOSED");
                                ContactInfo.networkTypeLTE = true;
                                ContactInfo.vopsEnabled = false;
                                ContactInfo.networkTypeEHRPD = false;
                                editor.putString("Basic Status", "Closed");
                                editor.commit();
                                Log.d(TAG, "Schedule publish.");
                                invokePublish();
                                break;
                            }
                            case QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_EHRPD: {
                                Log.d(TAG, "Publish trigger for IMSP_RAT_CHANGE_EHRPD");
                                Log.d(TAG, "Status is CLOSED");
                                ContactInfo.networkTypeLTE = false;
                                ContactInfo.vopsEnabled = false;
                                ContactInfo.networkTypeEHRPD = true;
                                editor.putString("Basic Status", "Closed");
                                editor.commit();
                                Log.d(TAG, "Schedule publish.");
                                invokePublish();
                                break;

                            }
                            case QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_HSPAPLUS: {
                                Log.d(TAG, "Publish trigger for IMSP_AIRPLANE_MODE");
                                break;
                            }
                            case QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_2G:
                            {
                                Log.d(TAG, "Publish trigger for QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_2G");
                                Log.d(TAG, "Status is CLOSED");
                                ContactInfo.networkTypeLTE = false;
                                ContactInfo.vopsEnabled = false;
                                ContactInfo.networkTypeEHRPD = true;
                                editor.putString("Basic Status", "Closed");
                                editor.commit();
                                Log.d(TAG, "Schedule publish.");
                                invokePublish();
                                break;
                            }
                            case QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_3G:
                            {
                                Log.d(TAG, "Publish trigger for QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_3G");
                                Log.d(TAG, "Status is CLOSED");
                                ContactInfo.networkTypeLTE = false;
                                ContactInfo.vopsEnabled = false;
                                ContactInfo.networkTypeEHRPD = true;
                                editor.putString("Basic Status", "Closed");
                                editor.commit();
                                Log.d(TAG, "Schedule publish.");
                                invokePublish();
                                break;
                            }
                            default:
                                Log.d(TAG,
                                        "Publish trigger for unknown value="
                                                + publishTrigger.getPublishTrigeerType());
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            };
            Log.d("PRESENCE_UI", "ListenerHandler : before posting to UI tread");
            MainActivity.mainActivityObject.runOnUiThread(show_UI);
            Log.d("PRESENCE_UI", "ListenerHandler : after posting to UI tread");

            Message updateUiMesg = uiThreadHandler.obtainMessage(
                    PRESENCE_IMS_UNSOL_PUBLISH_TRIGGER, publishTrigger);
            uiThreadHandler.sendMessage(updateUiMesg);

            Log.d("PRESENCE_UI", "ListenerHandler : after calling ui update");
        }

        public void IQPresListener_ListCapInfoReceived(int pPresListenerHandle,
                PresRlmiInfo pRlmiInfo, PresResInfo[] pResInfo)
                throws RemoteException {

            Log.d("PRESENCE_UI", "aks2 ListenerHandler : ListCapInfoReceived : pPresListenerHandle "+pPresListenerHandle);
            if(pRlmiInfo != null)
            {
                Log.d("PRESENCE_UI", "ListenerHandler : ListCapInfoReceived : pRlmiInfo.getListName "+pRlmiInfo.getListName());
                Log.d("PRESENCE_UI", "ListenerHandler : ListCapInfoReceived : pRlmiInfo.isFullState "+pRlmiInfo.isFullState());
                Log.d("PRESENCE_UI", "ListenerHandler : ListCapInfoReceived : pRlmiInfo.getUri "+pRlmiInfo.getUri());
                Log.d("PRESENCE_UI", "ListenerHandler : ListCapInfoReceived : pRlmiInfo.getVersion "+pRlmiInfo.getVersion());
            }
            if(pResInfo != null)
            {
                for(int i=0; i < pResInfo.length; i++ )
                {
                    Log.d("PRESENCE_UI", "ListenerHandler : ListCapInfoReceived : pResInfo[i].getDisplayName() "+pResInfo[i].getDisplayName());
                    Log.d("PRESENCE_UI", "ListenerHandler : ListCapInfoReceived : pResInfo[i].getResUri() "+pResInfo[i].getResUri());
                    Log.d("PRESENCE_UI", "ListenerHandler : ListCapInfoReceived : pResInfo.getInstanceInfo().getPresentityUri() "+pResInfo[i].getInstanceInfo().getPresentityUri());
                    Log.d("PRESENCE_UI", "ListenerHandler : ListCapInfoReceived : pResInfo[i].getInstanceInfo().getResId() "+pResInfo[i].getInstanceInfo().getResId());
                    Log.d("PRESENCE_UI", "ListenerHandler : ListCapInfoReceived : pResInfo[i].getInstanceInfo().getsReason() "+pResInfo[i].getInstanceInfo().getReason());
                    Log.d("PRESENCE_UI", "ListenerHandler : ListCapInfoReceived : pResInfo[i].getInstanceInfo().getResInstanceState() "+pResInfo[i].getInstanceInfo().getResInstanceState());
                    if(pResInfo[i].getInstanceInfo() != null && pResInfo[i].getInstanceInfo().getTupleInfo() != null)
                    {
                        Log.d("PRESENCE_UI", "ListenerHandler : ListCapInfoReceived : pResInfo[i].getInstanceInfo().getTupleInfo().length "+pResInfo[i].getInstanceInfo().getTupleInfo().length);
                        for(int j = 0; j < pResInfo[i].getInstanceInfo().getTupleInfo().length; j++ )
                        {
                            Log.d("PRESENCE_UI", "ListenerHandler : ListCapInfoReceived : pResInfo[i].getInstanceInfo().getTupleInfo()[j].getFeatureTag "+pResInfo[i].getInstanceInfo().getTupleInfo()[j].getFeatureTag());
                            Log.d("PRESENCE_UI", "ListenerHandler : ListCapInfoReceived : pResInfo[i].getInstanceInfo().getTupleInfo()[j].getsContactURI "+pResInfo[i].getInstanceInfo().getTupleInfo()[j].getContactURI());
                            Log.d("PRESENCE_UI", "ListenerHandler : ListCapInfoReceived : pResInfo[i].getInstanceInfo().getTupleInfo()[j].getsTimestamp "+pResInfo[i].getInstanceInfo().getTupleInfo()[j].getTimestamp());
                        }
                    }
                }
            }
            updateStatus(pRlmiInfo, pResInfo, null, null , 0);
            Message updateUiMesg = uiThreadHandler
                    .obtainMessage(
                            PRESENCE_IMS_UNSOL_NOTIFY_UPDATE,
                            pResInfo);
            uiThreadHandler.sendMessage(updateUiMesg);

        }

        public void IQPresListener_GetVersion(int pPresListenerHandle,
                QRCSString pVersion, QRCSInt pVersionLen) throws RemoteException {

            Log.d("PRESENCE_UI", "aks2 ListenerHandler : GetVersion : pPresListenerHandle "+pPresListenerHandle);
            Log.d("PRESENCE_UI", "ListenerHandler : GetVersion : pVersion "+pVersion.getQRCSString().toString());
            Log.d("PRESENCE_UI", "ListenerHandler : GetVersion : pVersionLen "+pVersionLen.getQRCSInt());

        }

        public void IQPresListener_CapInfoReceived(int pPresListenerHandle,
                QRCSString pPresentityURI, PresTupleInfo[] pTupleInfo)
                throws RemoteException {

            Log.d("PRESENCE_UI", "aks2 ListenerHandler : CapInfoReceived : pPresListenerHandle "+pPresListenerHandle);
            if(pPresentityURI != null)
            {
                Log.d("PRESENCE_UI", "ListenerHandler : CapInfoReceived : pPresentityURI "+pPresentityURI.getQRCSString().toString());
            }
            if(pTupleInfo != null)
            {
                for(int i = 0; i < pTupleInfo.length; i++)
                {
                    Log.d("PRESENCE_UI", "ListenerHandler : CapInfoReceived : pTupleInfo[i].getFeatureTag() "+pTupleInfo[i].getFeatureTag());
                    Log.d("PRESENCE_UI", "ListenerHandler : CapInfoReceived : pTupleInfo[i].getsContactURI() "+pTupleInfo[i].getContactURI());
                    Log.d("PRESENCE_UI", "ListenerHandler : CapInfoReceived : pTupleInfo[i].getsTimestamp() "+pTupleInfo[i].getTimestamp());
                }
            }

            updateStatus(null, null, pPresentityURI, pTupleInfo, 0);
            Message updateUiMesg = uiThreadHandler
                    .obtainMessage(
                            PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_AVAILABILITY,
                            pPresentityURI);
            uiThreadHandler.sendMessage(updateUiMesg);
        }

        public void IQPresListener_CMDStatus(int pPresListenerHandle,
                PresCmdStatus pCmdStatus) throws RemoteException {

            Log.d("PRESENCE_UI", "aks2 ListenerHandler : CMDStatus : pPresListenerHandle "+pPresListenerHandle);
            Log.d("PRESENCE_UI", "ListenerHandler : CMDStatus : pCmdStatus.getRequestID() "+pCmdStatus.getRequestID());
            Log.d("PRESENCE_UI", "ListenerHandler : CMDStatus : pCmdStatus.getUserData() "+pCmdStatus.getUserData());

            for(int i = 0; i < AppGlobalState.requestinfo.size(); i++)
            {
                if(pCmdStatus.getUserData() == AppGlobalState.requestinfo.get(i).userData)
                {
                    AppGlobalState.requestinfo.get(i).requestID = pCmdStatus.getRequestID();
                    Log.d("PRESENCE_UI", "ListenerHandler : CMDStatus : AppGlobalState.requestinfo.get(i).requestID "+AppGlobalState.requestinfo.get(i).requestID);
                    break;
                }
            }

            Log.d("PRESENCE_UI", "ListenerHandler : CMDStatus : pCmdStatus.getCmdId().toString() "+pCmdStatus.getCmdId().getCmdId().toString());
            Log.d("PRESENCE_UI", "ListenerHandler : CMDStatus : pCmdStatus.getStatus().toString() "+pCmdStatus.getStatus().getStatusCode().toString());
            if(!pCmdStatus.getStatus().getStatusCode().toString().equals("QRCS_SUCCESS"))
            {
            switch (pCmdStatus.getCmdId().getCmdId()) {
            case QRCS_PRES_CMD_PUBLISHMYCAP:
            {
                Message updateUiMesgCmdPub = uiThreadHandler
                .obtainMessage(
                        PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_CMDSTATUS,
                        "Publish Cmd Response "+pCmdStatus.getStatus().getStatusCode().toString());
                uiThreadHandler.sendMessage(updateUiMesgCmdPub);
                break;
            }
            case QRCS_PRES_CMD_GETCONTACTCAP:
            {
                Message updateUiMesgCmdContactCap = uiThreadHandler
                .obtainMessage(
                        PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_CMDSTATUS,
                        "Availability Cmd Response "+pCmdStatus.getStatus().getStatusCode().toString());
                uiThreadHandler.sendMessage(updateUiMesgCmdContactCap);
                break;
            }
            case QRCS_PRES_CMD_GETCONTACTLISTCAP:
            {
                Message updateUiMesgCmdContactListCap = uiThreadHandler
                .obtainMessage(
                        PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_CMDSTATUS,
                        "Polling Cmd Response "+pCmdStatus.getStatus().getStatusCode().toString());
                uiThreadHandler.sendMessage(updateUiMesgCmdContactListCap);
                break;
            }
            case QRCS_PRES_CMD_SETNEWFEATURETAG:
            {
                Message updateUiMesgCmdNewTag = uiThreadHandler
                .obtainMessage(
                        PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_CMDSTATUS,
                        "New Tag Cmd Response "+pCmdStatus.getStatus().getStatusCode().toString());
                uiThreadHandler.sendMessage(updateUiMesgCmdNewTag);
                break;
            }
            default:
                Log.d(TAG, "CMD ID for unknown value=" + pCmdStatus.getCmdId().getCmdId().toString());
            }
        }
        }
    };

    private int publishTry = 0;

    private void invokePublish() {
        try
        {
            if(AppGlobalState.isDataSettingNvSame || AppGlobalState.getOperatorMode() == AppGlobalState.ATT_MODE)
            {
                if (Utility.getNotifyFmt(mContext) == AppGlobalState.NOTIFY_FMT_STRUCT) {
                    Utility.sendLogMesg(mContext, "Trigger publish ind recvd," +
                            " Publish fmt=STRUCT based.");
                    new PublishTask().execute();
                }
            }
            else
            {
                ContactInfo.firstPublish = true;
                Log.d("TAG", "invokePublish : out side runnable");
                try {
                    if(MainActivity.imsSettingService!= null && AppGlobalState.getQrcsImsSettingsclienthandle() != 0)
                    {
                        QrcsImsSettingsIntType client_handle = new QrcsImsSettingsIntType();
                        client_handle.setQrcsImsSettingsIntType(AppGlobalState.getQrcsImsSettingsclienthandle());
                        MainActivity.imsSettingService
                                .QrcsImsSettings_GetQipcallConfig(client_handle);
                    }
                    else
                    {
                        Log.d("TAG", "MainActivity.imsSettingService : null");
                    }
                    Log.d("TAG", "invokePublish : before runnable");
                    if(publishTry < 10)
                    {
                        if(Looper.myLooper() == null)
                        {
                            Log.d("TAG", "before Looper.prepare()");
                            Looper.prepare();
                            Log.d("TAG", "after Looper.prepare()");
                        }
                        new Handler().postDelayed(new Runnable() {

                            public void run() {
                                Log.d("TAG", "invokePublish : inside runnable runnable");
                                invokePublish();
                            }
                        }, 500);
                        publishTry++;
                        Looper.loop();
                    }
                    Log.d("TAG", "invokePublish : after runnable");
                } catch (RemoteException e) {
                    e.printStackTrace();
                }catch (Exception e) {
                    e.printStackTrace();
                }
                Log.d("TAG", "invokePublish : after runnable");
            }
        }catch(Exception e)
        {
            e.printStackTrace();
        }
    }


    private void updateStatus(PresRlmiInfo pRlmiInfo, PresResInfo[] pResInfo, QRCSString pPresentityURI, PresTupleInfo[] pTupleInfo , int requestID) {

        ArrayList<Contact> contacts = AppGlobalState.getContacts();


        if(pRlmiInfo == null && pResInfo == null && pTupleInfo == null && pPresentityURI == null && requestID != 0)
        {
            for(int i = 0; i < AppGlobalState.requestinfo.size(); i++)
            {
                if(requestID == AppGlobalState.requestinfo.get(i).requestID)
                {
                    Log.d("PRESENCE_UI", "ListenerHandler : CMDStatus : AppGlobalState.requestinfo.get(i).requestID "+AppGlobalState.requestinfo.get(i).requestID);
                    String uriValue;
                    //ArrayList<Contact> contacts = AppGlobalState.getContacts();
                    for(int k = 0; k < AppGlobalState.requestinfo.get(i).URI.length; k++)
                    {
                        //uriValue = getPhoneFromUri(AppGlobalState.requestinfo.get(i).URI[k]);
                        uriValue = AppGlobalState.requestinfo.get(i).URI[k];
                        Log.d("PRESENCE_UI", "ListenerHandler : CMDStatus : uriValue "+uriValue);
                        if (AppGlobalState.requestinfo.get(i).URI[k].contains("tel:")) {
                            uriValue = AppGlobalState.requestinfo.get(i).URI[k].substring(4);
                            for(int j = 0; j < uriValue.length(); j++)
                            {
                                if(uriValue.charAt(j) == '@')
                                {
                                    uriValue = uriValue.substring(0, j);
                                    break;
                                }
                            }
                            Log.e("PRESENCE_UI", " IF uriValue =" + uriValue);
                        }else if (AppGlobalState.requestinfo.get(i).URI[k].contains("sip:")) {
                            uriValue = AppGlobalState.requestinfo.get(i).URI[k].substring(4);
                            for(int j = 0; j < uriValue.length(); j++)
                            {
                                if(uriValue.charAt(j) == '@')
                                {
                                    uriValue = uriValue.substring(0, j);
                                    break;
                                }
                            }
                            Log.e("PRESENCE_UI", " IF uriValue =" + uriValue);
                        }else {
                            uriValue = AppGlobalState.requestinfo.get(i).URI[k];
                            for(int j = 0; j < uriValue.length(); j++)
                            {
                                if(uriValue.charAt(j) == '@')
                                {
                                    uriValue = uriValue.substring(0, j);
                                    break;
                                }
                            }
                            Log.e("PRESENCE_UI", "ELSE uriValue =" + uriValue);

                        }
                        int index = getIndex(contacts, uriValue);
                        Log.d("PRESENCE_UI", "ListenerHandler : CMDStatus : index "+index);
                        //Contact temp = new Contact(contacts.get(index).name, uriValue, 0, "Closed");
                        Contact temp = (Contact) contacts.get(index);
                        if (AppGlobalState.getOperatorMode() == AppGlobalState.VZW_MODE)
                        {
                            temp = new Contact(contacts.get(index).name, uriValue, 0, "Closed" , "");
                        }
                        else
                        {
                            temp = new Contact(contacts.get(index).name, uriValue, 0, "Closed" , contacts.get(index).getIndividualContactURI());
                        }
                        contacts.set(index, temp);
                        Utility.rescheduleSubcribeTimer(index, temp);
                    }
                }
            }
        }

        if (pResInfo != null && pRlmiInfo != null) {
        for (PresResInfo c : pResInfo) {
        String uriValue1 = c.getResUri();
            Log.e("PRESENCE_UI", "uriValue1 =" + uriValue1);
            String uriValue;
            if (uriValue1.contains("tel:")) {
                uriValue = uriValue1.substring(4);
                Log.e("PRESENCE_UI", " IF uriValue =" + uriValue);
            } else {
                uriValue = uriValue1;
                Log.e("PRESENCE_UI", "ELSE uriValue =" + uriValue);

            }

            if (uriValue == null) {
                Log.e(TAG, "ContactUri is null, dont update on UI. c=" + c);
                continue;
            }

            int i = getIndex(contacts, uriValue);

            if (i < 0) {
                    Log.e(TAG, "Contact=" + uriValue
                            + " does not exist in phone book, dont update.");
                continue;
            }

                Contact temp = (Contact) contacts.get(i);
                temp.setAvailability(0);
                temp.setBasicStatus("Closed");
                temp.setListContactUri(pRlmiInfo.getUri());
                temp.setListName(pRlmiInfo.getListName());
                temp.setListVersion(""+pRlmiInfo.getVersion());
                temp.setListFullState(""+pRlmiInfo.isFullState());
                temp.setResourceUri(c.getResUri());
                temp.setResourceId(c.getInstanceInfo().getResId());
                temp.setResourceState(c.getInstanceInfo().getResInstanceState().toString());
                temp.setResourceReason(c.getInstanceInfo().getReason());
                temp.setResourceCid(c.getInstanceInfo().getResId());
                temp.setServiceId(c.getInstanceInfo().getResId());

                temp.setAudio("False");
                temp.setVideo("False");
                if(c.getInstanceInfo().getReason().equals("rejected") || c.getInstanceInfo().getReason().equals("noresource") || c.getInstanceInfo().getReason().equals("giveup"))
                {
                Log.d("PRESENCE_UI", "ListenerHandler : updateStatus : Not VoLTE contact");
                temp.setNote("Not VoLTE contact");
                }
                else if (!c.getInstanceInfo().getReason().equals(""))
                {
                Log.d("PRESENCE_UI", "ListenerHandler : updateStatus : VoLTE contact");
                temp.setNote("VoLTE Contact");
                }
                temp.setIsVolteContact("false");

                temp.setAudioCapabilities("false");
                temp.setVideoCapabilities("false");
                temp.setVersion("false");
                temp.setDescription("false");
                int isChatEnabled = 0;

                if(c.getInstanceInfo() != null && c.getInstanceInfo().getTupleInfo() != null && c.getInstanceInfo().getTupleInfo().length > 0)
                {
                    for (int j = 0; j < c.getInstanceInfo().getTupleInfo().length; j++) {
                        temp.setTimeStamp(c.getInstanceInfo().getTupleInfo()[j].getTimestamp());
                        temp.setContactUri(c.getInstanceInfo().getTupleInfo()[j].getContactURI());
                        Log.e("PPRESENCE_UI",
                            "mContactUri ="    + c.getInstanceInfo().getTupleInfo()[j].getContactURI());
                        String tempFeatureTag = c.getInstanceInfo().getTupleInfo()[j].getFeatureTag();
                        Log.e("PPRESENCE_UI",
                                "tempFeatureTag ="    + tempFeatureTag);
                        if(tempFeatureTag != null && tempFeatureTag.equals("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\""))
                        {
                            Log.d("PRESENCE_UI", "ListenerHandler : updateStatus : 1111");
                            temp.setAvailability(1);
                            temp.setBasicStatus("Open");
                            temp.setAudio("True");
                            temp.setVideo("False");
                            temp.setNote("VoLTE Contact");
                            temp.setIsVolteContact("true");
                            temp.setAudioCapabilities("true");
                            temp.setVideoCapabilities("false");
                        }
                        if(tempFeatureTag != null && tempFeatureTag.equals("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\";video"))
                        {
                            Log.d("PRESENCE_UI", "ListenerHandler : updateStatus : 2222");
                            temp.setAvailability(1);
                            temp.setBasicStatus("Open");
                            temp.setAudio("True");
                            temp.setVideo("True");
                            temp.setNote("VoLTE Contact");
                            temp.setIsVolteContact("true");
                            temp.setAudioCapabilities("true");
                            temp.setVideoCapabilities("true");
                        }
                        if(tempFeatureTag != null && tempFeatureTag.equals("+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.im\""))
                        {
                            isChatEnabled ++;
                            Log.d("PRESENCE_UI", "ListenerHandler : updateStatus : isChatEnabled "+isChatEnabled);

                        }
                        if(tempFeatureTag != null && tempFeatureTag.equals("+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gppapplication.ims.iari.rcs.fullsfgroupchat\""))
                        {
                            isChatEnabled ++;
                            Log.d("PRESENCE_UI", "ListenerHandler : updateStatus : isChatEnabled "+isChatEnabled);
                        }
                        if(tempFeatureTag != null && tempFeatureTag.equals("+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.ft\""))
                        {
                            temp.setAvailability(1);
                            temp.setBasicStatus("Open");
                            temp.setVersion("true");
                            Log.d("PRESENCE_UI", "ListenerHandler : updateStatus : 5555");
                        }
                        if(tempFeatureTag != null && tempFeatureTag.equals("+g.3gpp.iari-ref=\"urn%urn-7%3gpp-application.ims.iari.rcse.dp\""))
                        {
                            temp.setAvailability(1);
                            temp.setBasicStatus("Open");
                            Log.d("PRESENCE_UI", "ListenerHandler : updateStatus : 6666");
                        }

                        if((isChatEnabled == 2) || (isChatEnabled == 1 && AppGlobalState.getOperatorMode() == AppGlobalState.ATT_MODE))
                        {
                            temp.setAvailability(1);
                            temp.setBasicStatus("Open");
                            temp.setDescription("true");

                        }
                    }
                }
                else
                {
                    temp.setAvailability(0);
                    temp.setBasicStatus("Closed");
                    temp.setAudio("False");
                    temp.setVideo("False");
                    temp.setNote("Not VoLTE contact");
                    temp.setIsVolteContact("false");
                    temp.setAudioCapabilities("false");
                    temp.setVideoCapabilities("false");
                    temp.setVersion("false");
                    temp.setDescription("false");
                }

            Utility.rescheduleSubcribeTimer(i, temp);
        }
        }

        if(pPresentityURI != null /*pTupleInfo != null && pTupleInfo.length > 0*/)
        {


            //String uriValue1 = pTupleInfo[0].getContactURI();
            String uriValue1 = pPresentityURI.getQRCSString().toString();
            Log.e("PRESENCE_UI", "uriValue1 =" + uriValue1);
            String uriValue;
            if (uriValue1.contains("tel:")) {
                uriValue = uriValue1.substring(4);
                for(int j = 0; j < uriValue.length(); j++)
                {
                    if(uriValue.charAt(j) == '@')
                    {
                        uriValue = uriValue.substring(0, j);
                        break;
                    }
                }
                Log.e("PRESENCE_UI", " IF uriValue =" + uriValue);
            }else if (uriValue1.contains("sip:")) {
                uriValue = uriValue1.substring(4);
                for(int j = 0; j < uriValue.length(); j++)
                {
                    if(uriValue.charAt(j) == '@')
                    {
                        uriValue = uriValue.substring(0, j);
                        break;
                    }
                }
                Log.e("PRESENCE_UI", " IF uriValue =" + uriValue);
            }else {
                uriValue = uriValue1;
                for(int j = 0; j < uriValue.length(); j++)
                {
                    if(uriValue.charAt(j) == '@')
                    {
                        uriValue = uriValue.substring(0, j);
                        break;
                    }
                }
                Log.e("PRESENCE_UI", "ELSE uriValue =" + uriValue);

            }

            if (uriValue == null) {
            Log.e(TAG, "ContactUri is null, dont update on UI. c= null");
            return;
            }


            int i = getIndex(contacts, uriValue);

            if (i < 0) {
                Log.e(TAG, "Contact=" + uriValue
                        + " does not exist in phone book, dont update.");
            return;
            }

            Contact temp = (Contact) contacts.get(i);

            temp.setAvailability(0);
            temp.setBasicStatus("Closed");
            temp.setAudio("False");
            temp.setVideo("False");
            temp.setNote("Not VoLTE contact");
            temp.setIsVolteContact("false");
            temp.setAudioCapabilities("false");
            temp.setVideoCapabilities("false");
            temp.setVersion("false");
            temp.setDescription("false");
            temp.setContactUri(pPresentityURI.getQRCSString().toString());
            int isChatEnabled = 0;

            if(pTupleInfo != null)
            {
                temp.setTimeStamp(pTupleInfo[0].getTimestamp());
                Log.e("PPRESENCE_UI", "mContactUri ="    + pTupleInfo[0].getContactURI());
                for (PresTupleInfo c : pTupleInfo)
                {
                    String tempFeatureTag = c.getFeatureTag();
                    Log.e("PPRESENCE_UI", "tempFeatureTag =" + tempFeatureTag);
                    if(tempFeatureTag != null && tempFeatureTag.equals("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\""))
                    {
                        Log.d("PRESENCE_UI", "ListenerHandler : updateStatus : 1111");
                        temp.setAvailability(1);
                        temp.setBasicStatus("Open");
                        temp.setAudio("True");
                        temp.setVideo("False");
                        temp.setNote("VoLTE Contact");
                        temp.setIsVolteContact("true");
                        temp.setAudioCapabilities("true");
                        temp.setVideoCapabilities("false");
                    }
                    if(tempFeatureTag != null && tempFeatureTag.equals("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\";video"))
                    {
                        Log.d("PRESENCE_UI", "ListenerHandler : updateStatus : 2222");
                        temp.setAvailability(1);
                        temp.setBasicStatus("Open");
                        temp.setAudio("True");
                        temp.setVideo("True");
                        temp.setNote("VoLTE Contact");
                        temp.setIsVolteContact("true");
                        temp.setAudioCapabilities("true");
                        temp.setVideoCapabilities("true");
                    }
                    if(tempFeatureTag != null && tempFeatureTag.equals("+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.im\""))
                    {
                        isChatEnabled ++;
                        Log.d("PRESENCE_UI", "ListenerHandler : updateStatus : isChatEnabled "+isChatEnabled);
                    }
                    if(tempFeatureTag != null && tempFeatureTag.equals("+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gppapplication.ims.iari.rcs.fullsfgroupchat\""))
                    {
                        isChatEnabled ++;
                        Log.d("PRESENCE_UI", "ListenerHandler : updateStatus : isChatEnabled "+isChatEnabled);
                    }
                    if(tempFeatureTag != null && tempFeatureTag.equals("+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.ft\""))
                    {
                        temp.setAvailability(1);
                        temp.setBasicStatus("Open");
                        temp.setVersion("true");
                        Log.d("PRESENCE_UI", "ListenerHandler : updateStatus : 5555");
                    }
                    if(tempFeatureTag != null && tempFeatureTag.equals("+g.3gpp.iari-ref=\"urn%urn-7%3gpp-application.ims.iari.rcse.dp\""))
                    {
                        temp.setAvailability(1);
                        temp.setBasicStatus("Open");
                        Log.d("PRESENCE_UI", "ListenerHandler : updateStatus : 6666");
                    }

                    if((isChatEnabled == 2) || (isChatEnabled == 1 && AppGlobalState.getOperatorMode() == AppGlobalState.ATT_MODE))
                    {
                        temp.setAvailability(1);
                        temp.setBasicStatus("Open");
                        temp.setDescription("true");
                    }

                    Utility.rescheduleSubcribeTimer(i, temp);
                }
            }
            else
            {
                Log.d("PRESENCE_UI", "ListenerHandler : updateStatus : 7777");
                temp.setAvailability(0);
                temp.setBasicStatus("Closed");
                temp.setAudio("False");
                temp.setVideo("False");
                temp.setNote("Not VoLTE contact");
                temp.setIsVolteContact("false");
                temp.setAudioCapabilities("false");
                temp.setVideoCapabilities("false");
                temp.setVersion("false");
                temp.setDescription("false");
            }
        }
    }

    private int getIndex(ArrayList<Contact> contacts, String uriValue) {

        String phone = getPhoneFromUri(uriValue);

        Log.d(TAG, "getIndex() phone from uri=" + phone);
        int i = 0;
        for (Contact c : contacts) {
            if (c.getPhone().equals(phone)) {
                Log.d(TAG, "Found phone=" + phone + " at index =" + i);
                return i;
            }
            i++;
        }
        return -1;
    }

    private String getPhoneFromUri(String uriValue) {
        int startIndex = uriValue.indexOf(":", 0);
        int endIndex = uriValue.indexOf("@", startIndex);

        if (startIndex == -1 || endIndex == -1) {
            return uriValue;
        } else {
            return uriValue.substring(startIndex + 1, endIndex);
        }
    }

    Handler uiThreadHandler = new Handler(Looper.getMainLooper()) {
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            Log.d(TAG, "Thread=" + Thread.currentThread().getName() + " received " + msg);

            ContactArrayAdapter<Contact> adapter = AppGlobalState.getMainListAdapter();

            switch (msg.what) {
                case 0:
                {
                    Toast.makeText(mContext, "NOTIFY UPDATE with No data Received",
                            Toast.LENGTH_SHORT).show();
                    removeProgressBar();
                    break;
                }

                case PRESENCE_IMS_UNSOL_PUBLISH_TRIGGER:
                {
                    PresPublishTriggerType val = (PresPublishTriggerType) msg.obj;

                    switch (val.getPublishTrigeerType())
                    {
                        case QRCS_PRES_PUBLISH_TRIGGER_ETAG_EXPIRED:
                        {
                            Log.d(TAG, "IMSP_ETAG_EXPIRED");
                            Toast.makeText(mContext,
                                    "Publish trigger for IMSP_ETAG_EXPIRED",
                                    Toast.LENGTH_SHORT).show();
                            break;

                        }
                        case QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_LTE_VOPS_ENABLED:
                        {
                            Log.d(TAG, "IMSP_RAT_CHANGE_LTE");
                            Toast.makeText(mContext,
                                    "Publish trigger for IMSP_RAT_CHANGE_LTE_VOPS_ENABLED",
                                    Toast.LENGTH_SHORT).show();
                            ContactInfo.networkTypeLTE = true;
                            ContactInfo.vopsEnabled = true;
                            ContactInfo.networkTypeEHRPD = false;
                            Log.d(TAG, "Before triggerUpdateOnDisplayedContactInfo");
                            triggerUpdateOnDisplayedContactInfo();
                            Log.d(TAG, "After triggerUpdateOnDisplayedContactInfo");
                            break;

                        }
                        case QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_LTE_VOPS_DISABLED:
                        {
                            Toast.makeText(mContext,
                                    "Publish trigger for IMSP_RAT_CHANGE_LTE_VOPS_DISABLED",
                                    Toast.LENGTH_SHORT).show();
                            ContactInfo.networkTypeLTE = true;
                            ContactInfo.vopsEnabled = false;
                            ContactInfo.networkTypeEHRPD = false;
                            Log.d(TAG, "Before triggerUpdateOnDisplayedContactInfo");
                            triggerUpdateOnDisplayedContactInfo();
                            Log.d(TAG, "After triggerUpdateOnDisplayedContactInfo");
                            break;
                        }
                        case QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_EHRPD:
                        {
                            Log.d(TAG, "IMSP_RAT_CHANGE_EHRPD");
                            Toast.makeText(mContext,
                                    "Publish trigger for IMSP_RAT_CHANGE_EHRPD",
                                    Toast.LENGTH_SHORT).show();
                            ContactInfo.networkTypeLTE = false;
                            ContactInfo.vopsEnabled = false;
                            ContactInfo.networkTypeEHRPD = true;
                            Log.d(TAG, "Before triggerUpdateOnDisplayedContactInfo");
                            triggerUpdateOnDisplayedContactInfo();
                            Log.d(TAG, "After triggerUpdateOnDisplayedContactInfo");
                            break;

                        }
                        case QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_HSPAPLUS:
                        {
                            Log.d(TAG, "IMSP_AIRPLANE_MODE");
                            Toast.makeText(mContext,
                                    "Publish trigger for IMSP_AIRPLANE_MODE",
                                    Toast.LENGTH_SHORT).show();
                            break;
                        }
                        case QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_2G:
                        {
                            Log.d(TAG, "QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_2G");
                            Toast.makeText(mContext,
                                    "Publish trigger for QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_2G",
                                    Toast.LENGTH_SHORT).show();
                            ContactInfo.networkTypeLTE = false;
                            ContactInfo.vopsEnabled = false;
                            ContactInfo.networkTypeEHRPD = true;
                            Log.d(TAG, "Before triggerUpdateOnDisplayedContactInfo");
                            triggerUpdateOnDisplayedContactInfo();
                            Log.d(TAG, "After triggerUpdateOnDisplayedContactInfo");
                            break;
                        }
                        case QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_3G:
                        {
                            Log.d(TAG, "QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_3G");
                            Toast.makeText(mContext,
                                    "Publish trigger for QRCS_PRES_PUBLISH_TRIGGER_MOVE_TO_3G",
                                    Toast.LENGTH_SHORT).show();
                            ContactInfo.networkTypeLTE = false;
                            ContactInfo.vopsEnabled = false;
                            ContactInfo.networkTypeEHRPD = true;
                            Log.d(TAG, "Before triggerUpdateOnDisplayedContactInfo");
                            triggerUpdateOnDisplayedContactInfo();
                            Log.d(TAG, "After triggerUpdateOnDisplayedContactInfo");
                            break;
                        }
                        default:
                            Toast.makeText(mContext, "Publish trigger for unknow val=" +
                                    val, Toast.LENGTH_SHORT).show();
                    }
                    break;
                }

                case PRESENCE_IMS_UNSOL_NOTIFY_UPDATE:
                {
                    PresResInfo [] parsedContactList = (PresResInfo[]) msg.obj;
                    if(parsedContactList != null && parsedContactList.length > 0)
                    {
                        int num = parsedContactList.length;
                            Log.d(TAG,"  aks2    num   =  "+num);
                        if (num > 1) {
                            Toast.makeText(mContext, "NOTIFY UPDATE Received for  " +
                                    num + " Contacts.", Toast.LENGTH_SHORT).show();
                        } else {
                        if(parsedContactList[0].getResUri() != null)
                        {
                            String phone = parsedContactList[0].getResUri();
                                Toast.makeText(mContext, "NOTIFY UPDATE Received for " +
                                        phone, Toast.LENGTH_SHORT).show();
                        }
                        else
                        {
                            Toast.makeText(mContext, "NOTIFY UPDATE Received for empty number"
                                                , Toast.LENGTH_SHORT).show();
                        }
                    }

                    adapter.notifyDataSetChanged();
                    removeProgressBar();
                    triggerUpdateOnDisplayedContactInfo();
                    }

                    break;
                }
                case PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_AVAILABILITY:
                {

                    QRCSString pPresentityURI = (QRCSString) msg.obj;
                    if(pPresentityURI != null)
                    {
                        String phone = pPresentityURI.getQRCSString().toString();
                        Toast.makeText(mContext, "NOTIFY UPDATE Received for " +
                                phone, Toast.LENGTH_SHORT).show();

                        adapter.notifyDataSetChanged();
                        removeProgressBar();
                        triggerUpdateOnDisplayedContactInfo();
                    }
                    else
                    {

                        Toast.makeText(mContext, "NOTIFY UPDATE Received for " +
                                "empty number", Toast.LENGTH_SHORT).show();
                    }
                    /*PresTupleInfo [] parsedContactList = (PresTupleInfo[]) msg.obj;
                    if(parsedContactList != null && parsedContactList.length > 0)
                    {

                        String phone = parsedContactList[0].getContactURI();
                        Toast.makeText(mContext, "NOTIFY UPDATE Received for " +
                                phone, Toast.LENGTH_SHORT).show();

                        adapter.notifyDataSetChanged();
                        removeProgressBar();
                        triggerUpdateOnDisplayedContactInfo();
                    }*/
                    break;
                }

                case PRESENCE_IMS_UNSOL_ENABLER_STATE:
                {
                    String enabler_state = (String) msg.obj;
                    Toast.makeText(mContext, "IMS Enabler State Result =" +
                            enabler_state,
                            Toast.LENGTH_SHORT).show();
                    break;
                }
                case PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_CMDSTATUS:
                {
                    String enabler_state = (String) msg.obj;
                    Toast.makeText(mContext, "" +
                            enabler_state,
                            Toast.LENGTH_SHORT).show();
                    break;
                }
                case PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_SIPRESPONSE:
                {
                    String enabler_state = (String) msg.obj;
                    Toast.makeText(mContext, "" +
                            enabler_state,
                            Toast.LENGTH_SHORT).show();
                    break;
                }

                case PRESENCE_IMS_UNSOL_NOTIFY_UPDATE_SIPRESPONSE404:
                {
                    Log.d(TAG, "ListenerHandler : 404 11111");
                    String sip_message_404 = (String) msg.obj;
                    Toast.makeText(mContext, "" +
                            sip_message_404,
                            Toast.LENGTH_SHORT).show();
                    adapter.notifyDataSetChanged();
                    Log.d(TAG, "ListenerHandler : 404 444444");
                    removeProgressBar();
                    Log.d(TAG, "ListenerHandler : 404 5555555");
                    triggerUpdateOnDisplayedContactInfo();
                    Log.d(TAG, "ListenerHandler : 404 222222");
                    break;
                }

                default:
                    Toast.makeText(mContext, "Unknown mesg " + msg.what + " recieved.",
                            Toast.LENGTH_SHORT).show();

            }
        }

        private void triggerUpdateOnDisplayedContactInfo() {
            ContactInfo contactInfo = AppGlobalState.getContactInfo();
            Log.d(TAG, "ListenerHandler : triggerUpdateOnDisplayedContactInfo");
            if (contactInfo != null) {
                Log.d(TAG, "ListenerHandler : triggerUpdateOnDisplayedContactInfo contactInfo != null");
                Log.d(TAG, "ListenerHandler : triggerUpdateOnDisplayedContactInfo contactInfo.getIndexOfDisplayedContact() "+contactInfo.getIndexOfDisplayedContact());
                contactInfo.populateForm(contactInfo.getIndexOfDisplayedContact());
            }

        }

        private void removeProgressBar() {
            if (AppGlobalState.getProgressDialog() != null) {
                // only in case of subscribe polling request
                AppGlobalState.getProgressDialog().dismiss();
            }
        }
    };
}
