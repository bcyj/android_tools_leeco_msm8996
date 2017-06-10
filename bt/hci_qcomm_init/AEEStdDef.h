#include <stdlib.h>
#include "comdef.h"

#define std_memmove(args...) memmove(args)
#define std_memset(args...) memset(args)
#define std_strlen(arg) strlen(arg)
#define std_strchr(args...) strchr(args)
#define std_strstr(args...) strstr(args)

#define STD_NEGATIVE 1
#define STD_NODIGITS 2
#define STD_OVERFLOW 3
#define STD_BADPARAM 4

unsigned long std_scanul(const char *pchBuf, int nRadix,
                            const char **ppchEnd, int *pnError);
