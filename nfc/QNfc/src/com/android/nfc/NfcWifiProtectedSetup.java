/*
* Copyright (C) 2014 The Android Open Source Project
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
package com.android.nfc;


import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiConfiguration;
import android.nfc.NdefMessage;
import android.nfc.NdefRecord;
import android.nfc.tech.Ndef;
import android.os.UserHandle;
import android.os.UserManager;

import java.util.Arrays;

public final class NfcWifiProtectedSetup {

    public static final String NFC_TOKEN_MIME_TYPE = "application/vnd.wfa.wsc";

    public static final String EXTRA_WIFI_CONFIG = "com.android.nfc.WIFI_CONFIG_EXTRA";

    /*
     * ID into configuration record for SSID and Network Key in hex.
     * Obtained from WFA Wifi Simple Configuration Technical Specification v2.0.2.1.
     */
    private static final String SSID_ID = "1045";
    private static final String NETWORK_KEY_ID = "1027";

    private static final int SIZE_FIELD_WIDTH = 4;
    private static final int MAX_SSID_SIZE_BYTES = 32;
    private static final int MAX_NETWORK_KEY_SIZE_BYTES = 64;
    private static final int HEX_CHARS_PER_BYTE = 2;

    private NfcWifiProtectedSetup() {}

    public static boolean tryNfcWifiSetup(Ndef ndef, Context context) {

        if (ndef == null || context == null) {
            return false;
        }

        NdefMessage cachedNdefMessage = ndef.getCachedNdefMessage();
        if (cachedNdefMessage == null) {
            return false;
        }

        final WifiConfiguration wifiConfiguration = parse(cachedNdefMessage);

        if (wifiConfiguration != null &&!UserManager.get(context).hasUserRestriction(
                UserManager.DISALLOW_CONFIG_WIFI, UserHandle.CURRENT)) {
            Intent configureNetworkIntent = new Intent()
                    .putExtra(EXTRA_WIFI_CONFIG, wifiConfiguration)
                    .setClass(context, ConfirmConnectToWifiNetworkActivity.class)
                    .setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

            context.startActivityAsUser(configureNetworkIntent, UserHandle.CURRENT);
            return true;
        }

        return false;
    }

    private static WifiConfiguration parse(NdefMessage message) {
        NdefRecord[] records = message.getRecords();

        for (int i = 0; i < records.length; ++i) {
            NdefRecord record = records[i];
            if (new String(record.getType()).equals(NFC_TOKEN_MIME_TYPE)) {
                String hexStringPayload = bytesToHex(record.getPayload());

                int ssidStringIndex = hexStringPayload.indexOf(SSID_ID);

                if (ssidStringIndex > 0) {
                    int networkKeyStringIndex = hexStringPayload.indexOf(NETWORK_KEY_ID);
                    if (networkKeyStringIndex > 0) {

                        ssidStringIndex += SSID_ID.length();
                        networkKeyStringIndex += NETWORK_KEY_ID.length();

                        String ssidSize;
                        try {
                            ssidSize = hexStringPayload.substring(ssidStringIndex,
                                    ssidStringIndex + SIZE_FIELD_WIDTH);
                        } catch(IndexOutOfBoundsException ex) {
                            return null;
                        }

                        int ssidSizeBytes = hexStringToInt(ssidSize);
                        if (ssidSizeBytes > MAX_SSID_SIZE_BYTES) {
                            return null;
                        }

                        String networkKeySize;
                        try {
                            networkKeySize = hexStringPayload.substring(networkKeyStringIndex,
                                    networkKeyStringIndex + SIZE_FIELD_WIDTH);
                        } catch (IndexOutOfBoundsException ex) {
                            return null;
                        }

                        int networkKeySizeBytes = hexStringToInt(networkKeySize);
                        if (networkKeySizeBytes > MAX_NETWORK_KEY_SIZE_BYTES) {
                            return null;
                        }

                        ssidStringIndex += SIZE_FIELD_WIDTH;
                        networkKeyStringIndex += SIZE_FIELD_WIDTH;

                        String ssid;
                        String networkKey;
                        try {
                            int ssidByteIndex = ssidStringIndex / HEX_CHARS_PER_BYTE;
                            ssid = new String(Arrays.copyOfRange(record.getPayload(), ssidByteIndex,
                                    ssidByteIndex + ssidSizeBytes));

                            int networkKeyByteIndex = networkKeyStringIndex / HEX_CHARS_PER_BYTE;
                            networkKey = new String(Arrays.copyOfRange(record.getPayload(),
                                            networkKeyByteIndex,
                                            networkKeyByteIndex + networkKeySizeBytes));
                        } catch (ArrayIndexOutOfBoundsException ex) {
                            return null;
                        }

                        WifiConfiguration configuration = new WifiConfiguration();
                        configuration.preSharedKey = '"' + networkKey + '"';
                        configuration.SSID = '"' + ssid + '"';

                        return configuration;
                    }
                }
            }
        }

        return null;
    }

    private static int hexStringToInt(String bigEndianHexString) {
        int val = 0;

        for (int i = 0; i < bigEndianHexString.length(); ++i) {
            val = (val | Character.digit(bigEndianHexString.charAt(i), 16));

            if (i < bigEndianHexString.length() - 1) {
                val <<= 4;
            }
        }

        return val;
    }

    private static String bytesToHex(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 2];
        for ( int j = 0; j < bytes.length; j++) {
            int value = bytes[j] & 0xFF;
            hexChars[j * 2] = Character.forDigit(value >>> 4, 16);
            hexChars[j * 2 + 1] = Character.forDigit(value & 0x0F, 16);
        }
        return new String(hexChars);
    }

}
