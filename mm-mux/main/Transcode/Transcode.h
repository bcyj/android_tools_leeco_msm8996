/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                 Securemux

GENERAL DESCRIPTION
 Dummy Transcode class to handle secure mpeg2 mux output buffers
  for testing purposes.

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
#ifndef TRANSCODE_H_
#define TRANSCODE_H_

#include <stdio.h>
#include <stdlib.h>
#include "comdef.h"
#include <ctype.h>
#include <linux/msm_ion.h>
#include <utils/Log.h>
#include "common_log.h"

// Data structures
struct qsee_ion_info {
   int32_t ion_fd;
   int32_t ifd_data_fd;
   struct ion_handle_data ion_alloc_handle;
   unsigned char * ion_sbuffer;
   uint32_t sbuf_len;
};

class Transcode
{
public:
	Transcode(uint32_t buffers_num,uint32_t size_of_buffers, bool is_secure, const char* fileName):
		num_of_buffers(buffers_num),buffer_size(size_of_buffers),output_stream(NULL),is_secure(is_secure)
	{
		buffer_array = new uint32_t[num_of_buffers];
		if(allocate_buffers(num_of_buffers, buffer_size, is_secure, buffer_array) <= 0)
		{
			LOGE("error allocate buffer\n");
		}
		output_stream = fopen(fileName, "wb+");
	}

	~Transcode()
	{
		deallocate_buffers(num_of_buffers, buffer_array, is_secure);
		if(output_stream != NULL)
		{
			fclose(output_stream);
			output_stream = NULL;
		}
	}

	uint32_t GetBuffer()
	{
		return buffer_array[0];
	}

	void SendCommand(void* buffer ,uint32_t len)
	{
		if(output_stream && ((struct qsee_ion_info*)buffer)->ion_sbuffer)
		{
			fwrite(((struct qsee_ion_info*)buffer)->ion_sbuffer, 1 , len, output_stream);
		}
	}

	uint32_t allocate_buffers (uint32_t num_buffers, uint32_t buffer_size,  bool is_secure,  uint32_t* buffers_array);
	void deallocate_buffers (uint32_t num_buffers, uint32_t* buffers_array, bool is_secure);
	uint32_t* buffer_array;
	uint32_t num_of_buffers;
	uint32_t buffer_size;
	FILE *output_stream;
	bool is_secure;
};

#endif /* TRANSCODE_H_ */

