/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2007 The Android Open Source Project
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

package com.wdstechnology.android.kryten.security;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.security.InvalidKeyException;
import java.security.Key;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;

import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;

public abstract class OmaSigner {

    public static final int NETWPIN = 0;
    public static final int USERPIN = 1;
    public static final int USERNETWPIN = 2;
    public static final int USERPINMAC = 3;
    byte[] mDocument;
    String mImsi;

    private OmaSigner(byte[] document, String imsi) {
        this.mDocument = document;
        this.mImsi = imsi;
    };

    public boolean isDocumentValid(String userSuppliedPin, byte[] mac) {

        try {
            byte[] pin = getSigningPin(userSuppliedPin);
            return Arrays.equals(mac, sign(mDocument, pin));
        } catch (Exception e) {
            return false;
        }
    }

    abstract byte[] getSigningPin(String userSuppliedPin) throws IOException;

    public byte[] sign(byte[] document, byte[] pin) throws NoSuchAlgorithmException,
            InvalidKeyException {
        Mac hmacSha1 = Mac.getInstance("HmacSHA1");
        Key key = new SecretKeySpec(pin, 0, pin.length, hmacSha1.getAlgorithm());
        hmacSha1.init(key);
        return hmacSha1.doFinal(document);
    }

    public abstract boolean isUserPinRequired();

    public static OmaSigner signerFor(int securityType, byte[] document, String imsi)
            throws UnknownSecurityMechanismException {
        switch (securityType) {
            case NETWPIN:
                return new NetwPinOmaSigner(document, imsi);
            case USERPIN:
                return new UserPinOmaSigner(document, imsi);
            case USERNETWPIN:
                return new UserNetwPinOmaSigner(document, imsi);
            case USERPINMAC:
                return new UserPinMacOmaSigner(document, imsi);
            default:
                throw new UnknownSecurityMechanismException("Do not recognise security mechanism "
                        + securityType);
        }
    }

    static byte[] nibbleSwapAndAddParityToImsi(String imsi) throws UnsupportedEncodingException {
        byte[] bytes = (imsi.length() % 2 == 1) ? hexAsciiToByteArray("9" + imsi)
                : hexAsciiToByteArray("1" + imsi + "F");

        for (int i = 0; i < bytes.length; i++) {
            int upper = (bytes[i] & 0xFF) >> 4;
            bytes[i] <<= 4;
            bytes[i] |= upper;
        }

        return bytes;
    }

    static byte[] hexAsciiToByteArray(String input) {
        ByteArrayOutputStream bytes = new ByteArrayOutputStream();

        if (input.length() % 2 == 1) {
            throw new RuntimeException("Bad HEXASCII - odd number of digits");
        }

        for (int i = 0; i < input.length(); i += 2) {
            bytes.write(Integer.parseInt(input.substring(i, i + 2), 16));
        }

        return bytes.toByteArray();
    }

    private static class UserPinOmaSigner extends OmaSigner {

        public UserPinOmaSigner(byte[] document, String imsi) {
            super(document, imsi);
        }

        @Override
        public boolean isUserPinRequired() {
            return true;
        }

        @Override
        byte[] getSigningPin(String userSuppliedPin) throws IOException {
            return userSuppliedPin.getBytes("US-ASCII");
        }
    }

    private static class NetwPinOmaSigner extends OmaSigner {

        public NetwPinOmaSigner(byte[] document, String imsi) {
            super(document, imsi);
        }

        @Override
        public boolean isUserPinRequired() {
            return false;
        }

        @Override
        byte[] getSigningPin(String userSuppliedPin) throws IOException {
            return nibbleSwapAndAddParityToImsi(mImsi);
        }
    }

    private static class UserNetwPinOmaSigner extends OmaSigner {

        public UserNetwPinOmaSigner(byte[] document, String imsi) {
            super(document, imsi);
        }

        @Override
        public boolean isUserPinRequired() {
            return true;
        }

        @Override
        byte[] getSigningPin(String userSuppliedPin) throws IOException {
            ByteArrayOutputStream pin = new ByteArrayOutputStream();
            pin.write(nibbleSwapAndAddParityToImsi(mImsi));
            pin.write(userSuppliedPin.getBytes("US-ASCII"));
            return pin.toByteArray();
        }
    }

    private static class UserPinMacOmaSigner extends OmaSigner {

        public UserPinMacOmaSigner(byte[] document, String imsi) {
            super(document, imsi);
        }

        @Override
        public boolean isUserPinRequired() {
            return true;
        }

        @Override
        byte[] getSigningPin(String userSuppliedPin) throws IOException {
            throw new RuntimeException("Shouldn't be called");
        }

        @Override
        public boolean isDocumentValid(String userSuppliedPin, byte[] mac) {

            try {
                int sectionLength = userSuppliedPin.length() / 2;
                if (sectionLength < 5) {
                    return false;
                }
                String randomSection = userSuppliedPin.substring(0, sectionLength);
                String macDerivedSection = userSuppliedPin.substring(sectionLength);

                byte[] digest = sign(mDocument, randomSection.getBytes("US-ASCII"));
                StringBuffer ourDerivedSection = new StringBuffer();
                for (int i = 0; i < sectionLength; i++) {
                    ourDerivedSection.append((char) ((digest[i] & 0xFF) % 10 + 48));
                }
                return macDerivedSection.equals(ourDerivedSection.toString());
            } catch (Exception e) {
                return false;
            }
        }

    }
}
