/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.carrierconfigure;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.TelephonyProperties;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

public class Utils {
    private static final String TAG = "Utils";

    public static final boolean DEBUG = true;

    // The action and extra key for start activity.
    public static final String ACTION_TRIGGER =
            "com.qualcomm.qti.carrierconfigure.trigger";
    public static final String ACTION_TRIGGER_START =
            "com.qualcomm.qti.carrierconfigure.trigger.start";
    public static final String ACTION_TRIGGER_WELCOME =
            "com.qualcomm.qti.carrierconfigure.trigger.welcome";

    public static final String EXTRA_PATH_LIST    = "trigger_path_list";
    public static final String EXTRA_CARRIER_LIST = "trigger_carrier_list";

    //Special ROW package name. Should be sync with CarrierLoadService/Utils.java
    public static final String SPECIAL_ROW_PACKAGE_NAME = "ROW";

    public static final String CARRIER_TO_DEFAULT_NAME = "2Default";

    // The default sim mode.
    private static final String DEFAULT_SIM_MODE = "ssss";

    // The modem will use this path to load MBN files as default.
    private static final String DEFAULT_MBN_PATH = "/data/misc/radio";
    // The prop key used to get the MBN file path.
    private static final String PROP_MBN_PATH = "persist.radio.mbn_path";

    // The carrier's MBN file path in OTA pack.
    private static final String CARRIER_MBN_PATH_IN_OTA = "modem_config/mcfg_sw";
    // The ROW MBN file path in OTA pack.
    private static final String ROW_MBN_PATH_IN_OTA = "modem_row_config/mcfg_sw";
    /**
     * Get the current multi-sim mode.
     */
    public static String getSimMode() {
        String res = SystemProperties.get(
                TelephonyProperties.PROPERTY_MULTI_SIM_CONFIG, DEFAULT_SIM_MODE);
        return res;
    }

    /**
     * Set the multi-sim configuration as newValue.
     * @param newValue
     */
    public static void setSimMode(String newValue) {
        SystemProperties.set(TelephonyProperties.PROPERTY_MULTI_SIM_CONFIG, newValue);
    }

    /**
     * Get the MBN file path which will be used to load the mbn file. The default path
     * is {@link DEFAULT_MBN_PATH}.
     */
    public static String getMBNPath() {
        return SystemProperties.get(PROP_MBN_PATH, DEFAULT_MBN_PATH);
    }

    /**
     * Get carrier's MBN file path in ota pack. The path is {@link CARRIER_MBN_PATH_IN_OTA}.
     */
    public static String getCarrierMBNPathInOta() {
        return CARRIER_MBN_PATH_IN_OTA;
    }

    /**
     * Get ROW MBN file path in ota pack. The path is {@link ROW_MBN_PATH_IN_OTA}.
     */
    public static String getCarrierROWMBNPathInOta() {
        return ROW_MBN_PATH_IN_OTA;
    }

    /**
     * To read the first line from the given file. If the line do not match the regular
     * expression, return null.
     * @param regularExpression used to find the matched line, if it is null, do not check
     *                          if the line matched this expression.
     */
    public static String readFirstLine(File file, String regularExpression) {
        ArrayList<String> res = readFile(file, regularExpression, true);
        if (res == null || res.size() < 1) return null;
        return res.get(0);
    }

    /**
     * To read the content from the given file. If the lines do not match the regular
     * expression, do not add to result.
     * @param regularExpression used to find the matched line, if it is null, do not check
     *                          if the line matched this expression.
     */
    public static ArrayList<String> readFile(File file, String regularExpression,
            boolean onlyReadFirstLine) {
        if (file == null || !file.exists() || !file.canRead()) return null;

        ArrayList<String> contents = new ArrayList<String>();

        FileReader fr = null;
        BufferedReader br = null;
        try {
            fr = new FileReader(file);
            br = new BufferedReader(fr);
            // Read the lines, and get the current carrier.
            String line = null;
            while ((line = br.readLine()) != null && (line = line.trim()) != null) {
                if (!TextUtils.isEmpty(regularExpression)) {
                    if (line.matches(regularExpression)) {
                        contents.add(line);
                    }
                } else {
                    contents.add(line);
                }
                if (onlyReadFirstLine) break;
            }
        } catch (IOException e) {
            Log.e(TAG, "Read File error, caused by: " + e.getMessage());
        } finally {
            try {
                if (br != null) br.close();
                if (fr != null) fr.close();
            } catch (IOException e) {
                Log.e(TAG, "Close the reader error, caused by: " + e.getMessage());
            }
        }

        return contents;
    }

    /**
     * Write the content to this file with append value.
     */
    public static void writeFile(File file, String content, boolean append) throws IOException {
        if (file == null || !file.exists() || !file.canWrite()) {
            throw new IOException("Write file error. Please check the file status.");
        }

        if (TextUtils.isEmpty(content)) return;

        // Write the file with the content.
        FileWriter fw = new FileWriter(file, append);
        try {
            fw.write(content);
        } finally {
            if (fw != null) fw.close();
        }
    }

    /**
     * Return the file size for the given file.
     */
    public static int getFileSize(File file) throws IOException {
        int size = 0;
        if (file != null && file.exists()) {
            FileInputStream fis = new FileInputStream(file);
            try {
                size = fis.available();
            } finally {
                if (fis != null) fis.close();
            }
        }
        return size;
    }

    /**
     * Prompt the alert dialog. If you want to handle the button click action, please
     * implements the OnAlertDialogButtonClick interface.
     */
    public static class MyAlertDialog extends DialogFragment implements OnClickListener {
        public static final String TAG_LABEL = "alert";

        private static final String TITLE = "title";
        private static final String MESSAGE = "message";

        public interface OnAlertDialogButtonClick {
            public void onAlertDialogButtonClick(int which);
        }

        public static MyAlertDialog newInstance(Fragment targetFragment, int titleResId,
                int messageResId) {
            MyAlertDialog dialog = new MyAlertDialog();

            Bundle bundle = new Bundle();
            bundle.putInt(TITLE, titleResId);
            bundle.putInt(MESSAGE, messageResId);
            dialog.setArguments(bundle);

            dialog.setCancelable(false);
            dialog.setTargetFragment(targetFragment, 0);
            return dialog;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final int titleResId = getArguments().getInt(TITLE);
            final int messageResId = getArguments().getInt(MESSAGE);

            AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
            builder.setIcon(android.R.drawable.ic_dialog_alert)
                    .setTitle(titleResId)
                    .setMessage(messageResId)
                    .setPositiveButton(android.R.string.ok, this)
                    .setNegativeButton(android.R.string.cancel, this);

            AlertDialog dialog = builder.create();
            dialog.setCanceledOnTouchOutside(false);
            return dialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            Fragment target = getTargetFragment();
            if (target instanceof OnAlertDialogButtonClick) {
                ((OnAlertDialogButtonClick) target).onAlertDialogButtonClick(which);
            }
        }
    }

    /**
     * Prompt the notice dialog. If you want to handle the button click action, please
     * implements the OnNoticeDialogButtonClick interface.
     */
    public static class MyNoticeDialog extends DialogFragment implements OnClickListener {
        public static final String TAG_LABEL = "notification";

        private static final String ID = "id";
        private static final String TITLE = "title";
        private static final String MESSAGE = "message";

        private int mDialogId = -1;

        public interface OnNoticeDialogButtonClick {
            public void onNoticeDialogButtonClick(int dialogId);
        }

        public static MyNoticeDialog newInstance(Fragment targetFragment, int dialogId,
                int titleResId, int messageResId) {
            MyNoticeDialog dialog = new MyNoticeDialog();

            Bundle bundle = new Bundle();
            bundle.putInt(ID, dialogId);
            bundle.putInt(TITLE, titleResId);
            bundle.putInt(MESSAGE, messageResId);
            dialog.setArguments(bundle);

            dialog.setCancelable(false);
            dialog.setTargetFragment(targetFragment, 0);
            return dialog;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final int titleResId = getArguments().getInt(TITLE);
            final int messageResId = getArguments().getInt(MESSAGE);
            mDialogId = getArguments().getInt(ID);

            AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
            builder.setIcon(android.R.drawable.ic_dialog_alert)
                    .setTitle(titleResId)
                    .setMessage(messageResId)
                    .setPositiveButton(android.R.string.ok, this);

            AlertDialog dialog = builder.create();
            dialog.setCanceledOnTouchOutside(false);
            return dialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            Fragment target = getTargetFragment();
            if (target instanceof OnNoticeDialogButtonClick) {
                ((OnNoticeDialogButtonClick) target).onNoticeDialogButtonClick(mDialogId);
            }
        }
    }

    /**
     * Prompt the wait progress dialog.
     */
    public static class WaitDialog extends DialogFragment {
        public static final String TAG_LABEL = "wait";

        public static WaitDialog newInstance() {
            WaitDialog dialog = new WaitDialog();
            dialog.setCancelable(false);
            return dialog;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            ProgressDialog dialog = new ProgressDialog(getActivity());
            dialog.setMessage(getString(R.string.progress_wait));
            dialog.setCanceledOnTouchOutside(false);
            return dialog;
        }
    }

    @SuppressWarnings("unchecked")
    public static String getMBNFileFromOta(String srcZipFileName, String dstOutputDirectory, String fileName) {
        Log.d(TAG, "fileName="+fileName);
        String resultString = null;
        JarFile jarFile = null;
        try {
            jarFile = new JarFile(srcZipFileName);
            Enumeration entries = jarFile.entries();
            JarEntry jarEntry = null;
            File outputDirectory = new File(dstOutputDirectory);
            outputDirectory.mkdirs();
            while (entries.hasMoreElements()) {
                jarEntry = (JarEntry) entries.nextElement();
                String entryName = jarEntry.getName();
                Log.d(TAG, "entryName="+entryName);
                if (!entryName.endsWith(fileName)) {
                    continue;
                }
                String filePath = dstOutputDirectory + File.separator
                        + entryName.substring(entryName.lastIndexOf("/") + 1);
                if (unzipOneFile(jarFile, jarEntry, filePath)) {
                    resultString = filePath;
                }
                Log.d(TAG,"filename="+filePath + "resultString="+resultString);
                break;
            }
        } catch (IOException ex) {
            ex.printStackTrace();
        } finally {
            if (jarFile != null) {
                try {
                    jarFile.close();
                } catch (IOException e) {}
            }
        }
        return resultString;
    }

    private static boolean unzipOneFile(JarFile jarFile, JarEntry jarEntry, String filePath) {
        boolean result = true;
        InputStream in = null;
        FileOutputStream out = null;
        try {
            File file = new File(filePath);
            in = jarFile.getInputStream(jarEntry);
            out = new FileOutputStream(file);

            int byteCount;
            byte[] by = new byte[1024];

            while ((byteCount = in.read(by)) != -1) {
                out.write(by, 0, byteCount);
            }
            out.flush();
        } catch (IOException ex) {
            ex.printStackTrace();
            result = false;
        } finally {
            if (in != null)
                try {
                    in.close();
                } catch (IOException e) {}
            if (out != null)
                try {
                    out.close();
                } catch (IOException e) {}
        }
        return result;
    }

    public static String getPresetMBNFile(String srcDirPath, String fileName) {
        File srcDir = new File(srcDirPath);
        if (!srcDir.isDirectory()) {
            return null;
        }

        String resultString = null;
        ArrayList<String> fileList = new ArrayList<String>();
        listAllFiles(srcDirPath, fileList);
        for(String file : fileList) {
            if (file.endsWith(fileName)) {
                resultString = file;
                break;
            }
        }

        Log.d(TAG, "resultString=" + resultString);
        return resultString;
    }

    private static void listAllFiles(String dirPath, ArrayList<String> fileList) {
        if (fileList == null) {
            return;
        }
        File dir = new File(dirPath);
        if (!dir.isDirectory()) {
            return;
        }
        File[] files = dir.listFiles();
        for (File file : files) {
            if (file.isDirectory()) {
                listAllFiles(file.getAbsolutePath(), fileList);
            } else {
                fileList.add(file.getAbsolutePath());
            }
        }
    }
}
