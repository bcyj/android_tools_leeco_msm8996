/* Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __VOICE_SVC_CLIENT_H__
#define __VOICE_SVC_CLIENT_H__

struct voice_svc_data {
    __u32 src_port;
    __u32 dest_port;
    __u32 token;
    __u32 opcode;
    __u32 payload_size;
    __u8 payload[0];
};

enum voice_svc {
    VOICE_SVC_CVS = 0,
    VOICE_SVC_MVM,
    VOICE_SVC_MAX
};

typedef int (*voice_svc_recv_response_fn)(struct voice_svc_data*);

/*
 * Init function
 *
 * parameters:
 * recv_resp_fn   - callback function that the client wants to register
 *                  that ill be used to receive responses
 *
 * return: 0 if successfull, generic error on failure.
 */
int voice_svc_init(voice_svc_recv_response_fn recv_resp_fn);

/*
 * voice_svc_register
 *
 * parameters:
 * svc        - one of the services defined in enum voice_svc
 * reg_flag   - set to 1 to register otherwise 0 to deregister
 * src_port   - voice services use src_port for session_no.
 *              The clients should start with value starting from 0,
 * svc_handle - Handle that will be updated according to the service
 *              type that will be later sent from client to send
 *              requests to a specific service.
 *
 * return:
 * 0 if successfull, generic error on failure.
 */
int voice_svc_register(enum voice_svc svc, uint32_t src_port,
                       uint8_t reg_flag, uint32_t *svc_handle);

/*
 * voice_svc_send_request
 *
 * parameters:
 * svc_handle - service handle returned in voice_svc_register()
 * svc_data   - service data for the send request
 *
 * return:
 * 0 if successfull, generic error on failure.
 */
int voice_svc_send_request(uint32_t svc_handle, struct voice_svc_data *svc_data);

/*
 * Deinit function
 *
 * return: NA
 */
void voice_svc_deinit();

#endif
