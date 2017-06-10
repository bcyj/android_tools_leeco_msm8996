/**
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.homelocation;

import java.util.ArrayList;
import java.util.List;

import android.content.ContentProviderOperation;
import android.content.ContentValues;
import android.content.Context;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.net.Uri;
import android.os.RemoteException;

import com.qti.homelocation.GeocodedLocation.Area;
import com.qti.homelocation.GeocodedLocation.AreaCode;

public class LocationDataUtils {

    public static List<Area> listAreas(Context context) {
        List<Area> results = new ArrayList<Area>();
        Cursor c = null;
        try {
            c = context.getContentResolver().query(Area.CONTENT_URI, null, null, null, null);
            while (c != null && c.moveToNext()) {
                results.add(new Area(c));
            }
        } finally {
            if (c != null) {
                c.close();
            }
        }
        return results;
    }

    public static List<AreaCode> listAreaCodes(Context context, Area area) {
        List<AreaCode> results = new ArrayList<AreaCode>();
        Cursor c = null;
        try {
            c = context.getContentResolver().query(AreaCode.CONTENT_URI, null,
                    AreaCode.AREA_ID + "=?", new String[] {
                        String.valueOf(area.getAreaId())
                    }, AreaCode.CODE);
            while (c != null && c.moveToNext()) {
                results.add(new AreaCode(c));
            }
        } finally {
            if (c != null) {
                c.close();
            }
        }
        return results;
    }

    public static void attach(final ArrayList<ContentProviderOperation> list,
            final ContentValues values, Uri uri) {
        ContentProviderOperation.Builder builder = ContentProviderOperation.newInsert(uri);
        builder.withValues(values);
        list.add(builder.build());
    }

    public static boolean flush(Context context, final ArrayList<ContentProviderOperation> list) {
        try {
            context.getContentResolver().applyBatch(GeocodedLocation.AUTHORITY, list);
            return true;
        } catch (RemoteException e) {
        } catch (OperationApplicationException e) {
        }
        return false;
    }
}
