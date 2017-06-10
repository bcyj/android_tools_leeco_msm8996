/******************************************************************************

  @file   subsystem_control_client.c

  ---------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

#define LOG_TAG "subsystem_control"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <cutils/log.h>
#include <common_log.h>

#include <qmi.h>
#include <qmi_client.h>
#include <qmi_idl_lib_internal.h>
#include <qmi_client_instance_defs.h>

#include <subsystem_control_v01.h>
#include <subsystem_control_client.h>
#include <mdm_detect.h>

#define MAX_SHUTDOWN_CHECK_COUNT 100
#define SHUTDOWN_CHECK_RETRY_US 1000 * 100
qmi_idl_service_object_type ssc_service_object;
qmi_client_type ssc_clnt, ssc_notifier;
int ssc_active = 0;

struct ssctl_data {
	void *id;
	char device_name[MAX_NAME_LEN];
	pthread_mutex_t shutdown_wait;
};
static volatile int ssc_in_progress = 0;
static pthread_mutex_t ssc_wait = PTHREAD_MUTEX_INITIALIZER;

static int qmi_send_msg_sync(int cmd, void *req, int req_len, void *resp,
        int resp_len)
{
    int rc;
    rc = qmi_client_send_msg_sync(ssc_clnt, cmd, req, req_len, resp,
            resp_len, TIMEOUT_MS);
    return rc;
}

static void ssc_qmi_send_cmd(struct ssc_req *client_req,
        struct ssc_resp *client_resp)
{
    int rc, req_len, resp_len, cmd, retry = 0;
    void *req, *resp;
    qmi_ssctl_restart_resp_msg_v01 restart_resp;
    qmi_ssctl_shutdown_resp_msg_v01 shutdown_resp;
    qmi_ssctl_get_failure_reason_resp_msg_v01 failure_reason_resp;

    if (client_req->cmd < 0 ||
        client_req->cmd >= (sizeof(qmi_command)/sizeof(qmi_service_instance))) {
        client_resp->rc = -1;
        return;
    }

    cmd = qmi_command[client_req->cmd];
    client_resp->length = 0;

    switch (client_req->cmd) {
        case CMD_SHUTDOWN:
            pthread_mutex_lock(&ssc_wait);
            req = NULL;
            req_len = 0;
            resp = &shutdown_resp;
            resp_len = sizeof(shutdown_resp);
            rc = qmi_send_msg_sync(cmd, req, req_len, resp, resp_len);
            if (rc == QMI_NO_ERR) {
                client_resp->rc = shutdown_resp.resp.error;
                for (retry = 0; retry <= 10; retry++) {
                    rc = pthread_mutex_trylock(&ssc_wait);
                    if (rc == 0)
                        break;
                    sleep(1);
                }
                if (rc == 0)
                    ALOGI("Received subsystem shutdown complete in %d second(s)", retry);
                else {
                    ALOGE("Timed out waiting for shutdown completion; continuing anyways");
                    client_resp->rc = -1;
                }
            } else
                client_resp->rc = -1;
            pthread_mutex_unlock(&ssc_wait);
            break;

        default:
            client_resp->rc = -1;
    };
}

static void ssc_indication(qmi_client_type handle, unsigned long msg_id,
                unsigned char *buffer, int buffer_len, void *indication_data)
{
    if (msg_id == QMI_SSCTL_SHUTDOWN_READY_IND_V01)
        pthread_mutex_unlock(&ssc_wait);
    else
        LOGE("Got unknown subsystem control indicator 0x%08lx", msg_id);
}

static int ssc_qmi_init(int proc_num)
{
    int rc;
    qmi_cci_os_signal_type os_params;
    qmi_service_info info;

    /* Init subsystem control object */
    ssc_service_object = ssctl_get_service_object_v01();

    rc = qmi_client_notifier_init(ssc_service_object, &os_params,
            &ssc_notifier);
    if (rc != QMI_NO_ERR)
    {
        LOGE("Failed to initialize notifier.");
        return -EINVAL;
    }

    /* wait for server to come up */
    QMI_CCI_OS_SIGNAL_WAIT(&os_params, TIMEOUT_MS);
    if(os_params.timed_out)
    {
        LOGE("Timeout! No signal received.");
        qmi_client_release(ssc_notifier);
        return -EINVAL;
    }
    QMI_CCI_OS_SIGNAL_CLEAR(&os_params);

    rc = qmi_client_get_service_instance(ssc_service_object,
            qmi_instance_id[proc_num], &info);

    if(rc != QMI_NO_ERR)
    {
        LOGE("Failed to find service.");
        qmi_client_release(ssc_notifier);
        return -EINVAL;
    }

    /* Initialize connection to QMI subsystem control port */
    rc = qmi_client_init(&info, ssc_service_object,
                         (qmi_client_ind_cb)ssc_indication,
                         NULL, &os_params,
            &ssc_clnt);
    if (rc != QMI_NO_ERR)
    {
        LOGE("Failed to initialize client.");
        qmi_client_release(ssc_notifier);
        return -EINVAL;
    }

    ssc_active = 1;

    return 0;
}

static void ssc_qmi_release(void)
{
    if(ssc_active)
    {
        qmi_client_release(ssc_clnt);
        qmi_client_release(ssc_notifier);
        ssc_active = 0;
    }
}

static int ssc_process_command(struct ssc_req *req, struct ssc_resp *resp)
{
    int rc;

    if((req->cmd < CMD_START) || (req->cmd > CMD_END))
    {
        LOGE("CMD[%u] is invalid.", req->cmd);
        return -EINVAL;
    }

    rc = ssc_qmi_init(req->proc_num);
    if(rc)
    {
        LOGE("Failed to do QMI init");
        return -EINVAL;
    }

    ssc_qmi_send_cmd(req, resp);
    ssc_qmi_release();

    return 0;
}

void ssctl_shutdown_cb(void *handle, enum pm_event event)
{
	struct ssctl_data *data = (struct ssctl_data*)handle;
	if (event == EVENT_PERIPH_IS_OFFLINE) {
		ALOGI("%s is now offline", data->device_name);
		pthread_mutex_unlock(&(data->shutdown_wait));
	}
	pm_client_event_acknowledge(data->id, event);
}
int subsystem_control_shutdown(unsigned proc_num, char *name)
{
    int rc;
    struct ssc_req req;
    struct ssc_resp resp;
    int count = 0;
    enum pm_event state;
    int per_mgr_supported = 0;
    int per_shutdown = 0;
    struct ssctl_data data;

    ALOGI("In subsystem_control_shutdown");
    pthread_mutex_init(&(data.shutdown_wait), NULL);
    if (name) {
            pthread_mutex_lock(&(data.shutdown_wait));
            strlcpy(data.device_name, name, sizeof(data.device_name));
            rc= pm_client_register(ssctl_shutdown_cb,
                            &data, name,
                            "oem-services",
                            &state,
                            &data.id);
            if (rc == PM_RET_FAILED) {
                    ALOGE("Failed to register with peripheral-manager");
                    goto error;
            } else if (rc == PM_RET_SUCCESS) {
                    ALOGI("Peripheral manager supported");
                    per_mgr_supported = 1;
                    if (state == EVENT_PERIPH_IS_OFFLINE) {
                            //Modem is already offline. No need to wait
                            pthread_mutex_unlock(&(data.shutdown_wait));
                   }
            } else {
                    ALOGI("Using legacy shutdown path");
                    pthread_mutex_unlock(&(data.shutdown_wait));
            }
    }
    if (per_mgr_supported) {
            //If peripheral manager supported then we wait for the callback
            //function to get invoked and free the mutex. This invocation
            //would happen when all modem clients disconnect from peripheral
            //manager and the modem shuts down.
            do {
                    rc = pthread_mutex_trylock(&(data.shutdown_wait));
                    if (!rc) {
                            per_shutdown = 1;
                            break;
                    }
                    usleep(SHUTDOWN_CHECK_RETRY_US);
                    count++;
            } while (count < MAX_SHUTDOWN_CHECK_COUNT);
            if (per_shutdown) {
                    ALOGI("%s shutdown successful", name);
                    return RET_SUCCESS;
            }
            ALOGE("%s did not shutdown within timeout", name);
            goto error;
    }
    if((proc_num < PROC_START) || (proc_num > PROC_END))
    {
        LOGE("PROC ID[%u] is invalid.", proc_num);
        return -EINVAL;
    }
    if (proc_num == PROC_MDM) {
            if (!name) {
                    ALOGE("Invalid name passed to subsystem_control_shutdown");
                    return RET_FAILED;
            }
            ALOGI("Shutting down %s", name);
	    do {
                   if (get_peripheral_state(name) != PER_STATE_OFFLINE)
                           usleep(SHUTDOWN_CHECK_RETRY_US);
                   else {
                           ALOGI("%s shutdown successful", name);
                           return RET_SUCCESS;
                   }
                   count++;
            } while(count < MAX_SHUTDOWN_CHECK_COUNT);
    } else {
            if (!__sync_bool_compare_and_swap(&ssc_in_progress, 0, 1))
            {
                    ALOGI("Shutdown request already in progress");
                    return -EINVAL;
            }
            req.proc_num = proc_num;
            req.cmd = CMD_SHUTDOWN;
            resp.rc = -1;
            rc = ssc_process_command(&req, &resp);
            __sync_bool_compare_and_swap(&ssc_in_progress, 1, 0);
            if(rc)
            {
                    ALOGE("Failed to process shutdown command");
                    return rc;
            }
            return resp.rc;
    }
error:
    if (name)
        ALOGE("Failed to shutdown %s", name);
    return RET_FAILED;
}
