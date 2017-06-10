/********************************************************************
---------------------------------------------------------------------
 Copyright (c) 2012 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
----------------------------------------------------------------------
QSEECom Security Test
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
#include <linux/ion.h>
#include <utils/Log.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <dlfcn.h>
#include <common_log.h>

#include "QSEEComAPI.h"
//#include "wv_oemcrypto_clnt.h"

/** adb log */
#undef LOG_TAG
#define LOG_TAG "QSEECOM_SECURITY_CHECK: "

#define SCC_MAX_CHECKNUM 11

// PlayReady Dynamic Library.
static const char* PLAYREADY_LIB  = "libtzplayready.so";


/*****************************************************
 * the following are copied from wv_oemcrypto_clnt.h *
 * since it's in no-ship directory.                  *
 *****************************************************/

#ifdef __cplusplus
extern "C"
{
#endif
#ifndef _OEMCRYPTO_L1
#define OEMCrypto_GetKeyboxData _oec01
#define OEMCrypto_EncryptAndStoreKeyBox _oec02
#define OEMCrypto_IdentifyDevice _oec03
#define OEMCrypto_GetRandom _oec04
#else
#define OEMCrypto_Initialize _oec01
#define OEMCrypto_Terminate _oec02
#define OEMCrypto_SetEntitlementKey _oec03
#define OEMCrypto_DeriveControlWord _oec04
#define OEMCrypto_DecryptVideo _oec05
#define OEMCrypto_DecryptAudio _oec06
#define OEMCrypto_InstallKeybox _oec07
#define OEMCrypto_GetKeyData _oec08
#define OEMCrypto_IsKeyboxValid _oec09
#define OEMCrypto_GetRandom _oec10
#define OEMCrypto_GetDeviceID _oec11
#define OEMCrypto_EnterSecurePlayback _oec12
#define OEMCrypto_ExitSecurePlayback _oec13
#define OEMCrypto_WrapKeybox _oec14
#endif

typedef enum OEMCryptoResult {
  OEMCrypto_SUCCESS = 0,
  OEMCrypto_ERROR_INIT_FAILED,
  OEMCrypto_ERROR_TERMINATE_FAILED,
  OEMCrypto_ERROR_ENTER_SECURE_PLAYBACK_FAILED,
  OEMCrypto_ERROR_EXIT_SECURE_PLAYBACK_FAILED,
  OEMCrypto_ERROR_SHORT_BUFFER,
  OEMCrypto_ERROR_NO_DEVICE_KEY,
  OEMCrypto_ERROR_NO_ASSET_KEY,
  OEMCrypto_ERROR_KEYBOX_INVALID,
  OEMCrypto_ERROR_NO_KEYDATA,
  OEMCrypto_ERROR_NO_CW,
  OEMCrypto_ERROR_DECRYPT_FAILED,
  OEMCrypto_ERROR_WRITE_KEYBOX,
  OEMCrypto_ERROR_WRAP_KEYBOX,
  OEMCrypto_ERROR_BAD_MAGIC,
  OEMCrypto_ERROR_BAD_CRC,
  OEMCrypto_ERROR_NO_DEVICEID,
  OEMCrypto_ERROR_RNG_FAILED,
  OEMCrypto_ERROR_RNG_NOT_SUPPORTED,
  OEMCrypto_ERROR_SETUP,
  OEMCrypto_FAILURE = 0x7FFFFFFF
} OEMCryptoResult;
 
OEMCryptoResult OEMCrypto_Initialize(void);
OEMCryptoResult OEMCrypto_Terminate(void);
OEMCryptoResult OEMCrypto_IsKeyboxValid();
#ifdef __cplusplus
}
#endif


enum SCC_CATEGORY {
  SCCCAT_NONE = 0,
  SCCCAT_PLATFORM,
  SCCCAT_CPZ,
  SCCCAT_APPLICATION,
  SCCCAT_MAX
};

enum SCC_QSEECMD {
	QSECmd_NONE = 0,
  QSECmd_SecureBootCheck,
  QSECmd_BBSFSKeyCheck,
  QSECmd_TZSFSKeyCheck,
  QSECmd_DebugDisableCheck,
  QSECmd_QSEEComCheck,
  QSECmd_DRMPRKeyCheck,
  QSECmd_DRMWVKeyCheck
};

enum SCC_CHECKENV {
  SCCENV_NONE = 0,
  SCCENV_HLOS,
  SCCENV_QSEE,
  SCCENV_QSAPP,
  SCCENV_TZBSP
};

typedef int (* SecCfgCheck_func_t)(void *);

typedef struct SecCfgCheck
{
  char CheckName[32];
  char CheckDesc[128];
  enum SCC_CATEGORY Category;
  enum SCC_QSEECMD  QSEEComCmd;
  enum SCC_CHECKENV CheckEnv;
  SecCfgCheck_func_t CheckFunction;
} SecCfgCheck_t;

static int TZBSPHWSecCfgCheck(void *);
static int SecuredMemoryCheck(void *);
static int SecuredPILCheck(void *);
static int QSEEComInterfaceCheck(void *);
static int DRMDiagDisabledCheck(void *);
static int DRMPRKeyProvisionCheck(void *);
static int DRMWVKeyProvisionCheck(void *);

//const char StrSccCategory[][] = {"", "Platform", "ContProt", "Application"};

SecCfgCheck_t SecCfgCheckTable[ ] =
{
//{ "" Category, QSEECom_CMD,Checking_Environment,Function_Name},
  { "", "starting", 0, 0, 0, NULL},
  { "SecureBoot",  "SecureBoot Enabled",                 SCCCAT_PLATFORM, QSECmd_SecureBootCheck,   SCCENV_TZBSP, &TZBSPHWSecCfgCheck},
  { "BBKeyEnable", "Basebanes SFS Key Enabled",          SCCCAT_PLATFORM, QSECmd_BBSFSKeyCheck,     SCCENV_TZBSP, &TZBSPHWSecCfgCheck},
  { "TZKeyEnable", "Trustzone SFS Key Enabled",          SCCCAT_PLATFORM, QSECmd_TZSFSKeyCheck,     SCCENV_TZBSP, &TZBSPHWSecCfgCheck},
  { "DebuDisable", "Debug Disabled",                     SCCCAT_PLATFORM, QSECmd_DebugDisableCheck, SCCENV_TZBSP, &TZBSPHWSecCfgCheck},
  { "SecureMem",   "XPU Protected Memory Access",        SCCCAT_CPZ,      QSECmd_NONE,              SCCENV_QSEE,  &SecuredMemoryCheck},
  { "SecurePIL",   "Secure PIL Validation",              SCCCAT_CPZ,      QSECmd_NONE,              SCCENV_QSEE,  &SecuredPILCheck},
  { "QSEECom",     "QSEECom Interface Validation",       SCCCAT_CPZ,      QSECmd_QSEEComCheck,      SCCENV_QSEE,  &QSEEComInterfaceCheck},
  { "DRMDiag",     "DRM DIAG Service Disabled",          SCCCAT_APPLICATION, QSECmd_NONE,           SCCENV_HLOS,  &DRMDiagDisabledCheck},
  { "TZPRKey",     "Trustzone Plaready Keys Provisioned",SCCCAT_APPLICATION, QSECmd_DRMPRKeyCheck,  SCCENV_QSAPP, &DRMPRKeyProvisionCheck},
  { "TZWVKey",     "Trustzone Widevine Keys Provisioned",SCCCAT_APPLICATION, QSECmd_DRMWVKeyCheck,  SCCENV_QSAPP, &DRMWVKeyProvisionCheck},
};

/********************************************************
 * SECURITY TEST STATICS
 ********************************************************/
static void *start_scc_func(void* data)
{
  SecCfgCheck_t *scc_data = (SecCfgCheck_t *)data;

  scc_data->CheckFunction(data);

  pthread_exit( NULL );
  return NULL;
}

/* Function/thread to perform the various security tests */
static int TZBSPHWSecCfgCheck(void* data)
{
  SecCfgCheck_t *scc_data = (SecCfgCheck_t *)data;

  do {
    LOGD( "Security Configuration %s is not supported ...", scc_data->CheckName );
  } while( 0 );

  return 0;
}

/* Function/thread to perform the various security tests */
static int SecuredMemoryCheck(void* data)
{
  SecCfgCheck_t *scc_data = (SecCfgCheck_t *)data;

  do {
    LOGD( "Security Configuration %s is not supported ...", scc_data->CheckName );
  } while( 0 );

  return 0;
}

/* Function/thread to perform the various security tests */
static int SecuredPILCheck(void* data)
{
  SecCfgCheck_t *scc_data = (SecCfgCheck_t *)data;

  do {
    LOGD( "Security Configuration %s is not supported ...", scc_data->CheckName );
  } while( 0 );

  return 0;
}

/* Function/thread to perform the various security tests */
static int QSEEComInterfaceCheck(void* data)
{
  SecCfgCheck_t *scc_data = (SecCfgCheck_t *)data;

  do {
    LOGD( "Security Configuration %s is not supported ...", scc_data->CheckName );
  } while( 0 );

  return 0;
}

/* Function/thread to perform the various security tests */
static int DRMDiagDisabledCheck(void* data)
{
  SecCfgCheck_t *scc_data = (SecCfgCheck_t *)data;

  do {
    LOGD( "Security Configuration %s is not supported ...", scc_data->CheckName );
  } while( 0 );

  return 0;
}

/* Function/thread to perform the various security tests */
static int DRMPRKeyProvisionCheck(void* data)
{
  SecCfgCheck_t *scc_data = (SecCfgCheck_t *)data;

  long (*playready_verify_keys)() = NULL;
  void *m_prhnd = NULL;
  int ret = -1;

  LOGD("PLAYREADY KEY CHECK ----------------------------\n");

  do{
    m_prhnd   = dlopen(PLAYREADY_LIB, RTLD_NOW);
    if(m_prhnd)
    {
      LOGD("Init dlopen(PLAYREADY_LIB, RTLD_NOW) succeeds\n");
      *(void **)(&playready_verify_keys)= dlsym(m_prhnd, "playready_verify_keys");
      if(playready_verify_keys)
      {
        ret = (*playready_verify_keys)();
        if(ret == 0){
          LOGD("PLAYREADY THE KEY VERIFICATION PASSED\n");
        }
        else{
          LOGE("PLAYREADY THE KEY VERIFICATION FAILED\n");
        }
      }
      else
      {
        LOGE("Init dlsym(m_prhnd, playready_verify_keys) is failed....\n");
        ret = -1;
        break;
      }
      dlclose(m_prhnd);
    }
    else
    {
      LOGE("Init dlopen(PLAYREADY_LIB, RLTD_NOW) is failed....\n");
      ret = -1;
      break;
    }
  }while(0);

  return ret;
}

/* Function/thread to perform the various security tests */
static int DRMWVKeyProvisionCheck(void* data)
{
  SecCfgCheck_t *scc_data = (SecCfgCheck_t *)data;

  LOGD("WIDEVINE KEY CHECK ----------------------------\n");

  do {
    OEMCryptoResult ret = OEMCrypto_SUCCESS;

    ret = OEMCrypto_Initialize();
    if(ret != 0 )
    {
      LOGE("OEMCrypto_Initialize test failed\n");
    }

    ret = OEMCrypto_IsKeyboxValid();
    if( ret != OEMCrypto_SUCCESS )
    {
      LOGE("Failed: WIDEVINE The Keybox is NOT valid. ret: %u \n", ret);
    }
    else
    {
      LOGD("Succeed: WIDEVINE The Keybox is valid\n");
    }

    /* Terminate */
    ret = OEMCrypto_Terminate();
    if(ret != 0 )
    {
      LOGE("OEMCrypto_Terminate test failed\n");
    }
  } while( 0 );

  return 0;
}

static void PrintHelp()
{
  printf("\n\n-------------------------------\n");
  printf("Security Feature Configuration Check\n");
  printf("USAGE: SecConfigCheck [option]. \n\n");
  printf("EXAMPLE: SecConfigCheck SecureBoot \n\n");
  printf("[pf] - Platform\n");
  printf("  [SecureBoot] - SecureBoot Enabled\n");
  printf("  [BBKeyEnable] - Basebane Secure File System(SFS) Key Enabled\n");
  printf("  [TZKeyEnable] - Trustzone Secure File System(SFS) Key Enabled\n");
  printf("  [DebugDisable] - JTAG Debug Disabled\n");
  printf("\n");
  printf("[cp] - Content Protection\n");
  printf("  [SecureMem] - MPU/XPU Protected Memory Access validation\n");
  printf("  [SecurePIL] - Secure PIL feature validation\n");
  printf("  [QSEECom] - QSEEcom interface validation\n");
  printf("\n");
  printf("[app] - Application\n");
  printf("  [DRMDiag] - DRM DIAG Service disabled\n");
  printf("  [TZPRKey] - Trustzone Playready keys provisioned\n");
  printf("  [TZWVKey] - Trustzone Widevine keys provisioned\n");
  printf("[help] - Help\n");
  printf("[exit] - Exit\n");
  printf("\n");
  return;
}

int main( int argc, char *argv[] )
{
  SecCfgCheck_func_t scc_func;
  SecCfgCheck_t *scc_data;
  pthread_t process_scc_thread;
  const char strExit[] = "exit";
  const char strHelp[] = "help";
  int rc = -1;
  void *funcRet = NULL;
  unsigned int ck = 0, CheckNum = 0;
  int i = 0;
  int j = 0;

  if( argc <= 1 )
  {
    PrintHelp();
    return 0;
  }

  /* process multiple argument options. */
  while(argc-- >= 1) {
    *(argv++);
    printf("input: %s\n", *argv);

    if( strcmp(*argv, strExit) == 0 )
    {
      break;
    }

    else if( strcmp(*argv, strHelp) == 0 )
    {
      PrintHelp();
      break;
    }

    /* match the option with checklist name*/
    CheckNum = 0;
    for(ck = 1; ck <= (SCC_MAX_CHECKNUM + 1); ck++) {
      LOGE("compare arg:'%s' with CheckName '%s' (Description '%s')\n",
             *argv, SecCfgCheckTable[ck].CheckName, SecCfgCheckTable[ck].CheckDesc);

      if( strncmp(*argv, SecCfgCheckTable[ck].CheckName, sizeof(*argv)) == 0 ) {
        CheckNum = ck;
        break;
      }
    }
    if(CheckNum == 0) {
      LOGE("Error - Invalid check name input %s\n", *argv);
      continue;
    }

    /* get the check data structure from the big table */
    scc_data = &SecCfgCheckTable[CheckNum];
    if(scc_data == NULL) {
      LOGE("Error - Invalid scc item in SecCfgChecktable!\n ");
      continue;
    }

    /* get the check execution function */
    scc_func = scc_data->CheckFunction;
    if(scc_func == NULL) {
      LOGE("Error - Invalid scc function!\n ");
      continue;
    }

    /* execute the check function. if the real checking is
       not executed in HLOS, we need run it in another thread */
    if(scc_data->CheckEnv == SCCENV_HLOS ) {
      rc = scc_func((void *)scc_data);
      if(rc != 0) {
        LOGE("Error - Security Check '%s' returns failure\n", scc_data->CheckName);
      }
      else {
        LOGE("Successful - Security Check '%s' returns successful\n", scc_data->CheckName);
      }
    }
    else {
      rc = pthread_create( &process_scc_thread,
                            NULL,
                           &start_scc_func,
                            scc_data);

      pthread_join(process_scc_thread, &funcRet);
    }
  }

  return 0;
}
