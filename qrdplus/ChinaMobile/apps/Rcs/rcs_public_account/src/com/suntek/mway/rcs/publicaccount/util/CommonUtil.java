/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.util;

import com.suntek.mway.rcs.client.aidl.provider.SuntekMessageData;
import com.suntek.mway.rcs.publicaccount.PublicAccountApplication;
import com.suntek.mway.rcs.publicaccount.R;
import com.suntek.mway.rcs.publicaccount.ui.widget.RcsEmojiGifView;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Environment;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.inputmethod.InputMethodManager;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;
import android.widget.Toast;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

public class CommonUtil {

    private static final String ENCODING_TYPE = "UTF-8";

    public static final int FILE_TYPE_IMAGE = SuntekMessageData.MSG_TYPE_IMAGE;

    public static final int FILE_TYPE_AUDIO = SuntekMessageData.MSG_TYPE_AUDIO;

    public static final int FILE_TYPE_VIDEO = SuntekMessageData.MSG_TYPE_VIDEO;

    @SuppressWarnings("static-access")
    public static void closeKB(Activity activity) {
        if (activity.getCurrentFocus() != null) {
            ((InputMethodManager)activity.getSystemService(activity.INPUT_METHOD_SERVICE))
                    .hideSoftInputFromWindow(activity.getCurrentFocus().getWindowToken(),
                            InputMethodManager.HIDE_NOT_ALWAYS);
        }
    }

    public static void openKB(Context context) {
        InputMethodManager inputMethodManager = (InputMethodManager)context
                .getSystemService(Context.INPUT_METHOD_SERVICE);
        inputMethodManager.toggleSoftInput(0, InputMethodManager.HIDE_NOT_ALWAYS);
    }

    public static int dip2px(Context context, float dipValue) {
        final float scale = context.getResources().getDisplayMetrics().density;
        return (int)(dipValue * scale + 0.5f);
    }

    public static int px2dip(Context context, float pxValue) {
        final float scale = context.getResources().getDisplayMetrics().density;
        return (int)(pxValue / scale + 0.5f);
    }

    public static int getScreenWidth(Context context) {
        return context.getResources().getDisplayMetrics().widthPixels;
    }

    public static int getScreenHeight(Context context) {
        return context.getResources().getDisplayMetrics().heightPixels;
    }

    public static String getFileCacheLocalPath(int fileType, String fileUrl) {
        String sdcardRootPath = Environment.getExternalStorageDirectory().getPath();
        String packagePath = sdcardRootPath + "/Android/data/"
                + PublicAccountApplication.getInstance().getApplicationContext().getPackageName();
        String rootPath = packagePath + "/cache/";
        String fileName = getFileNameByUrl(fileUrl);
        String absolutePath = rootPath + fileName;
        if (fileType == FILE_TYPE_IMAGE) {
            absolutePath = rootPath + "image/" + fileName;
        }
        if (fileType == FILE_TYPE_AUDIO) {
            absolutePath = rootPath + "audio/" + fileName;
        }
        if (fileType == FILE_TYPE_VIDEO) {
            absolutePath = rootPath + "video/" + fileName;
        }
        return absolutePath;
    }

    private static String getFileSuffixName(String fileUrl) {
        if (fileUrl != null) {
            String suffixName = fileUrl.substring(fileUrl.lastIndexOf("."));
            return suffixName;
        }
        return null;
    }

    public static String getFileNameByUrl(String url) {
        return MD5Util.encode_32(url) + getFileSuffixName(url);
    }

    public static boolean isFileExistsByUrl(int fileType, String fileUrl) {
        String filePath = getFileCacheLocalPath(fileType, fileUrl);
        File file = new File(filePath);
        if (file.exists())
            return true;
        return false;
    }

    public static boolean isFileExists(String filePath) {
        if (filePath == null) {
            return false;
        }
        File file = new File(filePath);
        if (file.exists())
            return true;
        return false;
    }

    public static void createFolderIfNotExist(String absFolderPath) {
        File file = new File(absFolderPath);
        if (!file.exists())
            file.mkdirs();
    }

    public static byte[] inputStreamToBytes(InputStream inputStream) throws IOException {
        if (inputStream == null)
            return null;
        int length;
        byte[] returnData = null;
        int bufferSize = 1024;
        byte[] buffer = new byte[bufferSize];
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        while ((length = inputStream.read(buffer)) != -1) {
            baos.write(buffer, 0, length);
        }
        returnData = baos.toByteArray();
        baos.close();
        inputStream.close();
        return returnData;
    }

    public static String byteArrayToString(byte[] datas) {
        if (datas == null || datas.length < 1)
            return "";

        try {
            return new String(datas, ENCODING_TYPE);
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
        return "";
    }

    public static byte[] stringToByteArray(String content) {
        try {
            if (!TextUtils.isEmpty(content)) {
                return content.getBytes(ENCODING_TYPE);
            }
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
        return null;
    }

    public static boolean existAvailableNetwork(Context context) {
        ConnectivityManager connectivity = (ConnectivityManager)context
                .getSystemService(Context.CONNECTIVITY_SERVICE);
        if (connectivity == null) {
            return false;
        } else {
            NetworkInfo[] info = connectivity.getAllNetworkInfo();
            if (info != null) {
                for (int i = 0; i < info.length; i++) {
                    if (info[i].getState() == NetworkInfo.State.CONNECTED) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    public static File createFile(String path) throws Exception {
        File file = new File(path);
        File dir = file.getParentFile();
        if (dir != null && !dir.exists()) {
            dir.mkdirs();
        }
        if (!file.exists()) {
            file.createNewFile();
        }
        return file;
    }

    public static Uri getOutputVideoFileUri(String filePath) {
        if (getOutputVideoFile(filePath) != null) {
            return Uri.fromFile(getOutputVideoFile(filePath));
        } else {
            return null;
        }
    }

    private static File getOutputVideoFile(String path) {
        File file = new File(path);
        File dir = file.getParentFile();
        if (dir != null && !dir.exists()) {
            dir.mkdirs();
        }
        return file;
    }

    public static void showToast(Context context, int resId) {
        Toast.makeText(context, resId, Toast.LENGTH_SHORT).show();
    }

    public static String getTimeStamp(long timeStamp){
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        Date date = new Date(timeStamp);
        return sdf.format(date);
    }

    public static String getTimeStamp(String gmtTimeStamp) {
        try {
            SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
            Date d = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ssZ").parse(gmtTimeStamp);
            return sdf.format(d);
        } catch (ParseException e) {
            e.printStackTrace();
            return "";
        }
    }

    public static void startEmojiStore(Context context) {
        if (isPackageInstalled(context, "com.temobi.dm.emoji.store")) {
            Intent mIntent = new Intent();
            ComponentName comp = new ComponentName("com.temobi.dm.emoji.store",
                    "com.temobi.dm.emoji.store.activity.EmojiActivity");
            mIntent.setComponent(comp);
            context.startActivity(mIntent);
        } else {
            Toast.makeText(context, R.string.install_emoj_store, Toast.LENGTH_SHORT).show();
        }
    }

    public static boolean isPackageInstalled(Context context, String packageName) {
        PackageManager pm = context.getPackageManager();
        List<ApplicationInfo> installedApps = pm
                .getInstalledApplications(PackageManager.GET_UNINSTALLED_PACKAGES);
        for (ApplicationInfo info : installedApps) {
            if (packageName.equals(info.packageName)) {
                return true;
            }
        }
        return false;
    }

    public static void openPopupWindow(Context context, View view, byte[] data) {
        LinearLayout.LayoutParams mGifParam = new LinearLayout.LayoutParams(
                LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
        Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
        int windowWidth = bitmap.getWidth() + CommonUtil.dip2px(context, 40);
        int windowHeight = bitmap.getHeight() + CommonUtil.dip2px(context, 40);
        ColorDrawable transparent = new ColorDrawable(Color.TRANSPARENT);
        RelativeLayout relativeLayout = new RelativeLayout(context);
        relativeLayout
                .setLayoutParams(new LinearLayout.LayoutParams(windowWidth, windowHeight));
        relativeLayout.setBackgroundResource(R.drawable.rcs_emoji_popup_bg);
        relativeLayout.setGravity(Gravity.CENTER);
        RcsEmojiGifView emojiGifView = new RcsEmojiGifView(context);
        emojiGifView.setLayoutParams(mGifParam);
        emojiGifView.setBackground(transparent);
        emojiGifView.setMonieByteData(data);
        relativeLayout.addView(emojiGifView);
        PopupWindow popupWindow = new PopupWindow(view, windowWidth, windowHeight);
        popupWindow.setBackgroundDrawable(transparent);
        popupWindow.setFocusable(true);
        popupWindow.setOutsideTouchable(true);
        popupWindow.setContentView(relativeLayout);
        popupWindow.showAtLocation(view, Gravity.CENTER, 0, 0);
        popupWindow.update();
    }

}
