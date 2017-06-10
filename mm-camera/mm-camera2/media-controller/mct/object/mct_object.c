/* mct_object.c
 *
 * This file contains the default infrastructure and implementation for
 * the objects. The object serves the purpose of a base structure providing
 * the functionalities common to all other child structures(stream, module,
 * port etc.).
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "mct_object.h"

char* mct_strdup (const char *str)
{
  char *new_str;
  size_t length;

  if (str) {
    length = strlen (str) + 1;
    new_str = malloc(length);
    if (!new_str)
      return NULL;
    memcpy(new_str, str, length);
  } else
    new_str = NULL;

  return new_str;
}

/**
 * mct_object_set_name:
 * */
boolean mct_object_set_name(mct_object_t *object, const char *name)
{
  if (!object)
    return FALSE;

  /* parented objects cannot be renamed */
  if (object->parent != NULL)
    return FALSE;

  if (object->name != NULL) {
    free(object->name);
  }

  object->name = mct_strdup(name);
  return TRUE;
}


/**
 * mct_object_get_name:
 *
 * */
char* mct_object_get_name(mct_object_t * object)
{
  char *result = NULL;

  if (!object)
    return result;

  result = mct_strdup((const char*)(object->name));

  return result;
}

/**
 * mct_object_set_parent:
 * */
boolean mct_object_set_parent(mct_object_t *object, mct_object_t *parent)
{
  if (!object || !parent || object == parent)
    return FALSE;
  mct_list_t *temp_parent, *temp_child;
  /* set parent */
  temp_parent = mct_list_append(MCT_OBJECT_PARENT(object),
    parent, NULL, NULL);
  if (!temp_parent)
    return FALSE;
  MCT_OBJECT_PARENT(object) = temp_parent;
  MCT_OBJECT_NUM_PARENTS(object) += 1;

  /* set child */
  temp_child =
    mct_list_append(MCT_OBJECT_CHILDREN(parent), object, NULL, NULL);
  if (!temp_child) {
    MCT_OBJECT_PARENT(object) = mct_list_remove(MCT_OBJECT_PARENT(object),
      parent);
    MCT_OBJECT_NUM_PARENTS(object) -= 1;
    return FALSE;
  }
  MCT_OBJECT_CHILDREN(parent) = temp_child;
  MCT_OBJECT_NUM_CHILDREN(parent) += 1;

  return TRUE;
}

/* Normal function to return an Object's parent */
mct_list_t* mct_object_get_parent(mct_object_t *object)
{
  mct_list_t *result = NULL;

  if (!object)
    return result;

  return object->parent;
}


boolean mct_object_unparent(mct_object_t *object, mct_object_t *parent)
{
  if (!object || !parent)
    return FALSE;
  if (!MCT_OBJECT_NUM_CHILDREN(parent) || !MCT_OBJECT_NUM_PARENTS(object))
    return FALSE;

  MCT_OBJECT_CHILDREN(parent) =
    mct_list_remove(MCT_OBJECT_CHILDREN(parent), object);
  MCT_OBJECT_NUM_CHILDREN(parent)--;

  MCT_OBJECT_PARENT(object) =
      mct_list_remove(MCT_OBJECT_PARENT(object), parent);
  MCT_OBJECT_NUM_PARENTS(object)--;

  return TRUE;
}

static boolean object_traverse_compare_name(void *data, void *user_data)
{
  mct_object_t *child;

  child = MCT_OBJECT_CAST(data);

  if (!strcmp((char const *)(MCT_OBJECT_NAME(child)), (char const*)user_data)) {
    return TRUE;
  }

  return FALSE;
}

/**
 * mct_object_check_uniqueness:
 *
 *
 * */
mct_list_t* mct_object_check_uniqueness(mct_list_t *mct_list, const char *name)
{
  mct_list_t *result = NULL;

  if (!name || !mct_list)
    return NULL;

  result = mct_list_find_custom(mct_list, (void *)name, object_traverse_compare_name);

  return result;
}

boolean mct_object_find_common_parent(mct_object_t *obj1, mct_object_t *obj2)
{
  if (!obj1 || !obj2)
    return FALSE;

  return (obj1->parent == obj2->parent) ? TRUE : FALSE;
}
