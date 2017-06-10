/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.encryptdata;

import java.io.File;

import com.qapp.secprotect.Configs;
import com.qapp.secprotect.framework.MainApp;
import com.qapp.secprotect.utils.UtilsConvert;
import com.qapp.secprotect.utils.UtilsEncrypt;
import com.qapp.secprotect.utils.UtilsFileOperation;
import com.qapp.secprotect.utils.UtilsLog;

public class UtilsPassword {

    public final static int INTERNAL_INDEX = 0;
    public final static int SDCARD_INDEX = 1;

    public static boolean writePasswordToFile(String password, int storage) {
        final String KEY_DIR;
        String key = null;
        switch (storage) {
        case INTERNAL_INDEX:
            KEY_DIR = Configs.INTERNAL_ENCRYPT_CONFIG;
            break;
        case SDCARD_INDEX:
            KEY_DIR = Configs.SDCARD_ENCRYPT_CONFIG;
            break;
        default:
            return false;
        }
        byte[] encryptedPasswordBytes = UtilsEncrypt.encrypt(
                password.getBytes(), Configs.MAGIC);

        String encryptedPasswordString = UtilsConvert
                .byteToHex(encryptedPasswordBytes);
        String keyFileName = encryptedPasswordString
                + Configs.ENCRYPT_KEY_FILE_POFIX;
        String keyFilePath = KEY_DIR + "/" + keyFileName;

        File internalEncryptConfigDir = new File(KEY_DIR);
        internalEncryptConfigDir.mkdirs();

        if (internalEncryptConfigDir.canWrite()) {

            // search key file with .key
            File file, curKeyFile = null;
            File[] files = internalEncryptConfigDir.listFiles();
            for (int i = 0; i < files.length; i++) {
                file = files[i];
                if (file.getName().endsWith(Configs.ENCRYPT_KEY_FILE_POFIX)) {
                    curKeyFile = file;
                    break;
                }
            }

            // key file exists. To decrypt the key
            if (curKeyFile != null) {
                String curKeyFileName = curKeyFile.getName();
                // password is same with current password
                if (curKeyFileName.equals(keyFileName)) {
                    return true;
                }
                String encryptedPasswordHex = curKeyFileName.substring(0,
                        curKeyFileName.length()
                                - Configs.ENCRYPT_KEY_FILE_POFIX.length());
                byte[] passwordBytes = UtilsEncrypt.decrypt(
                        UtilsConvert.hexToByte(encryptedPasswordHex),
                        Configs.MAGIC);
                String passwordString = new String(passwordBytes);
                UtilsLog.logd("passwordString=" + passwordString);

                if (key == null) {
                    byte[] encryptedKeyBytes = UtilsFileOperation
                            .readFileBytes(curKeyFile.getPath());
                    byte[] keyBytes = UtilsEncrypt.decrypt(encryptedKeyBytes,
                            passwordString);
                    key = new String(keyBytes);
                }
            } else {
                // generate the new key
                key = generate16RandomKey(Configs.MAGIC);
            }

            byte[] encryptedKeyBytes = UtilsEncrypt.encrypt(key.getBytes(),
                    password);
            UtilsFileOperation.writeFile(keyFilePath, encryptedKeyBytes);
            if (curKeyFile != null)
                UtilsFileOperation.removeFile(curKeyFile);

            if (storage == INTERNAL_INDEX) {
                MainApp.getInstance().mInternalKey = key;
            } else
                MainApp.getInstance().mSdcardKey = key;

        } else
            return false;
        return true;
    }

    public static String loadKeyFromFile(int storage) {

        final String KEY_DIR;
        switch (storage) {
        case INTERNAL_INDEX:
            KEY_DIR = Configs.INTERNAL_ENCRYPT_CONFIG;
            break;
        case SDCARD_INDEX:
            KEY_DIR = Configs.SDCARD_ENCRYPT_CONFIG;
            break;
        default:
            return null;
        }

        File encryptConfigDir = new File(KEY_DIR);

        if (encryptConfigDir.canRead()) {

            // search key file with .key
            File file, keyFile = null;
            File[] files = encryptConfigDir.listFiles();
            for (int i = 0; i < files.length; i++) {
                file = files[i];
                if (file.getName().endsWith(Configs.ENCRYPT_KEY_FILE_POFIX)) {
                    keyFile = file;
                    break;
                }
            }

            // key file exists. To decrypt the key
            if (keyFile != null) {
                String curKeyFileName = keyFile.getName();

                String encryptedPasswordHex = curKeyFileName.substring(0,
                        curKeyFileName.length()
                                - Configs.ENCRYPT_KEY_FILE_POFIX.length());
                byte[] passwordBytes = UtilsEncrypt.decrypt(
                        UtilsConvert.hexToByte(encryptedPasswordHex),
                        Configs.MAGIC);
                String passwordString = new String(passwordBytes);

                byte[] encryptedKeyBytes = UtilsFileOperation
                        .readFileBytes(keyFile.getPath());
                byte[] keyBytes = UtilsEncrypt.decrypt(encryptedKeyBytes,
                        passwordString);
                return new String(keyBytes);
            } else {
                return null;
            }
        }
        return null;
    }

    public static String loadPasswordFromFile(int storage) {

        final String KEY_DIR;
        switch (storage) {
        case INTERNAL_INDEX:
            KEY_DIR = Configs.INTERNAL_ENCRYPT_CONFIG;
            break;
        case SDCARD_INDEX:
            KEY_DIR = Configs.SDCARD_ENCRYPT_CONFIG;
            break;
        default:
            return null;
        }

        File encryptConfigDir = new File(KEY_DIR);

        if (encryptConfigDir.canRead()) {

            // search key file with .key
            File file, keyFile = null;
            File[] files = encryptConfigDir.listFiles();
            for (int i = 0; i < files.length; i++) {
                file = files[i];
                if (file.getName().endsWith(Configs.ENCRYPT_KEY_FILE_POFIX)) {
                    keyFile = file;
                    break;
                }
            }

            // key file exists. To decrypt the key
            if (keyFile != null) {
                String curKeyFileName = keyFile.getName();

                String encryptedPasswordHex = curKeyFileName.substring(0,
                        curKeyFileName.length()
                                - Configs.ENCRYPT_KEY_FILE_POFIX.length());
                byte[] passwordBytes = UtilsEncrypt.decrypt(
                        UtilsConvert.hexToByte(encryptedPasswordHex),
                        Configs.MAGIC);
                return new String(passwordBytes);

            } else {
                return null;
            }
        }
        return null;
    }

    public static String loadPassword(int storage) {

        switch (storage) {
        case INTERNAL_INDEX:
            if (MainApp.getInstance().mInternalPassword == null) {
                MainApp.getInstance().mInternalPassword = loadPasswordFromFile(storage);
            }
            return MainApp.getInstance().mInternalPassword;
        case SDCARD_INDEX:
            if (MainApp.getInstance().mSdcardPassword == null) {
                MainApp.getInstance().mSdcardPassword = loadPasswordFromFile(storage);
            }
            return MainApp.getInstance().mSdcardPassword;
        default:
            return null;
        }
    }

    private static String generate16RandomKey(String head) {
        if (head == null)
            head = "";
        String key = head;
        key += UtilsEncrypt.getRandomString(16 - head.length());
        return key;
    }

}
