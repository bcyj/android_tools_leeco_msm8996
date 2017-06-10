/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.setupwizard.data;

import java.util.ArrayList;

public class PageList extends ArrayList<Page> implements PageNode {

    public PageList(Page... pages) {
        for (Page page : pages) {
            add(page);
        }
    }

    @Override
    public Page findPage(String key) {
        for (Page childPage : this) {
            Page found = childPage.findPage(key);
            if (found != null) {
                return found;
            }
        }
        return null;
    }

    @Override
    public Page findPage(int id) {
        for (Page childPage : this) {
            Page found = childPage.findPage(id);
            if (found != null) {
                return found;
            }
        }
        return null;
    }

}
