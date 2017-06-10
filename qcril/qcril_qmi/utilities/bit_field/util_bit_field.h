/***************************************************************************************************
    @file
    util_bit_field.h

    @brief
    Facilitates bit field operations by providing bit field related utilities.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef UTIL_BIT_FIELD
#define UTIL_BIT_FIELD

#include "utils_standard.h"

#define UTIL_BIT_FIELD_MAX (64)
typedef uint64_t util_bit_field_type;





/***************************************************************************************************
    @function
    util_bit_field_set_bits

    @brief
    Sets bits in a bit field.

    @param[in]
        bits_to_be_set
            bits to be set in the bit field

    @param[out]
        bit_field
            bit field to be used for setting bits

    @retval
    none
***************************************************************************************************/
void util_bit_field_set_bits(util_bit_field_type *bit_field,
                             util_bit_field_type bits_to_be_set);





/***************************************************************************************************
    @function
    util_bit_field_remove_bits

    @brief
    Removes bits in a bit field.

    @param[in]
        bits_to_be_removed
            bits to be removed from the bit field

    @param[out]
        bit_field
            bit field to be used for removed bits

    @retval
    none
***************************************************************************************************/
void util_bit_field_remove_bits(util_bit_field_type *bit_field,
                                util_bit_field_type bits_to_be_removed);





/***************************************************************************************************
    @function
    util_bit_field_is_bits_set

    @brief
    Checks If specific set of bits are set in a bit field.

    @param[in]
        bit_field
            bit field to be used for checking If a specific set of bits are set
        bits_to_be_checked
            bits to be checked in the bit field
        is_partial_match_accepted
            TRUE If a subset of set bits is good enough for a successful match, FALSE otherwise

    @param[out]
        none

    @retval
    TRUE If the match has been successful, FALSE otherwise
***************************************************************************************************/
int util_bit_field_is_bits_set(util_bit_field_type bit_field,
                               util_bit_field_type bits_to_be_checked,
                               int is_partial_match_accepted);





/***************************************************************************************************
    @function
    util_bit_field_log_set_bits

    @brief
    Logs the set bits with their corresponding names.

    @param[in]
        bit_field_to_be_logged
            bit field to be used for logging
        bits_names
            array of strings containing name for each bit
        bits_names_size
            length of array of strings containing name for each bit

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void util_bit_field_log_set_bits(util_bit_field_type bit_field_to_be_logged,
                                 const char *bits_names[],
                                 int bits_names_size);

#endif
