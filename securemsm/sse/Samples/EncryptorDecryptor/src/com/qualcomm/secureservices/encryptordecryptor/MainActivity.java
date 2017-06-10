/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.secureservices.encryptordecryptor;

import android.os.Bundle;
import android.app.Activity;
import android.graphics.PixelFormat;
import android.view.Menu;
import android.view.WindowManager;
import android.widget.Toast;

/**
 * MainActivity Class. This class is responsible for the creation of the
 * application. An interface will be introduced and functionalities on basic UI
 * items will be provided, such as buttons, text viewers, radio buttons,
 * dialogues.
 *
 * @see android.app.Activity
 */
public class MainActivity extends Activity {

    // -------------------------Constants-------------------------
    /** A constant of type String holds the tag for this class. */
    private static final String TAG = "EncryptorDecryptor";
    /** A constant of type String holds the token's Label. */
    public static final String TOKEN_LABEL = "Token1";
    /** A constant of type String holds the key's Label. */
    public static final String KEY_LABEL = "Key1";

    // -------------------------Global Variables-------------------------
    /** A variable of type Functionality holds the functionality object. */
    private Functionality functionality = null;
    /** A variable of type Cryptography holds the cryptography object. */
    private Cryptography cryptography = null;

    /**
     * Constructor. This is an unimplemented constructor.
     */
    public MainActivity() {
    }

    /**
     * Method. This method is called when the application's activity is
     * initialized (first created). This is always followed by onStart() method.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Smooth the application's rendering.
        getWindow().setFormat(PixelFormat.RGBA_8888);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_DITHER);
        setContentView(R.layout.activity_main);
        // Set the context of the application.
        Configuration.context = getApplicationContext();
        // Make a call to initializationState method
        // to initialize a PKI and a token for the application.
        if (null == Configuration.initializationState()) {
          Toast toast = Toast.makeText(getApplicationContext(), "Failed to start PKCS11", Toast.LENGTH_SHORT);
          toast.show();
          finish();
          return;
        }

        this.functionality = new Functionality(this);
        this.cryptography = new Cryptography(this);
    }

    /**
     * Called when the application's activity becomes visible to the user. This
     * method is followed by onResume() method if the activity comes to the
     * foreground, or onStop() method if it becomes hidden.
     */
    public void onStart() {
        super.onStart();

        if (Pki.getPkiInstance() == null) {
            // Step 1: Initialize Pki library.
            Configuration.initializationState();
        }
    }

    /**
     * Called when the system is about to start resuming a previous activity.
     * This method is followed by either onResume() if the activity returns back
     * to the front, or onStop() if it becomes invisible to the user.
     */
    public void onPause() {
        super.onPause();

        // Step 5: Unload Pki library.
        Pki.closePki();
    }

    /**
     * Called after your activity has been stopped, prior to it being started
     * again. This is always followed by onStart() method.
     */
    public void onResume() {
        super.onResume();

        if (Pki.getPkiInstance() == null) {
            // Step 1: Initialize Pki library.
            Configuration.initializationState();
        } else {
            // Step 1: Reload Pki library.
            Pki.getPkiInstance();
        }
    }

    /**
     * Called when the activity is no longer visible to the user, because
     * another activity has been resumed and is covering this one. This method
     * is followed by either onRestart() if this activity is coming back to
     * interact with the user, or onDestroy() if this activity is going away.
     */
    public void onStop() {
        super.onStop();

        // Step 5: Unload Pki library.
        Pki.closePki();
    }

    /**
     * Final call received before the activity is destroyed. This can happen
     * either because the activity is finishing or because the system is
     * temporarily destroying this instance of the activity to save space.
     */
    public void onDestroy() {
        super.onDestroy();

        // Step 5: Unload Pki library.
        Pki.closePki();
    }

}
