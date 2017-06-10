/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.backupagent;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.net.Uri;
import android.os.Environment;
import android.provider.ContactsContract;
import android.util.Log;

import com.android.vcard.exception.VCardException;
import com.qti.backupagent.vcard.ExportProcessor;
import com.qti.backupagent.vcard.ExportRequest;
import com.qti.backupagent.vcard.ExportVCard;
import com.qti.backupagent.vcard.ImportProcessor;
import com.qti.backupagent.vcard.ImportRequest;
import com.qti.backupagent.vcard.ImportVCard;

import java.io.File;
import java.io.IOException;

public class QtiBackupContacts {
    String mfile;
    Context mContext;
    private final boolean DEBUG = false;
    private final String TAG = "QtiBackupContacts";

    public QtiBackupContacts(Context context, String vfile) {
        mContext = context;
        mfile = vfile;
    }

    public void backupContacts() {
        exportRequest(mContext.getFilesDir()+ "/" + mfile);
    }

    private void exportRequest(String path) {
        final ExportRequest request;
        request = ExportVCard.constructExportRequest(path);
        ExportProcessor ep = new ExportProcessor(mContext, request);
        ep.run();
    }

    public void restoreContacts(File file) {
        importRequest(Uri.fromFile(file));
    }

    private void importRequest(Uri uri) {
        final ImportRequest request;
        try{
            request = ImportVCard.constructImportRequest(mContext, null, uri, null);
            ImportProcessor ip= new ImportProcessor(mContext, request);
            ip.run();
        }
        catch (VCardException e) {
            if (DEBUG) Log.w(TAG, e.getLocalizedMessage());
        } catch (IOException e) {
            if (DEBUG) Log.w(TAG, e.getLocalizedMessage());
        }
    }
}
