/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.util;

import android.annotation.SuppressLint;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory.Options;
import android.graphics.PorterDuff.Mode;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.util.LruCache;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

public class AsynImageLoader {

    private static LruCache<String, Bitmap> caches;

    private List<LoaderImageTask> taskQueue;

    private boolean isRuning = false;

    private boolean isFirst = true;

    private static final int CACHE_SIZE = (int)(Runtime.getRuntime().maxMemory() / 8);

    public interface ImageCallback {
        public void loadImageCallback(Bitmap bitmap);
    }

    public void onDestroy() {
        taskQueue.clear();
        caches.evictAll();
    }

    static {
        caches = new LruCache<String, Bitmap>(CACHE_SIZE) {
            @Override
            protected int sizeOf(String key, Bitmap value) {
                return value.getRowBytes() * value.getHeight();
            }
        };
    }

    public AsynImageLoader() {
        taskQueue = new ArrayList<AsynImageLoader.LoaderImageTask>();
        isRuning = true;
        new Thread(runnable).start();
    }

    public Bitmap loadImageAsynByLocalPath(String absFilePath) {
        if (TextUtils.isEmpty(absFilePath))
            return null;
        Bitmap bitmap = getBitmapFromMemCache(absFilePath);
        if (bitmap != null) {
            return bitmap;
        } else {
            try {
                File imageFile = new File(absFilePath);
                if (imageFile.exists()) {
                    Options opts = new Options();
                    opts.inPurgeable = true;
                    bitmap = BitmapFactory.decodeFile(absFilePath, opts);
                    if (bitmap != null) {
                        addBitmapToMemoryCache(absFilePath, bitmap);
                        return bitmap;
                    } else {
                        return null;
                    }
                }
            } catch (Exception e) {
                Log.e("Exception", "fail to create bitmap: " + absFilePath);
            } catch (OutOfMemoryError e) {
                Log.e("Exception", "OutOfMemoryError, fail to create bitmap: " + absFilePath);
            }
        }
        return null;
    }

    public void loadImageAsynByUrl(LoaderImageTask loaderImageTask, ImageCallback callback) {
        Bitmap bitmap = null;
        bitmap = getBitmapFromMemCache(loaderImageTask.path);
        if (bitmap != null) {
            callback.loadImageCallback(bitmap);
        } else if (CommonUtil.isFileExistsByUrl(CommonUtil.FILE_TYPE_IMAGE, loaderImageTask.path)) {
            String imagePath = CommonUtil.getFileCacheLocalPath(CommonUtil.FILE_TYPE_IMAGE,
                    loaderImageTask.path);
            bitmap = loadImageAsynByLocalPath(imagePath);
            if (bitmap != null) {
                callback.loadImageCallback(bitmap);
            }
        } else {
            loaderImageTask.callback = callback;
            if (!taskQueue.contains(loaderImageTask)) {
                taskQueue.add(loaderImageTask);
                synchronized (runnable) {
                    runnable.notify();
                }
            }
        }
    }

    @SuppressLint("HandlerLeak")
    public void loadBitmapByRawContactId(final Context context, final String rawContactId,
            final ImageCallback imageCallback) {
        if (TextUtils.isEmpty(rawContactId)) {
            return;
        }

        Bitmap bitmap = getBitmapFromMemCache(rawContactId);
        if (bitmap != null) {
            imageCallback.loadImageCallback(bitmap);
        }
        final LoaderImageTask task = new LoaderImageTask();
        task.callback = imageCallback;
        new Thread() {
            @Override
            public void run() {
                Bitmap bitmap = getMyProfilePhotoOnData(context, rawContactId);
                if (bitmap != null) {
                    bitmap = generateBitmapRoundCorner(bitmap, 10);
                    task.bitmap = bitmap;
                    Message msg = handler.obtainMessage();
                    msg.obj = task;
                    handler.sendMessage(msg);
                }
            }
        }.start();
    }

    private Bitmap getMyProfilePhotoOnData(Context context, String rawContactId) {
        Bitmap bitmap = null;
        Uri uri = Uri.parse("content://com.android.contacts/profile/data/");
        Cursor cursor = context.getContentResolver().query(uri, new String[] {
                "_id", "mimetype", "data15"
        }, " raw_contact_id = ?  AND mimetype = ? ", new String[] {
                rawContactId, "vnd.android.cursor.item/photo"
        }, null);
        try {
            if (cursor != null && cursor.moveToFirst()) {
                byte[] data = cursor.getBlob(cursor.getColumnIndexOrThrow("data15"));
                bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return bitmap;
    }

    private static Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            LoaderImageTask task = (LoaderImageTask)msg.obj;
            task.callback.loadImageCallback(task.bitmap);
        }
    };

    private Runnable runnable = new Runnable() {
        @Override
        public void run() {
            while (isRuning) {
                while (taskQueue.size() > 0) {
                    LoaderImageTask task = taskQueue.remove(0);
                    task.bitmap = getbitmap(task.path, task.isRoundCorner, task.isZoomImage,
                            task.isCacheLocal);
                    if (task.isCacheMemory) {
                        addBitmapToMemoryCache(task.path, task.bitmap);
                    }
                    if (handler != null) {
                        Message msg = handler.obtainMessage();
                        msg.obj = task;
                        handler.sendMessage(msg);
                    }
                    synchronized (this) {
                        try {
                            this.wait();
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                }
                if (isFirst ) {
                    synchronized (this) {
                        try {
                            this.wait();
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                    isFirst = false;
                }
            }
        }
    };

    private Bitmap getbitmap(String imageUri, boolean isRoundCorner, boolean isZoomImage,
            boolean isCacheLocal) {
        InputStream is = null;
        HttpURLConnection conn = null;
        try {
            URL myFileUrl = new URL(imageUri);
            conn = (HttpURLConnection)myFileUrl.openConnection();
            conn.setRequestMethod("GET");
            conn.setReadTimeout(6 * 1000);
            conn.setDoInput(true);
            conn.connect();
            if (conn.getResponseCode() == 200) {
                is = conn.getInputStream();
                byte[] data = readStream(is);
                if (isCacheLocal) {
                    saveDataToLocal(imageUri, data);
                }
                
                Bitmap bitmap = null;
                if (isZoomImage) {
                    bitmap = generateCompressBitmap(data);
                } else {
                    if (data != null) {
                        bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
                    }
                }
                if (bitmap != null && isRoundCorner) {
                    bitmap = generateBitmapRoundCorner(bitmap, bitmap.getWidth() / 2);
                    if (bitmap != null)
                        return bitmap;
                } else {
                    return bitmap;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }finally {
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (conn != null) {
                conn.disconnect();
            }
        }
        return null;
    }

    private byte[] readStream(InputStream is) {
        ByteArrayOutputStream outStream = null;
        try {
            outStream = new ByteArrayOutputStream();
            byte[] buffer = new byte[1024];
            int len = -1;
            while ((len = is.read(buffer)) != -1) {
                outStream.write(buffer, 0, len);
            }
            return outStream.toByteArray();
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        } finally {
            if (outStream != null) {
                try {
                    outStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void saveDataToLocal(String imageUrl, byte[] data) {
        ByteArrayInputStream is = new ByteArrayInputStream(data);
        FileOutputStream fos = null;
        try {
            String filePath = getSaveFilePath(imageUrl);
            fos = new FileOutputStream(filePath);
            int bufferSize = 1024;
            byte[] buffer = new byte[bufferSize];
            int length = -1;
            while ((length = is.read(buffer)) != -1) {
                fos.write(buffer, 0, length);
            }
            fos.flush();
        } catch (FileNotFoundException e1) {
            e1.printStackTrace();
        } catch (IOException e2) {
            e2.printStackTrace();
        } catch (Exception e3) {
            e3.printStackTrace();
        } finally {
            if (fos != null) {
                try {
                    fos.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private String getSaveFilePath(String imageUrl) {
        String absFilePath = CommonUtil.getFileCacheLocalPath(CommonUtil.FILE_TYPE_IMAGE,
                imageUrl);
        if (absFilePath != null) {
            File file = new File(absFilePath);
            File dirFile = file.getParentFile();
            if (dirFile != null && !dirFile.exists()) {
                dirFile.mkdirs();
            }
        }
        return absFilePath;
    }

    private Bitmap generateCompressBitmap(byte[] data) {
        if (data != null) {
            try {
                final BitmapFactory.Options opts = new BitmapFactory.Options();
                opts.inPurgeable = true;
                opts.inJustDecodeBounds = true;
                BitmapFactory.decodeByteArray(data, 0, data.length, opts);
                opts.inJustDecodeBounds = false;
                opts.inSampleSize = computeSampleSize(opts, 160, 200);
                return BitmapFactory.decodeByteArray(data, 0, data.length, opts);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return null;
    }

    private Bitmap generateBitmapRoundCorner(Bitmap bitmap, int roundCornerPx) {
        if (bitmap == null)
            return Bitmap.createBitmap(10, 10, Config.ARGB_8888);
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        Bitmap output = Bitmap
                .createBitmap(width, height, Config.ARGB_8888);
        Canvas canvas = new Canvas(output);
        final Rect rect = new Rect(0, 0, width, height);
        final RectF rectF = new RectF(rect);
        final float rPx = roundCornerPx;
        final Paint paint = new Paint();
        paint.setAntiAlias(true);
        canvas.drawARGB(0, 0, 0, 0);
        final int color = 0xff424242;
        paint.setColor(color);
        canvas.drawRoundRect(rectF, rPx, rPx, paint);
        paint.setXfermode(new PorterDuffXfermode(Mode.SRC_IN));
        canvas.drawBitmap(bitmap, rect, rect, paint);
        return output;
    }

    public static class LoaderImageTask {

        String path;

        Bitmap bitmap;

        ImageCallback callback;

        boolean isRoundCorner = false;

        boolean isZoomImage = false;

        boolean isCacheLocal = false;

        boolean isCacheMemory = false;

        public LoaderImageTask() {
        }

        public LoaderImageTask(String pathUrl, boolean isRoundCorner, boolean isZoomImage,
                boolean isCacheMemory, boolean isCacheLocal) {
            this.path = pathUrl;
            this.isCacheLocal = isCacheLocal;
            this.isCacheMemory = isCacheMemory;
            this.isRoundCorner = isRoundCorner;
            this.isZoomImage = isZoomImage;
        }

        @Override
        public boolean equals(Object o) {
            LoaderImageTask task = (LoaderImageTask)o;
            return task.path.equals(path);
        }
    }

    public void addBitmapToMemoryCache(String path, Bitmap bitmap) {
        if (path != null) {
            if (getBitmapFromMemCache(path) == null && bitmap != null) {
                caches.put(path, bitmap);
            }
        }
    }

    public Bitmap getBitmapFromMemCache(String path) {
        if (path != null) {
            return caches.get(path);
        }
        return null;
    }

    private int computeSampleSize(BitmapFactory.Options options, int reqHeight, int reqWidth) {
        int sampleSize = 1;
        final int height = options.outHeight;
        final int width = options.outWidth;
        if (width > reqWidth || height > reqHeight) {
            final int wRatio = Math.round((float)width / (float)reqWidth);
            final int hRatio = Math.round((float)height / (float)reqHeight);
            sampleSize = hRatio < wRatio ? hRatio : wRatio;
        }
        return sampleSize;
    }
}
