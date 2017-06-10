/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.secureservices.encryptordecryptor;

import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.TabHost.TabSpec;
import android.widget.Toast;

/**
 * Functionality Class. This class is responsible for the functionality of the
 * application. This class will set up the tabs of the application, the
 * listeners on the buttons and also the enabled/disabled settings.
 */
public class Functionality {

    // -------------------------Constants-------------------------
    /** A constant of type String holds the tag for this class. */
    private static final String TAG = "EncryptorDecryptor";

    // -------------------------Constants-------------------------
    /**
     * A static final variable of type int holds the dialog error number for
     * setting a password.
     */
    public static final int DIALOG_PASSWORD_SET = 0;
    /**
     * A static final variable of type int holds the dialog error number for
     * reseting a password.
     */
    public static final int DIALOG_PASSWORD_RESET = 1;
    /**
     * A static final variable of type int holds the dialog error number for
     * inputting the text to be encrypted.
     */
    public static final int DIALOG_TEXT_INPUT = 2;
    /**
     * A static final variable of type int holds the dialog shown when the
     * application is corrupted.
     */
    public static final int DIALOG_CORRUPTED = 3;
    /**
     * A static final variable of type int holds the dialog shown when help
     * button is pressed.
     */
    public static final int DIALOG_HELP = 4;

    // -------------------------Global Variables-------------------------
    /** A variable of type Activity holds the activity of the application. */
    private Activity activity = null;

    /**
     * Constructor.
     *
     * @param activity
     *            the activity of the application.
     */
    public Functionality(Activity activity) {
        this.activity = activity;
    }

    /**
     * Call the fundamental methods of Pki class to generate an AES key for the
     * user.
     *
     * @param userPassword
     *            the given password to be used for generating the OTP
     */
    public static void initAESGenerator(String userPassword) {

        Pki pki = Pki.getPkiInstance();
        if (pki == null) {
            pki = Configuration.initializationState();
            if (pki == null) {
                Log.e(Functionality.TAG, "Failed to initialize the library!");
                return;
            }
        }

        // Step 3: Creating an AES key.
        pki.createAESKey(userPassword, MainActivity.KEY_LABEL);
    }

    /**
     * Call the fundamental methods of Pki class to regenerate an OTP for the
     * user.
     *
     * @param userPassword
     *            the new given password to be used for regenerating the OTP
     */
    public static void resetAESGenerator(String userPassword) {

        Pki pki = Pki.getPkiInstance();
        if (pki == null) {
            pki = Configuration.initializationState();
            if (pki == null) {
                Log.e(Functionality.TAG, "Failed to initialize the library!");
                return;
            }
        }
        pki.deleteAESKey(MainActivity.KEY_LABEL); // Delete the old key.
        // Step 3: Recreating an AES key.
        pki.createAESKey(userPassword, MainActivity.KEY_LABEL);
    }

} // End of class.
