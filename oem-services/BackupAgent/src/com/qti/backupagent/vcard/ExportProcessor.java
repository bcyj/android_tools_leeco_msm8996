/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.qti.backupagent.vcard;

import android.content.ContentResolver;
import android.content.Context;
import android.net.Uri;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.RawContactsEntity;
import android.util.Log;

import com.android.vcard.VCardComposer;
import com.android.vcard.VCardConfig;

import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;

/**
 * Class for processing one export request from a user. Dropped after exporting requested Uri(s).
 * {@link VCardService} will create another object when there is another export request.
 */
public class ExportProcessor {
    private static final String LOG_TAG = "VCardExport";
    private static final boolean DEBUG = true;

    private final Context mContext;
    private final ContentResolver mResolver;
    private final ExportRequest mExportRequest;


    private volatile boolean mCanceled;
    private volatile boolean mDone;

    public ExportProcessor(Context context, ExportRequest exportRequest) {
        mContext = context;
        mResolver = context.getContentResolver();
        mExportRequest = exportRequest;
    }

    public void run() {
        // ExecutorService ignores RuntimeException, so we need to show it here.
        try {
            runInternal();

            if (isCancelled()) {
                doCancelNotification();
            }
        } catch (OutOfMemoryError e) {
            Log.e(LOG_TAG, "OutOfMemoryError thrown during import", e);
            throw e;
        } catch (RuntimeException e) {
            Log.e(LOG_TAG, "RuntimeException thrown during export", e);
            throw e;
        } finally {
            synchronized (this) {
                mDone = true;
            }
        }
    }

    private void runInternal() {
        if (DEBUG) Log.d(LOG_TAG, String.format("vCard export has started."));
        final ExportRequest request = mExportRequest;
        VCardComposer composer = null;
        Writer writer = null;
        boolean successful = false;
        try {
            if (isCancelled()) {
                Log.i(LOG_TAG, "Export request is cancelled before handling the request");
                return;
            }
            final Uri uri = request.destUri;
            final OutputStream outputStream;
            try {
                outputStream = mResolver.openOutputStream(uri);
            } catch (FileNotFoundException e) {
                Log.w(LOG_TAG, "FileNotFoundException thrown", e);
                // Need concise title.

                return;
            }

            final String exportType = request.exportType;
            final int vcardType;
            vcardType = VCardConfig.getVCardTypeFromString(exportType);

            composer = new VCardComposer(mContext, vcardType, true);

            // for test
            // int vcardType = (VCardConfig.VCARD_TYPE_V21_GENERIC |
            //     VCardConfig.FLAG_USE_QP_TO_PRIMARY_PROPERTIES);
            // composer = new VCardComposer(ExportVCardActivity.this, vcardType, true);

            writer = new BufferedWriter(new OutputStreamWriter(outputStream));
            final Uri contentUriForRawContactsEntity = RawContactsEntity.CONTENT_URI.buildUpon()
                .appendQueryParameter(RawContactsEntity.FOR_EXPORT_ONLY, "1")
                .build();
            // TODO: should provide better selection.
            if (!composer.init(Contacts.CONTENT_URI, new String[] {Contacts._ID},
                        null, null,
                        null, contentUriForRawContactsEntity)) {
                final String errorReason = composer.getErrorReason();
                Log.e(LOG_TAG, "initialization of vCard composer failed: " + errorReason);
                return;
            }

            final int total = composer.getCount();
            if (total == 0) {
                return;
            }

            int current = 1;  // 1-origin
            while (!composer.isAfterLast()) {
                if (isCancelled()) {
                    Log.i(LOG_TAG, "Export request is cancelled during composing vCard");
                    return;
                }
                try {
                    writer.write(composer.createOneEntry());
                } catch (IOException e) {
                    final String errorReason = composer.getErrorReason();
                    Log.e(LOG_TAG, "Failed to read a contact: " + errorReason);
                    return;
                }

                // vCard export is quite fast (compared to import), and frequent notifications
                // bother notification bar too much.
                if (current % 100 == 1) {
                    doProgressNotification(uri, total, current);
                }
                current++;
            }
            Log.i(LOG_TAG, "Successfully finished exporting vCard " + request.destUri);

            successful = true;
        } finally {
            if (composer != null) {
                composer.terminate();
            }
            if (writer != null) {
                try {
                    writer.close();
                } catch (IOException e) {
                    Log.w(LOG_TAG, "IOException is thrown during close(). Ignored. " + e);
                }
            }
        }
    }

    private String translateComposerError(String errorMessage) {
        return errorMessage;
    }

    private void doProgressNotification(Uri uri, int totalCount, int currentCount) {
    }

    private void doCancelNotification() {
    }

    private void doFinishNotification(final String title, final String description) {
    }

    public synchronized boolean cancel(boolean mayInterruptIfRunning) {
        if (DEBUG) Log.d(LOG_TAG, "received cancel request");
        if (mDone || mCanceled) {
            return false;
        }
        mCanceled = true;
        return true;
    }

    public synchronized boolean isCancelled() {
        return mCanceled;
    }

    public synchronized boolean isDone() {
        return mDone;
    }

    public ExportRequest getRequest() {
        return mExportRequest;
    }
}
