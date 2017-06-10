/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

public class GeneralModeAlertDialog extends AlertActivity implements DialogInterface.OnClickListener {
	final static String PROMPT_RES_ID = "prompt_res_id";
	protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        int promptResID = intent.getIntExtra(PROMPT_RES_ID, R.string.call_failed_prompt);

        final AlertController.AlertParams p = mAlertParams;
        p.mMessage = this.getString(promptResID);
        p.mIconId = R.drawable.cmcc_dialog_information;
		p.mTitle = this.getString(R.string.alert_dialog_title) ;
        p.mPositiveButtonText = getString(android.R.string.ok);
        p.mPositiveButtonListener = this;
        setupAlert();
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
	 public void onClick(DialogInterface dialog, int which) {
		 this.dismiss();
	    }

}
