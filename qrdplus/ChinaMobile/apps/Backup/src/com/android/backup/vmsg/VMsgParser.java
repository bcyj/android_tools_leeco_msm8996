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

package com.android.backup.vmsg;

import java.nio.charset.Charset;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.NoSuchElementException;
import android.util.Log;
import java.util.ArrayList;
import android.text.TextUtils;

public class VMsgParser extends AbstractStringMessageParser {
    private static final String TAG = "VMsgParser";

    private static final Charset CHARSET = Charset.forName("UTF-16LE");
    private static final String BEGIN_MARKER = "BEGIN";
    private static final String END_MARKER = "END";
    private static final String VMSG_TAG = "VMSG";
    private static final String VENV_TAG = "VENV";
    private static final String VCARD_TAG = "VCARD";
    private static final String VBODY_TAG = "VBODY";
    private static final String TEL_TAG = "TEL";
    private static final String DATE_TAG = "Date";
    private static final String BOX_TAG = "X-IRMC-BOX";
    private static final String STATUS_TAG = "X-IRMC-STATUS";

    private static final String BOX_TAG2 = "X-BOX";
    private static final String STATUS_TAG2 = "X-READ";
    private static final String TEXT_TAG2 = "Text";
    private static final String SUBJECT_TAG2 = "Subject";
    private static final String LOCKED_TAG2 = "X-LOCKED";
    private static final String SUBID_TAG2 = "X-SIMID";

    private static final String[] STATUS_PATH = new String[] {
            VMSG_TAG, STATUS_TAG
    };
    private static final String[] BOX_PATH = new String[] {
            VMSG_TAG, BOX_TAG
    };
    private static final String[] FROM_PATH = new String[] {
            VMSG_TAG, VCARD_TAG, TEL_TAG
    };
    private static final String[] TO_PATH = new String[] {
            VMSG_TAG, VENV_TAG, VCARD_TAG, TEL_TAG
    };
    private static final String[] BODY_PATH = new String[] {
            VMSG_TAG, VENV_TAG, VBODY_TAG
    };
    private static final String[] DATE_PATH = new String[] {
            VMSG_TAG, VENV_TAG, VBODY_TAG, DATE_TAG
    };

    private static final String[] STATUS_PATH2 = new String[] {
            VMSG_TAG, VBODY_TAG, STATUS_TAG2
    };
    private static final String[] BOX_PATH2 = new String[] {
            VMSG_TAG, VBODY_TAG, BOX_TAG2
    };
    private static final String[] FROM_PATH2 = new String[] {
            VMSG_TAG, VCARD_TAG, TEL_TAG
    };
    private static final String[] TO_PATH2 = new String[] {
            VMSG_TAG, VCARD_TAG, TEL_TAG
    };
    private static final String[] BODY_PATH2 = new String[] {
            VMSG_TAG, VBODY_TAG, SUBJECT_TAG2
    };
    private static final String[] DATE_PATH2 = new String[] {
            VMSG_TAG, VBODY_TAG, DATE_TAG
    };
    private static final String[] LOCKED_PATH2 = new String[] {
            VMSG_TAG, VBODY_TAG, LOCKED_TAG2
    };
    private static final String[] SUBID_PATH2 = new String[] {
            VMSG_TAG, VBODY_TAG, SUBID_TAG2
    };

    private static final String[] OUT_BODY_PATH = new String[] {
            VMSG_TAG, VENV_TAG, VENV_TAG, VBODY_TAG
    };
    private static final String[] OUT_DATE_PATH = new String[] {
            VMSG_TAG, VENV_TAG, VENV_TAG, VBODY_TAG, DATE_TAG
    };

    private static SimpleDateFormat sdf = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");

    public ShortMessage[] parseMessage(String msg) throws ParseException {
        Log.d(TAG, "z36 msg = " + msg);
        String[] messages = msg.split("END:VMSG");
        Log.d(TAG, "z39 messages.length = " + messages.length);

        for (int i = 0; i < messages.length - 1; i++)
        {
            messages[i] = messages[i] + "END:VMSG";
            Log.d(TAG, "z44 i = " + i + ";messages[i] = " + messages[i]);
        }

        ArrayList<ShortMessage> messagesArray = new ArrayList<ShortMessage>(messages.length);
        Log.d(TAG, "z41 messagesArray.size = " + messagesArray.size());
        for (int index = 0; index < messages.length - 1; index++)
        {
            Log.d(TAG, "z60 index = " + index + ";messages[index] = " + messages[index]);

            String mStr = messages[index].replaceAll("=\r\n", "");
            Log.d(TAG, "-----> mStr = " + mStr);
            VMsgElement root = parseElement2(new VMsgTokenizer(mStr), VMSG_TAG);
            String statusRead = getPath(root, STATUS_PATH2);
            Log.d(TAG, "z67 statusRead = " + statusRead);

            String boxType = getPath(root, BOX_PATH2);
            Log.d(TAG, "z70 boxType = " + boxType);

            String from = getPath(root, FROM_PATH2);
            Log.d(TAG, "z47 from = " + from);

            String to = getPath(root, TO_PATH2);
            Log.d(TAG, "z47 to = " + to);

            String mLocked = getPath(root, LOCKED_PATH2);
            Log.d(TAG, "Locked=" + mLocked);
            int locked = 0;// LOCKED
            if (mLocked.equalsIgnoreCase("LOCKED")) {
                Log.d(TAG, "Locked=1");
                locked = 1;
            }
            String mSubId = getPath(root, SUBID_PATH2);
            int subId = 0;
            if (mSubId.equals("1")) {
                subId = 1;
            }
            String body = "";
            String dateStr = "";
            if (boxType.equalsIgnoreCase("INBOX"))
            {
                body = getPath(root, BODY_PATH2);
                dateStr = getPath(root, DATE_PATH2);
            }
            else
            {
                body = getPath(root, BODY_PATH2);
                dateStr = getPath(root, DATE_PATH2);
            }
            Log.d(TAG, "z79 body = " + body);

            Log.d(TAG, "z74 date = " + dateStr);
            Date date = null;
            if (TextUtils.isEmpty(dateStr))
            {
                date = new Date();
            }
            else
            {
                date = sdf.parse(dateStr);
                Log.d(TAG, "z72 date = " + date);
            }
            messagesArray.add(new ShortMessage(date, from, to, body, statusRead, boxType, locked,
                    subId));
        }
        Log.d(TAG, "z52 messagesArray.size = " + messagesArray.size());
        return messagesArray.toArray(new ShortMessage[0]);
    }

    public Charset getCharset() {
        return CHARSET;
    }

    private String getPath(VMsgElement elem, String[] path) throws ParseException {
        return getPath(elem, path, 1);
    }

    private String getPath(VMsgElement elem, String[] path, int i) throws ParseException {
        Log.d(TAG, "z73 elem = " + elem);
        if (elem == null)
        {
            Log.d(TAG, "z80 ");
            return "";
        }

        if (elem == null)
        {
            Log.d(TAG, "z80 ");
            throw new ParseException("Unexpected structure", 0);
        }
        Log.d(TAG, "Path[i]=" + path[i]);
        VMsgElement next = elem.data.get(path[i]);
        if (next == null)
        {
            Log.d(TAG, "z120");
            return "";
        }

        if (i < path.length - 1)
        {
            return getPath(next, path, i + 1);
        }
        else
        {
            return next.value;
        }
    }

    private void checkStartTag(VMsgTokenizer tokenizer, String tag) throws ParseException {
        VMsgTokenizer.Token token = tokenizer.current();
        String key = token.getKey();
        String value = token.getValue();
        if (!key.equals(BEGIN_MARKER) || !value.equals(tag)) {
            throw new ParseException("Missing start tag: " + BEGIN_MARKER + ":" + tag, 0);
        }
    }

    private void checkEndTag(VMsgTokenizer tokenizer, String tag) throws ParseException {
        // Same as checkStartTag.
        VMsgTokenizer.Token token = tokenizer.current();
        String key = token.getKey();
        String value = token.getValue();
        if (!key.equals(END_MARKER) || !value.equals(tag)) {
            throw new ParseException("Missing end tag: " + END_MARKER + ":" + tag, 0);
        }
    }

    private VMsgElement parseElement(VMsgTokenizer tokenizer, String tag) throws ParseException {
        VMsgElement elem = new VMsgElement(tag);

        try {
            checkStartTag(tokenizer, tag);

            // The tokenizer currently points at the start tag. Let's look at
            // the next token.
            VMsgTokenizer.Token token = tokenizer.next();
            String key = token.getKey();
            String value = token.getValue();

            while (!key.equals(END_MARKER)) {
                if (tag.equals(VBODY_TAG) && !key.equals(DATE_TAG)) {
                    // The VBODY_TAG contains both the actual message and the
                    // date it was sent, so this is a special case.
                    // Lines are separated by a newline.
                    if (elem.value.length() > 0) {
                        elem.value += "\n";
                    }
                    elem.value += token.getToken();

                } else if (key.equals(BEGIN_MARKER)) {
                    // The BEGIN_TAG starts a new segment, so it will be parsed
                    // recursively.
                    elem.data.put(value, parseElement(tokenizer, value));

                } else {
                    // All items are stored as a key/value-pair in the data map.
                    elem.data.put(key, new VMsgElement(key, value));
                }

                token = tokenizer.next();
                key = token.getKey();
                value = token.getValue();
                Log.d(TAG, "Key:" + key + " value" + value);
            }

            checkEndTag(tokenizer, tag);
        } catch (NoSuchElementException e) {
            Log.d(TAG, "z146 e = " + e);
            throw new ParseException("Error while parsing message.", 0);
        }

        return elem;
    }

    private VMsgElement parseElement2(VMsgTokenizer tokenizer, String tag) throws ParseException {
        VMsgElement elem = new VMsgElement(tag);

        try {
            checkStartTag(tokenizer, tag);

            // The tokenizer currently points at the start tag. Let's look at
            // the next token.
            VMsgTokenizer.Token token = tokenizer.next();
            String key = token.getKey();
            String value = token.getValue();

            while (!key.equals(END_MARKER)) {
                /*
                 * if (tag.equals(VBODY_TAG) && !key.equals(DATE_TAG)) { // The
                 * VBODY_TAG contains both the actual message and the // date it
                 * was sent, so this is a special case. // Lines are separated
                 * by a newline. if (elem.value.length() > 0) { elem.value +=
                 * "\n"; } elem.value += token.getToken(); } else
                 */

                if (key.equals(BEGIN_MARKER)) {
                    // The BEGIN_TAG starts a new segment, so it will be parsed
                    // recursively.
                    elem.data.put(value, parseElement2(tokenizer, value));

                } else {
                    // All items are stored as a key/value-pair in the data map.
                    elem.data.put(key, new VMsgElement(key, value));
                }

                token = tokenizer.next();
                key = token.getKey();
                value = token.getValue();
                Log.d(TAG, "Key:" + key + " value" + value);
            }

            checkEndTag(tokenizer, tag);
        } catch (NoSuchElementException e) {
            Log.d(TAG, "z146 e = " + e);
            throw new ParseException("Error while parsing message.", 0);
        }

        return elem;
    }

    private class VMsgElement {

        private Map<String, VMsgElement> data = new HashMap<String, VMsgElement>();
        public String value = "";
        public String tag;

        public VMsgElement(String tag) {
            this.tag = tag;
        }

        public VMsgElement(String tag, String value) {
            this.tag = tag;
            this.value = value;
        }

        public void printData()
        {

        }
    }
}
