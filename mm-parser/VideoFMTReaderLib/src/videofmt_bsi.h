#ifndef VIDEOFMT_BSI_H
#define VIDEOFMT_BSI_H
/*===========================================================================

             V I D E O   F O R M A T S   -   B I T S T R E A M
                  I N T E R N A L   H E A D E R   F I L E

DESCRIPTION
  This header file contains all the internal definitions to the video format
  bitstream services.

  Copyright (c) 2008-2013 QUALCOMM Technologies Inc, All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to this file.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/VideoFMTReaderLib/main/latest/src/videofmt_bsi.h#6 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/05/03   enj     Removing FEATURE_VIDEOFMT (i.e. permanently enabling it)
06/23/03   rpw     Replaced FEATURE_MP4_DECODER with FEATURE_VIDEOFMT.
03/17/03   rpw     Fixed potential unaligned memory accesses caused by using
                   a buffer of characters as a heap for structures.
11/18/02   rpw     Created file.

===========================================================================*/

/* <EJECT> */
/*===========================================================================

                      INCLUDE FILES FOR MODULE

===========================================================================*/
#include "AEEStdDef.h"              /* Definitions for byte, word, etc.        */

/* <EJECT> */
/*===========================================================================

                        DATA DECLARATIONS

===========================================================================*/

/* This is the maximum depth for the video bitstream parse position stack. */
#define VIDEO_FMT_BS_POS_STACK_DEPTH    64

/* This is the maximum size of each input ping-pong buffer. */
#define VIDEO_FMT_BS_BUFFER_SIZE        256

/* This is the number of entries to reserve for the variable heap. */
#define VIDEO_FMT_BS_HEAP_SIZE          4096

/* These are the different states of the video bitstream services state
** machine.
*/
typedef enum {
  VIDEO_FMT_BS_STATE_INIT,
  VIDEO_FMT_BS_STATE_GET_DATA,
  VIDEO_FMT_BS_STATE_UPDATE_BUFFER,
  VIDEO_FMT_BS_STATE_STATEMENT,
  VIDEO_FMT_BS_STATE_FIXED_CONST,
  VIDEO_FMT_BS_STATE_RANGED_CONST,
  VIDEO_FMT_BS_STATE_FIXED_VAR,
  VIDEO_FMT_BS_STATE_SIMPLE_VAR,
  VIDEO_FMT_BS_STATE_VAR_LENGTH_VAR,
  VIDEO_FMT_BS_STATE_FUNCTION_DECL,
  VIDEO_FMT_BS_STATE_FUNCTION_CALL,
  VIDEO_FMT_BS_STATE_CONDITIONAL,
  VIDEO_FMT_BS_STATE_WHILE,
  VIDEO_FMT_BS_STATE_DO,
  VIDEO_FMT_BS_STATE_TABLE,
  VIDEO_FMT_BS_STATE_PARSE_NBITS,
  VIDEO_FMT_BS_STATE_PARSE_VALUE,
  VIDEO_FMT_BS_STATE_PARSE_HEX,
  VIDEO_FMT_BS_STATE_PARSE_VAR_REF,
  VIDEO_FMT_BS_STATE_PARSE_EXPR,
  VIDEO_FMT_BS_STATE_NEXT_FIXED_CONST,
  VIDEO_FMT_BS_STATE_NEXT_FIXED_CONST_ALIGNED,
  VIDEO_FMT_BS_STATE_NEXT_RANGED_CONST,
  VIDEO_FMT_BS_STATE_INVALID
} video_fmt_bs_state_type;

/* This enumerated type lists the different expressions which can be used to
** combine values in the bitstream parser.
*/
typedef enum {
  VIDEO_FMT_BS_EXPR_LOGICAL_OR,
  VIDEO_FMT_BS_EXPR_LOGICAL_AND,
  VIDEO_FMT_BS_EXPR_LOGICAL_NOT,
  VIDEO_FMT_BS_EXPR_BITWISE_OR,
  VIDEO_FMT_BS_EXPR_BITWISE_AND,
  VIDEO_FMT_BS_EXPR_BITWISE_NOT,
  VIDEO_FMT_BS_EXPR_SHIFT_LEFT,
  VIDEO_FMT_BS_EXPR_SHIFT_RIGHT,
  VIDEO_FMT_BS_EXPR_ADD,
  VIDEO_FMT_BS_EXPR_SUBTRACT,
  VIDEO_FMT_BS_EXPR_MULTIPLY,
  VIDEO_FMT_BS_EXPR_DIVIDE,
  VIDEO_FMT_BS_EXPR_EQUAL,
  VIDEO_FMT_BS_EXPR_LESS,
  VIDEO_FMT_BS_EXPR_GREATER,
  VIDEO_FMT_BS_EXPR_INVALID
} video_fmt_bs_expr_type;

/* This defines the different kinds of name heap entries. */
typedef enum {
  VIDEO_FMT_BS_HEAP_VARIABLE,
  VIDEO_FMT_BS_HEAP_FUNCTION,
  VIDEO_FMT_BS_HEAP_TABLE,
  VIDEO_FMT_BS_HEAP_INVALID
} video_fmt_bs_heap_entry_type;

/* This structure represents the header of a heap entry. */
typedef struct {
  uint16                        size;        /* size of entry in bytes     */
  video_fmt_bs_heap_entry_type  type;        /* type of entry              */
  uint8                         name_length; /* size of name in bytes      */
  char                          name [1];    /* name starts here           */
} video_fmt_bs_heap_header_type;

/* This structure represents the part of a heap variable entry that comes
** after the entry name.
*/
typedef struct {
  uint32 value;      /* variable value               */
} video_fmt_bs_heap_var_body_type;

/* This structure represents the part of a heap function entry that comes
** after the entry name.
*/
typedef struct {
  uint32 start;      /* offset of function start in syntax */
  uint32 end;        /* offset of function end in syntax   */
} video_fmt_bs_heap_func_body_type;

/* This structure represents the beginning part of a heap table entry that
** comes after the entry name.
*/
typedef struct {
  int rows;       /* number of rows in the table       */
  int columns;    /* number of columns in the table    */
  uint32 widest;     /* largest value in bit width column */
  uint32 data [1];   /* placeholder for table data - actual data starts   */
                     /*   here but extends to rows * columns words        */
} video_fmt_bs_heap_table_body_type;

/* This structure is used to keep track of a current position for the video
** bitstream parser within a certain parsing scope.
*/
typedef struct {
  video_fmt_bs_state_type  state;    /* current parser state at this scope */
  uint32                   substate; /* current position in state sequence */

  uint32  syntax_beg;   /* position of current statement beginning         */
  int  syntax_pos;   /* current position in syntax string               */
  int  syntax_end;   /* end marker for current scope in syntax          */

  /* These values cache pointers to the header and body of the last heap
  ** variable entry accessed.
  */
  video_fmt_bs_heap_header_type  *last_var_header;
  void                           *last_var_body;

  /* These values cache pointers to the header and body of the last heap
  ** table entry accessed.
  */
  video_fmt_bs_heap_header_type      *last_table_header;
  video_fmt_bs_heap_table_body_type  *last_table_body;
  uint32                             last_table_entry;

  /* These fields store data parsed from the syntax. */
  uint32                  nbits;
  uint32                  value [2];
  uint32                  which_value;
  video_fmt_bs_expr_type  expr;
} video_fmt_bs_pos_type;

/* This structure contains all the state information for the video bitstream
** services functions.
*/
typedef struct {
  /* This is the status callback function pointer registered by
  ** the client.
  */
  video_fmt_bs_status_cb_func_type callback_ptr;

  /* This is the client data passed to the bitstream services, used to tell
  ** whether a request is currently active or not.
  */
  void *client_data;

  /* This is a stack of parser locations, used to track where the bitstream
  ** services is in decoding a bitstream.
  */
  video_fmt_bs_pos_type  bs_stack [VIDEO_FMT_BS_POS_STACK_DEPTH];
  uint32                 bs_stack_top;

  /* These are the input ping-pong buffers used to receive data from the
  ** client.  The size and position are in bits, but must always sum up to a
  ** multiple of 8 since data is read in multiples of 8 bits at a time.
  */
  uint8  in_buffers [2] [VIDEO_FMT_BS_BUFFER_SIZE];
  uint32 in_buffer_which;  /* which buffer currently in use    */
  uint32 in_buffer_size;   /* number of bytes in buffer        */
  uint32 in_buffer_pos;    /* offset of first byte in buffer   */

  /* These variables are used by the VIDEO_FMT_BS_STATE_GET_DATA state to
  ** track the progress of getting data from the application layer.
  */
  uint8  *get_data_dst;
  uint32 get_data_src;
  uint32 get_data_size;
  uint32 get_data_needed;
  uint32 get_data_read;

  /* This flag indicates whether or not and end of file can be expected when
  ** getting input data from the client.
  */
  boolean expect_eof;

  /* This union is used to pass information back to the client through the
  ** status callback function.
  */
  video_fmt_bs_status_cb_info_type  cb_info;

  /* This is the pointer supplied by the client, which defines the bitstream
  ** format string.
  */
  const char  *syntax;

  /* This keeps track of the current absolute bit position within the
  ** bitstream.
  */
  uint32 abs_pos;

  /* This is the variable heap, or the buffer used to store variable names and
  ** values.  The heap is declared as an array of 32-bit integers to make sure
  ** the data is aligned on a good memory boundary.
  */
  uint32  var_heap [VIDEO_FMT_BS_HEAP_SIZE];
  int  var_heap_end;
} video_fmt_bs_context_type;

/* <EJECT> */
/*---------------------------------------------------------------------------
** VIDEOFMT_BS Private Function Prototypes
**---------------------------------------------------------------------------
*/

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_bs_end

DESCRIPTION
  This function terminates the current video bitstream parsing session.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
extern void video_fmt_bs_end (void *server_data);

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_bs_process

DESCRIPTION
  This function runs the video bitstream services state machine.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
extern void video_fmt_bs_process (void *server_data);

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_bs_failure

DESCRIPTION
  This function is called whenever there is a critical failure in the video
  bitstream services, in order to free any resources that the services had
  allocated.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
extern void video_fmt_bs_failure (
  video_fmt_bs_context_type  *context
);

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_bs_read_buffer

DESCRIPTION
  This function moves the video bitstream services state machine as necessary
  to get the requested number of bits into an input buffer.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the requested number of bits is available in the active input
  buffer.

  If FALSE, not all the bits are available, and the state machine needs to be
  run to retrieve the needed bits.

SIDE EFFECTS
  None

===========================================================================*/
extern boolean video_fmt_bs_read_buffer (
  video_fmt_bs_context_type  *context,
  uint32                     num_bits,
  boolean                    expect_eof
);

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_bs_data_out

DESCRIPTION
  This function removes the given number of bits from the video bitstream
  services input buffers, storing the data in the given destination buffer,
  and updating the state of the input buffers.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
extern uint32 video_fmt_bs_data_out (
  video_fmt_bs_context_type  *context,
  uint32                     amount,
  uint32                     skipBits,
  boolean                    consume
);

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_bs_skip_data

DESCRIPTION
  This function advances the given number of bits in the bitstream being
  parsed.  Data skipped that is already in the input buffers is removed.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
extern void video_fmt_bs_skip_data (
  video_fmt_bs_context_type  *context,
  uint32                     amount
);

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_bs_parse_nbits

DESCRIPTION
  This function advances the video bitstream services state machine to begin
  parsing the format syntax for a "# bits" field.

DEPENDENCIES
  None

RETURN VALUE
  The value TRUE is returned if the state machine was updated properly.
  Otherwise, the value FALSE is returned upon failure.

SIDE EFFECTS
  None

===========================================================================*/
extern boolean video_fmt_bs_parse_nbits (
  video_fmt_bs_context_type  *context
);

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_bs_parse_value

DESCRIPTION
  This function advances the video bitstream services state machine to begin
  parsing the format syntax for a "value" field.

DEPENDENCIES
  None

RETURN VALUE
  The value TRUE is returned if the state machine was updated properly.
  Otherwise, the value FALSE is returned upon failure.

SIDE EFFECTS
  None

===========================================================================*/
extern boolean video_fmt_bs_parse_value (
  video_fmt_bs_context_type  *context,
  uint32                     which
);

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_bs_locate_name

DESCRIPTION
  This function searches for a variable or function that matches the name
  given at the current position in the bitstream syntax.  If found, the heap
  offset of the entry's header and body are returned.

DEPENDENCIES
  None

RETURN VALUE
  The value TRUE is returned if the variable or function was found, and the
  given heap offsets for the entry header and body are updated.
  Otherwise, the value FALSE is returned, indicating the entry was not found.

SIDE EFFECTS
  None

===========================================================================*/
extern boolean video_fmt_bs_locate_name (
  video_fmt_bs_context_type      *context,
  video_fmt_bs_heap_header_type  **ref_header,
  uint8                          *ref_name_length,
  void                           **ref_body
);

#endif /* VIDEOFMT_BSI_H */
