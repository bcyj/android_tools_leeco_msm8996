/********************************************************************
---------------------------------------------------------------------
 Copyright (c) 2014 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
----------------------------------------------------------------------
Macchiato Sample/Test Client app.
*********************************************************************/

#include <utils/Log.h>
#include "QSEEComAPI.h"
#include "common_log.h"
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>


/** adb log */
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "MACCHIATO_SAMPLE_CLIENT: "
#ifdef LOG_NDDEBUG
#undef LOG_NDDEBUG
#endif
#define LOG_NDDEBUG 1 //Define to enable LOGD
#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#endif
#define LOG_NDEBUG  0 //Define to enable LOGV

#define LOGD_PRINT(...) do { LOGD(__VA_ARGS__); printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGE_PRINT(...) do { LOGE(__VA_ARGS__); printf(__VA_ARGS__); printf("\n"); } while(0)

/* common declarations with TZ side */

#define MACCHIATO_PUB_KEY_LEN              64
#define MACCHIATO_ECC_SIG_LEN              64

#define MACCHIATO_PROTOCOL_VERSION      1
#define MACCHIATO_AUTHENTICATE_MSG_ID   1

#define MACCHIATO_SAMPLE_CMD0_SIGN_SERVICE_DATA   0
#define MACCHIATO_SAMPLE_CMD1_AUTH_CHALLENGE      1

#define MACCHIATO_CHALLENGE_RESPONSE_LEN   16
#define MAX_DATA_SIZE                      512
#define MACCHIATO_HDR_MAX_SIZE             1024
#define MAX_OUTPUT_SIZE                    (MACCHIATO_HDR_MAX_SIZE + MAX_DATA_SIZE)
#define TA_HASH_LEN                        32

#define SIGNED_MSG_FILE "/data/local/tmp/signed_svc_data.macc"
#define AUTH_MSG_FILE   "/data/local/tmp/auth_msg.macc"
#define CHALLENGE_FILE  "/data/local/tmp/challenge.macc"

#pragma pack (push, macchiato, 1)

/* This struct represents the common header fields in every Macchiato message. */
struct macchiato_msg_hdr_common_fields{
	uint32_t u32ProtocolVersion;
	uint32_t u32MsgId;
	uint32_t u32MsgLength;
	uint32_t u32ErrorCode;
};

/* This struct represents the fields that identify the device in the Macchiato signed message*/
struct macchiato_device_identifier{
	uint16_t u16HwVersion;
	uint32_t u32ChipSerialNum;
};

/* This struct represents the Macchiato challenge message. */
struct macchiato_challenge_msg{
	uint8_t challenge[MACCHIATO_CHALLENGE_RESPONSE_LEN];
};

struct macchiato_sample_cmd{
	uint32_t cmd_id;
	uint8_t data[MAX_DATA_SIZE];
	uint32_t data_len;
	struct macchiato_challenge_msg challenge_msg;
 };

struct macchiato_sample_cmd_rsp{
	int32_t status;
	uint8_t output[MAX_OUTPUT_SIZE];
	uint32_t output_len;
 };

struct macchiato_signed_msg_format{
	struct macchiato_msg_hdr_common_fields signed_msg_hdr;
	uint8_t uccSignature[MACCHIATO_ECC_SIG_LEN];
	uint8_t devicePublicSigningKey[MACCHIATO_PUB_KEY_LEN];
	uint8_t devicePublicEncryptionKey[MACCHIATO_PUB_KEY_LEN];
	uint8_t TaHash[TA_HASH_LEN];
	uint32_t u32NumSvcIDs;
};

struct macciato_signed_msg_additional_fields{
	struct macchiato_device_identifier device_id_fields;
	uint8_t challenge[MACCHIATO_CHALLENGE_RESPONSE_LEN];
	uint8_t response[MACCHIATO_CHALLENGE_RESPONSE_LEN];
	uint8_t opaque_data[1];
};

#pragma pack (pop, macchiato)

/*****************************************/

#define SHARED_BUFFER_SIZE (sizeof(struct macchiato_sample_cmd) + sizeof(struct macchiato_sample_cmd_rsp) + 1042)

static pthread_mutex_t g_qseecom_cmd_mutex = PTHREAD_MUTEX_INITIALIZER;


static int32_t qsc_start_app(struct QSEECom_handle **l_QSEEComHandle,
                        const char *appname, int32_t buf_size)
{
	int32_t ret = 0;

	/* start the application */
	ret = QSEECom_start_app(l_QSEEComHandle, "/firmware/image",
				appname, buf_size);
	if (ret) {
		/* try symbolic link */
		ret = QSEECom_start_app(l_QSEEComHandle, "/system/etc/firmware",
				appname, buf_size);
		if (ret) {
			LOGE_PRINT("Loading app -%s failed",appname);
		} else {
			LOGD_PRINT("Loading app -%s succeded, with buffer size %d",appname, buf_size);
		}

	} else {
		LOGD_PRINT("Loading app -%s succeded, with buffer size %d",appname, buf_size);
	}


	return ret;
}

/**@brief:  Implement simple shutdown app
 * @param[in]	handle.
 * @return	zero on success or error count on failure.
 */
static int32_t qsc_shutdown_app(struct QSEECom_handle **l_QSEEComHandle)
{
	int32_t ret = 0;

	LOGD("qsc_shutdown_app: start");

	/* shutdown the application */
	if (*l_QSEEComHandle != NULL) {
		ret = QSEECom_shutdown_app(l_QSEEComHandle);
		if (ret) {
			LOGE_PRINT("Shutdown app failed with ret = %d", ret);
		} else {
			LOGD("shutdown app: pass");
		}
	} else {
		LOGE_PRINT("cannot shutdown as the handle is NULL");
	}
	return ret;
}

static int32_t issue_send_cmd(struct QSEECom_handle *l_QSEEComHandle,
                                         struct macchiato_sample_cmd *send_cmd, struct macchiato_sample_cmd_rsp *rsp)
{
	int32_t ret = 0;
	int32_t req_len = 0;
	int32_t rsp_len = 0;
	struct macchiato_sample_cmd_rsp *msgrsp;	/* response data sent from QSEE */

	/* populate the data in shared buffer */
	pthread_mutex_lock(&g_qseecom_cmd_mutex);

	do {
		memcpy(l_QSEEComHandle->ion_sbuffer,send_cmd, sizeof(struct macchiato_sample_cmd));

		req_len = sizeof(struct macchiato_sample_cmd);
		rsp_len = sizeof(struct macchiato_sample_cmd_rsp);

		if (req_len & QSEECOM_ALIGN_MASK)
			req_len = QSEECOM_ALIGN(req_len);

		if (rsp_len & QSEECOM_ALIGN_MASK)
			rsp_len = QSEECOM_ALIGN(rsp_len);

		msgrsp=(struct macchiato_sample_cmd_rsp *)(l_QSEEComHandle->ion_sbuffer + req_len);

		QSEECom_set_bandwidth(l_QSEEComHandle, true);

		/* send request from HLOS to QSEApp */
		ret = QSEECom_send_cmd(l_QSEEComHandle,
				l_QSEEComHandle->ion_sbuffer,
				req_len,
				msgrsp,
				rsp_len);

		QSEECom_set_bandwidth(l_QSEEComHandle, false);

		if (ret) {
			LOGE_PRINT("send command %d failed with ret = %d\n", send_cmd->cmd_id,ret);
			break;
		}

		memcpy(rsp,msgrsp,sizeof(struct macchiato_sample_cmd_rsp));
		ret = msgrsp->status;

	} while(0);

	pthread_mutex_unlock(&g_qseecom_cmd_mutex);
	return ret;
}

/**
@brief print_sub_buffer
Internal function used for printing a sub buffer of the Macchiato message

*/
static void print_sub_buffer(uint8_t* buffer, uint32_t len, const char* title){
	uint32_t i = 0;

	printf("\t %s = ", title);
	for(i=0; i < len; i++){
		printf(" 0x%x",buffer[i]);
	}
	printf("\n");
}
/**
@brief print_macchiato_msg_common_hdr
Internal function used for parsing and printing the common header fields of a Macchiato message:
	 -----------------------------------------
	|      32 bits     | 32 bits |   32 bits  |
	|------------------|---------|------------|
	| Protocol Version | MSG ID  | MSG length |
	 -----------------------------------------
*/
static void print_macchiato_msg_common_hdr(struct macchiato_msg_hdr_common_fields* hdr){
	LOGD_PRINT("\nPrinting Macchiato Message");
	LOGD_PRINT("\t Protocol Version = %d", hdr->u32ProtocolVersion);
	LOGD_PRINT("\t MSG ID = %d", hdr->u32MsgId);
	LOGD_PRINT("\t MSG length = %d", hdr->u32MsgLength);
}

/**
@brief print_macchiato_auth_msg
Internal function used for parsing and printing the fields of authentication Macchiato messages (sign and authenticate).
The format of the authentication message is:
	                                                                         <----------------------------------------------------------------- ECC signature covers this area -------------------------------------------------------------------->
	 -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	|      32 bits     | 32 bits |   32 bits  |   32 bits  |   512 bits    |          512 bits         |           512 bits           | 256 bits |   32 bits   | 32 bits each |  16 bits   |    32 bits    |  128 bits | 128 bits | variable length |
	|------------------|---------|------------|------------|---------------|---------------------------|------------------------------|----------|-------------|--------------|------------|---------------|-----------|----------|-----------------|
	| Protocol Version | MSG ID  | MSG length | Error Code | ECC Signature | Device Public Signing Key | Device Public Encryption Key | TA Hash  | Num Svc IDs |    Svc IDs   | HW Version | Serial Number | Challenge | Response |   Opaque Data   |
	 -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Where for signed message the challenge field will be zeroed
*/
static void print_macchiato_auth_msg(uint8_t * signed_msg){

	struct macchiato_signed_msg_format* signed_msg_common_fields = (struct macchiato_signed_msg_format*) signed_msg;
	struct macciato_signed_msg_additional_fields * signed_msg_additional_fields;
	uint32_t i, read_bytes = 0;

	if(signed_msg_common_fields->signed_msg_hdr.u32MsgLength < sizeof(struct macchiato_signed_msg_format)){
		LOGE_PRINT("Invalid signed message length %d, should be at least %d", signed_msg_common_fields->signed_msg_hdr.u32MsgLength, sizeof(struct macchiato_signed_msg_format));
		return;
	}

	printf("\t Error code: %d\n", signed_msg_common_fields->signed_msg_hdr.u32ErrorCode);

	print_sub_buffer(&(signed_msg_common_fields->uccSignature[0]), MACCHIATO_ECC_SIG_LEN, "ECC Signature");

	print_sub_buffer(&(signed_msg_common_fields->devicePublicSigningKey[0]), MACCHIATO_PUB_KEY_LEN, "Device Public Signing Key");
	print_sub_buffer(&(signed_msg_common_fields->devicePublicEncryptionKey[0]), MACCHIATO_PUB_KEY_LEN, "Device Public Encryption Key");

	print_sub_buffer(&(signed_msg_common_fields->TaHash[0]), TA_HASH_LEN, "TA HASH");

	printf("\t Num Svc IDs: %d\n", signed_msg_common_fields->u32NumSvcIDs);

	if(signed_msg_common_fields->signed_msg_hdr.u32MsgLength < sizeof(struct macchiato_signed_msg_format) + sizeof(uint32_t)*signed_msg_common_fields->u32NumSvcIDs){
		LOGE_PRINT("Invalid signed message length %d, should be at least %d", signed_msg_common_fields->signed_msg_hdr.u32MsgLength, sizeof(struct macchiato_signed_msg_format)+ sizeof(uint32_t)*signed_msg_common_fields->u32NumSvcIDs);
		return;
	}

	for(i=0; i < signed_msg_common_fields->u32NumSvcIDs; i++){
		printf("\t Svc id %d = %d \n",i, ((uint32_t*)(signed_msg+sizeof(struct macchiato_signed_msg_format)))[i]);
	}

	read_bytes += sizeof(struct macchiato_signed_msg_format) + sizeof(uint32_t)*signed_msg_common_fields->u32NumSvcIDs;
	signed_msg_additional_fields = (struct macciato_signed_msg_additional_fields *) (signed_msg + read_bytes);


	printf("\t HW version: %d\n", signed_msg_additional_fields->device_id_fields.u16HwVersion);
	printf("\t Chip Serial Number: %d\n", signed_msg_additional_fields->device_id_fields.u32ChipSerialNum);

	print_sub_buffer(signed_msg_additional_fields->challenge, MACCHIATO_CHALLENGE_RESPONSE_LEN, "Challenge");
	print_sub_buffer(signed_msg_additional_fields->response, MACCHIATO_CHALLENGE_RESPONSE_LEN, "Response");

	read_bytes += sizeof(struct macciato_signed_msg_additional_fields) - 1;
	print_sub_buffer(signed_msg_additional_fields->opaque_data, signed_msg_common_fields->signed_msg_hdr.u32MsgLength - read_bytes, "Opaque data");

}

/**
@brief print_macchiato_msg
Internal function used for parsing and printing Macchiato messages.

*/
static void print_macchiato_msg(uint8_t * msg, uint32_t len){
	struct macchiato_msg_hdr_common_fields* msg_header = (struct macchiato_msg_hdr_common_fields*) msg;

	if(NULL == msg){
		LOGE_PRINT("NULL Macchiato message");
		return;
	}

	if(len < sizeof(struct macchiato_msg_hdr_common_fields)){
		LOGE_PRINT("Too short Macchiato message, length = %d", len);
		return;
	}

	print_macchiato_msg_common_hdr(msg_header);

	switch(msg_header->u32MsgId){

	case MACCHIATO_AUTHENTICATE_MSG_ID:

		print_macchiato_auth_msg(msg);
		break;

	default:
		LOGE_PRINT("Unknown msg number %d", msg_header->u32MsgId);
		return;
	}
}

/**
@brief print_buffer_to_file
Internal function used for printing Macchiato message binary representation to a file.

*/
static void print_buffer_to_file(uint8_t * buffer, uint32_t len, const char * filename){

	FILE *fp = NULL;
	uint32_t written_bytes = 0;

	fp=fopen(filename, "wb");

	if(NULL == fp){
		LOGE_PRINT("failed to open file in %s", filename);
		return;
	}

	written_bytes = fwrite(buffer, sizeof(uint8_t), len, fp);
		if(written_bytes < len){
		LOGE_PRINT("failed to write buffer of length %d to file %s, return value = %d", len, filename, written_bytes);
	}
	fclose(fp);
}

static void fill_buffer_from_file(uint8_t * buffer, uint32_t len, const char * filename){

	FILE *fp = NULL;
	uint32_t read_bytes = 0;

	fp=fopen(filename, "rb");

	if(NULL == fp){
		LOGE_PRINT("failed to open file in %s", filename);
		return;
	}

	read_bytes = fread(buffer, sizeof(uint8_t), len, fp);
		if(read_bytes < len){
		LOGE_PRINT("failed to read buffer of length %d from file %s, return value = %d", len, filename, read_bytes);
	}
	fclose(fp);
}

int main(int argc, char *argv[])
{
	struct QSEECom_handle *l_QSEEComHandle = NULL;
	struct macchiato_sample_cmd cmd = {0};
	struct macchiato_sample_cmd_rsp rsp = {0};
	int32_t ret;
	uint32_t i;

	ret = qsc_start_app(&l_QSEEComHandle,"macchiato_sample", SHARED_BUFFER_SIZE);
	if (ret) {
		LOGE_PRINT("Start app: fail");
		return ret;
	} else {
		LOGD_PRINT("Start app: pass");
	}

	/* test Macchiato signing API */
	cmd.cmd_id = MACCHIATO_SAMPLE_CMD0_SIGN_SERVICE_DATA;

	/* fill service data buffer with arbitrary data*/
	for(i=0; i< MAX_DATA_SIZE; i++){
		cmd.data[i] = i % 256;
	}
	cmd.data_len = MAX_DATA_SIZE;

	ret = issue_send_cmd(l_QSEEComHandle,&cmd, &rsp);
	if(ret < 0){
		LOGD_PRINT("   MACCHIATO_SAMPLE_CMD0_SIGN_SERVICE_DATA returned (%d)",ret);
		return -1;
	}
	/* print parsed Macchiato message */
	print_macchiato_msg(rsp.output, rsp.output_len);

	/* print msg to file */
	print_buffer_to_file(rsp.output, rsp.output_len, SIGNED_MSG_FILE);

	/* test Macchiato authentication API */
	cmd.cmd_id = MACCHIATO_SAMPLE_CMD1_AUTH_CHALLENGE;

	/* read challenge from a file */
	fill_buffer_from_file(cmd.challenge_msg.challenge,MACCHIATO_CHALLENGE_RESPONSE_LEN, CHALLENGE_FILE);

	ret = issue_send_cmd(l_QSEEComHandle,&cmd, &rsp);
	if(ret < 0){
		LOGD_PRINT("   MACCHIATO_SAMPLE_CMD1_AUTH_CHALLENGE returned (%d)",ret);
		return -1;
	}

	/* print parsed Macchiato message */
	print_macchiato_msg(rsp.output, rsp.output_len);

	/* print msg to file */
	print_buffer_to_file(rsp.output, rsp.output_len, AUTH_MSG_FILE);

	ret = qsc_shutdown_app(&l_QSEEComHandle);
	if (ret) {
		LOGE_PRINT("   Failed to shutdown app: %d",ret);
		return ret;
	}
	LOGD_PRINT("shutdown: pass\n");

	return 0;

}
