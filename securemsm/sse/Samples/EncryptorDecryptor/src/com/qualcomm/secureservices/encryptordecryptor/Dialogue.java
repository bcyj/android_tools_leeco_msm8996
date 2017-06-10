/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.secureservices.encryptordecryptor;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.ContextThemeWrapper;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.Toast;

/**
 * Dialogue Class. This class is responsible for the creation of the different
 * dialogues for the application. Depending on the dialogue, a different
 * operation/function is called.
 */
public class Dialogue extends DialogFragment {

    // -------------------------Constants-------------------------
    /** A constant of type String holds the tag for this class. */
    private static final String TAG = "DIALOGUE";

    // -------------------------Global Variables-------------------------
    /** A variable of type int holds the id of the dialog. */
    int dialogId = 0;

    public Dialogue(int dialogId) {
        this.dialogId = dialogId;
    }

    /**
     *
     * @param id
     * @return
     */
    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        final AlertDialog.Builder builder = new AlertDialog.Builder(
                getActivity());
        LayoutInflater inflater = getActivity().getLayoutInflater();

        switch (dialogId) {

        // The dialog for setting the password.
        case Functionality.DIALOG_PASSWORD_SET:
            // Dialog as a warning if the user has not set a password.
            // with a custom layout.
            View view = inflater.inflate(R.layout.set_password_dialog, null);
            builder.setView(view);
            builder.setTitle(R.string.setTitle);

            // Hook up the textView, the editText and the Image
            // with the appropriate variables.
            TextView text = (TextView) view.findViewById(R.id.dialog_text);
            ImageView image = (ImageView) view.findViewById(R.id.dialog_image);
            final EditText passwordInput = (EditText) view
                    .findViewById(R.id.dialog_password_input);

            // Customize the dialog.
            text.setText(R.string.requiredPasswordWarning);
            image.setImageResource(R.drawable.warning_sign);
            builder.setPositiveButton(R.string.setButton,
                    new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {
                            // Given password must exist.
                            if (passwordInput.getText().length() != 0) {
                                Toast toast = Toast.makeText(
                                        builder.getContext(),
                                        R.string.setSuccess, Toast.LENGTH_SHORT);
                                toast.setGravity(Gravity.CENTER, 0, 0);
                                toast.show();
                                // Call to the initOTPGenerator method.
                                Functionality.initAESGenerator(passwordInput
                                        .getText().toString());
                            }
                        }
                    });

            builder.setNegativeButton(R.string.cancelButton, null);

            break;

        // The dialog for resetting the password.
        case Functionality.DIALOG_PASSWORD_RESET:

            // Create a new dialog for when the user wants to change his
            // password with a custom layout.
            View resetPasswordView = inflater.inflate(
                    R.layout.reset_password_dialog, null);
            builder.setView(resetPasswordView);
            builder.setTitle(R.string.resetTitle);

            // Hook up the textView, the editText and the Image
            // with the appropriate variables.
            TextView resetText = (TextView) resetPasswordView
                    .findViewById(R.id.dialog_text);
            ImageView resetImage = (ImageView) resetPasswordView
                    .findViewById(R.id.dialog_image);
            final EditText resetPasswordInput = (EditText) resetPasswordView
                    .findViewById(R.id.dialog_password_input);

            // Customize the dialog.
            resetText.setText(R.string.resetPasswordWarning);
            resetImage.setImageResource(R.drawable.warning_sign);
            builder.setPositiveButton(R.string.resetButton,
                    new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {
                            // Given password must exist.
                            if (resetPasswordInput.getText().length() != 0) {
                                Toast toast = Toast.makeText(
                                        builder.getContext(),
                                        R.string.resetSuccess,
                                        Toast.LENGTH_SHORT);
                                toast.setGravity(Gravity.CENTER, 0, 0);
                                toast.show();
                                // Call to the resetOTPGenerator method.
                                Functionality
                                        .resetAESGenerator(resetPasswordInput
                                                .getText().toString());
                            }
                        }
                    });

            builder.setNegativeButton(R.string.cancelButton, null);

            break;

        // The dialog for inputting a text.
        case Functionality.DIALOG_TEXT_INPUT:

            // Create a new dialog for when the user wants to insert text
            // to be encrypted/decrypted.
            View inputTextView = inflater.inflate(R.layout.text_input_dialog,
                    null);
            builder.setView(inputTextView);
            builder.setTitle(R.string.inputTextTitle);

            // Hook up the textView, the editText and the Image
            // with the appropriate variables.
            TextView inputTextDetails = (TextView) inputTextView
                    .findViewById(R.id.dialog_text);
            ImageView inputTextImage = (ImageView) inputTextView
                    .findViewById(R.id.dialog_image);
            final EditText textInput = (EditText) inputTextView
                    .findViewById(R.id.dialog_text_input);

            // Customize the dialog.
            inputTextDetails.setText(R.string.inputTextWarning);
            inputTextImage.setImageResource(R.drawable.warning_sign);
            builder.setPositiveButton(R.string.setButton,
                    new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {

                            // Hook up the textView, the buttons with the
                            // appropriate values.
                            final TextView textViewer = (TextView) getActivity()
                                    .findViewById(R.id.textViewer);
                            final Button encryptButton = (Button) getActivity()
                                    .findViewById(R.id.encryptButton);
                            final Button decryptButton = (Button) getActivity()
                                    .findViewById(R.id.decryptButton);
                            textViewer.setText(textInput.getText());
                            // Enable scrolling for the textViewer.
                            textViewer
                                    .setMovementMethod(new ScrollingMovementMethod());
                            encryptButton.setEnabled(true);
                            decryptButton.setEnabled(false);
                        }
                    });

            builder.setNegativeButton(R.string.cancelButton, null);

            break;

        // The dialog showing an error to the application (corrupted data).
        case Functionality.DIALOG_CORRUPTED:

            // Create a new dialog as a warning if the application returned '-1'
            // (error).
            View corruptedView = inflater.inflate(
                    R.layout.corrupted_application_dialog, null);
            builder.setView(corruptedView);
            builder.setTitle(R.string.corruptedTitle);

            // Hook up the buttons, the textView and the Image
            // with the appropriate variables.
            TextView corruptedText = (TextView) corruptedView
                    .findViewById(R.id.dialog_text);
            ImageView corruptedImage = (ImageView) corruptedView
                    .findViewById(R.id.dialog_image);
            final Button powerOnButton = (Button) getActivity().findViewById(
                    R.id.powerOnButton);

            // Customize the dialog.
            corruptedText.setText(R.string.corruptedApplicationWarning);
            corruptedImage.setImageResource(R.drawable.corrupted_sign);
            builder.setPositiveButton(R.string.resetButton,
                    new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {
                            // Show the reset dialog.
                            powerOnButton.setSelected(false);
                            DialogFragment resetDialog = new Dialogue(
                                    Functionality.DIALOG_PASSWORD_RESET);
                            resetDialog.setCancelable(false);
                            resetDialog.show(getFragmentManager(),
                                    "resetPassword");
                        }
                    });

            builder.setNegativeButton(R.string.dismissButton,
                    new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {
                            powerOnButton.setSelected(false);
                        }
                    });

            break;

        // The dialog showing help for the application.
        case Functionality.DIALOG_HELP:

            // Create a new dialog as a warning if the user has not set a
            // password.
            View helpView = inflater.inflate(R.layout.help_dialog, null);
            builder.setView(helpView);
            builder.setTitle(R.string.helpTitle);

            break;

        }

        return builder.create();
    }

}
