/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount;

import com.suntek.mway.rcs.client.api.RCSServiceListener;
import com.suntek.mway.rcs.client.api.autoconfig.RcsAccountApi;
import com.suntek.mway.rcs.client.api.emoticon.EmoticonApi;
import com.suntek.mway.rcs.client.api.im.impl.MessageApi;
import com.suntek.mway.rcs.client.api.im.impl.PaMessageApi;
import com.suntek.mway.rcs.client.api.impl.groupchat.ConfApi;
import com.suntek.mway.rcs.client.api.mcontact.McontactApi;
import com.suntek.mway.rcs.client.api.profile.impl.ProfileApi;
import com.suntek.mway.rcs.client.api.publicaccount.impl.PublicAccountApi;
import com.suntek.mway.rcs.client.api.support.RcsSupportApi;

import android.content.Context;
import android.content.Intent;
import android.os.RemoteException;
import android.util.Log;

public class RcsApiManager {

    public static final String CONF_API_BIND_ACITON = "com.suntek.mway.rcs.CONF_API_BIND_ACITON";

    private static boolean isMessageApiBind = false;

    private static ConfApi mConfApi = new ConfApi();

    private static RcsAccountApi mAccountApi = new RcsAccountApi();

    private static PublicAccountApi mPublicAccount = new PublicAccountApi();

    private static McontactApi mMcontactApi = new McontactApi();

    private static MessageApi mMessageApi = new MessageApi();

    private static ProfileApi mProfileApi = new ProfileApi();

    private static PaMessageApi mPaMessageApi = new PaMessageApi();

    private static EmoticonApi mEmoticonApi = new EmoticonApi();

    private static boolean mIsRcsServiceInstalled;

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

    public static ProfileApi getProfileApi() {
        return mProfileApi;
    }

    public static PaMessageApi getPaMessageApi() {
        return mPaMessageApi;
    }

    public static EmoticonApi getEmoticonApi() {
        return mEmoticonApi;
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
        mProfileApi.init(context, new RCSServiceListener() {

            @Override
            public void onServiceDisconnected() throws RemoteException {
                mProfileApi = null;
            }

            @Override
            public void onServiceConnected() throws RemoteException {
            }
        });
        mPaMessageApi.init(context, new RCSServiceListener() {

            @Override
            public void onServiceDisconnected() throws RemoteException {
                mPaMessageApi = null;
            }

            @Override
            public void onServiceConnected() throws RemoteException {
            }
        });
        mEmoticonApi.init(context,new RCSServiceListener() {
            public void onServiceDisconnected() throws RemoteException {
                Log.d("RCS_UI", "EmoticonApi disconnected");
            }

            public void onServiceConnected() throws RemoteException {
                Log.d("RCS_UI", "EmoticonApi connected");
            }
        });
    }

    public static boolean isRcsServiceInstalled() {
        return mIsRcsServiceInstalled;
    }
}
