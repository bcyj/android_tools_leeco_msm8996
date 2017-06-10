/*============================================================================
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

==============================================================================
*/
#ifndef Scale_H
#define Scale_H
#include <cutils/properties.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/msm_mdp.h>

#define PHASE_STEP_SHIFT    21
#define PHASE_STEP_UNIT_SCALE   ((int) (1 << PHASE_STEP_SHIFT))
#define FIR_PIXEL_SHIFT     16
#define PHASE_RESIDUAL      (FIR_PIXEL_SHIFT - 1)
#define PCMN_PHASE_SHIFT    5
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

namespace scale {

struct scaleData {
    uint8_t chroma_sample_v[2];
    uint8_t chroma_sample_h[2];
};

class Scale {
public:
    static Scale *getInstance();
    bool mDebug;
    bool mDisableSingle;
    unsigned int mMaxMixerWidth;
    virtual int applyScale(struct mdp_overlay_list *list);

    Scale();
    virtual ~Scale();

private:
    static Scale *sScaleObj;
    static struct scaleData scale;
    int dualQseedScalar(struct mdp_overlay *left, struct mdp_overlay *right);
    int dualRgbScalar(struct mdp_overlay *left, struct mdp_overlay *right);
    int singleQseedScalar(struct mdp_overlay *overlay);
    int singleRgbScalar(struct mdp_overlay *overlay);
    int singleOverlay(struct mdp_overlay *overlay);
    int dualOverlay(struct mdp_overlay *left, struct mdp_overlay *right);
    bool findOverlayPair(struct mdp_overlay *one, struct mdp_overlay *two,
        int *rc);
    bool isYuv(uint32_t format);
    void setInterlaceCrop(struct mdp_overlay *overlay);
    int setCrop(struct mdp_overlay *left, struct mdp_overlay *right);
    int calcPhaseStep(uint32_t src, uint32_t dst, int *out_phase);
    void setChromaSample(uint32_t format, int index);
    void calcSingleRgbInitPhase(struct mdp_overlay *overlay);
    int calcDualInitPhase(struct mdp_overlay *left, struct mdp_overlay *right);
    int calcPixelExtn(struct mdp_overlay *ov, int index);
    int calcPixelFetch(struct mdp_overlay *ov, int index);
    bool needScale(struct mdp_overlay *ov);
};

};

#endif

