/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 *  common.c : Implementations of commonly used functions.
 *
 */
#include "common.h"
#include "log.h"

const char* path_to_save_files = "./";

FILE* open_file (const char *filename, boolean for_reading)
{
    FILE* fp = NULL;
    char full_file_path[MAX_FILE_NAME_LENGTH];

    if (NULL == filename) {
        logmsg (LOG_ERROR, "Invalid filename");
        goto return_open_file;
    }

    if (FALSE == for_reading) {
        if (strlcpy (full_file_path, path_to_save_files, sizeof(full_file_path)) >= sizeof(full_file_path)) {
            logmsg (LOG_ERROR, "String was truncated while copying");
            goto return_open_file;
        }
        if (strlcat (full_file_path, filename, sizeof(full_file_path)) >= sizeof (full_file_path)) {
            logmsg (LOG_ERROR, "String was truncated while concatenating");
            goto return_open_file;
        }
    }

    logmsg (LOG_INFO, "Opening file '%s'", full_file_path);
    /* Open the file */
    fp = fopen (full_file_path, /*O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH*/ for_reading == TRUE ? "rb" : "wb");

    if (NULL == fp) {
        logmsg (LOG_ERROR, "Unable to open file '%s'. Error %d: %s",
            full_file_path,
            errno,
            strerror (errno));
    }

return_open_file:
    return fp;
}

boolean close_file (FILE *fp)
{
    if (fclose (fp))
    {
        logmsg (LOG_ERROR, "Unable to close fd: %d: System error code %s", fp, strerror(errno));
        return FALSE;
    }
   return TRUE;
}

#if defined(LINUXPC) || defined(WINDOWSPC)
size_t strlcpy(char *dst, const char *src, size_t size) {
    size_t i = 0;
    while (i < size && ((dst[i] = src[i]) != '\0'))
        i++;
    if (i == size && size != 0)
        dst[i] = '\0';
    while (src[i])
        i++;
    return i;
}

size_t strlcat(char *dst, const char *src, size_t size) {
    size_t i = 0, j = 0;
    /* Iterate through till
     * 1. the end of the string in dst, or
     * 2. the end of the buffer indicated by size
     */
    while (i < size && dst[i] != '\0')
        i++;

    /* IF we reach the end of the buffer space without having encountered a
     * NULL, OR
     * IF we reach the end of the string in dst and it turns out that
     * taking the NULL-termination into account there's no more space left,
     * THEN just do the simple calculation and return
     */
    if (i == size || (i == size - 1 && dst[i] == '\0'))
        return i+strlen(src);

    /* Iterate only till size - 1 so as to leave space for the NULL */
    while (i < size - 1 && ((dst[i] = src[j]) != '\0')) {
        i++; j++;
    }

    /* If we exited the loop not due to NULL-termination, then NULL terminate
     * the destination
     */
    if (i == size - 1) {
        i++;
        dst[i] = '\0';
    }
    return i;
}
#endif
