/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                 SecureMux

GENERAL DESCRIPTION
 ION manipulation functions for the SecureMux interface.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2013 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================

                            EDIT HISTORY FOR MODULE

$Header:

when       who    what, where, why
--------   ---    ----------------------------------------------------------
09/30/2013  yb    Created
===========================================================================*/
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "common_log.h"
#include <sys/mman.h>
#include <linux/msm_ion.h>
#include "QSEEComAPI.h"
#include <utils/Log.h>
#include "common_log.h"
#include "smux_mem.h"

/*-------------------------------------------------------------------------*/

int32_t smux_ION_memalloc(struct smux_ion_info *handle,
                                uint32_t size)
{
	int32_t ret = 0;
	int32_t iret = 0;
	int32_t fd = 0;
	unsigned char *v_addr;
	struct ion_allocation_data ion_alloc_data;
	int32_t ion_fd;
	int32_t rc;
	struct ion_fd_data ifd_data;
	struct ion_handle_data handle_data;

	/* open ION device for memory management
	 * O_DSYNC -> uncached memory
	*/
	if(handle == NULL)
	{
		LOGE("%s: Error:: null handle received\n", __func__);
		return -1;
	}

	ion_fd  = open("/dev/ion", O_RDONLY);
	if (ion_fd < 0)
	{
		LOGE("%s: Error:: Cannot open ION device\n", __func__);
		return -1;
	}
	handle->ion_sbuffer = NULL;
	handle->ifd_data_fd = 0;

	/* Size of allocation */
	ion_alloc_data.len = (size + 4095) & (~4095);
	/* 4K aligned */
	ion_alloc_data.align = 4096;

	/* memory is allocated from EBI heap */
	ion_alloc_data.heap_id_mask= ION_HEAP(ION_QSECOM_HEAP_ID);

	/* Set the memory to be uncached */
	ion_alloc_data.flags = 0;

	/* IOCTL call to ION for memory request */
	rc = ioctl(ion_fd, ION_IOC_ALLOC, &ion_alloc_data);
	if (rc)
	{
		ret = -1;
	  LOGE("%s: Error while trying to allocate data\n", __func__);
		goto alloc_fail;
	}

	if (ion_alloc_data.handle != NULL) {
		 ifd_data.handle = ion_alloc_data.handle;
	}
	else
	{
	  LOGE("%s: Error:: ION alloc data returned a NULL\n", __func__);
		goto alloc_fail;
	}
	 /* Call MAP ioctl to retrieve the ifd_data.fd file descriptor */
	rc = ioctl(ion_fd, ION_IOC_MAP, &ifd_data);
	if (rc)
	{
		LOGE("%s: Error:: Failed doing ION_IOC_MAP call\n",__func__);
		goto ioctl_fail;
	}

	/* Make the ion mmap call */
	v_addr = (unsigned char *)mmap(NULL, ion_alloc_data.len,
									PROT_READ | PROT_WRITE,
									MAP_SHARED, ifd_data.fd, 0);
	if (v_addr == MAP_FAILED)
	{
		LOGE("%s: Error:: ION MMAP failed\n",__func__);
		ret = -1;
		goto map_fail;
	}
	handle->ion_fd = ion_fd;
	handle->ifd_data_fd = ifd_data.fd;
	handle->ion_sbuffer = v_addr;
	handle->ion_alloc_handle.handle = ion_alloc_data.handle;
	handle->sbuf_len = size;
	return ret;

	map_fail:
	if(handle->ion_sbuffer != NULL)
	{
		ret = munmap(handle->ion_sbuffer, ion_alloc_data.len);
		if (ret)
		{
			LOGE("%s:Error:Failed to unmap memory for load image.ret = %d\n", __func__, ret);
		}
	}

	ioctl_fail:
	handle_data.handle = ion_alloc_data.handle;
	if (handle->ifd_data_fd)
		 close(handle->ifd_data_fd);
	iret = ioctl(ion_fd, ION_IOC_FREE, &handle_data);
	if (iret)
	{
		LOGE("%s: Error::ION FREE ioctl returned error = %d\n", __func__, iret);
	}

	alloc_fail:
	if (ion_fd)
		close(ion_fd);
	return ret;
}
/*-------------------------------------------------------------------------*/
int32_t smux_ion_dealloc(struct smux_ion_info *handle)
{
	struct ion_handle_data handle_data;
	int32_t ret = 0;

	/* Deallocate the memory for the listener */
	ret = munmap(handle->ion_sbuffer,
		(handle->sbuf_len + 4095) & (~4095));
	if (ret)
	{
		LOGE("%s: Error:: Unmapping ION Buffer failed with ret = %d\n", __func__, ret);
	}

	handle_data.handle = handle->ion_alloc_handle.handle;
	close(handle->ifd_data_fd);
	ret = ioctl(handle->ion_fd, ION_IOC_FREE, &handle_data);
	if (ret)
	{
		LOGE("%s: Error:: ION Memory FREE ioctl failed with ret = %d\n", __func__, ret);
	}
	close(handle->ion_fd);
	return ret;
}
/*-------------------------------------------------------------------------*/

int32_t smux_start_app(struct QSEECom_handle **l_QSEEComHandle,
                        const char *appname, int32_t buf_size)
{
	int32_t ret = 0;

	/* start the application */
	ret = QSEECom_start_app(l_QSEEComHandle, "/system/etc/firmware",
		appname, buf_size);
	if (ret)
	{
		LOGE("%s: Loading app -%s failed\n",__func__,appname);
	}

	return ret;
}
/*-------------------------------------------------------------------------*/

int32_t smux_shutdown_app(struct QSEECom_handle **l_QSEEComHandle)
{
	int32_t ret = 0;

	/* shutdown the application */
	if (*l_QSEEComHandle != NULL)
	{
		ret = QSEECom_shutdown_app(l_QSEEComHandle);
		if (ret)
		{
			LOGE("%s: Shutdown app failed with ret = %d\n",__func__,ret);
		}
	}
	else
	{
		LOGE("%s:cannot shutdown as the handle is NULL\n",__func__);
	}
	return ret;
}
