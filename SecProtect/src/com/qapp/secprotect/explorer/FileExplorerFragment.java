/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.explorer;

import static com.qapp.secprotect.Configs.ENCRYPT_FILE_SUFFIX;
import static com.qapp.secprotect.Configs.INTERNAL_PROTECTED_PATH;
import static com.qapp.secprotect.Configs.INTERNAL_STORAGE;
import static com.qapp.secprotect.Configs.MODE_DECRYPT;
import static com.qapp.secprotect.Configs.MODE_DEPROTECT;
import static com.qapp.secprotect.Configs.MODE_ENCRYPT;
import static com.qapp.secprotect.Configs.MODE_PROTECT;
import static com.qapp.secprotect.Configs.PROTECTED_PATH;
import static com.qapp.secprotect.Configs.SDCARD_PROTECTED_PATH;
import static com.qapp.secprotect.Configs.SDCARD_ROOT;
import static com.qapp.secprotect.Configs.STORAGE_ROOT;
import static com.qapp.secprotect.utils.UtilsLog.logd;

import java.io.File;
import java.text.Collator;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Stack;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;
import android.widget.TextView;

import com.qapp.secprotect.Configs;
import com.qapp.secprotect.R;
import com.qapp.secprotect.authaccess.protectops.ProtectAsyncTask;
import com.qapp.secprotect.encryptdata.EncryptAsyncTask;
import com.qapp.secprotect.encryptdata.PasswordActivity;
import com.qapp.secprotect.framework.MainApp;
import com.qapp.secprotect.utils.UtilsLog;

public class FileExplorerFragment extends Fragment implements
        OnItemClickListener {

    private static String mRootPath = STORAGE_ROOT;

    private ArrayList<File> mFileList = new ArrayList<File>();
    private Stack<File> mPathStack = new Stack<File>();
    private ListView mListView;
    private TextView mCurrentDirectoryTextView;
    private FileExplorerAdapter mFileExplorerAdapter;

    private static int mFileExplorerMode = MODE_ENCRYPT;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        logd("");
        super.onCreate(savedInstanceState);
        mFileExplorerAdapter = new FileExplorerAdapter(getActivity(), mFileList);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        logd("");
        View view = inflater.inflate(R.layout.file_explorer_fragment,
                container, false);

        mCurrentDirectoryTextView = (TextView) view
                .findViewById(R.id.current_directory);

        mListView = (ListView) view.findViewById(R.id.file_list);
        mListView.setAdapter(mFileExplorerAdapter);
        mListView.setOnItemClickListener(this);
        mPathStack.clear();
        refreshExplorer(new File(mRootPath));
        return view;
    }

    @Override
    public void onResume() {
        logd("");
        super.onResume();
    }

    @Override
    public void onInflate(Activity activity, AttributeSet attrs,
            Bundle savedInstanceState) {
        logd("");
        super.onInflate(activity, attrs, savedInstanceState);
    }

    private boolean isFiltered(File file) {

        if (!file.exists() || !file.canRead()) {
            return true;
        }
        String filePath = file.getPath();
        String fileName = file.getName();
        String parentPath = file.getParent();
        // only show sdcard0 and sdcard1
        if (STORAGE_ROOT.equals(parentPath)
                && (!INTERNAL_STORAGE.equals(filePath) && !SDCARD_ROOT
                        .equals(filePath))) {
            return true;
        }

        switch (mFileExplorerMode) {
        case MODE_PROTECT:
            if (file.isDirectory()
                    && (INTERNAL_PROTECTED_PATH.equals(filePath))
                    || SDCARD_PROTECTED_PATH.equals(filePath)) {
                return true;
            }
            break;
        case MODE_DEPROTECT:
            if (!PROTECTED_PATH.equals(fileName)
                    && (INTERNAL_STORAGE.equals(parentPath) || SDCARD_ROOT
                            .equals(parentPath))) {
                return true;
            }
            break;

        case MODE_DECRYPT:
            if (file.isFile() && !fileName.endsWith(ENCRYPT_FILE_SUFFIX)) {
                return true;
            }
            break;
        default:
            break;
        }
        return false;
    }

    public void refreshExplorer(File topPath) {

        if (topPath == null || !topPath.exists()) {
            return;
        }
        logd("open " + topPath.getPath() + " current stack="
                + mPathStack.size());
        File[] files = topPath.listFiles();
        if (files == null)
            return;
        mPathStack.push(topPath);
        mFileList.clear();
        for (int i = 0; i < files.length; i++) {
            File file = files[i];

            if (isFiltered(file)) {
                continue;
            }
            // logd(files[i].getPath());
            mFileList.add(file);
        }

        Collections.sort(mFileList, ALPHA_COMPARATOR);
        mFileExplorerAdapter.notifyDataSetChanged();
        mCurrentDirectoryTextView.setText(topPath.getPath());
    }

    public static final Comparator<File> ALPHA_COMPARATOR = new Comparator<File>() {
        private final Collator sCollator = Collator.getInstance();

        @Override
        public int compare(File object1, File object2) {
            return sCollator.compare(object1.getName(), object2.getName());
        }
    };

    public void refreshExplorer() {

        File rootPath = null;
        if (mPathStack.empty())
            return;
        rootPath = mPathStack.pop();
        if (rootPath == null || !rootPath.exists()) {
            return;
        }
        mPathStack.push(rootPath);
        logd("open " + rootPath.getPath() + " current stack="
                + mPathStack.size());
        File[] files = rootPath.listFiles();
        mFileList.clear();
        for (int i = 0; i < files.length; i++) {
            File file = files[i];
            if (isFiltered(file)) {
                continue;
            }
            // logd(files[i].getPath());
            mFileList.add(file);
        }

        Collections.sort(mFileList, ALPHA_COMPARATOR);
        mFileExplorerAdapter.notifyDataSetChanged();
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {

        File file = mFileList.get(position);
        String filePath = file.getPath();
        switch (mFileExplorerMode) {
        case MODE_ENCRYPT:

            if (file.isFile() && filePath.startsWith(INTERNAL_STORAGE)
                    && MainApp.getInstance().mInternalKey == null) {
                UtilsLog.toast(getActivity(),
                        getString(R.string.need_password_toast));

                Intent intent = new Intent(getActivity(),
                        PasswordActivity.class);
                intent.putExtra("mode", Configs.INTENT_CREATE_PASSWORD);
                startActivity(intent);
                return;
            }
            // else if (MainApp.getInstance().mSdcardKey == null) {
            // UtilsLog.toast(getActivity(), "Need password for sdcard");
            // }
            if (file.isFile()
                    && !file.getName().endsWith(Configs.ENCRYPT_FILE_SUFFIX)) {
                doProcess(MODE_ENCRYPT, filePath, filePath
                        + ENCRYPT_FILE_SUFFIX);
            }

            break;
        case MODE_DECRYPT:
            if (file.isFile()) {
                if (!filePath.endsWith(ENCRYPT_FILE_SUFFIX)) {
                    return;
                }
                int suffixIndex = filePath.lastIndexOf(ENCRYPT_FILE_SUFFIX);
                doProcess(MODE_DECRYPT, filePath,
                        filePath.substring(0, suffixIndex));
            }
            break;
        case MODE_PROTECT:
            if (file.isFile()) {
                new ProtectAsyncTask(getActivity(), MODE_PROTECT)
                        .execute(filePath);
            }
            break;

        case MODE_DEPROTECT:
            if (file.isFile()) {
                new ProtectAsyncTask(getActivity(), MODE_DEPROTECT)
                        .execute(filePath);
            }
            break;

        default:
            break;
        }
        if (file.isDirectory()) {
            refreshExplorer(mFileList.get(position));
        }
    }

    @Override
    public void onPause() {
        logd();
        super.onPause();
    }

    @Override
    public void onDestroy() {
        logd();
        super.onDestroy();
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        logd();
        super.onSaveInstanceState(outState);
        outState.putString("rootPath", mPathStack.get(mPathStack.size() - 1)
                .getPath());
    }

    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (mPathStack.size() >= 2) {
                // pop the current path first
                mPathStack.pop();
                refreshExplorer(mPathStack.pop());
                return false;
            }
        }
        return true;
    }

    private void doProcess(int mode, String srcPath, String outPath) {
        new EncryptAsyncTask(getActivity(), mode).execute(srcPath, outPath);
    }

    public static void init(int fileExplorerMode, String rootPath) {
        mFileExplorerMode = fileExplorerMode;
        mRootPath = rootPath;
    }

    public static int getFileExplorerMode() {
        return mFileExplorerMode;
    }
}
