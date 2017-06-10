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

#ifndef USND_ROUTE_H
#define USND_ROUTE_H

#if defined(__cplusplus)
extern "C" {
#endif

/* Initialize and free the audio routes */
struct usnd_route *usnd_route_init(unsigned int card,
                                     const char *xml_path);
void usnd_route_free(struct usnd_route *ar);

/* Prepare the router for support of forwarding mixer controls */
/* in order defined by a caller */
/* "max_ctls_amount" parameter defines max number of the controls, */
/* intended for the mixer updating */
struct usnd_route *usnd_route_init_ext(unsigned int card,
                                         const char *xml_path,
                                         unsigned int max_ctls_amount);

/* Apply an audio route path by name */
int usnd_route_apply_path(struct usnd_route *ur, const char *name);

/* Reset an audio route path by name */
int usnd_route_reset_path(struct usnd_route *ur, const char *name);

/* Reset the audio routes back to the initial state */
void usnd_route_reset(struct usnd_route *ur);

/* Update the mixer with any changed values */
int usnd_route_update_mixer(struct usnd_route *ur);

#if defined(__cplusplus)
}  /* extern "C" */
#endif

#endif
