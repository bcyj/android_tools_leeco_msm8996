/*!
  @file
  qcril_cm_ons.c

  @brief
  Encapsulates the information related to operator name display.

*/

/*===========================================================================

  Copyright (c) 2009 - 2010 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_cm_ons.c#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/01/10   fc      Re-architecture to support split modem.
05/14/09   pg      Mainlined FEATURE_MULTIMODE_ANDROID.
04/05/09   fc      Cleanup log macros and mutex macros.
02/26/09   fc      First cut.


===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <cutils/memory.h>
#include <cutils/properties.h>
#include <pthread.h>
#include <string.h>
#include "IxErrno.h"
#include "comdef.h"
#include "cm.h"
#include "nv.h"
#include "qcrili.h"
#include "qcril_cm.h"
#include "qcril_cmi.h"
#include "qcril_cm_ons.h"
#include "qcril_cm_onsi.h"
#include "qcril_log.h"
#include "qcril_arb.h"
#include "qcril_other_api_map.h"


/*===========================================================================

                   EXTERNAL DEFINITIONS AND TYPES

===========================================================================*/



/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/



/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

/* Variables internal to module qcril_cm_ons.c  */
static qcril_cm_ons_struct_type qcril_cm_ons[ QCRIL_MAX_INSTANCE_ID ];

const char *nitz_long_ons_property_list[] = { QCRIL_CM_ONS_NITZ_LONS_0, QCRIL_CM_ONS_NITZ_LONS_1, 
                                              QCRIL_CM_ONS_NITZ_LONS_2, QCRIL_CM_ONS_NITZ_LONS_3 };
const char *nitz_short_ons_property_list[] = { QCRIL_CM_ONS_NITZ_SONS_0, QCRIL_CM_ONS_NITZ_SONS_1, 
                                               QCRIL_CM_ONS_NITZ_SONS_2, QCRIL_CM_ONS_NITZ_SONS_3 }; 


/*===================================================================================
       G S M / W C D M A    N E T W O R K    N A M E S    M E M O R Y    L I S T 
=====================================================================================*/
/*
** Define a static table of network names.  This table does not include
** all of the networks and is to be used as a reference only.  The mcc
** and mnc MUST be in ASCENDING order.
*/
static const qcril_cm_ons_memory_entry_type qcril_cm_ons_memory_list[] =
{
  /***********************
   **** Test PLMN 1-1 ****
   ***********************/
  { 001,   "01", "Test1-1", "Test PLMN 1-1" },

  /***********************
   **** Test PLMN 1-2 ****
   ***********************/
  { 001,   "02", "Test1-2", "Test PLMN 1-2" },

  /***********************
   **** Test PLMN 2-1 ****
   ***********************/
  { 002,   "01", "Test2-1", "Test PLMN 2-1" },

  /****************
   **** Greece ****
   ****************/
  { 202,  "01", "Cosmote",  "COSMOTE - Mobile Telecommunications S.A." },
  { 202,  "05", "Vodafone", "Vodafone Greece" },
  { 202,  "09", "Wind",     "Wind Hella telecommunications S.A." },
  { 202,  "10", "Wind",     "Wind Hella telecommunications S.A." },

  /*********************
   **** Netherlands ****
   *********************/
  { 204,  "03", "Rabo Mobiel",             "KPN" },
  { 204,  "04", "Vodafone",                "Vodafone Netherlands" },
  { 204,  "08", "KPN",                     "KPN" },
  { 204,  "12", "Telfort",                 "KPN" },
  { 204,  "16", "T-Mobile / Ben",          "T-Mobile Netherlands B.V" },
  { 204,  "20", "Orange Nederland",        "T-Mobile Netherlands B.V" },
  { 204,  "21", "NS Railinfrabeheer B.V.", "NS Railinfrabeheer B.V." },

  /*****************
   **** Belgium ****
   *****************/
  { 206,  "01", "Proximus", "Belgacom Mobile" },
  { 206,  "10", "Mobistar", "France Telecom" },
  { 206,  "20", "BASE",     "KPN" },

  /****************
   **** France ****
   ****************/
  { 208,  "00", "Orange",                "Orange" },
  { 208,  "01", "France Telecom Mobile", "France Orange" },
  { 208,  "02", "Orange",                "Orange" },
  { 208,  "05", "Globalstar Europe",     "Globalstar Europe" },
  { 208,  "06", "Globalstar Europe",     "Globalstar Europe" },
  { 208,  "07", "Globalstar Europe",     "Globalstar Europe" },
  { 208,  "10", "SFR",                   "SFR" },
  { 208,  "11", "SFR",                   "SFR" },
  { 208,  "20", "Bouygues",              "Bouygues Telecom" },
  { 208,  "21", "Bouygues",              "Bouygues Telecom" },

  /*****************
   **** Andorra ****
   *****************/
  { 213,  "03", "Mobiland", "Servei De Tele. DAndorra" },

  /***************
   **** Spain ****
   ***************/
  { 214,  "01", "Vodafone",  "Vodafone Spain" },
  { 214,  "03", "Orange",    "France Telcom Espana SA" },
  { 214,  "04", "Yoigo",     "Xfera Moviles SA" },
  { 214,  "05", "TME",       "Telefonica Moviles Espana" },
  { 214,  "06", "Vodafone",  "Vodafone Spain" },
  { 214,  "07", "movistar",  "Telefonica Moviles Espana" },
  { 214,  "09", "Orange",    "France Telcom Espana SA" },

  /*****************
   **** Hungary ****
   *****************/
  { 216,  "20", "Pannon",   "Pannon GSM Tavkozlesi Zrt." },
  { 216,  "30", "T-Mobile", "Magyar Telkom Plc" },
  { 216,  "70", "Vodafone", "Vodafonei Magyarorszag Zrt." },

  /********************************
   **** Bosnia and Herzegovina ****
   ********************************/
  { 218,  "03", "ERONET",    "Public Enterprise Croatian telecom Ltd." },
  { 218,  "05", "m:tel",     "RS Telecommunications JSC Banja Luka" },
  { 218,  "90", "BH Mobile", "BH Telecom" },

  /*****************
   **** Croatia ****
   *****************/
  { 219,  "01", "T-Mobile", "T-Mobile Croatia" },
  { 219,  "02", "Tele2",    "Tele2" },
  { 219,  "10", "VIPnet",   "Vipnet" },

  /****************
   **** Serbia ****
   ****************/
  { 220,  "01", "Telenor",         "Telenor Serbia" },
  { 220,  "03", "Telekom Sribija", "Telekom Srbija" },
  { 220,  "05", "VIP Mobile",      "VIP Mobile" },

  /***************
   **** Italy ****
   ***************/
  { 222,  "01", "TIM",      "Telecom Italiz SpA" },
  { 222,  "02", "Elsacom",  "Elsacom" },
  { 222,  "10", "Vodafone", "Vodafone Omnitel N.V." },
  { 222,  "30", "RRI",      "Rete  Ferroviaria Italiana" },
  { 222,  "88", "Wind",     "Wind Telecomunicazioni SpA" },
  { 222,  "99", "3 Italia", "Hutchison 3G" },

  /*****************
   **** Romania ****
   *****************/
  { 226,  "01", "Vodafone",   "Vodafone Romania" },
  { 226,  "03", "Cosmote",    "Cosmote Romania" },
  { 226,  "05", "DIGI.mobil", "RCS&RDS" },
  { 226,  "10", "Orange",     "Orange Romania" },

  /*********************
   **** Switzerland ****
   *********************/
  { 228,  "01", "Swisscom",     "Swisscom Ltd" },
  { 228,  "02", "Sunrise",      "Sunrise Communications AG" },
  { 228,  "03", "Orange",       "Orange Communications SA" },
  { 228,  "06", "SBB AG",       "SBB AG" },
  { 228,  "07", "IN&Phone",     "IN&Phone SA" },
  { 228,  "08", "Tele2",        "Tele2 Telecommunications AG" },

  /************************
   **** Czech Republic ****
   ************************/
  { 230,  "01", "T-Mobile",      "T-Mobile Czech Republic" },
  { 230,  "02", "EUROTEL PRAHA", "Telefonica O2 Czech Republic" },
  { 230,  "03", "OSKAR",         "Vodafone Czech Republic" },
  { 230,  "98", "CZDC s.o.",     "CZDC s.o." },

  /******************
   **** Slovakia ****
   ******************/
  { 231,  "01", "Orange",   "Orange Slovensko" },
  { 231,  "02", "T-Mobile", "T-Mobile Slovensko" },
  { 231,  "04", "T-Mobile", "T-Mobile Slovensko" },
  { 231,  "06", "O2",       "Telefonica O2 Slovakia" },

  /*****************
   **** Austria ****
   *****************/
  { 232,  "01", "A1",       "Mobilkom Austria" },
  { 232,  "03", "T-Mobile", "T-Mobile Austria" },
  { 232,  "05", "Orange",   "Orange Austria" },
  { 232,  "07", "T-Mobile", "T-Mobile Austria" },
  { 232,  "10", "3",        "Hutchison 3G" },

  /************************
   **** United Kingdom ****
   ************************/
  { 234,  "00", "BT",                                               "British Telecom" },
  { 234,  "01", "UK01",                                             "Mapesbury Communications Ltd." },
  { 234,  "02", "O2",                                               "O2" },
  { 234,  "03", "Jersey Telenet",                                   "Jersey Telnet" },
  { 234,  "10", "O2",                                               "Telefonica O2 UK Limited" },
  { 234,  "11", "O2",                                               "Telefonica Europe" },
  { 234,  "12", "Railtrack",                                        "Network Rail Infrastructure Ltd" },
  { 234,  "15", "Vodafone",                                         "Vodafone United Kingdom" },
  { 234,  "16", "Opal Telecom Ltd",                                 "Opal Telecom Ltd" },
  { 234,  "18", "Cloud9",                                           "Wire9 Telecom plc" },
  { 234,  "19", "Telaware",                                         "Telaware plc" },
  { 234,  "20", "3",                                                "Hutchison 3G UK Ltd" },
  { 234,  "30", "T-Mobile",                                         "T-Mobile" },
  { 234,  "31", "Virgin",                                           "Virgin Mobile" },
  { 234,  "32", "Virgin",                                           "Virgin Mobile" },
  { 234,  "33", "Orange",                                           "Orange PCS Ltd" },
  { 234,  "34", "Orange",                                           "Orange PCS Ltd" },
  { 234,  "50", "JT-Wave",                                          "Jersey Telecoms" },
  { 234,  "55", "Cable & Wireless Guernsey / Sure Mobile (Jersey)", "Cable & Wireless Guernsey / Sure Mobile (Jersey)" },
  { 234,  "58", "Manx Telecom",                                     "Manx Telecom" },
  { 234,  "75", "Inquam",                                           "Inquam Telecom (Holdings) Ltd" },

  /*****************
   **** Denmark ****
   *****************/
  { 238,  "01", "TDC",                 "TDC A/S" },
  { 238,  "02", "Sonofon",             "Telenor" },
  { 238,  "06", "3",                   "Hi3G Denmark ApS" },
  { 238,  "30", "Telia",               "Telia Nattjanster Norden AB" },
  { 238,  "70", "Tele2",               "Telenor" },

  /****************
   **** Sweden ****
   ****************/
  { 240,  "01", "Telia",                              "TeliaSonera Mobile Networks" },
  { 240,  "02", "3",                                  "3" },
  { 240,  "04", "3G Infrastructure Services",         "3G Infrastructure Services" },
  { 240,  "05", "Sweden 3G",                          "Sweden 3G" },
  { 240,  "06", "Telenor",                            "Telenor" },
  { 240,  "07", "Tele2",                              "Tele2 AB" },
  { 240,  "08", "Telenor",                            "Telenor" },
  { 240,  "21", "Banverket",                          "Banverket" },

  /****************
   **** Norway ****
   ****************/
  { 242,  "01", "Telenor",           "Telenor" },
  { 242,  "02", "NetCom",            "NetCom GSM" },
  { 242,  "05", "Network Norway",    "Network Norway" },
  { 242,  "20", "Jernbaneverket AS", "Jernbaneverket AS" },

  /*****************
   **** Finland ****
   *****************/
  { 244,  "03", "DNA",        "DNA Oy" },
  { 244,  "05", "Elisa",      "Elisa Oyj" },
  { 244,  "12", "DNA Oy",     "DNA Oy" },
  { 244,  "14", "AMT",        "Alands Mobiltelefon" },
  { 244,  "91", "Sonera",     "TeliaSonera Finland Oyj" },

  /*******************
   **** Lithuania ****
   *******************/
  { 246,  "01", "Omnitel", "Omnitel" },
  { 246,  "02", "BITE",    "UAB Bite Lietuva" },
  { 246,  "03", "Tele 2",  "Tele 2" },

  /****************
   **** Latvia ****
   ****************/
  { 247,  "01", "LMT",   "Latvian Mobile Telephone" },
  { 247,  "02", "Tele2", "Tele2" },
  { 247,  "05", "Bite",  "Bite Latvija" },

  /*****************
   **** Estonia ****
   *****************/
  { 248,  "01", "EMT",    "Estonian Mobile Telecom" },
  { 248,  "02", "Elisa",  "Elisa Eesti" },
  { 248,  "03", "Tele 2", "Tele 2 Eesti" },

  /***************************
   **** Russia Federation ****
   ***************************/
  { 250,  "01", "MTS",                   "Mobile Telesystems" },
  { 250,  "02", "MegaFon",               "MegaFon OJSC" },
  { 250,  "03", "NCC",                   "Nizhegorodskaya Cellular Communications" },
  { 250,  "05", "ETK",                   "Yeniseytelecom" },
  { 250,  "07", "SMARTS",                "Zao SMARTS" },
  { 250,  "12", "Baykalwstern",          "Baykal Westcom/New Telephone Company/Far Eastern Cellular" },
  { 250,  "14", "SMARTS",                "SMARTS Ufa" },
  { 250,  "16", "NTC",                   "New Telephone Company" },
  { 250,  "17", "Utel",                  "JSC Uralsvyazinform" },
  { 250,  "19", "INDIGO",                "INDIGO" },
  { 250,  "20", "Tele2",                 "Tele2" },
  { 250,  "23", "Mobicom - Novosibirsk", "Mobicom - Novosibirsk" },
  { 250,  "39", "Utel",                  "Uralsvyazinform" },
  { 250,  "99", "Beeline",               "OJSC VimpelCom" },

  /*****************
   **** Ukraine ****
   *****************/
  { 255,  "01", "MTS", "Ukrainian Mobile Communications" },
  { 255,  "02", "Beeline", "Ukrainian Radio Systems" },
  { 255,  "03", "Kyivstar", "Kyivstar GSM JSC" },
  { 255,  "05", "Golden Telecom", "Golden Telecom" },
  { 255,  "06", "life:)", "Astelit" },
  { 255,  "07", "Utel", "Ukrtelecom" },

  /*****************
   **** Belarus ****
   *****************/
  { 257,  "01", "Velcom", "Velcom" },
  { 257,  "02", "MTS",    "JLLC Mobile TeleSystems" },
  { 257,  "04", "life:)", "Belarussian Telecommunications Network" },

  /*****************
   **** Moldova ****
   *****************/
  { 259,  "01", "Orange",   "Orange Moldova" },
  { 259,  "02", "Moldcell", "Moldcell" },
  { 259,  "04", "Eventis",  "Eventis Telecom" },

  /****************
   **** Poland ****
   ****************/
  { 260,  "01", "Plus",           "Polkomtel" },
  { 260,  "02", "Era",            "Polska Telefonia Cyfrowa (PTC)" },
  { 260,  "03", "Orange",         "PTK Centertel" },
  { 260,  "06", "Play",           "P4 Sp. zo.o" },
  { 260,  "12", "Cyfrowy Polsat", "Cyfrowy Polsat" },
  { 260,  "14", "Sferia",         "Sferia S.A." },

  /*****************
   **** Germany ****
   *****************/
  { 262,  "01", "T-Mobile",      "T-Mobile" },
  { 262,  "02", "Vodafone",      "Vodafone D2 GmbH" },
  { 262,  "03", "E-Plus",        "E-Plus Mobilfunk" },
  { 262,  "04", "Vodafone",      "Vodafone" },
  { 262,  "05", "E-Plus",        "E-Plus Mobilfunk" },
  { 262,  "06", "T-Mobile",      "T-Mobile" },
  { 262,  "07", "O2",            "O2 (Germany) GmbH & Co. OHG" },
  { 262,  "08", "O2",            "O2" },
  { 262,  "09", "Vodafone",      "Vodafone" },
  { 262,  "10", "Arcor AG & Co", "Arcor AG * Co" },
  { 262,  "11", "O2",            "O2" },
  { 262,  "15", "Airdata",       "Airdata" },
  { 262,  "60", "DB Telematik",  "DB Telematik" },
  { 262,  "76", "Siemens AG",    "Siemens AG" },
  { 262,  "77", "E-Plus",        "E-Plus" },

  /*******************
   **** Gibraltar ****
   *******************/
  { 266,  "01", "GibTel", "Gibraltar Telecoms" },

  /******************
   **** Portugal ****
   ******************/
  { 268,  "01", "Vodafone", "Vodafone Portugal" },
  { 268,  "03", "Optimus",  "Sonaecom - Servicos de Comunicacoes, S.A." },
  { 268,  "06", "TMN",      "Telecomunicacoes Moveis Nacionais" },

  /********************
   **** Luxembourg ****
   ********************/
  { 270,  "01", "LuxGSM",    "P&T Luxembourg" },
  { 270,  "77", "Tango",     "Tango SA" },
  { 270,  "99", "Voxmobile", "VOXmobile S.A" },

  /*****************
   **** Ireland ****
   *****************/
  { 272,  "01", "Vodafone", "Vodafone Ireland" },
  { 272,  "02", "O2",       "O2 Ireland" },
  { 272,  "03", "Meteor",   "Meteor" },
  { 272,  "05", "3",        "Hutchison 3G IReland limited" },

  /*****************
   **** Iceland ****
   *****************/
  { 274,  "01", "Siminn",   "Iceland Telecom" },
  { 274,  "02", "Vodafone", "iOg fjarskipti hf" },
  { 274,  "04", "Viking",   "IMC Island ehf" },
  { 274,  "07", "IceCell",  "IceCell ehf" },
  { 274,  "11", "Nova",     "Nova ehf" },

  /*****************
   **** Albania ****
   *****************/
  { 276,  "01", "AMC",          "Albanian Mobile Communications" },
  { 276,  "02", "Vodafone",     "Vodafone Albania" },
  { 276,  "03", "Eagle Mobile", "Eagle Mobile" },

  /***************
   **** Malta ****
   ***************/
  { 278,  "01", "Vodafone", "Vodafone Malta" },
  { 278,  "21", "GO",       "Mobisle Communications Limited" },
  { 278,  "77", "Melita",   "Melita Mobile Ltd. (3G Telecommunictaions Limited" },

  /****************
   **** Cyprus ****
   ****************/
  { 280,  "01", "Cytamobile-Vodafone", "Cyprus Telcommunications Auth" },
  { 280,  "10", "MTN",                 "Areeba Ltde" },

  /*****************
   **** Georgia ****
   *****************/
  { 282,  "01", "Geocell",  "Geocell Limited" },
  { 282,  "02", "Magti",    "Magticom GSM" },
  { 282,  "04", "Beeline",  "Mobitel LLC" },
  { 282,  "67", "Aquafon",  "Aquafon" },
  { 282,  "88", "A-Mobile", "A-Mobile" },

  /*****************
   **** Armenia ****
   *****************/
  { 283,  "01", "Beeline",      "ArmenTel" },
  { 283,  "05", "VivaCell-MTS", "K Telecom CJSC" },

  /******************
   **** Bulgaria ****
   ******************/
  { 284,  "01", "M-TEL",   "Mobiltel" },
  { 284,  "03", "Vivatel", "BTC" },
  { 284,  "05", "GLOBUL",  "Cosmo Bulgaria Mobile" },

  /****************
   **** Turkey ****
   ****************/
  { 286,  "01", "Turkcell", "Turkcell lletisim Hizmetleri A.S." },
  { 286,  "02", "Vodafone", "Vodafone Turkey" },
  { 286,  "03", "Avea",     "Avea" },

  /********************************
   **** Faroe Islands (Demark) ****
   ********************************/
  { 288,  "01", "Faroese",  "Faroese Telecom" },
  { 288,  "02", "Vodafone", "Vodafone Faroe Islands" },

  /*******************
   **** Greenland ****
   *******************/
  { 290,  "01", "TELE Greenland A/S", "Tele Greenland A/S" },

  /********************
   **** San Marino ****
   ********************/
  { 292,  "01", "PRIMA", "San Marino Telecom" },

  /******************
   **** Slovenia ****
   ******************/
  { 293,  "40", "Si.mobil", "SI.MOBIL d.d" },
  { 293,  "41", "Si.mobil", "Mobitel D.D." },
  { 293,  "64", "T-2",      "T-2 d.o.o." },
  { 293,  "70", "Tusmobil", "Tusmobil d.o.o." },

  /*******************************
   **** Republic of Macedonia ****
   *******************************/
  { 294,  "01", "T-Mobile",     "T-Mobile Makedonija" },
  { 294,  "02", "Cosmofon",     "Cosmofon" },
  { 294,  "02", "VIP Operator", "VIP Operator" },

  /***********************
   **** Liechtenstein ****
   ***********************/
  { 295,  "01", "Swisscom", "Swisscom Schweiz AG" },
  { 295,  "02", "Orange",   "Orange Liechtenstein AG" },
  { 295,  "05", "FL1",      "Mobilkom Liechtenstein AG" },
  { 295,  "77", "Tele 2",   "Belgacom" },

  /********************
   **** Montenegro ****
   ********************/
  { 297,  "01", "ProMonte", "ProMonte GSM" },
  { 297,  "02", "T-Mobile", "T-Mobile Montenegro LLC" },
  { 297,  "03", "m:tel CG", "MTEL CG" },

  /****************
   **** Canada ****
   ****************/
  { 302, "370", "Fido",                    "Fido" },
  { 302, "620", "ICE Wireless",            "ICE Wireless" },
  { 302, "720", "Rogers Wireless",         "Rogers Wireless" },

  /********************************************
   **** Saint Pierre and Miquelon (France) ****
   ********************************************/
  { 308,  "01", "Ameris", "St. Pierre-et-Miquelon Telecom" },

  /****************************************
   **** United States of America, Guam ****
   ****************************************/
  { 310,  "20", "Union Telephony Company",          "Union Telephony Company" },
  { 310,  "26", "T-Mobile",                         "T-Mobile" },
  { 310,  "30", "Centennial",                       "Centennial Communications" },
  { 310,  "38", "AT&T",                             "AT&T Mobility" },
  { 310,  "40", "Concho",                           "Concho Cellular Telephony Co., Inc." },
  { 310,  "46", "SIMMETRY",                         "TMP Corp" },
  { 310,  "70", "AT&T",                             "AT&T" },
  { 310,  "80", "Corr",                             "Corr Wireless Communications LLC" },
  { 310,  "90", "AT&T",                             "AT&T" },
  { 310, "100", "Plateau Wireless",                 "New Mexico RSA 4 East Ltd. Partnership" },
  { 310, "110", "PTI Pacifica",                     "PTI Pacifica Inc." },
  { 310, "150", "AT&T",                             "AT&T" },
  { 310, "170", "AT&T",                             "AT&T" },
  { 310, "180", "West Cen",                         "West Central" },
  { 310, "190", "Dutch Harbor",                     "Alaska Wireless Communications, LLC" },
  { 310, "260", "T-Mobile",                         "T-Mobile" },
  { 310, "300", "Get Mobile Inc",                   "Get Mobile Inc" },
  { 310, "311", "Farmers Wireless",                 "Farmers Wireless" },
  { 310, "330", "Cell One",                         "Cellular One" },
  { 310, "340", "Westlink",                         "Westlink Communications" },
  { 310, "380", "AT&T",                             "AT&T" },
  { 310, "400", "i CAN_GSM",                        "Wave runner LLC (Guam)" },
  { 310, "410", "AT&T",                             "AT&T" },
  { 310, "420", "Cincinnati Bell",                  "Cincinnati Bell Wireless" },
  { 310, "430", "Alaska Digitel",                   "Alaska Digitel" },
  { 310, "450", "Viaero",                           "Viaero Wireless" },
  { 310, "460", "Simmetry",                         "TMP Corporation" },
  { 310, "540", "Oklahoma Western",                 "Oklahoma Western Telephone Company" },
  { 310, "560", "AT&T",                             "AT&T" },
  { 310, "570", "Cellular One",                     "MTPCS, LLC" },
  { 310, "590", "Alltel",                           "Alltel Communications Inc" },
  { 310, "610", "Epic Touch",                       "Elkhart Telephone Co." },
  { 310, "620", "Coleman County Telecom",           "Coleman County Telecommunications" },
  { 310, "640", "Airadigim",                        "Airadigim Communications" },
  { 310, "650", "Jasper",                           "Jasper wireless, inc" },
  { 310, "680", "AT&T",                             "AT&T" },
  { 310, "770", "i wireless",                       "lows Wireless Services" },
  { 310, "790", "PinPoint",                         "PinPoint Communications" },
  { 310, "830", "Caprock",                          "Caprock Cellular" },
  { 310, "850", "Aeris",                            "Aeris Communications, Inc." },
  { 310, "870", "PACE",                             "Kaplan Telephone Company" },
  { 310, "880", "Advantage",                        "Advantage Cellular Systems" },
  { 310, "890", "Unicel",                           "Rural cellular Corporation" },
  { 310, "900", "Taylor",                           "Taylor Telecommunications" },
  { 310, "910", "First Cellular",                   "First Cellular of Southern Illinois" },
  { 310, "950", "XIT Wireless",                     "Texas RSA 1 dba XIT Cellular" },
  { 310, "970", "Globalstar",                       "Globalstar" },
  { 310, "980", "AT&T",                             "AT&T" },
  { 311,  "10", "Chariton Valley",                  "Chariton Valley Communications" },
  { 311,  "20", "Missouri RSA 5 Partnership",       "Missouri RSA 5 Partnership" },
  { 311,  "30", "Indigo Wireless",                  "Indigo Wireless" },
  { 311,  "40", "Commnet Wireless",                 "Commnet Wireless" },
  { 311,  "50", "Wikes Cellular",                   "Wikes Cellular" },
  { 311,  "60", "Farmers Cellular",                 "Farmers Cellular Telephone" },
  { 311,  "70", "Easterbrooke",                     "Easterbrooke Cellular Corporation" },
  { 311,  "80", "Pine Cellular",                    "Pine Telephone Company" },
  { 311,  "90", "Long Lines Wireless",              "Long Lines Wireless LLC" },
  { 311, "100", "High Plains Wireless",             "High Plains Wireless" },
  { 311, "110", "High Plains Wireless",             "High Plains Wireless" },
  { 311, "130", "Cell One Amarillo",                "Cell One Amarillo" },
  { 311, "150", "Wilkes Cellular",                  "Wilkes Cellular" },
  { 311, "170", "PetroCom",                         "Broadpoint Inc" },
  { 311, "180", "AT&T",                             "AT&T" },
  { 311, "210", "Farmers Cellular",                 "Farmers Cellular Telephone" },

  /*********************
   **** Puerto Rico ****
   *********************/
  { 330,  "11", "Claro", "Puerto Rico Telephony Company" },

  /****************
   **** Mexico ****
   ****************/
  { 334,  "02", "Telcel",   "America Movil" },
  { 334,  "03", "movistar", "Pegaso Comunicaciones y Sistemas" },

  /*****************
   **** Jamaica ****
   *****************/
  { 338,  "20", "Cable & Wireless", "Cable & Wireless" },
  { 338,  "50", "Digicel",          "Digicel (Jamaica) Limited" },
  { 338,  "70", "Claro",            "Oceanic Digital Jamaica Limited" },

  /*****************************
   **** Guadeloupe (France) ****
   *****************************/
  { 340,  "01", "Orange",   "Orange Caraibe Mobiles" },
  { 340,  "02", "Outremer", "Outremer Telecom" },
  { 340,  "03", "Teleceli", "Saint Martin et Saint Barthelemy Telcell Sarl" },
  { 340,  "08", "MIO GSM",  "Dauphin Telecom" },
  { 340,  "20", "Digicel",  "DIGICEL Antilles Franccaise Guyane" },

  /******************
   **** Barbados ****
   ******************/
  { 342, "600", "bmobile", "cable &Wireless Barbados Ltd." },
  { 342, "750", "Digicel", "Digicel (Jamaica) Limited" },

  /*****************************
   **** Antigua and Barbuda ****
   *****************************/
  { 344,  "30", "APUA",    "Antigua Public Utilities Authority" },
  { 344, "920", "bmobile", "Cable & Wireless Caribbean Cellular (Antigua) Limited" },
  { 344, "930", "Digicel", "Antigua Wireless Ventures Limited" },

  /*****************************************
   **** Cayman Islands (United Kingdom) ****
   *****************************************/
  { 346,   "50", "Digicel",          "Digicel Cayman Ltd." },
  { 346,  "140", "Cable & Wireless", "Cable & Wireless (Caymand Islands) Limited" },

  /*************************************************
   **** British Virgin Islands (United Kingdom) ****
   *************************************************/
  { 348, "170", "Cable & Wireless",             "Cable & Wireless (West Indies)" },
  { 348, "570", "Caribbean Cellular Telephone", "Caribbean Cellular Telephone" },

  /*****************
   **** Bermuda ****
   *****************/
  { 350,  "01", "Digicel Bermuda", "Telecommunications (Bermuda & West Indies) Ltd" },
  { 350,  "02", "Mobility",        "M3 wireless" },
  { 350,  "38", "Digicel",         "Digicel" },

  /*****************
   **** Grenada ****
   *****************/
  { 352,  "30", "Digicel",          "Digicel Grenada Ltd." },
  { 352, "110", "Cable & Wireless", "Cable & Wireless Grenada Ltd." },

  /******************************
   **** Netherlands Antilles ****
   ******************************/
  { 362,  "51", "Telcell", "Telcell N.V." },
  { 362,  "69", "Digicel", "Curacao Telecom N.V." },
  { 362,  "91", "UTS",     "Setel NV" },

  /********************************************
   **** Aruba (Kingdom of the Netherlands) ****
   ********************************************/
  { 363,  "01", "SETAR",    "SETAR (Servicio di Telecommunication diAruba" },
  { 363,  "20", "Digicell", "Digicell" },

  /*****************
   **** Bahamas ****
   *****************/
  { 364, "390", "BaTelCo", "The Bahamas Telecommunications Company Ltd" },

  /***********************************
   **** Anguilla (United Kingdom) ****
   ***********************************/
  { 365,  "10", "Weblinks Limited", "Weblinks Limited" },

  /**************
   **** Cuba ****
   **************/
  { 368,  "01", "ETECSA", "Empresa de Telecomunicaciones de Cuba, SA" },

  /****************************
   **** Dominican Republic ****
   ****************************/
  { 370,  "01", "Orange", "Orange Dominicana" },
  { 370,  "02", "Claro",  "Compania Dominicana de Telefonos, C por" },
  { 370,  "04", "ViVa",   "Centennial Dominicana" },

  /***************
   **** Haiti ****
   ***************/
  { 372,  "10", "Comcel / Voila", "Comcel / Voila" },
  { 372,  "50", "Digicel",        "Digicel" },

  /*****************************
   **** Trinidad and Tobaga ****
   *****************************/
  { 374,  "12", "bmobile", "TSTT" },
  { 374,  "13", "Digicel", "Digicel" },

  /********************
   **** Azerbaijan ****
   ********************/
  { 400,  "01", "Azercell",   "Azercell" },
  { 400,  "02", "Bakcell",    "Bakcell" },
  { 400,  "04", "Nar Mobile", "Azerfon" },

  /********************
   **** Kazakhstan ****
   ********************/
  { 401,  "01", "Beeline",                "KaR-TeL LLP" },
  { 401,  "02", "K'Cell",                 "GSM Kazakhstan Ltdx." },
  { 401,  "77", "Mobile Telecom Service", "Mobile Telecom Service LLP" },

  /****************
   **** Bhutan ****
   ****************/
  { 402,  "11", "B-Mobile",  "B-Mobile" },
  { 402,  "77", "TashiCell", "Tashi InfoComm Limited" },

  /***************
   **** India ****
   ***************/
  { 404,  "01", "Vodafone - Haryana",            "Vodafone" },
  { 404,  "02", "Airtel - Punjab",               "Bharti Airtel" },
  { 404,  "03", "Airtel - Himachal Pradesh",                        "Bharti Airtel" },
  { 404,  "04", "Idea - Delhi",                  "Idea cellular Limited " },
  { 404,  "05", "Vodafone - Gujarat",            "Vodafone" },
  { 404,  "07", "Idea - Andhra Pradesh",                 "Idea Cellular Limited" },
  { 404,  "09", "Reliance - Assam",                 "Reliance Communications" },
  { 404,  "10", "Airtel Delhi",                 "Bharti Airtel" },
  { 404,  "11", "Vodafone - Delhi",                "Vodafone" },
  { 404,  "12", "Idea - Haryana",                  "Idea Cellular Limited" },
  { 404,  "13", "Vodafone - Andhra Pradesh",                      "Vodafone" },
  { 404,  "14", "Spice Telecom - Punjab",     "Spice Communications Limited" },
  { 404,  "15", "Vodafone - Uttar Pradesh (East)",                      "Vodafone" },
  { 404,  "16", "Airtel - North East",              "Bharti Airtel" },
  { 404,  "17", "Aircel - West Bengal",               "Dishnet Wireless/Aircel" },
  { 404,  "18", "Reliance - Himachal Pradesh",        "Reliance Communications" },
  { 404,  "19", "Idea - Kerala",       "Idea Cellular Limited" },
  { 404,  "20", "Vodafone - Mumbai",                      "Vodafone" },
  { 404,  "21", "BPL Mobile Mumbai",             "BPL Mobile Mumbai" },
  { 404,  "22", "Idea - Maharashtra",   "Idea Cellular Limited" },
  { 404,  "24", "Idea - Gujarat",       "Idea Cellular Limited" },
  { 404,  "25", "Aircel - Bihar",               "Dishnet Wireless/Aircel" },
  { 404,  "27", "Vodafone - Maharashtra",        "Vodafone" },
  { 404,  "28", "Aircel - Orissa",               "Dishnet Wireless/Aircel" },
  { 404,  "29", "Aircel - Assam",               "Dishnet Wireless/Aircel" },
  { 404,  "30", "Vodafone - Kolkata",            "Vodafone" },
  { 404,  "31", "Airtel - Kolkata",              "Bharti Airtel" },
  { 404,  "33", "Aircel - North East",               "Dishnet Wireless/Aircel" },
  { 404,  "34", "BSNL - Haryana",            "Bharat Sanchar Nigam Limited" },
  { 404,  "35", "Aircel - Himachal Pradesh",               "Dishnet Wireless/Aircel" },
  { 404,  "36", "Reliance - Bihar",                      "Reliance Communications" },
  { 404,  "37", "Aircel - Jammu & Kashmir",               "Dishnet Wireless/Aircel" },
  { 404,  "38", "BSNL - Assam",            "Bharat Sanchar Nigam Limited" },
  { 404,  "40", "Airtel - Chennai",          "Bharti Airtel" },
  { 404,  "41", "Aircel - Chennai",               "Dishnet Wireless/Aircel" },
  { 404,  "42", "Aircel - Tamilnadu",               "Dishnet Wireless/Aircel" },
  { 404,  "43", "Vodafone - Tamilnadu",        "Vodafone" },
  { 404,  "44", "Spice Telecom - Karnataka",     "Spice Communications Limited" },
  { 404,  "46", "Vodafone - Kerala",        "Vodafone" },
  { 404,  "49", "Airtel - Andhra Pradesh",          "Bharti Airtel" },
  { 404,  "50", "Reliance - North East",        "Reliance Communications" },
  { 404,  "51", "BSNL - Himachal Pradeshl",            "Bharti Sanchar Nigam Limited" },
  { 404,  "52", "Reliance - Orissa",             "Reliance Communications" },
  { 404,  "53", "BSNL - Punjab",            "Bharti Sanchar Nigam Limited" },
  { 404,  "54", "BSNL - Uttar Pradesh (West)",            "Bharti Sanchar Nigam Limited" },
  { 404,  "55", "BSNL - Uttar Pradesh (East)",            "Bharti Sanchar Nigam Limited" },
  { 404,  "56", "Idea - Uttar Pradesh West",                "Idea Cellular Limited" },
  { 404,  "57", "BSNL - Gujarat",            "Bharat Sanchar Nigam Limited" },
  { 404,  "58", "BSNL - Madhya Pradesh",            "Bharat Sanchar Nigam Limited" },
  { 404,  "59", "BSNL - Rajasthan",            "Bharat Sanchar Nigam Limited" },
  { 404,  "60", "Vodafone - Rajasthan",        "Vodafone" },
  { 404,  "62", "BSNL - Jammu & Kashmir",            "Bharat Sanchar Nigam Limited" },
  { 404,  "64", "BSNL - Chennai",            "Bharat Sanchar Nigam Limited" },
  { 404,  "66", "BSNL - Maharashtra",            "Bharat Sanchar Nigam Limited" },
  { 404,  "67", "Vodafone - West Bengal",        "Vodafone" },
  { 404,  "68", "MTNL - Delhi",                  "Mahanagar Telephone Nigam Ltd" },
  { 404,  "69", "MTNL - Mumbai",                 "Mahanagar Telephone Nigam Ltd" },
  { 404,  "70", "Airtel - Rajasthan",        "Bharti Airtel" },
  { 404,  "71", "BSNL - Karnataka",                 "Bharti Sanchar Nigam Limited" },
  { 404,  "72", "BSNL - Kerala",                 "Bharti Sanchar Nigam Limited" },
  { 404,  "73", "BSNL - Andhra Pradesh",                 "Bharti Sanchar Nigam Limited" },
  { 404,  "74", "BSNL - West Bengal",            "Bharti Sanchar Nigam Limited" },
  { 404,  "75", "BSNL - Bihar",                "Bharti Sanchar Nigam Limited" },
  { 404,  "76", "BSNL - Orissa",                 "Bharti Sanchar Nigam Limited" },
  { 404,  "77", "BSNL - North East",                 "Bharti Sanchar Nigam Limited" },
  { 404,  "78", "Idea - Madhya Pradesh",       "Idea Cellular Limited" },
  { 404,  "79", "BSNL - Andaman Nicobar",                 "Bharti Sanchar Nigam Limited" },
  { 404,  "80", "BSNL - Tamilnadu",                 "Bharti Sanchar Nigam Limited" },
  { 404,  "81", "BSNL - Kolkata",                "Bharti Sanchar Nigam Limited" },
  { 404,  "82", "Idea - Himachal Pradesh",       "Idea Cellular Limited" },
  { 404,  "83", "Reliance - Kolkata",                      "Reliance Communications" },
  { 404,  "84", "Vodafone - Chennai",        "Vodafone" },
  { 404,  "85", "Reliance - West Bengal",                      "Reliance Communications" },
  { 404,  "86", "Vodafone - Karnataka",        "Vodafone" },
  { 404,  "87", "Idea - Rajasthan",       "Idea Cellular Limited" },
  { 404,  "88", "Vodafone - Punjab",        "Vodafone" },
  { 404,  "89", "Idea - Uttar Pradesh (East)",       "Idea Cellular Limited" },
  { 404,  "90", "Airtel - Maharashtra",          "Bharti Airtel" },
  { 404,  "91", "Airtel - Kolkata Metro Circle", "Bharti Airtel" },
  { 404,  "92", "Airtel Mumbai",                 "Bharti Airtel" },
  { 404,  "93", "Airtel Madhya Pradesh",                 "Bharti Airtel" },
  { 404,  "94", "Airtel Tamilnadu",                 "Bharti Airtel" },
  { 404,  "95", "Airtel - Kerala",                        "Bharti Airtel" },
  { 404,  "96", "Airtel - Haryana",              "Bharti Airtel" },
  { 404,  "97", "Airtel - Uttar Pradesh (West)",              "Bharti Airtel" },
  { 405,  "01", "Reliance - Andhra Pradesh",        "Reliance Communications" },
  { 405,  "03", "Reliance - Bihar",        "Reliance Communications" },
  { 405,  "04", "Reliance - Chennai",        "Reliance Communications" },
  { 405,  "05", "Reliance - Delhi",        "Reliance Communications" },
  { 405,  "06", "Reliance - Gujarat",        "Reliance Communications" },
  { 405,  "07", "Reliance - Haryana",        "Reliance Communications" },
  { 405,  "08", "Reliance - Himachal Pradesh",        "Reliance Communications" },
  { 405,  "09", "Reliance - Jammu & Kashmir",        "Reliance Communications" },
  { 405,  "10", "Reliance - Karnataka",        "Reliance Communications" },
  { 405,  "11", "Reliance - Kerala",        "Reliance Communications" },
  { 405,  "12", "Reliance - Kolkata",        "Reliance Communications" },
  { 405,  "13", "Reliance - Maharashtra",        "Reliance Communications" },
  { 405,  "14", "Reliance - Madhya Pradesh",        "Reliance Communications" },
  { 405,  "15", "Reliance - Mumbai",        "Reliance Communications" },
  { 405,  "17", "Reliance - Orissa",        "Reliance Communications" },
  { 405,  "18", "Reliance - Punjab",        "Reliance Communications" },
  { 405,  "19", "Reliance - Rajasthan",        "Reliance Communications" },
  { 405,  "20", "Reliance - Tamilnadu",        "Reliance Communications" },
  { 405,  "21", "Reliance - Uttar Pradesh (East)",        "Reliance Communications" },
  { 405,  "22", "Reliance - Uttar Pradesh (West)",        "Reliance Communications" },
  { 405,  "23", "Reliance - West Bengal",        "Reliance Communications" },
  { 405,  "23", "Reliance - West Bengal",        "Reliance Communications" },
  { 405,  "25", "Tata - Andhra Pradesh",        "Tata Teleservices" },
  { 405,  "26", "Tata - Assam",        "Tata Teleservices" },
  { 405,  "27", "Tata - Bihar",        "Tata Teleservices" },
  { 405,  "28", "Tata - Chennai",        "Tata Teleservices" },
  { 405,  "29", "Tata - Delhi",        "Tata Teleservices" },
  { 405,  "30", "Tata - Gujarat",        "Tata Teleservices" },
  { 405,  "31", "Tata - Haryana",        "Tata Teleservices" },
  { 405,  "32", "Tata - Himachal Pradesh",        "Tata Teleservices" },
  { 405,  "33", "Tata - Jammu & Kashmir",        "Tata Teleservices" },
  { 405,  "34", "Tata - Karnataka",        "Tata Teleservices" },
  { 405,  "35", "Tata - Kerala",        "Tata Teleservices" },
  { 405,  "36", "Tata - Kolkata",        "Tata Teleservices" },
  { 405,  "37", "Tata - Maharashtra",        "Tata Teleservices" },
  { 405,  "38", "Tata - Madhya Pradesh",        "Tata Teleservices" },
  { 405,  "39", "Tata - Mumbai",        "Tata Teleservices" },
  { 405,  "40", "Tata - North East",        "Tata Teleservices" },
  { 405,  "41", "Tata - Orissa",        "Tata Teleservices" },
  { 405,  "42", "Tata - Punjab",        "Tata Teleservices" },
  { 405,  "43", "Tata - Rajasthan",        "Tata Teleservices" },
  { 405,  "44", "Tata - Tamilnadu",        "Tata Teleservices" },
  { 405,  "45", "Tata - Uttar Pradesh (East)",        "Tata Teleservices" },
  { 405,  "46", "Tata - Uttar Pradesh (West)",        "Tata Teleservices" },
  { 405,  "47", "Tata - West Bengal",        "Tata Teleservices" },
  { 405,  "51", "Airtel - West Bengal",              "Bharti Airtel" },
  { 405,  "52", "Airtel - Bihar",              "Bharti Airtel" },
  { 405,  "53", "Airtel - Orissa",              "Bharti Airtel" },
  { 405,  "54", "Airtel - Uttar Pradesh (East)",              "Bharti Airtel" },
  { 405,  "55", "Airtel - Jammu & Kashmir",              "Bharti Airtel" },
  { 405,  "56", "Airtel - Assam",              "Bharti Airtel" },
  { 405,  "66", "Vodafone - Uttar Pradesh (West)",                "Vodafone" },
  { 405,  "67", "Vodafone - West Bengal",                "Vodafone" },
  { 405,  "70", "Idea - Bihar",       "Idea Cellular Limited" },
  { 405,  "750", "Vodafone - Jammu & Kashmir",                "Vodafone" },
  { 405,  "751", "Vodafone - Assam",                "Vodafone" },
  { 405,  "752", "Vodafone - Bihar",                "Vodafone" },
  { 405,  "753", "Vodafone - Orissa",                "Vodafone" },
  { 405,  "754", "Vodafone - Himachal Pradesh",                "Vodafone" },
  { 405,  "755", "Vodafone - North East",                "Vodafone" },
  { 405,  "756", "Vodafone - Madhya Pradesh",                "Vodafone" },
  { 405,  "799", "Idea - Mumbai",       "Idea Cellular Limited" },
  { 405,  "800", "Aircel - Delhi",               "Dishnet Wireless/Aircel" },
  { 405,  "801", "Aircel - Andhra Pradesh",               "Dishnet Wireless/Aircel" },
  { 405,  "802", "Aircel - Gujarat",               "Dishnet Wireless/Aircel" },
  { 405,  "803", "Aircel - Karnataka",               "Dishnet Wireless/Aircel" },
  { 405,  "804", "Aircel - Maharashtra",               "Dishnet Wireless/Aircel" },
  { 405,  "805", "Aircel - Mumbai",               "Dishnet Wireless/Aircel" },
  { 405,  "806", "Aircel - Rajasthan",               "Dishnet Wireless/Aircel" },
  { 405,  "807", "Aircel - Haryana",               "Dishnet Wireless/Aircel" },
  { 405,  "808", "Aircel - Punjab",               "Dishnet Wireless/Aircel" },
  { 405,  "809", "Aircel - Kerala",               "Dishnet Wireless/Aircel" },
  { 405,  "810", "Aircel - Uttar Pradesh (East)",               "Dishnet Wireless/Aircel" },
  { 405,  "811", "Aircel - Uttar Pradesh (West)",               "Dishnet Wireless/Aircel" },
  { 405,  "812", "Aircel - Madhya Pradesh",               "Dishnet Wireless/Aircel" },
  { 405,  "813", "Unitech - Haryana",               "Unitech Wireless" },
  { 405,  "814", "Unitech - Himachal Pradesh",               "Unitech Wireless" },
  { 405,  "815", "Unitech - Jammu & Kashmir",               "Unitech Wireless" },
  { 405,  "816", "Unitech - Punjab",               "Unitech Wireless" },
  { 405,  "817", "Unitech - Rajasthan",               "Unitech Wireless" },
  { 405,  "818", "Unitech - Uttar Pradesh (West)",               "Unitech Wireless" },
  { 405,  "819", "Unitech - Andhra Pradesh",               "Unitech Wireless" },
  { 405,  "820", "Unitech - Karnataka",               "Unitech Wireless" },
  { 405,  "821", "Unitech - Kerala",               "Unitech Wireless" },
  { 405,  "822", "Unitech - Kolkata",               "Unitech Wireless" },
  { 405,  "844", "Unitech - Delhi",               "Unitech Wireless" },
  { 405,  "845", "Idea - Assam",       "Idea Cellular Limited" },
  { 405,  "846", "Idea - Jammu & Kashmir",       "Idea Cellular Limited" },
  { 405,  "847", "Idea - Karnataka",       "Idea Cellular Limited" },
  { 405,  "848", "Idea - Kolkata",       "Idea Cellular Limited" },
  { 405,  "849", "Idea - North East",       "Idea Cellular Limited" },
  { 405,  "850", "Idea - Orissa",       "Idea Cellular Limited" },
  { 405,  "851", "Idea - Punjab",       "Idea Cellular Limited" },
  { 405,  "852", "Idea - Tamilnadu",       "Idea Cellular Limited" },
  { 405,  "853", "Idea - West Bengal",       "Idea Cellular Limited" },
  { 405,  "854", "Loop - Andhra Pradesh",       "Loop Mobile" },
  { 405,  "875", "Unitech - Assam",               "Unitech Wireless" },
  { 405,  "876", "Unitech - Bihar",               "Unitech Wireless" },
  { 405,  "877", "Unitech - North East",               "Unitech Wireless" },
  { 405,  "878", "Unitech - Orissa",               "Unitech Wireless" },
  { 405,  "879", "Unitech - Uttar Pradesh (East)",               "Unitech Wireless" },
  { 405,  "880", "Unitech - West Bengal",               "Unitech Wireless" },
  { 405,  "887", "Shyam - Andhra Pradesh",               "Sistema Shyam" },
  { 405,  "888", "Shyam - Assam",               "Sistema Shyam" },
  { 405,  "889", "Shyam - Bihar",               "Sistema Shyam" },
  { 405,  "890", "Shyam - Delhi",               "Sistema Shyam" },
  { 405,  "891", "Shyam - Gujarat",               "Sistema Shyam" },
  { 405,  "892", "Shyam - Haryana",               "Sistema Shyam" },
  { 405,  "893", "Shyam - Himachal Pradesh",               "Sistema Shyam" },
  { 405,  "894", "Shyam - Jammu & Kashmir",               "Sistema Shyam" },
  { 405,  "895", "Shyam - Karnataka",               "Sistema Shyam" },
  { 405,  "896", "Shyam - Kerala",               "Sistema Shyam" },
  { 405,  "897", "Shyam - Kolkata",               "Sistema Shyam" },
  { 405,  "898", "Shyam - Maharashtra",               "Sistema Shyam" },
  { 405,  "899", "Shyam - Madhya Pradesh",               "Sistema Shyam" },
  { 405,  "900", "Shyam - Mumbai",               "Sistema Shyam" },
  { 405,  "901", "Shyam - North East",               "Sistema Shyam" },
  { 405,  "902", "Shyam - Orissa",               "Sistema Shyam" },
  { 405,  "912", "Etisalat  - Andhra Pradesh",               "Etisalat DB" },
  /******************
   **** Pakistan ****
   ******************/
  { 410,  "01", "Mobilink", "Mobilink-PMCL" },
  { 410,  "03", "Ufone",    "Pakistan Telecommunication Mobile Ltd" },
  { 410,  "04", "Zong",     "China Mobile" },
  { 410,  "06", "Telenor",  "Telenor Pakistan" },
  { 410,  "07", "Warid",    "WaridTel" },

  /*******************
   *** Afghanistan ***
   ********************/
  { 412,  "01", "AWCC",     "Afghan wireless Communication Company" },
  { 412,  "20", "Roshan",   "Telecom Development Company Afghanistan Ltd." },
  { 412,  "40", "Areeba",   "MTN Afghanistan" },
  { 412,  "50", "Etisalat", "Etisalat Afghanistan" },

  /*******************
   **** Sri Lanka ****
   *******************/
  { 413,  "01", "Mobitel",         "Mobitel Lanka Ltd." },
  { 413,  "02", "Dialog",          "Dialog Telekom PLC." },
  { 413,  "03", "Tigo",            "Celtel Lanka Ltd" },
  { 413,  "08", "Hutch Sri Lanka", "Hutch Sri Lanka" },

  /*****************
   **** Myanmar ****
   *****************/
  { 414,  "01", "MPT", "Myanmar Post and Telecommunication" },

  /*****************
   **** Lebanon ****
   *****************/
  { 415,  "01", "Alfa",      "Alfa" },
  { 415,  "03", "MTC-Touch", "MIC 2" },

  /****************
   **** Jordan ****
   ****************/
  { 416,  "01", "Zain",   "Jordan Mobile Teelphone Services" },
  { 416,  "03", "Umniah", "Umniah" },
  { 416,  "77", "Orange", "Oetra Jordanian Mobile Telecommunications Company (MobileCom)" },

  /***************
   **** Syria ****
   ***************/
  { 417,  "01", "SyriaTel", "SyriaTel" },
  { 417,  "02", "MTN Syria", "MTN Syria (JSC)" },

  /****************
   **** Iraq ****
   ****************/
  { 418,  "20", "Zain Iraq", "Zain Iraq" },
  { 418,  "30", "Zain Iraq", "Zain Iraq" },
  { 418,  "50", "Asia Cell", "Asia Cell Telecommunications Company" },
  { 418,  "40", "Korek",     "Korel Telecom Ltd" },

  /****************
   **** Kuwait ****
   ****************/
  { 419,  "02", "Zain",     "Mobile Telecommunications Co." },
  { 419,  "03", "Wataniya", "National Mobile Telecommunications" },
  { 419,  "04", "Viva",     "Kuwait Telecommunication Company" },

  /**********************
   **** Saudi Arabia ****
   **********************/
  { 420,  "01", "STC",     "Saudi Telecom Company" },
  { 420,  "03", "Mobily",  "Etihad Etisalat Company" },
  { 420,  "04", "Zain SA", "MTC Saudi Arabia" },

  /***************
   **** Yemen ****
   ***************/
  { 421,  "01", "SabaFon", "SabaFon" },
  { 421,  "02", "MTN",     "Spacetel" },

  /**************
   **** Oman ****
   **************/
  { 422,  "02", "Oman Mobile", "Oman Telecommunications Company" },
  { 422,  "03", "Nawras",      "Omani Qatari Telecommunications Company SAOC" },

  /******************************
   **** United Arab Emirates ****
   ******************************/
  { 424,  "02", "Etisalat", "Emirates Telecom Corp" },
  { 424,  "03", "du",       "Emirates Integrated Telecommunications Company" },

  /****************
   **** Israel ****
   ****************/
  { 425,  "01", "Orange",    "Partner Communications Company Ltd" },
  { 425,  "02", "Cellcom",   "Cellcom" },
  { 425,  "03", "Pelephone", "Pelephone" },

  /*******************************
   **** Palestinian Authority ****
   *******************************/
  { 425,  "05", "JAWWAL", "Palestine Cellular Communications, Ltd." },

  /***************
   **** Qatar ****
   ***************/
  { 427,  "01", "Qatarnet", "Q-Tel" },

  /******************
   **** Mongolia ****
   ******************/
  { 428,  "88", "Unitel", "Unitel LLC" },
  { 428,  "99", "MobiCom", "MobiCom Corporation" },

  /***************
   **** Nepal ****
   ***************/
  { 429,  "01", "Nepal Telecom", "Nepal Telecom" },
  { 429,  "02", "Mero Mobile",   "Spice Nepal Private Ltd" },

  /**************
   **** Iran ****
   **************/
  { 432,  "11", "MCI",      "Mobile Communications Company of Iran" },
  { 432,  "14", "TKC",      "KFZO" },
  { 432,  "19", "MTCE",     "Mobile Telecommunications Company of Esfahan" },
  { 432,  "32", "Taliya",   "Taliya" },
  { 432,  "35", "Irancell", "Irancell Telecommunications Services Company" },

  /********************
   **** Uzbekistan ****
   ********************/
  { 434,  "04", "Beeline", "Unitel LLC" },
  { 434,  "05", "Ucell",   "Coscom" },
  { 434,  "07", "MTS",     "Mobile teleSystems (FE 'Uzdunrobita' Ltd)" },

  /********************
   **** Tajikistan ****
   ********************/
  { 436,  "01", "Somoncom",   "JV Somoncom" },
  { 436,  "02", "Indigo",     "Indigo Tajikistan" },
  { 436,  "03", "MLT",        "TT Mobile, Closed joint-stock company" },
  { 436,  "04", "Babilon-M",  "CJSC Babilon-Mobile" },
  { 436,  "05", "Beeline TJ", "Co Ltd. Tacom" },

  /********************
   **** Kyrgyzstan ****
   ********************/
  { 437,  "01", "Bitel",   "Sky Mobile LLC" },
  { 437,  "05", "MegaCom", "BiMoCom Ltd" },
  { 437,  "09", "O!",      "NurTelecom LLC" },

  /**********************
   **** Turkmenistan ****
   **********************/
  { 438,  "01", "MTS",     "Barash Communication Technologies" },
  { 438,  "02", "TM-Cell", "TM-Cell" },

  /***************
   **** Japan ****
   ***************/
  { 440,  "00", "eMobile",  "eMobile, Ltd." },
  { 440,  "01", "DoCoMo",   "NTT DoCoMo" },
  { 440,  "02", "DoCoMo",   "NTT DoCoMo Kansai" },
  { 440,  "03", "DoCoMo",   "NTT DoCoMo Hokuriku" },
  { 440,  "04", "SoftBank", "SoftBank Mobile Corp" },
  { 440,  "06", "SoftBank", "SoftBank Mobile Corp" },
  { 440,  "10", "DoCoMo",   "NTT DoCoMo Kansai" },
  { 440,  "20", "SoftBank", "SoftBank Mobile Corp" },

  /*********************
   **** South Korea ****
   *********************/
  { 450,  "05", "SKT",      "SK Telecom" },
  { 450,  "08", "KTF SHOW", "KTF" },

  /*****************
   **** Vietnam ****
   *****************/
  { 452,  "01", "MobiFone",       "Vietnam Mobile Telecom Services Company" },
  { 452,  "02", "Vinaphone",      "Vietnam Telecoms Services Company" },
  { 452,  "04", "Viettel Mobile", "iViettel Corporation" },

  /*******************
   **** Hong Kong ****
   *******************/
  { 454,  "00", "CSL",                   "Hong Kong CSL Limited" },
  { 454,  "01", "CITIC Telecom 1616",    "CITIC Telecom 1616" },
  { 454,  "02", "CSL 3G",                "Hong Kong CSL Limited" },
  { 454,  "03", "3(3G)",                 "Hutchison Telecom" },
  { 454,  "04", "3 DualBand (2G)",       "Hutchison Telecom" },
  { 454,  "06", "Smartone-Vodafone",     "SmarTone Mobile Comms" },
  { 454,  "07", "China Unicom",          "China Unicom" },
  { 454,  "08", "Trident",               "Trident" },
  { 454,  "09", "China Motion Telecom",  "China Motion Telecom" },
  { 454,  "10", "New World",             "Hong Kong CSL Limited" },
  { 454,  "11", "Chia-HongKong Telecom", "Chia-HongKong Telecom" },
  { 454,  "12", "CMCC Peoples",          "China Mobile Hong Kong Company Limited" },
  { 454,  "14", "Hutchison Telecom",     "Hutchison Telecom" },
  { 454,  "15", "SmarTone Mobile Comms", "SmarTone Mobile Comms" },
  { 454,  "16", "PCCW",                  "PCCW Mobile (PCCW Ltd)" },
  { 454,  "17", "SmarTone Mobile Comms", "SmarTone Mobile Comms" },
  { 454,  "18", "Hong Kong CSL Limited", "Hong Kong CSL Limited" },
  { 454,  "19", "PCCW",                  "PCCW Mobile (PCCW Ltd)" },

  /***************
   **** Macau ****
   ***************/
  { 455,  "00", "SmarTone", "SmarTone Macau" },
  { 455,  "01", "CTM",      "C.T.M. Telemovel+" },
  { 455,  "03", "3",        "Hutchison Telecom" },
  { 455,  "04", "CTM",      "C.T.M. Telemovel+" },
  { 455,  "05", "3",        "Hutchison Telecom" },

  /******************
   **** Cambodia ****
   ******************/
  { 456,  "01", "Mobitel",    "CamGSM" },
  { 456,  "02", "hello",      "Telekom Malaysia International (Cambodia) Co. Ltd" },
  { 456,  "04", "qb",         "Cambodia Advance Communications Co. Ltd" },
  { 456,  "05", "Star-Cell",  "APPLIFONE CO. LTD." },
  { 456,  "18", "Shinawatra", "Shinawatra" },

  /**************
   **** Laos ****
   **************/
  { 457,  "01", "LaoTel", "Lao Shinawatra Telecom" },
  { 457,  "02", "ETL",    "Enterprise of Telecommunications Lao" },
  { 457,  "03", "LAT",    "Lao Asia Telecommunication State Enterprise (LAT)" },
  { 457,  "08", "Tigo",   "Millicom Lao Co Ltd" },

  /***************
   **** China ****
   ***************/
  { 460,  "00", "China Mobile", "China Mobile" },
  { 460,  "01", "China Unicom", "China Unicom" },

  /****************
   **** Taiwan ****
   ****************/
  { 466,  "01", "FarEasTone", "Far EasTone Telecommunications Co Ltd" },
  { 466,  "06", "Tuntex", "Tuntex Telecom" },
  { 466,  "88", "KG Telecom", "KG Telecom" },
  { 466,  "89", "VIBO", "VIBO Telecom" },
  { 466,  "92", "Chungwa", "Chungwa" },
  { 466,  "93", "MobiTai", "iMobitai Communications" },
  { 466,  "97", "Taiwan Mobile", "Taiwan Mobile Co. Ltd" },
  { 466,  "99", "TransAsia", "TransAsia Telecoms" },

  /*********************
   **** North Korea ****
   *********************/
  { 467, "193", "SUN NET", "Korea Posts and Telecommunications Corporation" },

  /********************
   **** Bangladesh ****
   ********************/
  { 470,   "01", "Grameenphone", "GrameenPhone Ltd" },
  { 470,   "02", "Aktel",        "Aktel" },
  { 470,   "03", "Banglalink",   "Orascom telecom Bangladesh Limited" },
  { 470,   "04", "TeleTalk",     "TeleTalk" },
  { 470,   "06", "Citycell",     "Citycell" },
  { 470,   "07", "Warid",        "Warid Telecom" },

  /******************
   **** Maldives ****
   ******************/
  { 472,   "01", "Dhiraagu", "Dhivehi Raajjeyge Gulhun" },
  { 472,   "02", "Wataniya", "Wataniya Telecom Maldives" },

  /******************
   **** Malaysia ****
   ******************/
  { 502,  "12", "Maxis",    "Maxis Communications Berhad" },
  { 502,  "13", "Celcom",   "Celcom Malaysia Sdn Bhd" },
  { 502,  "16", "DiGi",     "DiGi Telecommunications" },
  { 502,  "18", "U Mobile", "U Mobile Sdn Bhd" },
  { 502,  "19", "Celcom",   "Celcom Malaysia Sdn Bhd" },

  /*******************
   **** Australia ****
   *******************/
  { 505,  "01", "Telstra",               "Telstra Corp. Ltd." },
  { 505,  "02", "YES OPTUS",             "Singtel Optus Ltd" },
  { 505,  "03", "Vodafone",              "Vodafone Australia" },
  { 505,  "06", "3",                     "Hutchison 3G" },
  { 505,  "90", "YES OPTUS",             "Singtel Optus Ltd" },

  /*******************
   **** Indonesia ****
   *******************/
  { 510,  "00", "PSN",          "PT Pasifik Satelit Nusantara (ACeS)" },
  { 510,  "01", "INDOSAT",      "PT Indonesian Satellite Corporation Tbk (INDOSAT)" },
  { 510,  "08", "AXIS",         "PT Natrindo Telepon Seluler" },
  { 510,  "10", "Telkomsel",    "PT Telkomunikasi Selular" },
  { 510,  "11", "XL",           "PT Excelcomindo Pratama" },
  { 510,  "89", "3",            "PT Hutchison CP Telecommunications" },

  /********************
   **** East Timor ****
   ********************/
  { 514,   "02", "Timor Telecom", "Timor Telecom" },

  /********************
   **** Philipines ****
   ********************/
  { 515,  "01", "Islacom",    "Innove Communicatiobs Inc" },
  { 515,  "02", "Globe",      "Globe Telecom" },
  { 515,  "03", "Smart Gold", "Smart Communications Inc" },
  { 515,  "05", "Digitel",    "Digital Telecommunications Philppines" },
  { 515,  "18", "Red Mobile", "Connectivity Unlimited resource Enterprise" },

  /******************
   **** Thailand ****
   ******************/
  { 520,  "01", "Advanced Info Service", "Advanced Info Service" },
  { 520,  "15", "ACT Mobile",            "ACT Mobile" },
  { 520,  "18", "DTAC",                  "Total Access Communication" },
  { 520,  "23", "Advanced Info Service", "Advanced Info Service" },
  { 520,  "99", "True Move",             "True Move" },

  /*******************
   **** Singapore ****
   *******************/
  { 525,   "01", "SingTel",                       "Singapore Telecom" },
  { 525,   "02", "SingTel-G18",                   "Singapore Telecom" },
  { 525,   "03", "M1",                            "MobileOne Asia" },
  { 525,   "05", "StarHub",                       "StarHub Mobile" },

  /****************
   **** Brunei ****
   ****************/
  { 528,   "02", "B-Mobile", "B-Mobile Communications Sdn Bhd" },
  { 528,  "11", "DTSCom",   "DataStream Technology" },

  /*********************
   **** New Zealand ****
   *********************/
  { 530,  "01", "Vodafone", "Vodafone New Zealand" },
  { 530,  "03", "Woosh",    "Woosh wireless New Zealand" },
  { 530,  "05", "Telecom",  "Telecom New Zealand" },
  { 530,  "24", "NZ Comms", "NZ Communications New Zealand" },

  /**************************
   **** Papua New Guinea ****
   **************************/
  { 537,  "01", "B-Mobile", "Pacific Mobile Communications" },

  /*************************
   **** Solomon Islands ****
   *************************/
  { 540,  "01", "BREEZE", "Solomon Telekom Co Ltd" },

  /*****************
   **** Vanuatu ****
   *****************/
  { 541,  "01", "SMILE", "telecom Vanuatu Ltd" },

  /**************
   **** Fiji ****
   **************/
  { 542,  "01", "Vodafone", "Vodafone Fiji" },

  /******************
   **** Kiribati ****
   ******************/
  { 545,  "09", "Kiribati Frigate", "Telecom services Kiribati Ltd" },

  /***********************
   **** New Caledonia ****
   ***********************/
  { 546,  "01", "Mobilis", "OPT New Caledonia" },

  /**************************
   **** French Polynesia ****
   **************************/
  { 547,  "20", "VINI", "Tikiphone SA" },

  /************************************
   **** Cook Islands (New Zealand) ****
   ************************************/
  { 548,  "01", "Telecom Cook", "Telecom Cook" },

  /***************
   **** Samoa ****
   ***************/
  { 549,  "01", "Digicel",  "Digicel Pacific Ltd." },
  { 549,  "27", "SamoaTel", "SamoaTel Ltd" },

  /********************
   **** Micronesia ****
   ********************/
  { 550,   "01", "FSM Telecom", "FSM Telecom" },

  /***************
   **** Palau ****
   ***************/
  { 552,   "01", "PNCC",         "Palau National Communications Corp." },
  { 552,   "80", "Palau Mobile", "Palau Mobile Corporation" },

  /***************
   **** Egypt ****
   ***************/
  { 602,   "01", "Mobinil",  "ECMS-Mobinil" },
  { 602,   "02", "Vodafone", "Vodafone Egypt" },
  { 602,   "03", "etisalat", "Etisalat Egypt" },

  /*****************
   **** Algeria ****
   *****************/
  { 603,   "01", "Mobilis", "ATM Mobilis" },
  { 603,   "02", "Djezzy", "Orascom Telecom Algerie Spa" },
  { 603,   "03", "Nedjma", "Wataniya Telecom Algerie" },

  /*****************
   **** Morocco ****
   *****************/
  { 604,   "00", "Meditel", "Medi Telecom" },
  { 604,   "01", "IAM",     "Ittissalat Al Maghrib (Maroc Telecom)" },

  /*****************
   **** Tunisia ****
   *****************/
  { 605,   "02", "Tunicell", "Tunisie Telecom" },
  { 605,   "03", "Tunisiana", "Orascom Telecom Tunisie" },

  /***************
   **** Libya ****
   ***************/
  { 606,   "00", "Libyana", "Libyana" },
  { 606,   "01", "Madar",   "Al Madar" },

  /*******************
   **** Mauritius ****
   *******************/
  { 609,   "01", "Mattel",   "Mattel" },
  { 609,  "10", "Mauritel", "Mauritel Mobiles" },

  /**************
   **** Mali ****
   **************/
  { 610,   "01", "Malitel", "Malitel SA" },
  { 610,   "02", "Orange",  "Orange Mali SA" },

  /****************
   **** Guinea ****
   ****************/
  { 611,   "02", "Lagui",          "Sotelgui Lagui" },
  { 611,   "03", "Telecel Guinee", "INTERCEL Guinee" },
  { 611,   "04", "MTN",            "Areeba Guinea" },

  /*********************
   **** Ivory Coast ****
   *********************/
  { 612,   "02", "Moov",   "Moov" },
  { 612,   "03", "Orange", "Orange" },
  { 612,   "04", "KoZ",    "Comium Ivory Coast Inc" },
  { 612,   "05", "MTN",    "MTN" },
  { 612,   "06", "ORICEL", "ORICEL" },

  /**********************
   **** Burkina Faso ****
   **********************/
  { 613,   "01", "Onatel",       "Onatel" },
  { 613,   "02", "Zain",         "Celtel Burkina Faso" },
  { 613,   "03", "Telecel Faso", "Telecel Faso SA" },

  /*****************
   **** Nigeria ****
   *****************/
  { 614,   "01", "SahelCom", "SahelCom" },
  { 614,   "02", "Zain",     "Celtel Niger" },
  { 614,   "03", "Telecel",  "Telecel Niger SA" },
  { 614,   "04", "Orange",   "Orange Niger" },

  /**************
   **** Togo ****
   **************/
  { 615,   "01", "Togo Cell", "Togo Telecom" },
  { 615,   "05", "Telecel",   "Telecel Togo" },

  /***************
   **** Benin ****
   ***************/
  { 616,   "00", "BBCOM",   "Bell Benin Communications" },
  { 616,   "02", "Telecel", "Telecel Benin Ltd" },
  { 616,   "03", "Areeba",  "Spacetel Benin" },

  /*******************
   **** Mauritius ****
   *******************/
  { 617,   "01", "Orange", "Cellplus Mobile Communications Ltd" },
  { 617,  "10", "Emtel",  "Emtel Ltd" },

  /*****************
   **** Liberia ****
   *****************/
  { 618,   "01", "LoneStar Cell", "Lonestar Communications Corporation" },

  /***************
   **** Ghana ****
   ***************/
  { 620,   "01", "MTN",                   "ScanCom Ltd" },
  { 620,   "02", "Ghana Telecomi Mobile", "Ghana Telecommunications Company Ltd" },
  { 620,   "03", "tiGO",                  "Millicom Ghana Limited" },

  /*****************
   **** Nigeria ****
   *****************/
  { 621,  "20", "Zain",  "Celtel Nigeria Ltd." },
  { 621,  "30", "MTN",   "MTN Nigeria Communications Limited" },
  { 621,  "40", "M-Tel", "Nigerian Mobile Telecommunications Limited" },
  { 621,  "50", "Glo",   "Globacom Ltd" },

  /**************
   **** Chad ****
   **************/
  { 622,   "01", "Zain",            "CelTel Tchad SA" },
  { 622,   "03", "TIGO - Millicom", "TIGO - Millicom" },

  /**********************************
   **** Central African Republic ****
   **********************************/
  { 623,   "01", "CTP", "Centrafrique Telecom Plus" },
  { 623,   "02", "TC", "iTelecel Centrafrique" },
  { 623,   "03", "Orange", "Orange RCA" },
  { 623,   "04", "Nationlink", "Nationlink Telecom RCA" },

  /******************
   **** Cameroon ****
   ******************/
  { 624,   "01", "MTN-Cameroon", "Mobile Telephone Network Cameroon Ltd" },
  { 624,   "02", "Orange",       "Orange Cameroun S.A." },

  /********************
   **** Cabo Verde ****
   ********************/
  { 625,   "01", "CMOVEL", "CVMovel, S.A." },

  /*******************************
   **** Sao Tome and Principe ****
   *******************************/
  { 626,   "01", "CSTmovel", "Companhia Santomese de Telecomunicacoe" },

  /**************************
   *** Equatorial Guinea ****
   **************************/
  { 627,   "01", "Orange GQ", "GETESA" },

  /***************
   **** Gabon ****
   ***************/
  { 628,  "01", "Libertis",                  "Libertis S.A." },
  { 628,  "02", "Moov (Telecel) Gabon S.A.", "Moov (Telecel) Gabon S.A." },
  { 628,  "03", "Zain",                      "Celtel Gabon S.A." },

  /*******************************
   **** Republic of the Congo ****
   *******************************/
  { 629,  "10", "Libertis Telecom", "MTN CONGO S.A" },

  /******************************************
   **** Democratic Republic of the Congo ****
   ******************************************/
  { 630,  "01", "Vodacom",      "Vodacom Congo RDC sprl" },
  { 630,  "02", "Zain",         "Celtel Congo" },
  { 630,  "05", "Supercell",    "Supercell SPRL" },
  { 630,  "86", "CCT",          "Congo-Chine Telecom s.a.r.l" },
  { 630,  "89", "SAIT Telecom", "OASIS SPRL" },

  /*****************
   **** Angola ****
   *****************/
  { 631,   "02", "UNITEL", "UNITEL S.a.r.l." },

  /***********************
   **** Guinea-Bissau ****
   ***********************/
  { 632,   "02", "Areeba", "Spacetel Guine-Bissau S.A." },

  /********************
   **** Seychelles ****
   ********************/
  { 633,   "02", "Mdeiatech International", "Mdeiatech International Ltd." },

  /***************
   **** Sudan ****
   ***************/
  { 634,   "01", "Mobitel/Mobile Telephone Company", "Mobitel/Mobile Telephone Company" },
  { 634,   "02", "MTN",                              "MTN Sudan" },

  /****************
   **** Rwanda ****
   ****************/
  { 635,  "10", "MTN", "MTN Rwandacell SARL" },

  /******************
   **** Ethiopia ****
   ******************/
  { 636,   "01", "ETMTN", "Ethiopian Telecommmunications Corporation" },

  /*****************
   **** Somalia ****
   *****************/
  { 637,  "04", "Somafona",       "Somafona FZLLC" },
  { 637,  "10", "Nationalink",    "Nationalink" },
  { 637,  "19", "Hormuud",        "Hormuud Telecom Somalia Inc" },
  { 637,  "30", "Golis",          "Golis Telecommunications Company" },
  { 637,  "62", "Telcom Mobile",  "Telcom Mobile" },
  { 637,  "65", "Telcom Mobile",  "Telcom Mobile" },
  { 637,  "82", "Telcom Somalia", "Telcom Somalia" },

  /******************
   **** Djibouti ****
   ******************/
  { 638,   "01", "Evatis", "Djibouti Telecom SA" },

  /***************
   **** Kenya ****
   ***************/
  { 639,   "02", "Safaricom",    "Safaricom Limited" },
  { 639,   "03", "Zain",         "Celtel Kenya Limited" },
  { 639,   "07", "Orange Kenya", "Telkom Kemya" },

  /******************
   **** Tanzania ****
   ******************/
  { 640,   "02", "Mobitel", "MIC Tanzania Limited" },
  { 640,   "03", "Zantel",  "Zanzibar Telecom Ltd" },
  { 640,   "04", "Vodacom", "Vodacom Tanzania Limited" },

  /****************
   **** Uganda ****
   ****************/
  { 641,  "10", "MTN",                 "MTN Uganda" },
  { 641,  "14", "Orange",              "Orange Uganda" },
  { 641,  "22", "Warid Telecom",       "Warid Telecom" },

  /*****************
   **** Burundi ****
   *****************/
  { 642,   "01", "Spacetel", "Econet Wireless Burundi PLC" },
  { 642,   "02", "Aficell",  "Africell PLC" },
  { 642,   "03", "Telecel",  "Telecel Burundi Company" },

  /********************
   **** Mozambique ****
   ********************/
  { 643,   "01", "mCel",    "Mocambique Celular S.A.R.L." },

  /****************
   **** Zambia ****
   ****************/
  { 645,   "01", "Zain",   "Zain" },
  { 645,   "02", "MTN",    "MTN" },
  { 645,   "03", "ZAMTEL", "ZAMTEL" },

  /********************
   **** Madagascar ****
   ********************/
  { 646,   "01", "Zain",   "Celtel" },
  { 646,   "02", "Orange", "Orange Madagascar S.A." },
  { 646,   "04", "Telma",  "Telma Mobile S.A." },

  /**************************
   **** Reunion (France) ****
   **************************/
  { 647,  "00", "Orange",      "Orange La Reunion" },
  { 647,  "02", "Outremer",    "Outremer Telecom" },
  { 647,  "10", "SFR Reunion", "Societe Reunionnaisei de Radiotelephone" },

  /******************
   **** Zimbabwe ****
   ******************/
  { 648,  "01", "Net*One", "Net*One cellular (Pvt) Ltd" },
  { 648,  "03", "Telecel", "Telecel Zimbabwe (PVT) Ltd" },
  { 648,  "04", "Econet",  "Econet Wireless (Private) Limited" },

  /*****************
   **** Namibia ****
   *****************/
  { 649,  "01", "MTC",      "MTC Namibia" },
  { 649,  "03", "Cell One", "Telecel Globe (Orascom)" },

  /****************
   **** Malawi ****
   ****************/
  { 650,  "01", "TNM",  "Telecom Network Malawi" },
  { 650,  "10", "Zain", "Celtel Limited" },

  /*****************
   **** Lesotho ****
   *****************/
  { 651,  "01", "Vodacom",          "Vodacom Lesotho (Pty) Ltd" },

  /******************
   **** Botswana ****
   ******************/
  { 652,  "01", "Mascom",     "Mascom Wirelessi (Pty) Limited" },
  { 652,  "02", "Orange",     "Orange (Botswans) Pty Limited" },
  { 652,  "04", "BTC Mobile", "Botswana Telecommunications Corporation" },

  /**********************
   **** South Africa ****
   **********************/
  { 655,  "01", "Vodacom",                          "Vodacom" },
  { 655,  "02", "Telkom",                           "Telkom" },
  { 655,  "07", "Cell C",                           "Cell C" },
  { 655,  "10", "MTN",                              "MTN Group" },

  /*****************
   **** Eritrea ****
   *****************/
  { 657,  "01", "Eritel", "Eritel Telecommunications Services Corporation" },

  /****************
   **** Belize ****
   ****************/
  { 702,  "67", "Belize Telemedia",                      "Belize Telemedia" },
  { 702,  "68", "International Telecommunications Ltd.", "International Telecommunications Ltd." },

  /*******************
   **** Guatemala ****
   *******************/
  { 704,  "01", "Claro",         "Servicios de Comunicaciones Personales Inalambricas (SRECOM)" },
  { 704,  "02", "Comcel / Tigo", "Millicom / Local partners" },
  { 704,  "03", "movistar",      "Telefonica Moviles Guatemala (Telefonica)" },

  /*********************
   **** El Salvador ****
   *********************/
  { 706,  "01", "CTE Telecom Personal",  "CTE Telecom Personal SA de CV" },
  { 706,  "02", "digicel",               "Digicel Group" },
  { 706,  "03", "Telemovil EL Salvador", "Telemovil EL Salvador S.A" },
  { 706,  "04", "movistar",              "Telfonica Moviles El Salvador" },
  { 706,  "10", "Claro",                 "America Movil" },

  /******************
   **** Honduras ****
   ******************/
  { 708,  "01", "Claro",          "Servicios de Comunicaciones de Honduras S.A. de C.V." },
  { 708,  "02", "Celtel / Tigo",  "Celtel / Tigo" },
  { 708,  "04", "DIGICEL",        "Digicel de Honduras" },
  { 708,  "30", "Hondutel",       "Empresa Hondurena de telecomunicaciones" },

  /*******************
   **** Nicaragua ****
   *******************/
  { 710,  "21", "Claro",    "Empresa Nicaraguense de Telecomunicaciones,S.A." },
  { 710,  "30", "movistar", "Telefonica Moviles de Nicaragua S.A." },
  { 710,  "73", "SERCOM",   "Servicios de Comunicaciones S.A." },

  /*******************
   **** Cost Rica ****
   *******************/
  { 712,  "01", "ICE", "Instituto Costarricense de Electricidad" },
  { 712,  "02", "ICE", "Instituto Costarricense de Electricidad" },

  /****************
   **** Panama ****
   ****************/
  { 714,  "01", "Cable & Wireless", "Cable & Wireless Panama S.A." },
  { 714,  "02", "movistar",         "Telefonica Moviles Panama S.A" },
  { 714,  "04", "Digicel",          "Digicel (Panama) S.A." },

  /**************
   **** Peru ****
   **************/
  { 716,  "06", "movistar", "Telefonica Moviles Peru" },
  { 716,  "10", "Claro",    "America Movil Peru" },

  /*******************
   **** Argentina ****
   *******************/
  { 722,  "10", "Movistar", "Telefonica Moviles Argentina SA" },
  { 722,  "70", "Movistar", "Telefonica Moviles Argentina SA" },
  { 722, "310", "Claro",    "AMX Argentina S.A" },
  { 722, "320", "Claro",    "AMX Argentina S.A" },
  { 722, "330", "Claro",    "AMX Argentina S.A" },
  { 722, "340", "Personal", "Teecom Personal SA" },

  /****************
   **** Brazil ****
   ****************/
  { 724,  "02", "TIM",                   "Telecom Italia Mobile" },
  { 724,  "03", "TIM",                   "Telecom Italia Mobile" },
  { 724,  "04", "TIM",                   "Telecom Italia Mobile" },
  { 724,  "05", "Claro",                 "Claro (America Movil)" },
  { 724,  "06", "Vivo",                  "Vivo S.A." },
  { 724,  "07", "CTBC Celular",           "CTBC Telecom" },
  { 724,  "08", "TIM",                   "Telecom Italiz Mobile" },
  { 724,  "10", "Vivo",                  "Vivo S.A." },
  { 724,  "11", "Vivo",                  "Vivo S.A." },
  { 724,  "15", "Sercomtel",             "Sercomtel Celular" },
  { 724,  "16", "Oi / Brasil Telecom",   "Brasil Telecom Celular SA" },
  { 724,  "23", "Vivo",                  "Vivo S.A." },
  { 724,  "24", "Oi / Amazonia Celular", "Amazonia Celular S.A." },
  { 724,  "31", "Oi",                    "TNL PCS" },
  { 724,  "37", "aeiou",                 "Unicel do Brasil" },

  /***************
   **** Chile ****
   ***************/
  { 730,  "01", "Entel",    "Entel Pcs" },
  { 730,  "02", "movistar", "Movistar Chile" },
  { 730,  "03", "Claro",    "Claro Chile"},
  { 730,  "10", "Entel",    "Entel Telefonica Movil" },

  /******************
   **** Colombia ****
   ******************/
  { 732, "101", "Comcel",   "Comcel Colombia" },
  { 732, "102", "movistar", "Bellsouth Colombia" },
  { 732, "103", "Tigo",     "Colombia Movil" },
  { 732, "111", "Tigo",     "Colombia Movil" },
  { 732, "123", "movistar", "Telefonica Moviles Colombia" },

  /*******************
   **** Venezuela ****
   *******************/
  { 734,  "01", "Digitel",  "Corporacion Digitel C.A." },
  { 734,  "02", "Digitel",  "Corporacion Digitel C.A." },
  { 734,  "03", "Digitel",  "Corporacion Digitel C.A." },
  { 734,  "04", "movistar", "Telefonica Moviles Venezuela" },
  { 734,  "06", "Movilnet", "Telecommunicaciones Movilnet" },

  /*****************
   **** Bolivia ****
   *****************/
  { 736,  "01", "Nuevatel", "Nuevatel PCS De Bolivia SA" },
  { 736,  "02", "Entel",    "Entel SA" },
  { 736,  "03", "Tigo",     "Telefonica Celular De Bolivia S.A" },

  /****************
   **** Guyana ****
   ****************/
  { 738,  "01", "Digicel", "U-Mobile (Cellular) Inc." },

  /*****************
   **** Ecuador ****
   *****************/
  { 740,  "00", "Movistar", "Otecel S.A." },
  { 740,  "01", "Porta",    "America Movil" },

  /******************
   **** Paraguay ****
   ******************/
  { 744,  "01", "VOX",      "Hola Paraguay S.A." },
  { 744,  "02", "Claro",    "AMX Paraguay S.A." },
  { 744,  "04", "Tigo",     "Telefonica Celular Del Paraguay S.A. (Telecel)" },
  { 744,  "05", "Personal", "Nucleo S.A." },

  /*****************
   **** Uruguay ****
   *****************/
  { 748,  "01", "Ancel",    "Ancel" },
  { 748,  "07", "Movistar", "Telefonica Moviles Uruguay" },
  { 748,  "10", "Claro",    "AM Wireless Uruguay S.A." },

  /*******************
   **** Satellite ****
   *******************/
  { 901,  "01", "ICO",                                          "ICO Satellite Management" },
  { 901,  "02", "Sense Communications International",           "Sense Communications International" },
  { 901,  "03", "Iridium",                                      "Iridium" },
  { 901,  "04", "GlobalStar",                                   "Globalstar" },
  { 901,  "05", "Thuraya RMSS Network",                         "Thuraya RMSS Network" },
  { 901,  "06", "Thuraya Satellite telecommunications Company", "Thuraya Satellite Telecommunications Company" },
  { 901,  "07", "Ellipso",                                      "Ellipso" },
  { 901,  "09", "Tele1 Europe",                                 "Tele1 Europe" },
  { 901,  "10", "ACeS",                                         "ACeS" },
  { 901,  "11", "Immarsat",                                     "Immarsat" },

  /*************
   **** Sea ****
   *************/
  { 901,  "12", "MCP",                                          "Maritime Communications Partner AS" },

  /****************
   **** Ground ****
   ****************/
  { 901,  "13", "GSM.AQ",                                       "GSM.AQ" },

  /*************
   **** Air ****
   *************/
  { 901,  "14", "AeroMobile AS",                                "AeroMobile AS" },
  { 901,  "15", "OnAir Switzerland Sarl",                       "OnAir Switzerland Sarl" },

  /*******************
   **** Satellite ****
   *******************/
  { 901,  "16", "Jasper Systems",                               "Jasper Systems" },
  { 901,  "17", "Navitas",                                      "Navitas" },
  { 901,  "18", "Cingular Wireless",                            "Cingular Wireless" },
  { 901,  "19", "Vodafone Malta Maritime",                      "Vodafone Malta Maritime" }

}; /* qcril_cm_ons_memory_list */


/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/


/*===========================================================================

                                FUNCTIONS

===========================================================================*/

/*=========================================================================
  FUNCTION:  qcril_cm_ons_init

===========================================================================*/
/*!
    @brief
    Initialize the ONS structure of the RIL.

    @return
    E_SUCCESS or E_FAILURE
*/
/*=========================================================================*/
void qcril_cm_ons_init
( 
  void 
)
{ 
  uint8 i;
  qcril_cm_ons_struct_type *i_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "%s", "qcril_cm_ons_init()\n" );

  for ( i = 0; i < QCRIL_MAX_INSTANCE_ID; i++ )
  {
    i_ptr = &qcril_cm_ons[ i ];

    /* Initialize the mutex */
    pthread_mutex_init( &i_ptr->nitz_mutex, NULL );

    /* Initialize NITZ info */
    i_ptr->nitz_available = FALSE;   
  }

} /* qcril_cm_ons_init() */


/*=========================================================================
  FUNCTION: qcril_cm_ons_decode_packed_7bit_gsm_string

===========================================================================*/
/*!
    @brief
    Decode the packed 7-bit GSM string

    @return
    None
    
*/
/*=========================================================================*/
void qcril_cm_ons_decode_packed_7bit_gsm_string
(
  char *dest,
  const uint8 *src,
  uint8 src_length
)
{
  uint16 dest_length = 0;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( dest != NULL );
  QCRIL_ASSERT( src != NULL );

  /*-----------------------------------------------------------------------*/
 
  dest_length = qcril_cm_ss_convert_gsm_def_alpha_string_to_utf8( ( const char * ) src, src_length, dest );

  /* Spare bits is set to '0' as documented in 3GPP TS24.008 Section 10.5.3.5a, and
     the CM util function unpacks it assuming USSD packing (packing for 7 spare bits is carriage return = 0x0D).
     Thus, an '@' is appended when there are 7 spare bits. So remove it. */
  if ( !( src_length % 7 ) && !( src[ src_length - 1 ] & 0xFE ) && ( dest[ dest_length - 1 ] == '@' ) )
  {
    MSG_HIGH( "Detected 7 spare bits in network name, removing trailing @", 0, 0, 0 );
    dest[ dest_length - 1 ] = '\0';
  }

} /* qcril_cm_ons_decode_packed_7bit_gsm_string */


/*=========================================================================
  FUNCTION: qcril_cm_ons_decode_nitz_operator_name

===========================================================================*/
/*!
    @brief
    Decode the NITZ operator name

    @return
    None
    
*/
/*=========================================================================*/
void qcril_cm_ons_decode_nitz_operator_name
(
  char *dest,
  uint8 max_dest_length,
  sys_network_name_coding_scheme_type_e_type coding_scheme,
  const uint8 *src,
  uint8 src_length
)
{
  uint8 data_length;
  char *temp_buf;
  uint8 i;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( dest != NULL );
  QCRIL_ASSERT( src != NULL );

  /*-----------------------------------------------------------------------*/

  data_length = ( src_length > max_dest_length ) ? max_dest_length : src_length;

  switch ( coding_scheme )
  {
    case SYS_NW_NAME_CODING_SCHEME_CELL_BROADCAST_DEFAULT_ALPHABET:
      QCRIL_LOG_INFO( "%s", "7-bit coding scheme for NITZ ONS\n" );
      qcril_cm_ons_decode_packed_7bit_gsm_string( dest, src, data_length );
      QCRIL_LOG_DEBUG( "NITZ 7-bit GSM str: %s\n", dest );
      break;

    case SYS_NW_NAME_CODING_SCHEME_UCS2:
      QCRIL_LOG_INFO( "UC2 coding scheme for NITZ ONS, len %d\n", data_length );
      if ( ( data_length % 2 ) != 0 )
      {
        QCRIL_LOG_ERROR( "Invalid UCS length %d\n", data_length );
        return;
      }

      temp_buf = (char *) qcril_malloc( data_length );
      if ( temp_buf == NULL )
      {
        QCRIL_LOG_ERROR( "%s\n", "Fail to allocate buffer for decoding UCS2" );
        return;
      }

      for ( i = 0; i < data_length; i = i + 2 )
      {
        temp_buf[ i ] = src[ i + 1 ];
        temp_buf[ i + 1 ] = src[ i ];
      }

      (void) qcril_cm_ss_convert_ucs2_to_utf8( temp_buf, data_length, dest );
      QCRIL_LOG_DEBUG( "NITZ UCS str: %s\n", dest );

      qcril_free( temp_buf );
      break;

    default:
      QCRIL_LOG_INFO( "Unknown coding scheme %d for NITZ ONS\n", coding_scheme );
      break;
  }

} /* qcril_cm_ons_decode_nitz_operator_name */


/*=========================================================================
  FUNCTION:  qcril_cm_ons_match_plmn

===========================================================================*/
/*!
    @brief
    Compares two PLMN IDs and determines if they are equal.
    If the third MNC digit in the plmn_2 is 0xF, then the plmn_2
    is assumed to contain a two digit MNC so the third MNC digit is not compared.

    @return
    TRUE if the two PLMN IDs are equal. Otherwise, FALSE.
*/
/*=========================================================================*/
boolean qcril_cm_ons_match_plmn
(
  sys_plmn_id_s_type  plmn_1,
  sys_plmn_id_s_type  plmn_2
)
{
  boolean result = FALSE;
  uint32 plmn1_mcc_digit_1  = plmn_1.identity[0] & 0x0F;
  uint32 plmn1_mcc_digit_2  = plmn_1.identity[0] / 0x10;
  uint32 plmn1_mcc_digit_3  = plmn_1.identity[1] & 0x0F;

  uint32 plmn1_mnc_digit_1  = plmn_1.identity[2] & 0x0F;
  uint32 plmn1_mnc_digit_2  = plmn_1.identity[2] / 0x10;
  uint32 plmn1_mnc_digit_3  = plmn_1.identity[1] / 0x10;

  uint32 plmn2_mcc_digit_1 = plmn_2.identity[0] & 0x0F;
  uint32 plmn2_mcc_digit_2 = plmn_2.identity[0] / 0x10;
  uint32 plmn2_mcc_digit_3 = plmn_2.identity[1] & 0x0F;

  uint32 plmn2_mnc_digit_1 = plmn_2.identity[2] & 0x0F;
  uint32 plmn2_mnc_digit_2 = plmn_2.identity[2] / 0x10;
  uint32 plmn2_mnc_digit_3 = plmn_2.identity[1] / 0x10;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "plmn1 mcc = %d %d %d, mnc = %d %d %d",
                   plmn1_mcc_digit_1, plmn1_mcc_digit_2, plmn1_mcc_digit_3,
                   plmn1_mnc_digit_1, plmn1_mnc_digit_2, plmn1_mnc_digit_3 );

  QCRIL_LOG_DEBUG( "plmn2 mcc = %d %d %d, mnc = %d %d %d",
                   plmn2_mcc_digit_1, plmn2_mcc_digit_2, plmn2_mcc_digit_3,
                   plmn2_mnc_digit_1, plmn2_mnc_digit_2, plmn2_mnc_digit_3 );

  if ( ( plmn1_mcc_digit_1 == plmn2_mcc_digit_1 ) &&
       ( plmn1_mcc_digit_2 == plmn2_mcc_digit_2 ) &&
       ( plmn1_mcc_digit_3 == plmn2_mcc_digit_3 ) )
  {
    if ( plmn2_mnc_digit_3 == 0xF )
    {
      if ( ( plmn2_mcc_digit_1 == 3 ) &&
           ( plmn2_mcc_digit_2 == 1 ) &&
           ( plmn2_mcc_digit_3 <= 6 ) )
      {
        if ( ( plmn1_mnc_digit_3 == 0 ) || ( plmn1_mnc_digit_3 == 0xF ) )
        {
          result = ( boolean )( ( plmn1_mnc_digit_1 == plmn2_mnc_digit_1 ) &&
                                ( plmn1_mnc_digit_2 == plmn2_mnc_digit_2 ) );
        }
      }
      else
      {
        result = ( boolean )( ( plmn1_mnc_digit_1 == plmn2_mnc_digit_1 ) &&
                              ( plmn1_mnc_digit_2 == plmn2_mnc_digit_2 ) &&
                              ( plmn1_mnc_digit_3 == plmn2_mnc_digit_3 ) );
      }
    }
    else
    {
      if ( ( plmn2_mcc_digit_1 == 3 ) &&
           ( plmn2_mcc_digit_2 == 1 ) &&
           ( plmn2_mcc_digit_3 <= 6 ) &&
           ( plmn2_mnc_digit_3 == 0 ) &&
           ( plmn1_mnc_digit_3 == 0xF ) )
      {
        result = ( boolean )( ( plmn1_mnc_digit_1 == plmn2_mnc_digit_1 ) &&
                              ( plmn1_mnc_digit_2 == plmn2_mnc_digit_2 ) );        
      }
      else
      {
        result = ( boolean )( ( plmn1_mnc_digit_1 == plmn2_mnc_digit_1 ) &&
                              ( plmn1_mnc_digit_2 == plmn2_mnc_digit_2 ) &&
                              ( plmn1_mnc_digit_3 == plmn2_mnc_digit_3 ) );
      }
    }
  }

  return result;

} /* qcril_cm_ons_match_plmn */


/*=========================================================================
  FUNCTION:  qcril_cm_ons_convert_mcc_mnc_to_ascii

===========================================================================*/
/*!
    @brief
    Convert the MCCMNC from BCD to ASCII

    @return
    None.
*/
/*=========================================================================*/
void qcril_cm_ons_convert_mcc_mnc_to_ascii
(
  const sys_plmn_id_s_type plmn,
  char mcc_mnc_ascii[]
)
{
  uint8 mcc_digit_1, mcc_digit_2, mcc_digit_3;
  uint8 mnc_digit_1, mnc_digit_2, mnc_digit_3;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( mcc_mnc_ascii != NULL ); 

  /*-----------------------------------------------------------------------*/

  /* Check for wildcard MCC and MNC */
  if ( ( plmn.identity[ 0 ] == 0xFF ) && ( plmn.identity[ 1 ] == 0xFF ) && ( plmn.identity[ 2 ] == 0xFF ) )
  {
    mcc_mnc_ascii[ 0 ] = '0';
    mcc_mnc_ascii[ 1 ] = '\0';
    QCRIL_LOG_DEBUG( "%s", "Wildcard MccMnc\n" ); 
  }
  else
  {
    /* Extract the mcc and mnc from the PLMN. */
    mcc_digit_1 = plmn.identity[ 0 ] & 0x0F;
    mcc_digit_2 = plmn.identity[ 0 ] / 0x10;
    mcc_digit_3 = plmn.identity[ 1 ] & 0x0F;

    mnc_digit_1 = plmn.identity[ 2 ] & 0x0F;
    mnc_digit_2 = plmn.identity[ 2 ] / 0x10;
    mnc_digit_3 = plmn.identity[ 1 ] / 0x10;

    mcc_mnc_ascii[ 0 ] = '0' + mcc_digit_1;
    mcc_mnc_ascii[ 1 ] = '0' + mcc_digit_2;
    mcc_mnc_ascii[ 2 ] = '0' + mcc_digit_3;
    mcc_mnc_ascii[ 3 ] = '0' + mnc_digit_1;
    mcc_mnc_ascii[ 4 ] = '0' + mnc_digit_2;
    if ( mnc_digit_3 == 0xF )
    {
      mcc_mnc_ascii[ 5 ] = '\0';
    }
    else
    {
      mcc_mnc_ascii[ 5 ] = '0' + mnc_digit_3;
      mcc_mnc_ascii[ 6 ] = '\0';
    }

    QCRIL_LOG_INFO( "PLMN[0] = %d, PLMN[1] = %d, PLMN[2] = %d\n", 
                    plmn.identity[ 0 ], plmn.identity[ 1 ], plmn.identity[ 2 ] ); 
    QCRIL_LOG_INFO( "MCC digit1 = %d, MCC digit2 = %d, MCC digit3 = %d\n", mcc_digit_1, mcc_digit_2, mcc_digit_3 ); 
    QCRIL_LOG_INFO( "MNC digit1 = %d, MNC digit2 = %d, MNC digit3 = %d\n", mnc_digit_1, mnc_digit_2, mnc_digit_3 ); 
  }

} /* qcril_cm_ons_convert_mcc_mnc_to_ascii */


/*=========================================================================
  FUNCTION:  qcril_cm_ons_convert_mcc_mnc_to_bcd

===========================================================================*/
/*!
    @brief
    Convert the MCCMNC from ASCII to BCD format

    @return
    None.
*/
/*=========================================================================*/
void qcril_cm_ons_convert_mcc_mnc_to_bcd
(
  const char *ascii_mcc_mnc,
  uint8 len,
  sys_plmn_id_s_type *plmn_ptr
)
{
  uint8 upper_digit, lower_digit;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( ascii_mcc_mnc != NULL );

  /*-----------------------------------------------------------------------*/

  /* Wildcard MccMnc */
  if ( strcmp( ascii_mcc_mnc, "0" ) == 0 )
  {
    QCRIL_LOG_DEBUG( "%s", "Selected wildcard MccMnc\n" ); 
    plmn_ptr->identity[ 0 ] = 0xFF;  
    plmn_ptr->identity[ 1 ] = 0xFF;  
    plmn_ptr->identity[ 2 ] = 0xFF;  
  }
  else
  {
    lower_digit = ascii_mcc_mnc[ 0 ] - '0';
    upper_digit = ascii_mcc_mnc[ 1 ] - '0';
    plmn_ptr->identity[ 0 ] = ( upper_digit << 4 ) | lower_digit;  

    lower_digit = ascii_mcc_mnc[ 2 ] - '0';
    if ( len == 5 )
    {
      upper_digit = 0xf;  
    }
    else
    {
      upper_digit = ascii_mcc_mnc[ 5 ] - '0';
    }

    plmn_ptr->identity[ 1 ] = ( upper_digit << 4 ) | lower_digit;  

    lower_digit = ascii_mcc_mnc[ 3 ] - '0';
    upper_digit = ascii_mcc_mnc[ 4 ] - '0';
    plmn_ptr->identity[ 2 ] = ( upper_digit << 4 ) | lower_digit;  
  }

} /* qcril_cm_ons_convert_mcc_mnc_to_bcd */


/*=========================================================================
  FUNCTION:  qcril_cm_ons_read_nitz_ons

===========================================================================*/
/*!
    @brief
    Read saved NITZ's ONS from system property.

    @return
    None
    
*/
/*=========================================================================*/
void qcril_cm_ons_read_nitz_ons
(
  qcril_instance_id_e_type instance_id,
  char *ons_ptr,
  const char **ons_property_list_ptr
)
{
  char args[ PROPERTY_VALUE_MAX ];
  uint8 i;
  int ons_len, read_len = 0;
  char property_name[ 40 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( ons_ptr != NULL );
  QCRIL_ASSERT( ons_property_list_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  *ons_ptr = '\0';
  for ( i = 0; i < QCRIL_ARR_SIZE( ons_property_list_ptr ); i++ )
  {
    QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s%d", *ons_property_list_ptr, instance_id );

    property_get( property_name, args, "" );
    ons_len = strlen( args );
    if ( ons_len > 0 )
    {                                                                                       
      read_len += ons_len;
      if ( read_len >= QCRIL_CM_ONS_MAX_LENGTH )
      {
        QCRIL_LOG_ERROR( "ONS length %d >= %d ( system property %s )\n", read_len, QCRIL_CM_ONS_MAX_LENGTH, property_name );
        break;
      }

      memcpy( ons_ptr, args, strlen( args ) );
      ons_ptr += ons_len;
      *ons_ptr = '\0';
    }

    ons_property_list_ptr++;

  } /* end for */

} /* qcril_cm_ons_read_nitz_ons */


/*=========================================================================
  FUNCTION:  qcril_cm_ons_write_nitz_ons

===========================================================================*/
/*!
    @brief
    Write NITZ's ONS to system property.

    @return
    None
    
*/
/*=========================================================================*/
void qcril_cm_ons_write_nitz_ons
(
  qcril_instance_id_e_type instance_id,
  char *ons_ptr,
  const char **ons_property_list_ptr
)
{
  char args[ PROPERTY_VALUE_MAX ];
  uint8 i;
  int ons_len, saved_len, rem_len, property_len;
  char property_name[ 40 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( ons_ptr != NULL );
  QCRIL_ASSERT( ons_property_list_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  ons_len = strlen( ons_ptr );
  for ( i = 0, saved_len = 0; i < QCRIL_ARR_SIZE( ons_property_list_ptr ); i++ )
  {
    args[ 0 ] = '\0';
    rem_len = ons_len - saved_len;
    if ( rem_len != 0 ) 
    {
      if ( rem_len >= PROPERTY_VALUE_MAX )
      {
        property_len = PROPERTY_VALUE_MAX - 1;
      }
      else
      {
        property_len = rem_len;
      }

      memcpy( args, ons_ptr, property_len );
      args[ property_len ] = '\0';
      ons_ptr += property_len;
      saved_len += property_len;
    }

    QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s%d", *ons_property_list_ptr, instance_id );

    if ( property_set( property_name, args ) != E_SUCCESS )
    {
      QCRIL_LOG_ERROR( "Fail to save %s to system property\n", property_name );
    }                                                                                
    else
    {
      QCRIL_LOG_DEBUG( "Save %s to system property: %s\n", property_name, args );
    }

    ons_property_list_ptr++;

  } /* end for */

} /* qcril_cm_ons_write_nitz_ons */


/*=========================================================================
  FUNCTION:  qcril_cm_ons_store_nitz

===========================================================================*/
/*!
    @brief
    Save ONS from NITZ.

    @return
    None
    
*/
/*=========================================================================*/
void qcril_cm_ons_store_nitz
(                                     
  qcril_instance_id_e_type instance_id,
  const qcril_cm_ss_info_type *ss_info_ptr
)
{
  char property_name[ 40 ];
  qcril_cm_ons_struct_type *i_ptr;
  char mcc_mnc_ascii[ QCRIL_CM_ONS_MCC_MNC_ASCII_MAX_LEN ]; 
  char details[ 40 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_cm_ons[ instance_id ];
  QCRIL_ASSERT( ss_info_ptr != NULL ); 

  /*-----------------------------------------------------------------------*/

  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_cm_ons[%d].nitz_mutex", instance_id );

  QCRIL_MUTEX_LOCK( &i_ptr->nitz_mutex, details );

  QCRIL_LOG_DEBUG( "RID %d Save NITZ info\n", instance_id );

  /* NITZ is optional info even though current operator is GW */
  i_ptr->nitz_available = FALSE;

  /* Operator name can be available from MM Info while in GW limited or full service */
  if ( QCRIL_CM_SRV_STATUS_INDICATES_GW_SRV_AVAILABLE( ss_info_ptr->srv_status, ss_info_ptr->sys_mode ) )
  {
    /* If coding scheme is UCS2 or GSM 7-bit coding with the full or short name is not equal to NULL, decode ONS from NITZ  */
    if ( ( ss_info_ptr->mode_info.gw_info.mm_information.full_name_avail && 
           ( ss_info_ptr->mode_info.gw_info.mm_information.full_name.length > 0 ) && 
           ( ( ss_info_ptr->mode_info.gw_info.mm_information.full_name.coding_scheme !=
                SYS_NW_NAME_CODING_SCHEME_CELL_BROADCAST_DEFAULT_ALPHABET ) ||
             ( ss_info_ptr->mode_info.gw_info.mm_information.full_name.name[ 0 ] != '\0' ) ) ) || 
         ( ss_info_ptr->mode_info.gw_info.mm_information.short_name_avail &&
           ( ss_info_ptr->mode_info.gw_info.mm_information.short_name.length > 0 ) && 
           ( ( ss_info_ptr->mode_info.gw_info.mm_information.short_name.coding_scheme !=
               SYS_NW_NAME_CODING_SCHEME_CELL_BROADCAST_DEFAULT_ALPHABET ) ||
             ( ss_info_ptr->mode_info.gw_info.mm_information.short_name.name[ 0 ] != '\0' ) ) ) )
    {
      QCRIL_LOG_INFO( "%s", "Store NITZ's ONS info\n" );

      i_ptr->nitz_available = TRUE;

      i_ptr->nitz_long_ons[ 0 ] = '\0';
      i_ptr->nitz_short_ons[ 0 ] = '\0';

      /* Decode long ONS */
      if ( ss_info_ptr->mode_info.gw_info.mm_information.full_name_avail )
      {
        qcril_cm_ons_decode_nitz_operator_name( i_ptr->nitz_long_ons, 
                                                (uint8) QCRIL_CM_ONS_MAX_LENGTH,
                                                ss_info_ptr->mode_info.gw_info.mm_information.full_name.coding_scheme, 
                                                ss_info_ptr->mode_info.gw_info.mm_information.full_name.name,
                                                (uint8) ss_info_ptr->mode_info.gw_info.mm_information.full_name.length );
      }

      /* Decode short ONS */
      if ( ss_info_ptr->mode_info.gw_info.mm_information.short_name_avail )
      {
        qcril_cm_ons_decode_nitz_operator_name( i_ptr->nitz_short_ons,
                                                (uint8) QCRIL_CM_ONS_MAX_LENGTH,
                                                ss_info_ptr->mode_info.gw_info.mm_information.short_name.coding_scheme, 
                                                ss_info_ptr->mode_info.gw_info.mm_information.short_name.name,
                                                (uint8) ss_info_ptr->mode_info.gw_info.mm_information.short_name.length );
      }

      QCRIL_LOG_DEBUG( "RID %d write received NITZ's info to system property\n", instance_id );

      /* Save NITZ's PLMN to system property */
      qcril_cm_ons_convert_mcc_mnc_to_ascii( ss_info_ptr->mode_info.gw_info.mm_information.plmn, mcc_mnc_ascii );
      QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s%d", QCRIL_CM_ONS_NITZ_PLMN, instance_id );
      if ( property_set( property_name, mcc_mnc_ascii ) != E_SUCCESS )
      {
        QCRIL_LOG_ERROR( "RID %d Fail to save NITZ PLMN %s to system property %s\n", instance_id, mcc_mnc_ascii, property_name );
      }                                                                                
      else
      {
        QCRIL_LOG_DEBUG( "RID %d Save NITZ PLMN %s to system property %s\n", instance_id, mcc_mnc_ascii, property_name );
      }

      /* Save NITZ's LONG ONS to system property */
      qcril_cm_ons_write_nitz_ons( instance_id, i_ptr->nitz_long_ons, nitz_long_ons_property_list );

      /* Save NITZ's SHORT ONS to system property */
      qcril_cm_ons_write_nitz_ons( instance_id, i_ptr->nitz_short_ons, nitz_short_ons_property_list );

    }   
  }

  QCRIL_MUTEX_UNLOCK( &i_ptr->nitz_mutex, details );

} /* qcril_cm_ons_store_nitz */


/*=========================================================================
  FUNCTION:  qcril_cm_ons_lookup_nitz

===========================================================================*/
/*!
    @brief
    Lookup current operator name from NITZ.

    @return
    E_SUCCESS if NITZ ONS available. Otherwise, E_FAILURE.
    
*/
/*=========================================================================*/
IxErrnoType qcril_cm_ons_lookup_nitz
(
  qcril_instance_id_e_type instance_id,
  const qcril_cm_ss_info_type *ss_info_ptr,
  char **long_ons_ptr,
  char **short_ons_ptr
)
{
  qcril_cm_ons_struct_type *i_ptr;
  IxErrnoType status = E_FAILURE;
  sys_plmn_id_s_type plmn;
  char args[ PROPERTY_VALUE_MAX ];
  char property_name[ 40 ];
  char details[ 40 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_cm_ons[ instance_id ];
  QCRIL_ASSERT( ss_info_ptr != NULL );
  QCRIL_ASSERT( long_ons_ptr != NULL );
  QCRIL_ASSERT( short_ons_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_cm_ons[%d].nitz_mutex", instance_id );

  QCRIL_MUTEX_LOCK( &i_ptr->nitz_mutex, details );

  QCRIL_LOG_DEBUG( "%s", "Lookup NITZ info\n" );

  /* NITZ received from the current operator */
  if ( i_ptr->nitz_available )
  {
    status = E_SUCCESS;
    QCRIL_SNPRINTF( *long_ons_ptr, QCRIL_CM_ONS_MAX_LENGTH, "%s", i_ptr->nitz_long_ons );
    QCRIL_SNPRINTF( *short_ons_ptr, QCRIL_CM_ONS_MAX_LENGTH, "%s", i_ptr->nitz_short_ons );

    QCRIL_LOG_INFO( "%s", "ONS from received NITZ\n" );
  }
  /* Lookup saved NITZ info from system property if available */
  else
  {
    /* Read saved NITZ's PLMN from system property */
    QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s%d", QCRIL_CM_ONS_NITZ_PLMN, instance_id );
    property_get( property_name, args, "" );
    if ( args[ 0 ] != '\0' )
    {
      /* Current operator's PLMN matches saved NITZ's PLMN */
      qcril_cm_ons_convert_mcc_mnc_to_bcd( args, strlen( args ), &plmn );
      if ( qcril_cm_ons_match_plmn( ss_info_ptr->sys_id.id.plmn_lac.plmn, plmn ) )
      {                  
        /* Read saved NITZ's long ONS and short ONS from system property */
        qcril_cm_ons_read_nitz_ons( instance_id, i_ptr->nitz_long_ons, nitz_long_ons_property_list );
        qcril_cm_ons_read_nitz_ons( instance_id, i_ptr->nitz_short_ons, nitz_short_ons_property_list );

        status = E_SUCCESS;
        QCRIL_SNPRINTF( *long_ons_ptr, QCRIL_CM_ONS_MAX_LENGTH, "%s", i_ptr->nitz_long_ons );
        QCRIL_SNPRINTF( *short_ons_ptr, QCRIL_CM_ONS_MAX_LENGTH, "%s", i_ptr->nitz_short_ons );
        QCRIL_LOG_DEBUG( "%s", "ONS from saved NITZ\n" );
      }
    }
  }

  QCRIL_MUTEX_UNLOCK( &i_ptr->nitz_mutex, details );

  return status;

} /* qcril_cm_ons_lookup_nitz */


/*=========================================================================
  FUNCTION:  qcril_cm_ons_lookup_memory_list

===========================================================================*/
/*!
    @brief
    Lookup the operator names from UE memory list.

    @return
    None
*/
/*=========================================================================*/
void qcril_cm_ons_lookup_memory_list
(
  const sys_plmn_id_s_type plmn,
  char **long_ons_ptr,
  char **short_ons_ptr,
  char **mcc_mnc_ptr
)
{
  const int number_of_entries = sizeof( qcril_cm_ons_memory_list ) / sizeof( qcril_cm_ons_memory_entry_type );
  int i = 0;
  boolean continue_search = TRUE;
  uint32 mcc_digit_1, mcc_digit_2, mcc_digit_3, mcc;
  uint32 mnc_digit_1, mnc_digit_2, mnc_digit_3, mnc;
  const qcril_cm_ons_memory_entry_type *ons_mem_ptr = NULL;
  char mnc_str[4];
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( long_ons_ptr != NULL );
  QCRIL_ASSERT( short_ons_ptr != NULL );
  QCRIL_ASSERT( mcc_mnc_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Extract the mcc and mnc from the PLMN. */
  mcc_digit_1 = plmn.identity[0] & 0x0F;
  mcc_digit_2 = plmn.identity[0] / 0x10;
  mcc_digit_3 = plmn.identity[1] & 0x0F;

  mnc_digit_1 = plmn.identity[2] & 0x0F;
  mnc_digit_2 = plmn.identity[2] / 0x10;
  mnc_digit_3 = plmn.identity[1] / 0x10;

  mcc = 100 * mcc_digit_1 + 10 * mcc_digit_2 + mcc_digit_3;

  if ( mnc_digit_3 == 0xF )
  {
    mnc = 10 * mnc_digit_1 + mnc_digit_2;
  }
  else
  {
    mnc = 100 * mnc_digit_1 + 10 * mnc_digit_2 + mnc_digit_3;
  }

  /* convert mnc to string to differenciate between two digit and three digit mnc
     i.e. "02" and "002" */
  if( mnc_digit_3 == 0xF )
  {
    QCRIL_SNPRINTF(mnc_str, 03, "%02d", (int)mnc );
  }
  else
  {
    QCRIL_SNPRINTF(mnc_str, 04, "%03d", (int)mnc );
  }

  QCRIL_LOG_DEBUG("%s -- mnc = %s", __FUNCTION__, mnc_str);

  /* Search the table for the MCC and MNC */
  while ( continue_search && ( i < number_of_entries ) )
  {
    if ( mcc == qcril_cm_ons_memory_list[ i ].mcc )
    {
      if ( strcmp( mnc_str, qcril_cm_ons_memory_list[ i ].mnc ) == 0 )
      {
        ons_mem_ptr = &qcril_cm_ons_memory_list[ i ];
        continue_search = FALSE;
      }
      else if ( atoi(mnc_str) < atoi(qcril_cm_ons_memory_list[ i ].mnc) )
      {
        /*
        ** Terminate the search because the MNCs are stored in ascending
        ** order in the table and the MNC being searched is less than the
        ** current MNC in the table.
        */
        continue_search = FALSE;
      }
    }

    else if ( mcc < qcril_cm_ons_memory_list[ i ].mcc )
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
    /* WildCard PLMN */
    if ( ( plmn.identity[ 0 ] == 0xFF ) && ( plmn.identity[ 1 ] == 0xFF ) && ( plmn.identity[ 2 ] == 0xFF ) )
    {
      *long_ons_ptr = "";
    }
    else
    {
      /* Use MCC+MNC as long ONS */
      *long_ons_ptr = *mcc_mnc_ptr;
    }

    /* Short ONS is not available */
    *short_ons_ptr = "";
    QCRIL_LOG_DEBUG( "%s", "ONS info from MCCMNC\n" );
  }
  else
  {
    *long_ons_ptr = ons_mem_ptr->full_name_ptr;
    *short_ons_ptr = ons_mem_ptr->short_name_ptr;
    QCRIL_LOG_DEBUG( "%s", "ONS info from UE Memory List\n" );
  }

} /* qcril_cm_ons_lookup_memory_list */   


/*=========================================================================
  FUNCTION:  qcril_cm_ons_lookup_current_operator

===========================================================================*/
/*!
    @brief
    Lookup the current operator names from NITZ or memory list.

    @return
    None
*/
/*=========================================================================*/
void qcril_cm_ons_lookup_current_operator
(
  qcril_instance_id_e_type instance_id,
  const qcril_cm_ss_info_type *ss_info_ptr,
  char **long_ons_ptr,
  char **short_ons_ptr,
  char **mcc_mnc_ptr
)
{
  IxErrnoType status = E_FAILURE;
  nv_stat_enum_type nv_status = NV_FAIL_S;
  nv_item_type nv_item;
  uint32 len;
  uint16 mcc, tmp, dig;
  uint8 i, pwr;
  uint8 mnc;
  qcril_modem_id_e_type modem_id, cdma_modem_id, evdo_modem_id, gwl_modem_id; 

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_as_id_e_type as_id = SYS_MODEM_AS_ID_NONE;
  char *nv_api_name = "nv_cmd_ext_remote()";
  #else
  char *nv_api_name = "nv_cmd_remote()";
  #endif /* FEATURE_QCRIL_DSDS */

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( ss_info_ptr != NULL );
  QCRIL_ASSERT( long_ons_ptr != NULL );
  QCRIL_ASSERT( short_ons_ptr != NULL );
  QCRIL_ASSERT( mcc_mnc_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* If GW_service is available, lookup the current operator name */
  if ( QCRIL_CM_SRV_STATUS_INDICATES_GW_FULL_SRV( ss_info_ptr->srv_status, ss_info_ptr->sys_mode ) )
  {
    /* Convert MCC+MNC to ASCII format */
    qcril_cm_ons_convert_mcc_mnc_to_ascii( ss_info_ptr->sys_id.id.plmn_lac.plmn, *mcc_mnc_ptr );

    /* Lookup current operator name from NITZ */
    status = qcril_cm_ons_lookup_nitz( instance_id, ss_info_ptr, long_ons_ptr, short_ons_ptr );

    if ( status != E_SUCCESS )
    {
      /* Lookup current operator name from UE memory list */
      qcril_cm_ons_lookup_memory_list( ss_info_ptr->sys_id.id.plmn_lac.plmn, long_ons_ptr, short_ons_ptr, mcc_mnc_ptr );
      status = E_SUCCESS;
    }
  }
  /* Current operator is 1xEvDo */
  else if ( QCRIL_CM_SRV_STATUS_INDICATES_1XEVDO_FULL_SRV( ss_info_ptr->srv_status, ss_info_ptr->sys_mode,
                                                           ss_info_ptr->hdr_hybrid, ss_info_ptr->hdr_srv_status ) )
  {
    status = E_SUCCESS;

    *short_ons_ptr = "";

    if ( QCRIL_CM_SYS_MODE_IS_CDMA( ss_info_ptr->sys_mode ) )
    {
      /* Wildcard MCC */
      if ( ss_info_ptr->sys_id.id.is95.mcc >= 1023 )
      {
        mcc = 1023;
      }
      else
      {
        mcc = 0;
        tmp = ss_info_ptr->sys_id.id.is95.mcc + 111;
        pwr = 1;

        for( i = 0; i < 3; i++ )
        {
          dig = tmp % 10;
          mcc += (pwr * dig);

          if (dig == 0)
          {
            tmp -= 10;
          }
          tmp /= 10;
          pwr *= 10;
        }
      }

      /* Wildcard MNC */
      if ( ss_info_ptr->sys_id.id.is95.imsi_11_12 >= 127 )
      {
        mnc = 127;
      }
      else
      {
        mnc = ss_info_ptr->sys_id.id.is95.imsi_11_12;   
      }

      if( mcc == 1023 )
      {
        /* Convert MCC+MNC to ASCII format */
        len = QCRIL_SNPRINTF( *mcc_mnc_ptr, QCRIL_CM_ONS_MCC_MNC_ASCII_MAX_LEN, "%04d%03d", mcc, mnc ); 
        QCRIL_ASSERT( len <= QCRIL_CM_ONS_MCC_MNC_ASCII_MAX_LEN );
      }
      else
      {
        /* Convert MCC+MNC to ASCII format */
        len = QCRIL_SNPRINTF( *mcc_mnc_ptr, QCRIL_CM_ONS_MCC_MNC_ASCII_MAX_LEN, "%03d%03d", mcc, mnc ); 
        QCRIL_ASSERT( len <= QCRIL_CM_ONS_MCC_MNC_ASCII_MAX_LEN );
      }
    }
    else
    {
      *mcc_mnc_ptr = "";
    }

    if ( QCRIL_CM_SYS_MODE_IS_CDMA( ss_info_ptr->sys_mode ) )
    {
      if( mcc == 1023 )
      {
        /* Use MCC+MNC as long ONS */
        len = QCRIL_SNPRINTF( *long_ons_ptr, QCRIL_CM_ONS_MCC_MNC_ASCII_MAX_LEN, "%04d%03d", mcc, mnc ); 
        QCRIL_ASSERT( len <= QCRIL_CM_ONS_MCC_MNC_ASCII_MAX_LEN );
      }
      else
      {
        /* Use MCC+MNC as long ONS */
        len = QCRIL_SNPRINTF( *long_ons_ptr, QCRIL_CM_ONS_MCC_MNC_ASCII_MAX_LEN, "%03d%03d", mcc, mnc ); 
        QCRIL_ASSERT( len <= QCRIL_CM_ONS_MCC_MNC_ASCII_MAX_LEN );
      }
    }
    else
    {
      **long_ons_ptr = '\0';
    }

    /* Lookup the modem that should be requested for NV service */ 
    qcril_arb_query_arch_modem_id( &cdma_modem_id, &evdo_modem_id, &gwl_modem_id );
    
    modem_id = cdma_modem_id;
    QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

    #ifdef FEATURE_QCRIL_DSDS
    /* Lookup as_id */
    (void) qcril_arb_lookup_as_id_from_instance_id( instance_id, &as_id );
    #endif /* FEATURE_QCRIL_DSDS */

    /* Lookup long operator name from NAM */
    nv_item.name_nam.nam = 0;
    QCRIL_LOG_RPC2A( modem_id, nv_api_name, "Read NV_NAME_NAM_I" );
    nv_status = qcril_other_api_funcs[ modem_id ].nv_cmd_remote_func( NV_READ_F, NV_NAME_NAM_I, (nv_item_type *) &nv_item
                                                                      #ifdef FEATURE_QCRIL_DSDS
                                                                      , as_id
                                                                      #endif /* FEATURE_QCRIL_DSDS */
                                                                    );
    if ( nv_status == NV_DONE_S ) 
    {
      len = QCRIL_SNPRINTF( *long_ons_ptr, QCRIL_CM_ONS_MAX_LENGTH, "%s", nv_item.name_nam.name ); 
      QCRIL_ASSERT( len <= QCRIL_CM_ONS_MAX_LENGTH );
      QCRIL_LOG_DEBUG( "NAM name %s read from NV\n", *long_ons_ptr );
    }
    else if ( nv_status == NV_NOTACTIVE_S )
    {
      QCRIL_LOG_DEBUG( "%s", "NAM not programmed in NV\n" );
    }
    else
    {
      QCRIL_LOG_DEBUG( "Error %d reading NAM from NV\n", nv_status );
    }
  }
  else
  {
    QCRIL_LOG_INFO( "Current operator not available: system mode %d ( srv status %d )\n", ss_info_ptr->sys_mode, 
                    ss_info_ptr->srv_status ); 
    *long_ons_ptr = NULL;
    *short_ons_ptr = NULL;
    *mcc_mnc_ptr = NULL;
  }

} /* qcril_cm_ons_lookup_current_operator */


/*=========================================================================
  FUNCTION:  qcril_cm_ons_lookup_available_operator

===========================================================================*/
/*!
    @brief
    Lookup the available operator names from NITZ or memory list.

    @return
    TRUE - current operator. Otherwise, FALSE.
*/
/*=========================================================================*/
boolean qcril_cm_ons_lookup_available_operator
(
  qcril_instance_id_e_type instance_id,
  const qcril_cm_ss_info_type *ss_info_ptr,
  const sys_plmn_id_s_type plmn,
  char **long_ons_ptr,
  char **short_ons_ptr,
  char **mcc_mnc_ptr
)
{

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( ss_info_ptr != NULL );
  QCRIL_ASSERT( long_ons_ptr != NULL );
  QCRIL_ASSERT( short_ons_ptr != NULL );
  QCRIL_ASSERT( mcc_mnc_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Convert MCC+MNC to ASCII format */
  qcril_cm_ons_convert_mcc_mnc_to_ascii( plmn, *mcc_mnc_ptr );

  /* Lookup the current operator info */
  if ( QCRIL_CM_SRV_STATUS_INDICATES_GW_FULL_SRV( ss_info_ptr->srv_status, ss_info_ptr->sys_mode ) &&
       qcril_cm_ons_match_plmn( plmn, ss_info_ptr->sys_id.id.plmn_lac.plmn ) )
  {
    qcril_cm_ons_lookup_current_operator( instance_id, ss_info_ptr, long_ons_ptr, short_ons_ptr, mcc_mnc_ptr );

    return TRUE;
  }
  /* Lookup the available operator info */
  else
  {
    qcril_cm_ons_lookup_memory_list( plmn, long_ons_ptr, short_ons_ptr, mcc_mnc_ptr );

    return FALSE;
  }

} /* qcril_cm_ons_lookup_available_operator */
