/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* This file contains the wrapper class around the actual VFM
*/
#include <utils/Log.h>
#include "vpu.h"
#include "vfm.h"

namespace vpu {

VPU::VPU()
{
}
VPU::~VPU()
{
}
status_t VPU::init()
{
    return VFM::getInstance()->init();
}
status_t VPU::init(notifyCb_t* notifyCb)
{
    return VFM::getInstance()->init(notifyCb);
}
status_t VPU::setupVpuSession(DISPLAY_ID dispId, LayerList* layerList)
{
    return VFM::getInstance()->setupVpuSession(dispId, layerList);
}
status_t VPU::prepare(DISPLAY_ID dispId, LayerList* layerList)
{
    return VFM::getInstance()->prepare(dispId, layerList);
}
status_t VPU::draw(DISPLAY_ID dispId, LayerList* layerList)
{
    return VFM::getInstance()->draw(dispId, layerList);
}
status_t VPU::processCommand(const uint32_t command,
                        const Parcel* inParcel, Parcel* outParcel)
{
    return VFM::getInstance()->processCommand(command, inParcel, outParcel);
}
status_t VPU::setDisplayAttr(DISPLAY_ID dispId, DispAttr_t& dispAttr)
{
    return VFM::getInstance()->setDisplayAttr(dispId, dispAttr);
}
extern "C" VPU* getObject()
{
    return new VPU();
}
extern "C" void deleteObject(VPU* object)
{
    delete object;
}

};
