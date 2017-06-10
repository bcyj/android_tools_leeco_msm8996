#ifndef LOC_API_INI_H
#define LOC_API_INI_H

#define LOC_MAX_PARAM_LENGTH               36
#define LOC_MAX_PARAM_STRING               80
#define LOC_MAX_PARAM_LINE                 80

/*=============================================================================
 *
 *                        MODULE TYPE DECLARATION
 *
 *============================================================================*/
#if !defined(LOC_ENG_CFG_H)
typedef struct
{
  char                           param_name[LOC_MAX_PARAM_LENGTH];
  void                          *param_ptr;
  char                           param_type;  /* 's' for number; 's' for string */
} loc_param_s_type;
#endif

/*=============================================================================
 *
 *                          MODULE EXTERNAL DATA
 *
 *============================================================================*/


/*=============================================================================
 *
 *                       MODULE EXPORTED FUNCTIONS
 *
 *============================================================================*/
extern void loc_read_parameter(void);

#endif /* LOC_API_INI_H */
