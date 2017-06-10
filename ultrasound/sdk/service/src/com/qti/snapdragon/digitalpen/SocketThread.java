/*===========================================================================
                           SocketThread.java

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.

Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.snapdragon.digitalpen;

import java.io.IOException;
import java.io.InputStream;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.util.Log;

/**
 * ReceiverSocketThread: The socket thread class, each socket that is shared
 * with the daemon is from (or extends) this class. In the run function, the
 * thread continuously tries to connect to the socket shared with the daemon
 * (since we do not know when the daemon is up and ready, after the connection
 * is set, in case the thread needs to wait for incoming data from the daemon
 * (aka receiver), the thread invokes the "receiveWorker" method.
 */
class SocketThread extends Thread {
    private static final int MAX_CONNECTION_RETRIES = 50;
    private static final int CONNECTION_SLEEP_TIME = 500;
    private LocalSocket mSocket;
    private final String mSocketPath;
    private boolean mConnected;
    private ReceiveWorker mWorker;
    private static final String TAG = "SocketThread";

    public interface ReceiveWorker {
        /**
         * If the extending thread is a receiver (waits for incoming data), then
         * he needs to implement the receiverWorker, please note that after
         * invoking this method the thread exists.
         */
        void receiveLoop(InputStream input) throws IOException, InterruptedException;
    }

    /**
     * C'tor: Argument is the path (file system type) for the socket.
     */
    public SocketThread(String socketPath, ReceiveWorker worker) {
        mSocketPath = socketPath;
        mConnected = false;
        mWorker = worker;
    }

    /** Checks if the socket is connected */
    public boolean isConnected() {
        return mConnected;
    }

    /** Returns the LocalSocket object */
    public LocalSocket getSocket() {
        return mSocket;
    }

    @Override
    public void run() {
        // The recover mechanism is for retrying to reconnect to socket
        // in-case
        // socket closes unexpectedly, this could happen because in some
        // cases
        // the daemon has to be restarted. Recover mechanism is there as
        // long
        // as connecting to the socket succeeds. If the 5 seconds retries to
        // connect to socket fails, we disable recover mechanism.
        boolean recover = true;
        try {
            while (recover) {
                // max retry time: MAX_CONNECTION_RETRIES * SHORT_SLEEP TIME
                int retriesLeft = MAX_CONNECTION_RETRIES;
                while (0 < --retriesLeft) {
                    try {
                        // Connects to the socket path given
                        Log.d(TAG, "connecting to : " + mSocketPath + ", retriesLeft: "
                                + retriesLeft);
                        mSocket = new LocalSocket();
                        mSocket.connect(new LocalSocketAddress(mSocketPath,
                                LocalSocketAddress.Namespace.FILESYSTEM));
                        mConnected = true;
                        break;
                    } catch (IOException e) {
                        Log.d(TAG, "socket not connecting to: " + mSocketPath);
                        // Do nothing, the loop will make sure we keep
                        // trying
                    }

                    // Sleep Short time msec before trying again
                    Thread.sleep(CONNECTION_SLEEP_TIME);
                }
                // We had no luck trying to connect to socket
                if (retriesLeft == 0) {
                    Log.e(TAG, "no more retries: " + mSocketPath);
                    // No point in recovering
                    recover = false;
                }

                if (!mConnected) { // Thread wasn't able to connect
                    Log.w(TAG, "Connection to " + mSocketPath + " unsuccessful");
                    return;
                }

                InputStream socketInput = null;
                try {
                    // Invoke the receiverWorker which should wait for all
                    // the incoming
                    // data from the daemon
                    socketInput = mSocket.getInputStream();
                    mWorker.receiveLoop(socketInput);
                } catch (IOException e) {
                    Log.w(TAG, "IOException" + e.getMessage());
                } catch (InterruptedException e) {
                    throw e; // expected, rethrow
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } // while (recover)
        } catch (InterruptedException e) {
            // Do nothing, falls through to close
        }
        if (null != mSocket) {
            try {
                mSocket.close();
            } catch (IOException e) {
                Log.w(TAG, "IOException while closing socket " + e.getMessage());
            }
        }
        Log.d(TAG, "Thread closing: " + getClass());
    }

}
