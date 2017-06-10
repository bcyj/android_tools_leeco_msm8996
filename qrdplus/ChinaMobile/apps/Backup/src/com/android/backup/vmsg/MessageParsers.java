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

import java.io.File;
import java.io.FilenameFilter;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

public class MessageParsers {

    private static final Map<String, MessageParser> PARSERS = new HashMap<String, MessageParser>();
    private static final FilenameFilter FILENAME_FILTER = new FilenameFilter() {

        public boolean accept(File dir, String name) {
            for (String ext : PARSERS.keySet()) {
                if (name.toLowerCase().endsWith(ext)) {
                    return true;
                }
            }
            return false;
        }
    };

    // Initialisation of parsers.
    static {
        PARSERS.put(".vmsg", new VMsgParser());
    }

    private MessageParsers() {
    }

    /**
     * @return
     */
    public static FilenameFilter getFilenameFilter() {
        return FILENAME_FILTER;
    }

    /**
     * @return
     */
    public static Map<String, MessageParser> getParserMap() {
        return Collections.unmodifiableMap(PARSERS);
    }

    /**
     * @param path
     * @return
     */
    public static MessageParser getParser(File path) {
        return getParser(path.getName());
    }

    /**
     * @param filename
     * @return
     */
    public static MessageParser getParser(String filename) {
        String fn = filename.toLowerCase();
        for (Map.Entry<String, MessageParser> entry : PARSERS.entrySet()) {
            if (fn.endsWith(entry.getKey())) {
                return entry.getValue();
            }
        }
        return null;
    }
}
