#ifndef HEADER_FILE_SYNCML_COMM
#define HEADER_FILE_SYNCML_COMM

#include "sci_types.h"
#include"comdef.h"
#include "dm_pl_os.h"

typedef enum {
    XML_UNDEF = 0, XML_WBXML = 1, XML_XML
} XmlCodeType;

typedef struct syncml_Comm_type {
    char* cache;                   //buffer for globle bearer pointer to use.
    uint32 cache_length;      //buffer length
    XmlCodeType codetype;   //syncml is xml or wbxml
} syncml_Comm_type;

short dm_syncml_Comm_Init(void);

short dm_syncml_Comm_Open(void);
short dm_syncml_Comm_RecData(short id);
short dm_syncml_Comm_SendData(short id, BOOLEAN is_resent);

short dm_syncml_Comm_Close(void);

short dm_syncml_Comm_Destory(void);

#endif
