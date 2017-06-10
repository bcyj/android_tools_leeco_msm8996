/*===========================================================================
                           CustomHoverIcon.java

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.snapdragon.sdk.digitalpen;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.drawable.Drawable;
import android.os.RemoteException;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

/**
 * This class allows setting a custom hovering cursor for an Android View.
 * <p>
 * After creating an object, register a listener to the View using
 * <code>View.setOnHoverListener</code> and call
 * {@link #onHover(View, MotionEvent)} from within that callback. from within
 * that callback.
 *
 * @see android.view.View
 */
public class CustomHoverIcon
{
    private static final String TAG = "CustomHoverIcon";

    private static Method setCustomHoverIconMethod;

    private Bitmap bitmap;
    private int hotSpotX;
    private int hotSpotY;

    /**
     * Specify a custom hover cursor for a view. Specify a hot-spot other than
     * (0,0) for cursors with natural "pointing spot" not at the top-left
     * corner. For example, a circle cursor will have a hot-spot at the center
     * of the bitmap
     *
     * @param drawable The hovering cursor drawable
     * @param hotSpotX x location of hot-spot for cursor offset adjustment
     * @param hotSpotY y location of hot-spot for cursor offset adjustment
     */
    public CustomHoverIcon(Drawable drawable, int hotSpotX, int hotSpotY) {
        this.bitmap = extractBitmapFromDrawable(drawable);
        this.hotSpotX = hotSpotX;
        this.hotSpotY = hotSpotY;
    }

    /**
     * Call from your a view's registered OnHoverListener.onHover event to
     * change the cursor as it enters and leaves the view's area.
     *
     * @param view the view argument from the OnHoverListener.onHover call
     * @param event the event argument from the OnHoverListener.onHover call
     */
    public void onHover(View view, MotionEvent event)
    {
        if (view == null) {
            throw new IllegalArgumentException("view is null.");
        }

        try
        {
            switch (event.getAction())
            {
                case MotionEvent.ACTION_HOVER_ENTER:
                    setCustomHoverIcon(bitmap, hotSpotX, hotSpotY);
                    break;
                case MotionEvent.ACTION_HOVER_EXIT:
                    setCustomHoverIcon(null, 0, 0);
                    break;
            }
        } catch (RemoteException e)
        {
            e.printStackTrace();
        }

    }

    /**
     * a utility method to extract bitmap from drawable
     */
    private Bitmap extractBitmapFromDrawable(Drawable d)
    {
        Bitmap bitmap = Bitmap.createBitmap(d.getIntrinsicWidth(), d.getIntrinsicHeight(),
                Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        d.setBounds(0, 0, d.getIntrinsicWidth(), d.getIntrinsicHeight());
        d.draw(canvas);
        return bitmap;
    }

    static {
        try
        {
            Class<?> pointerIconClass = Class.forName("android.view.PointerIcon");
            setCustomHoverIconMethod = pointerIconClass.getMethod("setCustomHoverIcon",
                    Bitmap.class, int.class, int.class);
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }
    }

    /**
     * Check if the framework supports custom hovering cursor
     *
     * @return true if feature is supported
     */
    public static boolean checkSupport()
    {
        return setCustomHoverIconMethod != null;
    }

    /**
     * actually changes the shape of the hovering cursor globally. called
     * regularly by the OnHover listener when hover enters and exits the view
     */
    private void setCustomHoverIcon(Bitmap bitmap, int hotSpotX, int hotSpotY)
            throws RemoteException
    {
        if (!checkSupport()) {
            Log.e(TAG, "setCustomHoverIcon failed, not supported in the framework");
            return;
        }

        try
        {
            setCustomHoverIconMethod.invoke(null, bitmap, hotSpotX, hotSpotY);
        } catch (InvocationTargetException e) {
            e.printStackTrace();
            if (e.getCause() instanceof RemoteException) {
                throw (RemoteException) e.getCause();
            }
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
    }
}
