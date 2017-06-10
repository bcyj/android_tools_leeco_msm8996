/*
 * Copyright (c) 2011-2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qrd.wappush;

import android.net.Uri;
import java.io.IOException;
import org.xml.sax.SAXException;
import android.content.Context;
import java.io.InputStream;

public interface IWapPushHandler {
    // handle wap push function
    public Uri handleWapPush(InputStream inputstream, String mime,
            Context context, int slotID, String address)
        throws SAXException, IOException;

    // get the wap push thread ID
    public long getThreadID();
}
