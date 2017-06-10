/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.client.aidl.ClientInterfaceIntents;
import com.suntek.mway.rcs.client.aidl.constant.BroadcastConstants;
import com.suntek.mway.rcs.client.aidl.provider.SuntekMessageData;
import com.suntek.mway.rcs.client.aidl.provider.model.ChatMessage;
import com.suntek.mway.rcs.client.api.exception.OperatorException;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.client.api.util.log.LogHelper;
import com.suntek.mway.rcs.nativeui.RcsApiManager;
import com.suntek.mway.rcs.nativeui.utils.ImageUtils;
import com.suntek.mway.rcs.nativeui.utils.RcsChatMessageUtils;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.telephony.TelephonyManager;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.VideoView;

import java.util.HashMap;

public class BurnFlagMessageActivity extends Activity {

    public static final String ACTION_REGISTER_STATUS_CHANGED = "com.suntek.mway.rcs.ACTION_REGISTER_STATUS_CHANGED";

    private static final int BURN_TIME_REFRESH = 1;

    private static final int AUDIO_TIME_REFRESH = 2;

    private static final int VIDEO_TIME_REFRESH = 3;

    public static final String VIDEO_HEAD = "[video]";

    public static final String SPLIT = "-";

    private ImageView image;

    private VideoView video;

    private TextView audio;

    private TextView text;

    private TextView time;

    private TextView video_len;

    private TextView progressText;


    private long tempTime;

    private MediaPlayer mediaPlayer;

    private int msgType;

    private int screenWidth;

    private boolean bSelectMode = false;

    private int playingId = -1;

    private String filePath;

    private int len = 0;

    private boolean isSimCardExist = true;

    private long lastProgress = 0;

    private ChatMessage msg;

    private HashMap<String, Long> fileTrasnfer = new HashMap<String, Long>();

    private BroadcastReceiver receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context arg0, Intent intent) {
            String messageId = intent
                    .getStringExtra(BroadcastConstants.BC_VAR_TRANSFER_PRG_MESSAGE_ID);
            long sessionId = intent.getLongExtra(
                    BroadcastConstants.BC_VAR_TRANSFER_PRG_SESSION_ID, -1);
            long start = intent.getLongExtra(
                    BroadcastConstants.BC_VAR_TRANSFER_PRG_START, -1);
            long end = intent.getLongExtra(
                    BroadcastConstants.BC_VAR_TRANSFER_PRG_END, -1);
            long total = intent.getLongExtra(
                    BroadcastConstants.BC_VAR_TRANSFER_PRG_TOTAL, -1);
            String notifyMessageId = intent
                    .getStringExtra(BroadcastConstants.BC_VAR_TRANSFER_PRG_MESSAGE_ID);
            LogHelper.trace("messageId =" + messageId + ",sessionId ="
                    + sessionId + "start =" + start + ";end =" + end
                    + ";total =" + total);
            if (notifyMessageId != null && notifyMessageId.equals(messageId)
                    && start == end) {
                LogHelper.trace("loadImage");
                progressText.setVisibility(View.GONE);
                if (msgType == SuntekMessageData.MSG_TYPE_IMAGE) {
                    loadImage();
                } else if (msgType == SuntekMessageData.MSG_TYPE_VIDEO) {
                    loadVideo();
                }
            }
            if (notifyMessageId != null && notifyMessageId.equals(messageId)
                    && total != 0) {
                long temp = end * 100 / total;
                LogHelper.trace("file tranfer progress = " + temp
                        + "% ; lastprogress = " + lastProgress + "% .");
                if (lastProgress == 0 || temp - lastProgress >= 5) {
                    lastProgress = temp;
                    progressText.setText(String
                            .format(getString(R.string.image_downloading),
                                    lastProgress));
                }
            }

            String action = intent.getAction();
            if (action.equals(ACTION_REGISTER_STATUS_CHANGED)) {
                int status = intent.getIntExtra("status", -1);
                if (ClientInterfaceIntents.REGISTER_FAILED == status) {
                    onPause();
                }
            }
            else if (action
                    .equals(BroadcastConstants.UI_MESSAGE_STATUS_CHANGE_NOTIFY)) {
                int status = intent.getIntExtra(
                        BroadcastConstants.BC_VAR_MSG_STATUS, -1);
                if (SuntekMessageData.MSG_BURN_HAS_BEEN_BURNED == status) {
                    Toast.makeText(getBaseContext(), R.string.message_is_burnd,
                            Toast.LENGTH_SHORT).show();
                    onPause();
                }
            }
        }

    };

    public static void start(Context context, String messageId) {
        Intent intent = new Intent(context, BurnFlagMessageActivity.class);
        intent.putExtra("smsId", messageId);
        context.startActivity(intent);
    }

    private Runnable refresh = new Runnable() {

        @Override
        public void run() {
            tempTime = tempTime - 1000;
            time.setText(tempTime / 1000 + "");
            if (tempTime != 0) {
                handler.postDelayed(this, 1000);
            } else {
                Toast.makeText(getBaseContext(), R.string.message_is_burnd,
                        Toast.LENGTH_SHORT).show();
                finish();
            }

        }
    };

    private Runnable refreshAudio = new Runnable() {

        @Override
        public void run() {
            len = len - 1000;
            audio.setText(getString(R.string.audio_length) + len / 1000 + "\"");
            // 当前显示事件为O的时候跳出
            if (len != 0) {
                handler.postDelayed(this, 1000);
            } else {
                Toast.makeText(getBaseContext(), R.string.message_is_play_over,
                        Toast.LENGTH_SHORT).show();
                audio.setVisibility(View.GONE);
            }

        }
    };

    private Runnable refreshvideo = new Runnable() {

        @Override
        public void run() {
            len = len - 1000;
            video_len.setText(getString(R.string.video_length) + len / 1000
                    + "\"");
            if (len != 0) {
                handler.postDelayed(this, 1000);
            } else {
                Toast.makeText(getBaseContext(), R.string.message_is_play_over,
                        Toast.LENGTH_SHORT).show();
            }

        }
    };

    private Handler handler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case BURN_TIME_REFRESH:
                    time.setText(tempTime / 1000 + "");
                    handler.postDelayed(refresh, 1000);
                    break;
                case AUDIO_TIME_REFRESH:
                    len = len * 1000;
                    audio.setText(getString(R.string.audio_length) + len / 1000
                            + "\"");
                    handler.postDelayed(refreshAudio, 1000);
                    break;
                case VIDEO_TIME_REFRESH:
                    len = len * 1000;
                    video_len.setText(getString(R.string.video_length) + len / 1000
                            + "\"");
                    handler.postDelayed(refreshvideo, 1000);
                    break;
                default:
                    break;
            }
        }

    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Window win = getWindow();
        win.addFlags(WindowManager.LayoutParams.FLAG_SECURE);
        setContentView(R.layout.burn_message_activity);

        WindowManager mWindowManager = (WindowManager) getSystemService(WINDOW_SERVICE);
        screenWidth = mWindowManager.getDefaultDisplay().getWidth();

        IntentFilter filter = new IntentFilter();
        filter.addAction(BroadcastConstants.UI_DOWNLOADING_FILE_CHANGE);
        filter.addAction(ACTION_REGISTER_STATUS_CHANGED);
        registerReceiver(receiver, filter);

        String smsId = getIntent().getStringExtra("smsId");
        msg = RcsChatMessageUtils.getChatMessageOnSMSDB(this, smsId);

        // TODO
        msg = RcsChatMessageUtils.getTestChatMessage();
        if (msg == null) {
            finish();
        }
        findView();
        initView();

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onPause() {
        super.onPause();

        if (mediaPlayer != null) {
            mediaPlayer.stop();
            mediaPlayer.release();
        }

        try {
            if (msg != null) {
               RcsApiManager.getMessageApi().burnMessageAtOnce(msg.getId() + "");
            }
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }
        if (receiver != null) {
            unregisterReceiver(receiver);
        }
        this.finish();
    }

    private void loadVideo() {
        String filepath = null;
        try {
            filepath = RcsChatMessageUtils.getFilePath(msg);
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }
        if (RcsChatMessageUtils.isFileDownload(filepath, msg.getFilesize())) {
            video.setVisibility(View.VISIBLE);
            video_len.setVisibility(View.VISIBLE);
            video_len.setText(getString(R.string.video_length) + len / 1000
                    + "\"");
            video.setVideoURI(Uri.parse(filepath));
            video.start();
            handler.sendEmptyMessage(VIDEO_TIME_REFRESH);
        } else {
            video.setVisibility(View.GONE);
            video_len.setVisibility(View.GONE);
            acceptFile();
        }
    }

    private void loadImage() {

        try {
            filePath = RcsChatMessageUtils.getFilePath(msg);

            // TODO
            filePath = "/sdcard/a.jpg";
        } catch (ServiceDisconnectedException e1) {
            e1.printStackTrace();
        }
        if (RcsChatMessageUtils.isFileDownload(filePath, msg.getFilesize())) {
            Bitmap imageBm = ImageUtils.getBitmap(filePath);
            image.setImageBitmap(imageBm);
        } else {
            acceptFile();
        }
    }

    private void acceptFile() {
        try {
            RcsApiManager.getMessageApi().acceptFile(msg);
        } catch (OperatorException e) {
            e.printStackTrace();
            Toast.makeText(this, R.string.image_msg_load_fail_tip,
                    Toast.LENGTH_SHORT).show();
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }
    }

    private void initView() {
        msgType = msg.getMsgType();
        switch (msgType) {
            case SuntekMessageData.MSG_TYPE_TEXT:
                text.setVisibility(View.VISIBLE);
                text.setText(msg.getData());
                break;
            case SuntekMessageData.MSG_TYPE_IMAGE:
                image.setVisibility(View.VISIBLE);
                loadImage();
                break;
            case SuntekMessageData.MSG_TYPE_AUDIO:
                audio.setVisibility(View.VISIBLE);
                // 语音消息长度
                try {
                    String lens = msg.getData().substring(7);
                    String[] m = lens.split(SPLIT);
                    len = Integer.parseInt(m[0]);
                } catch (Exception e) {
                    e.printStackTrace();
                    len = 0;
                }

                // 语音消息，播放
                mediaPlayer = new MediaPlayer();
                // 为播放器设置数据文件
                try {
                    if (mediaPlayer != null && mediaPlayer.isPlaying()) {
                        mediaPlayer.stop();
                        mediaPlayer.release();
                    }
                    String filePath = RcsApiManager.getMessageApi()
                            .getFilepath(msg);
                    mediaPlayer.setDataSource(filePath);
                    mediaPlayer.prepare();
                } catch (Exception e) {

                    e.printStackTrace();

                    if (mediaPlayer != null && mediaPlayer.isPlaying()) {
                        mediaPlayer.stop();
                    }
                    Toast.makeText(this, R.string.open_file_fail,
                            Toast.LENGTH_SHORT).show();
                    // return;
                    finish();
                }
                mediaPlayer.start();
                handler.sendEmptyMessage(AUDIO_TIME_REFRESH);
                break;

            case SuntekMessageData.MSG_TYPE_VIDEO:
                video.setVisibility(View.VISIBLE);
                video_len.setVisibility(View.VISIBLE);
                len = getVideoLength(msg.getData());
                loadVideo();
                break;
            default:
                break;
        }

    }

    private void findView() {
        progressText = (TextView) findViewById(R.id.progress_text);
        image = (ImageView) findViewById(R.id.image);
        video = (VideoView) findViewById(R.id.video);
        audio = (TextView) findViewById(R.id.audio);
        text = (TextView) findViewById(R.id.text);
        time = (TextView) findViewById(R.id.burn_time);
        video_len = (TextView) findViewById(R.id.video_len);
    }

    public static int getVideoLength(String message) {
        if (message.startsWith(VIDEO_HEAD)) {
            return Integer.parseInt(message.substring(VIDEO_HEAD.length())
                    .split(SPLIT)[0]);
        }
        return 0;
    }

}
