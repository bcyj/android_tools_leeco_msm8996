/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.universaldownload;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.net.HttpURLConnection;
import java.net.URL;
import java.security.SecureRandom;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

import android.util.Log;
import org.apache.http.conn.ssl.AllowAllHostnameVerifier;

import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;


public class DownloadFile {
    private static final String TAG = "FileDownloader";
    /* downloaded file size */
    private int downloadSize = 0;
    /* origin file size */
    private int fileSize = 0;
    /* download thread */
    private DownloadThread threads;
    /* local file */
    private File saveFile;
    /* download path  */
    private String downloadUrl;
    /* download state */
    private boolean isdownload = false;

    private DownloadInfo dowloadInfo;

    public String getFilePath() {
        return saveFile.getPath();
    }

    public interface DownloadListener {
        public void onProgressUpdate(int progress);
        public void onComplete();
        void onDownloadFailed();
    }

    public boolean getDownloadState() {
        return isdownload;
    }

    public void setDownloadState(boolean state) {
        isdownload = state;
    }

    /**
     * Get the download file size
     * @return the file size
     */
    public int getFileSize() {
        return fileSize;
    }

    /**
     * append already download size
     * @param size
     */
    protected synchronized void append(int size) {
        downloadSize += size;
    }

    /**
     * Trust all server - don't check for any certificate
     */
    private void trustEveryHosts() {
        // Create a trust manager that does not validate certificate chains
        X509TrustManager x509TM = new X509TrustManager() {
            public X509Certificate[] getAcceptedIssuers() {
                return new X509Certificate[] {};
            }

            public void checkClientTrusted(X509Certificate[] chain, String authType)
                    throws CertificateException {
            }

            public void checkServerTrusted(X509Certificate[] chain, String authType)
                    throws CertificateException {
            }
        };
        TrustManager[] trustAllCerts = new TrustManager[] { x509TM };

        //Install the all-trusting trust manager
        try {
            SSLContext sc = SSLContext.getInstance("TLS");
            sc.init(null, trustAllCerts, new SecureRandom());
            HttpsURLConnection.setDefaultSSLSocketFactory(sc.getSocketFactory());
        } catch (Exception e) {
            Log.e(TAG, "trust all host error: " + e);
        }
    }

    /**
     * Constructor
     * @param info download information contains url
     * @param dir the save directory of download file
     */
    public DownloadFile(DownloadInfo info, File dir) {
        try {
            dowloadInfo = info;
            this.downloadUrl = info.getAddr();
            Log.d(TAG, "DownloadFile downloadUrl is " + downloadUrl + "savefile dir is " + dir);
            URL url = new URL(downloadUrl);
            if(!dir.exists()) {
                dir.mkdirs();
            }
            HttpURLConnection conn;
            if(url.getProtocol().toLowerCase().equals("https")) {
                trustEveryHosts();
                HttpsURLConnection https = (HttpsURLConnection) url.openConnection();
                https.setHostnameVerifier(new AllowAllHostnameVerifier());
                conn = https;
            } else {
                conn = (HttpURLConnection) url.openConnection();
            }
            this.fileSize = conn.getContentLength();//Get the file size according response
            if (this.fileSize <= 0) {
                throw new RuntimeException("Unkown file size ");
            }

            String filename = this.downloadUrl.substring(this.downloadUrl.lastIndexOf('/') + 1);
            this.saveFile = new File(dir, filename);
        } catch (Exception e) {
            throw new RuntimeException("don't connection this url" + e);
        }
    }

    /**
     *  Start download file
     * @param listener
     * @return the file size of downloaded
     * @throws Exception
     */
    public int download(DownloadListener listener) throws Exception{
        try {
            URL url = new URL(this.downloadUrl);

            // has been download length
            downloadSize = dowloadInfo.getDownloadSize();
            Log.d(TAG, "download size is " + downloadSize);

            if(downloadSize < this.fileSize) {
                this.threads = new DownloadThread(this, url, this.saveFile,
                        downloadSize, fileSize, listener);
                this.threads.setPriority(7);
                this.threads.start();
            }else{
                this.threads = null;
            }
        } catch (Exception e) {
            throw new Exception("file download fail");
        }
        return this.downloadSize;
    }

    public class DownloadThread extends Thread {
        private static final String TAG = "DownloadThread";
        private static final int BUFFER_SIZE = 1024;
        private final DownloadListener mListener;
        private File saveFile;
        private URL downUrl;
        /* the download file size  */
        private int fileSize;
        /* download position  */
        private int downLength;
        private boolean finish = false;
        private DownloadFile downloader;

        public DownloadThread(DownloadFile downloader, URL downUrl, File saveFile,
                              int downLength, int fileSize, DownloadListener listener) {
            this.downloader = downloader;
            this.downUrl = downUrl;
            this.saveFile = saveFile;
            this.fileSize = fileSize;
            this.downLength = downLength;
            this.mListener = listener;
        }

        @Override
        public void run() {
            if(downLength < fileSize){
                int tryTime = 3;
                do {
                    RandomAccessFile raf = null;
                    InputStream is = null;
                    HttpURLConnection http = null;
                    try {
                        setDownloadState(true);
                        http = (HttpURLConnection) downUrl.openConnection();
                        http.setConnectTimeout(5 * 1000);
                        http.setRequestMethod("GET");
                        http.setRequestProperty("Accept", "image/gif, image/jpeg, image/pjpeg," +
                                " image/pjpeg, application/x-shockwave-flash," +
                                " application/xaml+xml, application/vnd.ms-xpsdocument," +
                                " application/x-ms-xbap, application/x-ms-application," +
                                " application/vnd.ms-excel, application/vnd.ms-powerpoint," +
                                " application/msword, */*");
                        http.setRequestProperty("Accept-Language", "zh-CN");
                        http.setRequestProperty("Referer", downUrl.toString());
                        http.setRequestProperty("Charset", "UTF-8");
                        http.setRequestProperty("Range", "bytes=" + downLength + "-"+ fileSize);
                        http.setRequestProperty("User-Agent", "Mozilla/4.0 (compatible; " +
                                "MSIE 8.0; Windows NT 5.2; Trident/4.0; .NET CLR 1.1.4322;" +
                                " .NET CLR 2.0.50727; .NET CLR 3.0.04506.30;" +
                                " .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729)");
                        http.setRequestProperty("Connection", "Keep-Alive");
                        if (http.getResponseCode() != 200 && http.getResponseCode() != 206) {
                            Log.e(TAG, "dowmload file response " + http.getResponseCode());
                            tryTime--;
                            continue;
                        }

                        is = http.getInputStream();
                        byte[] buffer = new byte[BUFFER_SIZE];
                        int offset = 0;
                        raf = new RandomAccessFile(this.saveFile, "rwd");
                        raf.seek(downLength);
                        while (getDownloadState() &&
                                (offset = is.read(buffer, 0, BUFFER_SIZE)) != -1) {
                            raf.write(buffer, 0, offset);
                            downLength += offset;
                            downloader.append(offset);
                            if(mListener != null) {
                                mListener.onProgressUpdate(downLength);
                            }
                        }
                        this.finish = true;
                        if (downloadSize >= fileSize) {
                            mListener.onComplete();
                        }
                    } catch (Exception e) {
                        Log.e(TAG, "download file error " + e);
                        e.printStackTrace();
                        tryTime--;
                    } finally {
                        try {
                            if(raf != null) {
                                raf.close();
                            }
                            if(is != null) {
                                is.close();
                            }
                            http.disconnect();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                } while (!finish && tryTime > 0);

                if(tryTime == 0) {
                    if(mListener != null) {
                        mListener.onDownloadFailed();
                    }
                }
            }
        }

        /**
         * @return whether download finish
         */
        public boolean isFinish() {
            return finish;
        }

        /**
         * Get the size of downloaded file
         * @return the download size, -1 if download failed
         */
        public int getDownLength() {
            return downLength;
        }
    }
}
