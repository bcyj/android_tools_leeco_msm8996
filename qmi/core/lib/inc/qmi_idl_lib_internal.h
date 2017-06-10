/*---------------------------------------------------------------------------
   QMI_IDL_LIB_INTERNAL.H
---------------------------------------------------------------------------*/
/*!
  @file
    qmi_idl_lib_internal.h

  @brief
    This file contains the private definitions for the QMI IDL message library.
*/

/*--------------------------------------------------------------------------
  Copyright (c) 2011-2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
---------------------------------------------------------------------------*/

#ifndef QMI_IDL_LIB_INTERNAL_H
#define QMI_IDL_LIB_INTERNAL_H

#include "stdint.h"
#include "stddef.h"
#include "qmi_idl_lib.h"  /* for IPC_NUM_MSG_TYPES */
#include <setjmp.h>


/* Define Library Version Number */
#define QMI_IDL_LIB_ENCDEC_VERSION 5
// Deprecated ,remove in next release
#define TYPE16(table, type) QMI_IDL_TYPE16(table, type)
#define OFFSET_IS_16 (1 << 7)
#define IS_ARRAY (1 << 6)
#define SZ_IS_16 (1 << 5)
#define VARIABLE_LENGTH (1 << 4)
//#define FUTURE_USE (1 << 3)

#define OFFSET8(t,f)          ((uint8_t) offsetof(t,f))
#define OFFSET16RELATIVE(t,f) offsetof(t,f)
#define OFFSET16ARRAY(t,f)    ((uint8_t) offsetof(t,f)), \
                              ((uint8_t) (offsetof(t,f) >> 8))

#define FLAG_END_VALUE 0x20

#define LAST_TLV (1 << 7)
#define OPTIONAL (1 << 6)
//#define FUTURE_USE (3 << 4)

#define GENERIC_1_BYTE     0
#define GENERIC_2_BYTE     1
#define GENERIC_4_BYTE     2
#define GENERIC_8_BYTE     3
#define _1_BYTE_ENUM       4
#define _2_BYTE_ENUM       5
#define STRING             6
#define AGGREGATE          7

/*---------New Definitions for namespace safety-----------*/

/* Generic macros for bit field extraction */
#define QMI_IDL_GET_BITS( VALUE, OFFSET, LENGTH ) \
        (((VALUE) >> OFFSET) & ((0x1 << LENGTH) - 1))

/* Definitions for bit fields in 16-bit type field */
#define QMI_IDL_TYPE_TABLE(type) QMI_IDL_GET_BITS( (type), 8, 4 )
#define QMI_IDL_TYPE_TYPE(type)  QMI_IDL_GET_BITS( (type), 0, 8 )\
                              | (QMI_IDL_GET_BITS( (type), 12, 4 ) << 8)
#define QMI_IDL_MSG_TYPE_TABLE(type) QMI_IDL_GET_BITS( (type), 12, 4 )
#define QMI_IDL_MSG_TYPE_TYPE(type) QMI_IDL_GET_BITS( (type), 0, 12 )

/* Build a 16-bit type field from a type index and a table index */
#define QMI_IDL_TYPE16(table, type) (((table) << 12) | (type))
#define QMI_IDL_TYPE88(table, type) (type & 0xFF), (((type & 0xFF00) >> 4) | table)

/* Definitions for bit fields in 8-bit flags field */
/* Library code dependes on flags type being LSBs - used without shifting */
#define QMI_IDL_FLAGS_OFFSET_IS_16    0x80
#define QMI_IDL_FLAGS_IS_ARRAY        0x40
#define QMI_IDL_FLAGS_SZ_IS_16        0x20
#define QMI_IDL_FLAGS_IS_VARIABLE_LEN 0x10
#define QMI_IDL_FLAGS_FIRST_EXTENDED  0x08
#define QMI_IDL_FLAGS_TYPE            0x07

#define QMI_IDL_FLAGS_SECOND_EXTENDED  0x80
#define QMI_IDL_FLAGS_SZ_IS_32         0x40
#define QMI_IDL_FLAGS_ARRAY_DATA_ONLY     0x20
#define QMI_IDL_FLAGS_ARRAY_LENGTH_ONLY   0x10
#define QMI_IDL_FLAGS_ARRAY_IS_LENGTHLESS 0x08
#define QMI_IDL_FLAGS_ENUM_IS_UNSIGNED    0x04
#define QMI_IDL_FLAGS_EXTENDED_OFFSET     0x02
#define QMI_IDL_FLAGS_UTF16_STRING        0x01

#define QMI_IDL_FLAGS_THIRD_EXTENDED      0x80
#define QMI_IDL_FLAGS_RANGE_CHECKED       0x40


#define QMI_IDL_OFFSET8(t,f)          ((uint8_t) offsetof(t,f))
#define QMI_IDL_OFFSET16RELATIVE(t,f) offsetof(t,f)
#define QMI_IDL_OFFSET16ARRAY(t,f)    ((uint8_t) offsetof(t,f)), \
                                      ((uint8_t) (offsetof(t,f) >> 8))
#define QMI_IDL_OFFSET32ARRAY(t,f)    ((uint8_t) offsetof(t,f)), \
                                      ((uint8_t) (offsetof(t,f) >> 8)), \
                                      ((uint8_t) (offsetof(t,f) >> 16))

#define QMI_IDL_FLAG_END_VALUE 0x20

#define QMI_IDL_TLV_FLAGS_LAST_TLV		0x80
#define QMI_IDL_TLV_FLAGS_OPTIONAL		0x40


// Definitions for bit fields in 8-bit TLV flags field that precedes each TLV's element data in the
// encoded message data.
#define QMI_IDL_TLV_FLAGS_TLV_TYPE_FOR_REQUIRED_TLV	0x0F
#define QMI_IDL_TLV_FLAGS_OFFSET_FOR_OPTIONAL_TLV	0x0F

#define QMI_IDL_GENERIC_1_BYTE     0
#define QMI_IDL_GENERIC_2_BYTE     1
#define QMI_IDL_GENERIC_4_BYTE     2
#define QMI_IDL_GENERIC_8_BYTE     3
#define QMI_IDL_1_BYTE_ENUM        4
#define QMI_IDL_2_BYTE_ENUM        5
#define QMI_IDL_STRING             6
#define QMI_IDL_AGGREGATE          7

#define QMI_IDL_RANGE_INT     0
#define QMI_IDL_RANGE_UINT    1
#define QMI_IDL_RANGE_ENUM    2
#define QMI_IDL_RANGE_MASK    3
#define QMI_IDL_RANGE_FLOAT   4
#define QMI_IDL_RANGE_DOUBLE  5

#define QMI_IDL_RANGE_RESPONSE_IGNORE  -2
#define QMI_IDL_RANGE_RESPONSE_DEFAULT -1
#define QMI_IDL_RANGE_RESPONSE_SUCCESS 0

typedef struct {
  uint32_t c_struct_sz;
  const uint8_t *p_encoded_type_data;
} qmi_idl_type_table_entry;

typedef struct {
  uint32_t c_struct_sz;
  const uint8_t *p_encoded_tlv_data;
} qmi_idl_message_table_entry;

typedef struct
{
  int8_t (*range_check)(void *val);
}qmi_idl_range_table_entry;

typedef struct {
  uint16_t qmi_message_id;
  uint16_t message_table_message_id;
  uint16_t max_msg_len;
} qmi_idl_service_message_table_entry;

struct qmi_idl_type_table_object {
  uint16_t n_types;
  uint16_t n_messages;
  uint8_t n_referenced_tables;
  const qmi_idl_type_table_entry *p_types;
  const qmi_idl_message_table_entry *p_messages;
  const struct qmi_idl_type_table_object **p_referenced_tables;
  const qmi_idl_range_table_entry *p_ranges;
};

struct qmi_idl_service_object {
  uint32_t library_version;
  uint32_t idl_version;
  uint32_t service_id;
  uint32_t max_msg_len;
  uint16_t n_msgs[QMI_IDL_NUM_MSG_TYPES];
  const qmi_idl_service_message_table_entry *msgid_to_msg[QMI_IDL_NUM_MSG_TYPES];
  const qmi_idl_type_table_object *p_type_table;
  uint32_t idl_minor_version;
  struct qmi_idl_service_object *parent_service_obj;
};
#endif  /* QMI_IDL_LIB_INTERNAL_H */
