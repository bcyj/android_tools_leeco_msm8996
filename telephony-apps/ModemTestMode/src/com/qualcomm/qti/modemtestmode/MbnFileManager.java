/**
 * Copyright (c) 2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 **/

package com.qualcomm.qti.modemtestmode;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.channels.FileChannel;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Stack;

import android.content.Context;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.util.Log;
import android.view.KeyEvent;

public class MbnFileManager {
    private final static String TAG = "MbnTest_fileMgr";
    private final int SORT_NONE = 1;
    private final int SORT_BY_NAME_ASC = 2;
    private int mSortType = SORT_BY_NAME_ASC;

    private ArrayList<String> mLastDirContent;
    private ArrayList<String> mDirList;
    private Stack<String> mPathStack;
    private String mRoot = null;
    //DO NOT CLEAR mConfig
    private NamingConfig mNormalConfig, mAbNornalConfig = null;
    private NamingConfig mConfig = null;
    private int mPathLevel = 0;
    private int mCurrentChoice = 0;
    private String mPreviousChoice = null;

    private class NamingConfig {
        private String[] mConfig;
        private int len;

        public NamingConfig(String[] config, int len) {
            this.mConfig = config;
            this.len = len;
        }
    }

    public MbnFileManager(String[] config, String[] abnormalConfig) {
        this.mDirList = new ArrayList<String>();
        this.mPathStack = new Stack<String>();
        mNormalConfig = new NamingConfig(config,
                MbnTestUtils.MBN_PATH_NORMAL_MAX_LEVEL);
        mAbNornalConfig = new NamingConfig(abnormalConfig,
                MbnTestUtils.MBN_PATH_ABNORMAL_MAX_LEVEL);

        // default to china
        mConfig = mNormalConfig;
    }

    // Here expect /firmware/image/ or /sdcard/xxxx/
    public ArrayList<String> setRootDir(String name) {
        mPathLevel = 0;
        mCurrentChoice = 0;
        mPreviousChoice = null;
        mPathStack.clear();
        mPathStack.push(name);
        mRoot = name;
        log("Root Path:" + name);
        return showDirContent();
    }

    private ArrayList<String> showDirContent() {
        // Need clear previous content
        if (!mDirList.isEmpty()) {
            mDirList.clear();
        }

        File file = new File(getCurAbsolutelyPath());
        if (file.exists() && file.canRead()) {
            String[] tmp = file.list();

            for (int i = 0; i < tmp.length; i++) {
                if(tmp[i].toString().charAt(0) != '.') {
                    mDirList.add(tmp[i]);
                }
            }
        }

        sortList(mDirList);
        return mDirList;
    }

    public void setSortType(int sort) {
        this.mSortType = sort;
    }

    public void sortList(ArrayList<String> list) {
        switch (mSortType) {
        case SORT_BY_NAME_ASC:
            Collections.sort(list);
            break;
        case SORT_NONE:
        default:
            break;
        }
    }

    public String getCurrentConfig() {
        if (mPathLevel < mConfig.mConfig.length) {
            return mConfig.mConfig[mPathLevel];
        }
        return "UNKNOWN";
    }

    public boolean isDirectory(String name) {
        return new File(mPathStack.peek() + "/" + name).isDirectory();
    }

    public String getCurrentDir() {
        return mPathStack.peek();
    }

    public boolean isRootDir() {
        if (mPathStack.peek() != null) {
            return mPathStack.peek().equals(mRoot);
        }
        return true;
    }

    public void walk( File root ) {
        File[] list = root.listFiles();
        if (list == null) return;
        for ( File f : list ) {
            if ( f.isDirectory() ) {
                walk( f );
            }
            else {
                mLastDirContent.add(f.getAbsolutePath().substring(
                        getCurAbsolutelyPath().length()));
            }
        }
    }

    // show next Directory con
    public ArrayList<String> getNextDir() {
        if (mDirList != null) {
            //differentiate china and non-china
            if (mPathLevel == 0) {
                if (mDirList.get(mCurrentChoice).toLowerCase().equals("china")) {
                    mConfig = mNormalConfig;
                } else {
                    mConfig = mAbNornalConfig;
                }
            }
            mPathStack.push(mDirList.get(mCurrentChoice));
            mPathLevel++;
        }

        //Is last Directory, need recursively search all files;
        if (mPathLevel == mConfig.len) {
            if (mLastDirContent == null) {
                mLastDirContent = new ArrayList<String>();
            }
            if (!mLastDirContent.isEmpty()) {
                mLastDirContent.clear();
            }
            walk(new File(getCurAbsolutelyPath()));
            return mLastDirContent;
        } else {
            return showDirContent();
        }
    }

    // check if it is last directory
    public boolean isLastDir() {
        // hack here for firmware partition use
        if (mPathLevel == mConfig.len) {
            return true;
        } else {
            String path = getCurAbsolutelyPath();
            File f = new File(path);
            if (f.exists() && f.isDirectory()) {
                File[] list = f.listFiles();
                for (int i = 0; i < list.length; i++) {
                    if (list[i].isDirectory()) {
                        return false;
                    }
                }
            }
            return true;
        }
    }

    public String getCurAbsolutelyPath() {
        String path = "";
        int size = mPathStack.size();
        for (int i = 0; i < size; i++) {
            path += mPathStack.get(i) + "/";
        }
        return path;
    }

    // will be set after clicking an item
    public void setCurrentChoice(int position) {
        this.mCurrentChoice = position;
    }

    // get current positon for back use
    public int getPreviousChoice() {
        return mDirList.indexOf(mPreviousChoice);
    }

    public ArrayList<String> handleKeyDown(int keyCode) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (mPathStack.peek().equals(mRoot)) {
                return null;
            }
            mPreviousChoice = mPathStack.pop();
            // For first time return back;
            if (mPathLevel > 0) {
                mPathLevel--;
            }
        }
        return showDirContent();
    }

    public static String getInternalStorage() {
        return System.getenv("EXTERNAL_STORAGE");
    }

    public static String getSDPath(Context context) {
        StorageManager storageManager =
                (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
        StorageVolume[] volumes = storageManager.getVolumeList();
        for (int i = 0; i < volumes.length; i++) {
            if (volumes[i].isRemovable() && volumes[i].allowMassStorage() &&
                    volumes[i].getDescription(context).contains("SD")) {
                return volumes[i].getPath();
            }
        }
        return null;
    }

    public static boolean copyWithChannels(File src, File dest, boolean append) {
        FileChannel inChannel = null;
        FileChannel outChannel = null;
        FileInputStream inStream = null;
        FileOutputStream outStream = null;
        try{
            try {
                inStream = new FileInputStream(src);
                inChannel = inStream.getChannel();
                outStream = new FileOutputStream(dest, append);
                outChannel = outStream.getChannel();
                long bytesTransferred = 0;
                while(bytesTransferred < inChannel.size()){
                    bytesTransferred += inChannel.transferTo(
                            0, inChannel.size(), outChannel);
                }
            }
            finally {
                if (inChannel != null) inChannel.close();
                if (outChannel != null) outChannel.close();
                if (inStream != null) inStream.close();
                if (outStream != null) outStream.close();
            }
        }
        catch (FileNotFoundException e){
            Log.e(TAG, "MbnTest_ File not found: " + e);
            return false;
        }
        catch (IOException e){
            Log.e(TAG, "MbnTest_ IO exception:" + e);
            return false;
        }

        return true;
    }

    private void log(String msg) {
        Log.d(TAG, "MbnTest_ " + msg);
    }
}
