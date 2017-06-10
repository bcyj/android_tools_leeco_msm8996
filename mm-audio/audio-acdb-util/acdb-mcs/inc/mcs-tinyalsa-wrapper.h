/*
 * Copyright (c) 2014 QUALCOMM Technologies, Inc. All Rights Reserved.
 * QUALCOMM Technologies Proprietary and Confidential.
 *
 */
#ifndef MCS_TINY_ALSA_WRAPPER_H
#define MCS_TINY_ALSA_WRAPPER_H

#define get_ctl(mixer, name) mixer_get_ctl_by_name(mixer, name)
#define mixer_ctl_select(ctrl, value) mixer_ctl_set_enum_by_string(ctrl, value)
#define mixer_ctl_set(ctrl, val) mixer_ctl_set_value(ctrl, 0, val)
#endif
