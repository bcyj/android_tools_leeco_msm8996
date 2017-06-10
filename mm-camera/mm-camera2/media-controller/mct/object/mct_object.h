
/* mct_object.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MCT_OBJECT_H__
#define __MCT_OBJECT_H__

#include "media_controller.h"
#include "mct_list.h"
#include "mct_event.h"

#define MCT_OBJECT_FLAG_LAST 1
/* *
 * The top-most parent should be pipeline;
 * every module may have multiple pipelines;
 * but every port shall have only one parent(module).
 *
 */
struct _mct_object {
  /*
   * Parents and Children information:
   *
   * One Object matches to One Pipeline;
   * One Pipeline may have multiple children(Streams);
   * One Stream   may have multiple children(Modules);
   * One Module   may have multiple children(Ports);
   * One Port     may have multiple children(Stream and Session
   *              Indics sets);
   *
   * !!!!!!!!! Note !!!!!!!!!
   *   - Port MUST have ONLY ONE parent(Module);
   *   - Module MIGHT have MULTIPLE parent(Stream);
   *   - Stream MUST have ONLY ONLE parent(Pipeline).
   */
  mct_list_t       *parent;
  unsigned int     parentsnum;
  mct_list_t       *children;
  unsigned int     childrennum;

  char             *name; /* individual object's name              */
  /* invidual Object information */
  int              refcount; /* individual object's reference count   */

  /* Note: For future usage,
   * object LOCK, should be initialized to PTHREAD_MUTEX_INITIALIZER */
  pthread_mutex_t  lock;
  unsigned int     flags;
  void            *priv;
};
#define MCT_OBJECT_PRIVATE(obj)          (MCT_OBJECT_CAST(obj)->priv)
#define MCT_OBJECT_CAST(obj)             ((mct_object_t*)(obj))
#define MCT_OBJECT_CHILDREN(obj)         (MCT_OBJECT_CAST(obj)->children)
#define MCT_OBJECT_NUM_CHILDREN(obj)     (MCT_OBJECT_CAST(obj)->childrennum)
#define MCT_OBJECT_PARENT(obj)           (MCT_OBJECT_CAST(obj)->parent)
#define MCT_OBJECT_NUM_PARENTS(obj)      (MCT_OBJECT_CAST(obj)->parentsnum)
#define MCT_OBJECT_REFCOUNT(obj)         (MCT_OBJECT_CAST(obj)->refcount)
#define MCT_OBJECT_NAME(obj)             (MCT_OBJECT_CAST(obj)->name)

#define MCT_OBJECT_FLAGS(obj)            (MCT_OBJECT_CAST(obj)->flags)
#define MCT_OBJECT_FLAG_IS_SET(obj,flag) ((MCT_OBJECT_FLAGS(obj) \
                                         & (flag)) == (flag))
#define MCT_OBJECT_FLAG_SET(obj,flag)    (MCT_OBJECT_FLAGS(obj) |= (flag))
#define MCT_OBJECT_FLAG_UNSET(obj,flag)  (MCT_OBJECT_FLAGS(obj) &= ~(flag))

#define MCT_OBJECT_GET_LOCK(obj)         (&MCT_OBJECT_CAST(obj)->lock)
#define MCT_OBJECT_LOCK(obj)             pthread_mutex_lock \
                                         (MCT_OBJECT_GET_LOCK(obj))
#define MCT_OBJECT_TRYLOCK(obj)          pthread_mutex_trylock \
                                         (MCT_OBJECT_GET_LOCK(obj))
#define MCT_OBJECT_UNLOCK(obj)           pthread_mutex_unlock \
                                         (MCT_OBJECT_GET_LOCK(obj))


char* mct_strdup (const char *str);

boolean mct_object_set_name (mct_object_t *object, const char *name);
char*   mct_object_get_name (mct_object_t *object);
boolean mct_object_set_parent (mct_object_t *object, mct_object_t *parent);
mct_list_t *mct_object_get_parent (mct_object_t *object);
boolean mct_object_unparent(mct_object_t *object, mct_object_t *parent);
mct_list_t* mct_object_check_uniqueness(mct_list_t *mct_list, const char *name);
boolean mct_object_find_common_parent(mct_object_t *obj1, mct_object_t *obj2);

#endif /* __MCT_OBJECT_H__ */
