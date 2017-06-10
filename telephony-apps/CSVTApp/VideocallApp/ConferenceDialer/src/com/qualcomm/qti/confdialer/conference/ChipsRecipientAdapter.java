/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution. Apache license notifications and license are retained
 * for attribution purposes only.
 *
 */

/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 *
 * Copyright (C) 2012 The Android Open Source Project
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

package com.qualcomm.qti.confdialer.conference;

import android.content.Context;

import com.android.ex.chips.BaseRecipientAdapter;
import com.qualcomm.qti.confdialer.R;
public class ChipsRecipientAdapter extends BaseRecipientAdapter {
    private static final int DEFAULT_PREFERRED_MAX_RESULT_COUNT = 10;

    public ChipsRecipientAdapter(Context context) {
        // The Chips UI is email-centric by default. By setting QUERY_TYPE_PHONE, the chips UI
        // will operate with phone numbers instead of emails.
        super(context, DEFAULT_PREFERRED_MAX_RESULT_COUNT, QUERY_TYPE_PHONE);
    }

    /**
     * Returns a layout id for each item inside auto-complete list.
     *
     * Each View must contain two TextViews (for display name and destination) and one ImageView
     * (for photo). Ids for those should be available via {@link #getDisplayNameId()},
     * {@link #getDestinationId()}, and {@link #getPhotoId()}.
     */
    protected int getItemLayout() {
        return R.layout.conference_chips_recipient_dropdown_item;
    }

}
