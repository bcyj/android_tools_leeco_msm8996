/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

/*========================================================================
  Include Files
 ==========================================================================*/
#include "vt_file.h"
#include "vt_debug.h"
#include <stdio.h>

#define  ALIGN(val, grid) ((!(grid)) ? (val) : \
        ((((val) + (grid) - 1) / (grid)) * (grid)))

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int vt_file_open(void **handle, char *filename, int readonly) {

    int result = 0;
    FILE *f = NULL;

    if (handle != NULL && filename != NULL && (readonly == 0 || readonly == 1)) {
        if (readonly == 1) {
            f = fopen(filename, "rb");
        } else {
            f = fopen(filename, "wb");
        }

        if (f == NULL) {
            VTEST_MSG_ERROR("Unable to open file");
            result = 1;
        }

        *handle = f;
    } else {
        VTEST_MSG_ERROR("bad param");
        result = 1;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int vt_file_close(void *handle) {

    int result = 0;

    if (handle != NULL) {
        fclose((FILE *)handle);
    } else {
        VTEST_MSG_ERROR("handle is null");
        result = 1;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int vt_file_read(void *handle, void *buffer, int width, int height, int config) {

    int bytes = width;

    int result = 0;

    if (buffer != NULL) {
        if (bytes > 0) {
            if (!config) {
                char *yuv = (char *)buffer;
                int i, lscanl, lstride, cscanl, cstride;

                lstride = VENUS_Y_STRIDE(COLOR_FMT_NV12, width);
                lscanl = VENUS_Y_SCANLINES(COLOR_FMT_NV12, height);
                cstride = VENUS_UV_STRIDE(COLOR_FMT_NV12, width);
                cscanl = VENUS_UV_SCANLINES(COLOR_FMT_NV12, height);
                for (i = 0; i < height; i++) {
                    result += (int)fread(yuv, 1, width, (FILE *)handle);
                    yuv += lstride;
                }
                yuv = (char *)buffer + (lscanl * lstride);
                for (i = 0; i < ((height + 1) >> 1); i++) {
                    result += (int)fread(yuv, 1, width, (FILE *)handle);
                    yuv += cstride;
                }
                buffer = ((char *)buffer) + VENUS_BUFFER_SIZE(COLOR_FMT_NV12, width, height);
            } else {
                result = (int)fread(buffer, 1, width, (FILE *)handle);
            }
        } else {
            VTEST_MSG_ERROR("Bytes must be > 0");
            result = -1;
        }
    } else {
        VTEST_MSG_ERROR("Null param");
        result = -1;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int vt_file_write(void *handle, void *buffer, int bytes) {

    int result = 0;

    if (buffer != NULL) {
        if (bytes > 0) {
            result = (int)fwrite(buffer, 1, bytes, (FILE *)handle);
        } else {
            VTEST_MSG_ERROR("Bytes must be > 0");
            result = -1;
        }
    } else {
        VTEST_MSG_ERROR("Null param");
        result = -1;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int vt_file_seek_start(void *handle, int bytes) {

    int result = 0;

    if (bytes >= 0) {
        if (fseek((FILE *)handle, bytes, SEEK_SET) != 0) {
            VTEST_MSG_ERROR("failed to seek");
            result = 1;
        }
    }
    return result;
}
