/******************************************************************************
  @file    qcril_qmi_nas2.c
  @brief   qcril qmi - NAS 2nd portion

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI NAS.

  ---------------------------------------------------------------------------

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <errno.h>
#include <cutils/memory.h>
#include <cutils/properties.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ril.h"
#include "IxErrno.h"
#include "comdef.h"
#include "qcrili.h"
#include "qcril_reqlist.h"
#include "qcril_arb.h"
#include "qcril_qmi_nas2.h"
#include "qcril_qmi_nas.h"
#include "voice_service_v02.h"
#include "specific_absorption_rate_v01.h"
#include "qcril_qmi_client.h"
#include "radio_frequency_radiated_performance_enhancement_v01.h"
#include "qcril_data.h"
#include "qmi_ril_platform_dep.h"

/*===========================================================================

                    INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

#define NAS_SINGLE_DIGIT_MNC_LEN (1)
#define NAS_MNC_MAX_LEN (4)

#define NAS_ICCID_POSSIBLE_LENGTH_1         (19)
#define NAS_ICCID_POSSIBLE_LENGTH_2         (20)
#define NAS_ICCID_POSSIBLE_LENGTH_3         (12)
#define NAS_ICCID_IIN_START_POS             (2)
#define NAS_ICCID_IIN_SKIP_LITERAL          '0'
#define NAS_ICCID_PREFIX_CODE_ANOMALY_1     "1"
#define NAS_ICCID_PREFIX_CODE_ANOMALY_2     "7"

/*===========================================================================

                     GLOBALS

===========================================================================*/

/*===================================================================================
       G S M / W C D M A    N E T W O R K    N A M E S    M E M O R Y    L I S T
=====================================================================================*/
/*
** Define a static table of network names.  This table does not include
** all of the networks and is to be used as a reference only.  The mcc
** and mnc MUST be in ASCENDING order.
*/

typedef struct
{
  char * mcc_str;                          /* Mobile Network Code */
  char * mnc_str;                          /* Mobile Country Code */
  char *short_name_ptr;                /* Pointer to a null terminated string containing the network's short name */
  char *full_name_ptr;                 /* Pointer to a null terminated string containing the network's full name */
} qcril_qmi_ons_memory_entry_type;

typedef struct
{
  char * mcc_str;                          /* Mobile Network Code */
  char * mnc_str;                          /* Mobile Country Code */
  uint32_t sid;                            /* System ID */
  uint32_t nid;                            /* Network ID */
  char *short_name_ptr;                /* Pointer to a null terminated string containing the network's short name */
  char *full_name_ptr;                 /* Pointer to a null terminated string containing the network's full name */
} qcril_qmi_ons_3gpp2_memory_entry_type;

typedef struct
{
  char * mcc_mnc_str;
  char * short_name_ptr;
  char * full_name_ptr;
} qcril_qmi_ons_elaboration_memory_entry_type;

typedef struct
{
  char *ccc_str;
  char *mcc_str;
} qcril_qmi_ccc_mcc_map_type;

static const qcril_qmi_ons_memory_entry_type qcril_qmi_ons_memory_list[] =
{
  /***********************
   **** Test PLMN 1-1 ****
   ***********************/
  { "001",   "01", "Test1-1", "Test PLMN 1-1" },

  /***********************
   **** Test PLMN 1-2 ****
   ***********************/
  { "001",   "02", "Test1-2", "Test PLMN 1-2" },

  /***********************
   **** Test PLMN 2-1 ****
   ***********************/
  { "002",   "01", "Test2-1", "Test PLMN 2-1" },

  /****************
   **** Greece ****
   ****************/
  { "202",   "01", "Cosmote",  "COSMOTE - Mobile Telecommunications S.A." },
  { "202",   "05", "Vodafone", "Vodafone Greece" },
  { "202",   "09", "Wind",     "Wind Hella telecommunications S.A." },
  { "202",  "10", "Wind",     "Wind Hella telecommunications S.A." },

  /*********************
   **** Netherlands ****
   *********************/
  { "204",   "03", "Rabo Mobiel",             "KPN" },
  { "204",   "04", "Vodafone",                "Vodafone Netherlands" },
  { "204",   "08", "KPN",                     "KPN" },
  { "204",  "12", "Telfort",                 "KPN" },
  { "204",  "16", "T-Mobile / Ben",          "T-Mobile Netherlands B.V" },
  { "204",  "20", "Orange Nederland",        "T-Mobile Netherlands B.V" },
  { "204",  "21", "NS Railinfrabeheer B.V.", "NS Railinfrabeheer B.V." },

  /*****************
   **** Belgium ****
   *****************/
  { "206",   "01", "Proximus", "Belgacom Mobile" },
  { "206",  "10", "Mobistar", "France Telecom" },
  { "206",  "20", "BASE",     "KPN" },

  /****************
   **** France ****
   ****************/
  { "208",   "00", "Orange",                "Orange" },
  { "208",   "01", "France Telecom Mobile", "France Orange" },
  { "208",   "02", "Orange",                "Orange" },
  { "208",   "05", "Globalstar Europe",     "Globalstar Europe" },
  { "208",   "06", "Globalstar Europe",     "Globalstar Europe" },
  { "208",   "07", "Globalstar Europe",     "Globalstar Europe" },
  { "208",  "10", "SFR",                   "SFR" },
  { "208",  "11", "SFR",                   "SFR" },
  { "208",  "20", "Bouygues",              "Bouygues Telecom" },
  { "208",  "21", "Bouygues",              "Bouygues Telecom" },

  /*****************
   **** Andorra ****
   *****************/
  { "213",   "03", "Mobiland", "Servei De Tele. DAndorra" },

  /***************
   **** Spain ****
   ***************/
  { "214",   "01", "Vodafone",  "Vodafone Spain" },
  { "214",   "03", "Orange",    "France Telcom Espana SA" },
  { "214",   "04", "Yoigo",     "Xfera Moviles SA" },
  { "214",   "05", "TME",       "Telefonica Moviles Espana" },
  { "214",   "06", "Vodafone",  "Vodafone Spain" },
  { "214",   "07", "movistar",  "Telefonica Moviles Espana" },
  { "214",   "09", "Orange",    "France Telcom Espana SA" },

  /*****************
   **** Hungary ****
   *****************/
  { "216",  "20", "Pannon",   "Pannon GSM Tavkozlesi Zrt." },
  { "216",  "30", "T-Mobile", "Magyar Telkom Plc" },
  { "216",  "70", "Vodafone", "Vodafonei Magyarorszag Zrt." },

  /********************************
   **** Bosnia and Herzegovina ****
   ********************************/
  { "218",   "03", "ERONET",    "Public Enterprise Croatian telecom Ltd." },
  { "218",   "05", "m:tel",     "RS Telecommunications JSC Banja Luka" },
  { "218",  "90", "BH Mobile", "BH Telecom" },

  /*****************
   **** Croatia ****
   *****************/
  { "219",   "01", "T-Mobile", "T-Mobile Croatia" },
  { "219",   "02", "Tele2",    "Tele2" },
  { "219",  "10", "VIPnet",   "Vipnet" },

  /****************
   **** Serbia ****
   ****************/
  { "220",   "01", "Telenor",         "Telenor Serbia" },
  { "220",   "03", "Telekom Sribija", "Telekom Srbija" },
  { "220",   "05", "VIP Mobile",      "VIP Mobile" },

  /***************
   **** Italy ****
   ***************/
  { "222",   "01", "TIM",      "Telecom Italiz SpA" },
  { "222",   "02", "Elsacom",  "Elsacom" },
  { "222",  "10", "Vodafone", "Vodafone Omnitel N.V." },
  { "222",  "30", "RRI",      "Rete  Ferroviaria Italiana" },
  { "222",  "88", "Wind",     "Wind Telecomunicazioni SpA" },
  { "222",  "99", "3 Italia", "Hutchison 3G" },

  /*****************
   **** Romania ****
   *****************/
  { "226",   "01", "Vodafone",   "Vodafone Romania" },
  { "226",   "03", "Cosmote",    "Cosmote Romania" },
  { "226",   "05", "DIGI.mobil", "RCS&RDS" },
  { "226",  "10", "Orange",     "Orange Romania" },

  /*********************
   **** Switzerland ****
   *********************/
  { "228",   "01", "Swisscom",     "Swisscom Ltd" },
  { "228",   "02", "Sunrise",      "Sunrise Communications AG" },
  { "228",   "03", "Orange",       "Orange Communications SA" },
  { "228",   "06", "SBB AG",       "SBB AG" },
  { "228",   "07", "IN&Phone",     "IN&Phone SA" },
  { "228",   "08", "Tele2",        "Tele2 Telecommunications AG" },

  /************************
   **** Czech Republic ****
   ************************/
  { "230",   "01", "T-Mobile",      "T-Mobile Czech Republic" },
  { "230",   "02", "EUROTEL PRAHA", "Telefonica O2 Czech Republic" },
  { "230",   "03", "OSKAR",         "Vodafone Czech Republic" },
  { "230",  "98", "CZDC s.o.",     "CZDC s.o." },

  /******************
   **** Slovakia ****
   ******************/
  { "231",   "01", "Orange",   "Orange Slovensko" },
  { "231",   "02", "T-Mobile", "T-Mobile Slovensko" },
  { "231",   "04", "T-Mobile", "T-Mobile Slovensko" },
  { "231",   "06", "O2",       "Telefonica O2 Slovakia" },

  /*****************
   **** Austria ****
   *****************/
  { "232",   "01", "A1",       "Mobilkom Austria" },
  { "232",   "03", "T-Mobile", "T-Mobile Austria" },
  { "232",   "05", "Orange",   "Orange Austria" },
  { "232",   "07", "T-Mobile", "T-Mobile Austria" },
  { "232",  "10", "3",        "Hutchison 3G" },

  /************************
   **** United Kingdom ****
   ************************/
  { "234",   "00", "BT",                                               "British Telecom" },
  { "234",   "01", "UK01",                                             "Mapesbury Communications Ltd." },
  { "234",   "02", "O2",                                               "O2" },
  { "234",   "03", "Jersey Telenet",                                   "Jersey Telnet" },
  { "234",   "07", "C&W UK",                                           "Cable&Wireless UK" },
  { "234",  "10", "O2",                                               "Telefonica O2 UK Limited" },
  { "234",  "11", "O2",                                               "Telefonica Europe" },
  { "234",  "12", "Railtrack",                                        "Network Rail Infrastructure Ltd" },
  { "234",  "15", "Vodafone",                                         "Vodafone United Kingdom" },
  { "234",  "16", "Opal Telecom Ltd",                                 "Opal Telecom Ltd" },
  { "234",  "18", "Cloud9",                                           "Wire9 Telecom plc" },
  { "234",  "19", "Telaware",                                         "Telaware plc" },
  { "234",  "20", "3",                                                "Hutchison 3G UK Ltd" },
  { "234",  "30", "T-Mobile",                                         "T-Mobile" },
  { "234",  "31", "Virgin",                                           "Virgin Mobile" },
  { "234",  "32", "Virgin",                                           "Virgin Mobile" },
  { "234",  "33", "Orange",                                           "Orange PCS Ltd" },
  { "234",  "34", "Orange",                                           "Orange PCS Ltd" },
  { "234",  "50", "JT-Wave",                                          "Jersey Telecoms" },
  { "234",  "55", "Cable & Wireless Guernsey / Sure Mobile (Jersey)", "Cable & Wireless Guernsey / Sure Mobile (Jersey)" },
  { "234",  "58", "Manx Telecom",                                     "Manx Telecom" },
  { "234",  "75", "Inquam",                                           "Inquam Telecom (Holdings) Ltd" },

  /*****************
   **** Denmark ****
   *****************/
  { "238",   "01", "TDC",                 "TDC A/S" },
  { "238",   "02", "Sonofon",             "Telenor" },
  { "238",   "06", "3",                   "Hi3G Denmark ApS" },
  { "238",  "30", "Telia",               "Telia Nattjanster Norden AB" },
  { "238",  "70", "Tele2",               "Telenor" },

  /****************
   **** Sweden ****
   ****************/
  { "240",   "01", "Telia",                              "TeliaSonera Mobile Networks" },
  { "240",   "02", "3",                                  "3" },
  { "240",   "04", "3G Infrastructure Services",         "3G Infrastructure Services" },
  { "240",   "05", "Sweden 3G",                          "Sweden 3G" },
  { "240",   "06", "Telenor",                            "Telenor" },
  { "240",   "07", "Tele2",                              "Tele2 AB" },
  { "240",   "08", "Telenor",                            "Telenor" },
  { "240",  "21", "Banverket",                          "Banverket" },

  /****************
   **** Norway ****
   ****************/
  { "242",   "01", "Telenor",           "Telenor" },
  { "242",   "02", "NetCom",            "NetCom GSM" },
  { "242",   "05", "Network Norway",    "Network Norway" },
  { "242",  "20", "Jernbaneverket AS", "Jernbaneverket AS" },

  /*****************
   **** Finland ****
   *****************/
  { "244",   "03", "DNA",        "DNA Oy" },
  { "244",   "05", "Elisa",      "Elisa Oyj" },
  { "244",  "12", "DNA Oy",     "DNA Oy" },
  { "244",  "14", "AMT",        "Alands Mobiltelefon" },
  { "244",  "91", "Sonera",     "TeliaSonera Finland Oyj" },

  /*******************
   **** Lithuania ****
   *******************/
  { "246",   "01", "Omnitel", "Omnitel" },
  { "246",   "02", "BITE",    "UAB Bite Lietuva" },
  { "246",   "03", "Tele 2",  "Tele 2" },

  /****************
   **** Latvia ****
   ****************/
  { "247",   "01", "LMT",   "Latvian Mobile Telephone" },
  { "247",   "02", "Tele2", "Tele2" },
  { "247",   "05", "Bite",  "Bite Latvija" },

  /*****************
   **** Estonia ****
   *****************/
  { "248",   "01", "EMT",    "Estonian Mobile Telecom" },
  { "248",   "02", "Elisa",  "Elisa Eesti" },
  { "248",   "03", "Tele 2", "Tele 2 Eesti" },

  /***************************
   **** Russia Federation ****
   ***************************/
  { "250",   "01", "MTS",                   "Mobile Telesystems" },
  { "250",   "02", "MegaFon",               "MegaFon OJSC" },
  { "250",   "03", "NCC",                   "Nizhegorodskaya Cellular Communications" },
  { "250",   "05", "ETK",                   "Yeniseytelecom" },
  { "250",   "07", "SMARTS",                "Zao SMARTS" },
  { "250",  "12", "Baykalwstern",          "Baykal Westcom/New Telephone Company/Far Eastern Cellular" },
  { "250",  "14", "SMARTS",                "SMARTS Ufa" },
  { "250",  "16", "NTC",                   "New Telephone Company" },
  { "250",  "17", "Utel",                  "JSC Uralsvyazinform" },
  { "250",  "19", "INDIGO",                "INDIGO" },
  { "250",  "20", "Tele2",                 "Tele2" },
  { "250",  "23", "Mobicom - Novosibirsk", "Mobicom - Novosibirsk" },
  { "250",  "39", "Utel",                  "Uralsvyazinform" },
  { "250",  "99", "Beeline",               "OJSC VimpelCom" },

  /*****************
   **** Ukraine ****
   *****************/
  { "255",   "01", "MTS", "Ukrainian Mobile Communications" },
  { "255",   "02", "Beeline", "Ukrainian Radio Systems" },
  { "255",   "03", "Kyivstar", "Kyivstar GSM JSC" },
  { "255",   "05", "Golden Telecom", "Golden Telecom" },
  { "255",   "06", "life:)", "Astelit" },
  { "255",   "07", "Utel", "Ukrtelecom" },

  /*****************
   **** Belarus ****
   *****************/
  { "257",   "01", "Velcom", "Velcom" },
  { "257",   "02", "MTS",    "JLLC Mobile TeleSystems" },
  { "257",   "04", "life:)", "Belarussian Telecommunications Network" },

  /*****************
   **** Moldova ****
   *****************/
  { "259",   "01", "Orange",   "Orange Moldova" },
  { "259",   "02", "Moldcell", "Moldcell" },
  { "259",   "04", "Eventis",  "Eventis Telecom" },

  /****************
   **** Poland ****
   ****************/
  { "260",   "01", "Plus",           "Polkomtel" },
  { "260",   "02", "Era",            "Polska Telefonia Cyfrowa (PTC)" },
  { "260",   "03", "Orange",         "PTK Centertel" },
  { "260",   "06", "Play",           "P4 Sp. zo.o" },
  { "260",  "12", "Cyfrowy Polsat", "Cyfrowy Polsat" },
  { "260",  "14", "Sferia",         "Sferia S.A." },

  /*****************
   **** Germany ****
   *****************/
  { "262",   "01", "T-Mobile",      "T-Mobile" },
  { "262",   "02", "Vodafone",      "Vodafone D2 GmbH" },
  { "262",   "03", "E-Plus",        "E-Plus Mobilfunk" },
  { "262",   "04", "Vodafone",      "Vodafone" },
  { "262",   "05", "E-Plus",        "E-Plus Mobilfunk" },
  { "262",   "06", "T-Mobile",      "T-Mobile" },
  { "262",   "07", "O2",            "O2 (Germany) GmbH & Co. OHG" },
  { "262",   "08", "O2",            "O2" },
  { "262",   "09", "Vodafone",      "Vodafone" },
  { "262",  "10", "Arcor AG & Co", "Arcor AG * Co" },
  { "262",  "11", "O2",            "O2" },
  { "262",  "15", "Airdata",       "Airdata" },
  { "262",  "60", "DB Telematik",  "DB Telematik" },
  { "262",  "76", "Siemens AG",    "Siemens AG" },
  { "262",  "77", "E-Plus",        "E-Plus" },

  /*******************
   **** Gibraltar ****
   *******************/
  { "266",   "01", "GibTel", "Gibraltar Telecoms" },

  /******************
   **** Portugal ****
   ******************/
  { "268",   "01", "Vodafone", "Vodafone Portugal" },
  { "268",   "03", "Optimus",  "Sonaecom - Servicos de Comunicacoes, S.A." },
  { "268",   "06", "TMN",      "Telecomunicacoes Moveis Nacionais" },

  /********************
   **** Luxembourg ****
   ********************/
  { "270",   "01", "LuxGSM",    "P&T Luxembourg" },
  { "270",  "77", "Tango",     "Tango SA" },
  { "270",  "99", "Voxmobile", "VOXmobile S.A" },

  /*****************
   **** Ireland ****
   *****************/
  { "272",   "01", "Vodafone", "Vodafone Ireland" },
  { "272",   "02", "O2",       "O2 Ireland" },
  { "272",   "03", "Meteor",   "Meteor" },
  { "272",   "05", "3",        "Hutchison 3G IReland limited" },

  /*****************
   **** Iceland ****
   *****************/
  { "274",   "01", "Siminn",   "Iceland Telecom" },
  { "274",   "02", "Vodafone", "iOg fjarskipti hf" },
  { "274",   "04", "Viking",   "IMC Island ehf" },
  { "274",   "07", "IceCell",  "IceCell ehf" },
  { "274",  "11", "Nova",     "Nova ehf" },

  /*****************
   **** Albania ****
   *****************/
  { "276",   "01", "AMC",          "Albanian Mobile Communications" },
  { "276",   "02", "Vodafone",     "Vodafone Albania" },
  { "276",   "03", "Eagle Mobile", "Eagle Mobile" },

  /***************
   **** Malta ****
   ***************/
  { "278",   "01", "Vodafone", "Vodafone Malta" },
  { "278",  "21", "GO",       "Mobisle Communications Limited" },
  { "278",  "77", "Melita",   "Melita Mobile Ltd. (3G Telecommunictaions Limited" },

  /****************
   **** Cyprus ****
   ****************/
  { "280",   "01", "Cytamobile-Vodafone", "Cyprus Telcommunications Auth" },
  { "280",  "10", "MTN",                 "Areeba Ltde" },

  /*****************
   **** Georgia ****
   *****************/
  { "282",   "01", "Geocell",  "Geocell Limited" },
  { "282",   "02", "Magti",    "Magticom GSM" },
  { "282",   "04", "Beeline",  "Mobitel LLC" },
  { "282",  "67", "Aquafon",  "Aquafon" },
  { "282",  "88", "A-Mobile", "A-Mobile" },

  /*****************
   **** Armenia ****
   *****************/
  { "283",   "01", "Beeline",      "ArmenTel" },
  { "283",   "05", "VivaCell-MTS", "K Telecom CJSC" },

  /******************
   **** Bulgaria ****
   ******************/
  { "284",   "01", "M-TEL",   "Mobiltel" },
  { "284",   "03", "Vivatel", "BTC" },
  { "284",   "05", "GLOBUL",  "Cosmo Bulgaria Mobile" },

  /****************
   **** Turkey ****
   ****************/
  { "286",   "01", "Turkcell", "Turkcell lletisim Hizmetleri A.S." },
  { "286",   "02", "Vodafone", "Vodafone Turkey" },
  { "286",   "03", "Avea",     "Avea" },

  /********************************
   **** Faroe Islands (Demark) ****
   ********************************/
  { "288",   "01", "Faroese",  "Faroese Telecom" },
  { "288",   "02", "Vodafone", "Vodafone Faroe Islands" },

  /*******************
   **** Greenland ****
   *******************/
  { "290",   "01", "TELE Greenland A/S", "Tele Greenland A/S" },

  /********************
   **** San Marino ****
   ********************/
  { "292",   "01", "PRIMA", "San Marino Telecom" },

  /******************
   **** Slovenia ****
   ******************/
  { "293",  "40", "Si.mobil", "SI.MOBIL d.d" },
  { "293",  "41", "Si.mobil", "Mobitel D.D." },
  { "293",  "64", "T-2",      "T-2 d.o.o." },
  { "293",  "70", "Tusmobil", "Tusmobil d.o.o." },

  /*******************************
   **** Republic of Macedonia ****
   *******************************/
  { "294",   "01", "T-Mobile",     "T-Mobile Makedonija" },
  { "294",   "02", "Cosmofon",     "Cosmofon" },
  { "294",   "02", "VIP Operator", "VIP Operator" },

  /***********************
   **** Liechtenstein ****
   ***********************/
  { "295",   "01", "Swisscom", "Swisscom Schweiz AG" },
  { "295",   "02", "Orange",   "Orange Liechtenstein AG" },
  { "295",   "05", "FL1",      "Mobilkom Liechtenstein AG" },
  { "295",  "77", "Tele 2",   "Belgacom" },

  /********************
   **** Montenegro ****
   ********************/
  { "297",   "01", "ProMonte", "ProMonte GSM" },
  { "297",   "02", "T-Mobile", "T-Mobile Montenegro LLC" },
  { "297",   "03", "m:tel CG", "MTEL CG" },

  /****************
   **** Canada ****
   ****************/
  { "302", "64", "Bell",                 "Bell" },
  { "302", "66",  "MTS",                 "MTS" },
  { "302", "68",  "SaskTel",             "SaskTel" },
  { "302", "72",  "ROGERS",              "CAN Rogers Wireless Inc." },
  { "302", "270", "Eastlink",            "Eastlink" },
  { "302", "320", "Mobilicity",          "Mobilicity" },
  { "302", "340", "Execulink",           "Execulink Telecom" },
  { "302", "370", "Fido",                "Fido" },
  { "302", "380", "DMTS",                "DMTS GSM" },
  { "302", "490", "WIND",                "WIND" },
  { "302", "620", "ICE Wireless",        "ICE Wireless" },
  { "302", "720", "Rogers Wireless",     "Rogers Wireless" },
  { "302", "730", "TStarSol",            "CAN TerreStar Solutions" },
  { "302", "770", "RuralCom",            "CANRU" },
  { "302", "940", "Wightman",            "Wightman" },

  /********************************************
   **** Saint Pierre and Miquelon (France) ****
   ********************************************/
  { "308",   "01", "Ameris", "St. Pierre-et-Miquelon Telecom" },

  /****************************************
   **** United States of America, Guam ****
   ****************************************/
  { "310",  "20", "Union Telephony Company",          "Union Telephony Company" },
  { "310",  "26", "T-Mobile",                         "T-Mobile" },
  { "310",  "30", "Centennial",                       "Centennial Communications" },
  { "310",  "38", "AT&T",                             "AT&T Mobility" },
  { "310",  "40", "Concho",                           "Concho Cellular Telephony Co., Inc." },
  { "310",  "46", "SIMMETRY",                         "TMP Corp" },
  { "310",  "70", "AT&T",                             "AT&T" },
  { "310",  "80", "Corr",                             "Corr Wireless Communications LLC" },
  { "310",  "90", "AT&T",                             "AT&T" },
  { "310", "100", "Plateau Wireless",                 "New Mexico RSA 4 East Ltd. Partnership" },
  { "310", "110", "PTI Pacifica",                     "PTI Pacifica Inc." },
  { "310", "150", "AT&T",                             "AT&T" },
  { "310", "170", "AT&T",                             "AT&T" },
  { "310", "180", "West Cen",                         "West Central" },
  { "310", "190", "Dutch Harbor",                     "Alaska Wireless Communications, LLC" },
  { "310", "260", "T-Mobile",                         "T-Mobile" },
  { "310", "300", "Get Mobile Inc",                   "Get Mobile Inc" },
  { "310", "311", "Farmers Wireless",                 "Farmers Wireless" },
  { "310", "330", "Cell One",                         "Cellular One" },
  { "310", "340", "Westlink",                         "Westlink Communications" },
  { "310", "380", "AT&T",                             "AT&T" },
  { "310", "400", "i CAN_GSM",                        "Wave runner LLC (Guam)" },
  { "310", "410", "AT&T",                             "AT&T" },
  { "310", "420", "Cincinnati Bell",                  "Cincinnati Bell Wireless" },
  { "310", "430", "Alaska Digitel",                   "Alaska Digitel" },
  { "310", "450", "Viaero",                           "Viaero Wireless" },
  { "310", "460", "Simmetry",                         "TMP Corporation" },
  { "310", "540", "Oklahoma Western",                 "Oklahoma Western Telephone Company" },
  { "310", "560", "AT&T",                             "AT&T" },
  { "310", "570", "Cellular One",                     "MTPCS, LLC" },
  { "310", "590", "Alltel",                           "Alltel Communications Inc" },
  { "310", "610", "Epic Touch",                       "Elkhart Telephone Co." },
  { "310", "620", "Coleman County Telecom",           "Coleman County Telecommunications" },
  { "310", "640", "Airadigim",                        "Airadigim Communications" },
  { "310", "650", "Jasper",                           "Jasper wireless, inc" },
  { "310", "680", "AT&T",                             "AT&T" },
  { "310", "770", "i wireless",                       "lows Wireless Services" },
  { "310", "790", "PinPoint",                         "PinPoint Communications" },
  { "310", "830", "Caprock",                          "Caprock Cellular" },
  { "310", "850", "Aeris",                            "Aeris Communications, Inc." },
  { "310", "870", "PACE",                             "Kaplan Telephone Company" },
  { "310", "880", "Advantage",                        "Advantage Cellular Systems" },
  { "310", "890", "Unicel",                           "Rural cellular Corporation" },
  { "310", "900", "Taylor",                           "Taylor Telecommunications" },
  { "310", "910", "First Cellular",                   "First Cellular of Southern Illinois" },
  { "310", "950", "XIT Wireless",                     "Texas RSA 1 dba XIT Cellular" },
  { "310", "970", "Globalstar",                       "Globalstar" },
  { "310", "980", "AT&T",                             "AT&T" },
  { "311",  "10", "Chariton Valley",                  "Chariton Valley Communications" },
  { "311",  "20", "Missouri RSA 5 Partnership",       "Missouri RSA 5 Partnership" },
  { "311",  "30", "Indigo Wireless",                  "Indigo Wireless" },
  { "311",  "40", "Commnet Wireless",                 "Commnet Wireless" },
  { "311",  "50", "Wikes Cellular",                   "Wikes Cellular" },
  { "311",  "60", "Farmers Cellular",                 "Farmers Cellular Telephone" },
  { "311",  "70", "Easterbrooke",                     "Easterbrooke Cellular Corporation" },
  { "311",  "80", "Pine Cellular",                    "Pine Telephone Company" },
  { "311",  "90", "Long Lines Wireless",              "Long Lines Wireless LLC" },
  { "311", "100", "High Plains Wireless",             "High Plains Wireless" },
  { "311", "110", "High Plains Wireless",             "High Plains Wireless" },
  { "311", "130", "Cell One Amarillo",                "Cell One Amarillo" },
  { "311", "150", "Wilkes Cellular",                  "Wilkes Cellular" },
  { "311", "170", "PetroCom",                         "Broadpoint Inc" },
  { "311", "180", "AT&T",                             "AT&T" },
  { "311", "210", "Farmers Cellular",                 "Farmers Cellular Telephone" },

  /*********************
   **** Puerto Rico ****
   *********************/
  { "330",  "11", "Claro", "Puerto Rico Telephony Company" },

  /****************
   **** Mexico ****
   ****************/
  { "334",   "02", "Telcel",   "America Movil" },
  { "334",   "03", "movistar", "Pegaso Comunicaciones y Sistemas" },

  /*****************
   **** Jamaica ****
   *****************/
  { "338",  "20", "Cable & Wireless", "Cable & Wireless" },
  { "338",  "50", "Digicel",          "Digicel (Jamaica) Limited" },
  { "338",  "70", "Claro",            "Oceanic Digital Jamaica Limited" },

  /*****************************
   **** Guadeloupe (France) ****
   *****************************/
  { "340",   "01", "Orange",   "Orange Caraibe Mobiles" },
  { "340",   "02", "Outremer", "Outremer Telecom" },
  { "340",   "03", "Teleceli", "Saint Martin et Saint Barthelemy Telcell Sarl" },
  { "340",   "08", "MIO GSM",  "Dauphin Telecom" },
  { "340",  "20", "Digicel",  "DIGICEL Antilles Franccaise Guyane" },

  /******************
   **** Barbados ****
   ******************/
  { "342", "600", "bmobile", "cable &Wireless Barbados Ltd." },
  { "342", "750", "Digicel", "Digicel (Jamaica) Limited" },

  /*****************************
   **** Antigua and Barbuda ****
   *****************************/
  { "344",  "30", "APUA",    "Antigua Public Utilities Authority" },
  { "344", "920", "bmobile", "Cable & Wireless Caribbean Cellular (Antigua) Limited" },
  { "344", "930", "Digicel", "Antigua Wireless Ventures Limited" },

  /*****************************************
   **** Cayman Islands (United Kingdom) ****
   *****************************************/
  { "346",   "50", "Digicel",          "Digicel Cayman Ltd." },
  { "346",  "140", "Cable & Wireless", "Cable & Wireless (Caymand Islands) Limited" },

  /*************************************************
   **** British Virgin Islands (United Kingdom) ****
   *************************************************/
  { "348", "170", "Cable & Wireless",             "Cable & Wireless (West Indies)" },
  { "348", "570", "Caribbean Cellular Telephone", "Caribbean Cellular Telephone" },

  /*****************
   **** Bermuda ****
   *****************/
  { "350",   "01", "Digicel Bermuda", "Telecommunications (Bermuda & West Indies) Ltd" },
  { "350",   "02", "Mobility",        "M3 wireless" },
  { "350",  "38", "Digicel",         "Digicel" },

  /*****************
   **** Grenada ****
   *****************/
  { "352",  "30", "Digicel",          "Digicel Grenada Ltd." },
  { "352", "110", "Cable & Wireless", "Cable & Wireless Grenada Ltd." },

  /******************************
   **** Netherlands Antilles ****
   ******************************/
  { "362",  "51", "Telcell", "Telcell N.V." },
  { "362",  "69", "Digicel", "Curacao Telecom N.V." },
  { "362",  "91", "UTS",     "Setel NV" },

  /********************************************
   **** Aruba (Kingdom of the Netherlands) ****
   ********************************************/
  { "363",   "01", "SETAR",    "SETAR (Servicio di Telecommunication diAruba" },
  { "363",  "20", "Digicell", "Digicell" },

  /*****************
   **** Bahamas ****
   *****************/
  { "364", "390", "BaTelCo", "The Bahamas Telecommunications Company Ltd" },

  /***********************************
   **** Anguilla (United Kingdom) ****
   ***********************************/
  { "365",  "10", "Weblinks Limited", "Weblinks Limited" },

  /**************
   **** Cuba ****
   **************/
  { "368",   "01", "ETECSA", "Empresa de Telecomunicaciones de Cuba, SA" },

  /****************************
   **** Dominican Republic ****
   ****************************/
  { "370",   "01", "Orange", "Orange Dominicana" },
  { "370",   "02", "Claro",  "Compania Dominicana de Telefonos, C por" },
  { "370",   "04", "ViVa",   "Centennial Dominicana" },

  /***************
   **** Haiti ****
   ***************/
  { "372",  "10", "Comcel / Voila", "Comcel / Voila" },
  { "372",  "50", "Digicel",        "Digicel" },

  /*****************************
   **** Trinidad and Tobaga ****
   *****************************/
  { "374",  "12", "bmobile", "TSTT" },
  { "374",  "13", "Digicel", "Digicel" },

  /********************
   **** Azerbaijan ****
   ********************/
  { "400",   "01", "Azercell",   "Azercell" },
  { "400",   "02", "Bakcell",    "Bakcell" },
  { "400",   "04", "Nar Mobile", "Azerfon" },

  /********************
   **** Kazakhstan ****
   ********************/
  { "401",   "01", "Beeline",                "KaR-TeL LLP" },
  { "401",   "02", "K'Cell",                 "GSM Kazakhstan Ltdx." },
  { "401",  "77", "Mobile Telecom Service", "Mobile Telecom Service LLP" },

  /****************
   **** Bhutan ****
   ****************/
  { "402",  "11", "B-Mobile",  "B-Mobile" },
  { "402",  "77", "TashiCell", "Tashi InfoComm Limited" },

  /***************
   **** India ****
   ***************/
  { "404",   "01", "Vodafone - Haryana",            "Vodafone Essar" },
  { "404",   "02", "Airtel - Punjab",               "Bharti Airtel" },
  { "404",   "03", "Airtel",                        "Bharti Airtel" },
  { "404",   "04", "Idea - Delhi",                  "Idea cellular" },
  { "404",   "05", "Vodafone - Gujarat",            "Vodafone Essar - Gujarat Limited" },
  { "404",   "07", "Idea Cellular",                 "Idea Cellular" },
  { "404",  "12", "Idea Haryana",                  "IDEA Cellular Limited" },
  { "405",  "13", "Reliance - Maharashtra",        "Reliance GSM" },
  { "404",  "20", "Vodafone",                      "Vodafone Mumbai" },
  { "404",  "21", "BPL Mobile Mumbai",             "BPL Mobile Mumbai" },
  { "404",  "22", "IDEA Cellular - Maharashtra",   "Idea cellular" },
  { "404",  "24", "IDEA Cellular - Gujarat",       "Idea cellular" },
  { "404",  "27", "Vodafone - Maharashtra",        "Vodafone Essar" },
  { "404",  "28", "Aircel - Orissa",               "Dishnet Wireless/Aircel" },
  { "404",  "30", "Vodafone - Kolkata",            "Vodafone Essar East Limited" },
  { "404",  "31", "Airtel - Kolkata",              "Bharti Airtel" },
  { "404",  "44", "Spice Telecom - Karnataka",     "Spice Communications Limited" },
  { "404",  "50", "Reliance",                      "Reliance GSM" },
  { "404",  "52", "Reliance - Orissa",             "Reliance Telecom Private" },
  { "404",  "56", "Idea - UP West",                "IDEA Cellular Limited" },
  { "404",  "66", "BSNL - Maharashtra",            "BSNL Maharashtra & Goa" },
  { "404",  "67", "Vodafone - West Bengal",        "Vodafone Essar East" },
  { "404",  "68", "MTNL - Delhi",                  "Mahanagar Telephone Nigam Ltd" },
  { "404",  "69", "MTNL - Mumbai",                 "Mahanagar Telephone Nigam Ltd" },
  { "404",  "72", "BSNL - Kerala",                 "Bharti Sanchar Nigam Limited" },
  { "404",  "74", "BSNL - West Bengal",            "Bharti Sanchar Nigam Limited" },
  { "405",  "75", "Vodafone-Bihar",                "Vodafone Essar" },
  { "404",  "81", "BSNL - Kolkata",                "Bharti Sanchar Nigam Limited" },
  { "404",  "83", "Reliance",                      "Reliance GSM - West Bengal" },
  { "404",  "85", "Reliance",                      "Reliance GSM - West Bengal" },
  { "404",  "90", "Airtel - Maharashtra",          "Airtel - Maharashtra & Goa" },
  { "404",  "91", "Airtel - Kolkata Metro Circle", "Dishnet Wireless/Aircel" },
  { "404",  "92", "Airtel Mumbai",                 "Airtel Mumbai" },
  { "404",  "95", "Airtel",                        "Bharti Airtel" },
  { "404",  "96", "Airtel - Haryana",              "Bharti Airtel" },

  /******************
   **** Pakistan ****
   ******************/
  { "410",   "01", "Mobilink", "Mobilink-PMCL" },
  { "410",   "03", "Ufone",    "Pakistan Telecommunication Mobile Ltd" },
  { "410",   "04", "Zong",     "China Mobile" },
  { "410",   "06", "Telenor",  "Telenor Pakistan" },
  { "410",   "07", "Warid",    "WaridTel" },

  /*******************
   *** Afghanistan ***
   ********************/
  { "412",   "01", "AWCC",     "Afghan wireless Communication Company" },
  { "412",  "20", "Roshan",   "Telecom Development Company Afghanistan Ltd." },
  { "412",  "40", "Areeba",   "MTN Afghanistan" },
  { "412",  "50", "Etisalat", "Etisalat Afghanistan" },

  /*******************
   **** Sri Lanka ****
   *******************/
  { "413",   "01", "Mobitel",         "Mobitel Lanka Ltd." },
  { "413",   "02", "Dialog",          "Dialog Telekom PLC." },
  { "413",   "03", "Tigo",            "Celtel Lanka Ltd" },
  { "413",   "08", "Hutch Sri Lanka", "Hutch Sri Lanka" },

  /*****************
   **** Myanmar ****
   *****************/
  { "414",   "01", "MPT", "Myanmar Post and Telecommunication" },

  /*****************
   **** Lebanon ****
   *****************/
  { "415",   "01", "Alfa",      "Alfa" },
  { "415",   "03", "MTC-Touch", "MIC 2" },

  /****************
   **** Jordan ****
   ****************/
  { "416",   "01", "Zain",   "Jordan Mobile Teelphone Services" },
  { "416",   "03", "Umniah", "Umniah" },
  { "416",  "77", "Orange", "Oetra Jordanian Mobile Telecommunications Company (MobileCom)" },

  /***************
   **** Syria ****
   ***************/
  { "417",   "01", "SyriaTel", "SyriaTel" },
  { "417",   "02", "MTN Syria", "MTN Syria (JSC)" },

  /****************
   **** Iraq ****
   ****************/
  { "418",  "20", "Zain Iraq", "Zain Iraq" },
  { "418",  "30", "Zain Iraq", "Zain Iraq" },
  { "418",  "50", "Asia Cell", "Asia Cell Telecommunications Company" },
  { "418",  "40", "Korek",     "Korel Telecom Ltd" },

  /****************
   **** Kuwait ****
   ****************/
  { "419",   "02", "Zain",     "Mobile Telecommunications Co." },
  { "419",   "03", "Wataniya", "National Mobile Telecommunications" },
  { "419",   "04", "Viva",     "Kuwait Telecommunication Company" },

  /**********************
   **** Saudi Arabia ****
   **********************/
  { "420",   "01", "STC",     "Saudi Telecom Company" },
  { "420",   "03", "Mobily",  "Etihad Etisalat Company" },
  { "420",   "04", "Zain SA", "MTC Saudi Arabia" },

  /***************
   **** Yemen ****
   ***************/
  { "421",   "01", "SabaFon", "SabaFon" },
  { "421",   "02", "MTN",     "Spacetel" },

  /**************
   **** Oman ****
   **************/
  { "422",   "02", "Oman Mobile", "Oman Telecommunications Company" },
  { "422",   "03", "Nawras",      "Omani Qatari Telecommunications Company SAOC" },

  /******************************
   **** United Arab Emirates ****
   ******************************/
  { "424",   "02", "Etisalat", "Emirates Telecom Corp" },
  { "424",   "03", "du",       "Emirates Integrated Telecommunications Company" },

  /****************
   **** Israel ****
   ****************/
  { "425",   "01", "Orange",    "Partner Communications Company Ltd" },
  { "425",   "02", "Cellcom",   "Cellcom" },
  { "425",   "03", "Pelephone", "Pelephone" },

  /*******************************
   **** Palestinian Authority ****
   *******************************/
  { "425",   "05", "JAWWAL", "Palestine Cellular Communications, Ltd." },

  /***************
   **** Qatar ****
   ***************/
  { "427",   "01", "Qatarnet", "Q-Tel" },

  /******************
   **** Mongolia ****
   ******************/
  { "428",  "88", "Unitel", "Unitel LLC" },
  { "428",  "99", "MobiCom", "MobiCom Corporation" },

  /***************
   **** Nepal ****
   ***************/
  { "429",   "01", "Nepal Telecom", "Nepal Telecom" },
  { "429",   "02", "Mero Mobile",   "Spice Nepal Private Ltd" },

  /**************
   **** Iran ****
   **************/
  { "432",  "11", "MCI",      "Mobile Communications Company of Iran" },
  { "432",  "14", "TKC",      "KFZO" },
  { "432",  "19", "MTCE",     "Mobile Telecommunications Company of Esfahan" },
  { "432",  "32", "Taliya",   "Taliya" },
  { "432",  "35", "Irancell", "Irancell Telecommunications Services Company" },

  /********************
   **** Uzbekistan ****
   ********************/
  { "434",   "04", "Beeline", "Unitel LLC" },
  { "434",   "05", "Ucell",   "Coscom" },
  { "434",   "07", "MTS",     "Mobile teleSystems (FE 'Uzdunrobita' Ltd)" },

  /********************
   **** Tajikistan ****
   ********************/
  { "436",   "01", "Somoncom",   "JV Somoncom" },
  { "436",   "02", "Indigo",     "Indigo Tajikistan" },
  { "436",   "03", "MLT",        "TT Mobile, Closed joint-stock company" },
  { "436",   "04", "Babilon-M",  "CJSC Babilon-Mobile" },
  { "436",   "05", "Beeline TJ", "Co Ltd. Tacom" },

  /********************
   **** Kyrgyzstan ****
   ********************/
  { "437",   "01", "Bitel",   "Sky Mobile LLC" },
  { "437",   "05", "MegaCom", "BiMoCom Ltd" },
  { "437",   "09", "O!",      "NurTelecom LLC" },

  /**********************
   **** Turkmenistan ****
   **********************/
  { "438",   "01", "MTS",     "Barash Communication Technologies" },
  { "438",   "02", "TM-Cell", "TM-Cell" },

  /***************
   **** Japan ****
   ***************/
  { "440",   "00", "eMobile",  "eMobile, Ltd." },
  { "440",   "01", "DoCoMo",   "NTT DoCoMo" },
  { "440",   "02", "DoCoMo",   "NTT DoCoMo Kansai" },
  { "440",   "03", "DoCoMo",   "NTT DoCoMo Hokuriku" },
  { "440",   "04", "SoftBank", "SoftBank Mobile Corp" },
  { "440",   "06", "SoftBank", "SoftBank Mobile Corp" },
  { "440",  "10", "DoCoMo",   "NTT DoCoMo Kansai" },
  { "440",  "20", "SoftBank", "SoftBank Mobile Corp" },

  /*********************
   **** South Korea ****
   *********************/
  { "450",   "05", "SKT",      "SK Telecom" },
  { "450",   "08", "KTF SHOW", "KTF" },

  /*****************
   **** Vietnam ****
   *****************/
  { "452",   "01", "MobiFone",       "Vietnam Mobile Telecom Services Company" },
  { "452",   "02", "Vinaphone",      "Vietnam Telecoms Services Company" },
  { "452",   "04", "Viettel Mobile", "iViettel Corporation" },

  /*******************
   **** Hong Kong ****
   *******************/
  { "454",   "00", "CSL",                   "Hong Kong CSL Limited" },
  { "454",   "01", "CITIC Telecom 1616",    "CITIC Telecom 1616" },
  { "454",   "02", "CSL 3G",                "Hong Kong CSL Limited" },
  { "454",   "03", "3(3G)",                 "Hutchison Telecom" },
  { "454",   "04", "3 DualBand (2G)",       "Hutchison Telecom" },
  { "454",   "06", "Smartone-Vodafone",     "SmarTone Mobile Comms" },
  { "454",   "07", "China Unicom",          "China Unicom" },
  { "454",   "08", "Trident",               "Trident" },
  { "454",   "09", "China Motion Telecom",  "China Motion Telecom" },
  { "454",  "10", "New World",             "Hong Kong CSL Limited" },
  { "454",  "11", "Chia-HongKong Telecom", "Chia-HongKong Telecom" },
  { "454",  "12", "CMCC Peoples",          "China Mobile Hong Kong Company Limited" },
  { "454",  "14", "Hutchison Telecom",     "Hutchison Telecom" },
  { "454",  "15", "SmarTone Mobile Comms", "SmarTone Mobile Comms" },
  { "454",  "16", "PCCW",                  "PCCW Mobile (PCCW Ltd)" },
  { "454",  "17", "SmarTone Mobile Comms", "SmarTone Mobile Comms" },
  { "454",  "18", "Hong Kong CSL Limited", "Hong Kong CSL Limited" },
  { "454",  "19", "PCCW",                  "PCCW Mobile (PCCW Ltd)" },

  /***************
   **** Macau ****
   ***************/
  { "455",   "00", "SmarTone", "SmarTone Macau" },
  { "455",   "01", "CTM",      "C.T.M. Telemovel+" },
  { "455",   "03", "03",        "Hutchison Telecom" },
  { "455",   "04", "CTM",      "C.T.M. Telemovel+" },
  { "455",   "05", "03",        "Hutchison Telecom" },

  /******************
   **** Cambodia ****
   ******************/
  { "456",   "01", "Mobitel",    "CamGSM" },
  { "456",   "02", "hello",      "Telekom Malaysia International (Cambodia) Co. Ltd" },
  { "456",   "04", "qb",         "Cambodia Advance Communications Co. Ltd" },
  { "456",   "05", "Star-Cell",  "APPLIFONE CO. LTD." },
  { "456",  "18", "Shinawatra", "Shinawatra" },

  /**************
   **** Laos ****
   **************/
  { "457",   "01", "LaoTel", "Lao Shinawatra Telecom" },
  { "457",   "02", "ETL",    "Enterprise of Telecommunications Lao" },
  { "457",   "03", "LAT",    "Lao Asia Telecommunication State Enterprise (LAT)" },
  { "457",   "08", "Tigo",   "Millicom Lao Co Ltd" },

  /***************
   **** China ****
   ***************/
  { "460",   "00", "China Mobile", "China Mobile" },
  { "460",   "01", "China Unicom", "China Unicom" },

  /****************
   **** Taiwan ****
   ****************/
  { "466",   "01", "FarEasTone", "Far EasTone Telecommunications Co Ltd" },
  { "466",   "06", "Tuntex", "Tuntex Telecom" },
  { "466",  "88", "KG Telecom", "KG Telecom" },
  { "466",  "89", "VIBO", "VIBO Telecom" },
  { "466",  "92", "Chungwa", "Chungwa" },
  { "466",  "93", "MobiTai", "iMobitai Communications" },
  { "466",  "97", "Taiwan Mobile", "Taiwan Mobile Co. Ltd" },
  { "466",  "99", "TransAsia", "TransAsia Telecoms" },

  /*********************
   **** North Korea ****
   *********************/
  { "467", "193", "SUN NET", "Korea Posts and Telecommunications Corporation" },

  /********************
   **** Bangladesh ****
   ********************/
  { "470",   "01", "Grameenphone", "GrameenPhone Ltd" },
  { "470",   "02", "Aktel",        "Aktel" },
  { "470",   "03", "Banglalink",   "Orascom telecom Bangladesh Limited" },
  { "470",   "04", "TeleTalk",     "TeleTalk" },
  { "470",   "06", "Citycell",     "Citycell" },
  { "470",   "07", "Warid",        "Warid Telecom" },

  /******************
   **** Maldives ****
   ******************/
  { "472",   "01", "Dhiraagu", "Dhivehi Raajjeyge Gulhun" },
  { "472",   "02", "Wataniya", "Wataniya Telecom Maldives" },

  /******************
   **** Malaysia ****
   ******************/
  { "502",  "12", "Maxis",    "Maxis Communications Berhad" },
  { "502",  "13", "Celcom",   "Celcom Malaysia Sdn Bhd" },
  { "502",  "16", "DiGi",     "DiGi Telecommunications" },
  { "502",  "18", "U Mobile", "U Mobile Sdn Bhd" },
  { "502",  "19", "Celcom",   "Celcom Malaysia Sdn Bhd" },

  /*******************
   **** Australia ****
   *******************/
  { "505",   "01", "Telstra",               "Telstra Corp. Ltd." },
  { "505",   "02", "YES OPTUS",             "Singtel Optus Ltd" },
  { "505",   "03", "Vodafone",              "Vodafone Australia" },
  { "505",   "06", "03",                     "Hutchison 3G" },
  { "505",  "90", "YES OPTUS",             "Singtel Optus Ltd" },

  /*******************
   **** Indonesia ****
   *******************/
  { "510",   "00", "PSN",          "PT Pasifik Satelit Nusantara (ACeS)" },
  { "510",   "01", "INDOSAT",      "PT Indonesian Satellite Corporation Tbk (INDOSAT)" },
  { "510",   "08", "AXIS",         "PT Natrindo Telepon Seluler" },
  { "510",  "10", "Telkomsel",    "PT Telkomunikasi Selular" },
  { "510",  "11", "XL",           "PT Excelcomindo Pratama" },
  { "510",  "89", "03",            "PT Hutchison CP Telecommunications" },

  /********************
   **** East Timor ****
   ********************/
  { "514",   "02", "Timor Telecom", "Timor Telecom" },

  /********************
   **** Philipines ****
   ********************/
  { "515",   "01", "Islacom",    "Innove Communicatiobs Inc" },
  { "515",   "02", "Globe",      "Globe Telecom" },
  { "515",   "03", "Smart Gold", "Smart Communications Inc" },
  { "515",   "05", "Digitel",    "Digital Telecommunications Philppines" },
  { "515",  "18", "Red Mobile", "Connectivity Unlimited resource Enterprise" },

  /******************
   **** Thailand ****
   ******************/
  { "520",   "01", "Advanced Info Service", "Advanced Info Service" },
  { "520",  "15", "ACT Mobile",            "ACT Mobile" },
  { "520",  "18", "DTAC",                  "Total Access Communication" },
  { "520",  "23", "Advanced Info Service", "Advanced Info Service" },
  { "520",  "99", "True Move",             "True Move" },

  /*******************
   **** Singapore ****
   *******************/
  { "525",   "01", "SingTel",                       "Singapore Telecom" },
  { "525",   "02", "SingTel-G18",                   "Singapore Telecom" },
  { "525",   "03", "M1",                            "MobileOne Asia" },
  { "525",   "05", "StarHub",                       "StarHub Mobile" },

  /****************
   **** Brunei ****
   ****************/
  { "528",   "02", "B-Mobile", "B-Mobile Communications Sdn Bhd" },
  { "528",  "11", "DTSCom",   "DataStream Technology" },

  /*********************
   **** New Zealand ****
   *********************/
  { "530",   "01", "Vodafone", "Vodafone New Zealand" },
  { "530",   "03", "Woosh",    "Woosh wireless New Zealand" },
  { "530",   "05", "Telecom",  "Telecom New Zealand" },
  { "530",  "24", "NZ Comms", "NZ Communications New Zealand" },

  /**************************
   **** Papua New Guinea ****
   **************************/
  { "537",   "01", "B-Mobile", "Pacific Mobile Communications" },

  /*************************
   **** Solomon Islands ****
   *************************/
  { "540",   "01", "BREEZE", "Solomon Telekom Co Ltd" },

  /*****************
   **** Vanuatu ****
   *****************/
  { "541",   "01", "SMILE", "telecom Vanuatu Ltd" },

  /**************
   **** Fiji ****
   **************/
  { "542",   "01", "Vodafone", "Vodafone Fiji" },

  /******************
   **** Kiribati ****
   ******************/
  { "545",   "09", "Kiribati Frigate", "Telecom services Kiribati Ltd" },

  /***********************
   **** New Caledonia ****
   ***********************/
  { "546",   "01", "Mobilis", "OPT New Caledonia" },

  /**************************
   **** French Polynesia ****
   **************************/
  { "547",  "20", "VINI", "Tikiphone SA" },

  /************************************
   **** Cook Islands (New Zealand) ****
   ************************************/
  { "548",   "01", "Telecom Cook", "Telecom Cook" },

  /***************
   **** Samoa ****
   ***************/
  { "549",   "01", "Digicel",  "Digicel Pacific Ltd." },
  { "549",  "27", "SamoaTel", "SamoaTel Ltd" },

  /********************
   **** Micronesia ****
   ********************/
  { "550",   "01", "FSM Telecom", "FSM Telecom" },

  /***************
   **** Palau ****
   ***************/
  { "552",   "01", "PNCC",         "Palau National Communications Corp." },
  { "552",  "80", "Palau Mobile", "Palau Mobile Corporation" },

  /***************
   **** Egypt ****
   ***************/
  { "602",   "01", "Mobinil",  "ECMS-Mobinil" },
  { "602",   "02", "Vodafone", "Vodafone Egypt" },
  { "602",   "03", "etisalat", "Etisalat Egypt" },

  /*****************
   **** Algeria ****
   *****************/
  { "603",   "01", "Mobilis", "ATM Mobilis" },
  { "603",   "02", "Djezzy", "Orascom Telecom Algerie Spa" },
  { "603",   "03", "Nedjma", "Wataniya Telecom Algerie" },

  /*****************
   **** Morocco ****
   *****************/
  { "604",   "00", "Meditel", "Medi Telecom" },
  { "604",   "01", "IAM",     "Ittissalat Al Maghrib (Maroc Telecom)" },

  /*****************
   **** Tunisia ****
   *****************/
  { "605",   "02", "Tunicell", "Tunisie Telecom" },
  { "605",   "03", "Tunisiana", "Orascom Telecom Tunisie" },

  /***************
   **** Libya ****
   ***************/
  { "606",   "00", "Libyana", "Libyana" },
  { "606",   "01", "Madar",   "Al Madar" },

  /*******************
   **** Mauritius ****
   *******************/
  { "609",   "01", "Mattel",   "Mattel" },
  { "609",  "10", "Mauritel", "Mauritel Mobiles" },

  /**************
   **** Mali ****
   **************/
  { "610",   "01", "Malitel", "Malitel SA" },
  { "610",   "02", "Orange",  "Orange Mali SA" },

  /****************
   **** Guinea ****
   ****************/
  { "611",   "02", "Lagui",          "Sotelgui Lagui" },
  { "611",   "03", "Telecel Guinee", "INTERCEL Guinee" },
  { "611",   "04", "MTN",            "Areeba Guinea" },

  /*********************
   **** Ivory Coast ****
   *********************/
  { "612",   "02", "Moov",   "Moov" },
  { "612",   "03", "Orange", "Orange" },
  { "612",   "04", "KoZ",    "Comium Ivory Coast Inc" },
  { "612",   "05", "MTN",    "MTN" },
  { "612",   "06", "ORICEL", "ORICEL" },

  /**********************
   **** Burkina Faso ****
   **********************/
  { "613",   "01", "Onatel",       "Onatel" },
  { "613",   "02", "Zain",         "Celtel Burkina Faso" },
  { "613",   "03", "Telecel Faso", "Telecel Faso SA" },

  /*****************
   **** Nigeria ****
   *****************/
  { "614",   "01", "SahelCom", "SahelCom" },
  { "614",   "02", "Zain",     "Celtel Niger" },
  { "614",   "03", "Telecel",  "Telecel Niger SA" },
  { "614",   "04", "Orange",   "Orange Niger" },

  /**************
   **** Togo ****
   **************/
  { "615",   "01", "Togo Cell", "Togo Telecom" },
  { "615",   "05", "Telecel",   "Telecel Togo" },

  /***************
   **** Benin ****
   ***************/
  { "616",   "00", "BBCOM",   "Bell Benin Communications" },
  { "616",   "02", "Telecel", "Telecel Benin Ltd" },
  { "616",   "03", "Areeba",  "Spacetel Benin" },

  /*******************
   **** Mauritius ****
   *******************/
  { "617",   "01", "Orange", "Cellplus Mobile Communications Ltd" },
  { "617",  "10", "Emtel",  "Emtel Ltd" },

  /*****************
   **** Liberia ****
   *****************/
  { "618",   "01", "LoneStar Cell", "Lonestar Communications Corporation" },

  /***************
   **** Ghana ****
   ***************/
  { "620",   "01", "MTN",                   "ScanCom Ltd" },
  { "620",   "02", "Ghana Telecomi Mobile", "Ghana Telecommunications Company Ltd" },
  { "620",   "03", "tiGO",                  "Millicom Ghana Limited" },

  /*****************
   **** Nigeria ****
   *****************/
  { "621",  "20", "Zain",  "Celtel Nigeria Ltd." },
  { "621",  "30", "MTN",   "MTN Nigeria Communications Limited" },
  { "621",  "40", "M-Tel", "Nigerian Mobile Telecommunications Limited" },
  { "621",  "50", "Glo",   "Globacom Ltd" },

  /**************
   **** Chad ****
   **************/
  { "622",   "01", "Zain",            "CelTel Tchad SA" },
  { "622",   "03", "TIGO - Millicom", "TIGO - Millicom" },

  /**********************************
   **** Central African Republic ****
   **********************************/
  { "623",   "01", "CTP", "Centrafrique Telecom Plus" },
  { "623",   "02", "TC", "iTelecel Centrafrique" },
  { "623",   "03", "Orange", "Orange RCA" },
  { "623",   "04", "Nationlink", "Nationlink Telecom RCA" },

  /******************
   **** Cameroon ****
   ******************/
  { "624",   "01", "MTN-Cameroon", "Mobile Telephone Network Cameroon Ltd" },
  { "624",   "02", "Orange",       "Orange Cameroun S.A." },

  /********************
   **** Cabo Verde ****
   ********************/
  { "625",   "01", "CMOVEL", "CVMovel, S.A." },

  /*******************************
   **** Sao Tome and Principe ****
   *******************************/
  { "626",   "01", "CSTmovel", "Companhia Santomese de Telecomunicacoe" },

  /**************************
   *** Equatorial Guinea ****
   **************************/
  { "627",   "01", "Orange GQ", "GETESA" },

  /***************
   **** Gabon ****
   ***************/
  { "628",   "01", "Libertis",                  "Libertis S.A." },
  { "628",   "02", "Moov (Telecel) Gabon S.A.", "Moov (Telecel) Gabon S.A." },
  { "628",   "03", "Zain",                      "Celtel Gabon S.A." },

  /*******************************
   **** Republic of the Congo ****
   *******************************/
  { "629",  "10", "Libertis Telecom", "MTN CONGO S.A" },

  /******************************************
   **** Democratic Republic of the Congo ****
   ******************************************/
  { "630",   "01", "Vodacom",      "Vodacom Congo RDC sprl" },
  { "630",   "02", "Zain",         "Celtel Congo" },
  { "630",   "05", "Supercell",    "Supercell SPRL" },
  { "630",  "86", "CCT",          "Congo-Chine Telecom s.a.r.l" },
  { "630",  "89", "SAIT Telecom", "OASIS SPRL" },

  /*****************
   **** Angola ****
   *****************/
  { "631",   "02", "UNITEL", "UNITEL S.a.r.l." },

  /***********************
   **** Guinea-Bissau ****
   ***********************/
  { "632",   "02", "Areeba", "Spacetel Guine-Bissau S.A." },

  /********************
   **** Seychelles ****
   ********************/
  { "633",   "02", "Mdeiatech International", "Mdeiatech International Ltd." },

  /***************
   **** Sudan ****
   ***************/
  { "634",   "01", "Mobitel/Mobile Telephone Company", "Mobitel/Mobile Telephone Company" },
  { "634",   "02", "MTN",                              "MTN Sudan" },

  /****************
   **** Rwanda ****
   ****************/
  { "635",  "10", "MTN", "MTN Rwandacell SARL" },

  /******************
   **** Ethiopia ****
   ******************/
  { "636",   "01", "ETMTN", "Ethiopian Telecommmunications Corporation" },

  /*****************
   **** Somalia ****
   *****************/
  { "637",   "04", "Somafona",       "Somafona FZLLC" },
  { "637",  "10", "Nationalink",    "Nationalink" },
  { "637",  "19", "Hormuud",        "Hormuud Telecom Somalia Inc" },
  { "637",  "30", "Golis",          "Golis Telecommunications Company" },
  { "637",  "62", "Telcom Mobile",  "Telcom Mobile" },
  { "637",  "65", "Telcom Mobile",  "Telcom Mobile" },
  { "637",  "82", "Telcom Somalia", "Telcom Somalia" },

  /******************
   **** Djibouti ****
   ******************/
  { "638",   "01", "Evatis", "Djibouti Telecom SA" },

  /***************
   **** Kenya ****
   ***************/
  { "639",   "02", "Safaricom",    "Safaricom Limited" },
  { "639",   "03", "Zain",         "Celtel Kenya Limited" },
  { "639",   "07", "Orange Kenya", "Telkom Kemya" },

  /******************
   **** Tanzania ****
   ******************/
  { "640",   "02", "Mobitel", "MIC Tanzania Limited" },
  { "640",   "03", "Zantel",  "Zanzibar Telecom Ltd" },
  { "640",   "04", "Vodacom", "Vodacom Tanzania Limited" },

  /****************
   **** Uganda ****
   ****************/
  { "641",  "10", "MTN",                 "MTN Uganda" },
  { "641",  "14", "Orange",              "Orange Uganda" },
  { "641",  "22", "Warid Telecom",       "Warid Telecom" },

  /*****************
   **** Burundi ****
   *****************/
  { "642",   "01", "Spacetel", "Econet Wireless Burundi PLC" },
  { "642",   "02", "Aficell",  "Africell PLC" },
  { "642",   "03", "Telecel",  "Telecel Burundi Company" },

  /********************
   **** Mozambique ****
   ********************/
  { "643",   "01", "mCel",    "Mocambique Celular S.A.R.L." },

  /****************
   **** Zambia ****
   ****************/
  { "645",   "01", "Zain",   "Zain" },
  { "645",   "02", "MTN",    "MTN" },
  { "645",   "03", "ZAMTEL", "ZAMTEL" },

  /********************
   **** Madagascar ****
   ********************/
  { "646",   "01", "Zain",   "Celtel" },
  { "646",   "02", "Orange", "Orange Madagascar S.A." },
  { "646",   "04", "Telma",  "Telma Mobile S.A." },

  /**************************
   **** Reunion (France) ****
   **************************/
  { "647",   "00", "Orange",      "Orange La Reunion" },
  { "647",   "02", "Outremer",    "Outremer Telecom" },
  { "647",  "10", "SFR Reunion", "Societe Reunionnaisei de Radiotelephone" },

  /******************
   **** Zimbabwe ****
   ******************/
  { "648",   "01", "Net*One", "Net*One cellular (Pvt) Ltd" },
  { "648",   "03", "Telecel", "Telecel Zimbabwe (PVT) Ltd" },
  { "648",   "04", "Econet",  "Econet Wireless (Private) Limited" },

  /*****************
   **** Namibia ****
   *****************/
  { "649",   "01", "MTC",      "MTC Namibia" },
  { "649",   "03", "Cell One", "Telecel Globe (Orascom)" },

  /****************
   **** Malawi ****
   ****************/
  { "650",   "01", "TNM",  "Telecom Network Malawi" },
  { "650",  "10", "Zain", "Celtel Limited" },

  /*****************
   **** Lesotho ****
   *****************/
  { "651",   "01", "Vodacom",          "Vodacom Lesotho (Pty) Ltd" },

  /******************
   **** Botswana ****
   ******************/
  { "652",   "01", "Mascom",     "Mascom Wirelessi (Pty) Limited" },
  { "652",   "02", "Orange",     "Orange (Botswans) Pty Limited" },
  { "652",   "04", "BTC Mobile", "Botswana Telecommunications Corporation" },

  /**********************
   **** South Africa ****
   **********************/
  { "655",   "01", "Vodacom",                          "Vodacom" },
  { "655",   "02", "Telkom",                           "Telkom" },
  { "655",   "07", "Cell C",                           "Cell C" },
  { "655",  "10", "MTN",                              "MTN Group" },

  /*****************
   **** Eritrea ****
   *****************/
  { "657",   "01", "Eritel", "Eritel Telecommunications Services Corporation" },

  /****************
   **** Belize ****
   ****************/
  { "702",  "67", "Belize Telemedia",                      "Belize Telemedia" },
  { "702",  "68", "International Telecommunications Ltd.", "International Telecommunications Ltd." },

  /*******************
   **** Guatemala ****
   *******************/
  { "704",   "01", "Claro",         "Servicios de Comunicaciones Personales Inalambricas (SRECOM)" },
  { "704",   "02", "Comcel / Tigo", "Millicom / Local partners" },
  { "704",   "03", "movistar",      "Telefonica Moviles Guatemala (Telefonica)" },

  /*********************
   **** El Salvador ****
   *********************/
  { "706",   "01", "CTE Telecom Personal",  "CTE Telecom Personal SA de CV" },
  { "706",   "02", "digicel",               "Digicel Group" },
  { "706",   "03", "Telemovil EL Salvador", "Telemovil EL Salvador S.A" },
  { "706",   "04", "movistar",              "Telfonica Moviles El Salvador" },
  { "706",  "10", "Claro",                 "America Movil" },

  /******************
   **** Honduras ****
   ******************/
  { "708",   "01", "Claro",          "Servicios de Comunicaciones de Honduras S.A. de C.V." },
  { "708",   "02", "Celtel / Tigo",  "Celtel / Tigo" },
  { "708",   "04", "DIGICEL",        "Digicel de Honduras" },
  { "708",  "30", "Hondutel",       "Empresa Hondurena de telecomunicaciones" },

  /*******************
   **** Nicaragua ****
   *******************/
  { "710",  "21", "Claro",    "Empresa Nicaraguense de Telecomunicaciones,S.A." },
  { "710",  "30", "movistar", "Telefonica Moviles de Nicaragua S.A." },
  { "710",  "73", "SERCOM",   "Servicios de Comunicaciones S.A." },

  /*******************
   **** Cost Rica ****
   *******************/
  { "712",   "01", "ICE", "Instituto Costarricense de Electricidad" },
  { "712",   "02", "ICE", "Instituto Costarricense de Electricidad" },

  /****************
   **** Panama ****
   ****************/
  { "714",   "01", "Cable & Wireless", "Cable & Wireless Panama S.A." },
  { "714",   "02", "movistar",         "Telefonica Moviles Panama S.A" },
  { "714",   "04", "Digicel",          "Digicel (Panama) S.A." },

  /**************
   **** Peru ****
   **************/
  { "716",   "06", "movistar", "Telefonica Moviles Peru" },
  { "716",  "10", "Claro",    "America Movil Peru" },

  /*******************
   **** Argentina ****
   *******************/
  { "722",  "10", "Movistar", "Telefonica Moviles Argentina SA" },
  { "722",  "70", "Movistar", "Telefonica Moviles Argentina SA" },
  { "722", "310", "Claro",    "AMX Argentina S.A" },
  { "722", "320", "Claro",    "AMX Argentina S.A" },
  { "722", "330", "Claro",    "AMX Argentina S.A" },
  { "722", "340", "Personal", "Teecom Personal SA" },

  /****************
   **** Brazil ****
   ****************/
  { "724",   "02", "TIM",                   "Telecom Italia Mobile" },
  { "724",   "03", "TIM",                   "Telecom Italia Mobile" },
  { "724",   "04", "TIM",                   "Telecom Italia Mobile" },
  { "724",   "05", "Claro",                 "Claro (America Movil)" },
  { "724",   "06", "Vivo",                  "Vivo S.A." },
  { "724",   "07", "CTBC Celular",           "CTBC Telecom" },
  { "724",   "08", "TIM",                   "Telecom Italiz Mobile" },
  { "724",  "10", "Vivo",                  "Vivo S.A." },
  { "724",  "11", "Vivo",                  "Vivo S.A." },
  { "724",  "15", "Sercomtel",             "Sercomtel Celular" },
  { "724",  "16", "Oi / Brasil Telecom",   "Brasil Telecom Celular SA" },
  { "724",  "23", "Vivo",                  "Vivo S.A." },
  { "724",  "24", "Oi / Amazonia Celular", "Amazonia Celular S.A." },
  { "724",  "31", "Oi",                    "TNL PCS" },
  { "724",  "37", "aeiou",                 "Unicel do Brasil" },

  /***************
   **** Chile ****
   ***************/
  { "730",   "01", "Entel",    "Entel Pcs" },
  { "730",   "02", "movistar", "Movistar Chile" },
  { "730",   "03", "Claro",    "Claro Chile"},
  { "730",  "10", "Entel",    "Entel Telefonica Movil" },

  /******************
   **** Colombia ****
   ******************/
  { "732", "101", "Comcel",   "Comcel Colombia" },
  { "732", "102", "movistar", "Bellsouth Colombia" },
  { "732", "103", "Tigo",     "Colombia Movil" },
  { "732", "111", "Tigo",     "Colombia Movil" },
  { "732", "123", "movistar", "Telefonica Moviles Colombia" },

  /*******************
   **** Venezuela ****
   *******************/
  { "734",   "01", "Digitel",  "Corporacion Digitel C.A." },
  { "734",   "02", "Digitel",  "Corporacion Digitel C.A." },
  { "734",   "03", "Digitel",  "Corporacion Digitel C.A." },
  { "734",   "04", "movistar", "Telefonica Moviles Venezuela" },
  { "734",   "06", "Movilnet", "Telecommunicaciones Movilnet" },

  /*****************
   **** Bolivia ****
   *****************/
  { "736",   "01", "Nuevatel", "Nuevatel PCS De Bolivia SA" },
  { "736",   "02", "Entel",    "Entel SA" },
  { "736",   "03", "Tigo",     "Telefonica Celular De Bolivia S.A" },

  /****************
   **** Guyana ****
   ****************/
  { "738",   "01", "Digicel", "U-Mobile (Cellular) Inc." },

  /*****************
   **** Ecuador ****
   *****************/
  { "740",   "00", "Movistar", "Otecel S.A." },
  { "740",   "01", "Porta",    "America Movil" },

  /******************
   **** Paraguay ****
   ******************/
  { "744",   "01", "VOX",      "Hola Paraguay S.A." },
  { "744",   "02", "Claro",    "AMX Paraguay S.A." },
  { "744",   "04", "Tigo",     "Telefonica Celular Del Paraguay S.A. (Telecel)" },
  { "744",   "05", "Personal", "Nucleo S.A." },

  /*****************
   **** Uruguay ****
   *****************/
  { "748",   "01", "Ancel",    "Ancel" },
  { "748",   "07", "Movistar", "Telefonica Moviles Uruguay" },
  { "748",  "10", "Claro",    "AM Wireless Uruguay S.A." },

  /*******************
   **** Satellite ****
   *******************/
  { "901",   "01", "ICO",                                          "ICO Satellite Management" },
  { "901",   "02", "Sense Communications International",           "Sense Communications International" },
  { "901",   "03", "Iridium",                                      "Iridium" },
  { "901",   "04", "GlobalStar",                                   "Globalstar" },
  { "901",   "05", "Thuraya RMSS Network",                         "Thuraya RMSS Network" },
  { "901",   "06", "Thuraya Satellite telecommunications Company", "Thuraya Satellite Telecommunications Company" },
  { "901",   "07", "Ellipso",                                      "Ellipso" },
  { "901",   "09", "Tele1 Europe",                                 "Tele1 Europe" },
  { "901",  "10", "ACeS",                                         "ACeS" },
  { "901",  "11", "Immarsat",                                     "Immarsat" },

  /*************
   **** Sea ****
   *************/
  { "901",  "12", "MCP",                                          "Maritime Communications Partner AS" },

  /****************
   **** Ground ****
   ****************/
  { "901",  "13", "GSM.AQ",                                       "GSM.AQ" },

  /*************
   **** Air ****
   *************/
  { "901",  "14", "AeroMobile AS",                                "AeroMobile AS" },
  { "901",  "15", "OnAir Switzerland Sarl",                       "OnAir Switzerland Sarl" },

  /*******************
   **** Satellite ****
   *******************/
  { "901",  "16", "Jasper Systems",                               "Jasper Systems" },
  { "901",  "17", "Navitas",                                      "Navitas" },
  { "901",  "18", "Cingular Wireless",                            "Cingular Wireless" },
  { "901",  "19", "Vodafone Malta Maritime",                      "Vodafone Malta Maritime" }

}; // qcril_qmi_ons_memory_list

// this wild number is beyond uint16, sid and nid could not be this value
#define SID_NID_WILD_NUMBER 0x10000
static const qcril_qmi_ons_3gpp2_memory_entry_type qcril_qmi_ons_3gpp2_memory_list[] =  //3gpp2 static operator table - arranged in ascending order of MCC,
{                                                                                       //then MNC
  { "", "", 4107, SID_NID_WILD_NUMBER, "Sprint", "Sprint" }, // atoi("") value is 0
  { "001", "01", 1, SID_NID_WILD_NUMBER, "Test1-1", "Test PLMN 1-1" },
  { "302", "86", 16384, SID_NID_WILD_NUMBER, "Telus", "Telus" },
  { "310", "00", 4, SID_NID_WILD_NUMBER, "Verizon", "Verizon" },
  { "310", "00", 60, SID_NID_WILD_NUMBER, "Verizon", "Verizon" },
  { "310", "00", 331, SID_NID_WILD_NUMBER, "Test1-2", "Test PLMN 1-2" },
  { "310", "099", 331, SID_NID_WILD_NUMBER, "Test1-3", "Test PLMN 1-3" },
  { "404", "00", 14655, SID_NID_WILD_NUMBER, "Reliance", "Reliance" },
  { "440", "07", 12288, SID_NID_WILD_NUMBER, "KDDI", "KDDI" },
  { "440", "07", 12336, SID_NID_WILD_NUMBER, "KDDI", "KDDI" },
  { "440", "53", 12304, SID_NID_WILD_NUMBER, "KDDI", "KDDI" },
  { "440", "54", 12305, SID_NID_WILD_NUMBER, "KDDI", "KDDI" },
  { "440", "70", 12341, SID_NID_WILD_NUMBER, "KDDI", "KDDI" },
  { "440", "71", 12336, SID_NID_WILD_NUMBER, "KDDI", "KDDI" },
  { "440", "72", 12339, SID_NID_WILD_NUMBER, "KDDI", "KDDI" },
  { "440", "73", 12337, SID_NID_WILD_NUMBER, "KDDI", "KDDI" },
  { "440", "74", 12338, SID_NID_WILD_NUMBER, "KDDI", "KDDI" },
  { "440", "75", 12342, SID_NID_WILD_NUMBER, "KDDI", "KDDI" },
  { "440", "76", 12340, SID_NID_WILD_NUMBER, "KDDI", "KDDI" },
  { "440", "78", 12343, SID_NID_WILD_NUMBER, "KDDI", "KDDI" },
  { "450", "05", 2176, SID_NID_WILD_NUMBER, "SK", "SK" },
  { "450", "05", 2220, SID_NID_WILD_NUMBER, "SK", "SK" },
  { "450", "05", 2221, SID_NID_WILD_NUMBER, "SK", "SK" },
  { "450", "05", 2222, SID_NID_WILD_NUMBER, "SK", "SK" },
  { "450", "05", 2226, SID_NID_WILD_NUMBER, "SK", "SK" },
  { "450", "05", 2236, SID_NID_WILD_NUMBER, "SK", "SK" },
  { "450", "05", 2238, SID_NID_WILD_NUMBER, "SK", "SK" },
  { "454", "29", 10640,SID_NID_WILD_NUMBER, "PCCW", "PCCW" },
  { "455", "02", 11296, SID_NID_WILD_NUMBER, "CT Macao", "CT Macao" },
  { "460", "03", SID_NID_WILD_NUMBER, SID_NID_WILD_NUMBER, "China Telecom", "China Telecom" },
  { "466", "05", 13504, SID_NID_WILD_NUMBER, "APBW", "APBW" },
}; // qcril_qmi_ons_3gpp2_memory_list


static const qcril_qmi_ons_elaboration_memory_entry_type qcril_qmi_ons_elaboration_memory_list[] =
{
    { "23430", "EE (T-Mobile)", "Everything Everywhere Ltd (T-Mobile)" },
    { "23431", "EE (T-Mobile)", "Everything Everywhere Ltd (T-Mobile)" },
    { "23432", "EE (T-Mobile)", "Everything Everywhere Ltd (T-Mobile)" },
    { "23433", "EE (Orange)",   "Everything Everywhere Ltd (Orange)" }
}; // qcril_qmi_ons_elaboration_memory_list

static const qcril_qmi_ccc_mcc_map_type qcril_qmi_ccc_mcc_map[] =
{
    {"1"   , "310"}, {"1242", "364"}, {"1246", "342"}, {"1264", "365"}, {"1268", "344"}, {"1284", "348"},
    {"1345", "346"}, {"1441", "350"}, {"1473", "352"}, {"1649", "338"}, {"1664", "354"}, {"1684", "544"},
    {"1758", "358"}, {"1767", "366"}, {"1784", "360"}, {"1787", "330"}, {"1809", "370"}, {"1829", "370"},
    {"1849", "370"}, {"1868", "374"}, {"1869", "356"}, {"1876", "338"}, {"1939", "330"},

    {"20"  , "602"}, {"211" , "659"}, {"212" , "604"}, {"213" , "603"}, {"216" , "605"}, {"218" , "606"},
    {"220" , "607"}, {"221" , "608"}, {"222" , "609"}, {"223" , "610"}, {"224" , "611"}, {"225" , "612"},
    {"226" , "613"}, {"227" , "614"}, {"228" , "615"}, {"229" , "616"}, {"230" , "617"}, {"231" , "618"},
    {"232" , "619"}, {"233" , "620"}, {"234" , "621"}, {"235" , "622"}, {"236" , "623"}, {"237" , "624"},
    {"238" , "625"}, {"239" , "626"}, {"240" , "627"}, {"241" , "628"}, {"242" , "629"}, {"243" , "630"},
    {"244" , "631"}, {"245" , "632"}, {"248" , "633"}, {"249" , "634"}, {"250" , "635"}, {"251" , "636"},
    {"252" , "637"}, {"253" , "638"}, {"254" , "639"}, {"255" , "640"}, {"256" , "641"}, {"257" , "642"},
    {"258" , "643"}, {"260" , "645"}, {"261" , "646"}, {"262" , "647"}, {"263" , "648"}, {"264" , "649"},
    {"265" , "650"}, {"266" , "651"}, {"267" , "652"}, {"268" , "653"}, {"269" , "654"}, {"27"  , "655"},
    {"291" , "657"}, {"297" , "363"}, {"298" , "288"}, {"299" , "290"},

    {"30"  , "202"}, {"31"  , "204"}, {"32"  , "206"}, {"33"  , "208"}, {"34"  , "214"}, {"350" , "266"},
    {"351" , "268"}, {"352" , "270"}, {"353" , "272"}, {"354" , "274"}, {"355" , "276"}, {"356" , "278"},
    {"357" , "280"}, {"358" , "244"}, {"359" , "284"}, {"36"  , "216"}, {"370" , "246"}, {"371" , "247"},
    {"372" , "248"}, {"373" , "259"}, {"374" , "283"}, {"375" , "257"}, {"376" , "213"}, {"377" , "212"},
    {"378" , "292"}, {"380" , "255"}, {"381" , "220"}, {"382" , "297"}, {"385" , "219"},
    {"386" , "293"}, {"387" , "218"}, {"389" , "294"}, {"39"  , "222"},

    {"40"  , "226"}, {"41"  , "228"}, {"420" , "230"}, {"421" , "231"}, {"423" , "295"}, {"43"  , "232"},
    {"44"  , "234"}, {"45"  , "238"}, {"46"  , "240"}, {"47"  , "242"}, {"48"  , "260"}, {"49"  , "262"},

    {"501" , "702"}, {"502" , "704"}, {"503" , "706"}, {"504" , "708"}, {"505" , "710"}, {"506" , "712"},
    {"507" , "714"}, {"508" , "308"}, {"509" , "372"}, {"51"  , "716"}, {"52"  , "334"}, {"53"  , "368"},
    {"54"  , "722"}, {"55"  , "724"}, {"56"  , "730"}, {"57"  , "732"}, {"58"  , "734"}, {"590" , "340"},
    {"591" , "736"}, {"592" , "738"}, {"593" , "740"}, {"595" , "744"}, {"596" , "340"}, {"597" , "746"},
    {"598" , "748"},

    {"60"  , "502"}, {"61"  , "505"}, {"62"  , "510"}, {"63"  , "515"}, {"64"  , "530"}, {"65"  , "525"},
    {"66"  , "520"}, {"670" , "514"}, {"672" , "505"}, {"673" , "528"}, {"674" , "536"}, {"675" , "537"},
    {"676" , "539"}, {"677" , "540"}, {"678" , "541"}, {"679" , "542"}, {"680" , "552"}, {"682" , "548"},
    {"683" , "555"}, {"685" , "549"}, {"686" , "545"}, {"687" , "546"}, {"688" , "553"}, {"689" , "547"},
    {"691" , "550"},

    {"7"   , "250"}, {"73"  , "250"}, {"74"  , "250"}, {"76"  , "401"}, {"77"  , "401"}, {"78"  , "250"},

    {"81"  , "440"}, {"82"  , "450"}, {"84"  , "452"}, {"850" , "467"}, {"852" , "454"}, {"853" , "455"},
    {"855" , "456"}, {"856" , "457"}, {"86"  , "460"}, {"870" , "901"}, {"880" , "470"}, {"886" , "466"},


    {"90"  , "286"}, {"91"  , "404"}, {"92"  , "410"}, {"93"  , "412"}, {"94"  , "413"}, {"95"  , "414"},
    {"960" , "472"}, {"961" , "415"}, {"962" , "416"}, {"963" , "417"}, {"964" , "418"}, {"965" , "419"},
    {"966" , "420"}, {"967" , "421"}, {"968" , "422"}, {"970" , "425"}, {"971" , "424"}, {"972" , "425"},
    {"973" , "426"}, {"974" , "427"}, {"975" , "402"}, {"976" , "428"}, {"977" , "429"}, {"98"  , "432"},
    {"992" , "436"}, {"993" , "438"}, {"994" , "400"}, {"995" , "282"}, {"996" , "437"}, {"998" , "434"},

    {NULL, NULL}
}; //qcril_qmi_ccc_mcc_map

static nas_radio_if_enum_v01 acq_order_map_table[][NAS_ACQ_ORDER_LIST_MAX_V01] =
{
    [QCRIL_QMI_ACQ_ORDER_NONE] = {0},
    [QCRIL_QMI_ACQ_ORDER_LTE_TDS_GSM] = {NAS_RADIO_IF_LTE_V01, NAS_RADIO_IF_TDSCDMA_V01, NAS_RADIO_IF_GSM_V01},
    [QCRIL_QMI_ACQ_ORDER_TDS_GSM_LTE] = {NAS_RADIO_IF_TDSCDMA_V01, NAS_RADIO_IF_GSM_V01, NAS_RADIO_IF_LTE_V01},
    [QCRIL_QMI_ACQ_ORDER_LTE_UMTS_GSM] = {NAS_RADIO_IF_LTE_V01, NAS_RADIO_IF_UMTS_V01, NAS_RADIO_IF_GSM_V01},
}; //acq_order_map_table

struct sar_rf_state
{
  int rf_state_index;
  uint32_t compatibility_key;
};

/*===========================================================================

                                FUNCTIONS

===========================================================================*/





/*===========================================================================

  FUNCTION:  qcril_qmi_nas_request_set_preferred_network_type

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE.

    @return
    None.
*/
/*=========================================================================*/


void qcril_qmi_nas_request_set_preferred_network_type
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  int                            mode_pref_param;
  qcril_request_resp_params_type resp;
  RIL_Errno                      ril_req_res = RIL_E_SUCCESS;
  int                            is_deferred;
  uint8 is_change = FALSE;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_NOTUSED( ret_ptr );

  qcril_qmi_arb_reset_pref_data_snapshot();

  do
  {
      if ( params_ptr->datalen == 0 || params_ptr->data == NULL  )
      {
          qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
          qcril_send_request_response( &resp );
          break;
      }

      mode_pref_param = *((int *)params_ptr->data);
      QCRIL_LOG_INFO( "network pref android param %d", mode_pref_param );

      is_deferred = qmi_ril_nwreg_mode_pref_enforcement_deferred_ncl( mode_pref_param );
      if ( !is_deferred )
      { // set now
        ril_req_res =  qmi_ril_nwreg_request_mode_pref( mode_pref_param, &is_change );
        if( ( TRUE == is_change ) && ( RIL_E_SUCCESS == ril_req_res ) )
        {
            ril_req_res = qcril_qmi_nas_mode_pref_request_response_helper( params_ptr, &is_change );
        }
      }

      if( ( RIL_E_SUCCESS != ril_req_res ) || ( FALSE == is_change ) )
      {
        qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_req_res, &resp );
        qcril_send_request_response( &resp );
      }
  } while ( FALSE );

  QCRIL_LOG_FUNC_RETURN();

} /* qcril_qmi_nas_request_set_preferred_network_type() */

/*===========================================================================

  FUNCTION:  qcril_qmi_nas_request_get_preferred_network_band_pref

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_GET_PREFERRED_NETWORK_BAND_PREF

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_nas_request_get_preferred_network_band_pref
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_request_resp_params_type resp;
  qcril_qmi_band_pref_e_type     deferred_band_pref_map = QCRIL_QMI_BAND_PREF_NONE;
  qcril_qmi_rat_band_e_type      band_type = QCRIL_QMI_RAT_BAND_NONE;
  uint8_t                        band_pref_valid = FALSE;
  uint8_t                        band_pref_map = QCRIL_QMI_BAND_PREF_NONE;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ret_ptr);

  do {
    if ( params_ptr->datalen == 0 || params_ptr->data == NULL  )
    {
        QCRIL_LOG_DEBUG("must specify band type");
        qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t,
                                  params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
        qcril_send_request_response( &resp );
        break;
    }

    band_type = *((qcril_qmi_band_pref_e_type*)params_ptr->data);
    band_pref_valid = qcril_qmi_nas_get_band_pref_map( band_type, &deferred_band_pref_map);
    QCRIL_LOG_DEBUG("band_pref_valid: %d band_pref_map:%d band_type: %d",
        band_pref_valid, deferred_band_pref_map, band_type);

    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                         params_ptr->t,
                                         params_ptr->event_id,
                                         RIL_E_SUCCESS,
                                         &resp );

    if ( band_pref_valid ) {
        band_pref_map = (uint8_t)deferred_band_pref_map;
    }
    resp.resp_pkt = (void *)&band_pref_map;
    resp.resp_len = sizeof(band_pref_map);
    qcril_send_request_response( &resp );
  } while ( FALSE );

  QCRIL_LOG_FUNC_RETURN();

} /* qcril_qmi_nas_request_get_preferred_network_band_pref() */

//===========================================================================
/*===========================================================================

  FUNCTION:  qcril_qmi_nas_request_set_preferred_network_band_pref

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_SET_PREFERRED_NETWORK_BAND_PREF.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_nas_request_set_preferred_network_band_pref
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  uint8_t                        deferred_band_pref_valid = FALSE;
  qcril_request_resp_params_type resp;
  qcril_qmi_band_pref_e_type     band_pref_map = QCRIL_QMI_BAND_PREF_NONE;
  RIL_Errno                      ril_req_res = RIL_E_GENERIC_FAILURE;
  int                            res = FALSE;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED( ret_ptr );
  do
  {
      if ( params_ptr->datalen == 0 || params_ptr->data == NULL  )
      {
          QCRIL_LOG_ERROR("must specify band_pref_map");
          qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t,
                                    params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
          qcril_send_request_response( &resp );
          break;
      }

      band_pref_map = *((qcril_qmi_band_pref_e_type*)params_ptr->data);

      switch ( band_pref_map )
      {
          case QCRIL_QMI_BAND_PREF_LTE_FULL:
          case QCRIL_QMI_BAND_PREF_TDD_LTE:
          case QCRIL_QMI_BAND_PREF_FDD_LTE:
                res = qmi_ril_nas_cache_deferred_band_pref(QCRIL_QMI_LTE_BAND, band_pref_map);
            break;

          default:
            res = FALSE;
            break;
      }

      if ( res == TRUE)
      {
          ril_req_res = RIL_E_SUCCESS;
      } else {
          ril_req_res = RIL_E_GENERIC_FAILURE;
      }

      QCRIL_LOG_DEBUG("band_pref_map:%d res:%d", band_pref_map, res);
      qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t,
                                params_ptr->event_id, ril_req_res, &resp );
      qcril_send_request_response( &resp );

  } while ( FALSE );
}


/*===========================================================================

  FUNCTION:  qcril_qmi_nas_request_set_preferred_network_acq_order

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_SET_PREFERRED_NETWORK_ACQ_ORDER.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_nas_request_set_preferred_network_acq_order
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_request_resp_params_type resp;
  qcril_qmi_acq_order_e_type     acq_order_map;
  uint32_t                       deferred_acq_order_len = 0;
  RIL_Errno                      ril_req_res;
  nas_radio_if_enum_v01          cached_acq_order[NAS_ACQ_ORDER_LIST_MAX_V01];
  nas_radio_if_enum_v01          deferred_acq_order[NAS_ACQ_ORDER_LIST_MAX_V01];

  qcril_qmi_acq_order_e_type     tmp_deferred_acq_order_map = QCRIL_QMI_ACQ_ORDER_NONE;
  uint8_t                        tmp_deferred_acq_order_valid = FALSE;
  uint8_t                        cached_acq_order_valid = FALSE;
  uint32_t                       cached_acq_order_len  = 0;
  uint32_t                       count = 0;
  uint32_t                       i = 0;
  uint32_t                       j = 0;
  int                            exit_flag = 0;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_NOTUSED( ret_ptr );
  do
  {
      if ( params_ptr->datalen == 0 || params_ptr->data == NULL  )
      {
          QCRIL_LOG_ERROR("data is NULL or datalen is 0");
          qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t,
                                    params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
          qcril_send_request_response( &resp );
          break;
      }

      memset( deferred_acq_order, 0, sizeof(deferred_acq_order) );
      acq_order_map = *((qcril_qmi_acq_order_e_type *)params_ptr->data);

      tmp_deferred_acq_order_valid = qmi_ril_nas_get_deferred_acq_order_map( &tmp_deferred_acq_order_map );
      QCRIL_LOG_DEBUG("deferred acq order valid:%d map:%d acq_order_map:%d",
                        tmp_deferred_acq_order_valid, tmp_deferred_acq_order_map, acq_order_map);
      if ( tmp_deferred_acq_order_valid && tmp_deferred_acq_order_map == acq_order_map )
      {
         qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
         qcril_send_request_response( &resp );
         break;
      }

      switch ( acq_order_map )
      {
          case QCRIL_QMI_ACQ_ORDER_LTE_TDS_GSM:
          case QCRIL_QMI_ACQ_ORDER_TDS_GSM_LTE:
          case QCRIL_QMI_ACQ_ORDER_LTE_UMTS_GSM:
            for ( i = 0; i < NAS_ACQ_ORDER_LIST_MAX_V01; i++ ) {
                if ( acq_order_map_table[acq_order_map][i] != 0 ) {
                    deferred_acq_order_len++;
                }
                else
                {
                    break;
                }
            }
            break;

          default:
            deferred_acq_order_len = 0;
            break;
      }

      QCRIL_LOG_DEBUG( "acq order map len:%d", deferred_acq_order_len );
      if ( deferred_acq_order_len == 0 )
      {
          qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t,
                                    params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
          qcril_send_request_response( &resp );
          break;
      }

      memcpy(deferred_acq_order, acq_order_map_table[acq_order_map],
                deferred_acq_order_len*sizeof(nas_radio_if_enum_v01));

      // fetch sys_sel to make sure acq order is newest
      qcril_qmi_fetch_system_selection_preference();
      cached_acq_order_valid = qcril_qmi_nas_get_acq_order( &cached_acq_order_len, cached_acq_order );
      for ( i = 0; i < cached_acq_order_len && i < NAS_ACQ_ORDER_LIST_MAX_V01; i++ )
      {
        QCRIL_LOG_DEBUG( "cached acq order valid:%d index:%d mode:%d", cached_acq_order_valid, i, cached_acq_order[i]);
      }

      if ( cached_acq_order_valid && deferred_acq_order_len <= cached_acq_order_len )
      {
          // check whether requested acq order was included in cached order
          for ( j = 0; j < deferred_acq_order_len; j++ )
          {
              for ( i = 0; i < cached_acq_order_len; i++ )
              {
                  if ( deferred_acq_order[j] == cached_acq_order[i] )
                  {
                      break;
                  }
              }

              if ( i == cached_acq_order_len )
              {
                  QCRIL_LOG_DEBUG("request mode:%d not include in acq order", deferred_acq_order[j]);
                  exit_flag = 1;
                  break;
              }
          }

          if ( exit_flag == 1 )
          {
              qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t,
                                        params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
              qcril_send_request_response( &resp );
              break;
          }

          count = deferred_acq_order_len;

          // must re-compose acq order
          for ( i = 0; i < cached_acq_order_len && count != cached_acq_order_len; i++ )
          {
              for ( j = 0; j < deferred_acq_order_len; j++ )
              {
                  if ( cached_acq_order[i] == deferred_acq_order[j] )
                  {
                      break;
                  }
              }

              if ( j == deferred_acq_order_len )
              {
                  deferred_acq_order[count++] = cached_acq_order[i];
              }
          }
      }
      else
      {
          qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t,
                                    params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
          qcril_send_request_response( &resp );
          break;
      }

      // for debug
      for ( i = 0; i < cached_acq_order_len; i++ )
      {
        QCRIL_LOG_DEBUG( "deferred acq order index:%d mode:%d", i, deferred_acq_order[i]);
      }
      // cache acq order
      qmi_ril_nas_cache_deferred_acq_order( cached_acq_order_len, acq_order_map, deferred_acq_order );

      qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
      qcril_send_request_response( &resp );

  } while ( FALSE );

  QCRIL_LOG_FUNC_RETURN();

} /* qcril_qmi_nas_request_set_preferred_network_acq_order() */


/*===========================================================================

  FUNCTION:  qcril_qmi_nas_request_get_preferred_network_acq_order

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_GET_PREFERRED_NETWORK_ACQ_ORDER.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_nas_request_get_preferred_network_acq_order
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_request_resp_params_type resp;
  qcril_qmi_acq_order_e_type     deferred_acq_order_map = QCRIL_QMI_ACQ_ORDER_NONE;
  uint8_t                        deferred_acq_order_valid = FALSE;
  uint8_t                        acq_order_map = QCRIL_QMI_ACQ_ORDER_NONE;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ret_ptr);

  deferred_acq_order_valid = qmi_ril_nas_get_deferred_acq_order_map( &deferred_acq_order_map );

  QCRIL_LOG_DEBUG("deferred acq order valid:%d map:%d", deferred_acq_order_valid, deferred_acq_order_map);

  if ( deferred_acq_order_valid )
  {
      acq_order_map = (uint8_t)deferred_acq_order_map;
  }

  qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                       params_ptr->t,
                                       params_ptr->event_id,
                                       RIL_E_SUCCESS,
                                       &resp );

  resp.resp_pkt = (void *)&acq_order_map;
  resp.resp_len = sizeof(acq_order_map);

  qcril_send_request_response( &resp );
  QCRIL_LOG_FUNC_RETURN();

} /* qcril_qmi_nas_request_get_preferred_network_acq_order() */

//===========================================================================
//qmi_ril_nwreg_request_mode_pref
//===========================================================================
RIL_Errno qmi_ril_nwreg_request_mode_pref( int android_mode_pref, uint8 *is_change )
{
    RIL_Errno                                           res;
    qmi_client_error_type                               qmi_client_error;
    nas_set_system_selection_preference_req_msg_v01     set_system_selection_preference_req_msg;
    nas_set_system_selection_preference_resp_msg_v01    set_system_selection_preference_resp_msg;

    boolean same_mode_pref = FALSE;
    uint8_t mode_pref_valid = FALSE;
    uint16_t mode_pref;
    uint8_t gw_acq_order_pref_valid = FALSE;
    uint16_t gw_acq_order_pref;
    uint8_t deferred_acq_order_valid = FALSE;
    nas_radio_if_enum_v01 deferred_acq_order[NAS_ACQ_ORDER_LIST_MAX_V01];
    qcril_qmi_acq_order_e_type deferred_acq_order_map = QCRIL_QMI_ACQ_ORDER_NONE;
    uint32_t deferred_acq_order_len = 0;
    uint8_t cached_acq_order_valid = FALSE;
    nas_radio_if_enum_v01 cached_acq_order[NAS_ACQ_ORDER_LIST_MAX_V01];
    uint32_t cached_acq_order_len = 0;
    uint8_t set_acq_order_flag = FALSE;
    uint32_t i = 0;
    uint8_t lte_disable_cause_valid = FALSE;
    nas_lte_disable_cause_enum_type_v01 lte_disable_cause;
    uint8_t lte_band_pref_valid = FALSE;
    uint64_t lte_band_pref = 0;
    qcril_qmi_band_pref_e_type lte_band_pref_map = QCRIL_QMI_BAND_PREF_NONE;


    QCRIL_LOG_FUNC_ENTRY();

    if ( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_SGLTE ) )
    {
          switch( android_mode_pref )
          {
              case QCRIL_PREF_NET_TYPE_LTE_WCDMA:             // following modes are not supported on SGLTE and hence need to be blocked
              case QCRIL_PREF_NET_TYPE_CDMA_EVDO_AUTO:
              case QCRIL_PREF_NET_TYPE_CDMA_ONLY:
              case QCRIL_PREF_NET_TYPE_EVDO_ONLY:
              case QCRIL_PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
              case QCRIL_PREF_NET_TYPE_LTE_CDMA_EVDO:
              case QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
                  res = RIL_E_MODE_NOT_SUPPORTED;
                  break;

              default:                                  // valid mode
                  res = RIL_E_SUCCESS;
                  break;
          }

          QCRIL_LOG_INFO( "intermed filter %d", (int) res );
    }
    else
    {
        res = RIL_E_SUCCESS;
    }
    QCRIL_LOG_INFO( "intermed filter %d", (int) res );

    if ( RIL_E_SUCCESS == res )
    {
          memset(&set_system_selection_preference_req_msg, 0, sizeof(set_system_selection_preference_req_msg));
          set_system_selection_preference_req_msg.mode_pref_valid =  TRUE;
          switch ( android_mode_pref )
          {
              case QCRIL_PREF_NET_TYPE_GSM_WCDMA:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS;
                  set_system_selection_preference_req_msg.gw_acq_order_pref_valid = TRUE;
                  set_system_selection_preference_req_msg.gw_acq_order_pref = NAS_GW_ACQ_ORDER_PREF_WCDMA_GSM_V01;
                  break;

              case QCRIL_PREF_NET_TYPE_GSM_ONLY:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_GSM;
                  break;

              case QCRIL_PREF_NET_TYPE_WCDMA:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_UMTS;
                  break;

              case QCRIL_PREF_NET_TYPE_GSM_WCDMA_AUTO:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS;
                  set_system_selection_preference_req_msg.gw_acq_order_pref_valid = TRUE;
                  set_system_selection_preference_req_msg.gw_acq_order_pref = NAS_GW_ACQ_ORDER_PREF_AUTOMATIC_V01;
                  break;

              case QCRIL_PREF_NET_TYPE_CDMA_EVDO_AUTO:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_CDMA_HRPD;
                  break;

              case QCRIL_PREF_NET_TYPE_CDMA_ONLY:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_CDMA;
                  break;

              case QCRIL_PREF_NET_TYPE_EVDO_ONLY:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_HRPD;
                  break;

              case QCRIL_PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS_CDMA_HRPD;
                  break;

              case QCRIL_PREF_NET_TYPE_LTE_CDMA_EVDO:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_CDMA_HRPD_LTE;
                  break;

              case QCRIL_PREF_NET_TYPE_LTE_CDMA_EVDO_GSM:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_CDMA_HRPD_LTE;
                  break;

              case QCRIL_PREF_NET_TYPE_LTE_GSM_WCDMA:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS_LTE;
                  break;

              case QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS_CDMA_HRPD_LTE;
                  break;

              case QCRIL_PREF_NET_TYPE_LTE_ONLY:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_LTE;
                  break;

              case QCRIL_PREF_NET_TYPE_LTE_WCDMA:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_UMTS_LTE;
                  break;

              case QCRIL_PREF_NET_TYPE_TD_SCDMA_ONLY:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_TDSCDMA;
                  break;
              case QCRIL_PREF_NET_TYPE_TD_SCDMA_WCDMA:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_UMTS_TDSCDMA;
                  break;
              case QCRIL_PREF_NET_TYPE_TD_SCDMA_LTE:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_TDSCDMA_LTE;
                  break;
              case QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA;
                  break;
              case QCRIL_PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_UMTS_TDSCDMA_LTE;
                  break;
              case QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_LTE:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_LTE;
                  break;
              case QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA;
                  break;
              case QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_LTE;
                  break;
              case QCRIL_PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE;
                  break;
              case QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_CDMA_HRPD_UMTS;
                  break;
              default:
                  set_system_selection_preference_req_msg.mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS;
                  break;
          }


          QCRIL_LOG_INFO("network preference ril pref = %d, qmi mode pref = %d, gw ack ord vld = %d, gw ack ord vld = %d",
                                                             (int)android_mode_pref,
                                                             (int)set_system_selection_preference_req_msg.mode_pref,
                                                             (int)set_system_selection_preference_req_msg.gw_acq_order_pref_valid,
                                                             (int)set_system_selection_preference_req_msg.gw_acq_order_pref
                         );

          mode_pref_valid = qcril_qmi_nas_get_mode_pref(&mode_pref);
          gw_acq_order_pref_valid = qcril_qmi_nas_get_gw_acq_order_pref(&gw_acq_order_pref);
          memset(cached_acq_order, 0, sizeof(cached_acq_order));
          cached_acq_order_valid = qcril_qmi_nas_get_acq_order(&cached_acq_order_len, cached_acq_order);

          if( !mode_pref_valid || !gw_acq_order_pref_valid || !cached_acq_order_valid)
          {
              qcril_qmi_fetch_system_selection_preference();
          }

          mode_pref_valid = qcril_qmi_nas_get_mode_pref(&mode_pref);
          gw_acq_order_pref_valid = qcril_qmi_nas_get_gw_acq_order_pref(&gw_acq_order_pref);
          cached_acq_order_valid = qcril_qmi_nas_get_acq_order(&cached_acq_order_len, cached_acq_order);

          memset(deferred_acq_order, 0, sizeof(deferred_acq_order));
          deferred_acq_order_valid = qmi_ril_nas_get_deferred_acq_order(&deferred_acq_order_len, deferred_acq_order);

          QCRIL_LOG_DEBUG("deferred acq order valid:%d len:%d; cached acq order valid:%d len:%d",
                    deferred_acq_order_valid, deferred_acq_order_len, cached_acq_order_valid, cached_acq_order_len);

          if ( deferred_acq_order_valid && cached_acq_order_valid )
          {
              for ( i = 0; i < NAS_ACQ_ORDER_LIST_MAX_V01; i++ )
              {
                  QCRIL_LOG_DEBUG("index:%d cached mode:%d deferred mode:%d", i, cached_acq_order[i], deferred_acq_order[i]);
              }
          }

          if ( deferred_acq_order_valid && deferred_acq_order_len == cached_acq_order_len &&
               ( deferred_acq_order_len > 0 && deferred_acq_order_len < NAS_ACQ_ORDER_LIST_MAX_V01 ) &&
               ( 0 != memcmp( cached_acq_order, deferred_acq_order, deferred_acq_order_len * sizeof(nas_radio_if_enum_v01)) ) )
          {
              set_acq_order_flag = TRUE;
              set_system_selection_preference_req_msg.acq_order_valid = TRUE;
              set_system_selection_preference_req_msg.acq_order_len = deferred_acq_order_len;
              memcpy( set_system_selection_preference_req_msg.acq_order, deferred_acq_order,
                                            deferred_acq_order_len*sizeof(nas_radio_if_enum_v01) );
          }

          lte_disable_cause_valid = qcril_qmi_nas_get_lte_disable_cause(&lte_disable_cause);
          lte_band_pref_valid = qcril_qmi_nas_get_band_pref(QCRIL_QMI_LTE_BAND, &lte_band_pref);
          QCRIL_LOG_DEBUG("lte_band_pref_valid:%d, lte_band_pref:0x%llx", lte_band_pref_valid, lte_band_pref);

          if( !mode_pref_valid || (mode_pref != set_system_selection_preference_req_msg.mode_pref) ||
              set_acq_order_flag == TRUE ||
              ( ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS == set_system_selection_preference_req_msg.mode_pref ) &&
              (( FALSE == gw_acq_order_pref_valid ) ||
              ( gw_acq_order_pref != set_system_selection_preference_req_msg.gw_acq_order_pref )) ) ||
              (lte_disable_cause_valid &&
                      (NAS_LTE_DISABLE_CAUSE_DOM_SEL_V01 == lte_disable_cause ||
                       NAS_LTE_DISABLE_CAUSE_DAM_V01 == lte_disable_cause)) ||
              (lte_band_pref_valid)
            )
          {
              qcril_qmi_nas_initialize_is_indication_received();
              if( is_change )
              {
                  *is_change = TRUE;
              }

              if( ( !mode_pref_valid || ( qcril_qmi_nas_get_radio_tech(mode_pref) != RADIO_TECH_3GPP2 ) ) &&
                  ( qcril_qmi_nas_get_radio_tech(set_system_selection_preference_req_msg.mode_pref) == RADIO_TECH_3GPP2 ) )
              {
                  //Mode changing from 3gpp to 3gpp2
                  set_system_selection_preference_req_msg.net_sel_pref_valid = TRUE;
                  set_system_selection_preference_req_msg.net_sel_pref.net_sel_pref = NAS_NET_SEL_PREF_AUTOMATIC_V01;
              }

              if ( lte_band_pref_valid ) {
                  set_system_selection_preference_req_msg.lte_band_pref_valid = TRUE;
                  set_system_selection_preference_req_msg.lte_band_pref = lte_band_pref;
              }

              qmi_client_error = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle ( QCRIL_QMI_CLIENT_NAS ),
                                                                 QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                                                 (void*) &set_system_selection_preference_req_msg,
                                                                 sizeof(set_system_selection_preference_req_msg),
                                                                 (void*) &set_system_selection_preference_resp_msg,
                                                                 sizeof(set_system_selection_preference_resp_msg),
                                                                 QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT );
              res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_client_error, &set_system_selection_preference_resp_msg.resp );
              QCRIL_LOG_INFO(".. feedback %d / %d", (int) res, (int)set_system_selection_preference_resp_msg.resp.error );
          }
          else
          {
              QCRIL_LOG_INFO("ignore this request as same mode pref in this request.");
              same_mode_pref = TRUE;
          }
      }

      if( ( RIL_E_SUCCESS == res ) && !same_mode_pref )
      {
        qcril_qmi_arb_reset_pref_data_snapshot();
#ifndef QMI_RIL_UTF
        qmi_ril_nw_reg_initiate_post_cfg_ban_for_data_reg_extrapolation_ncl();
#endif
      }

    QCRIL_LOG_FUNC_RETURN_WITH_RET( res );
    return res;
} // qmi_ril_nwreg_request_mode_pref

/*===========================================================================

  FUNCTION:  qcril_qmi_nas_request_get_preferred_network_type

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE.

    @return
    None.
*/
/*=========================================================================*/

void qcril_qmi_nas_request_get_preferred_network_type
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_request_resp_params_type                    resp;
  nas_get_system_selection_preference_resp_msg_v01  get_system_selection_preference_resp_msg;
  nas_get_mode_pref_resp_msg_v01                    get_mode_pref_resp_msg;
  int                                               preferred_network_type = 0;
  qmi_client_error_type                             qmi_client_error;
  RIL_Errno                                         ril_req_res = RIL_E_GENERIC_FAILURE;
  uint8_t                                           mode_pref_valid = FALSE;
  mode_pref_mask_type_v01                           mode_pref;
  int                                               deferred_mode_pref;


  QCRIL_NOTUSED( ret_ptr );

  mode_pref = 0;

  if ( !qcril_qmi_nas_dms_is_in_online_mode() && qmi_ril_nwreg_mode_pref_is_pending_deferred_enforcement_ncl( &deferred_mode_pref ) )
  {
      qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
      resp.resp_pkt = &deferred_mode_pref;
      resp.resp_len = sizeof(deferred_mode_pref);
      qcril_send_request_response( &resp );
  }
  else
  {
      memset(&get_system_selection_preference_resp_msg, 0, sizeof(get_system_selection_preference_resp_msg));
      if( !qcril_qmi_nas_get_mode_pref_from_nv_10() )
      {

          qmi_client_error =  qcril_qmi_client_send_msg_sync_ex ( QCRIL_QMI_CLIENT_NAS,
                                            QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                            NULL,
                                            0,
                                            &get_system_selection_preference_resp_msg,
                                            sizeof(get_system_selection_preference_resp_msg),
                                            QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT );
          ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_client_error, &get_system_selection_preference_resp_msg.resp );

          if( RIL_E_SUCCESS == ril_req_res && TRUE == get_system_selection_preference_resp_msg.mode_pref_valid)
          {
              mode_pref_valid = TRUE;
              mode_pref       = get_system_selection_preference_resp_msg.mode_pref;
          }
          else
          {
              QCRIL_LOG_INFO( "system selection preference unavailable");
          }
      }
      else
      {
          memset(&get_mode_pref_resp_msg, 0, sizeof(get_mode_pref_resp_msg));

          qmi_client_error =  qcril_qmi_client_send_msg_sync_ex ( QCRIL_QMI_CLIENT_NAS,
                                            QMI_NAS_GET_MODE_PREF_REQ_MSG_V01,
                                            NULL,
                                            0,
                                            &get_mode_pref_resp_msg,
                                            sizeof(get_mode_pref_resp_msg),
                                            QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT );
          ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_client_error, &get_mode_pref_resp_msg.resp );

          // use idx0 in get_mode_pref_resp_msg
          if ( qmi_ril_get_process_instance_id() == QCRIL_DEFAULT_INSTANCE_ID && RIL_E_SUCCESS == ril_req_res && TRUE == get_mode_pref_resp_msg.idx0_mode_pref_valid )
          {
              mode_pref_valid = TRUE;
              mode_pref       = get_mode_pref_resp_msg.idx0_mode_pref;
          }
          else if ( qmi_ril_get_process_instance_id() == QCRIL_SECOND_INSTANCE_ID && RIL_E_SUCCESS == ril_req_res && TRUE == get_mode_pref_resp_msg.idx1_mode_pref_valid )
          {
              mode_pref_valid = TRUE;
              mode_pref       = get_mode_pref_resp_msg.idx1_mode_pref;
          }
          else if ( qmi_ril_get_process_instance_id() == QCRIL_THIRD_INSTANCE_ID && RIL_E_SUCCESS == ril_req_res && TRUE == get_mode_pref_resp_msg.idx2_mode_pref_valid )
          {
              mode_pref_valid = TRUE;
              mode_pref       = get_mode_pref_resp_msg.idx2_mode_pref;
          }
          else
          {
              QCRIL_LOG_INFO( "Mode preference unavailable");
          }
      }

      if ( mode_pref_valid )
      {
          switch ( mode_pref )
          {
              case QMI_NAS_RAT_MODE_PREF_CDMA:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_CDMA_ONLY;
                  break;

              case QMI_NAS_RAT_MODE_PREF_HRPD:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_EVDO_ONLY;
                  break;

              case QMI_NAS_RAT_MODE_PREF_GSM:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_GSM_ONLY;
                  break;

              case QMI_NAS_RAT_MODE_PREF_UMTS:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_WCDMA;
                  break;

              case QMI_NAS_RAT_MODE_PREF_LTE:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_LTE_ONLY;
                  break;

              case QMI_NAS_RAT_MODE_PREF_GSM_UMTS:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_GSM_WCDMA_AUTO;
                  if(get_system_selection_preference_resp_msg.gw_acq_order_pref_valid == TRUE)
                  {
                      if (get_system_selection_preference_resp_msg.gw_acq_order_pref == NAS_GW_ACQ_ORDER_PREF_WCDMA_GSM_V01)
                      {
                          preferred_network_type = QCRIL_PREF_NET_TYPE_GSM_WCDMA;
                      }
                  }
                  break;

              case QMI_NAS_RAT_MODE_PREF_CDMA_HRPD:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_CDMA_EVDO_AUTO;
                  break;

              case QMI_NAS_RAT_MODE_PREF_GSM_UMTS_CDMA_HRPD:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
                  break;

              case QMI_NAS_RAT_MODE_PREF_CDMA_HRPD_LTE:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_LTE_CDMA_EVDO;
                  break;

              case QMI_NAS_RAT_MODE_PREF_GSM_CDMA_HRPD_LTE:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_LTE_CDMA_EVDO_GSM;
                  break;

              case QMI_NAS_RAT_MODE_PREF_GSM_UMTS_LTE:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_LTE_GSM_WCDMA;
                  break;

              case QMI_NAS_RAT_MODE_PREF_GSM_UMTS_CDMA_HRPD_LTE:
              case QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_CDMA_HRPD_LTE:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA;
                  break;

              case QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA;
                  break;

              case QMI_NAS_RAT_MODE_PREF_UMTS_LTE:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_LTE_WCDMA;
                  break;

              case QMI_NAS_RAT_MODE_PREF_TDSCDMA:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_TD_SCDMA_ONLY;
                  break;

              case QMI_NAS_RAT_MODE_PREF_UMTS_TDSCDMA:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_TD_SCDMA_WCDMA;
                  break;

              case QMI_NAS_RAT_MODE_PREF_TDSCDMA_LTE:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_TD_SCDMA_LTE;

                  break;
              case QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM;
                  break;

              case QMI_NAS_RAT_MODE_PREF_UMTS_TDSCDMA_LTE:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE;
                  break;

              case QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_LTE:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_LTE;
                  break;

              case QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA;
                  break;

              case QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_LTE:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE;
                  break;

              case QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_CDMA_HRPD_UMTS:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO;
                  break;

              default:
                  preferred_network_type = QCRIL_PREF_NET_TYPE_GSM_WCDMA;
                  break;
                  QCRIL_LOG_INFO("network preference ril pref = %d, qmi mode pref = %d",
                                 (int)preferred_network_type,
                                 (int)mode_pref );
         }
      }
      else
      {
          QCRIL_LOG_INFO( "Mode preference unavailable");
          preferred_network_type = 0;
      }
      QCRIL_LOG_INFO("network preference ril=%d qmi=%d",preferred_network_type,mode_pref);

      qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_req_res, &resp );
      if( RIL_E_SUCCESS == ril_req_res )
      {
        resp.resp_pkt = &preferred_network_type;
        resp.resp_len = sizeof(preferred_network_type);
      }
      qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(ril_req_res);

} /* qcril_qmi_nas_request_get_preferred_network_type() */


/*===========================================================================

  FUNCTION:  qcril_qmi_nas_request_set_roaming_preference

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE.

    @return
    None.
*/
/*=========================================================================*/

void qcril_qmi_nas_request_set_roaming_preference
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  int *in_data_ptr;
  qcril_request_resp_params_type resp;
  nas_set_system_selection_preference_req_msg_v01 set_system_selection_preference_req_msg;
  nas_set_system_selection_preference_resp_msg_v01 set_system_selection_preference_resp_msg;

  QCRIL_LOG_FUNC_ENTRY();
  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  in_data_ptr = (int *)params_ptr->data;

  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
      if ( params_ptr->datalen == 0 || params_ptr->data == NULL )
  {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
          break;
  }

  memset(&set_system_selection_preference_req_msg, 0, sizeof(set_system_selection_preference_req_msg));
  set_system_selection_preference_req_msg.roam_pref_valid =  TRUE;
  switch(*in_data_ptr)
  {
      case 0:
          set_system_selection_preference_req_msg.roam_pref = 0x0001;
          break;
      case 1:
          set_system_selection_preference_req_msg.roam_pref = 0x0003;
          break;
      case 2:
          set_system_selection_preference_req_msg.roam_pref = 0x00FF;
          break;
      default:
          set_system_selection_preference_req_msg.roam_pref = 0x0000;
  }
  QCRIL_LOG_INFO("roaming preference set %d",set_system_selection_preference_req_msg.roam_pref);

  memset(&set_system_selection_preference_resp_msg, 0, sizeof(set_system_selection_preference_resp_msg));
  /* Send QMI NAS SET ROAMING PREFERENCE TYPE REQ */
  if ( qcril_qmi_client_send_msg_sync ( QCRIL_QMI_CLIENT_NAS,
                                    QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                    &set_system_selection_preference_req_msg,
                                    sizeof(set_system_selection_preference_req_msg),
                                    &set_system_selection_preference_resp_msg,
                                    sizeof(set_system_selection_preference_resp_msg)) != E_SUCCESS )
  {
      /* In case of ARM9 reset, the command callback will never be executed. So, need to
       delete the entry from the ReqList, and call OnRequestComplete() */
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
  }
  else
  {
      if(set_system_selection_preference_resp_msg.resp.result == QMI_RESULT_FAILURE_V01)
      {
          QCRIL_LOG_INFO("roaming preference set error %d",set_system_selection_preference_resp_msg.resp.error);
          qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
          qcril_send_request_response( &resp );
      }
      else
      {
          qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
          qcril_send_request_response( &resp );
      }
  }
  }while(0);
} /* qcril_qmi_nas_request_set_roaming_preference() */


/*===========================================================================

  FUNCTION:  qcril_qmi_nas_request_query_roaming_preference

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE.

    @return
    None.
*/
/*=========================================================================*/

void qcril_qmi_nas_request_query_roaming_preference
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_request_resp_params_type resp;
  nas_get_system_selection_preference_resp_msg_v01 get_system_selection_preference_resp_msg;
  int roaming_preference=0;

  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  memset(&get_system_selection_preference_resp_msg, 0, sizeof(get_system_selection_preference_resp_msg));
  /* Send QMI NAS QUERY ROAMING PREFERENCE TYPE REQ */
  if ( qcril_qmi_client_send_msg_sync ( QCRIL_QMI_CLIENT_NAS,
                                    QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                    NULL,
                                    0,
                                    &get_system_selection_preference_resp_msg,
                                    sizeof(get_system_selection_preference_resp_msg)) != E_SUCCESS )
  {
      /* In case of ARM9 reset, the command callback will never be executed. So, need to
       delete the entry from the ReqList, and call OnRequestComplete() */
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
  }
  else
  {
      if(get_system_selection_preference_resp_msg.resp.result == QMI_RESULT_FAILURE_V01)
      {
          QCRIL_LOG_INFO("roaming preference get error %d",get_system_selection_preference_resp_msg.resp.error);
          qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
          qcril_send_request_response( &resp );
      }
      else
      {
          if(get_system_selection_preference_resp_msg.roam_pref_valid == TRUE)
          {
              switch(get_system_selection_preference_resp_msg.roam_pref)
              {
                  case 0x0001:
                      roaming_preference = 0;
                      break;
                  case 0x0003:
                      roaming_preference = 1;
                      break;
                  case 0x00FF:
                      roaming_preference = 2;
                      break;
                  default:
                      roaming_preference = 0;
              }
              QCRIL_LOG_INFO("roaming preference retrieved %d",roaming_preference);
              if( roaming_preference != 0 )
              {
                  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
                  resp.resp_pkt = &roaming_preference;
                  resp.resp_len = sizeof(roaming_preference);
                  qcril_send_request_response( &resp );
              }
              else
              {
                  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
                  qcril_send_request_response( &resp );
              }
          }
          else
          {
              qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
              qcril_send_request_response( &resp );
          }
      }
  }
} /* qcril_qmi_nas_request_query_roaming_preference() */

//===========================================================================
// qcril_qmi_nas2_find_startic_operator_name
//===========================================================================
void qcril_qmi_nas2_find_startic_operator_name
(
  char * mcc_str,
  char * mnc_str,
  char * mcc_mnc_str_ref,
  char **long_ons_ptr,
  char **short_ons_ptr
)
{
  const int number_of_entries = sizeof( qcril_qmi_ons_memory_list ) / sizeof( qcril_qmi_ons_memory_entry_type );
  int i = 0;
  boolean continue_search = TRUE;
  const qcril_qmi_ons_memory_entry_type *ons_mem_ptr = NULL;
  char temp_mnc_str[NAS_MNC_MAX_LEN];

  if(long_ons_ptr != NULL && short_ons_ptr != NULL)
  {
  /* Search the table for the MCC and MNC */
  while ( continue_search && ( i < number_of_entries ) )
  {
    if ( !strcmp(mcc_str,qcril_qmi_ons_memory_list[ i ].mcc_str) )
    {
      if ( !strcmp(mnc_str,qcril_qmi_ons_memory_list[ i ].mnc_str) )
      {
        ons_mem_ptr = &qcril_qmi_ons_memory_list[ i ];
        continue_search = FALSE;
      }
      else if(NAS_SINGLE_DIGIT_MNC_LEN == strlen(qcril_qmi_ons_memory_list[ i ].mnc_str))
      {
          temp_mnc_str[NAS_SINGLE_DIGIT_MNC_LEN-1] = '0';
          temp_mnc_str[NAS_SINGLE_DIGIT_MNC_LEN] = qcril_qmi_ons_memory_list[ i ].mnc_str[NAS_SINGLE_DIGIT_MNC_LEN-1];
          temp_mnc_str[NAS_SINGLE_DIGIT_MNC_LEN+1] = '\0';
          if ( !strcmp(mnc_str,temp_mnc_str) )
          {
            ons_mem_ptr = &qcril_qmi_ons_memory_list[ i ];
            continue_search = FALSE;
          }
      }
      else if ( atoi(mnc_str) < atoi(qcril_qmi_ons_memory_list[ i ].mnc_str) )
      {
        /*
        ** Terminate the search because the MNCs are stored in ascending
        ** order in the table and the MNC being searched is less than the
        ** current MNC in the table.
        */
        continue_search = FALSE;
      }
    }

    else if ( atoi(mcc_str) < atoi(qcril_qmi_ons_memory_list[ i ].mcc_str) )
    {
      /*
      ** Terminate the search because the MCCs are stored in ascending
      ** order in the table and the MCC being searched is less than the
      ** current MCC in the table.
      */
      continue_search = FALSE;
    }

    i++;

  } /* end while */

  if ( ons_mem_ptr == NULL )
  {
    *long_ons_ptr = mcc_mnc_str_ref;

    /* Short ONS is not available */
    *short_ons_ptr = "";
    QCRIL_LOG_DEBUG( "ONS info from MCCMNC" );
  }
  else
  {
    *long_ons_ptr = ons_mem_ptr->full_name_ptr;
    *short_ons_ptr = ons_mem_ptr->short_name_ptr;
    QCRIL_LOG_DEBUG( "ONS info from UE Memory List" );
  }
  }
  else
  {
      QCRIL_LOG_FATAL("FATAL : CHECK FAILED");
  }


} // qcril_qmi_nas2_find_startic_operator_name

//===========================================================================
// qcril_qmi_nas2_find_elaboration_static_name
//===========================================================================
void qcril_qmi_nas2_find_elaboration_static_name( char * mcc_mnc_str, char** long_name_res, char ** short_name_res )
{
    const int                                       number_of_entries = sizeof( qcril_qmi_ons_elaboration_memory_list ) / sizeof( qcril_qmi_ons_elaboration_memory_entry_type );
    int                                             res = FALSE;
    qcril_qmi_ons_elaboration_memory_entry_type*    entry = (qcril_qmi_ons_elaboration_memory_entry_type*)qcril_qmi_ons_elaboration_memory_list;
    int                                             idx;

    for ( idx = 0; idx < number_of_entries && !res; idx++ )
    {
        if ( 0 == strcmp( mcc_mnc_str, entry->mcc_mnc_str ) )
        {
            res = TRUE;
            if ( NULL != long_name_res )
            {
                *long_name_res = entry->full_name_ptr;
            }
            if ( NULL != short_name_res )
            {
                *short_name_res = entry->short_name_ptr;
            }
        } // if ( 0 == strcmp( mcc_mnc_str, entry->mcc_mnc_str ) )
        entry++;
    } // for ( idx = 0; idx < number_of_entries && !res; idx++ )

    if ( !res )
    {
        if ( NULL != long_name_res )
        {
            *long_name_res = NULL;
        }
        if ( NULL != short_name_res )
        {
            *short_name_res = NULL;
        }
    } // if ( !res )

} // qcril_qmi_nas2_find_elaboration_static_name

// qcril_qmi_nas2_find_3gpp2_static_operator_name
//===========================================================================
int qcril_qmi_nas2_find_3gpp2_static_operator_name
(
  char * mcc_str,
  char * mnc_str,
  uint16_t sid,
  uint16_t nid,
  char **long_ons_ptr,
  char **short_ons_ptr
)
{
  const int number_of_entries = sizeof( qcril_qmi_ons_3gpp2_memory_list ) / sizeof( qcril_qmi_ons_3gpp2_memory_entry_type );
  int i = 0,res = FALSE;
  boolean continue_search = TRUE;
  const qcril_qmi_ons_3gpp2_memory_entry_type *ons_mem_ptr = NULL;

  if(long_ons_ptr != NULL && short_ons_ptr != NULL)
  {
  /* Search the table for the MCC and MNC */
  while ( continue_search && ( i < number_of_entries ) )
  {
    if ( !strcmp(mcc_str,qcril_qmi_ons_3gpp2_memory_list[ i ].mcc_str) )
    {
      if ( !strcmp(mnc_str,qcril_qmi_ons_3gpp2_memory_list[ i ].mnc_str) )
      {
          if (sid == (uint16_t)qcril_qmi_ons_3gpp2_memory_list[ i ].sid ||
                qcril_qmi_ons_3gpp2_memory_list[ i ].sid == SID_NID_WILD_NUMBER)
          {
              if (nid == (uint16_t)qcril_qmi_ons_3gpp2_memory_list[ i ].nid ||
                    qcril_qmi_ons_3gpp2_memory_list[ i ].nid == SID_NID_WILD_NUMBER)
              {
                  ons_mem_ptr = &qcril_qmi_ons_3gpp2_memory_list[ i ];
                  continue_search = FALSE;
                  res = TRUE;
              }
          }
      }
      else if ( atoi(mnc_str) < atoi(qcril_qmi_ons_3gpp2_memory_list[ i ].mnc_str) )
      {
        /*
        ** Terminate the search because the MNCs are stored in ascending
        ** order in the table and the MNC being searched is less than the
        ** current MNC in the table.
        */
        continue_search = FALSE;
      }
    }

    else if ( atoi(mcc_str) < atoi(qcril_qmi_ons_3gpp2_memory_list[ i ].mcc_str) )
    {
      /*
      ** Terminate the search because the MCCs are stored in ascending
      ** order in the table and the MCC being searched is less than the
      ** current MCC in the table.
      */
      continue_search = FALSE;
    }

    i++;

  } /* end while */

  if ( ons_mem_ptr == NULL )
  {
    QCRIL_LOG_DEBUG( "UE Memory List does not contain the specified operator" );
  }
  else
  {
    *long_ons_ptr = ons_mem_ptr->full_name_ptr;
    *short_ons_ptr = ons_mem_ptr->short_name_ptr;
    QCRIL_LOG_DEBUG( "ONS info from UE Memory List" );
  }
  }
  else
  {
      QCRIL_LOG_FATAL("FATAL : CHECK FAILED");
  }

  return res;
} // qcril_qmi_nas2_find_3gpp2_static_operator_name

//===========================================================================
// qcril_qmi_nas2_retrieve_mcc_from_iccid
//===========================================================================
char* qcril_qmi_nas2_retrieve_mcc_from_iccid(char *iccid)
{
    int res;
    char *mcc_ptr;
    char *iccid_iin_ptr;
    const qcril_qmi_ccc_mcc_map_type *iter_ptr;

    res = 0;
    mcc_ptr= NULL;
    iccid_iin_ptr = NULL;
    iter_ptr = qcril_qmi_ccc_mcc_map;

    if(iccid && ((NAS_ICCID_POSSIBLE_LENGTH_1 == strlen(iccid)) || (NAS_ICCID_POSSIBLE_LENGTH_2 == strlen(iccid)) || (NAS_ICCID_POSSIBLE_LENGTH_3 == strlen(iccid))))
    {
        iccid_iin_ptr = &iccid[NAS_ICCID_IIN_START_POS];
        if(NAS_ICCID_IIN_SKIP_LITERAL == *iccid_iin_ptr)
        {
            iccid_iin_ptr++;
        }

        while(iter_ptr && iter_ptr->ccc_str)
        {
            res = memcmp(iter_ptr->ccc_str, iccid_iin_ptr, strlen(iter_ptr->ccc_str));
            if(0 == res)
            {
                mcc_ptr = iter_ptr->mcc_str;
                if(strcmp(iter_ptr->ccc_str,NAS_ICCID_PREFIX_CODE_ANOMALY_1) && strcmp(iter_ptr->ccc_str,NAS_ICCID_PREFIX_CODE_ANOMALY_2))
                {
                    break;
                }
            }
            else if(0 < res)
            {
                break;
            }
            iter_ptr++;
        }
    }

    return mcc_ptr;
} //qcril_qmi_nas2_retrieve_mcc_from_iccid

//===========================================================================
// qcril_qmi_nas2_convert_rat_to_mode_pref
//===========================================================================
mode_pref_mask_type_v01 qcril_qmi_nas2_convert_rat_to_mode_pref(int rat)
{
    mode_pref_mask_type_v01 mode_pref;

    QCRIL_LOG_FUNC_ENTRY();

    switch(rat)
    {
        case RADIO_TECH_GPRS:
        case RADIO_TECH_GSM:
        case RADIO_TECH_EDGE:
            mode_pref = QMI_NAS_RAT_MODE_PREF_GSM;
            break;

        case RADIO_TECH_UMTS:
        case RADIO_TECH_HSDPA:
        case RADIO_TECH_HSUPA:
        case RADIO_TECH_HSPAP:
        case RADIO_TECH_HSPA:
            mode_pref = QMI_NAS_RAT_MODE_PREF_UMTS;
            break;

        case RADIO_TECH_TD_SCDMA:
            mode_pref = QMI_NAS_RAT_MODE_PREF_TDSCDMA;
            break;

        case RADIO_TECH_LTE:
            mode_pref = QMI_NAS_RAT_MODE_PREF_LTE;
            break;

        default:
            mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_LTE;
            break;
    }


    QCRIL_LOG_FUNC_RETURN_WITH_RET(mode_pref);
    return mode_pref;
} //qcril_qmi_nas2_convert_rat_to_mode_pref

//===========================================================================
// qcril_qmi_nas2_convert_qcril_rat_to_qmi_rat
//===========================================================================
nas_radio_if_enum_v01
    qcril_qmi_nas2_convert_qcril_rat_to_qmi_rat(RIL_RadioTechnology qcril_rat)
{
    nas_radio_if_enum_v01 qmi_rat;

    QCRIL_LOG_FUNC_ENTRY();

    switch(qcril_rat)
    {
        case RADIO_TECH_GSM:
        case RADIO_TECH_GPRS:
        case RADIO_TECH_EDGE:
            qmi_rat = NAS_RADIO_IF_GSM_V01;
            break;

        case RADIO_TECH_UMTS:
        case RADIO_TECH_HSDPA:
        case RADIO_TECH_HSUPA:
        case RADIO_TECH_HSPAP:
        case RADIO_TECH_HSPA:
            qmi_rat = NAS_RADIO_IF_UMTS_V01;
            break;

        case RADIO_TECH_TD_SCDMA:
            qmi_rat = NAS_RADIO_IF_TDSCDMA_V01;
            break;

        case RADIO_TECH_LTE:
            qmi_rat = NAS_RADIO_IF_LTE_V01;
            break;

        case RADIO_TECH_1xRTT:
        case RADIO_TECH_IS95A:
        case RADIO_TECH_IS95B:
            qmi_rat = NAS_RADIO_IF_CDMA_1X_V01;
            break;


       case RADIO_TECH_EHRPD:
       case RADIO_TECH_EVDO_0:
       case RADIO_TECH_EVDO_A:
       case RADIO_TECH_EVDO_B:
            qmi_rat = NAS_RADIO_IF_CDMA_1XEVDO_V01;
            break;

        case RADIO_TECH_IWLAN:
            qmi_rat = NAS_RADIO_IF_WLAN_V01;
            break;

        default:
            qmi_rat = NAS_RADIO_IF_NO_CHANGE_V01;
            break;
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET(qmi_rat);
    return qmi_rat;
} //qcril_qmi_nas2_convert_qcril_rat_to_qmi_rat


//===========================================================================
//QCRIL_EVT_HOOK_SET_TRANSMIT_POWER
//===========================================================================
void qcril_qmi_nas2_set_max_transmit_power
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
  qcril_request_resp_params_type    resp;

  qmi_client_error_type             qmi_client_error;
  RIL_Errno                         ril_req_res = RIL_E_GENERIC_FAILURE;

  sar_rf_set_state_req_msg_v01      qmi_request;
  sar_rf_set_state_resp_msg_v01     qmi_response;

  struct sar_rf_state *                ril_param;

  qmi_client_type                   rf_sar_client;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_NOTUSED( ret_ptr );

  rf_sar_client = qcril_qmi_client_get_user_handle ( QCRIL_QMI_CLIENT_RF_SAR );
  QCRIL_LOG_DEBUG( ".. rf sar client obj %"PRIdPTR, (intptr_t)rf_sar_client );

  if ( NULL != params_ptr->data && params_ptr->datalen > QMI_RIL_ZERO && NULL != rf_sar_client )
  {
      ril_param = (struct sar_rf_state *)params_ptr->data;

      memset( &qmi_request, 0, sizeof( qmi_request ) );
      memset( &qmi_response, 0, sizeof( qmi_response ) );
      switch ( ril_param->rf_state_index )
      {
          case 1:
              qmi_request.sar_rf_state = QMI_SAR_RF_STATE_1_V01;
              break;

          case 2:
              qmi_request.sar_rf_state = QMI_SAR_RF_STATE_2_V01;
              break;

          case 3:
              qmi_request.sar_rf_state = QMI_SAR_RF_STATE_3_V01;
              break;

          case 4:
              qmi_request.sar_rf_state = QMI_SAR_RF_STATE_4_V01;
              break;

          case 5:
              qmi_request.sar_rf_state = QMI_SAR_RF_STATE_5_V01;
              break;

          case 6:
              qmi_request.sar_rf_state = QMI_SAR_RF_STATE_6_V01;
              break;

          case 7:
              qmi_request.sar_rf_state = QMI_SAR_RF_STATE_7_V01;
              break;

          case 8:
              qmi_request.sar_rf_state = QMI_SAR_RF_STATE_8_V01;
              break;

          case 0:     // fallthrough
          default:
              qmi_request.sar_rf_state = QMI_SAR_RF_STATE_DEFAULT_V01;
              break;
      }
      QCRIL_LOG_INFO(".. params rf state index %d / sar rf state %d", (int)ril_param->rf_state_index, (int)qmi_request.sar_rf_state );

      qmi_request.compatibility_key_valid = TRUE;
      qmi_request.compatibility_key = ril_param->compatibility_key;
      QCRIL_LOG_INFO(".. params compatibility_key %ld / qmi request compatibility_key %ld",
                      (uint32_t)ril_param->compatibility_key, (uint32_t)qmi_request.compatibility_key );

      qmi_client_error = qmi_client_send_msg_sync_with_shm( rf_sar_client,
                                                     QMI_SAR_RF_SET_STATE_REQ_MSG_V01,
                                                     (void*) &qmi_request ,
                                                     sizeof( qmi_request ),
                                                     (void*) &qmi_response,
                                                     sizeof( qmi_response ),
                                                     QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT );


      ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_client_error, &qmi_response.resp );
      QCRIL_LOG_QMI_RESP_STATUS(qmi_client_error, &(qmi_response.resp), qmi_client_error);
  }
  // ** respond always
  qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_req_res, &resp );
  qcril_send_request_response( &resp );

  QCRIL_LOG_FUNC_RETURN_WITH_RET((int) ril_req_res);
} // qcril_qmi_nas2_set_max_transmit_power

//===========================================================================
//QCRIL_EVT_HOOK_GET_SAR_REV_KEY
//===========================================================================
void qcril_qmi_nas2_get_sar_rev_key
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
    qmi_client_error_type qmi_client_error;
    qcril_instance_id_e_type instance_id;
    qcril_modem_id_e_type modem_id;
    qcril_request_resp_params_type resp;
    uint32_t key = 0;
    sar_rf_get_compatibility_key_resp_msg_v01 qmi_response;

    QCRIL_LOG_DEBUG("qcril_qmi_nas2_get_sar_rev_key entry");

    QCRIL_ASSERT( params_ptr != NULL );
    QCRIL_NOTUSED(ret_ptr);

    if(NULL != params_ptr)
    {
      instance_id = params_ptr->instance_id;
      QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
      modem_id = params_ptr->modem_id;
      QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

      memset(&qmi_response, 0, sizeof(qmi_response));
      qmi_client_error = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle ( QCRIL_QMI_CLIENT_RF_SAR),
                                                    QMI_SAR_GET_COMPATIBILITY_KEY_REQ_MSG_V01,
                                                    NULL,
                                                    QMI_RIL_ZERO,
                                                    (void*) &qmi_response,
                                                    sizeof( qmi_response ),
                                                    QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT );

      QCRIL_LOG_DEBUG("qmi_client_error=%d",qmi_client_error);
      if (qmi_client_error != QMI_NO_ERR)
      {
        qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
        qcril_send_request_response( &resp );
      }
      else
      {
        qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );

        if( TRUE == qmi_response.compatibility_key_valid )
        {
          key = qmi_response.compatibility_key;
        }
        resp.resp_pkt = (void *) &key;
        resp.resp_len = sizeof( key );
        qcril_send_request_response( &resp );
      }
    }
    else
    {
      QCRIL_LOG_ERROR("params_ptr is null");
    }
} // qcril_qmi_nas2_get_sar_rev_key

// QTuner service support (RFPE)


//===========================================================================
// qcril_qmi_nas_get_rfm_scenario_req
//===========================================================================
void qcril_qmi_nas_get_rfm_scenario_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_request_resp_params_type resp;

  RIL_Errno   ril_req_res = RIL_E_GENERIC_FAILURE;

  Qtuner_get_rfm_scenarios_resp_v01 ril_response;
  int                               ril_response_len = 0;
  rfrpe_get_rfm_scenarios_resp_v01 qmi_response;
  qmi_client_error_type qmi_client_error;
  qmi_client_type rfpe_client_handle;

  int       scenario_num = 0;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED( ret_ptr );

  memset(&ril_response, 0, sizeof(ril_response));
  memset(&qmi_response,0,sizeof(qmi_response));

  rfpe_client_handle = qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_RFPE );
  QCRIL_LOG_INFO("rfpe_client_handle %"PRIdPTR, (intptr_t) rfpe_client_handle);

  if ( NULL != rfpe_client_handle )
  {
      qmi_client_error = qmi_client_send_msg_sync_with_shm( rfpe_client_handle,
                                QMI_RFRPE_GET_RFM_SCENARIO_REQ_V01,
                                NULL,
                                QMI_RIL_ZERO,
                                (void*) &qmi_response,
                                sizeof( qmi_response ),
                                QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT);

      ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_client_error, &qmi_response.resp );
      QCRIL_LOG_INFO( "code=%d, RFPE scenario valid=%d, RFPE scenario len=%d",ril_req_res,qmi_response.active_scenarios_valid, qmi_response.active_scenarios_len);

      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, ril_req_res, &resp );

      if( RIL_E_SUCCESS == ril_req_res )
      {
          if ( qmi_response.active_scenarios_valid )
          {
             for ( scenario_num = 0; scenario_num < (int) qmi_response.active_scenarios_len; scenario_num++ )
             {
               QCRIL_LOG_INFO( "RFPE scenario %d is %d",scenario_num, qmi_response.active_scenarios[scenario_num]);
             }

             ril_response.active_scenarios_valid = qmi_response.active_scenarios_valid;
             ril_response.active_scenarios_len= qmi_response.active_scenarios_len;
             memcpy( ril_response.active_scenarios, qmi_response.active_scenarios, sizeof(qmi_response.active_scenarios[0]) * qmi_response.active_scenarios_len );

             ril_response_len = sizeof(qmi_response.active_scenarios_valid) + sizeof(qmi_response.active_scenarios_len) + sizeof(qmi_response.active_scenarios[0]) * qmi_response.active_scenarios_len;
          }

          resp.resp_pkt = (void *) &ril_response;
          resp.resp_len =  ril_response_len;
      }
  }
  else
  {
      // Nothing to send since no client available
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, ril_req_res, &resp );
      resp.resp_pkt = (void *) NULL;
      resp.resp_len =  0;
  }
  qcril_send_request_response( &resp );
  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// qcril_qmi_nas_get_provisioned_table_revision_req
//===========================================================================
void qcril_qmi_nas_get_provisioned_table_revision_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_request_resp_params_type resp;

  RIL_Errno   ril_req_res = RIL_E_GENERIC_FAILURE;

  Qtuner_get_provisioned_table_revision_resp_v01 ril_response;
  int                                            ril_response_len = 0;
  rfrpe_get_provisioned_table_revision_resp_v01 qmi_response;
  qmi_client_error_type qmi_client_error;
  qmi_client_type rfpe_client_handle;

  int       provisioned_table_OEM_idx = 0;


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED( ret_ptr );

  memset(&ril_response, 0, sizeof(ril_response));
  memset(&qmi_response,0,sizeof(qmi_response));

  rfpe_client_handle = qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_RFPE );
  QCRIL_LOG_INFO("rfpe_client_handle %"PRIdPTR, (intptr_t) rfpe_client_handle);

  if ( NULL != rfpe_client_handle )
  {
      qmi_client_error = qmi_client_send_msg_sync_with_shm( rfpe_client_handle,
                                QMI_RFRPE_GET_PROVISIONED_TABLE_REVISION_REQ_V01,
                                NULL,
                                QMI_RIL_ZERO,
                                (void*) &qmi_response,
                                sizeof( qmi_response ),
                                QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT);
      ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_client_error, &qmi_response.resp );
      QCRIL_LOG_INFO( "code=%d, provision table valid=%d, provision table revision = %d",ril_req_res, qmi_response.provisioned_table_revision_valid, qmi_response.provisioned_table_revision );
      QCRIL_LOG_INFO( "provisioned_table_OEM_valid=%d, provisioned_table_OEM_len=%d",qmi_response.provisioned_table_OEM_valid, qmi_response.provisioned_table_OEM_len )

      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, ril_req_res, &resp );

      if( RIL_E_SUCCESS == ril_req_res )
      {
          ril_response.provisioned_table_revision_valid = qmi_response.provisioned_table_revision_valid;
          ril_response_len += sizeof(ril_response.provisioned_table_revision_valid);

          if ( ril_response.provisioned_table_revision_valid )
          {
              ril_response.provisioned_table_revision       = qmi_response.provisioned_table_revision;
              ril_response_len += sizeof(ril_response.provisioned_table_revision);
          }

          ril_response.provisioned_table_OEM_valid = qmi_response.provisioned_table_OEM_valid;
          ril_response_len +=  sizeof(ril_response.provisioned_table_OEM_valid) ;

          if ( qmi_response.provisioned_table_OEM_valid )
          {
             ril_response.provisioned_table_OEM_len   = qmi_response.provisioned_table_OEM_len;

             for ( provisioned_table_OEM_idx = 0; provisioned_table_OEM_idx < (int) qmi_response.provisioned_table_OEM_len; provisioned_table_OEM_idx++ )
             {
               QCRIL_LOG_INFO( "Provisioned table (OEM) %d is %d", provisioned_table_OEM_idx, qmi_response.provisioned_table_OEM[provisioned_table_OEM_idx]);
             }

             memcpy(ril_response.provisioned_table_OEM, qmi_response.provisioned_table_OEM, sizeof(qmi_response.provisioned_table_OEM[0]) * qmi_response.provisioned_table_OEM_len);

             //update response len
             ril_response_len += sizeof(ril_response.provisioned_table_OEM_len)  + sizeof(ril_response.provisioned_table_OEM[0]) * ril_response.provisioned_table_OEM_len;
          }

          resp.resp_pkt = (void *) &ril_response;
          resp.resp_len =  ril_response_len;
      }
  }
  else
  {
      // Nothing to send since no client available
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, ril_req_res, &resp );
      resp.resp_pkt = (void *) NULL;
      resp.resp_len =  0;
  }

  qcril_send_request_response( &resp );
  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// qcril_qmi_nas_set_rfm_scenario_req
//===========================================================================
void qcril_qmi_nas_set_rfm_scenario_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_request_resp_params_type resp;

  RIL_Errno   ril_req_res = RIL_E_GENERIC_FAILURE;

  Qtuner_set_scenario_req_v01 ril_request;
  rfrpe_set_scenario_req_v01  qmi_request;
  rfrpe_set_scenario_resp_v01 qmi_response;
  qmi_client_error_type qmi_client_error;
  qmi_client_type rfpe_client_handle;

  int scenario_idx = 0;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED( ret_ptr );

  memset(&qmi_request,0,sizeof(qmi_request));
  memset(&ril_request,0,sizeof(ril_request));
  memset(&qmi_response,0,sizeof(qmi_response));

  if (params_ptr->datalen <= sizeof(Qtuner_set_scenario_req_v01))
  {
    memcpy(&ril_request, params_ptr->data, params_ptr->datalen);
    QCRIL_LOG_INFO( "Qtuner set rfpe scenarios params_ptr->datalen %d",params_ptr->datalen );
    QCRIL_LOG_INFO( "Qtuner set rfpe scenarios len %d",ril_request.scenarios_len );

    qmi_request.scenarios_len = ril_request.scenarios_len ;

    if (ril_request.scenarios_len <= Qtuner_CONCURRENT_SCENARIOS_MAX_V01)
    {
      for( scenario_idx = 0; scenario_idx < (int) ril_request.scenarios_len; scenario_idx++ )
      {
        QCRIL_LOG_INFO( "Qtuner set rfpe scenario %d = %d", scenario_idx, ril_request.scenarios[scenario_idx] );
        qmi_request.scenarios[scenario_idx] = ril_request.scenarios[scenario_idx];
      }

      rfpe_client_handle = qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_RFPE );
      QCRIL_LOG_INFO("rfpe_client_handle %"PRIdPTR, (intptr_t) rfpe_client_handle);

      if ( NULL != rfpe_client_handle )
      {
          qmi_client_error = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_RFPE ),
                                    QMI_RFRPE_SET_RFM_SCENARIO_REQ_V01,
                                    &qmi_request,
                                    sizeof( qmi_request ),
                                    (void*) &qmi_response,
                                    sizeof( qmi_response ),
                                    QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT);
          ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_client_error, &qmi_response.resp );
          QCRIL_LOG_INFO( "Qtuner set rfpe qmi code=%d", ril_req_res );
      }
    }
  }

  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, ril_req_res, &resp );
  resp.resp_pkt = NULL;
  resp.resp_len =  0;
  qcril_send_request_response( &resp );

  QCRIL_LOG_FUNC_RETURN();
}
// ** Qtuner

//===========================================================================
// qcril_qmi_nas_get_radio_tech
//===========================================================================
unsigned int qcril_qmi_nas_get_radio_tech(uint16_t mode_pref)
{
    unsigned int radio_tech_family = RADIO_TECH_UNKNOWN;

    switch( mode_pref )
    {
        case QMI_NAS_RAT_MODE_PREF_GSM_UMTS:
        case QMI_NAS_RAT_MODE_PREF_GSM:
        case QMI_NAS_RAT_MODE_PREF_UMTS:
        case QMI_NAS_RAT_MODE_PREF_GSM_UMTS_LTE:
        case QMI_NAS_RAT_MODE_PREF_LTE:
        case QMI_NAS_RAT_MODE_PREF_UMTS_LTE:
        case QMI_NAS_RAT_MODE_PREF_TDSCDMA:
        case QMI_NAS_RAT_MODE_PREF_UMTS_TDSCDMA:
        case QMI_NAS_RAT_MODE_PREF_TDSCDMA_LTE:
        case QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA:
        case QMI_NAS_RAT_MODE_PREF_UMTS_TDSCDMA_LTE:
        case QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_LTE:
        case QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA:
        case QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_LTE:
            radio_tech_family = RADIO_TECH_3GPP;
            break;

        case QMI_NAS_RAT_MODE_PREF_CDMA_HRPD:
        case QMI_NAS_RAT_MODE_PREF_CDMA:
        case QMI_NAS_RAT_MODE_PREF_HRPD:
            radio_tech_family = RADIO_TECH_3GPP2;
            break;
    }

    return radio_tech_family;
}

/*===========================================================================
  qcril_qmi_nas2_create_reqlist_setup_timer_helper
============================================================================*/
RIL_Errno qcril_qmi_nas2_create_reqlist_setup_timer_helper( const qcril_request_params_type *const params_ptr )
{
    RIL_Errno ril_req_res = RIL_E_SUCCESS;
    qcril_reqlist_public_type      qcril_req_info_ptr;
    qcril_instance_id_e_type       instance_id = QCRIL_DEFAULT_INSTANCE_ID;

    if ( NULL != params_ptr )
    {
        memset( &qcril_req_info_ptr, 0, sizeof(qcril_req_info_ptr) );
        qcril_reqlist_default_entry( params_ptr->t,
                                params_ptr->event_id,
                                QCRIL_DEFAULT_MODEM_ID,
                                QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                                QCRIL_EVT_QMI_REQUEST_SET_SYS_SEL_PREF,
                                NULL,
                                &qcril_req_info_ptr );
        QCRIL_LOG_INFO( "created entry for QCRIL_EVT_QMI_REQUEST_SET_SYS_SEL_PREF " );

        if ( qcril_reqlist_new( instance_id, &qcril_req_info_ptr ) == E_SUCCESS )
        {
            QCRIL_LOG_INFO( "Added entry for QCRIL_EVT_QMI_REQUEST_SET_SYS_SEL_PREF " );
            if( qcril_qmi_nas_sys_sel_pref_setup_timed_callback() != E_SUCCESS )
            {
                ril_req_res = RIL_E_GENERIC_FAILURE;
            }
        }
        else
        {
            ril_req_res = RIL_E_GENERIC_FAILURE;
        }
    }
    else
    {
        ril_req_res = RIL_E_GENERIC_FAILURE;
    }

    return ril_req_res;
}
