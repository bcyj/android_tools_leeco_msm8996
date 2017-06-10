/*******************************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#ifndef EZTUNE_PROTOCOL_H
#define EZTUNE_PROTOCOL_H

#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <assert.h>
#include <vector>

#include "eztune_interface.h"

namespace eztune
{
using std::string;

const uint16_t kMaxProtocolBufferSize = 32768;

//!  Implements host to target eztune protocol.
/*!
  This class deals with analysing the buffer obtained from host side server and
  deciding the next steps.
  This class do not deal with getting the data from network (which could be done
  throuh a separate socket class) or processing payload data
*/
class ProtocolLayer
{
public:
    //! Constructor : Expects max size of network packet that needs to be analysed
    ProtocolLayer(size_t protocol_buffer_size);
    ~ProtocolLayer() = default;

    void DeInit();

    void Init(eztune_server_t mode) { m_mode = mode; }

    bool GenerateAcceptAck(string &response, size_t &response_size);
    bool GenerateCommandAck(string &response, size_t &response_size);

    //! Returns the size of next packet size that needs to be fetched from network
    /*!
        Client needs to calls this API before calling ProcessData as every ProcessData
        call expects exact size of data as returned by this API
    */
    bool NextProcessDataSize(size_t &size);

    //! This function analyse the network packet data
    /*!
        payload_recieved flag is set to true when all data required to process payload
        is received. Once this flag is set, client should call GetPayloadInfo to get
        payload data which can be passed to a processing object (a separate class)
      \param in input: Stream received from the host
      \param in input_size: Size of valid data in input stream
      \param out payload_recieved Boolean flag to indicate if payload data is received.
    */
    bool ProcessData(string input, size_t input_size, bool &payload_recieved);

    //! This function returns the payload info
    /*!
        Should be called only when ProcessData function set payload_recieved flag as true
      \param out cmd: 16-bit command from host
      \param out payload_size: Payload data from host. The string should be allocated
       by client to a size equal to max expected payload size from network
      \param out payload_size: Actual size of payload data
      \param out response_size: Expected size of response data to be send back to host for
       this cmd+payload
    */
    bool GetPayloadInfo(uint16_t &cmd, string &payload, size_t &payload_size, size_t &response_size);

private:
    enum class State
    {
        TUNESERVER_RECV_COMMAND,
        TUNESERVER_RECV_PAYLOAD_SIZE,
        TUNESERVER_RECV_PAYLOAD,
        TUNESERVER_RECV_RESPONSE,
        TUNESERVERER_RECV_INVALID,
    };

    State m_state;
    eztune_server_t m_mode;
    uint16_t m_cmd;
    uint32_t m_payload_size;
    uint32_t m_response_size;
    string m_payload;
    size_t m_protocol_buffer_size;
};

};

#endif
