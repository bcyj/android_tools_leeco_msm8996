/*******************************************************************************
 * -----------------------------------------------------------------------------
 * Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * -----------------------------------------------------------------------------
 ******************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/rtc.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include "header.h"
#include <dirent.h>


int directory_exists(char *path)
{
	struct stat s;
	int rv;

	rv = stat(path, &s);
	return rv ? 0 : S_ISDIR(s.st_mode);
}

int file_exists(char *path)
{
	struct stat s;
	int rv;

	rv = stat(path, &s);
	return rv ? 0 : S_ISREG(s.st_mode);
}

int file_exists_with_prefix(char *prefix, char *path)
{
	struct stat s;
	int rv;
	char *fullpath;
	int prefix_len = (prefix == NULL) ? 0 : strlen(prefix);
	int path_len = strlen(path);

	fullpath = malloc(prefix_len + path_len + 1);
	if (fullpath == NULL)
		return -ENOMEM;

	memcpy(fullpath, prefix, prefix_len);
	memcpy(fullpath + prefix_len, path, path_len + 1);

	rv = stat(fullpath, &s);

	free(fullpath);

	return rv ? 0 : S_ISREG(s.st_mode);
}

static char *get_full_path(char *prefix, char *path)
{
	int prefix_len = (prefix == NULL) ? 0 : strlen(prefix);
	int path_len = strlen(path);
	char *fullpath;

	fullpath = malloc(prefix_len + path_len + 2);
	if (fullpath == NULL)
		return NULL;

	memcpy(fullpath, prefix, prefix_len);
	memcpy(fullpath + prefix_len, path, path_len + 1);
	fullpath[prefix_len + path_len] = '/';
	fullpath[prefix_len + path_len + 1] = '\0';
	return fullpath;
}

/*
 * prefix can be NULL, path must not be NULL
 */
FILE *fopen_file(char *prefix, char *path, char *flag_str)
{

	int prefix_len = (prefix == NULL) ? 0 : strlen(prefix);
	int path_len = strlen(path);
	char *fullpath;
	FILE *fptr;

	fullpath = malloc(prefix_len + path_len + 1);
	if (fullpath == NULL)
		return NULL;

	memcpy(fullpath, prefix, prefix_len);
	memcpy(fullpath + prefix_len, path, path_len + 1);

	fptr = fopen(fullpath, flag_str);

	free(fullpath);
	return fptr;
}

char *get_process_pid_from_name(char *process_name)
{

	DIR *tdir = NULL;
	struct dirent *cpu_dirent;
	FILE *fptr;
	char proc_name[256] = {0};
	char *dir_path;
	char cwd[256] = {0};
	char *dir_name;
	int cmp_result;
	int len1, len2;

	if (!getcwd(cwd, sizeof(cwd)))
		return NULL;

	chdir(PROC_NODE);

	tdir = opendir(PROC_NODE);
	if (!tdir) {

		fprintf(stdout, "Unable to open %s\n", PROC_NODE);
		return NULL;
	}

	dir_name = (char *)malloc(256);
	if (dir_name == NULL)
		return NULL;

	while ((cpu_dirent = readdir(tdir))) {
		strncpy(dir_name, cpu_dirent->d_name, strlen(cpu_dirent->d_name));
		dir_name[strlen(cpu_dirent->d_name)] = '\0';
		dir_path = get_full_path(PROC_NODE, dir_name);
		if (file_exists_with_prefix(dir_path, "status")) {

			fptr = fopen_file(dir_path, "status", "r");

			/* scan the process name */
			if (fptr == NULL) {
				free(dir_path);
				return NULL;
			}
			else {
				fscanf( fptr, "Name:   %s", proc_name);

				len1 = strlen(process_name);
				len2 = strlen(proc_name);

				if (len1 != len2) {
					free(dir_path);
					continue;
				}

				cmp_result = strncmp(process_name, proc_name, len1);

				if (cmp_result == 0) {
					free(dir_path);
					break;
				}
			}
		}
		free(dir_path);
	}

	closedir(tdir);
	chdir(cwd); /* Restore current working dir */

	return dir_name;
}


/*
 * prefix can be NULL, path must not be NULL
 * return
 *      >=0: fd
 * 	<0: -errno
 */
int open_file(char *prefix, char *path, int flags)
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

	rv = open(fullpath, flags);
	if (rv < 0)
		rv = -errno;

	free(fullpath);
	return rv;
}

/*
 * block until end of file or the specified amount is read
 * <0: -errno
 */
int read_from_fd(int fd, char *buf, ssize_t count)
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
int write_to_fd(int fd, char *buf, size_t count)
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

int read_unsigned_int_from_fd(int fd)
{
	char buffer[16];
	char *endptr;
	unsigned long value;
	int rv;

	rv = read_from_fd(fd, buffer, sizeof(buffer) - 1);
	if (rv < 0)
		return rv;
	if (rv == 0)
		return -ENODATA;

	buffer[rv] = '\0';

	value = strtoul(buffer, &endptr, 10);
	if (endptr == buffer)
		return -EBADMSG;
	if (value == ULONG_MAX || value > INT_MAX)
		return -ERANGE;

	return (int)value;
}

int read_from_file(char *prefix, char *path, char *buf, size_t count)
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

int read_unsigned_int_from_file(char *prefix, char *path)
{
	int fd;
	int rv;

	fd = open_file(prefix, path, O_RDONLY);
	if (fd < 0)
		return fd;

	rv = read_unsigned_int_from_fd(fd);
	close(fd);

	return rv;
}

int write_to_file(char *prefix, char *path, char *buf, size_t count)
{
	int fd;
	int rv;

	fd = open_file(prefix, path, O_RDWR);
	if (fd < 0)
		return fd;

	rv = write_to_fd(fd, buf, count);
	close(fd);

	return rv;
}

int write_string_to_file(char *prefix, char *path, char *string)
{
	return write_to_file(prefix, path, string, strlen(string));
}

int write_int_to_file(char *prefix, char *path, int value)
{
	char buffer[16];

	snprintf(buffer, sizeof(buffer) - 1, "%d", value);
	buffer[sizeof(buffer) - 1] = '\0';

	return write_string_to_file(prefix, path, buffer);
}

int fork_exec(char *file, char *arg1, char *arg2, char *arg3, int dev_null)
{
	char *argv[5];
	pid_t pid;
	int stat;

	argv[0] = file;
	argv[1] = arg1;
	argv[2] = arg2;
	argv[3] = arg3;
	argv[4] = NULL;

	pid = fork();
	switch (pid) {
	case -1:
		fprintf(stdout, "fork failed: %s\n", strerror(errno));
		return -errno;
	case 0:
		if (dev_null) {
			int dev_null_fd;
			int rv;

			dev_null_fd = open("/dev/null", O_WRONLY);
			if (dev_null_fd < 0) {
				fprintf(stdout, "/dev/null: %s\n",
					strerror(errno));
				return -errno;
			}

			rv = close(STDOUT_FILENO);
			if (rv < 0) {
				fprintf(stdout, "failed to close stdout: %s\n",
					strerror(errno));
				return -errno;
			}

			rv = dup2(dev_null_fd, STDOUT_FILENO);
			if (rv != STDOUT_FILENO) {
				fprintf(stdout,
					"failed to dup /dev/null to stdout: %s\n",
					strerror(errno));
				return -errno;
			}
		}
		execvp(file, argv);
		fprintf(stdout, "exec failed: %s\n", strerror(errno));
		return -errno;
	default:
		waitpid(pid, &stat, 0);
		return 0;
	}
}

/* src must be null-terminated
 * <0: errno
 */
int parse_pm_stats_count(char *src, char *tag)
{
	char *pos;
	unsigned long value;
	char *endptr;

	pos = strstr(src, tag);
	if (pos == NULL)
		return -EBADMSG;

	pos += strlen(tag);
	value = strtoul(pos, &endptr, 10);

	if (endptr == pos || *endptr != '\n')
		return -EBADMSG;
	if (value == ULONG_MAX || value > INT_MAX)
		return -ERANGE;

	return (int)value;
}

/* src must be null-terminated
 * <0: errno
 */
signed long long parse_wakelock_stats_for_active_wl(char *src, char *wl_name)
{
	char *pos;
	int value;
	char *endptr;
	int lock_count, expire_count, wakeup_count;
	signed long long active_time;

	pos = strstr(src, wl_name);
	if (pos == NULL)
		return -EBADMSG;

	pos += strlen(wl_name);
	value = sscanf(pos, "\t%d\t%d\t%d\t%lld", &lock_count, &expire_count,
		       &wakeup_count, &active_time);

	if ((value == EOF) || (value < 4))
		return -EBADMSG;

	return active_time;
}

/* src must be null-terminated
 * >=0: 0 based line number
 * <0: errno
 */
int get_pm_stats_line_no(char *src, char *tag)
{
	char *pos;
	int count;

	pos = strstr(src, tag);
	if (pos == NULL)
		return -EBADMSG;

	for (count = 0, --pos; pos >= src; pos--)
		if (*pos == '\n')
			count++;

	return count;
}
