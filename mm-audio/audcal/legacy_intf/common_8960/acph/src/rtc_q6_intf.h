#ifndef _RTC_Q6_INTF_H_
#define _RTC_Q6_INTF_H_
/*============================================================================

FILE:       rtc_q6_intf.h

DESCRIPTION: 

PUBLIC CLASSES:  Not Applicable

INITIALIZATION AND SEQUENCING REQUIREMENTS:  N/A

Copyright (c) 2010-2011 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*! \brief	Domain */
#define PC_DOMAIN  							(2)
#define MODEM_DOMAIN  					    (2)
#define ADSP_DOMAIN							(4)
#define APPS_DOMAIN						    (5)

/*! \brief local services */
#define RTC_INTF_SERVICE_DM					(0x03)
#define RTC_INTF_SERVICE_AS					(0x04)
#define RTC_INTF_SERVICE_AC					(0x05)
#define RTC_INTF_SERVICE_VS					(0x06)
#define RTC_INTF_SERVICE_VC					(0x07)
#define RTC_INTF_SERVICE_ACDB      			(0x08)

#define RTC_INTF_AC_ADDR                    ((APPS_DOMAIN<<8)|RTC_INTF_SERVICE_AC)
#define RTC_INTF_AS_ADDR                    ((APPS_DOMAIN<<8)|RTC_INTF_SERVICE_AS)
#define RTC_INTF_DM_ADDR                    ((APPS_DOMAIN<<8)|RTC_INTF_SERVICE_DM)
#define RTC_INTF_VC_ADDR                    ((APPS_DOMAIN<<8)|RTC_INTF_SERVICE_VC)
#define RTC_INTF_VS_ADDR                    ((APPS_DOMAIN<<8)|RTC_INTF_SERVICE_VS)
#define RTC_INTF_ACDB_ADDR                  ((APPS_DOMAIN<<8)|RTC_INTF_SERVICE_ACDB)
#define RTCPCINTF_ACDB_ADDR                 ((PC_DOMAIN<<8)|RTC_INTF_SERVICE_ACDB)

/** @brief  Q6 services */
#define ADSP_CORE_SERVICE                   (0x03)
#define ADSP_AFE_SERVICE                    (0x04)
#define ADSP_VSM_SERVICE                    (0x05)
#define ADSP_VPM_SERVICE                    (0x06)
#define ADSP_ASM_SERVICE                    (0x07)
#define ADSP_ADM_SERVICE                    (0x08)

#define ADSP_AFE_ADDR                       ((ADSP_DOMAIN<<8)|ADSP_AFE_SERVICE)
#define ADSP_ADM_ADDR                       ((ADSP_DOMAIN<<8)|ADSP_ADM_SERVICE)
#define ADSP_ASM_ADDR                       ((ADSP_DOMAIN<<8)|ADSP_ASM_SERVICE)
#define ADSP_VPM_ADDR                       ((ADSP_DOMAIN<<8)|ADSP_VPM_SERVICE)
#define ADSP_VSM_ADDR                       ((ADSP_DOMAIN<<8)|ADSP_VSM_SERVICE)

/*! \brief	Q6 message version */
#define Q6MSG_VERSION						(0)

/*! \brief	Q6 message type */
#define Q6MSG_TYPE_RSP						(1)
#define Q6MSG_TYPE_EVT						(0)
#define Q6MSG_TYPE_SQCMD					(2)
#define Q6MSG_TYPE_NSCMD					(3)

#define RTC_INTF_SUCCESS                    0
#define RTC_INTF_EFAILURE                   0x21
#define RTC_INTF_EBADPARAM                  0x22
#define RTC_INTF_ETIMEOUT                   0x23
#define RTC_INTF_ENORESOURCE                0x24
#define RTC_INTF_ESHMEM                     0x25

int32_t rtc_q6_intf_init(void);

int32_t rtc_q6_intf_deinit(void);

void rtc_get_q6_cal_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        );

void rtc_set_q6_cal_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        );

void rtc_get_q6_cal_data_shmem (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        );

void rtc_set_q6_cal_data_shmem (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        );

int32_t rtc_q6_intf_send (
        void* send_buf,        /* sending buf ptr */
        void* rsp_buf,         /* rsp buf ptr for sync send only */
        uint32_t* rsp_buf_len /* rsp buf length in bytes */
        );

int32_t rtc_q6_intf_send_get_shmem (
        void* send_buf,        /* sending buf ptr */
        void* rsp_buf,         /* rsp buf ptr for sync send only */
        uint32_t* rsp_buf_len /* rsp buf length in bytes */
        );

int32_t rtc_q6_intf_send_set_shmem (
        void* send_buf,        /* sending buf ptr */
        void* rsp_buf,         /* rsp buf ptr for sync send only */
        uint32_t* rsp_buf_len /* rsp buf length in bytes */
        );

#endif /* _RTC_Q6_INTF_H_ */

