/*
 * Copyright (c) 2015 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import java.util.ArrayList;
import java.util.List;
import android.app.ActionBar;
import android.app.Activity;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.RemoteException;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;

import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccountConstant;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccounts;
import com.suntek.mway.rcs.client.api.publicaccount.callback.PublicAccountCallback;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.nativeui.RcsApiManager;

public class RecommendPublicAccountActivity extends Activity implements OnItemClickListener {
	public static final String TAG = "RecommendPublicAccountActivity";

	private ListView mListView;
	private View mEmptyView;
	private List<PublicAccounts> mList = new ArrayList<PublicAccounts>();
	private RecommendListAdapter mAdapter;
	private static final int PAGE_SIZE = 10;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_publicaccount_recommend_list);
		initActionBar();
		initView();
	}

	private void initActionBar() {
		ActionBar actionBar = getActionBar();
		actionBar.setDisplayUseLogoEnabled(false);
		actionBar.setDisplayHomeAsUpEnabled(true);
		actionBar.setDisplayShowHomeEnabled(false);
		Drawable myDrawable = getResources().getDrawable(
				R.color.public_account_bar_background_color);
		actionBar.setBackgroundDrawable(myDrawable);
	}

	private void initView() {
		mListView = (ListView) findViewById(R.id.recom_list);
		mEmptyView = findViewById(R.id.empty);
		mAdapter = new RecommendListAdapter(this, mList);
		mListView.setAdapter(mAdapter);
		mListView.setOnItemClickListener(this);
	}

	private void getRecommendPublicAccounts(int page) {
		try {
			RcsApiManager.getPublicAccountApi().getRecommendPublic(1, PAGE_SIZE, page, new PublicAccountCallback() {
				@Override
				public void respSetAcceptStatus(boolean arg0, String arg1)
						throws RemoteException {
				}

				@Override
				public void respGetPublicRecommend(boolean arg0,
						List<PublicAccounts> arg1) throws RemoteException {
					super.respGetPublicRecommend(arg0, arg1);
					
				}
			});
		} catch (ServiceDisconnectedException e) {
			e.printStackTrace();
		}
	}
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case android.R.id.home:
			finish();
			return true;
		}
		return false;
	}

	@Override
	public void onItemClick(AdapterView<?> parent, View view, int position,
			long id) {
		
	}
}
