/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

import java.util.ArrayList;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;


public class VTCallDetailAlertActivity extends AlertActivity implements CallTime.OnTickListener {

	static final String TAG = "VTCallDetailAlert";
	private CallTime mTimer;
	private static final int DETAIL_ITEM_NUM = 3;
	private String [] mStrDetailItems = new String[DETAIL_ITEM_NUM];
	private String mStrMsg;

	private final long DETAIL_STAY_TIME = 3;		//seconds

	static final String CALL_DETAIL_KEY = "Call_Detail_Key";
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		if (MyLog.DEBUG) MyLog.d(TAG, "onCreate()...");
		Intent intent = this.getIntent();
		mStrMsg = intent.getStringExtra(CALL_DETAIL_KEY);

		final AlertController.AlertParams p = mAlertParams;

		//p.mItems = mStrDetailItems;
		p.mMessage = mStrMsg;
        setupAlert();

		mTimer = new CallTime (this);
		mTimer.startTimer();
	}

	public void onTickForCallTimeElapsed(long timeElapsed) {
		// TODO Auto-generated method stub
		if (MyLog.DEBUG) MyLog.d(TAG, "time for cancel timer: " + timeElapsed + ".");
		if (timeElapsed < DETAIL_STAY_TIME) {
            return;
        } else {
		// 1 seconds
		if (MyLog.DEBUG) MyLog.d(TAG, "destroy");
            finish();
            return;
        }
	}



	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		mTimer.cancelTimer();
		super.onDestroy();
		//VideoCallApp.getInstance().tryEndApplication();
		if (MyLog.DEBUG) MyLog.d(TAG, "Call Detail Activity destoryed.");
	}
}
