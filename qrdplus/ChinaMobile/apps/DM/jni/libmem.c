#ifdef WIN32
#include "std_header.h"
#endif
#define _LIBMEM_C_

#include "stdlib.h"
#include "libmem.h"

#ifdef DM_MEMORY_LEAK_DEBUG

typedef struct LeakLink_s
{
    uint32 ptr_addr;
    uint32 line;
    char filename[70];
    struct LeakLink_s * next;
}LeakLink_t, *LeakLinkPtr_t;

LeakLinkPtr_t leak_link_head_ptr = NULL;
LeakLinkPtr_t leak_link_tail_ptr = NULL;

void AddToLeakLink(void *ptr, const char * file, int line)
{
    if (NULL == leak_link_head_ptr)
    {
        leak_link_head_ptr = (LeakLinkPtr_t)SCI_ALLOC(sizeof(LeakLink_t);
                leak_link_tail_ptr = leak_link_head_ptr;
            }
            else
            {
                leak_link_tail_ptr->next = (LeakLinkPtr_t)SCI_ALLOC(sizeof(LeakLink_t);
                        leak_link_tail_ptr = leak_link_tail_ptr->next;
                    }
                    leak_link_tail_ptr->ptr_addr = (uint32)ptr;
                    SCI_MEMCPY(leak_link_tail_ptr->filename, file, strlen(file);
                            leak_link_tail_ptr->filename[strlen(file)] = 0;
                            leak_link_tail_ptr->line = (uint32)line;
                            leak_link_tail_ptr->next = NULL;
                        }

                        void DeleteFromLeakLink(void *ptr)
                        {
                            LeakLinkPtr_t temp_ptr = leak_link_head_ptr;
                            LeakLinkPtr_t temp_ptr_2 = leak_link_head_ptr;
                            while ((NULL != temp_ptr) && (temp_ptr->ptr_addr != (uint32)ptr) )
                            {
                                temp_ptr_2= temp_ptr;
                                temp_ptr = temp_ptr->next;
                            }
                            if (NULL != temp_ptr)
                            {
                                temp_ptr_2->next = temp_ptr->next;
                                if (leak_link_tail_ptr == temp_ptr)
                                {
                                    leak_link_tail_ptr = temp_ptr_2;
                                }
                                if (leak_link_head_ptr == temp_ptr)
                                {
                                    leak_link_head_ptr = temp_ptr->next;
                                }
                                if ((leak_link_head_ptr == temp_ptr) && (leak_link_tail_ptr == temp_ptr))
                                {
                                    leak_link_head_ptr = NULL;
                                    leak_link_tail_ptr = NULL;
                                }
                                SCI_FREE(temp_ptr);
                            }
                            else
                            {
                                SCI_TRACE_MID("MMIDM ptr is not in the link ! pObject = 0x%x", (uint32)ptr);
                            }
                        }

                        void DestroyLeakLink()
                        {
                            LeakLinkPtr_t temp_ptr = leak_link_head_ptr;
                            LeakLinkPtr_t temp_ptr_2 = leak_link_head_ptr;
                            while (NULL != temp_ptr)
                            {
                                temp_ptr_2 = temp_ptr;
                                temp_ptr = temp_ptr->next;
                                SCI_FREE(temp_ptr_2);
                            }
                            leak_link_head_ptr = NULL;
                            leak_link_tail_ptr = NULL;
                            sml_malloc_num = 0;
                        }

                        void PrintLeakLink()
                        {
                            LeakLinkPtr_t temp_ptr = leak_link_head_ptr;
                            uint32 i = 0;
                            while (NULL != temp_ptr)
                            {
                                i++;
                                SCI_TRACE_MID("MMIDM PrintLeakLink %d  ptr: %d  file : %s  line %d", i, temp_ptr->ptr_addr, temp_ptr->filename, temp_ptr->line);
                                temp_ptr = temp_ptr->next;
                            }
                            SCI_TRACE_MID("MMIDM PrintLeakLink total: %d  sml_malloc_num = %d", i, sml_malloc_num);
                            DestroyLeakLink();
                        }
#endif // DM_MEMORY_LEAK_DEBUG
/**
 * FUNCTION: dm_smlLibFree
 *
 * Deallocates the memory of object "pObject", which has been allocated
 * previously.
 * If "pObject" is a NULL pointer nothing happens.
 * If "pObject" is a pointer to memory which has not been allocated
 * previouly, the behaviour is undefined.
 * The contents of the deallocated memory object is destroyed.
 */
void dm_smlLibFree(void *pObject) {
    if (!pObject)
        return;

#ifdef DM_MEMORY_LEAK_DEBUG
    DeleteFromLeakLink(pObject);
#endif
    SCI_FREE(pObject);
    pObject = NULL;
}

/**
 * FUNCTION: dm_smlLibRealloc
 *
 * Changes size of preallocated space for memory object "pObject"
 * to the new size specified by "constSize".
 *
 * If the new size is larger than the old size, the old contents
 * is not changed. Additionally space is added at the the end of
 * "pObject". The new allocated space is not initialized
 * to any special value.
 * If the new size is smaller than the old size, the unused space
 * is discarded.
 *
 * If "pObject" is a NULL pointer, this function behaves just like
 * dm_smlLibMalloc().
 * If "pObject" does not point to a previously allocated memory area,
 * the behavior is undefined.
 * If "constSize" is 0, a NULL pointer is returned and the space
 * which "pObject" points to is freed up.
 *
 * Returns a pointer to the first byte of the resized object.
 * If no new memory could be allocated, a NULL Pointer is returned
 * without changing the memory object "pObject" (Nothing happens to the content).
 *
 * IN/OUT           void *pObject,      memory object, which size should be changed
 * IN:              MemSize_t constSize new size the memory object shall use
 * RETURN:          void*               Pointer to memory object, which size has been
 *                                      be changed
 *                                      NULL, if not successfull or
 *                                            if constSize==0
 */
void *dm_smlLibRealloc(void *pObject, long constSize) {
    return realloc(pObject, constSize);
}

void *dm_smlLibMemset(void *pObject, int value, long count) {
    return memset(pObject, value, count);
}
void *dm_smlLibMemcpy(void *pTarget, const void *pSource, long count) {
    return memcpy(pTarget, pSource, count);
}
void *dm_smlLibMemmove(void *pTarget, const void *pSource, long count) {
    return memmove(pTarget, pSource, count);
}
int dm_smlLibMemcmp(const void *pTarget, const void *pSource, long count) {
    return memcmp(pTarget, pSource, count);
}

void *dm_smlLibMalloc(const char * file, int line, long size) {
    void * ptr;
    ptr = SCI_ALLOC_APP(size);

#ifdef DM_MEMORY_LEAK_DEBUG
    AddToLeakLink(ptr, file, line);
#endif
    return (void *) ptr;
}

int dm_smlLibStrlen(const char *pString) {
    return strlen((char *) pString);
}

char* dm_smlLibStrcpy(const char *pTarget, const char *pSource) {
    return strcpy((char *) pTarget, (char *) pSource);
}
/**
 * FUNCTION: dm_smlLibStrdup
 *
 * Duplicates the String "constStringP".
 * Returns a pointer to the new copy of "constStringP".
 *
 * IN:              String_t   constStringP     string, which is duplicated
 * RETURN:          String_t   pointer to the new copy,
 *                             null, if no copy could be allocated
 */
char* dm_smlLibStrdup(const char *constStringP) {
    char* _new_str = NULL;

    // allocate memory for new copy
    _new_str = (char*) dm_smlLibMalloc(__FILE__, __LINE__,
            dm_smlLibStrlen(constStringP) + 1);

    // Copy the string into the new memory
    if (_new_str != NULL)
        dm_smlLibStrcpy(_new_str, constStringP);

    return _new_str;
}

int dm_smlLibStrcmp(const char *pTarget, const char *pSource) {
    return strcmp((char *) pTarget, (char *) pSource);
}
int dm_smlLibStrcmp_ext(const char *pTarget, const char *pSource) {
    int targetlen;
    int sourcelen;
    int lesslen;
    int i;
    targetlen = strlen((char *) pTarget);
    sourcelen = strlen((char *) pSource);
    if (targetlen > sourcelen) {
        lesslen = sourcelen;
    } else {
        lesslen = targetlen;
    }
    for (i = 0; i < lesslen; i++) {
        //it is english letter
        if (*(pSource + i) == *(pTarget + i)) {
            continue;
        } else {
            if ((0x41 <= *(pSource + i) && 0x5A >= *(pSource + i))
                    || (0x61 <= *(pSource + i) && 0x7A >= *(pSource + i))) {
                if (*(pSource + i) < *(pTarget + i)) {
                    if ((*(pSource + i) + 0x20) == *(pTarget + i)) {
                        continue;
                    } else {
                        return 1;
                    }
                } else {
                    if ((*(pSource + i) - 0x20) == *(pTarget + i)) {
                        continue;
                    } else {
                        return -1;
                    }
                }
            } else {
                if (*(pSource + i) < *(pTarget + i)) {
                    return 1;
                } else {
                    return -1;
                }
            }
        }
    }

    return 0;
}

char* dm_smlLibStrcat(const char *pTarget, const char *pSource) {
    return strcat((char *) pTarget, (char *) pSource);
}

char* dm_smlLibStrncpy(const char *pTarget, const char *pSource, int count) {
    return strncpy((char *) pTarget, (char *) pSource, count);
}

int dm_smlLibStrncmp(const char *pTarget, const char *pSource, int count) {
    if (PNULL != pSource) {
        return strncmp((char *) pTarget, (char *) pSource, count);
    } else {
        return -1;
    }
}

int dm_smlLibStrncmp_ext(const char *pTarget, const char *pSource, int count) {
    int targetlen;
    int sourcelen;
    int lesslen;
    int i;
    targetlen = strlen((char *) pTarget);
    sourcelen = strlen((char *) pSource);
    if (targetlen < count) {
        return -1;
    }
    if (sourcelen < count) {
        return 1;
    }
    lesslen = count;
    for (i = 0; i < lesslen; i++) {
        //it is english letter
        if (*(pSource + i) == *(pTarget + i)) {
            continue;
        } else {
            if ((0x41 <= *(pSource + i) && 0x5A >= *(pSource + i))
                    || (0x61 <= *(pSource + i) && 0x7A >= *(pSource + i))) {
                if (*(pSource + i) < *(pTarget + i)) {
                    if ((*(pSource + i) + 0x20) == *(pTarget + i)) {
                        continue;
                    } else {
                        return 1;
                    }
                } else {
                    if ((*(pSource + i) - 0x20) == *(pTarget + i)) {
                        continue;
                    } else {
                        return -1;
                    }
                }
            } else {
                if (*(pSource + i) < *(pTarget + i)) {
                    return 1;
                } else {
                    return -1;
                }
            }
        }
    }

    return 0;
}

char* dm_smlLibStrchr(const char *pString, char character) {
    return strchr((char *) pString, character);
}

void dm_smlLibSleep(unsigned long ticks) {
    SCI_Sleep(ticks);
}
