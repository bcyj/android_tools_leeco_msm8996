/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.

 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.

 * Copyright (C) 2008 The Android Open Source Project
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
package com.qualcomm.universaldownload;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.PowerManager;
import android.util.Log;
import android.provider.Telephony.Sms.Intents;
import com.android.internal.telephony.WspTypeDecoder;
import android.telephony.SmsMessage;

import java.io.ByteArrayOutputStream;

public class WapPushSLReceiver extends BroadcastReceiver {
    public static final String WAP_PUSH_RECEIVED = "android.provider.Telephony.WAP_PUSH_RECEIVED";
    public static final String DATA_SMS_RECEIVED = "android.intent.action.DATA_SMS_RECEIVED";
    private static final String TAG = "WapPushSLReceiver";

    private class ReceivePushTask extends AsyncTask<Intent, Void, String> {
        private final Context mContext;

        public ReceivePushTask(Context context){
            mContext = context;
        }
        @Override
        protected String doInBackground(Intent... intents) {
            Intent intent = intents[0];
            byte[] rawData = null;
            if(WAP_PUSH_RECEIVED.equals(intent.getAction())) {
                rawData = intent.getByteArrayExtra("data");
            } else {
                ByteArrayOutputStream output = new ByteArrayOutputStream();
                SmsMessage[] messages = Intents.getMessagesFromIntent(intent);
                for (int i=0; i <messages.length; i++) {
                    output.write(messages[i].getUserData(), 0, messages[i].getUserData().length);
                }
                rawData = createDataFromPdu(output.toByteArray());
            }
            WapPushSLWbxmlParser parserWbxml = null;
            try {
                parserWbxml = new WapPushSLWbxmlParser(rawData);
                parserWbxml.Parser();
            }
            catch (Exception ex) {
                Log.d(TAG, ex.toString());
            }
            Log.d(TAG, "content: " + parserWbxml.getContent());
            String address = parserWbxml.getContent();
            return address;
        }

        @Override
        protected void onPostExecute(String address) {
            super.onPostExecute(address);
            Intent newIntent = new Intent(WAP_PUSH_RECEIVED);
            newIntent.setClass(mContext, DownloadService.class);
            newIntent.putExtra(DownloadService.EXTRA_UPDATE_ADDRESS, address);
            mContext.startService(newIntent);
        }

        byte[] createDataFromPdu(byte[] pdu) {
            int index = 0;
            int transactionId = pdu[index++] & 0xFF;
            int pduType = pdu[index++] & 0xFF;
            int headerLength = 0;

            if ((pduType != WspTypeDecoder.PDU_TYPE_PUSH) &&
                    (pduType != WspTypeDecoder.PDU_TYPE_CONFIRMED_PUSH)) {
                if (false) Log.w(TAG, "Received non-PUSH WAP PDU. Type = " + pduType);
                return null;
            }

            WspTypeDecoder pduDecoder = new WspTypeDecoder(pdu);

            /**
             * Parse HeaderLen(unsigned integer).
             * From wap-230-wsp-20010705-a section 8.1.2
             * The maximum size of a uintvar is 32 bits.
             * So it will be encoded in no more than 5 octets.
             */
            if (pduDecoder.decodeUintvarInteger(index) == false) {
                if (false) Log.w(TAG, "Received PDU. Header Length error.");
                return null;
            }
            headerLength = (int)pduDecoder.getValue32();
            index += pduDecoder.getDecodedDataLength();

            int headerStartIndex = index;

            /**
             * Parse Content-Type.
             * From wap-230-wsp-20010705-a section 8.4.2.24
             *
             * Content-type-value = Constrained-media | Content-general-form
             * Content-general-form = Value-length Media-type
             * Media-type = (Well-known-media | Extension-Media) *(Parameter)
             * Value-length = Short-length | (Length-quote Length)
             * Short-length = <Any octet 0-30>   (octet <= WAP_PDU_SHORT_LENGTH_MAX)
             * Length-quote = <Octet 31>         (WAP_PDU_LENGTH_QUOTE)
             * Length = Uintvar-integer
             */
            if (pduDecoder.decodeContentType(index) == false) {
                if (false) Log.w(TAG, "Received PDU. Header Content-Type error.");
                return null;
            }

            String mimeType = pduDecoder.getValueString();
            long binaryContentType = pduDecoder.getValue32();
            index += pduDecoder.getDecodedDataLength();

            byte[] header = new byte[headerLength];
            System.arraycopy(pdu, headerStartIndex, header, 0, header.length);

            byte[] intentData;

            if (mimeType != null && mimeType.equals(WspTypeDecoder.CONTENT_TYPE_B_PUSH_CO)) {
                intentData = pdu;
            } else {
                int dataIndex = headerStartIndex + headerLength;
                intentData = new byte[pdu.length - dataIndex];
                System.arraycopy(pdu, dataIndex, intentData, 0, intentData.length);
            }

            return intentData;
        }
    }


    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if(WAP_PUSH_RECEIVED.equals(action) || DATA_SMS_RECEIVED.equals(action)) {
            Log.v(TAG, "Received WapPush intent: " + intent);
            PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
            PowerManager.WakeLock wl = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
            wl.acquire(5000);

            ReceivePushTask task = new ReceivePushTask(context);
            task.execute(intent);
        }
    }
}
