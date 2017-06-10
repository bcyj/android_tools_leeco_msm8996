// -*- Mode: C++ -*-
//=============================================================================
// FILE: os_fs_layer.cpp
//
// SERVICES: file parser
//
// DESCRIPTION:
/// Declarations required for file parser implementation
///
/// Copyright (c) 2011 Qualcomm Technologies, Inc.
/// All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

/* $Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/src/os_fs_layer.cpp#5 $ */

#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include "os_fs_layer.h"

extern "C"
{
EFS_FILE *efs_fopen (const char *path, const char *mode)
{
  EFS_FILE* pfile = NULL;
  if( path && mode)
  {
#ifdef PLATFORM_LTK
    (void)fopen_s(&pfile,path,mode);
#else
    pfile = fopen(path,mode);
#endif

  }
  return pfile;
}

int efs_fclose (EFS_FILE *stream)
{
  int ret = FS_ERROR;
  if(stream)
  {
    ret = fclose(stream);
  }
  return ret;
}
int efs_fseek (EFS_FILE *stream, long offset, int whence)
{
  int ret = FS_ERROR;
  if(stream)
  {
    ret = fseek(stream,offset,whence);
  }
  return ret;
}
long efs_ftell (EFS_FILE *stream)
{
  long ret = FS_ERROR;
  if(stream)
  {
    ret = ftell(stream);
  }
  return ret;
}

fs_size_t efs_fread (void *ptr, fs_size_t size, fs_size_t nitems, EFS_FILE *stream)
{
  fs_size_t ret = FS_ERROR;
  if(stream && ptr)
  {
    ret = (fs_size_t)fread(ptr,size,nitems,stream);
  }
  return ret;
}
fs_size_t efs_fwrite (void* /*ptr*/, fs_size_t /*size*/,
                      fs_size_t /*nitems*/, EFS_FILE* /*stream*/)
{
  fs_size_t ret = FS_ERROR;
  return ret;
}
int efs_fileno (EFS_FILE* /*stream*/)
{
  int ret = FS_ERROR;
  return ret;
}
int efs_unlink (const char* /*path*/)
{
  int ret = FS_ERROR;
  return ret;
}
int efs_rename (const char* /*oldpath*/, const char* /*newpath*/)
{
  int ret = FS_ERROR;
  return ret;
}
int efs_stat (const char *path, struct fs_stat *buf)
{
  int ret = FS_ERROR;
  if(path && buf)
  {
    EFS_FILE* fptr = efs_fopen(path,"rb");
    if(fptr)
    {
      ret = fseek(fptr,0,SEEK_END);
      if(FS_SUCCESS == ret)
      {
        buf->st_size = ftell(fptr);
      }
      fclose(fptr);
    }
  }
  return ret;
}
int efs_fstat(int /*fildes*/, struct fs_stat *buf)
{
  int ret = FS_ERROR;
  if(buf)
  {
    buf->st_size = 0;
  }
  return ret;
}
int efs_statvfs (const char *path, struct fs_statvfs *buf)
{
  int ret = FS_ERROR;
  if(path && buf)
  {
    buf->f_bsize = 0;
    buf->f_bavail = 1;
  }
  return ret;
}
}
