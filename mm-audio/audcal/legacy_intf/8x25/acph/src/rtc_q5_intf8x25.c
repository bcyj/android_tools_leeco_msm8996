/*============================================================================

FILE:       rtc_q5_intf.c

DESCRIPTION: abstract transport layer interface file.

PUBLIC CLASSES:  Not Applicable

INITIALIZATION AND SEQUENCING REQUIREMENTS:  N/A

Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

#include <stddef.h>
#include <string.h> /* memcpy */
#include <ctype.h>
#include <errno.h>

#include "acph8x25.h"
#include "rtc_q5_intf8x25.h"

#define RTC_MAX_EQ_COEFF_SIZE 264
#define RTC_MAX_ACTIVE_AUD_DEVICES 4
#define TEMP_BUF_LEN 128
// Enable this to get data sent/received to/from driver.
//#define RTC_DEBUG

/* Struture equalizer */
typedef struct {
    uint16_t mbeqflag;
    uint16_t mbeqnumbands;
    uint16_t mbcoeffs[RTC_MAX_EQ_COEFF_SIZE];
} rtc_eq_param_type;

typedef enum _rtcoperationmode {
    RTC_GET_PARAM_TYPE = 0,
    RTC_SET_PARAM_TYPE = 1
} rtcoperationmode;

typedef struct _getsetabidread{
    uint32_t command_id;
    uint32_t device_id;
    uint32_t ab_id;
    uint32_t i_id;
    uint32_t error_code;
}getsetabidread;

typedef struct _getsetabidwrite{
    uint32_t command_id;
    uint32_t device_id;
    uint32_t ab_id;
    uint32_t i_id;
}getsetabidwrite;

/*===========================================================================
Static Function Declarations and Definitions
===========================================================================*/

extern void create_error_resp (
        uint32_t error_code,
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        );

extern void create_suc_resp (
        uint32_t data_length,
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        );

/*===========================================================================
  STATIC VARIABLES
  ===========================================================================*/

// DebugFS to get the list of active devices
static char* rtcdevctrl = "/sys/kernel/debug/rtc_get_device";
// DebugFS to get/set abid and also the error codes from driver
static char* rtcabidintf = "/sys/kernel/debug/get_set_abid";
// DebugFS to get/set actual abid data
static char* rtcabiddataintf = "/sys/kernel/debug/get_set_abid_data";

extern char_t * acph_main_buffer;

static interface_size_map_type interface_size_map[]=
{
    {INF(IID_AUDIO_IIR_COEFF)}, // ABID_AUDIO_STF_RX, ABID_AUDIO_IIR_RX, ABID_AUDIO_IIR_TX
    {INF(IID_RTC_MBADRC_PARAMETERS)}, //ABID_AUDIO_MBADRC_RX
    {INF(IID_AUDIO_AGC_PARAMETERS)}, //ABID_AUDIO_AGC_TX
    {INF(IID_NS_PARAMETERS)}, //ABID_AUDIO_NS_TX
    {INF(IID_AUDIO_RTC_AGC_PARAMETERS)}, //ABID_AUDIO_AGC_TX
    {INF(IID_AUDIO_RTC_VOLUME_PAN_PARAMETERS)}, // ABID_AUDIO_RTC_VOLUME_PAN_RX
    {INF(IID_AUDIO_RTC_SPA_PARAMETERS)}, // ABID_AUDIO_RTC_SPA
    {INF(IID_AUDIO_RTC_EQUALIZER_PARAMETERS)}, //ABID_AUDIO_RTC_EQUALIZER_PARAMETERS
    {INF(IID_AUDIO_RTC_TX_IIR_COEFF)}, // ABID_AUDIO_IIR_TX
};

/*===========================================================================
  INTERNAL FUNCTIONS
  ===========================================================================*/

static uint32_t rtc_get_interface_size(uint32_t inf_id)
{

    uint32_t i=0;
    uint32_t inf_size = 0;
    for (i=0;i<ARRAY_SIZE(interface_size_map);++i)
    {
        if (interface_size_map[i].interface_id==inf_id)
        {
            inf_size = interface_size_map[i].interface_size;
            break;
        }
    }
    return inf_size;
}

static uint32_t rtc_get_error_code_from_abid_intf()
{
    getsetabidread abid_read_buf;
    int fd_abid = 0;
    size_t bytes_read = 0;
    fd_abid = open(rtcabidintf, O_RDONLY);
    if (fd_abid < 0)
    {
        LOGE("libaudcal: rtc_get_error_code_from_abid_intf() - Failed to open file %s for read\n", rtcabidintf);
        return ACPH_ERR_CSD_OPEN_HANDLE;
    }
    bytes_read = read(fd_abid,&abid_read_buf,sizeof(getsetabidread));

    if(bytes_read <= 0)
    {
        LOGE("libaudcal: rtc_get_error_code_from_abid_intf() - Failed to read %s \n", rtcabidintf);
        return ACPH_ERR_CSD_OPEN_HANDLE;
    }

    LOGE("libaudcal: Read bytes %d, Error_code is %d\n",bytes_read,abid_read_buf.error_code);
    return abid_read_buf.error_code;

}

/**
 * FUNCTION : rtc_transfer_q5_cal_data
 *
 * DESCRIPTION : Get/Set real time calibration data
 *
 * DEPENDENCIES : DSP must be active, Audcal should be initialized
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *   mode - indicating set/get operation
 *          0 - get_param
 *          1 - set_param
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
static void rtc_transfer_q5_cal_data(
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length,
        rtcoperationmode mode
        )
{
    /*
    *  The request buffer should be in following format:
    *  <CMD_ID: 2 bytes, DATA_LENGTH:4 bytes, DATA>
    *  The format of DATA is:
    *  Device ID - 4 bytes
    *  Algorithm Block ID - 4bytes
    *  Interface ID - 4 bytes
    */
    uint32_t uldata_length = 0;
    uint32_t device_id = 0;
    uint32_t ab_id = 0;
    uint32_t i_id = 0;
    uint32_t output_len = 0;

    int fd;
    int fd_abid = 0;
    size_t bytes_read = 0;
    uint32_t i=0;

    uint32_t inf_size = 0;
    uint32_t command_id = 0;
    uint32_t error_code = 0;
    uint32_t data_offset = 0;
    uint32_t data_len = 0;

    getsetabidwrite abid_write_buf;
    rtc_eq_param_type* eqparams;

    #ifdef RTC_DEBUG
    char_t *temp_buf;
    #endif

    memcpy(&uldata_length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (3 * ACPH_CAL_DATA_UNIT_LENGTH > uldata_length)
    {
        // command parameter missing
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }

    // get device ID
    memcpy(&device_id,
            req_buf_ptr + ACPH_HEADER_LENGTH,
            sizeof(uint32_t));
    // get ABID ID
    memcpy(&ab_id,
            req_buf_ptr + ACPH_HEADER_LENGTH + sizeof(uint32_t),
            sizeof(uint32_t));
    // get Interface ID
    memcpy(&i_id,
            req_buf_ptr + ACPH_HEADER_LENGTH + 2*sizeof(uint32_t),
            sizeof(uint32_t));

    // we are expected to set command_id, device_id,ab_id and i_id in the following debugfs before querying data
    fd_abid = open(rtcabidintf, O_WRONLY);
    if (fd_abid < 0)
    {
        LOGE("libaudcal: Failed to open file %s\n", rtcabidintf);
        create_error_resp(ACPH_ERR_CSD_OPEN_HANDLE, req_buf_ptr,
                resp_buf_ptr, resp_buf_length);
        return;
    }

    if(mode == RTC_GET_PARAM_TYPE)
    {
        command_id = ACPH_CMD_RTC_GET_CAL_DATA;
    }
    else
    {
        command_id = ACPH_CMD_RTC_SET_CAL_DATA;
    }

    // setting command_id, device_id, ab_id and i_id
    abid_write_buf.command_id = command_id;
    abid_write_buf.device_id = device_id;
    abid_write_buf.ab_id = ab_id;
    abid_write_buf.i_id = i_id;

    LOGE("libaudcal: Writing into file %s -> %d %d %08X %08X\n",rtcabidintf,command_id,device_id,ab_id,i_id);

    bytes_read = write(fd_abid,&abid_write_buf,sizeof(getsetabidwrite));
    if(bytes_read <= 0)
    {
        create_error_resp(ACPH_ERR_CSD_OPEN_HANDLE, req_buf_ptr,
                    resp_buf_ptr, resp_buf_length);
        return;
    }
    // we need not close the file, as said by driver team.
    //close(fd_abid);

    error_code = rtc_get_error_code_from_abid_intf();

    if(error_code != RTC_SUCCESS)
    {
        create_error_resp(error_code, req_buf_ptr,
                    resp_buf_ptr, resp_buf_length);
        return;
    }

    if (mode == RTC_GET_PARAM_TYPE)
    {
        // opening the actual interface to get data
        fd = open(rtcabiddataintf, O_RDONLY);
        if (fd < 0)
        {
            LOGE("libaudcal: rtc_transfer_q5_cal_data() - Failed to open file %s for GET\n", rtcabiddataintf);
            create_error_resp(ACPH_ERR_CSD_OPEN_HANDLE, req_buf_ptr,
                    resp_buf_ptr, resp_buf_length);
            return;
        }

        //rtc_get_interface_size(i_id) gives the actual length to be returned back.
        inf_size = rtc_get_interface_size(i_id);

        // read the req data
        bytes_read = read(fd, acph_main_buffer + ACPH_ACDB_BUFFER_POSITION, inf_size);

        // check if the data is got properly
        error_code = rtc_get_error_code_from_abid_intf();

        // sending all zeros if the filter is disabled (RTC_DSP_FEATURE_NOT_AVAILABLE,RTC_DSP_FAILURE signify disable)
        if(bytes_read <= 0){
            if(error_code == RTC_DSP_FEATURE_NOT_AVAILABLE || error_code == RTC_DSP_FAILURE){
                LOGE("libaudcal: rtc_transfer_q5_cal_data() - ABID: %08X -> Inactive \n",ab_id);
                memset(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,0,inf_size);
                create_suc_resp(inf_size,req_buf_ptr,
                resp_buf_ptr, resp_buf_length);
                return;
            }
        }

        if(error_code != RTC_SUCCESS){
            LOGE("libaudcal: rtc_transfer_q5_cal_data() - Error in GET -> %d\n",error_code);
            create_error_resp(error_code, req_buf_ptr,
                resp_buf_ptr, resp_buf_length);
            return;
        }

        #ifdef RTC_DEBUG
        temp_buf = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;

        LOGE("Data for GET is\n");
        for(i=0;i<inf_size;i++){
            LOGE("-%x- ",*(temp_buf+i));
        }
        LOGE("\n");
        #endif

        if ( IID_AUDIO_RTC_EQUALIZER_PARAMETERS   == i_id &&
              ABID_AUDIO_RTC_EQUALIZER_PARAMETERS  == ab_id
            )
        {
            eqparams =(rtc_eq_param_type *)(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION);
            // 6 words NumCoeff per band
            // 4 words DenCoeff per band
            // 1 word Shiftfactor
            inf_size = (eqparams->mbeqnumbands * (6+4+1)* 2);
            inf_size += 4; //sizeof((enable_flag) + sizeof(numBands)
            LOGE("libaudcal: rtc_transfer_q5_cal_data(get) - RTC_EQUALIZER_PARAMETERS inf_size is -> %d",inf_size);
        }

        create_suc_resp(inf_size,req_buf_ptr,
                resp_buf_ptr, resp_buf_length);

    }

    else // Set_Param
    {
        // opening the actual interface to set data
        fd = open(rtcabiddataintf, O_WRONLY);
        if (fd < 0)
        {
            LOGE("libaudcal: rtc_transfer_q5_cal_data() - Failed to open file %s for SET\n", rtcabiddataintf);
            create_error_resp(ACPH_ERR_CSD_OPEN_HANDLE, req_buf_ptr,
                    resp_buf_ptr, resp_buf_length);
            return;
        }
        data_offset = (ACPH_HEADER_LENGTH + 3*ACPH_CAL_DATA_UNIT_LENGTH);
        data_len = (uldata_length - 3*ACPH_CAL_DATA_UNIT_LENGTH);

        LOGE("Got data length from PC: %d\n",data_len);

        #ifdef RTC_DEBUG
        temp_buf = req_buf_ptr + data_offset;

        LOGE("Data for SET is\n");
        for(i=0;i<data_len;i++){
            LOGE("-%x-",*(temp_buf+i));
        }
        LOGE("\n");
        #endif

        if ( IID_AUDIO_RTC_EQUALIZER_PARAMETERS   == i_id &&
              ABID_AUDIO_RTC_EQUALIZER_PARAMETERS  == ab_id
            )
        {
            eqparams =(rtc_eq_param_type *)(req_buf_ptr + data_offset);
            // 6 words NumCoeff per band
            // 4 words DenCoeff per band
            // 1 word Shiftfactor
            data_len = (eqparams->mbeqnumbands * (6+4+1)*2 );
            data_len += 4; //sizeof((enable_flag) + sizeof(numBands)

            LOGE("libaudcal: rtc_transfer_q5_cal_data(set) - RTC_EQUALIZER_PARAMETERS inf_size is -> %d",data_len);
        }

        // writing the data to debug fs
        bytes_read = write(fd, req_buf_ptr + data_offset ,data_len);

        if(bytes_read <= 0){
            LOGE("libaudcal: rtc_transfer_q5_cal_data(set) - ACPH_ERR_CSD_WRITE_FAILURE\n");
            create_error_resp(ACPH_ERR_CSD_WRITE_FAILURE, req_buf_ptr,
                resp_buf_ptr, resp_buf_length);
            return;
        }
        // check if the data is set properly
        error_code = rtc_get_error_code_from_abid_intf();
        if(error_code != RTC_SUCCESS)
        {
            LOGE("libaudcal: rtc_transfer_q5_cal_data() - Error in SET -> %d\n",error_code);
            create_error_resp(error_code, req_buf_ptr,
                resp_buf_ptr, resp_buf_length);
            return;
        }

        create_suc_resp(0,req_buf_ptr,
            resp_buf_ptr, resp_buf_length);
    }
}


/*===========================================================================
  Externalized Function Definitions
  ===========================================================================*/

/**
 * FUNCTION : rtc_get_q5_cal_data
 *
 * DESCRIPTION : Get real time calibration data
 *
 * DEPENDENCIES : DSP must be active, Audcal should be initialized
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
void rtc_get_q5_cal_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    rtc_transfer_q5_cal_data(req_buf_ptr, resp_buf_ptr, resp_buf_length,
            RTC_GET_PARAM_TYPE);
}

/**
 * FUNCTION : rtc_set_q5_cal_data
 *
 * DESCRIPTION : Set real time calibration data
 *
 * DEPENDENCIES : DSP must be active, Audcal should be initialized
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
void rtc_set_q5_cal_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    rtc_transfer_q5_cal_data(req_buf_ptr, resp_buf_ptr, resp_buf_length,
            RTC_SET_PARAM_TYPE);
}

/**
 * FUNCTION : rtc_query_all_active_devices
 *
 * DESCRIPTION : Query for all the active devices
 *
 * DEPENDENCIES : DSP must be active, Audcal should be initialized
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */

void rtc_query_all_active_devices (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint8_t* nBufferPointer = (uint8_t *)(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION);
    int i;
    FILE *fd;
    uint32_t num_devices = 0;

    char *str = (char *)malloc(sizeof(char)*TEMP_BUF_LEN);
    char *temp_str = NULL;

    uint32_t abid[RTC_MAX_ACTIVE_AUD_DEVICES];
    uint32_t comp[RTC_MAX_ACTIVE_AUD_DEVICES];

    // open device control debugfs
    fd = fopen(rtcdevctrl, "r");
    if (fd == NULL)
    {
        LOGE("Could not open %s to read\n",rtcdevctrl);
        create_error_resp(ACPH_ERR_CSD_OPEN_HANDLE, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }

    // reading num_devices from debugfs
    fgets(str,TEMP_BUF_LEN,fd);
    temp_str = strtok(str,":");
    temp_str = strtok(NULL,":");
    sscanf(temp_str,"%x",&num_devices);
//LOGE("Number of devices read: %d\n",num_devices);

    if (num_devices > RTC_MAX_ACTIVE_AUD_DEVICES)
    {
        create_error_resp(ACPH_ERR_CSD_AUD_CMD_FAILURE, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }

    if(num_devices == 0)
    {
        create_suc_resp(0, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    // read the device list
    for(i=0;i<(int)num_devices;++i)
    {
        fgets(str,TEMP_BUF_LEN,fd);
        temp_str = strtok(str,":;");

        //abid
        temp_str = strtok(NULL,":;");
        sscanf(temp_str,"%x",&abid[i]);
        temp_str = strtok(NULL,":;");

        //comp
        temp_str = strtok(NULL,":;");
        sscanf(temp_str,"%x",&comp[i]);
        temp_str = strtok(NULL,":;");

        // prepare response buffer
        memcpy(nBufferPointer,
                &abid[i],
                sizeof(uint32_t));

        nBufferPointer += sizeof(uint32_t);
    }

    // close debugfs after read
    fclose(fd);
    free(str);

    create_suc_resp((num_devices)*sizeof(uint32_t),
            req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

