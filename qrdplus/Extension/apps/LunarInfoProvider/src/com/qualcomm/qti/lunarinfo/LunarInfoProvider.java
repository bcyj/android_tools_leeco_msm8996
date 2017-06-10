/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.lunarinfo;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.database.Cursor;
import android.net.Uri;
import android.text.TextUtils;
import android.util.Log;

import java.util.Locale;

/**
 * For this content provider, we accept these content uri with the query parameters:<br>
 * To get one day info, please use this content uri:<br>
 * CONTENT_URI_GET_ONE_DAY = Uri.parse("content://" + {@link #AUTHORITY} + "/"
 * + {@link #FUN_GET_ONE_DAY});<br>
 * To get one month info, please use this content uri:<br>
 * CONTENT_URI_GET_ONE_MONTH = Uri.parse("content://" + {@link #AUTHORITY} + "/"
 * + {@link #FUN_GET_ONE_MONTH});<br>
 * To get the from-to info, please use this content uri:<br>
 * CONTENT_URI_GET_FROM_TO = Uri.parse("content://" + {@link #AUTHORITY} + "/"
 * + {@link #FUN_GET_FROM_TO});<br>
 */
public class LunarInfoProvider extends ContentProvider {
    private static final String TAG = "LunarInfoProvider";

    // Defined the authority for this provider.
    private static final String AUTHORITY = "com.qualcomm.qti.lunarinfo";

    // Defined the support get function.
    private static final String FUN_GET_ONE_DAY = "one_day";
    private static final String FUN_GET_ONE_MONTH = "one_month";
    private static final String FUN_GET_FROM_TO = "from_to";
    private static final int ONE_DAY = 1;
    private static final int ONE_MONTH = 2;
    private static final int FROM_TO = 3;

    private static UriMatcher sUriMatcher;
    static {
        sUriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
        sUriMatcher.addURI(AUTHORITY, FUN_GET_ONE_DAY, ONE_DAY);
        sUriMatcher.addURI(AUTHORITY, FUN_GET_ONE_MONTH, ONE_MONTH);
        sUriMatcher.addURI(AUTHORITY, FUN_GET_FROM_TO, FROM_TO);
    }

    // The supported query parameters.
    private static final String PARAM_YEAR = "year";
    private static final String PARAM_MONTH = "month";
    private static final String PARAM_DAY = "day";
    private static final String PARAM_FROM_YEAR = "from_year";
    private static final String PARAM_FROM_MONTH = "from_month";
    private static final String PARAM_FROM_DAY = "from_day";
    private static final String PARAM_TO_YEAR = "to_year";
    private static final String PARAM_TO_MONTH = "to_month";
    private static final String PARAM_TO_DAY = "to_day";

    @Override
    public boolean onCreate() {
        // We use this provider to return the lunar info.
        // And needn't to create the database here.
        return true;
    }

    /**
     * We only support three query functions: {@link #FUN_GET_ONE_DAY}, {@link #FUN_GET_ONE_MONTH}
     * and {@link #FUN_GET_FROM_TO} by <B>appending the query parameter</B>.<br>
     * For {@link #FUN_GET_ONE_DAY}, could append the parameter {@link #PARAM_YEAR},
     * {@link #PARAM_MONTH} and {@link #PARAM_DAY} to get one day info.<br>
     * For {@link #FUN_GET_ONE_MONTH}, could append the parameter {@link #PARAM_YEAR} and
     * {@link #PARAM_MONTH} to get one month info.<br>
     * For {@link #FUN_GET_FROM_TO}, could append the parameter {@link #PARAM_FROM_YEAR},
     * {@link #PARAM_FROM_MONTH}, {@link #PARAM_FROM_DAY}, {@link #PARAM_TO_YEAR},
     * {@link #PARAM_TO_MONTH} and {@link #PARAM_TO_DAY} to get the info between the from-to.
     */
    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs,
            String sortOrder) {
        Log.i(TAG, "Get the lunar info for URI: " + uri.toString());
        if (projection != null
                || selection != null
                || selectionArgs != null
                || sortOrder != null) {
            Log.w(TAG, "We only support the uri with the query params, others will be ignore.");
        }

        Cursor res = null;

        // Check if need to show the lunar info.
        if (!showLunar()) {
            Log.w(TAG, "Needn't show lunar info as the locale is not zh-cn, return empty cursor.");
            return res;
        }

        // Check the URI if accept.
        if (TextUtils.isEmpty(uri.getPath())) {
            Log.w(TAG, "Do not given the query function, return empty cursor.");
            return res;
        }

        // Check the query parameter if empty.
        if (uri.getQueryParameterNames().size() < 1) {
            Log.w(TAG, "Do not given the query parameter, return empty cursor.");
            return res;
        }

        try {
            switch (sUriMatcher.match(uri)) {
                case ONE_DAY: {
                    // As defined, get the support query parameter and ignore others.
                    String year = uri.getQueryParameter(PARAM_YEAR);
                    String month = uri.getQueryParameter(PARAM_MONTH);
                    String day = uri.getQueryParameter(PARAM_DAY);
                    if (TextUtils.isEmpty(year)
                            || TextUtils.isEmpty(month)
                            || TextUtils.isEmpty(day)) {
                        Log.w(TAG, "The request function is one day, but do not give the date"
                            + ", year=" + year + ", month=" + month + ", day=" + day);
                        return res;
                    }
                    LunarInfo info = LunarInfo.getInstance(getContext());
                    res = info.buildCursor(Integer.parseInt(year), Integer.parseInt(month),
                            Integer.parseInt(day));
                    break;
                }
                case ONE_MONTH: {
                    // As defined, get the support query parameter and ignore others.
                    String year = uri.getQueryParameter(PARAM_YEAR);
                    String month = uri.getQueryParameter(PARAM_MONTH);
                    if (TextUtils.isEmpty(year)
                            || TextUtils.isEmpty(month)) {
                        Log.w(TAG, "The request function is one month, but do not give the month"
                            + ", year=" + year + ", month=" + month);
                        return res;
                    }
                    LunarInfo info = LunarInfo.getInstance(getContext());
                    res = info.buildCursor(Integer.parseInt(year), Integer.parseInt(month));
                    break;
                }
                case FROM_TO: {
                    // As defined, get the support query parameter and ignore others.
                    String from_year = uri.getQueryParameter(PARAM_FROM_YEAR);
                    String from_month = uri.getQueryParameter(PARAM_FROM_MONTH);
                    String from_day = uri.getQueryParameter(PARAM_FROM_DAY);
                    String to_year = uri.getQueryParameter(PARAM_TO_YEAR);
                    String to_month = uri.getQueryParameter(PARAM_TO_MONTH);
                    String to_day = uri.getQueryParameter(PARAM_TO_DAY);
                    if (TextUtils.isEmpty(from_year)
                            || TextUtils.isEmpty(from_month)
                            || TextUtils.isEmpty(from_day)
                            || TextUtils.isEmpty(to_year)
                            || TextUtils.isEmpty(to_month)
                            || TextUtils.isEmpty(to_day)) {
                        Log.w(TAG, "The request function is from_to, but do not give the range"
                                + ", from: year=" + from_year + ", month=" + from_month
                                + ", day=" + from_day + " | to: year=" + to_year
                                + ", month=" + to_month + ", day=" + to_day);
                            return res;
                    }
                    LunarInfo info = LunarInfo.getInstance(getContext());
                    res = info.buildCursor(
                            Integer.parseInt(from_year),
                            Integer.parseInt(from_month),
                            Integer.parseInt(from_day),
                            Integer.parseInt(to_year),
                            Integer.parseInt(to_month),
                            Integer.parseInt(to_day));
                    break;
                }
            }
        } catch (NumberFormatException ex) {
            Log.e(TAG, "Catch the NumberFormatException, please check the URI.");
        }

        return res;
    }

    @Override
    public String getType(Uri uri) {
        // Do not support.
        return null;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        // Do not support.
        return null;
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        // Do not support.
        return 0;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        // Do not support.
        return 0;
    }

    private boolean showLunar() {
        Locale locale = Locale.getDefault();
        String language = locale.getLanguage().toLowerCase();
        String country = locale.getCountry().toLowerCase();
        return ("zh".equals(language) && "cn".equals(country));
    }

}
