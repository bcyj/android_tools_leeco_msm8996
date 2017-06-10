/*****************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef _ADSP_JPEGE_SKEL_H
#define _ADSP_JPEGE_SKEL_H
#include "adsp_jpege.h"
#include "remote.h"
#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdlib.h>
#include "AEEStdDef.h"

typedef struct _heap _heap;
struct _heap {
   _heap* pPrev;
   const char* loc;
   uint64 buf;
};

typedef struct allocator {
   _heap* pheap;
   byte* stack;
   byte* stackEnd;
   int nSize;
} allocator;

static __inline int _heap_alloc(_heap** ppa, const char* loc, int size, void** ppbuf) {
   _heap* pn = 0;
   pn = malloc(size + sizeof(_heap) - sizeof(uint64));
   if(pn != 0) {
      pn->pPrev = *ppa;
      pn->loc = loc;
      *ppa = pn;
      *ppbuf = (void*)&(pn->buf);
      return 0;
   } else {
      return -1;
   }
}

#define _ALIGN_SIZE(sz, al) (al != 0 ? ((uint32)(sz) % (al) == 0 ? (sz) : (sz) + (al - (((uint32)(sz)) % al))) : sz)

static __inline int allocator_alloc(allocator* me,
                                    const char* loc,
                                    int size,
                                    int al,
                                    void** ppbuf) {
   if(size < 0) {
      return -1;
   } else if (size == 0) {
      *ppbuf = 0;
      return 0;
   }
   if((_ALIGN_SIZE(me->stackEnd, al) + size) < me->stack + me->nSize) {
      *ppbuf = _ALIGN_SIZE(me->stackEnd, al);
      me->stackEnd = _ALIGN_SIZE(me->stackEnd, al) + size;
      return 0;
   } else {
      return _heap_alloc(&me->pheap, loc, size, ppbuf);
   }
}

static __inline void allocator_deinit(allocator* me) {
   _heap* pa = me->pheap;
   while(pa != 0) {
      _heap* pn = pa;
      const char* loc = pn->loc;
      (void)loc;
      pa = pn->pPrev;
      free(pn);
   }
}

static __inline void allocator_init(allocator* me, byte* stack, int stackSize) {
   me->stack =  stack;
   me->stackEnd =  stack + stackSize;
   me->nSize = stackSize;
   me->pheap = 0;
}


#endif // ALLOCATOR_H

#ifndef SLIM_H
#define SLIM_H

#include "AEEStdDef.h"

//a C data structure for the idl types that can be used to implement
//static and dynamic language bindings fairly efficiently.
//
//the goal is to have a minimal ROM and RAM footprint and without
//doing too many allocations.  A good way to package these things seemed
//like the module boundary, so all the idls within  one module can share
//all the type references.


#define PARAMETER_IN       0x0
#define PARAMETER_OUT      0x1
#define PARAMETER_INOUT    0x2
#define PARAMETER_ROUT     0x3
#define PARAMETER_INROUT   0x4

//the types that we get from idl
#define TYPE_OBJECT             0x0
#define TYPE_INTERFACE          0x1
#define TYPE_PRIMITIVE          0x2
#define TYPE_ENUM               0x3
#define TYPE_STRING             0x4
#define TYPE_WSTRING            0x5
#define TYPE_STRUCTURE          0x6
#define TYPE_UNION              0x7
#define TYPE_ARRAY              0x8
#define TYPE_SEQUENCE           0x9

//these require the pack/unpack to recurse
//so it's a hint to those languages that can optimize in cases where
//recursion isn't necessary.
#define TYPE_COMPLEX_STRUCTURE  (0x10 | TYPE_STRUCTURE)
#define TYPE_COMPLEX_UNION      (0x10 | TYPE_UNION)
#define TYPE_COMPLEX_ARRAY      (0x10 | TYPE_ARRAY)
#define TYPE_COMPLEX_SEQUENCE   (0x10 | TYPE_SEQUENCE)


typedef struct Type Type;

#define INHERIT_TYPE\
   int32 nativeSize;                /*in the simple case its the same as wire size and alignment*/\
   union {\
      struct {\
         const uint32      p1;\
         const uint32      p2;\
      } _cast;\
      struct {\
         AEEIID  iid;\
         uint32  bNotNil;\
      } object;\
      struct {\
         const Type  *arrayType;\
         int32       nItems;\
      } array;\
      struct {\
         const Type *seqType;\
         int32       nMaxLen;\
      } seqSimple; \
      struct {\
         uint32 bFloating;\
         uint32 bSigned;\
      } prim; \
      const SequenceType* seqComplex;\
      const UnionType  *unionType;\
      const StructType *structType;\
      int32          stringMaxLen;\
      boolean        bInterfaceNotNil;\
   } param;\
   uint8    type;\
   uint8    nativeAlignment\

typedef struct UnionType UnionType;
typedef struct StructType StructType;
typedef struct SequenceType SequenceType;
struct Type {
   INHERIT_TYPE;
};

struct SequenceType {
   const Type *         seqType;
   uint32               nMaxLen;
   uint32               inSize;
   uint32               routSizePrimIn;
   uint32               routSizePrimROut;
};

//byte offset from the start of the case values for
//this unions case value array.  it MUST be aligned
//at the alignment requrements for the descriptor
//
//if negative it means that the unions cases are
//simple enumerators, so the value read from the descriptor
//can be used directly to find the correct case
typedef union CaseValuePtr CaseValuePtr;
union CaseValuePtr {
   const uint8*   value8s;
   const uint16*  value16s;
   const uint32*  value32s;
   const uint64*  value64s;
};

//these are only used in complex cases
//so I pulled them out of the type definition as references to make
//the type smaller
struct UnionType {
   const Type           *descriptor;
   uint32               nCases;
   const CaseValuePtr   caseValues;
   const Type * const   *cases;
   int32                inSize;
   int32                routSizePrimIn;
   int32                routSizePrimROut;
   uint8                inAlignment;
   uint8                routAlignmentPrimIn;
   uint8                routAlignmentPrimROut;
   uint8                inCaseAlignment;
   uint8                routCaseAlignmentPrimIn;
   uint8                routCaseAlignmentPrimROut;
   uint8                nativeCaseAlignment;
   boolean              bDefaultCase;
};

struct StructType {
   uint32               nMembers;
   const Type * const   *members;
   int32                inSize;
   int32                routSizePrimIn;
   int32                routSizePrimROut;
   uint8                inAlignment;
   uint8                routAlignmentPrimIn;
   uint8                routAlignmentPrimROut;
};

typedef struct Parameter Parameter;
struct Parameter {
   INHERIT_TYPE;
   uint8    mode;
   boolean  bNotNil;
};

#define SLIM_SCALARS_IS_DYNAMIC(u) (((u) & 0x00ffffff) == 0x00ffffff)

typedef struct Method Method;
struct Method {
   uint32                      uScalars;            //no method index
   int32                       primInSize;
   int32                       primROutSize;
   int                         maxArgs;
   int                         numParams;
   const Parameter * const     *params;
   uint8                       primInAlignment;
   uint8                       primROutAlignment;
};

typedef struct Interface Interface;

struct Interface {
   int                            nMethods;
   const Method  * const          *methodArray;
   int                            nIIds;
   const AEEIID                   *iids;
   const uint16*                  methodStringArray;
   const uint16*                  methodStrings;
   const char*                    strings;
};


#endif //SLIM_H


#ifndef _ADSP_JPEGE_SLIM_H
#define _ADSP_JPEGE_SLIM_H
#include "remote.h"

#ifndef __QAIC_SLIM
#define __QAIC_SLIM(ff) ff
#endif
#ifndef __QAIC_SLIM_EXPORT
#define __QAIC_SLIM_EXPORT
#endif

static const Type types[9];
static const Type* const typeArrays[28] = {&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[4]),&(types[4]),&(types[4]),&(types[5]),&(types[5]),&(types[5]),&(types[5]),&(types[4]),&(types[4]),&(types[4]),&(types[4]),&(types[4]),&(types[7]),&(types[8]),&(types[4]),&(types[4]),&(types[4]),&(types[4]),&(types[4]),&(types[4]),&(types[4]),&(types[2]),&(types[1]),&(types[3])};
static const StructType structTypes[3] = {{0x13,&(typeArrays[0]),0x688,0x0,0x688,0x4,0x1,0x4},{0x2,&(typeArrays[26]),0x111,0x0,0x111,0x1,0x1,0x1},{0x7,&(typeArrays[19]),0x1c,0x0,0x1c,0x4,0x1,0x4}};
static const Type types[9] = {{0x111,{(const uint32)&(structTypes[1]),0}, 6,0x1},{0x11,{(const uint32)&(types[2]),(const uint32)0x11}, 8,0x1},{0x1,{0,0}, 2,0x1},{0x100,{(const uint32)&(types[2]),(const uint32)0x100}, 8,0x1},{0x4,{0,0}, 2,0x4},{0x80,{(const uint32)&(types[6]),(const uint32)0x40}, 8,0x2},{0x2,{0,0}, 2,0x2},{0x4,{0,0}, 3,0x4},{0x1c,{(const uint32)&(structTypes[2]),0}, 6,0x4}};
static const Parameter parameters[5] = {{0x688,{(const uint32)&(structTypes[0]),0}, 6,0x4,0,0},{0x8,{(const uint32)&(types[2]),(const uint32)0x0}, 9,0x4,0,0},{0x4,{0,0}, 2,0x4,3,0},{0x8,{(const uint32)&(types[2]),(const uint32)0x0}, 9,0x4,3,0},{0x4,{0,0}, 2,0x4,0,0}};
static const Parameter* const parameterArrays[8] = {(&(parameters[0])),(&(parameters[1])),(&(parameters[1])),(&(parameters[1])),(&(parameters[1])),(&(parameters[2])),(&(parameters[3])),(&(parameters[4]))};
static const Method methods[2] = {{REMOTE_SCALARS_MAKEX(0,0,0x0,0x0,0x0,0x0),0x0,0x0,0,0,0,0x0,0x0},{REMOTE_SCALARS_MAKEX(0,0,0x5,0x2,0x0,0x0),0x6a0,0x4,14,8,(&(parameterArrays[0])),0x4,0x4}};
static const Method* const methodArrays[4] = {&(methods[0]),&(methods[0]),&(methods[1]),&(methods[0])};
static const char strings[477] = "output_start_mcu_index\0output_buffer_length\0base_restart_marker\0chroma_ac_huff_tbl\0chroma_dc_huff_tbl\0output_buffer_ptr\0restart_interval\0luma_ac_huff_tbl\0luma_dc_huff_tbl\0output_height\0fastrpc_start\0output_width\0color_format\0input_stride\0input_height\0thread_flag\0output_size\0plane_3_ptr\0plane_2_ptr\0plane_1_ptr\0plane_0_ptr\0crop_height\0output_MCUs\0input_width\0crop_width\0q6_process\0scale_cfg\0v_offset\0h_offset\0rotation\0deinit\0enable\0values\0qtbl_3\0qtbl_2\0qtbl_1\0qtbl_0\0bits\0args\0";
static const uint16 methodStrings[46] = {370,472,154,83,137,64,409,120,44,460,453,446,439,347,238,225,335,23,212,381,0,467,432,467,432,467,432,467,432,359,323,400,391,199,171,425,311,299,287,275,263,102,251,418,420,185};
static const uint16 methodStringsArrays[4] = {45,44,0,43};
__QAIC_SLIM_EXPORT const Interface __QAIC_SLIM(adsp_jpege_slim) = {4,&(methodArrays[0]),0,0,&(methodStringsArrays [0]),methodStrings,strings};
#endif //_ADSP_JPEGE_SLIM_H
#ifdef __qdsp6__
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wuninitialized"
#endif
#endif

#ifndef __QAIC_REMOTE
#define __QAIC_REMOTE(ff) ff
#endif //__QAIC_REMOTE

#ifndef __QAIC_HEADER
#define __QAIC_HEADER(ff) ff
#endif //__QAIC_HEADER

#ifndef __QAIC_HEADER_EXPORT
#define __QAIC_HEADER_EXPORT
#endif // __QAIC_HEADER_EXPORT

#ifndef __QAIC_HEADER_ATTRIBUTE
#define __QAIC_HEADER_ATTRIBUTE
#endif // __QAIC_HEADER_ATTRIBUTE

#ifndef __QAIC_IMPL
#define __QAIC_IMPL(ff) ff
#endif //__QAIC_IMPL

#ifndef __QAIC_IMPL_EXPORT
#define __QAIC_IMPL_EXPORT
#endif // __QAIC_IMPL_EXPORT

#ifndef __QAIC_IMPL_ATTRIBUTE
#define __QAIC_IMPL_ATTRIBUTE
#endif // __QAIC_IMPL_ATTRIBUTE

#ifndef __QAIC_STUB
#define __QAIC_STUB(ff) ff
#endif //__QAIC_STUB

#ifndef __QAIC_STUB_EXPORT
#define __QAIC_STUB_EXPORT
#endif // __QAIC_STUB_EXPORT

#ifndef __QAIC_STUB_ATTRIBUTE
#define __QAIC_STUB_ATTRIBUTE
#endif // __QAIC_STUB_ATTRIBUTE

#ifndef __QAIC_SKEL
#define __QAIC_SKEL(ff) ff
#endif //__QAIC_SKEL__

#ifndef __QAIC_SKEL_EXPORT
#define __QAIC_SKEL_EXPORT
#endif // __QAIC_SKEL_EXPORT

#ifndef __QAIC_SKEL_ATTRIBUTE
#define __QAIC_SKEL_ATTRIBUTE
#endif // __QAIC_SKEL_ATTRIBUTE

#ifdef __QAIC_DEBUG__
   #ifndef __QAIC_DBG_PRINTF__
   #define __QAIC_DBG_PRINTF__( ee ) do { printf ee ; } while(0)
   #endif
#else
   #define __QAIC_DBG_PRINTF__( ee ) (void)0
#endif


#define _OFFSET(src, sof)  ((void*)(((char*)(src)) + (sof)))

#define _COPY(dst, dof, src, sof, sz)  \
   do {\
         struct __copy { \
            char ar[sz]; \
         };\
         *(struct __copy*)_OFFSET(dst, dof) = *(struct __copy*)_OFFSET(src, sof);\
   } while (0)

#define _ASSIGN(dst, src, sof)  \
   do {\
      dst = OFFSET(src, sof); \
   } while (0)

#define _STD_STRLEN_IF(str) (str == 0 ? 0 : strlen(str))

#include "AEEStdErr.h"

#define _TRY(ee, func) \
   do { \
      if (AEE_SUCCESS != ((ee) = func)) {\
         __QAIC_DBG_PRINTF__((__FILE_LINE__  ": error: %d\n", (int)(ee)));\
         goto ee##bail;\
      } \
   } while (0)

#define _CATCH(exception) exception##bail: if (exception != AEE_SUCCESS)

#define _ASSERT(nErr, ff) _TRY(nErr, 0 == (ff) ? AEE_EBADPARM : AEE_SUCCESS)

#ifdef __QAIC_DEBUG__
#define _ALLOCATE(nErr, pal, size, alignment, pv) _TRY(nErr, allocator_alloc(pal, __FILE_LINE__, size, alignment, (void**)&pv))
#else
#define _ALLOCATE(nErr, pal, size, alignment, pv) _TRY(nErr, allocator_alloc(pal, 0, size, alignment, (void**)&pv))
#endif


#ifdef __cplusplus
extern "C" {
#endif
static __inline int _skel_method(int (*_pfn)(), uint32 _sc, remote_arg* _pra) {
   remote_arg* _praEnd;
   int _nErr = 0;
   _praEnd = ((_pra + REMOTE_SCALARS_INBUFS(_sc)) + REMOTE_SCALARS_OUTBUFS(_sc));
   _ASSERT(_nErr, (_pra + 0) <= _praEnd);
   _TRY(_nErr, _pfn());
   _CATCH(_nErr) {}
   return _nErr;
}
static __inline int _skel_method_1(int (*_pfn)(uint32*, char*, uint32, char*, uint32, char*, uint32, char*, uint32, uint32*, char*, uint32, uint32), uint32 _sc, remote_arg* _pra) {
   remote_arg* _praEnd;
   uint32 _in0[418];
   char* _in1[1];
   uint32 _in1Len[1];
   char* _in2[1];
   uint32 _in2Len[1];
   char* _in3[1];
   uint32 _in3Len[1];
   char* _in4[1];
   uint32 _in4Len[1];
   uint32 _rout5[1];
   char* _rout6[1];
   uint32 _rout6Len[1];
   uint32 _in7[1];
   uint32* _primIn;
   int _numIn[1];
   uint32* _primROut;
   remote_arg* _praIn;
   remote_arg* _praROut;
   int _nErr = 0;
   _praEnd = ((_pra + REMOTE_SCALARS_INBUFS(_sc)) + REMOTE_SCALARS_OUTBUFS(_sc));
   _ASSERT(_nErr, (_pra + 7) <= _praEnd);
   _numIn[0] = (REMOTE_SCALARS_INBUFS(_sc) - 1);
   _ASSERT(_nErr, _pra[0].buf.nLen >= 1696);
   _primIn = _pra[0].buf.pv;
   _ASSERT(_nErr, _pra[(_numIn[0] + 1)].buf.nLen >= 4);
   _primROut = _pra[(_numIn[0] + 1)].buf.pv;
   _COPY(_in0, 0, _primIn, 0, 1672);
   _COPY(_in1Len, 0, _primIn, 1672, 4);
   _praIn = (_pra + 1);
   _ASSERT(_nErr, (_praIn[0].buf.nLen / 1) >= (int)_in1Len[0]);
   _in1[0] = _praIn[0].buf.pv;
   _COPY(_in2Len, 0, _primIn, 1676, 4);
   _ASSERT(_nErr, (_praIn[1].buf.nLen / 1) >= (int)_in2Len[0]);
   _in2[0] = _praIn[1].buf.pv;
   _COPY(_in3Len, 0, _primIn, 1680, 4);
   _ASSERT(_nErr, (_praIn[2].buf.nLen / 1) >= (int)_in3Len[0]);
   _in3[0] = _praIn[2].buf.pv;
   _COPY(_in4Len, 0, _primIn, 1684, 4);
   _ASSERT(_nErr, (_praIn[3].buf.nLen / 1) >= (int)_in4Len[0]);
   _in4[0] = _praIn[3].buf.pv;
   _COPY(_rout6Len, 0, _primIn, 1688, 4);
   _praROut = (_praIn + _numIn[0] + 1);
   _ASSERT(_nErr, (_praROut[0].buf.nLen / 1) >= (int)_rout6Len[0]);
   _rout6[0] = _praROut[0].buf.pv;
   _COPY(_in7, 0, _primIn, 1692, 4);
   _TRY(_nErr, _pfn(_in0, *_in1, *_in1Len, *_in2, *_in2Len, *_in3, *_in3Len, *_in4, *_in4Len, _rout5, *_rout6, *_rout6Len, *_in7));
   _COPY(_primROut, 0, _rout5, 0, 4);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_SKEL_EXPORT int __QAIC_SKEL(adsp_jpege_skel_invoke)(uint32 _sc, remote_arg* _pra) __QAIC_SKEL_ATTRIBUTE {
   switch(REMOTE_SCALARS_METHOD(_sc))
   {
      case 0:
      return _skel_method((void*)__QAIC_IMPL(adsp_jpege_fastrpc_start), _sc, _pra);
      case 1:
      return _skel_method((void*)__QAIC_IMPL(adsp_jpege_init), _sc, _pra);
      case 2:
      return _skel_method_1((void*)__QAIC_IMPL(adsp_jpege_q6_process), _sc, _pra);
      case 3:
      return _skel_method((void*)__QAIC_IMPL(adsp_jpege_deinit), _sc, _pra);
   }
   return AEE_EUNSUPPORTED;
}
#ifdef __cplusplus
}
#endif
#endif //_ADSP_JPEGE_SKEL_H
