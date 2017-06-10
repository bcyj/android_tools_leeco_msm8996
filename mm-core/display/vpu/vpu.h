/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef VPU_H
#define VPU_H
#include <sys/types.h>
//#include <utils/Mutex.h>
#include "vfm_defs.h"
#include "vfm_cmds.h"

//using namespace android;
namespace vpu {

//Forward declaration
class VPU;

/*!
  @brief         Returns the VPU object

  @param[in]     None

  @return        VPU object

  @note    Synchronous function
 */
extern "C" VPU* getObject();

/*!
  @brief         Since its a singleton object, does nothing.

  @param[in]     VPU object

  @return        None

  @note    Synchronous function
 */
extern "C" void deleteObject(VPU* object);

class VPU {
public:
    VPU();
    virtual ~VPU();

    // To be called during construction
    virtual status_t init();

    // To be called during construction to register notifyCb_t
    virtual status_t init(notifyCb_t*);

/*!
  @brief         Analyzes the layer list and marks the layers that will be
                 processed by VPU. Upon success, LayerList->Layer->
                 canVpuProcessLayer, Layer->reservePrevPipes and
                 Layer->sDestPipes  will be set appropriately

  @param[inout]  layerList pointer to the LayerList structure

  @param[in]     DISPLAY_ID corresponding to this LayerList

  @return        status_t Error.

  @note    Synchronous function
 */
    virtual status_t setupVpuSession(DISPLAY_ID, LayerList* layerList);

/*!
  @brief         Sets the MDSS pipe information to VPU

  @param[inout]  layerList pointer to the LayerList structure

  @param[in]     DISPLAY_ID corresponding to this LayerList

  @return        status_t Error.

  @note    Synchronous function
 */
    virtual status_t prepare(DISPLAY_ID, LayerList* layerList);

/*!
  @brief         Queues the buffers to VPU

  @param[in]     layerList pointer to the LayerList structure

  @param[in]     DISPLAY_ID corresponding to this LayerList

  @return        status_t Error.

  @note    Synchronous function
 */
    virtual status_t draw(DISPLAY_ID, LayerList* layerList);

/*!
  @brief         Setting properties

  @param[in]     command - key. Refer to vpu_cmds.h

  @param[in]     settting - value

  @return        status_t Error.

  @note    Synchronous function
 */
    virtual status_t processCommand(const uint32_t command,
                        const Parcel* inParcel, Parcel* outParcel);

/*
    Set the display attributes - fps and resolution of the display
*/
    virtual status_t setDisplayAttr(DISPLAY_ID, DispAttr_t&);
};

}; //namespace VPU

#endif /* end of include guard: VPU_H */
