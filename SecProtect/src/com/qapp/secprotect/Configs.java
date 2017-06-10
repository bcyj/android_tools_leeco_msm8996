/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect;

public class Configs {

    public static final String TAG = "kunl";
    public static final String version = "1.0";
    public static final String ENCRYPT_FILE_SUFFIX = ".encrypted";

    public static final String STORAGE_ROOT = "/storage";
    public static final String INTERNAL_STORAGE = "/storage/sdcard0";
    public static final String SDCARD_ROOT = "/storage/sdcard1";
    public static final String INTERNAL_STORAGE_REAL = "/data/media/0";
    public static final String SDCARD_REAL = "/mnt/media_rw/sdcard1";

    public static final String PROTECTED_PATH = ".secprotect";
    public static final String INTERNAL_PROTECTED_PATH = INTERNAL_STORAGE + "/"
            + PROTECTED_PATH;
    public static final String SDCARD_PROTECTED_PATH = SDCARD_ROOT + "/"
            + PROTECTED_PATH;

    public static final String ENCRYPT_CONFIG_PATH = ".encrypt_config";
    public static final String ENCRYPT_KEY_FILE_POFIX = ".key";
    public static final String INTERNAL_ENCRYPT_CONFIG = INTERNAL_PROTECTED_PATH
            + "/" + ENCRYPT_CONFIG_PATH;
    public static final String SDCARD_ENCRYPT_CONFIG = SDCARD_PROTECTED_PATH
            + "/" + ENCRYPT_CONFIG_PATH;
    

    public final static int MODE_ENCRYPT = 1;
    public final static int MODE_DECRYPT = 2;
    public final static int MODE_PROTECT = 3;
    public final static int MODE_DEPROTECT = 4;

    /**
     * Encrypt
     */
    public static final String MAGIC = "SecProtect";
    public static final String PREF_PASSWORD = "password";
    public static final String PREF_ENCRYPTED_KEY = "encrypted_key";
    public static final String INTENT_CREATE_PASSWORD = "create";
    public static final String INTENT_CHANGE_PASSWORD = "change";
    public static final String INTENT_LOGIN = "login";
    
}
