/* server_debug.c
 *
 * Copyright (c) 2014-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <string.h>
#include "camera_dbg.h"
#include "cam_intf.h"
#include "server_debug.h"

/** Name:dump_list_of_daemon_fd()
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:Dump list of the fd's of mm-qcamera-daemon process.
 *
 **/
void dump_list_of_daemon_fd()
{
  pid_t c_pid;
  char path[128] = {0};
  char data[256] = {0};
  char link_data[256] = {0};
  int ret = 0;
  DIR *dir;
  DIR *fd_dir;
  struct dirent* de;
  int t_fd1, t_fd2;
  int debug_file_fd;
  c_pid = getpid();

  snprintf(path, sizeof(path), "/proc/%d/", c_pid);
  strncat(path, "fd/", 3);
  dir = opendir(path);
  if (dir == NULL) {
    CDBG_ERROR("%s opendir fails\n",__func__);
    return ;
  }
  fd_dir = opendir("/data/tombstones");
  if (fd_dir == NULL) {
    if ((mkdir("/data/tombstones", 0777)) < 0) {
      closedir(dir);
      return ;
    }
  }
  else {
    closedir(fd_dir);
  }
  debug_file_fd = open("/data/tombstones/tombstone_fd.txt", O_RDWR|O_CREAT, 0777);
  if (debug_file_fd < 0) {
    if (debug_file_fd = open("/data/tombstones/tombstone_fd.txt", O_RDWR) < 0) {
      closedir(dir);
      return ;
    }
  }
  if (!dump_done) {
    CDBG_ERROR("%s: Error fd leak in mm-qcamera-daemon dump the fd's\n",__func__);
    while ((de = readdir(dir))) {
      if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
        continue;
      strncpy(data, path, sizeof(path));
      strncat(data, de->d_name, sizeof(de->d_name));
      memset(link_data, 0, 256);
      readlink(data, link_data, sizeof(link_data) - 1);
      write(debug_file_fd, "fd = ", 5);
      write(debug_file_fd, de->d_name, strlen(de->d_name));
      write(debug_file_fd, "->", 2);
      write(debug_file_fd, link_data, strlen(link_data));
      write(debug_file_fd, "\n", 1);
    }
    close(debug_file_fd);
    dump_done=1;
  }
  closedir(dir);
}

/** reset_fd_dump:
 *
 *  reset fd dump flag after stop session of moduels.
 *
 *  Return:
 **/
void reset_fd_dump()
{
  dump_done = 0;
}

void* mct_controller_dump_data_for_sof_freeze(void *data1)
{
  int fd;
  char buf[1024] = {0};
  int len;
  DIR *dir;
  struct dirent* de;
  DIR *dir1;
  DIR *sof_dir;
  struct dirent* de1;
  char input[256] = {0};
  int debug_file_fd;

  sof_dir = opendir("/data/tombstones");
  if (sof_dir == NULL) {
    if ((mkdir("/data/tombstones", 0777)) < 0)
      return NULL;
  }
  else {
     closedir(sof_dir);
  }
  debug_file_fd = open("/data/tombstones/tombstone_sof.txt", O_RDWR|O_CREAT, 0777);
  if (debug_file_fd < 0) {
    if (debug_file_fd = open("/data/tombstones/tombstone_sof.txt", O_RDWR) < 0)
      return NULL;
  }

  write(debug_file_fd, "**********clock start**********\n", 32);
  dir = opendir("/sys/kernel/debug/clk");
  if (dir == NULL) {
    CDBG_ERROR("%s: opendir fail",__func__);
    close(debug_file_fd);
    return NULL;
  }
  while ((de = readdir(dir))) {
    if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
      continue;
    write(debug_file_fd, "******************************\n", 31);
    write(debug_file_fd, de->d_name, strlen(de->d_name));
    write(debug_file_fd, "\n", 1);
    write(debug_file_fd, "******************************\n", 31);
    memset(input, 0, 256);
    strncpy(input, "/sys/kernel/debug/clk/", 22);
    strncat(input, de->d_name, sizeof(de->d_name));
    if (de->d_type != DT_DIR)
      continue;
    dir1 = opendir(input);
    if (dir1 == NULL) {
      CDBG_ERROR("%s: opendir fail",__func__);
      close(debug_file_fd);
      closedir(dir);
      return NULL;
    }
    while ((de1 = readdir(dir1))) {
      if (!strcmp(de1->d_name, ".") || !strcmp(de1->d_name, ".."))
        continue;

      if (de1->d_type == DT_DIR)
        continue;

      if (!strcmp(de1->d_name, "enable")) {
        strncpy(input, "/sys/kernel/debug/clk/", 22);
        strncat(input, de->d_name, sizeof(de->d_name));
        strncat(input, "/", 1);
        strncat(input, "enable", 6);
        fd = open(input, O_RDONLY | O_SYNC);
        if (fd > 0) {
          memset(buf, 0, 1024);
          len = read(fd, buf, 1024);
          if (len > 0) {
            write(debug_file_fd, "enable = ", 9);
            write(debug_file_fd, buf, len);
            write(debug_file_fd, "\n", 1);
          }
          close(fd);
        }
      }
      if (!strcmp(de1->d_name, "rate")) {
        strncpy(input, "/sys/kernel/debug/clk/", 22);
        strncat(input, de->d_name, sizeof(de->d_name));
        strncat(input, "/", 1);
        strncat(input, "rate", 4);
        fd = open(input, O_RDONLY);
        if (fd > 0) {
          memset(buf, 0, 1024);
          len = read(fd, buf, 1024);
          if (len > 0) {
            write(debug_file_fd, "rate = ", 7);
            write(debug_file_fd, buf, len);
            write(debug_file_fd, "\n", 1);
          }
          close(fd);
        }
      }
    }
    closedir(dir1);
  }
  closedir(dir);
  write(debug_file_fd, "**********clock end**********\n", 30);
  write(debug_file_fd, "**********Regulator start**********\n", 36);
  dir = opendir("/sys/kernel/debug/regulator");
  if (dir == NULL) {
    CDBG_ERROR("%s: opendir fail",__func__);
    close(debug_file_fd);
    return NULL;
  }
  while ((de = readdir(dir))) {
    if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
      continue;
    write(debug_file_fd, "******************************\n", 31);
    write(debug_file_fd, de->d_name, strlen(de->d_name));
    write(debug_file_fd, "\n", 1);
    write(debug_file_fd, "******************************\n", 31);
    memset(input, 0, 256);
    strncpy(input, "/sys/kernel/debug/regulator/", 28);
    strncat(input, de->d_name,sizeof(de->d_name));
    if (de->d_type != DT_DIR)
      continue;
    dir1 = opendir(input);
    if (dir1 == NULL) {
      CDBG_ERROR("%s: opendir fail",__func__);
      close(debug_file_fd);
      closedir(dir);
      return NULL;
    }
    while ((de1 = readdir(dir1))) {
      if (!strcmp(de1->d_name, ".") || !strcmp(de1->d_name, ".."))
        continue;

      if (de1->d_type == DT_DIR)
        continue;

      if (!strcmp(de1->d_name, "enable")) {
        strncpy(input, "/sys/kernel/debug/regulator/", 28);
        strncat(input, de->d_name, sizeof(de->d_name));
        strncat(input, "/", 1);
        strncat(input, "enable", 6);
        fd = open(input, O_RDONLY);
        if (fd > 0) {
          memset(buf, 0, 1024);
          len = read(fd, buf, 1024);
          if (len > 0) {
            write(debug_file_fd, "enable = ", 9);
            write(debug_file_fd, buf, len);
            write(debug_file_fd, "\n", 1);
          }
          close(fd);
        }
      }
      if (!strcmp(de1->d_name, "voltage")) {
        strncpy(input, "/sys/kernel/debug/regulator/", 28);
        strncat(input, de->d_name, sizeof(de->d_name));
        strncat(input, "/", 1);
        strncat(input, "voltage", 7);
        fd = open(input, O_RDONLY);
        if (fd > 0) {
          memset(buf, 0, 1024);
          len = read(fd, buf, 1024);
          if (len > 0) {
            write(debug_file_fd, "voltage = ", 10);
            write(debug_file_fd, buf, len);
            write(debug_file_fd, "\n", 1);
          }
          close(fd);
        }
      }
    }
    closedir(dir1);
  }
  closedir(dir);
  write(debug_file_fd, "**********Regulator end**********\n", 34);

  write(debug_file_fd, "**********GPIO start**********\n", 31);
  fd = open("/sys/kernel/debug/gpio", O_RDONLY);
  if (fd > 0) {
    do {
     len = read(fd, buf, 1024);
     if (len < 1024) {
       write(debug_file_fd, buf, len);
       break;
     }
     if (len == 0)
       break;

    write(debug_file_fd, buf, len);
    } while (1);
    close(fd);
  }
  write(debug_file_fd, "**********GPIO end**********\n", 29);

 close(debug_file_fd);
 return NULL;
}
