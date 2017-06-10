#ifndef VIDEOFMT_BS_H
#define VIDEOFMT_BS_H
/*===========================================================================

             V I D E O   F O R M A T S   -   B I T S T R E A M
                          H E A D E R   F I L E

DESCRIPTION
  This header file contains all the definitions necessary to interface with
  the video format bitstream services.

  Copyright (c) 2008-2013 QUALCOMM Technologies Inc, All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to this file.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/VideoFMTReaderLib/main/latest/inc/videofmt_bs.h#5 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/05/03   enj     Removing FEATURE_VIDEOFMT (i.e. permanently enabling it)
06/23/03   rpw     Replaced FEATURE_MP4_DECODER with FEATURE_VIDEOFMT.
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

/* This enumerated type lists all the possible status conditions used by the
** video bitstream services.  For each status, the comments below indicate
** which field of the "info" union parameters to the
** "video_fmt_bs_status_cb_func_type" callback contains additional information
** ("NULL" indicates there is no additional information).
*/
typedef enum {
  VIDEO_FMT_BS_ALLOC,          /* alloc     */
  VIDEO_FMT_BS_FREE,           /* free      */
  VIDEO_FMT_BS_GET_DATA,       /* get_data  */
  VIDEO_FMT_BS_VAR_INFO,       /* var_info  */
  VIDEO_FMT_BS_CONST_INFO,     /* var_info  */
  VIDEO_FMT_BS_FUNC_CALL,      /* func_call */
  VIDEO_FMT_BS_FUNC_DONE,      /* func_done */
  VIDEO_FMT_BS_CONTINUE,       /* cont      */
  VIDEO_FMT_BS_DONE,           /* NULL      */
  VIDEO_FMT_BS_ABORT,          /* NULL      */
  VIDEO_FMT_BS_FAILURE,        /* NULL      */
  VIDEO_FMT_BS_STATUS_INVALID  /* NULL      */
} video_fmt_bs_status_type;

/* This callback function type is used for continuing a current video
** bitstream services peration in progress.  It is given to the client in the
** server data of many of the status callbacks.  The "server_data" here must
** be the same as the "server_data" given to the client during the last status
** callback.
*/
typedef void (*video_fmt_bs_cont_cb_func_type) (void *server_data);

/* This callback function type is used as a means for the client to tell the
** bitstream services to terminate the current session.  The session is not
** actually terminated until the next VIDEO_FMT_BS_DONE status callback.
*/
typedef void (*video_fmt_bs_end_cb_func_type) (void *server_data);

/* These structures each correspond to one of the structures in the union
** pointed to by "info" in the "video_fmt_bs_status_cb_func_type" callback,
** for each of the possible values for the "status" parameter.
*/

typedef struct {
  uint32                           size;    /* number of bytes to allocate */
  void                             *ptr;    /* replace with ptr to memory  */
} video_fmt_bs_alloc_type;

typedef struct {
  void                             *ptr;    /* pointer to memory to free   */
} video_fmt_bs_free_type;

typedef struct {
  uint8                            *buffer;      /* where to store data    */
  uint32                           offset;       /* byte offset in stream  */
  uint32                           num_bytes;    /* client should replace  */
                                                 /*   with actual number   */
                                                 /*   of bytes read        */
  video_fmt_bs_cont_cb_func_type   callback_ptr;
  void                             *server_data;
} video_fmt_bs_get_data_type;

typedef struct {
  const char                       *name;        /* name of variable       */
  uint32                           value;        /* value of variable      */
  uint32                           offset;       /* bit offset in stream   */
  uint32                           size;         /* size in bits           */
  video_fmt_bs_cont_cb_func_type   callback_ptr;
  void                             *server_data;
} video_fmt_bs_var_info_type;

typedef struct {
  const char                       *name;        /* name of function       */
  uint32                           offset;       /* bit offset in stream   */
  video_fmt_bs_cont_cb_func_type   callback_ptr;
  void                             *server_data;
} video_fmt_bs_func_call_type;

typedef struct {
  uint32                           offset;       /* bit offset in stream   */
  video_fmt_bs_cont_cb_func_type   callback_ptr;
  void                             *server_data;
} video_fmt_bs_func_done_type;

typedef struct {
  video_fmt_bs_cont_cb_func_type   callback_ptr;
  void                             *server_data;
} video_fmt_bs_continue_type;

/* This union is used to represent one of the many status callback information
** structures.  The status code set when each field is used is indicated.
*/
typedef union {
  video_fmt_bs_alloc_type     alloc;     /* VIDEO_FMT_BS_ALLOC     */
  video_fmt_bs_free_type      free;      /* VIDEO_FMT_BS_FREE      */
  video_fmt_bs_get_data_type  get_data;  /* VIDEO_FMT_BS_GET_DATA  */
  video_fmt_bs_var_info_type  var_info;  /* VIDEO_FMT_BS_VAR_INFO or
                                         ** VIDEO_FMT_BS_CONST_INFO
                                         */
  video_fmt_bs_func_call_type func_call; /* VIDEO_FMT_BS_FUNC_CALL */
  video_fmt_bs_func_done_type func_done; /* VIDEO_FMT_BS_FUNC_DONE */
  video_fmt_bs_continue_type  cont;      /* VIDEO_FMT_BS_CONTINUE  */
} video_fmt_bs_status_cb_info_type;

/* This callback function type is used for returning status to the client,
** and for requesting or returning more data.  The client registers this
** callback function when it initiates bitstream decoding.
*/
typedef void (*video_fmt_bs_status_cb_func_type) (
  video_fmt_bs_status_type          status,
  void                              *client_data,
  void                              *info,
  video_fmt_bs_end_cb_func_type     end
);

/* <EJECT> */
/*---------------------------------------------------------------------------
** VIDEOFMT_BITSTREAM Public Function Prototypes
**---------------------------------------------------------------------------
*/

/*===========================================================================

FUNCTION  video_fmt_bs_decode

DESCRIPTION
  This function initiates the decoding of a bitstream.  The bitstream format
  is described using a micro-language, defined here.

  In the micro-language, the format of the bitstream is defined by a sequence
  of parsing "statements" used to describe the format.  Each parsing statement
  consists of the statement type, specified as a single character, followed by
  the arguments to the statement, as defined by the statement type.

  No whitespace characters may be used in a bitstream format declaration.
  Each statement immediately follows the one before it, and after the last
  statement is read the bitstream is considered fully parsed.

  Some statements take an argument of the form "# bits".  There are two ways
  to describe this type of argument:

  1) A variable-sized string of decimal digits, together defining a decimal
  number.  The string of decimal digits ends with the last decimal digit
  character, or the end of the format string.

  2) A name delimited by single-quote characters.  This defines the "# bits"
  to be the value currently assigned to the variable with the given name.

  Some statements take an argument of the form "value".  There are nine ways
  to describe this type of argument:

  1) A string of hexidecimal digits, delimited by pound sign characters.  The
  digits together define the value.

  2) A name delimited by single-quote characters.  This defines the value to
  be the current value of the variable with the given name.

  3) One or more other "value" arguments preceeded by a logical operator ('|'
  for "OR", '&' for "AND", or '!' for "NOT").  This defines the value to be
  either TRUE (non-zero) or FALSE (zero) depending on the result of the
  given logical operation on the given argument(s) which follow.

  4) One or more other "value" arguments preceeded by a bit-wise operator
  (capital 'O' for "OR", capital 'A' for "AND", '~' for "NOT", '<' for
  left-shift, or '>' for right-shift).  This defines the value to be the
  result of the given bit-wise operation on the given argument(s) which
  follow.

  5) One or more other "value" arguments preceeded by an arithmetic operator
  ('+' for addition, '-' for subtraction, '*' for multiplication, or '/' for
  division).  This defines the value to be the result of the given bit-wise
  operation on the given argument(s) which follow.

  6) Two "value" arguments preceeded by a comparison operator ('=' for equal,
  'L' for less than, or 'G' for greater than).  This defines the value to be
  either TRUE (non-zero) or FALSE (zero) depending on how the given arguments
  compare.

  7) The character 'n' followed by a "# bits" argument, followed by another
  "value" argument.  This defines the value to be TRUE (non-zero) if the next
  number of bits in the bitstream matches the given value, or FALSE (zero)
  otherwise.

  8) The character 'N' followed by a "# bits" argument, followed by another
  "value" argument.  This defines the value to be TRUE (non-zero) if the next
  number of bits in the bitstream, starting with the first bit in the next
  byte, matches the given value, or FALSE (zero) otherwise.

  9) The character 'r' followed by a "# bits" argument, followed by two more
  "value" arguments.  This defines the value to be TRUE (non-zero) if the next
  number of bits in the bitstream matches any value between or including the
  two given values, or FALSE (zero) otherwise.

  10) The character 'o'.  This defines the value to be the current absolute
  bit position in the bitstream.

  Some statements take an argument of the form "name".  This is a sequence of
  characters which together defines a name, to be used to name a variable or
  function.  A name may not include the single-quote, parenthesis, or
  curly-bracket characters.

  In the definition of any statement, a character enclosed in single-quote
  characters defines a literal character expected in the statement.  Also, the
  use of logical operators implies two or more alternatives.

  Some statements take an argument of the form "block".  This is actually a
  sub-sequence of statements controlled by the statement.  They specify
  statements to be parsed only if a conditional is met, or statements to be
  parsed later when a function call statement is encountered.

  Variables can be defined and used in this micro-language.  Whenever a
  variable is changed, a call-back to the client is given to indicate the
  variable's parsed value.

  The following statements are defined:

  1) Fixed Constant:  'c' <# bits> <name> <value> - expect the given value in
  the bitstream, otherwise generate a parsing exception.  The name is provided
  only for passing back to the client as part of the VIDEO_FMT_BS_CONST_INFO
  status, and may be empty ('') if not needed.

  2) Range Constant:  'r' <# bits> <name> <value> <value> - expect a value in
  the bitstream which is equal to one of the two given values or between them,
  otherwise generate a parsing exception.  The name is provided only for
  passing back to the client as part of the VIDEO_FMT_BS_CONST_INFO status,
  and may be empty ('') if not needed.

  3) Fixed variable:  '=' <name> <value> - defines a variable with the
  given name to have the given value.  The bitstream is not parsed in this
  statement.

  4) Simple variable:  'v' <# bits> <name> - store the next number of bits in
  the variable with the given name (replace the variable if it already exists)

  5) Variable-length variable:  'V' <table name> <name>.. - signifies the next
  bits in the bitstream should be decoded as a variable-length variable, using
  the given lookup table.  The table name is followed by names of variables in
  which to store the columns of the matching table entry, starting with the
  second column (since the first is a place-holder for the first variable's
  length in bits.)

  6) Function declaration:  'f' <name> '{' <block> '}' - defines a function of
  statements which can be parsed later by referencing the function by name.

  7) Function call:  's' <name> - pushes the current parsing context onto the
  parsing stack, and begins parsing the block of statements in the given
  function's declaraction.  After the last statement in that function is
  parsed, the parsing stack is popped, restoring the parsing context, so that
  the next statement parsed after the end of the function will be the next
  statement after the function call.

  8) Conditinal:  '?' <value> '{' <block> '}' <'{' <block> '}'> -
  test the given value against TRUE (non-zero) and FALSE (zero), and parse the
  statements in the first block if the value matched TRUE.  If there is a
  second block and the given value matched FALSE, the statements in the second
  block are parsed instead (as an "else" type condition).

  9) While loop:  'w' <value> '{' <block> '}' - test the given value against
  TRUE (non-zero) and FALSE (zero), and parse the statements in the block if
  the value matched TRUE.  After the last statement in the block is parsed,
  repeat the while loop statement from the beginning.  If the given value
  matched FALSE, skip past the statements in the block and do not parse the
  statements in the while loop.

  10) Do loop:  'd' '{' <block> '}' <value> - parse the statemtnts in the
  block, and then test the given value against TRUE (non-zero) and FALSE
  (zero).  If the value matches TRUE, repeat the do loop statement from the
  beginning.

  11) Exception:  '!' - immediately issue a parsing exception.  This is useful
  for complex bitstream formats where the client is not yet capable of parsing
  some parts.

  12) Return statement:  'R' - immediately return from the current function.
  The parsing stack is popped, restoring the parsing context, so that the next
  statement parsed will be the next statement after the last function call.

  13) Table declaration:  't' <# bits> <name> '{' <value>.. '}' - defines a
  lookup table with the "# bits" field interpreted as the number of columns to
  build in the table, and the name of the table following the "# bits" field.
  The rows of the table are filled in using the values enclosed in
  curly-bracket characters.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
extern void video_fmt_bs_decode (
  video_fmt_bs_status_cb_func_type  callback_ptr,
  const char                        *syntax,
  uint32                            syntax_length,
  void                              *client_data
);

#endif /* VIDEOFMT_BS_H */

