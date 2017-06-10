/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.ui.widget;

import com.suntek.mway.rcs.client.aidl.provider.model.ChatMessage;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicMessage;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicTopicMessage;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicTopicMessage.PublicTopicContent;
import com.suntek.mway.rcs.publicaccount.R;
import com.suntek.mway.rcs.publicaccount.ui.PAMessageUtil;
import com.suntek.mway.rcs.publicaccount.ui.WebViewActivity;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader.ImageCallback;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader.LoaderImageTask;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

public class TopicViewHolder {

    private TextView mTopicReceiveTime, mTopContentText;

    private ImageView mTopImageView;

    private RelativeLayout mTopLayout;

    private LinearLayout mOtherItemLayout;

    private AsynImageLoader mAsynImageLoader;

    private Context mContext;

    private ChatMessage mChatMessage;

	private ImageView mTopicHeadIcon;

    public TopicViewHolder(Context context, View convertView, AsynImageLoader asynImageLoader) {
        this.mContext = context;
        this.mAsynImageLoader = asynImageLoader;
        this.mTopicHeadIcon = (ImageView)convertView.findViewById(R.id.topic_head_icon);
        this.mTopLayout = (RelativeLayout)convertView.findViewById(R.id.top_iamge_view);
        this.mOtherItemLayout = (LinearLayout)convertView.findViewById(R.id.other_item_layout);
        this.mTopImageView = (ImageView)convertView.findViewById(R.id.top_iamge);
        this.mTopContentText = (TextView)convertView.findViewById(R.id.top_content_text);
        this.mTopicReceiveTime = (TextView)convertView.findViewById(R.id.topic_reveive_time);
    }

    public void showViewData(ChatMessage chatMessage, BitmapDrawable bitmapDrawable) {
    	mChatMessage = chatMessage;
    	mTopicHeadIcon.setImageDrawable(bitmapDrawable);
        mTopicReceiveTime.setText(PAMessageUtil.getTimeStr(chatMessage.getTime()));
        PublicMessage paMsg = chatMessage.getPublicMessage();
        PublicTopicMessage topicMessage = (PublicTopicMessage)paMsg;
        final List<PublicTopicContent> list = topicMessage.getTopics();
        int size = list.size();
        if (list != null && size > 0) {
            for (int i = 0; i < size; i++) {
                PublicTopicContent topicContent = list.get(i);
                if (i == 0) {
                    mOtherItemLayout.removeAllViews();
                    if (mOtherItemLayout.getVisibility() == View.VISIBLE) {
                        mOtherItemLayout.setVisibility(View.GONE);
                    }
                    mTopContentText.setText(topicContent.getTitle());
                    LoaderImageTask loaderImageTask = new LoaderImageTask(
                            topicContent.getThumbLink(), false, false, true, true);
                    mAsynImageLoader.loadImageAsynByUrl(loaderImageTask, new ImageCallback() {
                        @Override
                        public void loadImageCallback(Bitmap bitmap) {
                            if (bitmap != null) {
                                mTopImageView.setImageBitmap(bitmap);
                            }
                        }
                    });
                    mTopLayout.setTag(topicContent);
                    mTopLayout.setOnClickListener(mOnClickListener);
                } else {
                    if (mOtherItemLayout.getVisibility() == View.GONE) {
                        mOtherItemLayout.setVisibility(View.VISIBLE);
                    }
                    View view = LayoutInflater.from(mContext).inflate(
                            R.layout.public_message_second_item, null);
                    TextView textView = (TextView)view.findViewById(R.id.message_title_second);
                    final ImageView imageView = (ImageView)view
                            .findViewById(R.id.message_image_second);
                    textView.setText(topicContent.getTitle());
                    LoaderImageTask loaderImageTask = new LoaderImageTask(
                            topicContent.getThumbLink(), false, true, true, true);
                    mAsynImageLoader.loadImageAsynByUrl(loaderImageTask, new ImageCallback() {
                        @Override
                        public void loadImageCallback(Bitmap bitmap) {
                            if (bitmap != null) {
                                imageView.setImageBitmap(bitmap);
                            }
                        }
                    });
                    view.setTag(topicContent);
                    view.setOnClickListener(mOnClickListener);
                    mOtherItemLayout.addView(view);
                }
            }
        }
    }

    private OnClickListener mOnClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            PublicTopicContent topicContent = (PublicTopicContent)v.getTag();
            WebViewActivity.start(mContext, topicContent.getTitle(), topicContent.getBodyLink(), mChatMessage, topicContent);
        }
    };

}
