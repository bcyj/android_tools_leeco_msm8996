/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.secureservices.encryptordecryptor;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;

/**
 * Configuration Class. This class is used as a static utility class and it
 * offers the initializationState method. This method is used to initialize the
 * state of the application. It also offers the methods that are needed for the
 * application's configuration, returning the appropriate configuration in a
 * String format and also reading/writing from/to a file.
 *
 */
public class Configuration {

    // -------------------------Constants-------------------------
    /** A constant of type String holds the tag for this class. */
    private static final String TAG = "CONFIGURATION";
    /**
     * A constant of type String holds the application's storage direction
     * (name).
     */
    private static final String APP_STORAGE = "pkcs11";
    /**
     * A constant of type String holds the configuration file for the
     * application.
     */
    private static final String CONFIGURATION_FILE = "SSEConfiguration.txt";
    /** A variable of type Context holds the context of the application. */
    public static Context context;

    /**
     * Private Constructor. Make sure that this class is noninstantiable.
     */
    private Configuration() {
        throw new AssertionError();
    }

    /**
     * Initialise the Pki library. The initialisation will be made with the
     * written configurations given in a text file.
     */
    public static Pki initializationState() {

        Pki pki = null;

        try {
            // -------------------------Local Variables-------------------------
            // A private storage for the application.
            File storage = Configuration.context.getDir(
                    Configuration.APP_STORAGE, Context.MODE_PRIVATE);
            // Save path to the application.
            String path = storage.getAbsolutePath();
            // Call to getConfiguration returns 'SSEConfiguration.txt' file.
            String readConfiguration = Configuration.getConfiguration();
            // Form the initialization needed for Pki object.
            String initPki = readConfiguration + "\n" + "DB_PATH=" + path;
            // Step 1: Initialize Pki library.
            pki = Pki.initPkiInstance(initPki);

            if (pki == null) {
                Log.e(Configuration.TAG, "Failed to load PKI library");
                return null;
            }

            // Step 2: Create/Obtain Token for the application.
            if (false == pki.obtainToken(MainActivity.TOKEN_LABEL)) {
                Log.e(Configuration.TAG, "Failed to load/initialize the token");
                Pki.closePki();
                return null;
            }
        }

        catch (IOException ex) {
            Log.e(Configuration.TAG,
                    "OTP Generation Failed! \nDetails: " + ex.getMessage());
        }
        return pki;
    }

    /**
     * Read the configuration file from the asset folder and the salt file from
     * the files folder of the application.
     *
     * @return Configuration, as a string
     */
    public static String getConfiguration() {

        // Local variable holds what is read from the configuration file found
        // in assets folder.
        String configurationText;

        AssetManager assetManager = context.getAssets();
        InputStream inputStream1;

        try {
            // Read configuration file.
            inputStream1 = assetManager.open(Configuration.CONFIGURATION_FILE,
                    AssetManager.ACCESS_BUFFER);
            if (inputStream1 == null) {
                Log.e(Configuration.TAG, "Failed to open configuration file!");
                return null;
            }

        }

        catch (IOException ex) {
            Log.e(Configuration.TAG, ex.getMessage());
            return null;
        }

        // Read the file.
        configurationText = readTextFile(inputStream1);

        // Close file.
        try {
            inputStream1.close();
        } catch (IOException ex) {
            Log.e(Configuration.TAG, ex.getMessage());
        }

        return configurationText;
    }

    /**
     * Read a text file.
     *
     * @param inputStream
     *            Stream to read from
     * @return Data read from the file
     */
    private static String readTextFile(InputStream inputStream) {

        // --- local variables ---
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        byte buf[] = new byte[1024];
        int length;

        try {

            while ((length = inputStream.read(buf)) != -1) {
                outputStream.write(buf, 0, length);
            }

            outputStream.close();
            inputStream.close();
        }

        catch (IOException ex) {
            Log.e(Configuration.TAG, "Error readying the file! \nDetails: "
                    + ex.getMessage());
        }

        return outputStream.toString();
    }

} // End of class.
