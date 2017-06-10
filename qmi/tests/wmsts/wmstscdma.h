#ifndef WMSTSCDMA_H
#define WMSTSCDMA_H


/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

          W I R E L E S S    M E S S A G I N G   S E R V I C E S

             ------ CDMA Message Encoding and Decoding Services

GENERAL DESCRIPTION
  This source file contains the encoding and decoding functions of CDMA SMS
  messages.

Copyright (c) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008
              by Qualcomm Technologies, Inc.  All Rights Reserved.

Export of this technology or software is regulated by the U.S. Government.
Diversion contrary to U.S. law prohibited.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/


/* <EJECT> */
/*===========================================================================

$Header:

===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
                        INCLUDE FILES
===========================================================================*/

/*===========================================================================
                        FUNCTIONS
===========================================================================*/

/*=========================================================================
FUNCTION
  wms_ts_encode_CDMA_bd

DESCRIPTION
  This function encodes the bearer data from the client structure to
  its raw format in CDMA mode.

DEPENDENCIES
  None

RETURN VALUE
  Status of the translation

SIDE EFFECTS
  None

=========================================================================*/
wms_status_e_type wms_ts_encode_CDMA_bd
(
  const wms_client_bd_s_type                * cl_bd_ptr,       /* IN */
  wms_raw_ts_data_s_type                    * raw_bd_ptr       /* OUT */
);

/*=========================================================================
FUNCTION
  wms_ts_decode_CDMA_bd

DESCRIPTION
  This function decodes the raw CDMA bearer data into the client's structure.

DEPENDENCIES
  None

RETURN VALUE
  Status of the translation.

SIDE EFFECTS
  None

COMMENTS
  If is_unpacked_user_data = TRUE, then there should be no UDH.

=========================================================================*/
wms_status_e_type  wms_ts_decode_CDMA_bd
(
  const wms_raw_ts_data_s_type       * raw_bd_ptr,   /* IN */
  boolean                            is_unpacked_user_data, /* IN */
  boolean                            decode_other,    /* IN */
  wms_client_bd_s_type               * cl_bd_ptr     /* OUT */
);

wms_status_e_type  wms_ts_decode_CDMA_bd_from_uint8
(
  const uint8                        * p_bd_data,   /* IN */
  const uint32                       len, /* IN */
  boolean                            is_unpacked_user_data, /* IN */
  boolean                            decode_other,     /* IN */
  wms_client_bd_s_type               * cl_bd_ptr       /* OUT */
);
/*=========================================================================
FUNCTION
  wms_ts_encode_bearer_data

DESCRIPTION
  This function encodes the SMS bearer data from the client
  structure to the raw format.

DEPENDENCIES
  None

RETURN VALUE
  Encoding result. WMS_OK_S means encoding succeeded; otherwise it failed.

SIDE EFFECTS
  None

=========================================================================*/
wms_status_e_type wms_ts_encode_bearer_data
(
  const wms_client_bd_s_type                * cl_bd_ptr,       /* IN */
  wms_raw_ts_data_s_type                    * raw_bd_ptr       /* OUT */
);

/*=========================================================================
FUNCTION
  wms_ts_decode_bearer_data

DESCRIPTION
  This function decodes the SMS bearer data from the raw format
  to the client structure.

DEPENDENCIES
  None

RETURN VALUE
  Encoding result. WMS_OK_S means encoding succeeded; otherwise it failed.

SIDE EFFECTS
  None

=========================================================================*/
wms_status_e_type wms_ts_decode_bearer_data
(
  const wms_raw_ts_data_s_type            * raw_bd_ptr,   /* IN */
  wms_client_bd_s_type                    * cl_bd_ptr     /* OUT */
);


wms_status_e_type wms_ts_cdma_cl2OTA
(
  wms_client_message_s_type   *cl_msg,
  uint8                       *data_buf,
  uint16                       data_buf_len,
  uint16                      *ota_data_len
);

/*===========================================================================

FUNCTION    wms_ts_cdma_OTA2cl

DESCRIPTION
  Convert the OTA data to client message format.

DEPENDENCIES
  None

RETURN VALUE
  status of the operation.

SIDE EFFECTS
  *cl_msg modified directly.

===========================================================================*/
wms_status_e_type wms_ts_cdma_OTA2cl
(
  const uint8                 *data,
  uint16                      length,
  wms_client_message_s_type   *cl_msg
);

/*===========================================================================

FUNCTION    wms_ts_unpack_ascii

DESCRIPTION
  Unpack the 7-bit ASCII string from the packed format in a byte array.

DEPENDENCIES
  None

RETURN VALUE
  The string length.

SIDE EFFECTS
  None

===========================================================================*/
uint8 wms_ts_unpack_ascii
(
  const wms_cdma_user_data_s_type    *ud,        /* IN */
  uint8    buf_len,                /* IN */
  uint8    *buf                    /* OUT */
);

#ifdef __cplusplus
}
#endif

#endif /* WMSTSCDMA_H */
