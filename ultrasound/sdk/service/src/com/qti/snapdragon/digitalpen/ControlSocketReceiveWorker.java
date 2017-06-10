/*===========================================================================
                           ControlSocketReceiveWorker.java

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.

Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.snapdragon.digitalpen;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;

import com.qti.snapdragon.digitalpen.SocketThread.ReceiveWorker;

// Control socket worker:
// Parses event updates from the daemon. Check {@link DigitalPenEvent} to
// know more about the format of the events data received.
class ControlSocketReceiveWorker implements ReceiveWorker {

    public interface OnEventListener {
        void onEvent(int eventType, int[] params);
    }

    public ControlSocketReceiveWorker(ControlSocketReceiveWorker.OnEventListener listener) {
        this.listener = listener;
    }

    private ControlSocketReceiveWorker.OnEventListener listener;

    // packet size constants; all 4-byte words
    static final int HEADER_WORDS = 1; // eventType
    static final int NUM_PARAMS = 2;
    static final int RAW_PACKET_SIZE = (HEADER_WORDS + NUM_PARAMS) * Integer.SIZE / 8;

    @Override
    public void receiveLoop(InputStream input) throws IOException, InterruptedException {
        if (null == input) {
            throw new IllegalArgumentException("null not a valid InputStream");
        }

        // create receive buffer
        byte[] data = new byte[RAW_PACKET_SIZE];
        IntBuffer buf = ByteBuffer.wrap(data).order(ByteOrder.LITTLE_ENDIAN).asIntBuffer();

        // receive loop
        while (input.read(data, 0, RAW_PACKET_SIZE) == RAW_PACKET_SIZE) {
            int eventType = buf.get(0);
            int[] params = new int[] {
                    buf.get(1),
                    buf.get(2)
            };
            listener.onEvent(eventType, params);
        }
    }
}