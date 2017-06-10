/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */
/*
 * Copyright (C) 2007 The Android Open Source Project
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

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.widget.EditText;

/**
 * Defines a custom EditText View that draws lines between each line of text
 * that is displayed.
 */
public class NewEditText extends EditText {

    private Rect mRect;
    private Paint mPaint;
    private static final float MULT_HEIGHT = 1.0f;

    // This constructor is used by LayoutInflater
    public NewEditText(Context context, AttributeSet attrs) {
        super(context, attrs);

        // Creates a Rect and a Paint object, and sets the style and color of
        // the Paint object.
        mRect = new Rect();
        mPaint = new Paint();
        mPaint.setStyle(Paint.Style.STROKE);
        mPaint.setColor(0x800000FF);
        this.setLineSpacing(1.5f, MULT_HEIGHT);
    }

    /**
     * This is called to draw the NewEditText object
     *
     * @param canvas The canvas on which the background is drawn.
     */
    @Override
    protected void onDraw(Canvas canvas) {

        // Gets the number of lines of text in the View.
        int count = getLineCount();

        // Gets the global Rect and Paint objects
        Rect r = mRect;
        Paint paint = mPaint;

        /*
         * Draws one line in the rectangle for every line of text in the
         * EditText
         */
        for (int i = 0; i < count; i++) {

            // Gets the baseline coordinates for the current line of text
            getLineBounds(i, r);
            int baseline = (i + 1) * getLineHeight();

            /*
             * Draws a line in the background from the left of the rectangle to
             * the right, at a vertical position one dip below the baseline,
             * using the "paint" object for details.
             */
            canvas.drawLine(r.left, baseline + getPaddingTop() + 5, r.right, baseline + getPaddingTop() + 5, paint);
        }

        // Finishes up by calling the parent method
        super.onDraw(canvas);
    }

}
