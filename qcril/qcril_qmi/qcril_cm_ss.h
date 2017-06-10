/******************************************************************************
  @file    qcril_cm_ss.h
  @brief   qcril qmi - compatibility layer for CM

  DESCRIPTION
    Contains information related to supplimentary services.

  ---------------------------------------------------------------------------

  Copyright (c) 2008-2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


/*
 * Copyright 2001-2004 Unicode, Inc.
 *
 * Disclaimer
 *
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 *
 * Limitations on Rights to Redistribute This Code
 *
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */


#ifndef QCRIL_CM_SS_H
#define QCRIL_CM_SS_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "comdef.h"


/*===========================================================================

                   EXTERNAL DEFINITIONS AND TYPES

===========================================================================*/
/* TELE-SERVICES */
#define qcril_cm_ss_allTeleservices                    0x00
#define qcril_cm_ss_allSpeechTransmissionservices      0x10
#define qcril_cm_ss_telephony                          0x11
#define qcril_cm_ss_emergencyCalls                     0x12
#define qcril_cm_ss_allShortMessageServices            0x20
#define qcril_cm_ss_shortMessageMT_PP                  0x21
#define qcril_cm_ss_shortMessageMO_PP                  0x22
#define qcril_cm_ss_allFacsimileTransmissionServices   0x60
#define qcril_cm_ss_facsimileGroup3AndAlterSpeech      0x61
#define qcril_cm_ss_automaticFacsimileGroup3           0x62
#define qcril_cm_ss_facsimileGroup4                    0x63
#define qcril_cm_ss_allDataTeleservices                0x70
#define qcril_cm_ss_allTeleservices_ExeptSMS           0x80

/* BEARER-SERVICES */

#define qcril_cm_ss_allBearerServices                 0x00
#define qcril_cm_ss_allDataCDA_Services               0x10
#define qcril_cm_ss_dataCDA_300bps                    0x11
#define qcril_cm_ss_dataCDA_1200bps                   0x12
#define qcril_cm_ss_dataCDA_1200_75bps                0x13
#define qcril_cm_ss_dataCDA_2400bps                   0x14
#define qcril_cm_ss_dataCDA_4800bps                   0x15
#define qcril_cm_ss_dataCDA_9600bps                   0x16
#define qcril_cm_ss_allDataCDS_Services               0x18
#define qcril_cm_ss_dataCDS_1200bps                   0x1A
#define qcril_cm_ss_dataCDS_2400bps                   0x1C
#define qcril_cm_ss_dataCDS_4800bps                   0x1D
#define qcril_cm_ss_dataCDS_9600bps                   0x1E
#define qcril_cm_ss_allPadAccessCA_Services           0x20
#define qcril_cm_ss_padAccessCA_300bps                0x21
#define qcril_cm_ss_padAccessCA_1200bps               0x22
#define qcril_cm_ss_padAccessCA_1200_75bps            0x23
#define qcril_cm_ss_padAccessCA_2400bps               0x24
#define qcril_cm_ss_padAccessCA_4800bps               0x25
#define qcril_cm_ss_padAccessCA_9600bps               0x26
#define qcril_cm_ss_allDataPDS_Services               0x28
#define qcril_cm_ss_dataPDS_2400bps                   0x2C
#define qcril_cm_ss_dataPDS_4800bps                   0x2D
#define qcril_cm_ss_dataPDS_9600bps                   0x2E
#define qcril_cm_ss_allAlternateSpeech_DataCDA        0x30
#define qcril_cm_ss_allAlternateSpeech_DataCDS        0x38
#define qcril_cm_ss_allSpeechFollowedByDataCDA        0x40
#define qcril_cm_ss_allSpeechFollowedByDataCDS        0x48
#define qcril_cm_ss_allDataCircuitAsynchronous        0x50
#define qcril_cm_ss_allAsynchronousServices           0x60
#define qcril_cm_ss_allDataCircuitSynchronous         0x58
#define qcril_cm_ss_allSynchronousServices            0x68

#define  QCRIL_CM_SS_UPCASE( c ) ( ((c) >= 'a' && (c) <= 'z') ? ((c) - 0x20) : (c) )

#define QCRIL_CM_SS_TA_UNKNOWN       129 /* 0x80|CM_TON_UNKNOWN      |CM_NPI_ISDN */
#define QCRIL_CM_SS_TA_INTERNATIONAL 145 /* 0x80|CM_TON_INTERNATIONAL|CM_NPI_ISDN */
#define QCRIl_CM_SS_TA_INTER_PREFIX  '+' /* ETSI international call dial prefix */
#define CHAR_CR           0x0D
#define MAX_MT_USSD_CHAR    183  /* (160*8)/7 Max num of char is MT USS data */
#define QCRIL_CM_SS_USS_DEF_ALPHABET_LANG_UNSPEC     0x0F  /*default coding scheme */

#define   QCRIL_CM_SS_INVOKE_PROBLEM                        0x81
#define   QCRIL_CM_SS_UNRECOGNISED_OPERATION                0x01
#define   QCRIL_CM_SS_FACILITY_REJECTED                     29


/* Possible combinations for the class values for supplimentary services
    taken from ATCOP spec 27.007 */
typedef enum qcril_cm_ss_class_e
{
  QCRIL_CM_SS_CLASS_MIN                 = 0x0,
  QCRIL_CM_SS_CLASS_VOICE               = 0x1,
  QCRIL_CM_SS_CLASS_DATA                = 0x2,
  QCRIL_CM_SS_CLASS_FAX                 = 0x4,
  QCRIL_CM_SS_CLASS_SMS                 = 0x8,
  QCRIL_CM_SS_ALL_TELE_SERV             = ( QCRIL_CM_SS_CLASS_VOICE |
                                            QCRIL_CM_SS_CLASS_SMS |
                                            QCRIL_CM_SS_CLASS_FAX ),
  QCRIL_CM_SS_ALL_TELE_SERV_EX_SMS      = ( QCRIL_CM_SS_CLASS_VOICE |
                                            QCRIL_CM_SS_CLASS_FAX ),
  QCRIL_CM_SS_CLASS_ALL_TS_DATA         = ( QCRIL_CM_SS_CLASS_FAX |
                                            QCRIL_CM_SS_CLASS_SMS ),
  QCRIL_CM_SS_CLASS_DATA_SYNC           = 0x10,
  QCRIL_CM_SS_CLASS_DATA_ASYNC          = 0x20,
  QCRIL_CM_SS_CLASS_DATA_PKT            = 0x40,
  QCRIL_CM_SS_CLASS_ALL_DATA_SYNC       = ( QCRIL_CM_SS_CLASS_DATA_SYNC |
                                            QCRIL_CM_SS_CLASS_DATA_PKT ),
  QCRIL_CM_SS_CLASS_DATA_PAD            = 0x80,
  QCRIL_CM_SS_CLASS_ALL_DATA_ASYNC      = ( QCRIL_CM_SS_CLASS_DATA_ASYNC |
                                            QCRIL_CM_SS_CLASS_DATA_PAD),
  QCRIL_CM_SS_CLASS_ALL_DATA_SYNC_ASYNC = ( QCRIL_CM_SS_CLASS_DATA_SYNC |
                                            QCRIL_CM_SS_CLASS_DATA_ASYNC ),
  QCRIL_CM_SS_CLASS_ALL_DATA_PDS        = ( QCRIL_CM_SS_CLASS_DATA_SYNC |
                                            QCRIL_CM_SS_CLASS_VOICE ),
  QCRIL_CM_SS_CLASS_DATA_ALL            = 0xF0,
  QCRIL_CM_SS_CLASS_ALL                 = 0xFF,
  QCRIL_CM_SS_CLASS_MAX                 = 0xFFFF
} qcril_cm_ss_class_e_type;

/* Call Independent supplimenary services modes
     like de-activation, activation, registration , erasure etc..
*/
typedef enum qcril_cm_ss_mode_e
{
  QCRIL_CM_SS_MODE_DISABLE    = 0,
  QCRIL_CM_SS_MODE_ENABLE     = 1,
  QCRIL_CM_SS_MODE_QUERY      = 2,
  QCRIL_CM_SS_MODE_REG        = 3,
  QCRIL_CM_SS_MODE_ERASURE    = 4,
  QCRIL_CM_SS_MODE_REG_PASSWD = 5,
  QCRIL_CM_SS_MODE_MAX
} qcril_cm_ss_mode_e_type;

/* different types of call forwarding SS */
typedef enum qcril_cm_ss_ccfc_reason_e
{
  QCRIL_CM_SS_CCFC_REASON_UNCOND    = 0,
  QCRIL_CM_SS_CCFC_REASON_BUSY      = 1,
  QCRIL_CM_SS_CCFC_REASON_NOREPLY   = 2,
  QCRIL_CM_SS_CCFC_REASON_NOTREACH  = 3,
  QCRIL_CM_SS_CCFC_REASON_ALLCALL   = 4,
  QCRIL_CM_SS_CCFC_REASON_ALLCOND   = 5,
  QCRIL_CM_SS_CCFC_REASON_MAX
} qcril_cm_ss_ccfc_reason_e_type;

typedef enum qcril_cm_ss_supps_notification_mo_e
{
  QCRIL_CM_SS_CSSI_ORIG_FWD_STAT         = -1,
  QCRIL_CM_SS_CSSI_UNCOND_FWD_ACTIVE     = 0,
  QCRIL_CM_SS_CSSI_COND_FWD_ACTIVE       = 1,
  QCRIL_CM_SS_CSSI_CALL_FORWARDED        = 2,
  QCRIL_CM_SS_CSSI_CALL_WAITING          = 3,
  QCRIL_CM_SS_CSSI_CUG_CALL              = 4,
  QCRIL_CM_SS_CSSI_OUTGOING_CALLS_BARRED = 5,
  QCRIL_CM_SS_CSSI_INCOMING_CALLS_BARRED = 6,
  QCRIL_CM_SS_CSSI_CLIR_SUPPRESSION_REJ  = 7,
  QCRIL_CM_SS_CSSI_CALL_DEFLECTED        = 8,
  QCRIL_CM_SS_CSSI_MAX
}qcril_cm_ss_supps_notification_mo_e_type;

typedef enum qcril_cm_ss_supps_notification_mt_e
{
  QCRIL_CM_SS_CSSU_FORWARDED_CALL                = 0,
  QCRIL_CM_SS_CSSU_CUG_CALL                      = 1,
  QCRIL_CM_SS_CSSU_CALL_HOLD                     = 2,
  QCRIL_CM_SS_CSSU_CALL_RETRIEVED                = 3,
  QCRIL_CM_SS_CSSU_MPTY_CALL                     = 4,
  QCRIL_CM_SS_CSSU_CALL_HOLD_RELEASED            = 5,
  QCRIL_CM_SS_CSSU_FWD_CHECK_SS_RECVD            = 6,
  QCRIL_CM_SS_CSSU_ECT_CALL_REMOTE_PTY_ALERT     = 7,
  QCRIL_CM_SS_CSSU_ECT_CALL_REMOTE_PTY_CONNECTED = 8,
  QCRIL_CM_SS_CSSU_DEFLECTED_CALL                = 9,
  QCRIL_CM_SS_CSSU_ADDITIONAL_INCOM_CALL_FWD     = 10,
  QCRIL_CM_SS_CSSU_MAX
}qcril_cm_ss_supps_notification_mt_e_type;

typedef enum qcril_cm_ss_notification_ect_call_state_e
{
  qcril_cm_ss_notification_alerting_ECT,
  qcril_cm_ss_notification_alerting_active_ECT

} qcril_cm_ss_notification_ect_call_state_e_type;

typedef enum qcril_cm_ss_notification_type_e
{
 QCRIL_CM_SS_MIN_NOTIFICATION = -1,
 QCRIL_CM_SS_MO_NOTIFICATION = 0,
 QCRIL_CM_SS_MT_NOTIFICATION = 1,
 QCRIL_CM_SS_MAX_NOTIFICATION
}qcril_cm_ss_notification_type_e_type;

#define QCRIL_CM_SS_CCFC_NUM_MAX_LEN 80

/*  Network SS Codes taken from mn_cm_exp.h
*/

typedef enum qcril_cm_ss_operation_code_e{
       qcril_cm_ss_all                     =   0x00,
       qcril_cm_ss_allLineIdentificationSS =   0x10, /* all_line_identification */
       qcril_cm_ss_clip                    =   0x11, /* calling_line_identification_
                                                         presentation_service */
       qcril_cm_ss_clir                    =   0x12, /* calling_line_identification_
                                                         restriction_service
                                                presentation_service */
       qcril_cm_ss_colp                    =   0x13, /* connected_line_identification_
                                            presentation_service */
       qcril_cm_ss_colr                    =   0x14, /* connected_line identification_
                                             restriction_service */
       qcril_cm_ss_cnap                    =   0x19, /* call name identification presentation */
       qcril_cm_ss_mci                     =   0x1a, /* malicious call identify */
       qcril_cm_ss_allForwardingSS         =   0x20, /* all_call_forwarding */
       qcril_cm_ss_cfu                     =   0x21, /*  call_forwarding_unconditional */
       qcril_cm_ss_cd                      =   0x24, /* call deflection */
       qcril_cm_ss_allCondForwardingSS     =   0x28, /* all conditional call forwarding  */
       qcril_cm_ss_cfb                     =   0x29, /* call_forwarding_on_mobile_sub-
                                                scriber_busy */
       qcril_cm_ss_cfnry                   =   0x2a, /* call_forwarding_on_no_reply */
       qcril_cm_ss_cfnrc                   =   0x2b, /* call_forwarding_on_mobile_subsc-
                                                riber_unreachable */
       qcril_cm_ss_allCallOfferingSS       =   0x30, /* all call offering sub-services*/
       qcril_cm_ss_ect                     =   0x31, /* call transfer */
       qcril_cm_ss_mah                     =   0x32, /* mobile access hunting */
       qcril_cm_ss_allCallCompletionSS     =   0x40, /* all call completion */
       qcril_cm_ss_cw                      =   0x41, /* call waiting */
       qcril_cm_ss_hold                    =   0x42,
       qcril_cm_ss_ccbs                    =   0x43, /* completion of call to busy
                                                                 subscribers */
       qcril_cm_ss_allMultiPartySS         =   0x50, /* all multi-party services */
       qcril_cm_ss_multiPTY                =   0x51, /* multi_party_service */
       qcril_cm_ss_allCommunityOfInterest_SS  = 0x60, /* *** NB name deviates from
                                               09.02 ***/
       qcril_cm_ss_cug                     =   0x61, /* closed_user_group */
       qcril_cm_ss_allChargingSS           =   0x70,

       qcril_cm_ss_aoci                    =   0x71, /* advice_of_charge_information */
       qcril_cm_ss_aocc                    =   0x72, /* advice_of_charge_charge */

       qcril_cm_ss_allAdditionalInfoTransferSS = 0x80,
       qcril_cm_ss_uus                     =   0x81, /* user to user signalling */
       qcril_cm_ss_allCallRestrictionSS    =   0x90,
       qcril_cm_ss_barringOfOutgoingCalls  =   0x91,
       qcril_cm_ss_baoc                    =   0x92, /* barring_of_all_outgoing_calls */
       qcril_cm_ss_boic                    =   0x93, /* barring_of_outgoing_inter-
                                                national_calls */
       qcril_cm_ss_boicExHC                =   0x94, /* barring_of_outgoing_inter-
                                                national_calls_except_those_
                                                to_home_plmn */
       qcril_cm_ss_barringOfIncomingCalls  =   0x99,
       qcril_cm_ss_baic                    =   0x9a, /* barring of all incoming calls */
       qcril_cm_ss_bicRoam                 =   0x9b, /* barring of incomming calls when
                                                roaming outside home PLMN
                                                Country */
       qcril_cm_ss_allPLMN_specificSS      =   0xf0, /* all PLMN specific Supplementary
                                                services *** NB name deviates
                                                from 09.02 ***/
       qcril_cm_ss_chargingInfoId          =   0xa1
} qcril_cm_ss_operation_code_e_type;


/*  facility values for FACLITY LOCK supplimentary services
    taken from 27.007 7.4 section
*/
typedef enum qcril_cm_ss_facility_lock_string_code_e
{
  QCRIL_CM_SS_LOCK_CS    = 0,
  QCRIL_CM_SS_LOCK_PS    = 1,
  QCRIL_CM_SS_LOCK_PF    = 2,
  QCRIL_CM_SS_LOCK_SC    = 3,
  QCRIL_CM_SS_LOCK_AO    = 4,
  QCRIL_CM_SS_LOCK_OI    = 5,
  QCRIL_CM_SS_LOCK_OX    = 6,
  QCRIL_CM_SS_LOCK_AI    = 7,
  QCRIL_CM_SS_LOCK_IR    = 8,
  QCRIL_CM_SS_LOCK_NT    = 9,
  QCRIL_CM_SS_LOCK_NM    = 10,
  QCRIL_CM_SS_LOCK_NS    = 11,
  QCRIL_CM_SS_LOCK_NA    = 12,
  QCRIL_CM_SS_LOCK_AB    = 13,
  QCRIL_CM_SS_LOCK_AG    = 14,
  QCRIL_CM_SS_LOCK_AC    = 15,
  QCRIL_CM_SS_LOCK_FD    = 16,
  QCRIL_CM_SS_LOCK_PN    = 17,
  QCRIL_CM_SS_LOCK_PU    = 18,
  QCRIL_CM_SS_LOCK_PP    = 19,
  QCRIL_CM_SS_LOCK_PC    = 20,
  QCRIL_CM_SS_LOCK_MAX
} qcril_cm_ss_facility_lock_string_code_e_type;

/* This enum represents the CLIR provisioning status at the network */
typedef enum qcril_cm_ss_clir_status_e {
  QCRIL_CM_SS_CLIR_SRV_NOT_PROVISIONED         = 0,
  QCRIL_CM_SS_CLIR_SRV_PROVISIONED_PERMANENT   = 1,
  QCRIL_CM_SS_CLIR_SRV_NO_NETWORK              = 2,
  QCRIL_CM_SS_CLIR_SRV_PRESENTATION_RESTRICTED = 3,
  QCRIL_CM_SS_CLIR_SRV_PRESENTATION_ALLOWED    = 4
} qcril_cm_ss_clir_status_e_type;

/* CLIR enable/disable settings */
typedef enum qcril_cm_ss_clir_type_e{
  QCRIl_CM_SS_CLIR_PRESENTATION_INDICATOR = 0,
  QCRIL_CM_SS_CLIR_INVOCATION_OPTION  = 1,
  QCRIL_CM_SS_CLIR_SUPPRESSION_OPTION = 2
}qcril_cm_ss_clir_type_e_type;

/* SVC enable/disable notification settings */
typedef enum qcril_cm_ss_supps_notification_e{
  QCRIl_CM_SS_DISABLE_NOTIFICATION = 0,
  QCRIL_CM_SS_ENABLE_NOTIFICATION = 1
}qcril_cm_ss_supps_notification_e_type;

/* States for USSD */
//typedef enum qcril_cm_ss_ussd_state_e
//{
//  QCRIL_CM_SS_USSD_STATE_NULL,            /* No pending USSD transaction */
//  QCRIL_CM_SS_USSD_STATE_MO_REQ,          /* MS initiated request  */
//  QCRIL_CM_SS_USSD_STATE_MT_REQ,          /* MT initiated request */
//  QCRIL_CM_SS_USSD_STATE_MT_NOTIFY,       /* MT initiated notification */
//  QCRIL_CM_SS_USSD_STATE_MS_ABORT,        /* MS abort of transaction     */
//  QCRIL_CM_SS_USSD_STATE_MAX
//} qcril_cm_ss_ussd_state_e_type;


typedef enum qcril_cm_ss_ussd_result_e
{
  QCRIL_CM_SS_CUSD_RESULT_DONE    = 0,    /* No further action required   */
  QCRIL_CM_SS_CUSD_RESULT_MORE    = 1,    /* Further user action required */
  QCRIL_CM_SS_CUSD_RESULT_ABORT   = 2,    /* USSD terminated by network   */
  QCRIL_CM_SS_CUSD_RESULT_OTHER   = 3,    /* Other local client responded */
  QCRIL_CM_SS_CUSD_RESULT_NOSUP   = 4,    /* Operation not supported      */
  QCRIL_CM_SS_CUSD_RESULT_TIMEOUT = 5,    /* Network time out             */
  QCRIL_CM_SS_CUSD_RESULT_MAX
} qcril_cm_ss_ussd_result_e_type;

/* supplimentary services operations taken from mn_cm_exp.h */
#define qcril_cm_ss_processUnstructuredSS_Request   59
#define qcril_cm_ss_unstructuredSS_Request          60
#define qcril_cm_ss_unstructuredSS_Notify           61

/* For mapping the ATCOP class value to basic service type and code
    which is recognized by the network
*/
typedef struct qcril_cm_ss_bs_mapping_s
{
  qcril_cm_ss_class_e_type bs_class;
  uint8  bs_type;   /* BS type - bearer services, teleservices */
  uint8  bs_code;   /* BS Code */
} qcril_cm_ss_bs_mapping_s_type;

#define MAX_PWD_CHAR                         4     // replicated from cm.h

typedef struct
{
   boolean                       present;
   char                          password[MAX_PWD_CHAR];

} qcril_cm_ss_password_T;


/* Call forwarding information passed from ANDROID RIL, defined as per design doc */
typedef struct qcril_cm_ss_callforwd_info_param_u
{
  int         status;
  int         reason;           /* "reason" from 27.007 7.11              */
  int         service_class;    /* "class" for CCFC/CLCK from 27.007  */
  int         toa;              /* type of address 27.007 7.11           */
  char        *number;          /* "number" from  27.007 7.11           */
  int         no_reply_timer;   /*  CFU timer */
} qcril_cm_ss_callforwd_info_param_u_type;

/* Data for a call forwarding registration */
typedef struct qcril_cm_ss_reg_data_s
{
  char *number;       /* call forwarding number */
  int nr_timer;    /*no reply timer*/
} qcril_cm_ss_reg_data_s_type;

/* Password information during password change or barring related sups */
typedef struct qcril_cm_ss_change_passwd_s
{
  char *old_passwd;
  char *new_passwd;
  char *new_passwd_again;
} qcril_cm_ss_change_passwd_s_type;

/* Information required to construct a sups string */
typedef struct qcril_cm_ss_sups_params_s
{
  qcril_cm_ss_mode_e_type             mode;            /* Indicates enable/disable etc */
  qcril_cm_ss_operation_code_e_type   code;            /* Sups network service code */
  qcril_cm_ss_class_e_type            service_class;   /* Service class as per 27.007 */
  union
  {
    qcril_cm_ss_reg_data_s_type         reg;             /* Registration data */
    qcril_cm_ss_change_passwd_s_type    passwd;          /* Password information */
  } req;
} qcril_cm_ss_sups_params_s_type;

/* Holds MMI Service code and corresponding Network service code */
typedef struct qcril_cm_ss_sc_table_s
{
  char *mmi_sc;       /* MMI value of Service Code */
  byte net_sc;      /* Network value of Service Code */
} qcril_cm_ss_sc_table_s_type;

/* Holds MMI BSG code and corresponding Service class */
typedef struct qcril_cm_ss_bsg_table_s
{
  char *mmi_bsg;           /* MMI value of Basic Service Group */
  byte service_class;    /* Service Class as per 27.007 */
} qcril_cm_ss_bsg_table_s_type;

/* Holds Sups type string corresponding to the Sups mode*/
typedef struct qcril_cm_ss_mode_table_s
{
  char *sups_mode_str;                /* Sups operation type string */
  qcril_cm_ss_mode_e_type sups_mode;  /* Indicates enable/disable etc */
} qcril_cm_ss_mode_table_s_type;

/* Maximum possible digits for an integer (of 4 bytes) in decimal representation */
#define QCRIL_CM_SS_MAX_INT_DIGITS        10

/* Maximum sups string length */
#define QCRIL_CM_SS_MAX_SUPS_LENGTH       128

/* Defining the variables required for converting the USSD strings from
   UTF8 to UCS2 and vice versa */

typedef unsigned long  UTF32; /* at least 32 bits */
typedef unsigned short UTF16; /* at least 16 bits */
typedef unsigned char  UTF8;  /* typically 8 bits */

/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD
#define UNI_MAX_BMP (UTF32)0x0000FFFF
#define UNI_MAX_UTF16 (UTF32)0x0010FFFF
#define UNI_MAX_UTF32 (UTF32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (UTF32)0x0010FFFF

#define UNI_SUR_HIGH_START  (UTF32)0xD800
#define UNI_SUR_HIGH_END    (UTF32)0xDBFF
#define UNI_SUR_LOW_START   (UTF32)0xDC00
#define UNI_SUR_LOW_END     (UTF32)0xDFFF

typedef enum {
  conversionOK,     /* conversion successful */
  sourceExhausted,  /* partial character in source, but hit end */
  targetExhausted,  /* insuff. room in target for conversion */
  sourceIllegal    /* source sequence is illegal/malformed */
} ConversionResult;

typedef enum {
  strictConversion = 0,
  lenientConversion
} ConversionFlags;

/* All the above definitions are required for converting from UTF8 to UCS2 and
   Vice Versa */

/* USSD GSM 7-bit coding scheme mask */
#define QCRIL_CM_SS_USSD_DCS_7_BIT_MASK         0x00
/* USSD UCS2 coding preceded by language preference scheme mask */
#define QCRIL_CM_SS_USSD_DCS_UCS2_LANG_IND_MASK 0x01
/* USSD 8-bit coding scheme mask */
#define QCRIL_CM_SS_USSD_DCS_8_BIT_MASK         0x04
/* USSD UCS2 coding scheme mask */
#define QCRIL_CM_SS_USSD_DCS_UCS2_MASK          0x08

/** Enumeration of Data coding schemes used by USSD
*/
/* Clients need to check for CM_API_USSD_DCS before
** using this enumeration
*/
typedef enum qcril_cm_ss_ussd_dcs_e {

  QCRIL_CM_SS_USSD_DCS_NONE = -1,
    /**< @internal */

  QCRIL_CM_SS_USSD_DCS_7_BIT = 0x00,
    /**< 7 bit Data encoding scheme used for ussd */

  QCRIL_CM_SS_USSD_DCS_8_BIT = 0x04,
    /**< 8 bit Data encoding scheme used for ussd */

  QCRIL_CM_SS_USSD_DCS_UCS2  = 0x08,
    /**< Universal multi-octet character set encoding
    ** Clients need to check for CM_API_USSD_DCS
    ** and CM_API_USSD_UCS2_DCS
    */

  QCRIL_CM_SS_USSD_DCS_UNSPECIFIED = 0x0F,
    /**< Data encoding scheme unspecified */

  CM_USSD_DCS_MAX

} qcril_cm_ss_ussd_dcs_e_type;

/* These two extern declarations are needed here for using
   them in qcril_cm.c file */
extern  qcril_cm_ss_bs_mapping_s_type qcril_cm_ss_bs_mapping_table[];
extern  uint32 qcril_cm_ss_cw_allowed_classes;

/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/

boolean qcril_cm_ss_req_set_call_wait_is_valid( unsigned int length, int status, int service_class );

boolean qcril_cm_ss_req_set_call_fwd_is_valid( unsigned int in_data_len, qcril_cm_ss_callforwd_info_param_u_type *in_data_ptr );

boolean qcril_cm_ss_req_set_fac_lck_is_valid( unsigned int in_data_len, int facility, int status, const char *password, int service_class );

boolean qcril_cm_ss_req_change_cb_pwd_is_valid( unsigned int in_data_len, int facility, const char *old_password, const char *new_password );

boolean qcril_cm_ss_query_facility_lock_is_valid( unsigned int in_data_len, int facility, const char *password, int service_class );

boolean qcril_cm_ss_facility_value_is_valid( int facility, int status );

uint8 qcril_cm_ss_get_ss_ref( void );

uint8  qcril_cm_ss_get_cfw_ss_code( int reason, char **cf_reason_name );

int  qcril_cm_ss_get_cfw_reason( int ss_code );

int  qcril_cm_ss_get_cb_ss_code( int facility );

int  qcril_cm_ss_get_facility_value( const char * facility, char *facility_name );

void qcril_cm_ss_add_service_class_based_on_number( qcril_cm_ss_callforwd_info_param_u_type *a[], int *LEN );

void qcril_cm_ss_convert_utf8_to_ucs2( const char *rcvd_ussd_str, char *ussd_str, int *length );

uint8 qcril_cm_ss_convert_ucs2_to_utf8( char *ussData, int size, char *ussd_str );

ConversionResult qcril_cm_ss_ConvertUTF8toUTF16( const UTF8** sourceStart, const UTF8* sourceEnd,
                                                 UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags );

ConversionResult qcril_cm_ss_ConvertUTF16toUTF8( const UTF16** sourceStart, const UTF16* sourceEnd,
                                                 UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags );

IxErrnoType qcril_cm_ss_build_sups_string( qcril_cm_ss_sups_params_s_type* ss_params, char* buf, int max_len );

uint16 qcril_cm_ss_ascii_to_utf8( unsigned char* ascii_str, uint8 ascii_len, char* utf_str, uint16 utf_len );

boolean qcril_cm_ss_UssdStringIsAscii( const char *string );

uint16 qcril_cm_ss_convert_gsm_def_alpha_string_to_utf8( const char *gsm_data, byte gsm_data_len, char *utf8_buf );

uint16 qcril_cm_ss_convert_gsm8bit_alpha_string_to_utf8( const char *gsm_data, uint16 gsm_data_len, char *utf8_buf );

uint16 qcril_cm_ss_convert_gsm_def_alpha_unpacked_to_utf8 ( const char *gsm_data, byte gsm_data_len, char *utf8_buf);

uint16 qcril_cm_ss_convert_ussd_string_to_utf8( byte uss_data_coding_scheme, byte uss_data_len, const byte *uss_data, char *utf8_buf );


void qcril_cm_ons_decode_packed_7bit_gsm_string
(
  char *dest,
  const uint8 *src,
  uint8 src_length
);

#endif /* QCRIL_CM_SS_H */
