/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.nativeui.RcsApiManager;
import com.suntek.mway.rcs.nativeui.image.ImageLoader;
import com.suntek.mway.rcs.nativeui.utils.RcsContactUtils;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.MenuInfo;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.MenuInfoMode;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccountConstant;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccountReqEntity;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccounts;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccountsDetail;
import com.suntek.mway.rcs.client.api.publicaccount.callback.PublicAccountCallback;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.text.TextUtils;
import android.text.method.ScrollingMovementMethod;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import java.util.Arrays;
import java.util.List;

public class PublicDetailActivity extends Activity implements OnClickListener, OnCheckedChangeListener {
    private ImageView imagePhoto;
    private TextView textName, textCompany, textUuid, textIntro, textConversation, textHistoryMsg,
            textEmptyMsg, textComplain, textStatus;
    private View converDivider;
    private Button btnFollow;
    //private CheckBox checkReceiveMessage;
    private ProgressDialog dlg;
    private CheckBox mCbReceiveMsg;

    private String id;
    private PublicAccountsDetail account;
	private int mLastReceiveStatus;

    public static final String CONVERSATION_ACTIVITY_ACTION = 
            "com.suntek.mway.rcs.publicaccount.ACTION_LUNCHER_RCS_PULBIC_ACCOUT_CONVERSATION";

    private boolean mFromPublicAccount;

    public static final int REQUEST_CODE_START_CONVERSATION_ACTIVITY = 0;

    private PublicAccountCallback callback = new PublicAccountCallback() {

        @Override
        public void respGetUserSubscribePublicList(boolean arg0, List<PublicAccounts> arg1)
                throws RemoteException {
        }

        @Override
        public void respGetPublicMenuInfo(final boolean result, final MenuInfoMode menuInfoMode)
                throws RemoteException {
        }

        @Override
        public void respGetPublicList(boolean arg0, List<PublicAccounts> arg1)
                throws RemoteException {

        }

        @Override
        public void respGetPublicDetail(final boolean result,
                final PublicAccountsDetail accountDetail) throws RemoteException {
            try {
                RcsApiManager.getPublicAccountApi().unregisterCallback(this);
            } catch (ServiceDisconnectedException e) {
                e.printStackTrace();
            }
            runOnUiThread(new Runnable() {

                @Override
                public void run() {
                    // dlg.dismiss();
                    if (result && accountDetail != null) {
                        account = accountDetail;
                        if (TextUtils.equals(account.getPaUuid(), id)) {
                            initViewByAccount();
                        }
                    } else {
                        Toast.makeText(PublicDetailActivity.this,
                                getString(R.string.get_public_detail_fail), Toast.LENGTH_LONG)
                                .show();
                        showFailView();
                    }
                }
            });

        }

        @Override
        public void respComplainPublicAccount(final boolean arg0, PublicAccounts arg1)
                throws RemoteException {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if (arg0) {
                        Toast.makeText(PublicDetailActivity.this, R.string.complain_success,
                                Toast.LENGTH_LONG).show();
                    } else {
                        Toast.makeText(PublicDetailActivity.this, R.string.complain_fail,
                                Toast.LENGTH_LONG).show();
                    }
                }
            });
        }

        @Override
        public void respAddSubscribeAccount(final boolean result, PublicAccounts ac)
                throws RemoteException {
            super.respAddSubscribeAccount(result, ac);
            if (result) {
                account.setSubscribeStatus(1);
            }
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    btnFollow.setClickable(true);
                    if (result) {
                        btnFollow.setText(R.string.unfollow);
                        try {
                            RcsApiManager.getPublicAccountApi().getPublicDetail(id, callback);
                        } catch (ServiceDisconnectedException e) {
                            e.printStackTrace();
                        }
                        textConversation.setVisibility(View.VISIBLE);
                        converDivider.setVisibility(View.VISIBLE);
                        SharedPreferences prefs = PreferenceManager
                                .getDefaultSharedPreferences(PublicDetailActivity.this);
                        SharedPreferences.Editor editor = prefs.edit();
                        editor.putBoolean(RcsContactUtils.PREF_FOLLOW_STATE_CHANGED, true);
                        editor.apply();
                    } else {
                        Toast.makeText(PublicDetailActivity.this, R.string.fail_and_try_again,
                                Toast.LENGTH_SHORT).show();
                        btnFollow.setText(R.string.follow);
                    }
                    dlg.dismiss();
                }
            });

        }

        @Override
        public void respCancelSubscribeAccount(final boolean result, PublicAccounts ac)
                throws RemoteException {
            super.respCancelSubscribeAccount(result, ac);
            if (result) {
                account.setSubscribeStatus(0);
            }
            runOnUiThread(new Runnable() {

                @Override
                public void run() {
                    btnFollow.setClickable(true);
                    if (result) {
                        btnFollow.setText(R.string.follow);
                        try {
                            RcsApiManager.getPublicAccountApi().getPublicDetail(id, callback);
                        } catch (ServiceDisconnectedException e) {
                            e.printStackTrace();
                        }
                        textConversation.setVisibility(View.GONE);
                        converDivider.setVisibility(View.GONE);
                        SharedPreferences prefs = PreferenceManager
                                .getDefaultSharedPreferences(PublicDetailActivity.this);
                        SharedPreferences.Editor editor = prefs.edit();
                        editor.putBoolean(RcsContactUtils.PREF_FOLLOW_STATE_CHANGED, true);
                        editor.apply();
                    } else {
                        Toast.makeText(PublicDetailActivity.this, R.string.fail_and_try_again,
                                Toast.LENGTH_SHORT).show();
                        btnFollow.setText(R.string.unfollow);
                    }
                    dlg.dismiss();
                }
            });
        }

        @Override
		public void respSetAcceptStatus(boolean arg0, String arg1)
				throws RemoteException {
			if (arg0) {
				account.setAcceptstatus(mLastReceiveStatus == PublicAccountConstant.ACCEPT_STATUS_ACCEPT ? PublicAccountConstant.ACCEPT_STATUS_NOT
						: PublicAccountConstant.ACCEPT_STATUS_ACCEPT);
			} else {
				account.setAcceptstatus(mLastReceiveStatus);
			}
		}
    };
    private ImageLoader mImageLoader;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.public_account_detail);
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayUseLogoEnabled(false);
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setDisplayShowHomeEnabled(false);
        Drawable myDrawable = getResources().getDrawable(
                R.color.public_account_bar_background_color);
        actionBar.setBackgroundDrawable(myDrawable);
        id = getIntent().getStringExtra("id");
        if (TextUtils.isEmpty(id)) {
            Toast.makeText(this, "id null", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        
        if (getIntent().hasExtra("publicaccount")) {
            mFromPublicAccount = getIntent().getBooleanExtra("publicaccount", false);
        }

        imagePhoto = (ImageView)findViewById(R.id.img_photo);
        btnFollow = (Button)findViewById(R.id.btn_follow);
        // checkReceiveMessage = (CheckBox)
        // findViewById(R.id.check_receiveMessage);
        // textTitle = (TextView) findViewById(R.id.text_title);
        textName = (TextView)findViewById(R.id.text_name);
        // textDesc = (TextView) findViewById(R.id.text_desc);
        // textRcs = (TextView) findViewById(R.id.text_rcs);
        textCompany = (TextView)findViewById(R.id.text_company);
        textStatus = (TextView)findViewById(R.id.text_status);
        textUuid = (TextView)findViewById(R.id.text_uuid);
        textIntro = (TextView)findViewById(R.id.intro_content);
        converDivider = findViewById(R.id.conver_divider);
        textConversation = (TextView) findViewById(R.id.conversation);
        textHistoryMsg = (TextView) findViewById(R.id.history_message);
        textEmptyMsg = (TextView) findViewById(R.id.message_empty);
        textComplain = (TextView) findViewById(R.id.complain);
        mCbReceiveMsg = (CheckBox)findViewById(R.id.cb_receive_msg);
        mCbReceiveMsg.setOnCheckedChangeListener(this);
        textIntro.setMovementMethod(ScrollingMovementMethod.getInstance());
        try {
            account = RcsApiManager.getPublicAccountApi().getPublicDetailCache(id);
        } catch (ServiceDisconnectedException e) {
            Toast.makeText(this,
                    this.getResources().getString(R.string.rcs_service_is_not_available),
                    Toast.LENGTH_SHORT).show();
            e.printStackTrace();
        }
        if (account != null) {
            initViewByAccount();
        }
        try {
            // dlg.show();
            RcsApiManager.getPublicAccountApi().getPublicDetail(id, callback);
        } catch (ServiceDisconnectedException e) {
            Toast.makeText(this,
                    this.getResources().getString(R.string.rcs_service_is_not_available),
                    Toast.LENGTH_SHORT).show();
            e.printStackTrace();
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                if (mFromPublicAccount) {
                    toPaConversation();
                }
                finish();
                return true;
        }
        return false;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        // dlg.dismiss();
        mImageLoader.destroy();
    }

    private void initViewByAccount() {
        textName.setText(account.getName());
        // textDesc.setText(account.getIntro());
        textCompany.setText(account.getCompany());
        setPublicAccountStatus();
        setReceiveMsgStatus();
        textUuid.setText(account.getNumber());
        textIntro.setText(account.getIntro());
        if (account.getSubscribeStatus() == 1) {
            btnFollow.setText(R.string.unfollow);
            textConversation.setVisibility(View.VISIBLE);
            converDivider.setVisibility(View.VISIBLE);
        } else {
            btnFollow.setText(R.string.follow);
            textConversation.setVisibility(View.GONE);
            converDivider.setVisibility(View.GONE);
        }
        String avatarUrl = getIntent().getStringExtra("avatar_url");
        mImageLoader = new ImageLoader(this);
        mImageLoader.load(imagePhoto, avatarUrl, R.drawable.public_account_default_ic,
                R.drawable.public_account_default_ic);
    }

    private void setPublicAccountStatus() {
        int resId = R.string.rcs_public_account_status_normal;
        switch (account.getActiveStatus()) {
            case PublicAccountConstant.ACCOUNT_STATUS_NORMAL:
                resId = R.string.rcs_public_account_status_normal;
                break;
            case PublicAccountConstant.ACCOUNT_STATUS_PAUSE:
                resId = R.string.rcs_public_account_status_pause;
                break;
            case PublicAccountConstant.ACCOUNT_STATUS_CLOSE:
                resId = R.string.rcs_public_account_status_closed;
                break;
            default:
                break;
        }
        String status = getResources().getString(resId);
        textStatus.setText(this.getResources()
                .getString(R.string.rcs_public_account_status, status));
    }

	private void setReceiveMsgStatus() {
		if (account.getAcceptstatus() == PublicAccountConstant.ACCEPT_STATUS_ACCEPT) {
			mCbReceiveMsg.setChecked(true);
		} else if (account.getAcceptstatus() == PublicAccountConstant.ACCEPT_STATUS_NOT) {
			mCbReceiveMsg.setChecked(false);
		}
	}

    private void showFailView() {
        textName.setText(R.string.load_detail_fail);
        // textDesc.setText(R.string.load_detail_fail);
        // textRcs.setText(R.string.load_detail_fail);
    }

    private void follow() {
        // btnFollow.setText(R.string.please_wait);
        btnFollow.setClickable(false);
        try {
            RcsApiManager.getPublicAccountApi().addSubscribe(id, callback);
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }
    }

    private void unfollow() {
        // btnFollow.setText(R.string.please_wait);
        btnFollow.setClickable(false);
        PublicAccountReqEntity entity = new PublicAccountReqEntity();
        entity.setPaUuid(id);
        try {
            RcsApiManager.getPublicAccountApi().cancelSubscribe(id, callback);
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }
    }

    private void acceptMessage() {
        final Thread thread = new Thread() {

            @Override
            public void run() {
            // TODO receive message
            }
        };

        dlg = ProgressDialog.show(this, getString(R.string.please_wait),
                getString(R.string.loading), true, true, new OnCancelListener() {

                    @Override
                    public void onCancel(DialogInterface dialog) {
                        thread.interrupt();
                    }
                });
        thread.start();
    }

    private void rejectMessage() {
        final Thread thread = new Thread() {

            @Override
            public void run() {
                // TODO reject message
            }
        };

        dlg = ProgressDialog.show(this, getString(R.string.please_wait),
                getString(R.string.loading), true, true, new OnCancelListener() {

                    @Override
                    public void onCancel(DialogInterface dialog) {
                        thread.interrupt();
                    }
                });
        thread.start();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_follow:
                if (account == null) {
                    return;
                }

                dlg = ProgressDialog.show(this, getString(R.string.please_wait), account
                        .getSubscribeStatus() == 1 ? getString(R.string.unfollowing)
                        : getString(R.string.following));
                dlg.setCanceledOnTouchOutside(true);
                dlg.setOnCancelListener(new OnCancelListener() {

                    @Override
                    public void onCancel(DialogInterface arg0) {
                        btnFollow.setClickable(true);
                        Toast.makeText(
                                PublicDetailActivity.this,
                                PublicDetailActivity.this.getResources().getString(
                                    R.string.cancel_option),
                            Toast.LENGTH_SHORT).show();
                }});
                if (account.getSubscribeStatus() == 1) {
                    unfollow();
                } else {
                    follow();
                }
                break;
            case R.id.conversation:
            try {
                String number = id.substring(0, id.indexOf("@"));
                String threadId = RcsApiManager
                        .getMessageApi()
                        .getThreadIdByNumber(Arrays.asList(new String[] { number }));
                Intent intent = new Intent(CONVERSATION_ACTIVITY_ACTION);
                intent.putExtra("PublicAccountUuid", id);
                intent.putExtra("ThreadId", Long.parseLong(threadId));
                startActivityForResult(intent, REQUEST_CODE_START_CONVERSATION_ACTIVITY);
            } catch (ServiceDisconnectedException e) {
                e.printStackTrace();
            }

            break;
        case R.id.history_message:
            Intent it = new Intent("com.suntek.mway.rcs.publicaccount.ACTION_LUNCHER_PAHISTORY_MESSAGE");
            it.putExtra("PublicAccountUuid", id);
            it.putExtra("ThreadId", getThreadIdById(id));
            startActivity(it);
            break;
        case R.id.message_empty:
            emptyMsg();
            break;
        case R.id.complain:
            showComplainDialog();
            break;
        /*case R.id.intro_content:
              textIntro.getText()
              Dialog dialog = new Dialog(this,
                      R.style.AccountIntroDialog);
              dialog.setContentView(R.layout.account_intro_content_detail);
              Window dialogWindow = dialog.getWindow();
              WindowManager.LayoutParams lp = dialogWindow.getAttributes();
              lp.width = PublicAccountUtils.dip2px(this, 300); // with
              lp.height = PublicAccountUtils.dip2px(this, 300); // height
              dialogWindow.setGravity(Gravity.CENTER | Gravity.CENTER);
              dialogWindow.setAttributes(lp);
              dialog.show();
            break;*/
        /*case R.id.text_descLabel:
            if (textDesc.isShown()) {
                textDesc.setVisibility(View.GONE);
            } else {
                textDesc.setVisibility(View.VISIBLE);
            }
            if (textRcs.isShown()) {
                textRcs.setVisibility(View.GONE);
            }
            break;
        case R.id.text_rcsLabel:
            if (textRcs.isShown()) {
                textRcs.setVisibility(View.GONE);
            } else {
                textRcs.setVisibility(View.VISIBLE);
            }
            if (textDesc.isShown()) {
                textDesc.setVisibility(View.GONE);
            }
            break;

        case R.id.text_history_message:
            //startActivity(new Intent(this, PublicPreMessageActivity.class)
                    //.putExtra("accountId", account.getPaUuid()).putExtra(
                            //"accountName", account.getName()));
            break;
        case R.id.text_show_message:
            //startActivity(new Intent(this, MessageActivity.class).putExtra(
                    //"accountId", account.getPaUuid()).putExtra("accountName",
                    //account.getName()));
            break;
        case R.id.layout_receiveMessage:
            if (account == null) {
                return;
            }       
			// if (account.isAccept()) {
            // rejectMessage();
            // } else {
            // acceptMessage();
            // }

            break;

        case R.id.public_account_complain:
            if (account == null) {
                return;
            }
            Intent intent = new Intent(this,ComplainAccountActivity.class);
            intent.putExtra("accountId", id);
            startActivity(intent);
            //showComplainDialog();
            break;

        case R.id.text_del_all_message:
            //try {
                //RcsApiManager.getPaMessageApi()
                        //.deleteMessageByUuid(id);
            //} catch (ServiceDisconnectedException e) {
                //e.printStackTrace();
            //}
            break;
            */
        }

    }

    private void emptyMsg() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setCancelable(true);
        builder.setMessage(R.string.rcs_confirm_empty_msgs);
        builder.setPositiveButton(R.string.rcs_confirm, new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                try {
                    boolean success = RcsApiManager.getMessageApi().removeMessageByThreadId(getThreadIdById(id));
                    Toast.makeText(PublicDetailActivity.this,
                            success ? R.string.rcs_empty_msg_success : R.string.rcs_empty_msg_fail,
                            Toast.LENGTH_SHORT).show();
                    dialog.dismiss();
                } catch (ServiceDisconnectedException e) {
                    e.printStackTrace();
                }
            }
        });
        builder.setNegativeButton(R.string.rcs_cancel, null);
        builder.show();
    }

    private void showComplainDialog() {

        final EditText reason = new EditText(this);
        new AlertDialog.Builder(this)
                .setTitle(R.string.dialog_title_complain)
                .setView(reason)
                .setPositiveButton(R.string.confirm,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface arg0, int arg1) {
                                String reasonStr = reason.getText().toString();
                                if (!TextUtils.isEmpty(reasonStr)) {
                                    try {
                                        RcsApiManager
                                        .getPublicAccountApi()
                                        .complainPublic(id, reasonStr, "", 1, "", callback);
                                    } catch (ServiceDisconnectedException e) {
                                        e.printStackTrace();
                                    }
                                } else {
                                    Toast.makeText(PublicDetailActivity.this,
                                            R.string.reason_is_empty,
                                            Toast.LENGTH_LONG).show();
                                }
                            }
                        }).setNegativeButton(R.string.cancel, null).show();

    }

    private long getThreadIdById(String id) {
        String threadId = null;
        try {
            threadId = RcsApiManager.getMessageApi().getThreadIdByNumber(
                    Arrays.asList(new String[] {
                        id.substring(0, id.indexOf("@"))
                    }));
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
            return -1;
        }
        if (threadId == null) {
            return -1;
        }
        return Long.parseLong(threadId);
    }

    @Override
    public void onBackPressed() {
        if (mFromPublicAccount) {
            toPaConversation();
        }
        super.onBackPressed();
    }

    private void toPaConversation(){
        Intent data = new Intent();
        data.putExtra("publicaccountdetail", account);
        setResult(RESULT_OK, data);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        switch (requestCode) {
            case REQUEST_CODE_START_CONVERSATION_ACTIVITY:
                if (resultCode == RESULT_OK) {
                    boolean unfollow = data.getBooleanExtra("unfollow", false);
                    if (unfollow) {
                        account = data.getParcelableExtra("publicaccountdetail");
                        btnFollow.setText(R.string.follow);
                        textConversation.setVisibility(View.GONE);
                        converDivider.setVisibility(View.GONE);
                    }
                }
                break;

            default:
                break;
        }
    }

	@Override
	public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
		mLastReceiveStatus = account.getAcceptstatus();
		try {
			if (isChecked) {
				RcsApiManager.getPublicAccountApi().setAcceptStatus(id,
						PublicAccountConstant.ACCEPT_STATUS_ACCEPT, callback);
			} else {
				RcsApiManager.getPublicAccountApi().setAcceptStatus(id,
						PublicAccountConstant.ACCEPT_STATUS_NOT, callback);
			}
		} catch (ServiceDisconnectedException e) {
			e.printStackTrace();
		}
	}
}
