/*
    Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*/

#ifndef __PM_SERVICE_H__
#define __PM_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define PERIPH_MAX_SUPPORTED    8

#define PM_RET_SUCCESS          0
#define PM_RET_FAILED           -1
#define PM_RET_UNSUPPORTED      1

enum pm_event {
    /*
       Peripheral hardware request shutdown/restart, and all connected
       clients have to be disconnected. Client should not really care
       whether this is because device restart request or whatever else.
     */
    EVENT_PERIPH_GOING_OFFLINE,
    /*
       Peripheral hardware is not powered on or it have been just powered
       off. When connected client receive this event it sould call
       acknowledge() functions. This is acknowledge from client that it have
       done with this peripheral and server can power down/restart device.
       When reference counter for this device drops to Zero (all connected
       clients have been disconnected), or a timeout has expired, Peripheral
       Manager will close device node, thus power off device for real.
     */
    EVENT_PERIPH_IS_OFFLINE,
    /*
       Event is sent to all registered clients when one of them have been
       requesting peripheral device to be powered on.
     */
    EVENT_PERIPH_GOING_ONLINE,
    /*
       Peripheral is on-line (after restart or just powered up for the
       first time). When clients receive this, they could call connect() if
       they want to use the device. If this is the first client,
       Peripheral Manager will open the device node controlled peripheral
       device, which powers up peripheal. Peripheral manager will
       notify all "peripheral device status listeners" that peripheral have
       been ONLINE (powered-up).
     */
    EVENT_PERIPH_IS_ONLINE,
};

/*
   Function returns number of available peripheral devices in the system
   and their names. It will return 0 if no peripherals are available,
   positive when there are devices which can be controlled by Peripheral
   Manager and negative number on error.
 */
int pm_show_peripherals(const char *names[PERIPH_MAX_SUPPORTED]);

/*
   Prototype of the function which have to be implemented from clients and
   passed to registar() call.
 */
typedef void (*pm_client_notifier) (void *, enum pm_event);

/*
   Function registers user for particular peripheral device. Events
   emitted from device trough QMI interface will be delivered to client
   via pm_client_notifier callback. On success the function will return
   PM_RET_SUCCESS and the handle will be a valid entry which should be used
   for subsequent transactions with peripheral manager.On failure either
   PM_RET_FAILED or PM_RET_UNSUPPORTED will be returned.
   @clientData is client managed structure.It will be passed as a first
   argument to above callback function.
   @state[out] - if non-NULL on succesfull registration it will
   hold current state of peripheral device.
 */
int pm_client_register(pm_client_notifier notifier, void *clientData,
                       const char *devName, const char *clientName,
                       enum pm_event *state, void **handle);

/*
   Functions should be executed from library users for every event that
   they receive from Peripheral Manager Service. This is an
   acknowledgement to the service, that event have been delivered and
   Peripheral Manager can continue with power-on or off procedure. Service
   schedules a timeout for every acknowledge, so after this timer ticks,
   ack is ignored and power on/off procedure is continued without waiting
   for the client.
   Return 0 on success or -1 on error
 */
int pm_client_event_acknowledge(void *clientId, enum pm_event event);

/*
   Function unregisters user from particular peripheral device.
   It is not allowed to unregister client which is still connected to
   peripheral
   Return 0 on success or -1 on error
 */
int pm_client_unregister(void *clientId);

/*
   Client requests peripheral to be powered up. If this is the first user
   of the peripheral device Peripheral Manager will open device node which
   controls device power otherwise the manager will just increment its
   internal reference counter.
   It is not allowed to connect the same client more than one time.
   Return 0 on success or -1 on error
 */
int pm_client_connect(void *clientId);

/*
   Client requests peripheral to be powered down. If this is last user of
   the peripheral device, Peripheral Manager will close device node which
   controls device power.
   It is not allowed to disconnect the same client more than one time.
   It is not allowed to disconnect client which is not connected
   Return 0 on success or -1 on error
 */
int pm_client_disconnect(void *clientId);

#ifdef __cplusplus
}
#endif
#endif                           /* __PM_SERVICE_H__ */
