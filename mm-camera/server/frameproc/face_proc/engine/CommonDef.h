
#ifndef COMMONDEF_H__
#define COMMONDEF_H__

/* Error Code for FACEPROC Vision library */
#define     FACEPROC_MEMORYSHORTAGE          4      /* Memory Shortage */
#define     FACEPROC_HALT                    3      /* Halt in Process */
#define     FACEPROC_NOTIMPLEMENTED          2      /* Not Implemented */
#define     FACEPROC_TIMEOUT                 1      /* Time Out */
#define     FACEPROC_NORMAL                  0      /* Normal End */
#define     FACEPROC_ERR_VARIOUS            -1      /* Unexpected Error */
#define     FACEPROC_ERR_INITIALIZE         -2      /* Initialize Error */
#define     FACEPROC_ERR_INVALIDPARAM       -3      /* Invalid Parameter Error */
#define     FACEPROC_ERR_ALLOCMEMORY        -4      /* Memory Allocation Error */
#define     FACEPROC_ERR_MODEDIFFERENT      -5      /* Mode Error */
#define     FACEPROC_ERR_NOALLOC            -6      /* (reserved.) */
#define     FACEPROC_ERR_NOHANDLE           -7      /* Handle Error */
#define     FACEPROC_ERR_PROCESSCONDITION   -8      /* Can't process by condition */

/* Halt Status */
#define     HALT_STATUS_OFF  0      /* Halt status OFF */
#define     HALT_STATUS_ON   1      /* Halt status ON  */

#endif  /* COMMONDEF_H__ */

