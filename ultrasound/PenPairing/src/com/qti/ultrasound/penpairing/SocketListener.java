/******************************************************************************
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2014 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/
package com.qti.ultrasound.penpairing;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;

public class SocketListener extends Thread {

    private static final int NUM_TRIES_SOCKET = 10;

    private static final int DELAY_SOCKET_RECONNECT_MSEC = 500;

    private final int MESSAGE_LEN_BYTES;

    private final String SOCKET_PATH;

    private final DaemonWrapper mDaemon;

    public static final String CONNECTION_FAILED_MESSAGE = "Connection to daemon socket failed. Please try again";

    // Background threads use this Handler to post messages to
    // the main application thread
    private Handler mHandler;

    private LocalSocket mReceiver;

    SocketListener(Handler handler, String localSocketPath, DaemonWrapper daemon, int msgLenBytes) {
        mHandler = handler;
        SOCKET_PATH = localSocketPath;
        mDaemon = daemon;
        mReceiver = new LocalSocket();
        MESSAGE_LEN_BYTES = msgLenBytes;
    }

    @Override
    public void run() {
        Log.d(this.toString(), "Socket thread running");
        int numTries = 0;
        boolean isConnected = false;
        InputStream input = null;
        while (NUM_TRIES_SOCKET >= ++numTries && !isConnected && mDaemon.isStarted()) {
            try {
                Log.d(this.toString(), "Attempt " + numTries + " to connect to socket...");
                mReceiver.connect(new LocalSocketAddress(SOCKET_PATH,
                        LocalSocketAddress.Namespace.FILESYSTEM));
                Log.d(this.toString(), "Connected to socket!");
                input = mReceiver.getInputStream();
                isConnected = true;
            } catch (IOException e) {
                Log.e(this.toString(), e.getMessage());
            }
            try {
                Thread.sleep(DELAY_SOCKET_RECONNECT_MSEC);
            } catch (InterruptedException e) {
                Log.e(this.toString(), "App socket Thread Interruped while sleeping");
            }
        }

        if (!isConnected) {
            // Show error msg only in case of connection failure
            if (mDaemon.isStarted()) {
                Message msg = Message.obtain();
                msg.what = SemiAutomaticActivity.ALERT_FAIL_MESSAGE;
                msg.obj = CONNECTION_FAILED_MESSAGE;
                mHandler.sendMessage(msg);
            }
            return;
        }

        while (mDaemon.isStarted()) {
            try {
                byte[] buffer = new byte[MESSAGE_LEN_BYTES];
                int bytesRead = input.read(buffer);
                Log.d(this.toString(), "Bytes read: " + bytesRead);
                if (-1 == bytesRead) {
                    break;
                }
                IntBuffer intBuf = ByteBuffer.wrap(buffer).order(ByteOrder.LITTLE_ENDIAN)
                        .asIntBuffer();
                final int[] res = new int[intBuf.remaining()];
                intBuf.get(res);
                Message msg = Message.obtain();
                msg.what = SemiAutomaticActivity.SOCKET_EVENT_MESSAGE;
                msg.obj = res;
                mHandler.sendMessage(msg);
            } catch (IOException e) {
                Log.e(this.toString(), e.getMessage());
                mDaemon.stop();
            }
        }
    }

}
