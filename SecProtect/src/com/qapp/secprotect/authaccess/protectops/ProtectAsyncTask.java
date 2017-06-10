/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.authaccess.protectops;

import static com.qapp.secprotect.Configs.INTERNAL_PROTECTED_PATH;
import static com.qapp.secprotect.Configs.INTERNAL_STORAGE;
import static com.qapp.secprotect.Configs.MODE_DEPROTECT;
import static com.qapp.secprotect.Configs.MODE_PROTECT;
import static com.qapp.secprotect.Configs.PROTECTED_PATH;
import static com.qapp.secprotect.Configs.SDCARD_PROTECTED_PATH;
import static com.qapp.secprotect.Configs.SDCARD_ROOT;
import static com.qapp.secprotect.utils.UtilsLog.logd;
import static com.qapp.secprotect.utils.UtilsLog.loge;
import static com.qapp.secprotect.utils.UtilsLog.toast;

import java.io.File;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.DialogInterface.OnCancelListener;
import android.net.Uri;
import android.os.AsyncTask;

import com.qapp.secprotect.R;
import com.qapp.secprotect.utils.UtilsFileOperation;
import com.qapp.secprotect.utils.UtilsStrings;
import com.qapp.secprotect.utils.UtilsSystem;

public class ProtectAsyncTask extends AsyncTask<String, String, Boolean>
        implements OnCancelListener {

    Activity mAttachedActivity;
    private ProgressDialog mProgressDialog;
    int mMode;

    public ProtectAsyncTask(Activity activity, int mode) {
        mAttachedActivity = activity;
        mMode = mode;
    }

    @Override
    protected void onPostExecute(Boolean result) {
        logd();
        if (!result) {
            logd("failed");
            toast(mAttachedActivity,
                    mAttachedActivity.getString(R.string.failed));
        }

        if (ProtectOpsFragment.mFileExplorerFragment != null)
            ProtectOpsFragment.mFileExplorerFragment.refreshExplorer();
        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            try {
                mProgressDialog.dismiss();
            } catch (Exception e) {
                loge(e);
            }
            mProgressDialog = null;
        }

        mAttachedActivity = null;
    }

    @Override
    public void onCancel(DialogInterface dialog) {
        logd();
    }

    @Override
    protected void onPreExecute() {

        logd("");
        mProgressDialog = new ProgressDialog(mAttachedActivity);
        mProgressDialog.setIcon(android.R.drawable.ic_dialog_info);
        // if (mMode == OperationFragment.MODE_DECRYPT) {
        // mProgressDialog.setTitle(com.qapp.secprotect.R.string.decrypt);
        // mProgressDialog.setMessage(mAttachedActivity
        // .getString(com.qapp.secprotect.R.string.decrypting));
        // } else {
        // mProgressDialog.setTitle(com.qapp.secprotect.R.string.encrypt);
        // mProgressDialog.setMessage(mAttachedActivity
        // .getString(com.qapp.secprotect.R.string.encrypting));
        // }
        mProgressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        mProgressDialog.setOnCancelListener(this);
        mProgressDialog.setCanceledOnTouchOutside(false);
        mProgressDialog.setCancelable(false);
        mProgressDialog.show();
    }

    private static String getProtectedPath(String inPath) {
        String outPath = null;
        if (inPath.startsWith(SDCARD_ROOT)) {
            outPath = SDCARD_ROOT + "/" + PROTECTED_PATH
                    + inPath.substring(SDCARD_ROOT.length());
        } else if (inPath.startsWith(INTERNAL_STORAGE)) {
            outPath = INTERNAL_STORAGE + "/" + PROTECTED_PATH
                    + inPath.substring(INTERNAL_STORAGE.length());
        }
        return outPath;
    }

    private static String getDeprotectedPath(String inPath) {
        String outPath = null;
        if (inPath.startsWith(SDCARD_PROTECTED_PATH)) {
            outPath = SDCARD_ROOT
                    + inPath.substring(SDCARD_PROTECTED_PATH.length());
        } else if (inPath.startsWith(INTERNAL_PROTECTED_PATH)) {
            outPath = INTERNAL_STORAGE
                    + inPath.substring(INTERNAL_PROTECTED_PATH.length());
        }
        return outPath;
    }

    private boolean protectFile(String inPath) {
        boolean ret = false;
        File inFile = new File(inPath);

        String outPath = getProtectedPath(inPath);
        if (outPath == null) {
            return false;
        }
        File outFile = new File(outPath);
        if (outFile.exists()) {
            outPath = UtilsStrings.getFileNameWithoutPostfix(outPath)
                    + " - Copy" + UtilsStrings.getFileType(outFile.getName());
            outFile = new File(outPath);
        }

        File outDir = outFile.getParentFile();
        if (!outDir.exists()) {
            outDir.mkdirs();
        }

        ret = UtilsFileOperation.renameFile(inFile, outFile);
        if (ret && UtilsSystem.isImageFile(inPath)) {
            Uri uri = Uri.fromFile(inFile);
            mAttachedActivity.sendBroadcast(new Intent(
                    Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, uri));
        }
        return ret;
    }

    private boolean deprotectFile(String inPath) {
        boolean ret = false;
        File inFile = new File(inPath);

        String outPath = getDeprotectedPath(inPath);
        if (outPath == null) {
            return false;
        }
        File outFile = new File(outPath);
        if (outFile.exists()) {
            outPath = UtilsStrings.getFileNameWithoutPostfix(outPath)
                    + " - Copy" + UtilsStrings.getFileType(outFile.getName());
            outFile = new File(outPath);
        }

        File outDir = outFile.getParentFile();
        if (!outDir.exists()) {
            outDir.mkdirs();
        }

        ret = UtilsFileOperation.renameFile(inFile, outFile);
        if (ret && UtilsSystem.isImageFile(outPath)) {
            Uri uri = Uri.fromFile(outFile);
            mAttachedActivity.sendBroadcast(new Intent(
                    Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, uri));
        }
        return ret;
    }

    @Override
    protected Boolean doInBackground(String... params) {
        logd();
        boolean ret = false;
        String inPath = params[0];
        switch (mMode) {
        case MODE_PROTECT:
            ret = protectFile(inPath);
            break;
        case MODE_DEPROTECT:
            ret = deprotectFile(inPath);
            break;
        default:
            break;
        }
        return ret;
    }
}
