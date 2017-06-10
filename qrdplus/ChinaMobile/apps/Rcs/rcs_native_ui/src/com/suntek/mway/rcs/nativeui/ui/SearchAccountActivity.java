/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.List;

import android.app.ActionBar;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.RemoteException;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;
import android.widget.SearchView;
import android.widget.SearchView.OnQueryTextListener;
import android.widget.Toast;

import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.nativeui.RcsApiManager;
import com.suntek.mway.rcs.nativeui.utils.ImageUtils;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.MenuInfo;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccounts;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccountsDetail;
import com.suntek.mway.rcs.client.api.publicaccount.callback.PublicAccountCallback;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;

public class SearchAccountActivity extends Activity {
    public static final String TAG = "SearchAccountActivity";

    private ListView listView;

    private ProgressDialog dlg;

    private OnItemClickListener itemListener;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_search_account);
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayUseLogoEnabled(false);
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setDisplayShowHomeEnabled(false);
        Drawable myDrawable = getResources().getDrawable(
                R.color.public_account_bar_background_color);
        actionBar.setBackgroundDrawable(myDrawable);
        listView = (ListView)findViewById(R.id.list_accounts);
        itemListener = new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                AccountListAdapter adapter = (AccountListAdapter)listView.getAdapter();
                PublicAccounts account = adapter.getItem(position);
                Intent intent = new Intent(SearchAccountActivity.this, PublicDetailActivity.class);
                if (adapter.getBitmapViaUrl(account.getLogo()) != null) {
                    intent.putExtra("photo", adapter.getBitmapViaUrl(account.getLogo()));
                }
                intent.putExtra("id", account.getPaUuid());
                intent.putExtra("name", account.getName());
                startActivityForResult(intent, 0);
            }
        };
    }

    private void searchAccount(final String text) throws ServiceDisconnectedException {
        dlg = ProgressDialog.show(this, getString(R.string.please_wait),
                getString(R.string.searching));
        dlg.setCancelable(true);
        RcsApiManager.getPublicAccountApi().getPublicList(text, 1000, 1, 1,
                new PublicAccountCallback() {

                    @Override
                    public void respGetUserSubscribePublicList(boolean arg0,
                            List<PublicAccounts> arg1) throws RemoteException {
                    }

                    @Override
                    public void respGetPublicList(boolean arg0, final List<PublicAccounts> list)
                            throws RemoteException {
                        try {
                            RcsApiManager.getPublicAccountApi().unregisterCallback(this);
                        } catch (ServiceDisconnectedException e) {
                            e.printStackTrace();
                        }
                        runOnUiThread(new Runnable() {

                            @Override
                            public void run() {
                                dlg.dismiss();
                                if (list != null && list.size() > 0) {
                                    // show data
                                    AccountListAdapter adpter = new AccountListAdapter(
                                            SearchAccountActivity.this, list);
                                    listView.setAdapter(adpter);
                                    listView.setOnItemClickListener(itemListener);
                                    new UpdateAccountPhotoTask(list).execute();
                                } else {
                                    // show null
                                    listView.setAdapter(null);
                                    listView.setOnItemClickListener(null);
                                    Toast.makeText(SearchAccountActivity.this,
                                             R.string.rcs_search_no_data, 0).show();
                                }
                            }
                        });

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
                    public void respSetAcceptStatus(boolean arg0, String arg1)
                            throws RemoteException {

                    }
                });

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
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.search_account, menu);
        MenuItem searchItem = menu.findItem(R.id.action_search);
        SearchView searchView = (SearchView)searchItem.getActionView();
        searchView.setOnQueryTextListener(new OnQueryTextListener() {

            @Override
            public boolean onQueryTextChange(String input) {
                return false;
            }

            @Override
            public boolean onQueryTextSubmit(String input) {
                try {
                    if (!TextUtils.isEmpty(input)) {
                        if (RcsApiManager.getAccountApi().isOnline()) {
                            searchAccount(input);
                        }else {
                            Toast.makeText(SearchAccountActivity.this, R.string.rcs_service_is_not_online, Toast.LENGTH_SHORT).show();
                        }
                    }
                } catch (ServiceDisconnectedException e) {
                    e.printStackTrace();
                    Toast.makeText(SearchAccountActivity.this, R.string.rcs_search_fail, 0).show();
                }
                return true;
            }
        });

        return super.onCreateOptionsMenu(menu);
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
                getPhoto(imageUrl);
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
            Bitmap bitmap = null;
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
