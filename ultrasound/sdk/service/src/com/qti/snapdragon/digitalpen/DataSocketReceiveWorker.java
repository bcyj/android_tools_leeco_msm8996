/*===========================================================================
                           DataSocketReceiveWorker.java

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
import com.qti.snapdragon.digitalpen.util.DigitalPenData;

// Data socket worker:
// Receives pen points sent by the daemon. Check {@link DigitalPenData} to
// know more about the format of the data received.
class DataSocketReceiveWorker implements ReceiveWorker {

    public interface OnDataListener {
        void onData(DigitalPenData data);
    }

    private DataSocketReceiveWorker.OnDataListener listener;

    public DataSocketReceiveWorker(DataSocketReceiveWorker.OnDataListener listener) {
        this.listener = listener;
    }

    // packet size constants; all 4-byte words
    static final int NUM_PARAMS = 12;
    static final int RAW_PACKET_SIZE = NUM_PARAMS * Integer.SIZE / 8;

    @Override
    public void receiveLoop(InputStream input) throws IOException, InterruptedException {

        if (null == input) {
            throw new IllegalArgumentException("null not a valid InputStream");
        }

        // setup receive buffer
        byte[] data = new byte[RAW_PACKET_SIZE];
        IntBuffer buf = ByteBuffer.wrap(data).order(ByteOrder.LITTLE_ENDIAN).asIntBuffer();

        // Read a packet from the data socket
        while (input.read(data, 0, RAW_PACKET_SIZE) == RAW_PACKET_SIZE) {

            // Side buttons *up* or *down*
            int[] sideButtonsState = new int[] {
                    buf.get(8),
                    buf.get(9),
                    buf.get(10)
            };

            DigitalPenData dataToSend =
                    new DigitalPenData(
                            buf.get(0), // x
                            buf.get(1), // y
                            buf.get(2), // z
                            buf.get(3), // tiltX
                            buf.get(4), // tiltY
                            buf.get(5), // tiltZ
                            buf.get(6), // pressure
                            (buf.get(7) == 1), // penState
                            sideButtonsState, // sideButtonsState
                            buf.get(11) // region
                    );

            listener.onData(dataToSend);
        }
    }
}
