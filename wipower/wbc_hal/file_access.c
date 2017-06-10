/*=========================================================================
  file_access.c
  DESCRIPTION
  File access functions for WBC service

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  =========================================================================*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

int directory_exists(const char *prefix, const char *path)
{
    int prefix_len = (prefix == NULL) ? 0 : strlen(prefix);
    int path_len = strlen(path);
    char *fullpath;
    struct stat s;
    int rv;

    fullpath = malloc(prefix_len + path_len + 1);
    if (fullpath == NULL)
        return -ENOMEM;

    memcpy(fullpath, prefix, prefix_len);
    memcpy(fullpath + prefix_len, path, path_len + 1);

    rv = stat(fullpath, &s);
    free(fullpath);
    return rv ? 0 : S_ISDIR(s.st_mode);
}

int file_exists(const char *prefix, const char *path)
{
    int prefix_len = (prefix == NULL) ? 0 : strlen(prefix);
    int path_len = strlen(path);
    char *fullpath;
    struct stat s;
    int rv;

    fullpath = malloc(prefix_len + path_len + 1);
    if (fullpath == NULL)
        return -ENOMEM;

    memcpy(fullpath, prefix, prefix_len);
    memcpy(fullpath + prefix_len, path, path_len + 1);

    rv = stat(fullpath, &s);
    free(fullpath);
    return rv ? 0 : S_ISREG(s.st_mode);
}

/*
 * prefix can be NULL, path must not be NULL
 * return
 *      >=0: fd
 *  <0: -errno
 */
int open_file(const char *prefix, const char *path, int flags)
{
    int prefix_len = (prefix == NULL) ? 0 : strlen(prefix);
    int path_len = strlen(path);
    char *fullpath;
    int rv;

    fullpath = malloc(prefix_len + path_len + 1);
    if (fullpath == NULL)
        return -ENOMEM;

    memcpy(fullpath, prefix, prefix_len);
    memcpy(fullpath + prefix_len, path, path_len + 1);

    rv = open(fullpath, flags, 0660);
    if (rv < 0)
        rv = -errno;

    free(fullpath);
    return rv;
}

/*
 * block until end of file or the specified amount is read
 * <0: -errno
 */
static int read_from_fd(int fd, char *buf, ssize_t count)
{
    ssize_t pos = 0;
    ssize_t rv = 0;

    do {
        pos += rv;
        rv = read(fd, buf + pos, count - pos);
    } while (rv > 0);

    return (rv < 0) ? -errno : pos;
}

/*
 * block until all data is written out
 * <0: -errno
 */
static int write_to_fd(int fd, char *buf, ssize_t count)
{
    ssize_t pos = 0;
    ssize_t rv = 0;

    do {
        rv = write(fd, buf + pos, count - pos);
        if (rv < 0)
            return -errno;
        pos += rv;
    } while (count > pos);

    return count;
}

static int read_int_from_fd(int fd, int *ret_val)
{
    char buffer[16];
    char *endptr;
    long value;
    int rv;

    rv = read_from_fd(fd, buffer, sizeof(buffer) - 1);
    if (rv < 0)
        return rv;
    if (rv == 0)
        return -ENODATA;

    buffer[rv] = '\0';

    value = strtol(buffer, &endptr, 10);
    if (endptr == buffer)
        return -EBADMSG;
    /* if (value == LONG_MAX || value > INT_MAX)
        return -ERANGE;
        */

    *ret_val = (int)value;
    return 0;
}

static int read_int_from_vadc_fd(int fd, int *ret_val)
{
    char buffer[16];
    char *endptr;
    char *valptr;
    long value;
    int rv;

    rv = read_from_fd(fd, buffer, sizeof(buffer) - 1);
    if (rv < 0)
        return rv;
    if (rv == 0)
        return -ENODATA;

    buffer[rv] = '\0';

    valptr = strtok(buffer, "Result:");
    if (!valptr)
        return -EBADMSG;
    value = strtol(valptr, &endptr, 10);
    if (endptr == buffer)
        return -EBADMSG;
    /* if (value == LONG_MAX || value > INT_MAX)
        return -ERANGE;
    */

    *ret_val = (int)value;
    return 0;
}

int read_from_file(const char *prefix, const char *path, char *buf, size_t count)
{
    int fd;
    int rv;

    fd = open_file(prefix, path, O_RDONLY);
    if (fd < 0)
        return fd;

    rv = read_from_fd(fd, buf, count);
    close(fd);

    return rv;
}

int read_int_from_file(const char *prefix, const char *path, int *ret_val)
{
    int fd;
    int rv;

    fd = open_file(prefix, path, O_RDONLY);
    if (fd < 0)
        return fd;

    rv = read_int_from_fd(fd, ret_val);
    close(fd);

    return rv;
}

static int append_to_file(const char *prefix, const char *path,
                        char *buf, ssize_t count)
{
    int fd;
    int rv;

    fd = open_file(prefix, path, O_WRONLY|O_CREAT|O_APPEND);
    if (fd < 0) {
        return fd;
    }

    rv = write_to_fd(fd, buf, count);
    close(fd);

    return rv;
}

static int write_to_file(const char *prefix, const char *path,
                        char *buf, ssize_t count)
{
    int fd;
    int rv;

    fd = open_file(prefix, path, O_WRONLY|O_CREAT|O_TRUNC);
    if (fd < 0) {
        return fd;
    }

    rv = write_to_fd(fd, buf, count);
    close(fd);

    return rv;
}

int append_string_to_file(const char *prefix, const char *path, char *string)
{
    int rv = 0;
    rv = append_to_file(prefix, path, string, strlen(string));
    return rv;
}

int write_string_to_file(const char *prefix, const char *path, char *string)
{
    int rv = 0;
    rv = write_to_file(prefix, path, string, strlen(string));
    return rv;
}

int write_int_to_file(const char *prefix, const char *path, int value)
{
    char buffer[16];

    snprintf(buffer, sizeof(buffer) - 1, "%d", value);
    buffer[sizeof(buffer) - 1] = '\0';

    return write_string_to_file(prefix, path, buffer);
}

static int sync_file(const char *prefix, const char *path)
{
    int fd;
    int rv;

    fd = open_file(prefix, path, O_WRONLY);
    if (fd < 0)
        return fd;

    rv = fsync(fd);
    close(fd);

    return rv;
}

int rename_file(char *oldprefix, char *oldpath, char *newprefix, char *newpath)
{
    int oldprefix_len = (oldprefix == NULL) ? 0 : strlen(oldprefix);
    int oldpath_len = strlen(oldpath);
    int newprefix_len = (newprefix == NULL) ? 0 : strlen(newprefix);
    int newpath_len = strlen(newpath);
    char *oldfullpath;
    char *newfullpath;
    int rv;

    oldfullpath = malloc(oldprefix_len + oldpath_len + 1);
    if (oldfullpath == NULL) {
        rv = -ENOMEM;
        goto out;
    }

    newfullpath = malloc(newprefix_len + newpath_len + 1);
    if (newfullpath == NULL) {
        rv = -ENOMEM;
        goto free_oldpath;
    }

    memcpy(oldfullpath, oldprefix, oldprefix_len);
    memcpy(oldfullpath + oldprefix_len, oldpath, oldpath_len + 1);

    memcpy(newfullpath, newprefix, newprefix_len);
    memcpy(newfullpath + newprefix_len, newpath, newpath_len + 1);

    rv = rename(oldfullpath, newfullpath);
    if (rv < 0)
        rv = -errno;

    rv = sync_file(newprefix, newpath);
    if (rv < 0)
        rv = -errno;

    free(newfullpath);
free_oldpath:
    free(oldfullpath);
out:
    return rv;
}
