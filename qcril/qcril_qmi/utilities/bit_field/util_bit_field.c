/***************************************************************************************************
    @file
    util_bit_field.c

    @brief
    Implements functions supported in util_bit_field.h.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#include "util_bit_field.h"
#include "util_log.h"






/***************************************************************************************************
    @function
    util_bit_field_set_bits

    @implementation detail
    None.
***************************************************************************************************/
void util_bit_field_set_bits(util_bit_field_type *bit_field,
                             util_bit_field_type bits_to_be_set)
{
    if(bit_field)
    {
        *bit_field |= bits_to_be_set;
    }
}

/***************************************************************************************************
    @function
    util_bit_field_remove_bits

    @implementation detail
    None.
***************************************************************************************************/
void util_bit_field_remove_bits(util_bit_field_type *bit_field,
                                util_bit_field_type bits_to_be_removed)
{
    if(bit_field)
    {
        *bit_field &= (~bits_to_be_removed);
    }
}

/***************************************************************************************************
    @function
    util_bit_field_is_bits_set

    @implementation detail
    Uses XOR operation.
***************************************************************************************************/
int util_bit_field_is_bits_set(util_bit_field_type bit_field,
                               util_bit_field_type bits_to_be_checked,
                               int is_partial_match_accepted)
{
    int is_set;
    util_bit_field_type temp_bit_field;

    is_set = FALSE;
    temp_bit_field = NIL;

    temp_bit_field = bit_field & bits_to_be_checked;
    if(is_partial_match_accepted)
    {
        is_set = temp_bit_field ? TRUE : FALSE;
    }
    else
    {
        is_set = (temp_bit_field ^ bits_to_be_checked) ? FALSE : TRUE;
    }

    return is_set;
}

/***************************************************************************************************
    @function
    util_bit_field_log_set_bits

    @implementation detail
    None.
***************************************************************************************************/
void util_bit_field_log_set_bits(util_bit_field_type bit_field_to_be_logged,
                                 const char *bits_names[],
                                 int bits_names_size)
{
    int iter_bit_field;
    int number_of_bits_per_batch;
    int batch_bits_count;
    char batch_bits[256];

    iter_bit_field = 0;
    batch_bits_count = 0;
    number_of_bits_per_batch = 8;
    memset(batch_bits,
           0,
           sizeof(batch_bits));


    if(bits_names && bits_names_size <= UTIL_BIT_FIELD_MAX)
    {
        while(iter_bit_field < bits_names_size)
        {
            if(bit_field_to_be_logged & 1)
            {
                if(batch_bits_count)
                {
                    strlcat(batch_bits,
                            " ",
                            sizeof(batch_bits));
                }
                strlcat(batch_bits,
                        bits_names[iter_bit_field],
                        sizeof(batch_bits));
                batch_bits_count++;
            }

            if(batch_bits_count == number_of_bits_per_batch)
            {
                UTIL_LOG_MSG("%s\n",
                             batch_bits);
                batch_bits_count = 0;
                memset(batch_bits,
                       0,
                       sizeof(batch_bits));
            }

            bit_field_to_be_logged >>= 1;
            iter_bit_field++;
        }

        if(batch_bits_count)
        {
            UTIL_LOG_MSG("%s\n",
                         batch_bits);
            batch_bits_count = 0;
            memset(batch_bits,
                   0,
                   sizeof(batch_bits));
        }
    }
}

