/*===========================================================================
*  @file UIBCHIDInjector.cpp
 *
 *  @par  DESCRIPTION:
*             Class Definition of the UIBC HID Injector (Wifi Display Source)
*             Contains interfaces and members to inject HID data to the HID
*             subsystem of the kernel. This is Linux/Android specific.

Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
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

#include "UIBCHIDInjector.h"
#include "MMMemory.h"
#include "MMThread.h"
#include "MMTimer.h"
#include <threads.h>
#include <pthread.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "UIBC_HID_INJECTOR"
#define WFD_UIBC_HID_POLL_THREAD_PRIORITY -8
#define HID_NULL_CHECK(x, y) if(x == NULL) {                                 \
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,y);                            \
        return -EINVAL;                                                      \
}

//The report descriptors are standard report descriptors as outlined in
//www.usb.org/developers/devclass_docs/HID1_11.pdf.

//As specified in E.10 of  USB_HID v1.11
static unsigned char defMouseRepDesc[50] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x01,                    //     INPUT (Cnst,Ary,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
    0xc0,                          //     END_COLLECTION
    0xc0                           // END_COLLECTION
};

//As specified in E.6 of  USB_HID v1.11
//With modifications to maximum values
// for supporting extra keys
static unsigned char defKeyboardRepDesc[63] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x01,                    //   INPUT (Cnst,Ary,Abs) ;Reserved Byte
    0x95, 0x05,                    //   REPORT_COUNT (5)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x05, 0x08,                    //   USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x03,                    //   REPORT_SIZE (3)
    0x91, 0x01,                    //   OUTPUT (Cnst,Ary,Abs)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0xff,                    //   LOGICAL_MAXIMUM (255)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0xff,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0                           // END_COLLECTION
};

/*=============================================================================

         FUNCTION:          CTOR

         DESCRIPTION:
           @brief          Constructor for UIBCHIDInjector class

         DEPENDENCIES:
                            None

         PARAMETERS:
          @param[in] None

         RETURN VALUE:
           @return None

        SIDE EFFECTS:
                            None
=============================================================================*/

UIBCHIDInjector::UIBCHIDInjector():
mRepDesc(NULL),
m_eStatus(HID_INVALID_STATE),
mFd(-1),
m_pPollThreadHandle(NULL),
mHIDType(-1)
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"UIBCHIDInjector ctor()");
    if(setup_HID()) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Something really bad happened");
    }
}


/*=============================================================================

         FUNCTION:          DTOR

         DESCRIPTION:
           @brief          Constructor for UIBCHIDInjector class

         DEPENDENCIES:
                            None

         PARAMETERS:
          @param[in] None

         RETURN VALUE:
           @return None

        SIDE EFFECTS:
                            None
=============================================================================*/


UIBCHIDInjector::~UIBCHIDInjector()
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"UIBCHIDInjector dtor()");
    if(m_eStatus == HID_INITIALIZED) {
        //simply close the uhid-dev node
        close(mFd);
        mFd = -1;
        m_eStatus = HID_DEINITIALIZED;
        if(m_pPollThreadHandle) {
          int exitCode = -1;
          MM_Thread_Join(m_pPollThreadHandle,&exitCode);
          m_pPollThreadHandle = NULL;
        }
        m_eStatus = HID_INVALID_STATE;
        return;
    }
    if(m_eStatus!=HID_INVALID_STATE) {
        destroy_uhid_device();
    }
}

/*=============================================================================

         FUNCTION:          setup_HID

         DESCRIPTION:
           @brief          Function for setting up the uhid-device. It opens
                           the uhid driver and spawns the event poll thread.
                           Once it returns it guarantees that the HID state
                           machine is in HID_INITIALIZED state with both the
                           uhid-dev node and the poll thread setup properly

         DEPENDENCIES:
                            None

         PARAMETERS:
          @param[in] None

         RETURN VALUE:
           @return returns 0 on success, -1 on failure

        SIDE EFFECTS:
                            None
=============================================================================*/

int UIBCHIDInjector::setup_HID()
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"setup_HID()");
    if(m_eStatus == HID_INVALID_STATE) {
        if(mFd != -1) {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"!!!uhid-dev is already opened!!!");
            return -1;
        }
        //Try to open the uhid node
       mFd = open(UIBC_UHID_DEV_PATH, O_RDWR | O_CLOEXEC);
       if (mFd < 0) {
           mFd = -1;
           MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_ERROR, "Cannot open uhid-dev %s due to %s", UIBC_UHID_DEV_PATH, strerror(errno));
           return -1;
       }
       if(m_pPollThreadHandle) {
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"!!! HID Poll thread already created !!!");
          return -1;
       }
       //Spawn a thread to poll the uhid device for receiving uhid events
       int32 threadStatus = MM_Thread_Create(MM_Thread_DefaultPriority,
                                          0, HID_poll_thread_entry, (void *) this,
                                          UIBC_HID_POLL_THREAD_STACK_SIZE, &m_pPollThreadHandle);
       if(threadStatus) {
          //Something is terribly wrong here
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"!!!Poll thread creation failed!!!");
          close(mFd);
          mFd = -1;
          m_eStatus = HID_INVALID_STATE;
          return -1;
       }
       while(m_eStatus != HID_INITIALIZED) {
            MM_Timer_Sleep(2);
       }
       MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR, "setup_HID() successful");
    } else {
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR, "setup_HID() called in invalid state %d",m_eStatus);
        return -1;
    }
    return 0;
}

/*=============================================================================

         FUNCTION:          uhid_write

         DESCRIPTION:
           @brief          Function for sending uhid events to uhid device

         DEPENDENCIES:
                            None

         PARAMETERS:
          @param[in] ev The uhid event that is to be written to uhid
                              device

         RETURN VALUE:
           @return return value of write to uhid device

        SIDE EFFECTS:
                            None
=============================================================================*/

int UIBCHIDInjector::uhid_write(const struct uhid_event *ev)
{
    ssize_t ret;
    if(!ev) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR, "!!! Cannot write null event to uhid device!!!");
        return -EINVAL;
    }
    ret = write(mFd, ev, sizeof(uhid_event));
    if (ret < 0) {
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR, "!!! Cannot write to uhid device %s!!!", strerror(errno));
        return -errno;
    } else if (ret != sizeof(*ev)) {
        MM_MSG_PRIO3(MM_GENERAL,MM_PRIO_ERROR, "Wrong size written to uhid: %d != %u error: %s",\
            ret, sizeof(ev), strerror(errno));
        return -errno;
    } else {
        return 0;
    }
}


/*=============================================================================

         FUNCTION:          create_uhid_device

         DESCRIPTION:
           @brief          Function for creating and registering a uhid device
                             with HID susbsystem, once it returns successfully
                             it guarantees that the device is created i.e. HID
                             state machine is in HID_DEV_CREATED state

         DEPENDENCIES:
                            None

         PARAMETERS:
          @param[in] None

         RETURN VALUE:
           @return 0 on successful uhid device creation and registry, else -1

        SIDE EFFECTS:
                            None
=============================================================================*/

int UIBCHIDInjector::create_uhid_device()
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"create_uhid_device()");
    if(mRepDesc == NULL || mRepDesc->repDesc == NULL || mRepDesc->len == 0) {
       //Don't inject bogus device specs into kernel!
       MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Something really fishy");
       return -1;
    }
    if(m_eStatus == HID_INITIALIZED) {
       struct uhid_event ev;
       memset(&ev, 0, sizeof(ev));
       ev.type = UHID_CREATE;
       strlcpy((char*)ev.u.create.name, UIBC_UHID_DEVICE_NAME,sizeof(ev.u.create.name));
       ev.u.create.rd_data = mRepDesc->repDesc;
       ev.u.create.rd_size = static_cast<uint16>(mRepDesc->len) ;
       ev.u.create.bus = BUS_USB; //in kernel only BUS_USB and BUS_BT are defined
       ev.u.create.vendor = UIBC_HID_VENDOR;
       ev.u.create.product = UIBC_HID_PRODUCT;
       m_eStatus = HID_DEV_CREATING;
       if(uhid_write(&ev)) {
          return -1;
       }
       //Wait for notification from uhid-dev for the device to be created
       while(m_eStatus != HID_DEV_CREATED) {
            MM_Timer_Sleep(2);
       }
       MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"HID Device creation successful");
    } else {
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR,"Invalid state %d to create uhid device",m_eStatus);
        return -1;
    }
    return 0;
}

/*=============================================================================

         FUNCTION:          destroy_uhid_device

         DESCRIPTION:
           @brief          Function for destroying a device registered with the
                             HID susbystem and cleaning up all the allocated
                             resources. Once it returns it guarantees that the
                             HID state machine is in HID_INVALID_STATE

         DEPENDENCIES:
                            None

         PARAMETERS:
          @param[in] None

         RETURN VALUE:
           @return return 0 on success, -1 on failure

        SIDE EFFECTS:
                            None
=============================================================================*/

int UIBCHIDInjector::destroy_uhid_device()
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"destroy_uhid_device()");
    if(m_eStatus == HID_DEV_CREATED) {
       struct uhid_event ev;
       memset(&ev, 0, sizeof(ev));
       ev.type = UHID_DESTROY;
       m_eStatus = HID_DEV_DESTROYING;
       if (0 == uhid_write(&ev)) {
           while(m_eStatus != HID_DEV_DESTOYED) {
                MM_Timer_Sleep(2);
           }
       }
       if(mFd>0) {
        close(mFd);
        mFd = -1;
       }
       if(mRepDesc) {
         delete mRepDesc;
         mRepDesc = NULL;
       }
       m_eStatus = HID_DEINITIALIZED;
       if(m_pPollThreadHandle) {
          int exitCode = -1;
          MM_Thread_Join(m_pPollThreadHandle,&exitCode);
          m_pPollThreadHandle = NULL;
       }
       MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Performed clean up after destroying device");
       m_eStatus = HID_INVALID_STATE;
       return 0;
    } else {
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR,"Invalid state %d to destroy uhid device",m_eStatus);
    }
    return -1;
}

/*=============================================================================

         FUNCTION:          HID_poll_thread_entry

         DESCRIPTION:
           @brief          Function providing for entry point to the HID poll
                             thread

         DEPENDENCIES:
                            None

         PARAMETERS:
          @param[in] pData pointer to the instance spawning the thread

         RETURN VALUE:
           @return return 0 on success, -1 on failure

        SIDE EFFECTS:
                            None
=============================================================================*/

int UIBCHIDInjector::HID_poll_thread_entry(void* pData)
{
    if (pData) {
        ((UIBCHIDInjector*) pData)->HID_poll_thread();
        return 0;
    }
    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR,"NULL data pointer passed to %s", __FUNCTION__);
    return -1;
}

/*=============================================================================

         FUNCTION:          HID_poll_thread

         DESCRIPTION:
           @brief          Function for HID poll thread

         DEPENDENCIES:
                            None

         PARAMETERS:
          @param[in] None

         RETURN VALUE:
           @return None

        SIDE EFFECTS:
                            None
=============================================================================*/


void UIBCHIDInjector::HID_poll_thread()
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"In HID_poll_thread");
    //If ever rechristening of this thread is implemented bear in
    //mind that there is an upper limit to the name (as of now 16)
    int ret = pthread_setname_np(pthread_self(),"HID_Poll_Thread");
    if(ret)
    {
        MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_ERROR,"Failed to set thread name due to %d %s",
                    ret, strerror(errno));
        //No need to panic
    }
    m_eStatus = HID_INITIALIZED;
    struct pollfd pfds[1];
    int tid = androidGetTid();
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,"HID Poll thread priority b4 %d ",
                 androidGetThreadPriority(tid));
    androidSetThreadPriority(0, WFD_UIBC_HID_POLL_THREAD_PRIORITY);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "HID Poll thread priority after %d ",
                 androidGetThreadPriority(tid));
    ret = -1;
    pfds[0].fd = mFd;
    pfds[0].events = POLLIN;
    //right now POLLIN seems to be the only interesting option to worry about
    while(m_eStatus != HID_DEINITIALIZED){
        ret = poll(pfds, 1, 500);
        if (ret < 0) {
            MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR,"Cannot poll for fds: %s\n",strerror(errno));
            if(errno != EINTR) {
                //Since something's up in the system, don't leak the fd, close it
                //No need to send destroy event to uhid device since closing fd
                //automatically guarantees destruction and clean up
               close(mFd);
               break;
            }
            //For some reason an interrupt signal was delivered, so don't (f)break out yet
        }
        if (pfds[0].revents & POLLIN) {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR, "Received event from uhid device");
            ret = read_uhid_event();
            if (ret){
                break;
            }
        }
    }
    MM_Thread_Exit(m_pPollThreadHandle,0);
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Exiting HID_poll_thread");
}

/*=============================================================================

         FUNCTION:          read_uhid_event

         DESCRIPTION:
           @brief          Function for reading uhid events from uhid device

         DEPENDENCIES:
                            None

         PARAMETERS:
          @param[in] None

         RETURN VALUE:
           @return 0 on succes, -1 on failure

        SIDE EFFECTS:
                            None
=============================================================================*/

int UIBCHIDInjector::read_uhid_event()
{
    struct uhid_event ev;
    ssize_t ret;
    memset(&ev, 0, sizeof(ev));
    ret = read(mFd, &ev, sizeof(ev));
    if (ret <= 0) {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "Error in reading from uhid-dev with error: %s",strerror(errno));
        return -errno;
    } else if (ret != sizeof(ev)) {
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "Invalid size read from uhid-dev: %d != %u",
            ret, sizeof(ev));
        return -EFAULT;
    }
    //read was uneventful atleast! Now read the actual event delivered from uhid
    switch (ev.type) {
    case UHID_START:
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UHID_START from uhid-dev");
        m_eStatus = HID_DEV_CREATED;
        break;
    case UHID_STOP:
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UHID_STOP from uhid-dev");
            m_eStatus = HID_DEV_DESTOYED;
        break;
        //ToDo: May need to handle these uhid events in the future
    case UHID_OPEN:
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UHID_OPEN from uhid-dev");
        break;
    case UHID_CLOSE:
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UHID_CLOSE from uhid-dev");
        break;
    case UHID_OUTPUT:
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UHID_OUTPUT from uhid-dev");
        break;
    case UHID_OUTPUT_EV:
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UHID_OUTPUT_EV from uhid-dev");
        break;
    case UHID_FEATURE:
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UHID_FEATURE from uhid-dev");
        break;
    default:
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "Invalid event from uhid-dev: %u", ev.type);
    }
    return 0;
}

/*=============================================================================

         FUNCTION:          send_report

         DESCRIPTION:
           @brief          Function for sending HID reports to the HID subsys
                             through the uhid-dev node

         DEPENDENCIES:
                            None

         PARAMETERS:
          @param[in] report The report to be sent to the HID subsys

          @param [in] len The lenght of the report to be sent

         RETURN VALUE:
           @return 0 on succes, -1 on failure

        SIDE EFFECTS:
                            None
=============================================================================*/

int UIBCHIDInjector::send_report(uint8* report, size_t len)
{
    HID_NULL_CHECK(report,"Report is null");
    //Wait for the device to be created, only then can a report be sent to UHID
    if(m_eStatus!= HID_DEV_CREATED){
       MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR, "cannot send report in invalid state %d",m_eStatus);
       return -1;
    }
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"Sending report to uhid device");
    struct uhid_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = UHID_INPUT;
    ev.u.input.size = static_cast<uint16>(len);
    if(len > sizeof(ev.u.input.data)){
        //There is a max to the data that can be pushed in one shot
        //defined as UHID_DATA_MAX in uhid.h
       MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR,"Report size %u greater than allowed size %d",len,UHID_DATA_MAX);
       return -1;
    }
    memcpy(ev.u.input.data, report, len);
    return uhid_write(&ev);
}

/*=============================================================================

         FUNCTION:          setup_uhid_device

         DESCRIPTION:
           @brief          Function for setting registering a device with the
                           HID susbystem and cleaning up all the allocated
                           resources. Once it returns it guarantees that the
                           HID state machine is in HID_DEV_CREATED

         DEPENDENCIES:
                            Should be called only once HID state machine has
                            setup and moved to HID_INITIALIZED

         PARAMETERS:
          @param[in] rDesc The report descriptor of the device, use NULL to
                          indicate that a default report descriptor is to be used

          @param[in] len The length of the report descriptor, should be 0 if
                          a default resport descriptor is to be used

         RETURN VALUE:
           @return return 0 on success, -1 on failure

        SIDE EFFECTS:
                            None
=============================================================================*/

int UIBCHIDInjector::setup_uhid_device(unsigned char* rDesc, size_t len)
{
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"%s",__FUNCTION__);
    if(NULL!= mRepDesc) {
        delete mRepDesc;
    }
    // WARNING: DO NOT do a null check on incoming rDesc
    mRepDesc = new hidRepDesc();
    HID_NULL_CHECK(mRepDesc,"Memory allocation failed for mRepDesc");
    if(m_eStatus == HID_INITIALIZED) {
        if(rDesc) {
           mRepDesc->repDesc = new unsigned char [len];
           HID_NULL_CHECK(mRepDesc->repDesc,"Memory allocation failed for mRepDesc->repDesc");
           mRepDesc->len = len;
           memcpy(mRepDesc->repDesc,rDesc,len);
        } else if(rDesc == NULL && len == 0) {
            //Device setup was requested w/o a Reprt Descriptor,
            //hence use default report descriptors as specified in 4.11.3.2 of
            //Wi-Fi Display Specification
            if(mHIDType == HID_KEYBOARD) {
               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"Setting rdesc for Keyboard");
               mRepDesc->repDesc = new unsigned char [sizeof(defKeyboardRepDesc)];
               HID_NULL_CHECK(mRepDesc->repDesc,"Memory allocation failed for mRepDesc->repDesc");
               mRepDesc->len = sizeof(defKeyboardRepDesc);
               memcpy(mRepDesc->repDesc,defKeyboardRepDesc,sizeof(defKeyboardRepDesc));
            } else if (mHIDType == HID_MOUSE) {
               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"Setting rdesc for Mouse");
               mRepDesc->repDesc = new unsigned char [sizeof(defMouseRepDesc)];
               HID_NULL_CHECK(mRepDesc->repDesc,"Memory allocation failed for mRepDesc->repDesc");
               mRepDesc->len = sizeof(defMouseRepDesc);
               memcpy(mRepDesc->repDesc,defMouseRepDesc,sizeof(defMouseRepDesc));
            } else {
                //As of latest spec there are no default report descriptors
                //for other HID types, hence throw an error
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"!!!Invalid argument, \
                        Report Descriptor for device % d cannot be NULL!!!", mHIDType);
                return -EINVAL;
            }
        } else {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"Invalid args passed to %s ",__FUNCTION__);
        }
        return create_uhid_device();
    } else {
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR,"Invalid state %d for %s ", m_eStatus,__FUNCTION__);
    }
    return -1;
}

/*=============================================================================

         FUNCTION:          getStatus

         DESCRIPTION:
           @brief          Function to query the current status of the HID
                             state machine

         DEPENDENCIES:
                           None

         PARAMETERS:
          @param[in] None

         RETURN VALUE:
           @return The current state of the HID state machine (UIBC_HID_status)

        SIDE EFFECTS:
                            None
=============================================================================*/

UIBC_HID_status UIBCHIDInjector::getStatus() const
{
    return m_eStatus;
}

/*=============================================================================

         FUNCTION:          set_HID_type

         DESCRIPTION:
           @brief          Function to set the HID type (mouse/keyboard) and
                             preaparing the base for registering a device with the
                             HID subsytem. This function guarantees that once it
                             returns the HID state machine is in HID_INITIALIZED
                             state

         DEPENDENCIES:
                           None

         PARAMETERS:
          @param[in] hidType The HID type to be set as per the Wi-Fi display
                                      specification v 1.0.0 Table 4-17

         RETURN VALUE:
           @return None

        SIDE EFFECTS:
                            None
=============================================================================*/

int UIBCHIDInjector::set_HID_type(uint8 hidType)
{
    if(hidType != mHIDType) {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "set_HID_type called");
      //A request to set a new device type has arrived which implies that the
      //current device (if) registered with the HID subsystem nneds to be
      //destroyed
      if(m_eStatus == HID_INVALID_STATE) {
        //This is a fresh device that's going to be registered
      } else if(m_eStatus == HID_INITIALIZED) {
        //HID setup has been done, just set the current device type
        mHIDType = hidType;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"set HIDType to %d", mHIDType);
        return 0;
      } else if (m_eStatus == HID_DEV_CREATING) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"A device was being created, wait for it to be created then destroy it ");
        while(m_eStatus != HID_DEV_CREATED) {
            MM_Timer_Sleep(2);
        }
        destroy_uhid_device();
      } else if(m_eStatus == HID_DEV_CREATED) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Destroying existing device to prepare for a new one");
        destroy_uhid_device();
      }else if (m_eStatus == HID_DEV_DESTROYING) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"A device was being destroyeded, wait for it to be destroyed ");
        while(m_eStatus != HID_INVALID_STATE) {
            MM_Timer_Sleep(2);
        }
      }
      if (setup_HID()) {
        return -1;
      }
      mHIDType = hidType;
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"set HIDType to %d", mHIDType);
    }
    return 0;
}
