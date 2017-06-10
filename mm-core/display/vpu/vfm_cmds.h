/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
 /*****************************************************************************
   This file contains the commands and command inargs and outargs structures
   for "VFM process" api
 *****************************************************************************/

#ifndef VFM_CMDS_H
#define VFM_CMDS_H
#include <sys/types.h>
#include <utils/String8.h>
#include <binder/Parcel.h>
#include "gralloc_priv.h"
#include "IQService.h"
#include <media/msm_vpu.h>

using namespace android;

//Number of minimum Ints in the inArgs sessionId + videoFlowId + flags
#define VFM_MIN_IN_SIZE  (3)

//Number of minimum Ints in the outArgs
#define VFM_MIN_OUT_SIZE (0)

//Count the number of ints for a given inArgs structure
#define VFM_INT_COUNT(a) ( sizeof(a) / 4 + VFM_MIN_IN_SIZE )

//Maximum number of integers per command argument
//Account for huge payload VPU_MAX_EXT_DATA_SIZE and extra 10 ints
#define MAX_INT_PER_CMD_ARG (VPU_MAX_EXT_DATA_SIZE / 4 + VFM_MIN_IN_SIZE + 10)

//Invalid code
#define VFM_INVALID (-1)

/*****************************************************************************/
/* Enumerations                                                              */
/*****************************************************************************/
/*! Command list */
typedef enum VFM_COMMAND_TYPE
{
    //! Unused. Start and end range defined by hwc binder
    VFM_CMD_START   = qService::IQService::VPU_COMMAND_LIST_START + 1,

    //! Commands related to buffer called from the player
    VFM_CMD_BUFFER_START        = VFM_CMD_START,

    //! Register a pre-allocated buffer for VFM-tunnel session
    VFM_CMD_REGISTER_BUFFER     = VFM_CMD_BUFFER_START,

    //! Dequeue a free buffer, returns an id
    VFM_CMD_DQ_BUF,

    //! Queue buffer to be vpu processed
    VFM_CMD_Q_BUF,

    //! Cancel buffer. queue a buffer to VFM and request not to process it
    VFM_CMD_CANCEL_BUF,

    VFM_CMD_BUFFER_END          = VFM_CMD_CANCEL_BUF,

    //! Command to set the display attributes
    VFM_CMD_SET_DISPLAY_ATTRIBUTES,

    //! Generic set command to configure post processing parameters for VPU
    VFM_CMD_SET_VPU_CONTROL,

    //! Generic get command to query post processing parameters from VPU
    VFM_CMD_GET_VPU_CONTROL,

    //! Generic set command to configure huge payload for VPU
    VFM_CMD_SET_VPU_CONTROL_EXTENDED,

    //! Generic get command to configure huge payload for VPU
    VFM_CMD_GET_VPU_CONTROL_EXTENDED,

    //! Unused
    VFM_CMD_END                 = qService::IQService::VPU_COMMAND_LIST_END,

} VFM_COMMAND_TYPE;

typedef enum
{
    VFM_FLAG_SESSION_ID     = 0x01,

    /* Set this flag to indicate that the setting is global, applicable to
        all video flow ids or vpu sessions. */
    VFM_FLAG_ALL_SESSIONS   = 0x02,
} VFM_FLAGS;

/*! Event list. This is notified in callback notifyCb_t->eventHdlr */
typedef enum VFM_EVENT_TYPE
{
    VFM_EVT_START = 100,

    //! VPU has been setup for this session
    VFM_EVT_VPU_SETUP_DONE = VFM_EVT_START,

    //! VPU latency has changed for this session
    VFM_EVT_VPU_LATENCY_CHANGE,

    VFM_EVT_END = 200,
} VFM_EVENT_TYPE;

/*****************************************************************************/
/* Structures                                                                */
/*****************************************************************************/
//! InArgs struct CMD_REGISTER_BUFFER
struct vfmInArgBufReg_t {
    private_handle_t*   hnd; //<! gralloc handle
    int32_t             id;  //<! bufferId for subsequent Queuing and De-queuing
};

//! InArgs for CMD_Q_BUF
struct vfmInArgQBuf_t {
    int32_t     id;
};

//! InArgs for CMD_CANCEL_BUF
struct vfmInArgCancelBuf_t {
    int32_t     id;
};

//! InArgs for VFM_CMD_SET_VPU_CONTROL_EXTENDED and
//VFM_CMD_GET_VPU_CONTROL_EXTENDED
struct vfmInArgVpuControlExt_t {
    /* extended control type
     * 0: system
     * 1: session */
    uint32_t type;

    /*
     * size and ptr of the data to send
     * maximum VPU_MAX_EXT_DATA_SIZE bytes
     */
    uint32_t dataLength;
    char     dataBuffer[VPU_MAX_EXT_DATA_SIZE];

};

//! InArgs for VFM_CMD_SET_DISPLAY_ATTRIBUTES
struct vfmInArgSetDisplayAttr_t {
    //display id. refer to enum DISPLAY_ID
    int32_t     dpy;

    //width of the display in number of pixels
    int32_t     width;

    //height of the display in number of pixels
    int32_t     height;

    //frame rate of the display: 100 * frame rate. 3000 for 30fps and 2997 for
    //29.97 fps
    int32_t     fp100s;
};

/*! Generic input argument structure for all commands */
    //! Union across the generic binder struct and the actual cmd struct
union vfmCmdInArg_t {
    /*! An array of integers used by generic parcel and unparcel commands to
        access the elements on this union */
    int32_t         intList[MAX_INT_PER_CMD_ARG];

    //! Union across all the command structures
    struct {
       /*! Video session ID. Same as the id set in metdata VFM_SESSION_ID
            This is set for commands related to buffer operations from the
            players. This is also set by the players if any post processing
            commands are issued from the players which do not understand
            video flow ID.
            Ignored if VFM_FLAG_ALL_SESSIONS is set  */
        int32_t     sessionId;

        /*! Video flow ID. 1: First video flow, 2: Second and so on...
            This is set by the "global settings manager" which has view
            across video flows and understands video flow ID
            Ignored if VFM_FLAG_ALL_SESSIONS is set */
        int32_t     vidFlowId;

        /*! bitmap flags. Set FLAG_SESSION_ID bit if sessionId is set */
        int32_t     flags;

        //! All the inArgs structures for the VFM_COMMAND_TYPE cmds
        union {
            vfmInArgBufReg_t            iaBufReg;
            vfmInArgQBuf_t              iaQBuf;
            vfmInArgCancelBuf_t         iaCancelBuf;
            vfmInArgSetDisplayAttr_t    iaDispAttr;
            struct vpu_control          iaVpuSetCtrl; //VFM_CMD_SET_VPU_CONTROL
            //VFM_CMD_SET_VPU_CONTROL_EXTENDED
            //VFM_CMD_GET_VPU_CONTROL_EXTENDED
            vfmInArgVpuControlExt_t     iaVpuCtrlExt;
        };//union of args
    };//struct sessionId, union
}; // union vfmCmdInArg_t

//! InArgs for VFM_CMD_SET_DISPLAY_ATTRIBUTES
//! OutArgs for CMD_Q_BUF
struct vfmOutArgQBuf_t {
    int32_t     releaseFenceFd;
};

//! outArgs struct for CMD_DQ_BUF
struct vfmOutArgDQBuf_t {
    int32_t id;
};

//! OutArgs for VFM_CMD_SET_VPU_CONTROL_EXTENDED and
//VFM_CMD_GET_VPU_CONTROL_EXTENDED
struct vfmOutArgVpuControlExt_t {
    /*
     * size and ptr of the data to send
     * maximum VPU_MAX_EXT_DATA_SIZE bytes
     */
    uint32_t responseLength;
    char     responseBuffer[VPU_MAX_EXT_DATA_SIZE];

};

/*! Generic output argument structure for all commands */
    //! Union of all outargs structure and generic outarg array
union vfmCmdOutArg_t {
    /*! An array of integers used by generic parcel and unparcel commands to
        access the elements on this union */
    int32_t         intList[MAX_INT_PER_CMD_ARG];

    /*! sessionId and vidFlowId are not required as it is already part of
        inArgs */
    vfmOutArgQBuf_t     oaQBuf;
    vfmOutArgDQBuf_t    oaDQBuf;
    struct vpu_control  oaVpuGetCtrl; //VFM_CMD_GET_VPU_CONTROL
    //VFM_CMD_SET_VPU_CONTROL_EXTENDED, VFM_CMD_GET_VPU_CONTROL_EXTENDED
    vfmOutArgVpuControlExt_t oaVpuCtrlExt;
}; //vfmCmdOutArg_t

/*! Generic payload structure for all events */
union vfmEvtPayload_t {
    /*! An array of integers used by generic parcel and unparcel commands to
        access the elements on this union */
    int32_t         intList[MAX_INT_PER_CMD_ARG];
};

/* Command entry structure */
struct vfmCmdEntry_t {
    VFM_COMMAND_TYPE    eCmd;

    /* Number of ints in the inargs structure of this command */
    int32_t         numIntInArgs;

    /* Number of ints in the outargs structure of this command */
    int32_t         numIntOutArgs;

public:
    String8 dump() const {
        String8 str("\n");
        char s[128];

        snprintf(s, 128, "eCmd: %d \n", eCmd); str += s;
        snprintf(s, 128, "numIntInArgs: %d \n", numIntInArgs); str += s;
        snprintf(s, 128, "numIntOutArgs: %d \n", numIntOutArgs); str += s;
        return str;
    }
};

/*! Global table with all the command entries. Commands which have non-ints
    in the inargs or outargs should not be entered here */
static struct vfmCmdEntry_t vfmCmdInOutArgsTable[] = {
    { VFM_CMD_DQ_BUF,     VFM_MIN_IN_SIZE,    VFM_INT_COUNT(vfmOutArgDQBuf_t)},
    { VFM_CMD_Q_BUF,      VFM_INT_COUNT(vfmInArgQBuf_t),
                                            VFM_INT_COUNT(vfmOutArgQBuf_t) },
    { VFM_CMD_CANCEL_BUF, VFM_INT_COUNT(vfmInArgCancelBuf_t), VFM_MIN_IN_SIZE },
    { VFM_CMD_SET_VPU_CONTROL, VFM_INT_COUNT(struct vpu_control),
                                            VFM_MIN_IN_SIZE },
    { VFM_CMD_GET_VPU_CONTROL, VFM_MIN_IN_SIZE,
                                VFM_INT_COUNT(struct vpu_control) },
};

//Get command entry from the table
inline vfmCmdEntry_t* vfmGetCmdEntry(VFM_COMMAND_TYPE command)
{
    int32_t count = 0;
    vfmCmdEntry_t* cmdEntry = NULL;

    count = sizeof(vfmCmdInOutArgsTable) / sizeof(vfmCmdInOutArgsTable[0]);
    for(int32_t i = 0; i < count ; i++){
        cmdEntry = &vfmCmdInOutArgsTable[i];
        if(command == cmdEntry->eCmd){
            break;
        }
    }
    /* If the command is not found in the table, throw error */
    return cmdEntry;
}

//Utility functions for VFM clients to parcel inargs before calling binder
status_t vfmParcelInArgs(uint32_t command, const vfmCmdInArg_t& inArg,
                            Parcel& data);

//Utility functions for VFM clients to unparcel outargs after calling binder
status_t vfmUnparcelOutArgs(uint32_t command, vfmCmdOutArg_t& outArg,
                            const Parcel& reply);

#endif /* end of include guard: VFM_CMDS_H */
