
package com.android.ultrasound.scopedebuggerrecordingtool;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.text.format.Time;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.util.Arrays;

public class ScopeDbgRecActivity extends Activity {

    private static final String REC_FILE_PATH = "/data/usf/epos/rec/";

    // Background threads use this Handler to post messages to
    // the main application thread
    private final Handler mMainHandler = new Handler();

    private EditText mFileNameTextBox = null;

    private final Time now = new Time();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_scope_dbgrec);

        Button button = (Button)findViewById(R.id.button1);
        button.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                (new SocketThread(mMainHandler)).start();
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        mFileNameTextBox = (EditText)findViewById(R.id.editText1);
        mFileNameTextBox.setText(this.getString(R.string.file_name_default));
    }

    private void onSuccessfulDump() {
        Toast.makeText(this,
                "Successfully saved dump file to " + REC_FILE_PATH + mFileNameTextBox.getText(),
                Toast.LENGTH_LONG).show();
    }

    private void onNotEnoughData(int expected, int received) {
        Toast.makeText(this,
                "Socket closed unexpectedly. Received " + received + " bytes out of the " + expected + " which were expected",
                Toast.LENGTH_LONG).show();
    }

    public class SocketThread extends Thread {

        private static final int DELAY_SOCKET_RECONNECT_MSEC = 500;

        private static final int MIN_DST_PORT = 18251;

        private static final int MAX_DST_PORT = 18258;

        private final byte[] SOCKET_MAGIC = {
                0x3F, 0x35, (byte)0x82, 0x20
        };

        private static final int CMD_SEND_BUFFER_SIZE = 256;

        private final byte[] CMD = {
                (byte)0x80, 0x1, 0xE, 0x1, 0x1, 0x1, 0x1
        };

        private static final int DATA_SIZE_LEN_BYTES = 4;

        private Handler mHandler;

        private Socket mSocket;

        private InputStream mSocketRead = null;

        private OutputStream mSocketWrite = null;

        public SocketThread(Handler handler) {
            mHandler = handler;
            mSocket = new Socket();
        }

        @Override
        public void run() {
            Log.d(this.toString(), "Socket thread started");

            for (int portNum = MIN_DST_PORT; portNum <= MAX_DST_PORT; ++portNum) {
                try {
                    Log.d(this.toString(), "Trying to connect to port " + portNum);
                    InetSocketAddress addr = new InetSocketAddress(InetAddress.getLocalHost(),
                            portNum);
                    mSocket.connect(addr, DELAY_SOCKET_RECONNECT_MSEC);
                    if (mSocket.isConnected()) {
                        Log.d(this.toString(), "Connected to Scope Debugger at port " + portNum);
                        mSocketRead = mSocket.getInputStream();
                        mSocketWrite = mSocket.getOutputStream();

                        Log.d(this.toString(), "Saving dump to file " + getFileName());
                        if (getScopeDebuggerRecording()) {
                            Log.d(this.toString(), "Successfuly saved dump file!");
                            mHandler.post(new Runnable() {
                                @Override
                                public void run() {
                                    onSuccessfulDump();
                                }
                            });
                        }
                        closeSocket();
                        return;
                    }
                } catch (IOException e) {
                    Log.e(this.toString(), e.toString());
                }
            }
        }

        private boolean getScopeDebuggerRecording() {
            if (!waitForMagic()) {
                Log.e(this.toString(), "Failed getting magic from socket");
                return false;
            }

            if (!sendCmd()) {
                Log.e(this.toString(), "Failed sending command to scope debugger");
                return false;
            }

            int dataSizeBytes = getDataSize();
            if (dataSizeBytes < 0) {
                Log.e(this.toString(), "Failed getting size of dump");
                return false;
            }

            writeRecordingToFile(dataSizeBytes);

            return true;
        }

        private void closeSocket() {
            try {
                mSocketRead.close();
                mSocketWrite.close();
                mSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        private void writeRecordingToFile(final int dataSizeBytes) {
            OutputStream outputStream = null;
            try {
                now.setToNow();
                File outFile = new File(getFileName());
                outFile.setWritable(true);
                outputStream = new FileOutputStream(outFile);
                int chunkRead = 0;
                int totalRead = 0;
                byte[] bytes = new byte[1024];

                while (totalRead < dataSizeBytes && (chunkRead = mSocketRead.read(bytes)) != -1) {
                    totalRead += chunkRead;
                    outputStream.write(bytes, 0, chunkRead);
                }
                Log.d(this.toString(), "Read " + totalRead + " bytes of dump file");
                if (totalRead < dataSizeBytes) {
                    final int totalReadFinal = totalRead;
                    mHandler.post(new Runnable() {
                                @Override
                                public void run() {
                                    onNotEnoughData(dataSizeBytes, totalReadFinal);
                                }
                            });
                }
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                if (outputStream != null) {
                    try {
                        outputStream.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

        private int getDataSize() {
            byte[] dataSizeLen = new byte[DATA_SIZE_LEN_BYTES];
            try {
                for (int i = 0; i < DATA_SIZE_LEN_BYTES; ++i) {
                    dataSizeLen[i] = (byte)mSocketRead.read();
                }
                Log.d(this.toString(), "received data size as 0x" + byteArrayToHex(dataSizeLen));
            } catch (IOException e) {
                e.printStackTrace();
                return -1;
            }

            reverseByteArray(dataSizeLen); // bytes come in reversed order in socket
            int sizeInBytes = ByteBuffer.wrap(dataSizeLen).getInt();
            Log.d(this.toString(), "Size of dump in bytes: " + sizeInBytes);
            return sizeInBytes;
        }

        private boolean sendCmd() {
            // Sending a 256 byte buffer, of which the 7 first bytes contain the command
            // for the scope debugger to send us the signal buffer. We should receive these 7 bytes
            // over the socket as an acknowledgment.
            byte[] sendBuf = new byte[CMD_SEND_BUFFER_SIZE];

            try {
                Arrays.fill(sendBuf, 0, sendBuf.length, (byte)0);
                System.arraycopy(CMD, 0, sendBuf, 0, CMD.length);
                Log.d(this.toString(), "Sending cmd to scope debugger");
                mSocketWrite.write(sendBuf);
            } catch (IOException e) {
                e.printStackTrace();
                return false;
            }

            return isExpectedAck(CMD);
        }

        private boolean waitForMagic() {
            return isExpectedAck(SOCKET_MAGIC);
        }

        private boolean isExpectedAck(byte[] expected) {
            byte[] buffer = new byte[expected.length];

            try {
                for (int i = 0; i < expected.length; ++i) {
                    buffer[i] = (byte)mSocketRead.read();
                }
                Log.d(this.toString(), "received ack 0x" + byteArrayToHex(buffer));
            } catch (IOException e) {
                e.printStackTrace();
                return false;
            }

            return Arrays.equals(buffer, expected);
        }

        private String byteArrayToHex(byte[] byteArray) {
            StringBuilder sb = new StringBuilder();
            for (byte b : byteArray)
                sb.append(String.format("%02x", b & 0xff));
            return sb.toString();
        }

        private void reverseByteArray(byte[] byteArray) {
            for (int i = 0; i < byteArray.length / 2; ++i) {
                byte tmp = byteArray[i];
                byteArray[i] = byteArray[byteArray.length - 1 - i];
                byteArray[byteArray.length - 1 - i] = tmp;
            }
        }

       private String getFileName() {
           return REC_FILE_PATH + mFileNameTextBox.getText() + "_" + now.format("%Y_%m_%d_%H%M");
       }
    }

}
