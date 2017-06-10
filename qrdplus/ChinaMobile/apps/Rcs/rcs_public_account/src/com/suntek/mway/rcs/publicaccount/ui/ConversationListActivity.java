/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.ui;

import com.suntek.mway.rcs.client.aidl.constant.BroadcastConstants;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccounts;
import com.suntek.mway.rcs.client.aidl.provider.SuntekMessageData;
import com.suntek.mway.rcs.client.aidl.provider.model.MessageSessionModel;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.publicaccount.R;
import com.suntek.mway.rcs.publicaccount.RcsApiManager;
import com.suntek.mway.rcs.publicaccount.receiver.RcsNotifyManager;
import com.suntek.mway.rcs.publicaccount.ui.adapter.ConversationListAdapter;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader;

import android.annotation.SuppressLint;
import android.app.ActionBar;
import android.app.Activity;
import android.app.ListActivity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;
import android.widget.TextView;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

public class ConversationListActivity extends Activity implements OnItemClickListener {

    private ListView mListView;

    private ConversationListAdapter mConversationListAdapter;

    private BroadcastReceiver refreshReceiver;

    private ArrayList<MessageSessionModel> mSessionList = new ArrayList<MessageSessionModel>();

    private AsynImageLoader mImageLoader;

    private View mEmptyView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.conversation_list_view);
        mImageLoader = new AsynImageLoader();
        initActionBar();
        initView();
        listenNotify();
        getDatas();
    }

    private void initActionBar(){
        ActionBar mActionBar = getActionBar();
        mActionBar.setDisplayHomeAsUpEnabled(true);
        mActionBar.setTitle(R.string.public_account_conversation_list);
        mActionBar.setIcon(R.drawable.logo_icon);
    }

    private void initView() {
        this.mListView = (ListView)findViewById(R.id.conv_list);
        this.mEmptyView = findViewById(R.id.empty);
//        this.mListView.setEmptyView(mEmptyView);
        this.mConversationListAdapter = new ConversationListAdapter(this, mImageLoader);
        this.mListView.setAdapter(mConversationListAdapter);
        this.mListView.setOnItemClickListener(this);
    }

    private void listenNotify() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(BroadcastConstants.UI_REFRESH_MESSAGE_LIST);
        refreshReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                getDatas();
            }
        };
        registerReceiver(refreshReceiver, filter);
        registerTimeTickRecevier();
    }

    private void registerTimeTickRecevier() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_TIME_TICK);
        registerReceiver(timeTickReceiver, filter);
    }

    private BroadcastReceiver timeTickReceiver = new BroadcastReceiver() {
        @SuppressLint("SimpleDateFormat")
        @Override
        public void onReceive(Context context, Intent intent) {
            if (mConversationListAdapter != null) {
                String[] day = mConversationListAdapter.getToday();
                Date date = new Date();
                SimpleDateFormat dateFormat = new SimpleDateFormat(
                        context.getString(R.string.message_list_adapter_yyyy_mm_dd_));
                String[] newDay = dateFormat.format(date).split(" ");
                if (!day[0].equals(newDay[0]) || !day[1].equals(newDay[1])) {
                    mConversationListAdapter.setToday(newDay);
                }
            }
        }
    };

    @Override
    protected void onResume() {
        super.onResume();
        RcsNotifyManager.getInstance().cancelAllMessageNotif();
        mEmptyView.setVisibility(View.GONE);
        getDatas();
    };

    private void getDatas(){
        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                new LoadSessionTask().execute();
            }
        }, 500);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    class LoadSessionTask extends AsyncTask<Void, Void, List<MessageSessionModel>> {

        @Override
        protected List<MessageSessionModel> doInBackground(Void... params) {
            List<MessageSessionModel> sessionList = new ArrayList<MessageSessionModel>();
            try {
                List<MessageSessionModel> chatSessionList = RcsApiManager.getMessageApi()
                        .getMessageSessionList(0, 10000);
                if (chatSessionList != null) {
                    sessionList.clear();
                    for (MessageSessionModel messageSessionModel : chatSessionList) {
                        if (messageSessionModel != null
                                && messageSessionModel.getChatType() == SuntekMessageData.CHAT_TYPE_PUBLIC) {
                            sessionList.add(messageSessionModel);
                        }
                    }
                }
            } catch (ServiceDisconnectedException e) {
                e.printStackTrace();
            }
            return sessionList;
        }

        @Override
        protected void onPostExecute(List<MessageSessionModel> result) {
            super.onPostExecute(result);
            if (result != null && result.size() > 0) {
                mSessionList.clear();
                mSessionList.addAll(result);
                mConversationListAdapter.setDatas(mSessionList);
            } else {
                mEmptyView.setVisibility(View.VISIBLE);
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(refreshReceiver);
        unregisterReceiver(timeTickReceiver);
//        mImageLoader.onDestroy();
//        mImageLoader = null;
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        MessageSessionModel sessageSessionModel = mConversationListAdapter.getItem(position);
        if (sessageSessionModel == null)
            return;
        PublicAccounts publicAccounts = sessageSessionModel.getPublicAccountModel();
        if (publicAccounts == null)
            return;
        Intent intent = new Intent(ConversationListActivity.this, PAConversationActivity.class);
        intent.putExtra("PublicAccountUuid", publicAccounts.getPaUuid());
        intent.putExtra("ThreadId", sessageSessionModel.getThreadId());
        startActivity(intent);
    }

}
