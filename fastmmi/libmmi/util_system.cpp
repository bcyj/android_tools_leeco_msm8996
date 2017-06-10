/*
 * Copyright (c) 2014-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "utils.h"

bool check_file_exist(const char *path) {
    struct stat buffer;
    int err = stat(path, &buffer);

    if(err == 0)
        return true;
    if(errno == ENOENT) {
        ALOGE("file: %s  do not exist , %s\n", path, strerror(errno));
        return false;
    }
    return false;
}

int clear_file(const char *filepath) {
    FILE *fp = NULL;

    /*open as Write, and then close, mean clear file content */
    fp = fopen(filepath, "w");
    if(!fp)
        return -1;

    fclose(fp);
    return 0;
}

int read_file(const char *filepath, char *buf, int size) {
    int fd, len;

    fd = open(filepath, O_RDONLY);
    if(fd == -1) {
        ALOGE("%s:Open desc file fail: %s\n", __FUNCTION__, strerror(errno));
        return -1;
    }

    len = read(fd, buf, size - 1);
    if(len > 0) {
        if(buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        else
            buf[len] = '\0';
    }

    close(fd);
    return 0;
}

int write_file(const char *path, const char *content) {

    FILE *fp = fopen(path, "a");

    if(fp == NULL) {
        ALOGE("%s:Open file fail: %s\n", __FUNCTION__, strerror(errno));
        return -1;
    }

    fwrite(content, sizeof(char), strlen(content), fp);
    fclose(fp);

    return 0;
}

int copy_file(char *src, char *dest) {

    int fd_src, fd_dest;
    struct stat st;
    int length = 0;
    char buffer[SIZE_1K];
    char *p = buffer;
    int rlen = 0, wlen = 0, tmp = 0;

    if(src == NULL || dest == NULL) {
        ALOGE("NULL point. \n");
        return -1;
    }
    fd_src = open(src, O_RDONLY, 0);
    if(fd_src == -1) {
        ALOGE("%s:Open source file fail: %s\n", __FUNCTION__, strerror(errno));
        return -1;
    }

    fd_dest = open(dest, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if(fd_dest == -1) {
        ALOGE("%s:Open desc file fail: %s\n", __FUNCTION__, strerror(errno));
        close(fd_src);
        return -1;
    }

    fstat(fd_src, &st);
    length = st.st_size;

    while(length > 0) {
        rlen = read(fd_src, p, SIZE_1K);
        tmp = rlen;
        while(rlen > 0) {
            wlen = write(fd_dest, p, rlen);
            rlen -= wlen;
            p += wlen;
        }
        p = buffer;
        length -= tmp;
    }
    close(fd_src);
    close(fd_dest);
    return 0;

}

void *zmalloc(size_t n) {
    void *r = malloc(n);

    if(r != NULL) {
        memset(r, 0, n);
    }
    return r;
}

int fork_daemon(const char *filepath, char **args, int *cid) {

    if(!check_file_exist(filepath))
        return -1;

    int pid = fork();

    if(pid == 0) {
        int ret = execv(filepath, args);

        // We shouldn't be here...
        if(ret < 0) {
            ALOGE("execv fail exit: %s\n", strerror(errno));
            exit(1);
        }
    } else if(pid > 0) {
        ALOGI("Main thread will exit successfully");
        *cid = pid;
        return 0;
    } else if(pid < 0) {
        ALOGE("fork failed");
        return -1;
    }
    return -1;
}
