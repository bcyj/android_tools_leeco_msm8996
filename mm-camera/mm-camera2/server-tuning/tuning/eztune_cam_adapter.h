/*******************************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#ifndef EZTUNE_CAM_ADAPTER_H
#define EZTUNE_CAM_ADAPTER_H

#include <string>
#include <map>
#include <cstdint>
#include <pthread.h>

#include "eztune.h"
#include "eztune_interface.h"
#include "eztune_internal_types.h"

namespace eztune
{
using std::string;

//!  Implements Adapter to camera APIs
/*!
  This class deals with send/receive commands to camera and getting chromatix
  and tuning table data from camera
*/
class CamAdapter
{
public:
    CamAdapter(void *handle);

    ~CamAdapter();

    //! Fetches chromatix data and caches in internal private structure
    /*!
        This API called when process layer receives init cmd
    */
    bool FetchChromatixData();

    //! Fetches Auto focus tune data and caches in internal private structure
    /*!
        This API called when process layer receives init cmd
    */
    bool FetchAFTuneData();

    //! Sends command to camera backend to update Chromatix to updated value
    /*!
        In essence copies the cached version of values present in eztune layer to
        camera backend. This API is called when eztune updates any chromatix parameter
    */
    bool UpdateCamChromatixData();

    //! Sends command to camera backend to update AF tune data to updated value
    /*!
        In essence copies the cached version of values present in eztune layer to
        camera backend. This API is called when eztune updates any Auto focus tune data
    */
    bool UpdateCamAFTuneData();

    //! Gets pointers to cached version of tuning tables (chromatix and AF tunetables)
    /*!
        This API is called by eztune process layer when it needs to update any parameter.
        Parameter update will first update the cache version first and then send a command to camera
        backend to update the values

        \param in/out ezctrl: Pointer to eztune_t structure which contains pointers to chromatix, AF tune
        table and meta data table. The pointers are updated to point to the cached version of the tables
    */
    bool GetTuningTables(eztune_t *ezctrl);

    //! Gets pointer to latest metadata from camera backend
    /*!
        This API is called by eztune process layer when it needs to update any parameter. The implementation
        queries camera backend to get the latest meta data pointer
    */

    //! Gets pointer to latest preview buffer from camera
    /*!
    */
    bool GetPreviewBuffer(uint8_t **buffer);

    //! Gets pointer to latest snapshot buffer from camera
    /*!
    */
    bool GetSnapshotBuffer(uint8_t **buffer);

    //! Gets pointer to latest preview buffer from camera
    /*!
    */
    bool GetPreviewInfo(uint16_t &width, uint16_t &height, uint8_t &format, uint32_t &size);

    bool GetSnapshotInfo(uint16_t &width, uint16_t &height, uint8_t &format, uint32_t &size);

    bool Get3AliveInfo(uint8_t &fps, uint16_t &width, uint16_t &height, uint8_t &format);

    //! Notify camera client events. Events like new meta data available
    /*
       \param in type: Event type
       \param in data: Data associated with event
    */
    void EventNotify(uint32_t type, void *data, int pipe_fd, bool live_started);

    //! Adapter may update some data structure like meta data in async fashion. If clients calls this API
    //  Async updates are not done
    int Lock() { return pthread_mutex_lock(&m_lock); }

    //! Disable locks and Enable internal updates
    int UnLock() { return pthread_mutex_unlock(&m_lock); }

    //static wrapper call for C API
    static void SendCmdWrapper(void *ptr, uint32_t cmd, void *value);

    //! Sends command to camera backend
    /*!
        This API is called by eztune process layer when it needs get/set any parameters including getting
        chromatix tables and AF tune tables

        \param in cmd: Command that is understood by camera
        \param in value: Pointer to paramter structure for the command
        \get_param: If set to true, it gets a param else set param
    */
    bool SendCmd(uint32_t cmd, void *value, bool get_param);

    void TriggerPreviewBufferCopy(bool value);

    void TriggerSnapshotBufferCopy(bool value);

    void SetMetadataPending(bool value);

    bool TriggerSnapshot();

    void GetPreviewDimension(uint32_t &width, uint32_t &height);

    cam_metadata_info_t *GetMetadata();

    void MetadataLock();

    void MetadataUnlock();

    // This function returns the current chromatix version
    // which is generally a non-zero value
    int GetChromatixVersion();

private:
    int TryLock() { return pthread_mutex_trylock(&m_lock); }

    bool DataWait();
    void DataWaitNotify();

    void CopyPreviewImage(void *buffer);
    void CopySnapshotImage(void *ptr);
    void SendProcessEvent(EztuneNotify event_id, void *data, uint32_t size, int pipe_fd);

    void *m_client_handle;
    tune_chromatix_t *m_chromatix;
    tune_autofocus_t *m_autofocus;
    cam_metadata_info_t *m_metadata;
    pthread_mutex_t m_lock, m_cond_mutex;
    pthread_cond_t m_data_cond;
    bool m_pending_metadata_request;
    bool m_pending_preview_request;
    bool m_pending_snapshot_request;
    bool m_snapshot_triggered;

    int32_t m_preview_image_width;
    int32_t m_preview_image_height;
    int32_t m_snapshot_image_width;
    int32_t m_snapshot_image_height;
    cam_format_t m_preview_format;
    uint8_t *m_preview_buffer;
    uint8_t *m_snapshot_buffer;
    uint32_t m_old_preview_size;
    uint32_t m_curr_snapshot_size;
    ez_af_tuning_params_t m_af_tuning;
};

};

#endif //EZTUNE_CAM_ADAPTER_H
