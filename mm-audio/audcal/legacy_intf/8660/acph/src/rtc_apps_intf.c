/** 
  \file **************************************************************************
 *
 *  A U D I O   C A L I B R A T I O N   P A C K E T   H A N D L E R   
 *
 *DESCRIPTION
 * This file contains the implementation of ACPH 
 *
 *REFERENCES
 * None.
 *
 * Copyright (c) 2010-2011 by Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *******************************************************************************
 */
/*
   -------------------------------
   |Include Files                |
   -------------------------------
   */
#include "acph.h"
#include <ctype.h>
#include <errno.h>
#include "acdb-id-mapper.h"
#include <linux/msm_audio_acdb.h>

/*
   -------------------------------
   |Macros                       |
   -------------------------------
   */

extern uint32_t ACPH_HALF_BUF_LEN;
/*
   ---------------------------------
   |Static Variable Definitions    |
   ---------------------------------
   */
extern char_t * acph_main_buffer;
extern char_t * acph_sub_buffer;

static char* MsmAdieCodecPoke = "/sys/kernel/debug/msm_adie_codec/poke";
static char* MsmAdieCodecPeek = "/sys/kernel/debug/msm_adie_codec/peek";
static char* MsmAdieCodecPower = "/sys/kernel/debug/msm_adie_codec/power";
static char* RtacDev = "/dev/msm_rtac";

#define RTC_MAX_ACTIVE_AUD_DEVICES 4
#define RTC_MAX_ACTIVE_AUD_STREAMS 8
#define RTC_MAX_ACTIVE_VOC_DEVICES 2

/** RTC adm data */
typedef struct _rtc_adm_data
{
   uint32_t topology_id;
   uint32_t afe_port;
	uint32_t	copp;
   uint32_t num_of_popp;
	uint32_t	popp[RTC_MAX_ACTIVE_AUD_STREAMS];
} rtc_adm_data;

/** RTC ADM info as an array of adm data */
typedef struct _rtc_adm
{
	uint32_t	num_of_handles;
	rtc_adm_data handle[RTC_MAX_ACTIVE_AUD_DEVICES];
} rtc_adm;

/** RTC voice data */
typedef struct _rtc_voice_data
{
	uint32_t	tx_topology_id;
	uint32_t	rx_topology_id;
	uint32_t	tx_afe_port;
	uint32_t	rx_afe_port;
	uint16_t	cvs_handle;
	uint16_t	cvp_handle;
} rtc_voice_data;

/** RTC voice info as an array of voice data */
typedef struct _rtc_voice
{
	uint32_t	num_of_voice_combos;
	rtc_voice_data	voice_combo[RTC_MAX_ACTIVE_VOC_DEVICES];
} rtc_voice;

#define RTC_IO_BUF_SIZE 1024
static char rtc_io_buf[RTC_IO_BUF_SIZE];

static rtc_adm active_copp_info;
static rtc_voice active_voice_info;

/*
   --------------------------------------------------
   |Static Function Declarations and Definitions    |
   --------------------------------------------------
   */
int32_t adie_execute_command(void *input_buf_ptr,
                             uint32_t *resp_buf_length_ptr
                             );

extern void create_error_resp (uint32_t error_code,
                               char_t *req_buf_ptr,
                               char_t **resp_buf_ptr,
                               uint32_t *resp_buf_length
                               );

extern void create_suc_resp (uint32_t data_length,
                             char_t *req_buf_ptr,
                             char_t **resp_buf_ptr,
                             uint32_t *resp_buf_length
                             );

/**
 * FUNCTION : query_aud_topology_copp_handles
 *
 * DESCRIPTION : query for audio copp handles with a given device
 *
 * DEPENDENCIES : CSD needs to be available and initialized
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 *
 * Expected data input: None
 *
 * Expected data output:
 *              number of audio copp handles (32-bit)
 *              audio_copp_handle_array[n]
 *      Each element of audio_copp_handle_array is:
 *              topology_id (32-bit)
 *              ac_handle (32-bit)
 *              audproc_copp_id (32-bit)
 *              num_as_handles (32-bit)
 *
 */
void query_aud_topology_copp_handles (char_t *req_buf_ptr,
                                      char_t **resp_buf_ptr,
                                      uint32_t *resp_buf_length
                                      )
{
    uint8_t* nBufferPointer = (uint8_t *)(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION);
    size_t bytes_read = 0;
    uint32_t num_as_handles = 0;
    int i, fd, result;
    uint32_t real_acdb_id = 0;
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr,
                          resp_buf_ptr, resp_buf_length);
        return;
    }

    /** open adm debugfs */
    fd = open(RtacDev, O_RDWR);
    if (fd < 0)
    {
        create_error_resp(ACPH_ERR_CSD_OPEN_HANDLE, req_buf_ptr,
                          resp_buf_ptr, resp_buf_length);
#ifdef LOGE
        LOGE("[ACDB RTC]->(get aud copp handles)->open device control, response [%d]\n", fd);
#endif
        return;
    }

    /* get the copp info from debugfs */
#ifdef ACDB_RTC_DEBUG
    LOGE("[ACDB RTC]->(get aud copp info)->enter ioctl\n");
#endif
    bytes_read = ioctl(fd, AUDIO_GET_RTAC_ADM_INFO, &active_copp_info);
#ifdef ACDB_RTC_DEBUG
    LOGE("[ACDB RTC]->(get aud copp info)->exit ioctl, actual byte read[%d]\n",bytes_read);
#endif
    /* close debugfs after read */
    close(fd);
    if (bytes_read <= 0 || active_copp_info.num_of_handles > RTC_MAX_ACTIVE_AUD_DEVICES)
    {
#ifdef LOGE
        LOGE("[ACDB RTC ERROR]->(get aud copp handles)->bytes read less than 0 or number of active dev > %d\n", 
             RTC_MAX_ACTIVE_AUD_DEVICES);
#endif
        create_error_resp(ACPH_ERR_CSD_AUD_CMD_FAILURE, req_buf_ptr,
                          resp_buf_ptr, resp_buf_length);
        return;
    }

    /* prepare response buffer */
    for (i=0; i<(int)active_copp_info.num_of_handles; i++)
    {
        num_as_handles = active_copp_info.handle[i].num_of_popp;

        nBufferPointer += sizeof(uint32_t);
        memcpy(nBufferPointer,
               &(active_copp_info.handle[i].topology_id),
               sizeof(uint32_t));
        nBufferPointer += sizeof(uint32_t);
        memcpy(nBufferPointer,
               &(active_copp_info.handle[i].copp),
               sizeof(uint32_t));
        nBufferPointer += sizeof(uint32_t);
        memcpy(nBufferPointer,
               &(active_copp_info.handle[i].copp),
               sizeof(uint32_t));
        /* find the number of as handles */
        nBufferPointer += sizeof(uint32_t);
        memcpy(nBufferPointer, &num_as_handles, sizeof(uint32_t));
    }
    memcpy(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,
           &(active_copp_info.num_of_handles), sizeof(uint32_t));
    create_suc_resp(((active_copp_info.num_of_handles)*4)*sizeof(uint32_t)
                    + sizeof(uint32_t), req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

/**
 * FUNCTION : query_aud_copp_stream_handles
 *
 * DESCRIPTION : query for audio stream handles with a given copp handle
 *
 * DEPENDENCIES : CSD needs to be available and initialized
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 *
 * Expected data input:
 *              audio_copp_handle (32-bit)
 *              num_as_handles (32-bit)
 *
 * Expected data output:
 *              number of audio stream handles (32-bit)
 *              audio_stream_handle_array[n]
 *      Each element of audio_stream_handle_array is:
 *              audio_stream_handle (32-bit) (including both session ID and stream ID)
 *
 */
void query_aud_copp_stream_handles (char_t *req_buf_ptr,
                                    char_t **resp_buf_ptr,
                                    uint32_t *resp_buf_length
                                    )
{
    uint32_t ac_handle = 0;
    uint32_t num_as_handles = 0;
    uint8_t* nBufferPointer = (uint8_t *)(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION);
    int i, j;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
           ACPH_DATA_LENGTH_LENGTH);
    if (2 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr,
                          resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr,
                          resp_buf_ptr, resp_buf_length);
        return;
    }

    /**copy value to csd command*/
    memcpy(&ac_handle, 
           req_buf_ptr + ACPH_HEADER_LENGTH, 
           ACPH_CAL_DATA_UNIT_LENGTH);
    for (i=0; i<(int)active_copp_info.num_of_handles; i++)
    {
        if (ac_handle == active_copp_info.handle[i].copp)
        {
            num_as_handles = active_copp_info.handle[i].num_of_popp;
            for (j=0; j<(int)num_as_handles; j++)
            {
                nBufferPointer += sizeof(uint32_t);
                memcpy(nBufferPointer,
                       &(active_copp_info.handle[i].popp[j]),
                       sizeof(uint32_t));
            }
            break;
        }
    }
    memcpy(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,
           &num_as_handles, sizeof(uint32_t));
    create_suc_resp((num_as_handles+1)*sizeof(uint32_t),
                    req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

/**
 * FUNCTION : query_voc_all_active_streams
 *
 * DESCRIPTION : query for all active voice streams
 *
 * DEPENDENCIES : CSD needs to be available and initialized
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 *
 * Expected data input: None
 *
 * Expected data output:
 *              number of streams (32-bit)
 *              voice_stream_array[n]
 *     Each element of voice_stream_array is:
 *              cvs_handle (32-bit)
 *              voc_vs_handle (32-bit)
 *
 */
void query_voc_all_active_streams (char_t *req_buf_ptr,
                                   char_t **resp_buf_ptr,
                                   uint32_t *resp_buf_length
                                   )
{
    /* assumption is there will only be 1 stream, so it will at most
     * have 1 cvs_handle, 1 cvp_handle
     */
    uint8_t* nBufferPointer = (uint8_t *)(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION);
    size_t bytes_read = 0;
    uint32_t num_streams = 0;
    uint32_t my_cvs_handle = 0;
    int i,fd, result;
    uint32_t real_tx_acdb_id = 0, real_rx_acdb_id = 0;
    if (NULL == acph_main_buffer)
    {
#ifdef LOGE
        LOGE("[ACDB RTC ERROR]->(get voc all active strm)->null acph_main_buffer\n");
#endif
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr,
                resp_buf_ptr, resp_buf_length);
        return;
    }
    /** open device control debugfs */
    fd = open(RtacDev, O_RDWR);
    if (fd < 0)
    {
        create_error_resp(ACPH_ERR_CSD_OPEN_HANDLE, req_buf_ptr,
                resp_buf_ptr, resp_buf_length);
#ifdef LOGE
        LOGE("[ACDB RTC ERROR]->(get voc all active strm)->open device control, response [%d]\n", fd);
#endif
        return;
    }

    /* get the voice info from debugfs */
#ifdef ACDB_RTC_DEBUG
    LOGE("[ACDB RTC]->(get voc all active strm)->enter ioctl\n");
#endif
    bytes_read = ioctl(fd, AUDIO_GET_RTAC_VOICE_INFO, &active_voice_info);
#ifdef ACDB_RTC_DEBUG
    LOGE("[ACDB RTC]->(get voc all active strm)->exit ioctl, actual byte read[%d]\n",bytes_read);
#endif
    /* close debugfs after read */
    close(fd);

    if (bytes_read <= 0 || active_voice_info.num_of_voice_combos > RTC_MAX_ACTIVE_VOC_DEVICES)
    {
#ifdef LOGE
        LOGE("[ACDB RTC]->(get voc all active strm)->bytes read less than 0 or number of active dev pair > %d\n", 
             RTC_MAX_ACTIVE_VOC_DEVICES);
#endif
        create_error_resp(ACPH_ERR_CSD_VOC_CMD_FAILURE,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }

    /* prepare response buffer */
    for (i=0; i<(int)active_voice_info.num_of_voice_combos; i++)
    {
        num_streams++;
        nBufferPointer += sizeof(uint32_t);
        /* QACT is expecting a 32-bit number */
        my_cvs_handle = (uint32_t)active_voice_info.voice_combo[i].cvs_handle;
        memcpy(nBufferPointer, &my_cvs_handle, sizeof(uint32_t));
        nBufferPointer += sizeof(uint32_t);
        memcpy(nBufferPointer, &my_cvs_handle, sizeof(uint32_t));
    }
    memcpy(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,
           &num_streams, sizeof(uint32_t));
    create_suc_resp((num_streams*2+1)*sizeof(uint32_t),
                    req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

/**
 * FUNCTION : query_voc_vs_copp_handles
 *
 * DESCRIPTION : query for voice copp handles with a given voc_vs_handle
 *
 * DEPENDENCIES : CSD needs to be available and initialized
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 *
 * Expected data input: voc_vs_handle (32-bit)
 *
 * Expected data output:
 *              number of voice copp handles (32-bit)
 *              voice_copp_handle_array[n]
 *      Each element of voice_copp_handle_array is:
 *              cvp_handle (32-bit)
 *              voc_vc_handle (32-bit)
 *
 */
void query_voc_vs_copp_handles (char_t *req_buf_ptr,
                                char_t **resp_buf_ptr,
                                uint32_t *resp_buf_length
                                )
{
    /* assumption is for each stream (cvs), there will only be 1 copp (cvp)
     */
    uint8_t* nBufferPointer = (uint8_t *)(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION);
    uint32_t num_copps = 0;
    int i;
    uint32_t ulData_Length = 0;
    uint32_t cvs_handle = 0;
    uint16_t my_cvs_handle = 0;
    uint32_t my_cvp_handle = 0;

    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
           ACPH_DATA_LENGTH_LENGTH);
    if (ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr,
                          resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr,
                          resp_buf_ptr, resp_buf_length);
        return;
    }

    memcpy(&cvs_handle, req_buf_ptr + ACPH_HEADER_LENGTH, 
           ACPH_CAL_DATA_UNIT_LENGTH);
    /* QACT is sending down a 32-bit number */
    my_cvs_handle = (uint16_t) (cvs_handle & 0x0000FFFF);
     /* prepare response buffer */
    for (i=0; i<(int)active_voice_info.num_of_voice_combos; i++)
    {
        if (my_cvs_handle == active_voice_info.voice_combo[i].cvs_handle)
        {
            num_copps++;
            nBufferPointer += sizeof(uint32_t);
            /* QACT is expecting a 32-bit number */
            my_cvp_handle = (uint32_t)active_voice_info.voice_combo[i].cvp_handle;
            memcpy(nBufferPointer, &my_cvp_handle, sizeof(uint32_t));
            nBufferPointer += sizeof(uint32_t);
            memcpy(nBufferPointer, &my_cvp_handle, sizeof(uint32_t));
        }
    }
    memcpy(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,
           &num_copps, sizeof(uint32_t));
    create_suc_resp((num_copps*2+1)*sizeof(uint32_t),
                    req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

/**
 * FUNCTION : query_voc_vc_topology
 *
 * DESCRIPTION : query for voice devices with a given voc_vc_handle
 *
 * DEPENDENCIES : CSD needs to be available and initialized
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 *
 * Expected data input: voc_vc_handle (32-bit)
 *
 * Expected data output:
 *              number of voice device pairs (32-bit)
 *              voice_device_array[n]
 *      Each element of voice_device_array is:
 *              voice_rx_topology (32-bit)
 *              voice_tx_topology (32-bit)
 *
 */
void query_voc_vc_topology (char_t *req_buf_ptr,
                           char_t **resp_buf_ptr,
                           uint32_t *resp_buf_length
                           )
{
    /* assumption is there will only be 1 stream, so it will at most
     * have 1 cvs_handle, 1 cvp_handle
     */
    uint8_t* nBufferPointer = (uint8_t *)(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION);
    uint32_t num_topology_pairs = 0;
    int i;
    uint32_t ulData_Length = 0;
    uint32_t cvp_handle = 0;
    uint16_t my_cvp_handle = 0;

    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
           ACPH_DATA_LENGTH_LENGTH);
    if (ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr,
                          resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }

    memcpy(&cvp_handle, req_buf_ptr + ACPH_HEADER_LENGTH, 
           ACPH_CAL_DATA_UNIT_LENGTH);
    /* QACT is sending down a 32-bit number */
    my_cvp_handle = (uint16_t) (cvp_handle & 0x0000FFFF);
    /* prepare response buffer */
    for (i=0; i<(int)active_voice_info.num_of_voice_combos; i++)
    {
        if (my_cvp_handle == active_voice_info.voice_combo[i].cvp_handle)
        {
            num_topology_pairs++;
            nBufferPointer += sizeof(uint32_t);
            memcpy(nBufferPointer, &active_voice_info.voice_combo[i].rx_topology_id,
                    sizeof(uint32_t));
            nBufferPointer += sizeof(uint32_t);
            memcpy(nBufferPointer, &active_voice_info.voice_combo[i].tx_topology_id,
                    sizeof(uint32_t));
        }
    }
    memcpy(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,
           &num_topology_pairs, sizeof(uint32_t));
    create_suc_resp((num_topology_pairs*2+1)*sizeof(uint32_t),
                    req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

/*
   ADIE execute command
   */
int32_t adie_execute_command(void *input_buf_ptr,
                             uint32_t *resp_buf_length_ptr)
{
    int16_t nCommand = 0;
    int32_t result = ACPH_SUCCESS;
    int     fd = -1;

    memcpy(&nCommand,input_buf_ptr, ACPH_COMMAND_ID_LENGTH);
    switch(nCommand)
    {
        /** this is the init on LA */
        case ACPH_CMD_ADIE_DAL_ATTACH:
            {
                break;
            }
        /** this is the deinit on LA */
        case ACPH_CMD_ADIE_DAL_DETACH:
            {
                break;
            }
        case ACPH_CMD_GET_ADIE_REGISTER:
            {
                if(resp_buf_length_ptr!=NULL)
                {
                    int32_t nReqBufLen = 0;
                    int32_t nRespBufLen = 0;
                    int32_t lRegValue = 0;
                    uint32_t regAddr = 0;
                    uint32_t regMask = 0;
                    size_t numBytes = 0;
                    int errval = 0;
                    uint32_t nOutputBufPos = (uint32_t)acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
                    char_t *pInputBuf = (char_t *)input_buf_ptr;

                    memcpy(&nReqBufLen,
                           (pInputBuf+ACPH_DATA_LENGTH_POSITION),
                           ACPH_DATA_LENGTH_LENGTH);
                    if(2*ACPH_CAL_DATA_UNIT_LENGTH > nReqBufLen)
                    {
#ifdef LOGE
                        LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->insufficient length of req buffer to get data\n");
#endif
                        result = ACPH_ERR_LENGTH_NOT_MATCH;
                    }
                    else
                    {
                        memcpy(&regAddr,
                               (pInputBuf+ACPH_HEADER_LENGTH),
                               ACPH_CAL_DATA_UNIT_LENGTH);
                        memcpy(&regMask,
                               (pInputBuf+ACPH_HEADER_LENGTH+ACPH_CAL_DATA_UNIT_LENGTH),
                               ACPH_CAL_DATA_UNIT_LENGTH);

                        asprintf(rtc_io_buf, "0x%x", regAddr);
                        fd = open(MsmAdieCodecPeek, O_RDWR);
                        if(fd < 0)
                        {
                            result = ACPH_ERR_ADIE_INIT_FAILURE;
#ifdef LOGE
                            LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->ERROR! cannot open adie peek %d", fd);
#endif
                            break;
                        }
                        numBytes = write(fd, rtc_io_buf, strlen(rtc_io_buf));
                        /** make sure the write is successful */
                        if (numBytes < strlen(rtc_io_buf))
                        {
                            result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
#ifdef LOGE
                            LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->ERROR! length of written bytes does not match expected value %d %d", numBytes, strlen(rtc_io_buf));
#endif
                            close(fd);
                            break;
                        }
                        numBytes = read(fd, rtc_io_buf, sizeof(uint32_t));
                        close(fd);
                        /** make sure the read is successful */
                        if (numBytes <= 0 || numBytes >= 80)
                        {
                            result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
#ifdef LOGE
                            LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->ERROR! length of written bytes does not match expected value %d", numBytes);
#endif
                            break;
                        }
                        rtc_io_buf[numBytes] = '\0';
                        /* reset errno first */
                        errno = 0;
                        lRegValue = strtol(rtc_io_buf, NULL, 0);
                        errval = errno;

                        /** make sure the conversion is successful */
                        if (errval != 0)
                        {
                            result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
#ifdef LOGE
                            LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->ERROR! get adie register strtol() failed %d", errval);
#endif
                            break;
                        }
                        /* return a masked value */
                        lRegValue &= regMask;
                        memcpy((void *)nOutputBufPos,(const void *)&lRegValue, sizeof(uint32_t));
                        nRespBufLen = ACPH_CAL_DATA_UNIT_LENGTH;
                        memcpy((void *)resp_buf_length_ptr, (const void *)&nRespBufLen, sizeof(int32_t));
                    }
                }//check for resp_buf_length_ptr
                break;
            }
        case ACPH_CMD_SET_ADIE_REGISTER:
            {
                if(NULL!=resp_buf_length_ptr)
                {
                    int32_t nReqBufLen = 0;
                    int32_t nRespBufLen = 0;
                    uint32_t ulRegValue = 0;
                    uint32_t regAddr = 0;
                    uint32_t regMask = 0;
                    size_t numBytes = 0;
                    char_t *pInputBuf = (char_t *)input_buf_ptr;

                    memcpy(&nReqBufLen,
                            (pInputBuf+ACPH_DATA_LENGTH_POSITION),
                            ACPH_DATA_LENGTH_LENGTH);
                    if(3*ACPH_CAL_DATA_UNIT_LENGTH > nReqBufLen)
                    {
#ifdef LOGE
                        LOGE("[ACPH_CMD_SET_ADIE_REGISTER]->insufficient length of req buffer to get data\n");
#endif
                        result = ACPH_ERR_LENGTH_NOT_MATCH;
                    }
                    else
                    {
                        memcpy(&regAddr,
                               (pInputBuf+ACPH_HEADER_LENGTH),
                               ACPH_CAL_DATA_UNIT_LENGTH);
                        memcpy(&regMask,
                               (pInputBuf+ACPH_HEADER_LENGTH+ACPH_CAL_DATA_UNIT_LENGTH),
                               ACPH_CAL_DATA_UNIT_LENGTH);
                        memcpy(&ulRegValue,
                               (pInputBuf+ACPH_HEADER_LENGTH + 2*ACPH_CAL_DATA_UNIT_LENGTH),
                               ACPH_CAL_DATA_UNIT_LENGTH);
                        /* set the value as masked one*/
                        ulRegValue &= regMask;
                        asprintf(rtc_io_buf, "0x%x 0x%x", regAddr, ulRegValue);
                        fd = open(MsmAdieCodecPoke, O_RDWR);
                        if(fd < 0)
                        {
                            result = ACPH_ERR_ADIE_INIT_FAILURE;
#ifdef LOGE
                            LOGE("[ACPH_CMD_SET_ADIE_REGISTER]->ERROR! cannot open adie poke %d", fd);
#endif
                            break;
                        }
                        numBytes = write(fd, rtc_io_buf, strlen(rtc_io_buf));
                        close(fd);
                        /** make sure the write is successful */
                        if (numBytes < strlen(rtc_io_buf))
                        {
                            result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
#ifdef LOGE
                            LOGE("[ACPH_CMD_SET_ADIE_REGISTER]->ERROR! set adie register failed, numBytes %d", numBytes);
#endif
                        }
                        memcpy((void *)resp_buf_length_ptr, (const void *)&nRespBufLen, sizeof(uint32_t));
                    }
                }//end of checking for resp_buf_length_ptr
                break;
            }
        case ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS:
            {
                if(NULL!=resp_buf_length_ptr)
                {
                    int32_t nReqBufLen = 0;
                    int32_t nRespBufLen = 0;
                    uint32_t nOutputBufPos = (uint32_t) acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
                    int32_t nTotalRegisters = 0;
                    int32_t lRegValue = 0;
                    uint32_t regAddr = 0;
                    uint32_t regMask = 0;
                    int errval = 0;
                    size_t numBytes = 0;
                    int8_t i=0;
                    char_t *pInputBuf = (char_t *)input_buf_ptr;
                    char_t *pCurInputBuf = NULL;

                    memcpy(&nReqBufLen,
                            (pInputBuf+ACPH_DATA_LENGTH_POSITION),
                            ACPH_DATA_LENGTH_LENGTH);
                    if(ACPH_CAL_DATA_UNIT_LENGTH>nReqBufLen)
                    {
#ifdef LOGE
                        LOGE("[ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS]->insufficient length of req buffer to get data\n");
#endif
                        result = ACPH_ERR_LENGTH_NOT_MATCH;
                    }
                    else
                    {
                        pCurInputBuf = pInputBuf + ACPH_HEADER_LENGTH;
                        memcpy(&nTotalRegisters,pCurInputBuf,ACPH_CAL_DATA_UNIT_LENGTH);
                        pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;
                        /** Check for the following conditions
                          1.  If the output buffer has enough space to store all the data requested
                          2.  If there is enough input data (register ID and mask) for the total number of
                          registers i.e. nTotalRegisters
                          3.  Value for at least 1 register is being requested i.e. nTotalRegisters>0
                          */
                        if(
                                (0<nTotalRegisters)&&
                                ((nTotalRegisters*2*ACPH_CAL_DATA_UNIT_LENGTH) +ACPH_CAL_DATA_UNIT_LENGTH ==nReqBufLen)&&
                                (ACPH_BUFFER_LENGTH >= nReqBufLen)
                          )
                        {
                            for(i=0;i<nTotalRegisters;i++)
                            {
                                memcpy(&regAddr,pCurInputBuf,ACPH_CAL_DATA_UNIT_LENGTH);
                                pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;
                                memcpy(&regMask,pCurInputBuf,ACPH_CAL_DATA_UNIT_LENGTH);
                                pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;

                                asprintf(rtc_io_buf, "0x%x", regAddr);
                                /** debugfs file handle has to be open and close
                                 * for each read/write access
                                 */
                                fd = open(MsmAdieCodecPeek, O_RDWR);
                                if(fd < 0)
                                {
                                    result = ACPH_ERR_ADIE_INIT_FAILURE;
#ifdef LOGE
                                    LOGE("[ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS]->ERROR! cannot open adie peek %d", fd);
#endif
                                    break;
                                }
                                numBytes = write(fd, rtc_io_buf, strlen(rtc_io_buf));
                                /** make sure the write is successful */
                                if (numBytes < strlen(rtc_io_buf))
                                {
#ifdef LOGE
                                    LOGE("[ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS]->get multi register write failed, numBytes: %d\n", numBytes);
#endif
                                    result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
                                    close(fd);
                                    continue;
                                }
                                numBytes = read(fd, rtc_io_buf, sizeof(uint32_t));
                                close(fd);
                                /** make sure the read is successful */
                                if (numBytes <= 0 || numBytes >= 80)
                                {
#ifdef LOGE
                                    LOGE("[ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS]->get multi register read failed, numBytes: %d\n", numBytes);
#endif
                                    result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
                                    continue;
                                }
                                rtc_io_buf[numBytes] = '\0';
                                /* reset errno first */
                                errno = 0;
                                lRegValue = strtol(rtc_io_buf, NULL, 0);
                                errval = errno;

                                /** make sure the conversion is successful */
                                if (errval != 0)
                                {
                                    result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
#ifdef LOGE
                                    LOGE("[ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS]->ERROR! get multi adie register strtol() failed %d", errval);
#endif
                                    continue;
                                }
                                /* return a masked value */
                                lRegValue &= regMask;
                                memcpy((void *)nOutputBufPos,(const void *)&lRegValue, sizeof(uint32_t));
                                nOutputBufPos += ACPH_CAL_DATA_UNIT_LENGTH;
                                usleep(30);
                            }//end of for loop
                            if(ACPH_SUCCESS==result)
                            {
                                nRespBufLen = sizeof(uint32_t)*(i-1);
                            }
                            memcpy((void *)resp_buf_length_ptr,(const void *)&nRespBufLen,sizeof(int32_t));
                        }//end of checking for nTotalRegisters and sizes of input and output buffers
                        else
                        {
#ifdef LOGE
                            LOGE("[ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS]->Error in lengths of input or output buffers or total registers\n");
#endif
                            result = ACPH_ERR_UNKNOWN_REASON;
                        }
                    }
                }//end of checking for resp_buf_length_ptr
                break;
            }
        case ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS:
            {
                if(NULL!=resp_buf_length_ptr)
                {
                    int32_t nReqBufLen = 0;
                    int32_t nRespBufLen = 0;
                    char_t *pInputBuf = (char_t *)input_buf_ptr;
                    char_t *pCurInputBuf = NULL;
                    int32_t nTotalRegisters = 0;
                    int32_t nTotalRegistersSet = 0;
                    uint32_t ulRegValue = 0;
                    uint32_t regAddr = 0;
                    uint32_t regMask = 0;
                    size_t numBytes = 0;
                    int8_t i=0;

                    memcpy(&nReqBufLen,
                            (pInputBuf+ACPH_DATA_LENGTH_POSITION),
                            ACPH_DATA_LENGTH_LENGTH);
                    if(ACPH_CAL_DATA_UNIT_LENGTH>nReqBufLen)
                    {
#ifdef LOGE
                        LOGE("[ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS]->insufficient length of req buffer to get data\n");
#endif
                        result = ACPH_ERR_LENGTH_NOT_MATCH;
                    }
                    else
                    {
                        pCurInputBuf = pInputBuf + ACPH_HEADER_LENGTH;
                        memcpy(&nTotalRegisters,pCurInputBuf,ACPH_CAL_DATA_UNIT_LENGTH);
                        pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;

                        /** Check for the following conditions
                          1.  nTotalRegisters is a non null quantity.
                          2.  Length of the remainder of the req_buf is nTotalRegisters * length of data for a register-mask-value 3-tuple
                          */
                        if(
                                (0<nTotalRegisters)&&
                                (nReqBufLen==(nTotalRegisters*3*ACPH_CAL_DATA_UNIT_LENGTH)+ ACPH_CAL_DATA_UNIT_LENGTH)
                          )
                        {
                            for(i=0;i<nTotalRegisters;i++)
                            {
                                memcpy(&regAddr,pCurInputBuf,ACPH_CAL_DATA_UNIT_LENGTH);
                                pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;
                                memcpy(&regMask,pCurInputBuf,ACPH_CAL_DATA_UNIT_LENGTH);
                                pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;

                                memcpy(&ulRegValue, pCurInputBuf, ACPH_CAL_DATA_UNIT_LENGTH);
                                pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;
                                /* set the value as masked one*/
                                ulRegValue &= regMask;

                                asprintf(rtc_io_buf, "0x%x 0x%x", regAddr, ulRegValue);
                                /** debugfs file handle has to be open and close
                                 * for each read/write access
                                 */
                                fd = open(MsmAdieCodecPoke, O_RDWR);
                                if(fd < 0)
                                {
                                    result = ACPH_ERR_ADIE_INIT_FAILURE;
#ifdef LOGE
                                    LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->ERROR! cannot open adie poke %d", fd);
#endif
                                    break;
                                }
                                numBytes = write(fd, rtc_io_buf, strlen(rtc_io_buf));
                                close(fd);
                                /** make sure the write is successful */
                                if (numBytes < strlen(rtc_io_buf))
                                {
#ifdef LOGE
                                    LOGE("[ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS]->set multi register failed, numBytes %d\n", numBytes);
#endif
                                    result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
                                    continue;
                                }
                                if(result == ACPH_SUCCESS)
                                {
                                    ++nTotalRegistersSet;
                                }
                                usleep(30);
                            }//end of for loop
                            memcpy((void *)resp_buf_length_ptr, (const void *)&nRespBufLen, sizeof(int32_t));
                        }//end of checking for input buf length and total registers
                        else
                        {
#ifdef LOGE
                            LOGE("[ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS]->Error in lengths of input or output buffers or total registers\n");
#endif
                            result = ACPH_ERR_UNKNOWN_REASON;
                        }
                    }
                }//end of checking for resp_buf_length_ptr
                break;
            }//end of case ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS
        default:
            {
#ifdef LOGE
                LOGE("Cannot recognize the ACPH_ADIE command\n");
#endif
                break;
            }
    }
    return result;
}
