/*========================================================================

*//** @file jpege_engine_sw_huff_bs.S
  This file contains the implementations for JPEG Encode Huffman DC
  and AC Runlength in ARM assembly.

@par EXTERNALIZED FUNCTIONS
  jpege_engine_sw_pack_bs
  jpege_engine_sw_huff_encode_dc
  jpege_engine_sw_huff_encode_ac

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2009, 2014 Qualcomm Technologies, Inc. All Rights reserved.
Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/still/v3/utils/gcc/main/latest/src/jpege_engine_sw_huff_bs_arm_gcc.S#1 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
05/07/09   zhiminl Split jpege_engine_sw_huff_encode_ac() from
                   jpege_engine_sw_huff.c.
05/04/09   zhiminl Split jpege_engine_sw_pack_bs() from jpege_engine_sw_huff.c.
04/16/09   zhiminl Created.

========================================================================== */

/*------------------------------------------------------------------------
 *                          Private Constants
 * ----------------------------------------------------------------------- */

.equ NUM_AC_CATEGORIES,              11   @ # of Huffman AC categories
.equ ZRL,                            0xF0 @ AC ZRL symbol

/* Offsets of data members in bitstream_t */
.equ BITASSEMBLY_BUFFER_OFFSET,      0    @ bitstream_t->bitAssemblyBufferUL32
.equ BITSFREE_OFFSET,                4    @ bitstream_t->bitsFree
.equ BITSTREAM_NEXT_BYTE_PTR_OFFSET, 8    @ bitstream_t->nextBytePtr

.equ BLOCK_SIZE_MINUS_1,             63   @ block size - 1

/*------------------------------------------------------------------------
 *                          EXPORTS
 * ----------------------------------------------------------------------- */

    .global jpege_engine_sw_pack_bs
    .global jpege_engine_sw_huff_encode_dc
    .global jpege_engine_sw_huff_encode_ac

/*------------------------------------------------------------------------
 *                          Function Definitions
 * ----------------------------------------------------------------------- */

    .section jpeg_huff_bs, "x"            @ AREA
    .text                                 @ |.text|, CODE, READONLY
    .align  2
    .code   32

/*========================================================================

 FUNCTION            : jpege_engine_sw_pack_bs

 DESCRIPTION         : Pack code bitstream temporarily in a 32-bit buffer,
                       then move the bitstream byte by byte to the
                       bitstream buffer.

 C PROTOTYPE         : void jpege_engine_sw_pack_bs (
                         uint32_t codeWord,
                         uint32_t codeLength,
                         bitstream_t *p_bitstream)

 REGISTER INPUTS     : R0: uint32_t codeWord
                             Huffman code word
                       R1: uint32_t codeLength
                             Huffman code length
                       R2: bitstream_t *p_bitstream
                             pointer of the bitstream buffer including
                             the temporary 32-bit assembly buffer.

 STACK ARGUMENTS     : None

 REGISTER OUTPUTS    : None

 MEMORY INPUTS       : the 32-bit assembly buffer

 MEMORY OUTPUTS      : the 32-bit assembly buffer and bitstream buffer

 REGISTERS AFFECTED  : R0-R4

 STACK USAGE         : None

 CYCLES              :

 NOTES               : None

========================================================================== */
.type jpege_engine_sw_pack_bs, %function
jpege_engine_sw_pack_bs:                  @ global label called outside of the file
__jpege_engine_sw_pack_bs:                @ local label called inside of the file
    codeWord              .req r0
    codeLength            .req r1
    bitstream             .req r2

    byteToMove            .req r0
    nextBytePtr           .req r1         @ bitstream->nextBytePtr
    bitBuffer             .req r3         @ bitstream->bitAssemblyBufferUL32
    bitsFree              .req r4         @ bitstream->bitsFree
    zeroByte              .req r12        @ scratch register

    STMFD   sp!, {r4}                     @ start of jpege_engine_sw_pack_bs()

    LDR     bitsFree,  [bitstream, #BITSFREE_OFFSET]
                                          @ bitFree (r4) = bitstream->bitsFree
    LDR     bitBuffer, [bitstream, #BITASSEMBLY_BUFFER_OFFSET]
                                          @ bitBuffer (r3) = bitstream->bitAssemblyBufferUL32

    /*--------------------------------------------------------------------
     * Pack codeWord into bitAssembly buffer
     * ------------------------------------------------------------------- */
    SUBS    bitsFree, bitsFree, codeLength
                                          @ bitsFree (r4) -= codeLength (r1)
    ORR     bitBuffer, bitBuffer, codeWord, LSL bitsFree
                                          @ bitBuffer (r3) |= (codeWord (r0) << bitsFree (r4))

    CMP     bitsFree, #24                 @ while (bitsFree (r4) <= 24)
    BGT     residue

    LDR     nextBytePtr,   [bitstream, #BITSTREAM_NEXT_BYTE_PTR_OFFSET]
                                          @ nextBytePtr (r1) = bitstream->nextBytePtr

    MOV     zeroByte, #0x0                @ r12 = 0x0

flush_bitstream:
    /*--------------------------------------------------------------------
     * Move bytes from bitAssembly buffer to final bitstream buffer
     * ------------------------------------------------------------------- */
    MOV     byteToMove, bitBuffer, LSR #24
                                          @ byteToMove (r0) = (uint8)(bitAssemblyBufferUL32 (r3) >> 24)

    ADD     bitsFree, bitsFree, #8        @ bitsFree (r4) += 8

    STRB    byteToMove, [nextBytePtr], #1 @ *(nextBytePtr++ (r1)) = byteToMove (r0)

    MOV     bitBuffer, bitBuffer, LSL #8  @ bitAssemblyBufferUL32 (r3) <<= 8

    TEQ     byteToMove, #0xFF             @ if (byteToMove (r0) == 0xFF)

                                          @ Insert byte 0x00 if the byteToMove is 0xFF
    STREQB  zeroByte, [nextBytePtr], #1   @ *(nextBytePtr++ (r1)) = 0x0

    CMP     bitsFree, #24                 @ while (bitsFree (r4) <= 24)
    BLE     flush_bitstream

    STR     nextBytePtr, [bitstream, #BITSTREAM_NEXT_BYTE_PTR_OFFSET]
                                          @ bitstream->nextBytePtr = nextBytePtr (r1)

residue:
    /*--------------------------------------------------------------------
     * It is possible there is some residue (less than 8 bits) in the
     * 32-bit assembly buffer for the next packing
     * ------------------------------------------------------------------- */
    STR     bitBuffer, [bitstream, #BITASSEMBLY_BUFFER_OFFSET]
                                          @ bitstream->bitAssemblyBufferUL32 = bitAssemblyBufferUL32 (r1)
    STR     bitsFree,  [bitstream, #BITSFREE_OFFSET]
                                          @ bitstream->bitsFree = bitFree (r5)

    LDMFD   sp!, {r4}
    BX      lr                            @ end of jpege_engine_sw_pack_bs()

/*========================================================================

 FUNCTION            : jpege_engine_sw_huff_encode_dc

 DESCRIPTION         : Huffman DC encoding + Zero byte insertion.

 C PROTOTYPE         : void jpege_engine_sw_huff_encode_dc (
                         int32_t dcDiff,
                         const huff_lookup_table_t *dcHuffLookupTablePtr,
                         bitstream_t *dcBitStream)

 REGISTER INPUTS     : R0: int32_t dcDiff
                             Huffman DC DIFF = ZZ(0) - PRED
                       R1: huff_lookup_table_t *dcHuffLookupTablePtr
                             pointer of the Huffman DC lookup table
                       R2: bitstream_t *dcBitStream
                             pointer of the bitstream buffer including
                             the temporary 32-bit assembly buffer.

 STACK ARGUMENTS     : None

 REGISTER OUTPUTS    : None

 MEMORY INPUTS       : the 32-bit assembly buffer

 MEMORY OUTPUTS      : the 32-bit assembly buffer and bitstream buffer

 REGISTERS AFFECTED  : R0-R4

 STACK USAGE         : None

 CYCLES              :

 NOTES               : None

========================================================================== */
.type jpege_engine_sw_huff_encode_dc, %function
jpege_engine_sw_huff_encode_dc:
    dcDiff                .req r0
    dcHuffLookupTablePtr  .req r1         @ DC HUFF lookup table pointer
    dcBitstream           .req r2

    dcMagnitude           .req r0         @ abs(dcDiff)
    dcLeadingZeros        .req r0         @ # of leading zeros in dcMagnitude
    dcCategory            .req r0         @ DC category derived from dcLeadingZeros
    dcDiffComplement      .req r3         @ 1's complement of dcDiff
    dcCodeword            .req r4         @ DC HUFFCODE
    dcCodelength          .req r12        @ DC HUFFSIZE - scratch register

    dcXcodeword           .req r0         @ DC XHUFFCODE with appended SSSS bits
    dcXcodelength         .req r1         @ DC XHUFFSIZE with appended SSSS bits

    STMFD   sp!, {r4, lr}                 @ start of jpege_engine_sw_huff_encode_dc()

    /*--------------------------------------------------------------------
     * Get complement value and DC category for dcDiff
     * ------------------------------------------------------------------- */
    MOVS    dcDiffComplement, dcDiff      @ dcDiffComplement (r3) = dcDiff (r0)
                                          @ if (dcDiff >= 0)
                                          @ dcMagnitude (r0) = dcDiff (r0)
                                          @ if (dcDiff >= 0)

negtive_dcDiff:
    SUBLT   dcDiffComplement, dcDiff, #1  @ dcDiffComplement (r3) = dcDiff (r0) - 1 if (dcDiff < 0)
    RSBLT   dcMagnitude, dcDiff, #0       @ dcMagnitude (r0)      = -dcDiff (r0) if (dcDiff < 0)

get_dcDiff_category:
    CLZ     dcLeadingZeros, dcMagnitude   @ get # of leading zeros in dcMagnitude (r0)

    MOVLT   dcDiffComplement, dcDiffComplement, LSL dcLeadingZeros
                                          @ dcDiffComplement (r3) <<= dcLeadingZeros (r0)

    MOVLT   dcDiffComplement, dcDiffComplement, LSR dcLeadingZeros
                                          @ dcDiffComplement (r3) >>= dcLeadingZeros (r0)

    RSB     dcCategory, dcLeadingZeros, #32
                                          @ dcCategory (r0) = 32 - dcLeadingZeros (r0)

lookup_dc_codeword_codelength:
    /*--------------------------------------------------------------------
     * Find codeword and codelength corresponding to DC Category
     * ------------------------------------------------------------------- */
    ADD     dcHuffLookupTablePtr, dcHuffLookupTablePtr, dcCategory, LSL #2
                                          @ (dcHuffLookupTablePtr (r1) += dcCategory (r0))

    LDRH    dcCodelength, [dcHuffLookupTablePtr, #0]
                                          @ codeLength (r12) = (dcHuffLookupTablePtr (r1))->codeLength

    LDRH    dcCodeword,   [dcHuffLookupTablePtr, #2]
                                          @ codeWord (r4) = (dcHuffLookupTablePtr (r1))->codeWord

pack_dc_xcodeword:
    /*--------------------------------------------------------------------
     * Pack codeWord and 1's complement of DC diff value into bitAssembly buffer
     * ------------------------------------------------------------------- */
    ADD     dcXcodelength, dcCodelength, dcCategory
                                          @ xcodeLength (r1) = codeLength (r12) + dcCategory (r0)

    ORR     dcXcodeword, dcDiffComplement, dcCodeword, LSL dcCategory
                                          @ xcodeWord (r0) = (codeWord (r4) << dcCategory (r0)) | dcDiffComplement (r3)

    BL      __jpege_engine_sw_pack_bs     @ jpege_engine_sw_pack_bs(xcodeword, xcodelength, dcBitstream)

    LDMFD   sp!, {r4, lr}
    BX      lr                            @ end of jpege_engine_sw_huff_encode_dc()

/*========================================================================

 FUNCTION            : jpege_engine_sw_runlength

 DESCRIPTION         : Huffman Run Length encoding + Zero byte insertion
                       for one symbol.

 C PROTOTYPE         : void jpege_engine_sw_runlength (
                         int32_t runLength,
                         int32_t termSymbol,
                         const huff_lookup_table_t *acHuffLookupTablePtr,
                         bitstream_t *acBitStream)

 REGISTER INPUTS     : R0: int32_t runLength
                             Huffman AC RRRR
                       R1: int32_t termSymbol
                             Next non-zero coefficient in ZZ
                       R2: huff_lookup_table_t *acHuffLookupTablePtr
                             pointer of the Huffman AC lookup table
                       R3: bitstream_t *acBitStream
                             pointer of the bitstream buffer including
                             the temporary 32-bit assembly buffer.

 STACK ARGUMENTS     : None

 REGISTER OUTPUTS    : None

 MEMORY INPUTS       : the 32-bit assembly buffer

 MEMORY OUTPUTS      : the 32-bit assembly buffer and bitstream buffer

 REGISTERS AFFECTED  : R0-R10

 STACK USAGE         : None

 CYCLES              :

 NOTES               : None

========================================================================== */
.type jpege_engine_sw_runlength, %function
jpege_engine_sw_runlength:
__jpege_engine_sw_runlength:
    runLength             .req r0
    termSymbol            .req r1
    acHuffLookupTablePtr  .req r2
    acBitstream           .req r6         @ in r3 initially, save it to r6

    acMagnitude           .req r1         @ abs(termSymbol)
    runSize               .req r1         @ RRRRSSSS

    acLeadingZeros        .req r4         @ # of leading zeros in acMagnitude
    acCategory            .req r4         @ AC category derived from acLeadingZeros
    acComplement          .req r5         @ 1's complement of termSymbol

    acCodeword            .req r0         @ AC HUFFCODE
    acCodelength          .req r1         @ AC HUFFSIZE

    acXcodeword           .req r0         @ AC XHUFFCODE with appended SSSS bits
    acXcodelength         .req r1         @ AC XHUFFSIZE with appended SSSS bits

    zrlCodeword           .req r0         @ ZRL HUFFCODE
    zrlCodelength         .req r1         @ ZRL HUFFSIZE

    STMFD   sp!, {r4-r8, lr}              @ start of jpege_engine_sw_runlength()
    MOV     acBitstream, r3               @ keep acBitstream pointer at r6

    /*--------------------------------------------------------------------
     * Check if ZRL encode is needed
     * ------------------------------------------------------------------- */
    CMP     runLength, #15                @ check if it is ZRL
    BLS     pack_other
    MOV     r4, runLength                 @ save runLength (r0)
    MOV     r5, termSymbol                @ save termSymbol (r1)
    MOV     r7, acHuffLookupTablePtr      @ save acHuffLookupTablePtr (r2)
    ADD     r8, acHuffLookupTablePtr, #(ZRL * 4)
                                          @ r8 = (acHuffLookupTablePtr + ZRL)

pack_zrl:
    MOV     r2, acBitstream               @ bitstream pointer
    LDRH    zrlCodelength, [r8]           @ codeLength (r1) = (acHuffLookupTablePtr + ZRL)->codeLength

    LDRH    zrlCodeword,   [r8, #2]       @ codeWord (r0) = (acHuffLookupTablePtr + ZRL)->codeWord

    BL      __jpege_engine_sw_pack_bs     @ jpege_engine_sw_pack_bs(codeword, codelength, acBitstream)
    SUB     r4, r4, #16                   @ runLength -= 16
    CMP     r4, #15                       @ while (runlength > 16)
    BGT     pack_zrl

    /*--------------------------------------------------------------------
     * Done with pack_zrl
     * ------------------------------------------------------------------- */
    MOV     runLength, r4                 @ restore runLength (r0) = r4
    MOV     termSymbol, r5                @ restore termSymbol (r1) = r5
    MOV     acHuffLookupTablePtr, r7      @ restore acHuffLookupTablePtr (r2) = r7

pack_other:
    /*--------------------------------------------------------------------
     * Get complement value and AC category for termSymbol (coefficient)
     * ------------------------------------------------------------------- */
    MOVS    acComplement, termSymbol      @ acComplement (r5) = termSymbol (r1)
                                          @ if (termSymbol >= 0)
                                          @ acMagnitude (r1) = termSymbol (r1)

negtive_termSymbol:
    SUBLT   acComplement, termSymbol, #1  @ acComplement (r5)= termSymbol (r1) - 1
                                          @ if (termSymbol < 0)
    RSBLT   acMagnitude, termSymbol, #0   @ acMagnitude (r1) = -termSymbol (r1)
                                          @ if (termSymbol < 0)

get_ac_category:
    CLZ     acLeadingZeros, acMagnitude   @ get # of leading zeros in acMagnitude (r1)

    MOVLT   acComplement, acComplement, LSL acLeadingZeros
                                          @ acComplement (r5) <<= acLeadingZeros (r4)

    MOVLT   acComplement, acComplement, LSR acLeadingZeros
                                          @ acComplement (r5) >>= acLeadingZeros (r4)

    RSB     acCategory, acLeadingZeros, #32
                                          @ acCategory (r4) = 32 - acLeadingZeros (r4)

RRRRSSSS:
    ORR     runSize, acCategory, runLength, LSL #4
                                          @ runSize (r1) = runLength (r0) << 4 | acCategory (r4)

lookup_ac_codeword_codelength:
    /*--------------------------------------------------------------------
     * Find codeword and codelength corresponding to runSize
     * ------------------------------------------------------------------- */
    ADD     acHuffLookupTablePtr, acHuffLookupTablePtr, runSize, LSL #2
                                          @ (acHuffLookupTablePtr (r2) += runSize (r1))

    LDRH    acCodelength, [acHuffLookupTablePtr, #0]
                                          @ codeLength (r1) = (acHuffLookupTablePtr (r2))->codeLength

    LDRH    acCodeword,   [acHuffLookupTablePtr, #2]
                                          @ codeWord (r0) = (acHuffLookupTablePtr (r2))->codeWord

    /*--------------------------------------------------------------------
     * Pack codeWord and 1's complement of AC coefficient value into bitAssembly buffer
     * ------------------------------------------------------------------- */
    CMP     acCategory, #(NUM_AC_CATEGORIES - 1)
    BGE     pack_ac_codeword

pack_ac_xcodeword:
    ADD     acXcodelength, acCodelength, acCategory
                                          @ xcodeLength (r1) = acCodelength (r1) + acCategory (r4)
    ORR     acXcodeword, acComplement, acCodeword, LSL acCategory
                                          @ xcodeWord (r0) = acCodeword (r0) << acCategory (r4) | acComplement (r5)
    MOV     r2, acBitstream               @ r2 = acBitstream (r6)
    BL      __jpege_engine_sw_pack_bs     @ jpege_engine_sw_pack_bs(xcodeWord, xcodeLength, acBitstream)
    LDMFD   sp!, {r4-r8, lr}
    BX      lr

pack_ac_codeword:
    MOV     r2, acBitstream               @ r2 = acBitstream (r6)
    BL      __jpege_engine_sw_pack_bs     @ jpege_engine_sw_pack_bs(codeword, codelength, acBitstream)

pack_ac_complement:
    MOV     r0, acComplement              @ r0 = acComplement (r5)
    MOV     r1, acCategory                @ r1 = acCategory (r4)
    MOV     r2, acBitstream               @ r2 = acBitstream (r6)
    BL      __jpege_engine_sw_pack_bs     @ jpege_engine_sw_pack_bs(acComplement, acCategory, acBitstream)
    LDMFD   sp!, {r4-r8, lr}
    BX      lr                            @ end of jpege_engine_sw_runlength()

/*========================================================================

 FUNCTION            : jpege_engine_sw_huff_encode_ac

 DESCRIPTION         : Huffman AC encoding + Zero byte insertion for one
                       block of data

 C PROTOTYPE         : void jpege_engine_sw_huff_encode_ac (
                         const int16_t *zigzagOutput,
                         const huff_lookup_table_t *p_acHuffLookupTable,
                         bitstream_t *p_bitstream)

 REGISTER INPUTS     : R0: int16_t zigzagOutput
                             pointer of the quantized and zigzaged 64 coefficients
                       R1: huff_lookup_table_t *p_acHuffLookupTable
                             pointer of the Huffman AC lookup table
                       R2: bitstream_t *p_bitstream
                             pointer of the bitstream buffer including
                             the temporary 32-bit assembly buffer.

 STACK ARGUMENTS     : None

 REGISTER OUTPUTS    : None

 MEMORY INPUTS       : zigzagOutput - quantized and zigzagged 64 coefficients
                       p_bitstrem   - 32-bit assembly buffer and bitstream buffer

 MEMORY OUTPUTS      : the 32-bit assembly buffer and bitstream buffer

 REGISTERS AFFECTED  : R0-R7

 STACK USAGE         : None

 CYCLES              :

 NOTES               : None

========================================================================== */
.type jpege_engine_sw_huff_encode_ac, %function
jpege_engine_sw_huff_encode_ac:
    runLength             .req r0
    currSymbol            .req r1
    acHuffLookupTable     .req r4
    rsBitstream           .req r5

    zigzagOutput          .req r6
    count                 .req r7

    eobCodeword           .req r0
    eobCodelength         .req r1

    STMFD   sp!, {r4-r7, lr}              @ start of jpege_engine_sw_huff_encode_ac()

    ADD     zigzagOutput, r0, #2          @ &zigzagOutput[1] (r6)
    MOV     acHuffLookupTable, r1         @ keep acHuffLookupTable pointer (r4)
    MOV     rsBitstream, r2               @ keep bitstream pointer (r5)

    MOV     runLength, #0                 @ runLength (r0) = 0

    MOV     count, #(BLOCK_SIZE_MINUS_1 - 1)
                                          @ from (BLOCK_SIZE_MINUS_1 - 1) down to 0

ac_loop:
    LDRSH   currSymbol, [zigzagOutput], #2
                                          @ currSymbol (r1) = [zigzagOutput]
                                          @ zigzagOutput++ (r6)

    CMP      currSymbol, #0               @ if (currSymbol (r1) == 0)
    ADDEQ    runLength, runLength, #1     @ runLength++ (r0)
    BEQ      next_ac

    MOV      r3, rsBitstream              @ r3 = bitstream pointer
    MOV      r2, acHuffLookupTable        @ r2 = acHuffLookupTable pointer
    BL       __jpege_engine_sw_runlength  @ jpege_engine_sw_runlength(runLength, currSymbol,
                                          @                           p_acHuffLookupTable,
                                          @                           p_bitstream);
    MOV      runLength, #0                @ runLength (r0) = 0

next_ac:
    SUBS     count, count, #1             @ count--
    BCS      ac_loop

pack_eob:
    CMP      runLength, #0                @ if (runLength (r0) != 0)
    LDRNEH   eobCodelength, [acHuffLookupTable]
                                          @ r1 = (p_acHuffLookupTable + EOB)->codeWord
    LDRNEH   eobCodeword, [acHuffLookupTable, #2]
                                          @ r0 = (p_acHuffLookupTable + EOB)->codeLength
    MOVNE    r2, rsBitstream              @ r2 = p_bitstream
    BLNE     __jpege_engine_sw_pack_bs    @ jpege_engine_sw_pack_bs(codeWord, codeLength, p_bitstream)

    LDMFD    sp!, {r4-r7, lr}
    BX       lr                           @ end of jpege_engine_sw_huff_encode_ac()

.end                                      @ end of jpege_engine_sw_huff_bs.S
