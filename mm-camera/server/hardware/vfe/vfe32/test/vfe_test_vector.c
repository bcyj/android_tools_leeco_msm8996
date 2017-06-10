/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "vfe_test_vector.h"
#include "vfe.h"
#include "camera_dbg.h"
#include <ctype.h>

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef ENABLE_TEST_VECTOR_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif
#define VFE_MOD_INVALID 0

#define VFE_TEST_VEC_PARSE_INPUT 0
#define VFE_TEST_VEC_PARSE_OUTPUT 1

#define MAX_LINE_LEN 256
#define FILE_PATH "/data/vfe/output%d.txt"

#define VFE_TEST_VEC_FALSE "FALSE"
#define VFE_TEST_VEC_TRUE "TRUE"

#define CASE_NUMBER              "case number"
#define CASE_NAME                "case name"
#define AWB_R_GAIN               "AWB_r_gain"
#define AWB_G_GAIN               "AWB_g_gain"
#define AWB_B_GAIN               "AWB_b_gain"
#define AWB_DECISION             "AWB_decision"
#define AWB_CCT                  "AWB_CCT"
#define LED_ENABLE               "LED_enable"
#define STROBE_ENABLE            "Strobe_enable"
#define LED_SENSITIVITY_OFF      "LED_sensitivity_off"
#define LED_SENSITIVITY_HIGH     "LED_sensitivity_high"
#define STROBE_SENSITIVITY_OFF   "strobe_sensitivity_off"
#define STROBE_SENSITIVITY_HIGH  "strobe_sensitivity_high"
#define LUX_INDEX                "Lux_index"
#define TOTAL_GAIN               "total_gain"
#define DIGITAL_GAIN             "digital_gain"
#define CAMIF_WIDTH              "CAMIF_width"
#define CAMIF_HEIGHT             "CAMIF_height"
#define OUTPUT_WIDTH             "output_width"
#define OUTPUT_HEIGHT            "output_height"
#define FULL_WIDTH               "Full_width"
#define FULL_HEIGHT              "Full_height"
#define AEC_SETTLE               "AEC_settle"
#define AEC_CONVERGENCE          "AEC_convergence"
#define AEC_MAX                  "AEC_max"
#define MODE                     "Mode"
#define PREVIEW                  "Preview"

#define VFE_TEST_VEC_COLOR_CORRECTION      "color correction"
#define VFE_TEST_VEC_DEMOSAIC              "demosaic"
#define VFE_TEST_VEC_CLF                   "clf"
#define VFE_TEST_VEC_BPC                   "bpc"
#define VFE_TEST_VEC_BCC                   "bcc"
#define VFE_TEST_VEC_LINEARIZATION         "linearization"
#define VFE_TEST_VEC_COLOR_CONVERSION      "Color Conversion"
#define VFE_TEST_VEC_SCE                   "SCE"
#define VFE_TEST_VEC_PCA_ROLLOFF           "PCA rolloff "
#define VFE_TEST_VEC_GAMMA                 "gamma"
#define VFE_TEST_VEC_ASF                   "ASF"
#define VFE_TEST_VEC_ABF                   "ABF"
#define VFE_TEST_VEC_AWB                   "AWB"
#define VFE_TEST_VEC_LA                    "LA"
#define VFE_TEST_VEC_MCE                   "MCE"
#define VFE_TEST_VEC_CS                    "CS"

#define VFE_TEST_VEC_ADDRESS "address"

#define VFE_TEST_VEC_IS_TRUE(bool_str) \
  (0 == strcmp(bool_str, VFE_TEST_VEC_TRUE))

static void vfe_test_vector_output_deinit(vfe_test_vector_t *mod);
static vfe_status_t vfe_test_vector_output_init(vfe_test_vector_t *mod);

/*===========================================================================
 * FUNCTION    - test_vector_str_to_lower -
 *
 * DESCRIPTION:
 *==========================================================================*/
static char* test_vector_str_to_lower(char *str)
{
  int i = 0;
  int len = strlen(str);
  for (i=0; i<len; i++)
    str[i] = tolower(str[i]);
  return str;
} /*test_vector_str_to_lower*/

/*===========================================================================
 * FUNCTION    - test_vector_str_to_token -
 *
 * DESCRIPTION:
 *==========================================================================*/
static char* test_vector_str_to_token(char *str)
{
  int i = 0;
  int len = 0;
  /* remove starting space*/
  while(*str == ' ')
    str++;

  if (*str == '\"')
    str++;
  len = strlen(str);
  for (i=(len-1); i>=0; i--) {
    if (str[i] != ' ')
      break;
  }
  if (str[i] == '\"')
    i--;
  str[i+1] = '\0';
  return str;
} /*test_vector_str_to_token*/

/*===========================================================================
 * FUNCTION    - test_vector_strcmp -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int test_vector_strcmp(const char *str1, const char *str2)
{
  int i = 0, j = 0;
  int len2 = strlen(str2);
  /* skip initial white spaces */
  while (str1[i] == ' ')
    i++;

  while((str1[i] != '\0') || (str1[i] != '\n')) {
    if (j >= len2)
      break;
    if (tolower(str1[i]) != tolower(str2[j]))
      return -1;
    i++;
    j++;
  }
  if (j == len2)
    return 0;
  return -1;
} /*test_vector_strcmp*/

/*===========================================================================
 * FUNCTION    - test_vector_update_param -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_update_param(vfe_test_vector_t *mod,
  const char* attribute, const char* value)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_ctrl_info_t *p_vfe_obj = (vfe_ctrl_info_t *)(mod->vfe_obj);
  vfe_params_t *params = &p_vfe_obj->vfe_params;

  if (!strcmp(AWB_R_GAIN, attribute)) {
    int data = atoi(value);
    CDBG("%s: AWB_r_gain %d", __func__, data);
    mod->params.awb_gains.r_gain = Q_TO_FLOAT(7, data);
  } else if (!strcmp(AWB_G_GAIN, attribute)) {
    int data = atoi(value);
    CDBG("%s: AWB_g_gain %d", __func__, data);
    mod->params.awb_gains.g_gain = Q_TO_FLOAT(7, data);
  } else if (!strcmp(AWB_B_GAIN, attribute)) {
    int data = atoi(value);
    CDBG("%s: AWB_b_gain %d", __func__, data);
    mod->params.awb_gains.b_gain = Q_TO_FLOAT(7, data);
  } else if (!strcmp(AWB_DECISION, attribute)) {
    CDBG("%s: AWB_decision %s", __func__, value);
  } else if (!strcmp(AWB_CCT, attribute)) {
    int data = atoi(value);
    CDBG("%s: AWB_CCT %d", __func__, data);
    mod->params.color_temp = data;
  } else if (!strcmp(LED_ENABLE, attribute)) {
    int data = VFE_TEST_VEC_IS_TRUE(value);
    CDBG("%s: LED_ENABLE %s %d", __func__, value, data);
    mod->params.flash_mode = (data == TRUE) ?
      VFE_FLASH_LED : VFE_FLASH_NONE;
  } else if (!strcmp(STROBE_ENABLE, attribute)) {
    int data = VFE_TEST_VEC_IS_TRUE(value);
    CDBG("%s: STROBE_ENABLE %s %d", __func__, value, data);
    mod->params.flash_mode = (data == TRUE) ?
      VFE_FLASH_STROBE : VFE_FLASH_NONE;
  } else if (!strcmp(LED_SENSITIVITY_OFF, attribute)) {
    float data = atof(value);
    CDBG("%s: LED_SENSITIVITY_OFF %f", __func__, data);
    mod->params.sensitivity_led_off = data;
  } else if (!strcmp(LED_SENSITIVITY_HIGH, attribute)) {
    float data = atof(value);
    CDBG("%s: LED_SENSITIVITY_HIGH %f", __func__, data);
    mod->params.sensitivity_led_hi = data;
  } else if (!strcmp(STROBE_SENSITIVITY_OFF, attribute)) {
    float data = atof(value);
    CDBG("%s: STROBE_SENSITIVITY_OFF %f", __func__, data);
    mod->params.sensitivity_led_off = data;
  } else if (!strcmp(STROBE_SENSITIVITY_HIGH, attribute)) {
    float data = atof(value);
    CDBG("%s: STROBE_SENSITIVITY_HIGH %f", __func__, data);
    mod->params.sensitivity_led_hi= data;
  } else if (!strcmp(LUX_INDEX, attribute)) {
    int data = atoi(value);
    mod->params.lux_idx = data;
    CDBG("%s: LUX_INDEX %d %f", __func__, data, mod->params.lux_idx);
  } else if (!strcmp(TOTAL_GAIN, attribute)) {
    int data = atoi(value);
    /* convert to float */
    CDBG("%s: TOTAL_GAIN %d", __func__, data);
    mod->params.cur_real_gain = Q_TO_FLOAT(8, data);
  } else if (!strcmp(DIGITAL_GAIN, attribute)) {
    int data = atoi(value);
    /* convert to float */
    mod->params.digital_gain = Q_TO_FLOAT(8, data);
    CDBG("%s: DIGITAL_GAIN %d", __func__, data);
  } else if (!strcmp(CAMIF_WIDTH, attribute)) {
    int data = atoi(value);
    CDBG("%s: CAMIF_WIDTH %d", __func__, data);
    mod->camif_size.width = data;
  } else if (!strcmp(CAMIF_HEIGHT, attribute)) {
    int data = atoi(value);
    CDBG("%s: CAMIF_HEIGHT %d", __func__, data);
    mod->camif_size.height = data;
  } else if (!strcmp(OUTPUT_WIDTH, attribute)) {
    int data = atoi(value);
    CDBG("%s: OUTPUT_WIDTH %d", __func__, data);
    mod->output_size.width = data;
  } else if (!strcmp(OUTPUT_HEIGHT, attribute)) {
    int data = atoi(value);
    CDBG("%s: OUTPUT_HEIGHT %d", __func__, data);
    mod->output_size.height = data;
  } else if (!strcmp(FULL_WIDTH, attribute)) {
    int data = atoi(value);
    CDBG("%s: FULL_WIDTH %d", __func__, data);
    mod->full_size.width = data;
  } else if (!strcmp(FULL_HEIGHT, attribute)) {
    int data = atoi(value);
    CDBG("%s: FULL_HEIGHT %d", __func__, data);
    mod->full_size.height = data;
  } else if (!strcmp(AEC_SETTLE, attribute)) {
    CDBG("%s: AEC_SETTLE %s", __func__, value);
  } else if (!strcmp(AEC_CONVERGENCE, attribute)) {
    CDBG("%s: AEC_CONVERGENCE %s", __func__, value);
  } else if (!strcmp(AEC_MAX, attribute)) {
    CDBG("%s: AEC_MAX %s", __func__, value);
  } else if (!strcmp(MODE, attribute)) {
    mod->snapshot_mode = strcmp(PREVIEW, value) ? 1 : 0;
    CDBG("%s: MODE %s %d", __func__, value, mod->snapshot_mode);
    /* change parse mode */
    mod->parse_mode = VFE_TEST_VEC_PARSE_OUTPUT;
  } else if (!strcmp(CASE_NAME, attribute)) {
    CDBG("%s: CASE_NAME %s", __func__, value);
  } else if (!strcmp(CASE_NUMBER, attribute)) {
    CDBG("%s: CASE_NUMBER %s", __func__, value);
  } else {
    CDBG("%s: invalid token %s %s", __func__, attribute, value);
  }
  return status;
}/*test_vector_update_param*/

/*===========================================================================
 * FUNCTION    - test_vector_extract_input_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_extract_input_params(vfe_test_vector_t *mod,
  char* line)
{
  const int MAX_TOKEN = 3;
  char *val[MAX_TOKEN];
  char *last;
  int index = 0;

  val[index] = strtok_r (line, "=", &last);
  while (val[index] != NULL) {
    if (index > 1) {
      CDBG("%s: line %s parse failed", __func__, val[index]);
      break;
    }
    ++index;
    val[index] = strtok_r (NULL, "=", &last);
  }

  val[1] = test_vector_str_to_token(val[1]);
  CDBG("%s: attr \"%s\" val \"%s\" ", __func__, val[0], val[1]);
  return test_vector_update_param(mod, val[0], val[1]);
}/*test_vector_extract_input_params*/

/*===========================================================================
 * FUNCTION    - test_vector_extract_input_module_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_extract_input_module_params(vfe_test_vector_t *mod,
  char* line, uint32_t *module_type)
{
  vfe_status_t status = VFE_SUCCESS;
  const int MAX_TOKEN = 2;
  char *val[MAX_TOKEN];
  char *last;
  int index = 0, i;

  if (0 == test_vector_strcmp(line, VFE_TEST_VEC_COLOR_CORRECTION)) {
    *module_type = VFE_MOD_COLOR_CORRECT;
    CDBG("%s: Color correction", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_COLOR_CONVERSION)) {
    *module_type = VFE_MOD_COLOR_CONV;
    CDBG("%s: Color conversion", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_PCA_ROLLOFF)) {
    *module_type = VFE_MOD_ROLLOFF;
    mod->rolloff_type = VFE_TEST_VEC_PCA;
    CDBG("%s: Rolloff", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_DEMOSAIC)) {
    *module_type = VFE_MOD_DEMOSAIC;
    CDBG("%s: Demosaic", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_CLF)) {
    *module_type = VFE_MOD_CLF;
    CDBG("%s: CLF", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_BPC)) {
    *module_type = VFE_MOD_BPC;
    CDBG("%s: BPC", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_BCC)) {
    *module_type = VFE_MOD_BCC;
    CDBG("%s: BCC", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_LINEARIZATION)) {
    *module_type = VFE_MOD_LINEARIZATION;
    CDBG("%s: Linearization", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_SCE)) {
    *module_type = VFE_MOD_SCE;
    CDBG("%s: SCE", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_GAMMA)) {
    *module_type = VFE_MOD_GAMMA;
    CDBG("%s: Gamma", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_ASF)) {
    *module_type = VFE_MOD_ASF;
    CDBG("%s: ASF", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_AWB)) {
    *module_type = VFE_MOD_WB;
    CDBG("%s: WB", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_ABF)) {
    *module_type = VFE_MOD_ABF;
    CDBG("%s: ABF", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_LA)) {
    *module_type = VFE_MOD_LA;
    CDBG("%s: LA", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_MCE)) {
    *module_type = VFE_MOD_MCE;
    CDBG("%s: MCE", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_CS)) {
    *module_type = VFE_MOD_CHROMA_SUPPRESS;
    CDBG("%s: Chroma SS", __func__);
  } else {
    /* not a header */
    *module_type = VFE_MOD_INVALID;
  }
  return status;
}/*test_vector_extract_input_module_params*/

/*===========================================================================
 * FUNCTION    - test_vector_extract_reg_values -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_extract_reg_values(vfe_test_vector_t *mod,
  char** p_val, int count)
{
  vfe_status_t status = VFE_SUCCESS;
  if (count < 3)
    return status;
  uint32_t val[3];
  CDBG("%s: %s %s %s", __func__, p_val[0], p_val[1], p_val[2]);
  sscanf(p_val[0],"%x", &val[0]);
  sscanf(p_val[1],"%x", &val[1]);
  sscanf(p_val[2],"%x", &val[2]);
  CDBG("%s: 0x%x 0x%x 0x%x", __func__, val[0], val[1], val[2]);
  if (val[0] > mod->mod_input.reg_size*4) {
    CDBG_ERROR("%s: invalid address", __func__);
    return VFE_ERROR_GENERAL;
  }
  mod->mod_input.reg_dump[val[0]/4] = val[1];
  mod->mod_input.reg_mask[val[0]/4] = val[2];
  return status;
}/*test_vector_extract_reg_values*/

/*===========================================================================
 * FUNCTION    - test_vector_extract_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_extract_table(vfe_test_vector_t *mod,
  char** p_val, int count, uint32_t *table, uint32_t size, uint32_t *mask,
  char *mod_name)
{
  int i;
  vfe_status_t status = VFE_SUCCESS;
  if (count < 3)
    return status;
  uint32_t j = 0;
  for (i = 0; i < count; i++) {
    if (!strcmp("value", p_val[i])) {
      i++;
      break;
    }
    if (!strcmp("mask", p_val[i])) {
      if (i+1 >= count)
        return VFE_ERROR_GENERAL;
      i++;
      sscanf(p_val[i],"%x", mask);
      CDBG("%s: %s mask 0x%x", __func__, mod_name, *mask);
    }
  }
  CDBG("%s: %s i %d", __func__, mod_name, i);
  for(; (i < count) && (j < size); i++) {
    sscanf(p_val[i],"%x", &table[j]);
    CDBG("%s: %s[%d] 0x%x %s", __func__, mod_name, j, table[j], p_val[i]);
    j++;
  }
  return status;
}/*test_vector_extract_table*/

/*===========================================================================
 * FUNCTION    - test_vector_extract_pca_rolloff_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_extract_pca_rolloff_table(vfe_test_vector_t *mod,
  char** p_val, int count)
{
  int i = 0;
  int shift = 0;
  vfe_status_t status = VFE_SUCCESS;
  int index = 1;
  uint64_t temp;
  vfe_test_table_64_t *pca_table = NULL;
  if (count < 3)
    return status;
  uint32_t j = 0;

  /* extract ram type */
  if (!test_vector_strcmp("table", p_val[i])) {
    if (i+1 >= count)
      return VFE_ERROR_GENERAL;
    i++;

    if (!test_vector_strcmp("PCA_rolloff_lower", p_val[i]))
      index = 1;
    else {
      index = 0;
      if (!test_vector_strcmp("PCA_rolloff_upper_hi", p_val[i]))
        shift = 32;
      else if (!test_vector_strcmp("PCA_rolloff_upper_lo", p_val[i]))
        shift = 0;
      else {
        CDBG("%s: Invalid table", __func__);
        return VFE_ERROR_GENERAL;
      }
    }
    i++;
  }
  CDBG("%s: index %d", __func__, index);
  pca_table = &mod->mod_input.pca_rolloff.ram[index];

  for (; i < count; i++) {
    if (!strcmp("value", p_val[i])) {
      i++;
      break;
    }
    if (!strcmp("mask", p_val[i])) {
      if (i+1 >= count)
        return VFE_ERROR_GENERAL;
      i++;
      sscanf(p_val[i],"%llx", &temp);
      pca_table->mask |= (temp << shift);
      CDBG("%s: mask 0x%llx", __func__, pca_table->mask);
    }
  }

  CDBG("%s: i %d", __func__, i);
  for(; (i < count) && (j < pca_table->size); i++) {
    sscanf(p_val[i],"%llx", &temp);
    pca_table->table[j] |= (temp << shift);
    CDBG("%s: PCA[%d] 0x%llx", __func__, j, pca_table->table[j]);
    j++;
  }
  return status;
}/*test_vector_extract_pca_rolloff_table*/

/*===========================================================================
 * FUNCTION    - test_vector_parse_module_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_extract_module_data(vfe_test_vector_t *mod,
  char** p_val, int count)
{
  vfe_status_t status = VFE_SUCCESS;
  uint32_t module_type = 1<<mod->current_index;
  int i = 0;
  CDBG("%s: current_index %d count %d", __func__, mod->current_index, count);
  switch(module_type) {
    case VFE_MOD_WB:
    case VFE_MOD_DEMUX:
    case VFE_MOD_CHROMA_SUPPRESS:
    case VFE_MOD_CHROMA_SS:
    case VFE_MOD_MCE:
    case VFE_MOD_AWB_STATS:
    case VFE_MOD_ABF:
    case VFE_MOD_ASF:
    case VFE_MOD_SCE:
    case VFE_MOD_COLOR_CONV:
    case VFE_MOD_BPC:
    case VFE_MOD_BCC:
    case VFE_MOD_CLF:
    case VFE_MOD_DEMOSAIC:
    case VFE_MOD_COLOR_CORRECT: {
      status = test_vector_extract_reg_values(mod, p_val, count);
      break;
    }
    case VFE_MOD_LINEARIZATION: {
      if (strcmp("table", test_vector_str_to_lower(p_val[0]))) {
        CDBG("%s: Processing Linearization reg %s", __func__, p_val[0]);
        status = test_vector_extract_reg_values(mod, p_val, count);
      } else {
        CDBG("%s: Processing Linearization table %s", __func__, p_val[0]);
        status = test_vector_extract_table(mod, p_val, count,
          mod->mod_input.linearization.table,
          mod->mod_input.linearization.size,
          &mod->mod_input.linearization.mask,
          VFE_TEST_VEC_LINEARIZATION);
      }
      break;
    }
    case VFE_MOD_GAMMA: {
      status = test_vector_extract_table(mod, p_val, count,
        mod->mod_input.gamma.table,
        mod->mod_input.gamma.size,
        &mod->mod_input.gamma.mask,
        VFE_TEST_VEC_GAMMA);
      break;
    }
    case VFE_MOD_LA: {
      status = test_vector_extract_table(mod, p_val, count,
        mod->mod_input.la.table,
        mod->mod_input.la.size,
        &mod->mod_input.la.mask,
        VFE_TEST_VEC_LA);
      break;
    }
    case VFE_MOD_ROLLOFF: {
      if (mod->rolloff_type == VFE_TEST_VEC_PCA)
        status = test_vector_extract_pca_rolloff_table(mod, p_val, count);
      else
        CDBG("%s: Mesh rolloff not supported", __func__);
      break;
    }
    default:;
  }
  return status;
}

/*===========================================================================
 * FUNCTION    - test_vector_parse_module_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_parse_module_data(vfe_test_vector_t *mod, char* line)
{
  vfe_status_t status = VFE_SUCCESS;
  const int max_token = strlen(line);
  char **p_val = NULL;
  char *last;
  int count = 0;

  if (max_token <= 1)
    return VFE_SUCCESS;

  if (mod->current_index < 0) {
    CDBG("%s: Invalid index %d", __func__, mod->current_index);
    return VFE_SUCCESS;
  }
  p_val = (char **)malloc(max_token * sizeof(char *));

  p_val[count] = strtok_r (line, " =,[]", &last);
  if (!strcmp(VFE_TEST_VEC_ADDRESS, p_val[0])) {
    /* skip this line */
    goto end;
  }
  while (p_val[count] != NULL) {
    ++count;
    p_val[count] = strtok_r (NULL, " =,[]", &last);
  }

  test_vector_extract_module_data(mod, p_val, count);

end:
  if (p_val)
    free(p_val);
  return status;
}/*test_vector_parse_module_data*/

/*===========================================================================
 * FUNCTION    - test_vector_parse_input -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_parse_input(vfe_test_vector_t *mod)
{
  vfe_status_t status = VFE_SUCCESS;
  char *token, *last;
  int bytes_read;
  /*calculate size*/
  fseek(mod->fp, 0L, SEEK_END);
  mod->input_size = ftell(mod->fp);
  fseek(mod->fp, 0L, SEEK_SET);
  CDBG("%s: input_size %d", __func__, mod->input_size);
  mod->input_data = (char *)malloc(mod->input_size * sizeof(char));
  if (mod->input_data == NULL) {
    CDBG_ERROR("%s: cannot allocate input", __func__);
    return VFE_ERROR_NO_MEMORY;
  }
  bytes_read = fread (mod->input_data, 1, mod->input_size, mod->fp);
  CDBG("%s: bytes_read %d input_size %d", __func__,
     bytes_read, mod->input_size);
  if (bytes_read < mod->input_size) {
    CDBG_ERROR("%s: cannot get the data from file %d %d", __func__,
      bytes_read, mod->input_size);
    return VFE_ERROR_GENERAL;
  }
  fclose(mod->fp); /* close file handle */

  mod->parse_mode = VFE_TEST_VEC_PARSE_INPUT;
  token = strtok_r (mod->input_data, "\n", &last);

  while (token != NULL) {
    int len = strlen(token);
    if (len > 0)
      token[len-1] = '\0';
    CDBG ("%s: token %s\n", __func__, token);
    if((len <=2) && (token[0] == '/') && (token[1] == '/')) {
      CDBG("%s: comment %s", __func__, token);
    } else if (mod->parse_mode == VFE_TEST_VEC_PARSE_INPUT) {
      status = test_vector_extract_input_params(mod, token);
      if (status != VFE_SUCCESS) {
        return status;
      }
    } else { /* parse output */
      uint32_t module_type;
      char *mod_ptr = NULL;
      status = test_vector_extract_input_module_params(mod, token,
        &module_type);
      if (status != VFE_SUCCESS)
        return status;
      if (module_type != VFE_MOD_INVALID) {
        uint32_t index = CEIL_LOG2(module_type);
        CDBG("%s: module_type %d index %d", __func__, module_type, index);
        if (index > VFE_MOD_COUNT) {
          CDBG_ERROR("%s: Invalid module %d", __func__, module_type);
          return VFE_ERROR_GENERAL;
        }
        mod->current_index = index;
      } else {
        status = test_vector_parse_module_data(mod, token);

        if (status != VFE_SUCCESS)
          return status;
      }
    }
    token = strtok_r (NULL, "\n", &last);
  }
  return status;
}/*test_vector_parse_input*/

/*===========================================================================
 * FUNCTION    - vfe_test_vector_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_test_vector_init(vfe_test_vector_t *mod, void* vfe_obj)
{
  vfe_status_t status = VFE_SUCCESS;
  int i = 0;
  memset(mod, 0, sizeof(vfe_test_vector_t));
  mod->vfe_obj = vfe_obj;
  mod->input_data = NULL;

#ifdef _ANDROID_
  char value[PROPERTY_VALUE_MAX];

  property_get("persist.camera.vfe.test.enable", value, "0");
  mod->enable = atoi(value);
  if (mod->enable != 1) {
    CDBG("%s: Not enabled", __func__);
    return VFE_SUCCESS;
  }

  property_get("persist.camera.vfe.test.index", value, "0");
  mod->index = atoi(value);
#endif
  sprintf(mod->filename, FILE_PATH, mod->index);
  CDBG_HIGH("%s: index %d name %s", __func__, mod->index, mod->filename);
  mod->fp = fopen(mod->filename, "r");
  if (NULL == mod->fp) {
    CDBG_HIGH("%s: fp NULL", __func__);
    return VFE_ERROR_GENERAL;
  }

  mod->current_index = -1;
  vfe_ctrl_info_t *p_vfe_obj = (vfe_ctrl_info_t *)(mod->vfe_obj);
  vfe_params_t *params = &p_vfe_obj->vfe_params;
  mod->mod_input.reg_size = (params->vfe_version == MSM8960V1) ?
    VFE32_REGISTER_TOTAL : VFE33_REGISTER_TOTAL;
  mod->mod_input.reg_dump = (uint32_t *)malloc(mod->mod_input.reg_size *
    sizeof(uint32_t));
  if (!mod->mod_input.reg_dump) {
    CDBG_ERROR("%s:%d] no memory", __func__, __LINE__);
    return VFE_ERROR_NO_MEMORY;
  }

  mod->mod_input.reg_mask = (uint32_t *)malloc(mod->mod_input.reg_size *
    sizeof(uint32_t));
  if (!mod->mod_input.reg_mask) {
    CDBG_ERROR("%s:%d] no memory", __func__, __LINE__);
    return VFE_ERROR_NO_MEMORY;
  }

  mod->mod_input.gamma.size = VFE_GAMMA_NUM_ENTRIES * 3;
  mod->mod_input.gamma.table = (uint32_t *)malloc(
    mod->mod_input.gamma.size * sizeof(uint32_t));
  if (!mod->mod_input.gamma.table) {
    CDBG_ERROR("%s:%d] no memory", __func__, __LINE__);
    return VFE_ERROR_NO_MEMORY;
  }

  mod->mod_input.linearization.size = VFE32_LINEARIZATON_TABLE_LENGTH;
  mod->mod_input.linearization.table = (uint32_t *)malloc(
    mod->mod_input.linearization.size * sizeof(uint32_t));
  if (!mod->mod_input.linearization.table) {
    CDBG_ERROR("%s:%d] no memory", __func__, __LINE__);
    return VFE_ERROR_NO_MEMORY;
  }

  for (i=0; i<2; i++) {
    mod->mod_input.pca_rolloff.ram[i].size = PCA_ROLLOFF_BASIS_TABLE_SIZE +
      PCA_ROLLOFF_COEFF_TABLE_SIZE;
    mod->mod_input.pca_rolloff.ram[i].table = (uint64_t *)malloc(
      mod->mod_input.pca_rolloff.ram[i].size * sizeof(uint64_t));
    if (!mod->mod_input.pca_rolloff.ram[i].table) {
      CDBG_ERROR("%s:%d] no memory", __func__, __LINE__);
      return VFE_ERROR_NO_MEMORY;
    }
    memset(mod->mod_input.pca_rolloff.ram[i].table, 0x0,
      mod->mod_input.pca_rolloff.ram[i].size * sizeof(uint64_t));
  }

  status = vfe_test_vector_output_init(mod);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_test_vector_init failed\n", __func__);
    return status;
  }

  status = test_vector_parse_input(mod);
  return status;
}/*vfe_test_vector_init*/

/*===========================================================================
 * FUNCTION    - vfe_test_vector_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_test_vector_deinit(vfe_test_vector_t *mod)
{
  int i = 0;
  if (mod->input_data) {
    free(mod->input_data);
    mod->input_data = NULL;
  }
  if (mod->mod_input.reg_dump) {
    free(mod->mod_input.reg_dump);
    mod->mod_input.reg_dump = NULL;
  }
  if (mod->mod_input.reg_mask) {
    free(mod->mod_input.reg_mask);
    mod->mod_input.reg_mask = NULL;
  }
  if (mod->mod_input.gamma.table) {
    free(mod->mod_input.gamma.table);
    mod->mod_input.gamma.table = NULL;
  }
  if (mod->mod_input.linearization.table) {
    free(mod->mod_input.linearization.table);
    mod->mod_input.linearization.table = NULL;
  }
  for (i=0; i<2; i++) {
    if (mod->mod_input.pca_rolloff.ram[i].table) {
      free(mod->mod_input.pca_rolloff.ram[i].table);
      mod->mod_input.pca_rolloff.ram[i].table = NULL;
    }
  }
  vfe_test_vector_output_deinit(mod);
  return VFE_SUCCESS;
}/*vfe_test_vector_deinit*/

/*===========================================================================
 * FUNCTION    - vfe_test_vector_enabled -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_test_vector_enabled(vfe_test_vector_t *mod)
{
  vfe_ctrl_info_t *p_vfe_obj = (vfe_ctrl_info_t *)(mod->vfe_obj);
  vfe_params_t *params = &p_vfe_obj->vfe_params;
  int is_snap = IS_SNAP_MODE(params);
  CDBG("%s: enable %d snap %d mode %d", __func__, mod->enable, is_snap,
    mod->snapshot_mode);
  return mod->enable && (is_snap == mod->snapshot_mode);
}/*vfe_test_vector_enabled*/

/*===========================================================================
 * FUNCTION    - vfe_test_vector_update_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_test_vector_update_params(vfe_test_vector_t *mod)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_ctrl_info_t *p_vfe_obj = (vfe_ctrl_info_t *)(mod->vfe_obj);
  vfe_params_t *params = &p_vfe_obj->vfe_params;
  vfe_test_params_t *test_params = &mod->params;

  if (mod->params_updated)
    return VFE_SUCCESS;

  params->sharpness_info.ui_sharp_ctrl_factor = 1/3.9;
  params->sharpness_info.downscale_factor =
    MIN(mod->camif_size.height / mod->output_size.height,
    mod->camif_size.width / mod->output_size.width);
  params->awb_params.gain = test_params->awb_gains;
  params->awb_params.color_temp = test_params->color_temp;
  params->flash_params.flash_mode = test_params->flash_mode;
  params->flash_params.sensitivity_led_hi = test_params->sensitivity_led_hi;
  params->flash_params.sensitivity_led_low = test_params->sensitivity_led_low;
  params->flash_params.sensitivity_led_off = test_params->sensitivity_led_off;
  params->digital_gain = test_params->digital_gain;
  params->aec_params.lux_idx = test_params->lux_idx;
  if (!mod->snapshot_mode)
    params->aec_params.cur_real_gain = test_params->cur_real_gain;
  else
    params->aec_params.snapshot_real_gain = test_params->cur_real_gain;

  CDBG("%s: UI downscale factor %f ", __func__,
    params->sharpness_info.ui_sharp_ctrl_factor);
  CDBG("%s: downscale factor %f ", __func__,
    params->sharpness_info.downscale_factor);
  CDBG("%s: AWB gains r %f g %f b %f", __func__,
    params->awb_params.gain.r_gain, params->awb_params.gain.g_gain,
    params->awb_params.gain.b_gain);
  CDBG("%s: color_temp %d", __func__, params->awb_params.color_temp);
  CDBG("%s: flash_mode %d", __func__, params->flash_params.flash_mode);
  CDBG("%s: sensitivity_led_hi %d", __func__,
    params->flash_params.sensitivity_led_hi);
  CDBG("%s: sensitivity_led_low %d", __func__,
    params->flash_params.sensitivity_led_low);
  CDBG("%s: sensitivity_led_off %d", __func__,
    params->flash_params.sensitivity_led_off);
  CDBG("%s: digital_gain %f", __func__, params->digital_gain);
  CDBG("%s: lux_idx %f", __func__, params->aec_params.lux_idx);
  CDBG("%s: cur_real_gain %f", __func__, params->aec_params.cur_real_gain);
  CDBG("%s: snapshot_real_gain %f", __func__,
    params->aec_params.snapshot_real_gain);

  mod->params_updated = TRUE;
  return VFE_SUCCESS;
}/*vfe_test_vector_update_params*/

/*===========================================================================
FUNCTION    - vfe_test_get_register_dump -

DESCRIPTION
===========================================================================*/
static vfe_status_t vfe_test_get_register_dump(vfe_test_vector_t *mod)
{
  vfe_status_t status = VFE_SUCCESS;
  uint32_t *data = NULL, *dummy = NULL;
  uint32_t i;
  vfe_test_module_output_t *tv_params = &(mod->mod_output);
  vfe_ctrl_info_t *p_vfe_obj = (vfe_ctrl_info_t *)(mod->vfe_obj);
  vfe_params_t *params = &p_vfe_obj->vfe_params;

  CDBG("%s: enter", __func__);
  data = tv_params->reg_dump_data;
  if (!data) {
    if (NULL == (data = malloc(tv_params->reg_dump_len))) {
      CDBG_HIGH("%s: Not enough memory\n", __func__);
      return VFE_ERROR_GENERAL;
    }
  }

  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL, data,
    tv_params->reg_dump_len, VFE_CMD_GET_REG_DUMP);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: VFE_CMD_GET_REG_VALUE failed\n", __func__);
    return status;
  }

  dummy = data;
  for (i = 0; i < (tv_params->register_total/4); i++) {
    CDBG_HIGH("%08x: %08x %08x %08x %08x\n", (uint32_t)(i*16), *dummy,
      *(dummy + 1),
      *(dummy + 2), *(dummy + 3));
    dummy += 4;
  }

  if ((tv_params->register_total % 4) == 3)
    CDBG_HIGH("%08x: %08x %08x %08x\n", (uint32_t)(i*16), *dummy, *(dummy + 1),
      *(dummy + 2));
  else if ((tv_params->register_total % 4) == 2)
    CDBG_HIGH("%08x: %08x %08x\n", (uint32_t)(i*16), *dummy, *(dummy + 1));
  else if ((tv_params->register_total % 4) == 1)
    CDBG_HIGH("%08x: %08x\n", (uint32_t)(i*16), *dummy);

  if (data != tv_params->reg_dump_data)
    tv_params->reg_dump_data = data;

  return status;
} /* vfe_test_get_register_dump */

/*===========================================================================
FUNCTION    - vfe_test_linearization_get_hw_table -

DESCRIPTION
===========================================================================*/
static vfe_status_t vfe_test_linearization_get_hw_table(vfe_test_vector_t *mod)
{
  vfe_status_t status = VFE_SUCCESS;
  uint32_t i;
  uint32_t *data = NULL, *dummy = NULL;
  vfe_test_module_output_t *tv_params = &(mod->mod_output);
  vfe_ctrl_info_t *p_vfe_obj = (vfe_ctrl_info_t *)(mod->vfe_obj);
  vfe_params_t *params = &p_vfe_obj->vfe_params;

  CDBG("%s: enter", __func__);
  data = tv_params->linearization_table;
  if (!data) {
    if (NULL == (data = malloc(tv_params->linearization_table_len))) {
      CDBG_HIGH("%s: Not enough memory\n", __func__);
      return VFE_ERROR_GENERAL;
    }
  }

  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL, data,
    tv_params->linearization_table_len, VFE_CMD_GET_LINEARIZATON_TABLE);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: VFE_CMD_GET_LINEARIZATON_TABLE failed\n", __func__);
    return status;
  }

  dummy = data;
  CDBG("%s: Linearization Table\n", __func__);
  for (i = 0; i < VFE32_LINEARIZATON_TABLE_LENGTH; i++) {
    CDBG("%s: %08x\n", __func__, *dummy);
    dummy++;
  }

  if (data != tv_params->linearization_table)
    tv_params->linearization_table = data;

  return status;
} /* vfe_test_linearization_get_hw_table */

/*===========================================================================
FUNCTION    - vfe_test_rolloff_get_hw_table -

DESCRIPTION
===========================================================================*/
static vfe_status_t vfe_test_rolloff_get_hw_table(vfe_test_vector_t *mod)
{
  vfe_status_t status = VFE_SUCCESS;
  uint32_t i, cmd_id, num_entries = 0;
  uint32_t *data = NULL, *dummy = NULL;
  vfe_test_module_output_t *tv_params = &(mod->mod_output);
  vfe_ctrl_info_t *p_vfe_obj = (vfe_ctrl_info_t *)(mod->vfe_obj);
  vfe_params_t *params = &p_vfe_obj->vfe_params;

  data = tv_params->rolloff_table;
  if (!data) {
    CDBG("%s: Allocate memory\n", __func__);
    if (NULL == (data = malloc(tv_params->rolloff_table_len))) {
      CDBG_HIGH("%s: Not enough memory\n", __func__);
      return VFE_ERROR_GENERAL;
    }
  }

  cmd_id = (params->vfe_version == MSM8960V1) ?
    VFE_CMD_GET_MESH_ROLLOFF_TABLE : VFE_CMD_GET_PCA_ROLLOFF_TABLE;
  CDBG("%s: cmd_id = %d\n", __func__, cmd_id);
  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL, data,
    tv_params->rolloff_table_len, cmd_id);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: VFE_CMD_GET_ROLLOFF_TABLE failed\n", __func__);
    return status;
  }

  dummy = data;
  if (params->vfe_version == MSM8960V1) {
    CDBG("%s: Mesh RollOff init table\n", __func__);
    num_entries = MESH_ROLL_OFF_INIT_TABLE_SIZE * 2;
    for (i = 0; i < num_entries; i++) {
      CDBG("%s %08x \n", __func__, *dummy);
      dummy++;
    }
    CDBG("%s: Mesh RollOff Delta table\n", __func__);
    num_entries = MESH_ROLL_OFF_DELTA_TABLE_SIZE * 2;
    for (i = 0; i < num_entries; i++) {
      CDBG("%s %08x \n", __func__, *dummy);
      dummy++;
    }
  } else {
    CDBG("%s: PCA RollOff Ram0 table\n", __func__);
    num_entries = (PCA_ROLLOFF_BASIS_TABLE_SIZE + PCA_ROLLOFF_COEFF_TABLE_SIZE);
    for (i = 0; i < num_entries; i++) {
      CDBG("%s: %16llx \n", __func__, *((uint64_t *)dummy));
      dummy += 2;
    }
    CDBG("%s: PCA RollOff Ram1 table\n", __func__);
    num_entries = (PCA_ROLLOFF_BASIS_TABLE_SIZE + PCA_ROLLOFF_COEFF_TABLE_SIZE);
    for (i = 0; i < num_entries; i++) {
      CDBG("%s: %16llx \n", __func__, *((uint64_t *)dummy));
      dummy += 2;
    }
  }

  if (data != tv_params->rolloff_table)
    tv_params->rolloff_table = data;

  return status;
} /* vfe_test_rolloff_get_hw_table */

/*===========================================================================
FUNCTION    - vfe_test_gamma_get_hw_table -

DESCRIPTION
===========================================================================*/
static vfe_status_t vfe_test_gamma_get_hw_table(vfe_test_vector_t *mod)
{
  vfe_status_t status = VFE_SUCCESS;
  uint32_t i;
  uint32_t *data = NULL, *dummy = NULL;
  vfe_test_module_output_t *tv_params = &(mod->mod_output);
  vfe_ctrl_info_t *p_vfe_obj = (vfe_ctrl_info_t *)(mod->vfe_obj);
  vfe_params_t *params = &p_vfe_obj->vfe_params;

  CDBG("%s: enter", __func__);
  data = tv_params->gamma_table;
  if (!data) {
    if (NULL == (data = malloc(tv_params->gamma_table_len))) {
      CDBG_HIGH("%s: Not enough memory\n", __func__);
      return VFE_ERROR_GENERAL;
    }
  }

  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL, data,
    tv_params->gamma_table_len, VFE_CMD_GET_RGB_G_TABLE);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: VFE_CMD_GET_GAMMA_TABLE failed\n", __func__);
    return status;
  }

  dummy = data;
  CDBG("%s: Channel 0\n", __func__);
  for (i = 0; i < VFE_GAMMA_NUM_ENTRIES; i++) {
    CDBG("%s: %08x \n", __func__, *dummy);
    dummy++;
  }
  CDBG("%s: Channel 1\n", __func__);
  for (i = 0; i < VFE_GAMMA_NUM_ENTRIES; i++) {
    CDBG("%s: %08x \n", __func__, *dummy);
    dummy++;
  }
  CDBG("%s: Channel 2\n", __func__);
  for (i = 0; i < VFE_GAMMA_NUM_ENTRIES; i++) {
    CDBG("%s: %08x \n", __func__, *dummy);
    dummy++;
  }

  if (data != tv_params->gamma_table)
    tv_params->gamma_table = data;

  return status;
} /* vfe_test_gamma_get_hw_table */

/*===========================================================================
FUNCTION    - vfe_test_get_tables_from_hw -

DESCRIPTION
===========================================================================*/
static vfe_status_t vfe_test_get_tables_from_hw(vfe_test_vector_t *mod)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_test_module_output_t *tv_params = &(mod->mod_output);

  CDBG("%s: enter", __func__);
  status = vfe_test_linearization_get_hw_table(mod);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_test_linearization_get_hw_table failed\n", __func__);
    return status;
  }
  status = vfe_test_rolloff_get_hw_table(mod);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_test_rolloff_get_hw_table failed\n", __func__);
    return status;
  }
  status = vfe_test_gamma_get_hw_table(mod);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_test_gamma_get_hw_table failed\n", __func__);
    return status;
  }

  return status;
} /* vfe_test_get_tables_from_hw */

/*===========================================================================
 * FUNCTION    - vfe_test_vector_output_init -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY:
 *==========================================================================*/
static vfe_status_t vfe_test_vector_output_init(vfe_test_vector_t *mod)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_test_module_output_t *tv_params = &(mod->mod_output);
  vfe_ctrl_info_t *p_vfe_obj = (vfe_ctrl_info_t *)(mod->vfe_obj);
  vfe_params_t *params = &p_vfe_obj->vfe_params;

  CDBG("%s: enter", __func__);
  /* VFE register specific */
  tv_params->reg_dump_enable = TRUE;
  tv_params->reg_dump_data = NULL;
  if (params->vfe_version == MSM8960V1)
    tv_params->register_total = VFE32_REGISTER_TOTAL;
  else
    tv_params->register_total = VFE33_REGISTER_TOTAL;
  tv_params->reg_dump_len = sizeof(uint32_t) * tv_params->register_total;

  /* VFE module's table specific */
  tv_params->table_dump_enable = TRUE;

  tv_params->linearization_table = NULL;
  tv_params->linearization_table_len =
    sizeof(uint32_t) * VFE32_LINEARIZATON_TABLE_LENGTH;

  tv_params->rolloff_table = NULL;
  if (params->vfe_version == MSM8960V1)
    tv_params->rolloff_table_len = sizeof(uint32_t) *
      ((MESH_ROLL_OFF_INIT_TABLE_SIZE*2) + (MESH_ROLL_OFF_DELTA_TABLE_SIZE*2));
  else
    tv_params->rolloff_table_len = sizeof(uint64_t) * 2 *
      (PCA_ROLLOFF_BASIS_TABLE_SIZE + PCA_ROLLOFF_COEFF_TABLE_SIZE);

  tv_params->gamma_table = NULL;
  tv_params->gamma_table_len = sizeof(uint32_t) * VFE_GAMMA_NUM_ENTRIES * 3;

  return status;
} /* vfe_test_vector_output_init */

/*===========================================================================
 * FUNCTION    - vfe_test_vector_output_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_test_vector_output_deinit(vfe_test_vector_t *mod)
{
  vfe_test_module_output_t *tv_params = &(mod->mod_output);

  CDBG("%s: enter", __func__);
  if (tv_params->reg_dump_data) {
    free(tv_params->reg_dump_data);
    tv_params->reg_dump_data = NULL;
  }
  if (tv_params->linearization_table) {
    free(tv_params->linearization_table);
    tv_params->linearization_table = NULL;
  }
  if (tv_params->rolloff_table) {
    free(tv_params->rolloff_table);
    tv_params->rolloff_table = NULL;
  }
  if (tv_params->gamma_table) {
    free(tv_params->gamma_table);
    tv_params->gamma_table = NULL;
  }
} /* vfe_test_vector_output_deinit */

/*===========================================================================
 * FUNCTION    - vfe_test_vector_get_output -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY:
 *==========================================================================*/
vfe_status_t vfe_test_vector_get_output(vfe_test_vector_t *mod)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_test_module_output_t *tv_params = &(mod->mod_output);

  if (tv_params->reg_dump_enable) {
    status = vfe_test_get_register_dump(mod);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: vfe_test_get_register_dump failed", __func__);
      return status;
    }
    tv_params->reg_dump_enable = FALSE;
  }
  if (tv_params->table_dump_enable) {
    status = vfe_test_get_tables_from_hw(mod);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: vfe_test_get_tables_from_hw failed", __func__);
      return status;
    }
    tv_params->table_dump_enable = FALSE;
  }
  return status;
}/*vfe_test_vector_get_output*/

/*===========================================================================
 * FUNCTION    - vfe_test_vector_validate -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY:
 *==========================================================================*/
vfe_status_t vfe_test_vector_validate(vfe_test_vector_t *mod)
{
  vfe_status_t status = VFE_SUCCESS;
  int mod_id = 0;

  if (mod->validate)
    return VFE_SUCCESS;

  CDBG("%s: enter", __func__);
  status = vfe_wb_tv_validate(mod_id, &mod->mod_input, &mod->mod_output);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_wb_tv_validate failed", __func__);
  }

  status = vfe_gamma_tv_validate(mod_id, &mod->mod_input, &mod->mod_output);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_gamma_tv_validate failed", __func__);
  }

  status = vfe_color_correct_tv_validate(mod_id, &mod->mod_input, &mod->mod_output);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_color_correct_tv_validate failed", __func__);
  }

  status = vfe_color_conversion_tv_validate(mod_id, &mod->mod_input, &mod->mod_output);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_color_conversion_tv_validate failed", __func__);
  }

  status = vfe_abf_tv_validate(mod_id, &mod->mod_input, &mod->mod_output);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_abf_tv_validate failed", __func__);
  }

  status = vfe_clf_tv_validate(mod_id, &mod->mod_input, &mod->mod_output);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_clf_tv_validate failed", __func__);
  }

  status = vfe_linearization_tv_validate(mod_id, &(mod->mod_input),
    &(mod->mod_output));
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_linearization_tv_validate failed", __func__);
  }

  status = vfe_mce_tv_validate(mod_id, &(mod->mod_input),
    &(mod->mod_output));
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_mce_tv_validate failed", __func__);
  }

  status = vfe_sce_tv_validate(mod_id, &(mod->mod_input),
    &(mod->mod_output));
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_sce_tv_validate failed", __func__);
  }

  status = vfe_chroma_suppression_test_vector_validate(mod_id, &(mod->mod_input),
    &(mod->mod_output));
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_chroma_suppression_test_vector_validate failed",
      __func__);
  }

  status = vfe_rolloff_tv_validate(mod_id, &mod->mod_input,
    &mod->mod_output);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_pca_tv_validate failed", __func__);
  }

  status = vfe_demosaic_tv_validate(mod_id, &(mod->mod_input),
    &(mod->mod_output));
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_demosaic_tv_validate failed", __func__);
  }
  status = vfe_bcc_test_vector_validation(mod_id, &(mod->mod_input),
    &(mod->mod_output));
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_bcc_test_vector_validation failed", __func__);
  }
  status = vfe_bpc_test_vector_validation(mod_id, &(mod->mod_input),
    &(mod->mod_output));
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_bpc_test_vector_validation failed", __func__);
  }
  status = vfe_asf_test_vector_validation(mod_id, &mod->mod_input, &mod->mod_output);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_asf_tv_validate failed", __func__);
  }
  status = vfe_la_tv_validate(mod_id, &(mod->mod_input), &(mod->mod_output));
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: vfe_la_tv_validate failed", __func__);
  }

  mod->validate = TRUE;
  return status;
}
