/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.ui;

import java.util.ArrayList;
import java.util.Arrays;

import com.suntek.mway.rcs.client.aidl.provider.model.ChatMessage;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicTopicMessage.PublicTopicContent;
import com.suntek.mway.rcs.publicaccount.R;

import android.annotation.SuppressLint;
import android.app.ActionBar;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.Menu;
import android.view.MenuItem;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.LinearLayout;
import android.widget.Toast;

public class WebViewActivity extends Activity {

    private static final int MENU_FORWARD_MSG = 0;
    private ChatMessage mChatMessage;
    private PublicTopicContent mTopicContent;

	public static void start(Context context, String title, String url) {
        Intent intent = new Intent(context, WebViewActivity.class);
        intent.putExtra("title", title);
        intent.putExtra("url", url);
        context.startActivity(intent);
    }

	public static void start(Context context, String title, String url, ChatMessage chatMessage, PublicTopicContent topicContent) {
		Intent intent = new Intent(context, WebViewActivity.class);
        intent.putExtra("title", title);
        intent.putExtra("url", url);
        intent.putExtra("chatMessage", chatMessage);
        intent.putExtra("topicContent", topicContent);
        context.startActivity(intent);
	}

    @SuppressLint("SetJavaScriptEnabled")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String url = getIntent().getStringExtra("url");
        getTopicIntentExtra();
        if (TextUtils.isEmpty(url)) {
            Toast.makeText(WebViewActivity.this, "url null", Toast.LENGTH_LONG).show();
            WebViewActivity.this.finish();
            return;
        }

        WebView web = getWebView();
        setContentView(web);

        initActionBar();

        web.getSettings().setJavaScriptEnabled(true);
        web.setWebViewClient(new WebViewClient() {
            public boolean shouldOverrideUrlLoading(WebView view, String url) {
                view.loadUrl(url);
                return true;
            }
        });
        web.loadUrl(url);
    }

	private void getTopicIntentExtra() {
		if (getIntent().hasExtra("chatMessage")) {
			mChatMessage = getIntent().getParcelableExtra("chatMessage");
		}
		if (getIntent().hasExtra("topicContent")) {
			mTopicContent = getIntent().getParcelableExtra("topicContent");
		}
	}

	private void initActionBar() {
        ActionBar mActionBar = getActionBar();
        mActionBar.setDisplayHomeAsUpEnabled(true);
        mActionBar.setTitle(R.string.webview_title);
        String title = getIntent().getStringExtra("title");
        if (!TextUtils.isEmpty(title))
            mActionBar.setTitle(title);
    }

    private WebView getWebView() {
        LinearLayout.LayoutParams param = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);
        WebView webView = new WebView(WebViewActivity.this);
        webView.setLayoutParams(param);
        return webView;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                return true;
            case MENU_FORWARD_MSG:
            	PASendMessageUtil.rcsForwardMessage(this, mChatMessage);
            	return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
    	super.onPrepareOptionsMenu(menu);
    	menu.clear();
    	if (mChatMessage != null) {
    		menu.add(0, MENU_FORWARD_MSG, Menu.NONE, R.string.menu_forward_msg).setShowAsAction(
                    MenuItem.SHOW_AS_ACTION_NEVER);
		}
    	return true;
    }
    
    @Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		switch (requestCode) {
		case PAConversationActivity.REQUEST_SELECT_GROUP:
		case PAConversationActivity.REQUEST_CODE_RCS_PICK:
			if (data != null) {
				ArrayList<String> numbers = data
						.getStringArrayListExtra("recipients");
				PASendMessageUtil.forwardRcsMessage(this, mChatMessage, mTopicContent, numbers);
			}
			break;
		case PAConversationActivity.REQUEST_SELECT_CONV:
			if (data != null) {
				PASendMessageUtil.forwardConversation(this, data, mChatMessage);
			}
			break;
		case PAConversationActivity.REQUEST_SELECT_PUBLIC_ACCOUNT:
			if (data != null) {
				String pcId = data.getStringExtra("selectPublicId");
				PASendMessageUtil.forwardPaMessage(this, mChatMessage, mTopicContent, Arrays.asList(pcId));
			}
			break;
		}
	}
}
