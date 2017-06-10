/*******************************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

//=============================================================================
/**
 * This file contains eztune network protocol class implementation
 */
//=============================================================================

#include "eztune_protocol.h"

#ifdef _ANDROID_
#include <log/log.h>
#endif
#include "mmcam_log_utils.h"

namespace eztune
{

const char kAcceptAck [] = {0, 1, 0, 0, 0, 1};
const char kCommandAck [] = { 1 };

ProtocolLayer::ProtocolLayer(size_t protocol_buffer_size)
    : m_state(State::TUNESERVER_RECV_COMMAND), m_protocol_buffer_size(protocol_buffer_size)

{
    m_payload.reserve(protocol_buffer_size);

    MMCAM_LOGV("Protocol buffer size: %zu. Capacity(%zu)", protocol_buffer_size, m_payload.capacity());
}

void ProtocolLayer::DeInit()
{
    m_state = State::TUNESERVER_RECV_COMMAND;
}

bool ProtocolLayer::GenerateAcceptAck(string &response, size_t &response_size)
{
    response.clear();

    if (m_mode == EZTUNE_SERVER_CONTROL) {
        response.assign(kAcceptAck, sizeof(kAcceptAck));
        response_size = response.size();
    } else {
        response_size = 0;
    }

    return true;
}

bool ProtocolLayer::GenerateCommandAck(string &response, size_t &response_size)
{
    response.clear();

    if (m_mode == EZTUNE_SERVER_CONTROL) {
        response.assign(kCommandAck, sizeof(kCommandAck));
        response_size = response.size();
    } else {
        response_size = 0;
    }

    return true;
}

bool ProtocolLayer::NextProcessDataSize(size_t &size)
{
    bool rc = true;

    if (m_mode == EZTUNE_SERVER_PREVIEW) {

        if (m_state == State::TUNESERVER_RECV_PAYLOAD)
            size = sizeof(uint32_t);
        else
            size = sizeof(uint16_t);

        goto end;
    }

    switch (m_state) {
    case State::TUNESERVER_RECV_COMMAND:
        size = sizeof(m_cmd);
        break;

    case State::TUNESERVER_RECV_PAYLOAD_SIZE:
        size = sizeof(m_payload_size);
        break;

    case State::TUNESERVER_RECV_PAYLOAD:
        size = m_payload_size;
        break;

    case State::TUNESERVER_RECV_RESPONSE:
        size = sizeof(m_response_size);
        break;

    default:
        size = 0;
        rc = false;
        break;
    }

end:
    return rc;
}

bool ProtocolLayer::ProcessData(string input, size_t input_size, bool &payload_recieved)
{
    payload_recieved = false;
    bool rc = true;

    //special case for preview server mode
    if (m_mode == EZTUNE_SERVER_PREVIEW) {

        switch (m_state) {
        case State::TUNESERVER_RECV_COMMAND:
            m_cmd = ((unsigned char)input[1]) << 8;
            m_cmd |= (unsigned char)input[0];

            if (m_cmd == 2) {
                m_state = State::TUNESERVER_RECV_PAYLOAD;
            } else {
                payload_recieved = true;
            }
            break;

        case State::TUNESERVER_RECV_PAYLOAD:
            m_payload.assign(input.data(), input_size);
            m_state = State::TUNESERVER_RECV_COMMAND;

            payload_recieved = true;

            break;

        default:
            MMCAM_ASSERT(1, "Unexpected state");
            break;
        }

        goto end;
    }

    switch (m_state) {

    case State::TUNESERVER_RECV_COMMAND:
        MMCAM_ASSERT_PRE(input.size() >= sizeof(m_cmd),
                         "Recv Command string size(%zu) less than cmd size(%u)", input.size(), sizeof(m_cmd));

        m_cmd = ((unsigned char)input[1]) << 8;
        m_cmd |= (unsigned char)input[0];

        m_state = State::TUNESERVER_RECV_PAYLOAD_SIZE;
        break;

    case State::TUNESERVER_RECV_PAYLOAD_SIZE:
        MMCAM_ASSERT_PRE(input.size() >= sizeof(m_payload_size),
                         "Payload Command string size(%zu) less than payload cmd size(%u)", input.size(), sizeof(m_payload_size));

        m_payload_size = ((unsigned char)input[3]) << 24;
        m_payload_size |= ((unsigned char)input[2]) << 16;
        m_payload_size |= ((unsigned char)input[1]) << 8;
        m_payload_size |= (unsigned char)input[0];

        m_state = State::TUNESERVER_RECV_PAYLOAD;

        MMCAM_LOGV("Payload size: %d", m_payload_size);

        if (!m_payload_size)
            m_state = State::TUNESERVER_RECV_RESPONSE;

        //Give hint to avoid resizing
        m_payload.reserve(m_protocol_buffer_size);

        break;

    case State::TUNESERVER_RECV_PAYLOAD:

        m_payload.assign(input.data(), input_size);
        m_state = State::TUNESERVER_RECV_RESPONSE;
        break;

    case State::TUNESERVER_RECV_RESPONSE:
        MMCAM_ASSERT_POST(input.size() >= sizeof(m_response_size),
                          "Response string size(%zu) less than response command (%lu)", input.size(), sizeof(m_response_size));

        m_response_size = ((unsigned char)input[3]) << 24;
        m_response_size |= ((unsigned char)input[2]) << 16;
        m_response_size |= ((unsigned char)input[1]) << 8;
        m_response_size |= (unsigned char)input[0];

        m_state = State::TUNESERVER_RECV_COMMAND;

        MMCAM_LOGV("Response size: %d", m_response_size);

        payload_recieved = true;
        break;

    default:
        m_state = State::TUNESERVERER_RECV_INVALID;
        rc = false;
        break;
    }

end:
    return rc;
}

bool ProtocolLayer::GetPayloadInfo(uint16_t &cmd, string &payload, size_t &payload_size, size_t &response_size)
{
    cmd = m_cmd;
    payload = m_payload;
    payload_size = m_payload_size;
    response_size = m_response_size;

    return true;
}

};
