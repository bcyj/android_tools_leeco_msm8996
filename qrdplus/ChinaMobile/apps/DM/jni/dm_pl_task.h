#ifndef _DM_PL_TASK_H_
#define _DM_PL_TASK_H_

#include "sci_types.h"
#include "dm_app.h"

extern void DMTaskStart(DM_SIGNAL_T* signal_ptr);

extern void DMTaskRun(void);

extern void DMTaskCancel(void);

extern void DMTaskExit(void);

extern void DMTaskRun(void);

extern void DMTaskTerminate(void);

#endif
