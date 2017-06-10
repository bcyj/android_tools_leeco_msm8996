#ifndef QMI_IDL_LIB_TARGET_H
#define QMI_IDL_LIB_TARGET_H

#include <setjmp.h>

typedef struct {
  jmp_buf jb;
  int err;
  int v1;
  int v2;
  int v3;
} qmi_idl_lib_exception_type;

#define QMI_IDL_LIB_DEBUG_PRINT(err)

#define ERROR_LABEL __idl_exception

#define QMI_IDL_LIB_TRY(exc) if(!((exc)->err= setjmp((exc)->jb))) 

#define QMI_IDL_LIB_CATCH(exc) else

#define QMI_IDL_HANDLE_ERROR(exc, err_val, value1, value2, value3 ) \
  do { \
    (exc)->v1 = value1; \
    (exc)->v2 = value2; \
    (exc)->v3 = value3; \
    longjmp((exc)->jb, err_val); \
  } while(0)

#define QMI_IDL_LIB_GET_ERROR(exc) ((exc)->err)

#endif
