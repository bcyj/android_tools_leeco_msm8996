/*
**
** Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights ReservedA
** Qualcomm Technologies Confidential and Proprietary.
**
** Not a Contribution, Apache license notifications and
** license are retained for attribution purposes only.
*/
/*
 * Copyright (C) 2013 The Android Open Source Project
 * Inspired by TinyHW, written by Mark Brown at Wolfson Micro
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "usnd_route"
/*#define LOG_NDEBUG 0*/

#include <errno.h>
#include <expat.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <cutils/log.h>

#include <tinyalsa/asoundlib.h>

#define BUF_SIZE 1024
#define MIXER_XML_PATH "/system/etc/mixer_paths.xml"
#define INITIAL_MIXER_PATH_SIZE 8

struct mixer_state {
    struct mixer_ctl *ctl;
    unsigned int num_values;
    int *old_value;
    int *new_value;
    int *reset_value;
};

struct mixer_setting {
    unsigned int ctl_index;
    unsigned int num_values;
    int *value;
};

struct mixer_value {
    unsigned int ctl_index;
    int index;
    int value;
};

struct mixer_path {
    char *name;
    unsigned int size;
    unsigned int length;
    struct mixer_setting *setting;
};

struct usnd_route {
    struct mixer *mixer;
    unsigned int num_mixer_ctls;
    struct mixer_state *mixer_state;

    unsigned int mixer_path_size;
    unsigned int num_mixer_paths;
    struct mixer_path *mixer_path;
    /* ordered indexes of mixer controls */
    unsigned int *ctls_indexes;
    unsigned int max_ctls_amount;
    unsigned int ctls_counter;
};

struct config_parse_state {
    struct usnd_route *ur;
    struct mixer_path *path;
    int level;
};

/* path functions */

static inline struct mixer_ctl *index_to_ctl(struct usnd_route *ur,
                                             unsigned int ctl_index)
{
    return ur->mixer_state[ctl_index].ctl;
}

static void path_print(struct usnd_route *ur, struct mixer_path *path)
{
    unsigned int i;
    unsigned int j;

    ALOGE("Path: %s, length: %d", path->name, path->length);
    for (i = 0; i < path->length; i++) {
        struct mixer_ctl *ctl = index_to_ctl(ur, path->setting[i].ctl_index);

        ALOGE("  id=%d: ctl=%s", i, mixer_ctl_get_name(ctl));
        for (j = 0; j < path->setting[i].num_values; j++)
            ALOGE("    id=%d value=%d", j, path->setting[i].value[j]);
    }
}

static void path_free(struct usnd_route *ur)
{
    unsigned int i;

    for (i = 0; i < ur->num_mixer_paths; i++) {
        if (ur->mixer_path[i].name)
            free(ur->mixer_path[i].name);
        if (ur->mixer_path[i].setting) {
            if (ur->mixer_path[i].setting->value)
                free(ur->mixer_path[i].setting->value);
            free(ur->mixer_path[i].setting);
        }
    }
    free(ur->mixer_path);
}

static struct mixer_path *path_get_by_name(struct usnd_route *ur,
                                           const char *name)
{
    unsigned int i;

    for (i = 0; i < ur->num_mixer_paths; i++)
        if (strcmp(ur->mixer_path[i].name, name) == 0)
            return &ur->mixer_path[i];

    return NULL;
}

static struct mixer_path *path_create(struct usnd_route *ur, const char *name)
{
    struct mixer_path *new_mixer_path = NULL;

    if (path_get_by_name(ur, name)) {
        ALOGE("Path name '%s' already exists", name);
        return NULL;
    }

    /* check if we need to allocate more space for mixer paths */
    if (ur->mixer_path_size <= ur->num_mixer_paths) {
        if (ur->mixer_path_size == 0)
            ur->mixer_path_size = INITIAL_MIXER_PATH_SIZE;
        else
            ur->mixer_path_size *= 2;

        new_mixer_path = realloc(ur->mixer_path, ur->mixer_path_size *
                                 sizeof(struct mixer_path));
        if (new_mixer_path == NULL) {
            ALOGE("Unable to allocate more paths");
            return NULL;
        } else {
            ur->mixer_path = new_mixer_path;
        }
    }

    /* initialise the new mixer path */
    ur->mixer_path[ur->num_mixer_paths].name = strdup(name);
    ur->mixer_path[ur->num_mixer_paths].size = 0;
    ur->mixer_path[ur->num_mixer_paths].length = 0;
    ur->mixer_path[ur->num_mixer_paths].setting = NULL;

    /* return the mixer path just added, then increment number of them */
    return &ur->mixer_path[ur->num_mixer_paths++];
}

static int find_ctl_index_in_path(struct mixer_path *path,
                                  unsigned int ctl_index)
{
    unsigned int i;

    for (i = 0; i < path->length; i++)
        if (path->setting[i].ctl_index == ctl_index)
            return i;

    return -1;
}

static int alloc_path_setting(struct mixer_path *path)
{
    struct mixer_setting *new_path_setting;
    int path_index;

    /* check if we need to allocate more space for path settings */
    if (path->size <= path->length) {
        if (path->size == 0)
            path->size = INITIAL_MIXER_PATH_SIZE;
        else
            path->size *= 2;

        new_path_setting = realloc(path->setting,
                                   path->size * sizeof(struct mixer_setting));
        if (new_path_setting == NULL) {
            ALOGE("Unable to allocate more path settings");
            return -1;
        } else {
            path->setting = new_path_setting;
        }
    }

    path_index = path->length;
    path->length++;

    return path_index;
}

static int path_add_setting(struct usnd_route *ur, struct mixer_path *path,
                            struct mixer_setting *setting)
{
    int path_index;

    if (find_ctl_index_in_path(path, setting->ctl_index) != -1) {
        struct mixer_ctl *ctl = index_to_ctl(ur, setting->ctl_index);

        ALOGE("Control '%s' already exists in path '%s'",
              mixer_ctl_get_name(ctl), path->name);
        return -1;
    }

    path_index = alloc_path_setting(path);
    if (path_index < 0)
        return -1;

    path->setting[path_index].ctl_index = setting->ctl_index;
    path->setting[path_index].num_values = setting->num_values;
    path->setting[path_index].value = malloc(setting->num_values * sizeof(int));
    /* copy all values */
    memcpy(path->setting[path_index].value, setting->value,
           setting->num_values * sizeof(int));

    return 0;
}

static int path_add_value(struct usnd_route *ur, struct mixer_path *path,
                          struct mixer_value *mixer_value)
{
    unsigned int i;
    int path_index;
    unsigned int num_values;
    struct mixer_ctl *ctl;

    /* Check that mixer value index is within range */
    ctl = index_to_ctl(ur, mixer_value->ctl_index);
    num_values = mixer_ctl_get_num_values(ctl);
    if (mixer_value->index >= (int)num_values) {
        ALOGE("mixer index %d is out of range for '%s'", mixer_value->index,
              mixer_ctl_get_name(ctl));
        return -1;
    }

    path_index = find_ctl_index_in_path(path, mixer_value->ctl_index);
    if (path_index < 0) {
        /* New path */

        path_index = alloc_path_setting(path);
        if (path_index < 0)
            return -1;

        /* initialise the new path setting */
        path->setting[path_index].ctl_index = mixer_value->ctl_index;
        path->setting[path_index].num_values = num_values;
        path->setting[path_index].value = malloc(num_values * sizeof(int));
        path->setting[path_index].value[0] = mixer_value->value;
    }

    if (mixer_value->index == -1) {
        /* set all values the same */
        for (i = 0; i < num_values; i++)
            path->setting[path_index].value[i] = mixer_value->value;
    } else {
        /* set only one value */
        path->setting[path_index].value[mixer_value->index] = mixer_value->value;
    }

    return 0;
}

static int path_add_path(struct usnd_route *ur, struct mixer_path *path,
                         struct mixer_path *sub_path)
{
    unsigned int i;

    if (NULL == sub_path)
    {
        return -1;
    }

    for (i = 0; i < sub_path->length; i++)
        if (path_add_setting(ur, path, &sub_path->setting[i]) < 0)
            return -1;

    return 0;
}

static int path_apply(struct usnd_route *ur, struct mixer_path *path)
{
    unsigned int i;
    unsigned int ctl_index;
    unsigned int ind = 0;

    for (i = 0; i < path->length; i++) {
        ctl_index = path->setting[i].ctl_index;

        /* apply the new value(s) */
        memcpy(ur->mixer_state[ctl_index].new_value, path->setting[i].value,
               path->setting[i].num_values * sizeof(int));
        /* store order of the control */
        if (ur->ctls_counter < ur->max_ctls_amount) {
            ind = ur->ctls_counter++;
            ur->ctls_indexes[ind] = ctl_index;
        }
    }

    return 0;
}

static int path_reset(struct usnd_route *ur, struct mixer_path *path)
{
    unsigned int i;
    unsigned int j;
    unsigned int ctl_index;

    for (i = 0; i < path->length; i++) {
        ctl_index = path->setting[i].ctl_index;

        /* reset the value(s) */
        memcpy(ur->mixer_state[ctl_index].new_value,
               ur->mixer_state[ctl_index].reset_value,
               ur->mixer_state[ctl_index].num_values * sizeof(int));
    }

    return 0;
}

/* mixer helper function */
static int mixer_enum_string_to_value(struct mixer_ctl *ctl, const char *string)
{
    unsigned int i;

    /* Search the enum strings for a particular one */
    for (i = 0; i < mixer_ctl_get_num_enums(ctl); i++) {
        char *enum_string = mixer_ctl_get_enum_string(ctl, i);
        if (NULL != enum_string)
        {
          if (strcmp(enum_string, string) == 0)
              break;
        }
    }

    return i;
}

static void start_tag(void *data, const XML_Char *tag_name,
                      const XML_Char **attr)
{
    const XML_Char *attr_name = NULL;
    const XML_Char *attr_id = NULL;
    const XML_Char *attr_value = NULL;
    struct config_parse_state *state = data;
    struct usnd_route *ur = state->ur;
    unsigned int i;
    unsigned int ctl_index;
    struct mixer_ctl *ctl;
    int value;
    unsigned int id;
    struct mixer_value mixer_value;

    /* Get name, id and value attributes (these may be empty) */
    for (i = 0; attr[i]; i += 2) {
        if (strcmp(attr[i], "name") == 0)
            attr_name = attr[i + 1];
        if (strcmp(attr[i], "id") == 0)
            attr_id = attr[i + 1];
        else if (strcmp(attr[i], "value") == 0)
            attr_value = attr[i + 1];
    }

    /* Look at tags */
    if (strcmp(tag_name, "path") == 0) {
        if (attr_name == NULL) {
            ALOGE("Unnamed path!");
        } else {
            if (state->level == 1) {
                /* top level path: create and stash the path */
                state->path = path_create(ur, (char *)attr_name);
            } else {
                /* nested path */
                struct mixer_path *sub_path = path_get_by_name(ur, attr_name);
                path_add_path(ur, state->path, sub_path);
            }
        }
    }

    else if (strcmp(tag_name, "ctl") == 0) {
        if (NULL == attr_name || NULL == attr_value) {
            ALOGE("Control is missing an attribute - skipping");
            goto done;
        }
        /* Obtain the mixer ctl and value */
        ctl = mixer_get_ctl_by_name(ur->mixer, attr_name);
        if (ctl == NULL) {
            ALOGE("Control '%s' doesn't exist - skipping", attr_name);
            goto done;
        }

        switch (mixer_ctl_get_type(ctl)) {
        case MIXER_CTL_TYPE_BOOL:
        case MIXER_CTL_TYPE_INT:
            value = atoi((char *)attr_value);
            break;
        case MIXER_CTL_TYPE_ENUM:
            value = mixer_enum_string_to_value(ctl, (char *)attr_value);
            break;
        default:
            value = 0;
            break;
        }

        /* locate the mixer ctl in the list */
        for (ctl_index = 0; ctl_index < ur->num_mixer_ctls; ctl_index++) {
            if (ur->mixer_state[ctl_index].ctl == ctl)
                break;
        }

        if (state->level == 1) {
            /* top level ctl (initial setting) */

            /* apply the new value */
            if (attr_id) {
                /* set only one value */
                id = atoi((char *)attr_id);
                if (id < ur->mixer_state[ctl_index].num_values)
                    ur->mixer_state[ctl_index].new_value[id] = value;
                else
                    ALOGE("value id out of range for mixer ctl '%s'",
                          mixer_ctl_get_name(ctl));
            } else {
                /* set all values the same */
                for (i = 0; i < ur->mixer_state[ctl_index].num_values; i++)
                    ur->mixer_state[ctl_index].new_value[i] = value;
            }
        } else {
            /* nested ctl (within a path) */
            mixer_value.ctl_index = ctl_index;
            mixer_value.value = value;
            if (attr_id)
                mixer_value.index = atoi((char *)attr_id);
            else
                mixer_value.index = -1;
            path_add_value(ur, state->path, &mixer_value);
        }
    }

done:
    state->level++;
}

static void end_tag(void *data, const XML_Char *tag_name)
{
    struct config_parse_state *state = data;

    state->level--;
}

static int alloc_mixer_state(struct usnd_route *ur)
{
    unsigned int i;
    unsigned int j;
    unsigned int num_values;
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;

    ur->num_mixer_ctls = mixer_get_num_ctls(ur->mixer);
    ur->mixer_state = malloc(ur->num_mixer_ctls * sizeof(struct mixer_state));
    if (!ur->mixer_state)
        return -1;

    for (i = 0; i < ur->num_mixer_ctls; i++) {
        ctl = mixer_get_ctl(ur->mixer, i);
        num_values = mixer_ctl_get_num_values(ctl);

        ur->mixer_state[i].ctl = ctl;
        ur->mixer_state[i].num_values = num_values;

        /* Skip unsupported types that are not supported yet in XML */
        type = mixer_ctl_get_type(ctl);
        if ((type != MIXER_CTL_TYPE_BOOL) && (type != MIXER_CTL_TYPE_INT) &&
            (type != MIXER_CTL_TYPE_ENUM)) {
            ur->mixer_state[i].old_value = NULL;
            ur->mixer_state[i].new_value = NULL;
            ur->mixer_state[i].reset_value = NULL;
            continue;
        }
        ur->mixer_state[i].old_value = malloc(num_values * sizeof(int));
        ur->mixer_state[i].new_value = malloc(num_values * sizeof(int));
        ur->mixer_state[i].reset_value = malloc(num_values * sizeof(int));

        if (type == MIXER_CTL_TYPE_ENUM)
            ur->mixer_state[i].old_value[0] = mixer_ctl_get_value(ctl, 0);
        else
            mixer_ctl_get_array(ctl, ur->mixer_state[i].old_value, num_values);
        memcpy(ur->mixer_state[i].new_value, ur->mixer_state[i].old_value,
               num_values * sizeof(int));
    }

    return 0;
}

static void free_mixer_state(struct usnd_route *ur)
{
    unsigned int i;

    for (i = 0; i < ur->num_mixer_ctls; i++) {
        free(ur->mixer_state[i].old_value);
        free(ur->mixer_state[i].new_value);
        free(ur->mixer_state[i].reset_value);
    }

    free(ur->mixer_state);
    ur->mixer_state = NULL;
}

/* Update the mixer control */
static void update_ctl(struct usnd_route *ur, unsigned int i)
{
    unsigned int j;
    struct mixer_ctl *ctl;
    unsigned int num_values = ur->mixer_state[i].num_values;
    enum mixer_ctl_type type;

    ctl = ur->mixer_state[i].ctl;

    /* Skip unsupported types */
    type = mixer_ctl_get_type(ctl);
    if ((type != MIXER_CTL_TYPE_BOOL) && (type != MIXER_CTL_TYPE_INT) &&
        (type != MIXER_CTL_TYPE_ENUM))
         return;

    /* if the value has changed, update the mixer */
    bool changed = false;
    for (j = 0; j < num_values; j++) {
         if (ur->mixer_state[i].old_value[j] != ur->mixer_state[i].new_value[j]) {
             changed = true;
             break;
         }
    }
    if (changed) {
        if (type == MIXER_CTL_TYPE_ENUM)
            mixer_ctl_set_value(ctl, 0, ur->mixer_state[i].new_value[0]);
        else
            mixer_ctl_set_array(ctl, ur->mixer_state[i].new_value, num_values);
        memcpy(ur->mixer_state[i].old_value, ur->mixer_state[i].new_value,
               num_values * sizeof(int));
    }
}

/* Update the mixer with any changed values */
int usnd_route_update_mixer(struct usnd_route *ur)
{
    unsigned int i;
    unsigned int j;
    struct mixer_ctl *ctl;

    if (ur->ctls_counter) {
        for (i = 0; i < ur->ctls_counter; i++)
             update_ctl(ur, ur->ctls_indexes[i]);
        ur->ctls_counter = 0;
    }
    else
        for (i = 0; i < ur->num_mixer_ctls; i++)
             update_ctl(ur, i);

    return 0;
}

/* saves the current state of the mixer, for resetting all controls */
static void save_mixer_state(struct usnd_route *ur)
{
    unsigned int i;
    enum mixer_ctl_type type;
    struct mixer_ctl *ctl;

    for (i = 0; i < ur->num_mixer_ctls; i++) {
        ctl = ur->mixer_state[i].ctl;
        type = mixer_ctl_get_type(ctl);

        if ((type != MIXER_CTL_TYPE_BOOL) && (type != MIXER_CTL_TYPE_INT) &&
            (type != MIXER_CTL_TYPE_ENUM)) {
            ALOGV("%s skip unsupported ctl %s", __func__, mixer_ctl_get_name(ctl));
            continue;
        }
        memcpy(ur->mixer_state[i].reset_value, ur->mixer_state[i].new_value,
               ur->mixer_state[i].num_values * sizeof(int));
    }
}

/* Reset the audio routes back to the initial state */
void usnd_route_reset(struct usnd_route *ur)
{
    unsigned int i;

    /* load all of the saved values */
    for (i = 0; i < ur->num_mixer_ctls; i++) {
        memcpy(ur->mixer_state[i].new_value, ur->mixer_state[i].reset_value,
               ur->mixer_state[i].num_values * sizeof(int));
    }
}

/* Apply an audio route path by name */
int usnd_route_apply_path(struct usnd_route *ur, const char *name)
{
    struct mixer_path *path;

    if (!ur) {
        ALOGE("invalid usnd_route");
        return -1;
    }

    path = path_get_by_name(ur, name);
    if (!path) {
        ALOGE("unable to find path '%s'", name);
        return -1;
    }

    path_apply(ur, path);

    return 0;
}

/* Reset an audio route path by name */
int usnd_route_reset_path(struct usnd_route *ur, const char *name)
{
    struct mixer_path *path;

    if (!ur) {
        ALOGE("invalid usnd_route");
        return -1;
    }

    path = path_get_by_name(ur, name);
    if (!path) {
        ALOGE("unable to find path '%s'", name);
        return -1;
    }

    path_reset(ur, path);

    return 0;
}

struct usnd_route *usnd_route_init_ext(unsigned int card,
                                         const char *xml_path,
                                         unsigned int max_ctls_amount)
{
    struct config_parse_state state;
    XML_Parser parser;
    FILE *file;
    int bytes_read;
    void *buf;
    int i;
    struct usnd_route *ur;

    ur = calloc(1, sizeof(struct usnd_route));
    if (!ur)
        goto err_calloc;

    /* allocate space for ordered indexes of mixer controls */
    ur->ctls_indexes = NULL;
    ur->max_ctls_amount = max_ctls_amount;
    ur->ctls_counter = 0;
    if (max_ctls_amount) {
        ur->ctls_indexes = malloc(max_ctls_amount * sizeof(unsigned int));
        if (!(ur->ctls_indexes)) {
            ALOGE("Controls indexes (%d) allocation failed", max_ctls_amount);
            goto err_ctls_alloc;
        }
    }

    ur->mixer = mixer_open(card);
    if (!ur->mixer) {
        ALOGE("Unable to open the mixer, aborting.");
        goto err_mixer_open;
    }

    ur->mixer_path = NULL;
    ur->mixer_path_size = 0;
    ur->num_mixer_paths = 0;

    /* allocate space for and read current mixer settings */
    if (alloc_mixer_state(ur) < 0)
        goto err_mixer_state;

    /* use the default XML path if none is provided */
    if (xml_path == NULL)
        xml_path = MIXER_XML_PATH;

    file = fopen(xml_path, "r");

    if (!file) {
        ALOGE("Failed to open %s", xml_path);
        goto err_fopen;
    }

    parser = XML_ParserCreate(NULL);
    if (!parser) {
        ALOGE("Failed to create XML parser");
        goto err_parser_create;
    }

    memset(&state, 0, sizeof(state));
    state.ur = ur;
    XML_SetUserData(parser, &state);
    XML_SetElementHandler(parser, start_tag, end_tag);

    for (;;) {
        buf = XML_GetBuffer(parser, BUF_SIZE);
        if (buf == NULL)
            goto err_parse;

        bytes_read = fread(buf, 1, BUF_SIZE, file);
        if (bytes_read < 0)
            goto err_parse;

        if (XML_ParseBuffer(parser, bytes_read,
                            bytes_read == 0) == XML_STATUS_ERROR) {
            ALOGE("Error in mixer xml (%s)", MIXER_XML_PATH);
            goto err_parse;
        }

        if (bytes_read == 0)
            break;
    }

    /* apply the initial mixer values, and save them so we can reset the
       mixer to the original values */
    usnd_route_update_mixer(ur);
    save_mixer_state(ur);

    XML_ParserFree(parser);
    fclose(file);
    return ur;

err_parse:
    XML_ParserFree(parser);
err_parser_create:
    fclose(file);
err_fopen:
    free_mixer_state(ur);
err_mixer_state:
    mixer_close(ur->mixer);
err_mixer_open:
    free(ur->ctls_indexes);
    ur->ctls_indexes = NULL;
err_ctls_alloc:
    free(ur);
    ur = NULL;
err_calloc:
    return NULL;
}

struct usnd_route *usnd_route_init(unsigned int card,
                                     const char *xml_path)
{
  return usnd_route_init_ext(card, xml_path, 0);
}

void usnd_route_free(struct usnd_route *ur)
{
    free_mixer_state(ur);
    mixer_close(ur->mixer);
    free(ur->ctls_indexes);
    free(ur);
}
