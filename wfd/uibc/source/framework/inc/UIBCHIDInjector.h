#ifndef _UIBCHIDINJECTOR_H
#define _UIBCHIDINJECTOR_H

/*===========================================================================
*  @file UIBCHIDInjector.h
*
*  @par  DESCRIPTION:
*             Class Declaration of the UIBC HID Injector (Wifi Display Source)
*             Contains interfaces and members to inject HID data to the HID
*             subsystem of the kernel. This is Linux/Android specific.

*  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.

========================================================================*/


/*===========================================================================
                             Edit History

when       who         what, where, why
--------   ---         -------------------------------------------------------
09/08/13   IC          Created file.


============================================================================*/


/*===========================================================================
 Include Files
============================================================================*/

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <linux/uhid.h>
#include "UIBCDefs.h"
#include "MMDebugMsg.h"

#define UIBC_UHID_DEV_PATH "/dev/uhid"
#define UIBC_UHID_DEVICE_NAME "wfd-uhid-device"

#define UIBC_HID_VENDOR 0x5143  //ASCII for QC !
#define UIBC_HID_PRODUCT 0x776664  //ASCII for WFD !

#define UIBC_HID_POLL_THREAD_STACK_SIZE 8192

//HID Types as per Wi-Fi Display spec v 1.0.0 Table 4-17
#define   HID_KEYBOARD      0
#define   HID_MOUSE         1
#define   HID_SINGLETOUCH   2
#define   HID_MULTITOUCH    3
#define   HID_JOYSTICK      4
#define   HID_CAMERA        5
#define   HID_GESTURE       6
#define   HID_REMOTECONTROL 7

typedef enum {
    HID_INVALID_STATE,
    HID_INITIALIZED,
    HID_DEV_CREATING,
    HID_DEV_CREATED,
    HID_DEV_DESTROYING,
    HID_DEV_DESTOYED,
    HID_DEINITIALIZED
}UIBC_HID_status;

class UIBCHIDInjector {
    public:
    UIBCHIDInjector();
    ~UIBCHIDInjector();
    int setup_uhid_device(unsigned char* rDesc, size_t len);
    UIBC_HID_status getStatus() const;
    int send_report(uint8* report, size_t len);
    int set_HID_type(uint8 hidType);

    private:
    static int HID_poll_thread_entry(void*);
    void HID_poll_thread();
    int setup_HID();
    int uhid_write(const struct uhid_event *ev);
    int read_uhid_event();
    int create_uhid_device();
    int destroy_uhid_device();
    /* =====================================
    ** HID Report Descriptor
    ** ===================================== */
    typedef struct hid_rep_desc_t
    {
        hid_rep_desc_t()
        {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"hid_rep_desc_t ctor()");
            repDesc = NULL;
            len = 0;
        }
        ~hid_rep_desc_t()
        {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"hid_rep_desc_t dtor()");
            if(repDesc)
            {
                delete[] repDesc;
            }
        }
        unsigned char* repDesc;
        size_t len;
    }hidRepDesc;
    //The report desriptor to be used to create the uhid device
    hidRepDesc* mRepDesc;
    //The status of the UIBC HID state machine
    UIBC_HID_status m_eStatus;
    //The file descriptor of the uhid device
    int mFd;
    void* m_pPollThreadHandle;
    uint8 mHIDType;
};
#endif
