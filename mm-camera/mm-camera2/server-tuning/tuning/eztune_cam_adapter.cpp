/*******************************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

//=============================================================================
/**
 * This file contains eztune network protocol class implementation
 */
//=============================================================================

#include <stdlib.h>

#include "mct_pipeline.h"
#include "mct_stream.h"
#include "mct_list.h"
#include "eztune.h"
#include "eztune_cam_adapter.h"

#ifdef _ANDROID_
#include <log/log.h>
#endif
#include "mmcam_log_utils.h"

typedef struct {
    tune_cmd_t module_cmd;
    int32_t action;
    cam_eztune_cmd_data_t aaa_cmd;
} tune_set_data_t;

extern "C" eztune_item_t eztune_get_item (int i);
extern "C" void eztune_change_item(eztune_item_t *item, eztune_set_val_t *item_data, eztune_t *ezctrl);

extern "C" mct_list_t *mct_list_find_custom (mct_list_t *mct_list, void *data, mct_list_find_func list_find);
extern "C" boolean mct_trigger_snapshot(void);
extern "C" void dump_list_of_daemon_fd ();

//to enable "c" function access to adapter class
static void *cam_adapter_instance = NULL;

void tuning_set_autofocus(void *ctrl, aftuning_optype_t optype, uint8_t value)
{
    void *param = NULL;

    eztune_t *ezctrl =  (eztune_t *) ctrl;
    tune_actuator_t act_tuning;
    param = &act_tuning;

    switch (optype) {
    case EZ_AF_LOADPARAMS:
        act_tuning.ttype = ACTUATOR_TUNE_RELOAD_PARAMS;
        break;
    case EZ_AF_LINEARTEST_ENABLE:
        act_tuning.ttype = ACTUATOR_TUNE_TEST_LINEAR;
        act_tuning.stepsize =
            ezctrl->af_tuning->linearstepsize;
        break;
    case EZ_AF_RINGTEST_ENABLE:
        act_tuning.stepsize =
            ezctrl->af_tuning->ringstepsize;
        act_tuning.ttype = ACTUATOR_TUNE_TEST_RING;
        break;
    case EZ_AF_MOVFOCUSTEST_ENABLE:
        act_tuning.ttype = ACTUATOR_TUNE_MOVE_FOCUS;
        act_tuning.direction =
            ezctrl->af_tuning->movfocdirection;
        act_tuning.num_steps =
            ezctrl->af_tuning->movfocsteps;
        break;
    case EZ_AF_DEFFOCUSTEST_ENABLE:
        act_tuning.ttype = ACTUATOR_TUNE_DEF_FOCUS;
        break;
    }

    eztune::CamAdapter::SendCmdWrapper(cam_adapter_instance, TUNING_SET_AUTOFOCUS_TUNING, param);

    return;
}

void tuning_set_vfe(void *ctrl, vfemodule_t module, optype_t optype, int32_t value)
{
    tune_set_data_t vfe_set;
    vfe_set.module_cmd.module = module;
    vfe_set.module_cmd.type = optype;
    vfe_set.module_cmd.value = value;

    eztune::CamAdapter::SendCmdWrapper(cam_adapter_instance, TUNING_SET_VFE_COMMAND, (void *)&vfe_set.module_cmd);
}

void tuning_set_pp(void *ctrl, pp_module_t module, optype_t optype, int32_t value)
{
    tune_set_data_t pp_set;
    pp_set.module_cmd.module = module;
    pp_set.module_cmd.type = optype;
    pp_set.module_cmd.value = value;

    eztune::CamAdapter::SendCmdWrapper(cam_adapter_instance, TUNING_SET_POSTPROC_COMMAND, (void *)&pp_set.module_cmd);
}

void tuning_set_3a(void *ctrl, aaa_set_optype_t optype, int32_t value)
{
    tune_set_t set_type = TUNING_SET_MAX;
    tune_set_data_t aaa_set;

    void *param = &aaa_set.aaa_cmd;

    switch (optype) {
    case EZ_STATUS:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_set.aaa_cmd.cmd = CAM_EZTUNE_CMD_STATUS;
        aaa_set.aaa_cmd.u.running = value;
        break;

    case EZ_AEC_ENABLE:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_set.aaa_cmd.cmd = CAM_EZTUNE_CMD_AEC_ENABLE;
        aaa_set.aaa_cmd.u.aec_enable = value;
        break;
    case EZ_AEC_TESTENABLE:
        break;
    case EZ_AEC_LOCK:
        if (value)
            set_type = TUNING_SET_AEC_LOCK;
        else
            set_type = TUNING_SET_AEC_UNLOCK;
        aaa_set.action = value;
        param = &value;
        break;
    case EZ_AEC_FORCEPREVEXPOSURE:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_set.aaa_cmd.cmd = CAM_EZTUNE_CMD_AEC_FORCE_EXP;
        aaa_set.aaa_cmd.u.ez_force_param.forced = 1;
        aaa_set.aaa_cmd.u.ez_force_param.u.force_exp_value = (float)(value) / Q10;
        break;
    case EZ_AEC_FORCEPREVGAIN:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_set.aaa_cmd.cmd = CAM_EZTUNE_CMD_AEC_FORCE_GAIN;
        aaa_set.aaa_cmd.u.ez_force_param.forced = 1;
        aaa_set.aaa_cmd.u.ez_force_param.u.force_gain_value = (float)(value) / Q10;
        break;
    case EZ_AEC_FORCEPREVLINECOUNT:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_set.aaa_cmd.cmd = CAM_EZTUNE_CMD_AEC_FORCE_LINECOUNT;
        aaa_set.aaa_cmd.u.ez_force_param.forced = 1;
        aaa_set.aaa_cmd.u.ez_force_param.u.force_linecount_value = value;
        break;
    case EZ_AEC_FORCESNAPEXPOSURE:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_set.aaa_cmd.cmd = CAM_EZTUNE_CMD_AEC_FORCE_SNAP_EXP;
        aaa_set.aaa_cmd.u.ez_force_param.forced = 1;
        aaa_set.aaa_cmd.u.ez_force_param.u.force_snap_exp_value = (float)(value) / Q10;;
        break;
    case EZ_AEC_FORCESNAPGAIN:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_set.aaa_cmd.cmd = CAM_EZTUNE_CMD_AEC_FORCE_SNAP_GAIN;
        aaa_set.aaa_cmd.u.ez_force_param.forced = 1;
        aaa_set.aaa_cmd.u.ez_force_param.u.force_snap_gain_value = (float)(value) / Q10;
        break;
    case EZ_AEC_FORCESNAPLINECOUNT:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_set.aaa_cmd.cmd = CAM_EZTUNE_CMD_AEC_FORCE_SNAP_LC;
        aaa_set.aaa_cmd.u.ez_force_param.forced = 1;
        aaa_set.aaa_cmd.u.ez_force_param.u.force_snap_linecount_value = value;
        break;
    case EZ_AWB_MODE:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_set.aaa_cmd.cmd = CAM_EZTUNE_CMD_AWB_MODE;
        aaa_set.aaa_cmd.u.awb_mode = value;
        break;
    case EZ_AWB_ENABLE:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_set.aaa_cmd.cmd = CAM_EZTUNE_CMD_AWB_ENABLE;
        aaa_set.aaa_cmd.u.awb_enable = value;
        break;
    case EZ_AWB_LOCK:
        if (value)
            set_type = TUNING_SET_AWB_LOCK;
        else
            set_type = TUNING_SET_AWB_UNLOCK;

        param = &value;
        break;
    case EZ_AF_ENABLE:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_set.aaa_cmd.cmd = CAM_EZTUNE_CMD_AF_ENABLE;
        aaa_set.aaa_cmd.u.af_enable = value;
        break;
    default:
        break;
    }

    if (set_type != TUNING_SET_MAX) {
        eztune::CamAdapter::SendCmdWrapper(cam_adapter_instance, set_type, param);
    } else {
        MMCAM_LOGW("No matching 3A command: %d", optype);
    }

    return;
}

namespace eztune {

static std::map<tune_set_t, cam_intf_parm_type_t> kTuningCmdToCamIntfCmd;

CamAdapter::CamAdapter(void *handle)
{
    cam_adapter_instance = this;

    MMCAM_ASSERT(handle != NULL, "Null Camera client handle");
    m_client_handle = handle;

    //map tuning cmds to cam intf commands
    kTuningCmdToCamIntfCmd[TUNING_SET_RELOAD_CHROMATIX] = CAM_INTF_PARM_SET_RELOAD_CHROMATIX;
    kTuningCmdToCamIntfCmd[TUNING_SET_RELOAD_AFTUNE] = CAM_INTF_PARM_SET_RELOAD_AFTUNE;
    kTuningCmdToCamIntfCmd[TUNING_SET_AUTOFOCUS_TUNING] = CAM_INTF_PARM_SET_AUTOFOCUSTUNING;
    kTuningCmdToCamIntfCmd[TUNING_SET_VFE_COMMAND] = CAM_INTF_PARM_SET_VFE_COMMAND;
    kTuningCmdToCamIntfCmd[TUNING_SET_POSTPROC_COMMAND] = CAM_INTF_PARM_SET_PP_COMMAND;
    kTuningCmdToCamIntfCmd[TUNING_SET_3A_COMMAND] = CAM_INTF_PARM_EZTUNE_CMD;
    kTuningCmdToCamIntfCmd[TUNING_SET_AEC_LOCK] = CAM_INTF_PARM_AEC_LOCK;
    kTuningCmdToCamIntfCmd[TUNING_SET_AEC_UNLOCK] = CAM_INTF_PARM_AEC_LOCK;
    kTuningCmdToCamIntfCmd[TUNING_SET_AWB_LOCK] = CAM_INTF_PARM_AWB_LOCK;
    kTuningCmdToCamIntfCmd[TUNING_SET_AWB_UNLOCK] = CAM_INTF_PARM_AWB_LOCK;

    MMCAM_ASSERT(sizeof(tune_autofocus_t) >= sizeof(actuator_driver_params_t) + sizeof(af_algo_tune_parms_t),
                 "AF tune buffer size smaller than needed");

    m_chromatix = new ::tune_chromatix_t;
    m_autofocus = new ::tune_autofocus_t;
    m_metadata  = new ::cam_metadata_info_t;

    MMCAM_ASSERT((m_chromatix != NULL) && (m_autofocus != NULL) && (m_metadata != NULL), "Null tuning pointers");

    memset(m_chromatix, 0, sizeof(::tune_chromatix_t));

    m_pending_metadata_request = false;
    m_pending_preview_request = false;
    m_pending_snapshot_request = false;
    // This is to keep track of the snapshot event
    // triggered from Camera app. This is used to reload tuning
    // while going back to preview in non-zsl snapshot use case
    m_snapshot_triggered = false;

    pthread_mutex_init(&m_lock, 0);
    pthread_mutex_init(&m_cond_mutex, 0);
    pthread_cond_init(&m_data_cond, 0);

    m_preview_image_width = 0;
    m_preview_image_height = 0;
    m_snapshot_image_width = 0;
    m_snapshot_image_height = 0;
    m_preview_buffer = NULL;
    m_snapshot_buffer = NULL;
    m_old_preview_size = 0;
    m_curr_snapshot_size = 0;
    memset(&m_af_tuning, 0, sizeof(ez_af_tuning_params_t));
}

CamAdapter::~CamAdapter()
{
    pthread_cond_destroy(&m_data_cond);
    pthread_mutex_destroy(&m_cond_mutex);
    pthread_mutex_destroy(&m_lock);

    delete m_chromatix;
    delete m_autofocus;
    delete m_metadata;
    delete[] m_preview_buffer;
    delete[] m_snapshot_buffer;

    cam_adapter_instance = NULL;
}

void CamAdapter::DataWaitNotify()
{
    pthread_mutex_lock(&m_cond_mutex);
    pthread_cond_signal(&m_data_cond);
    pthread_mutex_unlock(&m_cond_mutex);
}

bool CamAdapter::DataWait()
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

bool CamAdapter::GetTuningTables(eztune_t *ezctrl)
{
    ezctrl->chromatixptr = (chromatix_parms_type *)&m_chromatix->chromatixData[0];
    ezctrl->snapchromatixptr = (chromatix_parms_type *)&m_chromatix->snapchromatixData[0];
    ezctrl->common_chromatixptr = (chromatix_VFE_common_type *)&m_chromatix->common_chromatixData[0];

    ezctrl->af_driver_ptr = (actuator_driver_params_t *)m_autofocus->af_tuneData;

    ezctrl->af_tune_ptr = (af_algo_tune_parms_t *) ((size_t)m_autofocus->af_tuneData +
                          sizeof(actuator_driver_params_t));

    ezctrl->af_tuning = &m_af_tuning;

    //set metadata request as true. Meta data is fetched only when new
    //requests are made. Locks are used as metadata update can happen from
    //separate thread context
    Lock();
    m_pending_metadata_request = true;
    UnLock();

    if (DataWait() == true) {
        ezctrl->metadata = m_metadata;
        return true;
    } else {
        return false;
    }
}

bool CamAdapter::GetPreviewInfo(uint16_t &width, uint16_t &height, uint8_t &format, uint32_t &size)
{
    width = m_preview_image_width;
    height = m_preview_image_height;
    format = EZTUNE_FORMAT_YCrCb_420;
    size = m_old_preview_size;

    return true;
}

bool CamAdapter::GetSnapshotInfo(uint16_t &width, uint16_t &height, uint8_t &format, uint32_t &size)
{
    width = m_snapshot_image_width;
    height = m_snapshot_image_height;
    format = EZTUNE_FORMAT_JPG;
    size = m_curr_snapshot_size;

    return true;
}

bool CamAdapter::Get3AliveInfo(uint8_t &fps, uint16_t &width, uint16_t &height, uint8_t &format)
{
    fps = kEztuneMinFps; //tbd - get the mn fps from backend
    width = kEztuneScaledWidth;
    height = kEztuneScaledHeight;
    format = EZTUNE_FORMAT_YCrCb_420;

    return true;
}

void CamAdapter::TriggerPreviewBufferCopy(bool value)
{
    //set flag for preview buffer copy
    Lock();
    m_pending_preview_request = value;
    UnLock();
}

void CamAdapter::TriggerSnapshotBufferCopy(bool value)
{
    //set flag for snapshot buffer copy
    Lock();
    m_pending_snapshot_request = value;
    UnLock();
}

void CamAdapter::SetMetadataPending(bool value)
{
    //set flag for metadata copy
    Lock();
    m_pending_metadata_request = value;
    UnLock();
}

bool CamAdapter::TriggerSnapshot()
{
    TriggerSnapshotBufferCopy(true);

    //trigger snapshot at camera backend
    if (mct_trigger_snapshot() == false) {
        MMCAM_LOGW("Failed to trigger snapshot");
        TriggerSnapshotBufferCopy(false);
        return false;
    }

    return true;
}

void CamAdapter::GetPreviewDimension(uint32_t &width, uint32_t &height)
{
    width = m_preview_image_width;
    height = m_preview_image_height;

    return;
}

cam_metadata_info_t* CamAdapter::GetMetadata()
{
    return m_metadata;
}

void CamAdapter::MetadataLock()
{
    Lock();
}

void CamAdapter::MetadataUnlock()
{
    UnLock();
}

void CamAdapter::CopyPreviewImage(void *ptr)
{
    mct_stream_info_t *streaminfo = (mct_stream_info_t *)ptr;
    int32_t width = streaminfo->dim.width;
    int32_t  height = streaminfo->dim.height;
    cam_frame_len_offset_t plane_info = streaminfo->buf_planes.plane_info;
    cam_format_t format = streaminfo->fmt;

    MMCAM_ASSERT((format == CAM_FORMAT_YUV_420_NV12) || (format == CAM_FORMAT_YUV_420_NV21), "Do not recognize format");

    MMCAM_LOGV("num_planes: %d", plane_info.num_planes);

    uint32_t size = 0;
    for (uint32_t i = 0; i < plane_info.num_planes; i++) {
        size += plane_info.mp[i].height * plane_info.mp[i].width;
    }
    MMCAM_ASSERT(size != 0, "Zero preview buffer size");

    //delete and allocate new buffer if size differs
    if (m_old_preview_size != size) {
        delete[] m_preview_buffer;
        //mem aligned because of possible fast cv operation on buffer
        bool ret = posix_memalign((void **)&m_preview_buffer, 128, size);
        //additional check for safety
        MMCAM_ASSERT(ret == 0, "posix_memalign returned failure");
        m_old_preview_size = size;
    }

    MMCAM_ASSERT(m_preview_buffer != NULL, "preview buffer null");

    //Actual buffer pointer is passed through the unused img_buffer_list
    uint8_t *buffer = (uint8_t *)streaminfo->img_buffer_list;
    uint8_t *out_buffer = m_preview_buffer;

    MMCAM_LOGV("Buffer (%p), width(%d), height(%d), format(%d), size(%zu)", buffer, width, height, format, m_old_preview_size);

    for (uint32_t i = 0; i < plane_info.num_planes; i++) {
        uint32_t index = plane_info.mp[i].offset;
        if (i > 0) {
            index += plane_info.mp[i - 1].len;
        }
        for (int j = 0; j < plane_info.mp[i].height; j++) {
            void *data = (void *)(buffer + index);
            memcpy(out_buffer, data, plane_info.mp[i].width);
            index += plane_info.mp[i].stride;
            out_buffer = out_buffer + plane_info.mp[i].width;
        }
    }

    //update preview info
    m_preview_image_width = width;
    m_preview_image_height = height;
    m_preview_format = format;
}

void CamAdapter::CopySnapshotImage(void *ptr)
{
    cam_int_evt_params_t *params = (cam_int_evt_params_t *)ptr;

    MMCAM_ASSERT(params != NULL, "JPEG params null");
    MMCAM_ASSERT(params->path != NULL, "JPEG file path null");

    int file_fd = open(params->path, O_RDONLY);
    if (file_fd >= MAX_FD_PER_PROCESS) {
        dump_list_of_daemon_fd();
        file_fd = -1;
    }
    //assert on fd, as open should never fail in this case
    MMCAM_ASSERT(file_fd >= 0, "Snapshot file open failed");

    if (m_curr_snapshot_size != params->size) {
        delete[] m_snapshot_buffer;
        m_snapshot_buffer = new uint8_t[params->size];
        m_curr_snapshot_size = params->size;
    }

    MMCAM_ASSERT(m_snapshot_buffer != NULL, "snapshot buffer null");

    ssize_t read_len = read(file_fd, m_snapshot_buffer, params->size);
    MMCAM_ASSERT(read_len == (ssize_t)params->size,
            "Snapshot file read returned less size than expected");

    close(file_fd);

    m_snapshot_image_width = params->dim.width;
    m_snapshot_image_height = params->dim.height;
}

void CamAdapter::SendProcessEvent(EztuneNotify event_id, void *data, uint32_t size, int pipe_fd)
{
    ProcessThreadMessage process_thrd_msg;

    process_thrd_msg.event_id = (uint32_t)event_id;
    process_thrd_msg.data = data;
    process_thrd_msg.size = size;
    int val = write(pipe_fd, &process_thrd_msg, sizeof(process_thrd_msg));
    MMCAM_ASSERT(val != -1, "Write to pipe failed, write returned: %s", strerror(errno));
}

void CamAdapter::EventNotify(uint32_t type, void *data, int pipe_fd, bool live_started)
{
    if (TryLock() == 0) {
        switch (type) {
        case EZTUNE_METADATA_NOTIFY:
            if (m_pending_metadata_request == true) {
                //make a copy of metadata, clear pending request and notify
                MMCAM_LOGV("Copying new metadata");
                memcpy((void *)m_metadata, data, sizeof(cam_metadata_info_t));
                m_pending_metadata_request = false;
                DataWaitNotify();
            }
            break;

        case EZTUNE_PREVIEW_NOTIFY:
            if (m_pending_preview_request == true) {
                //make a copy of preview buff, clear pending request and notify
                MMCAM_LOGV("Copying preview buffer");
                CopyPreviewImage(data);
                m_pending_preview_request = false;
                UnLock();
                if (live_started == false) {
                    SendProcessEvent(EZTUNE_PREVIEW_NOTIFY, m_preview_buffer, m_old_preview_size, pipe_fd);
                } else {
                    SendProcessEvent(EZTUNE_3ALIVE_NOTIFY, m_preview_buffer, m_old_preview_size, pipe_fd);
                }
            }
            break;

        case EZTUNE_SNAPSHOT_NOTIFY:
            if (m_pending_snapshot_request == true) {
                //read the snapshot buff, clear pending request and notify
                MMCAM_LOGV("Copying snapshot buffer");
                //we can also read the snapshot buff in the proc thread, shall we?
                CopySnapshotImage(data);
                m_pending_snapshot_request = false;
                UnLock();
                SendProcessEvent(EZTUNE_SNAPSHOT_NOTIFY, m_snapshot_buffer, m_curr_snapshot_size, pipe_fd);
            }
            break;

        case EZTUNE_RELOAD_TUNING_NOTIFY:
            if (m_snapshot_triggered) {
                // In case of non ZSL snapshot
                // we need to update the backend with Eztune copy of tuning data
                UpdateCamChromatixData();
                m_snapshot_triggered = false;
            }
            break;

        case EZTUNE_SNAPSHOT_PENDING_NOTIFY:
            m_snapshot_triggered = true;
            break;

        default:
            MMCAM_LOGW("Unexpected Event: %d, Ignored", type);
            break;
        }

        UnLock();

    } else {
        MMCAM_LOGV("Lock is held. Event skipped");
    }

end:
    return;
}

bool CamAdapter::FetchChromatixData()
{
    SendCmd(CAM_INTF_PARM_GET_CHROMATIX, m_chromatix, true);
    return true;
}

int CamAdapter::GetChromatixVersion()
{
    eztune_t ezctrl;
    ezctrl.chromatixptr = (chromatix_parms_type *)&m_chromatix->chromatixData[0];
    MMCAM_LOGI("%s, Chromatix version %d", __func__, ezctrl.chromatixptr->chromatix_version);
    return ezctrl.chromatixptr->chromatix_version;
}

bool CamAdapter::FetchAFTuneData()
{
    SendCmd(CAM_INTF_PARM_GET_AFTUNE, m_autofocus, true);
    return true;
}

bool CamAdapter::UpdateCamChromatixData()
{
    if(GetChromatixVersion() != 0) {
        SendCmd(CAM_INTF_PARM_SET_RELOAD_CHROMATIX, m_chromatix, false);
    } else {
        // Eztune does not have a valid Chromatix data as the version is 0
        // Try to fetch the chromatix data  now from backend to update
        // Eztune copy of chromatix data
        FetchChromatixData();
        if(GetChromatixVersion() == 0) {
            MMCAM_LOGE("Ignore Reload as Eztune could not get valid data in second attempt also");
        }
    }
    return true;
}

bool CamAdapter::UpdateCamAFTuneData()
{
    SendCmd(CAM_INTF_PARM_SET_RELOAD_AFTUNE, m_autofocus, false);
    return true;
}

typedef enum _mct_pipeline_check_stream {
    CHECK_INDEX,
    CHECK_TYPE,
    CHECK_SESSION
} mct_pipeline_check_stream_t;

typedef struct _mct_pipeline_get_stream_info {
    mct_pipeline_check_stream_t check_type;
    unsigned int                stream_index;
    cam_stream_type_t           stream_type;
    unsigned int                session_index;
} mct_pipeline_get_stream_info_t;

static boolean check_mct_stream(void *d1, void *d2)
{
    mct_stream_t                   *stream = (mct_stream_t *)d1;
    mct_pipeline_get_stream_info_t *info   =
        (mct_pipeline_get_stream_info_t *)d2;

    if (info->check_type == CHECK_INDEX) {
        return (stream->streamid
                == (unsigned int)info->stream_index ? TRUE : FALSE);
    } else if (info->check_type == CHECK_TYPE) {
        if (MCT_STREAM_STREAMINFO(stream))
            return ((cam_stream_info_t *)(MCT_STREAM_STREAMINFO(stream)))->stream_type
                   == info->stream_type ? TRUE : FALSE;
    } else if (info->check_type == CHECK_SESSION) {
        return (((stream->streaminfo.identity & 0xFFFF0000) >> 16)
                == (unsigned int)info->session_index ? TRUE : FALSE);
    }

    return FALSE;
}

static mct_stream_t *get_mct_stream(mct_pipeline_t *pipeline,
                                    mct_pipeline_get_stream_info_t *get_info)
{
    mct_list_t *find_list = NULL;

    if (!MCT_PIPELINE_CHILDREN(pipeline)) {
        MMCAM_LOGE("%s: no children", __func__);
        return NULL;
    }

    MCT_OBJECT_LOCK(pipeline);
    find_list = ::mct_list_find_custom(MCT_PIPELINE_CHILDREN(pipeline),
                                       get_info, check_mct_stream);
    MCT_OBJECT_UNLOCK(pipeline);

    if (!find_list) {
        MMCAM_LOGE("%s: stream not found in the list", __func__);
        return NULL;
    }

    return MCT_STREAM_CAST(find_list->data);
}


bool CamAdapter::SendCmd(uint32_t cmd, void *value, bool get_param)
{
    MMCAM_LOGV("Sending command: %u, Value: %p", cmd, value);

    mct_pipeline_t *pipeline = (mct_pipeline_t *)m_client_handle;

    mct_event_t cmd_event;
    mct_event_control_t event_data;
    mct_event_control_parm_t event_parm;

    if (get_param == true)
        event_data.type = MCT_EVENT_CONTROL_GET_PARM;
    else
        event_data.type = MCT_EVENT_CONTROL_SET_PARM;

    event_data.control_event_data = &event_parm;

    event_parm.type = (cam_intf_parm_type_t) cmd;
    event_parm.parm_data = value;

    mct_pipeline_get_stream_info_t info;
    info.check_type   = CHECK_TYPE;
    info.stream_type  = CAM_STREAM_TYPE_PREVIEW;
    mct_stream_t *stream = get_mct_stream(pipeline, &info);

    //ignore sending message if stream is null or stream state not running
    if (stream != NULL && stream->state == MCT_ST_STATE_RUNNING) {

        cmd_event = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
                                            (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid)),
                                            MCT_EVENT_DOWNSTREAM, &event_data);

        pipeline->send_event(pipeline, stream->streamid, &cmd_event);
    } else {
        MMCAM_LOGW("Message send ignored as stream or stream state not valid");
    }

    return true;
}

void CamAdapter::SendCmdWrapper(void *ptr, uint32_t cmd, void *value)
{
    if (ptr) {
        auto elem = kTuningCmdToCamIntfCmd.find((tune_set_t)cmd);
        MMCAM_ASSERT(elem != kTuningCmdToCamIntfCmd.end(), "Error mapping tuning cmd to intf cmd");
        cam_intf_parm_type_t intf_cmd = elem->second;

        CamAdapter *obj = static_cast<CamAdapter *>(ptr);
        obj->SendCmd((uint32_t)intf_cmd, value, false);
    } else {
        MMCAM_LOGW("Null pointer to obj. Send cmd ignored");
    }
    return ;
}

};
