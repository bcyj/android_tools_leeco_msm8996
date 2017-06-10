#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include "camera.h"
#include "camera_dbg.h"
#include "mm_camera_interface.h"

pthread_cond_t  af_cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t af_mutex_for_cond = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t af_mutex_callback = PTHREAD_MUTEX_INITIALIZER;

static int is_camafctrl_thread_ready;
static pthread_t camafctrl_thread_id;

/*===========================================================================
 * FUNCTION    - camaf_ctrl -
 *
 * DESCRIPTION: AF control thread main loop
 *==========================================================================*/
void *camaf_ctrl(void *data)
{
  cam_af_ctrl_t *afCtrl = (cam_af_ctrl_t *)data;
  int fd;
  int rc;
  struct msm_ctrl_cmd ctrlCmd;

  if (afCtrl == NULL) {
    CDBG("%s: afCtrl is null. Returning from camaf_ctrl thread. \n", __func__);
    return NULL;
  }

  pthread_mutex_lock(&af_mutex_for_cond);
  char device[MAX_DEV_NAME_LEN];
  snprintf(device, MAX_DEV_NAME_LEN, MSM_CAMERA_CONTROL, get_device_id());
  fd = open(device, O_RDWR);
  if (fd < 0) {
    CDBG("AutoFocus: msm_camera opened failed!\n");
    is_camafctrl_thread_ready = 0;
    return FALSE;
  }

  /* This will block until either AF completes or is cancelled. */
  CDBG("af start (fd %d)\n", fd);

  /* Prepare for snapshot */
  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_PREPARE_SNAPSHOT;
  ctrlCmd.length = 0;
  ctrlCmd.value = NULL;
  ctrlCmd.resp_fd = fd;
  rc = ioctl(fd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  if (rc < 0)
    CDBG("%s: ioctl failed. errno is %d \n", __func__, errno);

  /* Do autofocus */
  ctrlCmd.timeout_ms = 5000;
  ctrlCmd.type = CAMERA_SET_PARM_AUTO_FOCUS;
  ctrlCmd.length = sizeof(int);
  ctrlCmd.value = (void *) &(afCtrl->af_mode);
  ctrlCmd.resp_fd = fd; // FIXME: this will be put in by the kernel

  rc = ioctl(fd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  if (rc < 0)
    CDBG("Autofous MSM_CAM_IOCTL_CTRL_COMMAND failed errno =%d\n", errno);

  is_camafctrl_thread_ready = 0;
  close(fd);
  fd = -1;
  pthread_mutex_unlock(&af_mutex_for_cond);

  CDBG("ctrlCmd->status %d\n", ctrlCmd.status);

  pthread_mutex_lock(&af_mutex_callback);
  afCtrl->af_cb(ctrlCmd.status);
  CDBG("%s:%d: After callback_afStatus\n", __func__, __LINE__);
  pthread_mutex_unlock(&af_mutex_callback);

  return NULL;
}

/*===========================================================================
 * FUNCTION    - launch_camafctrl_fb_thread -
 *
 * DESCRIPTION:
 *==========================================================================*/
int launch_camafctrl_thread(cam_af_ctrl_t *pAfctrl)
{
  is_camafctrl_thread_ready = 0;
  pthread_attr_t attr;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  pthread_create(&camafctrl_thread_id, &attr, camaf_ctrl, (void *)pAfctrl);

  return 0;
}

