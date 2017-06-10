#ifndef  HEADER_FILE_SYNCMLMGR
#define  HEADER_FILE_SYNCMLMGR

#include "smlcore.h"

short dm_syncml_init(void);

short dm_syncml_initance(short * pID, char* workspacename);

short dm_syncml_startmessage(short id);

short dm_syncml_StartSync(short id);

short dm_syncml_AlertCmd(short id, char* cmd);

short dm_syncml_PutCmd(short id);

short dm_syncml_GetCmd(short id);

short dm_syncml_AddCmd(short id);

short dm_syncml_DeleteCmd(short id);

short dm_syncml_StatusCmd(short id);

short dm_syncml_MapCmd(short id);

short dm_syncml_ReplaceCmd(short id);

short dm_syncml_EndSync(short id);

short dm_syncml_EndMessage(short id);

short dm_syncml_EndMessageWithoutFinal(short id);

short dm_syncml_ReceiveData(short id);

short dm_syncml_TerminateInstance(short id);

short dm_syncml_TerminateAllExitInstance(void);

short dm_syncml_Terminate(void);

#endif
