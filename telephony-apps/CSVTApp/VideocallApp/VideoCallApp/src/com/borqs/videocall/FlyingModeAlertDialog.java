/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

//import android.app.AlddertDialog;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Window;

public class FlyingModeAlertDialog extends Activity
    implements DialogInterface.OnClickListener, DialogInterface.OnDismissListener {

	final static String PROMPT_RES_ID = "prompt_res_id";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        Intent intent = getIntent();
        int promptResID = intent.getIntExtra(PROMPT_RES_ID, R.string.call_failed_prompt);

//        // Set up the "dialog"
//        final AlertController.AlertParams p = mAlertParams;
//        p.mMessage = this.getString( promptResID);
//        //p.mPositiveButtonText = getString(com.android.internal.R.string.usb_storage_button_mount);
//        //p.mPositiveButtonListener = this;
//        p.mPositiveButtonText = getText( R.string.btn_confirm);
//
//        p.mPositiveButtonListener = this;
//        setupAlert();
        //cli
        setContentView(R.layout.blank);

        new AlertDialog.Builder(this).setMessage(this.getString(promptResID))
			.setIcon(R.drawable.cmcc_dialog_notice)
			.setTitle(R.string.alert_dialog_title)
			.setPositiveButton(android.R.string.ok, this)
                        .show()
                        .setOnDismissListener(this);
    }
    public void onDestroy()
    {
        super.onDestroy();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    /**
     * //Cancel
     */

    public void onClick(DialogInterface dialog, int which) {

        //finish();
    }

    public void onDismiss(DialogInterface dialog) {
        finish();
    }
}
