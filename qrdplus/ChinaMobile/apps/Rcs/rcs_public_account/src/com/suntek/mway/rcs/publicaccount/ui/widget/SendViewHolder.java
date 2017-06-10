/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.ui.widget;

import com.suntek.mway.rcs.client.aidl.plugin.entity.emoticon.EmoticonBO;
import com.suntek.mway.rcs.client.aidl.plugin.entity.emoticon.EmoticonConstant;
import com.suntek.mway.rcs.client.aidl.provider.SuntekMessageData;
import com.suntek.mway.rcs.client.aidl.provider.model.ChatMessage;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicMediaMessage;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicMessage;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicTextMessage;
import com.suntek.mway.rcs.client.api.emoticon.EmoticonApi;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.publicaccount.R;
import com.suntek.mway.rcs.publicaccount.RcsApiManager;
import com.suntek.mway.rcs.publicaccount.data.RcsGeoLocation;
import com.suntek.mway.rcs.publicaccount.ui.PAMessageUtil;
import com.suntek.mway.rcs.publicaccount.ui.PASendMessageUtil;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader;
import com.suntek.mway.rcs.publicaccount.util.CommonUtil;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.text.TextUtils;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.util.HashMap;

public class SendViewHolder {

    private ImageView mHeadView_Send, mImageView_Send, mAudioView_Send, mVideoView_Send,
            mVcradView_Send, mMapView_Send, mResendButton;

    private TextView mTimeView_Send, mTextView_Send, mSendState;

    private AsynImageLoader mAsynImageLoader;

    private Context mContext;

    private HashMap<String, Long> mFileTrasnfer;

    private ChatMessage mChatMessage;

    public SendViewHolder(Context context, View convertView, HashMap<String, Long> fileTrasnfer,
            AsynImageLoader asynImageLoader) {
        this.mContext = context;
        this.mAsynImageLoader = asynImageLoader;
        this.mFileTrasnfer = fileTrasnfer;
        this.mHeadView_Send = (ImageView)convertView.findViewById(R.id.send_head_icon);
        this.mAudioView_Send = (ImageView)convertView.findViewById(R.id.send_audio_content);
        this.mImageView_Send = (ImageView)convertView.findViewById(R.id.send_image_content);
        this.mMapView_Send = (ImageView)convertView.findViewById(R.id.send_map_content);
        this.mTextView_Send = (TextView)convertView.findViewById(R.id.send_text_content);
        this.mTimeView_Send = (TextView)convertView.findViewById(R.id.send_time);
        this.mVcradView_Send = (ImageView)convertView.findViewById(R.id.send_vcrad_content);
        this.mVideoView_Send = (ImageView)convertView.findViewById(R.id.send_video_content);
        this.mResendButton = (ImageView)convertView.findViewById(R.id.resend_button);
        this.mSendState = (TextView)convertView.findViewById(R.id.send_state);
    }

    public void setViewDataAndPhoto(ChatMessage chatMessage, BitmapDrawable bitmapDrawable) {
        mChatMessage = chatMessage;
        mHeadView_Send.setImageDrawable(bitmapDrawable);
        updateSendState();
        showViewData();
    }

    private void showViewData() {
        mTimeView_Send.setText(PAMessageUtil.getTimeStr(mChatMessage.getTime()));
        PublicMessage paMsg = mChatMessage.getPublicMessage();
        int msgType = mChatMessage.getMsgType();
        if (mChatMessage.getFilepath() == null) {
            String filePath = PASendMessageUtil.getPaFilePath(mChatMessage);
            mChatMessage.setFilepath(filePath);
        }
        
        switch (msgType) { 
            case SuntekMessageData.MSG_TYPE_IMAGE:
                try {
                    String thImagePath = RcsApiManager.getMessageApi().getThumbFilepath(
                            mChatMessage);
//                    String imagePath = PAMessageUtil.getFilePath(mChatMessage);
                    String imagePath = PASendMessageUtil.getPaFilePath(mChatMessage);
                    mAudioView_Send.setVisibility(View.GONE);
                    mImageView_Send.setVisibility(View.VISIBLE);
                    mMapView_Send.setVisibility(View.GONE);
                    mTextView_Send.setVisibility(View.GONE);
                    mVcradView_Send.setVisibility(View.GONE);
                    mVideoView_Send.setVisibility(View.GONE);

                    mImageView_Send.setTag(mChatMessage);
                    Bitmap bitmap = null;
                    if (!TextUtils.isEmpty(thImagePath))
                        bitmap = mAsynImageLoader.loadImageAsynByLocalPath(thImagePath);
                    else if (!TextUtils.isEmpty(imagePath))
                        bitmap = mAsynImageLoader.loadImageAsynByLocalPath(imagePath);

                    if (bitmap != null) {
                        @SuppressWarnings("deprecation")
                        Drawable drawable = new BitmapDrawable(bitmap);
                        mImageView_Send.setBackground(drawable);
                    }
                    mImageView_Send.setOnClickListener(mSendClickListener);
                } catch (ServiceDisconnectedException e1) {
                    e1.printStackTrace();
                }
                break;

            case SuntekMessageData.MSG_TYPE_AUDIO:
                mAudioView_Send.setVisibility(View.VISIBLE);
                mImageView_Send.setVisibility(View.GONE);
                mMapView_Send.setVisibility(View.GONE);
                mTextView_Send.setVisibility(View.GONE);
                mVcradView_Send.setVisibility(View.GONE);
                mVideoView_Send.setVisibility(View.GONE);
                mAudioView_Send.setTag(mChatMessage);
                mAudioView_Send.setOnClickListener(mSendClickListener);
                break;

            case SuntekMessageData.MSG_TYPE_VIDEO:
                mAudioView_Send.setVisibility(View.GONE);
                mImageView_Send.setVisibility(View.GONE);
                mMapView_Send.setVisibility(View.GONE);
                mTextView_Send.setVisibility(View.GONE);
                mVcradView_Send.setVisibility(View.GONE);
                mVideoView_Send.setVisibility(View.VISIBLE);

                mVideoView_Send.setTag(mChatMessage);
                mVideoView_Send.setOnClickListener(mSendClickListener);

                try {
                    String thumbFilepath = RcsApiManager.getMessageApi().getThumbFilepath(
                            mChatMessage);
                    Bitmap bitmap = null;
                    if (!TextUtils.isEmpty(thumbFilepath)) {
                        bitmap = mAsynImageLoader.loadImageAsynByLocalPath(thumbFilepath);
                    }
                    if (bitmap != null) {
                        @SuppressWarnings("deprecation")
                        Drawable drawable = new BitmapDrawable(bitmap);
                        mVideoView_Send.setBackground(drawable);
                    }
                } catch (ServiceDisconnectedException e) {
                    e.printStackTrace();
                }
                break;

            case SuntekMessageData.MSG_TYPE_MAP:
                mAudioView_Send.setVisibility(View.GONE);
                mImageView_Send.setVisibility(View.GONE);
                mMapView_Send.setVisibility(View.VISIBLE);
                mTextView_Send.setVisibility(View.GONE);
                mVcradView_Send.setVisibility(View.GONE);
                mVideoView_Send.setVisibility(View.GONE);
                mMapView_Send.setTag(mChatMessage);
                mMapView_Send.setOnClickListener(mSendClickListener);
                break;
            case SuntekMessageData.MSG_TYPE_CONTACT:
                mAudioView_Send.setVisibility(View.GONE);
                mImageView_Send.setVisibility(View.GONE);
                mMapView_Send.setVisibility(View.GONE);
                mTextView_Send.setVisibility(View.GONE);
                mVcradView_Send.setVisibility(View.VISIBLE);
                mVideoView_Send.setVisibility(View.GONE);
                mVcradView_Send.setTag(mChatMessage);
                mVcradView_Send.setOnClickListener(mSendClickListener);
                break;
            case SuntekMessageData.MSG_TYPE_PAID_EMO:
                mImageView_Send.setBackground(null);
                mAudioView_Send.setVisibility(View.GONE);
                mImageView_Send.setVisibility(View.VISIBLE);
                mMapView_Send.setVisibility(View.GONE);
                mTextView_Send.setVisibility(View.GONE);
                mVcradView_Send.setVisibility(View.GONE);
                mVideoView_Send.setVisibility(View.GONE);
                mImageView_Send.setTag(mChatMessage);
                mImageView_Send.setOnClickListener(mSendClickListener);
//                String body = mChatMessage.getData() + "," + mChatMessage.getFilename();
                RcsEmojiStoreUtil.getInstance().loadImageAsynById(mImageView_Send,
                        mChatMessage.getData(), RcsEmojiStoreUtil.EMO_STATIC_FILE);
                break;
            default:
            case SuntekMessageData.MSG_TYPE_TEXT:
                mAudioView_Send.setVisibility(View.GONE);
                mImageView_Send.setVisibility(View.GONE);
                mMapView_Send.setVisibility(View.GONE);
                mTextView_Send.setVisibility(View.VISIBLE);
                mVcradView_Send.setVisibility(View.GONE);
                mVideoView_Send.setVisibility(View.GONE);

                mTextView_Send.setTag(mChatMessage);
//                mTextView_Send.setOnClickListener(mSendClickListener);
                mTextView_Send.setText(mChatMessage.getData());
//                PAMessageUtil.setHttpText(mTextView_Send, mChatMessage.getData());
                break;
        }
    }

    private OnClickListener mSendClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            ChatMessage chatMessage = (ChatMessage)v.getTag();
            String contentType = PAMessageUtil.getMessageContentType(chatMessage);
            // String filePath = PAMessageUtil.getFilePath(chatMessage);
            String filePath = PASendMessageUtil.getPaFilePath(chatMessage);
            if (!CommonUtil.isFileExists(filePath)) {
                Toast.makeText(mContext, R.string.file_not_exist, Toast.LENGTH_SHORT).show();
                return;
            }
            Intent intent = new Intent(Intent.ACTION_VIEW);
            intent.setDataAndType(Uri.fromFile(new File(filePath)), contentType.toLowerCase());
            switch (chatMessage.getMsgType()) {
                case SuntekMessageData.MSG_TYPE_IMAGE:
                    if (TextUtils.isEmpty(filePath))
                        return;
                    if (!chatMessage.getData().endsWith("gif")
                            && !chatMessage.getData().endsWith("GIF")) {
                        mContext.startActivity(intent);
                    } else {
                        intent.setAction("com.android.gallery3d.VIEW_GIF");
                        mContext.startActivity(intent);
                    }
                    break;

                case SuntekMessageData.MSG_TYPE_AUDIO:
                    // intent.setDataAndType(Uri.parse("file://" + filepath),
                    // "audio/*");
                    mContext.startActivity(intent);
                    break;

                case SuntekMessageData.MSG_TYPE_VIDEO:
                    mContext.startActivity(intent);
                    break;

                case SuntekMessageData.MSG_TYPE_MAP:
                    RcsGeoLocation geo = PASendMessageUtil.readPaMapXml(filePath);
                    String geourl = "geo:" + geo.getLat() + "," + geo.getLng();
                    try {
                        Uri uri = Uri.parse(geourl);
                        Intent intent_map = new Intent(Intent.ACTION_VIEW, uri);
                        mContext.startActivity(intent_map);
                    } catch (Exception e) {
                        Toast.makeText(mContext, R.string.toast_install_map, Toast.LENGTH_SHORT)
                                .show();
                    }
                    break;

                case SuntekMessageData.MSG_TYPE_CONTACT:
                    intent.putExtra("VIEW_VCARD_FROM_MMS", true);
                    mContext.startActivity(intent);
                    break;
                case SuntekMessageData.MSG_TYPE_PAID_EMO:
                    try {
                        EmoticonApi emoticonApi = RcsApiManager.getEmoticonApi();
                        EmoticonBO bean = emoticonApi.getEmoticon(chatMessage.getData());
                        byte[] data = emoticonApi.decrypt2Bytes(bean.getEmoticonId(),
                                EmoticonConstant.EMO_DYNAMIC_FILE);
                        CommonUtil.openPopupWindow(mContext, v, data);
                    } catch (ServiceDisconnectedException e) {
                        e.printStackTrace();
                    }
                    break;
                default:
                case SuntekMessageData.MSG_TYPE_TEXT:
                    break;
            }
        }
    };

    private void updateSendState() {
        int sendState = mChatMessage.getMsgState();
        switch (sendState) {
            case SuntekMessageData.MSG_STATE_SEND_FAIL:
                mResendButton.setVisibility(View.VISIBLE);
                mResendButton.setTag(mChatMessage);
                mResendButton.setOnClickListener(new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        ChatMessage chatMessage = (ChatMessage)v.getTag();
                        if (chatMessage != null) {
                            try {
                                RcsApiManager.getMessageApi().retransmitMessageById(
                                        String.valueOf(chatMessage.getId()));
                            } catch (ServiceDisconnectedException e) {
                                e.printStackTrace();
                            }
                        }
                    }
                });
                mSendState.setText(R.string.message_send_fail);
                break;
            case SuntekMessageData.MSG_STATE_SEND_REC:
                mResendButton.setVisibility(View.GONE);
                mSendState.setText(R.string.message_received);
                break;
            case SuntekMessageData.MSG_STATE_SENDED:
                mResendButton.setVisibility(View.GONE);
                mSendState.setText(R.string.message_adapter_has_send);
                break;
            case SuntekMessageData.MSG_STATE_SEND_ING:
                mResendButton.setVisibility(View.GONE);
                String messageId = "-1";
                if (mChatMessage != null)
                    messageId = mChatMessage.getMessageId();

                mSendState.setText(R.string.message_adapte_sening);
                if (mChatMessage.getMsgType() != SuntekMessageData.MSG_TYPE_TEXT
                        && mFileTrasnfer != null) {
                    Long percent = mFileTrasnfer.get(messageId);
                    if (percent != null) {
                        mSendState
                                .setText(String.format(
                                        mContext.getString(R.string.message_adapte_sening_percent),
                                        percent) + " %");
                    }
                }
                break;
            default:
                mResendButton.setVisibility(View.GONE);
                mSendState.setText(sendState + "");
                break;
        }
    }

    private boolean isMediaType(int type) {
        return (type == SuntekMessageData.MSG_TYPE_IMAGE)
                || (type == SuntekMessageData.MSG_TYPE_AUDIO)
                || (type == SuntekMessageData.MSG_TYPE_VIDEO);
    }
}
