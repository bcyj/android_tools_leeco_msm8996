/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import com.suntek.mway.rcs.client.aidl.plugin.entity.profile.QRCardImg;
import com.suntek.mway.rcs.client.aidl.plugin.entity.profile.QRCardInfo;
import com.suntek.mway.rcs.client.api.profile.callback.QRImgListener;
import com.suntek.mway.rcs.client.api.impl.callback.ConferenceCallback;
import com.suntek.mway.rcs.client.aidl.plugin.entity.profile.Profile;
import com.suntek.mway.rcs.client.aidl.plugin.entity.profile.Avatar;
import com.suntek.mway.rcs.client.aidl.constant.BroadcastConstants;
import com.suntek.mway.rcs.client.aidl.provider.model.GroupChatModel;
import com.suntek.mway.rcs.client.aidl.provider.model.GroupChatUser;
import com.suntek.mway.rcs.client.api.autoconfig.RcsAccountApi;
import com.suntek.mway.rcs.client.api.im.impl.MessageApi;
import com.suntek.mway.rcs.client.api.impl.groupchat.ConfApi;
import com.suntek.mway.rcs.client.api.profile.callback.ProfileListener;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.client.api.util.log.LogHelper;
import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.nativeui.RcsApiManager;
import com.suntek.mway.rcs.nativeui.model.ConferenceEvent;
import com.suntek.mway.rcs.nativeui.model.OnAliasUpdateEvent;
import com.suntek.mway.rcs.nativeui.model.OnMemberQuitEvent;
import com.suntek.mway.rcs.nativeui.model.OnReferErrorEvent;
import com.suntek.mway.rcs.nativeui.model.OnRemarkChangeEvent;
import com.suntek.mway.rcs.nativeui.model.OnSubjectChangeEvent;
import com.suntek.mway.rcs.nativeui.service.RcsConferenceListener;
import com.suntek.mway.rcs.nativeui.service.RcsConferenceListener.OnConferenceChangeListener;
import com.suntek.mway.rcs.nativeui.utils.EditTextInputFilter;
import com.suntek.mway.rcs.nativeui.utils.RcsContactUtils;
import com.suntek.mway.rcs.client.api.voip.impl.RichScreenApi;
import com.suntek.mway.rcs.nativeui.utils.GroupMemberPhotoCache;
import com.suntek.mway.rcs.nativeui.utils.GroupMemberPhotoCache.ImageCallback;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.IntentFilter;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnDismissListener;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.provider.ContactsContract.Contacts;
import android.provider.Settings;
import android.text.InputFilter;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.MotionEvent;
import android.view.View.OnTouchListener;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.QuickContactBadge;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

public class RcsGroupChatDetailActivity extends Activity implements OnConferenceChangeListener {

    private static final int ADD_CONTACT_TO_GROUP_CHAT = 1;
    private static final int ADD_GROUP_CHAT_BY_NUMBER = 0;
    private static final int ADD_GROUP_CHAT_BY_CONTACT = 1;
    private static final String INTENT_MULTI_PICK = "com.android.contacts.action.MULTI_PICK";
    private static final String ENHANCE_SCREEN_APK_NAME = "com.cmdm.rcs";
    private ProgressDialog mProgressDialog;

    private TextView mUserCount;
    private TextView mSubjectView;
    private TextView mGroupIdView;
    private TextView mGroupCapasityView;
    private TextView mMyAliasView;
    private TextView mGroupRemarkView;
    private TextView mSMSGroupSends;
    private View mUpdateSubjectView;
    private View mChangeGroupChairmanView;
    private View mEnhanceScreenView;
    private View mSetMyAliasView;
    private View mSetGroupRemarkView;
    private View clearChatHistory;
    private Button mLeaveGroupChatButton;
    private TextView notifySettings;

    private GridView mUserListView;
    private GroupChatModel mGroupChat;
    private String mProfilePhoneNumber = "";
    private RcsGroupChatUserListAdapter mAdapter;

    private MessageApi mMessageApi;
    private ConfApi mConfApi;
    private RcsAccountApi mAccountApi;
    private RichScreenApi mRichScreenApi;

    private Drawable mChairmanDrawable;
    private String mGroupId;

    public static void startByGroupId(Context context, String groupId) {
        Intent intent = new Intent(context, RcsGroupChatDetailActivity.class);
        intent.putExtra("groupId", groupId);
        context.startActivity(intent);
    }

    private BroadcastReceiver apiBindReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            Log.i("test", "onReceiver");
            loadGroupChatDetail();
        }
    };

    private BroadcastReceiver updatePhotoReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            ((RcsGroupChatUserListAdapter)mUserListView.getAdapter()).notifyDataSetChanged();
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.rcs_group_chat_detail_activity);

        registerReceiver(apiBindReceiver, new IntentFilter(RcsApiManager.CONF_API_BIND_ACITON));
        registerReceiver(updatePhotoReceiver, new IntentFilter(RcsContactUtils.NOTIFY_CONTACT_PHOTO_CHANGE));
        Intent intent = getIntent();
        mGroupId = intent.getStringExtra("groupId");

        if (TextUtils.isEmpty(mGroupId)) {
            toast(R.string.no_group_chat);
            finish();
            return;
        }
        mChairmanDrawable = getResources().getDrawable(R.drawable.chairman);
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        mUserCount = (TextView) findViewById(R.id.userCount);
        mSubjectView = (TextView) findViewById(R.id.subject);
        mGroupIdView = (TextView) findViewById(R.id.groupId);
        mGroupCapasityView = (TextView) findViewById(R.id.groupCapasity);
        mMyAliasView = (TextView) findViewById(R.id.my_alias);
        mUpdateSubjectView = findViewById(R.id.updateSubject);
        mSMSGroupSends = (TextView) findViewById(R.id.groupMemberSendSMS);
        mSetMyAliasView = findViewById(R.id.setMyAlias);
        mChangeGroupChairmanView = findViewById(R.id.changeGroupChairman);
        mLeaveGroupChatButton = (Button) findViewById(R.id.leaveGroupChat);
        mSetGroupRemarkView = findViewById(R.id.setGroupRemark);
        mGroupRemarkView = (TextView) findViewById(R.id.group_remark);
        clearChatHistory = findViewById(R.id.clear_chat_history);
        notifySettings = (TextView) findViewById(R.id.notify_settings);
        mUserListView = (GridView) findViewById(R.id.member_list_view);
        mEnhanceScreenView = findViewById(R.id.ehancedScreen);
        if (!isEnhanceScreenInstalled()){
            mEnhanceScreenView.setVisibility(View.GONE);
        }
        RcsConferenceListener.getInstance().addListener(mGroupId, this);
        mAdapter = new RcsGroupChatUserListAdapter(this, mGroupId);
        mUserListView.setAdapter(mAdapter);
        if (RcsApiManager.isMessageApiBind()) {
            loadGroupChatDetail();
        }
    }

    private void loadGroupMemberPhoto(Context context, List<GroupChatUser> groupChatUsers){
        for (GroupChatUser groupChatUser : groupChatUsers) {
            if(groupChatUser!=null){
                final String number = groupChatUser.getNumber();
                long rawContactId = RcsContactUtils.getContactIdByNumber(context,number);
                if(rawContactId != -1){
                    RcsContactUtils.updateContactPhotosByNumber(this,number);
                }else{
                    try{
                        RcsApiManager.getConfApi().refreshMemberHeadPic(mGroupId,number,120,new ConferenceCallback() {

                            @Override
                            public void onRefreshAvatar(Avatar avatar, int resultCode, String resultDesc)
                                throws RemoteException {
                                super.onRefreshAvatar(avatar, resultCode, resultDesc);
                                GroupMemberPhotoCache.getInstance().removeCache(number);
                                runOnUiThread(new Runnable() {

                                    @Override
                                    public void run() {
                                        ((RcsGroupChatUserListAdapter)mUserListView.getAdapter()).notifyDataSetChanged();
                                    }
                                });
                            }
                        });
                    }catch(Exception e){
                        e.printStackTrace();
                    }
                }
            }
        }
    }

    private void loadGroupChatDetail() {
        mMessageApi = RcsApiManager.getMessageApi();
        mConfApi = RcsApiManager.getConfApi();
        mAccountApi = RcsApiManager.getAccountApi();
        mRichScreenApi = RcsApiManager.getRichScreenApi();

        try {
            mGroupChat = mMessageApi.getGroupChatById(mGroupId);
            mProfilePhoneNumber = mAccountApi.getRcsUserProfileInfo().getUserName();
        } catch (ServiceDisconnectedException e) {
            toast(R.string.rcs_service_is_not_available);
            finish();
            LogHelper.printStackTrace(e);
            return;
        }
        if (mGroupChat == null) {
            toast(R.string.no_group_chat);
            finish();
            return;
        }
        if (mGroupChat.getStatus() == GroupChatModel.GROUP_STATUS_DELETED) {
            toast(R.string.group_chat_has_disband);
            finish();
            return;
        }
        mAdapter.isAddBtnShow(true);
        mAdapter.inittUserPhoneNumber();
        boolean isDelBtnShow = isChairman(mGroupChat.getUserList());
        mAdapter.isDelBtnShow(isDelBtnShow);

        mAdapter.bind(mGroupChat.getUserList());
        initialize();
        loadGroupMemberPhoto(this,mGroupChat.getUserList());
    }


    @Override
    protected void onResume() {
        super.onResume();
        // when activity onresume update display view
        if(null == mGroupChat){
            try {
                mGroupChat = RcsApiManager.getMessageApi().getGroupChatById(mGroupId);
            } catch (ServiceDisconnectedException e) {
                toast(R.string.rcs_service_is_not_available);
                finish();
                LogHelper.printStackTrace(e);
                return;
            }
        }
        if(null != mAdapter){
            mAdapter.bind(mGroupChat.getUserList());
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mGroupChat != null) {
            RcsConferenceListener.getInstance().removeListener(String.valueOf(mGroupChat.getId()),
                    this);
        }
        unregisterReceiver(apiBindReceiver);
        unregisterReceiver(updatePhotoReceiver);
    }

    private boolean isEnhanceScreenInstalled() {
        boolean installed = false;
        try {
            ApplicationInfo info = getPackageManager().getApplicationInfo(
                ENHANCE_SCREEN_APK_NAME, PackageManager.GET_PROVIDERS);
            installed = (info != null);
        } catch (NameNotFoundException e) {
        }
        Log.i("RCS_UI", "Is Enhance Screen installed ? " + installed);
        return installed;
    }

    private void initialize() {
        mUserCount.setText(getString(R.string.group_user_count, mGroupChat.getUserList().size()));
        mSubjectView.setText(mGroupChat.getSubject());
        mGroupIdView.setText(String.valueOf(mGroupChat.getId()));
        mGroupCapasityView.setText(String.valueOf(mGroupChat.getMaxCount()));
        mMyAliasView.setText(getMyAlias());
        mGroupRemarkView.setText(mGroupChat.getRemark());

        int msgNotifyType = mGroupChat.getRemindPolicy();
        notifySettings.setText(getRemindPolicyStr(msgNotifyType));

        mUpdateSubjectView.setOnClickListener(onClickListener);
        mSMSGroupSends.setOnClickListener(onClickListener);
        mSetMyAliasView.setOnClickListener(onClickListener);
        mChangeGroupChairmanView.setOnClickListener(onClickListener);
        mEnhanceScreenView.setOnClickListener(onClickListener);
        mLeaveGroupChatButton.setOnClickListener(onClickListener);
        mSetGroupRemarkView.setOnClickListener(onClickListener);
        clearChatHistory.setOnClickListener(onClickListener);
        findViewById(R.id.setGroupChatNotify).setOnClickListener(onClickListener);
    }

    private boolean isChairman(List<GroupChatUser> list) {
        if (TextUtils.isEmpty(mProfilePhoneNumber)) {
            return false;
        }

        // Find 'me' in the group.
        boolean isChairman = false;
        for (GroupChatUser user : list) {
            if (mProfilePhoneNumber.endsWith(user.getNumber())) {
                if (GroupChatUser.ROLE_ADMIN.equals(user.getRole())) {
                    isChairman = true;
                }

                break;
            }
        }

        return isChairman;
    }

    private void toast(int resId) {
        Toast.makeText(this, resId, Toast.LENGTH_LONG).show();
    }

    private void showUpdateSubjectDialog() {
        // 看看自己是不是群主
        if (!isChairman(mAdapter.getUserList())) {
            toast(R.string.only_chairman_can_edit_subject);
            return;
        }

        final EditText input = new EditText(RcsGroupChatDetailActivity.this);
        InputFilter[] filters = { new EditTextInputFilter(30) };
        input.setFilters(filters);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        input.setLayoutParams(lp);

        AlertDialog.Builder builder = new AlertDialog.Builder(RcsGroupChatDetailActivity.this);
        builder.setTitle(R.string.update_subject);
        builder.setView(input);
        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                String newSubject = input.getText().toString();

                if (TextUtils.isEmpty(newSubject)) {
                    return;
                }
                if(poupAirplainMode()){
                    return;
                }
                try {
                    mConfApi.updateGroupSubject(String.valueOf(mGroupChat.getId()), newSubject);
                } catch (ServiceDisconnectedException e) {
                    e.printStackTrace();
                }
            }
        });
        builder.setNegativeButton(android.R.string.cancel, null);

        AlertDialog dialog = builder.create();
        dialog.setCanceledOnTouchOutside(true);
        dialog.show();
    }

    private void showSetGroupRemarkDialog() {
        String remark = mGroupChat.getRemark();
        if (remark == null) {
            remark = "";
        }

        final EditText input = new EditText(RcsGroupChatDetailActivity.this);
        input.setText(remark);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        input.setLayoutParams(lp);
        input.setSelection(remark.length());
        AlertDialog.Builder builder = new AlertDialog.Builder(RcsGroupChatDetailActivity.this);
        builder.setTitle(R.string.group_remark);
        builder.setView(input);
        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                if(poupAirplainMode()){
                    return;
                }
                String newRemark = input.getText().toString();
                if (TextUtils.isEmpty(newRemark))
                    newRemark = "";
                try {
                    mConfApi.modifyGroupMemo(String.valueOf(mGroupChat.getId()), newRemark);
                } catch (ServiceDisconnectedException e) {
                    Log.w("RCS_UI", e);
                }
            }
        });
        builder.setNegativeButton(android.R.string.cancel, null);
        AlertDialog dialog = builder.create();
        dialog.setCanceledOnTouchOutside(true);
        dialog.show();
    }

    private void showSetMyAliasDialog() {
        final EditText input = new EditText(RcsGroupChatDetailActivity.this);
        input.setText(getMyAlias());
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        input.setLayoutParams(lp);
        if (!TextUtils.isEmpty(getMyAlias())) {
            input.setSelection(getMyAlias().length());
        }
        AlertDialog.Builder builder = new AlertDialog.Builder(RcsGroupChatDetailActivity.this);
        builder.setTitle(R.string.set_my_alias);
        builder.setView(input);
        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                if(poupAirplainMode()){
                    return;
                }
                String newAlias = input.getText().toString();
                if (TextUtils.isEmpty(newAlias))
                    newAlias = "";
                try {
                    mConfApi.setMyAlias(String.valueOf(mGroupChat.getId()), newAlias);
                } catch (ServiceDisconnectedException e) {
                    Log.w("RCS_UI", e);
                }
            }
        });
        builder.setNegativeButton(android.R.string.cancel, null);
        AlertDialog dialog = builder.create();
        dialog.setCanceledOnTouchOutside(true);
        dialog.show();
    }

    private void showLeaveGroupChatDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(
                RcsGroupChatDetailActivity.this);
        builder.setTitle(R.string.leave_group_chat);
        builder.setPositiveButton(android.R.string.ok,
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        try {
                            dialog.dismiss();
                            if (mAccountApi.isOnline()) {
                                mProgressDialog = ProgressDialog.show(
                                        RcsGroupChatDetailActivity.this,
                                        getString(R.string.quit_group),
                                        getString(R.string.please_wait));
                                mProgressDialog.setCancelable(false);
                                mProgressDialog
                                        .setOnCancelListener(new OnCancelListener() {
                                            @Override
                                            public void onCancel(
                                                    DialogInterface arg0) {
                                                toast(R.string.quit_group_fail);
                                            }
                                        });
                                if (isChairman(mAdapter.getUserList())) {
                                    Log.d("RCS_UI", "User is chairman, disband group chat.");
                                    mConfApi.disbandGroupChat(String.valueOf(mGroupChat.getId()));
                                } else {
                                    Log.d("RCS_UI", "User is not chairman, quit group chat.");
                                    mConfApi.quitGroupChat(
                                            String.valueOf(mGroupChat.getId()), null);
                                }
                            } else {
                                toast(R.string.rcs_service_is_not_available);
                            }
                        } catch (ServiceDisconnectedException e) {
                            Log.w("RCS_UI", e);
                            mProgressDialog.dismiss();
                            toast(R.string.rcs_service_is_not_available);
                        }
                    }
                });
        builder.setNegativeButton(android.R.string.cancel, null);

        AlertDialog dialog = builder.create();
        dialog.setOnDismissListener(new OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                mLeaveGroupChatButton.setEnabled(true);
                mLeaveGroupChatButton.setTextColor(Color.BLACK);
            }
        });
        dialog.setCanceledOnTouchOutside(true);
        dialog.show();
    }

    private void setEnhancedScreenNumber() {
        if (poupAirplainMode()) {
           return;
        }
        try {
            List<GroupChatUser> groupChatUser = mAdapter.getUserList();
            ArrayList<String> groupChatMemberNumberList = new ArrayList<String>();
            String numbers = "";
            if(!TextUtils.isEmpty(mProfilePhoneNumber)){
                for (GroupChatUser chatUser : groupChatUser) {
                    if(null != chatUser.getNumber() && !mProfilePhoneNumber.endsWith(chatUser.getNumber())){
                        groupChatMemberNumberList.add(chatUser.getNumber());
                    }
                }
            }
            mRichScreenApi.startSiteApk(groupChatMemberNumberList);
        } catch (ServiceDisconnectedException e1) {
            toast(R.string.rcs_service_is_not_available);
            return;
        }
    }

    private void showChangeGroupChairmanDialog() {
        // 看看自己是不是群主
        if (!isChairman(mAdapter.getUserList())) {
            toast(R.string.only_chairman_can_change_chairman);
            return;
        }
        if (poupAirplainMode()) {
           return;
        }

        final List<GroupChatUser> userList = new ArrayList<GroupChatUser>();
        try {
            List<GroupChatUser> canBeAsign = mConfApi.getGroupChatUsersAllowChairman(String
                    .valueOf(mGroupChat.getId()));
            if (canBeAsign != null) {
                userList.addAll(canBeAsign);
            }
        } catch (ServiceDisconnectedException e) {
            Log.w("RCS_UI", e);
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(RcsGroupChatDetailActivity.this);
        builder.setTitle(R.string.change_group_chairman);
        if (userList.size() == 0) {
            builder.setMessage(R.string.no_group_chat_user_can_be_chairman);
        } else {
            String[] items = new String[userList.size()];
            for (int i = 0, size = userList.size(); i < size; i++) {
                final String name = RcsContactUtils.getGroupChatMemberDisplayName(RcsGroupChatDetailActivity.this,
                        mGroupId, userList.get(i).getNumber(), null);
                items[i] = name;
            }
            builder.setItems(items, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    try {
                        mConfApi.assignGroupChairman(String.valueOf(mGroupChat.getId()), userList
                                .get(which).getNumber());
                    } catch (ServiceDisconnectedException e) {
                        e.printStackTrace();
                    }
                }
            });
        }
        builder.setNegativeButton(android.R.string.cancel, null);
        AlertDialog dialog = builder.create();
        dialog.setCanceledOnTouchOutside(true);
        dialog.show();
    }

    private void showAddGroupMemberDialog() {
        AlertDialog.Builder builder = new AlertDialog
                .Builder(RcsGroupChatDetailActivity.this);
        builder.setTitle(R.string.add_groupchat_member);

        String[] items = new String[2];
        items[0] = getResources().getString(
                R.string.add_groupchat_member_by_number);
        items[1] = getResources().getString(
                R.string.add_groupchat_member_by_contacts);
        builder.setItems(items, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                if (which == ADD_GROUP_CHAT_BY_NUMBER) {
                    showAddNumberDialog();
                } else {
                    try {
                        Intent intent = new Intent(INTENT_MULTI_PICK, Contacts.CONTENT_URI);
                        startActivityForResult(intent, ADD_CONTACT_TO_GROUP_CHAT);
                    } catch (ActivityNotFoundException ex) {
                        toast(R.string.contact_app_not_found);
                    }
                }
            }
        });

        builder.setNegativeButton(android.R.string.cancel, null);
        AlertDialog dialog = builder.create();
        dialog.setCanceledOnTouchOutside(true);
        dialog.show();
    }

    private void showAddNumberDialog() {
        final EditText editText = new EditText(this);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT);
        editText.setLayoutParams(lp);
        new AlertDialog.Builder(this)
        .setTitle(R.string.add_groupchat_member_by_number)
        .setView(editText)
        .setPositiveButton(android.R.string.ok,  new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                String input = editText.getText().toString();
                if (TextUtils.isEmpty(input)) {
                    toast(R.string.input_is_empty);
                } else {
                    processInputResult(input);
                }
            }
        }).setNegativeButton(android.R.string.cancel, null)
        .show();

    }

    private void processInputResult(final String number) {
        List<String> referUsers = new ArrayList<String>();
        StringBuffer sb = new StringBuffer();
        GroupChatUser user = mGroupChat.getUserByNumber(number);
        if (user == null) {
            referUsers.add(number);
        } else {
            sb.append(number);
        }
        String toastStr = sb.toString();
        if (!TextUtils.isEmpty(toastStr)) {
            Toast.makeText(this, getString(R.string.number_in_group, toastStr),
                    Toast.LENGTH_LONG).show();
        }
        if(referUsers.size() > 0){
            try {
                RcsApiManager.getConfApi()
                        .inviteToJoinGroupChat(mGroupChat.getId() + "", referUsers);
                toast(R.string.group_chat_has_invite_please_wait);
            }catch (ServiceDisconnectedException e) {
                e.printStackTrace();
            }
        }

    }

    private OnClickListener onClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.updateSubject: {
                    showUpdateSubjectDialog();
                    break;
                }
                case R.id.setMyAlias: {
                    showSetMyAliasDialog();
                    break;
                }
                case R.id.ehancedScreen:{
                    setEnhancedScreenNumber();
                }
                break;
                case R.id.changeGroupChairman: {
                    showChangeGroupChairmanDialog();
                    break;
                }
                case R.id.leaveGroupChat: {
                    showLeaveGroupChatDialog();
                    mLeaveGroupChatButton.setEnabled(false);
                    mLeaveGroupChatButton.setTextColor(Color.GRAY);
                    break;
                }
                case R.id.setGroupRemark: {
                    showSetGroupRemarkDialog();
                    break;
                }
                case R.id.groupMemberSendSMS: {
                    groupSendsSMS();
                    break;
                }
                case R.id.setGroupChatNotify: {
                    showRemindPolicyDialog();
                    break;
                }
                case R.id.clear_chat_history: {
                    clearChatHistory();
                    break;
                }
            }
        }
    };

    private void clearChatHistory() {
        AlertDialog.Builder mBuilder = new Builder(
                RcsGroupChatDetailActivity.this);
        mBuilder.setTitle(R.string.clear_chat_history);
        mBuilder.setMessage(R.string.sure_clear_chat_history);
        mBuilder.setNeutralButton(R.string.confirm,
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        try {
                            boolean isRemoveOK = mMessageApi
                                    .removeMessageByThreadId(mGroupChat
                                            .getThreadId());
                            if (isRemoveOK) {
                                Toast.makeText(RcsGroupChatDetailActivity.this,
                                        R.string.clear_all_succeed,
                                        Toast.LENGTH_SHORT).show();
                            } else {
                                Toast.makeText(RcsGroupChatDetailActivity.this,
                                        R.string.clear_all_fail,
                                        Toast.LENGTH_SHORT).show();
                            }
                        } catch (ServiceDisconnectedException e) {
                            e.printStackTrace();
                            Toast.makeText(RcsGroupChatDetailActivity.this,
                                    R.string.clear_all_fail, Toast.LENGTH_SHORT)
                                    .show();
                        }
                    }
                });
        mBuilder.setNegativeButton(R.string.cancel, null);
        mBuilder.create().show();
    }

    private void showRemindPolicyDialog() {
        int msgNotifyType = mGroupChat.getRemindPolicy();
        String[] items = this.getResources().getStringArray(R.array.group_chat_remind_policy);
        new AlertDialog.Builder(RcsGroupChatDetailActivity.this)
        .setTitle(R.string.set_group_chat_notify)
        .setIcon(null)
        .setSingleChoiceItems(items, msgNotifyType,
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog,
                            int which) {
                        try {
                            mConfApi.updateGroupPolicy(String.valueOf(mGroupChat.getId()), which);
                            mGroupChat.setRemindPolicy(which);
                            notifySettings.setText(getRemindPolicyStr(which));
                        } catch (ServiceDisconnectedException e) {
                            e.printStackTrace();
                        }
                        dialog.dismiss();
                    }
                }).show();
    }

    private String getRemindPolicyStr(int policyType) {
        if (policyType == 0) {
            return getString(R.string.receive_and_remind);
        } else if (policyType == 1) {
            return getString(R.string.receive_not_remind);
        } else if (policyType == 2) {
            return getString(R.string.not_receive);
        } else {
            return getString(R.string.receive_and_remind);
        }
    }

    private void groupSendsSMS() {
        List<GroupChatUser> groupChatUser = mAdapter.getUserList();
        if (!isChairman(groupChatUser)) {
            toast(R.string.only_chairman_can_SMS_group_sends);
            return;
        } else {
            String numbers = "";
            if(!TextUtils.isEmpty(mProfilePhoneNumber)){
                for (GroupChatUser chatUser : groupChatUser) {
                    if(!mProfilePhoneNumber.endsWith(chatUser.getNumber())){
                        numbers = numbers + chatUser.getNumber() + ";";
                    }
                }
            }
            Uri uri = Uri.parse("smsto:" + numbers);
            Intent sendIntent = new Intent(Intent.ACTION_VIEW, uri);
            startActivity(sendIntent);
        }
    }

    private String getMyAlias() {
        List<GroupChatUser> userList = mGroupChat.getUserList();
        for (GroupChatUser user : userList) {
            String number = user.getNumber();

            if (mProfilePhoneNumber != null && mProfilePhoneNumber.endsWith(number)) {
                String alias = user.getAlias();
                if (!TextUtils.isEmpty(alias)) {
                    return alias;
                }
            }
        }

        return "";
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        switch (requestCode) {
            case ADD_CONTACT_TO_GROUP_CHAT:
                if (resultCode == RESULT_OK) {
                    processPickResult(data);

                }
                break;

            default:
                break;
        }
    }

    private void processPickResult(final Intent data) {
        // The EXTRA_PHONE_URIS stores the phone's urls that were selected by
        // user in the
        // multiple phone picker.
        Bundle bundle = data.getExtras().getBundle("result");
        final Set<String> keySet = bundle.keySet();
        List<String> referUsers = new ArrayList<String>();
        StringBuffer sb = new StringBuffer();
        for (String key : keySet) {
            String[] numbers = bundle.getStringArray(key);
            if (numbers != null && numbers.length > 1 && !TextUtils.isEmpty(numbers[1])) {
                numbers[1] = RcsContactUtils.replaceNumberSpace(numbers[1]);
                GroupChatUser user = mGroupChat.getUserByNumber(numbers[1]);
                Log.i("test", "number = " + numbers[1]);
                if (user == null) {
                    referUsers.add(numbers[1]);
                } else {
                    sb.append(numbers[1]);
                    sb.append(",");
                }
            }
        }
        String toastStr = sb.toString();
        if (!TextUtils.isEmpty(toastStr)) {
            Toast.makeText(this, getString(R.string.number_in_group, toastStr), Toast.LENGTH_LONG)
                    .show();
        }
        if(referUsers.size() > 0){
            try {
                RcsApiManager.getConfApi()
                        .inviteToJoinGroupChat(mGroupChat.getId() + "", referUsers);
                Toast.makeText(this,
                        R.string.group_chat_has_invite_please_wait, Toast.LENGTH_SHORT).show();
            }catch (ServiceDisconnectedException e) {
                e.printStackTrace();
            }
        }

    }

    private void kickoutMember(String number) {
        if (mGroupChat == null) {
            toast(R.string.kick_out_member_fail);
            return;
        }
        mProgressDialog = ProgressDialog.show(this, getString(R.string.kick_out_member_ing),
                getString(R.string.please_wait));
        mProgressDialog.setCancelable(true);
        mProgressDialog.setOnCancelListener(new OnCancelListener() {

            @Override
            public void onCancel(DialogInterface arg0) {
                toast(R.string.kick_out_member_fail);
            }
        });
        try {
            RcsApiManager.getConfApi()
                    .kickedOutOfGroupChat(mGroupChat.getId() + "", number);
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }
        if (mProfilePhoneNumber.equals(number)) {
            RcsContactUtils.deleteGroupChat(this, String.valueOf(mGroupChat.getId()));
        }
    }

    private boolean poupAirplainMode(){
        int isAirplainOn = Settings.System.getInt(RcsGroupChatDetailActivity.this.getContentResolver(),
            Settings.System.AIRPLANE_MODE_ON, 0) ;
        if(isAirplainOn == 1){
            toast(R.string.airplain_mode);
            return true;
        }
        return false;
    }

    class RcsGroupChatUserListAdapter extends BaseAdapter {
        List<GroupChatUser> mGroupChatUsers;
        Context mContext;
        LayoutInflater mFactory;
        boolean isAddBtnShow;
        boolean isDelBtnShow;
        int groupChatCount;
        String myPhoneNumber;
        boolean delModel;
        boolean isChaiman;
        String groupId;

        public List<GroupChatUser> getUserList() {
            return mGroupChatUsers;
        }

        public RcsGroupChatUserListAdapter(Context context, String groupId) {
            mContext = context;
            mFactory = LayoutInflater.from(context);
            this.groupId = groupId;

        }

        public void inittUserPhoneNumber() {
            try {
                myPhoneNumber = RcsApiManager.getAccountApi().getRcsUserProfileInfo().getUserName();
            } catch (ServiceDisconnectedException e) {
                e.printStackTrace();
            }
        }

        public void isAddBtnShow(boolean isAddBtnShow) {
            this.isAddBtnShow = isAddBtnShow;
        }

        public void isDelBtnShow(boolean isDelBtnShow) {
            this.isDelBtnShow = isDelBtnShow;
        }

        public void setDelModel(boolean delModel){
            this.delModel = delModel;

        }

        public void bind(List<GroupChatUser> groupChatUsers) {
            if (groupChatUsers == null) {
                return;
            } else {
                isChaiman = isChairman(groupChatUsers);
            }
            mGroupChatUsers = groupChatUsers;
            groupChatCount = groupChatUsers.size();
            notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            if (mGroupChatUsers == null)
                return 0;
            int count = mGroupChatUsers.size();
            if (isDelBtnShow && groupChatCount == 1) {
                isDelBtnShow = false;
            } else if (groupChatCount != 1 && isChaiman) {
                isDelBtnShow = true;
            }
            if (isAddBtnShow) {
                count++;
            }
            if (isDelBtnShow) {
                count++;
            }
            return count;
        }

        @Override
        public GroupChatUser getItem(int position) {
            if (position >= groupChatCount) {
                return null;
            }
            return mGroupChatUsers.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            final ViewHolder holder;
            if (convertView == null) {
                convertView = mFactory.inflate(R.layout.rcs_group_chat_member_item, parent, false);
                holder = new ViewHolder(convertView);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }
            if (position < groupChatCount) {
                holder.mName.setVisibility(View.VISIBLE);
                final GroupChatUser groupChatUser = getItem(position);
                holder.mName.setTag(groupChatUser.getNumber());
                holder.mPhoto.assignContactFromPhone(groupChatUser.getNumber(), false);
                holder.mPhoto.setImageToDefault();
                holder.mPhoto.setOnTouchListener(new OnTouchListener() {

                    @Override
                    public boolean onTouch(View arg0, MotionEvent arg1) {
                        RcsContactUtils.updateContactPhotosByNumber(mContext,groupChatUser.getNumber());
                        return false;
                    }
                });
                final String name = RcsContactUtils.getGroupChatMemberDisplayName(mContext,
                        groupId, groupChatUser.getNumber(), myPhoneNumber);
                holder.mName.setText(name);

                if (GroupChatUser.ROLE_ADMIN.equals(groupChatUser.getRole())) {
                    mChairmanDrawable.setBounds(0, 0, mChairmanDrawable.getMinimumWidth(),
                            mChairmanDrawable.getMinimumHeight());
                    holder.mName.setCompoundDrawables(mChairmanDrawable, null, null, null);
                } else {
                    holder.mName.setCompoundDrawables(null, null, null, null);
                }

                if (delModel && groupChatCount != 1) {
                    holder.mPhoto.setClickable(!delModel);
                    if (myPhoneNumber != null && !myPhoneNumber.endsWith(groupChatUser.getNumber())) {
                        holder.delBtn.setVisibility(View.VISIBLE);
                        holder.mAvatarLayout.setOnClickListener(new OnClickListener() {
                            @Override
                            public void onClick(View arg0) {
                                kickoutMember(groupChatUser.getNumber());
                            }
                        });
                    } else {
                        holder.mPhoto.setClickable(!delModel);
                        holder.delBtn.setVisibility(View.INVISIBLE);
                    }
                } else {
                    holder.mPhoto.setClickable(!delModel);
                    holder.delBtn.setVisibility(View.INVISIBLE);
                }
                if(myPhoneNumber.endsWith(groupChatUser.getNumber())){
                    holder.mPhoto.setClickable(false);
                    holder.mAvatarLayout.setOnClickListener(new OnClickListener() {
                        @Override
                        public void onClick(View arg0) {
                            RcsContactUtils.startMyProfileActivity(mContext);
                        }
                    });
                }
                new Thread() {
                    @Override
                    public void run() {
                        final String number = groupChatUser.getNumber();
                        final Bitmap photo;
                        final String name = RcsContactUtils.getGroupChatMemberDisplayName(mContext,
                                groupId, number, myPhoneNumber);
                        if(myPhoneNumber.endsWith(groupChatUser.getNumber())){
                            //Me
                            photo = RcsContactUtils.getMyProfilePhotoOnData(mContext);
                        }else{
                            photo = RcsContactUtils.getPhotoByNumber(mContext, number);
                        }
                        Handler handler = new Handler(Looper.getMainLooper());
                        handler.post(new Runnable() {
                            public void run() {
                                if (holder.mName.getTag() == null) {
                                    return;
                                }

                                if (number != null && ("T" + number).equals(holder.mName.getTag())) {
                                    if (photo != null) {
                                        holder.mPhoto.setImageBitmap(photo);
                                    } else {
                                        GroupMemberPhotoCache.getInstance().loadGroupMemberPhoto(groupId, number,new ImageCallback(){

                                            @Override
                                            public void loadImageCallback(Bitmap bitmap) {
                                                if (number != null
                                                        && ("I" + number).equals(holder.mPhoto.getTag())) {
                                                    return;
                                                }
                                                if (bitmap!=null) {
                                                    Drawable drawable = new BitmapDrawable(bitmap);
                                                    holder.mPhoto.setImageDrawable(drawable);
                                                } else {
                                                    holder.mPhoto.setImageToDefault();
                                                }
                                            }
                                        });
//                                        holder.mName.setText(name);
                                     }
                                } else {
                                    holder.mPhoto.setImageToDefault();
                                }
                            }
                        });
                    }
                }.start();
            } else if (isAddBtnShow && (position == groupChatCount)) {
                holder.mPhoto.setImageResource(R.drawable.groupchat_add);
                holder.mName.setVisibility(View.INVISIBLE);
                holder.delBtn.setVisibility(View.INVISIBLE);
                holder.mName.setTag(null);
                holder.mName.setCompoundDrawables(null, null, null, null);
                holder.mPhoto.setOnClickListener(new OnClickListener() {
                    @Override
                    public void onClick(View arg0) {
                        if (poupAirplainMode()) {
                           return;
                        }
                        showAddGroupMemberDialog();
                    }
                });
            } else if (isDelBtnShow && (position == (groupChatCount + 1))) {
                holder.mPhoto.setImageResource(R.drawable.groupchat_delete);
                holder.mName.setVisibility(View.INVISIBLE);
                holder.delBtn.setVisibility(View.INVISIBLE);
                holder.mName.setTag(null);
                holder.mName.setCompoundDrawables(null, null, null, null);
                holder.mPhoto.setOnClickListener(new OnClickListener() {
                    @Override
                    public void onClick(View arg0) {
                        if (poupAirplainMode()) {
                           return;
                        }
                        delModel = !delModel;
                        RcsGroupChatUserListAdapter.this.notifyDataSetChanged();
                    }
                });
            }
            return convertView;
        }

        class ViewHolder {
            QuickContactBadge mPhoto;
            TextView mName;
            ImageView delBtn;
            RelativeLayout mAvatarLayout;

            public ViewHolder(View convertView) {
                mAvatarLayout = (RelativeLayout) convertView.findViewById(R.id.avatar_layout);
                mPhoto = (QuickContactBadge) convertView.findViewById(R.id.avatar);
                mName = (TextView) convertView.findViewById(R.id.name);
                delBtn = (ImageView) convertView.findViewById(R.id.delBtn);
            }
        }
    }

    private void toastReferError(OnReferErrorEvent event) {
        if (BroadcastConstants.REFER_TYPE_INVITE.equals(event.getReferErrorAction())) {
            toast(R.string.invite_error);
        } else if (BroadcastConstants.REFER_TYPE_SUBJECT.equals(event.getReferErrorAction())) {
            toast(R.string.subject_error);
        } else if (BroadcastConstants.REFER_TYPE_ALIAS.equals(event.getReferErrorAction())) {
            toast(R.string.alias_error);
        } else if (BroadcastConstants.REFER_TYPE_TRANSFER_CHAIRMAN.equals(event
                .getReferErrorAction())) {
            toast(R.string.chairman_error);
        } else if (BroadcastConstants.REFER_TYPE_KICKOUT.equals(event.getReferErrorAction())) {
            toast(R.string.kickout_error);
        } else if (BroadcastConstants.REFER_TYPE_QUIT.equals(event.getReferErrorAction())) {
            toast(R.string.quit_error);
        }
    }

    @Override
    public void onChanged(String groupId, ConferenceEvent event) {
        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
            mProgressDialog = null;
        }
        String actionType = event.getActionType();
        Log.d("NativeUI","actionType= "+actionType);
        if (OnReferErrorEvent.ON_REFER_ERROR_ACTION.equals(actionType)) {
            OnReferErrorEvent onReferError = (OnReferErrorEvent) event;
            toastReferError(onReferError);
            return;
        } else if (BroadcastConstants.ACTION_TYPE_DELETED.equals(actionType)
                || BroadcastConstants.REFER_TYPE_QUIT.equals(actionType)) {
            mLeaveGroupChatButton.setEnabled(true);
            mLeaveGroupChatButton.setTextColor(Color.BLACK);
            if (groupId != null && groupId.equals(event.getGroupId())) {
                toast(R.string.group_chat_deleted);
                finish();
            }
        } else if (BroadcastConstants.ACTION_TYPE_DEPARTED.equals(actionType)) {
            OnMemberQuitEvent onMemeberQuitEvent = (OnMemberQuitEvent)event;
            String memberNumber = onMemeberQuitEvent.getMemberNumber();
            if (groupId != null && groupId.equals(event.getGroupId())
                    && mProfilePhoneNumber.endsWith(memberNumber)) {
                finish();
            }
        }
        try {
            mGroupChat = mMessageApi.getGroupChatById(groupId);
            if(mGroupChat == null){
                toast(R.string.rcs_service_is_not_available);
                finish();
            }
        } catch (ServiceDisconnectedException e) {
            toast(R.string.rcs_service_is_not_available);
            finish();
            LogHelper.printStackTrace(e);
            return;
        }
        if (BroadcastConstants.ACTION_TYPE_UPDATE_SUBJECT.equals(actionType)) {
            OnSubjectChangeEvent onSubjectEvent = (OnSubjectChangeEvent) event;
            mSubjectView.setText(onSubjectEvent.getNewSubject());
            RcsContactUtils.UpdateGroupChatSubject(this,event.getGroupId(),onSubjectEvent.getNewSubject());
        } else if (BroadcastConstants.ACTION_TYPE_UPDATE_ALIAS.equals(actionType)) {
            OnAliasUpdateEvent onAliasEvent = (OnAliasUpdateEvent) event;
            String alias = onAliasEvent.getAlias();
            String number = onAliasEvent.getPhoneNumber();
            if (mGroupChat != null && !TextUtils.isEmpty(number)) {
                if (mAdapter != null && !TextUtils.isEmpty(mAdapter.myPhoneNumber)
                        && mAdapter.myPhoneNumber.endsWith(number)) {
                    mMyAliasView.setText(alias);
                }
                TextView nameTextView = (TextView) mUserListView.findViewWithTag("T" + number);
                if (nameTextView != null) {
                    nameTextView.setText(alias);
                }
            }
            if (TextUtils.isEmpty(mGroupChat.getSubject()) && TextUtils.isEmpty(mGroupChat.getRemark())) {
                RcsContactUtils.UpdateGroupChatSubject(this,event.getGroupId(),alias);
            }
        } else if (BroadcastConstants.ACTION_TYPE_UPDATE_REMARK.equals(actionType)) {
            OnRemarkChangeEvent onRemarkEvent = (OnRemarkChangeEvent) event;
            String remark = onRemarkEvent.getRemark();
            mGroupChat.setRemark(remark);
            mGroupRemarkView.setText(remark);
        } else if (BroadcastConstants.ACTION_TYPE_OVER_MAXCOUNT.equals(actionType)) {
            if (groupId != null && event != null && groupId.equals(event.getGroupId())) {
                toast(R.string.group_is_full);
            }
        } else if (BroadcastConstants.ACTION_TYPE_UPDATE_CHAIRMAN.equals(actionType)) {
            try {
               List<GroupChatUser> userList = RcsApiManager.getConfApi()
                    .getGroupChatUsersByGroupId(groupId);
                if (userList != null) {
                    boolean isDelBtnShow = isChairman(userList);
                    mAdapter.isDelBtnShow(isDelBtnShow);
                    mAdapter.setDelModel(false);
                    mAdapter.bind(userList);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        } else {
            if (mAdapter != null) {
                try {
                    List<GroupChatUser> userList = RcsApiManager.getConfApi()
                            .getGroupChatUsersByGroupId(groupId);
                    if (userList != null) {
                        mAdapter.bind(userList);
                        mUserCount.setText(getString(R.string.group_user_count, userList.size()));
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
