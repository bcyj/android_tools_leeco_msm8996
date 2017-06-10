/*============================================================================
   Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include <dlfcn.h>
#include <stdbool.h>
#include <stdint.h>
#include "dmlrocorrection.h"
#include "tintless_interface.h"
#include "isp_tintless_interface.h"

#define MESH_ROLL_OFF_V4_TABLE_SIZE  (13 * 10)
#if DMLROC_MESH_ROLLOFF_SIZE != MESH_ROLL_OFF_V4_TABLE_SIZE
#error "Rolloff mesh size mismatch"
#endif

#include "camera_dbg.h"

//#define TINTLESS_DEBUG
#ifdef TINTLESS_DEBUG
    #define CDBG_TINTLESS ALOGE
#else
    #define CDBG_TINTLESS CDBG
#endif

#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

#define BUFF_SIZE_255 255

// local structs

    // bitmask containing types of updates requested by the algo

typedef enum tintless_update_request_t {
    UPDATES_STAT_CONFIG,
    UPDATES_AEC,
    UPDATES_AWB,
    UPDATES_AF,
    UPDATES_CAMERA_MODE,
    UPDATES_FLASH_MODE,
    UPDATES_CHROMATIX_PARAMS,
    UPDATES_MESH_CONFIG,
    UPDATES_END
} tintless_update_request_t;

#define TINTLESS_WRAPPER_SUPPORTED_UPDATES \
    ((1 << UPDATES_STAT_CONFIG) | \
     (1 << UPDATES_AEC) | \
/*   (1 << UPDATES_AWB) | \
     (1 << UPDATES_AF) | \
     (1 << UPDATES_CAMERA_MODE) | \
     (1 << UPDATES_FLASH_MODE) | \
*/   (1 << UPDATES_CHROMATIX_PARAMS) | \
     (1 << UPDATES_MESH_CONFIG))

typedef struct {
    void * dmlroc_res;
    void * plib;
    dmlroc_return_t (*update_func)(void * res,
      const dmlroc_bayer_stats_info_t * const pbayer_r,
      const dmlroc_bayer_stats_info_t * const pbayer_gr,
      const dmlroc_bayer_stats_info_t * const pbayer_gb,
      const dmlroc_bayer_stats_info_t * const pbayer_b,
      const dmlroc_mesh_rolloff_array_t * const ptable_current,
      const dmlroc_mesh_rolloff_array_t * const ptable_3a,
      dmlroc_mesh_rolloff_array_t * const ptable_correction);
    void (*get_version_func)(dmlroc_version_t * const pversion);
    dmlroc_return_t (*init_func)(void ** res, const dmlroc_config_t * const cfg);
    void (*deinit_func)(void ** res);
    uint32_t updates;
    dmlroc_config_t cfg;
} tintless_lib_t;

typedef union {
    tintless_stats_config_t * stats;
    tintless_mesh_config_t * mesh;
    chromatix_color_tint_correction_type * chromatix;
} tintless_cfg_t;

/*============================================================================
Function DECLARATIONS
============================================================================*/
static tintless_return_t translate_return_code(dmlroc_return_t const in);
static void isp_tintless_req_updates(uint32_t * mask);
static tintless_return_t isp_tintless_config(tintless_lib_t * const tintless_lib, const tintless_update_request_t type, tintless_cfg_t c);

/*===========================================================================
FUNCTION      isp_tintless_open

DESCRIPTION
===========================================================================*/
static tintless_return_t isp_tintless_open(void ** const res, uint32_t * updates_needed)
{
    tintless_return_t rc = TINTLESS_LIB_NOT_LOADED;
    dmlroc_version_t v;
    tintless_lib_t * tintless_lib = NULL;
    tintless_lib_t ** pp_tintless;
    char lib_name[BUFF_SIZE_255] = { 0 };

    CDBG_TINTLESS("%s : Enter!\n", __func__);
    if (res != NULL){
        pp_tintless = (tintless_lib_t **)res;
    } else {
        CDBG_ERROR("%s : res pointer NULL!\n", __func__);
        goto ERROR;
    }


    *pp_tintless = (tintless_lib_t *) malloc(sizeof (tintless_lib_t));
    if (*pp_tintless == NULL) {
        rc = TINTLESS_NO_MEMORY;
        goto ERROR;
    }
    tintless_lib = (tintless_lib_t *) *pp_tintless;

    memset(tintless_lib, 0, sizeof(tintless_lib_t));

    strlcpy(lib_name, "libmmcamera_tintless_algo.so", BUFF_SIZE_255);

    dlerror();
    tintless_lib->plib = dlopen(lib_name, RTLD_NOW);

    if (!tintless_lib->plib) {
        CDBG_ERROR("%s:Failed to dlopen %s: %s", __func__, lib_name, dlerror());
        goto ERROR;
    }

  *(void **)&(tintless_lib->init_func) = dlsym(tintless_lib->plib,
                                              "dmlroc_init");
  if (!tintless_lib->init_func)
    CDBG_ERROR("%s:init Failed to dlsym %s: %s",
               __func__, lib_name, dlerror());

  *(void **)&(tintless_lib->update_func) = dlsym(tintless_lib->plib,
                                                "dmlroc_entry");
  if (!tintless_lib->update_func)
    CDBG_ERROR("%s:update Failed to dlsym %s: %s",
               __func__, lib_name, dlerror());

  *(void **)&(tintless_lib->get_version_func) =
                  dlsym(tintless_lib->plib, "dmlroc_get_version");
  if (!tintless_lib->get_version_func)
    CDBG_ERROR("%s:version Failed to dlsym %s: %s",
               __func__, lib_name, dlerror());

  *(void **)&(tintless_lib->deinit_func) = dlsym(tintless_lib->plib,
                                                "dmlroc_deinit");
  if (!tintless_lib->deinit_func)
    CDBG_ERROR("%s:deinit Failed to dlsym %s: %s",
               __func__, lib_name, dlerror());

  if (!tintless_lib->init_func || !tintless_lib->update_func ||
      !tintless_lib->get_version_func || !tintless_lib->deinit_func) {
    CDBG_ERROR("%s:Failed to dlsym %s: %s", __func__, lib_name, dlerror());
    goto ERROR;
  }

    // subscribe to cfg updates
    *updates_needed = (TINTLESS_UPDATE_STATS |
                       TINTLESS_UPDATE_MESH |
                       TINTLESS_UPDATE_CHROMATIX_PARAMS);
    tintless_lib->updates = *updates_needed;
    tintless_lib->cfg.tint_correction_strength = UINT8_MAX;
    return TINTLESS_SUCCESS;


ERROR:
    if (tintless_lib != NULL) {
        if (tintless_lib->plib) {
            dlclose(tintless_lib->plib);
        }
        free(tintless_lib);
        *pp_tintless = NULL;
    }
    return rc;
} /* isp_tintless_open */

/*===========================================================================
FUNCTION      isp_tintless_get_version

DESCRIPTION    Checks the version of the tintless wrapper. As a side effect,
               the wrapper will check the version of the algorithm. If there
               is a mismatch, it will show up in the return code.
===========================================================================*/
static tintless_return_t isp_tintless_get_version(void * const res, tintless_version_t * version)
{
    tintless_return_t rc = TINTLESS_SUCCESS;
    tintless_lib_t * const tintless_lib = (tintless_lib_t *) res;

    if (tintless_lib == NULL || tintless_lib->get_version_func == NULL) {
        rc = TINTLESS_LIB_NOT_LOADED;
        CDBG_ERROR("%s: pointer null: %p %p",
                   __func__, tintless_lib, tintless_lib);
    } else {
        tintless_lib->get_version_func((dmlroc_version_t *)version);

        CDBG_ERROR("%s: lib returned version %d.%d err=%d",
                   __func__, version->api_version, version->minor_version, rc);
    }

    if (rc == TINTLESS_SUCCESS) {
        if (version->api_version != DMLROC_API_VERSION ||
            version->minor_version != DMLROC_FUNC_VERSION)
            rc = TINTLESS_LIB_MISMATCH;
    }

    return rc;
} /* isp_tintless_get_version */

/*===========================================================================
FUNCTION      isp_tintless_stat_config

DESCRIPTION   Update the stat params for the tintless algo.
              Should be called after the BG stat config has
              been called.
===========================================================================*/
static tintless_return_t isp_tintless_stat_config(void * const res, tintless_stats_config_t * cfg)
{
    tintless_return_t rc;
    tintless_cfg_t c;
    tintless_lib_t * const tintless_lib = (tintless_lib_t *) res;

    CDBG_TINTLESS("%s: Enter \n", __func__);

    CDBG_HIGH("%s: stats : camif hxw %d x %d, ele col,row %d, %d, hxw %d x %d, type %d",__func__,
               cfg->camif_win_h, cfg->camif_win_w,
               cfg->num_stat_elem_cols, cfg->num_stat_elem_rows,
               cfg->stat_elem_h, cfg->stat_elem_w,
               cfg->stats_type);

    if (tintless_lib == NULL || tintless_lib->init_func == NULL)
    {
        CDBG_ERROR("%s: Tintless lib is NULL\n", __func__);
        rc = TINTLESS_LIB_NOT_LOADED;
    }
    else
    {
        CDBG_ERROR("%s: pointer okay \n", __func__);
        c.stats = cfg;
        if (tintless_lib->updates & ( 1 << UPDATES_STAT_CONFIG)) {
            rc = isp_tintless_config(tintless_lib, UPDATES_STAT_CONFIG, c);
            CDBG_ERROR("%s: lib returned config err=%d", __func__, rc);
        } else {
            CDBG_ERROR("%s: Stat cfg updates not needed", __func__);
            rc = TINTLESS_UPDATES_NOT_SUPPORTED;
        }
    }

    return rc;
} /* isp_tintless_stat_config */

/*===========================================================================
FUNCTION      isp_tintless_mesh_config

DESCRIPTION   Update the stat params for the tintless algo.
              Should be called after the BG stat config has
              been called.
===========================================================================*/
static tintless_return_t isp_tintless_mesh_config(void * const res, tintless_mesh_config_t * cfg)
{
    tintless_return_t rc;
    tintless_cfg_t c;
    tintless_lib_t * const tintless_lib = (tintless_lib_t *) res;

    CDBG_TINTLESS("%s: mesh : col, row %d ,%d offset h,v %d,%d, SG hxw %d x %d, subgrids %d",__func__,
               cfg->num_mesh_elem_cols, cfg->num_mesh_elem_rows,
               cfg->offset_horizontal, cfg->offset_vertical,
               cfg->subgrid_height, cfg->subgrid_width,
               cfg->subgrids);


    if (tintless_lib == NULL || tintless_lib->init_func == NULL)
    {
        rc = TINTLESS_LIB_NOT_LOADED;
    } else {
        c.mesh = cfg;
        if (tintless_lib->updates & ( 1 << UPDATES_MESH_CONFIG)) {
            rc = isp_tintless_config(tintless_lib, UPDATES_MESH_CONFIG, c);
            CDBG_ERROR("%s: lib returned mesh config err=%d", __func__, rc);
        } else {
            CDBG_ERROR("%s: Mesh cfg updates not needed", __func__);
            rc = TINTLESS_UPDATES_NOT_SUPPORTED;
        }
    }

    return rc;
} /* isp_tintless_mesh_config */

/*===========================================================================
FUNCTION      isp_tintless_update_chromatix_params

DESCRIPTION   Update the tintless related chromatix tuning parameters.
===========================================================================*/
static tintless_return_t isp_tintless_update_chromatix_params(
   void * const res, chromatix_color_tint_correction_type * p)
{
    tintless_return_t rc;
    tintless_cfg_t param;
    tintless_lib_t * const tintless_lib = (tintless_lib_t *) res;

    CDBG_TINTLESS("%s: chromatix : strength %d",__func__,
               p->tint_correction_strength);

    if (tintless_lib == NULL || tintless_lib->init_func == NULL)
    {
        rc = TINTLESS_LIB_NOT_LOADED;
    } else {
        param.chromatix = p;
        if (tintless_lib->updates & ( 1 << UPDATES_CHROMATIX_PARAMS)) {
            rc = isp_tintless_config(tintless_lib, UPDATES_CHROMATIX_PARAMS, param);
            CDBG_ERROR("%s: lib returned config err=%d", __func__, rc);
        } else {
            CDBG_ERROR("%s: chromatix parameter updates not needed", __func__);
            rc = TINTLESS_UPDATES_NOT_SUPPORTED;
        }
    }

    return rc;
} /* isp_tintless_update_chromatix_params */

/*===========================================================================
FUNCTION      isp_tintless_algo

DESCRIPTION   calling the main algorithm for tintless processing
===========================================================================*/
static tintless_return_t isp_tintless_algo(void * const res,
                                    tintless_stats_t * be_stats,
                                    tintless_mesh_rolloff_array_t * ptable_3a,
                                    tintless_mesh_rolloff_array_t * ptable_cur,
                                    tintless_mesh_rolloff_array_t * const ptable_correction)
{
    tintless_return_t rc;
    tintless_lib_t * const tintless_lib = (tintless_lib_t *) res;
    dmlroc_mesh_rolloff_array_t p_tbl_3a;
    dmlroc_mesh_rolloff_array_t p_tbl_correction;
    dmlroc_bayer_stats_info_t pbayer_r, pbayer_gr, pbayer_gb, pbayer_b;

    CDBG_TINTLESS("%s: Enter !\n", __func__);

    if (tintless_lib == NULL || tintless_lib->update_func == NULL) {
        rc = TINTLESS_LIB_NOT_LOADED;
    } else {
        p_tbl_3a.table_size = MESH_ROLL_OFF_V4_TABLE_SIZE;
        p_tbl_3a.r_gain     = (float (*)[DMLROC_MESH_ROLLOFF_WIDTH])ptable_3a->r_gain;
        p_tbl_3a.gr_gain    = (float (*)[DMLROC_MESH_ROLLOFF_WIDTH])ptable_3a->gr_gain;
        p_tbl_3a.gb_gain    = (float (*)[DMLROC_MESH_ROLLOFF_WIDTH])ptable_3a->gb_gain;
        p_tbl_3a.b_gain     = (float (*)[DMLROC_MESH_ROLLOFF_WIDTH])ptable_3a->b_gain;

        p_tbl_correction.table_size = MESH_ROLL_OFF_V4_TABLE_SIZE;
        p_tbl_correction.r_gain     = (float (*)[DMLROC_MESH_ROLLOFF_WIDTH])ptable_correction->r_gain;
        p_tbl_correction.gr_gain    = (float (*)[DMLROC_MESH_ROLLOFF_WIDTH])ptable_correction->gr_gain;
        p_tbl_correction.gb_gain    = (float (*)[DMLROC_MESH_ROLLOFF_WIDTH])ptable_correction->gb_gain;
        p_tbl_correction.b_gain     = (float (*)[DMLROC_MESH_ROLLOFF_WIDTH])ptable_correction->b_gain;
        pbayer_r.channel_counts = be_stats->r.channel_counts;
        pbayer_r.channel_sums   = be_stats->r.channel_sums;
        pbayer_r.array_length   = be_stats->r.array_length;
        pbayer_gr.channel_counts = be_stats->gr.channel_counts;
        pbayer_gr.channel_sums   = be_stats->gr.channel_sums;
        pbayer_gr.array_length   = be_stats->gr.array_length;
        pbayer_gb.channel_counts = be_stats->gb.channel_counts;
        pbayer_gb.channel_sums   = be_stats->gb.channel_sums;
        pbayer_gb.array_length   = be_stats->gb.array_length;
        pbayer_b.channel_counts = be_stats->b.channel_counts;
        pbayer_b.channel_sums   = be_stats->b.channel_sums;
        pbayer_b.array_length   = be_stats->b.array_length;
        rc = translate_return_code(
            tintless_lib->update_func(tintless_lib->dmlroc_res,
                                      &pbayer_r,
                                      &pbayer_gr,
                                      &pbayer_gb,
                                      &pbayer_b,
                                      NULL,
                                      &p_tbl_3a,
                                      &p_tbl_correction));
    }

    if (rc != TINTLESS_SUCCESS) {
        CDBG_ERROR("%s: dmlroCorrection returned err=%d", __func__, rc);
    }

  return rc;
} /* isp_tintless_algo */

/*===========================================================================
FUNCTION      isp_tintless_close

DESCRIPTION   close the library and free all allocated memory
===========================================================================*/
static tintless_return_t isp_tintless_close(void ** const res)
{
    tintless_return_t rc = TINTLESS_SUCCESS;
    tintless_lib_t * tintless_lib = NULL;

    CDBG_TINTLESS("%s: Enter\n",__func__);

    if (res != NULL)
        tintless_lib = (tintless_lib_t *) *res;

    if (tintless_lib) {
        CDBG_TINTLESS("%s: tint_lib %p, deinit_func %p \n", __func__,
            tintless_lib, tintless_lib->deinit_func);
        if (tintless_lib->deinit_func) {
            tintless_lib->deinit_func(&tintless_lib->dmlroc_res);
        }

        if (tintless_lib->plib) {
            dlclose(tintless_lib->plib);
        }

        free(*res);
        *res = NULL;
    } else {
        rc = TINTLESS_LIB_NOT_LOADED;
    }

    CDBG_TINTLESS("%s: close/unload tintless lib %d", __func__, rc);
    return rc;
} /* isp_tintless_close */

/*===========================================================================
FUNCTION      validate_config

DESCRIPTION
===========================================================================*/
static inline bool validate_config(tintless_lib_t * const tintless_lib, dmlroc_config_t * cfg)
{
    bool rv = true;

    if (tintless_lib->updates == 0)
        return false;

    if (tintless_lib->updates & TINTLESS_UPDATE_STATS)
        rv &= cfg->camif_win_h  != 0 &&
              cfg->camif_win_w  != 0 &&
              cfg->num_stat_elem_cols != 0 &&
              cfg->num_stat_elem_rows != 0 &&
              cfg->stat_elem_h != 0 &&
              cfg->stat_elem_w != 0 &&
              // make sure the stat region doesn't go outside the camif window
              cfg->num_stat_elem_rows * cfg->stat_elem_h <= cfg->camif_win_h &&
              cfg->num_stat_elem_cols * cfg->stat_elem_w <= cfg->camif_win_w;


    if (tintless_lib->updates & TINTLESS_UPDATE_MESH)
        rv &= cfg->mesh_subgrid_height  != 0 &&
              cfg->mesh_subgrid_width  != 0 &&
              cfg->num_mesh_elem_cols != 0 &&
              cfg->num_mesh_elem_rows != 0 &&
              cfg->num_mesh_subgrids != 0 &&
              // make sure the rolloff grid covers the entire camif window
              cfg->mesh_subgrid_height * cfg->num_mesh_subgrids * (cfg->num_mesh_elem_rows - 1) + 1 >= cfg->camif_win_h &&
              cfg->mesh_subgrid_width  * cfg->num_mesh_subgrids * (cfg->num_mesh_elem_cols - 1) + 1 >= cfg->camif_win_w;

    if (tintless_lib->updates & TINTLESS_UPDATE_CHROMATIX_PARAMS)
        rv &= cfg->tint_correction_strength != UINT8_MAX;

    return rv;
}

/*===========================================================================
FUNCTION      isp_tintless_config

DESCRIPTION
===========================================================================*/
static tintless_return_t
isp_tintless_config(tintless_lib_t * const tintless_lib, const tintless_update_request_t type, tintless_cfg_t c)
{
    tintless_stats_config_t * stats_cfg;
    tintless_mesh_config_t * mesh_cfg;
    chromatix_color_tint_correction_type * chromatix_param;
    dmlroc_config_t * const cfg = &tintless_lib->cfg;

    switch (type)
    {
    case UPDATES_STAT_CONFIG:
        stats_cfg = c.stats;

        // check if the new cfg is valid
        if (stats_cfg == NULL || stats_cfg->camif_win_w == 0 ||
            stats_cfg->stat_elem_w == 0 || stats_cfg->stat_elem_h == 0 ||
            stats_cfg->camif_win_h == 0 || stats_cfg->num_stat_elem_rows ==0 ||
            stats_cfg->num_stat_elem_cols == 0) {
            return TINTLESS_INVALID_STATS;
        }

        // check if the new cfg is different from current cfg
        if (cfg->camif_win_w        != stats_cfg->camif_win_w ||
            cfg->camif_win_h        != stats_cfg->camif_win_h ||
            cfg->stat_elem_w        != stats_cfg->stat_elem_w ||
            cfg->stat_elem_h        != stats_cfg->stat_elem_h ||
            cfg->num_stat_elem_rows != stats_cfg->num_stat_elem_rows ||
            cfg->num_stat_elem_cols != stats_cfg->num_stat_elem_cols ||
            cfg->b_post_stats       != stats_cfg->stats_type
            )
        {
            cfg->camif_win_w        = stats_cfg->camif_win_w;
            cfg->camif_win_h        = stats_cfg->camif_win_h;
            cfg->stat_elem_w        = stats_cfg->stat_elem_w;
            cfg->stat_elem_h        = stats_cfg->stat_elem_h;
            cfg->num_stat_elem_rows = stats_cfg->num_stat_elem_rows;
            cfg->num_stat_elem_cols = stats_cfg->num_stat_elem_cols;
            cfg->b_post_stats       = (stats_cfg->stats_type == STATS_TYPE_BG) ? 1 : 0;
            CDBG_ERROR("%s: cfg: camif %dx%d, elem sz %dx%d, elems %dx%d", __func__,stats_cfg->camif_win_w, stats_cfg->camif_win_h, stats_cfg->stat_elem_w, stats_cfg->stat_elem_h, stats_cfg->num_stat_elem_rows, stats_cfg->num_stat_elem_cols);
        } else {
            printf("same cfg as current\n");
            return TINTLESS_SUCCESS;
        }
        break;

    case UPDATES_MESH_CONFIG:
        mesh_cfg = c.mesh;

        // check if the new cfg is valid
        if (mesh_cfg == NULL || mesh_cfg->subgrids == 0 ||
            mesh_cfg->subgrid_height == 0 || mesh_cfg->subgrid_width == 0 ||
            mesh_cfg->num_mesh_elem_rows == 0 || mesh_cfg->num_mesh_elem_cols == 0)
        {
            return TINTLESS_INVALID_STATS;
        }

        // check if the new cfg is different from current cfg
        if (cfg->mesh_subgrid_width     != mesh_cfg->subgrid_width ||
            cfg->mesh_subgrid_height    != mesh_cfg->subgrid_height ||
            cfg->num_mesh_elem_rows     != mesh_cfg->num_mesh_elem_rows ||
            cfg->num_mesh_elem_cols     != mesh_cfg->num_mesh_elem_cols ||
            cfg->mesh_offset_horizontal != mesh_cfg->offset_horizontal ||
            cfg->mesh_offset_vertical   != mesh_cfg->offset_vertical ||
            cfg->num_mesh_subgrids      != (uint32_t)mesh_cfg->subgrids)
        {
            cfg->mesh_subgrid_width     = mesh_cfg->subgrid_width;
            cfg->mesh_subgrid_height    = mesh_cfg->subgrid_height;
            cfg->num_mesh_elem_rows     = mesh_cfg->num_mesh_elem_rows;
            cfg->num_mesh_elem_cols     = mesh_cfg->num_mesh_elem_cols;
            cfg->mesh_offset_horizontal = mesh_cfg->offset_horizontal;
            cfg->mesh_offset_vertical   = mesh_cfg->offset_vertical;
            cfg->num_mesh_subgrids      = (uint32_t)mesh_cfg->subgrids;

            CDBG_ERROR("%s: cfg: %dx%d mesh, %dx%d subgrid, subgrid sz %dx%d\n", __func__,
                       mesh_cfg->num_mesh_elem_rows, mesh_cfg->num_mesh_elem_cols,
                       cfg->num_mesh_subgrids, cfg->num_mesh_subgrids,
                       mesh_cfg->subgrid_width, mesh_cfg->subgrid_height);
        } else {
            CDBG_ERROR("%s: same cfg as current", __func__);
            return TINTLESS_SUCCESS;
        }
        break;

    case UPDATES_CHROMATIX_PARAMS:
        chromatix_param = c.chromatix;

        if (chromatix_param == NULL)
            return TINTLESS_INVALID_STATS;

        // check if the new cfg is different from current cfg
        if (cfg->tint_correction_strength != chromatix_param->tint_correction_strength)
        {
            cfg->tint_correction_strength = chromatix_param->tint_correction_strength;
            CDBG_ERROR("%s: tint_correction_strength updated to %d", __func__, cfg->tint_correction_strength);
        } else {
            CDBG_ERROR("%s: same cfg as current", __func__);
            return TINTLESS_SUCCESS;
        }
        break;
    default:
        return TINTLESS_UPDATES_NOT_SUPPORTED;
        break;
    } // end switch

    // validate config struct. If complete, init tintless lib
    if (validate_config(tintless_lib, cfg))
        return translate_return_code(tintless_lib->init_func(&tintless_lib->dmlroc_res, cfg));
    else
        return TINTLESS_SUCCESS;


} /* isp_tintless_config */



/*===========================================================================
FUNCTION      translate_return_code

DESCRIPTION   Convert return codes from the tintless lib to a tintless
              return code.
===========================================================================*/
static tintless_return_t translate_return_code(dmlroc_return_t const in)
{
    switch (in)
    {
    case DMLROC_SUCCESS:
        return TINTLESS_SUCCESS;
    case DMLROC_NO_MEMORY:
        return TINTLESS_NO_MEMORY;
    case DMLROC_ERROR:
        return TINTLESS_ERROR;
    case DMLROC_INVALID_STATS:
        return TINTLESS_INVALID_STATS;
    case DMLROC_BAD_OUTPUT_TABLE:
        return TINTLESS_BAD_OUTPUT_TABLE;
    case DMLROC_LIB_NOT_LOADED:
        return TINTLESS_LIB_NOT_LOADED;
    default:
        return TINTLESS_UNKNOWN_ERROR;
    }
}

void tintless_interface_open(tintless_rolloff_ops_t *ops)
{
  ops->isp_tintless_open = isp_tintless_open;
  ops->isp_tintless_stat_config = isp_tintless_stat_config;
  ops->isp_tintless_mesh_config = isp_tintless_mesh_config;
  ops->isp_tintless_update_chromatix_params = isp_tintless_update_chromatix_params;
  ops->isp_tintless_get_version = isp_tintless_get_version;
  ops->isp_tintless_algo = isp_tintless_algo;
  ops->isp_tintless_close = isp_tintless_close;
}
