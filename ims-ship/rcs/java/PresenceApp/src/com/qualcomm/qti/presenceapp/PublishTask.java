/*************************************************************************
 Copyright (c)2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
*************************************************************************/

package com.qualcomm.qti.presenceapp;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Looper;
import android.os.RemoteException;
import android.util.Log;
import android.widget.Toast;

import com.qualcomm.qti.presence.PresCapInfo;
import com.qualcomm.qti.presenceapp.R;
import com.qualcomm.qti.rcsimssettings.QrcsImsSettingsIntType;
import com.qualcomm.qti.rcsservice.CDInfo;
import com.qualcomm.qti.rcsservice.StatusCode;

public class PublishTask extends AsyncTask<Void, Integer, Integer> {

    final String TAG = "PublishTask";
    Context mContext;
    ProgressDialog dialog;
    PublishTask me;

    public PublishTask() {
        mContext = AppGlobalState.getMainActivityContext();
        me = this;
        ContactInfo.firstPublish = true;
    }

    @Override
    protected void onPreExecute() {
        super.onPreExecute();
    }

    private void initProgressBar() {
        mContext = AppGlobalState.getMainActivityContext();
        dialog = new ProgressDialog(mContext);
        dialog.setMessage("API Request in progress...");
        dialog.setCancelable(false);
        dialog.show();
    }

    @Override
    protected Integer doInBackground(Void... params) {
        Log.d(TAG, "doInBackground(), Thread=" +
                Thread.currentThread().getName());

        if (Looper.myLooper() == null) {
            Looper.prepare();
        }
        return sendPublishRequest();
    }

    private int sendPublishRequest() {
        Log.d(TAG, "sendPublishRequest");

        PresCapInfo pMyCapInfo = new PresCapInfo();

        try
        {
            if(MainActivity.imsSettingService != null)
            {
                QrcsImsSettingsIntType client_handle = new QrcsImsSettingsIntType();
                client_handle.setQrcsImsSettingsIntType(AppGlobalState.getQrcsImsSettingsclienthandle());
                MainActivity.imsSettingService
                        .QrcsImsSettings_GetQipcallConfig(client_handle);
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        catch (Exception e) {
            e.printStackTrace();
        }

        SharedPreferences setting = getSharedPrefHandle(AppGlobalState.IMS_PRESENCE_MY_INFO);

        String myNum = setting.getString(
                mContext.getString(R.string.myNumtext), "");
        String uri1 = setting.getString(
                mContext.getString(R.string.uri1text), "");
        String uri2 = setting.getString(
                mContext.getString(R.string.uri2text), "");

        int statusValue = 0;
        if (!ContactInfo.networkTypeLTE || !ContactInfo.vopsEnabled) {
            statusValue = 0; // CLOSED
            Log.d("PRESENCE_UI", "CLOSED statusValue ==" + statusValue);
        } else {
            statusValue = 1;
            Log.d("PRESENCE_UI", "statusValue ==" + statusValue);
        }

        String description = setting.getString(
                mContext.getString(R.string.descriptiontext), "");
        String ver = setting.getString(
                mContext.getString(R.string.vertext), "");
        String serviceId = setting.getString(
                mContext.getString(R.string.serviceIdtext), "");

        int audioSupported = 1;
        int videoSupported;

        SharedPreferences preferences = mContext.getSharedPreferences(
                "ImsPresencePrefMyInfo", mContext.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();

        if (Settings.isVt_calling_enabled && Settings.isMobile_data_enabled) {
            Log.d("PRESENCE_UI", "Video is supported 1 and DATA is ON from NV");
            videoSupported = 1;
            editor.putString("Description", "VoLTE Voice and Video Service");
            description = "VoLTE Voice and Video Service";

        } else {
            Log.d("PRESENCE_UI",
                    "Video is not supported and DATA is OFF from NV");
            videoSupported = 0;
            editor.putString("Description", "VoLTE Service");
            description = "VoLTE Service";
        }

        if(AppGlobalState.getOperatorMode() == AppGlobalState.ATT_MODE)
        {
            Log.d(TAG, "AppGlobalState.ATT_MODE");
            if (ContactInfo.networkTypeLTE && ContactInfo.vopsEnabled)
            {
                audioSupported = 1;
                videoSupported = 1;
            }
        }

        String myNumUri = uri1 + myNum + uri2;

        Log.d("PRESENCE_UI", "statusValue 2 ==" + statusValue);

        Log.d("PRESENCE_UI", "myNumUri = " + myNumUri + " description = " + description + " ver = "
                + ver +
                " serviceId=" + serviceId + " audioSupported =  " + audioSupported
                + " videoSupported=  " + videoSupported);

        if (!ContactInfo.networkTypeLTE || !ContactInfo.vopsEnabled) {
            audioSupported = 0;
            videoSupported = 0;
        }

        Log.d("PRESENCE_UI", "myNumUri is filled");
        if (myNumUri.length() > 0)
        {
            pMyCapInfo.setContactURI(myNumUri);
        }
            CDInfo cdInfo = new CDInfo();
            cdInfo.setIPVoiceSupported((audioSupported == 0) ? false : true);
            cdInfo.setIPVideoSupported((videoSupported == 0) ? false : true);
            cdInfo.setCDViaPresenceSupported(true);


            SharedPreferences presencePref = mContext.getSharedPreferences(
                    "presencedata", Context.MODE_PRIVATE);
            boolean ftSupported = presencePref.getBoolean("FT_KEY", false);
            boolean chatSupported = presencePref.getBoolean("CHAT_KEY", false);
            Log.d("PRESENCE_UI", "ftSupported =" + ftSupported);
            Log.d("PRESENCE_UI", "chatSupported =" + chatSupported);

            if (Settings.isMobile_data_enabled && AppGlobalState.getOperatorMode() == AppGlobalState.VZW_MODE) {

                if (ftSupported) {
                    cdInfo.setFtSupported(true);
                    Log.d("PRESENCE_UI", "cdInfo.setFtSupported is set to TRUE");
                } else {
                    cdInfo.setFtSupported(false);
                    Log.d("PRESENCE_UI", "cdInfo.setFtSupported is set to FALSE");
                }
                if (chatSupported) {
                    cdInfo.setImSupported(true);
                    if(AppGlobalState.getOperatorMode() == AppGlobalState.VZW_MODE)
                    {
                        Log.d(TAG, "AppGlobalState.VZW_MODE");
                        cdInfo.setbFullSnFGroupChatSupported(true);
                        Log.d("PRESENCE_UI", "IM and SnF Group chat is set to TRUE");
                    }
                } else {
                    cdInfo.setImSupported(false);
                    cdInfo.setbFullSnFGroupChatSupported(false);
                    Log.d("PRESENCE_UI", "IM and SnF Group chat is set to FALSE");
                }
            }

            else {
                cdInfo.setFtSupported(false);
                cdInfo.setImSupported(false);
                cdInfo.setbFullSnFGroupChatSupported(false);

                Log.d("PRESENCE_UI", "FT, IM and SnF Group chat is set to FALSE");
            }

            pMyCapInfo.setCdInfo(cdInfo);


            try
            {
                if(AppGlobalState.getPresenceService() != null)
                {
                    Log.d("PRESENCE_UI", "PublishTask : sendPublishRequest : AppGlobalState.getPresenceSerrviceHandle() "+AppGlobalState.getPresenceSerrviceHandle());
                    Log.d("PRESENCE_UI", "PublishTask : sendPublishRequest : pMyCapInfo "+pMyCapInfo);
                    RequestInfo requestinfoObject = new RequestInfo();
                    requestinfoObject.URI = new String[1];
                    requestinfoObject.URI[0] = pMyCapInfo.getContactURI();
                    requestinfoObject.userData = AppGlobalState.getpUserDataValue();
                    AppGlobalState.requestinfo.add(requestinfoObject);
                    StatusCode status = AppGlobalState.getPresenceService().QPresService_PublishMyCap(AppGlobalState.getPresenceSerrviceHandle(), pMyCapInfo, AppGlobalState.getpUserData());
                    Log.d("PRESENCE_UI", "PublishTask : sendPublishRequest : status "+status.getStatusCode().toString());
                    return status.getStatusCode().ordinal();
                }
                else
                {
                    Log.d("PRESENCE_UI", "PublishTask : sendPublishRequest ");
                    return -1;
                }
            } catch (RemoteException e) {
                e.printStackTrace();
                Log.d("PRESENCE_UI", "RemoteException, Dont sent PUBLISH");
                return -2;
            } catch (Exception e) {
                Log.d("PRESENCE_UI", "Exception, Dont sent PUBLISH");
                return -3;
            }
    }

    private SharedPreferences getSharedPrefHandle(String imsPresencePref) {
        SharedPreferences settings = mContext.getSharedPreferences(imsPresencePref, 0);

        return settings;
    }

    @Override
    protected void onPostExecute(Integer result) {
        super.onPostExecute(result);
        Log.d(TAG, "onPostExecute(), Thread=" + Thread.currentThread().getName());
        Toast.makeText(mContext, "Publish Rich Result =" +
                    result, Toast.LENGTH_SHORT).show();

    }
}
