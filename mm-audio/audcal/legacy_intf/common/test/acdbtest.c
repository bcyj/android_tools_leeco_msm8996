/*
* Copyright (c) 2010 Qualcomm Technologies, Inc.
* All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <linux/msm_audio.h>
#include <sys/time.h>

#ifdef _ANDROID_
#include <linux/time.h>
#endif

#include "acdb.h"

#define BUFF_SIZE	4096

void loop(uint8_t *buff);

static struct timeval	large;
static int		check;
static int		prints;
static int		control;

int main(int argc, char **argv)
{
	int			result = 0;
	int			breakloop = 1;
	int			i = 0;
	int			reset = 0;
	int			pause = 0;
	uint8_t			*buff;
	struct timeval		start;
	struct timeval		end;
	struct timeval		diff;

	prints = 1;
	argc--;
	argv++;

	if (argc > 0)
		breakloop = atoi(argv[0]);
	if (argc > 1)
		prints = atoi(argv[1]);
	if (argc > 2)
		reset = atoi(argv[2]);
	if (argc > 3)
		pause = atoi(argv[3]);


	printf("loop = %d, prints = %d, reset = %d, pause = %d\n",
		breakloop, prints, reset, pause);
	if (breakloop < 0 )
		return -1;
	buff = malloc(BUFF_SIZE);
	large.tv_usec = 0;

	if (prints)
		printf("Opening Mixer\n");
	control = msm_mixer_open("/dev/snd/controlC0", 0);

	if (prints)
		printf("initializing acdb\n");
	gettimeofday(&start, NULL);
	result = acdb_ioctl(ACDB_CMD_INITIALIZE, NULL, 0, NULL, 0);
	gettimeofday(&end, NULL);

	timersub(&end, &start, &diff);
	if (prints)
		printf("initializing acdb time: %ldus\n", diff.tv_usec);
	if (result)
		printf("Error: Returned = %d\n", result);


	while (1) {
		loop(buff);
		if (i == reset) {
			printf("Largest time delay: %ldus, on call #%d\n", large.tv_usec, check);
			i = 0;
		} else {
			i++;
		}

		if (breakloop)
			goto done;
		if (pause)
			usleep(pause);
	}

done:
	free(buff);
	return result;
}


void loop(uint8_t *buff)
{
	int				result = 0;
	int				device_id;
	struct timeval			start;
	struct timeval			end;
	struct timeval			diff;
	AcdbVocProcTableCmdType		voctable;
	AcdbVocStrmTableCmdType		vocstrmtable;
	//AcdbVocProcVolTblCmdType	vocvoltable;
	AcdbAudProcTableCmdType		audtable;
	//AcdbAudStrmCmdType		audstrmtable;
	//AcdbAudProcVolTblCmdType	audvoltable;
	AcdbQueryResponseType		response;


	/* Best Case Voc Proc */
	voctable.nTxDeviceId = 0x15;
	voctable.nRxDeviceId = 0x16;
	voctable.nTxDeviceSampleRateId = 0x1F40;
	voctable.nRxDeviceSampleRateId = 0x1F40;
	voctable.nNetworkId = 0x1;
	voctable.nVocProcSampleRateId = 0x1;
	voctable.nBufferLength = BUFF_SIZE;
	voctable.nBufferPointer = buff;


	gettimeofday(&start, NULL);
	result = acdb_ioctl(ACDB_CMD_GET_VOCPROC_COMMON_TABLE,
		(const uint8_t *)&voctable, sizeof(voctable),
		(uint8_t *)&response, sizeof(response));
	gettimeofday(&end, NULL);

	timersub(&end, &start, &diff);
	if (prints)
		printf("acdb time best case VocProc: %ldus\n", diff.tv_usec);
	if (result)
		printf("Error: Returned = %d\n", result);


	/* Worst Case Voc Proc */
	voctable.nTxDeviceId = 0x0D;
	voctable.nRxDeviceId = 0x0E;
	voctable.nTxDeviceSampleRateId = 0xBB80;
	voctable.nRxDeviceSampleRateId = 0xBB80;
	voctable.nNetworkId = 0x3;
	voctable.nVocProcSampleRateId = 0x3;
	voctable.nBufferLength = BUFF_SIZE;
	voctable.nBufferPointer = buff;

	gettimeofday(&start, NULL);
	result = acdb_ioctl(ACDB_CMD_GET_VOCPROC_COMMON_TABLE,
		(const uint8_t *)&voctable, sizeof(voctable),
		(uint8_t *)&response, sizeof(response));
	gettimeofday(&end, NULL);

	timersub(&end, &start, &diff);
	if (prints)
		printf("acdb time worst case VocProc: %ldus\n", diff.tv_usec);
	if (result)
		printf("Error: Returned = %d\n", result);


	/* Worst Case Bin-heap */
	voctable.nTxDeviceId = 0x04;
	voctable.nRxDeviceId = 0x07;
	voctable.nTxDeviceSampleRateId = 0x1F40;
	voctable.nRxDeviceSampleRateId = 0x1F40;
	voctable.nNetworkId = 0x1;
	voctable.nVocProcSampleRateId = 0x2;
	voctable.nBufferLength = BUFF_SIZE;
	voctable.nBufferPointer = buff;

	gettimeofday(&start, NULL);
	result = acdb_ioctl(ACDB_CMD_GET_VOCPROC_COMMON_TABLE,
		(const uint8_t *)&voctable, sizeof(voctable),
		(uint8_t *)&response, sizeof(response));
	gettimeofday(&end, NULL);

	timersub(&end, &start, &diff);
	if (prints)
		printf("acdb time worst case bin-heap VocProc: %ldus\n", diff.tv_usec);
	if (result)
		printf("Error: Returned = %d\n", result);


	/* 48k handset mic */
	audtable.nDeviceId = 0x04;
	audtable.nDeviceSampleRateId = 0xBB80;
	audtable.nApplicationType = 0x1;
	audtable.nBufferLength = BUFF_SIZE;
	audtable.nBufferPointer = buff;

	gettimeofday(&start, NULL);
	audtable.nDeviceId = msm_get_device("handset_tx");
	result = acdb_ioctl(ACDB_CMD_GET_AUDPROC_COMMON_TABLE,
		(const uint8_t *)&audtable, sizeof(audtable),
		(uint8_t *)&response, sizeof(response));
	gettimeofday(&end, NULL);

	timersub(&end, &start, &diff);
	if (prints)
		printf("acdb time 48K Handset Mic AudProc: %ldus\n", diff.tv_usec);
	if (result)
		printf("Error: Returned = %d\n", result);
	if (large.tv_usec < diff.tv_usec) {
		large.tv_usec = diff.tv_usec;
		check = 1;
	}


	/* 48k handset spkr */
	audtable.nDeviceId = 0x07;
	audtable.nDeviceSampleRateId = 0xBB80;
	audtable.nApplicationType = 0x1;
	audtable.nBufferLength = BUFF_SIZE;
	audtable.nBufferPointer = buff;

	gettimeofday(&start, NULL);
	audtable.nDeviceId = msm_get_device("handset_rx");
	result = acdb_ioctl(ACDB_CMD_GET_AUDPROC_COMMON_TABLE,
		(const uint8_t *)&audtable, sizeof(audtable),
		(uint8_t *)&response, sizeof(response));
	gettimeofday(&end, NULL);

	timersub(&end, &start, &diff);
	if (prints)
		printf("acdb time 48K Handset Spkr AudProc: %ldus\n", diff.tv_usec);
	if (result)
		printf("Error: Returned = %d\n", result);
	if (large.tv_usec < diff.tv_usec) {
		large.tv_usec = diff.tv_usec;
		check = 2;
	}


	/* 48k headset mic */
	audtable.nDeviceId = 0x08;
	audtable.nDeviceSampleRateId = 0xBB80;
	audtable.nApplicationType = 0x1;
	audtable.nBufferLength = BUFF_SIZE;
	audtable.nBufferPointer = buff;

	gettimeofday(&start, NULL);
	audtable.nDeviceId = msm_get_device("headset_mono_tx");
	result = acdb_ioctl(ACDB_CMD_GET_AUDPROC_COMMON_TABLE,
		(const uint8_t *)&audtable, sizeof(audtable),
		(uint8_t *)&response, sizeof(response));
	gettimeofday(&end, NULL);

	timersub(&end, &start, &diff);
	if (prints)
		printf("acdb time 48K Headset Mic AudProc: %ldus\n", diff.tv_usec);
	if (result)
		printf("Error: Returned = %d\n", result);
	if (large.tv_usec < diff.tv_usec) {
		large.tv_usec = diff.tv_usec;
		check = 3;
	}


	/* 48k headset stereo */
	audtable.nDeviceId = 0x0A;
	audtable.nDeviceSampleRateId = 0xBB80;
	audtable.nApplicationType = 0x1;
	audtable.nBufferLength = BUFF_SIZE;
	audtable.nBufferPointer = buff;

	gettimeofday(&start, NULL);
	audtable.nDeviceId = msm_get_device("headset_stereo_rx");
	result = acdb_ioctl(ACDB_CMD_GET_AUDPROC_COMMON_TABLE,
		(const uint8_t *)&audtable, sizeof(audtable),
		(uint8_t *)&response, sizeof(response));
	gettimeofday(&end, NULL);

	timersub(&end, &start, &diff);
	if (prints)
		printf("acdb time 48K Headset Stereo AudProc: %ldus\n", diff.tv_usec);
	if (result)
		printf("Error: Returned = %d\n", result);
	if (large.tv_usec < diff.tv_usec) {
		large.tv_usec = diff.tv_usec;
		check = 4;
	}


	/* 48k Mic Spkr Handset VocProc */
	voctable.nTxDeviceId = 0x04;
	voctable.nRxDeviceId = 0x07;
	voctable.nTxDeviceSampleRateId = 0xBB80;
	voctable.nRxDeviceSampleRateId = 0xBB80;
	voctable.nNetworkId = 0x2;
	voctable.nVocProcSampleRateId = 0x1;
	voctable.nBufferLength = BUFF_SIZE;
	voctable.nBufferPointer = buff;

	gettimeofday(&start, NULL);
	voctable.nTxDeviceId = msm_get_device("handset_tx");
	voctable.nRxDeviceId = msm_get_device("handset_rx");
	result = acdb_ioctl(ACDB_CMD_GET_VOCPROC_COMMON_TABLE,
		(const uint8_t *)&voctable, sizeof(voctable),
		(uint8_t *)&response, sizeof(response));
	gettimeofday(&end, NULL);

	timersub(&end, &start, &diff);
	if (prints)
		printf("acdb time Handset 48k GSM VocProc: %ldus\n", diff.tv_usec);
	if (result)
		printf("Error: Returned = %d\n", result);
	if (large.tv_usec < diff.tv_usec) {
		large.tv_usec = diff.tv_usec;
		check = 5;
	}


	/* 48k Mic Spkr Handset VocProc */
	voctable.nTxDeviceId = 0x04;
	voctable.nRxDeviceId = 0x07;
	voctable.nTxDeviceSampleRateId = 0xBB80;
	voctable.nRxDeviceSampleRateId = 0xBB80;
	voctable.nNetworkId = 0x1;
	voctable.nVocProcSampleRateId = 0x1;
	voctable.nBufferLength = BUFF_SIZE;
	voctable.nBufferPointer = buff;

	gettimeofday(&start, NULL);
	voctable.nTxDeviceId = msm_get_device("handset_tx");
	voctable.nRxDeviceId = msm_get_device("handset_rx");
	result = acdb_ioctl(ACDB_CMD_GET_VOCPROC_COMMON_TABLE, 
		(const uint8_t *)&voctable, sizeof(voctable),
		(uint8_t *)&response, sizeof(response));
	gettimeofday(&end, NULL);

	timersub(&end, &start, &diff);
	if (prints)
		printf("acdb time Handset 48k CDMA VocProc: %ldus\n", diff.tv_usec);
	if (result)
		printf("Error: Returned = %d\n", result);
	if (large.tv_usec < diff.tv_usec) {
		large.tv_usec = diff.tv_usec;
		check = 6;
	}


	/* 48k VocProcStrm */
	vocstrmtable.nNetworkId = 0x2;
	vocstrmtable.nVocProcSampleRateId = 0x1;
	vocstrmtable.nBufferLength = BUFF_SIZE;
	vocstrmtable.nBufferPointer = buff;

	gettimeofday(&start, NULL);
	result = acdb_ioctl(ACDB_CMD_GET_VOCPROC_STREAM_TABLE,
		(const uint8_t *)&vocstrmtable,	sizeof(vocstrmtable),
		(uint8_t *)&response, sizeof(response));
	gettimeofday(&end, NULL);

	timersub(&end, &start, &diff);
	if (prints)
		printf("acdb time GSM VocProcStrm: %ldus\n", diff.tv_usec);
	if (result)
		printf("Error: Returned = %d\n", result);
	if (large.tv_usec < diff.tv_usec) {
		large.tv_usec = diff.tv_usec;
		check = 7;
	}




	return;
}
