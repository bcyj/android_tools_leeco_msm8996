/**
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */

package com.qti.homelocation;

import android.content.SearchRecentSuggestionsProvider;

public class SearchSuggestionProvider extends SearchRecentSuggestionsProvider {

    public final static String AUTHORITY = "geocoded_location_suggestion";

    public SearchSuggestionProvider() {
        super();
        setupSuggestions(AUTHORITY, DATABASE_MODE_QUERIES);
    }
}
