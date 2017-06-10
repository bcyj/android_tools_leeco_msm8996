/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
 /*****************************************************************************
  This file interface structures required by VFM api other than process command
  This is the interface used by hardware composer to interact with VFM
 *****************************************************************************/

#ifndef VFM_DEFS_H
#define VFM_DEFS_H
#include <sys/types.h>
#include <utils/String8.h>
#include "gralloc_priv.h"

#include "vfm_cmds.h"

using namespace android;
namespace vpu {

#define MAX_VPU_SESSIONS (4)

#define MAX_LAYERS_PER_DISPLAY (32)

#define MAX_DISP_PIPE_PER_SESSION (2)

/*****************************************************************************/
/* Enumerations                                                              */
/*****************************************************************************/
/*! Display ID for layer list */
typedef enum {
    DISP_PRIMARY = 0x00,
    DISP_EXTERNAL,
    DISP_VIRTUAL,
    DISP_MAX
} DISPLAY_ID;

/*! IN flags */
enum {
    /* Same as defined in hwc_layer_1->flags = HWC_GEOMETRY_CHANGED */
    GEOMETRY_CHANGED    = 0x0001,

    //! VFM will skip processing this layer if this flag is set
    SKIP_LAYER          = 0x0002,
};

/*! OUT flags */
enum {
    //! This flag is set if a specific layer can be processed by VPU
    VPU_LAYER           = 0x0001,

    /*!
        If this flag is set, the client should reserve the same pipes
        allocated previously for this layer
    */
    RESERVE_PREV_PIPES  = 0x0002,
};

/*****************************************************************************/
/* Data structures                                                           */
/*****************************************************************************/
//! To indicate the rectangle using 4 co-ordinates
struct Rect{
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
public:
    inline bool operator==(const Rect& a) const {
        return ((left==a.left) && (top==a.top) && (right==a.right) &&
                (bottom==a.bottom));
    }
    inline bool operator!=(const Rect& rhs) const {
        return (!operator==(rhs));
    }
    String8 dump() const{
        String8 str("\n");
        char s[128];

        snprintf(s, 128,"l: %d \n", left); str += s;
        snprintf(s, 128,"t: %d \n", top); str += s;
        snprintf(s, 128,"r: %d \n", right); str += s;
        snprintf(s, 128,"b: %d \n", bottom); str += s;
        return str;
    }
};

//! To indicate a buffer's dimensions
struct Dim {
    int32_t width;
    int32_t height;
public:
    inline bool operator==(const Dim& rhs) const {
        return ((width == rhs.width) && (height == rhs.height));
    }
    inline bool operator!=(const Dim& rhs) const {
        return (!operator==(rhs));
    }
    String8 dump() const{
        String8 str("\n");
        char s[128];

        snprintf(s, 128,"w: %d \n", width); str += s;
        snprintf(s, 128,"h: %d \n", height); str += s;
        return str;
    }
};

//! Destination pipe information
struct DestinationPipes
{
    int32_t        numPipes;
    int32_t        pipe[MAX_DISP_PIPE_PER_SESSION];
public:
    inline bool operator==(const DestinationPipes& rhs) const {
        if(numPipes == rhs.numPipes){
            for(int32_t i = 0; i < numPipes; i++){
                    if(pipe[i] != rhs.pipe[i])
                        return false;
            }
            return true;
        }else{
            return false;
        }
    }
    inline bool operator!=(const DestinationPipes& rhs) const {
        return (!operator==(rhs));
    }
    String8 dump() const{
        String8 str("\n");
        char s[128];

        snprintf(s, 128,"numPipes: %d \n", numPipes); str += s;
        for(int i = 0; i < MAX_DISP_PIPE_PER_SESSION; i++){
            snprintf(s, 128,"pipe[%d]: %d \n", i, pipe[i]); str += s;
        }
        return str;
    }
};

struct Layer
{
    //! Same as hwc_layer_1->handle
    private_handle_t*   handle;
    //! Stride width and height/scan lines
    Dim                 srcStride;
    //! Based on hwc_layer_1->sourceCrop
    Rect                srcRect;
    //! Based on hwc_layer_1->displayFrame
    Rect                tgtRect;

    //! Same as defined in hwc_layer_1
    int                 acquireFenceFd;

    //! Same as defined in hwc_layer_1
    int                 releaseFenceFd;

    /*! Will be ignored by VFM during prepare call
        Has to be allocated by the client and set during draw call
        Will be filled by VFM in subsequenct prepare calls along with
         "RESERVE_PREV_PIPES" flag */
    DestinationPipes    sDestPipes;

    //! Out arg. Updated by VFM in setupVpuSession call
    int32_t             vpuOutPixFmt;

    //! IN flags
    int32_t             inFlags;

    //! OUT flags. Client should prefill this flag to zero
    int32_t             outFlags;
};

//!Do not pass HWC_BACKGROUND and HWC_FRAMEBUFFER_TARGET and "skip" layers
struct LayerList
{
    //! Same as hwc_display_contents_1_t->numHwLayers
    size_t            numLayers;
    Layer             layers[MAX_LAYERS_PER_DISPLAY];
};

//! Display Attributes structure
struct DispAttr_t {
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

struct notifyCb_t {
    int32_t (*eventHdlr) (const VFM_EVENT_TYPE event, const vfmEvtPayload_t&,
                           const int32_t vpuSessid, void* cookie);
    void*   cookie;
};

}; //namespace VPU

#endif /* end of include guard: VFM_DEFS_H */
