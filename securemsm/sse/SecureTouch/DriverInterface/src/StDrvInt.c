/*
 * Copyright(c) 2013-2014 Qualcomm Technologies, Inc.All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#define MAIN_C

#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <utils/Log.h>
#include "common_log.h"

#if defined(ST_TARGET_MSM8974)
#define SYSFS_CONTROL_FILE      "/sys/devices/f9924000.i2c/i2c-2/2-004a/secure_touch_enable"
#define SYSFS_IRQ_FILE          "/sys/devices/f9924000.i2c/i2c-2/2-004a/secure_touch"
#define SYSFS_CONTROL_FILE_ALT  "/sys/devices/f9924000.i2c/i2c-2/2-004a/secure_touch_enable"
#define SYSFS_IRQ_FILE_ALT      "/sys/devices/f9924000.i2c/i2c-2/2-004a/secure_touch"
#elif defined(ST_TARGET_APQ8084)
#define SYSFS_CONTROL_FILE      "/sys/devices/f9966000.i2c/i2c-1/1-004a/secure_touch_enable"
#define SYSFS_IRQ_FILE          "/sys/devices/f9966000.i2c/i2c-1/1-004a/secure_touch"
#define SYSFS_CONTROL_FILE_ALT  "/sys/devices/f9966000.i2c/i2c-1/1-0020/secure_touch_enable"
#define SYSFS_IRQ_FILE_ALT      "/sys/devices/f9966000.i2c/i2c-1/1-0020/secure_touch"
#elif defined(ST_TARGET_MSM8916)
#define SYSFS_CONTROL_FILE      "/sys/devices/soc.0/78b9000.i2c/i2c-5/5-0020/secure_touch_enable"
#define SYSFS_IRQ_FILE          "/sys/devices/soc.0/78b9000.i2c/i2c-5/5-0020/secure_touch"
#define SYSFS_CONTROL_FILE_ALT  "/sys/devices/soc.0/78b9000.i2c/i2c-5/5-0020/input/input2/secure_touch_enable"
#define SYSFS_IRQ_FILE_ALT      "/sys/devices/soc.0/78b9000.i2c/i2c-5/5-0020/input/input2/secure_touch"
#elif defined(ST_TARGET_MSM8994)
#define SYSFS_CONTROL_FILE      "/sys/devices/soc.0/f9924000.i2c/i2c-2/2-0020/input/input0/secure_touch_enable"
#define SYSFS_IRQ_FILE          "/sys/devices/soc.0/f9924000.i2c/i2c-2/2-0020/input/input0/secure_touch"
#define SYSFS_CONTROL_FILE_ALT  "/sys/devices/soc.0/f9924000.i2c/i2c-2/2-004a/secure_touch_enable"
#define SYSFS_IRQ_FILE_ALT      "/sys/devices/soc.0/f9924000.i2c/i2c-2/2-004a/secure_touch"
#else
#error "Secure Touch not supported on target platform"
#endif

/** adb log */
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "libStDrvInt: "
#ifdef LOG_NDDEBUG
#undef LOG_NDDEBUG
#endif
#define LOG_NDDEBUG 0 //Define to enable LOGD
#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#endif
#define LOG_NDEBUG  0 //Define to enable LOGV

static int fd_control = -1;
static int fd_irq = -1;
static char const * control_file = SYSFS_CONTROL_FILE;
static char const * irq_file = SYSFS_IRQ_FILE;



int32_t stStartSession(void)
{
  int32_t rv = 0;
  ssize_t writtenBytes = 0;
  do {
    if (fd_control != -1) {
      LOGE("Session already started");
      rv = EBUSY;
      break;
    }
    LOGD("Opening control file %s", control_file);
    fd_control = open(control_file, O_WRONLY);
    if (fd_control == -1) {
      if (errno == ENOENT) {
        LOGD("Switching to alternative files");
        control_file = SYSFS_CONTROL_FILE_ALT;
        irq_file = SYSFS_IRQ_FILE_ALT;
      }
      fd_control = open(control_file, O_WRONLY);
    }
    if (fd_control == -1) {
      rv = errno;
      LOGE("Error opening %s: %s (%d)", control_file, strerror(errno), errno);
      break;
    }
    writtenBytes = pwrite(fd_control,"1",1,0);
    if (writtenBytes <= 0) {
      rv = errno;
      LOGE("Error writing (1) to %s: %s (%d)", control_file, strerror(errno), errno);
      close(fd_control);
      fd_control = -1;
      break;
    }
  } while (0);
  return rv;
}

int32_t stTerminateSession(uint32_t force)
{
  int32_t rv = 0;
  ssize_t writtenBytes = 0;
  do {
    if (force && (fd_control == -1)) {
      fd_control = open(control_file, O_WRONLY);
      if (fd_control == -1) {
        if (errno == ENOENT) {
          LOGD("Switching to alternative files");
          control_file = SYSFS_CONTROL_FILE_ALT;
          irq_file = SYSFS_IRQ_FILE_ALT;
        }
        fd_control = open(control_file, O_WRONLY);
      }
    }
    if (fd_control == -1) {
      LOGE("No session active");
      rv = ENODEV;
      break;
    }
    writtenBytes = pwrite(fd_control,"0",1,0);
    if (writtenBytes <= 0) {
      LOGE("Error writing (0) to %s: %s (%d)", control_file, strerror(errno), errno);
      rv = errno;
      break;
    }
    close(fd_control);
    fd_control = -1;
    if (fd_irq != -1) {
      close(fd_irq);
      fd_irq = -1;
    }
  } while (0);
  return rv;
}

int32_t stWaitForEvent(int32_t abortFd, int32_t timeout)
{
  int32_t rv = 0;
  ssize_t readBytes = 0;
  char c;
  struct pollfd *fds = NULL; /* Used for poll() */
  size_t events = 1; /* Number of FD to poll */

  do {
    if (fd_irq == -1) {
      fd_irq = open(irq_file,O_RDONLY);
      if (fd_irq == -1) {
        LOGE("Error opening %s: %s (%d)", irq_file, strerror(errno), errno);
        rv = errno;
        break;
      }
    }

    // read and verify if an interrupt is already pending
    readBytes = pread(fd_irq,&c,1,0);
    if (readBytes <= 0) {
      LOGE("Error reading from %s: %s (%d)", irq_file, strerror(errno), errno);
      rv = errno;
      break;
    }

    if (c == '1') {
      // interrupt
      rv = 0;
      break;
    }

    if (abortFd != -1)
      events = 2;

    fds = (struct pollfd *)calloc(events, sizeof(struct pollfd));
    if (fds == NULL) {
      rv = -ENOMEM;
      break;
    }
    /* IRQ FD, always available */
    fds[0].fd = fd_irq;
    fds[0].events = POLLERR|POLLPRI;
    /* FD for abort requests */
    if (events == 2) {
      fds[1].fd = abortFd;
      fds[1].events = POLLIN;
    }

    rv = poll(fds, events, timeout);
    if (rv < 0) {
      /* Error, return error condition */
      LOGE("Error condition during polling: %s (%d)", strerror(errno), errno);
      rv = errno;
      break;
    }
    if (rv == 0) {
      /* timeout */
      rv = -ETIMEDOUT;
      break;
    }
    /* Check for external abort */
    if ((events == 2) && (fds[1].revents)) {
      rv = -ECONNABORTED;
      break;
    }
    /* Consume data, or error, and return */
    if (fds[0].revents) {
      readBytes = pread(fd_irq,&c,1,0);
      if (readBytes <= 0) {
        LOGE("Error reading from %s: %s (%d)", irq_file, strerror(errno), errno);
        rv = errno;
        break;
      }
      if (c == '1') {
        // interrupt
        rv = 0;
        break;
      }
      rv = 0;
    }
  } while (0);
  if (fds) {
    free (fds);
  }
  return rv;
}
