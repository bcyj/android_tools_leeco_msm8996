#ifndef COMDEF_H
#define COMDEF_H
/*===========================================================================

                   S T A N D A R D    D E C L A R A T I O N S

DESCRIPTION
  This header file contains general types and macros that are of use
  to all modules.  The values or definitions are dependent on the specified
  target.  T_WINNT specifies Windows NT based targets, otherwise the
  default is for ARM targets.

       T_WINNT  Software is hosted on an NT platforn, triggers macro and
                type definitions, unlike definition above which triggers
                actual OS calls

DEFINED TYPES

       Name      Definition
       -------   --------------------------------------------------------
       byte      8  bit unsigned value
       word      16 bit unsigned value
       dword     32 bit unsigned value

       uint1     byte
       uint2     word
       uint4     dword

       uint8     8  bit unsigned value
       uint16    16 bit unsigned value
       uint32    32 bit unsigned value
       uint64    64 bit unsigned value

       int8      8  bit signed value
       int16     16 bit signed value
       int32     32 bit signed value
       int 64    64 bit signed value

       sint31    32 bit signed value
       sint15    16 bit signed value
       sint7     8  bit signed value

       int1      8  bit signed value
       int2      16 bit signed value
       int4      32 bit signed value

       boolean   8 bit boolean value

DEFINED CONSTANTS

       Name      Definition
       -------   --------------------------------------------------------
       TRUE      Asserted boolean condition (Logical 1)
       FALSE     Deasserted boolean condition (Logical 0)

       ON        Asserted condition
       OFF       Deasserted condition

       NULL      Pointer to nothing

       PACKED    Used to indicate structures which should use packed
                 alignment

       INLINE    Used to inline functions for compilers which support this

  -----------------------------------------------------------------------------
  Copyright (c) 1990, 1992-1994, 1997-2007 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
===========================================================================*/


/*===========================================================================

                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$Header: //linux/pkgs/proprietary/oncrpc/main/source/inc/comdef.h#4 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/09/06   bfc     Added Pthe OST_ALIGN macro.
09/22/06    th     Added FIQLOCK_SAV/FIQFREE_SAV macros for Windows Mobile
05/10/06   taw     Added FIQLOCK_SAV/FIQFREE_SAV macros.
01/25/06   sam     Added MSM_INTLOCK/MSM_INTFREE macros.
07/13/05   adm     Merged in changes related to supporting BootLoader compilation
                   for L4 builds
07/06/05   ptm     Merge to 6550 for L4.
06/30/05   adm     Merging in 6800's later changes and made some fixes.
06/29/05   adm     Merged in 6800 l4 related changes
02/28/04   jcw     Removed TASKLOCK/TASKFREE definition to INTLOCK/INTFREE for L4.
02/14/04   hcg     Add TASKLOCK/TASKFREE definition to INTLOCK/INTFREE for L4.
08/25/04   jac     suppress lint error 413 for FPOS macro
11/19/03    gr     Added multiple definition protection for the ARR_SIZE
                   macro.
10/18/03    gr     Moved TASKLOCK/TASKFREE into this file from rex.h. DMSS
                   expects to pick these up just by including comdef.h.
10/17/03    gr     Made inclusion of target.h unconditional (TG is now
                   deprecated).
10/16/03    gr     Defined LOCAL away unconditionally. DMSS doesn't build with
                   LOCAL set to static.
01/07/04   ajn     Re-integrated 11/14/03 change
11/20/03   ak      ARR_SIZE conflict with BREW files fixed
11/14/03   ajn     Removed #include "rex.h" to avoid circular include issue.
                   Instead, prototypes for rex_int_lock()/rex_int_free()
                   have been explicitly included for INTLOCK/INTFREE macros.
06/11/01    gr     Added MOD_BY_POWER_OF_TWO macro.
04/12/01    sq     Added inpdw and outpdw macros
           vas     Added solaris support
04/06/01    gr     Removed the definitions of abs and labs.
01/25/01   day     Merged from MSM5105_COMMON.00.00.05.
             Added include for stdlib.h within WINNT ifdef
       Added support for ARM_ASM
       Added support SWIG preprocessor.
09/09/99   tac     Merged in a bunch of panther changes.
08/11/99   jct     Corrected conditional compilation around labs to check for
                   labs instead of abs
06/30/99   jct     Added PC_EMULATOR capability for INTLOCK and INTLOCKSAV
06/15/99   jct     Cleanup to make work in Windows NT environment.  Addition
                   of int(n) types where n is 8,16,32,64 and addition of
                   unit64.  Removal of OS comments and defines as this is replaced
                   by T_WINNT.  Conditional inclusion of MAX, MIN, labs, abs,
                   NULL definitions.  Removal of volatile from packed definition
                   stdlib.h included for Win32 based targets since this replaces several
                   of the define here.  Changed out uts calls to be comet calls.
                   Added a defines for Windows NT targets to define away symbols
                   no longer supported
04/09/99    ms     Lint cleanup.
04/01/99    ms     Lint cleanup.
02/17/99    ms     Parenthesized abs and labs.
                   Defined __packed to volatile to exploit lint.
12/16/98   jct     Removed 80186 support
12/01/98    ms     Removed definition of bsp_io_base.
11/20/98    ms     Typecast to volatile pointers in inp,outp,inpw and outpw.
10/10/98   jct     Honing of OS and T_ definitions, added T_WINNT as a target
                   which triggers something different then which OS since
                   we may be running REX on an NT platform, but strictly
                   speaking, the application software is using REX, not NT
                   as the OS.  Changed ROM for WINNT/ARM targets to be nothing
                   instead of const
09/09/98   jct     Updates to merge ARM support with 186 support, added some
                   new type aliases, added some segmented architecture macros,
                   updated comments, removed unused code, updated declaration
                   of inp/outp to have cdecl
08/01/98   jct     Updates to support ARM processor
03/25/96   jah     Moved SET_VECT to 80186.h
02/06/96   fkm     Put ()s around Macros to Make Lint Happy
01/12/95   jah     Updated dependency in MACRO INTLOCK_SAV / INTFREE_SAV.
12/20/94   jah     Corrected typo in ARR_SIZE() macro comments
03/11/94   jah     Added #ifdef _lint of NULL as 0 for near/far lint problem.
06/11/93   jah     Changed FPOS() to cast the pointer to 'near' to avoid the
                   compiler complaints about lost segments.
01/28/93   twp     Added ARR_SIZE macro to return number of array elements.
07/17/92   jah     Changed int1 from 'char' to 'signed char'
06/10/92   jah     Added WORD_LO/HI, INC_SAT
05/19/92   jah     Added header comments for macros & made them lint-friendly
05/04/92   jah     Added define for ROM
03/02/92   jah     Added in/outp prototypes, INT* Macro comments w/lint
02/20/92   arh     Added LOCAL macro and ifdef/endif around file

===========================================================================*/


/*===========================================================================

                            Data Declarations

===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "target.h"
#include "armasm.h"

#if defined FEATURE_L4  && !defined FEATURE_L4_KERNEL && \
    !defined BUILD_BOOT_CHAIN && !defined BUILD_BOOT_CHAIN_OEMSBL
  #ifndef _ARM_ASM_
    #ifdef FEATURE_QUARTZ_135
      #include <l4/cust/msm_syscall.h>
    #else /* FEATURE_QUARTZ_135 */
     #include <msm/msm_syscall.h>
    #endif /* FEATURE_QUARTZ_135 */
  #endif /* _ARM_ASM_ */
#endif /* FEATURE_L4 && !FEATURE_L4_KERNEL &&
          !FEATURE_L4_KERNEL && !BUILD_BOOT_CHAIN_OEMSBL */

/* For NT apps we want to use the Win32 definitions and/or those
** supplied by the Win32 compiler for things like NULL, MAX, MIN
** abs, labs, etc.
*/
#ifdef T_WINNT
   #ifndef WIN32
      #define WIN32
   #endif
   #include <stdlib.h>
#endif

#include <stdint.h>

/* ------------------------------------------------------------------------
** Constants
** ------------------------------------------------------------------------ */

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define TRUE   1   /* Boolean true value. */
#define FALSE  0   /* Boolean false value. */

#define  ON   1    /* On value. */
#define  OFF  0    /* Off value. */

#ifdef _lint
  #define NULL 0
#endif

#ifndef NULL
  #define NULL  0
#endif

/* -----------------------------------------------------------------------
** Standard Types
** ----------------------------------------------------------------------- */

/* The following definitions are the same accross platforms.  This first
** group are the sanctioned types.
*/
#ifndef _ARM_ASM_
typedef  uint8_t            boolean;     /* Boolean value type. */

typedef  uint32_t           uint32;      /* Unsigned 32 bit value */
typedef  uint16_t           uint16;      /* Unsigned 16 bit value */
typedef  uint8_t            uint8;       /* Unsigned 8  bit value */

typedef  int32_t            int32;       /* Signed 32 bit value */
typedef  int16_t            int16;       /* Signed 16 bit value */
typedef  int8_t             int8;        /* Signed 8  bit value */

/* This group are the deprecated types.  Their use should be
** discontinued and new code should use the types above
*/
typedef  uint8_t           byte;         /* Unsigned 8  bit value type. */
typedef  uint16_t          word;         /* Unsinged 16 bit value type. */
typedef  uint32_t          dword;        /* Unsigned 32 bit value type. */

typedef  uint8_t           uint1;        /* Unsigned 8  bit value type. */
typedef  uint16_t          uint2;        /* Unsigned 16 bit value type. */
typedef  uint32_t          uint4;        /* Unsigned 32 bit value type. */

typedef  int8_t            int1;         /* Signed 8  bit value type. */
typedef  int16_t           int2;         /* Signed 16 bit value type. */
typedef  int32_t           int4;         /* Signed 32 bit value type. */

typedef  int32_t           sint31;       /* Signed 32 bit value */
typedef  int16_t           sint15;       /* Signed 16 bit value */
typedef  int8_t            sint7;        /* Signed 8  bit value */

typedef  uint16_t          Word16;      /* 16 bit "register"   */

#ifdef FEATURE_WINCE
  #define WCE_MSG  RETAILMSG
  #define WCE_MSG6 RETAILMSG
  #define WCE_MSGT RETAILMSG
  #define WCE_MSG_SDBG RETAILMSG
#endif
/************************** The PACK() macro *****************************
  This block sets up the semantics for PACK() macro based upon
  upon the target compiler. PACK() is necessary to ensure portability of C
  variable/struct/union packing across many platforms.  For example, ARM
  compilers require the following:
    typedef __packed struct { ... } foo_t;

  But GCC requires this to achieve the same effect:
    typedef struct __attribute((__packed__)) struct { ... } foo_t;

  And, of course, Microsoft VC++ requires an alignment #pragma prologue and
  epilogue. To satisfy all three, the following form is recommended:

    #ifdef _WIN32
		#pragma pack(push,1) // Save previous, and turn on 1 byte alignment
    #endif

    typedef PACK(struct)
    {
      ...
    } such_and_such_t;

    typedef PACK(struct)
    {
      ...
    } this_and_that_t;

    typedef PACK(struct)
    {
      PACK(struct)
      {
        ...
      } hdr;
      PACK(union)
      {
        such_and_such_t sas;
        this_and_that_t tat;
      } payload;
    } cmd_t;

*********************** BEGIN PACK() Definition ***************************/
#if defined __GNUC__
  #define PACK(x)       x __attribute__((__packed__))
#elif defined __arm
  #define PACK(x)       __packed x
#else
  #error No PACK() macro defined for this compiler
#endif
/********************** END PACK() Definition *****************************/
/* ---------------------------------------------------------------------
** Compiler Keyword Macros
** --------------------------------------------------------------------- */
#if (! defined T_WINNT) && (! defined TARGET_OS_SOLARIS)
#ifndef SWIG  /* The SWIG preprocessor gets confused by these */
  /* Non WinNT Targets
  */
  typedef  int64_t            int64;       /* Signed 64 bit value */
  typedef  uint64_t           uint64;      /* Unsigned 64 bit value */

#if defined(__ARMCC_VERSION) 
  #define PACKED __packed
  #define PACKED_POST
  #define ALIGN(__value) __align(__value)
  #define POST_ALIGN(__value)
  #define INLINE __inline
  #define inline __inline
#else  /* __GNUC__ */
  #define PACKED 
  #ifndef FEATURE_WINCE
    #define ALIGN(__value) __attribute__((__aligned__(__value)))
    #define POST_ALIGN(__value)
    #define PACKED_POST    __attribute__((__packed__))
  #else /* FEATURE_WINCE */
    #ifdef WINCE_TODO
      /*----------------------------------------------------------------------
        Should we be using something closer to the T_WINNT case below?
      ----------------------------------------------------------------------*/
    #endif /* WINCE_TODO */
    #define ALIGN(__value)
    #define POST_ALIGN(__value)
    #define PACKED_POST
    #define INLINE 
  #endif /* FEATURE_WINCE */
    

  #ifndef INLINE
    #define INLINE inline
  #endif
#endif /* defined (__GNUC__) */

  #define CDECL
#endif /* SWIG */
  #define far
  #define near
  #define _far
  #define _near
//  #define _cdecl
  #define cdecl
  #define _pascal
  #define _interrupt

#else /* T_WINNT || TARGET_OS_SOLARIS */

  /* WINNT or SOLARIS based targets
  */
#if (defined __GNUC__) || (defined TARGET_OS_SOLARIS)
  typedef long long           int64;
  typedef unsigned long long  uint64;
#else
  typedef  __int64            int64;       /* Signed 64 bit value */
  typedef  unsigned __int64   uint64;      /* Unsigned 64 bit value */
#endif
  #define PACKED
  #define PACKED_POST
  #define ALIGN(__value)
  #define POST_ALIGN(__value)
  #ifndef INLINE

  /* INLINE is defined to __inline because WINNT targets work fine with it
   * and defining it to nothing does not work because the inline function
   * definition is then multiply defined.  Solaris may need a different value.
   */
  #define INLINE __inline
  #endif
  #ifndef CDECL
     #define CDECL __cdecl
  #endif
  #define _pascal
  #define _far
  #define far
  #define near
  #define _near
  #define cdecl
  #define _cdecl
  #define _interrupt
  #define __packed
  #define _fmemcpy memcpy

#endif /* T_WINNT */
#endif // #ifndef _ARM_ASM_

/* ----------------------------------------------------------------------
** Lint does not understand __packed, so we define it away here.  In the
** past we did this:
**   This helps us catch non-packed pointers accessing packed structures,
**   for example, (although lint thinks it is catching non-volatile pointers
**   accessing volatile structures).
**   This does assume that volatile is not being used with __packed anywhere
**   because that would make Lint see volatile volatile (grrr).
** but found it to be more trouble than it was worth as it would emit bogus
** errors
** ---------------------------------------------------------------------- */
#ifdef _lint
  #define __packed
#endif

/* ----------------------------------------------------------------------
**                          STANDARD MACROS
** ---------------------------------------------------------------------- */

#ifndef SWIG /* these confuse the SWIG preprocessor and aren't needed for it */


/*===========================================================================

MACRO MEM_B
MACRO MEM_W

DESCRIPTION
  Take an address and dereference it as a byte/word, allowing access to an
  arbitrary memory byte/word.

PARAMETERS
  x     address to be dereferenced

DEPENDENCIES
  None

RETURN VALUE
  MEM_B byte at the given address
  MEM_W word at the given address

SIDE EFFECTS
  None

===========================================================================*/

#define  MEM_B( x )  ( *( (byte *) (x) ) )
#define  MEM_W( x )  ( *( (word *) (x) ) )



/*===========================================================================

MACRO MAX
MACRO MIN

DESCRIPTION
  Evaluate the maximum/minimum of 2 specified arguments.

PARAMETERS
  x     parameter to compare to 'y'
  y     parameter to compare to 'x'

DEPENDENCIES
  'x' and 'y' are referenced multiple times, and should remain the same
  value each time they are evaluated.

RETURN VALUE
  MAX   greater of 'x' and 'y'
  MIN   lesser of 'x' and 'y'

SIDE EFFECTS
  None

===========================================================================*/
#ifndef MAX
   #define  MAX( x, y ) ( ((x) > (y)) ? (x) : (y) )
#endif

#ifndef MIN
   #define  MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
#endif



/*===========================================================================

MACRO FPOS

DESCRIPTION
  This macro computes the offset, in bytes, of a specified field
  of a specified structure or union type.

PARAMETERS
  type          type of the structure or union
  field         field in the structure or union to get the offset of

DEPENDENCIES
  None

RETURN VALUE
  The byte offset of the 'field' in the structure or union of type 'type'.

SIDE EFFECTS
  The lint error "Warning 545: Suspicious use of &" is suppressed within
  this macro.  This is due to the desire to have lint not complain when
  'field' is an array.

===========================================================================*/

#define FPOS offsetof



/*===========================================================================

MACRO FSIZ

DESCRIPTION
  This macro computes the size, in bytes, of a specified field
  of a specified structure or union type.

PARAMETERS
  type          type of the structure or union
  field         field in the structure or union to get the size of

DEPENDENCIES
  None

RETURN VALUE
  size in bytes of the 'field' in a structure or union of type 'type'

SIDE EFFECTS
  None

===========================================================================*/

#define FSIZ( type, field ) sizeof( ((type *) 0)->field )



/*===========================================================================

MACRO FLIPW

DESCRIPTION
  Takes a 2 byte array, with the most significant byte first, followed
  by the least significant byte, and converts the quantity into a word
  with Intel byte order (LSB first)

PARAMETERS
  ray   array of 2 bytes to be converted to a word

DEPENDENCIES
  None

RETURN VALUE
  word in Intel byte order comprised of the 2 bytes of ray.

SIDE EFFECTS
  None

===========================================================================*/

#define  FLIPW( ray ) ( (((word) (ray)[0]) * 256) + (ray)[1] )



/*===========================================================================

MACRO FLOPW

DESCRIPTION
  Reverses bytes of word value and writes them to a 2 byte array.
  Effectively the reverse of the FLIPW macro.

PARAMETERS
  ray   array to receive the 2 bytes from 'val'
  val   word to break into 2 bytes and put into 'ray'

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
#define  FLOPW( ray, val ) \
  (ray)[0] = ((val) / 256); \
  (ray)[1] = ((val) & 0xFF)



/*===========================================================================

MACRO B_PTR
MACRO W_PTR

DESCRIPTION
  Casts the address of a specified variable as a pointer to byte/word,
  allowing byte/word-wise access, e.g.
  W_PTR ( xyz )[ 2 ] = 0x1234;  -or-    B_PTR ( xyz )[ 2 ] = 0xFF;

PARAMETERS
  var   the datum to get a word pointer to

DEPENDENCIES
  None

RETURN VALUE
  Word pointer to var

SIDE EFFECTS
  None

===========================================================================*/

#define  B_PTR( var )  ( (byte *) (void *) &(var) )
#define  W_PTR( var )  ( (word *) (void *) &(var) )
#define  D_PTR( var )  ( (dword *) (void *) &(var) )


/*===========================================================================

MACRO WORD_LO
MACRO WORD_HI

DESCRIPTION
  Take a word and extract the least-significant or most-significant byte.

PARAMETERS
  xxx   word to extract the Low/High from

DEPENDENCIES
  None

RETURN VALUE
  WORD_LO       Least significant byte of xxx.
  WORD_HI       Most significant byte of xxx.

SIDE EFFECTS
  None

===========================================================================*/

#define  WORD_LO(xxx)  ((byte) ((word)(xxx) & 255))
#define  WORD_HI(xxx) \
  /*lint -e572 */  ((byte) ((word)(xxx) >> 8))  /*lint +e572 */


/*===========================================================================

MACRO RND8

DESCRIPTION
  RND8 rounds a number up to the nearest multiple of 8.

PARAMETERS
  x     Number to be rounded up

DEPENDENCIES
  None

RETURN VALUE
  x rounded up to the nearest multiple of 8.

SIDE EFFECTS
  None

===========================================================================*/

#define RND8( x )       ((((x) + 7) / 8 ) * 8 )

/* -----------------------------------------------------------------------
** Temporary inclusion of rex.h because there is code in AMSS that expects
** to pick up Rex definitions by just including comdef.h.
** ----------------------------------------------------------------------- */
//#include "rex.h"

/*===========================================================================

MACRO INTLOCK / INTFREE

DESCRIPTION
  INTLOCK Saves current interrupt state on stack then informs the kernel
  not to process interrupts in this thread. One interrupt may still occur, 
  but it will not be processed until either INTFREE is called or a context 
  switch to another thread which has interrupts enabled is made.

  INTFREE Restores previous interrupt state from stack. Used in conjunction
  with INTLOCK.

  INTLOCK/INTFREE may be nested, in which case inner INTLOCK/INTFREE
  pairs will have no effect.

PARAMETERS
  None

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  INTLOCK stops interrupt processing.
  INTFREE restores the interrupt mask saved previously.

===========================================================================*/
#if defined PC_EMULATOR_H && !defined T_REXNT

   #define PC_EMULATOR_INTLOCK
   #include PC_EMULATOR_H
   #undef PC_EMULATOR_INTLOCK

#elif defined T_WINNT || defined FEATURE_ANDROID

   #define INTLOCK()
   #define INTFREE()

#elif defined FEATURE_WINCE_OAL
  #define INTLOCK()  INTERRUPTS_OFF()
  #define INTFREE()  INTERRUPTS_ON()
#elif !defined _ARM_ASM_ && (!defined FEATURE_L4 || defined FEATURE_L4_KERNEL)

  #ifdef __cplusplus
    extern "C" {
  #endif
      extern dword rex_int_lock( void );
      extern dword rex_int_free( void );
#ifdef FEATURE_WINCE
      extern int rex_ints_are_locked( void );
#endif

  #ifdef __cplusplus
    }
  #endif

  #define  INTLOCK( )  { dword sav = rex_int_lock();
  #define  INTFREE( )  if(!sav) (void)rex_int_free();}

#endif


/*===========================================================================

MACRO MSM_INTLOCK / MSM_INTFREE

DESCRIPTION
  MSM_INTLOCK Saves current interrupt state on stack then locks interrupts. 
  No interrupts will be taken until either MSM_INTFREE is called or a context 
  switch to another thread which has interrupts enabled is made. This
  macro is slower than INTLOCK and should only be used when interrupt 
  locking is needed to ensure timing. To simply prevent context switching, 
  use the INTLOCK/INTFREE macros above.

  MSM_INTFREE Restores previous interrupt state from stack. Used in conjunction
  with MSM_INTLOCK.

  MSM_INTLOCK/MSM_INTFREE may be nested, in which case inner 
  MSM_INTLOCK/MSM_INTFREE pairs will have no effect. It may also be nested
  inside or outside of INTLOCK/INTFREE.

PARAMETERS
  None

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  MSM_INTLOCK prevents any interrupts from occurring until MSM_INTFREE is
    called or a context switch to a thread with interrupts enabled is made.
  MSM_INTFREE restores the interrupt mask saved previously.

===========================================================================*/
#ifndef _ARM_ASM
#if defined FEATURE_L4
  #ifdef __cplusplus
    extern "C" {
  #endif
      extern dword rex_msm_int_lock( void );
      extern void rex_msm_int_free( dword set );
  #ifdef __cplusplus
    }
  #endif

  #define MSM_INTLOCK()  { dword sav = rex_msm_int_lock();
  #define MSM_INTFREE()  rex_msm_int_free( sav ); }

#else

  #define MSM_INTLOCK() INTLOCK()
  #define MSM_INTFREE() INTFREE()

#endif
#endif

/*===========================================================================

MACRO FIQLOCK / FIQFREE

DESCRIPTION
  FIQLOCK Saves current FIQ state on stack then disables FIQ interrupts.
  Used in conjunction with FIQFREE.

  FIQFREE Restores previous FIQ interrupt state from stack.  Used in 
  conjunction with FIQLOCK.

PARAMETERS
  None

DEPENDENCIES
  This is currently only available in L4 based builds.

RETURN VALUE
  None

SIDE EFFECTS
  FIQLOCK turns off FIQ interrupts.
  FIQFREE restores the FIQ interrupt mask saved previously.

===========================================================================*/
#if defined FEATURE_L4 && !defined _ARM_ASM_

  #ifdef __cplusplus
    extern "C" {
  #endif
      extern dword rex_fiq_op( dword lock );
  #ifdef __cplusplus
    }
  #endif

  #define  FIQLOCK()  { dword sav = rex_fiq_op(1);
  #define  FIQFREE()  if(!(sav & PSR_Fiq_Mask)) (void)rex_fiq_op(0);}
    
#else

  #define FIQLOCK()
  #define FIQFREE()

#endif

/*===========================================================================

MACRO INTLOCK_SAV / INTFREE_SAV

DESCRIPTION
  INTLOCK_SAV Saves current interrupt state in specified variable sav_var
  then disables interrupts.  Used in conjunction with INTFREE_SAV.

  INTFREE_SAV Restores previous interrupt state from specified variable
  sav_var.  Used in conjunction with INTLOCK_SAV.

PARAMETERS
  sav_var       Current flags register, including interrupt status

DEPENDENCIES
  None.

RETURN VALUE
  None

SIDE EFFECTS
  INTLOCK_SAV turn off interrupts
  INTFREE_SAV restore the interrupt mask saved previously

===========================================================================*/


#ifdef _lint    /* get lint to 'know' the parameter is accessed */

  #define  INTLOCK_SAV(sav_var)   (sav_var = 1)
  #define  INTFREE_SAV(sav_var)   (sav_var = sav_var + 1)

#else

   #if defined PC_EMULATOR_H && !defined T_REXNT

      #define PC_EMULATOR_INTLOCKSAV
      #include PC_EMULATOR_H
      #undef  PC_EMULATOR_INTLOCKSAV

   #elif defined T_WINNT

     #define  INTLOCK_SAV( sav_var )
     #define  INTFREE_SAV( sav_var )

#elif !defined _ARM_ASM_ && (!defined FEATURE_L4 || defined FEATURE_L4_KERNEL)

     #define  INTLOCK_SAV( sav_var )  sav_var = rex_int_lock()
     #define  INTFREE_SAV( sav_var )  if(!sav_var) rex_int_free()

   #endif

#endif /* END if _lint */

/*===========================================================================

MACRO FIQLOCK_SAV / FIQFREE_SAV

DESCRIPTION
  FIQLOCK Saves current FIQ state in specified variable sav_var 
  then disables FIQ interrupts.  Used in conjunction with FIQFREE_SAV.

  FIQFREE_SAV Restores previous FIQ interrupt state from specified variable
  sav_var.  Used in conjunction with FIQLOCK_SAV.

PARAMETERS
  sav_var       Current flags register, including interrupt status

DEPENDENCIES
  This is currently only available in L4 based builds.

RETURN VALUE
  None

SIDE EFFECTS
  FIQLOCK_SAV turns off FIQ interrupts.
  FIQFREE_SAV restores the FIQ interrupt mask saved previously.

===========================================================================*/
#ifdef _lint    /* get lint to 'know' the parameter is accessed */

  #define  FIQLOCK_SAV(sav_var)   (sav_var = 1)
  #define  FIQFREE_SAV(sav_var)   (sav_var = sav_var + 1)

#else

  #if defined FEATURE_L4 && !defined _ARM_ASM_
  
    #ifdef __cplusplus
      extern "C" {
    #endif
        extern dword rex_fiq_op( dword lock );
    #ifdef __cplusplus
      }
    #endif
  
    #define  FIQLOCK_SAV(sav_var)  (sav_var = rex_fiq_op(1))
    #define  FIQFREE_SAV(sav_var)  if(!(sav_var & PSR_Fiq_Mask)) \
                                      (void)rex_fiq_op(0)

  #elif defined FEATURE_WINCE

    #define  FIQLOCK_SAV(sav_var)  (sav_var = FIQLockSave())
    #define  FIQFREE_SAV(sav_var)  if(!(sav_var & 0x40)) (void)FIQLockRestore()

  #else
  
    #define FIQLOCK_SAV()
    #define FIQFREE_SAV()
  
  #endif
  
#endif


/*===========================================================================
MACRO TASKLOCK/TASKFREE/TASKLOCK_SAV/TASKFREE_SAV

DESCRIPTION
  TASKLOCK prevents context switching from the point at which it is invoked
  until a matching TASKFREE is invoked.  While task locked, the current task
  is guaranteed to not be replaced by a different task, even if the other
  task has a higher priority.  However, TASKLOCK does not block interrupts
  from occurring.  An ISR will pre-empt the task, but even if the ISR causes
  a higher priority task to become ready, control will still resume at the
  interrupted task.

  Note that a rex-wait() while a TASKLOCK is in effect will not work as
  normal, as the calling task will not be suspended.

  Also, TASKLOCK/TASKFREE have no effect if called from within an ISR.

  TASKFREE re-enables context switching.

  TASKLOCK_SAV and TASKFREE_SAV are similar to TASKLOCK and TASKFREE
  respectively, except that the TASKFREE_SAV that matches a TASKLOCK_SAV
  does not have to be at the same static scope within a function. This
  makes it easier, for instance, to enable task scheduling and return from
  the middle of a function.

===========================================================================*/
#ifdef T_WINNT
#ifdef __cplusplus
   extern "C" {
#endif
   unsigned char rexnt_is_rex_thread( void );
#ifdef __cplusplus
   }
#endif
   /* tasks must not be locked by non-REX threads */
#define TASKLOCK( ) {\
   unsigned char REXNT_IS_REX_THREAD=rexnt_is_rex_thread();\
   if ( REXNT_IS_REX_THREAD ) rex_task_lock( ); {

#define TASKFREE( ) \
   } if ( REXNT_IS_REX_THREAD ) rex_task_free( ); }

#else

#define TASKLOCK( ) { rex_task_lock( );

#define TASKFREE( ) rex_task_free( ); }

#define TASKLOCK_SAV( ) rex_task_lock( )

#define TASKFREE_SAV( ) rex_task_free( )

#endif /* T_WINNT */

/*===========================================================================

MACRO UPCASE

DESCRIPTION
  Convert a character to uppercase, the character does not have to
  be printable or a letter.

PARAMETERS
  c             Character to be converted

DEPENDENCIES
  'c' is referenced multiple times, and should remain the same value
  each time it is evaluated.

RETURN VALUE
  Uppercase equivalent of the character parameter

SIDE EFFECTS
  None

===========================================================================*/

#define  UPCASE( c ) ( ((c) >= 'a' && (c) <= 'z') ? ((c) - 0x20) : (c) )


/*===========================================================================

MACRO DECCHK
MACRO HEXCHK

DESCRIPTION
  These character attribute macros are similar to the standard 'C' macros
  (isdec and ishex), but do not rely on the character attributes table used
  by Microsoft 'C'.

PARAMETERS
  c             Character to be examined

DEPENDENCIES
  None

RETURN VALUE
  DECCHK        True if the character is a decimal digit, else False
  HEXCHK        True if the chacters is a hexidecimal digit, else False

SIDE EFFECTS
  None

===========================================================================*/

#define  DECCHK( c ) ((c) >= '0' && (c) <= '9')

#define  HEXCHK( c ) ( ((c) >= '0' && (c) <= '9') ||\
                       ((c) >= 'A' && (c) <= 'F') ||\
                       ((c) >= 'a' && (c) <= 'f') )


/*===========================================================================

MACRO INC_SAT

DESCRIPTION
  Increment a value, but saturate it at its maximum positive value, do not
  let it wrap back to 0 (unsigned) or negative (signed).

PARAMETERS
  val           value to be incremented with saturation

DEPENDENCIES
  None

RETURN VALUE
  val

SIDE EFFECTS
  val is updated to the new value

===========================================================================*/

#define  INC_SAT( val )  (val = ((val)+1 > (val)) ? (val)+1 : (val))


/*===========================================================================

MACRO ARR_SIZE

DESCRIPTION
  Return the number of elements in an array.

PARAMETERS
  a             array name

DEPENDENCIES
  None

RETURN VALUE
  Number of elements in array a

SIDE EFFECTS
  None.

===========================================================================*/
#if !defined(ARR_SIZE)
#define  ARR_SIZE( a )  ( sizeof( (a) ) / sizeof( (a[0]) ) )
#endif
/*===========================================================================
                     MACRO MOD_BY_POWER_OF_TWO

     Will calculate x % y, where x is a non-negative integer and
     y is a power of 2 from 2^0..2^32.

     Will work for 2^0, 2^1, 2^2, 2^3, 2^4, ... 2^32
      ie.            1,   2,   4,   8,  16, ... 4294967296
===========================================================================*/

#define MOD_BY_POWER_OF_TWO( val, mod_by ) \
           ( (dword)(val) & (dword)((mod_by)-1) )

/* -------------------------------------------------------------------------
**                       Debug Declarations
** ------------------------------------------------------------------------- */

/* Define 'SHOW_STAT' in order to view static's as globals
** (e.g. cl /DSHOW_STAT foo.c) If 'SHOW_STAT' is not defined,
** it gets defined as 'static'
*/
#ifdef LOCAL
   #undef LOCAL
#endif

/* Permanently turn on SHOW_STAT since this has always been the case anyway
*/
#define SHOW_STAT

#ifdef SHOW_STAT
  #define LOCAL
#else
  #define LOCAL static
#endif

/* Provide a new macro that will hopefully be used correctly to conditionally
** turn static on/off
*/
#ifdef GLOBALIZE_STATIC
   #define STATIC
#else
   #define STATIC static
#endif


/*===========================================================================

                      FUNCTION DECLARATIONS

===========================================================================*/



/*===========================================================================

FUNCTION inp, outp, inpw, outpw, inpdw, outpdw

DESCRIPTION
  IN/OUT port macros for byte and word ports, typically inlined by compilers
  which support these routines

PARAMETERS
  inp(   xx_addr )
  inpw(  xx_addr )
  inpdw( xx_addr )
  outp(   xx_addr, xx_byte_val  )
  outpw(  xx_addr, xx_word_val  )
  outpdw( xx_addr, xx_dword_val )
      xx_addr      - Address of port to read or write (may be memory mapped)
      xx_byte_val  - 8 bit value to write
      xx_word_val  - 16 bit value to write
      xx_dword_val - 32 bit value to write

DEPENDENCIES
  None

RETURN VALUE
  inp/inpw/inpdw: the byte, word or dword read from the given address
  outp/outpw/outpdw: the byte, word or dword written to the given address

SIDE EFFECTS
  None.

===========================================================================*/

#ifdef PC_EMULATOR_H

  /* For PC emulation, include a header which defines inp/outp/inpw/outpw
  ** with the semantics above
  */
  #define PC_EMULATOR_IO
  #include PC_EMULATOR_H
  #undef  PC_EMULATOR_IO

#else

  /* ARM based targets use memory mapped i/o, so the inp/outp calls are
  ** macroized to access memory directly
  */

  #define inp(port)         (*((volatile byte *) (port)))
  #define inpw(port)        (*((volatile word *) (port)))
  #define inpdw(port)       (*((volatile dword *)(port)))

  #define outp(port, val)   (*((volatile byte *) (port)) = ((byte) (val)))
  #define outpw(port, val)  (*((volatile word *) (port)) = ((word) (val)))
  #define outpdw(port, val) (*((volatile dword *) (port)) = ((dword) (val)))

#endif


/*===========================================================================

FUNCTION enable, disable

DESCRIPTION
  Interrupt enable and disable routines.  Enable should cause the CPU to
  allow interrupts and disable should cause the CPU to disallow
  interrupts

PARAMETERS
  None

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None.

===========================================================================*/

#ifndef T_WINNT

  #if !defined _ARM_ASM_
    /* ARM has no such definition, so we provide one here to enable/disable
    ** interrupts
    */
    #define _disable() (void)rex_int_lock()
    #define _enable()  (void)rex_int_free()

  #endif
#endif

#endif /* SWIG */

#ifdef __cplusplus
}
#endif

#endif  /* COMDEF_H */
