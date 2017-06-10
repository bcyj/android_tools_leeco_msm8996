/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.example.android.notepad;

import com.example.android.notepad.NotePad.Notes;

public interface ConstInterfaceNotesList {
    /**
     * Standard projection for the interesting columns of a normal note.
     */
    public static final String[] PROJECTION = new String[] {
            Notes._ID, // 0
            Notes.COLUMN_NAME_NOTE, // 1
            Notes.COLUMN_NAME_TITLE, // 2
            Notes.COLUMN_NAME_MODIFICATION_DATE, // 3
            Notes.COLUMN_NAME_CREATE_DATE,
    };

    /** The index of the note column */
    public static final int COLUMN_INDEX_NOTE = 1;
    public static final int COLUMN_INDEX_TITLE = 2;// <= This title will be not
                                                   // used. But to suit for old
                                                   // version db, it will be
                                                   // reserved.
    public static final int COLUMN_INDEX_MODIFIED_DATE = 3;
    public static final String AUTHORITY = "com.google.provider.NotePad";
    public static final String TAG = "NotePad";
}
