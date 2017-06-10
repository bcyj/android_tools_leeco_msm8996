/*
 *qcom-system-daemon: A module to handle commands(diag) that can be processed
 * in native code.
 *
 *Copyright (C) 2013-2014 Qualcomm Technologies, Inc. All rights reserved.
 *        Qualcomm Technologies Proprietary and Confidential.
 *
 *qcom_system_daemon.c : Main implementation of qcom-system-daemon
 */
#include "qcom_system_daemon.h"
#include <poll.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

PACK(void *) ffbm_diag_reset_handler(PACK(void *)req_pkt, uint16 pkt_len);
PACK(void *) ffbm_diag_mode_handler(PACK(void *)req_pkt, uint16 pkt_len);
PACK(void *) reboot_to_edl(PACK(void *)req_pkt, uint16 pkt_len);
PACK(void *) diagpkt_version_handler(PACK(void*) req_ptr, uint16 pkt_len);
PACK(void *) qdss_diag_pkt_handler(PACK(void *) pReq, uint16 pkt_len);
static qmi_client_error_type pdc_clnt_init();
static int init_regional_pack_conf(void);

static int get_mode( ffbm_get_mode_rsq_type *pkt);
/*Table for commands handled only in ffbm mode*/
static const diagpkt_user_table_entry_type ftm_ffbm_mode_table[] =
{
	{DIAG_CONTROL_F, DIAG_CONTROL_F, ffbm_diag_reset_handler}
};

/*Table for commands handled in ffbm and normal mode*/
static const diagpkt_user_table_entry_type ftm_table[] =
{
	{FTM_FFBM_CMD_CODE, FTM_FFBM_CMD_CODE, ffbm_diag_mode_handler}
};

static const diagpkt_user_table_entry_type system_operations_table[] =
{
	{EDL_RESET_CMD_CODE, EDL_RESET_CMD_CODE, reboot_to_edl}
};

/* Table for image version command */
static const diagpkt_user_table_entry_type diagpkt_tbl[] =
{
   { VERSION_DIAGPKT_PREFIX, VERSION_DIAGPKT_PREFIX, diagpkt_version_handler }
};

/*qdss dispatch table*/
static const diagpkt_user_table_entry_type qdss_diag_pkt_tbl[] =
{
	{QDSS_DIAG_PROC_ID | QDSS_QUERY_STATUS,
	QDSS_DIAG_PROC_ID | QDSS_FILTER_HWEVENT_CONFIGURE,
	qdss_diag_pkt_handler}
};

typedef PACK(struct)
{
	uint8 command_code;
	uint8 subsys_id;
	uint16 subsys_cmd_code;
	uint32 status;
	uint16 delayed_rsp_id;
	uint16 rsp_cnt;         /* 0, means one response and 1, means two responses */
} diagpkt_version_subsys_hdr_v2_type;

typedef PACK(struct)
{
	diagpkt_version_subsys_hdr_v2_type hdr;
	uint16 version_data_len;
	unsigned char version_data[VERSION_TABLE_S];
} diagpkt_version_delayed_rsp;

static int fd_select_image = -1;
static int fd_image_version = -1;
static int fd_image_variant = -1;
static int fd_image_crm_version = -1;
static char qdss_sink = 0, qdss_hwevent = 0, qdss_stm = 0;
static volatile boolean set_selected_ind_flag=FALSE;
static volatile boolean set_load_ind_flag=FALSE;
static volatile boolean set_activate_flag=FALSE;
static int32_t idl_maj_version=1;
static int32_t idl_min_version=8;
static int32_t library_version=6;
static qmi_idl_service_object_type pdc_service_object;
static pdc_config_info_type pdc_config_info;
static qmi_client_type clnt;
static boolean pdc_client_init_flag=FALSE;
pthread_mutex_t set_flag_mutex;
int qcom_system_daemon_pipe[2];

static boot_selection_info bootselect_info;

#define REGIONAL_PACK_STRING_MAX_SIZE 128
#define REGIONAL_PACK_SETTING_CMD_MAX_SIZE 256
#define REGIONAL_PACK_PREFIX "strSpec="
#define CURRENT_REGION_PACK_SETTING_FILE "/system/vendor/speccfg/spec"
#define REGIONAL_PACK_CONF_XML "/system/vendor/speccfg/carrier_regional_pack_conf.xml"
#define ERR_UNSPEC -1
#define ERR_NOMEM -2
#define RET_OK 0
#define REGIONAL_PACK_STRING_MAX_SIZE 128
#define REGION_PACK_ID_MAX 128

int regional_pack_nodes_n = 0;
/* Configuration file doc handler.	Won't be changed after initialization. */
xmlDocPtr pconf = NULL;

/* Configuration file xml root handler.  Won't be changed after initialization. */
xmlNodePtr pconf_root = NULL;

struct regional_pack_node {
	unsigned int regional_pack_id;
	char * regional_pack_setting;
	char * mbn_path;
	char * regional_pack_cmd_string;
};
typedef struct regional_pack_node regional_pack_node_t;

regional_pack_node_t * regional_pack_nodes;

#define set_mbn_flags(var, value, mutex) \
	pthread_mutex_lock(&mutex);	\
	var=value; \
	pthread_mutex_unlock(&mutex);

void pdc_set_selected_config_ind(void *ind_data_ptr,uint32 ind_data_len)
{
	pdc_set_selected_config_ind_msg_v01 *selected_ind;
	ALOGI("begin qmi_pdc_set_selected_config_ind");
	if (ind_data_ptr != NULL) {
	    selected_ind = (pdc_set_selected_config_ind_msg_v01*)ind_data_ptr;
	    if ( selected_ind->error == QMI_ERR_NONE_V01 ) {
	        set_mbn_flags(set_activate_flag,TRUE,set_flag_mutex);
	    } else {
	        ALOGE("qmi_pdc_set_selected_config_ind error \n");
	    }
	}
}

qmi_client_error_type pdc_set_selected_config_req(qmi_client_type *clnt, const char *carrier_name)
{
	pdc_set_selected_config_req_msg_v01 qmi_request;
	pdc_set_selected_config_resp_msg_v01 qmi_response;
	pdc_config_info_req_type_v01 *p_config_info;
	qmi_client_error_type rc;
	set_mbn_flags(set_selected_ind_flag,FALSE,set_flag_mutex);
	rc = pdc_clnt_init();
	if (rc != QMI_NO_ERR) {
	    ALOGE("pdc_clnt_init failed with rc=%d", rc);
	    return rc;
	}
	memset( &qmi_request, 0, sizeof(qmi_request) );
	p_config_info = &qmi_request.new_config_info;
	p_config_info->config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
	strlcpy(p_config_info->config_id, carrier_name, PDC_CONFIG_ID_SIZE_MAX_V01);
	p_config_info->config_id_len = strlen(p_config_info->config_id);
	ALOGI("qmi_pdc_set_selected_config_req p_config_info->config_id=%s p_config_info->config_id_len=%d", p_config_info->config_id,p_config_info->config_id_len);
	rc = qmi_client_send_msg_sync( *clnt,
                               QMI_PDC_SET_SELECTED_CONFIG_REQ_V01,
                               &qmi_request,
                               sizeof( qmi_request ),
                               &qmi_response,
                               sizeof( qmi_response ),
                               QMI_PDC_SEND_MSG_TIME_OUT
                               );
	if ( rc != QMI_NO_ERR ) {
	    ALOGE("pdc_set_selected_config_req: qmi_client_send_msg_sync error: %d\n",rc);
	}
	return rc;
error:
	ALOGI("qmi_pdc_set_selected_config_req failed with rc=%d",rc);
	return rc;
}

qmi_client_error_type pdc_load_config_req(qmi_client_type *clnt)
{
	pdc_load_config_req_msg_v01 qmi_request;
	pdc_load_config_resp_msg_v01 qmi_response;
	pdc_load_config_info_type_v01 *p_load_info;
	qmi_client_error_type rc;
	int rd_len;
	ALOGI("begin pdc_load_config_req");
	set_mbn_flags(set_load_ind_flag,FALSE,set_flag_mutex);
	memset( &qmi_request, 0, sizeof(qmi_request) );
	p_load_info = &qmi_request.load_config_info;
	p_load_info->config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
	p_load_info->total_config_size = pdc_config_info.conf_size;
	strlcpy(p_load_info->config_id, pdc_config_info.config_id, PDC_CONFIG_ID_SIZE_MAX_V01);
	p_load_info->config_id_len = strlen(p_load_info->config_id);

	if (pdc_config_info.conf_fd == -1) {
	    rd_len = -1;
	} else {
	    do{
	        rd_len = read( pdc_config_info.conf_fd,
	        p_load_info->config_frame, CONFIG_FRAME_SIZE );
	     } while ( ( rd_len == -1 ) && ( errno == EINTR ) );
	}

	if (rd_len == -1) {
	    ALOGI("pdc_load_config_req read failed\n");
	} else {
	    p_load_info->config_frame_len = rd_len;
	    pdc_config_info.load_size += rd_len;
	    ALOGI("pdc_load_config_req: pdc_config_info.load_size =%d\n",pdc_config_info.load_size);
	    rc = qmi_client_send_msg_sync(*clnt,
                                        QMI_PDC_LOAD_CONFIG_REQ_V01,
                                        &qmi_request,
                                        sizeof( qmi_request ),
                                        &qmi_response,
                                        sizeof( qmi_response ),
                                        QMI_PDC_SEND_MSG_TIME_OUT
                                        );
	    if ( rc != QMI_NO_ERR ) {
	        ALOGE("pdc_load_config_req: qmi_client_send_msg_sync error: %d\n",rc);
	    }
	    if ( pdc_config_info.load_size >= pdc_config_info.conf_size ) {
	         close( pdc_config_info.conf_fd );
	         pdc_config_info.conf_fd = -1;
	    }
	}
	return rc;
}

void pdc_load_config_ind(void *ind_data_ptr,uint32 ind_data_len)
{
	pdc_load_config_ind_msg_v01 *load_ind;

	ALOGI("begin pdc_load_config_ind");
	if ( ind_data_ptr != NULL ) {
	    load_ind = (pdc_load_config_ind_msg_v01*)ind_data_ptr;
	    if ( load_ind->error == QMI_ERR_NONE_V01 ) {
	        if ( load_ind->remaining_config_size_valid ) {
	            if ( load_ind->remaining_config_size == 0 ) {
	                set_mbn_flags(set_selected_ind_flag,TRUE,set_flag_mutex);
	            } else {
	                ALOGI("pdc_load_config_ind: the remain config size is %d\n", load_ind->remaining_config_size);
	                set_mbn_flags(set_load_ind_flag,TRUE,set_flag_mutex);
	            }
	        } else {
	            if ( pdc_config_info.load_size >= pdc_config_info.conf_size ) {
	                set_mbn_flags(set_selected_ind_flag,TRUE,set_flag_mutex);
	            } else {
	                ALOGI("pdc_load_config_ind: the remain config size is %d\n", load_ind->remaining_config_size);
	                set_mbn_flags(set_load_ind_flag,TRUE,set_flag_mutex);
	            }
	        }
	    } else {
	         ALOGE("Failed to load configuration");
	    }
	}
}

qmi_client_error_type pdc_activate_config_req(qmi_client_type *clnt)
{
	pdc_activate_config_req_msg_v01 qmi_request;
	pdc_activate_config_resp_msg_v01 qmi_response;
	qmi_client_error_type rc;
	ALOGI("begin qmi_activate_config_req");
	rc = pdc_clnt_init();
	if ( rc != QMI_NO_ERR ) {
	    ALOGE("pdc_clnt_init failed with rc=%d", rc);
	    return rc;
	}
	memset( &qmi_request, 0, sizeof(qmi_request) );
	memset( &qmi_response, 0, sizeof(qmi_response) );
	qmi_request.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;

	rc = qmi_client_send_msg_sync( *clnt,QMI_PDC_ACTIVATE_CONFIG_REQ_V01, &qmi_request,sizeof( qmi_request ),
                                      &qmi_response,sizeof( qmi_response ),QMI_PDC_SEND_MSG_TIME_OUT);
	if ( rc != QMI_NO_ERR ) {
	    ALOGE("qmi_pdc_activate_config_request: qmi_client_send_msg_sync error: %d\n",rc);
	}
	return rc;
}


void pdc_activate_config_ind(void *ind_data_ptr,uint32 ind_data_len)
{
	pdc_activate_config_ind_msg_v01 *act_ind;
	ALOGI("begin qmi_activate_config_ind");
	if ( ind_data_ptr != NULL ) {
	    act_ind = (pdc_activate_config_ind_msg_v01*)ind_data_ptr;
	    if ( act_ind->error == QMI_ERR_NONE_V01 ) {
	         ALOGI("qmi_pdc_activate_config_ind successfuly");
	    } else {
	         ALOGE("qmi_pdc_activate_config_ind failed, act_ind->error =%d", act_ind->error);
	    }
	}
}

void pdc_ind_cb( qmi_client_type  user_handle,
                    unsigned long  msg_id,
                    unsigned char  *ind_buf,
                    int ind_buf_len,
                    void *ind_cb_data
)
{
	uint32_t decoded_payload_len = 0;
	qmi_client_error_type qmi_err;
	void* decoded_payload = NULL;
	ALOGI("begin qmi_pdc_ind_cb");
	qmi_err = qmi_idl_get_message_c_struct_len(pdc_service_object,
                                                QMI_IDL_INDICATION,
                                                msg_id,
                                                &decoded_payload_len);
	if ( qmi_err == QMI_NO_ERR ) {
	    decoded_payload = malloc( decoded_payload_len );
	    if ( NULL != decoded_payload ) {
	        qmi_err = qmi_client_message_decode(user_handle,
                                          QMI_IDL_INDICATION,
                                          msg_id,
                                          ind_buf,
                                          ind_buf_len,
                                          decoded_payload,
                                          (int)decoded_payload_len);
        if ( QMI_NO_ERR == qmi_err ) {
	        switch(msg_id){
	            case QMI_PDC_LOAD_CONFIG_IND_V01:
	                pdc_load_config_ind(decoded_payload, decoded_payload_len);
	                break;
	            case QMI_PDC_ACTIVATE_CONFIG_IND_V01:
	                pdc_activate_config_ind(decoded_payload, decoded_payload_len);
	                break;
	            case QMI_PDC_SET_SELECTED_CONFIG_IND_V01:
	                pdc_set_selected_config_ind(decoded_payload, decoded_payload_len);
	                break;
	            default:
	                ALOGI("Unsupported QMI PDC indication %x hex", msg_id);
	                break;
	        }
	    } else {
	             ALOGE("Indication decode failed for msg %d with error %d", msg_id,qmi_err );
	    }
	    free(decoded_payload);
	   }
	}
}

static qmi_client_error_type pdc_clnt_init()
{
	int rc=QMI_NO_ERR;
	uint32_t num_services, num_entries = 0;
	qmi_client_type notifier;
	qmi_cci_os_signal_type os_params;
	qmi_service_info info[10];
	if (pdc_client_init_flag == FALSE) {
	    /* Get the service object for the oem qmi API */
	    pthread_mutex_init(&set_flag_mutex, NULL);
	    pdc_service_object = pdc_get_service_object_internal_v01(idl_maj_version, idl_min_version, library_version);
	    if (!pdc_service_object) {
	        ALOGE("pdc_clnt_init: pdc_get_service_object_internal_v01 failed, verify .h and .c match.\n");
	        rc = QMI_SERVICE_ERR;
	        goto error;
	    }
	    rc = qmi_client_notifier_init(pdc_service_object, &os_params, &notifier);

	    /* Check if the service is up, if not wait on a signal */
	    while(1)
	    {
	        rc = qmi_client_get_service_list( pdc_service_object, NULL, NULL, &num_services);
	        ALOGI("pdc_clnt_init: qmi_client_get_service_list() returned %d num_services = %d\n", rc, num_services);
	        if(rc == QMI_NO_ERR)
	            break;
	        /* wait for server to come up */
	        QMI_CCI_OS_SIGNAL_WAIT(&os_params, 0);
	    };
	    num_entries = num_services;
	    /* The server has come up, store the information in info variable */
	    rc = qmi_client_get_service_list(pdc_service_object,
	             info,&num_entries,
	             &num_services);
	    if ( rc != QMI_NO_ERR ) {
	        ALOGI("pdc_clnt_init: qmi_client_get_service_list rc=%d", rc);
	        return rc;
	    }
	    rc = qmi_client_init(&info[0], pdc_service_object, pdc_ind_cb, NULL, &os_params, &clnt);
	    if( rc != QMI_NO_ERR ) {
	        ALOGI("pdc_clnt_init: qmi_client_init returned %d\n", rc);
	    }
	}
	return rc;
error:
	ALOGE("pdc_clnt_init fail");
	return rc;
}

static pdc_err_status pdc_load_mbn(const char *mbn_file_path, const char *carrier_name)
{
	char full_mbn_path[MBN_PATH_MAX_SIZE];
	struct stat f_stat;
	int rc;
	set_mbn_flags(set_selected_ind_flag,FALSE,set_flag_mutex);
	rc = pdc_clnt_init();
	if( rc != QMI_NO_ERR ){
	    ALOGE("pdc_clnt_init failed with rc=%d", rc);
	    return rc;
	}
	snprintf(full_mbn_path,MBN_PATH_MAX_SIZE, "%s/%s/mcfg_sw.mbn",mbn_file_path,carrier_name);
	pdc_config_info.conf_fd = open(full_mbn_path,O_RDONLY);
	if (pdc_config_info.conf_fd == -1) {
	    ALOGE("pdc_load_mbn:failed to open mbn file, %s", strerror(errno));
	    return PDC_ERROR;
	}
	if ( fstat( pdc_config_info.conf_fd, &f_stat ) == -1 ) {
	    ALOGE("pdc_load_mbn: Failed to fstat file: %s", strerror(errno));
	    close(pdc_config_info.conf_fd);
	    return PDC_ERROR;
	}
	pdc_config_info.conf_size = f_stat.st_size;
	strlcpy(pdc_config_info.config_id, carrier_name, PDC_CONFIG_ID_SIZE_MAX_V01);
	rc = pdc_load_config_req(&clnt);
	if (rc != QMI_NO_ERR)
	    goto error;
	do{
	    if (set_load_ind_flag == TRUE) {
	        rc = pdc_load_config_req(&clnt);
	        if  (rc != QMI_NO_ERR)
	            goto error;
	    }
	}while(set_selected_ind_flag != TRUE);
	set_selected_ind_flag = TRUE;
	memset(&pdc_config_info,0,sizeof(pdc_config_info_type));
	qmi_client_release(&clnt);
	pdc_client_init_flag==FALSE;
	return rc;
error:
	ALOGI("pdc_load_mbn: pdc_load_config_req rc=%d", rc);
	if (pdc_config_info.conf_fd != -1)
	     close(pdc_config_info.conf_fd);
	return PDC_ERROR;
}

static pdc_err_status pdc_load_select_activate_mbn(const char *mbn_file_path, const char *carrier_name)
{
	char full_mbn_path[MBN_PATH_MAX_SIZE];
	struct stat f_stat;
	int rc;

	if ( pdc_client_init_flag == FALSE ) {
	    rc = pdc_clnt_init();
	    if( rc != QMI_NO_ERR ){
	        ALOGE("pdc_load_select_activate_mbn: pdc_clnt_init failed with rc=%d", rc);
	        return PDC_ERROR;
	    }
	    pdc_client_init_flag = TRUE;
	}
	snprintf(full_mbn_path,MBN_PATH_MAX_SIZE, "%s/%s/mcfg_sw.mbn",mbn_file_path,carrier_name);
	pdc_config_info.conf_fd = open(full_mbn_path,O_RDONLY);
	if( pdc_config_info.conf_fd == -1 ) {
	    ALOGE("pdc_load_select_activate_mbn:failed to open mbn file, %s", strerror(errno));
	    return PDC_ERROR;
	}
	if ( fstat( pdc_config_info.conf_fd, &f_stat ) == -1 ){
	    ALOGE("pdc_load_select_activate_mbn: Failed to fstat file: %s", strerror(errno));
	    close(pdc_config_info.conf_fd);
	    return PDC_ERROR;
	}
	pdc_config_info.conf_size = f_stat.st_size;
	strlcpy(pdc_config_info.config_id, carrier_name, PDC_CONFIG_ID_SIZE_MAX_V01);
	rc = pdc_load_config_req(&clnt);
	if( rc != QMI_NO_ERR )
	    goto error;
	do{
	     if(set_load_ind_flag == TRUE) {
	        rc = pdc_load_config_req(&clnt);
	        if(rc != QMI_NO_ERR)
	            goto error;
	    }
	}while(set_selected_ind_flag != TRUE);
	rc = pdc_set_selected_config_req(&clnt,carrier_name);
	if(rc != QMI_NO_ERR)
	    goto error;
	//wait for select to complete
	while(set_activate_flag != TRUE);
	rc = pdc_activate_config_req(&clnt);
	if (rc != QMI_NO_ERR)
	    goto error;
	return rc;
error:
	ALOGI("pdc_load_mbn failed rc = %d", rc);
	if(pdc_config_info.conf_fd != -1)
	    close(pdc_config_info.conf_fd);
	return PDC_ERROR;
}



void* pdc_clnt_handler(ffbm_set_mbn_req_type *ffbm_pkt)
{
	ffbm_set_mbn_rsp_type *rsp;
	uint index;
	int rc;
	rsp = (ffbm_set_mbn_rsp_type*)diagpkt_subsys_alloc( DIAG_SUBSYS_FTM,
	        FTM_FFBM_CMD_CODE,
	        sizeof(ffbm_set_mbn_rsp_type));
	if (!rsp) {
	        ALOGE("Failed to allocate response packet");
	        return rsp;
	}
	if (RET_OK != init_regional_pack_conf()) {
	        ALOGE("read config xml failed");
	        goto error;
	}
	if (ffbm_pkt->regional_pack_id > REGION_PACK_ID_MAX)
	    goto error;
	index = ffbm_pkt->regional_pack_id;
	ALOGI("Setting regional pack to %s", regional_pack_nodes[index].regional_pack_setting);
	if( ffbm_pkt->mbn_operation_type == PDC_CONFIG_LOAD ) {
	    rc = pdc_load_mbn(regional_pack_nodes[0].mbn_path, regional_pack_nodes[index].regional_pack_setting);
	    if(rc != PDC_NO_ERROR)
	        goto error;
	} else if (ffbm_pkt->mbn_operation_type == PDC_CONFIG_SET_SELECT) {
	    rc=pdc_set_selected_config_req(&clnt,regional_pack_nodes[index].regional_pack_setting);
	    if (rc != QMI_NO_ERR)
	        goto error;
	} else if (ffbm_pkt->mbn_operation_type == PDC_CONFIG_ACTIVATE) {
	    rc = pdc_activate_config_req(&clnt);
	    if (rc != QMI_NO_ERR)
	        goto error;
	} else if (ffbm_pkt->mbn_operation_type == PDC_CONFIG_LOAD_SELECT_ACTIVATE) {
	    if (RET_OK != init_regional_pack_conf())
	    {
	     ALOGE("read config xml failed");
	     goto error;
	    }
	    if (ffbm_pkt->regional_pack_id > REGION_PACK_ID_MAX)
	      goto error;
	    index = ffbm_pkt->regional_pack_id;
	    ALOGI("Setting regional pack to %s, mbn path is %s", regional_pack_nodes[index].regional_pack_setting, regional_pack_nodes[index].mbn_path);
	    rc = pdc_load_select_activate_mbn(regional_pack_nodes[0].mbn_path, regional_pack_nodes[index].regional_pack_setting);
	    if(rc != QMI_NO_ERR)
	        goto error;
	}else
	    ALOGI("No PDC operation specific");
	rsp->iFTM_Error_Code = FTM_FFBM_SUCCESS;
	return (void*)rsp;
error:
    rsp->iFTM_Error_Code = FTM_FFBM_FAIL;
    return (void*)rsp;
}

static int qdss_file_write_str(const char *qdss_file_path, const char *str)
{
	int qdss_fd, ret;

	if (!qdss_file_path || !str) {
		return QDSS_RSP_FAIL;
	}

	qdss_fd = open(qdss_file_path, O_WRONLY);
	if (qdss_fd < 0) {
		ALOGE("qdss open file: %s error: %s", qdss_file_path, strerror(errno));
		return QDSS_RSP_FAIL;
	}

	ret = write(qdss_fd, str, strlen(str));
	if (ret < 0) {
		ALOGE("qdss write file: %s error: %s", qdss_file_path, strerror(errno));
		close(qdss_fd);
		return QDSS_RSP_FAIL;
	}

	close(qdss_fd);

	return QDSS_RSP_SUCCESS;
}

static int qdss_file_write_byte(const char *qdss_file_path, unsigned char val)
{
	int qdss_fd, ret;

	if (!qdss_file_path) {
		return QDSS_RSP_FAIL;
	}

	qdss_fd = open(qdss_file_path, O_WRONLY);
	if (qdss_fd < 0) {
		ALOGE("qdss open file: %s error: %s", qdss_file_path, strerror(errno));
		return QDSS_RSP_FAIL;
	}

	ret = write(qdss_fd, &val, 1);
	if (ret < 0) {
		ALOGE("qdss write file: %s error: %s", qdss_file_path, strerror(errno));
		close(qdss_fd);
		return QDSS_RSP_FAIL;
	}

	close(qdss_fd);

	return QDSS_RSP_SUCCESS;
}

/* Sets the Trace Sink */
static int qdss_trace_sink_handler(qdss_trace_sink_req *pReq, int req_len, qdss_trace_sink_rsp *pRsp, int rsp_len)
{
	int ret = 0;

	if (!pReq || !pRsp) {
		return QDSS_RSP_FAIL;
	}

	if (pReq->trace_sink == TMC_TRACESINK_ETB) {
		/* For enabling writing ASCII value of 1 i.e. 0x31 */
		ret = qdss_file_write_byte(QDSS_ETB_SINK_FILE, 0x31);
		if (ret) {
			return QDSS_RSP_FAIL;
		}
	} else if (pReq->trace_sink == TMC_TRACESINK_RAM) {
		ret = qdss_file_write_str(QDSS_ETR_OUTMODE_FILE, "mem");
		if (ret) {
			return QDSS_RSP_FAIL;
		}

		ret = qdss_file_write_byte(QDSS_ETR_SINK_FILE, 0x31);
		if (ret) {
			return QDSS_RSP_FAIL;
		}
	} else if (pReq->trace_sink == TMC_TRACESINK_USB) {
		ret = qdss_file_write_str(QDSS_ETR_OUTMODE_FILE, "usb");
		if (ret) {
			return QDSS_RSP_FAIL;
		}

		ret = qdss_file_write_byte(QDSS_ETR_SINK_FILE, 0x31);
		if (ret) {
			return QDSS_RSP_FAIL;
		}
	} else if (pReq->trace_sink == TMC_TRACESINK_TPIU) {
		ret = qdss_file_write_byte(QDSS_TPIU_SINK_FILE, 0x31);
		if (ret) {
			return QDSS_RSP_FAIL;
		}

		ret = qdss_file_write_str(QDSS_TPIU_OUTMODE_FILE, "mictor");
		if (ret) {
			return QDSS_RSP_FAIL;
		}
	} else if (pReq->trace_sink == TMC_TRACESINK_SD) {
		ret = qdss_file_write_byte(QDSS_TPIU_SINK_FILE, 0x31);
		if (ret) {
			return QDSS_RSP_FAIL;
		}

		ret = qdss_file_write_str(QDSS_TPIU_OUTMODE_FILE, "sdc");
		if (ret) {
			return QDSS_RSP_FAIL;
		}
	} else {
		qdss_sink = 0;
		return QDSS_RSP_FAIL;
	}

	qdss_sink = pReq->trace_sink;

	return QDSS_RSP_SUCCESS;
}

/* Enable/Disable STM */
static int qdss_filter_stm_handler(qdss_filter_stm_req *pReq, int req_len, qdss_filter_stm_rsp *pRsp, int rsp_len)
{
	char ret = 0, stm_state = 0;

	if (!pReq || !pRsp) {
		return QDSS_RSP_FAIL;
	}

	if (pReq->state) {
		stm_state = 1;
	} else {
		stm_state = 0;
	}

	ret = qdss_file_write_byte(QDSS_STM_FILE, stm_state + 0x30);
	if (ret) {
		return QDSS_RSP_FAIL;
	}

	qdss_stm = stm_state;

	return QDSS_RSP_SUCCESS;
}

/* Enable/Disable HW Events */
static int qdss_filter_hwevents_handler(qdss_filter_hwevents_req *pReq, int req_len, qdss_filter_hwevents_rsp *pRsp, int rsp_len)
{
	int ret = 0;

	if (!pReq || !pRsp) {
		return QDSS_RSP_FAIL;
	}

	if (pReq->state) {

		qdss_hwevent = 1;
		/* For disabling writing ASCII value of 0 i.e. 0x30 */
		ret = qdss_file_write_byte(QDSS_HWEVENT_FILE, 0x30);
		if (ret) {
			return QDSS_RSP_FAIL;
		}

		ret = qdss_file_write_byte(QDSS_STM_HWEVENT_FILE, 0x30);
		if (ret) {
			return QDSS_RSP_FAIL;
		}

		ret = qdss_file_write_byte(QDSS_HWEVENT_FILE, 0x31);
		if (ret) {
			return QDSS_RSP_FAIL;
		}

		ret = qdss_file_write_byte(QDSS_STM_HWEVENT_FILE, 0x31);
		if (ret) {
			return QDSS_RSP_FAIL;
		}

	} else {

		qdss_hwevent = 0;

		ret = qdss_file_write_byte(QDSS_HWEVENT_FILE, 0x30);
		if (ret) {
			return QDSS_RSP_FAIL;
		}

		ret = qdss_file_write_byte(QDSS_STM_HWEVENT_FILE, 0x30);
		if (ret) {
			return QDSS_RSP_FAIL;
		}
	}

	return QDSS_RSP_SUCCESS;
}

/* Programming registers to generate HW events */
static int qdss_filter_hwevents_configure_handler(qdss_filter_hwevents_configure_req *pReq, int req_len, qdss_filter_hwevents_configure_rsp *pRsp, int rsp_len)
{
	char reg_buf[100];
	int ret = 0, qdss_fd;

	if (!pReq || !pRsp) {
		return QDSS_RSP_FAIL;
	}

	snprintf(reg_buf, sizeof(reg_buf), "%x %x", pReq->register_addr, pReq->register_value);

	qdss_fd = open(QDSS_HWEVENT_SET_REG_FILE, O_WRONLY);
	if (qdss_fd < 0) {
		ALOGE("qdss open file: %s error: %s", QDSS_HWEVENT_SET_REG_FILE, strerror(errno));
		return QDSS_RSP_FAIL;
	}

	ret = write(qdss_fd, reg_buf, strlen(reg_buf));
	if (ret < 0) {
		ALOGE("qdss write file: %s error: %s", QDSS_HWEVENT_SET_REG_FILE, strerror(errno));
		close(qdss_fd);
		return QDSS_RSP_FAIL;
	}

	close(qdss_fd);

	return QDSS_RSP_SUCCESS;
}

/* Get the status of sink, stm and HW events */
static int qdss_query_status_handler(qdss_query_status_req *pReq, int req_len, qdss_query_status_rsp *pRsp, int rsp_len)
{

	if (!pReq || !pRsp) {
		return QDSS_RSP_FAIL;
	}

	pRsp->trace_sink = qdss_sink;
	pRsp->stm_enabled = qdss_stm;
	pRsp->hw_events_enabled = qdss_hwevent;

	return QDSS_RSP_SUCCESS;
}

/* QDSS commands handler */
PACK(void *) qdss_diag_pkt_handler(PACK(void *) pReq, uint16 pkt_len)
{
/*
 * 1) Checks the request command size. If it fails send error response.
 * 2) If request command size is valid then allocates response packet
 *    based on request.
 * 3) Invokes the respective command handler
 */

#define QDSS_HANDLE_DIAG_CMD(cmd)                              \
   if (pkt_len < sizeof(cmd##_req)) {                          \
      pRsp = diagpkt_err_rsp(DIAG_BAD_LEN_F, pReq, pkt_len);   \
   }                                                           \
   else {                                                      \
      pRsp =  diagpkt_subsys_alloc(DIAG_SUBSYS_QDSS,           \
                                   pHdr->subsysCmdCode,        \
                                   sizeof(cmd##_rsp));         \
      if (NULL != pRsp) {                                      \
         cmd##_handler((cmd##_req *)pReq,                      \
                       pkt_len,                                \
                       (cmd##_rsp *)pRsp,                      \
                       sizeof(cmd##_rsp));                     \
      }                                                        \
   }

	qdss_diag_pkt_hdr *pHdr;
	PACK(void *)pRsp = NULL;

	if (NULL != pReq) {
		pHdr = (qdss_diag_pkt_hdr *)pReq;

		switch (pHdr->subsysCmdCode & 0x0FF) {
		case QDSS_QUERY_STATUS:
			QDSS_HANDLE_DIAG_CMD(qdss_query_status);
			break;
		case QDSS_TRACE_SINK:
			 QDSS_HANDLE_DIAG_CMD(qdss_trace_sink);
			 break;
		case QDSS_FILTER_STM:
			 QDSS_HANDLE_DIAG_CMD(qdss_filter_stm);
			 break;
		case QDSS_FILTER_HWEVENT_ENABLE:
			 QDSS_HANDLE_DIAG_CMD(qdss_filter_hwevents);
			 break;
		case QDSS_FILTER_HWEVENT_CONFIGURE:
			 QDSS_HANDLE_DIAG_CMD(qdss_filter_hwevents_configure);
			 break;
		default:
			 pRsp = diagpkt_err_rsp(DIAG_BAD_CMD_F, pReq, pkt_len);
			 break;
		}

		if (NULL != pRsp) {
			diagpkt_commit(pRsp);
			pRsp = NULL;
		}
	}
	return (pRsp);
}

static int regional_pack_load_prop(xmlNodePtr cur_node, char *key, char **strvalue) {
	*strvalue = NULL;
	if (cur_node) {
	  *strvalue = (char*)xmlGetProp(cur_node, BAD_CAST key);
	}
	if (*strvalue == NULL) {
	  return (ERR_UNSPEC);
	} else {
	  return (RET_OK);
	}
}

static int regional_pack_load_prop_int(xmlNodePtr cur_node, char *key, int *value) {
	char * strvalue;
	if (regional_pack_load_prop(cur_node, key, &strvalue)) {
	   return (ERR_UNSPEC);
	}
	*value = atoi(strvalue);
	xmlFree(BAD_CAST strvalue);
	return (RET_OK);
}


static int regional_pack_load_prop_str(xmlNodePtr cur_node, char *key, char **value, int size) {
	char * strvalue;
	if (regional_pack_load_prop(cur_node, key, &strvalue)) {
	  return (ERR_UNSPEC);
	}
	if (size) {
	 /* Actually in if part, the value is type (char*) */
	  strlcpy(value, strvalue, size);
	  xmlFree(BAD_CAST strvalue);
	} else {
	   /*(char**) here*/
	  *value = strdup(strvalue);
	}
	  return (RET_OK);
}

static int regional_pack_node_init(int index, xmlNodePtr cur_node) {
	regional_pack_load_prop_int(cur_node, "regional_pack_id",
	  (int*)&regional_pack_nodes[index].regional_pack_id);
	regional_pack_load_prop_str(cur_node, "regional_pack_setting",
	  (int*)&regional_pack_nodes[index].regional_pack_setting,0);
	regional_pack_load_prop_str(cur_node, "regional_pack_cmd_string",
	  (int*)&regional_pack_nodes[index].regional_pack_cmd_string,0);
	return (RET_OK);
}

static int load_regional_pack_conf(char *conf_file, xmlDocPtr *ppconf, xmlNodePtr *ppconf_root) {
	xmlKeepBlanksDefault(0);
	*ppconf = xmlReadFile((const char*)conf_file, "UTF-8", XML_PARSE_RECOVER);
	if (!(*ppconf)) {
	  ALOGE("Can't open xml file (%s). err is %s", conf_file, strerror(errno));
	  return (ERR_UNSPEC);
	}
	*ppconf_root = xmlDocGetRootElement (*ppconf);
	if (!(*ppconf_root)) {
	  xmlFreeDoc(*ppconf);
	  xmlCleanupParser();
	  ALOGE("Can't read root node. err is %s", strerror(errno));
	  return (ERR_UNSPEC);
	}
	return (RET_OK);
}

static void regional_pack_print_regional_pack_node(regional_pack_node_t *pregional_pack_node) {
	ALOGI("regional_pack_id is %d , regional_pack_setting is %s, regional_pack_cmd_string is %s \n",
		pregional_pack_node->regional_pack_id,
		pregional_pack_node->regional_pack_setting,
		pregional_pack_node->regional_pack_cmd_string);
}

static int init_regional_pack_conf(void) {
	int i, err = 0;
	char *ver, *ver_sys;
	size_t nodes_size = 0;
	xmlNodePtr pcur_node = NULL, pconf_sys_root = NULL;

	if (access(REGIONAL_PACK_CONF_XML, F_OK)) {
		  err = 1;
		  goto load_etc;
	}
	if(load_regional_pack_conf(REGIONAL_PACK_CONF_XML, &pconf, &pconf_root) != RET_OK) {
		  err = 1;
		  goto load_etc;
	}
	ver_sys = (char *)xmlGetProp(pconf_sys_root, BAD_CAST "version");
	if (ver_sys) {
		ver = (char*)xmlGetProp(pconf_root, BAD_CAST "version");
		if (ver == NULL || strcmp(ver, ver_sys) != 0) {
			if (ver != NULL) {
				ALOGE("Configuration file version desn't match.  System requirement %s, current %s", ver_sys, ver);
				xmlFree(BAD_CAST ver);
			}
			xmlFree(BAD_CAST ver_sys);
			err = 1;
			goto load_etc;
		}
		xmlFree(BAD_CAST ver);
	}
	regional_pack_nodes_n = xmlChildElementCount(pconf_root);
	if (regional_pack_nodes_n == 0) {
		ALOGE("No log entry in log configuration file. Revert to default configuration file.");
		xmlFreeDoc(pconf);
		xmlCleanupParser();
		err = 1;
		goto load_etc;
	}
load_etc:
	if (err) {
		if(load_regional_pack_conf(REGIONAL_PACK_CONF_XML, &pconf, &pconf_root) != RET_OK) {
			return (ERR_UNSPEC);
		}

		regional_pack_nodes_n = xmlChildElementCount(pconf_root);
		if (regional_pack_nodes_n == 0) {
			ALOGE("No log entry in log configuration file.");
			return (ERR_UNSPEC);
		}
	}
	nodes_size = regional_pack_nodes_n * sizeof(regional_pack_node_t);
	ALOGE("regional_pack_nodes_n is %d \n", regional_pack_nodes_n);
	regional_pack_nodes = malloc(nodes_size);
	if (regional_pack_nodes == NULL) {
		ALOGE("Can't allocate memory for regional_pack_nodes. err:%s", strerror(errno));
		exit(ERR_UNSPEC);
	}
	memset(regional_pack_nodes, 0, nodes_size);
	pcur_node = pconf_root->xmlChildrenNode;;
	regional_pack_load_prop_str(pcur_node, "mbn_path",(int*)&regional_pack_nodes[0].mbn_path,0);
	pcur_node = pcur_node->next;
	for (i = 1; i < regional_pack_nodes_n; i++) {
		if(regional_pack_node_init(i, pcur_node) != RET_OK) {
		  ALOGE("regional_pack_node_init failed \n");
		  i--;
		  regional_pack_nodes_n--;
		  pcur_node = pcur_node->next;
		  continue;
		}
		pcur_node = pcur_node->next;
	}
	return (RET_OK);
}

static int get_current_regional_pack_id(ffbm_get_region_pack_rsp_type* rsp)
{
	char buf[REGIONAL_PACK_STRING_MAX_SIZE];
	int readSize,i=0,prefixlen=strlen(REGIONAL_PACK_PREFIX);
	int fd = open(CURRENT_REGION_PACK_SETTING_FILE,O_RDONLY);
	if (fd == -1) {
		ALOGE("cannot open regional setting file");
		return -1;
	}

	memset(buf,0,REGIONAL_PACK_STRING_MAX_SIZE);
	readSize = read(fd,buf,REGIONAL_PACK_STRING_MAX_SIZE);
	close(fd);
	if (readSize <= 0) {
		ALOGE("read regional setting file failed");
		return -1;
	}
	if (readSize < REGIONAL_PACK_STRING_MAX_SIZE)
		buf[readSize] = 0;
	else
		buf[REGIONAL_PACK_STRING_MAX_SIZE-1] = 0;
	if (strncmp(buf,REGIONAL_PACK_PREFIX,prefixlen) != 0) {
		ALOGE("spec file header error");
	}
	while(1) {
		if (regional_pack_nodes[i].regional_pack_id==REGION_PACK_ID_MAX) {
			ALOGE("not found regional pack id");
			break;
		}
		if (strncmp(regional_pack_nodes[i].regional_pack_setting,
			buf+prefixlen,strlen(regional_pack_nodes[i].regional_pack_setting)) == 0)
		{
			rsp->current_regional_pack_id = regional_pack_nodes[i].regional_pack_id;
			ALOGI("current setting %s, regional_pack_id=%d",regional_pack_nodes[i].regional_pack_setting,regional_pack_nodes[i].regional_pack_id);
			return 0;
		}
		i++;
	}
	return -1;
}

void *ffbm_set_regional_pack(ffbm_set_region_pack_req_type *ffbm_pkt)
{
	char cmd[REGIONAL_PACK_STRING_MAX_SIZE];
	ffbm_set_region_pack_rsp_type *rsp;
	ffbm_get_mode_rsq_type *get_mode_rsp;
	uint index;
	rsp = (ffbm_set_region_pack_rsp_type*)diagpkt_subsys_alloc( DIAG_SUBSYS_FTM,
			FTM_FFBM_CMD_CODE,
			sizeof(ffbm_set_region_pack_rsp_type));
	if (!rsp) {
		ALOGE("Failed to allocate response packet");
		return rsp;
	}
	get_mode_rsp=(ffbm_set_mode_rsq_type*)diagpkt_subsys_alloc( DIAG_SUBSYS_FTM,
				FTM_FFBM_CMD_CODE, sizeof(ffbm_get_mode_rsq_type));
	if (!get_mode_rsp) {
		ALOGE("Failed to allocate get_mode_rsp response packet");
		return get_mode_rsp;
	}
	get_mode(get_mode_rsp);
	if(get_mode_rsp->iCurrentBootMode == BOOT_MODE_FFBM)
	{
		if(RET_OK!=init_regional_pack_conf())
		{
		 ALOGE("read config xml failed");
		 goto error;
		}
		system("mount -o remount,rw /system");
		if(ffbm_pkt->regional_pack_id>REGION_PACK_ID_MAX)
		  goto error;
		index=ffbm_pkt->regional_pack_id;
		ALOGI("Setting regional pack to %s", regional_pack_nodes[index].regional_pack_cmd_string);
		snprintf(cmd,REGIONAL_PACK_SETTING_CMD_MAX_SIZE,"%s",regional_pack_nodes[index].regional_pack_cmd_string);
		system(cmd);
		system("mount -o remount,ro /system");
		rsp->iFTM_Error_Code = FTM_FFBM_SUCCESS;
		return (void*)rsp;
	}else{
		ALOGE("Cannot set regional pack in normal android mode, please switch to FFBM mode for the operation");
	goto error;
   }
error:
	rsp->iFTM_Error_Code = FTM_FFBM_FAIL;
	return (void*)rsp;
}

PACK(void *) ffbm_get_regional_pack()
{
	ffbm_get_region_pack_rsp_type *rsp;
	int rcode = -1;
	ALOGI("In ffbm_get_regional_pack");
	rsp = (ffbm_get_region_pack_rsp_type*)diagpkt_subsys_alloc( DIAG_SUBSYS_FTM,
			FTM_FFBM_CMD_CODE,
			sizeof(ffbm_get_region_pack_rsp_type));
	if (!rsp) {
		ALOGE("Failed to allocate response packet");
	} else {
		get_current_regional_pack_id(rsp);
		if (rcode!=0) {
			ALOGE("Failed to get region pack ID info");
					rsp->iFTM_Error_Code = FTM_FFBM_FAIL;
		} else{
			rsp->iFTM_Error_Code = FTM_FFBM_SUCCESS;
			ALOGE("get reigonal pack ID info succcessfully");
		}
	}
	return(void *) rsp;
}

PACK(void *) ffbm_diag_reset_handler(PACK(void *)req_pkt, uint16 pkt_len)
{
	void *rsp_pkt;
	rsp_pkt = diagpkt_alloc(DIAG_CONTROL_F, pkt_len);

	if (rsp_pkt)
		memcpy(rsp_pkt, req_pkt, pkt_len);
	else {
		ALOGE("diagpkt_alloc failed");
		return rsp_pkt;
	}
	if (write(qcom_system_daemon_pipe[1],
				REBOOT_CMD,
				sizeof(REBOOT_CMD)) < 0) {
		ALOGE("Failed to write to pipe: %s", strerror(errno));
		goto error;
	}
	close(qcom_system_daemon_pipe[1]);
error:
	return rsp_pkt;
}

PACK(void *) ffbm_diag_mode_handler(PACK(void *)req_pkt, uint16 pkt_len)
{
	PACK(void*)rsp = NULL;
	ALOGI("In ffbm_diag_mode_handler");
	rsp = ffbm_dispatch(req_pkt);
	return rsp;
}

void * ffbm_dispatch(ffbm_pkt_type *ffbm_pkt)
{
	ffbm_set_mode_rsq_type *rsp;
	if (ffbm_pkt->ffbm_cmd_code == FTM_FFBM_SET_MODE)
		rsp = ffbm_set_mode((ffbm_set_mode_req_type*)ffbm_pkt);
	else if (ffbm_pkt->ffbm_cmd_code == FTM_FFBM_GET_MODE)
		rsp = ffbm_get_mode();
	else if (ffbm_pkt->ffbm_cmd_code == FTM_FFBM_SET_REGIONAL_PACK)
		rsp = ffbm_set_regional_pack((ffbm_set_region_pack_req_type*)ffbm_pkt);
	else if (ffbm_pkt->ffbm_cmd_code == FTM_FFBM_GET_REGIONAL_PACK)
		rsp = ffbm_get_regional_pack();
	else if (ffbm_pkt->ffbm_cmd_code == FTM_FFBM_HANDLE_PDC){
		rsp=pdc_clnt_handler((ffbm_set_mbn_req_type*)ffbm_pkt);
	}
	else
		rsp = NULL;
	return (void*)rsp;
}

static int set_mode(const char* mode)
{
	int num_bytes = 0;
	unsigned int write_count = 0;
	int fd = -1;

	fd = open(MISC_PARTITION_LOCATION, O_RDWR);
	if (fd < 0) {
		fd = open(MISC_PARTITION_LOCATION_ALT, O_RDWR);
		if (fd < 0) {
			ALOGE("Error locating/opening misc partion: %s",
					strerror(errno));
			goto error;
		}
	}
	do {
		num_bytes = write(fd, mode + write_count/sizeof(char),
				strlen(mode) - write_count);
		if (num_bytes < 0) {
			ALOGE("Failed to write to partition");
			goto error;
		}
		write_count+=num_bytes;
	} while(write_count < strlen(mode));
	close(fd);
	return RET_SUCCESS;
error:
	if (fd >= 0)
		close(fd);
	return RET_FAILED;
}

static int get_mode( ffbm_get_mode_rsq_type *pkt)
{
	int bytes_read = 0;
	char bootmode[PROPERTY_VALUE_MAX];
	int fd = -1;
	int offset = 0;
	char buffer[FFBM_COMMAND_BUFFER_SIZE];
	if (!pkt) {
		ALOGE("Invalid argument to get_mode");
		goto error;
	}
	property_get("ro.bootmode", bootmode, NULL);
	if (!strncmp(bootmode, "ffbm", 4)) {
		ALOGI("Current mode : ffbm");
		pkt->iCurrentBootMode = BOOT_MODE_FFBM;
	} else {
		ALOGI("Current mode : hlos");
		pkt->iCurrentBootMode = BOOT_MODE_HLOS;
	}
	fd = open(MISC_PARTITION_LOCATION, O_RDWR);
	if (fd < 0) {
		fd = open(MISC_PARTITION_LOCATION_ALT, O_RDWR);
		if (fd < 0) {
			ALOGE("Error locating/opening misc partion: %s",
					strerror(errno));
			goto error;
		}
	}
	memset(buffer,'\0', sizeof(buffer));
	do {
		bytes_read = read(fd, buffer + offset,
				(FFBM_COMMAND_BUFFER_SIZE-1) - offset);
		if (bytes_read < 0) {
			ALOGE("Failed to read from misc partition");
			goto error;
		}
		offset += bytes_read;
	} while (bytes_read > 0 && offset < (FFBM_COMMAND_BUFFER_SIZE - 1));

	if (!strncmp(buffer, "ffbm", 4)) {
		ALOGI("Next mode: ffbm");
		pkt->iNextBootMode = BOOT_MODE_FFBM;
	} else {
		ALOGI("Next mode: hlos");
		pkt->iNextBootMode = BOOT_MODE_HLOS;
	}
	return 0;
error:
	if (fd >= 0)
		close(fd);
	return -1;
}

static int open_image_files(void)
{
	fd_select_image = open(SELECT_IMAGE_FILE, O_WRONLY);
	if (fd_select_image < 0) {
		ALOGE("version_diagpkt: could not open select image file: %s", strerror(errno));
		return -1;
	}
	fd_image_version = open(IMAGE_VERSION_FILE, O_RDONLY);
	if (fd_image_version < 0) {
		ALOGE("version_diagpkt: could not open image version file: %s", strerror(errno));
		close(fd_select_image);
		return -1;
	}

	fd_image_variant = open(IMAGE_VARIANT_FILE, O_RDONLY);
	if (fd_image_variant < 0) {
		ALOGE("version_diagpkt: could not open image variant file: %s", strerror(errno));
		close(fd_select_image);
		close(fd_image_version);
		return -1;
	}

	fd_image_crm_version = open(IMAGE_CRM_VERSION_FILE, O_RDONLY);
	if (fd_image_crm_version < 0) {
		ALOGE("version_diagpkt: could not open image crm version file: %s", strerror(errno));
		close(fd_select_image);
		close(fd_image_version);
		close(fd_image_variant);
		return -1;
	}
	return 0;
}

void close_image_files(void)
{
	if (fd_select_image >= 0)
		close(fd_select_image);
	if (fd_image_version >= 0)
		close(fd_image_version);
	if (fd_image_variant >= 0)
		close(fd_image_variant);
	if (fd_image_crm_version >= 0)
		close(fd_image_crm_version);
}

static int read_image_files(unsigned char *temp_version_table_p)
{
	int ret = 0;
	unsigned char *temp;

	if (!temp_version_table_p) {
		ALOGE("Bad Address for image version: %s", strerror(errno));
		return -1;
	}

	temp = temp_version_table_p;
	if (fd_image_version >= 0) {
		ret = read(fd_image_version, temp, IMAGE_VERSION_NAME_SIZE);
		if (ret < 0) {
			ALOGE("version_diagpkt: Unable to read image version file: %s", strerror(errno));
			return -1;
		}
	} else {
		ALOGE("version_diagpkt: Fail to read, invalid fd_image_version");
		return -1;
	}

	temp += (IMAGE_VERSION_NAME_SIZE - 1);
	*temp++ = '\0';
	if (fd_image_variant >= 0) {
		ret = read(fd_image_variant, temp, IMAGE_VERSION_VARIANT_SIZE);
		if (ret < 0) {
			ALOGE("version_diagpkt: Unable to read image variant file: %s", strerror(errno));
			return -1;
		}
	} else {
		ALOGE("version_diagpkt: Fail to read, invalid fd_image_variant");
		return -1;
	}

	temp += (IMAGE_VERSION_VARIANT_SIZE - 1);
	*temp++ = '\0';
	if (fd_image_crm_version >= 0) {
		ret = read(fd_image_crm_version, temp, IMAGE_VERSION_OEM_SIZE);
		if (ret < 0) {
			ALOGE("version_diagpkt: Unable to read image crm version file: %s", strerror(errno));
			return -1;
		}
	} else {
		ALOGE("version_diagpkt: Fail to read, invalid fd_image_crm_version");
		return -1;
	}
	return 0;
}

static int get_version_info(diagpkt_version_delayed_rsp *version_table_p)
{
	int err = 0, ret, i, count = 0;
	unsigned char *temp_version_table_p;
	char image_index[3];

	temp_version_table_p = version_table_p->version_data;

	for (i = 0; i < (VERSION_TABLE_S/IMAGE_VERSION_SINGLE_BLOCK_SIZE); i++) {
		err = open_image_files();
		if (err < 0) {
			ALOGE("version_diagpkt: could not open image files %d", i);
			return -1;
		}
		count = (i < 10) ? 1 : 2;
		snprintf(image_index, sizeof(image_index), "%d", i);
		ret = write(fd_select_image, image_index, count);
		if (ret < 0) {
			ALOGE("version_diagpkt: Unable to write %d in select image file: %s", i, strerror(errno));
			close_image_files();
			return -1;
		}
		ret = read_image_files(temp_version_table_p);
		if (ret < 0) {
			ALOGE("version_diagpkt: Unable to read image file %d", i);
			close_image_files();
			return -1;
		}
		temp_version_table_p += (IMAGE_VERSION_SINGLE_BLOCK_SIZE - 1);
		*temp_version_table_p++ = '\0';

		close_image_files();
	}
	return 0;
}

PACK(void*) diagpkt_version_handler(PACK(void*) req_ptr, uint16 pkt_len)
{
	PACK(void *)rsp = NULL;
	int err = 0;
	diagpkt_version_delayed_rsp *version_table_p = NULL;
	diagpkt_subsys_delayed_rsp_id_type delay_rsp_id = 0;

	/* Allocate the length for immediate response */
	rsp = diagpkt_subsys_alloc_v2(VERSION_DIAGPKT_SUBSYS,
		VERSION_DIAGPKT_PREFIX, sizeof(diagpkt_version_subsys_hdr_v2_type));
	if (rsp == NULL) {
		ALOGE("version_diagpkt: could not allocate memory for response");
		return NULL;
	}
	/* Get the delayed_rsp_id that was allocated by diag to
	* use for the delayed response we're going to send next.
	* This id is unique in the system.
	*/
	delay_rsp_id = diagpkt_subsys_get_delayed_rsp_id(rsp);
	diagpkt_commit(rsp);

	/* Response length can't be more than VERSION_TABLE_S - 2
	* i.e. 4094 bytes because of limitation at diag side
	*/
	rsp = diagpkt_subsys_alloc_v2_delay(VERSION_DIAGPKT_SUBSYS,
		VERSION_DIAGPKT_PREFIX, delay_rsp_id, VERSION_TABLE_S - 2);

	if ((NULL != rsp) && (pkt_len == 4)) {
		diagpkt_subsys_set_rsp_cnt(rsp,1);
		version_table_p = (diagpkt_version_delayed_rsp*) rsp;
	} else {
		ALOGE("version_diagpkt: diagpkt allocation failed:%p or wrong pkt_len:%d",
				rsp, pkt_len);
		return NULL;
	}
	err = get_version_info(version_table_p);
	if (err < 0) {
		ALOGE("version_diagpkt: could not get image version info");
		return NULL;
	}
	version_table_p->version_data_len =
		VERSION_TABLE_S - 2 - sizeof(diagpkt_version_subsys_hdr_v2_type);
	/* copy the version data length */
	memcpy((char *)rsp+sizeof(diagpkt_version_subsys_hdr_v2_type),
		&(version_table_p->version_data_len),
		sizeof(uint16));
	/* copy the image version info */
	memcpy((char *)rsp+sizeof(diagpkt_version_subsys_hdr_v2_type)+sizeof(uint16),
		version_table_p->version_data,
		VERSION_TABLE_S - 2 - sizeof(diagpkt_version_subsys_hdr_v2_type));
	diagpkt_delay_commit(rsp);

	return NULL;
}

void *ffbm_set_mode(ffbm_set_mode_req_type *ffbm_pkt)
{
	ffbm_set_mode_rsq_type *rsp;
	rsp = (ffbm_set_mode_rsq_type*)diagpkt_subsys_alloc( DIAG_SUBSYS_FTM,
		FTM_FFBM_CMD_CODE,
		sizeof(ffbm_set_mode_rsq_type));
	if (!rsp) {
		ALOGE("Failed to allocate response packet");
		return rsp;
	}
	if (ffbm_pkt->iNextBootMode == BOOT_MODE_FFBM) {
		ALOGI("Setting bootmode to FFBM");
		if (set_mode(MODE_FFBM) != RET_SUCCESS) {
			ALOGE("Failed to set bootmode");
			goto error;
		}
	} else if (ffbm_pkt->iNextBootMode == BOOT_MODE_HLOS) {
		ALOGI("Setting bootmode to Normal mode");
		if (set_mode(MODE_NORMAL) != RET_SUCCESS) {
			ALOGE("Failed to set bootmode");
			goto error;
		}
	} else {
		ALOGI("Unknown boot mode recieved");
		goto error;
	}
	rsp->iFTM_Error_Code = FTM_FFBM_SUCCESS;
	return (void*)rsp;
error:
	rsp->iFTM_Error_Code = FTM_FFBM_FAIL;
	return (void*)rsp;
}

void *ffbm_get_mode()
{
	   ffbm_get_mode_rsq_type *rsp;
	   int rcode = -1;
	   ALOGI("In ffbm_get_mode");
	   rsp = (ffbm_get_mode_rsq_type*)diagpkt_subsys_alloc( DIAG_SUBSYS_FTM,
			   FTM_FFBM_CMD_CODE,
			   sizeof(ffbm_get_mode_rsq_type));
	   if (!rsp) {
		   ALOGE("Failed to allocate response packet");
	   } else {
		   rcode = get_mode(rsp);
		   if (rcode) {
			   ALOGE("Failed to get boot mode info");
			   rsp->iFTM_Error_Code = FTM_FFBM_FAIL;
		   } else
			   rsp->iFTM_Error_Code = FTM_FFBM_SUCCESS;
		   /*Boot sub modes are 0 for LA*/
		   rsp->iCurrentBootSubMode = 0;
		   rsp->iNextBootSubMode = 0;
	   }
	   return(void *) rsp;
}

void *reboot_to_edl(PACK(void *)req_pkt, uint16 pkt_len)
{
	void *rsp_pkt = NULL;
	ALOGI("reboot to edl command recieved");
	rsp_pkt = diagpkt_alloc(EDL_RESET_CMD_CODE, pkt_len);
	if (rsp_pkt)
		memcpy(rsp_pkt, req_pkt, pkt_len);
	else {
		ALOGE("diagpkt_alloc failed");
		goto error;
	}
	if (write(qcom_system_daemon_pipe[1],
				EDL_REBOOT_CMD,
				sizeof(EDL_REBOOT_CMD)) < 0) {
		ALOGE("Failed to write command to pipe: %s",
				strerror(errno));
		goto error;
	}
	close(qcom_system_daemon_pipe[1]);
	ALOGI("returning response packet");
error:
	return rsp_pkt;
}

static int get_bootselect_info(boot_selection_info *out) {
	FILE* f = NULL;

	f = fopen(BLOCK_DEVICE_NODE, "rb");
	if (f == NULL) {
		f = fopen(BLOCK_DEVICE_NODE_ALT, "rb");
		if (f == NULL) {
			ALOGE("Can't find/open bootselect node: (%s)",
					strerror(errno));
			goto error;
		}
	}
	int count = fread(out, sizeof(*out), 1, f);
	if (count != 1) {
		ALOGE("Failed reading bootselect node: (%s)", strerror(errno));
		goto error;
	}
	fclose(f);
	return 0;
error:
	if (f)
		fclose(f);
	return -1;
}


static int reset_os_selector_bit(boot_selection_info *in) {
	FILE* f = NULL;
	struct stat block_dev_stat;
	int use_alt_location = 0;
	f = fopen(BLOCK_DEVICE_NODE, "rb");
	if (f == NULL) {
		f = fopen(BLOCK_DEVICE_NODE_ALT, "rb");
		if (f == NULL) {
			ALOGE("Can't find/open bootselect node: (%s)",
					strerror(errno));
			goto error;
		}
	}
	(*in).boot_partition_selection = 0;
	int count = fwrite(in, sizeof(*in), 1, f);

	if (count != 1) {
		ALOGE("Failed writing %s: (%s)",
				BLOCK_DEVICE_NODE,
				strerror(errno));
		goto error;
	}
	fclose(f);
	return 0;
error:
	if (f)
		fclose(f);
	return -1;
}

int main()
{
	int binit_Success = 0;
	int bytes_read = 0;
	int offset = 0;
	char bootmode[PROPERTY_VALUE_MAX];
	char diag_reset_handler[PROPERTY_VALUE_MAX];
	char buffer[FFBM_COMMAND_BUFFER_SIZE];
	struct pollfd fds[2];
	char buf[10];
	int ret, nfds = 1;
	int is_factory = 0;

	// Initialize the structure
	if (!get_bootselect_info(&bootselect_info)) {
		// Create Node only if the factory bit is set
		if ((bootselect_info.state_info & BOOTSELECT_FACTORY)) {
			if ((bootselect_info.signature == BOOTSELECT_SIGNATURE) &&
				(bootselect_info.version == BOOTSELECT_VERSION)) {
				mode_t mode = 0600 | S_IFIFO;
				if (mknod(SOS_FIFO, mode, 0) < 0) {
					if (errno != EEXIST) {
						ALOGE("Failed to create node: %s",
						strerror(errno));
						return -1;
					}
				}
				is_factory = 1;
			} else
				ALOGE("bootselect signature or version mis-matched!");
		}
	}

	ALOGI("Starting qcom system daemon");
	memset(buffer, '\0', sizeof(buffer));
	if (pipe(qcom_system_daemon_pipe) < 0) {
		ALOGE("Failed to create pipe: %s",
				strerror(errno));
		return -1;
	}
	binit_Success = Diag_LSM_Init(NULL);

	if (!binit_Success) {
		ALOGE(" Diag_LSM_Init failed : %d\n",binit_Success);
		close(qcom_system_daemon_pipe[0]);
		close(qcom_system_daemon_pipe[1]);
		return -1;
	}

	DIAGPKT_DISPATCH_TABLE_REGISTER_V2_DELAY(VERSION_DIAGPKT_PROCID,
			VERSION_DIAGPKT_SUBSYS, diagpkt_tbl);
	DIAGPKT_DISPATCH_TABLE_REGISTER(DIAG_SUBSYS_FTM ,
			ftm_table);
	DIAGPKT_DISPATCH_TABLE_REGISTER(DIAG_SUBSYS_SYSTEM_OPERATIONS,
			system_operations_table);
	DIAGPKT_DISPATCH_TABLE_REGISTER (DIAG_SUBSYS_QDSS, qdss_diag_pkt_tbl);

	property_get("ro.bootmode", bootmode, NULL);
	property_get("diag.reset_handler", diag_reset_handler, NULL);

	if (!strncmp(bootmode, "ffbm", 4) || !strncmp(diag_reset_handler, "true", 4)) {
		ALOGI("Registering mode reset handler");
		DIAGPKT_DISPATCH_TABLE_REGISTER(DIAGPKT_NO_SUBSYS_ID,
				ftm_ffbm_mode_table);
	}
	do {
		fds[0].fd = qcom_system_daemon_pipe[0];
		fds[0].events = POLLIN;
		fds[0].revents = 0;
		if (is_factory) {
			int fifo;
			if ((fifo = open(SOS_FIFO, O_RDONLY | O_NONBLOCK)) > 0) {
				fds[1].fd = fifo;
				fds[1].events = POLLIN;
				fds[1].revents = 0;
				nfds = 2;
			}
		}
		// start polling on fds
		ret = poll(fds, nfds, -1);
		if (ret < 0) {
			ALOGE("Failed to poll: %s", strerror(errno));
			goto error;
		}
		if ((fds[0].revents & POLLIN)) {
			do {
				bytes_read = read(qcom_system_daemon_pipe[0],
						buffer + offset,
						(FFBM_COMMAND_BUFFER_SIZE-1) - offset);
				if (bytes_read < 0) {
					ALOGE("Failed to read command from pipe : %s",
						strerror(errno));
				goto error;
				}
			offset += bytes_read;
			} while (bytes_read > 0 && offset <
				(FFBM_COMMAND_BUFFER_SIZE - 1));
			if (!strncmp(buffer, REBOOT_CMD, sizeof(REBOOT_CMD))) {
				ALOGW("Got request to reboot.Shutting down modem");
				subsystem_control_shutdown(PROC_MSM, NULL);
				ALOGW("modem shutdown complete");
				sleep(2);
				android_reboot(ANDROID_RB_RESTART, 0, 0);
			} else if (!strncmp(buffer, EDL_REBOOT_CMD,
				sizeof(EDL_REBOOT_CMD))) {
				ALOGW("Got request for EDL reboot.Shutting down modem");
				subsystem_control_shutdown(PROC_MSM, NULL);
				ALOGW("modem shutdown complete");
				sleep(2);
				android_reboot(ANDROID_RB_RESTART2, 0, "edl");
			}
		}
		if ((fds[1].revents & POLLIN)) {
			bytes_read = read(fds[1].fd, buf, sizeof(buf)-1);
			if (bytes_read <= 0) {
				ALOGE("Failed to read command from pipe : %s",
				strerror(errno));
			} else {
				buf[bytes_read-1] = '\0';
				if (!strncmp(buf, SWITCH_OS_CMD,
						sizeof(SWITCH_OS_CMD))) {
					ALOGI("Got request to switch OS");
					if (reset_os_selector_bit(&bootselect_info)) {
						ALOGE("Error updating bootselect partition");
						continue;
					} else {
						android_reboot(ANDROID_RB_RESTART, 0, 0);
					}
				}
			}
		}
error:
		ALOGE("Unrecognised command");
		bytes_read = 0;
		offset = 0;
	} while(1);
	return 0;
}
