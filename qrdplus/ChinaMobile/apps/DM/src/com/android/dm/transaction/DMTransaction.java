/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */
/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.dm.transaction;

import java.io.IOException;
import java.net.SocketException;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.net.NetworkUtils;
import android.util.Log;
import android.os.Handler;
import android.os.Message;
import android.content.Intent;
import java.net.InetAddress;
import java.net.UnknownHostException;

import com.android.dm.DmNetwork;
import com.android.internal.telephony.PhoneConstants;

import android.provider.ContactsContract;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.FileWriter;
import java.io.FileOutputStream;
import com.android.dm.DMNativeMethod;
import com.android.dm.DmService;
import com.android.dm.DmNetwork;
import com.android.dm.DmJniInterface;

public class DMTransaction implements Runnable {
    private final String TAG = "DMTransaction";

    private DMHttpUtils mDmHttpUtils;
    private Context mContext;
    private int mWorkSpaceId = -1;
    private final boolean DEBUG = true;
    private Handler mManagerHandler;
    private int num = 1;
    private Thread mThread;
    private byte[] response = null;
    private static final int DEFAULT_PROXY = (172<<24)|(0<<16)|(0<<8)|10;//"10.0.0.172"

    public DMTransaction(Context context, Handler handler) {
        mContext = context;
        mManagerHandler = handler;
        mDmHttpUtils = new DMHttpUtils(/* mprefs, */mManagerHandler);
    }

    public void process() {
        mThread = new Thread(this);
        mThread.start();
    }

    public void run() {
        mWorkSpaceId = DMNativeMethod.JgetWorkSpaceId();
        byte[] data = DMNativeMethod.JcopyDataForSending((short) mWorkSpaceId);
        // DMNativeMethod.JsetValueIsAlreadyReceive(false);

        if (data == null) {
            Log.d(TAG, "DMTransaction DATA null");
            return;
        }

        try {
            response = null;
            // Log.d(TAG, "sendData: " +new String(data));
            Log.d(TAG, "sendData: ");
            response = sendData(data);
        }
        /*
         * catch (SocketException e2) { try { Thread.sleep(30*1000);
         * }catch(InterruptedException e1) { } try { response = null; Log.d(TAG,
         * "sendData: " +new String(data)); response = sendData(data); } catch
         * (IOException f) { if (DEBUG) { Log.v(TAG, "Unexpected IOException",
         * f); } } }
         */
        catch (IOException e) {
            if (DEBUG) {
                Log.v(TAG, "Unexpected IOException", e);
            }

            if (DmService.getInstance().getDmInterface().getPppConnectStatus() != PhoneConstants.APN_ALREADY_ACTIVE)
            {
                try {
                    Thread.sleep(30 * 1000);
                } catch (InterruptedException e1) {

                }
            }
            try {
                response = null;
                // Log.d(TAG, "sendData: " +new String(data));
                Log.d(TAG, "sendData: ");
                response = sendData(data);
            } catch (IOException f) {
                if (DEBUG) {
                    Log.v(TAG, "Unexpected IOException2", f);
                }
            }
        }

        if (response != null) {
            // Log.i(TAG, "sendData return response: " +new String(response));
            Log.i(TAG, "sendData return response: ");
            mWorkSpaceId = DMNativeMethod.JgetWorkSpaceId();
            DMNativeMethod.JcopySourceDataToBuffer((short) mWorkSpaceId, response,
                    (long) response.length);
            stepNext();
        } else {
            // for more actions notifyCommBroken
            Log.i(TAG, "!!!!!    NULL response, sendData fail ");
            DMNativeMethod.JnotifyCommBroken();
        }
    }

    protected byte[] sendData(byte[] data) throws IOException {
        boolean isProxySet = true;
        if (DmService.getInstance().getAPN().equals(DmService.APN_CMWAP)) {
            isProxySet = true;
        } else {
            isProxySet = false;
        }
        // isProxySet = false;
        Log.d(TAG, "isProxySet==" + isProxySet);

        try {
            ensureRouteToHost();
        } catch (java.io.IOException e) {
            Log.i(TAG, "IOException : " + e.getMessage());
            Message msg = mManagerHandler.obtainMessage(DMDefine.PIM_EVENT.PIM_EVENT_COMM_ERROR);
            mManagerHandler.sendMessage(msg);
            throw new IOException(e.getMessage());
            // return null;
        }
        String replaceserver = new String(DMNativeMethod.JgetReplaceServerAddress());
        // String replaceserver=DmService.getInstance().getServerAddr();
        Log.d(TAG, "sendData replaceserver *****" + replaceserver);

        try {
            if (isProxySet == true) {
                int portNum = 0;
                try {
                    portNum = Integer.valueOf(DmService.getInstance().getProxyPort(mContext));
                } catch (java.lang.NumberFormatException exp) {
                    Log.d(TAG, "NumberFormatException");
                    portNum = 0;
                }
                return mDmHttpUtils.httpConnection(
                        mContext, /* mprefs.getServerAdress(), */replaceserver,
                        /*
                         * Integer.valueOf(DmService.getInstance().getServerPort(
                         * ))
                         */7001, // Use proxy port, need address port
                        data, mDmHttpUtils.HTTP_POST_METHOD,
                        isProxySet, DmService.getInstance().getProxy(mContext),
                        portNum, "OMADM", "mvpdm");
            } else {
                return mDmHttpUtils.httpConnection(
                        mContext, /* mprefs.getServerAdress(), */replaceserver,
                        /*
                         * Integer.valueOf(DmService.getInstance().getServerPort(
                         * ))
                         */7001, // Use proxy port, need address port
                        data, mDmHttpUtils.HTTP_POST_METHOD,
                        isProxySet, null,
                        0, "OMADM", "mvpdm");
            }
        } catch (java.io.IOException e)
        {
            throw new IOException(e.getMessage());
        }
    }

    private void ensureRouteToHost() throws IOException {
        ConnectivityManager connMgr =
                (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        String serverUrl = DmService.getInstance().getServerAddr();
        int inetAddr;
        if (DmService.getInstance().getProxy(mContext) == null
                ||
                (DmService.getInstance().getProxy(mContext) != null && DmService.getInstance()
                        .getProxy(mContext).equals(""))) {
            Uri serverUri = Uri.parse(serverUrl);
            inetAddr = lookupHost(serverUri.getHost());
            if (inetAddr == -1) {
                Log.e(TAG, "Cannot establish route for " + serverUrl + ": Unknown host");
                inetAddr = DEFAULT_PROXY;
            }
            if (!connMgr.requestRouteToHost(DmNetwork.DMNetworkType,inetAddr))
            {
                Log.e(TAG, "Cannot requestRouteToHost for " + inetAddr + "!");
                if (!connMgr.requestRouteToHost(DmNetwork.DMNetworkType,DEFAULT_PROXY)) {
                    Log.e(TAG, "requestRouteToHost fail for "+inetAddr);
                    throw new IOException("Cannot establish route to proxy " + inetAddr);
                }
            }
        } else {
            String proxyUrl = DmService.getInstance().getProxy(mContext);
            Log.i(TAG, "ensureRouteToHost proxyUrl: " + proxyUrl);
            // Uri proxyUri = Uri.parse(proxyUrl);
            // inetAddr = lookupHost(proxyUri.getHost());
            inetAddr = lookupHost(proxyUrl);
            Log.i(TAG, "ensureRouteToHost inetAddr: " + inetAddr);
            if (inetAddr == -1) {
                throw new IOException("Cannot establish route for " + proxyUrl + ": Unknown host");
            } else {
                Log.e(TAG, "It will be normal after DM has been installed!");
                if (!connMgr.requestRouteToHost(DmNetwork.DMNetworkType,
                    inetAddr)) {
                    throw new IOException("Cannot establish route to proxy " + inetAddr);
                }
            }
        }

    }

    public void setWorkSpaceId(int id) {
        mWorkSpaceId = id;
    }

    public boolean sendSyncMLData() {
        process();
        return true;
    }

    public void stepNext() {
        DMNativeMethod.JstepDataReceive();
    }

    // lookup host name
    private int lookupHost(String hostname) {
        InetAddress inetAddress;
        try {
            inetAddress = InetAddress.getByName(hostname);
        } catch (UnknownHostException e) {
            return -1;
        }
        byte[] addrBytes;
        int addr;
        addrBytes = inetAddress.getAddress();
        addr = ((addrBytes[3] & 0xff) << 24)
                | ((addrBytes[2] & 0xff) << 16)
                | ((addrBytes[1] & 0xff) << 8)
                | (addrBytes[0] & 0xff);
        return addr;
    }
}
