/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.ui.adapter;

import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.MsgContent;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccountsDetail;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicMessage;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.publicaccount.R;
import com.suntek.mway.rcs.publicaccount.RcsApiManager;
import com.suntek.mway.rcs.publicaccount.ui.widget.HistoryMessageView;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader.ImageCallback;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader.LoaderImageTask;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Message;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import java.util.List;

public class ConversationHistoryMsgAdapter extends BaseAdapter {

    private Context mContext;

    private List<MsgContent> mList;

    private AsynImageLoader mAsynImageLoader;

    public static final int TEXT = 0;

    public static final int MEDIA = 1;

    public static final int TOPIC = 2;

    private static final int TYPE_COUNT = 3;

    private PublicAccountsDetail mDetail;

    private BitmapDrawable mPaPhoto;

    public ConversationHistoryMsgAdapter(Context context, List<MsgContent> list) {
        this.mContext = context;
        this.mList = list;
        mAsynImageLoader = new AsynImageLoader();
    }

    @Override
    public int getViewTypeCount() {
        return TYPE_COUNT;
    }

    @Override
    public int getItemViewType(int position) {
        return getMediaType(mList.get(position).getMediaType());
    }

    @Override
    public int getCount() {
        return mList.size();
    }

    @Override
    public Object getItem(int position) {
        return mList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        int type = getItemViewType(position);
        MsgContent msgContent = mList.get(position);
        ViewHolder holder = null;
        if (convertView == null) {
            holder = new ViewHolder();
            convertView = LayoutInflater.from(mContext).inflate(
                    R.layout.conversation_history_msg_item, null);
            holder.historyMsgView = (HistoryMessageView)convertView
                    .findViewById(R.id.history_msg_item);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder)convertView.getTag();
        }
        holder.historyMsgView.setupItem(mAsynImageLoader, type, msgContent, mPaPhoto);
        return convertView;
    }

    private static class ViewHolder {
        private HistoryMessageView historyMsgView;
    }

    private int getMediaType(String mediaType) {
        if (mediaType.equals(PublicMessage.TEXT)) {
            return TEXT;
        } else if (mediaType.equals(PublicMessage.AUDIO) || mediaType.equals(PublicMessage.IMAGE)
                || mediaType.equals(PublicMessage.VEDIO) || mediaType.equals(PublicMessage.LOCATION)) {
            return MEDIA;
        } else if (mediaType.equals(PublicMessage.TOPIC)
                || mediaType.equals(PublicMessage.TOPIC_SINGLE)
                || mediaType.equals(PublicMessage.TOPIC_MORE)) {
            return TOPIC;
        }
        return TEXT;
    }
    
    public void addList(List<MsgContent> list){
        mList.addAll(0, list);
        notifyDataSetChanged();
    }

    private void getHeadIcon(String paUuid) {
        if (mDetail == null) {
            try {
                mDetail = RcsApiManager.getPublicAccountApi()
                        .getPublicDetailCache(paUuid);
            } catch (ServiceDisconnectedException e) {
                e.printStackTrace();
            }
        }
        if (mDetail != null) {
            LoaderImageTask loaderImageTask = new LoaderImageTask(
                    mDetail.getLogoUrl(), false, false, true, true);
            mAsynImageLoader.loadImageAsynByUrl(loaderImageTask, new ImageCallback() {
                @Override
                public void loadImageCallback(Bitmap bitmap) {
                    if (bitmap != null) {
                        mPaPhoto = new BitmapDrawable(mContext.getResources(), bitmap);
                        mPaPhoto.setFilterBitmap(false);
                        mPaPhoto.setDither(false);
                        mHandler.sendEmptyMessage(0);
                    }
                }
            });
        }
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            ConversationHistoryMsgAdapter.this.notifyDataSetChanged();
        }
    };

    public void setPaUuid(String paUuid) {
        getHeadIcon(paUuid);
    }
}
