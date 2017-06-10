/*******************************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#ifndef EZTUNE_PROCESS_H
#define EZTUNE_PROCESS_H

#include <string>
#include <map>
#include <cstdint>
#include <vector>

#include "eztune_interface.h"
#include "eztune_internal_types.h"

namespace eztune
{

using std::string;

const string kEztuneVersion("2.1.0\0", 6);
const string kEztuneExtendedVersion("2.1.2\0", 6);
//This file is always enabled to dump set params coming to Eztune
//to be applied at init, to maintain state between app exits and
//reboots
const char kDumpBinaryFile[] = "/data/misc/camera/eztune_intermediate.dat";


//forward declare
class CamAdapter;

//!  Implements processing of payload data associated with each eztune cmd
/*!
  The implementation uses eztune items c library and camera adapter to execute
  the low level functions
*/

class ProcessLayer
{
public:
    ProcessLayer(void *);
    ~ProcessLayer();

    //! This function process the payload data and generates the output response
    /*!
        Internally this class uses a camera adapter class to interface with camera and
        uses eztume_items c API to get/set eztune items

      \param in cmd: 16-bit command from host. The processing depends on this command
      \param in payload_size: Valid size of payload data in payload string
      \param in payload: Payload data string from host
      \param in/out response_size: Expected size of response data to be send back to host
      \param response: Reference to response string to which data has to be written
    */
    void ProcessAndGenerateResponse(uint16_t cmd, size_t payload_size, string &payload, size_t &response_size, string &response);

    //! Init funciton for the process layer. Called once per client
    /*!
        This function should be called to initialize internal data structures. Should be called
        once before calling ProcessAndGenerateResponse to process commands
    */
    bool Init(eztune_server_t mode, int pipe_fd);

    //! DeInit funciton for the process layer. Called once per client when client disconnect
    /*!
        This function should be called when the client disconnects after processing multiple
        ProcessAndGenerateResponse commands
    */
    bool DeInit();

    void PrepareResponse( string &response, int event_id);

    //! Notify camera client events. Events like new meta data available
    /*
       \param in type: Event type
       \param in data: Data associated with event
    */

    void EventNotify(uint32_t type, void *data);

    void StopProcessThread();

    void NotifyBack();

    //static function for starting the process thread worker routine
    static void *ProcThreadWrapper(void *ptr);
    //process thread worker routine
    void ProcessThread();

    void ResumeTuningSession();

    bool Get3ALiveStatus();

private:
    bool m_proc_thread_created;
    bool m_3Alive_active;
    uint8_t *m_preview_buffer;
    uint8_t *m_snapshot_buffer;
    uint8_t *m_scaled_buffer;
    uint8_t *m_exif_buffer;
    uint32_t m_chunk_size;
    uint32_t m_scaled_preview_size;
    uint32_t m_exif_size;
    string m_response;

    bool m_init_done;
    uint16_t m_items_added;
    bool m_pending_set_param_apply;
    bool m_control_port_init;

    eztune::CamAdapter *m_cam_adapter;

    pthread_mutex_t m_cond_mutex, m_lock;
    pthread_cond_t m_data_cond;
    pthread_t m_proc_thread;
    int m_notify_pipe_fds[2];
    int m_intf_pipe_fd;

    //workaround: In actual use this is a pointer to std::vector<eztune_set_val_t> *.
    //But declaring of this type would need including eztune related header files for all users
    void *m_pending_cmds;

    eztune_server_t m_mode;

    struct timeval m_prev_time;

    FILE *m_params_dump_fp;

    void DataWaitNotify();
    bool DataWait();
    void SendEventToInterface(uint32_t event_id, void *data, uint32_t size);
    void ScaleDownImage(void *data);
    void Process3ALiveData(void *data, string &response);
    void GetEXIFHeader();

    //cmd specific processing functions
    void ProcessGetListCmd(size_t payload_size, string &payload, size_t &response_size, string &response);
    void ProcessGetParamCmd(size_t payload_size, string &payload, size_t &response_size, string &response);
    void ProcessSetParamCmd(size_t payload_size, string &payload, size_t &response_size, string &response);
    void ProcessMiscCmd(size_t payload_size, string &payload, size_t &response_size, string &response);
    void ProcessImgTransInfo(size_t payload_size, string &payload, size_t &response_size, string &response);
    void ProcessChangeChunkSize(size_t payload_size, string &payload, size_t &response_size, string &response);
    void ProcessGetPreviewFrame(size_t payload_size, string &payload, size_t &response_size, string &response);
    void ProcessGetSnapshotFrame(size_t payload_size, string &payload, size_t &response_size, string &response);
    void ProcessStart3Alive(size_t payload_size, string &payload, size_t &response_size, string &response);
    void ProcessStop3Alive(size_t payload_size, string &payload, size_t &response_size, string &response);
    void ApplyItems(void *ezctrl);

    //binary command dump routines
    size_t DumpItemBinary(void *ez);
    size_t GetItemBinary(void *ez);
    bool OpenItemBinary();
    void CloseItemBinary();
    int GetItemBinarySize();

};

};

#endif
