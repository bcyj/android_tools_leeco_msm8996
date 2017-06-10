/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.android.nfc.dhimpl;

import com.android.nfc.DeviceHost;
import com.android.nfc.LlcpPacket;

// <DTA>
import android.nfc.dta.DtaHelper;
// </DTA>

import java.io.IOException;

/**
 * LlcpConnectionlessSocket represents a LLCP Connectionless object to be used
 * in a connectionless communication
 */
public class NativeLlcpConnectionlessSocket implements DeviceHost.LlcpConnectionlessSocket {

    private int mHandle;
    private int mSap;
    private int mLinkMiu;

    public NativeLlcpConnectionlessSocket() {
// <DTA>
        if (DtaHelper.isInDtaMode()) {
            mSap = -1; // -1 means the socket is closed.
            mLinkMiu = 128; // 128 is the default value for MIU.
        }
// </DTA>
    }

    public native boolean doSendTo(int sap, byte[] data);

    public native LlcpPacket doReceiveFrom(int linkMiu);

    public native boolean doClose();
// <DTA>
    /**
     * Opens and registers a socket with the specified local service name.
     * The local SAP can be defined as the second parameter.
     * If the parameter value is ANY_LOCAL_SAP, then the SAP is defined dynamically by the stack.
     *
     * @param localServiceName The local service name.
     * @param localSap The local SAP.
     *
     * @return The local SAP number that was registered for the socket. -1 if there occured an error while opening the socket.
     *
     * @see DeviceHost.LlcpConnectionlessSocket.ANY_LOCAL_SAP
     */
    public int open(String localServiceName, int localSap) {
        return mSap = doOpen(localServiceName, localSap);
    }

    /**
     * Sends data to the service corresponding the specified service name.
     *
     * @param remoteServiceName The remote service name.
     * @param data The data to send.
     *
     * @throws IOException if the operation fails.
     */
    public void send(String remoteServiceName, byte[] data) throws IOException {

        checkIsOpen();

        if (!doSendTo(remoteServiceName, data, mSap)) {
            throw new IOException();
        }
    }

    /**
     * Checks whether the socket is open and does nothing in the positive case.
     * In case the socket if closed, an exception is thrown.
     *
     * @throws IOException if the socket is closed.
     */
    private void checkIsOpen() throws IOException {
        if (mSap == -1) {
            throw new IOException("socket is closed");
        }
    }

    /**
     * JNI definition: (Ljava/lang/String;I)I
     */
    public native int doOpen(String localServiceName, int localSap);

    /**
     * JNI definition: (Ljava/lang/String;[B;I)Z
     */
    public native boolean doSendTo(String remoteServiceName, byte[] data, int localSap);

    /**
     * JNI definition: (I[B;I)Z
     */
    public native boolean doSendTo(int sap, byte[] data, int localSap);

    /**
     * JNI definition: (Lcom/android/nfc/LlcpPacket;I;I)I
     */
    public native int doReceiveFrom(LlcpPacket llcpPacket, int linkMiu, int localSap);

    /**
     * JNI definition: (I)Z
     */
    public native boolean doClose(int localSap);
// </DTA>

    @Override
    public int getLinkMiu(){
        return mLinkMiu;
    }

    @Override
    public int getSap(){
        return mSap;
    }

    @Override
    public void send(int sap, byte[] data) throws IOException {
        boolean status = false;
// <DTA>
        if (!DtaHelper.isInDtaMode()) {
            status = doSendTo(sap, data);
        } else {
            status = doSendTo(sap, data, mSap);
        }
        if (!status) {
            throw new IOException();
        }
// </DTA>
    }

    @Override
    public LlcpPacket receive() throws IOException {
        // <DTA>
        LlcpPacket packet = new LlcpPacket();
        if (!DtaHelper.isInDtaMode()) {
            packet = doReceiveFrom(mLinkMiu);
            if (packet == null) {
                throw new IOException();
            }
        } else {
            int remoteSap = doReceiveFrom(packet, mLinkMiu, mSap);
            if (remoteSap == -1) {
                throw new IOException();
            }
        }
        // </DTA>
        return packet;
    }

    public int getHandle(){
        return mHandle;
    }

    @Override
    public void close() throws IOException {
        boolean status = false;
        if (mSap == -1) {
            return;
        }
        // <DTA>
        if (!DtaHelper.isInDtaMode()) {
            status = doClose();
        } else {
            status = doClose(mSap);
        }
        // </DTA>
        if (!status) {
            throw new IOException();
        }
        mSap = -1;
    }
}
