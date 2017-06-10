/******************************************************************************

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  This file is originated from Android Open Source Project,
  platform/system/core.git/toolbox/getevent.c

  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/poll.h>
#include <linux/input.h>
#include <errno.h>
#include <utils/Log.h>
#include <linux/input.h>

static struct pollfd *ufds;
static char **device_names;
static int nfds;

void notifyTouch(int x, int y);
void notifyTouchDown();
void notifyTouchUp();

static int open_device(const char *device) {
	int version;
	int fd;
	int clkid = CLOCK_MONOTONIC;
	struct pollfd *new_ufds;
	char **new_device_names;
	char name[80];
	char location[80];
	char idstr[80];
	struct input_id id;

	fd = open(device, O_RDWR);
	if (fd < 0) {
		ALOGW("could not open %s, %s", device, strerror(errno));
		return -1;
	}

	if (ioctl(fd, EVIOCGVERSION, &version)) {
		ALOGW("could not get driver version for %s, %s",
				device, strerror(errno));
		return -1;
	}
	if (ioctl(fd, EVIOCGID, &id)) {
		ALOGW("could not get driver id for %s, %s", device, strerror(errno));
		return -1;
	}
	name[sizeof(name) - 1] = '\0';
	location[sizeof(location) - 1] = '\0';
	idstr[sizeof(idstr) - 1] = '\0';
	if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
		name[0] = '\0';
	}
	if (ioctl(fd, EVIOCGPHYS(sizeof(location) - 1), &location) < 1) {
		location[0] = '\0';
	}
	if (ioctl(fd, EVIOCGUNIQ(sizeof(idstr) - 1), &idstr) < 1) {
		idstr[0] = '\0';
	}

	if (ioctl(fd, EVIOCSCLOCKID, &clkid) != 0) {
		ALOGW("Can't enable monotonic clock reporting: %s", strerror(errno));
		// a non-fatal error
	}

	new_ufds = realloc(ufds, sizeof(ufds[0]) * (nfds + 1));
	if (new_ufds == NULL) {
		ALOGW("out of memory");
		return -1;
	}
	ufds = new_ufds;
	new_device_names = realloc(device_names,
			sizeof(device_names[0]) * (nfds + 1));
	if (new_device_names == NULL) {
		ALOGW("out of memory");
		return -1;
	}
	device_names = new_device_names;
	ufds[nfds].fd = fd;
	ufds[nfds].events = POLLIN;
	device_names[nfds] = strdup(device);
	nfds++;

	return 0;
}

int close_device(const char *device) {
	int i;
	for (i = 1; i < nfds; i++) {
		if (strcmp(device_names[i], device) == 0) {
			int count = nfds - i - 1;
			ALOGW("remove device %d: %s", i, device);
			free(device_names[i]);
			memmove(device_names + i, device_names + i + 1,
					sizeof(device_names[0]) * count);
			memmove(ufds + i, ufds + i + 1, sizeof(ufds[0]) * count);
			nfds--;
			return 0;
		}
	}
	ALOGW("remote device: %s not found", device);
	return -1;
}

static int read_notify(const char *dirname, int nfd) {
	int res;
	char devname[PATH_MAX];
	char *filename;
	char event_buf[512];
	int event_size;
	int event_pos = 0;
	struct inotify_event *event;

	res = read(nfd, event_buf, sizeof(event_buf));
	if (res < (int) sizeof(*event)) {
		if (errno == EINTR)
			return 0;
		ALOGW("could not get event, %s", strerror(errno));
		return 1;
	}

	strlcpy(devname, dirname, sizeof(devname));
	filename = devname + strlen(devname);
	*filename++ = '/';

	while (res >= (int) sizeof(*event)) {
		event = (struct inotify_event *) (event_buf + event_pos);
		if (event->len) {
			strlcpy(filename, event->name, sizeof(event->name));
			if (event->mask & IN_CREATE) {
				open_device(devname);
			} else {
				close_device(devname);
			}
		}
		event_size = sizeof(*event) + event->len;
		res -= event_size;
		event_pos += event_size;
	}
	return 0;
}

static int scan_dir(const char *dirname) {
	char devname[PATH_MAX];
	char *filename;
	DIR *dir;
	struct dirent *de;
	dir = opendir(dirname);
	if (dir == NULL)
		return -1;
	strlcpy(devname, dirname, sizeof(devname));
	filename = devname + strlen(devname);
	*filename++ = '/';
	while ((de = readdir(dir))) {
		if (de->d_name[0] == '.'
				&& (de->d_name[1] == '\0'
						|| (de->d_name[1] == '.' && de->d_name[2] == '\0')))
			continue;
		strlcpy(filename, de->d_name,sizeof(de->d_name));
		open_device(devname);
	}
	closedir(dir);
	return 0;
}

int main() {
	int i;
	int res;
	struct input_event event;
	const char *device_path = "/dev/input";
	int positionX = -1;
	int positionY = -1;

	opterr = 0;
	nfds = 1;
	ufds = calloc(1, sizeof(ufds[0]));
	if (ufds == NULL) {
		ALOGW("out of memory");
		return 0;
	}
	ufds[0].fd = inotify_init();
	ufds[0].events = POLLIN;
	res = inotify_add_watch(ufds[0].fd, device_path, IN_DELETE | IN_CREATE);
	if (res < 0) {
		ALOGW("could not add watch for %s, %s", device_path, strerror(errno));
		return 0;
	}
	res = scan_dir(device_path);
	if (res < 0) {
		ALOGW("scan dir failed for %s", device_path);
		return 0;
	}
	while (1) {
		poll(ufds, nfds, -1);
		if (ufds[0].revents & POLLIN) {
			read_notify(device_path, ufds[0].fd);
		}
		for (i = 1; i < nfds; i++) {
			if (ufds[i].revents) {
				if (ufds[i].revents & POLLIN) {
					res = read(ufds[i].fd, &event, sizeof(event));
					if (res < (int) sizeof(event)) {
						ALOGW("could not get event");
						return 0;
					}
					if (event.type == EV_KEY && event.code == BTN_TOUCH) {
						if (event.value == 0) {
							notifyTouchUp();
						} else if (event.value == 1) {
							notifyTouchDown();
						}
					} else if (event.type == EV_ABS) {
						switch (event.code) {
						case ABS_MT_POSITION_X:
							positionX = event.value;
							break;
						case ABS_MT_POSITION_Y:
							positionY = event.value;
							break;
						}
						if (positionX != -1 && positionY != -1) {
							notifyTouch(positionX, positionY);
						}
					}
				}
			}
		}
	}

	return 0;

}
