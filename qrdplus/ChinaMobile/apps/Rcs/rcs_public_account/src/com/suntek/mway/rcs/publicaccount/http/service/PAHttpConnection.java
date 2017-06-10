/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.http.service;

import com.suntek.mway.rcs.publicaccount.PublicAccountApplication;
import com.suntek.mway.rcs.publicaccount.R;
import com.suntek.mway.rcs.publicaccount.util.CommonUtil;

import android.util.Log;
import android.widget.Toast;

import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSession;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

public class PAHttpConnection {

    public static final String CONTENT_TYPE_DEFAULT = "application/x-www-form-urlencoded;charset=UTF-8";

    private static final int CONNECT_TIME_OUT = 6 * 1000;

    private static final int READ_TIME_OUT = 90 * 1000;

    private final static HostnameVerifier DO_NOT_VERIFY = new HostnameVerifier() {
        public boolean verify(String hostname, SSLSession session) {
            return true;
        }
    };

    private static void trustAllHosts() {
        TrustManager[] trustAllCerts = new TrustManager[] {
            new X509TrustManager() {
                public java.security.cert.X509Certificate[] getAcceptedIssuers() {
                    return new java.security.cert.X509Certificate[] {};
                }

                public void checkClientTrusted(X509Certificate[] chain, String authType)
                        throws CertificateException {
                }

                public void checkServerTrusted(X509Certificate[] chain, String authType)
                        throws CertificateException {
                }
            }
        };
        try {
            SSLContext sc = SSLContext.getInstance("TLS");
            sc.init(null, trustAllCerts, new java.security.SecureRandom());
            HttpsURLConnection.setDefaultSSLSocketFactory(sc.getSocketFactory());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static HttpURLConnection createConnection(String url) throws MalformedURLException,
            IOException, Exception {
        if (!CommonUtil.existAvailableNetwork(PublicAccountApplication.getInstance()
                .getApplicationContext()))
            CommonUtil.showToast(PublicAccountApplication.getInstance().getApplicationContext(),
                    R.string.http_no_available_network);
        HttpURLConnection httpUrlConnection = null;
        URL httpUrlObject = new URL(url);
        httpUrlConnection = (HttpURLConnection)httpUrlObject.openConnection();
        if (httpUrlConnection instanceof HttpsURLConnection) {
            ((HttpsURLConnection)httpUrlConnection).setHostnameVerifier(DO_NOT_VERIFY);
            trustAllHosts();
        }
        httpUrlConnection.setReadTimeout(READ_TIME_OUT);
        httpUrlConnection.setDoInput(true);
        httpUrlConnection.setConnectTimeout(CONNECT_TIME_OUT);
        httpUrlConnection.setUseCaches(false);
        return httpUrlConnection;
    }

    public static HttpURLConnection loopDownloadFile(String url) throws Exception {
        int retryCount = 3;
        HttpURLConnection conn = null;
        while (conn == null) {
            if (retryCount > 0) {
                try {
                    --retryCount;
                    conn = downloadFile(url);
                    return conn;
                } catch (Exception e) {
                    e.printStackTrace();
                    throw e;
                }
            }
        }
        return null;
    }

    private static HttpURLConnection downloadFile(String url) throws Exception {
        HttpURLConnection conn = null;
        byte[] responseData = null;

        try {
            conn = createConnection(url);
            conn.setRequestMethod("POST");
            conn.setRequestProperty("Content-Type", CONTENT_TYPE_DEFAULT);
            conn.connect();
            int responseCode = conn.getResponseCode();
            StringBuilder sbLog = new StringBuilder();
            sbLog.append("\nURL: ").append(url).append("\n");
            if (HttpURLConnection.HTTP_OK != responseCode) {
                String error = null;
                try {
                    responseData = CommonUtil.inputStreamToBytes(conn.getErrorStream());
                    error = CommonUtil.byteArrayToString(responseData);
                } catch (Exception e) {
                    e.printStackTrace();
                    error = e.getMessage();
                }
            } else {
                return conn;
            }

        } catch (Exception e) {
            throw e;
        }
        return null;
    }

    public static void closeConnection(HttpURLConnection conn) {
        if (conn != null) {
            try {
                conn.disconnect();
                conn = null;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
}
