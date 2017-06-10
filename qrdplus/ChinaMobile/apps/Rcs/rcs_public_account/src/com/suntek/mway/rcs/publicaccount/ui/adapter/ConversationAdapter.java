/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.ui.adapter;

import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccountsDetail;
import com.suntek.mway.rcs.client.aidl.provider.SuntekMessageData;
import com.suntek.mway.rcs.client.aidl.provider.model.ChatMessage;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicTopicMessage.PublicTopicContent;
import com.suntek.mway.rcs.publicaccount.R;
import com.suntek.mway.rcs.publicaccount.ui.PAConversationActivity;
import com.suntek.mway.rcs.publicaccount.ui.PAMessageUtil;
import com.suntek.mway.rcs.publicaccount.ui.PASendMessageUtil;
import com.suntek.mway.rcs.publicaccount.ui.widget.ReceiveViewHolder;
import com.suntek.mway.rcs.publicaccount.ui.widget.SendViewHolder;
import com.suntek.mway.rcs.publicaccount.ui.widget.TopicViewHolder;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader.ImageCallback;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader.LoaderImageTask;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ConversationAdapter extends BaseAdapter {

    public static final int VIEW_TYPE_SEND = 0;

    public static final int VIEW_TYPE_RECEIVE = 1;

    public static final int VIEW_TYPE_RECEIVE_TOPIC = 2;

    private Context mContext;

    private AsynImageLoader mAsynImageLoader;

    private BitmapDrawable mMyPhoto, mPaPhoto;

    private PublicAccountsDetail mPublicAccountsDetail;

    private List<ChatMessage> mSelectedList = new ArrayList<ChatMessage>();

    @SuppressLint("HandlerLeak")
    private Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
        }
    };

    public ConversationAdapter(Context context, AsynImageLoader imageLoader) {
        this.mContext = context;
        this.mAsynImageLoader = imageLoader;
        initMyHeadPhoto();
        initPaPhotoHead();
    }

    private ArrayList<ChatMessage> chatList = new ArrayList<ChatMessage>();

    private HashMap<String, Long> mFileTrasnfer = new HashMap<String, Long>();

    private Map<String, List<PublicTopicContent>> mMultiChecks = new HashMap<String, List<PublicTopicContent>>();

    private Bitmap mPaDefaultBitmap;

    private Bitmap mMyDefaultBitmap;

    private Bitmap mMyBitmap;

    protected Bitmap mPaBitmap;

    public void addChatMessageList(List<ChatMessage> list) {
//        chatList.clear();
        chatList.addAll(0, list);
        this.notifyDataSetChanged();
    }

    public synchronized int addChatMessage(ChatMessage chatMsg) {
        chatList.add(chatMsg);
        return chatMsg.getId();
    }

    public void setPublicAccountDetail(PublicAccountsDetail publicAccountsDetail) {
        this.mPublicAccountsDetail = publicAccountsDetail;
        initPaPhotoHead();
    }

    private void initMyHeadPhoto() {
        mMyDefaultBitmap = BitmapFactory.decodeResource(mContext.getResources(),
                R.drawable.contact_default_photo);
        mMyPhoto = new BitmapDrawable(mContext.getResources(), mMyDefaultBitmap);
        mMyPhoto.setFilterBitmap(false);
        mMyPhoto.setDither(false);
        String rawContactId = PAMessageUtil.getMyRcsRawContactId(mContext);
        mAsynImageLoader.loadBitmapByRawContactId(mContext, rawContactId, new ImageCallback() {
            @Override
            public void loadImageCallback(Bitmap bitmap) {
                mMyBitmap = bitmap;
                if (bitmap != null) {
                    mMyPhoto = new BitmapDrawable(mContext.getResources(), bitmap);
                    mMyPhoto.setFilterBitmap(false);
                    mMyPhoto.setDither(false);
                    handler.post(new Runnable() {
                        @Override
                        public void run() {
                            ConversationAdapter.this.notifyDataSetChanged();
                        }
                    });
                }
            }
        });
    }

    private void initPaPhotoHead() {
        mPaDefaultBitmap = BitmapFactory.decodeResource(mContext.getResources(),
                R.drawable.contact_default_photo);
        if (mPaDefaultBitmap != null) {
            mPaPhoto = new BitmapDrawable(mContext.getResources(), mPaDefaultBitmap);
            mPaPhoto.setFilterBitmap(false);
            mPaPhoto.setDither(false);
        }
        if (mPublicAccountsDetail != null) {
            LoaderImageTask loaderImageTask = new LoaderImageTask(
                    mPublicAccountsDetail.getLogoUrl(), false, false, true, false);
            mAsynImageLoader.loadImageAsynByUrl(loaderImageTask, new ImageCallback() {
                @Override
                public void loadImageCallback(Bitmap bitmap) {
                    mPaBitmap = bitmap;
                    if (bitmap != null) {
                        mPaPhoto = new BitmapDrawable(mContext.getResources(), bitmap);
                        mPaPhoto.setFilterBitmap(false);
                        mPaPhoto.setDither(false);
                        handler.post(new Runnable() {
                            @Override
                            public void run() {
                                ConversationAdapter.this.notifyDataSetChanged();
                            }
                        });
                    }
                }
            });
        }
    }

    public List<ChatMessage> getChatMessageList() {
        return chatList;
    }

    public HashMap<String, Long> getHashMap() {
        return mFileTrasnfer;
    }

    @Override
    public int getViewTypeCount() {
        return 3;
    }

    @Override
    public int getItemViewType(int position) {
        ChatMessage cMsg = chatList.get(position);
        int sendType = cMsg.getSendReceive();
        if (sendType == SuntekMessageData.MSG_SEND) {
            return VIEW_TYPE_SEND;
        } else {
            if (SuntekMessageData.MSG_TYPE_PUBLIC_TOPIC == cMsg.getMsgType()) {
                return VIEW_TYPE_RECEIVE_TOPIC;
            } else {
                return VIEW_TYPE_RECEIVE;
            }
        }
    }

    @Override
    public int getCount() {
        return chatList.size();
    }

    @Override
    public ChatMessage getItem(int position) {
        return chatList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return 0;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        int viewType = getItemViewType(position);
        ChatMessage chatMessage = getItem(position);
        if (viewType == VIEW_TYPE_SEND) {
            SendViewHolder sendViewHolder = null;
            if (convertView == null) {
                convertView = LayoutInflater.from(mContext).inflate(
                        R.layout.conversation_item_send_view, null);
                sendViewHolder = new SendViewHolder(mContext, convertView, mFileTrasnfer,
                        mAsynImageLoader);
                convertView.setTag(sendViewHolder);
            } else {
                sendViewHolder = (SendViewHolder)convertView.getTag();
            }
            sendViewHolder.setViewDataAndPhoto(chatMessage, mMyPhoto);
        } else if (viewType == VIEW_TYPE_RECEIVE) {
            ReceiveViewHolder receiveViewHolder = null;
            if (convertView == null) {
                convertView = LayoutInflater.from(mContext).inflate(
                        R.layout.conversation_item_receive_view, null);
                receiveViewHolder = new ReceiveViewHolder(mContext, convertView, mFileTrasnfer,
                        mAsynImageLoader);
                convertView.setTag(receiveViewHolder);
            } else {
                receiveViewHolder = (ReceiveViewHolder)convertView.getTag();
            }
            receiveViewHolder.setViewDataAndPhoto(chatMessage, mPaPhoto);
        } else if (viewType == VIEW_TYPE_RECEIVE_TOPIC) {
            TopicViewHolder topicViewHolder = null;
            if (convertView == null) {
                convertView = LayoutInflater.from(mContext).inflate(
                        R.layout.conversation_item_topic_view, null);
                topicViewHolder = new TopicViewHolder(mContext, convertView, mAsynImageLoader);
                convertView.setTag(topicViewHolder);
            } else {
                topicViewHolder = (TopicViewHolder)convertView.getTag();
            }

            topicViewHolder.showViewData(chatMessage, mPaPhoto);
        }
        updateCheckedState(position, convertView, parent);

        return convertView;
    }

    private void updateCheckedState(int position, View v, ViewGroup parent) {
        ListView lv = (ListView)parent;
        if (lv.isItemChecked(position + 1)) {
            v.setBackgroundResource(R.color.gray5);
        } else {
            v.setBackgroundResource(R.drawable.rcs_public_account_btn_bg);
        }
    }

    private boolean isItemChecked(int position, ListView lv){
        return lv.isItemChecked(position + 1);
    }

    public List<ChatMessage> getSelectedList() {
        return mSelectedList;
    }

    public Map<String, List<PublicTopicContent>> getMultiChecks() {
        return mMultiChecks;
    }

    public void removeMsgs() {
        chatList.removeAll(mSelectedList);
        notifyDataSetChanged();
    }

    public void itemCheck(int position, boolean isCheck) {
        ChatMessage chatMsg = chatList.get(position);
        if (isCheck) {
            if (!mSelectedList.contains(chatMsg)) {
                mSelectedList.add(chatMsg);
            }
        } else {
            if (mSelectedList.contains(chatMsg)) {
                mSelectedList.remove(chatMsg);
            }
        }
    }
    
    public void clearMultiData() {
        mSelectedList.clear();
        mMultiChecks.clear();
    }

    public int getTheFrontChatId(int msgId) {
        int size = chatList.size();
        int chatId = 0;
        for (int i = size - 1; i >= 0; i--) {
            if (msgId == chatList.get(i).getId()) {
                if (i >= 1) {
                    chatId = chatList.get(i - 1).getId();
                }
                return chatId;
            }
        }
        return chatId;
    }

    public void removeMsg(ChatMessage chatMessage) {
        chatList.remove(chatMessage);
        // String filePath = chatMessage.getFilepath();
/*        String filePath = PASendMessageUtil.getPaFilePath(chatMessage);
        boolean delFile = true;
        if (!TextUtils.isEmpty(filePath)) {
            File file = new File(filePath);
            if (file != null && file.exists()) {
                for (ChatMessage cMsg : chatList) {
                    if (cMsg.getFilepath() == chatMessage.getFilepath()) {
                        delFile = false;
                        break;
                    }
                }
                if (delFile) {
                    file.delete();
                }
            }
        }*/
    }
}
