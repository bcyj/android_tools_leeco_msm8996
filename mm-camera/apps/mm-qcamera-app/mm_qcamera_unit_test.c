/*============================================================================

   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <pthread.h>
#include "mm_camera_dbg.h"
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include "mm_qcamera_unit_test.h"

#define MM_QCAMERA_APP_UTEST_MAX_MAIN_LOOP 4
#define MM_QCAM_APP_TEST_NUM 128

static mm_app_tc_t mm_app_tc[MM_QCAM_APP_TEST_NUM];
static int num_test_cases = 0;
int mm_app_tc_0(mm_camera_app_t *cam_apps, int cam_id, int query_cam_info)
{
	int rc = MM_CAMERA_OK;
	uint32_t y_off[2], cbcr_off[2];
	uint32_t len[2];
	int w[2], h[2];
	uint32_t addr[2];
	int i, k, fd[2];

	printf("Running %s - user ptr map/unmap\n", __func__); 
	w[0] = 1280;
	w[1] = 320;
	h[0] = 960;
	h[1] = 240;

	CDBG("%s: BEGIN\n", __func__);

	for(k = 0; k < MM_QCAMERA_APP_UTEST_MAX_MAIN_LOOP; k++) {
		for(i = 0; i < 2; i++) {
			len[i] = mm_camera_get_msm_frame_len(CAMERA_YUV_420_NV12, 
																				 CAMERA_MODE_2D, w[i], 
																				h[i], &y_off[i], &cbcr_off[i], MM_CAMERA_PAD_WORD);
			addr[i] = (unsigned long) mm_camera_do_mmap(len[i], &fd[i]);
			if (!addr[i]) {
				CDBG("%s:no mem for snapshot buf\n", __func__);
				rc = -MM_CAMERA_E_NO_MEMORY;
				goto end;
			}
		}
		for(i = 0; i < 2; i++) {
			(void)mm_camera_do_munmap(fd[i], (void *)addr[i],len[i]);
		}
	}
end:
	CDBG("%s:END, rc = %d\n", __func__, rc);
	return rc;
}

int mm_app_tc_1(mm_camera_app_t *cam_apps, int cam_id, int query_cam_info)
{
	int i, rc = MM_CAMERA_OK;
	printf("Running %s - open/close ,video0,\n", __func__); 

	CDBG("%s:BEGIN\n", __func__);
	for(i = 0; i < MM_QCAMERA_APP_UTEST_MAX_MAIN_LOOP; i++) {
		memset(cam_apps, 0, sizeof(mm_camera_app_t));
		cam_apps->cam = (mm_camera_t *)mm_camera_query(&cam_apps->num_cameras);
		if(cam_apps->cam && cam_apps->num_cameras > 0) {
			cam_apps->use_overlay = TRUE;  /* deafult use overlay */
			cam_apps->use_user_ptr = TRUE; /* default use usr ptr */ 
		} else {
			rc = -1;
			break;
		}
	}
	CDBG("%s:END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_tc_2(mm_camera_app_t *cam_apps, int cam_id, int query_cam_info)
{
	int rc = MM_CAMERA_OK;
	int i;
	printf("Running %s - open/close ,video0, open/close all channel in notused mode\n", __func__); 

	for(i = 0; i < MM_QCAMERA_APP_UTEST_MAX_MAIN_LOOP; i++) {
		if ( 0 != (rc = mm_app_open(cam_id, MM_CAMERA_OP_MODE_NOTUSED))) {
			CDBG("%s: open cam %d at opmode = %d err, loop=%d, rc=%d\n", __func__, cam_id, MM_CAMERA_OP_MODE_NOTUSED, i, rc); 
			goto end;
		}
		if(0 != (rc = mm_app_open_ch(cam_id, MM_CAMERA_CH_MAX))) {
			CDBG("%s:open all channels error=%d\n", __func__, rc);
			goto end;
		}
		mm_app_close_ch(cam_id, MM_CAMERA_CH_MAX);
		if ( 0 != (rc = mm_app_close(cam_id))) {
			CDBG("%s: close cam %d at opmode = %d err,loop=%d, rc=%d\n", __func__, cam_id, MM_CAMERA_OP_MODE_NOTUSED, i, rc); 
			goto end;
		}
	}
end:
	CDBG("%s:END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_tc_3(mm_camera_app_t *cam_apps, int cam_id, int query_cam_info)
{
	int rc = MM_CAMERA_OK;
	int i;
	printf("Running %s - open/close ,video0, open/close all channel in video mode\n", __func__); 

	for(i = 0; i < MM_QCAMERA_APP_UTEST_MAX_MAIN_LOOP; i++) {
		if ( 0 != (rc = mm_app_open(cam_id, MM_CAMERA_OP_MODE_VIDEO))) {
			CDBG("%s: open cam %d at opmode = %d err, loop=%d, rc=%d\n", __func__, cam_id, MM_CAMERA_OP_MODE_VIDEO, i, rc); 
			goto end;
		}
		if(0 != (rc = mm_app_open_ch(cam_id, MM_CAMERA_CH_MAX))) {
			CDBG("%s:open all channels error=%d\n", __func__, rc);
			goto end;
		}
		mm_app_close_ch(cam_id, MM_CAMERA_CH_MAX);
		if ( 0 != (rc = mm_app_close(cam_id))) {
			CDBG("%s: close cam %d at opmode = %d err,loop=%d, rc=%d\n", __func__, cam_id, MM_CAMERA_OP_MODE_VIDEO, i, rc); 
			goto end;
		}
	}
end:
	CDBG("%s:END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_tc_4(mm_camera_app_t *cam_apps, int cam_id, int query_cam_info)
{
	int rc = MM_CAMERA_OK;
	int i;
	printf("Running %s - open/close ,video0, open/close preview channel only\n", __func__); 

	for(i = 0; i < MM_QCAMERA_APP_UTEST_MAX_MAIN_LOOP; i++) {
		if ( 0 != (rc = mm_app_open(cam_id, MM_CAMERA_OP_MODE_NOTUSED))) {
			CDBG("%s: open cam %d at opmode = %d err, loop=%d, rc=%d\n", __func__, cam_id, MM_CAMERA_OP_MODE_NOTUSED, i, rc); 
			goto end;
		}
		if(0 != (rc = mm_app_open_preview(cam_id))) {
			goto end;
		}
		if(0 != (rc = mm_app_close_preview(cam_id))) {
			goto end;
		}
		if ( 0 != (rc = mm_app_close(cam_id))) {
			CDBG("%s: close cam %d at opmode = %d err,loop=%d, rc=%d\n", __func__, cam_id, MM_CAMERA_OP_MODE_NOTUSED, i, rc); 
			goto end;
		}
	}
end:
	CDBG("%s:END, rc=%d\n", __func__, rc);
	return rc;
}



int mm_app_tc_5(mm_camera_app_t *cam_apps, int cam_id, int query_cam_info)
{
	int rc = MM_CAMERA_OK;
	int i;
	printf("Running %s - open/close ,video0, open/close snapshot channel only\n", __func__); 

	for(i = 0; i < MM_QCAMERA_APP_UTEST_MAX_MAIN_LOOP; i++) {
		if ( 0 != (rc = mm_app_open(cam_id, MM_CAMERA_OP_MODE_NOTUSED))) {
			CDBG("%s: open cam %d at opmode = %d err, loop=%d, rc=%d\n", 
					 __func__, cam_id, MM_CAMERA_OP_MODE_NOTUSED, i, rc); 
			goto end;
		}
		if(0 != (rc = mm_app_open_snapshot(cam_id))) {
			goto end;
		}
		if(0 != (rc = mm_app_close_snapshot(cam_id))) {
			goto end;
		}
		if ( 0 != (rc = mm_app_close(cam_id))) {
			CDBG("%s: close cam %d at opmode = %d err,loop=%d, rc=%d\n", 
					 __func__, cam_id, MM_CAMERA_OP_MODE_NOTUSED, i, rc); 
			goto end;
		}
	}
end:
	CDBG("%s:END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_tc_6(mm_camera_app_t *cam_apps, int cam_id, int query_cam_info)
{
	int rc = MM_CAMERA_OK;
	int i;
	printf("Running %s - simple preview \n", __func__); 

	if ( 0 != (rc = mm_app_open(cam_id, MM_CAMERA_OP_MODE_NOTUSED))) {
		CDBG("%s: open cam %d at opmode = %d err, loop=%d, rc=%d\n", 
				 __func__, cam_id, MM_CAMERA_OP_MODE_NOTUSED, i, rc); 
		goto end;
	}

	for(i = 0; i < MM_QCAMERA_APP_UTEST_MAX_MAIN_LOOP; i++) {
		if(0 != (rc = mm_app_init_preview(cam_id))) {
			goto end;
		}
		if(0 != (rc = mm_app_start_preview(cam_id))) {
			goto end;
		}
		/* sleep 8 seconds */
		usleep(8000000);
		if(0 != (rc = mm_app_stop_preview(cam_id))) {
			goto end;
		}
		if(0 != (rc=mm_app_deinit_preview(cam_id))) {
			goto end;
		}
		if(0 != (rc = mm_app_close_preview(cam_id))) {
			goto end;
		}
	}
	if ( 0 != (rc = mm_app_close(cam_id))) {
		CDBG("%s: close cam %d at opmode = %d err,loop=%d, rc=%d\n", 
				 __func__, cam_id, MM_CAMERA_OP_MODE_NOTUSED, i, rc); 
		goto end;
	}
end:
	CDBG("%s:END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_tc_7(mm_camera_app_t *cam_apps, int cam_id, int query_cam_info)
{
	int rc = MM_CAMERA_OK;
	int i;
	printf("Running %s - simple preview and recording \n", __func__); 

	if ( 0 != (rc = mm_app_open(cam_id, MM_CAMERA_OP_MODE_NOTUSED))) {
		CDBG("%s: open cam %d at opmode = %d err, loop=%d, rc=%d\n", 
				 __func__, cam_id, MM_CAMERA_OP_MODE_NOTUSED, i, rc); 
		goto end;
	}

	for(i = 0; i < MM_QCAMERA_APP_UTEST_MAX_MAIN_LOOP; i++) {
		if(0 != (rc = mm_app_init_preview(cam_id))) {
			goto end;
		}
		if(0 != (rc = mm_app_start_preview(cam_id))) {
			goto end;
		}
		/* sleep 8 seconds */
		usleep(8000000);
		if(0 != (rc = mm_app_start_recording(cam_id))) {
			goto end;
		}
		usleep(1000000);
		if(0 != (rc = mm_app_stop_recording(cam_id))) {
			goto end;
		}
		usleep(8000000);
		if(0 != (rc = mm_app_stop_preview(cam_id))) {
			goto end;
		}
		if(0 != (rc=mm_app_deinit_preview(cam_id))) {
			goto end;
		}
		if(0 != (rc = mm_app_close_preview(cam_id))) {
			goto end;
		}
	}
	if ( 0 != (rc = mm_app_close(cam_id))) {
		CDBG("%s: close cam %d at opmode = %d err,loop=%d, rc=%d\n", 
				 __func__, cam_id, MM_CAMERA_OP_MODE_NOTUSED, i, rc); 
		goto end;
	}
end:
	CDBG("%s:END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_tc_8(mm_camera_app_t *cam_apps, int cam_id, int query_cam_info)
{
	int rc = MM_CAMERA_OK;
	int i;
	printf("Running %s - preview, recording, and snapshot, then preview again \n", __func__); 

	if ( 0 != (rc = mm_app_open(cam_id, MM_CAMERA_OP_MODE_NOTUSED))) {
		CDBG("%s: open cam %d at opmode = %d err, loop=%d, rc=%d\n", 
				 __func__, cam_id, MM_CAMERA_OP_MODE_NOTUSED, i, rc); 
		goto end;
	}

	for(i = 0; i < MM_QCAMERA_APP_UTEST_MAX_MAIN_LOOP; i++) {
		if(0 != (rc = mm_app_init_preview(cam_id))) {
			goto end;
		}
		if(0 != (rc = mm_app_start_preview(cam_id))) {
			goto end;
		}
		/* sleep 8 seconds */
		usleep(8000000);
		if(0 != (rc = mm_app_start_recording(cam_id))) {
			goto end;
		}
		usleep(1000000);
		if(0 != (rc = mm_app_stop_recording(cam_id))) {
			goto end;
		}
		if(0 != (rc = mm_app_stop_preview(cam_id))) {
			goto end;
		}
		if(0!=(rc=mm_app_init_snapshot(cam_id))) {
			goto end;
		}
		if(0 != (rc=mm_app_take_picture(cam_id))) {
			goto end;
		}
		if( 0 != (rc = mm_app_deinit_snahspot(cam_id))) {
			goto end;
		}
		if(0 != (rc = mm_app_start_preview(cam_id))) {
			goto end;
		}
		usleep(8000000);
		if(0 != (rc=mm_app_deinit_preview(cam_id))) {
			goto end;
		}
		if(0 != (rc = mm_app_close_preview(cam_id))) {
			goto end;
		}
	}
	if ( 0 != (rc = mm_app_close(cam_id))) {
		CDBG("%s: close cam %d at opmode = %d err,loop=%d, rc=%d\n", 
				 __func__, cam_id, MM_CAMERA_OP_MODE_NOTUSED, i, rc); 
		goto end;
	}
end:
	CDBG("%s:END, rc=%d\n", __func__, rc);
	return rc;
}


int mm_app_gen_test_cases()
{
	int tc = 0;
	memset(mm_app_tc, 0, sizeof(mm_app_tc));
	if(tc < MM_QCAM_APP_TEST_NUM) mm_app_tc[tc++].f = mm_app_tc_0;
	if(tc < MM_QCAM_APP_TEST_NUM) mm_app_tc[tc++].f = mm_app_tc_1;
	if(tc < MM_QCAM_APP_TEST_NUM) mm_app_tc[tc++].f = mm_app_tc_2;
	if(tc < MM_QCAM_APP_TEST_NUM) mm_app_tc[tc++].f = mm_app_tc_3;
	if(tc < MM_QCAM_APP_TEST_NUM) mm_app_tc[tc++].f = mm_app_tc_4;
	if(tc < MM_QCAM_APP_TEST_NUM) mm_app_tc[tc++].f = mm_app_tc_5;
	if(tc < MM_QCAM_APP_TEST_NUM) mm_app_tc[tc++].f = mm_app_tc_6;
	if(tc < MM_QCAM_APP_TEST_NUM) mm_app_tc[tc++].f = mm_app_tc_7;
	if(tc < MM_QCAM_APP_TEST_NUM) mm_app_tc[tc++].f = mm_app_tc_8;
	return tc;
}

int mm_app_unit_test_entry(mm_camera_app_t *cam_app)
{
	int rc = MM_CAMERA_OK;
	int i, tc = 0;
	int cam_id = 0;

	CDBG("%s:BEGIN\n", __func__);
	tc = mm_app_gen_test_cases();
	for(i = 0; i < tc; i++) {
		mm_app_tc[i].r = mm_app_tc[i].f (cam_app, cam_id, 0);
		if(rc!= MM_CAMERA_OK) {
			CDBG("%s: test case %d error = %d, abort unit testing engine!!!!\n", 
					 __func__, i, mm_app_tc[i].r); 
			rc = mm_app_tc[i].r;
			goto end;
		}
	}
end:
	CDBG("%s:END, TOTAL_TSET_CASE = %d, NUM_TEST_RAN = %d, rc=%d\n", 
			 __func__, tc, i, rc);
	return rc;
}




