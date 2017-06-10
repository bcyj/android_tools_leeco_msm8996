/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.secureservices.encryptordecryptor;

import android.app.Activity;
import android.app.DialogFragment;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

/**
 * Cryptography Class. This class is responsible for setting the appropriate
 * behavior on the application's buttons. Buttons may display dialogs to
 * navigate the user for the needed input/adjustment for the application to
 * work.
 *
 */
public class Cryptography {

    /**
     * A variable of type DialogFragment holds the dialog for setting the
     * password.
     */
    final DialogFragment setPasswordDialog = new Dialogue(
            Functionality.DIALOG_PASSWORD_SET);
    /**
     * A variable of type DialogFragment holds the dialog for resetting the
     * password.
     */
    final DialogFragment resetPasswordDialog = new Dialogue(
            Functionality.DIALOG_PASSWORD_RESET);
    /** A variable of type DialogFragment holds the dialog for inputting a text. */
    final DialogFragment textInputDialog = new Dialogue(
            Functionality.DIALOG_TEXT_INPUT);
    /** A variable of type DialogFragment holds the help dialog. */
    final DialogFragment helpDialog = new Dialogue(Functionality.DIALOG_HELP);
    /** A variable of type DialogFragment holds the corrupted dialog. */
    final DialogFragment corruptedDialog = new Dialogue(
            Functionality.DIALOG_CORRUPTED);
    /** A variable of type Activity holds the activity of the application. */
    private Activity activity = null;

    /**
     * Constructor.
     *
     * @param activity
     *            The activity of the application.
     */
    public Cryptography(Activity activity) {
        this.activity = activity;
        this.addListenerOnButton();
    }

    private void addListenerOnButton() {

        // Hook up the buttons and the textView with the appropriate variables.
        final Button powerOnButton = (Button) activity
                .findViewById(R.id.powerOnButton);
        final Button encryptButton = (Button) activity
                .findViewById(R.id.encryptButton);
        final Button decryptButton = (Button) activity
                .findViewById(R.id.decryptButton);
        final Button helpButton = (Button) activity
                .findViewById(R.id.helpButton);
        final Button settingsButton = (Button) activity
                .findViewById(R.id.settingsButton);
        final TextView textViewer = (TextView) activity
                .findViewById(R.id.textViewer);

        // Set a click listener to the power on button.
        powerOnButton.setOnClickListener(new OnClickListener() {

            /**
             * Set the behavior of the generator button when pressed.
             */
            public void onClick(View v) {
                if (!powerOnButton.isSelected()) { // If the button is not
                                                    // selected,
                    // and if an AES key is already created,
                    if (Pki.getPkiInstance().retrieveAESKey(
                            MainActivity.KEY_LABEL) != 0) {
                        powerOnButton.setSelected(true); // Set the button as
                                                            // selected.
                        textViewer.setText(R.string.default_text); // Show the
                                                                    // default
                                                                    // text.
                        disableSettings(); // and disable some functions.
                    } else {
                        // Otherwise show a dialog for setting the password.
                        setPasswordDialog.setCancelable(false);
                        setPasswordDialog.show(activity.getFragmentManager(),
                                "setPassword");
                    }

                } else {
                    powerOnButton.setSelected(false); // Otherwise set the
                                                        // button as not
                                                        // selected.
                    textViewer.setText(R.string.empty_text); // Clear the
                                                                // screen.
                    enableSettings(); // Enable the functions.
                }
            }
        });

        // Set a click listener to the encrypt button.
        encryptButton.setOnClickListener(new OnClickListener() {

            /**
             * Set the behavior of the encrypt button when pressed.
             */
            public void onClick(View v) {
                Pki pki = Pki.getPkiInstance();
                if (pki != null) {
                    String textViewerValue = textViewer.getText().toString();
                    // Set the text of the viewer to the encrypted data.
                    textViewer.setText(pki.encrypt(textViewerValue,
                            pki.retrieveAESKey(MainActivity.KEY_LABEL)));
                    if (textViewer.getText().toString().length() == 0) {
                        corruptedDialog.show(activity.getFragmentManager(),
                                "corruptedApp");
                        enableSettings();
                    } else {
                        // Enable the viewer to be scrollable.
                        textViewer
                                .setMovementMethod(new ScrollingMovementMethod());
                        // Enable decrypt button.
                        decryptButton.setEnabled(true);
                        // Disable encrypt button.
                        encryptButton.setEnabled(false);
                    }
                }
            }
        });

        // Set a click listener to the decrypt button.
        decryptButton.setOnClickListener(new OnClickListener() {

            /**
             * Set the behavior of the decrypt button when pressed.
             */
            public void onClick(View v) {
                Pki pki = Pki.getPkiInstance();
                if (pki != null) {
                    // Set the text of the viewer to the decrypted data.
                    textViewer.setText(pki.decrypt(pki
                            .retrieveAESKey(MainActivity.KEY_LABEL)));
                    if (textViewer.getText().toString().length() == 0) {
                        corruptedDialog.show(activity.getFragmentManager(),
                                "corruptedApp");
                        enableSettings();
                    } else {
                        // Enable the viewer to be scrollable.
                        textViewer
                                .setMovementMethod(new ScrollingMovementMethod());
                        // Enable encrypt button.
                        encryptButton.setEnabled(true);
                        // Disable decrypt button.
                        decryptButton.setEnabled(false);
                    }
                }
            }
        });

        // Set a click listener to the help button.
        helpButton.setOnClickListener(new OnClickListener() {

            /**
             * Set the behavior of the help button when pressed.
             */
            public void onClick(View v) {
                helpDialog.show(activity.getFragmentManager(), "helpDialog");
            }
        });

        // Set a click listener to the settings button.
        settingsButton.setOnClickListener(new OnClickListener() {

            /**
             * Set the behavior of the help button when pressed.
             */
            public void onClick(View v) {
                // If an AES key is already created,
                if (Pki.getPkiInstance().retrieveAESKey(MainActivity.KEY_LABEL) != 0) {
                    // Show a dialog for resetting the password.
                    resetPasswordDialog.setCancelable(false);
                    resetPasswordDialog.show(activity.getFragmentManager(),
                            "resetPassword");
                } else {
                    // Otherwise show a dialog for setting the password.
                    setPasswordDialog.setCancelable(false);
                    setPasswordDialog.show(activity.getFragmentManager(),
                            "setPassword");
                }

            }
        });

        // Set a click listener to the textViewer.
        textViewer.setOnClickListener(new OnClickListener() {

            /**
             * Set the behavior of the text viewer when pressed.
             */
            public void onClick(View v) {
                textInputDialog.setCancelable(false);
                textInputDialog.show(activity.getFragmentManager(), "inputText");
            }
        });

    }

    /**
     * Enable settings for when the Power On button is switched off. Those
     * settings are enabled because when the Power On button is off, a password
     * can be changed when the key used before (if existing) will be deleted and
     * substituted with a new one. In addition, when the Power On button is off,
     * the label is disabled to simulate the Power off.
     */
    private void enableSettings() {
        // Hook up Buttons, TextView with the appropriate variables.
        final TextView textViewer = (TextView) activity
                .findViewById(R.id.textViewer);
        final Button encryptButton = (Button) activity
                .findViewById(R.id.encryptButton);
        final Button decryptButton = (Button) activity
                .findViewById(R.id.decryptButton);
        final Button settingsButton = (Button) activity
                .findViewById(R.id.settingsButton);

        textViewer.setEnabled(false); // Label is disabled (change of color).
        textViewer.setClickable(false); // and cannot be clicked.
        encryptButton.setEnabled(false);
        decryptButton.setEnabled(false);
        settingsButton.setEnabled(true);
    }

    /**
     * Disable settings for when the Power On button is switched on. Those
     * settings are disabled because when the Power On button is on, a key for
     * the AES is generated. When a key is generated, password cannot be
     * changed. In addition, when the Power On button is on, the label acts like
     * a button to input text to encrypt.
     */
    private void disableSettings() {
        // Hook up Buttons, TextView with the appropriate variables.
        final TextView textViewer = (TextView) activity
                .findViewById(R.id.textViewer);
        final Button encryptButton = (Button) activity
                .findViewById(R.id.encryptButton);
        final Button decryptButton = (Button) activity
                .findViewById(R.id.decryptButton);
        final Button settingsButton = (Button) activity
                .findViewById(R.id.settingsButton);

        textViewer.setEnabled(true); // Label is enabled (change of color),
        textViewer.setClickable(true); // and can be clicked.
        settingsButton.setEnabled(false);
    }
}
