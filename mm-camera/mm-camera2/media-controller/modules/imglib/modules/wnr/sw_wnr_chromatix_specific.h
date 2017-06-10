/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
***************************************************************************/

/* Default affinity table up to 20 mpix */
.afinity_table[0] = {
  .selection = {
    .valid = 1,
    .modes = (1 << WD_MODE_YCBCR_PLANE) | (1 << WD_MODE_STREAMLINE_YCBCR),
    .uv_subsampling = (1 << IMG_H2V2) | (1 << IMG_H2V1) |
                       (1 << IMG_H1V2) | (1 << IMG_H1V1),
    .min_size = 0,
    .max_size = 20000000,
  },
  .table = {
    {CORE_DSP, CORE_DSP, CORE_DSP, CORE_DSP, CORE_DSP, CORE_DSP, CORE_DSP, CORE_DSP},
    {CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM},
    {CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM}
  },
},

/* 2 to 9 mpix affinity table */
.afinity_table[1] = {
  .selection = {
    .valid = 1,
    .modes = (1 << WD_MODE_YCBCR_PLANE) | (1 << WD_MODE_STREAMLINE_YCBCR),
    .uv_subsampling = (1 << IMG_H2V2) | (1 << IMG_H2V1) |
                       (1 << IMG_H1V2) | (1 << IMG_H1V1),
    .min_size = 2000000,
    .max_size = 9000000,
  },
  .table = {
    {CORE_ARM, CORE_DSP, CORE_DSP, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM},
    {CORE_DSP, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM},
    {CORE_DSP, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM}
  },
},

/* 9 to 13 mpix affinity table */
.afinity_table[2] = {
  .selection = {
    .valid = 1,
    .modes = (1 << WD_MODE_YCBCR_PLANE) | (1 << WD_MODE_STREAMLINE_YCBCR),
    .uv_subsampling = (1 << IMG_H2V2) | (1 << IMG_H2V1) |
                       (1 << IMG_H1V2) | (1 << IMG_H1V1),
    .min_size = 9000000,
    .max_size = 13000000,
  },
  .table = {
    {CORE_DSP, CORE_DSP, CORE_DSP, CORE_DSP, CORE_DSP, CORE_DSP, CORE_DSP, CORE_ARM},
    {CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM},
    {CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM}
  },
},

/* Default chroma downsampled affinity table up to 20 mpix */
.afinity_table[3] = {
  .selection = {
    .valid = 1,
    .modes = (1 << WD_MODE_YCBCR_PLANE) | (1 << WD_MODE_STREAMLINE_YCBCR),
    .uv_subsampling = (1 << IMG_H4V4) | (1 << IMG_H4V2),
    .min_size = 0,
    .max_size = 20000000,
  },
  .table = {
    {CORE_ARM, CORE_DSP, CORE_DSP, CORE_ARM, CORE_DSP, CORE_DSP, CORE_DSP, CORE_DSP},
    {CORE_DSP, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM},
    {CORE_DSP, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM}
  },
},

/* 2 to 9 mpix chroma downsampled affinity table */
.afinity_table[4] = {
  .selection = {
    .valid = 1,
    .modes = (1 << WD_MODE_YCBCR_PLANE) | (1 << WD_MODE_STREAMLINE_YCBCR),
    .uv_subsampling = (1 << IMG_H4V4) | (1 << IMG_H4V2),
    .min_size = 2000000,
    .max_size = 9000000,
  },
  .table = {
    {CORE_ARM, CORE_DSP, CORE_DSP, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM},
    {CORE_DSP, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM},
    {CORE_DSP, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM}
  },
},

/* From 9 to 13 mpix chroma downsampled affinity table */
.afinity_table[5] = {
  .selection = {
    .valid = 1,
    .modes = (1 << WD_MODE_YCBCR_PLANE) | (1 << WD_MODE_STREAMLINE_YCBCR),
    .uv_subsampling = (1 << IMG_H4V4) | (1 << IMG_H4V2),
    .min_size = 9000000,
    .max_size = 13000000,
  },
  .table = {
    {CORE_DSP, CORE_DSP, CORE_DSP, CORE_DSP, CORE_DSP, CORE_ARM, CORE_ARM, CORE_ARM},
    {CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM},
    {CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM, CORE_ARM}
  },
},
/* Default chroma only affinity table */
.afinity_table[6] = {
  .selection = {
    .valid = 1,
    .modes = (1 << WD_MODE_CBCR_ONLY) | (1 << WD_MODE_STREAMLINED_CBCR),
    .uv_subsampling = (1 << IMG_HV_MAX) - 1,
    .min_size = 0,
    .max_size = 20000000,
  },
  .table = {
    {CORE_DSP, CORE_DSP, CORE_DSP, CORE_DSP, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY},
    {CORE_DSP, CORE_DSP, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY},
    {CORE_DSP, CORE_ARM, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY}
  },
},

/* 0 to 2 mpix segment divider */
.segment_divider[0] = {
  .selection = {
    .valid = 1,
    .modes = (1 << WD_MODE_MAX) - 1,
    .uv_subsampling = (1 << IMG_HV_MAX) - 1,
    .min_size = 0,
    .max_size = 2000000,
  },
  .divider = 1,
},

/* 2 to 4 mpix segment divider */
.segment_divider[1] = {
  .selection = {
    .valid = 1,
    .modes = (1 << WD_MODE_MAX) - 1,
    .uv_subsampling = (1 << IMG_HV_MAX) - 1,
    .min_size = 2000000,
    .max_size = 4000000,
  },
  .divider = 2,
},

/* 4 to 8 mpix segment divider */
.segment_divider[2] = {
  .selection = {
    .valid = 1,
    .modes = (1 << WD_MODE_MAX) - 1,
    .uv_subsampling = (1 << IMG_HV_MAX) - 1,
    .min_size = 4000000,
    .max_size = 8000000,
  },
  .divider = 4,
},

/* 8 to 13 mpix segment divider */
.segment_divider[3] = {
  .selection = {
    .valid = 1,
    .modes = (1 << WD_MODE_MAX) - 1,
    .uv_subsampling = (1 << IMG_HV_MAX) - 1,
    .min_size = 8000000,
    .max_size = 13000000,
  },
  .divider = 6,
},

/* 13 to 20 mpix segment divider */
.segment_divider[4] = {
  .selection = {
    .valid = 1,
    .modes = (1 << WD_MODE_MAX) - 1,
    .uv_subsampling = (1 << IMG_HV_MAX) - 1,
    .min_size = 13000000,
    .max_size = 20000000,
  },
  .divider = 8,
},
