/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import android.app.ActionBar;
import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.nativeui.RcsApiManager;
import com.suntek.mway.rcs.nativeui.image.ImageLoader;
import com.suntek.mway.rcs.nativeui.utils.ImageUtils;
import com.suntek.mway.rcs.nativeui.utils.RcsContactUtils;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.MenuInfo;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccountConstant;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccounts;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccountsDetail;
import com.suntek.mway.rcs.client.api.publicaccount.callback.PublicAccountCallback;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;

public class PublicAccountActivity extends Activity {
    public static final String TAG = "PublicAccountActivity";
    private static final int REQUEST_PUBLIC_ACCOUNT_DETAIL = 1;
    private ListView listView;
    private TextView textEmpty;
    private boolean mFollowStateChanged = false;
    private OnItemClickListener itemListener;
    private boolean mForward;

    private PublicAccountCallback callback = new PublicAccountCallback() {

        @Override
        public void respGetUserSubscribePublicList(final boolean result,
                final List<PublicAccounts> publicAccountList) throws RemoteException {
            try {
                RcsApiManager.getPublicAccountApi().unregisterCallback(this);
            } catch (ServiceDisconnectedException e) {
                e.printStackTrace();
            }
            runOnUiThread(new Runnable() {

                @Override
                public void run() {
                    if (publicAccountList != null && publicAccountList.size() > 0) {
                        listView.setAdapter(new AccountListAdapter(PublicAccountActivity.this,
                                publicAccountList));
                        listView.setOnItemClickListener(itemListener);
//                        new UpdateAccountPhotoTask(publicAccountList).execute();
                        Toast.makeText(
                                PublicAccountActivity.this,
                                PublicAccountActivity.this.getResources().getString(
                                        R.string.sync_success), Toast.LENGTH_SHORT).show();
                    } else {
                        boolean empty = (listView.getAdapter() == null)
                                || (listView.getAdapter() != null && listView.getAdapter()
                                        .getCount() == 0);
                        if (result) {
                            if (empty) {
                                Toast.makeText(
                                        PublicAccountActivity.this,
                                        PublicAccountActivity.this.getResources().getString(
                                                R.string.success_but_empty), Toast.LENGTH_SHORT)
                                        .show();
                            } else {
                                Toast.makeText(
                                        PublicAccountActivity.this,
                                        PublicAccountActivity.this.getResources().getString(
                                                R.string.sync_success), Toast.LENGTH_SHORT).show();
                            }
                        } else {
                            if (empty) {
                                Toast.makeText(PublicAccountActivity.this,
                                        R.string.failure_and_empty, Toast.LENGTH_SHORT).show();
                            }
                        }
                    }
                }
            });

        }

        @Override
        public void respGetPublicList(boolean arg0, List<PublicAccounts> arg1)
                throws RemoteException {
        }

        @Override
        public void respGetPublicDetail(boolean arg0, PublicAccountsDetail arg1)
                throws RemoteException {

        }

        @Override
        public void respComplainPublicAccount(boolean arg0, PublicAccounts arg1)
                throws RemoteException {

        }

        @Override
        public void respSetAcceptStatus(boolean arg0, String arg1) throws RemoteException {

        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.fragment_main);
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayUseLogoEnabled(false);
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setDisplayShowHomeEnabled(false);
        getIntentExtra();
        Drawable myDrawable = getResources().getDrawable(
                R.color.public_account_bar_background_color);
        actionBar.setBackgroundDrawable(myDrawable);
        textEmpty = (TextView) findViewById(R.id.text_empty);
        listView = (ListView) findViewById(R.id.list_accounts);
        itemListener = new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                AccountListAdapter adapter = (AccountListAdapter) listView.getAdapter();
                PublicAccounts account = adapter.getItem(position);
                Intent intent = new Intent(PublicAccountActivity.this, PublicDetailActivity.class);
                if (account.getLogo() != null) {
                    intent.putExtra("avatar_url", account.getLogo());
                }
                if (account != null && !mForward) {
                    intent.putExtra("id", account.getPaUuid());
                    startActivityForResult(intent, REQUEST_PUBLIC_ACCOUNT_DETAIL);
                }
                if (mForward) {
                    Intent it = new Intent();
                    it.putExtra("selectPublicId", account.getPaUuid());
                    setResult(RESULT_OK, it);
                    finish();
                }
            }
        };
        textEmpty.setVisibility(View.GONE);
        getUserSubscribePublicListCache();
    }

    private void getIntentExtra(){
        if (getIntent().hasExtra("forward")) {
            mForward = getIntent().getBooleanExtra("forward", false);
        }
    }

    private void getUserSubscribePublicListCache() {
    	final Handler handler = new Handler();
    	Thread t = new Thread() {
            @Override
            public void run() {
                if (!RcsContactUtils.isRcsConnection()) {
                    RcsContactUtils.sleep(500);
                    RcsContactUtils.setRcsConnectionState(true);
                }
                try {
                    final List<PublicAccounts> publicAccountList = RcsApiManager
                            .getPublicAccountApi().getUserSubscribePublicListCache(
                                    PublicAccountConstant.ACCOUNTLIST_ORDER_TYPE_FOLLOWTIME_DESC,
                                    1000, 1);
                    if (publicAccountList != null && publicAccountList.size() > 0) {
                        handler.post(new Runnable() {
                            @Override
                            public void run() {
                                listView.setAdapter(new AccountListAdapter(
                                        PublicAccountActivity.this, publicAccountList));
                                listView.setOnItemClickListener(itemListener);
//                                new UpdateAccountPhotoTask(publicAccountList).execute();
                            }
                        });
                    } else {
                        handler.post(new Runnable() {
                            @Override
                            public void run() {
                                listView.setAdapter(null);
                                listView.setOnItemClickListener(null);
                            }
                        });
                    }
                } catch (ServiceDisconnectedException e) {
                    handler.post(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(
                                    PublicAccountActivity.this,
                                    PublicAccountActivity.this.getResources().getString(
                                            R.string.rcs_service_is_not_available),
                                    Toast.LENGTH_SHORT).show();
                        }
                    });
                    e.printStackTrace();
                }
            }
        };
        t.start();
    }
    
	private void getUserSubscribePublicList() {
		try {
			RcsApiManager
					.getPublicAccountApi()
					.getUserSubscribePublicList(callback);
		} catch (ServiceDisconnectedException e) {
			Toast.makeText(
					this,
					this.getResources().getString(
							R.string.rcs_service_is_not_available),
					Toast.LENGTH_SHORT).show();
			e.printStackTrace();
		}
	}
    
    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case android.R.id.home:
			finish();
			return true;
//		case R.id.action_recommend:
//			startActivityForResult(
//					new Intent(this, RecommendPublicAccountActivity.class), 1);
//			return true;
		case R.id.action_search:
			startActivityForResult(
					new Intent(this, SearchAccountActivity.class), 0);
			return true;
		case R.id.action_refresh:
			if (listView != null) {
				listView.setAdapter(null);
				listView.setOnItemClickListener(null);
			}
			getUserSubscribePublicList();
			return true;
		}
		return false;
	}

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.account_list, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    protected void onResume() {
        super.onResume();
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        if (prefs.getBoolean(RcsContactUtils.PREF_FOLLOW_STATE_CHANGED, false)) {
        	getUserSubscribePublicListCache();
            SharedPreferences.Editor editor = prefs.edit();
            editor.putBoolean(RcsContactUtils.PREF_FOLLOW_STATE_CHANGED, false);
            editor.apply();
        }
    }

    private class UpdateAccountPhotoTask extends AsyncTask<Void, Void, Void> {
        List<PublicAccounts> mPublicAccountList;

        AccountListAdapter mAdapter = (AccountListAdapter)listView.getAdapter();

        UpdateAccountPhotoTask(List<PublicAccounts> publicAccountList) {
            mPublicAccountList = publicAccountList;
        }

        @Override
        protected Void doInBackground(Void... params) {
            if (mAdapter == null) {
                return null;
            }
            for (PublicAccounts account : mPublicAccountList) {
                String imageUrl = account.getLogo();
//                getPhoto(imageUrl);
                this.publishProgress(params);
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... progresses) {
            if (mAdapter != null) {
                mAdapter.notifyDataSetChanged();
            }
        }

        @Override
        protected void onPostExecute(Void result) {
            if (mAdapter != null) {
                mAdapter.notifyDataSetChanged();
            }
        }

        private void getPhoto(String imageUrl) {
        	if (mAdapter != null) {
        		Bitmap bitmap = mAdapter.getBitmapViaUrl(imageUrl);
        		if (bitmap == null) {
        			try {
        				URL myFileUrl = new URL(imageUrl);
        				HttpURLConnection httpURLconnection = (HttpURLConnection)myFileUrl.openConnection();
        				httpURLconnection.setRequestMethod("GET");
        				httpURLconnection.setReadTimeout(6 * 1000);
        				InputStream is = null;
        				if (httpURLconnection.getResponseCode() == 200) {
        					is = httpURLconnection.getInputStream();
        					bitmap = BitmapFactory.decodeStream(is);
        					Bitmap roundCornerBitMap = ImageUtils.createBitmapRoundCorner(bitmap,
        							bitmap.getHeight() / 2);
        					if (mAdapter != null) {
        						mAdapter.addPhotoMap(imageUrl, roundCornerBitMap);
        					}
        					is.close();
        					Log.i(TAG, "image download finished." + imageUrl);
        				}
        			} catch (IOException e) {
        				e.printStackTrace();
        			}
				}
			}
        }
    }

}
