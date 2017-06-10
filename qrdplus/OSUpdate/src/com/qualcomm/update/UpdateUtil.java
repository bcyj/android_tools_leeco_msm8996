/**
 * Copyright (c) 2011-2012, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */

package com.qualcomm.update;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import javax.net.ssl.HttpsURLConnection;

import java.net.URL;
import java.util.List;

import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.InputSource;
import org.xml.sax.XMLReader;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.preference.PreferenceManager;
import android.util.Log;

public class UpdateUtil {

    private static final String KEY_UPDATE_FILE_PATH = "update_path";

    private static final String KEY_UPDATE_FILE_SIZE = "update_size";

    private static final String KEY_UPDATE_FILE_DATE = "update_date";

    /**
     * 0: normal 1:delta
     */
    private static final String KEY_UPDATE_MODE = "update_mode";

    private static final String TAG = "QRDUpdate";

    private static final boolean DEBUG = true;

    public static List<UpdateInfo> getUpdateInfo(String address) {
        log("get updates from: " + address);
        UpdateInfoHandler updateInfoHandler = new UpdateInfoHandler();
        HttpsURLConnection connection = null;
        InputStream in = null;
        try {
            URL url = new URL(address);
            connection = (HttpsURLConnection) url.openConnection();
            connection.setRequestProperty("User-Agent", "PacificHttpClient");
            connection.setConnectTimeout(30000);
            connection.setReadTimeout(20000);
            if (connection.getResponseCode() == 404) {
                throw new Exception("404 error!");
            }
            in = connection.getInputStream();
            SAXParserFactory spf = SAXParserFactory.newInstance();
            SAXParser sp;
            sp = spf.newSAXParser();
            XMLReader reader = sp.getXMLReader();
            reader.setContentHandler(updateInfoHandler);
            reader.parse(new InputSource(new InputStreamReader(in, "GBK")));
        } catch (Exception e) {
            log("get updates error:" + e);
            return null;
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException e) {
                }
            }
            if (connection != null) {
                connection.disconnect();
            }
        }
        return updateInfoHandler.getUpdates();
    }

    public static void updateFileSize(Context context, List<UpdateInfo> updates) {
        if (updates == null || updates.size() == 0)
            return;
        for (UpdateInfo update : updates) {
            HttpsURLConnection connection = null;
            try {
                URL url = new URL(DownloadManager.getDefault(context).getServerUrl() + "/"
                        + update.getFileName());
                connection = (HttpsURLConnection) url.openConnection();
                connection.setRequestProperty("User-Agent", "PacificHttpClient");
                connection.setConnectTimeout(30000);
                connection.setReadTimeout(20000);
                update.setSize(connection.getContentLength());
            } catch (Exception e) {
                log("update updates error:" + e);
            } finally {
                if (connection != null) {
                    connection.disconnect();
                }
            }
        }
    }

    public static boolean rebootInstallDelta(Context context) {
        try {
            Intent intent = new Intent();
            intent.setAction("android.system.agent");
            intent.putExtra("para", "reboot,recovery");
            context.startService(intent);
            return true;
        } catch (Exception e) {
            Log.e(TAG, "failed to reboot and install delta:" + e);
            return false;
        }
    }

    /**
     * call uid system to create system directory
     */
    public static boolean mkDeltaDir(Context context) {
        try {
            Intent intent = new Intent();
            intent.setAction("android.system.fullagent");
            String extra = "mkdir,/cache/fota";
            intent.putExtra("para", extra);
            context.startService(intent);
        } catch (Exception e) {
            Log.e(TAG, "make delta dir failed:" + e);
            return false;
        }
        return true;
    }

    public static boolean writeDeltaCommand() {
        String filePath = "/cache/fota/ipth_config_dfs.txt";
        String command = "IP_PREVIOUS_UPDATE_IN_PROGRESS";
        boolean res = true;
        FileWriter mFileWriter = null;
        try {
            mFileWriter = new FileWriter(new File(filePath));
            mFileWriter.write(command);
        } catch (IOException e) {
            Log.e(TAG, "write delta command failed:" + e);
            res = false;
        } finally {
            try {
                mFileWriter.close();
            } catch (IOException e) {
            }
        }
        return res;
    }

    public static boolean copyToDeltaFile(File srcFile) {
        File dstFile = new File("/cache/fota/ipth_package.bin");
        if (dstFile.exists()) {
            dstFile.delete();
        }
        InputStream in = null;
        OutputStream out = null;
        int cnt;
        byte[] buf = new byte[4096];
        try {
            in = new FileInputStream(srcFile);
            out = new FileOutputStream(dstFile);
            while ((cnt = in.read(buf)) >= 0) {
                out.write(buf, 0, cnt);
            }
            return true;
        } catch (IOException e) {
            Log.e(TAG, "failed to copy delta file:" + e);
            return false;
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException e) {
                }
            }
            if (out != null) {
                try {
                    out.close();
                } catch (IOException e) {
                }
            }
        }
    }

    public static String formatSize(float size) {
        long kb = 1024;
        long mb = (kb * 1024);
        long gb = (mb * 1024);
        if (size < kb) {
            return String.format("%d bytes", (int) size);
        } else if (size < mb) {
            return String.format("%.1f kB", size / kb);
        } else if (size < gb) {
            return String.format("%.1f MB", size / mb);
        } else {
            return String.format("%.1f GB", size / gb);
        }
    }

    private static void log(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }

    /**
     * @param context
     * @param path
     * @param isDelta 0:normai 1:delta
     */
    public static void saveUpdate(Context context, String path, boolean isDelta) {
        Editor edit = PreferenceManager.getDefaultSharedPreferences(context).edit();
        edit.putString(KEY_UPDATE_FILE_PATH, path);
        edit.putLong(KEY_UPDATE_FILE_SIZE, new File(path).length());
        edit.putLong(KEY_UPDATE_FILE_DATE, new File(path).lastModified());
        edit.putInt(KEY_UPDATE_MODE, isDelta ? 1 : 0);
        edit.commit();
    }

    public static void deteteUpdate(Context context) {
        Editor edit = PreferenceManager.getDefaultSharedPreferences(context).edit();
        edit.remove(KEY_UPDATE_FILE_PATH);
        edit.remove(KEY_UPDATE_FILE_SIZE);
        edit.remove(KEY_UPDATE_FILE_DATE);
        edit.remove(KEY_UPDATE_MODE);
        edit.commit();
    }

    public static String getLastUpdate(Context context) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
        String filepath = sp.getString(KEY_UPDATE_FILE_PATH, null);
        if (filepath != null) {
            File f = new File(filepath);
            if (f.length() == sp.getLong(KEY_UPDATE_FILE_SIZE, -1)
                    && f.lastModified() == sp.getLong(KEY_UPDATE_FILE_DATE, -1))
                return filepath;
        }
        return null;
    }

    /**
     * 0:normal 1:delta
     *
     * @param context
     * @return
     */
    public static boolean getLastIsDelta(Context context) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
        return sp.getInt(KEY_UPDATE_MODE, 0) == 1;
    }
}
