/*=========================================================================
  wbc_hal_interface.h
  DESCRIPTION
  API declarations for Wipower Battery Control HAL

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  =========================================================================*/

#ifndef WBC_HAL_INTERFACE_H
#define WBC_HAL_INTERFACE_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <hardware/hardware.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WBC_HARDWARE_MODULE_ID "wbc_hal"

/* Status return values */
typedef enum {
    //WBC_STATUS_OK = 0,
    //WBC_STATUS_ERROR,
    WBC_ERROR_NONE = 0,
    WBC_ERROR_UNDEFINED
} wbc_error_t;

typedef wbc_error_t wbc_status_t;

/* Indicates whether the device has Wipower */
typedef enum {
    WBC_WIPOWER_INCAPABLE = 0,
    WBC_WIPOWER_CAPABLE,
} wbc_wipower_capable_t;

/* WiPower PTU Presence */
typedef enum {
    WBC_PTU_STATUS_NOT_PRESENT = 0,
    WBC_PTU_STATUS_PRESENT,
} wbc_ptu_presence_t;

/* WiPower charging status */
typedef enum {
    WBC_WIPOWER_STATUS_NOT_CHARGING = 0,
    WBC_WIPOWER_STATUS_CHARGING_ACTIVE,
} wbc_wipower_charging_t;

/* WiPower charging required status */
typedef enum {
    WBC_BATTERY_STATUS_CHARGING_NOT_REQUIRED = 0,
    WBC_BATTERY_STATUS_CHARGING_REQUIRED,
} wbc_chg_required_t;

/* Event type used in callback notification */
typedef enum {
    WBC_EVENT_TYPE_WIPOWER_CAPABLE_STATUS = 1,
    WBC_EVENT_TYPE_PTU_PRESENCE_STATUS,
    WBC_EVENT_TYPE_WIPOWER_CHARGING_ACTIVE_STATUS,
    WBC_EVENT_TYPE_CHARGING_REQUIRED_STATUS
} wbc_event_type_t;

/* WBC callback event */
typedef struct {
    wbc_event_type_t event_type;
    union {
        wbc_wipower_capable_t  wipower_capable;
        wbc_ptu_presence_t     ptu_presence;
        wbc_wipower_charging_t wipower_charging;
        wbc_chg_required_t     chg_required;
    } u;
} wbc_event_t;

/* WBC Callback function for notifying client */
typedef void (wbc_callback_t) (wbc_event_t * p_event);

/* WBC HAL Module Interface */
typedef struct wbc_hal_module {
    struct hw_module_t common;

    wbc_status_t (*init) (wbc_callback_t * wbc_callback);

    wbc_status_t (*finish) (void);

    void (*echo) (int); /* TODO: temporary for testing */

    wbc_wipower_capable_t (*get_wipower_capable) (void);

    wbc_ptu_presence_t (*get_ptu_presence) (void);

    wbc_wipower_charging_t (*get_wipower_charging) (void);

    wbc_chg_required_t (*get_chg_required) (void);
} wbc_hal_module_t;

#ifdef __cplusplus
}
#endif

#endif /* WBC_HAL_INTERFACE_H */

