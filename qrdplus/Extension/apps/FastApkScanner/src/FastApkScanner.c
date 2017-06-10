/*
 * Copyright (c) 2015, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include "FastApkScanner.h"


#define CENTRAL_DIRECTORY_SIZE          12
#define CENTRAL_DIRECTORY_OFFSET        16
#define CENTRAL_DIRECTORY_ENTRY_SIZE    46
#define CENTRAL_DIRECTORY_END_SIZE      22

#define ENTRY_NAME_OFFSET               28
#define ENTRY_FIELD_OFFSET              30
#define ENTRY_COMMENT_OFFSET            32

#define LIBPATH_MIN_SIZE                14
#define COMMENT_MAX_SIZE                0xffff

#define ERROR_SEEK                      -1
#define ERROR_MALLOC                    -2
#define ERROR_READ                      -3

#define WORD2INT(b1,b2) (((unsigned int)(unsigned char)(b1)) | (((unsigned int)(unsigned char)(b2))<<8))
#define GETWORD(ptr,offset) WORD2INT(ptr[offset],ptr[offset+1])
#define GETDWORD(ptr,offset) (WORD2INT(ptr[offset],ptr[offset+1]) | (WORD2INT(ptr[offset+2],ptr[offset+3])<<16))

#define TestCentralDirectory(ptr) ('P' == ptr[0] && 'K' == ptr[1] && 1 == ptr[2] && 2 == ptr[3])
#define TestCentralDirectoryEnd(ptr) ('P' == ptr[0] && 'K' == ptr[1] && 5 == ptr[2] && 6 == ptr[3])

static int testFilenameSafe(char chr)
{
    switch (chr) {
        case 'A' ... 'Z':
        case 'a' ... 'z':
        case '0' ... '9':
        case '+':
        case ',':
        case '-':
        case '.':
        case '/':
        case '=':
        case '_':
            return 1;

        default:
            // We found something that is not good.
            return 0;
    }
}

static int isFilenameSafe(const char* filename, int length)
{
    off_t offset = 0;
    for (;offset < length;) {
        char chr = *(filename + offset);
        if ('\0' == chr) {
            // If we've reached the end, all the other characters are good.
            return 1;
        } else if (testFilenameSafe(chr)) {
            offset++;
        } else {
            // We found something that is not good.
            return 0;
        }
    }
    return 1;
}

static void filterEntryNameInternal(const void* pZipCenterDirPtr, size_t length, PNameFilter filter, void* param) {
    if (pZipCenterDirPtr != NULL && length > 0 && filter != NULL) {
        const char* ptr = (const char*)pZipCenterDirPtr;
        const char* end = ptr + length;
        while (ptr < end) {
            if (TestCentralDirectory(ptr)) {
                unsigned int name_length = GETWORD(ptr,ENTRY_NAME_OFFSET);
                unsigned int field_length = GETWORD(ptr,ENTRY_FIELD_OFFSET);
                unsigned int comment_length = GETWORD(ptr,ENTRY_COMMENT_OFFSET);
                if (0 == filter(ptr+CENTRAL_DIRECTORY_ENTRY_SIZE, name_length, param)) {
                    return;
                } else {
                    ptr += CENTRAL_DIRECTORY_ENTRY_SIZE + name_length + field_length + comment_length;
                }
            } else {
                ptr++;
            }
        }
    }
}

#ifndef off64_t
#define off64_t size_t
#endif

static int findCenterDirOffset(int fd, int* size) {
    const off64_t max_offset = COMMENT_MAX_SIZE + CENTRAL_DIRECTORY_END_SIZE;
    if (lseek64(fd, -max_offset, SEEK_END)<0) {
        return ERROR_SEEK;
    }
    char* buffer = (char*)malloc(max_offset);
    if (NULL == buffer) {
        return ERROR_MALLOC;
    }
    int len = read(fd, buffer, max_offset);
    if (CENTRAL_DIRECTORY_END_SIZE > len) {
        free(buffer);
        return ERROR_READ;
    }
    const char* ptr = buffer + len - CENTRAL_DIRECTORY_END_SIZE;
    int ret = 0;
    while (ptr >= buffer) {
        if (TestCentralDirectoryEnd(ptr)) {
            if (size != NULL) {
                *size = GETDWORD(ptr,CENTRAL_DIRECTORY_SIZE);
            }
            ret = GETDWORD(ptr,CENTRAL_DIRECTORY_OFFSET);
            break;
        } else {
            ptr--;
        }
    }
    free(buffer);
    return ret;
}

typedef struct _FilterObject {
    struct _FilterObject* next;
    int fd;
    int offset;
    int size;
} FilterObject;

#define BUCKET_LENGTH 10
static FilterObject* g_filters[BUCKET_LENGTH] = {0};
pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static FilterObject* objectContains(int fd) {
    if (fd < 0) {
        return NULL;
    }
    pthread_mutex_lock(&g_lock);
    FilterObject* temp = g_filters[fd % BUCKET_LENGTH];
    while (temp != NULL) {
        if (temp->fd == fd) {
            pthread_mutex_unlock(&g_lock);
            return temp;
        } else {
            temp = temp->next;
        }
    }
    pthread_mutex_unlock(&g_lock);
    return NULL;
}

PFilterObject RegistFilterObject(int fd) {
    if (fd < 0) {
        return NULL;
    }
    FilterObject* ptr = objectContains(fd);
    if (ptr != NULL) {
        return (PFilterObject)ptr;
    }
    ptr = (FilterObject*)malloc(sizeof(FilterObject));
    if (NULL == ptr) {
        return NULL;
    }
    ptr->fd = fd;
    ptr->offset = findCenterDirOffset(fd, &(ptr->size));
    pthread_mutex_lock(&g_lock);
    ptr->next = g_filters[fd % BUCKET_LENGTH];
    g_filters[fd % BUCKET_LENGTH] = ptr;
    pthread_mutex_unlock(&g_lock);
    return (PFilterObject)ptr;
}

void UnRegistFilterObject(int fd) {
    if (fd < 0) {
        return;
    }
    pthread_mutex_lock(&g_lock);
    FilterObject* temp = g_filters[fd % BUCKET_LENGTH];
    FilterObject* prev = NULL;
    while (temp != NULL) {
        if (temp->fd == fd) {
            break;
        } else {
            prev = temp;
            temp = temp->next;
        }
    }
    if (temp != NULL) {
        if (prev != NULL) {
            prev->next = temp->next;
        } else {
            g_filters[fd % BUCKET_LENGTH] = temp->next;
        }
        pthread_mutex_unlock(&g_lock);
        free(temp);
    } else {
        pthread_mutex_unlock(&g_lock);
    }
}

PFilterObject GetFilterObject(int fd) {
    return (PFilterObject)objectContains(fd);
}

int FilterEntryName(PFilterObject obj, PNameFilter filter, void* param) {
    FilterObject* ptr = (FilterObject*) obj;
    if (lseek64(ptr->fd, ptr->offset, SEEK_SET) != ptr->offset) {
        return ERROR_SEEK;
    }
    char* buffer = (char*) malloc(ptr->size);
    if (read(ptr->fd, buffer, ptr->size) != ptr->size) {
        free(buffer);
        return ERROR_READ;
    }
    filterEntryNameInternal(buffer, ptr->size, filter, param);
    free(buffer);
    return 0;
}

static int checkLibFile(char* fileName, size_t fileNameLen, PNameFilter filter, void* param)
{
    if (!strncmp(fileName, "lib/", 4)) {
        static const size_t minLength = LIBPATH_MIN_SIZE;
        if (fileNameLen >= minLength) {
            char temp = fileName[fileNameLen];
            fileName[fileNameLen] = '\0';
            const char* lastSlash = strrchr(fileName, '/');
            fileName[fileNameLen] = temp;
            if (NULL == lastSlash) {
                return 0;
            }
            if (0!=strncmp(lastSlash + 1, "gdbserver", 9)) {
                if (!strncmp(fileName + fileNameLen - 3, ".so", 3)
                    && !strncmp(lastSlash, "/lib", 4)) {
                    if (isFilenameSafe(lastSlash + 1, fileNameLen - (lastSlash-fileName) - 1)) {
                        return filter(fileName,fileNameLen,param);
                    }
                }
            } else {
                return filter(fileName,fileNameLen,param);
            }
        }
    }
    return 0;
}

int FilterLibrary(PFilterObject obj, PNameFilter filter, void* param) {
    FilterObject* pObj = (FilterObject*) obj;
    if (lseek64(pObj->fd, pObj->offset, SEEK_SET) != pObj->offset) {
        return ERROR_SEEK;
    }
    char* buffer = (char*) malloc(pObj->size);
    if (NULL == buffer) {
        return ERROR_MALLOC;
    }
    if (read(pObj->fd, buffer, pObj->size) != pObj->size) {
        free(buffer);
        return ERROR_READ;
    }
    const char* ptr = buffer;
    const char* end = ptr + pObj->size;
    while (ptr < end) {
        if (TestCentralDirectory(ptr)) {
            unsigned int name_length = GETWORD(ptr,ENTRY_NAME_OFFSET);
            unsigned int field_length = GETWORD(ptr,ENTRY_FIELD_OFFSET);
            unsigned int comment_length = GETWORD(ptr,ENTRY_COMMENT_OFFSET);
            int ret = checkLibFile(ptr+CENTRAL_DIRECTORY_ENTRY_SIZE, name_length, filter, param);
            if (ret > 0) {
                free(buffer);
                return ret;
            } else {
                ptr += CENTRAL_DIRECTORY_ENTRY_SIZE + name_length + field_length + comment_length;
            }
        } else {
            ptr++;
        }
    }
    free(buffer);
    return 0;
}

int HasRenderScript(PFilterObject obj) {
    FilterObject* pObj = (FilterObject*) obj;
    if (lseek64(pObj->fd, pObj->offset, SEEK_SET) != pObj->offset) {
        return ERROR_SEEK;
    }
    char* buffer = (char*) malloc(pObj->size);
    if (NULL == buffer) {
        return ERROR_MALLOC;
    }
    if (read(pObj->fd, buffer, pObj->size) != pObj->size) {
        free(buffer);
        return ERROR_READ;
    }
    const char* ptr = buffer;
    const char* end = ptr + pObj->size;
    while (ptr < end) {
        if (TestCentralDirectory(ptr)) {
            unsigned int name_length = GETWORD(ptr,ENTRY_NAME_OFFSET);
            unsigned int field_length = GETWORD(ptr,ENTRY_FIELD_OFFSET);
            unsigned int comment_length = GETWORD(ptr,ENTRY_COMMENT_OFFSET);
            if (strncmp(ptr+CENTRAL_DIRECTORY_ENTRY_SIZE+name_length-3,".bc",3) == 0) {
                free(buffer);
                return 1;
            }
            else
                ptr += CENTRAL_DIRECTORY_ENTRY_SIZE + name_length + field_length + comment_length;
        } else {
            ptr++;
        }
    }
    free(buffer);
    return 0;
}


