/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.ui.adapter;

import com.suntek.mway.rcs.publicaccount.R;

import java.util.ArrayList;
import java.util.List;

import android.content.Context;

public class SelectorAdapter extends ListIconAdapter {
    
    public static class ListItem extends ListIconAdapter.ListIconItem {
        private int mCommand;

        public ListItem(String title, int resource, int command) {
            super(resource, title);
            mCommand = command;
        }

        public int getCommand() {
            return mCommand;
        }
    }
    
    public final static int PA_ADD_MAP = 7;

    public final static int PA_ADD_CONTACT_VCARD = 6;

    public final static int PA_RECORD_SOUND = 5;

    public final static int PA_ADD_SOUND = 4;

    public final static int PA_RECORD_VIDEO = 3;

    public final static int PA_ADD_VIDEO = 2;

    public final static int PA_TAKE_PICTURE = 1;

    public final static int PA_ADD_IMAGE = 0;

    private static int mMediaCount = 0;

    public SelectorAdapter(Context context) {
        super(context, getData(context));
    }

    protected static List<ListIconItem> getData(Context context) {
        List<ListIconItem> data = new ArrayList<ListIconItem>(7);
        mMediaCount = 0;
        addListItem(data, context.getString(R.string.rcs_attach_image), R.drawable.local_image,
                PA_ADD_IMAGE);

        addListItem(data, context.getString(R.string.rcs_attach_take_photo), R.drawable.take_picture,
                PA_TAKE_PICTURE);

        addListItem(data, context.getString(R.string.rcs_attach_video), R.drawable.local_video,
                PA_ADD_VIDEO);

        addListItem(data, context.getString(R.string.rcs_attach_record_video), R.drawable.record_video,
                PA_RECORD_VIDEO);

        addListItem(data, context.getString(R.string.rcs_attach_sound), R.drawable.local_audio,
                PA_ADD_SOUND);

        addListItem(data, context.getString(R.string.rcs_attach_record_sound), R.drawable.record_audio,
                PA_RECORD_SOUND);

        addListItem(data, context.getString(R.string.rcs_attach_add_vcard), R.drawable.contact_vcard,
                PA_ADD_CONTACT_VCARD);

        addListItem(data, context.getString(R.string.rcs_attach_map), R.drawable.location_message,
                PA_ADD_MAP);

        return data;
    }

    @Override
    public int getCount() {
        return mMediaCount;
    }
    
    public int buttonToCommand(int whichButton) {
        ListItem item = (ListItem)getItem(whichButton);
        return item.getCommand();
    }

    protected static void addListItem(List<ListIconItem> data, String title, int resource, int command) {
        ListItem temp = new ListItem(title, resource, command);
        data.add(temp);
        mMediaCount++;
    }

}
