
/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_NAS_UTILS
#define CRI_NAS_UTILS

#include "utils_common.h"
#include "cri_core.h"

#define CHAR_CR           0x0D

static const int gsm_def_alpha_to_utf8_table[] =
{
    /* DEC   0       1       2       3       4       5       6       7
    HEX 0X00    0X01    0X02    0X03    0X04    0X05    0X06    0X07
    @       £       $     Yen      e\       e/      u\      i\.         */
    0x0040,    0xC2A3, 0x0024, 0xC2A5, 0xC3A8, 0xC3A9, 0xC3B9, 0xC3AC,

    /* DEC  8       9       10      11      12      13      14      15
    HEX 0X08    0X09    0X0A    0X0B    0X0C    0X0D    0X0E    0X0F
    o\     C,      LF      O|     o|      CR       A0      a0           */
    0xC3B2,    0xC387, 0x000A, 0xC398, 0xC3B8, 0x000D, 0xC385, 0xC3A5,

    /* DEC  16      17      18      19      20      21      22      23
    HEX 0X10    0X11    0X12    0X13    0X14    0X15    0X16    0X17
    Delta    _     ... Greek ...                                         */
    0xCE94,    0x005F, 0xCEA6, 0xCE93, 0xCE9B, 0xCEA9, 0xCEA0, 0xCEA8,

    /* DEC  24      25      26      27      28      29      30      31
    HEX 0X18    0X19    0X1A    0X1B    0X1C    0X1D    0X1E    0X1F
    ... Greek ...           Esc     AE      ae    GrossS     E/          */
    0xCEA3,    0xCE98, 0xCE9E, 0x0020, 0xC386, 0xC3A6, 0xC39F, 0xC389,

    /* DEC  32      33      34      35      36      37      38      39
    HEX 0X20    0X21    0X22    0X23    0X24    0X25    0X26    0X27
    SPC      !       "       #      OX       %       &       '         */
    0x0020,    0x0021, 0x0022, 0x0023, 0xC2A4, 0x0025, 0x0026, 0x0027,

    /* DEC  40      41      42      43      44      45      46      47
    HEX 0X28    0X29    0X2A    0X2B    0X2C    0X2D    0X2E    0X2F
    (       )       *       +       ,       -       .       /          */
    0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,

    /* DEC  48      49      50      51      52      53      54      55
    HEX 0X30    0X31    0X32    0X33    0X34    0X35    0X36    0X37       */
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,

    /* DEC  56      57      58      59      60      61      62      63
    HEX 0X38    0X39    0X3A    0X3B    0X3C    0X3D    0X3E    0X3F
    8       9       :       ;       <       =       >       ?         */
    0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,

    /* DEC  64      65      66      67      68      69      70      71
    HEX 0X40    0X41    0X42    0X43    0X44    0X45    0X46    0X47
    .|      A       B       C       D       E       F       G         */
    0xC2A1,    0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,

    /* DEC  72      73      74      75      76      77      78      79
    HEX 0X48    0X49    0X4A    0X0X4B  0X4C    0X4D    0X4E    0X4F       */
    0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,

    /* DEC  80      81      82      83      84      85      86      87
    HEX 0X50    0X51    0X52    0X53    0X54    0X55    0X56    0X57       */
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,

    /* DEC  88      89      90      91      92      93      94      95
    HEX 0X58    0X59    0X5A    0X5B    0X5C    0X5D    0X5E    0X5F
    X       Y       Z       A"      0"      N~      U"     S\S       */
    0x0058,    0x0059, 0x005A, 0xC384, 0xC396, 0xC391, 0xC39C, 0xC2A7,

    /* DEC  96      97      98      99      100     101     102     103
    HEX 0X60    0X61    0X62    0X63    0X64    0X65    0X66    0X67
    ';      a       b       c       d       e       f       g         */
    0xC2BF,    0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,

    /* DEC  104     105     106     107     108     109     110     111
    HEX 0X68    0X69    0X6A    0X6B    0X6C    0X6D    0X6E    0X6F      */
    0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,

    /* DEC  112     113     114     115     116     117     118     119
    HEX 0X70    0X71    0X72    0X73    0X74    0X75    0X76    0X77     */
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,

    /* DEC  120     121     122     123     124     125     126     127
    HEX 0X78    0X79    0X7A    0X7B    0X7C    0X7D    0X7E    0X7F
    x       y       z       a"      o"      n~      u"      a      */
    0x0078,    0x0079, 0x007A, 0xC3A4, 0xC3B6, 0xC3B1, 0xC3BC, 0xC3A0
};

#define MAX_USS_CHAR 160


/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR (unsigned long)0x0000FFFD
#define UNI_MAX_BMP (unsigned long)0x0000FFFF
#define UNI_MAX_UTF16 (unsigned long)0x0010FFFF
#define UNI_MAX_unsigned long (unsigned long)0x7FFFFFFF
#define UNI_MAX_LEGAL_unsigned long (unsigned long)0x0010FFFF

#define UNI_SUR_HIGH_START  (unsigned long)0xD800
#define UNI_SUR_HIGH_END    (unsigned long)0xDBFF
#define UNI_SUR_LOW_START   (unsigned long)0xDC00
#define UNI_SUR_LOW_END     (unsigned long)0xDFFF


// NAS cache
uint32_t current_pref_mode;

qmi_error_type_v01 cri_nas_utils_init_client(hlos_ind_cb_type hlos_ind_cb);


void cri_nas_utils_release_client(int qmi_service_client_id);

int cri_nas_convert_gsm8bit_alpha_string_to_utf8
(
    const char *gsm_data,
    int gsm_data_len,
    char *utf8_buf
);


int cri_nas_convert_gsm_def_alpha_string_to_utf8
(
    const char *gsm_data,
    unsigned char gsm_data_len,
    char *utf8_buf
);

void cri_nas_decode_operator_name_in_little_endian
(
    char *dest,
    unsigned short max_dest_length,
    int coding_scheme,
    const unsigned char *src,
    unsigned short src_length
);

int cri_nas_is_operator_name_empty_or_white_space ( char * str, int max_len);


#endif

