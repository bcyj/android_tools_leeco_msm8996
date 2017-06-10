/* linesize(132)
** pagesize(60)
** title("Dual Mode Subscriber Station")
** subtitle("Video Formats Services")
*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

             V I D E O   F O R M A T S   -   B I T S T R E A M

GENERAL DESCRIPTION
  This module contains functions which decode a stream of bits representing a
  sequence of (possibly nested) data structures.

EXTERNALIZED FUNCTIONS
  video_fmt_bs_decode
    This function initiates the decoding of a bitstream.

INITIALIZATION AND SEQUENCING REQUIREMENTS

  Copyright (c) 2008-2013 QUALCOMM Technologies Inc, All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* <EJECT> */
/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/VideoFMTReaderLib/main/latest/src/videofmt_bs.c#13 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/05/03   enj     Removing FEATURE_VIDEOFMT (i.e. permanently enabling it)
06/23/03   rpw     Replaced FEATURE_MP4_DECODER with FEATURE_VIDEOFMT.
03/17/03   rpw     Fixed bug introduced by changes made previously to fix
                   unaligned memory accesses - size of table entries in
                   heap were not computed properly.
03/17/03   rpw     Fixed bug where local variable "heap_ptr" was not
                   recalculated between substates.
03/17/03   rpw     Fixed potential unaligned memory accesses caused by using
                   a buffer of characters as a heap for structures.
11/18/02   rpw     Created file.

===========================================================================*/

/* <EJECT> */
/*===========================================================================

                        INCLUDE FILES FOR MODULE

===========================================================================*/
#include <string.h>        /* for memcpy() */

#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"              /* Common definitions                      */

#include "videofmt_common.h"
#include "videofmt_bs.h"        /* bitstream typedefs and prototypes       */
#include "videofmt_bsi.h"       /* Internal bitstream definitions          */

/* <EJECT> */
/*===========================================================================

                DECLARATIONS FOR MODULE

===========================================================================*/

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
void video_fmt_bs_decode (
  video_fmt_bs_status_cb_func_type  callback_ptr,
  const char                        *syntax,
  uint32                            syntax_length,
  void                              *client_data
) {
    video_fmt_bs_context_type   *context;
    video_fmt_bs_alloc_type     alloc;

    /* Call the callback to allocate space for the decoder context. */
    alloc.size = (uint32)sizeof (video_fmt_bs_context_type);
    callback_ptr (VIDEO_FMT_BS_ALLOC, client_data,
                  &alloc, NULL);
    if ( alloc.ptr )
    context = (video_fmt_bs_context_type *) alloc.ptr;
    else
    {
       callback_ptr (VIDEO_FMT_BS_FAILURE, client_data, NULL, NULL);
       return;
    }

    /* Initialize state machine and start processing. */
    context->bs_stack_top = 1;
    context->bs_stack [0].state = VIDEO_FMT_BS_STATE_INIT;
    context->bs_stack [0].syntax_end = syntax_length;
    context->callback_ptr = callback_ptr;
    context->client_data = client_data;
    context->syntax = syntax;
    video_fmt_bs_process (context);
}

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
void video_fmt_bs_end (void *server_data) {
    video_fmt_bs_context_type         *context;
    video_fmt_bs_status_cb_func_type  callback_ptr;
    void                              *client_data;
    video_fmt_bs_free_type            free;

    /* Deallocate memory used by the video bitstream services. */
    context = (video_fmt_bs_context_type *) server_data;
    callback_ptr = context->callback_ptr;
    client_data = context->client_data;
    free.ptr = context;
    callback_ptr (VIDEO_FMT_BS_FREE, client_data,
                  &free, NULL);

    /* Stop internal processing. */
    callback_ptr (VIDEO_FMT_BS_DONE, client_data, NULL, NULL);
}

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
void video_fmt_bs_process (void *server_data) {
    video_fmt_bs_pos_type              *bs_stack_top;
    boolean                            exit_loop = FALSE;
    video_fmt_bs_context_type          *context;
    uint32                             temp32;
    video_fmt_bs_heap_header_type      *ref_header;
    uint8                              ref_name_length;
    void                               *ref_body;
    video_fmt_bs_heap_var_body_type    *var_body;
    video_fmt_bs_heap_func_body_type   *func_body;
    video_fmt_bs_heap_table_body_type  *table_body;
    int                                i, j;
    uint8                              temp8;
    uint32                             *table_data;
    uint8                              *heap_ptr;
    int                                nIndex;

    /* Repeatedly process video bitstream services state machine until a flag
    ** is set to indicate we should return to the client.
    */
    context = (video_fmt_bs_context_type *) server_data;
    while (!exit_loop)
    {
        bs_stack_top = &context->bs_stack [context->bs_stack_top - 1];
        switch (bs_stack_top->state)
        {
        case VIDEO_FMT_BS_STATE_INIT:
            /* Set up initial input buffer state. */
            context->in_buffer_which = 1;  /* will cause immediate switch  */
                                           /* to buffer 0 on first read    */
            context->in_buffer_size = 0;
            context->in_buffer_pos = 0;
            context->abs_pos = 0;

            /* Set up initial atom stack state. */
            context->bs_stack [0].syntax_pos = 0;

            /* Initialize variable heap. */
            context->var_heap_end = 0;

            /* Start processing. */
            bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
            break;

        case VIDEO_FMT_BS_STATE_GET_DATA:
            /* No matter what, exit the state machine loop each time this
            ** state is visited.  This is necessary in order to limit the
            ** amount of processing done each state machine iteration.
            */
            exit_loop = TRUE;

            /* Request more data from the user. */
            context->cb_info.get_data.buffer = context->get_data_dst;
            context->cb_info.get_data.offset = context->get_data_src;
            context->cb_info.get_data.num_bytes = context->get_data_size;
            context->cb_info.get_data.callback_ptr = video_fmt_bs_process;
            context->cb_info.get_data.server_data = context;
            context->callback_ptr (VIDEO_FMT_BS_GET_DATA,
                                   context->client_data,
                                   &context->cb_info,
                                   video_fmt_bs_end);

            /* Verify the user gave us a legal number of bytes. */
            if (context->cb_info.get_data.num_bytes
                > context->get_data_size)
            {
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                          "video_fmt_bs_process: get_data overrun!" );
              video_fmt_bs_failure (context);
              return;
            }

            /* Advance the internal variables tracking the reading. */
            context->get_data_dst += context->cb_info.get_data.num_bytes;
            context->get_data_src += (uint32)context->cb_info.get_data.num_bytes;
            context->get_data_size -= (uint32)context->cb_info.get_data.num_bytes;
            context->get_data_read += (uint32)context->cb_info.get_data.num_bytes;
            context->get_data_needed -= (uint32)FILESOURCE_MIN (context->get_data_needed,
                        context->cb_info.get_data.num_bytes);

            /* Move to the next state if no bytes were given, or no more bytes
            ** are needed.
            */
            if (!context->cb_info.get_data.num_bytes
                || !context->get_data_needed)
            {
                --context->bs_stack_top;
            }
            break;

        case VIDEO_FMT_BS_STATE_UPDATE_BUFFER:
            /* Advance the input buffer size by the number of bytes
            ** actually read.
            */
            context->in_buffer_size += context->get_data_read * 8;

            /* If not enough bytes were read, perform end of
            ** file processing.
            */
            if (context->get_data_needed)
            {
                /* If we are not expecting an end of file here, report an
                ** unexpected end of file.
                */
              if (!context->expect_eof)
              {
                MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_MEDIUM,"video_fmt_bs_process:"
                            "unexpected end of data!");
                video_fmt_bs_failure (context);
                return;
              }

              /* Otherwise, inform the client that parsing is complete. */
              else
              {
                /* Exit processing loop now that we're done parsing. */
                exit_loop = TRUE;

                /* Clean up bitstream services. */
                video_fmt_bs_end (context);
              }
            }

            /* Otherwise, move to the next state. */
            else
            {
                --context->bs_stack_top;
            }
            break;

        case VIDEO_FMT_BS_STATE_STATEMENT:
            /* Finish parsing this scope if end of syntax is reached, or next
            ** character is return command.
            */
            if ((bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
                || (context->syntax [bs_stack_top->syntax_pos] == 'R'))
            {
                /* Remember current position in syntax. */
                temp32 = bs_stack_top->syntax_pos;

                /* If the next character was return command, keep popping off
                ** the syntax stack until the stack location one level up
                ** indicates a function call, or we reach the top of the
                ** stack.
                */
                if (context->syntax [bs_stack_top->syntax_pos] == 'R')
                {
                    while ((context->bs_stack_top > 1)
                           && (context->bs_stack
                               [context->bs_stack_top - 2].state
                               != VIDEO_FMT_BS_STATE_FUNCTION_CALL))
                    {
                        --context->bs_stack_top;
                    }
                }

                /* Pop off one stack location. */
                --context->bs_stack_top;

                /* If at the global scope, clean up bitstream services. */
                if (context->bs_stack_top == 0)
                {
                    exit_loop = TRUE;
                    video_fmt_bs_end (context);
                }

                /* Otherwise, set syntax position at new stack level to be
                ** where we were in the previous level.
                */
                else
                {
                    bs_stack_top = &context->bs_stack
                        [context->bs_stack_top - 1];
                    bs_stack_top->syntax_pos = temp32;
                }
                break;
            }

            /* Read statement type character and translate into next parser
            ** state.
            */
            switch (context->syntax [bs_stack_top->syntax_pos++])
            {
            case 'c': /* fixed constant */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_FIXED_CONST;
                break;

            case 'r': /* ranged constant */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_RANGED_CONST;
                break;

            case '=': /* fixed variable */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_FIXED_VAR;
                break;

            case 'v': /* simple variable */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_SIMPLE_VAR;
                break;

            case 'V': /* variable-length variable */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_VAR_LENGTH_VAR;
                break;

            case 'f': /* function declaration */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_FUNCTION_DECL;
                break;

            case 's': /* function call */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_FUNCTION_CALL;
                break;

            case '?': /* conditional */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_CONDITIONAL;
                break;

            case 'w': /* "while" loop */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_WHILE;
                break;

            case 'd': /* "do" loop */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_DO;
                break;

            case '!': /* unsupported option */
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                             "video_fmt_bs_process: '!' statement hit at pos=%lu!",
                              bs_stack_top->syntax_pos);
                video_fmt_bs_failure (context);
                return;

            case 't': /* table */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_TABLE;
                break;

            default:
                /* If we reach here, there is a typo in the syntax. */
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                             "video_fmt_bs_process: unknown statement "
                             "type at pos=%lu!",bs_stack_top->syntax_pos);
                video_fmt_bs_failure (context);
                return;
            }

            /* Start in first substate. */
            bs_stack_top->substate = 0;
            break;

        case VIDEO_FMT_BS_STATE_FIXED_CONST:
            /* Continue processing state. */
            switch (bs_stack_top->substate)
            {
            case 0: /* parse n-bits */
                if (!video_fmt_bs_parse_nbits (context))
                {
                    return;
                }
                break;

            case 1: /* parse constant name */
                /* Generate exception if end of syntax is reached. */
              if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
              {
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                             "video_fmt_bs_process: unexpected end of"
                             "syntax at pos=%lu!",
                bs_stack_top->syntax_pos);
                video_fmt_bs_failure (context);
                return;
              }

              /* Expect single-quote character. */
              if (context->syntax [bs_stack_top->syntax_pos++] != '\'')
              {
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                              "video_fmt_bs_process: error in bitstream - "
                              "expected single-quoted name at pos=%lu!",
                               bs_stack_top->syntax_pos - 1);
                video_fmt_bs_failure (context);
                return;
              }

                /* Copy name to heap temporarily. */
                heap_ptr = (uint8 *) &context->var_heap
                    [context->var_heap_end];
                for (i = 0; bs_stack_top->syntax_pos + i
                         < bs_stack_top->syntax_end; ++i)
                {
                  if (context->var_heap_end + i >= VIDEO_FMT_BS_HEAP_SIZE)
                  {
                    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                                 "video_fmt_bs_process: name heap "
                                 "overflow at pos=%lu",
                    bs_stack_top->syntax_pos);
                    video_fmt_bs_failure (context);
                    return;
                  }
                  heap_ptr [i] = context->syntax
                    [bs_stack_top->syntax_pos + i];
                  if (context->syntax
                    [bs_stack_top->syntax_pos + i] == '\'')
                  {
                    break;
                  }
                }

                /* Fail if the end of the syntax is reached or the name is too
                ** long.
                */
                if ((bs_stack_top->syntax_pos + i
                     >= bs_stack_top->syntax_end)
                    || (i > 200))
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                               "constant name too long at pos=%lu!",
                  bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Add NULL chacter to temporary name copy. */
                heap_ptr [i] = '\0';

                /* Advance past the variable name. */
                bs_stack_top->syntax_pos += i + 1;
                ++bs_stack_top->substate;

            case 2: /*lint!e616 lint fallthorugh*/ /* parse value */
                if (!video_fmt_bs_parse_value (context, 0))
                {
                    return;
                }
                break;

            case 3: /* expect constant value */
                /* Read the next given number of bits from the bitstream. */
                if (!video_fmt_bs_read_buffer
                    (context, bs_stack_top->nbits, FALSE))
                {
                    break;
                }
                temp32 = video_fmt_bs_data_out
                    (context, bs_stack_top->nbits, 0, TRUE);

                /* Compare with the expected constant value. */
                if (temp32 != bs_stack_top->value [0])
                {
                  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                               "expected %lu at offset %lu, but found %lu!",
                                bs_stack_top->value [0],
                                context->abs_pos - bs_stack_top->nbits,
                                temp32);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Issue callback to client regarding the constant. */
                heap_ptr = (uint8 *) &context->var_heap
                    [context->var_heap_end];
                context->cb_info.var_info.name
                    = (const char*) heap_ptr;
                context->cb_info.var_info.value = temp32;
                context->cb_info.var_info.offset
                    = context->abs_pos - bs_stack_top->nbits;
                context->cb_info.var_info.size = bs_stack_top->nbits;
                context->cb_info.var_info.callback_ptr
                    = video_fmt_bs_process;
                context->cb_info.var_info.server_data = context;
                context->callback_ptr (VIDEO_FMT_BS_CONST_INFO,
                                       context->client_data,
                                       &context->cb_info,
                                       video_fmt_bs_end);

                /* Proceed to the next field in the bitstream. */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
            }
            break;

        case VIDEO_FMT_BS_STATE_RANGED_CONST:
            /* Continue processing state. */
            switch (bs_stack_top->substate)
            {
            case 0: /* parse n-bits */
                if (!video_fmt_bs_parse_nbits (context))
                {
                    return;
                }
                break;

            case 1: /* parse constant name */
                /* Generate exception if end of syntax is reached. */
              if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
              {
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                             "video_fmt_bs_process: unexpected end of "
                             "syntax at pos=%lu!",
                             bs_stack_top->syntax_pos);
                video_fmt_bs_failure (context);
                return;
              }

              /* Expect single-quote character. */
              if (context->syntax [bs_stack_top->syntax_pos++] != '\'')
              {
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                             "video_fmt_bs_process: error in bitstream - "
                             "expected single-quoted name at pos=%lu!",
                             bs_stack_top->syntax_pos - 1);
                video_fmt_bs_failure (context);
                return;
              }

                /* Copy name to heap temporarily. */
                heap_ptr = (uint8 *) &context->var_heap
                    [context->var_heap_end];
                for (i = 0; bs_stack_top->syntax_pos + i
                         < bs_stack_top->syntax_end; ++i)
                {
                  if (context->var_heap_end + i >= VIDEO_FMT_BS_HEAP_SIZE)
                  {
                    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                                 "video_fmt_bs_process: name heap "
                                 "overflow at pos=%lu",
                                 bs_stack_top->syntax_pos);
                    video_fmt_bs_failure (context);
                    return;
                  }
                  heap_ptr [i] = context->syntax
                    [bs_stack_top->syntax_pos + i];
                  if (context->syntax
                    [bs_stack_top->syntax_pos + i] == '\'')
                  {
                    break;
                  }
                }

                /* Fail if the end of the syntax is reached or the name is too
                ** long.
                */
                if ((bs_stack_top->syntax_pos + i
                     >= bs_stack_top->syntax_end)
                    || (i > 200))
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                               "constant name too long at pos=%lu!",
                  bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Add NULL chacter to temporary name copy. */
                heap_ptr [i] = '\0';

                /* Advance past the variable name. */
                bs_stack_top->syntax_pos += i + 1;
                ++bs_stack_top->substate;

            case 2: /* parse value #1 */
                if (!video_fmt_bs_parse_value (context, 0))
                {
                    return;
                }
                break;

            case 3: /* parse value #2 */
                /* Parse second constant value. */
                if (!video_fmt_bs_parse_value (context, 1))
                {
                    return;
                }
                break;

            case 4: /* expect constant value */
                /* Read the next given number of bits from the bitstream. */
                if (!video_fmt_bs_read_buffer
                    (context, bs_stack_top->nbits, FALSE))
                {
                    break;
                }
                temp32 = video_fmt_bs_data_out
                    (context, bs_stack_top->nbits, 0, TRUE);

                /* Compare with the expected constant value range. */
                if ((temp32 < bs_stack_top->value [0])
                    || (temp32 > bs_stack_top->value [1]))
                {
                  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                               "expected %lu-%lu at offset %lu!",
                               bs_stack_top->value [0],
                               bs_stack_top->value [1],
                               context->abs_pos - bs_stack_top->nbits);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Issue callback to client regarding the constant. */
                heap_ptr = (uint8 *) &context->var_heap
                    [context->var_heap_end];
                context->cb_info.var_info.name
                    = (const char*) heap_ptr;
                context->cb_info.var_info.value = temp32;
                context->cb_info.var_info.offset
                    = context->abs_pos - bs_stack_top->nbits;
                context->cb_info.var_info.size = bs_stack_top->nbits;
                context->cb_info.var_info.callback_ptr
                    = video_fmt_bs_process;
                context->cb_info.var_info.server_data = context;
                context->callback_ptr (VIDEO_FMT_BS_CONST_INFO,
                                       context->client_data,
                                       &context->cb_info,
                                       video_fmt_bs_end);

                /* Proceed to the next field in the bitstream. */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
            }
            break;

        case VIDEO_FMT_BS_STATE_FIXED_VAR:
            /* Continue processing state. */
            switch (bs_stack_top->substate)
            {
            case 0: /* parse variable name */
                /* Generate exception if end of syntax is reached. */
              if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
              {
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                             "video_fmt_bs_process: unexpected end of "
                             "syntax at pos=%lu!",
                             bs_stack_top->syntax_pos);
                video_fmt_bs_failure (context);
                return;
              }

              /* Expect single-quote character. */
              if (context->syntax [bs_stack_top->syntax_pos++] != '\'')
              {
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                            "video_fmt_bs_process: error in bitstream - "
                            "expected single-quoted name at pos=%lu!",
                            bs_stack_top->syntax_pos - 1);
                video_fmt_bs_failure (context);
                return;
              }

                /* Search for heap entry with given name.  If an entry was
                ** found but it is not a variable, generate an exception.
                */
                if (video_fmt_bs_locate_name
                    (context, &ref_header, &ref_name_length, &ref_body))
                {
                  if (ref_header->type != VIDEO_FMT_BS_HEAP_VARIABLE)
                  {
                    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                                "video_fmt_bs_process: name referenced "
                                "at pos=%lu is not a variable!",
                                 bs_stack_top->syntax_pos);
                    video_fmt_bs_failure (context);
                    return;
                  }
                  var_body = (video_fmt_bs_heap_var_body_type *) ref_body;
                }

                /* Otherwise, add a variable to the heap. */
                else
                {
                    /* Check for heap overflow. */
                    temp8 = (uint8)((sizeof (*ref_header) + 3) / 4
                        + (sizeof (*var_body) + 3) / 4
                        + (ref_name_length + 3) / 4);
                    if (context->var_heap_end + temp8
                        > VIDEO_FMT_BS_HEAP_SIZE)
                    {
                        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                                   "video_fmt_bs_process: name heap "
                                   "overflow at pos=%lu",
                                   bs_stack_top->syntax_pos);
                        video_fmt_bs_failure (context);
                        return;
                    }

                    /* Add heap entry. */
                    ref_header = (video_fmt_bs_heap_header_type *)((void*)
                                 &context->var_heap [context->var_heap_end]);
                    ref_header->size = temp8;
                    ref_header->type = VIDEO_FMT_BS_HEAP_VARIABLE;
                    ref_header->name_length = ref_name_length;
                    memcpy (ref_header->name,
                            &context->syntax [bs_stack_top->syntax_pos],
                            ref_name_length);
                    ref_header->name [ref_name_length] = '\0';
                    nIndex = (int)(context->var_heap_end
                                     + (sizeof (*ref_header) + 3) / 4
                                     + (ref_name_length + 3) / 4);
                    var_body = (video_fmt_bs_heap_var_body_type *)
                               ((void*)&context->var_heap [nIndex]);
                    context->var_heap_end += ref_header->size;
                }

                /* Cache heap entry location. */
                bs_stack_top->last_var_header = ref_header;
                bs_stack_top->last_var_body = var_body;

                /* Advance past the variable name. */
                bs_stack_top->syntax_pos += ref_name_length + 1;
                ++bs_stack_top->substate;

            case 1: /* parse variable value */
                if (!video_fmt_bs_parse_value (context, 0))
                {
                    return;
                }
                break;

            case 2: /* store variable value */
                var_body = (video_fmt_bs_heap_var_body_type *)
                    bs_stack_top->last_var_body;
                var_body->value = bs_stack_top->value [0];

                /* Issue callback to client regarding the variable. */
                context->cb_info.var_info.name
                    = bs_stack_top->last_var_header->name;
                context->cb_info.var_info.value = bs_stack_top->value [0];
                context->cb_info.var_info.offset = (uint32) -1;
                context->cb_info.var_info.size = 0;
                context->cb_info.var_info.callback_ptr
                    = video_fmt_bs_process;
                context->cb_info.var_info.server_data = context;
                context->callback_ptr (VIDEO_FMT_BS_VAR_INFO,
                                       context->client_data,
                                       &context->cb_info,
                                       video_fmt_bs_end);

                /* Proceed to the next field in the bitstream. */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
                exit_loop = TRUE;
            }
            break;

        case VIDEO_FMT_BS_STATE_SIMPLE_VAR:
            /* Continue processing state. */
            switch (bs_stack_top->substate)
            {
            case 0: /* parse n-bits */
                if (!video_fmt_bs_parse_nbits (context))
                {
                    return;
                }
                break;

            case 1: /* parse variable name */
                /* Generate exception if end of syntax is reached. */
                if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: unexpected end of "
                                "syntax at pos=%lu!",
                                bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Expect single-quote character. */
                if (context->syntax [bs_stack_top->syntax_pos++] != '\'')
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                               "expected single-quoted name at pos=%lu!",
                                bs_stack_top->syntax_pos - 1);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Search for heap entry with given name.  If an entry was
                ** found but it is not a variable, generate an exception.
                */
                if (video_fmt_bs_locate_name
                    (context, &ref_header, &ref_name_length, &ref_body))
                {
                  if (ref_header->type != VIDEO_FMT_BS_HEAP_VARIABLE)
                  {
                    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                                 "video_fmt_bs_process: name referenced "
                                 "at pos=%lu is not a variable!",
                                 bs_stack_top->syntax_pos);
                    video_fmt_bs_failure (context);
                    return;
                  }
                    var_body = (video_fmt_bs_heap_var_body_type *) ref_body;
                }

                /* Otherwise, add a variable to the heap. */
                else
                {
                    /* Check for heap overflow. */
                    temp8 = (uint8)((sizeof (*ref_header) + 3) / 4
                                    + (sizeof (*var_body) + 3) / 4
                                    + (ref_name_length + 3) / 4);
                    if (context->var_heap_end + temp8 > VIDEO_FMT_BS_HEAP_SIZE)
                    {
                      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                                   "video_fmt_bs_process: name heap "
                                   "overflow at pos=%lu",
                                    bs_stack_top->syntax_pos);
                      video_fmt_bs_failure (context);
                      return;
                    }

                    /* Add heap entry. */
                    ref_header = (video_fmt_bs_heap_header_type *)((void*)
                                &context->var_heap [context->var_heap_end]);
                    ref_header->size = temp8;
                    ref_header->type = VIDEO_FMT_BS_HEAP_VARIABLE;
                    ref_header->name_length = ref_name_length;
                    memcpy (ref_header->name,
                            &context->syntax [bs_stack_top->syntax_pos],
                            ref_name_length);
                    ref_header->name [ref_name_length] = '\0';
                    nIndex = (int)(context->var_heap_end
                                     + (sizeof (*ref_header) + 3) / 4
                                     + (ref_name_length + 3) / 4);
                    var_body = (video_fmt_bs_heap_var_body_type *)((void*)
                                &context->var_heap [nIndex]);
                    context->var_heap_end += ref_header->size;
                }

                /* Cache heap entry location. */
                bs_stack_top->last_var_header = ref_header;
                bs_stack_top->last_var_body = var_body;

                /* Advance past the variable name. */
                bs_stack_top->syntax_pos += ref_name_length + 1;
                ++bs_stack_top->substate;

            case 2: /* read variable value from bitstream */
                /* Read the next given number of bits from the bitstream. */
                if (!video_fmt_bs_read_buffer
                    (context, bs_stack_top->nbits, FALSE))
                {
                    break;
                }
                temp32 = video_fmt_bs_data_out
                    (context, bs_stack_top->nbits, 0, TRUE);

                /* Store variable value. */
                var_body = (video_fmt_bs_heap_var_body_type *)
                    bs_stack_top->last_var_body;
                var_body->value = temp32;

                /* Issue callback to client regarding the variable. */
                context->cb_info.var_info.name
                    = bs_stack_top->last_var_header->name;
                context->cb_info.var_info.value = temp32;
                context->cb_info.var_info.offset
                    = context->abs_pos - bs_stack_top->nbits;
                context->cb_info.var_info.size = bs_stack_top->nbits;
                context->cb_info.var_info.callback_ptr
                    = video_fmt_bs_process;
                context->cb_info.var_info.server_data = context;
                context->callback_ptr (VIDEO_FMT_BS_VAR_INFO,
                                       context->client_data,
                                       &context->cb_info,
                                       video_fmt_bs_end);

                /* Proceed to the next field in the bitstream. */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
                exit_loop = TRUE;
            }
            break;

        case VIDEO_FMT_BS_STATE_VAR_LENGTH_VAR:
            /* Continue processing state. */
            switch (bs_stack_top->substate)
            {
            case 0: /* parse table name */
                /* Generate exception if end of syntax is reached. */
                if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: unexpected end of "
                               "syntax at pos=%lu!",
                                bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Expect single-quote character. */
                if (context->syntax [bs_stack_top->syntax_pos++] != '\'')
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                               "expected single-quoted name at pos=%lu!",
                               bs_stack_top->syntax_pos - 1);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Search for heap entry with given name.  If an entry was
                ** not found, or was found to not be a table, generate an
                ** exception.
                */
                if ((!video_fmt_bs_locate_name
                     (context, &ref_header, &ref_name_length, &ref_body))
                    || (ref_header->type != VIDEO_FMT_BS_HEAP_TABLE))
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                              "video_fmt_bs_process: undeclared table "
                              "used at pos=%lu",
                               bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }
                table_body = (video_fmt_bs_heap_table_body_type *)ref_body;

                /* Cache heap entry location. */
                bs_stack_top->last_var_header = ref_header;
                bs_stack_top->last_table_body = table_body;

                /* Advance past the table name. */
                bs_stack_top->syntax_pos += ref_name_length + 1;
                ++bs_stack_top->substate;

            case 1: /* decode variable-length field */
                /* Peek out enough bits from the bitstream to cover the widest
                ** possible value for the variable.
                */
                table_body = bs_stack_top->last_table_body;
                table_data = &table_body->data [0];/*lint -e661 */
                if (!video_fmt_bs_read_buffer
                    (context, table_body->widest, FALSE))
                {
                    break;
                }
                temp32 = video_fmt_bs_data_out
                    (context, table_body->widest, 0, FALSE);

                /* Search the table for a match. */
                for (i = 0; i < table_body->rows; ++i)/*lint -e661 */
                {
                   if ((temp32 >> (table_body->widest - table_data [i * table_body->columns]))
                        == table_data [i * table_body->columns+1])/*lint -e661 */
                   {
                      break;
                   }
                }

                /* If no match in table, generate a parsing exception. */
                if (i >= table_body->rows)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                               "no match in table found, pos=%lu",
                               bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Consume actual bits in bitstream. */
                video_fmt_bs_skip_data
                    (context, table_data [i * table_body->columns]);

                /* Save matching table entry and start parsing variable
                ** names.
                */
                bs_stack_top->last_table_entry = i;
                bs_stack_top->value [0] = 1;
                ++bs_stack_top->substate;

            case 2:/*lint!e616 no break for aove case */ /* parse variable name */
                /* Generate exception if end of syntax is reached. */
                if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: unexpected end of "
                               "syntax at pos=%lu!",
                                bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Expect single-quote character. */
                if (context->syntax [bs_stack_top->syntax_pos++] != '\'')
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                               "expected single-quoted name at pos=%lu!",
                               bs_stack_top->syntax_pos - 1);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Search for heap entry with given name.  If an entry was
                ** found but it is not a variable, generate an exception.
                */
                if (video_fmt_bs_locate_name
                    (context, &ref_header, &ref_name_length, &ref_body))
                {
                    if (ref_header->type != VIDEO_FMT_BS_HEAP_VARIABLE)
                    {
                      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                                   "video_fmt_bs_process: name referenced "
                                   "at pos=%lu is not a variable!",
                                   bs_stack_top->syntax_pos);
                      video_fmt_bs_failure (context);
                      return;
                    }
                    var_body = (video_fmt_bs_heap_var_body_type *) ref_body;
                }

                /* Otherwise, add a variable to the heap. */
                else
                {
                    /* Check for heap overflow. */
                    temp8 = (uint8)((sizeof (*ref_header) + 3) / 4
                                    + (sizeof (*var_body) + 3) / 4
                                    + (ref_name_length + 3) / 4);
                    if (context->var_heap_end + temp8 > VIDEO_FMT_BS_HEAP_SIZE)
                    {
                      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                                   "video_fmt_bs_process: name heap "
                                   "overflow at pos=%lu",
                                    bs_stack_top->syntax_pos);
                      video_fmt_bs_failure (context);
                      return;
                    }

                    /* Add heap entry. */
                    ref_header = (video_fmt_bs_heap_header_type *)((void *)
                                 &context->var_heap [context->var_heap_end]);
                    ref_header->size = temp8;
                    ref_header->type = VIDEO_FMT_BS_HEAP_VARIABLE;
                    ref_header->name_length = ref_name_length;
                    memcpy (ref_header->name,
                            &context->syntax [bs_stack_top->syntax_pos],
                            ref_name_length);
                    ref_header->name [ref_name_length] = '\0';
                    nIndex = (int)(context->var_heap_end
                                   + (sizeof (*ref_header) + 3) / 4
                                   + (ref_name_length + 3) / 4);
                    var_body = (video_fmt_bs_heap_var_body_type *)((void*)
                                                &context->var_heap [nIndex]);
                    context->var_heap_end += ref_header->size;
                }

                /* Advance past the variable name. */
                bs_stack_top->syntax_pos += ref_name_length + 1;

                /* Copy variable value from table. */
                table_body = bs_stack_top->last_table_body;
                table_data = &table_body->data [0];
                var_body->value = table_data
                    [bs_stack_top->last_table_entry
                     * table_body->columns + bs_stack_top->value [0]];

                /* Issue callback to client regarding the variable.  Note that
                ** only the first variable gets data from the actual
                ** bitstream.  The rest are purely informational.
                */
                context->cb_info.var_info.name = ref_header->name;
                context->cb_info.var_info.value = var_body->value;
                if (bs_stack_top->value [0] == 1)
                {
                    context->cb_info.var_info.size = table_data
                        [bs_stack_top->last_table_entry
                         * table_body->columns];
                    context->cb_info.var_info.offset
                        = context->abs_pos - context->cb_info.var_info.size;
                }
                else
                {
                    context->cb_info.var_info.offset = (uint32) -1;
                    context->cb_info.var_info.size = 0;
                }
                context->cb_info.var_info.callback_ptr
                    = video_fmt_bs_process;
                context->cb_info.var_info.server_data = context;
                context->callback_ptr (VIDEO_FMT_BS_VAR_INFO,
                                       context->client_data,
                                       &context->cb_info,
                                       video_fmt_bs_end);

                /* Advance to next column in table.  Loop back to read more
                ** variable names if not at the end of the table yet.
                */
                ++bs_stack_top->value [0];
                if ((int)bs_stack_top->value [0] < table_body->columns)
                {
                    break;
                }

                /* Proceed to the next field in the bitstream. */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
                exit_loop = TRUE;
            }
            break;

        case VIDEO_FMT_BS_STATE_FUNCTION_DECL:
            /* Generate exception if end of syntax is reached. */
            if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                           "video_fmt_bs_process: unexpected end of "
                           "syntax at pos=%lu!",
                            bs_stack_top->syntax_pos);
              video_fmt_bs_failure (context);
              return;
            }

            /* Expect single-quote character. */
            if (context->syntax [bs_stack_top->syntax_pos++] != '\'')
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                           "video_fmt_bs_process: error in bitstream - "
                           "expected single-quoted name at pos=%lu!",
                           bs_stack_top->syntax_pos - 1);
              video_fmt_bs_failure (context);
              return;
            }

            /* Search for heap entry with given name.  If an entry was
            ** found, generate an exception.
            */
            if (video_fmt_bs_locate_name
                (context, &ref_header, &ref_name_length, &ref_body))
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                           "video_fmt_bs_process: name referenced "
                           "at pos=%lu is already declared!",
                           bs_stack_top->syntax_pos);
              video_fmt_bs_failure (context);
              return;
            }

            /* Check for heap overflow. */
            temp8 = (uint8)((sizeof (*ref_header) + 3) / 4
                            + (sizeof (*func_body) + 3) / 4
                            + (ref_name_length + 3) / 4);
            if (context->var_heap_end + temp8 > VIDEO_FMT_BS_HEAP_SIZE)
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                           "video_fmt_bs_process: name heap "
                           "overflow at pos=%lu",
                            bs_stack_top->syntax_pos);
              video_fmt_bs_failure (context);
              return;
            }

            /* Add heap entry. */
            ref_header = (video_fmt_bs_heap_header_type *)((void*)
                         &context->var_heap [context->var_heap_end]);
            ref_header->size = temp8;
            ref_header->type = VIDEO_FMT_BS_HEAP_FUNCTION;
            ref_header->name_length = ref_name_length;
            memcpy (ref_header->name,
                    &context->syntax [bs_stack_top->syntax_pos],
                    ref_name_length);
            ref_header->name [ref_name_length] = '\0';
            nIndex = (int)(context->var_heap_end
                           + (sizeof (*ref_header) + 3) / 4
                           + (ref_name_length + 3) / 4);
            func_body = (video_fmt_bs_heap_func_body_type*)
                        ((void*)&context->var_heap [nIndex]);
            context->var_heap_end += ref_header->size;

            /* Advance past the function name. */
            bs_stack_top->syntax_pos += ref_name_length + 1;

            /* Generate exception if end of syntax is reached. */
            if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                           "video_fmt_bs_process: unexpected end of "
                           "syntax at pos=%lu!",
                            bs_stack_top->syntax_pos);
              video_fmt_bs_failure (context);
              return;
            }

            /* Expect open-brace character. */
            if (context->syntax [bs_stack_top->syntax_pos++] != '{')
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                           "video_fmt_bs_process: error in bitstream - "
                           "expected open-brace at pos=%lu!",
                            bs_stack_top->syntax_pos - 1);
              video_fmt_bs_failure (context);
              return;
            }

            /* Mark beginning of function. */
            func_body->start = bs_stack_top->syntax_pos;

            /* Search for end of function. */
            for (i = bs_stack_top->syntax_pos, j = 1;
                 (i < bs_stack_top->syntax_end) && (j > 0); ++i)
            {
                switch (context->syntax [i])
                {
                case '{': ++j; break;
                case '}': --j; break;
                }
            }

            /* If no end of function was found, generate an exception. */
            if (j > 0)
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                           "video_fmt_bs_process: error in bitstream - "
                           "unmatched open-brace at pos=%lu!",
                            func_body->start - 1);
              video_fmt_bs_failure (context);
              return;
            }

            /* Mark end of function. */
            func_body->end = i - 1;

            /* Proceed to the next field in the bitstream. */
            bs_stack_top->syntax_pos = i;
            bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
            break;

        case VIDEO_FMT_BS_STATE_FUNCTION_CALL:
            /* Continue processing state. */
            switch (bs_stack_top->substate)
            {
            case 0: /* parse function name */
                /* Generate exception if end of syntax is reached. */
                if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: unexpected end of "
                               "syntax at pos=%lu!", bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Expect single-quote character. */
                if (context->syntax [bs_stack_top->syntax_pos++] != '\'')
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                               "expected single-quoted name at pos=%lu!",
                                bs_stack_top->syntax_pos - 1);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Search for heap entry with given name.  If an entry was
                ** not found, generate an exception.
                */
                if (!video_fmt_bs_locate_name
                    (context, &ref_header, &ref_name_length, &ref_body))
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: undeclared function "
                               "called at pos=%lu",
                                bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* If the entry is not for a function, generate an
                ** exception.
                */
                if (ref_header->type != VIDEO_FMT_BS_HEAP_FUNCTION)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: undeclared function "
                               "called at pos=%lu",
                                bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }
                func_body = (video_fmt_bs_heap_func_body_type *) ref_body;

                /* Issue callback to the client to indicate a function was
                ** called at the current offset in the bitstream.
                */
                context->cb_info.func_call.name = ref_header->name;
                context->cb_info.func_call.offset = context->abs_pos;
                context->cb_info.func_call.callback_ptr
                    = video_fmt_bs_process;
                context->cb_info.func_call.server_data = context;
                context->callback_ptr (VIDEO_FMT_BS_FUNC_CALL,
                                       context->client_data,
                                       &context->cb_info,
                                       video_fmt_bs_end);

                /* Record syntax position past the function name. */
                bs_stack_top->syntax_pos += ref_name_length + 1;
                bs_stack_top->syntax_beg = bs_stack_top->syntax_pos;

                /* Call function by pushing new parsing position on stack, set
                ** to parse the referenced function.
                */
                ++bs_stack_top->substate;
                ++context->bs_stack_top;
                memcpy (&context->bs_stack [context->bs_stack_top - 1],
                        &context->bs_stack [context->bs_stack_top - 2],
                        sizeof (context->bs_stack [0]));
                bs_stack_top = &context->bs_stack [context->bs_stack_top - 1];
                bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
                bs_stack_top->syntax_pos = func_body->start;
                bs_stack_top->syntax_end = func_body->end;
                break;

            case 1: /* function returned */
                /* Issue callback to the client to indicate a function
                ** returned at the current offset in the bitstream.
                */
                context->cb_info.func_done.offset = context->abs_pos;
                context->cb_info.func_done.callback_ptr
                    = video_fmt_bs_process;
                context->cb_info.func_done.server_data = context;
                context->callback_ptr (VIDEO_FMT_BS_FUNC_DONE,
                                       context->client_data,
                                       &context->cb_info,
                                       video_fmt_bs_end);

                /* Return to the position in the syntax after the function
                ** call.
                */
                bs_stack_top->syntax_pos = bs_stack_top->syntax_beg;

                /* Proceed to the next field in the bitstream. */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
                break;
            }
            break;

        case VIDEO_FMT_BS_STATE_CONDITIONAL:
            /* Continue processing state. */
            switch (bs_stack_top->substate)
            {
            case 0: /* parse value */
                if (!video_fmt_bs_parse_value (context, 0))
                {
                    return;
                }
                break;

            case 1: /* evaluate conditional */
                /* Generate exception if end of syntax is reached. */
                if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: unexpected end of "
                               "syntax at pos=%lu!",
                                bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Expect open-brace character. */
                if (context->syntax [bs_stack_top->syntax_pos++] != '{')
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                               "expected open-brace at pos=%lu!",
                               bs_stack_top->syntax_pos - 1);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Search for end of block. */
                for (i = bs_stack_top->syntax_pos, j = 1;
                     (i < bs_stack_top->syntax_end) && (j > 0); ++i)
                {
                    switch (context->syntax [i])
                    {
                    case '{': ++j; break;
                    case '}': --j; break;
                    }
                }

                /* If no end of block was found, generate an exception. */
                if (j > 0)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                               "unmatched open-brace at pos=%lu!",
                                bs_stack_top->syntax_pos - 1);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* If the value just parsed evaluates to non-zero, evaluate
                ** "if" block.
                */
                if (bs_stack_top->value [0] != 0)
                {
                    ++bs_stack_top->substate;
                    ++context->bs_stack_top;
                    memcpy (&context->bs_stack [context->bs_stack_top - 1],
                            &context->bs_stack [context->bs_stack_top - 2],
                            sizeof (context->bs_stack [0]));
                    bs_stack_top = &context->bs_stack
                        [context->bs_stack_top - 1];
                    bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
                    bs_stack_top->syntax_end = i - 1;
                }

                /* Otherwise, skip past matching closed-brace and evaluate
                ** "else" block.
                */
                else
                {
                    /* Skip past "if" block. */
                    bs_stack_top->syntax_pos = i;

                    /* Generate exception if end of syntax is reached. */
                    if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
                    {
                      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                                   "video_fmt_bs_process: unexpected end of "
                                   "syntax at pos=%lu!",
                                    bs_stack_top->syntax_pos);
                      video_fmt_bs_failure (context);
                      return;
                    }

                    /* Expect open-brace character. */
                    if (context->syntax [bs_stack_top->syntax_pos++] != '{')
                    {
                      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                                   "video_fmt_bs_process: error in "
                                   "bitstream - expected open-brace "
                                   "at pos=%lu!",
                                    bs_stack_top->syntax_pos - 1);
                      video_fmt_bs_failure (context);
                      return;
                    }

                    /* Search for end of block. */
                    for (i = bs_stack_top->syntax_pos, j = 1;
                         (i < bs_stack_top->syntax_end) && (j > 0); ++i)
                    {
                        switch (context->syntax [i])
                        {
                        case '{': ++j; break;
                        case '}': --j; break;
                        }
                    }

                    /* If no end of block was found, generate an exception. */
                    if (j > 0)
                    {
                      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                                   "video_fmt_bs_process: error in "
                                   "bitstream - unmatched open-brace "
                                   "at pos=%lu!",
                                    bs_stack_top->syntax_pos - 1);
                      video_fmt_bs_failure (context);
                      return;
                    }

                    /* Evaluate block of statements as if it was
                    ** a function.
                    */
                    bs_stack_top->substate += 2;
                    ++context->bs_stack_top;
                    memcpy (&context->bs_stack [context->bs_stack_top - 1],
                            &context->bs_stack [context->bs_stack_top - 2],
                            sizeof (context->bs_stack [0]));
                    bs_stack_top = &context->bs_stack
                        [context->bs_stack_top - 1];
                    bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
                    bs_stack_top->syntax_end = i - 1;
                }
                break;

            case 2: /* skip "else" block */
                /* Skip end of "if" block. */
                ++bs_stack_top->syntax_pos;

                /* Generate exception if end of syntax is reached. */
                if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: unexpected end of "
                               "syntax at pos=%lu!",
                                bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Expect open-brace character. */
                if (context->syntax [bs_stack_top->syntax_pos++] != '{')
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in "
                               "bitstream - expected open-brace "
                               "at pos=%lu!",
                                bs_stack_top->syntax_pos - 1);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Search for end of block. */
                for (i = bs_stack_top->syntax_pos, j = 1;
                     (i < bs_stack_top->syntax_end) && (j > 0); ++i)
                {
                    switch (context->syntax [i])
                    {
                    case '{': ++j; break;
                    case '}': --j; break;
                    }
                }

                /* If no end of block was found, generate an exception. */
                if (j > 0)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in "
                               "bitstream - unmatched open-brace "
                               "at pos=%lu!",
                                bs_stack_top->syntax_pos - 1);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Set position to end of block. */
                bs_stack_top->syntax_pos = i - 1;
                ++bs_stack_top->substate;

            case 3: /*lint!e616 no break for aove case */ /* after end of "else" block */
                /* Skip end of "else" block. */
                ++bs_stack_top->syntax_pos;

                /* Proceed to the next field in the bitstream. */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
                break;
            }
            break;

        case VIDEO_FMT_BS_STATE_WHILE:
            /* Continue processing state. */
            switch (bs_stack_top->substate)
            {
            case 0: /* parse value */
                bs_stack_top->syntax_beg = bs_stack_top->syntax_pos;
                if (!video_fmt_bs_parse_value (context, 0))
                {
                    return;
                }
                break;

            case 1: /* evaluate conditional */
                /* Generate exception if end of syntax is reached. */
                if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: unexpected end of "
                                "syntax at pos=%lu!",
                                bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Expect open-brace character. */
                if (context->syntax [bs_stack_top->syntax_pos++] != '{')
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                               "expected open-brace at pos=%lu!",
                                bs_stack_top->syntax_pos - 1);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Search for end of block. */
                for (i = bs_stack_top->syntax_pos, j = 1;
                     (i < bs_stack_top->syntax_end) && (j > 0); ++i)
                {
                    switch (context->syntax [i])
                    {
                    case '{': ++j; break;
                    case '}': --j; break;
                    }
                }

                /* If no end of block was found, generate an exception. */
                if (j > 0)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                               "unmatched open-brace at pos=%lu!",
                                bs_stack_top->syntax_pos - 1);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* If the value just parsed evaluates to non-zero, evaluate
                ** "while" block, but set up to re-evaluate this statement
                ** afterwards.
                */
                if (bs_stack_top->value [0] != 0)
                {
                    ++bs_stack_top->substate;
                    ++context->bs_stack_top;
                    memcpy (&context->bs_stack [context->bs_stack_top - 1],
                            &context->bs_stack [context->bs_stack_top - 2],
                            sizeof (context->bs_stack [0]));
                    bs_stack_top = &context->bs_stack
                        [context->bs_stack_top - 1];
                    bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
                    bs_stack_top->syntax_end = i - 1;
                }

                /* Otherwise, skip past matching closed-brace and move on to
                ** the next statement.
                */
                else
                {
                    /* Skip past "while" block. */
                    bs_stack_top->syntax_pos = i;

                    /* Proceed to the next field in the bitstream. */
                    bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
                }
                break;

            case 2: /* loop back to top */
                bs_stack_top->syntax_pos = bs_stack_top->syntax_beg;
                bs_stack_top->substate = 0;
                break;
            }
            break;

        case VIDEO_FMT_BS_STATE_DO:
            /* Continue processing state. */
            switch (bs_stack_top->substate)
            {
            case 0: /* parse block */
                /* Generate exception if end of syntax is reached. */
                bs_stack_top->syntax_beg = bs_stack_top->syntax_pos;
                if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: unexpected end of "
                               "syntax at pos=%lu!",
                                bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Expect open-brace character. */
                if (context->syntax [bs_stack_top->syntax_pos++] != '{')
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                               "expected open-brace at pos=%lu!",
                                bs_stack_top->syntax_pos - 1);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Search for end of block. */
                for (i = bs_stack_top->syntax_pos, j = 1;
                     (i < bs_stack_top->syntax_end) && (j > 0); ++i)
                {
                    switch (context->syntax [i])
                    {
                    case '{': ++j; break;
                    case '}': --j; break;
                    }
                }

                /* If no end of block was found, generate an exception. */
                if (j > 0)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                                "unmatched open-brace at pos=%lu!",
                                bs_stack_top->syntax_pos - 1);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Evaluate the "do" block. */
                ++bs_stack_top->substate;
                ++context->bs_stack_top;
                memcpy (&context->bs_stack [context->bs_stack_top - 1],
                        &context->bs_stack [context->bs_stack_top - 2],
                        sizeof (context->bs_stack [0]));
                bs_stack_top = &context->bs_stack
                    [context->bs_stack_top - 1];
                bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
                bs_stack_top->syntax_end = i - 1;
                break;

            case 1: /* parse value */
                ++bs_stack_top->syntax_pos;
                if (!video_fmt_bs_parse_value (context, 0))
                {
                    return;
                }
                break;

            case 2: /* evaluate conditional */
                /* If "do" condition is non-zero, repeat this statement. */
                if (bs_stack_top->value [0] != 0)
                {
                    bs_stack_top->syntax_pos = bs_stack_top->syntax_beg;
                    bs_stack_top->substate = 0;
                }

                /* Otherwise, proceed to the next field in the bitstream. */
                else
                {
                    bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
                }
                break;
            }
            break;

        case VIDEO_FMT_BS_STATE_TABLE:
            /* Continue processing state. */
            switch (bs_stack_top->substate)
            {
            case 0: /* parse column count */
                if (!video_fmt_bs_parse_nbits (context))
                {
                    return;
                }
                break;

            case 1: /* parse table name */
                /* Generate exception if end of syntax is reached. */
                if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: unexpected end of "
                               "syntax at pos=%lu!",
                                bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Expect single-quote character. */
                if (context->syntax [bs_stack_top->syntax_pos++] != '\'')
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                               "expected single-quoted name at pos=%lu!",
                               bs_stack_top->syntax_pos - 1);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Search for heap entry with given name.  If an entry was
                ** found, generate an exception.
                */
                if (video_fmt_bs_locate_name
                    (context, &ref_header, &ref_name_length, &ref_body))
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: name referenced "
                               "at pos=%lu is already declared!",
                                bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Check for heap overflow. */
                /* first word of table data counted twice */
                temp8 = (uint8)((sizeof (*ref_header) + 3) / 4
                                + (sizeof (*table_body) + 3) / 4
                                + (ref_name_length + 3) / 4
                                - 1);
                if (context->var_heap_end + temp8> VIDEO_FMT_BS_HEAP_SIZE)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: name heap "
                               "overflow at pos=%lu",
                                bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Add heap entry. */
                ref_header = (video_fmt_bs_heap_header_type *)((void*)
                             &context->var_heap [context->var_heap_end]);
                ref_header->size = temp8;
                ref_header->type = VIDEO_FMT_BS_HEAP_TABLE;
                ref_header->name_length = ref_name_length;
                memcpy (ref_header->name,
                        &context->syntax [bs_stack_top->syntax_pos],
                        ref_name_length);
                ref_header->name [ref_name_length] = '\0';
                nIndex = (int)(context->var_heap_end
                               + (sizeof (*ref_header) + 3) / 4
                               + (ref_name_length + 3) / 4);
                table_body = (video_fmt_bs_heap_table_body_type*)
                             ((void*)&context->var_heap [nIndex]);
                context->var_heap_end += ref_header->size;

                /* Advance past the function name. */
                bs_stack_top->syntax_pos += ref_name_length + 1;

                /* Generate exception if end of syntax is reached. */
                if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: unexpected end of "
                               "syntax at pos=%lu!",
                               bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Expect open-brace character. */
                if (context->syntax [bs_stack_top->syntax_pos++] != '{')
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: error in bitstream - "
                               "expected open-brace at pos=%lu!",
                               bs_stack_top->syntax_pos - 1);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Cache heap entry location. */
                bs_stack_top->last_table_header = ref_header;
                bs_stack_top->last_table_body = table_body;

                /* Set up to parse table values. */
                bs_stack_top->value [1] = 0;
                table_body->widest = 0;
                ++bs_stack_top->substate;

            case 2: /*lint!e616 no break for aove case */ /* read table value */
                /* Generate exception if end of syntax is reached. */
                if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: unexpected end of "
                                "syntax at pos=%lu!",
                                bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Check for end of table. */
                if (context->syntax [bs_stack_top->syntax_pos] == '}')
                {
                    /* Eat delimiter. */
                    ++bs_stack_top->syntax_pos;

                    /* If a whole number of rows has not been read, throw a
                    ** parsing exception.
                    */
                    if (bs_stack_top->value [1] % bs_stack_top->nbits)
                    {
                      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                                   "video_fmt_bs_process: "
                                   "missing table elements at pos=%lu!",
                                   bs_stack_top->syntax_pos - 1);
                      video_fmt_bs_failure (context);
                      return;
                    }

                    /* Finish up table and proceed to the next statement. */
                    table_body = bs_stack_top->last_table_body;
                    table_body->rows = bs_stack_top->value [1]
                        / bs_stack_top->nbits;
                    table_body->columns = bs_stack_top->nbits;
                    ref_header = bs_stack_top->last_table_header;
                    ref_header->size = (uint16)
                        ((sizeof (*ref_header) + 3) / 4
                         + (sizeof (*table_body) + 3) / 4
                         + (ref_header->name_length + 3) / 4
                         + (bs_stack_top->value [1] - 1));
                    bs_stack_top->state = VIDEO_FMT_BS_STATE_STATEMENT;
                    break;
                }

                /* Parse next table value. */
                if (!video_fmt_bs_parse_value (context, 0))
                {
                    return;
                }
                break;

            case 3: /* add table value */
                /* Check for heap overflow. */
                if (context->var_heap_end + 1 > VIDEO_FMT_BS_HEAP_SIZE)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: name heap "
                               "overflow at pos=%lu",
                               bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* Add value to table on heap. */
                table_body = bs_stack_top->last_table_body;
                table_data = &table_body->data [0];
                table_data [bs_stack_top->value [1]++]
                    = bs_stack_top->value [0];
                ++context->var_heap_end;

                /* For the first column, accumulate largest value. */
                if (!((bs_stack_top->value [1] - 1) % bs_stack_top->nbits))
                {
                    table_body = bs_stack_top->last_table_body;
                    table_body->widest
                        = FILESOURCE_MAX (table_body->widest, bs_stack_top->value [0]);
                }

                /* Go back and parse for more table entries or end of table
                ** delimiter.
                */
                bs_stack_top->substate = 2;
            }
            break;

        case VIDEO_FMT_BS_STATE_PARSE_NBITS:
            /* Continue processing state. */
            switch (bs_stack_top->substate)
            {
            case 0: /* first character */
                /* Generate exception if end of syntax is reached. */
                if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                               "video_fmt_bs_process: unexpected end of "
                                "syntax at pos=%lu!",
                                bs_stack_top->syntax_pos);
                  video_fmt_bs_failure (context);
                  return;
                }

                /* If the first character is a single-quote, look up variable
                ** and set n-bits to be the minimum number of bits needed to
                ** store the variable's value.
                */
                if (context->syntax [bs_stack_top->syntax_pos] == '\'')
                {
                    /* Locate the heap entry with the name given in the
                    ** syntax.  If the name was not found, generate an
                    ** exception.
                    */
                    ++bs_stack_top->syntax_pos;
                    if (!video_fmt_bs_locate_name
                        (context, &ref_header, &ref_name_length, &ref_body))
                    {
                      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                                   "video_fmt_bs_process: unknown variable "
                                   "referenced at pos=%lu",
                                    bs_stack_top->syntax_pos);
                      video_fmt_bs_failure (context);
                      return;
                    }

                    /* Generate an exception if the entry is not a
                    ** variable.
                    */
                    if (ref_header->type != VIDEO_FMT_BS_HEAP_VARIABLE)
                    {
                      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                                   "video_fmt_bs_process: name referenced "
                                   "at pos=%lu is not a variable!",
                                    bs_stack_top->syntax_pos);
                      video_fmt_bs_failure (context);
                      return;
                    }

                    /* Store the value of the referenced variable in state
                    ** above as the nbits value, and return to that state.
                    */
                    var_body = (video_fmt_bs_heap_var_body_type *) ref_body;
                    bs_stack_top->syntax_pos += ref_header->name_length + 1;
                    --context->bs_stack_top;
                    bs_stack_top = &context->bs_stack
                        [context->bs_stack_top - 1];
                    bs_stack_top->nbits = var_body->value;
                    bs_stack_top->syntax_pos = context->bs_stack
                        [context->bs_stack_top].syntax_pos;
                    ++bs_stack_top->substate;
                    break;
                }

                /* Otherwise, proceed to the next state. */
                ++bs_stack_top->substate;

            case 1: /*lint!e616 no break for aove case */ /* number field */
                /* If the next character is not a decimal digit, finish parsing
                ** this field.
                */
                if ((context->syntax [bs_stack_top->syntax_pos] < '0')
                    || (context->syntax [bs_stack_top->syntax_pos] > '9'))
                {
                    /* If no characters have yet been read, generate an
                    ** exception.
                    */
                    if (bs_stack_top->substate == 0)
                    {
                      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                                   "video_fmt_bs_process: expected decimal "
                                   "digit for #bits field in syntax "
                                   "at pos=%lu!",
                                   bs_stack_top->syntax_pos);
                      video_fmt_bs_failure (context);
                      return;
                    }

                    /* Copy the "# bits" field to the state above, and return
                    ** to that state.
                    */
                    --context->bs_stack_top;
                    bs_stack_top = &context->bs_stack
                        [context->bs_stack_top - 1];
                    bs_stack_top->nbits = context->bs_stack
                        [context->bs_stack_top].nbits;
                    bs_stack_top->syntax_pos = context->bs_stack
                        [context->bs_stack_top].syntax_pos;
                    ++bs_stack_top->substate;
                    break;
                }

                /* Add decimal digit to "# bits" field. */
                bs_stack_top->nbits *= 10;
                bs_stack_top->nbits += (uint32)
                    (context->syntax [bs_stack_top->syntax_pos++] - '0');
                break;
            }
            break;

        case VIDEO_FMT_BS_STATE_PARSE_VALUE:
            /* Generate exception if end of syntax is reached. */
            if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                           "video_fmt_bs_process: unexpected end of "
                           "syntax at pos=%lu!",
                           bs_stack_top->syntax_pos);
              video_fmt_bs_failure (context);
              return;
            }

            /* Read value type character and translate into next parser
            ** state.
            */
            switch (context->syntax [bs_stack_top->syntax_pos++])
            {
            case '#': /* hexadecimal digits */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_HEX;
                break;

            case '\'': /* named variable */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_VAR_REF;
                break;

            case '|': /* logical OR */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_EXPR;
                bs_stack_top->expr = VIDEO_FMT_BS_EXPR_LOGICAL_OR;
                break;

            case '&': /* logical AND */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_EXPR;
                bs_stack_top->expr = VIDEO_FMT_BS_EXPR_LOGICAL_AND;
                break;

            case '!': /* logical NOT */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_EXPR;
                bs_stack_top->expr = VIDEO_FMT_BS_EXPR_LOGICAL_NOT;
                break;

            case 'O': /* bitwise OR */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_EXPR;
                bs_stack_top->expr = VIDEO_FMT_BS_EXPR_BITWISE_OR;
                break;

            case 'A': /* bitwise AND */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_EXPR;
                bs_stack_top->expr = VIDEO_FMT_BS_EXPR_BITWISE_AND;
                break;

            case '~': /* bitwise NOT */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_EXPR;
                bs_stack_top->expr = VIDEO_FMT_BS_EXPR_BITWISE_NOT;
                break;

            case '<': /* left-shift */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_EXPR;
                bs_stack_top->expr = VIDEO_FMT_BS_EXPR_SHIFT_LEFT;
                break;

            case '>': /* right-shift */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_EXPR;
                bs_stack_top->expr = VIDEO_FMT_BS_EXPR_SHIFT_RIGHT;
                break;

            case '+': /* add */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_EXPR;
                bs_stack_top->expr = VIDEO_FMT_BS_EXPR_ADD;
                break;

            case '-': /* subtract */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_EXPR;
                bs_stack_top->expr = VIDEO_FMT_BS_EXPR_SUBTRACT;
                break;

            case '*': /* multiply */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_EXPR;
                bs_stack_top->expr = VIDEO_FMT_BS_EXPR_MULTIPLY;
                break;

            case '/': /* divide */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_EXPR;
                bs_stack_top->expr = VIDEO_FMT_BS_EXPR_DIVIDE;
                break;

            case '=': /* equal comparison */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_EXPR;
                bs_stack_top->expr = VIDEO_FMT_BS_EXPR_EQUAL;
                break;

            case 'L': /* less-than comparison */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_EXPR;
                bs_stack_top->expr = VIDEO_FMT_BS_EXPR_LESS;
                break;

            case 'G': /* greater-than comparison */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_EXPR;
                bs_stack_top->expr = VIDEO_FMT_BS_EXPR_GREATER;
                break;

            case 'n': /* look-ahead fixed constant */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_NEXT_FIXED_CONST;
                break;

            case 'N': /* look-ahead byte-aligned fixed constant */
                bs_stack_top->state
                    = VIDEO_FMT_BS_STATE_NEXT_FIXED_CONST_ALIGNED;
                break;

            case 'r': /* look-ahead ranged constant */
                bs_stack_top->state = VIDEO_FMT_BS_STATE_NEXT_RANGED_CONST;
                break;

            case 'o': /* current bit offset into bitstream */
                --context->bs_stack_top;
                bs_stack_top = &context->bs_stack [context->bs_stack_top - 1];
                bs_stack_top->value [bs_stack_top->which_value]
                    = context->abs_pos;
                bs_stack_top->syntax_pos
                    = context->bs_stack [context->bs_stack_top].syntax_pos;
                ++bs_stack_top->substate;
                break;

            default:
                /* If we reach here, there is a typo in the syntax. */
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                           "video_fmt_bs_process: unknown value "
                           "type at pos=%lu!",
                            bs_stack_top->syntax_pos);
              video_fmt_bs_failure (context);
              return;
            }
            break;

        case VIDEO_FMT_BS_STATE_PARSE_HEX:
            /* Generate exception if end of syntax is reached. */
            if (bs_stack_top->syntax_pos >= bs_stack_top->syntax_end)
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                           "video_fmt_bs_process: unexpected end of "
                           "syntax at pos=%lu!",
                           bs_stack_top->syntax_pos);
              video_fmt_bs_failure (context);
              return;
            }

            /* If the next character is a pound sign, finish parsing this
            ** field.
            */
            if (context->syntax [bs_stack_top->syntax_pos] == '#')
            {
                /* Copy the "value" field to the state above, and return to
                ** that state.
                */
                ++bs_stack_top->syntax_pos;
                --context->bs_stack_top;
                bs_stack_top = &context->bs_stack [context->bs_stack_top - 1];
                bs_stack_top->value [bs_stack_top->which_value]
                    = context->bs_stack [context->bs_stack_top].value [0];
                bs_stack_top->syntax_pos
                    = context->bs_stack [context->bs_stack_top].syntax_pos;
                ++bs_stack_top->substate;
                break;
            }

            /* Generate exception if 32 bits is exceeded. */
            if (bs_stack_top->substate >= 8)
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                           "video_fmt_bs_process: hexadecimal constant "
                           "exceeds 32 bits at pos=%lu!",
                           bs_stack_top->syntax_pos);
              video_fmt_bs_failure (context);
              return;
            }

            /* Translate hexadecimal digit to number and add to value. */
            bs_stack_top->value [0] <<= 4;
            if ((context->syntax [bs_stack_top->syntax_pos] >= '0')
                && (context->syntax [bs_stack_top->syntax_pos] <= '9'))
            {
                bs_stack_top->value [0] += (uint32)
                    (context->syntax [bs_stack_top->syntax_pos++] - '0');
            }
            else if ((context->syntax [bs_stack_top->syntax_pos] >= 'A')
                     && (context->syntax [bs_stack_top->syntax_pos] <= 'F'))
            {
                bs_stack_top->value [0] += 10 + (uint32)
                    (context->syntax [bs_stack_top->syntax_pos++] - 'A');
            }
            else
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                           "video_fmt_bs_process: expected hexadecimal "
                           "digit for value field in syntax at pos=%lu",
                           bs_stack_top->syntax_pos);
              video_fmt_bs_failure (context);
              return;
            }
            ++bs_stack_top->substate;
            break;

        case VIDEO_FMT_BS_STATE_PARSE_VAR_REF:
            /* Locate the heap entry with the name given in the syntax.  If
            ** the name was not found, generate an exception.
            */
            if (!video_fmt_bs_locate_name
                (context, &ref_header, &ref_name_length, &ref_body))
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                           "video_fmt_bs_process: unknown variable "
                           "referenced at pos=%lu",
                           bs_stack_top->syntax_pos);
              video_fmt_bs_failure (context);
              return;
            }

            /* Generate an exception if the entry is not a variable. */
            if (ref_header->type != VIDEO_FMT_BS_HEAP_VARIABLE)
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                           "video_fmt_bs_process: name referenced "
                           "at pos=%lu is not a variable!",
                           bs_stack_top->syntax_pos);
              video_fmt_bs_failure (context);
              return;
            }

            /* Store the value of the referenced variable in state above, and
            ** return to that state.
            */
            bs_stack_top->syntax_pos += ref_header->name_length + 1;
            var_body = (video_fmt_bs_heap_var_body_type *) ref_body;
            --context->bs_stack_top;
            bs_stack_top = &context->bs_stack [context->bs_stack_top - 1];
            bs_stack_top->value [bs_stack_top->which_value]
                = var_body->value;
            bs_stack_top->syntax_pos
                = context->bs_stack [context->bs_stack_top].syntax_pos;
            ++bs_stack_top->substate;
            break;

        case VIDEO_FMT_BS_STATE_PARSE_EXPR:
            /* Continue processing state. */
            switch (bs_stack_top->substate)
            {
            case 0: /* parse value #1 */
                if (!video_fmt_bs_parse_value (context, 0))
                {
                    return;
                }
                break;

            case 1: /* parse delimiter */
                /* If the expression only needs one value, skip the second
                ** value.
                */
                if ((bs_stack_top->expr == VIDEO_FMT_BS_EXPR_LOGICAL_NOT)
                    || (bs_stack_top->expr == VIDEO_FMT_BS_EXPR_BITWISE_NOT))
                {
                    bs_stack_top->substate = 3;
                    break;
                }

                /* Fall through to next substate. */
                ++bs_stack_top->substate;

            case 2: /*lint!e616 no break for aove case */ /* parse value #2 */
                if (!video_fmt_bs_parse_value (context, 1))
                {
                    return;
                }
                break;

            case 3: /* evaluate expression on the value(s) */
                switch (bs_stack_top->expr)
                {
                case VIDEO_FMT_BS_EXPR_LOGICAL_OR:
                    bs_stack_top->value [0]
                        = (bs_stack_top->value [0]
                           || bs_stack_top->value [1]);
                    break;

                case VIDEO_FMT_BS_EXPR_LOGICAL_AND:
                    bs_stack_top->value [0]
                        = (bs_stack_top->value [0]
                           && bs_stack_top->value [1]);
                    break;

                case VIDEO_FMT_BS_EXPR_LOGICAL_NOT:
                    bs_stack_top->value [0] = !bs_stack_top->value [0];
                    break;

                case VIDEO_FMT_BS_EXPR_BITWISE_OR:
                    bs_stack_top->value [0]
                        = (bs_stack_top->value [0]
                           | bs_stack_top->value [1]);
                    break;

                case VIDEO_FMT_BS_EXPR_BITWISE_AND:
                    bs_stack_top->value [0]
                        = (bs_stack_top->value [0]
                           & bs_stack_top->value [1]);
                    break;

                case VIDEO_FMT_BS_EXPR_BITWISE_NOT:
                    bs_stack_top->value [0] = ~bs_stack_top->value [0];
                    break;

                case VIDEO_FMT_BS_EXPR_SHIFT_LEFT:
                    bs_stack_top->value [0]
                        = (bs_stack_top->value [0]
                           << bs_stack_top->value [1]);
                    break;

                case VIDEO_FMT_BS_EXPR_SHIFT_RIGHT:
                    bs_stack_top->value [0]
                        = (bs_stack_top->value [0]
                           >> bs_stack_top->value [1]);
                    break;

                case VIDEO_FMT_BS_EXPR_ADD:
                    bs_stack_top->value [0]
                        = (bs_stack_top->value [0]
                           + bs_stack_top->value [1]);
                    break;

                case VIDEO_FMT_BS_EXPR_SUBTRACT:
                    bs_stack_top->value [0]
                        = (bs_stack_top->value [0]
                           - bs_stack_top->value [1]);
                    break;

                case VIDEO_FMT_BS_EXPR_MULTIPLY:
                    bs_stack_top->value [0]
                        = (bs_stack_top->value [0]
                           * bs_stack_top->value [1]);
                    break;

                case VIDEO_FMT_BS_EXPR_DIVIDE:
                    bs_stack_top->value [0]
                        = (bs_stack_top->value [0]
                           / bs_stack_top->value [1]);
                    break;

                case VIDEO_FMT_BS_EXPR_EQUAL:
                    bs_stack_top->value [0]
                        = (bs_stack_top->value [0]
                           == bs_stack_top->value [1]);
                    break;

                case VIDEO_FMT_BS_EXPR_LESS:
                    bs_stack_top->value [0]
                        = (bs_stack_top->value [0]
                           < bs_stack_top->value [1]);
                    break;

                case VIDEO_FMT_BS_EXPR_GREATER:
                    bs_stack_top->value [0]
                        = (bs_stack_top->value [0]
                           > bs_stack_top->value [1]);
                    break;
                default:
                    break;
                }

                /* Copy the result to the "value" field in the state above,
                ** and return to that state.
                */
                --context->bs_stack_top;
                bs_stack_top = &context->bs_stack [context->bs_stack_top - 1];
                bs_stack_top->value [bs_stack_top->which_value]
                    = context->bs_stack [context->bs_stack_top].value [0];
                bs_stack_top->syntax_pos
                    = context->bs_stack [context->bs_stack_top].syntax_pos;
                ++bs_stack_top->substate;
            }
            break;

        case VIDEO_FMT_BS_STATE_NEXT_FIXED_CONST:
            /* Continue processing state. */
            switch (bs_stack_top->substate)
            {
            case 0: /* parse n-bits */
                if (!video_fmt_bs_parse_nbits (context))
                {
                    return;
                }
                break;

            case 1: /* parse value */
                if (!video_fmt_bs_parse_value (context, 0))
                {
                    return;
                }
                break;

            case 2: /* compare value with next bits in bitstream */
                /* Peek out the next given number of bits from the
                ** bitstream. */
                if (!video_fmt_bs_read_buffer
                    (context, bs_stack_top->nbits, TRUE))
                {
                    break;
                }
                temp32 = video_fmt_bs_data_out
                    (context, bs_stack_top->nbits, 0, FALSE);

                /* Compare with the expected constant value and store
                ** comparison result in value one position up stack.
                */
                --context->bs_stack_top;
                bs_stack_top = &context->bs_stack [context->bs_stack_top - 1];
                bs_stack_top->value [bs_stack_top->which_value]
                    = (temp32 == context->bs_stack
                       [context->bs_stack_top].value [0]);
                bs_stack_top->syntax_pos
                    = context->bs_stack [context->bs_stack_top].syntax_pos;
                ++bs_stack_top->substate;
            }
            break;

        case VIDEO_FMT_BS_STATE_NEXT_FIXED_CONST_ALIGNED:
            /* Continue processing state. */
            switch (bs_stack_top->substate)
            {
            case 0: /* parse n-bits */
                if (!video_fmt_bs_parse_nbits (context))
                {
                    return;
                }
                break;

            case 1: /* parse value */
                if (!video_fmt_bs_parse_value (context, 0))
                {
                    return;
                }
                break;

            case 2: /* compare value with next bits in bitstream */
                /* Peek out the next given number of bits from the
                ** bitstream, from the first byte-aligned position.
                */
                i = (context->abs_pos + 7) / 8 * 8 - context->abs_pos;
                j = bs_stack_top->nbits + i;
                if (!video_fmt_bs_read_buffer (context, j, TRUE))
                {
                    break;
                }
                temp32 = video_fmt_bs_data_out
                    (context, bs_stack_top->nbits, i, FALSE);

                /* Compare with the expected constant value and store
                ** comparison result in value one position up stack.
                */
                --context->bs_stack_top;
                bs_stack_top = &context->bs_stack
                    [context->bs_stack_top - 1];
                bs_stack_top->value [bs_stack_top->which_value]
                    = (temp32 == context->bs_stack
                       [context->bs_stack_top].value [0]);
                bs_stack_top->syntax_pos = context->bs_stack
                    [context->bs_stack_top].syntax_pos;
                ++bs_stack_top->substate;
            }
            break;

        case VIDEO_FMT_BS_STATE_NEXT_RANGED_CONST:
            /* Continue processing state. */
            switch (bs_stack_top->substate)
            {
            case 0: /* parse n-bits */
                if (!video_fmt_bs_parse_nbits (context))
                {
                    return;
                }
                break;

            case 1: /* parse value #1 */
                if (!video_fmt_bs_parse_value (context, 0))
                {
                    return;
                }
                break;

            case 2: /* parse value #2 */
                /* Parse second constant value. */
                if (!video_fmt_bs_parse_value (context, 1))
                {
                    return;
                }
                break;

            case 3: /* expect constant value */
                /* Read the next given number of bits from the bitstream. */
                if (!video_fmt_bs_read_buffer
                    (context, bs_stack_top->nbits, TRUE))
                {
                    break;
                }
                temp32 = video_fmt_bs_data_out
                    (context, bs_stack_top->nbits, 0, FALSE);

                /* Compare with the expected constant value range and store
                ** comparison result in value one position up stack.
                */
                --context->bs_stack_top;
                bs_stack_top = &context->bs_stack [context->bs_stack_top - 1];
                bs_stack_top->value [bs_stack_top->which_value]
                    = ((temp32 >= context->bs_stack
                        [context->bs_stack_top].value [0])
                       && (temp32 <= context->bs_stack
                           [context->bs_stack_top].value [1]));
                bs_stack_top->syntax_pos
                    = context->bs_stack [context->bs_stack_top].syntax_pos;
                ++bs_stack_top->substate;
            }
            break;

        default:
            /* We should hopefully never reach here.  However, if we do, clean
            ** up as best we can, and exit.
            */
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                      "video_fmt_bs_process: Invalid video "
                      "bitstream services state");
          video_fmt_bs_failure (context);
          return;
        }
    }
}

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
void video_fmt_bs_failure (
  video_fmt_bs_context_type  *context
) {
    video_fmt_bs_status_cb_func_type  callback_ptr;
    void                              *client_data;
    video_fmt_bs_free_type            free;

    /* Deallocate memory used by the video bitstream services. */
    callback_ptr = context->callback_ptr;
    client_data = context->client_data;
    free.ptr = context;
    callback_ptr (VIDEO_FMT_BS_FREE, client_data,
                  &free, NULL);

    /* Stop services by indicating failure code to client. */
    callback_ptr (VIDEO_FMT_BS_FAILURE, client_data, NULL, NULL);
}

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
boolean video_fmt_bs_read_buffer (
  video_fmt_bs_context_type  *context,
  uint32                     num_bits,
  boolean                    expect_eof
) {
    video_fmt_bs_pos_type  *bs_stack_top;

    /* If the requested number of bits is already available, return TRUE. */
    bs_stack_top = &context->bs_stack [context->bs_stack_top - 1];
    if (context->in_buffer_size >= num_bits)
    {
        return TRUE;
    }

    /* If the total number of bits will not fit in the current buffer, move
    ** the data we already have into the other buffer.
    */
    if (context->in_buffer_pos + num_bits >= VIDEO_FMT_BS_BUFFER_SIZE * 8)
    {
        if (context->in_buffer_size > 0)
        {
            memcpy (&context->in_buffers [1 - context->in_buffer_which] [0],
                    &context->in_buffers [context->in_buffer_which]
                    [context->in_buffer_pos / 8],
                    (context->in_buffer_pos + context->in_buffer_size + 7) / 8
                    - context->in_buffer_pos / 8);
        }
        context->in_buffer_which = 1 - context->in_buffer_which;
        context->in_buffer_pos %= 8;
    }

    /* Check to make sure we are not going to overflow the input buffer. */
    if (context->in_buffer_pos + num_bits - context->in_buffer_size
        > VIDEO_FMT_BS_BUFFER_SIZE * 8)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "video_fmt_bs_process: in_buffer too small to "
                  "support this read request!");
      video_fmt_bs_failure (context);
      return FALSE;
    }

    /* Check for pending state machine stack overflow. */
    if (context->bs_stack_top + 2 > VIDEO_FMT_BS_POS_STACK_DEPTH)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "video_fmt_bs_read_buffer: video bitstream position "
                  "stack overflow");
      video_fmt_bs_failure (context);
      return FALSE;
    }

    /* Set up variables for getting data into the buffer. */
    context->get_data_dst = &context->in_buffers
        [context->in_buffer_which]
        [(context->in_buffer_pos + context->in_buffer_size) / 8];
    context->get_data_src = (context->abs_pos + context->in_buffer_size) / 8;
    context->get_data_size = VIDEO_FMT_BS_BUFFER_SIZE
        - (context->in_buffer_pos + context->in_buffer_size) / 8;
    context->get_data_needed = (context->in_buffer_pos + num_bits + 7) / 8
        - (context->in_buffer_pos + context->in_buffer_size + 7) / 8;
    context->get_data_read = 0;
    context->expect_eof = expect_eof;

    /* Set state machine to go into "update buffer" state once we get the
    ** data.
    */
    ++context->bs_stack_top;
    memcpy (&context->bs_stack [context->bs_stack_top - 1],
            &context->bs_stack [context->bs_stack_top - 2],
            sizeof (context->bs_stack [0]));
    bs_stack_top = &context->bs_stack [context->bs_stack_top - 1];
    bs_stack_top->state = VIDEO_FMT_BS_STATE_UPDATE_BUFFER;

    /* Set state machine to "get_data" state to get the data. */
    ++context->bs_stack_top;
    memcpy (&context->bs_stack [context->bs_stack_top - 1],
            &context->bs_stack [context->bs_stack_top - 2],
            sizeof (context->bs_stack [0]));
    bs_stack_top = &context->bs_stack [context->bs_stack_top - 1];
    bs_stack_top->state = VIDEO_FMT_BS_STATE_GET_DATA;
    return FALSE;
}

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
uint32 video_fmt_bs_data_out (
  video_fmt_bs_context_type  *context,
  uint32                     amount,
  uint32                     skipBits,
  boolean                    consume
) {
    uint32 value = 0;
    uint32 num_bits;
    uint32 offset = context->in_buffer_pos + skipBits;
    uint32 orig_amount = amount;

    /* Repeatedly consume data in bytes until all bits consumed. */
    while (amount)
    {
        /* Determine the number of bits to get from the current byte. */
        num_bits = FILESOURCE_MIN (8 - (offset % 8), amount);

        /* Shift up bits already read. */
        value <<= num_bits;

        /* Add in bits read from the current byte. */
        value |= ((context->in_buffers
                   [context->in_buffer_which] [offset / 8]
                   >> ((8 - num_bits) - (offset % 8)))
                  & ((1 << num_bits) - 1));

        /* Count the bits read, and advance the buffer pointer. */
        amount -= num_bits;
        offset += num_bits;
    }

    /* Update absolute bitstream position, and input buffer size, if the
    ** consume flag is set.
    */
    if (consume)
    {
        context->abs_pos += orig_amount;
        context->in_buffer_size -= orig_amount;
        context->in_buffer_pos = offset;
    }

    /* Return value parsed. */
    return value;
}

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
void video_fmt_bs_skip_data (
  video_fmt_bs_context_type  *context,
  uint32                     amount
) {
    uint32 amt_discarded;

    /* Remove any data from the input buffer that falls in the region being
    ** skipped.
    */
    amt_discarded = FILESOURCE_MIN (context->in_buffer_size, amount);
    context->in_buffer_pos += amt_discarded;
    context->in_buffer_size -= amt_discarded;

    /* Advance the absolute file pointer past the amount to be skipped. */
    context->abs_pos += amount;
}

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
boolean video_fmt_bs_parse_nbits (
  video_fmt_bs_context_type  *context
) {
    video_fmt_bs_pos_type  *bs_stack_top;

    /* Check for pending state machine stack overflow. */
    if (context->bs_stack_top >= VIDEO_FMT_BS_POS_STACK_DEPTH)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "video_fmt_bs_parse_nbits: video bitstream position "
                  "stack overflow");
      video_fmt_bs_failure (context);
      return FALSE;
    }

    /* Push new state onto bitstream position stack. */
    ++context->bs_stack_top;
    memcpy (&context->bs_stack [context->bs_stack_top - 1],
            &context->bs_stack [context->bs_stack_top - 2],
            sizeof (context->bs_stack [0]));
    bs_stack_top = &context->bs_stack [context->bs_stack_top - 1];
    bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_NBITS;
    bs_stack_top->nbits = 0;
    bs_stack_top->substate = 0;
    return TRUE;
}

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
boolean video_fmt_bs_parse_value (
  video_fmt_bs_context_type  *context,
  uint32                     which
) {
    video_fmt_bs_pos_type  *bs_stack_top;

    /* Check for pending state machine stack overflow. */
    if (context->bs_stack_top >= VIDEO_FMT_BS_POS_STACK_DEPTH)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "video_fmt_bs_parse_nbits: video bitstream position "
                  "stack overflow");
      video_fmt_bs_failure (context);
      return FALSE;
    }

    /* Push new state onto bitstream position stack. */
    bs_stack_top = &context->bs_stack [context->bs_stack_top - 1];
    bs_stack_top->which_value = which;
    ++context->bs_stack_top;
    memcpy (&context->bs_stack [context->bs_stack_top - 1],
            &context->bs_stack [context->bs_stack_top - 2],
            sizeof (context->bs_stack [0]));
    bs_stack_top = &context->bs_stack [context->bs_stack_top - 1];
    bs_stack_top->state = VIDEO_FMT_BS_STATE_PARSE_VALUE;
    bs_stack_top->value [0] = 0;
    bs_stack_top->substate = 0;
    return TRUE;
}

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
boolean video_fmt_bs_locate_name (
  video_fmt_bs_context_type      *context,
  video_fmt_bs_heap_header_type  **ref_header,
  uint8                          *ref_name_length,
  void                           **ref_body
) {
    video_fmt_bs_pos_type          *bs_stack_top;
    int                            i, j;
    video_fmt_bs_heap_header_type  *header;
    int                            name_length;

    /* Count the characters in the name. */
    bs_stack_top = &context->bs_stack [context->bs_stack_top - 1];
    for (name_length = 0;
         bs_stack_top->syntax_pos + name_length
             < bs_stack_top->syntax_end;
         ++name_length)
    {
        if (context->syntax [bs_stack_top->syntax_pos
                             + name_length] == '\'')
        {
            break;
        }
    }
    *ref_name_length = (uint8) name_length;

    /* Fail if the end of the syntax is reached or the name is too long. */
    if ((bs_stack_top->syntax_pos + (int)name_length
         >= bs_stack_top->syntax_end)
        || (name_length > 200))
    {
        return FALSE;
    }

    /* Walk the heap until the name indicated in the syntax is found. */
    for (i = 0; i < context->var_heap_end;)
    {
        /* Only check entries with names having the same length. */
        header = (video_fmt_bs_heap_header_type *)((void*)&context->var_heap [i]);
        if (header->name_length == name_length)
        {
            /* Compare name characters. */
            for (j = 0; j < name_length; ++j)
            {
                if (context->syntax [bs_stack_top->syntax_pos + j]
                    != header->name [j])
                {
                    break;
                }
            }

            /* If the names match, set pointers to entry header and body, and
            ** return.
            */
            if (j >= name_length)
            {
                *ref_header = header;
                *ref_body = (void *) &context->var_heap
                    [i + (sizeof (*header) + 3) / 4 + (name_length + 3) / 4];
                return TRUE;
            }
        }
        i += header->size;
    }
    return FALSE;
}

