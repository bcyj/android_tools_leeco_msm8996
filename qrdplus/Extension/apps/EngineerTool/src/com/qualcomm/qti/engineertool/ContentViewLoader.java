/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool;

import android.content.AsyncTaskLoader;
import android.content.Context;
import android.content.res.XmlResourceParser;
import android.os.Parcelable;
import android.text.TextUtils;
import android.util.Log;

import com.qualcomm.qti.engineertool.model.ListModel;
import com.qualcomm.qti.engineertool.model.XmlUtils;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

public class ContentViewLoader extends AsyncTaskLoader<Parcelable> {
    private static final String TAG = "ContentViewLoader";

    private String mCustomerPath;

    public ContentViewLoader(Context context) {
        super(context);
    }

    public void setCustomerContentView(String path) {
        mCustomerPath = path;
    }

    @Override
    protected void onStartLoading() {
        super.onStartLoading();
        forceLoad();
    }

    @Override
    public Parcelable loadInBackground() {
        Parcelable res = null;

        try {
            if (TextUtils.isEmpty(mCustomerPath)) {
                // Load the content view from the default defined xml.
                XmlResourceParser xml = getContext().getResources().getXml(R.xml.tools_config);
                res = parseContentViewFromXML(xml);
            } else {
                // Load the content view from the given path.
                File xmlFile = new File(mCustomerPath);
                if (xmlFile != null && xmlFile.exists()) {
                    FileReader xmlReader = null;
                    try {
                        XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
                        factory.setNamespaceAware(true);
                        XmlPullParser xmlParser = factory.newPullParser();
                        xmlReader = new FileReader(xmlFile);
                        xmlParser.setInput(xmlReader);
                        res = parseContentViewFromXML(xmlParser);
                    } catch (FileNotFoundException e) {
                        Log.w(TAG, "Should not here as FileNotFoundException: " + e.getMessage());
                    } finally {
                        try {
                            if (xmlReader != null) xmlReader.close();
                        } catch (IOException e) {
                            Log.e(TAG, "Close the reader error, caused by: " + e.getMessage());
                        }
                    }
                }
            }
        } catch (XmlPullParserException e) {
            Log.w(TAG, "Parse xml failed, XmlPullParserException: " + e.getMessage());
        } catch (IOException e) {
            Log.w(TAG, "Parse xml failed, IOException: " + e.getMessage());
        }

        return res;
    }

    private Parcelable parseContentViewFromXML(XmlPullParser xml) throws XmlPullParserException,
            IOException {
        Parcelable res = null;
        int eventType = -1;
        while ((eventType = xml.next()) != XmlPullParser.END_DOCUMENT) {
            // Need start with "list".
            if (eventType == XmlPullParser.START_TAG
                    && XmlUtils.TAG_LIST.equals(xml.getName())) {
                ListModel list = new ListModel();
                ListModel.getListModel(getContext(), list, xml, eventType);
                return list;
            }
        }

        return res;
    }
}
