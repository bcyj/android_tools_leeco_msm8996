/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2012 The Android Open Source Project
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

package com.android.nfc.echoserver;

import com.android.nfc.Debug;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.android.nfc.DeviceHost.LlcpConnectionlessSocket;
import com.android.nfc.DeviceHost.LlcpServerSocket;
import com.android.nfc.DeviceHost.LlcpSocket;
import com.android.nfc.LlcpException;
import com.android.nfc.LlcpPacket;
import com.android.nfc.NfcService;

import java.io.IOException;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

import android.nfc.dta.DtaHelper; // <DTA>
import com.android.nfc.dhimpl.NativeLlcpConnectionlessSocket;

/**
 * LLCP echo server conforming to the NFC Forum DTA specification 2.0
 */
public class EchoServer2 {
    static boolean DBG = Debug.EchoServer2;
/* update default values based on test cases for LLCP 1.0.3
    static final int DEFAULT_CO_IN_SAP = 0x11;
    static final int DEFAULT_CL_IN_SAP = 0x12;
    static final int DEFAULT_CO_OUT_SAP = 0x10;
    static final int DEFAULT_CL_OUT_SAP = 0x22;
*/
    static final int DEFAULT_CO_IN_SAP = 0x20;
    static final int DEFAULT_CL_IN_SAP = 0x21;
    static final int DEFAULT_CO_OUT_SAP = 0x12;
    static final int DEFAULT_CL_OUT_SAP = 0x11;

    // Link MIU
    static final int MIU = 128;

    static final String TAG = "EchoServer2";
    //static final String CONNECTION_SERVICE_NAME_IN = "urn:nfc:sn:dta-co-echo-in";
    //static final String CONNECTION_SERVICE_NAME_OUT = "urn:nfc:sn:dta-co-echo-out";
    static final String CONNECTIONLESS_SERVICE_NAME_IN = "urn:nfc:sn:dta-cl-echo-in";
    static final String CONNECTIONLESS_SERVICE_NAME_OUT = "urn:nfc:sn:dta-cl-echo-out";
    private String mCoServiceNameIn = null;
    private String mCoServiceNameOut = null;

    ServerThread mServerThread;
    ConnectionlessServer mConnectionlessServer;
    NfcService mService;

    boolean mLtMode;

    public interface WriteCallback {
        public void write(byte[] data);
    }

    public EchoServer2() {
        mService = NfcService.getInstance();
        mLtMode = false;
    }

    /**
     * @param serviceNameIn service name (URN) of the connection-oriented inbound service
     * @param serviceNameOut service name (URN) of the connection-oriented outbound service
     */
    public void setCoServiceNames(String serviceNameIn, String serviceNameOut) {
            mCoServiceNameIn = serviceNameIn;
            mCoServiceNameOut = serviceNameOut;
    }

    static class EchoMachine implements Handler.Callback {
        static final int QUEUE_SIZE = 2;
        //static final int ECHO_DELAY = 3000; // DTA delay = 3s
        static final int ECHO_DELAY = 3200; // increase to 3.2, bigger than 3s, smaller than 5s

        /**
         * ECHO_MIU must be set equal to default_miu in nfcpy.
         * The nfcpy echo server is expected to maintain the
         * packet boundaries and sizes of the requests - that is,
         * if the nfcpy client sends a service data unit of 48 bytes
         * in a packet, the echo packet should have a payload of
         * 48 bytes as well. The "problem" is that the current
         * Android LLCP implementation simply pushes all received data
         * in a single large buffer, causing us to loose the packet
         * boundaries, not knowing how much data to put in a single
         * response packet. The ECHO_MIU parameter determines exactly that.
         * We use ECHO_MIU=48 because of a bug in PN544, which does not respect
         * the target length reduction parameter of the p2p protocol.
         */
        static final int ECHO_MIU = 128;

        final BlockingQueue<byte[]> dataQueue;
        final Handler handler;
        final boolean dumpWhenFull;
        final WriteCallback callback;

        // shutdown can be modified from multiple threads, protected by this
        boolean shutdown = false;

        EchoMachine(WriteCallback callback, boolean dumpWhenFull) {
            this.callback = callback;
            this.dumpWhenFull = dumpWhenFull;
            dataQueue = new LinkedBlockingQueue<byte[]>(QUEUE_SIZE);
            handler = new Handler(this);
        }

        public void pushUnit(byte[] unit, int size) {
            if (dumpWhenFull && dataQueue.remainingCapacity() == 0) {
                if (DBG) Log.d(TAG, "Dumping data unit");
            } else {
                try {
                    // Split up the packet in ECHO_MIU size packets
                    int sizeLeft = size;
                    int offset = 0;
                    if (dataQueue.isEmpty()) {
                        // First message: start echo'ing in 3 seconds
                        handler.sendMessageDelayed(handler.obtainMessage(), ECHO_DELAY);
                    }

                    if (sizeLeft == 0) {
                        // Special case: also send a zero-sized data unit
                        dataQueue.put(new byte[] {});
                    }
                    while (sizeLeft > 0) {
                        int minSize = Math.min(size, ECHO_MIU);
                        byte[] data = new byte[minSize];
                        System.arraycopy(unit, offset, data, 0, minSize);
                        dataQueue.put(data);
                        sizeLeft -= minSize;
                        offset += minSize;
                    }
                } catch (InterruptedException e) {
                    // Ignore
                }
            }
        }

        /** Shuts down the EchoMachine. May block until callbacks
         *  in progress are completed.
         */
        public synchronized void shutdown() {
            dataQueue.clear();
            shutdown = true;
        }

        @Override
        public synchronized boolean handleMessage(Message msg) {
            if (shutdown) return true;
            while (!dataQueue.isEmpty()) {
                callback.write(dataQueue.remove());
            }
            return true;
        }
    }

    public class ServerThread extends Thread implements WriteCallback {
        final EchoMachine echoMachine;

        private static final int TIMEOUT = 3000;

        boolean running = true;
        LlcpServerSocket serverSocket = null;
        LlcpSocket clientSocketIn = null;
        LlcpSocket clientSocketOut = null;

        private volatile boolean timeout = false;
        private Timer timer;

        private int sapIn;
        private int sapOut;
        private String serviceNameIn;
        private String serviceNameOut;

        public ServerThread(String serviceNameIn, String serviceNameOut) {
            super();
            echoMachine = new EchoMachine(this, true);

            if (!mLtMode) {
                sapIn = DEFAULT_CO_IN_SAP;
                sapOut = DEFAULT_CO_OUT_SAP;
                this.serviceNameIn = serviceNameIn;
                this.serviceNameOut = serviceNameOut;
            } else {
                sapIn = DEFAULT_CO_OUT_SAP;
                sapOut = DEFAULT_CO_IN_SAP;
                this.serviceNameIn = serviceNameOut;
                this.serviceNameOut = serviceNameIn;
            }
        }

        private void handleClient(LlcpSocket socket) {
            if (DBG) Log.d(TAG, "handleClient()");
            boolean connectionBroken = false;
            byte[] dataUnit = new byte[1024];

            // Get raw data from remote server
            while (!connectionBroken) {
                try {
                    int size = socket.receive(dataUnit);
                    if (DBG) Log.d(TAG, "LLCP CO read " + size + " bytes");
                    if (size < 0) {
                        connectionBroken = true;
                        break;
                    } else {
                        echoMachine.pushUnit(dataUnit, size);
                    }
                } catch (IOException e) {
                    // Connection broken
                    connectionBroken = true;
                    if (DBG) Log.d(TAG, "connection broken by IOException", e);
                }
            }
        }

        @Override
        public void run() {
            if (DBG) Log.d(TAG, "about to create LLCP service socket, service name: "
                    + serviceNameIn + ", sap: " + sapIn);

            try {
                serverSocket = mService.createLlcpServerSocket(sapIn, serviceNameIn, MIU, 1, 1024);
            } catch (LlcpException e) {
                // Ignore here, handle below
            }
            if (serverSocket == null) {
                if (DBG) Log.e(TAG, "failed to create LLCP service socket");
                return;
            }
            if (DBG) Log.d(TAG, "created LLCP service socket");

            while (running) {
                try {
                    if (!mLtMode) {
                        if (DBG) Log.d(TAG, "service socket about to accept " + serviceNameIn + " " + sapIn);
                        clientSocketIn = serverSocket.accept();
                        if (DBG) Log.d(TAG, "service socket accept returned " + clientSocketIn + " " + serviceNameIn + " " + sapIn );

                        timer = new Timer();
                        timer.schedule(new TimeoutTask(), TIMEOUT);
                        if (DBG) Log.d(TAG, "timeout timer started");
                    }

                    boolean isOutboundConnectionEstablished = false;
                    if (DBG) Log.d(TAG, "isOutboundConnectionEstablished:" + isOutboundConnectionEstablished +  " timeout:" + timeout);

                    while (!isOutboundConnectionEstablished && !timeout) {
                        try {
                            if (DBG) Log.d(TAG, "Attempting to connect outbound connection");

                            clientSocketOut = mService.createLlcpSocket(0, MIU, 1, 1024);
                            if (clientSocketOut == null) {
                                throw new IOException("Could not create socket.");
                            }

                            /* in case of TC_CTO_INI_BV_04 we need to ask for the service's sap (SNL)
                             * before connecting to this sap. sapOut should take the value from SNL
                             * and not the default one.
                             * TODO: add SNL function to the socket, instead of using connectToSap() */
                            if (DtaHelper.isInLlcpSnl()) {
                                if (DBG) Log.d(TAG, "client socket starting snl: " + sapOut);
                                clientSocketOut.connectToSap(sapOut);
                            }
                            else if(DtaHelper.isInLlcpName()) {
                                if (DBG) Log.d(TAG, "client socket connecting to service: " + serviceNameOut);
                                clientSocketOut.connectToService(serviceNameOut);
                            }
                            else {
                                if (DBG) Log.d(TAG, "client socket connecting to sap: " + sapOut);
                                clientSocketOut.connectToSap(sapOut);
                            }

                            isOutboundConnectionEstablished = true;
                            if (DBG) Log.d(TAG, "client socket connected");
                            if (timer != null) {
                                if (DBG) Log.d(TAG, "cancelling timeout timer");
                                timer.cancel();
                            }
                        } catch (IOException ioe) {
                            Log.e(TAG, "IO error", ioe);
                            try {
                                if (DBG) Log.d(TAG, "Retry connecting outbound connection");
                                int interval = mLtMode ? 3000 : 250;
                                Thread.sleep(interval);
                            } catch (InterruptedException e) {
                                // Ignore
                            }
                        }
                    }

                    if (isOutboundConnectionEstablished) {
                        if (mLtMode) {
                            if (DBG) Log.d(TAG, "about to accept " + serviceNameIn + " " + sapIn);
                            clientSocketIn = serverSocket.accept();
                            if (DBG) Log.d(TAG, "accept returned " + clientSocketIn);

                            write("abcxyz".getBytes());
                        }

                        if (clientSocketIn != null) {
                            handleClient(clientSocketIn);
                        }
                    }

                    if (!mLtMode) {
                        timeout = false;
                    }
                } catch (LlcpException e) {
                    Log.e(TAG, "LLCP error", e);
                    running = false;
                } catch (IOException e) {
                    Log.e(TAG, "IO error", e);
                    running = false;
                } finally {
                    // connection broken
                    Log.e(TAG, "connection broken, closing client sockets");
                    closeClientSockets();
                }
            }

            if (DBG) Log.d(TAG, "CO shutdown: shutdown echomachine");
            echoMachine.shutdown();

            if (DBG) Log.d(TAG, "CO shutdown: closing client sockets");
            closeClientSockets();

            if (DBG) Log.d(TAG, "CO shutdown: closing server socket");
            if (serverSocket != null) {
                try {
                    serverSocket.close();
                } catch (IOException e) {
                    // Ignore
                }
                if (DBG) Log.d(TAG, "CO shutdown: server socket closed");
                serverSocket = null;
            }
        }

        private void closeClientSockets() {
            if (DBG) Log.d(TAG, "closeClientSockets()");
            if (clientSocketIn != null) {
                try {
                    clientSocketIn.close();
                } catch (IOException e) {
                    // Ignore
                }
                if (DBG) Log.d(TAG, "clientSocketIn closed");
                clientSocketIn = null;
            }

            if (clientSocketOut != null) {
                try {
                    clientSocketOut.close();
                } catch (IOException e) {
                    // Ignore
                }
                if (DBG) Log.d(TAG, "clientSocketOut closed");
                clientSocketOut = null;
            }
        }

        @Override
        public void write(byte[] data) {
            if (clientSocketOut != null) {
                try {
                    clientSocketOut.send(data);
                    Log.e(TAG, "Send success!");
                } catch (IOException e) {
                    Log.e(TAG, "Send failed.");
                }
            }
        }

        public void shutdown() {
            if (DBG) Log.d(TAG, "shutdown()");
            running = false;
            if (serverSocket != null) {
                try {
                    serverSocket.close();
                } catch (IOException e) {
                    // ignore
                }
                serverSocket = null;
            }
        }

        private void timeout() {
            if (DBG) Log.d(TAG, "LLCP timeout");
            timeout = true;
            timer.cancel();
        }

        private class TimeoutTask extends TimerTask {
            @Override
            public void run() {
                timeout();
            }
        }
    }


    /**
     * The start of test byte sequence.
     */
    public static final byte[] START_OF_TEST = new byte[]{ 0x53, 0x4F, 0x54 };

    /**
     * The repsonse message to the START_OF_TEST UI.
     */
    public static final byte[] START_OF_TEST_RESPONSE = new byte[]{ 0x18, 0x18, 0x18 };

    public static final int ANY_SAP = 0xFF;
    public static final char SEPARATOR = ',';

    public static final int DEFAULT_ECHO_DELAY = 3000;

    /**
     * Checks whether the specified data array is the start of test sequence.
     */
    public static boolean isStartOfTest(byte[] data) {

        if (data == null || data.length != START_OF_TEST.length) {
            return false;
        }

        for (int i = 0; i < START_OF_TEST.length; ++i) {
            if (data[i] != START_OF_TEST[i]) {
                return false;
            }
        }

        return true;
    }

    public static final String getValue(String str, String key) {

        key += '=';
        int i = str.indexOf(key);
        if (i == -1) {
            return null;
        }

        i += key.length();

        int j = str.indexOf(SEPARATOR, i);

        if (j == -1) {
            j = str.length();
        }

        return str.substring(i, j).trim();
    }

    /**
     * Parses the service name from the parameter list.
     *
     * @return the service name or <code>null</code> if it is not present in the list.
     */
    public static final String getServiceName(String parameters) {

        final String serviceName = getValue(parameters, "sn");
        if (serviceName == null || serviceName.length() == 0) {
            return null;
        }

        return serviceName;
    }

    /**
     * Parses an integer from the string parameter.
     * The valid formats for the integer are "<int>h" (radix 16), "0x<int>" (radix 16) or "<int>" (radix 10).
     *
     * @param str The string containing the integer.
     *
     * @return The parsed integer.
     */
    public static final int parseIntValue(String str) {

        if (str.endsWith("h")) {
            return Integer.parseInt(str.substring(0, str.length()-1), 16);
        }
        else if (str.startsWith("0x")) {
            return Integer.parseInt(str.substring(2, str.length()-2), 16);
        }

        return Integer.parseInt(str);
    }

    public static final int getSAP(String parameters) {

        final String sapStr = getValue(parameters, "sap");
        if (sapStr == null) {
            return ANY_SAP;
        }

        int sap = ANY_SAP;

        try {
            sap = parseIntValue(sapStr);
        }
        catch (Exception e) {
            Log.e(TAG, "LLCP CL error: parsing SAP failed", e);
        }

        return sap;
    }

    public static final int getEchoDelay(String parameters) {

        final String delayStr = getValue(parameters, "echo_delay");
        if (delayStr == null) {
            return DEFAULT_ECHO_DELAY;
        }

        int delay = DEFAULT_ECHO_DELAY;

        try {
            delay = Integer.parseInt(delayStr);
        }
        catch (Exception e) {
            Log.e(TAG, "LLCP CL error: parsing echo delay failed", e);
        }

        return delay;
    }

    public static final boolean getLTMode(String parameters) {

        final String modeStr = getValue(parameters, "ltmode");
        if (modeStr == null || modeStr.length() == 0) {
            return false;
        }

        return modeStr.equalsIgnoreCase("true");
    }

    /**
     * The server thread for LLCP connectionless service.
     */
    public class ConnectionlessServer implements Runnable, WriteCallback {

        final String serviceNameIn;
        final String serviceNameOut;
        final int sapIn;
        final int remoteSap;

        final int echoDelay;

        final boolean ltMode;

        LlcpConnectionlessSocket inputSocket = null;
        LlcpConnectionlessSocket outputSocket = null;

        EchoMachine echoMachine = null;
        private volatile boolean isAlive = false;

        public ConnectionlessServer(String inputParameters, String outputParameters) {

            if (DBG) Log.d(TAG, "registering LLCP CL service.....");

            this.serviceNameIn = getServiceName(inputParameters);
            this.serviceNameOut = getServiceName(outputParameters);
            this.sapIn = 0x11;
            this.remoteSap = 0x21;
            //this.remoteSap = getSAP(outputParameters);

            this.echoDelay = getEchoDelay(outputParameters);

            this.ltMode = getLTMode(inputParameters);

            if (DBG) Log.d(TAG, "serviceName IN = " + this.serviceNameIn);
            if (DBG) Log.d(TAG, "serviceName OUT = " + this.serviceNameOut);
            if (DBG) Log.d(TAG, "input socket's SAP = " + this.sapIn);
            if (DBG) Log.d(TAG, "remote SAP for sending = " + this.remoteSap);

            try {
                inputSocket = new NativeLlcpConnectionlessSocket();
                if (((NativeLlcpConnectionlessSocket)inputSocket).open(this.serviceNameIn, this.sapIn) == -1) {
                    Log.e(TAG, "LLCP CL error: could not open the CL input socket!");
                }
                else {
                    //mService.createLlcpConnectionLessSocket(sapIn, serviceNameIn);
                    if (DBG) Log.d(TAG, "LLCP CL input socket created successfully");
                }

                outputSocket = new NativeLlcpConnectionlessSocket();
                if (((NativeLlcpConnectionlessSocket)outputSocket).open(null, this.remoteSap) == -1) {
                    Log.e(TAG, "LLCP CL error: could not open the CL output socket!");
                }
                else {
                    if (DBG) Log.d(TAG, "LLCP CL output socket created successfully");
                }
            }
            catch (Exception e) {
                Log.e(TAG, "LLCP CL error", e);
            }

        }

        /**
         * Closes the connectionless LLCP sockets.
         */
        public void closeSockets() {
            if (DBG) Log.d(TAG, "close LLCP CL sockets");

            if (inputSocket != null) {
                try {
                    inputSocket.close();
                    if (DBG) Log.d(TAG, "LLCP CL input socket closed");
                } catch (IOException e) {
                    if (DBG) Log.d(TAG, "LLCP CL input socket closing failed: IOException", e);
                }
            } else {
                if (DBG) Log.d(TAG, "LLCP CL input socket was already closed!");
            }

            if (outputSocket != null) {
                try {
                    outputSocket.close();
                    if (DBG) Log.d(TAG, "LLCP CL output socket closed");
                } catch (IOException e) {
                    if (DBG) Log.d(TAG, "LLCP CL output socket closing failed: IOException", e);
                }
            } else {
                if (DBG) Log.d(TAG, "LLCP CL output socket was already closed!");
            }
        }

        @Override
        public void run() {
            LlcpPacket packet;
            if (DBG) Log.d(TAG, "start LLCP CL echo server, LT mode: " + ltMode);
            try {
                while (isAlive) {
                    try {

                        if (DBG) Log.d(TAG, "LLCP CL about to receive");
                        packet = inputSocket.receive();
                        if (packet == null || packet.getDataBuffer() == null) {
                            if (DBG) Log.d(TAG, "LLCP CL received packet is null");
                            continue;
                        }
                        byte[] dataUnit = packet.getDataBuffer();
                        int size = dataUnit.length;
                        if (DBG) Log.d(TAG, "LLCP CL read " + size + " bytes");

                        if (isStartOfTest(dataUnit)) {
                            if (DBG) Log.d(TAG, "LLCP CL: received START OF TEST command");
                            outputSocket.send(serviceNameOut, START_OF_TEST_RESPONSE);
                            if (DBG) Log.d(TAG, "LLCP CL: SNL done");
                        } else {
                            // mRemoteSap = packet.getRemoteSap();
                            echoMachine.pushUnit(dataUnit, size);
                        }
                    } catch (IOException e) {
                        if (DBG) Log.d(TAG, "IOException while running LLCP CL server thread", e);
                    }
                }
            } catch (Exception e) {
                Log.e(TAG, "LLCP error", e);
            } finally {
                if (DBG) Log.d(TAG, "LLCP CL thread stopped");
                echoMachine.shutdown();
            }
        }

        /**
         * Starts the LLCP CL echo service thread and sends the start of test command in case the server is in LT emulator mode.
         */
        public void startThread() {
            synchronized (this) {

                if (!isAlive) {
                    isAlive = true;
                    echoMachine = new EchoMachine(this, true);
                    Thread th = new Thread(mConnectionlessServer);
                    th.start();
                }

                if (ltMode) {
                    if (DBG) Log.d(TAG, "writing START_OF_TEST to LLCP CL output socket");
                    write(START_OF_TEST);
                }
            }

        }

        public void shutdown() {
            isAlive = false;
        }

        @Override
        public void write(byte[] data) {
            try {
                if (remoteSap > 0 && remoteSap < ANY_SAP) outputSocket.send(remoteSap, data);
                else outputSocket.send(serviceNameOut, data);
            } catch (IOException e) {
                if (DBG) Log.d(TAG, "Error writing data.", e);
            }
        }
    }

    public void onLlcpActivated() {

        if (DBG) Log.d(TAG, "onLlcpActivated()");
        if (mConnectionlessServer != null) {
            // If we are in LT mode, we need to wait for 100ms here. This is to avoid
            // timing issue between EchoServer and LlcpConnectionlessSocket. The EchoServer
            // receives LLCP Activated -notification before LlcpConnectionlessSocket does.
            // If the EchoServer tries to send START_OF_TEST package right away, before
            // the LlcpConncetionlessSocket has received the LLCP Activated -notification,
            // there is a mutex issue as the sending method and activation callback are
            // using the same mainMutex. By adding the delay here, we let the activation
            // callback run first and send the packet after that. Without this fix,
            // the send method is waiting for sdpEvent and holding the key to mainMutex.
            // This causes the whole stack to freeze as the activated callback is not able to
            // complete.
            if (mConnectionlessServer.ltMode) {
               if (DBG) Log.d(TAG, "onLlcpActivated(): in LT mode -> sleep for 100ms before sending START_OF_TEST packet");
               try {
                   Thread.sleep(100);
               } catch (InterruptedException e) {
                 // Ignore
               }
            }
            mConnectionlessServer.startThread();
        }
    }

    public void onLlcpDeactivated() {

        if (DBG) Log.d(TAG, "onLlcpDeactivated()");
    }

    /**
     * Starts a connectionless echo service. The service will become active on
     * LLCP link activation.
     */
    public void startClService(String serviceNameIn, String serviceNameOut) {

        if (mConnectionlessServer == null) {
            mConnectionlessServer = new ConnectionlessServer(serviceNameIn, serviceNameOut);
            if (DBG) Log.d(TAG, "LLCP CL service registered");
            //mConnectionlessServer.startThread();
        }
        else {
            if (DBG) Log.d(TAG, "LLCP CL service is registered already - ignoring call!");
        }
    }

    /**
     * Stops a running connectionless echo service. The service is deactivated on
     * LLCP link deactivation.
     */
    public void stopClService() {

        if (mConnectionlessServer == null) {
            if (DBG) Log.d(TAG, "no LLCP CL service registered - ignoring call!");
            return;
        }

        mConnectionlessServer.shutdown();
        mConnectionlessServer.closeSockets();
        mConnectionlessServer = null;

        if (DBG) Log.d(TAG, "LLCP CL servise closed");
    }

    /**
     *  Needs to be called on the UI thread
     */
    public void startCoService() {
        synchronized (this) {
            if (mServerThread == null && mCoServiceNameIn != null && mCoServiceNameOut != null) {
                mServerThread = new ServerThread(mCoServiceNameIn, mCoServiceNameOut);
                mServerThread.start();
            } else {
                if (DBG) Log.d(TAG, "Couldn't start LLCP CO echo service");
            }
        }
    }

    /**
     * Stops a running connection-oriented echo service.
     * Needs to be called on the UI thread.
     */
    public void stopCoService() {
        synchronized (this) {
            if (mServerThread != null) {
                mServerThread.shutdown();
                mServerThread = null;
            }
        }
    }
}
