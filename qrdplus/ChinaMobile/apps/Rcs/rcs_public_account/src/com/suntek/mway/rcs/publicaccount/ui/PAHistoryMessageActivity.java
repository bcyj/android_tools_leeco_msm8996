/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.ui;

import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.MsgContent;
import com.suntek.mway.rcs.client.api.publicaccount.callback.PublicAccountCallback;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.client.api.util.log.LogHelper;
import com.suntek.mway.rcs.publicaccount.R;
import com.suntek.mway.rcs.publicaccount.RcsApiManager;
import com.suntek.mway.rcs.publicaccount.ui.adapter.ConversationHistoryMsgAdapter;
import com.suntek.mway.rcs.publicaccount.ui.widget.HistoryMsgListView;
import com.suntek.mway.rcs.publicaccount.ui.widget.HistoryMsgListView.OnPARefreshListener;
import com.suntek.mway.rcs.publicaccount.util.CommonUtil;

import android.app.ActionBar;
import android.app.Activity;
import android.os.Bundle;
import android.os.RemoteException;
import android.text.TextUtils;
import android.view.MenuItem;
import android.widget.Toast;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.List;

public class PAHistoryMessageActivity extends Activity implements OnPARefreshListener {

    private ActionBar mActionBar;
    private HistoryMsgListView mListView;
    private ConversationHistoryMsgAdapter mAdapter;
    private String mPublicAccountUuid;
    private long mServiceThreadId;
    private List<MsgContent> mList = new ArrayList<MsgContent>();
    private static final int PAGE_SIZE = 20;
    private int mPage = 1;
    private int mPrePage;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getIntentExtras();
        setContentView(R.layout.conversation_chat_history_view);
        initActionBar();
        initView();
        getHistoryMsgs(mPage = 1);
    }

    private void getIntentExtras() {
        mPublicAccountUuid = getIntent().getStringExtra("PublicAccountUuid");
        if (TextUtils.isEmpty(mPublicAccountUuid)) {
            errorParam();
        }
        mServiceThreadId = getIntent().getLongExtra("ThreadId", -1);
        if (mServiceThreadId == -1) {
            getServiceThreadId();
        }
    }

    private void getHistoryMsgs(final int page) {
        new Thread() {
            public void run() {
                try {
                    if (page == 1) {
                        Thread.sleep(500);
                    }
                    mPrePage = mPage;
                    RcsApiManager.getPublicAccountApi().getPreMessage(mPublicAccountUuid,
                            CommonUtil.getTimeStamp(System.currentTimeMillis()), 1, PAGE_SIZE, page, callback);
                } catch (ServiceDisconnectedException e) {
                    e.printStackTrace();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            };
        }.start();
    }

    private void initActionBar() {
        mActionBar = getActionBar();
        mActionBar.setDisplayHomeAsUpEnabled(true);
        mActionBar.setDisplayUseLogoEnabled(false);
        mActionBar.setIcon(R.drawable.public_account);
        mActionBar.setTitle(R.string.public_account_history_message_view);
    }

    private void initView() {
        mListView = (HistoryMsgListView)findViewById(R.id.list_history_msg);
        mAdapter = new ConversationHistoryMsgAdapter(this, mList);
        mAdapter.setPaUuid(mPublicAccountUuid);
        mListView.setAdapter(mAdapter);
        mListView.setPARefreshListener(this);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }
    private void errorParam() {
        LogHelper.trace("errorParam");
        Toast.makeText(this, R.string.message_uuid_empty, Toast.LENGTH_SHORT).show();
        finish();
    }

    private void getServiceThreadId() {
        try {
            String threadStr = RcsApiManager.getMessageApi().getThreadIdByNumber(
                    Arrays.asList(new String[] {
                        mPublicAccountUuid
                    }));
            if (!TextUtils.isEmpty(threadStr)) {
                mServiceThreadId = Long.parseLong(threadStr);
            }
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }
    }

    private PublicAccountCallback callback = new PublicAccountCallback() {

        @Override
        public void respSetAcceptStatus(boolean arg0, String arg1) throws RemoteException {}

        @Override
        public void respGetPreMessage(boolean arg0, List<MsgContent> arg1) throws RemoteException {
            final List<MsgContent> tmpList = arg1;
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    completeRefresh();
                    if (tmpList != null) {
                        Collections.reverse(tmpList);
                        mAdapter.addList(tmpList);
                        mPage ++;
                    }
                }
            });
            
        }
    };

    @Override
    public void onPARefresh() {
        if (mPrePage != mPage) {
            getHistoryMsgs(mPage);
        }else {
            completeRefresh();
        }
    }

    private void completeRefresh() {
        if (mListView != null) {
            mListView.onPARefreshComplete();
        }
    }
}
