/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.android.nfc.snep;

import com.android.nfc.Debug;
import com.android.nfc.DeviceHost.LlcpSocket;

import android.nfc.FormatException;
import android.util.Log;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.Arrays;

// <DTA>
import android.nfc.dta.DtaHelper;
// </DTA>

public class SnepMessenger {
    private static final String TAG = "SnepMessager";
    private static final boolean DBG = Debug.SnepMessenger;
    // <DTA>
    private static final int DEFAULT_ACCEPTABLE_LENGTH = 100*1024; // MAL_IUT
    // </DTA>
    private static final int HEADER_LENGTH = 6;
    final LlcpSocket mSocket;
    final int mFragmentLength;
    final boolean mIsClient;

    public SnepMessenger(boolean isClient, LlcpSocket socket, int fragmentLength) {
        mSocket = socket;
        mFragmentLength = fragmentLength;
        mIsClient = isClient;
    }

    public void sendMessage(SnepMessage msg) throws IOException {
        byte[] buffer = msg.toByteArray();
        byte remoteContinue;
        if (mIsClient) {
            remoteContinue = SnepMessage.RESPONSE_CONTINUE;
        } else {
            remoteContinue = SnepMessage.REQUEST_CONTINUE;
        }
        if (DBG) Log.d(TAG, "about to send a " + buffer.length + " byte message");

        // Send first fragment
        int length = Math.min(buffer.length, mFragmentLength);
        byte[] tmpBuffer = Arrays.copyOfRange(buffer, 0, length);
        if (DBG) Log.d(TAG, "about to send a " + length + " byte fragment");
        mSocket.send(tmpBuffer);

        if (length == buffer.length) {
            return;
        }

        // Look for Continue or Reject from peer.
        int offset = length;
        byte[] responseBytes = new byte[HEADER_LENGTH];
        mSocket.receive(responseBytes);
        SnepMessage snepResponse;
        try {
            snepResponse = SnepMessage.fromByteArray(responseBytes);
        } catch (FormatException e) {
            throw new IOException("Invalid SNEP message", e);
        }

        if (DBG) Log.d(TAG, "Got response from first fragment: " + snepResponse.getField());
        if (snepResponse.getField() != remoteContinue) {
            throw new IOException("Invalid response from server (" +
                    snepResponse.getField() + ")");
        }

        // Send remaining fragments.
        while (offset < buffer.length) {
            length = Math.min(buffer.length - offset, mFragmentLength);
            tmpBuffer = Arrays.copyOfRange(buffer, offset, offset + length);
            if (DBG) Log.d(TAG, "about to send a " + length + " byte fragment");
            mSocket.send(tmpBuffer);
            offset += length;
        }
    }

    public SnepMessage getMessage() throws IOException, SnepException {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream(mFragmentLength);
        byte[] partial = new byte[mFragmentLength];
        int size;
        // <DTA>
        // BUG FIX
        // Changed type from 'int' to 'long' because SNEP [3.1.3] specifies length to be a 32-bit unsigned integer
        // Java integers are always signed so we could not store the correct length in 'int'. Type needs to be a long.
        long requestSize = 0;
        long readSize = 0;
        // </DTA>
        byte requestVersion = 0;
        boolean doneReading = false;
        byte fieldContinue;
        byte fieldReject;
        if (mIsClient) {
            fieldContinue = SnepMessage.REQUEST_CONTINUE;
            fieldReject = SnepMessage.REQUEST_REJECT;
        } else {
            fieldContinue = SnepMessage.RESPONSE_CONTINUE;
            fieldReject = SnepMessage.RESPONSE_REJECT;
        }

        size = mSocket.receive(partial);
        if (DBG) Log.d(TAG, "read " + size + " bytes");
        if (size < 0) {
            try {
                mSocket.send(SnepMessage.getMessage(fieldReject).toByteArray());
            } catch (IOException e) {
                // Ignore
            }
            throw new IOException("Error reading SNEP message.");
        } else if (size < HEADER_LENGTH) {
            /* in case of dta don't send reject */
            if (!DtaHelper.isInLlcpOrSnepMode()) {
                try {
                mSocket.send(SnepMessage.getMessage(fieldReject).toByteArray());
                } catch (IOException e) {
                   //Ignore
                }
            }
            throw new IOException("Invalid fragment from sender.");
        } else {
            readSize = size - HEADER_LENGTH;
            buffer.write(partial, 0, size);
        }

        DataInputStream dataIn = new DataInputStream(new ByteArrayInputStream(partial));
        requestVersion = dataIn.readByte();
        byte requestField = dataIn.readByte();
        // <DTA>
        // Get the 32-bit unsigned integer part into our long. (see bug fix mentioned earlier)
        requestSize = dataIn.readInt() & 0x00000000ffffffffL;
        // </DTA>

        if (DBG) Log.d(TAG, "read " + readSize + " of " + requestSize);

        if (((requestVersion & 0xF0) >> 4) != SnepMessage.VERSION_MAJOR) {
            // Invalid protocol version; treat message as complete.
            return new SnepMessage(requestVersion, requestField, 0, 0, null);
        }
        // <DTA>
        // [SNEP 4.4] and [SNEP 5.8] Reject
        // Send "Reject" if request/response size > acceptable length
        if (DtaHelper.isInLlcpOrSnepMode()) {
            if (requestSize > DEFAULT_ACCEPTABLE_LENGTH) {
                try {
                    mSocket.send(SnepMessage.getMessage(fieldReject).toByteArray());
                } catch (IOException e) {
                    // Ignore
                }
                throw new IOException("SNEP message is longer than the acceptable length. Rejecting.");
            }
        }
        // </DTA>

        if (requestSize > readSize) {
            if (DBG) Log.d(TAG, "requesting continuation");
            mSocket.send(SnepMessage.getMessage(fieldContinue).toByteArray());
        } else {
            doneReading = true;
        }

        // Remaining fragments
        while (!doneReading) {
            try {
                size = mSocket.receive(partial);
                if (DBG) Log.d(TAG, "read " + size + " bytes");
                if (size < 0) {
                    try {
                        mSocket.send(SnepMessage.getMessage(fieldReject).toByteArray());
                    } catch (IOException e) {
                        // Ignore
                    }
                    throw new IOException();
                } else {
                    readSize += size;
                    buffer.write(partial, 0, size);
                    if (readSize == requestSize) {
                        doneReading = true;
                    }
                }
            } catch (IOException e) {
                try {
                    mSocket.send(SnepMessage.getMessage(fieldReject).toByteArray());
                } catch (IOException e2) {
                    // Ignore
                }
                throw e;
            }
        }

        // Build NDEF message set from the stream
        try {
            return SnepMessage.fromByteArray(buffer.toByteArray());
        } catch (FormatException e) {
            Log.e(TAG, "Badly formatted NDEF message, ignoring", e);
            throw new SnepException(e);
        }
    }

    public void close() throws IOException {
        mSocket.close();
    }
}
