/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.secureservices.encryptordecryptor;

import java.io.IOException;
import javax.crypto.spec.IvParameterSpec;
import android.util.Log;

/**
 * Java interface class for Cryptoki library.
 */
public class Pki {

    private static final String TAG = "EncryptorDecryptor.PKI";
    private static Pki instance = null;         /** A static variable of type Pki holds the instance of this class. */
    private static byte[] encryptedData = null; /** An array of type byte holds the encrypted data. */

    static {
       System.loadLibrary("P11EncryptorDecryptor"); // libP11EncryptorDecryptor.so
    }

    private native boolean p11CryptoInit        (String initStr);
    private native boolean p11CryptoObtainToken (String label);
    private native long    p11CryptoCreateKey   (String pasword, String label);
    private native long    p11CryptoRetrieve    (String label);
    private native boolean p11CryptoDelete      (String KeyLabel);
    private native byte[]  p11CryptoEncrypt     (byte[]in, long key_handle);
    private native byte[]  p11CryptoDecrypt     (byte[]in, long key_handle);
    private static native boolean p11CryptoClose();


    private Pki(String initStr) throws IOException {
        if (false == p11CryptoInit(initStr)) {
            throw new IOException("Cannot load shared library.");
        }
    }

    /**
     * Returns the Pki object instance, if already existing.
     * @return the instance of the Pki object (null if not initialized).
     */
    public static Pki getPkiInstance() {
        return Pki.instance;
    }

    /**
     * Initialize the Pki object, if not already initialized.
     * @param  initPki : the configuration string
     * @return the instance of the Pki object.
     * @throws IOException
     */
    public static Pki initPkiInstance(String initPki) throws IOException {
        if (Pki.instance == null) {
            Pki.instance = new Pki(initPki);
        }
        return Pki.instance;
    }

    /**
     * Finalize the PKCS module and reset it to null.
     */
    public static void closePki() {
        if (Pki.instance != null) {
            p11CryptoClose();
        }
        Pki.instance = null;
    }


    /**
     * Initialising the Token.
     * @return true if a token is initialized.
     * @return false if slots with tokens could not be loaded, or if tokens
     *         could not be loaded from the slots, or if a token without secured
     *         keypad could not be found, or if a token could not be
     *         initialized.
     */
    public boolean obtainToken(String tokenLabel) {
        return p11CryptoObtainToken(tokenLabel);
    }

    /**
     * Create an AES key using the PBKDF2 mechanism. The salt used in the
     * generation is configured as empty string.
     * @param password : the password to use for the key generation.
     * @param keyLabel : the label to be given to the key.
     * @return null if the key is not initialized
     * @return the generated key.
     */
    public boolean createAESKey(String password, String keyLabel) {
        if (0 != p11CryptoCreateKey(password,keyLabel)){
            return true;
        }
        return false;
    }

    /**
     * Retrieve the key generated and saved in the token.
     * @param keyLabel :the label to be used to find the key.
     * @return null if the key could not be found.
     * @return the retrieved key.
     */
    public long retrieveAESKey(String keyLabel) {
        return p11CryptoRetrieve(keyLabel);
    }

    /**
     * Delete the key generated and saved in the token.
     * @param keyLabel : the label to be used to delete the key.
     */
    public void deleteAESKey(String keyLabel) {
        p11CryptoDelete(keyLabel);
    }

    /**
     * Encrypt a text with a given AES key.
     * @param text : the text to be encrypted.
     * @param key  : the key to be used for encryption.
     * @return the encrypted value.
     */
    public String encrypt(String text, long key_handle) {
        byte[] in  = text.getBytes();
        StringBuffer res = new StringBuffer();
        encryptedData = p11CryptoEncrypt(in,key_handle);
        for (int i = 0; i < this.encryptedData.length; i++) {
            res.append(String.format("0x%02X ", this.encryptedData[i]));
        }
        return res.toString();
    }

    /**
     * Decrypt a ciphertext with a given AES key.
     * @param key : the key to be used for decryption.
     * @return the decrypted value.
     */
    public String decrypt(long key_handle) {
        byte[] byteText = p11CryptoDecrypt(encryptedData,key_handle);
        String str = new String(byteText);
        return str;
    }

} // End of class.
