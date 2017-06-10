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
import com.suntek.mway.rcs.client.aidl.provider.model.PublicMediaMessage.PublicMediaContent;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicMessage;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicTextMessage;
import com.suntek.mway.rcs.client.api.emoticon.EmoticonApi;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.publicaccount.R;
import com.suntek.mway.rcs.publicaccount.RcsApiManager;
import com.suntek.mway.rcs.publicaccount.data.RcsPropertyNode;
import com.suntek.mway.rcs.publicaccount.data.RcsGeoLocation;
import com.suntek.mway.rcs.publicaccount.data.RcsVcardNode;
import com.suntek.mway.rcs.publicaccount.data.RcsVcardNodeBuilder;
import com.suntek.mway.rcs.publicaccount.http.service.CommonHttpRequest;
import com.suntek.mway.rcs.publicaccount.http.service.PAHttpService.Response;
import com.suntek.mway.rcs.publicaccount.ui.PAMessageUtil;
import com.suntek.mway.rcs.publicaccount.ui.PASendMessageUtil;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader;
import com.suntek.mway.rcs.publicaccount.util.MD5Util;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader.ImageCallback;
import com.suntek.mway.rcs.publicaccount.util.AsynImageLoader.LoaderImageTask;
import com.suntek.mway.rcs.publicaccount.util.CommonUtil;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Handler.Callback;
import android.os.Message;
import android.text.Layout;
import android.text.Spannable;
import android.text.TextUtils;
import android.text.style.ClickableSpan;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileInputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

@SuppressLint("DefaultLocale")
public class ReceiveViewHolder {

    private ImageView mHeadView_Receive, mImageView_Receive, mAudioView_Receive,
            mVideoView_Receive, mMapView_Receive, mVcardView_Receive, mRedownload;

    private TextView mTimeView_Receive, mTextView_Receive, mReceiveState, mDownloadState;

    private AsynImageLoader mAsynImageLoader;

    private Context mContext;

    private HashMap<String, Long> mFileTrasnfer;

    private ChatMessage mChatMessage;

    public ReceiveViewHolder(Context context, View convertView, HashMap<String, Long> fileTrasnfer,
            AsynImageLoader asynImageLoader) {
        this.mContext = context;
        this.mAsynImageLoader = asynImageLoader;
        this.mFileTrasnfer = fileTrasnfer;
        this.mHeadView_Receive = (ImageView)convertView.findViewById(R.id.receive_head_icon);
        this.mAudioView_Receive = (ImageView)convertView.findViewById(R.id.receive_audio_content);
        this.mImageView_Receive = (ImageView)convertView.findViewById(R.id.receive_image_content);
        this.mTextView_Receive = (TextView)convertView.findViewById(R.id.receive_text_content);
        this.mTimeView_Receive = (TextView)convertView.findViewById(R.id.receive_time);
        this.mVideoView_Receive = (ImageView)convertView.findViewById(R.id.receive_video_content);
        this.mMapView_Receive = (ImageView)convertView.findViewById(R.id.receive_map_content);
        this.mVcardView_Receive = (ImageView)convertView.findViewById(R.id.receive_vcrad_content);
        this.mReceiveState = (TextView)convertView.findViewById(R.id.receive_state);
        this.mRedownload = (ImageView)convertView.findViewById(R.id.redownload_button);
        this.mDownloadState = (TextView)convertView.findViewById(R.id.download_state);
//        setTextViewTouch();
    }

    private void setTextViewTouch() {
        mTextView_Receive.setOnTouchListener(new OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                boolean ret = false;
                CharSequence text = ((TextView)v).getText();
                Spannable stext = Spannable.Factory.getInstance().newSpannable(text);
                TextView widget = (TextView)v;
                int action = event.getAction();
                if (action == MotionEvent.ACTION_UP || action == MotionEvent.ACTION_DOWN) {
                    int x = (int)event.getX();
                    int y = (int)event.getY();
                    x -= widget.getTotalPaddingLeft();
                    y -= widget.getTotalPaddingTop();
                    x += widget.getScrollX();
                    y += widget.getScrollY();
                    Layout layout = widget.getLayout();
                    int line = layout.getLineForVertical(y);
                    int off = layout.getOffsetForHorizontal(line, x);
                    ClickableSpan[] link = stext.getSpans(off, off, ClickableSpan.class);
                    if (link.length != 0) {
                        if (action == MotionEvent.ACTION_UP) {
                            link[0].onClick(widget);
                        }
                        ret = true;
                    }
                }
                return ret;
            }
        });
    }

    public void setViewDataAndPhoto(ChatMessage chatMessage, BitmapDrawable bitmapDrawable) {
        mChatMessage = chatMessage;
        mHeadView_Receive.setImageDrawable(bitmapDrawable);
        mTimeView_Receive.setText(PAMessageUtil.getTimeStr(mChatMessage.getTime()));
        updateReceiveState();
        showViewData();
    }

    private void showViewData() {
        int msgType = mChatMessage.getMsgType();
        PublicMessage paMsg = mChatMessage.getPublicMessage();

        if (mChatMessage.getFilepath() == null) {
            if (isMediaType(msgType)) {
                setChatMessageFileArgs(mChatMessage, ((PublicMediaMessage)paMsg).getMedia()
                        .getOriginalLink());
            } else if (msgType == SuntekMessageData.MSG_TYPE_MAP
                    || msgType == SuntekMessageData.MSG_TYPE_CONTACT) {
                setMapFileArgs(((PublicTextMessage)paMsg).getContent(), mChatMessage);
            }
        }

        switch (msgType) {
            case SuntekMessageData.MSG_TYPE_AUDIO: {
                mAudioView_Receive.setVisibility(View.VISIBLE);
                mImageView_Receive.setVisibility(View.GONE);
                mTextView_Receive.setVisibility(View.GONE);
                mVideoView_Receive.setVisibility(View.GONE);
                mMapView_Receive.setVisibility(View.GONE);
                mVcardView_Receive.setVisibility(View.GONE);
                mAudioView_Receive.setTag(mChatMessage);
                mAudioView_Receive.setOnClickListener(mReceiveClickListener);
                break;
            }
            case SuntekMessageData.MSG_TYPE_VIDEO: {
                PublicMediaMessage mMsg = (PublicMediaMessage)paMsg;
                mAudioView_Receive.setVisibility(View.GONE);
                mImageView_Receive.setVisibility(View.GONE);
                mTextView_Receive.setVisibility(View.GONE);
                mVideoView_Receive.setVisibility(View.VISIBLE);
                mMapView_Receive.setVisibility(View.GONE);
                mVcardView_Receive.setVisibility(View.GONE);
                String videoThumb = mMsg.getMedia().getThumbLink();
                if (!TextUtils.isEmpty(videoThumb)) {
                    LoaderImageTask loaderImageTask = new LoaderImageTask(videoThumb, false, false,
                            true, true);
                    mAsynImageLoader.loadImageAsynByUrl(loaderImageTask, new ImageCallback() {
                        @Override
                        public void loadImageCallback(Bitmap bitmap) {
                            if (bitmap != null) {
                                @SuppressWarnings("deprecation")
                                Drawable drawable = new BitmapDrawable(bitmap);
                                mVideoView_Receive.setBackground(drawable);
                            }
                        }
                    });
                }
                mVideoView_Receive.setTag(mChatMessage);
                mVideoView_Receive.setOnClickListener(mReceiveClickListener);
                break;
            }
            case SuntekMessageData.MSG_TYPE_IMAGE: {
                PublicMediaMessage mMsg = (PublicMediaMessage)paMsg;
                mAudioView_Receive.setVisibility(View.GONE);
                mImageView_Receive.setVisibility(View.VISIBLE);
                mTextView_Receive.setVisibility(View.GONE);
                mVideoView_Receive.setVisibility(View.GONE);
                mMapView_Receive.setVisibility(View.GONE);
                mVcardView_Receive.setVisibility(View.GONE);
                PublicMediaContent paContent = mMsg.getMedia();
                if (paContent != null) {
                    String imagePath = paContent.getThumbLink();
                    if (!TextUtils.isEmpty(imagePath)) {
                        LoaderImageTask loaderImageTask = new LoaderImageTask(imagePath, false,
                                false, true, true);
                        mAsynImageLoader.loadImageAsynByUrl(loaderImageTask, new ImageCallback() {
                            @Override
                            public void loadImageCallback(Bitmap bitmap) {
                                if (bitmap != null) {
                                    @SuppressWarnings("deprecation")
                                    Drawable drawable = new BitmapDrawable(bitmap);
                                    mImageView_Receive.setBackground(drawable);
                                }
                            }
                        });
                    }
                }
                mImageView_Receive.setTag(mChatMessage);
                mImageView_Receive.setOnClickListener(mReceiveClickListener);
                break;
            }
            case SuntekMessageData.MSG_TYPE_MAP: 
                PublicTextMessage mapMsg = (PublicTextMessage)paMsg;
                mAudioView_Receive.setVisibility(View.GONE);
                mImageView_Receive.setVisibility(View.GONE);
                mTextView_Receive.setVisibility(View.GONE);
                mVideoView_Receive.setVisibility(View.GONE);
                mMapView_Receive.setVisibility(View.VISIBLE);
                mVcardView_Receive.setVisibility(View.GONE);
                mMapView_Receive.setTag(mapMsg);
                mMapView_Receive.setOnClickListener(new OnClickListener() {
                    
                    @Override
                    public void onClick(View v) {
                    	PublicTextMessage mapMessage = (PublicTextMessage) v.getTag();
                        String filePath = mapMessage.getContent();
                        RcsGeoLocation geo = PASendMessageUtil.readPaMapXml(filePath);
                        String geourl = "geo:" + geo.getLat() + "," + geo.getLng();
                        setMapFileArgs(filePath, mChatMessage);
                        try {
                            Uri uri = Uri.parse(geourl);
                            Intent intent_map = new Intent(Intent.ACTION_VIEW, uri);
                            mContext.startActivity(intent_map);
                        } catch (Exception e) {
                            Toast.makeText(mContext, R.string.toast_install_map, Toast.LENGTH_SHORT)
                                    .show();
                        }
                    }
                });
                break;
            case SuntekMessageData.MSG_TYPE_PAID_EMO:
                mAudioView_Receive.setVisibility(View.GONE);
                mImageView_Receive.setVisibility(View.VISIBLE);
                mTextView_Receive.setVisibility(View.GONE);
                mVideoView_Receive.setVisibility(View.GONE);
                mMapView_Receive.setVisibility(View.GONE);
                mVcardView_Receive.setVisibility(View.GONE);
                mImageView_Receive.setTag(mChatMessage);
                mImageView_Receive.setOnClickListener(mReceiveClickListener);
                RcsEmojiStoreUtil.getInstance().loadImageAsynById(mImageView_Receive,
                        mChatMessage.getData(), RcsEmojiStoreUtil.EMO_STATIC_FILE);
                break;
            case SuntekMessageData.MSG_TYPE_CONTACT: {
                mAudioView_Receive.setVisibility(View.GONE);
                mImageView_Receive.setVisibility(View.GONE);
                mTextView_Receive.setVisibility(View.GONE);
                mVideoView_Receive.setVisibility(View.GONE);
                mMapView_Receive.setVisibility(View.GONE);
                mVcardView_Receive.setVisibility(View.VISIBLE);
                mVcardView_Receive.setTag(mChatMessage);
                mVcardView_Receive.setOnClickListener(new OnClickListener() {
					
					@Override
					public void onClick(View v) {
						ChatMessage chatMessage = (ChatMessage) v.getTag();
						PublicMessage publicMessage = chatMessage.getPublicMessage();
						PublicTextMessage tMsg = (PublicTextMessage)publicMessage;
	                    String vcardFilePath = tMsg.getContent();
	                    setMapFileArgs(vcardFilePath, chatMessage);
	                    ArrayList<RcsPropertyNode> propList = PAMessageUtil.openRcsVcardDetail(mContext, vcardFilePath);
	                    PAMessageUtil.showDetailVcard(mContext, propList);
					}
				});
                break;
            }
            default: {
                PublicTextMessage tMsg = (PublicTextMessage)paMsg;
                String text = "";
                if (tMsg != null)
                    text = tMsg.getContent();
                mAudioView_Receive.setVisibility(View.GONE);
                mImageView_Receive.setVisibility(View.GONE);
                mTextView_Receive.setVisibility(View.VISIBLE);
                mVideoView_Receive.setVisibility(View.GONE);
                mMapView_Receive.setVisibility(View.GONE);
                mVcardView_Receive.setVisibility(View.GONE);
                mTextView_Receive.setText(text);
                break;
            }
        }
    }

    private void updateReceiveState() {
        int sendState = mChatMessage.getMsgState();
        switch (sendState) {
            case SuntekMessageData.MSG_STATE_DOWNLOAD_FAIL:
                mReceiveState.setVisibility(View.VISIBLE);
                mReceiveState.setText(R.string.message_download_fail);
                mRedownload.setVisibility(View.VISIBLE);
                mRedownload.setTag(mChatMessage);
                mRedownload.setOnClickListener(new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                    	ChatMessage chatMessage = (ChatMessage) v.getTag();
                        if (mFileTrasnfer.containsKey(chatMessage.getMessageId())) {
                            mFileTrasnfer.remove(chatMessage.getMessageId());
                        }
                        downloadFile(chatMessage);
                    }
                });
                break;
            default:
                mReceiveState.setVisibility(View.GONE);
                mRedownload.setVisibility(View.GONE);
                break;
        }
    }

    private OnClickListener mReceiveClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
        	ChatMessage chatMessage = (ChatMessage) v.getTag();
            String contentType = PAMessageUtil.getMessageContentType(chatMessage);
            PublicMessage publicMessage = chatMessage.getPublicMessage();
            PublicMediaMessage mMsg = (PublicMediaMessage)publicMessage;
            PublicMediaContent paContent = mMsg.getMedia();
            String url = paContent.getOriginalLink();

            if (chatMessage.getMsgType() == SuntekMessageData.MSG_TYPE_TEXT) {
                return;
            }
            String filePath = PASendMessageUtil.getPaFilePath(chatMessage);
            boolean isDownloaded = CommonUtil.isFileExists(filePath);
            if (!isDownloaded) {
                downloadFile(chatMessage);
                return;
            }
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
                case SuntekMessageData.MSG_TYPE_VIDEO:
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

    private void downloadFile(final ChatMessage chatMessage) {
        if (mFileTrasnfer.containsKey(chatMessage.getMessageId()))
            return;
        mFileTrasnfer.put(chatMessage.getMessageId(), Long.parseLong("0"));
        mDownloadState.setVisibility(View.VISIBLE);
        mReceiveState.setVisibility(View.GONE);
        mRedownload.setVisibility(View.GONE);
        PublicMessage publicMessage = chatMessage.getPublicMessage();
        PublicMediaMessage mMsg = (PublicMediaMessage)publicMessage;
        final PublicMediaContent paContent = mMsg.getMedia();
        CommonHttpRequest.getInstance().downloadFile(paContent.getOriginalLink(),
                chatMessage.getMsgType(), new Callback() {
                    @Override
                    public boolean handleMessage(Message msg) {
                        if (msg != null) {
                            Response response = (Response)msg.obj;
                            if (response.state == Response.SECCESS) {
                                setChatMessageFileArgs(chatMessage, paContent.getOriginalLink());
                                mFileTrasnfer.remove(chatMessage.getMessageId());
                                mDownloadState.setVisibility(View.GONE);
                                mReceiveState.setVisibility(View.GONE);
                                mRedownload.setVisibility(View.GONE);
                            } else if (response.state == Response.PROGRESS) {
                                mFileTrasnfer.put(chatMessage.getMessageId(),
                                        Long.parseLong(response.feedback));
                                mDownloadState.setText(mContext.getString(R.string.downloading)
                                        + response.feedback + "%");
                            } else {
                                mFileTrasnfer.remove(chatMessage.getMessageId());
                                mDownloadState.setVisibility(View.GONE);
                                mReceiveState.setVisibility(View.VISIBLE);
                                mReceiveState.setText(R.string.message_download_fail);
                                mRedownload.setVisibility(View.VISIBLE);
                                mRedownload.setTag(chatMessage);
                                mRedownload.setOnClickListener(new OnClickListener() {
                                    @Override
                                    public void onClick(View v) {
                                    	ChatMessage message = (ChatMessage) v.getTag();
                                        if (mFileTrasnfer.containsKey(message.getMessageId())) {
                                            mFileTrasnfer.remove(message.getMessageId());
                                        }
                                        downloadFile(message);
                                    }
                                });
                            }
                        }
                        return false;
                    }
                });
    }

    private void setChatMessageFileArgs(ChatMessage chatMsg, String url) {
        File file = null;
        String filePath = chatMsg.getFilepath();
        if (filePath == null) {
            int fileType = chatMsg.getMsgType();
            filePath = CommonUtil.getFileCacheLocalPath(fileType, url);
            if (filePath != null) {
                chatMsg.setFilepath(filePath);
            }
        }
        if (filePath != null && chatMsg.getFilesize() == 0) {
            file = new File(filePath);
            if (file.exists()) {
                chatMsg.setFilesize(file.length());
            }
        }
        String fileName = chatMsg.getFilename();
        if (fileName == null && file != null) {
            fileName = file.getName();
            if (fileName != null) {
                chatMsg.setFilename(fileName);
            }else {
                fileName = CommonUtil.getFileNameByUrl(url);
                if (fileName != null) {
                    chatMsg.setFilename(fileName);
                }
            }
        }
    }

    private void setMapFileArgs(String mapFilePath, ChatMessage chatMsg) {
        File file = null;
        if (chatMsg.getFilepath() == null) {
            chatMsg.setFilepath(mapFilePath);
        }
        if (chatMsg.getFilesize() == 0) {
            file = new File(mapFilePath);
            if (file != null) {
                chatMsg.setFilesize(file.length());
            }
        }
        if (chatMsg.getFilename() == null && file != null) {
            chatMsg.setFilename(file.getName());
        }
    }

    private boolean isMediaType(int type) {
        return (type == SuntekMessageData.MSG_TYPE_IMAGE)
                || (type == SuntekMessageData.MSG_TYPE_AUDIO)
                || (type == SuntekMessageData.MSG_TYPE_VIDEO);
    }
}
