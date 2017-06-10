/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.utils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.Serializable;
import java.util.Random;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.spec.SecretKeySpec;

public class UtilsEncrypt {

    static final int HEADER_SIZE = 128;
    static final int AES_BLOCK_SIZE = 16;
    static final int BUFFER_SIZE = 4096;

    static class FileDescriptor implements Serializable {

        private static final long serialVersionUID = 1L;
        String path;
        int size;
    }

    static byte[] makeHeader(Object obj) {

        byte[] header = new byte[HEADER_SIZE];
        byte[] fileDescriptor = UtilsConvert.ObjectToByte(obj);
        System.arraycopy(fileDescriptor, 0, header, 0, fileDescriptor.length);
        return header;
    }

    public static byte[] encrypt(byte[] rawValue, String key) {

        // Pad the key to 16 characters (128 bytes)
        String temp = key + "1234567890123456";
        temp = temp.substring(0, 16);
        byte[] rawKey = temp.getBytes();

        SecretKeySpec skeySpec = new SecretKeySpec(rawKey, "AES");
        Cipher cipher;
        try {
            cipher = Cipher.getInstance("AES");
            cipher.init(Cipher.ENCRYPT_MODE, skeySpec);
            byte[] encrypted = cipher.doFinal(rawValue);
            return encrypted;
        } catch (Exception e) {
            UtilsLog.loge(e);
            return null;
        }
    }

    public static byte[] decrypt(byte[] rawValue, String key) {

        // Pad the key to 16 characters (128 bits)
        String temp = key + "1234567890123456";
        temp = temp.substring(0, 16);
        byte[] rawKey = temp.getBytes();

        SecretKeySpec skeySpec = new SecretKeySpec(rawKey, "AES");
        Cipher cipher;
        try {
            cipher = Cipher.getInstance("AES");
            cipher.init(Cipher.DECRYPT_MODE, skeySpec);
            byte[] decrypted = cipher.doFinal(rawValue);
            return decrypted;
        } catch (Exception e) {
            UtilsLog.loge(e);
            return null;
        }
    }

    public static byte[] getData(byte[] rawValue, Cipher cipher) {

        if (cipher == null) {
            return null;
        }
        byte[] encrypted = null;
        try {
            encrypted = cipher.doFinal(rawValue);
        } catch (IllegalBlockSizeException e) {
            e.printStackTrace();
        } catch (BadPaddingException e) {
            e.printStackTrace();
        }
        return encrypted;
    }

    static boolean encrytFile(String inPath, String outPath) {
        File inFile = new File(inPath);
        if (!inFile.exists())
            return false;

        File outFile = new File(outPath);

        FileInputStream fileInputStream = null;
        FileOutputStream fileOutputStream = null;
        FileDescriptor fileDescriptor = new FileDescriptor();

        try {
            fileInputStream = new FileInputStream(inFile);
            fileOutputStream = new FileOutputStream(outFile);

            fileDescriptor.path = inPath;
            fileDescriptor.size = fileInputStream.available();

            byte[] header = makeHeader(fileDescriptor);

            fileOutputStream.write(header);

            int cnt;
            byte[] buf = new byte[BUFFER_SIZE];
            while ((cnt = fileInputStream.read(buf)) >= 0) {
                byte[] rawData = UtilsConvert.subBytes(buf, 0, cnt);
                byte[] encryptData = UtilsEncrypt.encrypt(rawData, "hello");
                fileOutputStream.write(encryptData, 0, encryptData.length);
            }

        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return false;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        } finally {
            if (fileInputStream != null) {
                try {
                    fileInputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                    return false;
                }
            }
            if (fileOutputStream != null) {
                try {
                    fileOutputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                    return false;
                }
            }
        }
        return true;
    }

    static int roundUpBlockSize(int bufferSize) {
        int size = (bufferSize / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
        return size;
    }

    static void decrytFile(String path) {
        File file = new File(path);
        File out = new File("g:\\21.txt");

        FileDescriptor fileDescriptor = new FileDescriptor();

        try {
            FileInputStream fileInputStream = new FileInputStream(file);
            FileOutputStream outFileOutputStream = new FileOutputStream(out);

            byte[] header = new byte[HEADER_SIZE];
            fileInputStream.read(header);
            fileDescriptor = (FileDescriptor) UtilsConvert.ByteToObject(header);

            int cnt;
            byte[] buf = new byte[roundUpBlockSize(BUFFER_SIZE)];
            while ((cnt = fileInputStream.read(buf)) >= 0) {
                byte[] decryptData = UtilsEncrypt.decrypt(buf, "hello");
                outFileOutputStream.write(decryptData, 0, decryptData.length);
            }

            outFileOutputStream.close();
            fileInputStream.close();

        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static byte[] hexToByte(String hexString) {
        int len = hexString.length() / 2;
        byte[] result = new byte[len];
        for (int i = 0; i < len; i++)
            result[i] = Integer.valueOf(hexString.substring(2 * i, 2 * i + 2),
                    16).byteValue();
        return result;
    }

    public static String byteToHex(byte[] buf) {
        final String HEX = "0123456789ABCDEF";
        if (buf == null)
            return "";
        StringBuffer result = new StringBuffer(2 * buf.length);
        for (int i = 0; i < buf.length; i++) {
            result.append(HEX.charAt((buf[i] >> 4) & 0x0f)).append(
                    HEX.charAt(buf[i] & 0x0f));
        }
        return result.toString();
    }

    public static byte[] xorEncrypt(byte[] buf) {
        final byte XOR_MASK = 0x5;
        byte[] result = new byte[buf.length];
        for (int i = 0; i < result.length; i++) {
            result[i] = (byte) (buf[i] ^ XOR_MASK);
        }
        return result;
    }

    public static byte[] xorEncrypt(byte[] buf, byte mask) {
        byte[] result = new byte[buf.length];
        for (int i = 0; i < result.length; i++) {
            result[i] = (byte) (buf[i] ^ mask);
        }
        return result;
    }

    public static byte[] xorDecrypt(byte[] buf) {
        final byte XOR_MASK = 0x5;
        byte[] result = new byte[buf.length];
        for (int i = 0; i < result.length; i++) {
            result[i] = (byte) (buf[i] ^ XOR_MASK);
        }
        return result;
    }

    public static String getRandomString(int length) {
        final String LIBRARY = "0123456789abcdefghijklmnopqrstuvwxyz";
        Random random = new Random();
        StringBuffer stringBuffer = new StringBuffer();
        for (int i = 0; i < length; i++) {
            int number = random.nextInt(LIBRARY.length());
            stringBuffer.append(LIBRARY.charAt(number));
        }
        return stringBuffer.toString();
    }

}
