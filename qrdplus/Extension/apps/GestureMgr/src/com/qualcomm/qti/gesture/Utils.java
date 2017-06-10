/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.gesture;

import java.lang.reflect.Field;
import java.util.ArrayList;

import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.gesture.Gesture;
import android.gesture.GesturePoint;
import android.gesture.GestureStroke;
import android.graphics.Path;
import android.graphics.drawable.Drawable;

public class Utils {

    private static Context loadSettingsContext(Context context) {
        try {
            return context.createPackageContext("com.android.settings",
                    Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
        } catch (NameNotFoundException e) {
        }
        return null;
    }

    public static Drawable loadSettingsIcon(Context context) {
        Context settingsContext = loadSettingsContext(context);
        if (settingsContext == null) {
            return null;
        }
        return settingsContext.getApplicationInfo().loadIcon(settingsContext.getPackageManager());
    }

    public static Gesture revertGesture(Gesture gesture) {
        ArrayList<GestureStroke> strokes = gesture.getStrokes();
        Gesture gestureRevert = new Gesture();
        for (int i = strokes.size() - 1; i >= 0; i--) {
            GestureStroke stroke = strokes.get(i);
            if (stroke == null) {
                gestureRevert.addStroke(null);
                continue;
            }
            ArrayList<GesturePoint> buffer = new ArrayList<GesturePoint>();
            long[] timestamps = getStrokeTimestamps(stroke);
            for (int j = stroke.points.length / 2 - 1; j >= 0; j--) {
                buffer.add(new GesturePoint(stroke.points[j * 2], stroke.points[j * 2 + 1],
                        timestamps == null ? System.currentTimeMillis()
                                : timestamps[stroke.points.length / 2 - 1 - j]));
            }
            gestureRevert.addStroke(new GestureStroke(buffer));
        }
        return gestureRevert;
    }

    public static Gesture createRoundGesture() {
        int r = 500;
        ArrayList<GesturePoint> mStrokeBuffer = new ArrayList<GesturePoint>();
        for (double angle = 0; angle < 2 * Math.PI; angle += 0.01) {
            mStrokeBuffer.add(new GesturePoint((int) (r - r * Math.sin(angle)), (int) (r - r
                    * Math.cos(angle)), System.currentTimeMillis()));
        }
        GestureStroke gestureStroke = new GestureStroke(mStrokeBuffer);
        Gesture gesture = new Gesture();
        gesture.addStroke(gestureStroke);
        return gesture;
    }

    public static Gesture createHalfRoundGesture() {
        int r = 500;
        ArrayList<GesturePoint> mStrokeBuffer = new ArrayList<GesturePoint>();
        for (double angle = -Math.PI / 6; angle < Math.PI + Math.PI / 6; angle += 0.01) {
            mStrokeBuffer.add(new GesturePoint((int) (r - r * Math.sin(angle)), (int) (r - r
                    * Math.cos(angle)), System.currentTimeMillis()));
        }
        GestureStroke gestureStroke = new GestureStroke(mStrokeBuffer);
        Gesture gesture = new Gesture();
        gesture.addStroke(gestureStroke);
        return gesture;
    }

    public static boolean isGestureSame(Gesture src, Gesture target) {
        if (src == null || target == null) {
            return false;
        }
        if (src.getStrokesCount() != target.getStrokesCount()) {
            return false;
        }
        for (int index = 0; index < src.getStrokesCount(); index++) {
            float[] ponitsTarget = target.getStrokes().get(index).points;
            float[] ponitsSrc = src.getStrokes().get(index).points;
            if (ponitsSrc == null && ponitsTarget != null) {
                return false;
            }
            if (ponitsSrc != null && ponitsTarget == null) {
                return false;
            }
            if (ponitsSrc.length != ponitsTarget.length) {
                return false;
            }
            for (int position = 0; position < ponitsSrc.length; position++) {
                if (ponitsSrc[position] != ponitsTarget[position]) {
                    return false;
                }
            }
        }
        return true;
    }

    public static Gesture createLeftArrow() {
        ArrayList<GesturePoint> mStrokeBuffer = new ArrayList<GesturePoint>();
        int r = 500;
        for (int index = 0; index < 2 * r; index++) {
            if (index <= r) {
                mStrokeBuffer.add(new GesturePoint(2 * (r - index), index, System
                        .currentTimeMillis()));
            } else {
                mStrokeBuffer.add(new GesturePoint(2 * (index - r), index, System
                        .currentTimeMillis()));
            }
        }
        GestureStroke gestureStroke = new GestureStroke(mStrokeBuffer);
        Gesture gesture = new Gesture();
        gesture.addStroke(gestureStroke);
        return gesture;
    }

    public static Gesture createRightArrow() {
        ArrayList<GesturePoint> mStrokeBuffer = new ArrayList<GesturePoint>();
        int r = 500;
        for (int index = 0; index < 2 * r; index++) {
            if (index <= r) {
                mStrokeBuffer.add(new GesturePoint(2 * index, index, System.currentTimeMillis()));
            } else {
                mStrokeBuffer.add(new GesturePoint(2 * r - 2 * (index - r), index, System
                        .currentTimeMillis()));
            }
        }
        GestureStroke gestureStroke = new GestureStroke(mStrokeBuffer);
        Gesture gesture = new Gesture();
        gesture.addStroke(gestureStroke);
        return gesture;
    }

    public static Gesture createBottomArrow() {
        ArrayList<GesturePoint> mStrokeBuffer = new ArrayList<GesturePoint>();
        int r = 500;
        for (int index = 0; index < 2 * r; index++) {
            if (index <= r) {
                mStrokeBuffer.add(new GesturePoint(index, index * 2, System.currentTimeMillis()));
            } else {
                mStrokeBuffer.add(new GesturePoint(index, 2 * r - (index - r) * 2, System
                        .currentTimeMillis()));
            }
        }
        GestureStroke gestureStroke = new GestureStroke(mStrokeBuffer);
        Gesture gesture = new Gesture();
        gesture.addStroke(gestureStroke);
        return gesture;
    }

    public static Gesture createDoubleUpArrow() {
        ArrayList<GesturePoint> mStrokeBuffer = new ArrayList<GesturePoint>();
        int r = 250;
        for (int index = 0; index < 4 * r; index++) {
            if (index <= r) {
                mStrokeBuffer.add(new GesturePoint(index, 4 * r - 4 * index, System
                        .currentTimeMillis()));
            } else if (index <= 2 * r) {
                mStrokeBuffer.add(new GesturePoint(index, 4 * (index - r), System
                        .currentTimeMillis()));
            } else if (index <= 3 * r) {
                mStrokeBuffer.add(new GesturePoint(index, 4 * r - 4 * (index - 2 * r), System
                        .currentTimeMillis()));
            } else {
                mStrokeBuffer.add(new GesturePoint(index, 4 * (index - 3 * r), System
                        .currentTimeMillis()));
            }
        }
        GestureStroke gestureStroke = new GestureStroke(mStrokeBuffer);
        Gesture gesture = new Gesture();
        gesture.addStroke(gestureStroke);
        return gesture;
    }

    public static GestureStroke copyStroke(GestureStroke gestureStroke, int length) {
        long[] timestamps = getStrokeTimestamps(gestureStroke);
        ArrayList<GesturePoint> strokeBuffer = new ArrayList<GesturePoint>();
        for (int index = 0; index < length; index++) {
            strokeBuffer.add(new GesturePoint(gestureStroke.points[index * 2],
                    gestureStroke.points[index * 2 + 1], timestamps == null ? System
                            .currentTimeMillis() : timestamps[index]));
        }
        return new GestureStroke(strokeBuffer);
    }

    public static Path getGesturePath(Gesture gesture, int length) {
        Path path = new Path();
        ArrayList<GestureStroke> strokes = gesture.getStrokes();
        for (GestureStroke stroke : strokes) {
            if (stroke.points == null) {
                continue;
            }
            if (length >= stroke.points.length / 2) {
                path.addPath(stroke.getPath());
                length -= stroke.points.length / 2;
            } else if (length > 0) {
                GestureStroke temp = copyStroke(stroke, length);
                path.addPath(temp.getPath());
                return path;
            }
        }
        return path;
    }

    public static int getGestureLength(Gesture gesture) {
        int length = 0;
        for (GestureStroke stroke : gesture.getStrokes()) {
            if (stroke != null) {
                length += stroke.points.length / 2;
            }
        }
        return length;
    }

    private static long[] getStrokeTimestamps(GestureStroke stroke) {
        long[] timestamps = null;
        try {
            Field field = GestureStroke.class.getDeclaredField("timestamps");
            field.setAccessible(true);
            timestamps = (long[]) field.get(stroke);
        } catch (NoSuchFieldException e) {
        } catch (IllegalAccessException e) {
        } catch (IllegalArgumentException e) {
        }
        return timestamps;
    }
}
