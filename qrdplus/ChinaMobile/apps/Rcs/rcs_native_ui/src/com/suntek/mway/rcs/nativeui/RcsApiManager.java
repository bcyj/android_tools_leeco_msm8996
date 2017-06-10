/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui;

import com.suntek.mway.rcs.client.api.RCSServiceListener;
import com.suntek.mway.rcs.client.api.autoconfig.RcsAccountApi;
import com.suntek.mway.rcs.client.api.im.impl.MessageApi;
import com.suntek.mway.rcs.client.api.impl.groupchat.ConfApi;
import com.suntek.mway.rcs.client.api.mcloud.McloudFileApi;
import com.suntek.mway.rcs.client.api.mcontact.McontactApi;
import com.suntek.mway.rcs.client.api.publicaccount.impl.PublicAccountApi;
import com.suntek.mway.rcs.client.api.profile.impl.ProfileApi;
import com.suntek.mway.rcs.client.api.support.RcsSupportApi;
import com.suntek.mway.rcs.client.api.voip.impl.RichScreenApi;

import android.content.Context;
import android.content.Intent;
import android.os.RemoteException;

public class RcsApiManager {

    public static final String CONF_API_BIND_ACITON = "com.suntek.mway.rcs.CONF_API_BIND_ACITON";
    private static MessageApi mMessageApi = new MessageApi();
    private static boolean isMessageApiBind = false;
    private static ConfApi mConfApi = new ConfApi();
    private static RcsAccountApi mAccountApi = new RcsAccountApi();
    private static PublicAccountApi mPublicAccount = new PublicAccountApi();
    private static McontactApi mMcontactApi = new McontactApi();
    private static boolean mIsRcsServiceInstalled;
    private static McloudFileApi mMcloudFileApi = new McloudFileApi();
    private static RichScreenApi mRichScreenApi = new RichScreenApi(null);
    private static ProfileApi mProfileApi = new ProfileApi();

    public static McloudFileApi getMcloudFileApi(){
        return mMcloudFileApi;
    }
    public static PublicAccountApi getPublicAccountApi() {
        return mPublicAccount;
    }
    public static MessageApi getMessageApi() {
        return mMessageApi;
    }

    public static boolean isMessageApiBind() {
        return isMessageApiBind;
    }

    public static ConfApi getConfApi() {
        return mConfApi;
    }

    public static RcsAccountApi getAccountApi() {
        return mAccountApi;
    }

    public static McontactApi getMcontactApi() {
        return mMcontactApi;
    }

    public static RichScreenApi getRichScreenApi() {
        return mRichScreenApi;
    }

    public static ProfileApi getProfileApi() {
        return mProfileApi;
    }

    public static void init(final Context context) {
        mIsRcsServiceInstalled = RcsSupportApi.isRcsServiceInstalled(context);
        if (!mIsRcsServiceInstalled) {
            return;
        }

        mMessageApi.init(context, new RCSServiceListener() {
            @Override
            public void onServiceDisconnected() throws RemoteException {
                isMessageApiBind = false;
            }

            @Override
            public void onServiceConnected() throws RemoteException {
                isMessageApiBind = true;
            }
        });

        mConfApi.init(context, new RCSServiceListener() {
            @Override
            public void onServiceDisconnected() throws RemoteException {
            }

            @Override
            public void onServiceConnected() throws RemoteException {
                context.sendBroadcast(new Intent(CONF_API_BIND_ACITON));
            }
        });

        mAccountApi.init(context, new RCSServiceListener() {
            @Override
            public void onServiceDisconnected() throws RemoteException {
            }

            @Override
            public void onServiceConnected() throws RemoteException {
            }
        });
      mPublicAccount.init(context, new RCSServiceListener() {
            @Override
            public void onServiceDisconnected() throws RemoteException {
            }

            @Override
            public void onServiceConnected() throws RemoteException {
            }
        });
      mMcontactApi.init(context, new RCSServiceListener() {
          @Override
          public void onServiceDisconnected() throws RemoteException {
          }

          @Override
          public void onServiceConnected() throws RemoteException {
          }
      });

      mRichScreenApi.init(context, new RCSServiceListener() {
            public void onServiceDisconnected() throws RemoteException {
            }
            public void onServiceConnected() throws RemoteException {
            }
        });
      mMcloudFileApi.init(context,new RCSServiceListener() {
            public void onServiceDisconnected() throws RemoteException {
            }
            public void onServiceConnected() throws RemoteException {
            }
        });
      mProfileApi.init(context,new RCSServiceListener() {
            public void onServiceDisconnected() throws RemoteException {
            }
            public void onServiceConnected() throws RemoteException {
            }
        });
    }
}
