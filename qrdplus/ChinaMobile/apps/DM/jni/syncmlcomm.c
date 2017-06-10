#ifdef WIN32
#include "std_header.h"
#endif
#define _SYNCMLCOMM_C_

#include "syncmlcomm.h"

#include "smlerr.h"
#include "dm_task.h"
#include "md5.h"
#include "dm_pl_tree.h"

syncml_Comm_type* dm_global_Comm_Ptr = NULL;

extern task_relay_info_type* dm_task_relay_info;

extern void dm_calc_b64_cred(char* creddata, unsigned long cbLength);
extern char* dm_calc_md5_cred(char* creddata);

short dm_syncml_Comm_Init() {
    syncml_comm_message("MMIDM  *_*dm_syncml_Comm_Init");
    dm_global_Comm_Ptr = (syncml_Comm_type*) dm_smlLibMalloc(__FILE__, __LINE__,
            (long) sizeof(syncml_Comm_type));
    if (NULL == dm_global_Comm_Ptr) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(dm_global_Comm_Ptr, 0, sizeof(syncml_Comm_type));

    dm_global_Comm_Ptr->codetype = (XmlCodeType) dm_task_relay_info->encodetype;

    if(VDM_ERR_OK != VDM_Comm_PL_HTTP_init(&dm_task_relay_info->comm_context_p, "URI",
            dm_task_relay_info->maxmsg_size,
            &dm_task_relay_info->comm_observer))
    {
          dm_smlLibFree(dm_global_Comm_Ptr);
          dm_global_Comm_Ptr = NULL;
          return SML_ERR_NOT_ENOUGH_SPACE;
    }

    return SML_ERR_OK;
}

short dm_syncml_Comm_Open(void) {
    unsigned int mimetype = 0;

    syncml_comm_message("MMIDM  *_*dm_syncml_Comm_Open");

    mimetype = (unsigned int) (dm_task_relay_info->encodetype == SML_WBXML);

#if 0
    dm_smlLockReadBuffer( dm_task_relay_info->workspaceid, (unsigned char*)(&(dm_global_Comm_Ptr->cache)), (long *)(&(dm_global_Comm_Ptr->cache_length)));/*lint !e64*/
    syncml_comm_message("MMIDM  *_*dm_syncml_Comm_Open ,data_len=%d",dm_global_Comm_Ptr->cache_length);

    dm_task_relay_info->cach_buffer_ptr=dm_smlLibMalloc(__FILE__, __LINE__, dm_global_Comm_Ptr->cache_length+1);
    dm_smlLibMemset(dm_task_relay_info->cach_buffer_ptr, 0 , dm_global_Comm_Ptr->cache_length+1);
    xppStrcpy(dm_task_relay_info->cach_buffer_ptr,dm_global_Comm_Ptr->cache);
    dm_smlUnlockReadBuffer(dm_task_relay_info->workspaceid,dm_global_Comm_Ptr->cache_length);
    dm_global_Comm_Ptr->cache=dm_task_relay_info->cach_buffer_ptr;
#endif

    return VDM_Comm_PL_HTTP_open(dm_task_relay_info->comm_context_p,
            &dm_task_relay_info->comm_session_type,
            dm_task_relay_info->syncml_connect_addr, 0, mimetype,
            //&dm_task_relay_info->comm_hmac );
            NULL );
    //return 0;
}

short dm_syncml_Comm_SendData(short id, BOOLEAN is_resent) {
    /* Lock the workspace for reading the assembled SyncML document */
    syncml_comm_message("MMIDM  *_*dm_syncml_Comm_SendData ,enter!");
#if 0
//  dm_smlLockReadBuffer(id, (unsigned char*)(&(dm_global_Comm_Ptr->cache)), (long *)(&(dm_global_Comm_Ptr->cache_length)));/*lint !e64*/
    dm_smlLibStrcpy(dm_task_relay_info->client_nextnonce_bak,dm_task_relay_info->client_nextnonce);
    dm_task_relay_info->client_nextnonce_bak[xppStrlen(dm_task_relay_info->client_nextnonce)] = 0;

    syncml_comm_message("MMIDM  *_*dm_syncml_Comm_SendData ,data_len=%d",dm_global_Comm_Ptr->cache_length);
    VDM_Comm_PL_HTTP_send(dm_task_relay_info->comm_context_p,
            dm_task_relay_info->comm_session_type,
            dm_global_Comm_Ptr->cache,
            dm_global_Comm_Ptr->cache_length );
//  dm_smlUnlockReadBuffer(dm_task_relay_info->workspaceid,dm_global_Comm_Ptr->cache_length);
#else
    if (FALSE == is_resent) {
        //backup client_nonce
        dm_smlLibStrcpy(dm_task_relay_info->client_nextnonce_bak,
                dm_task_relay_info->client_nextnonce);
        dm_task_relay_info->client_nextnonce_bak[xppStrlen(dm_task_relay_info->client_nextnonce)] =
                0;

        dm_smlLockReadBuffer(dm_task_relay_info->workspaceid,
                (unsigned char*) (&(dm_global_Comm_Ptr->cache)),
                (long *) (&(dm_global_Comm_Ptr->cache_length)));/*lint !e64*/
        syncml_comm_message("MMIDM  *_*dm_syncml_Comm_Open ,data_len=%d",dm_global_Comm_Ptr->cache_length);

        dm_task_relay_info->cach_buffer_ptr = dm_smlLibMalloc(__FILE__,
                __LINE__, dm_global_Comm_Ptr->cache_length + 1);
        if(dm_task_relay_info->cach_buffer_ptr == NULL)
           return SML_ERR_NOT_ENOUGH_SPACE;
        dm_smlLibMemset(dm_task_relay_info->cach_buffer_ptr, 0,
                dm_global_Comm_Ptr->cache_length + 1);
        xppStrcpy(dm_task_relay_info->cach_buffer_ptr,
                dm_global_Comm_Ptr->cache);
        dm_smlUnlockReadBuffer(dm_task_relay_info->workspaceid,
                dm_global_Comm_Ptr->cache_length);
        dm_global_Comm_Ptr->cache = dm_task_relay_info->cach_buffer_ptr;
    }
    syncml_comm_message("MMIDM  *_*dm_syncml_Comm_SendData ,data_len=%d",dm_global_Comm_Ptr->cache_length);
    VDM_Comm_PL_HTTP_send(dm_task_relay_info->comm_context_p,
            dm_task_relay_info->comm_session_type, dm_global_Comm_Ptr->cache,
            dm_global_Comm_Ptr->cache_length);

#endif

    syncml_comm_message("MMIDM  *_*dm_syncml_Comm_SendData ,return!");
    return SML_ERR_OK;
}

/*==========================================================
 * function     : dm_syncml_is_total_package
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/11/2011
 ==========================================================*/
BOOLEAN dm_syncml_is_total_package(char * messagebody,
        VDM_Comm_HMAC_t* hmac_info) {
    //char username[] = "OMADM";
    //char password[] = "mvpdm";
    int len = 0;
    char nonce_char[30] = { 0 };
    char creddata[180] = { 0 };
    char * username = NULL;
    char * password = NULL;
    char * nonce = dm_task_relay_info->client_nextnonce_bak;

    if (0 == strlen(hmac_info->username)) {
        syncml_comm_message("MMIDM  *_* dm_syncml_is_total_package ,hmac_info is NULL!");
        return TRUE;
    }

    //username=hmac_info->username;
    username = dmTreeParam_getDmAccInfo()->accServerAuthName;
    password = dmTreeParam_getDmAccInfo()->accServerPw;
    syncml_comm_message("MMIDM  *_* dm_syncml_is_total_package ,mac=%s",hmac_info->mac);
    len = strlen(nonce);
    dm_calc_b64_cred(dm_calc_md5_cred(messagebody), HASHLEN);

    sprintf(creddata, "%s:%s", username, password);
    syncml_comm_message("MMIDM  *_* dm_syncml_is_total_package ,user:password=%s",creddata);
    dm_calc_b64_cred(dm_calc_md5_cred(creddata), HASHLEN);
    xppStrcat(creddata, ":");

    dm_base64Decode(nonce_char, 30, nonce, &len);
    xppStrcat(creddata, nonce_char);

    xppStrcat(creddata, ":");
    xppStrcat(creddata, messagebody);
    dm_calc_b64_cred(dm_calc_md5_cred(creddata), HASHLEN);

    syncml_comm_message("MMIDM  *_* dm_syncml_is_total_package ,creddata=%s",creddata);

    if (0 == xppStrcmp(creddata,hmac_info->mac)) {
        syncml_comm_message("MMIDM  *_* dm_syncml_is_total_package ,TRUE !");
        return TRUE;
    } else {
        syncml_comm_message("MMIDM  *_* dm_syncml_is_total_package ,FALSE !");
        return FALSE;
    }
}

short dm_syncml_Comm_RecData(short id) {
    int outMsgLen = 0;
    syncml_comm_message("MMIDM  *_*Begin dm_syncml_Comm_RecData!id=%d",id);

    /* Lock the workspace for reading the assembled SyncML document */
    dm_smlLockWriteBuffer(id, (unsigned char*) (&(dm_global_Comm_Ptr->cache)),
            (long *) (&(dm_global_Comm_Ptr->cache_length)));
    syncml_comm_message("MMIDM  *_*id=%d,cache=%d,cache_len=%d",id,dm_global_Comm_Ptr->cache,dm_global_Comm_Ptr->cache_length);

    /*use the bearer  interface*/
    VDM_Comm_PL_HTTP_receive(dm_task_relay_info->comm_context_p,
            dm_task_relay_info->comm_session_type, dm_global_Comm_Ptr->cache,
            dm_global_Comm_Ptr->cache_length, &outMsgLen,
            &dm_task_relay_info->comm_hmac);
    syncml_comm_message("MMIDM  *_* receive_data_len=%d,id=%d",outMsgLen,dm_task_relay_info->workspaceid);
    syncml_comm_message("MMIDM  *_* len=%d,receive_data=%s",strlen(dm_global_Comm_Ptr->cache) , dm_global_Comm_Ptr->cache);
    dm_smlUnlockWriteBuffer(dm_task_relay_info->workspaceid, outMsgLen);

    dm_syncml_Comm_Close();
    syncml_comm_message("MMIDM  *_*Finish  dm_syncml_Comm_RecData!");

    if (dm_task_relay_info->authored) {
        char * package_buf = NULL;

        package_buf = dm_smlLibMalloc(__FILE__, __LINE__, outMsgLen + 1);
        if (NULL == package_buf) {
            return SML_ERR_A_COMM_ERROR;
        }
        dm_smlLibMemset(package_buf, 0, outMsgLen + 1);

        xppStrcpy(package_buf, dm_global_Comm_Ptr->cache);
        if (dm_syncml_is_total_package(package_buf,
                &dm_task_relay_info->comm_hmac)) {
            return SML_ERR_OK;
        } else {
            return SML_ERR_A_COMM_ERROR;
        }
    } else {
        syncml_comm_message("MMIDM  *_* First package,without checkout!");
        return SML_ERR_OK;
    }
}

short dm_syncml_Comm_Close(void) {
    syncml_comm_message("MMIDM  *_* dm_syncml_Comm_Close");
    VDM_Comm_PL_HTTP_close(dm_task_relay_info->comm_context_p,
            dm_task_relay_info->comm_session_type);
    return SML_ERR_OK;
}

short dm_syncml_Comm_Destory() {
    syncml_comm_message("MMIDM  *_* dm_syncml_Comm_Destory!");
    if (NULL != dm_global_Comm_Ptr) {
        dm_smlLibFree(dm_global_Comm_Ptr);
        dm_global_Comm_Ptr = NULL;
    }
    VDM_Comm_PL_HTTP_term(dm_task_relay_info->comm_context_p);
    return SML_ERR_OK;
}
