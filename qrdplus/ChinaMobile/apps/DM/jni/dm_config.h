#ifndef _DM_CONFIG_H_
#define _DM_CONFIG_H_

/**--------------------------------------------------------------------------*
 **                         MACRO DEFINITION                                 *
 **--------------------------------------------------------------------------*/
#define DM_SELF_REGISTER_NUMBER             "10654040"
#define DM_SELF_REGISTER_PORT_NUMBER        "16998"
#define DM_SRV_ADDR_URL                     "http://dm.monternet.com:7001"
/*Begin MS00216980 cheney  follow config for CMCC test*/
#define DM_SELF_REGISTER_NUMBER_TEST             "1065840409"
#define DM_SELF_REGISTER_PORT_NUMBER_TEST        "16998"
#define DM_SRV_ADDR_URL_TEST                     "http://218.206.176.97:7001"
/*end  MS00216980 cheney */

#define DM_DEFAULT_IMEI                    "IMEI:001010523456789"
#define DM_DEFAULT_VER                     "MOCOR_W09.34_Debug"

#define DM_DEVICE_TYPE_INFO_STR             "phone"
#define DM_OEM_INFO_STR                     "Hisense"

#define DM_FIRMWARE_VERSION                 "MOCOR"
#define DM_HARDWARE_VERSION                 "intel"
#define DM_LANG_US_EN                       "en-US"
#define DM_VERSION                          "1.2"
#define DM_APN_CMNET_INFO                   "cmnet"
#define DM_APN_CMWAP_INFO                   "cmwap"
#ifdef FEA_APP_DM  //Added  2010.2.10 add realparam's cmdm apn
#define DM_APN_CMDM_INFO                    "cmdm"
#endif /* FEA_APP_DM */

#define DM_PROXY_IP_NUMBER                  "10.0.0.172"
#define DM_PROXY_PORT_NUMBER                "80"
#define DM_GATEWAY_IP_NUMBER                "http://10.0.0.172:80/"
#ifdef FEA_IDLE_DISP
#define DM_WAP_START_PAGE    "http://wap.monternet.com/?cp22=v22monternet"
#else /* FEA_IDLE_DISP */
#define DM_WAP_START_PAGE                   "http://wap.monternet.com"
#endif /* FEA_IDLE_DISP */
#define DM_MMS_MMC_ADDR                     "http://mms.monternet.com"
#define DM_PIM_SERVER_ADDR                  "http://pim.monternet.com"
#define DM_PIM_ADDRESS_BOOK_URL             "./Contact"
#define DM_PIM_CALENDAR_URL                 "./Calendar"

#define DM_CLIENT_PL_MAX_DL_PACKAGE_SIZE    (3*1024*1024)//1000000
#define DM_CLIENT_PL_MAX_OBJ_SIZE           524288
#define DM_CLIENT_PL_MAX_MSG_SIZE           15*1024
#define DM_CLIENT_PL_MAX_RETRIE_TIMES       5
#define DM_CLIENT_PL_DP_DEFAULTFILENAME     "vdm_update"
#define DM_CLIENT_PL_FILENAME_TREE          "tree.xml"
#define DM_CLIENT_PL_TMPFILENAME_TREE       "tree_new.xml"
#define DM_CLIENT_PL_FILENAME_CONFIG        "config.txt"
#define DM_CLIENT_PL_TMPFILENAME_CONFIG     "config_new.txt"
#define DM_CLIENT_PL_FILENAME_DLRESUME      "dlresume.dat"
#define DM_CLIENT_PL_REG_FILE               "reg.conf"
#define DM_CLIENT_PL_REGISTRY_KEY           0

#define DM_CLIENT_PL_RESUME_FILE            "dm_resume.dat"

#define DM_DC_LEAF_NODE_PKG_NAME            "PackageName"
#define DM_DC_LEAF_NODE_LOCATION            "Location"
#define DM_DC_LEAF_NODE_PARENT_PATH         "./SCOMO/Inventory/Deployed"
#define DM_MIME_TYPE_XML                    "application/vnd.syncml.dm+xml"
#define DM_MIME_TYPE_WXML                   "application/vnd.syncml.dm+wbxml"

#define DM_CLIENT_PL_RESUME_FUMO_FILE        "dm_resume_fumo.dat"
#define DM_CLIENT_PL_RESUME_SCOMOFILE        "dm_resume_scomo.dat"
#define DM_CLIENT_PL_RESUME_FLEX_FILE        "dm_resume_flex.dat"

#define DM_CLIENT_PL_FILENAME_DLRESUMETMP   "dlresumetem.dat"
#define DM_FUMO_UPDATE_FILE                 "__FwUpdate"
#define DM_SCOMO_UPDATE_FILE                "__SCOMO"

#endif
