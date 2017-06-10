/*******************************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

//=============================================================================
/**
 * This file contains eztune process class implementation
 */
//=============================================================================

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <poll.h>

#include "eztune.h"
#include "chromatix.h"
#include "chromatix_common.h"
#include "eztune_cam_adapter.h"
#include "eztune_process.h"
#include <fastcv/fastcv.h>

#ifdef _ANDROID_
#include <log/log.h>
#endif
#include "mmcam_log_utils.h"

extern "C" eztune_item_t eztune_get_item (int i);
extern "C" void eztune_change_item(eztune_item_t *item, eztune_set_val_t *item_data, eztune_t *ezctrl);
extern "C" int eztune_get_diagnostic_item(eztune_t *ezctrl, char *output_buf,
        int offset, eztune_parms_list_t id, uint16_t table_index);
extern "C" int eztune_get_af_item(eztune_t *ezctrl, char *output_buf, int offset,
                                  eztune_parms_list_t id, uint16_t table_index);
extern "C" void dump_list_of_daemon_fd ();

namespace eztune {
const char kImgTransInfoMajorVersion = 1;
const char kImgTransInfoMinorVersion = 0;
const uint16_t kImgTransInfoHeaderSize = 6;
const char kImgTransInfoTargetType = 1; //Linux
const char kImgTransInfoCapabilities = 0x1 | 0x2 | 0x10; //Preview | JPEG | 3A Streaming
const uint16_t kPreviewInfoChunkSize = 7168;
const uint16_t kPreviewMaxChunkSize = 10240;

const uint8_t kChunkStatusSuccess = 0;
const uint8_t kChunkStatusNewSize = 2;

char kMagicStr[] = "Qualcomm Camera Debug";
const char kAEString[] = "QCAEC";
const char kAWBString[] = "QCAWB";
const char kAFString[] = "QCAF";
const char kASDString[] = "QCASD";
const char kStatsString[] = "3a stats";

uint32_t kMaxExifSize = AEC_DEBUG_DATA_SIZE + AWB_DEBUG_DATA_SIZE +
                         AF_DEBUG_DATA_SIZE + ASD_DEBUG_DATA_SIZE +
                         STATS_BUFFER_DEBUG_DATA_SIZE + 24 +
                         sizeof(kMagicStr) + sizeof(kAEString) + sizeof(kAWBString) +
                         sizeof(kAFString) + sizeof(kASDString) + sizeof(kStatsString);

static std::map<eztune_item_data_t, size_t> kEzTypeToSizeMapping;

typedef void (ProcessLayer::*ProcessFunction)(size_t payload_size, string &payload, size_t &response_size, string &response);
typedef std::map<uint16_t, ProcessFunction> ProcessCmdMap;

static ProcessCmdMap kCmdToFunctionMap;

typedef enum {
    EZTUNE_ORIGIN_TOP_LEFT = 1,
    EZTUNE_ORIGIN_BOTTOM_LEFT,
    EZTUNE_ORIGIN_INVALID
} EztuneImgOriginType;

enum class TuneCmd : uint16_t {
    TUNESERVER_GET_PREVIEW_INFO = 1,
    TUNESERVER_CHANGE_CHUNK_SIZE = 2,
    TUNESERVER_GETPREVIEW_FRAME = 3,
    TUNESERVER_GETSNAPSHOT_FRAME = 4,
    TUNESERVER_3ALIVE_START = 7,
    TUNESERVER_3ALIVE_STOP = 8,
    TUNESERVER_GET_LIST = 1014,
    TUNESERVER_GET_PARMS = 1015,
    TUNESERVER_SET_PARMS = 1016,
    TUNESERVER_MISC_CMDS = 1021,
};

typedef struct {
    uint8_t status;
    uint16_t width;
    uint16_t height;
    uint8_t format;
    uint8_t origin;
    uint32_t frame_size;
} PreviewFrameHeader;

typedef struct  {
    uint8_t major_ver;
    uint8_t minor_ver;
} PreviewInfoVersion;

typedef struct  {
    uint16_t header_size;
    uint8_t target_type;
    uint8_t capabilities;
    uint32_t chunk_size;
} PreviewInfoHeader;

//Define below macro to enable preview frame dumps in a file for debug
//#define DEBUG_FILE_DUMP
#ifdef DEBUG_FILE_DUMP
#define DUMP_FILE_NAME "/data/misc/camera/process_img.yuv"
static int32_t file_fd = -1;

//only opens file one time, if file exits will not open
void debug_open_dump_file()
{
    if (access(DUMP_FILE_NAME, F_OK) != -1) {
        file_fd = -1;
    } else {
        file_fd = open(DUMP_FILE_NAME, O_RDWR | O_CREAT, 0777);
        if (file_fd >= MAX_FD_PER_PROCESS) {
            dump_list_of_daemon_fd();
            file_fd = -1;
    }
        if (file_fd < 0)
            MMCAM_LOGI("File open error: %s",  strerror(errno));
    }
}

void debug_close_dump_file()
{
    if (file_fd >= 0)
        close(file_fd);
}

void debug_write_dump_file(char *ptr, uint32_t size)
{
    if (file_fd >= 0)
        write(file_fd, ptr,  size);
}
#else //DEBUG_FILE_DUMP
void debug_open_dump_file() {}
void debug_close_dump_file() {}
void debug_write_dump_file(char *ptr, uint32_t size) {}
#endif //DEBUG_FILE_DUMP

//Define below macro to enable eztune params dump in a file for debug
//#define DEBUG_EZTUNE_PARAMS
#ifdef DEBUG_EZTUNE_PARAMS

#define DUMP_FILE_NAME "/data/misc/camera/eztune_params.txt"
static FILE *params_file_fp = 0;

//only opens file one time, if file exits will not open
void debug_open_paramsdump_file()
{
    params_file_fp = fopen(DUMP_FILE_NAME, "w+");
    if (params_file_fp != NULL) {
        if (fileno(params_file_fp) >= MAX_FD_PER_PROCESS) {
            dump_list_of_daemon_fd();
            params_file_fp = NULL;
        }
    } else {
        MMCAM_LOGI("File open error: %s",  strerror(errno));
    }
}

void debug_close_paramsdump_file()
{
    if (params_file_fp)
        fclose(params_file_fp);
}

#define debug_write_paramsdump_file(...) (\
{ \
    if (params_file_fp) \
        fprintf(params_file_fp, __VA_ARGS__); \
} \
)

#else //DEBUG_EZTUNE_PARAMS

void debug_open_paramsdump_file() {}
void debug_close_paramsdump_file() {}
void debug_write_paramsdump_file(...) {}
#endif //DEBUG_EZTUNE_PARAMS

ProcessLayer::ProcessLayer(void *camera_adapter_handle)
{
    //Register processing function for all eazytune cmds
    kCmdToFunctionMap[(uint16_t)TuneCmd::TUNESERVER_GET_LIST] = &ProcessLayer::ProcessGetListCmd;
    kCmdToFunctionMap[(uint16_t)TuneCmd::TUNESERVER_GET_PARMS] = &ProcessLayer::ProcessGetParamCmd;
    kCmdToFunctionMap[(uint16_t)TuneCmd::TUNESERVER_SET_PARMS] = &ProcessLayer::ProcessSetParamCmd;
    kCmdToFunctionMap[(uint16_t)TuneCmd::TUNESERVER_MISC_CMDS] = &ProcessLayer::ProcessMiscCmd;

    kCmdToFunctionMap[(uint16_t)TuneCmd::TUNESERVER_GET_PREVIEW_INFO] = &ProcessLayer::ProcessImgTransInfo;
    kCmdToFunctionMap[(uint16_t)TuneCmd::TUNESERVER_CHANGE_CHUNK_SIZE] = &ProcessLayer::ProcessChangeChunkSize;
    kCmdToFunctionMap[(uint16_t)TuneCmd::TUNESERVER_GETPREVIEW_FRAME] = &ProcessLayer::ProcessGetPreviewFrame;
    kCmdToFunctionMap[(uint16_t)TuneCmd::TUNESERVER_GETSNAPSHOT_FRAME] = &ProcessLayer::ProcessGetSnapshotFrame;
    kCmdToFunctionMap[(uint16_t)TuneCmd::TUNESERVER_3ALIVE_START] = &ProcessLayer::ProcessStart3Alive;
    kCmdToFunctionMap[(uint16_t)TuneCmd::TUNESERVER_3ALIVE_STOP] = &ProcessLayer::ProcessStop3Alive;

    pthread_mutex_init(&m_cond_mutex, 0);
    pthread_mutex_init(&m_lock, 0);
    pthread_cond_init(&m_data_cond, 0);

    //Add all eztune size to actual sizes here
    kEzTypeToSizeMapping[EZT_D_FLOAT] = sizeof(float);
    kEzTypeToSizeMapping[EZT_D_INT8] = sizeof(int8_t);
    kEzTypeToSizeMapping[EZT_D_INT16] = sizeof(int16_t);
    kEzTypeToSizeMapping[EZT_D_INT32] = sizeof(int32_t);
    kEzTypeToSizeMapping[EZT_D_UINT8] = sizeof(uint8_t);
    kEzTypeToSizeMapping[EZT_D_UINT16] = sizeof(uint16_t);
    kEzTypeToSizeMapping[EZT_D_UINT32] = sizeof(uint32_t);
    kEzTypeToSizeMapping[EZT_D_DOUBLE] = sizeof(double);

    //rest of the initialization
    m_init_done = false;
    m_items_added = 0;
    m_pending_set_param_apply = false;
    m_3Alive_active = false;
    m_control_port_init = false;

    m_proc_thread_created = false;
    m_chunk_size = kPreviewInfoChunkSize;
    m_scaled_buffer = NULL;
    m_exif_buffer = NULL;
    //size of a VGA YCbCr 420 (NV12/21) buffer
    m_scaled_preview_size = kEztuneScaledSize;
    m_exif_size = 0;

    m_params_dump_fp = NULL;

    m_pending_cmds = new std::vector<eztune_set_val_t>;

    m_cam_adapter = new eztune::CamAdapter(camera_adapter_handle);
}

ProcessLayer::~ProcessLayer()
{
    pthread_cond_destroy(&m_data_cond);
    pthread_mutex_destroy(&m_cond_mutex);
    pthread_mutex_destroy(&m_lock);

    delete m_cam_adapter;

    std::vector<eztune_set_val_t> *pending_cmds = (std::vector<eztune_set_val_t> *)m_pending_cmds;
    delete pending_cmds;

    if (m_scaled_buffer) {
        free(m_scaled_buffer);
    }

    if (m_exif_buffer) {
        free(m_exif_buffer);
    }
}

size_t ProcessLayer::DumpItemBinary(void *ez)
{
    size_t num = 0;
    eztune_set_val_t *item = (eztune_set_val_t *)ez;

    if (m_params_dump_fp != NULL) {
        num = fwrite(item, sizeof(eztune_set_val_t), 1, m_params_dump_fp);
        fflush(m_params_dump_fp);
    }

    return num;
}

size_t ProcessLayer::GetItemBinary(void *ez)
{
    size_t ret_num = 0;
    eztune_set_val_t *item = (eztune_set_val_t *)ez;

    if (m_params_dump_fp != NULL) {
        ret_num = fread(item, sizeof(eztune_set_val_t), 1, m_params_dump_fp);
    }

    return ret_num;
}

int ProcessLayer::GetItemBinarySize()
{
    int size = 0;
    if (m_params_dump_fp != NULL) {
        struct stat stat_result;
        if (stat(kDumpBinaryFile, &stat_result) != -1) {
            size = stat_result.st_size;
        }
    }
    MMCAM_LOGI("%s Item Binary Size: %d",__func__, size);
    return size;
}

bool ProcessLayer::OpenItemBinary()
{
    bool rc = false;

    m_params_dump_fp = fopen(kDumpBinaryFile, "ab+");
    if (m_params_dump_fp && fileno(m_params_dump_fp) >= MAX_FD_PER_PROCESS) {
        dump_list_of_daemon_fd();
        m_params_dump_fp = NULL;
    }
    if (m_params_dump_fp != NULL) {//file either created or opened
        rc = true;
        if (GetItemBinarySize() != 0) {//non-zero valid file exists
                fseek(m_params_dump_fp, 0, SEEK_SET);
            } else {
                rc = false;
            }
        }

    return rc;
}

void ProcessLayer::CloseItemBinary()
{
    if (m_params_dump_fp != NULL) {
        fclose(m_params_dump_fp);
    }
    m_params_dump_fp = NULL;
}

void ProcessLayer::ProcessAndGenerateResponse(uint16_t cmd, size_t payload_size, string &payload, size_t &response_size, string &response)
{
    ProcessCmdMap::const_iterator function = kCmdToFunctionMap.find(cmd);

    MMCAM_LOGV("Cmd received by eztune engine: %d", cmd);

    if (function == kCmdToFunctionMap.end()) {
        response_size = 0;
        MMCAM_LOGE("Cmd not recognized by eztune engine: %d", cmd);
        goto error;
    }
    return (this->*(function->second))(payload_size, payload, response_size, response);

error:
    return;
}

void ProcessLayer::ResumeTuningSession()
{
    eztune_set_val_t item;
    size_t item_size;

    std::vector<eztune_set_val_t> *pending_cmds = (std::vector<eztune_set_val_t> *)m_pending_cmds;

    Init(EZTUNE_SERVER_CONTROL, -1);

    pending_cmds->clear();
    //If there is an intermediate file to apply commands from,
    //all commands will be applied and no record will be deleted.
    //Tuning user is responsible for deleting this file to start fresh
    while ((item_size = GetItemBinary(&item)) == 1) {
        pending_cmds->push_back(item);
    }
    if (pending_cmds->empty() == false) {
        eztune_t ezctrl;
        m_cam_adapter->GetTuningTables(&ezctrl);//don't care about return value
        ApplyItems(&ezctrl);
    }

end:
    return;
}

bool ProcessLayer::Init(eztune_server_t mode, int pipe_fd)
{
    m_mode = mode;

    if (m_mode == EZTUNE_SERVER_CONTROL && m_control_port_init == false) {
        //Get tuning tables first to ensure the stream has started
        eztune_t ezctrl;

        bool read_dump = OpenItemBinary();
        if (read_dump == false) {
            //Dump file is not present or could not be opened
            //Not a serious failure, go ahead with normal initialization
            MMCAM_LOGW("No dump file present");
        }
        m_cam_adapter->GetTuningTables(&ezctrl);

        m_cam_adapter->FetchChromatixData();
        m_cam_adapter->FetchAFTuneData();

        tuning_set_vfe(NULL, VFE_MODULE_ALL, SET_STATUS, 1);
        tuning_set_pp(NULL, PP_MODULE_ALL, SET_STATUS, 1);
        tuning_set_3a(NULL, EZ_STATUS, 1);

        std::vector<eztune_set_val_t> *pending_cmds = (std::vector<eztune_set_val_t> *)m_pending_cmds;
        pending_cmds->clear();

        debug_open_paramsdump_file();

        m_control_port_init = true;
        MMCAM_LOGI("%s:Control Mode Init", __func__ );
    } else {
        //store the eztune thread pipe_fd here
        m_intf_pipe_fd = pipe_fd;

        //create pipe for communication with adaptor
        //only in case of image port
        int val = pipe(m_notify_pipe_fds);
        if (m_notify_pipe_fds[0] >= MAX_FD_PER_PROCESS) {
            dump_list_of_daemon_fd();
            m_notify_pipe_fds[0] = -1;
            val = -1;
        }
        MMCAM_ASSERT(val == 0, "Pipe creation failed: %s", strerror(errno));

        //create process worker thread
        val = pthread_create(&m_proc_thread, NULL, ProcessLayer::ProcThreadWrapper, static_cast<void *>(this));
        MMCAM_ASSERT(val == 0, "Process thread creation failed: %s", strerror(errno));

        //created the process thread in the above step
        //wait for the thread to actually get dispatched and start running
        pthread_mutex_lock(&m_cond_mutex);
        while(m_proc_thread_created == FALSE) {
          pthread_cond_wait(&m_data_cond, &m_cond_mutex);
        }
        pthread_mutex_unlock(&m_cond_mutex);

        MMCAM_LOGI("%s:Image Transmission Mode Init", __func__ );
    }

    m_init_done = true;
    return true;
}

bool ProcessLayer::DeInit()
{
    // This typically happens when the client closes the connection
    // first and then the camera app is closed. Its safe to ignore this
    //Deinit request
    if(m_init_done == false) {
        MMCAM_LOGE("Process layer not initialized. just return");
        return true;
    }
    if (m_mode == EZTUNE_SERVER_CONTROL) {

        debug_close_paramsdump_file();
        CloseItemBinary();

        tuning_set_vfe(NULL, VFE_MODULE_ALL, SET_STATUS, 0);
        tuning_set_pp(NULL, PP_MODULE_ALL, SET_STATUS, 0);
        tuning_set_3a(NULL, EZ_STATUS, 0);

        m_control_port_init = false;
        MMCAM_LOGV("%s:Server Mode DeInit", __func__ );
    } else {
        MMCAM_LOGV("%s:Preview Mode DeInit", __func__ );
    }

    m_init_done = false;
    return true;
}

void ProcessLayer::DataWaitNotify()
{
    pthread_mutex_lock(&m_cond_mutex);
    pthread_cond_signal(&m_data_cond);
    pthread_mutex_unlock(&m_cond_mutex);
}

bool ProcessLayer::DataWait()
{
    struct timespec   ts;
    struct timeval    tp;
    gettimeofday(&tp, NULL);
    ts.tv_sec  = tp.tv_sec;
    ts.tv_nsec = tp.tv_usec * 1000 + WAIT_TIME_MILLISECONDS * 1000000;

    pthread_mutex_lock(&m_cond_mutex);
    int rc = pthread_cond_timedwait(&m_data_cond, &m_cond_mutex, &ts);
    pthread_mutex_unlock(&m_cond_mutex);

    if (rc == ETIMEDOUT)
        return false;
    else
        return true;
}

void ProcessLayer::SendEventToInterface(uint32_t event_id, void *data, uint32_t size)
{
    InterfaceThreadMessage intf_thrd_msg;

    intf_thrd_msg.type = event_id;
    intf_thrd_msg.payload_ptr = data;
    intf_thrd_msg.payload_size = size;
    int val = write(m_intf_pipe_fd, &intf_thrd_msg, sizeof(intf_thrd_msg));
    MMCAM_ASSERT(val != -1, "Write to pipe failed, write returned: %s", strerror(errno));
}

void ProcessLayer::StopProcessThread()
{
    ProcessThreadMessage process_thread_message;
    int val = 0;

    // if the C-L or any Client app never connected, process thread
    //will not be created, and it will cause a crash if we post the message
    if (m_proc_thread_created == false) {
        goto end;
    }

    process_thread_message.event_id = EZTUNE_STOP_NOTIFY;
    process_thread_message.data = NULL;
    process_thread_message.size = 0;
    val = write(m_notify_pipe_fds[1], &process_thread_message, sizeof(process_thread_message));
    MMCAM_ASSERT(val != -1, "Write to pipe failed, write returned: %s", strerror(errno));

    val = pthread_join(m_proc_thread, NULL);
    MMCAM_ASSERT(val == 0, "pthread_join returned error: %s", strerror(errno));

    close(m_notify_pipe_fds[0]);
    close(m_notify_pipe_fds[1]);

end:
    return;
}


bool ProcessLayer::Get3ALiveStatus()
{
    bool livestatus = false;
    pthread_mutex_lock(&m_lock);
    livestatus = m_3Alive_active;
    pthread_mutex_unlock(&m_lock);
    return livestatus;
}

void ProcessLayer::NotifyBack()
{
    if (Get3ALiveStatus() == true) {
        //get ready for the next preview buffer
        m_cam_adapter->TriggerPreviewBufferCopy(true);
        //get ready for next metadata
        m_cam_adapter->SetMetadataPending(true);
    }
}

void ProcessLayer::ScaleDownImage(void *data)
{
    uint8_t *src_buff = (uint8_t *)data;
    uint32_t width = 0, height = 0;
    int32_t ret = 0;

    m_cam_adapter->GetPreviewDimension(width, height);
    MMCAM_ASSERT(width > 0 && height > 0, "Invalid Preview dimensions");

    if (m_scaled_buffer == NULL) {
        //fastcv needs 128 bit aligned buffers
        ret = posix_memalign((void **)&m_scaled_buffer, 128, m_scaled_preview_size);
        MMCAM_ASSERT(ret == 0 && m_scaled_buffer != NULL, "posix_memalign returned failure");
    }

    if (width <= kEztuneScaledWidth && height <= kEztuneScaledHeight) {
        MMCAM_LOGV("Scaled dimensions >= preview, using preview dimension directly for 3A");
        m_scaled_preview_size = (width * height *3) >> 1;
        //we can avoid this memcpy and directly use the preview buffer
        memcpy(m_scaled_buffer, data, m_scaled_preview_size);
        return;
    }

    uint8_t *src_buff_cb = NULL, *src_buff_cr = NULL;
    uint8_t *dst_buff_cb = NULL, *dst_buff_cr = NULL;

    uint8_t *src_buff_cbcr = src_buff + (width * height);
    uint8_t *dst_buff_cbcr = m_scaled_buffer + kEztuneScaledLumaSize;

    //allocate cb & cr src buffers (pre-scale)
    //fastcv needs 128 bit aligned buffers
    ret = posix_memalign((void **)&src_buff_cb, 128, (width * height)>>2);
    MMCAM_ASSERT(ret == 0 && src_buff_cb != NULL, "posix_memalign returned failure");
    ret = posix_memalign((void **)&src_buff_cr, 128, (width * height)>>2);
    MMCAM_ASSERT(ret == 0 && src_buff_cr != NULL, "posix_memalign returned failure");

    //allocate cb & cr dst buffers (post-scale)
    //fastcv needs 128 bit aligned buffers
    ret = posix_memalign((void **)&dst_buff_cb, 128, (kEztuneScaledLumaSize)>>2);
    MMCAM_ASSERT(ret == 0 && dst_buff_cb != NULL, "posix_memalign returned failure");
    ret = posix_memalign((void **)&dst_buff_cr, 128, (kEztuneScaledLumaSize)>>2);
    MMCAM_ASSERT(ret == 0 && dst_buff_cr != NULL, "posix_memalign returned failure");

    //1) De-interleave the CbCr planes, and create 3 separate color planes (Y, Cb, Cr)
    fcvDeinterleaveu8(src_buff_cbcr, width>>1, height>>1, 0, src_buff_cb, 0, src_buff_cr, 0);

    //2) Scale down each plane separately
    fcvScaleDownu8_v2(src_buff, width, height, 0, m_scaled_buffer, kEztuneScaledWidth, kEztuneScaledHeight, 0);
    fcvScaleDownu8_v2(src_buff_cb, width>>1, height>>1, 0, dst_buff_cb, kEztuneScaledWidth>>1, kEztuneScaledHeight>>1, 0);
    fcvScaleDownu8_v2(src_buff_cr, width>>1, height>>1, 0, dst_buff_cr, kEztuneScaledWidth>>1, kEztuneScaledHeight>>1, 0);

    //3) Interleave the CbCr planes back again to create a final NV12/21 buffer
    fcvInterleaveu8(dst_buff_cb, dst_buff_cr, kEztuneScaledWidth>>1, kEztuneScaledHeight>>1, 0, 0, dst_buff_cbcr, 0);

    //free all intermediate buffers
    free(src_buff_cb);
    free(src_buff_cr);
    free(dst_buff_cb);
    free(dst_buff_cr);

    MMCAM_LOGV("Scaled down image from (%zu/%zu) to (%zu/%zu)", width, height, kEztuneScaledWidth, kEztuneScaledHeight);

    return;
}

void ProcessLayer::GetEXIFHeader()
{
    int32_t ret = 0;

    // Major Revision|Minor Revision|Patch Revision
    // 2.0.4 = 2004
    uint32_t version = (2<<24) | (1<<2);

    cam_ae_exif_debug_t *data_ptr_ae = NULL;
    cam_awb_exif_debug_t *data_ptr_awb = NULL;
    cam_af_exif_debug_t *data_ptr_af = NULL;
    cam_asd_exif_debug_t *data_ptr_asd = NULL;
    cam_stats_buffer_exif_debug_t *data_ptr_stats = NULL;

    m_cam_adapter->MetadataLock();
    cam_metadata_info_t *pMetaData = m_cam_adapter->GetMetadata();
    MMCAM_ASSERT(pMetaData != NULL, "metadata not available");

    m_exif_size = 0;

    //measure the overall size of the exif buffer reqd
    if (pMetaData->is_ae_exif_debug_valid) {
        m_exif_size += sizeof(kAEString);
        m_exif_size += 4;
        data_ptr_ae = &pMetaData->ae_exif_debug_params;
        m_exif_size += data_ptr_ae->aec_debug_data_size;
    }

    if (pMetaData->is_awb_exif_debug_valid) {
        m_exif_size += sizeof(kAWBString);
        m_exif_size += 4;
        data_ptr_awb = &pMetaData->awb_exif_debug_params;
        m_exif_size += data_ptr_awb->awb_debug_data_size;
    }

    if (pMetaData->is_af_exif_debug_valid) {
        m_exif_size += sizeof(kAFString);
        m_exif_size += 4;
        data_ptr_af = &pMetaData->af_exif_debug_params;
        m_exif_size += data_ptr_af->af_debug_data_size;
    }

    if (pMetaData->is_asd_exif_debug_valid) {
        m_exif_size += sizeof(kASDString);
        m_exif_size += 4;
        data_ptr_asd = &pMetaData->asd_exif_debug_params;
        m_exif_size += data_ptr_asd->asd_debug_data_size;
    }

    if (pMetaData->is_stats_buffer_exif_debug_valid) {
        m_exif_size += sizeof(kStatsString);
        m_exif_size += 4;
        data_ptr_stats = &pMetaData->stats_buffer_exif_debug_params;
        m_exif_size += (data_ptr_stats->bg_stats_buffer_size +
                        data_ptr_stats->bhist_stats_buffer_size);
    }

    if (m_exif_size) {
        m_exif_size += sizeof(kMagicStr);
        m_exif_size += sizeof(version);
    }else {
        m_cam_adapter->MetadataUnlock();
        return;
    }

    //allocate the exif header buffer
    if (m_exif_buffer == NULL) {
        ret = posix_memalign((void **)&m_exif_buffer, 32, kMaxExifSize);
        MMCAM_ASSERT(ret == 0 && m_exif_buffer != NULL, "posix_memalign returned failure");
    }

    uint8_t *tmp_exif_buff = m_exif_buffer;

    //write "QTI Camera Debug"
    memcpy(tmp_exif_buff, kMagicStr, sizeof(kMagicStr));
    tmp_exif_buff += sizeof(kMagicStr);

    //write version number
    memcpy(tmp_exif_buff, &version, sizeof(version));
    tmp_exif_buff += sizeof(version);

    //write awb data
    if (pMetaData->is_awb_exif_debug_valid) {
        //write awb identifier
        memcpy(tmp_exif_buff, kAWBString, sizeof(kAWBString));
        tmp_exif_buff += sizeof(kAWBString);
        //write awb data size
        memcpy(tmp_exif_buff, &data_ptr_awb->awb_debug_data_size, sizeof(data_ptr_awb->awb_debug_data_size));
        tmp_exif_buff += sizeof(data_ptr_awb->awb_debug_data_size);
        //write awb payload
        memcpy(tmp_exif_buff, &data_ptr_awb->awb_private_debug_data[0], data_ptr_awb->awb_debug_data_size);
        tmp_exif_buff += data_ptr_awb->awb_debug_data_size;
        MMCAM_LOGV("Written AWB EXIF header");
    }

    //write aec data
    if (pMetaData->is_ae_exif_debug_valid) {
        //write aec identifier
        memcpy(tmp_exif_buff, kAEString, sizeof(kAEString));
        tmp_exif_buff += sizeof(kAEString);
        //write aec data size
        memcpy(tmp_exif_buff, &data_ptr_ae->aec_debug_data_size, sizeof(data_ptr_ae->aec_debug_data_size));
        tmp_exif_buff += sizeof(data_ptr_ae->aec_debug_data_size);
        //write aec payload
        memcpy(tmp_exif_buff, &data_ptr_ae->aec_private_debug_data[0], data_ptr_ae->aec_debug_data_size);
        tmp_exif_buff += data_ptr_ae->aec_debug_data_size;
        MMCAM_LOGV("Written AEC EXIF header");
    }

    //write af data
    if (pMetaData->is_af_exif_debug_valid) {
        //write af identifier
        memcpy(tmp_exif_buff, kAFString, sizeof(kAFString));
        tmp_exif_buff += sizeof(kAFString);
        //write af data size
        memcpy(tmp_exif_buff, &data_ptr_af->af_debug_data_size, sizeof(data_ptr_af->af_debug_data_size));
        tmp_exif_buff += sizeof(data_ptr_af->af_debug_data_size);
        //write af payload
        memcpy(tmp_exif_buff, &data_ptr_af->af_private_debug_data[0], data_ptr_af->af_debug_data_size);
        tmp_exif_buff += data_ptr_af->af_debug_data_size;
        MMCAM_LOGV("Written AF EXIF header");
    }

    //write asd data
    if (pMetaData->is_asd_exif_debug_valid) {
        //write asd identifier
        memcpy(tmp_exif_buff, kASDString, sizeof(kASDString));
        tmp_exif_buff += sizeof(kASDString);
        //write asd data size
        memcpy(tmp_exif_buff, &data_ptr_asd->asd_debug_data_size, sizeof(data_ptr_asd->asd_debug_data_size));
        tmp_exif_buff += sizeof(data_ptr_asd->asd_debug_data_size);
        //write asd payload
        memcpy(tmp_exif_buff, &data_ptr_asd->asd_private_debug_data[0], data_ptr_asd->asd_debug_data_size);
        tmp_exif_buff += data_ptr_asd->asd_debug_data_size;
        MMCAM_LOGV("Written ASD EXIF header");
    }

    //write stats data
    if (pMetaData->is_stats_buffer_exif_debug_valid) {
        //write stats identifier
        memcpy(tmp_exif_buff, kStatsString, sizeof(kStatsString));
        tmp_exif_buff += sizeof(kStatsString);
        //write asd data size
        int32_t data_size = (data_ptr_stats->bg_stats_buffer_size +
                                data_ptr_stats->bhist_stats_buffer_size);
        memcpy(tmp_exif_buff, &data_size, sizeof(data_size));
        tmp_exif_buff += sizeof(data_size);
        //write asd payload
        memcpy(tmp_exif_buff, &data_ptr_stats->stats_buffer_private_debug_data[0], data_size);
        tmp_exif_buff += data_size;//this is not really needed
        MMCAM_LOGV("Written Stats EXIF header");
    }

    m_cam_adapter->MetadataUnlock();

    MMCAM_LOGV("Processed EXIF header");

    return;
}

void ProcessLayer::Process3ALiveData(void *data, string &response)
{
    uint16_t marker = 0xFE7E;//start of 3ALive packet
    int8_t status = 0;
    struct timeval curr_time;
    unsigned long mtime, seconds, useconds;

    //start with clear string
    response.clear();

    if(false == Get3ALiveStatus()) {
        MMCAM_LOGE("%s 3A live stopped, Exit", __func__);
        return;
    }

    //pack the marker
    response.append((char *)&marker, sizeof(marker));
    //pack the status
    response.append((char *)&status, sizeof(status));

    //pack the relative timestamp
    gettimeofday(&curr_time, NULL);
    mtime =
     ((curr_time.tv_sec * 1000) + (curr_time.tv_usec / 1000)) -
     ((m_prev_time.tv_sec * 1000) + (m_prev_time.tv_usec / 1000));

    //pack the relative timestamp
    response.append((char *)&mtime, sizeof(mtime));
    MMCAM_LOGV("Relative timestamp: (%ld)", mtime);

    //get EXIF header
    GetEXIFHeader();
    if (m_exif_size == 0) {
        response.clear();
        MMCAM_LOGE("no 3A header found, nothing to send to C-L");
        return;
    }

    //downscale image to VGA
    ScaleDownImage(data);

    MMCAM_LOGV("3A EXIF header size: (%zu), Scaled down image size: (%zu)", m_exif_size, m_scaled_preview_size);

    //pack downscaled image size
    response.append((char *)&m_scaled_preview_size, sizeof(m_scaled_preview_size));
    //pack 3A EXIF header size
    response.append((char *)&m_exif_size, sizeof(m_exif_size));
    //pack downscaled image
    response.append((char *)m_scaled_buffer, m_scaled_preview_size);
    //pack 3A EXIF header
    response.append((char *)m_exif_buffer, m_exif_size);

    MMCAM_LOGV("Appended EXIF header and Scaled down image");

    return;
}

void* ProcessLayer::ProcThreadWrapper(void *ptr)
{
    ProcessLayer *obj = static_cast<ProcessLayer *>(ptr);

    obj->ProcessThread();

    MMCAM_LOGV("exiting proc thread");
    return  NULL;
}

void ProcessLayer::ProcessThread()
{
    //standard way to notify caller process that thread is
    //created and it's ok to invoke it's functionality
    pthread_mutex_lock(&m_cond_mutex);
    m_proc_thread_created = TRUE;
    pthread_cond_signal(&m_data_cond);
    pthread_mutex_unlock(&m_cond_mutex);

    boolean exit_thread = false;
    int32_t num_fds = 1, ready = 0, read_bytes = 0;
    ProcessThreadMessage process_thread_message;
    struct pollfd pollfds;

    while (exit_thread == false) {
        pollfds.fd = m_notify_pipe_fds[0];
        pollfds.events = POLLIN|POLLPRI;
        ready = poll(&pollfds, (nfds_t)num_fds, -1);
        if (ready > 0) {
            if (pollfds.revents & (POLLIN|POLLPRI)) {
              read_bytes = read(pollfds.fd, &process_thread_message,
                sizeof(ProcessThreadMessage));
              if ((read_bytes < 0) ||
                  (read_bytes != sizeof(ProcessThreadMessage))) {
                MMCAM_LOGE("failed: read_bytes %d", read_bytes);
                continue;
              }

              switch (process_thread_message.event_id) {
                case EZTUNE_PREVIEW_NOTIFY:
                case EZTUNE_SNAPSHOT_NOTIFY:
                  MMCAM_LOGV("%s, Event-id = %d", __func__, process_thread_message.event_id);
                  m_response.clear();
                  PrepareResponse(m_response, process_thread_message.event_id);
                  m_response.append((char *)process_thread_message.data, process_thread_message.size);
                  //send the async event to the eztune interface thread
                  SendEventToInterface(EZTUNE_INTF_ASYNC_RESP, (void *)&m_response, m_response.size());
                  break;

                case EZTUNE_3ALIVE_NOTIFY:
                  //process the 3A live data -
                  Process3ALiveData(process_thread_message.data, m_response);
                  //send the async event to the eztune interface thread
                  if (m_response.size()) { //there was a valid EXIF header, so packed it with preview and send to host
                    SendEventToInterface(EZTUNE_INTF_ASYNC_RESP, (void *)&m_response, m_response.size());
                  }else { //there was no valid EXIF header, nothing to send and just continue for the next set
                      //get ready for the next preview buffer
                      m_cam_adapter->TriggerPreviewBufferCopy(true);
                      //get ready for next metadata
                      m_cam_adapter->SetMetadataPending(true);
                  }
                  break;

                case EZTUNE_STOP_NOTIFY:
                  exit_thread = true;
                  break;

                default:
                  MMCAM_LOGE("invalid event type %d", process_thread_message.event_id);
                  break;
              }
            }
        } else if (ready <= 0) {
            MMCAM_LOGE("failed: exit thread");
            break;
        }
    }

    pthread_mutex_lock(&m_cond_mutex);
    m_proc_thread_created = FALSE;
    pthread_mutex_unlock(&m_cond_mutex);

    return;
}

void ProcessLayer::ProcessImgTransInfo(size_t payload_size, string &payload, size_t &response_size, string &response)
{
    response.clear();

    PreviewInfoVersion version = {
        .major_ver = kImgTransInfoMajorVersion,
        .minor_ver = kImgTransInfoMinorVersion
    };

    response.append((char *)&version, sizeof(version));

    PreviewInfoHeader header = {
        .header_size = kImgTransInfoHeaderSize,
        .target_type = kImgTransInfoTargetType,
        .capabilities = kImgTransInfoCapabilities,
        .chunk_size = kPreviewInfoChunkSize
    };

    response.append((char *)&header, sizeof(header));

    response_size = response.size();

    MMCAM_LOGV("%s: Size(%zu)", __func__, response_size);

    return;
}

void ProcessLayer::ProcessChangeChunkSize(size_t payload_size, string &payload, size_t &response_size, string &response)
{
    uint32_t new_chunk_size = *(uint32_t *)payload.data();
    uint8_t status;
    uint32_t chunk_size;

    if (new_chunk_size <= kPreviewMaxChunkSize) {
        status = kChunkStatusSuccess;
        chunk_size = new_chunk_size;
    } else {
        status = kChunkStatusNewSize;
        chunk_size = kPreviewMaxChunkSize;
    }
    m_chunk_size = chunk_size;

    response.clear();
    response.append((char *)&status, sizeof(status));
    response.append((char *)&chunk_size, sizeof(chunk_size));

    response_size = response.size();

    MMCAM_LOGV("%s: Size(%zu)", __func__, response_size);

    return;
}

void ProcessLayer::ProcessGetPreviewFrame(size_t payload_size, string &payload, size_t &response_size, string &response)
{
    int8_t status = 0;
    //start with clear string
    response.clear();

    if (Get3ALiveStatus() == true) {
        //set status as failure
        status = 1;
        response.append((char *)&status, sizeof(status));
        MMCAM_LOGV("3A Live streaming enabled");
    }
    else {
        //set preview buffer pending
        m_cam_adapter->TriggerPreviewBufferCopy(true);
    }

    response_size = response.size();
    MMCAM_LOGV("%s: Size(%zu)", __func__, response_size);

    return;
}

void ProcessLayer::ProcessGetSnapshotFrame(size_t payload_size, string &payload, size_t &response_size, string &response)
{
    int8_t status = 0;
    //start with clear string
    response.clear();

    if (Get3ALiveStatus() == true) {
        //set status as failure
        status = 1;
        response.append((char *)&status, sizeof(status));
        MMCAM_LOGI("Eztune Snapshot can not be triggered while 3A Live streaming enabled");
    }
    else {
        bool ret = m_cam_adapter->TriggerSnapshot();
        if (ret == false) {
            //Could not trigger snapshot
            //set status as failure
            status = 1;
        response.append((char *)&status, sizeof(status));
        MMCAM_LOGI("Snapshot could not be triggered");
        }
    }
    response_size = response.size();
    MMCAM_LOGV("%s: Size(%zu)", __func__, response_size);

    return;
}

void ProcessLayer::ProcessStart3Alive(size_t payload_size, string &payload, size_t &response_size, string &response)
{
    int8_t status = 0;
    //start with clear string
    response.clear();

    MMCAM_LOGV("Received 3A Live Start");

    pthread_mutex_lock(&m_lock);
    m_3Alive_active = true;
    pthread_mutex_unlock(&m_lock);

    //set status as success
    status = 0;
    response.append((char *)&status, sizeof(status));

    uint8_t fps;
    uint16_t width;
    uint16_t height;
    uint8_t format;
    uint8_t origin = EZTUNE_ORIGIN_BOTTOM_LEFT;

    m_cam_adapter->Get3AliveInfo(fps,
                                 width,
                                 height,
                                 format);

    //add 3Alive Start response
    response.append((char *)&fps, sizeof(fps));
    response.append((char *)&width, sizeof(width));
    response.append((char *)&height, sizeof(height));
    response.append((char *)&format, sizeof(format));
    response.append((char *)&origin, sizeof(origin));

    //we need to keep checking for new metadata
    m_cam_adapter->SetMetadataPending(true);
    //set preview buffer pending
    m_cam_adapter->TriggerPreviewBufferCopy(true);
    //set the expected scaled buffer size
    m_scaled_preview_size = kEztuneScaledSize;

    gettimeofday(&m_prev_time, NULL);

    response_size = response.size();
    MMCAM_LOGV("%s: Size(%zu)", __func__, response_size);

    return;
}

void ProcessLayer::ProcessStop3Alive(size_t payload_size, string &payload, size_t &response_size, string &response)
{
    uint16_t marker = 0xFE7E;//start of 3ALive packet
    int8_t status = 1;
    uint32_t size = 0;
    uint32_t mtime = 0;

    MMCAM_LOGV("Received 3A Live Stop");

    pthread_mutex_lock(&m_lock);
    m_3Alive_active = false;
    pthread_mutex_unlock(&m_lock);

    m_scaled_preview_size = 0;
    m_exif_size = 0;

    //start with clear string
    response.clear();

    //pack the marker
    response.append((char *)&marker, sizeof(marker));
    //pack the status = 1 = END
    response.append((char *)&status, sizeof(status));
    //no valid timestamp for stop response
    response.append((char *)&mtime, sizeof(mtime));
    //zero image data size
    response.append((char *)&m_scaled_preview_size, sizeof(m_scaled_preview_size));
    //zero 3A packet size
    response.append((char *)&m_exif_size, sizeof(m_exif_size));

    //we no longer need metadata
    m_cam_adapter->SetMetadataPending(false);
    //we no longer need preview
    m_cam_adapter->TriggerPreviewBufferCopy(false);

    response_size = response.size();
    MMCAM_LOGV("%s: Size(%zu)", __func__, response_size);

    return;
}

void ProcessLayer::ProcessGetListCmd(size_t payload_size, string &payload, size_t &response_size, string &response)
{
    MMCAM_ASSERT_PRE(m_init_done == true, "Process Layer is not yet initialized");

    //start with clear string
    response.clear();

    //set default first 3 bytes for status and number of records.
    //This gets overwritten to actual number of records outside the for loop
    response.append("\x00", 1);
    response.append((const char *)&m_items_added, 2);

    MMCAM_LOGV("Number of items to add: %d, Max buffer size: %zu", (EZT_PARMS_MAX - m_items_added), response_size);

    int i;
    for (i = m_items_added; i < EZT_PARMS_MAX; i++) {

        eztune_item_t item = eztune_get_item(i);

        //check if enough space to insert current entry
        if (response.size() >= (response_size - 8 - strlen(item.name) - 1))
            break;

        //insert index
        response.append((const char *)&i, 2);

        //insert offset
        if (item.offset)
            response.append((const char *)&item.size, 2);
        else
            response.append("\x01\x00", 2);

        //insert flag
        if (item.reg_flag == EZT_WRITE_FLAG) {
            response.append("\x00\x00\x00\x00", 4);
        } else if (item.reg_flag == EZT_READ_FLAG || item.reg_flag == EZT_3A_FLAG) {
            response.append("\x01\x00\x00\x00", 4);
        } else if (item.reg_flag == EZT_CHROMATIX_FLAG) {
            response.append("\x40\x00\x00\x00", 4);
        } else if (item.reg_flag == (EZT_CHROMATIX_FLAG | EZT_READ_FLAG)) {
            response.append("\x41\x00\x00\x00", 4);
        } else if (item.reg_flag == EZT_AUTOFOCUS_FLAG) {
            response.append("\x00\x04\x00\x00", 4);
        } else if (item.reg_flag == (EZT_AUTOFOCUS_FLAG | EZT_READ_FLAG)) {
            response.append("\x01\x04\x00\x00", 4);
        }

        //insert name
        response.append(item.name, strlen(item.name) + 1);
    }

    //replace the character 0 if not all params are added
    if (i != EZT_PARMS_MAX)
        response.replace(0, 1, "\x01");

    //replace character 1 and 2 to to indicate number of items added
    uint16_t items_added = i - m_items_added;
    response.replace(1, 2, (const char *)&items_added, 2);

    //update number of items added
    if (i == EZT_PARMS_MAX) {
        m_items_added = 0;
        MMCAM_LOGI("All getlist Items added");
    } else {
        m_items_added = i;
    }

    MMCAM_LOGV("Number of items added: %d, Response buffer size: %zu", items_added, response.size());

    //fill remaining data with 0 bytes
    MMCAM_ASSERT(response_size >= response.size(), "response string overflow");
    response.append(response_size - response.size(), 0);

    return;
}

void ProcessLayer::ProcessGetParamCmd(size_t payload_size, string &payload, size_t &response_size, string &response)
{
    MMCAM_ASSERT_PRE(m_init_done == true, "Process Layer is not yet initialized");

    const char *input = payload.data();

    uint16_t num_items =  *(uint16_t *)input;
    input = input + sizeof(uint16_t);

    response.clear();

    if (m_pending_set_param_apply == true)
        response.append("\x01", 1);
    else
        response.append("\x00", 1);

    //insert number of items
    response.append((const char *)&num_items, sizeof(uint16_t));

    eztune_t ezctrl;

    MMCAM_LOGV("Number of params to get: %d, Max buffer size: %zu", num_items, response_size);

    //get new tuning tables only if non zero items needs to be fetched
    //PS: eztune host tool periodically sends get param with no items
    if (num_items) {
        if (m_cam_adapter->GetTuningTables(&ezctrl) == false) {
            MMCAM_LOGW("Get Tuning tables failed perhaps due to timeout");
            //reset number of items added as zero
            uint16_t num_items_added = 0;
            response.replace(1, 2, (const char *)&num_items_added, 2);
            goto end;
        }
    }

    //debug
    debug_write_paramsdump_file("Adding GetParams. Num params: %d\n", num_items);

    int i;
    for (i = 0; i < num_items; ++i) {
        int rc = -1;

        uint16_t item_num = *(uint16_t *)input;
        input = input + sizeof(uint16_t);

        uint16_t table_index = *(uint16_t *)input;
        input = input + sizeof(uint16_t);

        //check if enough space to insert current entry
        if (response.size() >= (response_size - 4 - 64)) {
            break;
        }

        //insert item number and table index
        response.append((const char *)&item_num, sizeof(uint16_t));
        response.append((const char *)&table_index, sizeof(uint16_t));

        //fetch the required item and then extract info
        eztune_item_t item = eztune_get_item(item_num);

        void *chromatixptr;

        if (item.id < EZT_PARMS_SNAP_MISC_CHROMATIXVERSION)
            chromatixptr = (char *)ezctrl.chromatixptr;
        else if ((item.id >= EZT_PARMS_SNAP_MISC_CHROMATIXVERSION) &&
                 (item.id < EZT_PARMS_MISC_COMMONCHROMATIXVERSION))
            chromatixptr = (char *)ezctrl.snapchromatixptr;
        else
            chromatixptr = (char *)ezctrl.common_chromatixptr;

        char *ptr = (char *)chromatixptr + item.offset;

        //offset the pointer based on data type
        if (item.id == EZT_PARMS_AEC_TABLEEXPOSUREENTRIESGAIN ||
                item.id == EZT_PARMS_SNAP_AEC_TABLEEXPOSUREENTRIESGAIN) {

            ptr += table_index * sizeof(exposure_entry_type) + offsetof(exposure_entry_type, gain);

        } else if (item.id == EZT_PARMS_AEC_TABLEEXPOSUREENTRIESLINECT ||
                   item.id == EZT_PARMS_SNAP_AEC_TABLEEXPOSUREENTRIESLINECT) {

            ptr += table_index * sizeof(exposure_entry_type) + offsetof(exposure_entry_type, line_count);

        } else {

            auto elem = kEzTypeToSizeMapping.find(item.data);
            size_t size;

            if (elem == kEzTypeToSizeMapping.end()) {
                size = 0;
            } else {
                size = elem->second;
            }

            ptr += table_index * size;
        }

        char temp_string[EZTUNE_FORMAT_MAX];

        if (item.data == EZT_D_INVALID) {
            /* diagnostics */
            if (item.id >= EZT_PARMS_3A &&
                    item.id <= EZT_PARMS_MISC_SENSORQTRHEIGHT) {
                rc = eztune_get_diagnostic_item(&ezctrl, temp_string, 0, item.id,
                                                table_index);

            } else if (item.id >= EZT_PARMS_AFTUNE_RELOADPARAMS_ENABLE &&
                       item.id <= EZT_PARMS_AFTUNE_TUNING_TEST_MOVEFOCUS_STEPS) {
                rc = eztune_get_af_item(&ezctrl, temp_string, 0, item.id,
                                        table_index);
            } else {
                MMCAM_LOGW("Invalid item.id");
            }

        } else {

            switch (item.data) {
            case EZT_D_FLOAT:
                rc = snprintf(temp_string, EZTUNE_FORMAT_MAX,
                              "%f", *((float *)ptr));
                break;
            case EZT_D_INT8:
                rc = snprintf(temp_string, EZTUNE_FORMAT_MAX,
                              "%d", *((int8_t *)ptr));
                break;
            case EZT_D_INT16:
                rc = snprintf(temp_string, EZTUNE_FORMAT_MAX,
                              "%d", *((int16_t *)ptr));
                break;
            case EZT_D_INT32:
                rc = snprintf(temp_string, EZTUNE_FORMAT_MAX,
                              "%d", *((int32_t *)ptr));
                break;
            case EZT_D_UINT8:
                rc = snprintf(temp_string, EZTUNE_FORMAT_MAX,
                              "%d", *((uint8_t *)ptr));
                break;
            case EZT_D_UINT16:
                rc = snprintf(temp_string, EZTUNE_FORMAT_MAX,
                              "%d", *((uint16_t *)ptr));
                break;
            case EZT_D_UINT32:
                rc = snprintf(temp_string, EZTUNE_FORMAT_MAX,
                              "%d", *((uint32_t *)ptr));
                break;
            case EZT_D_DOUBLE:
                rc = snprintf(temp_string, EZTUNE_FORMAT_MAX,
                              "%lf", *((double *)ptr));
                break;

            default:
                MMCAM_LOGW("Invalid item.data");
                break;
            }
        }

        if (rc <= 0) {
            MMCAM_LOGW("Invalid item (%s). Writing 0 as item.value to avoid crash: item.id (%d), table_index(%d), item.data(%d)",
                item.name, item.id, table_index, item.data);

            rc = snprintf(temp_string, EZTUNE_FORMAT_MAX,
                          "%d", 0);
        }

        //debug
        debug_write_paramsdump_file("Item no: %d, Table Index: %d, Value: %s\n", item_num, table_index, temp_string);

        response.append((char *)temp_string, rc + 1);
    }

    MMCAM_LOGV("Number of params obtained: %d, Response buffer size: %zu", i, response.size());

    //debug
    debug_write_paramsdump_file("End of GetParams: Num items written: %d\n\n", i);

    //replace the number of items added with actual elements written
    response.replace(1, 2, (const char *)&i, 2);

end:
    //fill remaining data with 0 bytes
    MMCAM_ASSERT(response_size >= response.size(), "response string overflow");
    response.append(response_size - response.size(), 0);
    return;
}

void ProcessLayer::PrepareResponse( string &response, int event_id)
{
    int8_t status = 0;
    response.append((char *)&status, sizeof(status));

    uint16_t width;
    uint16_t height;
    uint8_t format;
    uint32_t frame_size;

    switch (event_id) {
      case EZTUNE_PREVIEW_NOTIFY:
          m_cam_adapter->GetPreviewInfo(width,
                                        height,
                                        format,
                                        frame_size);
          break;
      case EZTUNE_SNAPSHOT_NOTIFY:
          m_cam_adapter->GetSnapshotInfo(width,
                                height,
                                format,
                                frame_size);
          break;
      default:
          MMCAM_LOGV("%s Invalid Event Notified, Id = %d", __func__, event_id);
          break;
    }
    uint8_t origin = EZTUNE_ORIGIN_BOTTOM_LEFT;

    response.append((char *)&width, sizeof(width));
    response.append((char *)&height, sizeof(height));
    response.append((char *)&format, sizeof(format));
    response.append((char *)&origin, sizeof(origin));
    response.append((char *)&frame_size, sizeof(frame_size));

}

static void debug_item(eztune_set_val_t item)
{
    MMCAM_LOGV("Item num(%d), Item Index(%d), Item string(%s)", item.item_num, item.table_index, item.value_string);

    //debug
    debug_write_paramsdump_file("Item num(%d), Table Index(%d), Value(%s)\n", item.item_num, item.table_index, item.value_string);
}

void ProcessLayer::ProcessSetParamCmd(size_t payload_size, string &payload, size_t &response_size, string &response)
{
    MMCAM_ASSERT_PRE(m_init_done == true, "Process Layer is not yet initialized");

    const char *input = payload.data();

    uint16_t num_items =  *(uint16_t *)input;
    input = input + sizeof(uint16_t);

    MMCAM_ASSERT(num_items < EZT_PARMS_MAX, "Invalid number of items: %u", num_items);

    MMCAM_LOGV("Set Param command");

    //debug
    debug_write_paramsdump_file("Starting SetParams: Num items requested: %d", num_items);

    ::eztune_set_val_t item;

    std::vector<eztune_set_val_t> *pending_cmds = (std::vector<eztune_set_val_t> *)m_pending_cmds;
    int i;
    for (i = 0; i < num_items; ++i) {
        item.item_num = *(uint16_t *)input;
        input = input + sizeof(uint16_t);

        item.table_index = *(uint16_t *)input;
        input = input + sizeof(uint16_t);

        size_t copy_size = strlcpy(item.value_string, input, sizeof(item.value_string));
        MMCAM_ASSERT(copy_size < sizeof(item.value_string), "Item value too long");

        input = input + strlen(input) + 1;

        debug_item(item);

        //We need to maintain state of Eztune, if C-L disconnects and/or
        //camera app exits. We will save each set param coming-in into a
        //binary file and will read back the file (if present), during init and
        //apply the saved params. User can always delete the file if he
        //wants to start afresh
        size_t num = DumpItemBinary(&item);
        MMCAM_ASSERT(num == 1, "Couldn't write command to file, error: %s", strerror(errno));

        //push to pending cmds vector. This gets applied during mscl command
        pending_cmds->push_back(item);
    }

    //debug
    debug_write_paramsdump_file("Completed SetParams: Num items written: %d\n\n", i);

    m_pending_set_param_apply = true;

    response.clear();

    //set response string
    response.append("\x01", 1);

    //fill remaining data with 0 bytes
    MMCAM_ASSERT(response_size >= response.size(), "response string overflow");
    response.append(response_size - response.size(), 0);

    return;
}

void ProcessLayer::ApplyItems(void *ezctrl_data)
{
    eztune_item_t item;
    bool update_chromatix = false;
    bool update_autofocus = false;

    MMCAM_ASSERT(m_pending_cmds != NULL, "Null cmd vector pointter");
    MMCAM_ASSERT(ezctrl_data != NULL, "Null cmd vector pointter");

    eztune_t *ezctrl = (eztune_t *)ezctrl_data;
    std::vector<eztune_set_val_t> *pending_cmds = (std::vector<eztune_set_val_t> *)m_pending_cmds;

    //debug
    debug_write_paramsdump_file("Starting Apply. Number of params %zu\n", pending_cmds->size());

    //get elements from the pending cmds vector and apply change
    for (auto elem = pending_cmds->begin(); elem != pending_cmds->end(); ++elem) {

        item = eztune_get_item((*elem).item_num);

        MMCAM_ASSERT((item.id >= 0) && (item.id < EZT_PARMS_MAX),
                     "Out of range index ID: %d", item.id);

        debug_item(*elem);

        eztune_change_item(&item, &(*elem), ezctrl);

        if (item.type == EZT_T_CHROMATIX)
            update_chromatix = true;
        if ((item.id >= EZT_PARMS_AFTUNE_PROCESSTYPE) &&
                (item.id < EZT_PARMS_AFTUNE_TUNING_TEST_LINEAR_ENABLE))
            update_autofocus = true;
    }

    if (update_chromatix) {
        m_cam_adapter->UpdateCamChromatixData();
    }

    if (update_autofocus) {
        m_cam_adapter->UpdateCamAFTuneData();
    }

    //clear pending commands queue
    pending_cmds->clear();

    //debug
    debug_write_paramsdump_file("Completed Apply. Cleared size %zu\n\n", pending_cmds->size());

    return;
}

void ProcessLayer::ProcessMiscCmd(size_t payload_size, string &payload, size_t &response_size, string &response)
{
    MMCAM_ASSERT_PRE(m_init_done == true, "Process Layer is not yet initialized");

    const char *data = payload.data();
    uint8_t cmd = *(uint8_t *)data;

    response.clear();

    if (cmd == EZTUNE_MISC_GET_VERSION) {
        MMCAM_LOGI("Get version command");
        //insert version number and return
        response.append(eztune::kEztuneVersion);
        goto end;

    } else if (cmd == EZTUNE_MISC_APPLY_CHANGES) {

        MMCAM_LOGI("Apply changes command");

        eztune_t ezctrl;
        if (m_cam_adapter->GetTuningTables(&ezctrl) == false) {
            MMCAM_LOGW("Get Tuning tables failed perhaps due to timeout");
            response.clear();
            response_size = 0;
            goto end;
        }

        ApplyItems(&ezctrl);

        //Assign response
        //TODO: Instead of setting default to 00 (no error), check error condition
        //and set the string accordingly
        response.append("\x00", 1);

        m_pending_set_param_apply = false;
    }

end:
    //fill remaining data with 0 bytes
    MMCAM_ASSERT(response_size >= response.size(), "response string overflow");
    response.append(response_size - response.size(), 0);

    return;
}

void ProcessLayer::EventNotify(uint32_t type, void *data)
{
    if (m_init_done == true) {
        if(EZTUNE_RELOAD_TUNING_NOTIFY == type && GetItemBinarySize() == 0)
        {
            MMCAM_LOGI("Ignore Reload command as there is no tuning updated");
        } else {
            //pipe_fd and 3Alive needed only for Image instance
            //Contol instance will NOT use it
            m_cam_adapter->EventNotify(type, data, m_notify_pipe_fds[1], Get3ALiveStatus());
        }
    } else {
        MMCAM_LOGV("Event skipped as init not done");
    }
}

};
