/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.ui.widget;

import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.MediaArticle;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.MsgContent;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccountsDetail;
import com.suntek.mway.rcs.client.aidl.provider.SuntekMessageData;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicMessage;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicTopicMessage.PublicTopicContent;
import com.suntek.mway.rcs.publicaccount.R;
import com.suntek.mway.rcs.publicaccount.RcsApiManager;
import com.suntek.mway.rcs.publicaccount.ui.WebViewActivity;
import com.suntek.mway.rcs.publicaccount.ui.adapter.ConversationHistoryMsgAdapter;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader.ImageCallback;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader.LoaderImageTask;
import com.suntek.mway.rcs.publicaccount.util.CommonUtil;

import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnPreparedListener;
import android.net.Uri;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;

import java.io.IOException;
import java.util.List;

public class HistoryMessageView extends FrameLayout {

    private Context mContext;

    private TextView mTimeTv;

    private FrameLayout mContentLayout;

    private int mType;

    private RelativeLayout mTopLayout;

    private LinearLayout mOtherItemLayout;

    private ImageView mTopImageView;

    private TextView mTopContentText;

    private AsynImageLoader mAsynImageLoader;

    private ImageView mMediaIv;

    private ImageView mTextHeadIcon;

    private ImageView mMediaHeadIcon;

    private BitmapDrawable mHeadIcon;

    public HistoryMessageView(Context context) {
        super(context);
        initView(context);
    }

    public HistoryMessageView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initView(context);
    }

    public HistoryMessageView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        initView(context);
    }

    private void initView(Context context) {
        mContext = context;
        LayoutInflater.from(mContext).inflate(R.layout.conversation_history_msg_view, this);
        mTimeTv = (TextView)findViewById(R.id.push_reveive_time);
        mContentLayout = (FrameLayout)findViewById(R.id.push_content_layout);
    }

    public void setupItem(AsynImageLoader asynImageLoader, int type, MsgContent msgContent, BitmapDrawable photo) {
        this.mAsynImageLoader = asynImageLoader;
        this.mHeadIcon = photo;
        setType(type);
        setTime(CommonUtil.getTimeStamp(msgContent.getCreateTime().replace("+8:00", "+0800")));
        setContent(msgContent);
    }

    private void setContent(MsgContent msgContent) {
        switch (mType) {
            case ConversationHistoryMsgAdapter.TEXT:
                setupText(msgContent);
                break;
            case ConversationHistoryMsgAdapter.MEDIA:
                setupMedia(msgContent);
                break;
            case ConversationHistoryMsgAdapter.TOPIC:
                setupTopic(msgContent);
                break;
            default:
                break;
        }
    }

    private void setupTopic(MsgContent msgContent) {
        mContentLayout.removeAllViews();
        View view = LayoutInflater.from(mContext).inflate(R.layout.conversation_history_msg_topic, null);
        mContentLayout.addView(view);
        mTopLayout = (RelativeLayout)view.findViewById(R.id.top_iamge_view);
        mOtherItemLayout = (LinearLayout)view.findViewById(R.id.other_item_layout);
        mTopImageView = (ImageView)view.findViewById(R.id.top_iamge);
        mTopContentText = (TextView)view.findViewById(R.id.top_content_text);
        showTopicData(msgContent);
    }

    private void showTopicData(MsgContent msgContent) {
        final List<MediaArticle> list = msgContent.getArticleList();
        if (list != null && list.size() > 0) {
            for (int i = 0; i < list.size(); i++) {
                MediaArticle article = list.get(i);
                if (i == 0) {
                    mTopContentText.setText(article.getTitle());
                    LoaderImageTask loaderImageTask = new LoaderImageTask(
                            article.getThumbLink(), false, false, true, true);
                    mAsynImageLoader.loadImageAsynByUrl(loaderImageTask, new ImageCallback() {
                        @Override
                        public void loadImageCallback(Bitmap bitmap) {
                            @SuppressWarnings("deprecation")
                            Drawable drawable = new BitmapDrawable(bitmap);
                            mTopImageView.setBackground(drawable);
                        }
                    });
                    mTopLayout.setTag(article);
                    mTopLayout.setOnClickListener(mOnClickListener);
                } else {
                    if (mOtherItemLayout.getVisibility() == View.GONE) {
                        mOtherItemLayout.setVisibility(View.VISIBLE);
                    }
                    View view = LayoutInflater.from(mContext).inflate(
                            R.layout.public_message_second_item_history, null);
                    TextView textView = (TextView)view.findViewById(R.id.message_title_second);
                    final ImageView imageView = (ImageView)view
                            .findViewById(R.id.message_image_second);
                    textView.setText(article.getTitle());
                    LoaderImageTask loaderImageTask = new LoaderImageTask(
                            article.getThumbLink(), false, true, true, true);
                    mAsynImageLoader.loadImageAsynByUrl(loaderImageTask, new ImageCallback() {
                        @Override
                        public void loadImageCallback(Bitmap bitmap) {
                            if (bitmap != null) {
                                imageView.setImageBitmap(bitmap);
                            }
                        }
                    });
                    view.setTag(article);
                    view.setOnClickListener(mOnClickListener);
                    mOtherItemLayout.addView(view);
                }
            }
        }
    }

    private void setupMedia(MsgContent msgContent) {
        mContentLayout.removeAllViews();
        View view = LayoutInflater.from(mContext).inflate(R.layout.conversation_history_msg_media, null);
        mContentLayout.addView(view);
        mMediaHeadIcon = (ImageView)view.findViewById(R.id.media_head_icon);
        TextView titleTv = (TextView)view.findViewById(R.id.content_media_title);
        mMediaIv = (ImageView)view.findViewById(R.id.content_media_media);
        titleTv.setText(msgContent.getBasic().getTitle());
        mMediaIv.setTag(msgContent);
        showMediaData(msgContent);
    }

    private void showMediaData(final MsgContent msgContent) {
        setHeadIcon(mMediaHeadIcon);
        LoaderImageTask loaderImageTask = new LoaderImageTask(
                msgContent.getBasic().getThumbLink(), false, false, true, true);
        mAsynImageLoader.loadImageAsynByUrl(loaderImageTask, new ImageCallback() {
            @Override
            public void loadImageCallback(Bitmap bitmap) {
                @SuppressWarnings("deprecation")
                Drawable drawable = new BitmapDrawable(bitmap);
                mMediaIv.setBackground(drawable);
            }
        });

        mMediaIv.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                String mediaType = ((MsgContent)v.getTag()).getMediaType();
                if (mediaType.equals(PublicMessage.AUDIO)) {
                    playMedia(msgContent.getBasic().getOriginalLink(), "audio/*");
                }else if (mediaType.equals(PublicMessage.IMAGE)) {
                    showImage(msgContent.getBasic().getOriginalLink());
                }else if (mediaType.equals(PublicMessage.VEDIO)) {
                    playMedia(msgContent.getBasic().getOriginalLink(), "video/*");
                }
            }
        });
    }

    private void playMedia(String url, String type) {
        if (TextUtils.isEmpty(url)) {
            return;
        }
        Intent intent = new Intent(Intent.ACTION_VIEW);
        Uri uri = Uri.parse(url);
        intent.setDataAndType(uri, type);
        mContext.startActivity(intent);
    }

    private void showImage(String url) {
        final AlertDialog dialog = new AlertDialog.Builder(mContext).create();
        View view = LayoutInflater.from(mContext).inflate(
                R.layout.conversation_history_msg_media_img, null);
        dialog.setView(view);
        dialog.show();
        dialog.setCanceledOnTouchOutside(true);

        final ImageView largeIv = (ImageView)view.findViewById(R.id.image_show_large);
        final ProgressBar downloadPb = (ProgressBar)view.findViewById(R.id.image_download_progress);
        view.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                dialog.cancel();
            }
        });
        LoaderImageTask loaderImageTask = new LoaderImageTask(url, false, false, false, false);
        mAsynImageLoader.loadImageAsynByUrl(loaderImageTask, new ImageCallback() {
            @Override
            public void loadImageCallback(Bitmap bitmap) {
                largeIv.setImageBitmap(bitmap);
                downloadPb.setVisibility(View.GONE);
            }
        });
    }

    private void setupText(MsgContent msgContent) {
        mContentLayout.removeAllViews();
        View view = LayoutInflater.from(mContext).inflate(R.layout.conversation_history_msg_text, null);
        mContentLayout.addView(view);
        mTextHeadIcon = (ImageView)view.findViewById(R.id.text_head_icon);
        TextView contentTv = (TextView)view.findViewById(R.id.content_text);
        contentTv.setText(msgContent.getText());
        setHeadIcon(mTextHeadIcon);
    }

    private void setTime(String createTime) {
        mTimeTv.setText(createTime);
    }

    private void setHeadIcon(ImageView iv) {
        if (mHeadIcon != null) {
            iv.setImageDrawable(mHeadIcon);
        }
    }

    private void setType(int type) {
        this.mType = type;
    }

    private OnClickListener mOnClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            MediaArticle article = (MediaArticle)v.getTag();
            WebViewActivity.start(mContext, article.getTitle(), article.getSourceLink());
        }
    };

}
