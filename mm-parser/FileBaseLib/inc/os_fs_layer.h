//=============================================================================
// FILE: os_fs_layer.h
//
// SERVICES: file parser
//
// DESCRIPTION:
/// Declarations required for file parser implementation
///
/// Copyright (c) 2011 Qualcomm Technologies, Inc.
/// All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

/* $Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/inc/os_fs_layer.h#4 $ */
#ifndef __OS_FS_LAYER_H
#define __OS_FS_LAYER_H

#include <stdio.h>

typedef FILE  EFS_FILE;


#include "parserdatadef.h"
#include "parserinternaldefs.h"

typedef FILE  EFS_FILE;
typedef uint64 fs_size_t ;

#define FS_SUCCESS   0
#define FS_ERROR     1

typedef struct fs_stat
{
  fs_size_t  st_size;
}fs_stat;

typedef struct fs_statvfs
{
 unsigned long      f_bsize;
 bool f_bavail;
}fs_statvfs;

extern "C"
{
EFS_FILE* efs_fopen (const char *path, const char *mode);

int efs_fclose (EFS_FILE *stream);
int efs_fseek (EFS_FILE *stream, long offset, int whence);
long efs_ftell (EFS_FILE *stream);

fs_size_t efs_fread (void *ptr, fs_size_t size, fs_size_t nitems, EFS_FILE *stream);
fs_size_t efs_fwrite (void *ptr, fs_size_t size, fs_size_t nitems, EFS_FILE *stream);
int efs_fileno (EFS_FILE *stream);


int efs_unlink (const char *path);
int efs_rename (const char *oldpath, const char *newpath);
int efs_stat (const char *path, struct fs_stat *buf);
int efs_fstat(int fildes, struct fs_stat *buf);
int efs_statvfs (const char *path, struct fs_statvfs *buf);
}

#endif
