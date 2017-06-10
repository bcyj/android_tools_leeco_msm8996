/******************************************************************************
  @file    qcril_other.h
  @brief   qcril qmi - misc

  DESCRIPTION
    Handles RIL requests for common software functions an any other
    RIL function that doesn't fall in a different (more specific) category

  ---------------------------------------------------------------------------

  Copyright (c) 2008-2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when         who     what, where, why
--------   ---     ----------------------------------------------------------
08/11/09   nrn      Initial supporint code for NAM programming.

===========================================================================*/

#ifndef QCRIL_OTHER_H
#define QCRIL_OTHER_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "qcril_qmi_client.h"


/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

#define QCRIL_OTHER_CDMAMIN              1
#define QCRIL_OTHER_FMMIN                0
#define QCRIL_OTHER_IMSI_S1_0            16378855
#define QCRIL_OTHER_IMSI_S2_0            999
#define QCRIL_OTHER_IMSI_11_12_0         99
#define QCRIL_OTHER_NID_DEFAULTS         65535
#define QCRIL_OTHER_IMSI_MCC_0           999          /* 000 */
#define QCRIL_OTHER_IMSI_CLASS0_ADDR_NUM 0xFF
#define QCRIL_OTHER_PCH_A_DEFAULT        283          /* Primary Channel A Default */
#define QCRIL_OTHER_PCH_B_DEFAULT        384          /* Primary Channel B Default */
#define QCRIL_OTHER_SCH_A_DEFAULT        691          /* Secondary Channel A Default */
#define QCRIL_OTHER_SCH_B_DEFAULT        777          /* Secondary Channel B Default */
#define QCRIL_OTHER_NID_DEFAULTS         65535

#define QCRIL_OTHER_OEM_NAME_LENGTH      8            /* 8 bytes */
#define QCRIL_OTHER_OEM_REQUEST_ID_LEN   4            /* 4 bytes */
#define QCRIL_OTHER_OEM_REQUEST_DATA_LEN 4            /* 4 bytes */
#define QCRIL_OTHER_OEM_ITEMID_LEN       4            /* 4 bytes */
#define QCRIL_OTHER_OEM_ITEMID_DATA_LEN  4            /* 4 bytes */

/* Macro to check whether a CDMA band-class 0 channel is in the A side.
*/
#define QCRIL_OTHER_PRL_IS_IN_CHAN_CELLULAR_SYS_A( channel ) \
  ( ( ( channel ) >= 1   &&  ( channel ) <= 333 )  ||        \
    ( ( channel ) >= 667 &&  ( channel ) <= 716 )  ||        \
    ( ( channel ) >= 991 &&  ( channel ) <= 1323 ) )

/* Macro to check whether a CDMA band-class 0 channel is in the B side.
*/
#define QCRIL_OTHER_PRL_IS_IN_CHAN_CELLULAR_SYS_B( channel ) \
  ( ( ( channel ) >= 334 && ( channel ) <= 666 ) ||          \
    ( ( channel ) >= 717 && ( channel ) <= 799 ) )

#define QCRIL_OTHER_LTE_NGBR_WCDMA_NUM_FREQS_V01 2
#define QCRIL_OTHER_LTE_NGBR_WCDMA_NUM_CELLS_V01 8
#define QCRIL_OTHER_LTE_NGBR_GSM_NUM_FREQS_V01 2
#define QCRIL_OTHER_LTE_NGBR_GSM_NUM_CELLS_V01 8
#define QCRIL_OTHER_LTE_NGBR_NUM_FREQS_V01 3
#define QCRIL_OTHER_LTE_NGBR_NUM_CELLS_V01 8
#define QCRIL_OTHER_PLMN_LEN_V01 3
#define QCRIL_OTHER_NMR_MAX_NUM_V01 6
#define QCRIL_OTHER_UMTS_MAX_MONITORED_CELL_SET_NUM_V01 24
#define QCRIL_OTHER_UMTS_GERAN_MAX_NBR_CELL_SET_NUM_V01 8


typedef struct
{
  uint32 nv_item;
  uint32 nv_item_size;
  uint32 nv_item_offset;
  char * name;
} qcril_other_nv_table_entry_type;

typedef struct {

  /*  Cell id */
  uint32_t nmr_cell_id;
  /**<   Cell ID (0xFFFFFFFF indicates cell ID information not present)
   */

  /*  PLMN */
  char nmr_plmn[QCRIL_OTHER_PLMN_LEN_V01];
  /**<   MCC/MNC information coded as octet 3, 4, and 5 in [S5, Section 10.5.1.3]
       (this field is ignored when nmr_cell_id is not present)
   */

  /*  LAC */
  uint16_t nmr_lac;
  /**<   Location area code (this field is ignored when nmr_cell_id is not present)
   */

  /*  ARFCN */
  uint16_t nmr_arfcn;
  /**<   Absolute RF channel number
   */

  /*  BSIC */
  uint8_t nmr_bsic;
  /**<   Base station identity code
   */

  /*  Rx Lev */
  uint16_t nmr_rx_lev;
  /**<   Serving cell Rx measurement
   */
}qcril_other_nmr_cell_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcril_other_aggregates
    @{
  */
typedef struct {

  /*  Cell id */
  uint32_t cell_id;
  /**<   Cell ID (0xFFFFFFFF indicates cell ID information not present)
   */

  /*  PLMN */
  char plmn[QCRIL_OTHER_PLMN_LEN_V01];
  /**<   MCC/MNC information coded as octet 3, 4, and 5 in [S5, Section 10.5.1.3]
       (this field is ignored when cell_id is not present)
   */

  /*  LAC */
  uint16_t lac;
  /**<   Location area code (this field is ignored when cell_id not present)
   */

  /*  ARFCN */
  uint16_t arfcn;
  /**<   Absolute RF channel number
   */

  /*  BSIC */
  uint8_t bsic;
  /**<   Base station identity code
   */

  /*  Timing Advance */
  uint32_t timing_advance;
  /**<   Provides the timing advance currently in use on TCH/PDTCH
   */

  /*  Rx Lev */
  uint16_t rx_lev;
  /**<   Serving cell Rx measurement
   */

  /*  Neighbor cell information  */
  uint32_t nmr_cell_info_len;  /**< Must be set to # of elements in nmr_cell_info */
  qcril_other_nmr_cell_info_type_v01 nmr_cell_info[QCRIL_OTHER_NMR_MAX_NUM_V01];
  /**<   Contains information only if neighbors are present;
       includes: \n
       - nmr_cell_id \n
       - nmr_plmn \n
       - nmr_lac \n
       - nmr_arfcn \n
       - nmr_bsic \n
       - nmr_rx_lev
   */
}qcril_other_geran_cell_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcril_other_aggregates
    @{
  */
typedef struct {

  /*  UARFCN */
  uint16_t umts_uarfcn;
  /**<   UTRA absolute RF channel number
   */

  /*  PSC */
  uint16_t umts_psc;
  /**<   Primary scrambling code
   */

  /*  RSCP */
  int16_t umts_rscp;
  /**<   Received signal code power
   */

  /*  Ec/Io */
  int16_t umts_ecio;
  /**<   Ec/Io
   */
}qcril_other_umts_monitored_cell_set_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcril_other_aggregates
    @{
  */
typedef struct {

  /*  UARFCN */
  uint16_t geran_arfcn;
  /**<   Absolute RF channel number
   */

  /*  BSIC NCC */
  uint8_t geran_bsic_ncc;
  /**<   Base station identity code network color code
       (0xFF indicates information not present)
   */

  /*  BSIC BCC */
  uint8_t geran_bsic_bcc;
  /**<   Base station identity code base station color code
       (0xFF indicates information not present)
   */

  /*  GERAN RSSI */
  int16_t geran_rssi;
  /**<   RSSI
   */
}qcril_other_umts_geran_nbr_cell_set_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcril_other_aggregates
    @{
  */
typedef struct {

  /*  Cell id */
  uint16_t cell_id;
  /**<   Cell ID (0xFFFFFFFF indicates cell ID information not present)
   */

  /*  PLMN */
  char plmn[QCRIL_OTHER_PLMN_LEN_V01];
  /**<   MCC/MNC information coded as octet 3, 4, and 5 in [S5, Section 10.5.1.3] (this field is ignored when cell_id is not present)
   */

  /*  LAC */
  uint16_t lac;
  /**<   Location area code (this field is ignored when cell_id is not present)
   */

  /*  UARFCN */
  uint16_t uarfcn;
  /**<   UTRA absolute RF channel number
   */

  /*  PSC */
  uint16_t psc;
  /**<   Primary scrambling code
   */

  /*  RSCP */
  int16_t rscp;
  /**<   Received signal code power
   */

  /*  Ec/Io */
  int16_t ecio;
  /**<   Ec/Io
   */

  /*  UMTS Monitored Cell info set */
  uint32_t umts_monitored_cell_len;  /**< Must be set to # of elements in umts_monitored_cell */
  qcril_other_umts_monitored_cell_set_info_type_v01 umts_monitored_cell[QCRIL_OTHER_UMTS_MAX_MONITORED_CELL_SET_NUM_V01];

  /*  GERAN Neighbor cell info set */
  uint32_t umts_geran_nbr_cell_len;  /**< Must be set to # of elements in umts_geran_nbr_cell */
  qcril_other_umts_geran_nbr_cell_set_info_type_v01 umts_geran_nbr_cell[QCRIL_OTHER_UMTS_GERAN_MAX_NBR_CELL_SET_NUM_V01];
}qcril_other_umts_cell_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcril_other_aggregates
    @{
  */
typedef struct {

  uint16_t sid;
  /**<   System ID  */

  uint16_t nid;
  /**<   Network ID  */

  uint16_t base_id;
  /**<   Base station ID  */

  uint16_t refpn;
  /**<   Reference PN  */

  uint32_t base_lat;
  /**<   Base station latitude  */

  uint32_t base_long;
  /**<   Base station longitude  */
}qcril_other_cdma_cell_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcril_other_aggregates
    @{
  */
typedef struct {

  uint16_t pci;
  /**<   Physical cell ID. Range [0..503]  */

  int16_t rsrq;
  /**<   Current Reference Signal Receive Quality as measured by L1 in 1/10 dB.
    Range [-200..-30]. (e.g., -200 means -20.0 dB)  */

  int16_t rsrp;
  /**<   Current Reference Signal Receive Power in 1/10 dBm as measured by L1.
    Range [-1400..-440]. (e.g., -440 means -44.0 dBm)  */

  int16_t rssi;
  /**<   Current Received Signal Strength Indication in 1/10 dBm as measured by
    L1. Range [-1200..0]. (e.g., -440 means -44.0 dBm)  */

  int16_t srxlev;
  /**<   Cell Selection RX level value (Srxlev). Range [-128..128]
    (only valid when ue_in_idle is TRUE)  */
}qcril_other_lte_ngbr_cell_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcril_other_aggregates
    @{
  */
typedef struct {

  uint8_t ue_in_idle;
  /**<   TRUE if UE is in idle mode. FALSE otherwise.  */

  uint8_t plmn[QCRIL_OTHER_PLMN_LEN_V01];
  /**<   PLMN id coded as octet 3, 4, and 5 in [S5, Section 10.5.1.3]  */

  uint16_t tac;
  /**<   Tracking Area Code  */

  uint32_t global_cell_id;
  /**<   Global cell id in SIB  */

  uint16_t earfcn;
  /**<   E-UTRA Absolute Radio Frequency Number (EARFCN) of serving cell.
       Range [0..65535]  */

  uint16_t serving_cell_id;
  /**<   LTE Serving cell id. This is the cell id of the serving cell and can
    be found in the cell list. Range [0..503]  */

  uint8_t cell_resel_priority;
  /**<   Priority for serving freq. Range [0..7] (only valid when ue_in_idle is
    TRUE)  */

  uint8_t s_non_intra_search;
  /**<   S non intra search threshold to control non-intra freq searches. Range
    [0..31] (only valid when ue_in_idle is TRUE)  */

  uint8_t thresh_serving_low;
  /**<   threshServingLow. Range [0..31] (only valid when ue_in_idle is TRUE)  */

  uint8_t s_intra_search;
  /**<   Threshold current cell meas must fall below to consider intra freq for
    reselection. Range [0..31] (only valid when ue_in_idle is TRUE)  */

  uint32_t cells_len;  /**< Must be set to # of elements in cells */
  qcril_other_lte_ngbr_cell_type_v01 cells[QCRIL_OTHER_LTE_NGBR_NUM_CELLS_V01];
  /**<   Neighbor LTE cell info  */
}qcril_other_lte_intra_freq_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcril_other_aggregates
    @{
  */
typedef struct {

  uint16_t earfcn;
  /**<   E-UTRA Absolute Radio Frequency Number (EARFCN).
       Range [0..65535]  */

  uint8_t threshX_low;
  /**<   Cell Selection RX level value (Srxlev) - -value of an evaluated cell
    must be smaller than this value to be considered for reselection, when
    the serving cell does not exceed thresh serving low. Range [0..31]  */

  uint8_t threshX_high;
  /**<   Cell Selection RX level value (Srxlev)-value of an evaluated cell
    must be greater than this value to be considered for reselection,
    when the serving cell exceeds thresh serving low. Range [0..31]  */

  uint8_t cell_resel_priority;
  /**<   Cell reselection priority. Range [0..7] (only valid when ue_in_idle is
    TRUE)  */

  uint32_t cells_len;  /**< Must be set to # of elements in cells */
  qcril_other_lte_ngbr_cell_type_v01 cells[QCRIL_OTHER_LTE_NGBR_NUM_CELLS_V01];
  /**<   LTE cell parameters  */
}qcril_other_lte_inter_freq_freqs_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcril_other_aggregates
    @{
  */
typedef struct {

  uint8_t ue_in_idle;
  /**<   TRUE if UE is in idle mode. FALSE otherwise.  */

  uint32_t freqs_len;  /**< Must be set to # of elements in freqs */
  qcril_other_lte_inter_freq_freqs_type_v01 freqs[QCRIL_OTHER_LTE_NGBR_NUM_FREQS_V01];
}qcril_other_lte_inter_freq_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcril_other_aggregates
    @{
  */
typedef struct {

  uint16_t arfcn;
  /**<   GSM frequency being reported. Range [0..1023]  */

  uint8_t band_1900;
  /**<   Band indicator for the GSM ARFCN. Only valid if arfcn is in the
    overlapping region. If TRUE and the cell is in the overlapping region it
    is on the 1900 band, if FALSE it is on the 1800 band  */

  uint8_t cell_id_valid;
  /**<   Flag indicating whether the BSIC ID is valid  */

  uint8_t bsic_id;

  /*  BSIC ID including bcc and ncc. Lower 6 bits can be set to any value  */
  int16_t rssi;
  /**<   Measured RSSI value in 1/10 Db. Range [-2000..0]. (e.g., -800 means
    -80.0 Db  */

  int16_t srxlev;
  /**<   Cell Selection RX level value (Srxlev). Range [-128..128]
    (only valid when ue_in_idle is TRUE)  */
}qcril_other_lte_ngbr_gsm_cell_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcril_other_aggregates
    @{
  */
typedef struct {

  uint8_t cell_resel_priority;
  /**<   The priority of this frequency group. Range [0..7] (only valid when
    ue_in_idle is TRUE)  */

  uint8_t thresh_gsm_high;
  /**<   Reselection threshold for high priority layers. Range [0..31] (only
    valid when ue_in_idle is TRUE)  */

  uint8_t thresh_gsm_low;
  /**<   Reselection threshold for low priority layers. Range [0..31] (only
    valid when ue_in_idle is TRUE)  */

  uint8_t ncc_permitted;
  /**<    Bitmask specifying whether a neighbour with a particular Network Color
    Code is to be reported.  Bit "n" set to 1 means that a neighbour with NCC
    "n" should be included in the report. This flag is synonymous with a
    blacklist in other RATs. Range [0..255] (only valid when ue_in_idle is
    TRUE)  */

  uint32_t cells_len;  /**< Must be set to # of elements in cells */
  qcril_other_lte_ngbr_gsm_cell_type_v01 cells[QCRIL_OTHER_LTE_NGBR_GSM_NUM_CELLS_V01];
  /**<   The gsm cell parameters  */
}qcril_other_lte_ngbr_gsm_freq_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcril_other_aggregates
    @{
  */
typedef struct {

  uint8_t ue_in_idle;
  /**<   TRUE if UE is in idle mode. FALSE otherwise.  */

  uint32_t freqs_len;  /**< Must be set to # of elements in freqs */
  qcril_other_lte_ngbr_gsm_freq_type_v01 freqs[QCRIL_OTHER_LTE_NGBR_GSM_NUM_FREQS_V01];
}qcril_other_lte_ngbr_gsm_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcril_other_aggregates
    @{
  */
typedef struct {

  uint16_t psc;
  /**<   Primary Scrambling code. Range [0..511]  */

  int16_t cpich_rscp;
  /**<   Absolute power level of the CPICH as received by the UE in 1/10 dBm.
    Defined in 3GPP TS 25.304. Range [-1200..-250] (e.g., -250 means -25.0 dBm)
    */

  int16_t cpich_ecno;

  int16_t srxlev;
  /**<   Cell Selection RX level value (Srxlev). Range [-128..128]
    (only valid when ue_in_idle is TRUE)  */
}qcril_other_lte_ngbr_wcdma_cell_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcril_other_aggregates
    @{
  */
typedef struct {

  uint16_t uarfcn;
  /**<   wcdma Layer frequency. Range [0..16383]  */

  uint8_t cell_resel_priority;
  /**<   Cell reselection priority. Range [0..7] (only valid when ue_in_idle is
    TRUE)  */

  uint16_t thresh_Xhigh;
  /**<   Reselection threshold. Range [0..31] (only valid when ue_in_idle is
    TRUE)  */

  uint16_t thresh_Xlow;
  /**<   Reselection threshold. Range [0..31] (only valid when ue_in_idle is
    TRUE)  */

  uint32_t cells_len;  /**< Must be set to # of elements in cells */
  qcril_other_lte_ngbr_wcdma_cell_type_v01 cells[QCRIL_OTHER_LTE_NGBR_WCDMA_NUM_CELLS_V01];
  /**<   The wcdma cell parameters  */
}qcril_other_lte_ngbr_wcdma_freq_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcril_other_aggregates
    @{
  */
typedef struct {

  uint8_t ue_in_idle;
  /**<   TRUE if UE is in idle mode. FALSE otherwise.  */

  uint32_t freqs_len;  /**< Must be set to # of elements in freqs */
  qcril_other_lte_ngbr_wcdma_freq_type_v01 freqs[QCRIL_OTHER_LTE_NGBR_WCDMA_NUM_FREQS_V01];
}qcril_other_lte_ngbr_wcdma_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup qcril_other_messages
    @{
  */
/** Response Message; Retrieves cell location-related information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  GERAN Info */
  uint8_t geran_info_valid;  /**< Must be set to true if geran_info is being passed */
  qcril_other_geran_cell_info_type_v01 geran_info;

  /* Optional */
  /*  UMTS Info */
  uint8_t umts_info_valid;  /**< Must be set to true if umts_info is being passed */
  qcril_other_umts_cell_info_type_v01 umts_info;

  /* Optional */
  /*  CDMA Info */
  uint8_t cdma_info_valid;  /**< Must be set to true if cdma_info is being passed */
  qcril_other_cdma_cell_info_type_v01 cdma_info;

  /* Optional */
  /*  LTE Info - LTE Intra-frequency Info */
  uint8_t lte_intra_valid;  /**< Must be set to true if lte_intra is being passed */
  qcril_other_lte_intra_freq_type_v01 lte_intra;

  /* Optional */
  /*  LTE Info - Inter-frequency Info */
  uint8_t lte_inter_valid;  /**< Must be set to true if lte_inter is being passed */
  qcril_other_lte_inter_freq_type_v01 lte_inter;

  /* Optional */
  /*  LTE Info - Neighboring GSM Info */
  uint8_t lte_gsm_valid;  /**< Must be set to true if lte_gsm is being passed */
  qcril_other_lte_ngbr_gsm_type_v01 lte_gsm;

  /* Optional */
  /*  LTE Info - Neighboring WCDMA Info */
  uint8_t lte_wcdma_valid;  /**< Must be set to true if lte_wcdma is being passed */
  qcril_other_lte_ngbr_wcdma_type_v01 lte_wcdma;
}qcril_other_get_cell_location_info_resp_msg_v01;  /* Message */

typedef struct dirList {
    char *dir_name;
    struct dirList *next;
} qcril_other_dirlist;

int qcril_other_ascii_to_int(const char* str, int size);
void qcril_other_int_to_ascii(char* str, int size, int value);
int qcril_other_hex_to_int(char *hex_string,int *number);
int qcril_other_is_number_found(char * number, char *patterns);
void qmi_ril_retrieve_directory_list(char *path, char *subStr, qcril_other_dirlist **dir_list);
void qmi_ril_free_directory_list(qcril_other_dirlist *dir_list);

/***************************************************************************************************
    @function
    qmi_ril_get_property_value_from_string

    @brief
    Reads system property to retrieve the value when the type of the value is string.

    @param[in]
        property_name
            pointer to the name of property that needs to be read
        default_property_value
            pointer to the string that needs to be returned in case the read fails

    @param[out]
        property_value
            pointer to the variable that would hold the read property value

    @retval
    none
***************************************************************************************************/
void qmi_ril_get_property_value_from_string(const char *property_name,
                                            char *property_value,
                                            const char *default_property_value);

/***************************************************************************************************
    @function
    qmi_ril_get_property_value_from_integer

    @brief
    Reads system property to retrieve the value when the type of the value is integer.

    @param[in]
        property_name
            pointer to the name of property that needs to be read
        default_property_value
            value of interger that needs to be returned in case the read fails

    @param[out]
        property_value
            pointer to the variable that would hold the read property value

    @retval
    none
***************************************************************************************************/
void qmi_ril_get_property_value_from_integer(const char *property_name,
                                             int *property_value,
                                             int default_property_value);

/***************************************************************************************************
    @function
        qmi_ril_get_property_value_from_boolean

    @brief
        Reads system property to retrieve the value when the type of the value is boolean.

    @param[in]
        property_name
            pointer to the name of property that needs to be read
        default_property_value
            value of boolean that needs to be returned in case the read fails

    @param[out]
        property_value
            pointer to the variable that would hold the read property value

    @retval
        none
***************************************************************************************************/
void qmi_ril_get_property_value_from_boolean(const char *property_name,
                                             boolean *property_value,
                                             boolean default_property_value);

/***************************************************************************************************
    @function
    qmi_ril_set_property_value_to_string

    @brief
    Writes a value to a system property when the type of the value is string.

    @param[in]
        property_name
            pointer to the name of property that needs to be updated
        property_value
            pointer to the string that needs to be used for setting the value

    @param[out]
        none

    @retval
    E_SUCCESS If set operation is successful, appropriate error code otherwise
***************************************************************************************************/
errno_enum_type qmi_ril_set_property_value_to_string(const char *property_name,
                                                     const char *property_value);

/***************************************************************************************************
    @function
    qmi_ril_set_property_value_to_integer

    @brief
    Writes a value to a system property when the type of the value is integer.

    @param[in]
        property_name
            pointer to the name of property that needs to be updated
        property_value
            value of the integer that needs to be used for setting the value

    @param[out]
        none

    @retval
    E_SUCCESS If set operation is successful, appropriate error code otherwise
***************************************************************************************************/
errno_enum_type qmi_ril_set_property_value_to_integer(const char *property_name,
                                                      int property_value);
/***************************************************************************************************
    @function
    qmi_ril_retrieve_system_time_in_ms

    @brief
    Retrieves the system time in ms. Uses POSIX clocks If available, gettimeofday() otherwise.

    @param[in]
        none

    @param[out]
        none

    @retval
    system time in millisecs
***************************************************************************************************/
uint64_t qmi_ril_retrieve_system_time_in_ms();

#endif /* QCRIL_OTHER_H */
