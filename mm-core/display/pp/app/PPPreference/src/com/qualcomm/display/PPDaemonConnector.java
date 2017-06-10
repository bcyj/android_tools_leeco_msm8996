/******************************************************************************
 * @file    PPDaemonConnector.java
 * @brief   Connects to the Native PP Daemon using sockets
 *
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2012 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *******************************************************************************/
package com.qualcomm.display;

import android.net.LocalSocketAddress;
import android.net.LocalSocket;
import android.os.SystemClock;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class PPDaemonConnector implements Runnable {

    private static final String TAG = "PPDaemonConnector";
    private InputStream in = null;
    private OutputStream mOutStream;
    private boolean mPPOn = false;
    private boolean mStopListener = false;
    private boolean mConnected = false;
    private double saturation = 0;
    private double contrast = 0;
    private String mDaemonResponse = null;
    private static final int MAX_CMD_LEN = 64;
    private static final int MAX_RESP_TRY_COUNT = 100;
    private static final String DAEMON_SOCKET = "pps";
    public static final String CMD_POSTPROC_ON     =  "pp:on";
    public static final String CMD_POSTPROC_OFF    =  "pp:off";
    public static final String CMD_POSTPROC_SET    =  "pp:set:hsic";
    public static final String CMD_POSTPROC_STATUS =  "pp:query:status:postproc";


    public void run() {
        try {
            listenToSocket();
        } catch (Exception e) {
            Log.e(TAG, "Error in NativeDaemonConnector of PP");
            e.printStackTrace();
        }
    }

    private void listenToSocket() {
        LocalSocket daemonSock = new LocalSocket();
        LocalSocketAddress address = new LocalSocketAddress(DAEMON_SOCKET,
                                        LocalSocketAddress.Namespace.RESERVED);
        try {
            byte[] buf = new byte[MAX_CMD_LEN];
            daemonSock.connect(address);
            Log.d(TAG, "Connected to the daemon socket");
            synchronized (this) {
                in = daemonSock.getInputStream();
                mOutStream = daemonSock.getOutputStream();
                if ((in == null) || (mOutStream == null)) {
                    return;
                }
                mConnected = true;
            }
            while(!isListenerStopped()) {
                int len = in.read(buf, 0, buf.length);
                if (len <= 0)
                    break;
                mDaemonResponse = new String(buf, 0, len);
                Log.d(TAG, "Response from PostProc Daemon: " + mDaemonResponse);
            }
        } catch (IOException e) {
            Log.e(TAG,"Failed to connect to daemon socket");
            e.printStackTrace();
        } finally {
            try {
                Log.d(TAG, "Closing the socket!");
                synchronized (this) {
                    mConnected = false;
                    if ( in != null)
                        in.close();
                    if ( mOutStream != null)
                        mOutStream.close();
                    daemonSock.close();
                }
            } catch (IOException e) {
                Log.e(TAG, "Failed to release socket");
                e.printStackTrace();
            }
        }
    }

    public synchronized boolean startPP() {

        if (mPPOn)
            return true;

        if (mOutStream == null) {
            Log.e(TAG, "Socket not connected...");
            return false;
        }

        try {
            String cmd = CMD_POSTPROC_ON;
            byte[] buf = cmd.getBytes();
            mOutStream.write(buf);
            mOutStream.flush();
        } catch (IOException e) {
            Log.e(TAG, "Failed to start PP Daemon");
            e.printStackTrace();
            return false;
        }
        boolean isOpSuccess = checkDaemonResponse("Success");
        if (isOpSuccess)
            mPPOn = true;
        return isOpSuccess;
    }

    public synchronized boolean stopPP() {

        if (!mPPOn)
            return true;

        if (mOutStream == null) {
            Log.e(TAG, "Socket not connected...");
            return false;
        }

        try {
            String cmd = CMD_POSTPROC_OFF;
            byte[] buf = cmd.getBytes();
            mOutStream.write(buf);
            mOutStream.flush();
        } catch (IOException e) {
            Log.e(TAG, "Failed to stop PP");
            e.printStackTrace();
            return false;
        }

        boolean isOpSuccess = checkDaemonResponse("Success");
        if (isOpSuccess)
            mPPOn = false;
        return isOpSuccess;
    }

    public synchronized boolean getPPStatus() {
        if (mPPOn)
            return true;

        if (mOutStream == null) {
            Log.d(TAG, "Socket not connected...");
            return false;
        }
        try {
            String cmd = CMD_POSTPROC_STATUS;
            byte[] buf = cmd.getBytes();
            mOutStream.write(buf);
            mOutStream.flush();
        } catch (IOException e) {
            Log.e(TAG, "Failed to get status of PP");
            e.printStackTrace();
            return false;
        }
        boolean isOpSuccess = checkDaemonResponse("Running");
        mPPOn = isOpSuccess;
        return isOpSuccess;
    }

   public synchronized boolean setPPhsic(int h,int s,int i,int c) {

        if (!mPPOn)
            return false;

        if (mOutStream == null) {
            Log.d(TAG, "Socket not connected...");
            return false;
        }

        int hue = h-180;
        s = s-180;
        int intensity = i-255;
        c = c-180;

        saturation = (s/180.0);
        contrast = (c/180.0);

        try {
            StringBuilder builder = new StringBuilder(CMD_POSTPROC_SET);
            builder.append(" ");
            builder.append(hue);
            builder.append(" ");
            builder.append(saturation);
            builder.append(" ");
            builder.append(intensity);
            builder.append(" ");
            builder.append(contrast);
            builder.append('\0');
            mOutStream.write(builder.toString().getBytes());
            mOutStream.flush();
        } catch (IOException e) {
            Log.e(TAG, "Failed to set PP hsic values as"
                    + hue + " "+ saturation + " "+ intensity + " "
                    + contrast);
            e.printStackTrace();
            return false;
        }
        boolean isOpSuccess = checkDaemonResponse("Success");
        if(isOpSuccess) {
            mPPOn = true;
        }
        return isOpSuccess;
    }

    private boolean checkDaemonResponse(String expectedReply) {
        int tryCount = 0;
        boolean reply = false;
        while((mDaemonResponse == null) && (++tryCount < MAX_RESP_TRY_COUNT)) {
            SystemClock.sleep(5);
        }
        if (tryCount == MAX_RESP_TRY_COUNT) {
            Log.d(TAG, "Max tries exceeded waiting for Daemon response!");
            return false;
        }
        if (mDaemonResponse != null) {
            Log.d(TAG, "Expected outcome from daemon " + expectedReply +
                    " and response from daemon " + mDaemonResponse);
            reply = mDaemonResponse.startsWith(expectedReply);
        }
        mDaemonResponse = null;
        return reply;
    }

    public synchronized void stopListener() {
        mStopListener = true;
    }

    public synchronized boolean isListenerStopped() {
        return mStopListener;
    }

    public synchronized boolean isConnected() {
        return mConnected;
    }
}
