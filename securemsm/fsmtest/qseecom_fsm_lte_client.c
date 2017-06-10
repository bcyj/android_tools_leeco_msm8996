/********************************************************************
---------------------------------------------------------------------
 Copyright (c) 2013-2014 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
----------------------------------------------------------------------
QSEECom Sample/Test Client app.
*********************************************************************/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/msm_ion.h>
#include <time.h>
#ifdef _ANDROID_
#include <utils/Log.h>
#include "common_log.h"
#endif
#include "QSEEComAPI.h"
#include <sys/mman.h>
#include <getopt.h>
#include "toeKeyGenApi.h"
#include "toe_ike.h"
#include "procPrivsWrapper.h"

#ifndef _ANDROID_
#define MSG_LOG fprintf
#define LOGE(fmt, args...) MSG_LOG(stderr, fmt, ##args)
#define LOGD(fmt, args...) MSG_LOG(stdout, fmt, ##args)
#endif

#define SHA_256_SIZE_BYTES  32

typedef PACKED struct {
    uint32_t key_idx;
    uint8_t  data[SHA_256_SIZE_BYTES];
}req_buf_t;

typedef PACKED struct {
    uint8_t  data[SHA_256_SIZE_BYTES];
}rsp_buf_t;


req_buf_t req_buf;
rsp_buf_t rsp_buf;

tzbsp_rsa_key_t _key = {
1024,
/* n */
"9ec4d483330916b69eee4e9b7614eafc4fbf60e74b5127a3ff5bd9d48c7ecf8418d94d1e60388b"
"b68546f8bc92deb1974b9def6748fbb4ec93029ea8b7bea36f61c5c6aeedfd512a0f765846fad5"
"edacb08c3d75cf1d43b48b394c94323c3f3e9ba6612f93fe2900134217433afb088b5ca33fc4e6"
"b270194df077d2b6592743",
/* p */
"c3cd741d76dff6aebc64a234d077bc303c4b361ca9b52607f6ea787f8789e0b3e0dc13d9725f9a"
"7eb55dd8dc6335dd9603bdba29320ff371cc72593f78433c07",
/* q */
"cf94a874e82decba20a950449d225817a1e4ec0ee8c658cfd4bb97fdc7a4d1b0d06822228b5764"
"dc99b9e1b9ea43bb3fea530fc802124b73c67d523f8e24a3e5",
/* e */
"000000000000000000000000000000000000000000000000000000000000000000000000000000"
"000000000000000000000000000000000000000000000000000000000000000000000000000000"
"000000000000000000000000000000000000000000000000000000000000000000000000000000"
"0000000000000000000003",
/* d */
"1a76236b332c2e73c527b7c493ae272a0d4a90268c8d869b5539f9a36cbfcd40aecee22fbab417"
"4916367eca187a72ee8c9a529136d49e276dd5c51c1e9fc5e7a265f1af6d27e7a03311664cecdf"
"4ee6230f59e1b4c5a0cf754334ae7b2ceccefc65b1ddee61319b76070f722795929e3d1868f89d"
"c2b2ddf480220df2a8368f"
};

int32_t tz_ike_sign(void);
int32_t tz_ike_prov_key(void);

uint32_t random_hex(void)
{
    uint32_t x;
    x = (rand() & 0xff) << 24;
    x |= (rand() & 0xff) << 16;
    x |= (rand() & 0xff) << 8;
    x |= rand() & 0xff;

    LOGD("%08x",x);
    return x;
}


int32_t qsc_init_clr_db(void)
{
    int ret = 0;

    memset(&req_buf, 0, sizeof(req_buf));

    ret = QSEECom_send_service_cmd((void*) &req_buf, sizeof(uint32_t),
                    NULL, 0, QSEECOM_FSM_LTE_INIT_DB);

    if(!ret)
        LOGD("\nKey managment database inititialization was successful!!\n");
    else
        LOGD("\nInitialization failed :(  (%d)\n", ret);

    return ret;
}

int32_t lteStoreKenb (uint32_t keyIdx, uint8_t* kenb)
{
    int ret = 0;

    memset(&req_buf, 0, sizeof(req_buf));
    req_buf.key_idx = keyIdx;
    memcpy(req_buf.data, (uint8_t *)kenb, 32);
    size_t req_len = sizeof(req_buf);

    ret = QSEECom_send_service_cmd((void*) &req_buf, req_len,
                    NULL, 0, QSEECOM_FSM_LTE_STORE_KENB);

    return ret;
}


int32_t qsc_store_kenb(uint32_t keyIdx)
{
    int ret = 0;
    LOGD(" 128 bit KeNB: ");
    srand(time(NULL));
    uint32_t kenb[8] = {random_hex(), random_hex(), random_hex(),
         random_hex(), random_hex(), random_hex(), random_hex(), random_hex()};

    LOGD("\n");
    ret = lteStoreKenb(keyIdx, (uint8_t *)kenb);

    if(!ret)
        LOGD("Storing kenb is completed\n");
    else
        LOGD("Storing kenb failed (%d)\n", ret);

    return ret;
}


int32_t lteStoreNh (uint32_t keyIdx, uint8_t* nh)
{
    int ret = 0;

    memset(&req_buf, 0, sizeof(req_buf));
    req_buf.key_idx = keyIdx;
    memcpy(req_buf.data, (uint8_t *)nh, 32);
    size_t req_len = sizeof(req_buf);

    ret = QSEECom_send_service_cmd((void*) &req_buf, req_len,
                    NULL, 0, QSEECOM_FSM_LTE_STORE_NH);

    return ret;
}


int32_t qsc_store_nh(uint32_t keyIdx)
{
    int ret = 0;

    LOGD(" 128 bit NH: ");
    srand(time(NULL));
    uint32_t nh[8] = {random_hex(), random_hex(), random_hex(),
         random_hex(), random_hex(), random_hex(), random_hex(), random_hex()};

    LOGD("\n");

    ret = lteStoreNh(keyIdx, (uint8_t *)nh);
    if(!ret)
        LOGD("Storing nh is completed\n");
    else
        LOGD("Storing nh failed (%d)\n", ret);

    return ret;
}

int32_t lteDeleteNh (uint32_t keyIdx)
{
    int ret = 0;

    memset(&req_buf, 0, sizeof(req_buf));
    req_buf.key_idx = keyIdx;
    size_t req_len = sizeof(req_buf.key_idx);

    ret = QSEECom_send_service_cmd((void*) &req_buf, req_len,
                    NULL, 0, QSEECOM_FSM_LTE_DELETE_NH);
    return ret;
}

int32_t qsc_delete_nh(uint32_t keyIdx)
{
    int ret = 0;

    ret = lteDeleteNh (keyIdx);

    if(!ret)
        LOGD("deleting nh is completed\n");
    else
        LOGD("deleting nh failed (%d)\n", ret);

    return ret;
}


int32_t lteGenerateKeys (uint32_t keyIdx, uint32_t keyType,
                         LteKdfAlgTypes algId)
{
    int ret = 0;
    LteGenKeysParams* params = (LteGenKeysParams *)req_buf.data;

    memset(&req_buf, 0, sizeof(req_buf));
    req_buf.key_idx = keyIdx;
    params->keyType = (uint8_t)keyType;
    params->algId = (uint8_t)algId;
    size_t req_len = sizeof(req_buf.key_idx) + sizeof(LteGenKeysParams);

    ret = QSEECom_send_service_cmd((void*) &req_buf, req_len,
                    NULL, 0, QSEECOM_FSM_LTE_GEN_KEYS);
    return ret;
}

int32_t qsc_gen_keys(uint32_t keyIdx)
{
    int ret = 0;
    srand (time(NULL));
    LteKdfAlgTypes algId = LTE_SEC_ALG_ID_NULL + (rand() % 5);
    uint32_t keyType = K_RRC_ENC_ALG_P1 + (rand() % 4);
    LOGD(" Randomly selecting Alg Id = %d and key Type = %d\n", algId, keyType);

    ret = lteGenerateKeys (keyIdx, keyType, algId);

    if(!ret)
        LOGD("generating keys was completed\n");
    else
        LOGD("generating keys failed (%d)\n", ret);

    return ret;
}


int32_t lteGenerateKenbStar (uint32_t keyIdx, const LteKenbStarParams* params)
{
    int ret = 0;

    memset(&req_buf, 0, sizeof(req_buf));
    req_buf.key_idx = keyIdx;
    memcpy(req_buf.data, (uint8_t *)params, sizeof(LteKenbStarParams));
    size_t req_len = sizeof (req_buf.key_idx)+ sizeof(LteKenbStarParams);

    ret = QSEECom_send_service_cmd((void*) &req_buf, req_len,
                    NULL, 0, QSEECOM_FSM_LTE_GEN_KENB_STARS);
    return ret;
}

int32_t qsc_gen_kenb_star(uint32_t keyIdx, uint32_t kenbStarIdx)
{

    int ret = 0;
    LteKenbStarParams params;

    srand (time(NULL));
    params.pci = rand();
    params.earfcnDl= rand();
    params.useNH = rand() % 2;
    params.kenbStarIdx = kenbStarIdx;
    LOGD("Selecting random values for: pci = %d earfcnDl = %d\n", params.pci, params.earfcnDl);
    LOGD("Choosing randomly to %suse NH\n", (params.useNH ? "" : "NOT "));


    ret = lteGenerateKenbStar(keyIdx, (const LteKenbStarParams *) &params);

    if(!ret)
        LOGD("generating kenb* is completed\n");
    else
        LOGD("generating kenb* failed (%d)\n", ret);

    return ret;
}


int32_t lteGetKenbStar (uint32_t keyIdx, uint8_t* kenbStar)
{
    int ret = 0;

    memset(&req_buf, 0, sizeof(req_buf));
    req_buf.key_idx = keyIdx;
    size_t req_len = sizeof(req_buf.key_idx);

    ret = QSEECom_send_service_cmd((void*) &req_buf, req_len,
                    kenbStar, SHA_256_SIZE_BYTES,
                    QSEECOM_FSM_LTE_GET_KENB_STARS);
    return ret;
}


int32_t qsc_get_kenb_star(uint32_t keyIdx)
{
    int ret = 0;
    int i;

    memset(&rsp_buf, 0, sizeof(rsp_buf));
    ret = lteGetKenbStar (keyIdx, (uint8_t *)&(rsp_buf.data[0]));

    if(!ret)
    {
        LOGD("getting kenb* is completed\n");
        LOGD("kenb* (hex):\n   ");
        for (i=0; i<32; i++)
           LOGD("%02x",rsp_buf.data[i]);
        LOGD("\n");
    }
    else
        LOGD("geting kenb* failed (%d)\n", ret);

    return ret;
}


int32_t lteGetKeyOffsets (uint32_t keyIdx, LteSecImemOffsets * keyOffsets)
{
    int ret = 0;

    memset(&req_buf, 0, sizeof(req_buf));
    req_buf.key_idx = keyIdx;
    size_t req_len = sizeof(req_buf.key_idx);

    ret = QSEECom_send_service_cmd((void*) &req_buf, req_len,
                    keyOffsets, sizeof(LteSecImemOffsets),
                    QSEECOM_FSM_LTE_GET_KEY_OFFSETS);

    return ret;
}

int32_t qsc_get_key_offsets(uint32_t keyIdx)
{
    int ret = 0;

    memset(&rsp_buf, 0, sizeof(rsp_buf));
    LteSecImemOffsets* offsets = (LteSecImemOffsets *)rsp_buf.data;

    lteGetKeyOffsets(keyIdx, offsets);

    if(!ret)
    {
        LOGD("getting key offsets was completed\n");
        LOGD("0x%x ",offsets->kenbRrcIntOffset);
        LOGD("0x%x ",offsets->kenbRrcEncOffset);
        LOGD("0x%x ",offsets->kenbUpIntOffset);
        LOGD("0x%x ",offsets->kenbUpEncOffset);
        LOGD("\n");
    }
    else
        LOGD("getting key ofssets failed (%d)\n", ret);

    return ret;
}

int32_t lteDeleteKeys (uint32_t keyIdx)
{
    int ret = 0;

    memset(&req_buf, 0, sizeof(req_buf));
    req_buf.key_idx = keyIdx;
    size_t req_len = sizeof(req_buf.key_idx);

    ret = QSEECom_send_service_cmd((void*) &req_buf, req_len,
                    NULL, 0, QSEECOM_FSM_LTE_DELETE_KEYS);
    return ret;
}


int32_t qsc_delete_keys(uint32_t keyIdx)
{
    int ret = 0;

    ret = lteDeleteKeys (keyIdx);

    if(!ret)
        LOGD("deleting keys is completed\n");
    else
        LOGD("deleting keys failed (%d)\n", ret);

    return ret;
}

int32_t
tz_ike_prov_key(void)
{
    int ret = 0;
    tzbsp_ike_prov_key_cmd_t req;
    tzbsp_ike_rsp_prov_key_cmd_t rsp;
    size_t req_len = sizeof(_key);
    memcpy((char *)(&req.keybuf), (char *)&_key, req_len);
    ret = QSEECom_send_service_cmd((void *)&req,
                sizeof(tzbsp_ike_prov_key_cmd_t), (void *)&rsp,
                sizeof(tzbsp_ike_rsp_prov_key_cmd_t),
                QSEECOM_FSM_IKE_CMD_PROV_KEY);
    if(!ret)
        LOGD("IKE priv key provision success\n");
    else
        LOGD("KE priv key provision failed (%d)\n", ret);
    return ret;
}

int32_t
tz_ike_sign(void)
{
    int ret = 0;
    int i =0;
    tzbsp_ike_sign_cmd_t req;
    tzbsp_ike_rsp_sign_cmd_t rsp;
    req.rsa_pad = CE_RSA_PAD_PKCS1_V1_5_SIG;
    req.saltlen = 0;
    req.buflen = 0x20;
    for(i=0; i<32; i++){
        req.buf[i] = 0x34;
    }
    ret = QSEECom_send_service_cmd((void *)&req,
                sizeof(tzbsp_ike_sign_cmd_t), (void *)&rsp,
                sizeof(tzbsp_ike_rsp_sign_cmd_t),
                QSEECOM_FSM_IKE_CMD_SIGN);
    if(!ret)
        LOGD("IKE Signature success \n");
    else
        LOGD("IKE RSA Signature Failed (%d)\n", ret);
    return ret;
}

void print_menu(void)
{
    LOGD("\t-------------------------------------------------------\n");
    LOGD("\t-------------------------------------------------------\n");
    LOGD("\t 1 -> Store kenb in key dbase\n");
    LOGD("\t 2 -> Generate Keys and store in dbase\n");
    LOGD("\t 3 -> Generate Kenb* and store in dbase\n");
    LOGD("\t 4 -> Get stored Kenb* from dbase\n");
    LOGD("\t 5 -> Get key offsets from dbase\n");
    LOGD("\t 6 -> Store nh in key dbase\n");
    LOGD("\t 7 -> Delete nh from dbase\n");
    LOGD("\t 8 -> Delete keys for a key_idx from dbase\n");
    LOGD("\t 9 -> Invalidate all ota keys in dbase\n");
    LOGD("\t A -> Provision IKE Priv Key in IMEM\n");
    LOGD("\t B -> Sign the given buffer using IKE priv keys\n");
    LOGD("\t 0 -> Exit\n");
    LOGD("\t-------------------------------------------------------\n");
}

int main (int argc, char *argv[])
{
    int ret, ueid, kenb_star_id;
    char option, cr_ret;

    /* Declare a pointer to the dummy structure type and a return value. */
    ProcPrivs_t *procPrivsPtr = NULL;

    /* Create the ProcPrivs object */
    procPrivsPtr = createProcPrivs();

    /* Exit immediately if this is unsuccessful */
    if (procPrivsPtr == NULL)
    {
        exit(1);
    }

    /* User - apps */
    ret = procPrivsSetUser(procPrivsPtr, "apps");

    /* Exit immediately if this is unsuccessful */
    if (ret != 0)
    {
        /* Destroy the ProcPrivs object */
        deleteProcPrivs(procPrivsPtr);
        exit(1);
    }

    /* Group - apps */
    ret = procPrivsSetGroup(procPrivsPtr, "apps");

    /* Exit immediately if this is unsuccessful */
    if (ret != 0)
    {
        /* Destroy the ProcPrivs object */
        deleteProcPrivs(procPrivsPtr);
        exit(1);
    }

    /* Apply the privileges. */
    ret = procPrivsApplyPrivileges(procPrivsPtr);

    /* Exit immediately if this is unsuccessful */
    if (ret != 0)
    {
        /* Destroy the ProcPrivs object */
        deleteProcPrivs(procPrivsPtr);
        exit(1);
    }

    /* Don't need the object anymore */
    deleteProcPrivs(procPrivsPtr);
    procPrivsPtr = NULL;

    LOGD("Usage:qseecom_fsm_lte_client \n");
    ret = qsc_init_clr_db();

    while (1)
    {
      print_menu();
      LOGD("\t Select an option to proceed: ");
      scanf("%c%c", &option,&cr_ret);
      LOGD("\nRESULTS OUTPUT\n");
      LOGD("*****************************************************\n");
      LOGD("You selected option %c. \n\n", option);

      switch(option) {
        case '0':
          exit(0);
        case '1':
          LOGD("\t Enter the UE ID: ");
          scanf("%d%c", &ueid,&cr_ret);
          ret = qsc_store_kenb(ueid);
          break;
        case '2':
          LOGD("\t Enter the UE ID: ");
          scanf("%d%c", &ueid,&cr_ret);
          ret = qsc_gen_keys(ueid);
          break;
        case '3':
          LOGD("\t Enter the UE ID: ");
          scanf("%d%c", &ueid,&cr_ret);
          LOGD("\t Enter the KeNB* index: ");
          scanf("%d%c", &kenb_star_id, &cr_ret);
          ret = qsc_gen_kenb_star(ueid, kenb_star_id);
          break;
        case '4':
          LOGD("\t Enter the UE ID: ");
          scanf("%d%c", &ueid,&cr_ret);
          ret = qsc_get_kenb_star(ueid);
          break;
        case '5':
          LOGD("\t Enter the UE ID: ");
          scanf("%d%c", &ueid,&cr_ret);
          ret = qsc_get_key_offsets(ueid);
          break;
        case '6':
          LOGD("\t Enter the UE ID: ");
          scanf("%d%c", &ueid,&cr_ret);
          ret = qsc_store_nh(ueid);
          break;
        case '7':
          LOGD("\t Enter the UE ID: ");
          scanf("%d%c", &ueid,&cr_ret);
          ret = qsc_delete_nh(ueid);
          break;
        case '8':
          LOGD("\t Enter the UE ID: ");
          scanf("%d%c", &ueid,&cr_ret);
          ret = qsc_delete_keys(ueid);
          break;
        case '9':
          ret = qsc_init_clr_db();
          break;
        case 'A':
          ret = tz_ike_prov_key();
          break;
        case 'B':
          ret = tz_ike_sign();
          break;
        default:
          LOGD("bad option %c\n", option);
          break;
      }
      LOGD("*****************************************************\n\n");
    }
    return ret;
}
