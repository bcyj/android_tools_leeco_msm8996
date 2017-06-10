
/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef GEMINI_HW_REG_H
#define GEMINI_HW_REG_H
/* base is set to 0 as all addresses will be handled as offset in kernel */
#define GEMINI_BASE                                             0
#define GEMINI_BASE_SIZE                                        0x00001000
#define GEMINI_BASE_PHYS                                        0xa3a00000

/*------------------------------------------------------------------------------
 * MODULE: gemini
 *------------------------------------------------------------------------------*/

#define GEMINI_REG_BASE                                          (GEMINI_BASE + 0x00000000)
#define GEMINI_REG_BASE_PHYS                                     0xa3a00000

#define HWIO_JPEG_HW_VERSION_ADDR                                (GEMINI_REG_BASE      + 0x00000000)
#define HWIO_JPEG_HW_VERSION_PHYS                                (GEMINI_REG_BASE_PHYS + 0x00000000)
#define HWIO_JPEG_HW_VERSION_RMSK                                   0xfffff
#define HWIO_JPEG_HW_VERSION_SHFT                                         0
#define HWIO_JPEG_HW_VERSION_IN                                  \
        in_dword_masked(HWIO_JPEG_HW_VERSION_ADDR, HWIO_JPEG_HW_VERSION_RMSK)
#define HWIO_JPEG_HW_VERSION_INM(m)                              \
        in_dword_masked(HWIO_JPEG_HW_VERSION_ADDR, m)
#define HWIO_JPEG_HW_VERSION_CORE_VERSION_BMSK                      0xf0000
#define HWIO_JPEG_HW_VERSION_CORE_VERSION_SHFT                         0x10
#define HWIO_JPEG_HW_VERSION_MAJOR_VERSION_BMSK                      0xff00
#define HWIO_JPEG_HW_VERSION_MAJOR_VERSION_SHFT                         0x8
#define HWIO_JPEG_HW_VERSION_MINOR_VERSION_BMSK                        0xff
#define HWIO_JPEG_HW_VERSION_MINOR_VERSION_SHFT                           0

/* IMEM information: introduced in 8960 */
#define HWIO_JPEG_CFG_IMEM_MODE_BMSK                                               0x200
#define HWIO_JPEG_CFG_IMEM_MODE_DISABLE_BMSK                                       0x000
#define HWIO_JPEG_CFG_IMEM_MODE_DISABLE_SHFT                                         0x9
#define HWIO_JPEG_CFG_IMEM_MODE_DISABLE_IMEM_FIFO_BASED_ADDRESS_GENERATION_FVAL        0
#define HWIO_JPEG_CFG_IMEM_MODE_DISABLE_IMEM_PING_PONG_ADDRESS_GENERATION_FVAL       0x1

#define HWIO_JPEG_RESET_CMD_ADDR                                 (GEMINI_REG_BASE      + 0x00000004)
#define HWIO_JPEG_RESET_CMD_PHYS                                 (GEMINI_REG_BASE_PHYS + 0x00000004)
#define HWIO_JPEG_RESET_CMD_RMSK                                 0xe004ffff
#define HWIO_JPEG_RESET_CMD_SHFT                                          0
#define HWIO_JPEG_RESET_CMD_OUT(v)                               \
        out_dword(HWIO_JPEG_RESET_CMD_ADDR,v)
#define HWIO_JPEG_RESET_CMD_OUTM(m,v)                            \
        out_dword_masked(HWIO_JPEG_RESET_CMD_ADDR,m,v,HWIO_JPEG_RESET_CMD_shadow)
#define HWIO_JPEG_RESET_CMD_RESET_BYPASS_BMSK                    0x80000000
#define HWIO_JPEG_RESET_CMD_RESET_BYPASS_SHFT                          0x1f
#define HWIO_JPEG_RESET_CMD_BUS_DOMAIN_RESET_BMSK                0x40000000
#define HWIO_JPEG_RESET_CMD_BUS_DOMAIN_RESET_SHFT                      0x1e
#define HWIO_JPEG_RESET_CMD_JPEG_DOMAIN_RESET_BMSK               0x20000000
#define HWIO_JPEG_RESET_CMD_JPEG_DOMAIN_RESET_SHFT                     0x1d
#define HWIO_JPEG_RESET_CMD_CLKONIDLE_RESET_BMSK                    0x40000
#define HWIO_JPEG_RESET_CMD_CLKONIDLE_RESET_SHFT                       0x12
#define HWIO_JPEG_RESET_CMD_CORE_RESET_BMSK                          0x8000
#define HWIO_JPEG_RESET_CMD_CORE_RESET_SHFT                             0xf
#define HWIO_JPEG_RESET_CMD_BUS_STATS_RESET_BMSK                     0x4000
#define HWIO_JPEG_RESET_CMD_BUS_STATS_RESET_SHFT                        0xe
#define HWIO_JPEG_RESET_CMD_BUS_MISR_RESET_BMSK                      0x2000
#define HWIO_JPEG_RESET_CMD_BUS_MISR_RESET_SHFT                         0xd
#define HWIO_JPEG_RESET_CMD_TESTGEN_RESET_BMSK                       0x1000
#define HWIO_JPEG_RESET_CMD_TESTGEN_RESET_SHFT                          0xc
#define HWIO_JPEG_RESET_CMD_REGISTER_RESET_BMSK                       0x800
#define HWIO_JPEG_RESET_CMD_REGISTER_RESET_SHFT                         0xb
#define HWIO_JPEG_RESET_CMD_VFE_INTERFACE_RESET_BMSK                  0x400
#define HWIO_JPEG_RESET_CMD_VFE_INTERFACE_RESET_SHFT                    0xa
#define HWIO_JPEG_RESET_CMD_BRIDGE_RESET_BMSK                         0x200
#define HWIO_JPEG_RESET_CMD_BRIDGE_RESET_SHFT                           0x9
#define HWIO_JPEG_RESET_CMD_WRITE_ENGINE_RESET_BMSK                   0x100
#define HWIO_JPEG_RESET_CMD_WRITE_ENGINE_RESET_SHFT                     0x8
#define HWIO_JPEG_RESET_CMD_FETCH_ENGINE_RESET_BMSK                    0x80
#define HWIO_JPEG_RESET_CMD_FETCH_ENGINE_RESET_SHFT                     0x7
#define HWIO_JPEG_RESET_CMD_STATS_RESET_BMSK                           0x40
#define HWIO_JPEG_RESET_CMD_STATS_RESET_SHFT                            0x6
#define HWIO_JPEG_RESET_CMD_HUFFMAN_RESET_BMSK                         0x20
#define HWIO_JPEG_RESET_CMD_HUFFMAN_RESET_SHFT                          0x5
#define HWIO_JPEG_RESET_CMD_ZIGZAG_RESET_BMSK                          0x10
#define HWIO_JPEG_RESET_CMD_ZIGZAG_RESET_SHFT                           0x4
#define HWIO_JPEG_RESET_CMD_RLE_RESET_BMSK                              0x8
#define HWIO_JPEG_RESET_CMD_RLE_RESET_SHFT                              0x3
#define HWIO_JPEG_RESET_CMD_FSC_RESET_BMSK                              0x4
#define HWIO_JPEG_RESET_CMD_FSC_RESET_SHFT                              0x2
#define HWIO_JPEG_RESET_CMD_QUANTIZER_RESET_BMSK                        0x2
#define HWIO_JPEG_RESET_CMD_QUANTIZER_RESET_SHFT                        0x1
#define HWIO_JPEG_RESET_CMD_DCT_RESET_BMSK                              0x1
#define HWIO_JPEG_RESET_CMD_DCT_RESET_SHFT                                0

#define HWIO_JPEG_CFG_ADDR                                       (GEMINI_REG_BASE      + 0x00000008)
#define HWIO_JPEG_CFG_PHYS                                       (GEMINI_REG_BASE_PHYS + 0x00000008)
#define HWIO_JPEG_CFG_RMSK                                        0x7f777ff
#define HWIO_JPEG_CFG_SHFT                                                0
#define HWIO_JPEG_CFG_IN                                         \
        in_dword_masked(HWIO_JPEG_CFG_ADDR, HWIO_JPEG_CFG_RMSK)
#define HWIO_JPEG_CFG_INM(m)                                     \
        in_dword_masked(HWIO_JPEG_CFG_ADDR, m)
#define HWIO_JPEG_CFG_OUT(v)                                     \
        out_dword(HWIO_JPEG_CFG_ADDR,v)
#define HWIO_JPEG_CFG_OUTM(m,v)                                  \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_CFG_ADDR,m,v,HWIO_JPEG_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_CFG_MODE_BMSK                                   0x6000000
#define HWIO_JPEG_CFG_MODE_SHFT                                        0x19
#define HWIO_JPEG_CFG_JPEG_FORMAT_BMSK                            0x1800000
#define HWIO_JPEG_CFG_JPEG_FORMAT_SHFT                                 0x17
#define HWIO_JPEG_CFG_WE_INPUT_SEL_BMSK                            0x400000
#define HWIO_JPEG_CFG_WE_INPUT_SEL_SHFT                                0x16
#define HWIO_JPEG_CFG_JPEG_INPUT_SEL_BMSK                          0x200000
#define HWIO_JPEG_CFG_JPEG_INPUT_SEL_SHFT                              0x15
#define HWIO_JPEG_CFG_FE_INPUT_SEL_BMSK                            0x100000
#define HWIO_JPEG_CFG_FE_INPUT_SEL_SHFT                                0x14
#define HWIO_JPEG_CFG_CLKONIDLE_ENABLE_BMSK                         0x40000
#define HWIO_JPEG_CFG_CLKONIDLE_ENABLE_SHFT                            0x12
#define HWIO_JPEG_CFG_TESTBUS_ENABLE_BMSK                           0x20000
#define HWIO_JPEG_CFG_TESTBUS_ENABLE_SHFT                              0x11
#define HWIO_JPEG_CFG_HBFC_ENABLE_BMSK                              0x10000
#define HWIO_JPEG_CFG_HBFC_ENABLE_SHFT                                 0x10
#define HWIO_JPEG_CFG_BUS_STATS_ENABLE_BMSK                          0x4000
#define HWIO_JPEG_CFG_BUS_STATS_ENABLE_SHFT                             0xe
#define HWIO_JPEG_CFG_BUS_MISR_ENABLE_BMSK                           0x2000
#define HWIO_JPEG_CFG_BUS_MISR_ENABLE_SHFT                              0xd
#define HWIO_JPEG_CFG_TESTGEN_ENABLE_BMSK                            0x1000
#define HWIO_JPEG_CFG_TESTGEN_ENABLE_SHFT                               0xc
#define HWIO_JPEG_CFG_VFE_INTERFACE_ENABLE_BMSK                       0x400
#define HWIO_JPEG_CFG_VFE_INTERFACE_ENABLE_SHFT                         0xa
#define HWIO_JPEG_CFG_WE_ENABLE_BMSK                                  0x100
#define HWIO_JPEG_CFG_WE_ENABLE_SHFT                                    0x8
#define HWIO_JPEG_CFG_FE_ENABLE_BMSK                                   0x80
#define HWIO_JPEG_CFG_FE_ENABLE_SHFT                                    0x7
#define HWIO_JPEG_CFG_STATS_ENABLE_BMSK                                0x40
#define HWIO_JPEG_CFG_STATS_ENABLE_SHFT                                 0x6
#define HWIO_JPEG_CFG_HUFFMAN_ENABLE_BMSK                              0x20
#define HWIO_JPEG_CFG_HUFFMAN_ENABLE_SHFT                               0x5
#define HWIO_JPEG_CFG_ZIGZAG_ENABLE_BMSK                               0x10
#define HWIO_JPEG_CFG_ZIGZAG_ENABLE_SHFT                                0x4
#define HWIO_JPEG_CFG_RLE_ENABLE_BMSK                                   0x8
#define HWIO_JPEG_CFG_RLE_ENABLE_SHFT                                   0x3
#define HWIO_JPEG_CFG_FSC_ENABLE_BMSK                                   0x4
#define HWIO_JPEG_CFG_FSC_ENABLE_SHFT                                   0x2
#define HWIO_JPEG_CFG_QUANTIZER_ENABLE_BMSK                             0x2
#define HWIO_JPEG_CFG_QUANTIZER_ENABLE_SHFT                             0x1
#define HWIO_JPEG_CFG_DCT_ENABLE_BMSK                                   0x1
#define HWIO_JPEG_CFG_DCT_ENABLE_SHFT                                     0

#define HWIO_JPEG_REALTIME_CMD_ADDR                              (GEMINI_REG_BASE      + 0x0000000c)
#define HWIO_JPEG_REALTIME_CMD_PHYS                              (GEMINI_REG_BASE_PHYS + 0x0000000c)
#define HWIO_JPEG_REALTIME_CMD_RMSK                                     0x3
#define HWIO_JPEG_REALTIME_CMD_SHFT                                       0
#define HWIO_JPEG_REALTIME_CMD_IN                                \
        in_dword_masked(HWIO_JPEG_REALTIME_CMD_ADDR, HWIO_JPEG_REALTIME_CMD_RMSK)
#define HWIO_JPEG_REALTIME_CMD_INM(m)                            \
        in_dword_masked(HWIO_JPEG_REALTIME_CMD_ADDR, m)
#define HWIO_JPEG_REALTIME_CMD_OUT(v)                            \
        out_dword(HWIO_JPEG_REALTIME_CMD_ADDR,v)
#define HWIO_JPEG_REALTIME_CMD_OUTM(m,v)                         \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_REALTIME_CMD_ADDR,m,v,HWIO_JPEG_REALTIME_CMD_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_REALTIME_CMD_REALTIME_COMMAND_BMSK                    0x3
#define HWIO_JPEG_REALTIME_CMD_REALTIME_COMMAND_SHFT                      0

#define HWIO_JPEG_REALTIME_STATUS_ADDR                           (GEMINI_REG_BASE      + 0x00000010)
#define HWIO_JPEG_REALTIME_STATUS_PHYS                           (GEMINI_REG_BASE_PHYS + 0x00000010)
#define HWIO_JPEG_REALTIME_STATUS_RMSK                                  0x1
#define HWIO_JPEG_REALTIME_STATUS_SHFT                                    0
#define HWIO_JPEG_REALTIME_STATUS_IN                             \
        in_dword_masked(HWIO_JPEG_REALTIME_STATUS_ADDR, HWIO_JPEG_REALTIME_STATUS_RMSK)
#define HWIO_JPEG_REALTIME_STATUS_INM(m)                         \
        in_dword_masked(HWIO_JPEG_REALTIME_STATUS_ADDR, m)
#define HWIO_JPEG_REALTIME_STATUS_REALTIME_STATUS_BMSK                  0x1
#define HWIO_JPEG_REALTIME_STATUS_REALTIME_STATUS_SHFT                    0

#define HWIO_JPEG_IRQ_MASK_ADDR                                  (GEMINI_REG_BASE      + 0x00000014)
#define HWIO_JPEG_IRQ_MASK_PHYS                                  (GEMINI_REG_BASE_PHYS + 0x00000014)
#define HWIO_JPEG_IRQ_MASK_RMSK                                  0xffffffff
#define HWIO_JPEG_IRQ_MASK_SHFT                                           0
#define HWIO_JPEG_IRQ_MASK_IN                                    \
        in_dword_masked(HWIO_JPEG_IRQ_MASK_ADDR, HWIO_JPEG_IRQ_MASK_RMSK)
#define HWIO_JPEG_IRQ_MASK_INM(m)                                \
        in_dword_masked(HWIO_JPEG_IRQ_MASK_ADDR, m)
#define HWIO_JPEG_IRQ_MASK_OUT(v)                                \
        out_dword(HWIO_JPEG_IRQ_MASK_ADDR,v)
#define HWIO_JPEG_IRQ_MASK_OUTM(m,v)                             \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_IRQ_MASK_ADDR,m,v,HWIO_JPEG_IRQ_MASK_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_IRQ_MASK_MASK_BMSK                             0xffffffff
#define HWIO_JPEG_IRQ_MASK_MASK_SHFT                                      0

#define HWIO_JPEG_IRQ_CLEAR_ADDR                                 (GEMINI_REG_BASE      + 0x00000018)
#define HWIO_JPEG_IRQ_CLEAR_PHYS                                 (GEMINI_REG_BASE_PHYS + 0x00000018)
#define HWIO_JPEG_IRQ_CLEAR_RMSK                                 0xffffffff
#define HWIO_JPEG_IRQ_CLEAR_SHFT                                          0
#define HWIO_JPEG_IRQ_CLEAR_OUT(v)                               \
        out_dword(HWIO_JPEG_IRQ_CLEAR_ADDR,v)
#define HWIO_JPEG_IRQ_CLEAR_OUTM(m,v)                            \
        out_dword_masked(HWIO_JPEG_IRQ_CLEAR_ADDR,m,v,HWIO_JPEG_IRQ_CLEAR_shadow)
#define HWIO_JPEG_IRQ_CLEAR_CLEAR_BMSK                           0xffffffff
#define HWIO_JPEG_IRQ_CLEAR_CLEAR_SHFT                                    0

#define HWIO_JPEG_IRQ_STATUS_ADDR                                (GEMINI_REG_BASE      + 0x0000001c)
#define HWIO_JPEG_IRQ_STATUS_PHYS                                (GEMINI_REG_BASE_PHYS + 0x0000001c)
#define HWIO_JPEG_IRQ_STATUS_RMSK                                0xffffffff
#define HWIO_JPEG_IRQ_STATUS_SHFT                                         0
#define HWIO_JPEG_IRQ_STATUS_IN                                  \
        in_dword_masked(HWIO_JPEG_IRQ_STATUS_ADDR, HWIO_JPEG_IRQ_STATUS_RMSK)
#define HWIO_JPEG_IRQ_STATUS_INM(m)                              \
        in_dword_masked(HWIO_JPEG_IRQ_STATUS_ADDR, m)
#define HWIO_JPEG_IRQ_STATUS_STATUS_BMSK                         0xffffffff
#define HWIO_JPEG_IRQ_STATUS_STATUS_SHFT                                  0

#define HWIO_JPEG_BUS_CFG_ADDR                                   (GEMINI_REG_BASE      + 0x00000020)
#define HWIO_JPEG_BUS_CFG_PHYS                                   (GEMINI_REG_BASE_PHYS + 0x00000020)
#define HWIO_JPEG_BUS_CFG_RMSK                                          0x1
#define HWIO_JPEG_BUS_CFG_SHFT                                            0
#define HWIO_JPEG_BUS_CFG_IN                                     \
        in_dword_masked(HWIO_JPEG_BUS_CFG_ADDR, HWIO_JPEG_BUS_CFG_RMSK)
#define HWIO_JPEG_BUS_CFG_INM(m)                                 \
        in_dword_masked(HWIO_JPEG_BUS_CFG_ADDR, m)
#define HWIO_JPEG_BUS_CFG_OUT(v)                                 \
        out_dword(HWIO_JPEG_BUS_CFG_ADDR,v)
#define HWIO_JPEG_BUS_CFG_OUTM(m,v)                              \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_BUS_CFG_ADDR,m,v,HWIO_JPEG_BUS_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_BUS_CFG_OOOWR_ENABLE_BMSK                             0x1
#define HWIO_JPEG_BUS_CFG_OOOWR_ENABLE_SHFT                               0

#define HWIO_JPEG_BUS_CMD_ADDR                                   (GEMINI_REG_BASE      + 0x00000024)
#define HWIO_JPEG_BUS_CMD_PHYS                                   (GEMINI_REG_BASE_PHYS + 0x00000024)
#define HWIO_JPEG_BUS_CMD_RMSK                                          0x1
#define HWIO_JPEG_BUS_CMD_SHFT                                            0
#define HWIO_JPEG_BUS_CMD_OUT(v)                                 \
        out_dword(HWIO_JPEG_BUS_CMD_ADDR,v)
#define HWIO_JPEG_BUS_CMD_OUTM(m,v)                              \
        out_dword_masked(HWIO_JPEG_BUS_CMD_ADDR,m,v,HWIO_JPEG_BUS_CMD_shadow)
#define HWIO_JPEG_BUS_CMD_HALT_REQ_BMSK                                 0x1
#define HWIO_JPEG_BUS_CMD_HALT_REQ_SHFT                                   0

#define HWIO_JPEG_BUS_STATUS_ADDR                                (GEMINI_REG_BASE      + 0x00000028)
#define HWIO_JPEG_BUS_STATUS_PHYS                                (GEMINI_REG_BASE_PHYS + 0x00000028)
#define HWIO_JPEG_BUS_STATUS_RMSK                                      0x33
#define HWIO_JPEG_BUS_STATUS_SHFT                                         0
#define HWIO_JPEG_BUS_STATUS_IN                                  \
        in_dword_masked(HWIO_JPEG_BUS_STATUS_ADDR, HWIO_JPEG_BUS_STATUS_RMSK)
#define HWIO_JPEG_BUS_STATUS_INM(m)                              \
        in_dword_masked(HWIO_JPEG_BUS_STATUS_ADDR, m)
#define HWIO_JPEG_BUS_STATUS_SLAVE_ID_ERROR_BMSK                       0x30
#define HWIO_JPEG_BUS_STATUS_SLAVE_ID_ERROR_SHFT                        0x4
#define HWIO_JPEG_BUS_STATUS_BUS_IDLE_BMSK                              0x2
#define HWIO_JPEG_BUS_STATUS_BUS_IDLE_SHFT                              0x1
#define HWIO_JPEG_BUS_STATUS_HALT_ACK_BMSK                              0x1
#define HWIO_JPEG_BUS_STATUS_HALT_ACK_SHFT                                0

#define HWIO_JPEG_BUS_MISR_CFG_ADDR                              (GEMINI_REG_BASE      + 0x0000002c)
#define HWIO_JPEG_BUS_MISR_CFG_PHYS                              (GEMINI_REG_BASE_PHYS + 0x0000002c)
#define HWIO_JPEG_BUS_MISR_CFG_RMSK                                 0x70f0f
#define HWIO_JPEG_BUS_MISR_CFG_SHFT                                       0
#define HWIO_JPEG_BUS_MISR_CFG_IN                                \
        in_dword_masked(HWIO_JPEG_BUS_MISR_CFG_ADDR, HWIO_JPEG_BUS_MISR_CFG_RMSK)
#define HWIO_JPEG_BUS_MISR_CFG_INM(m)                            \
        in_dword_masked(HWIO_JPEG_BUS_MISR_CFG_ADDR, m)
#define HWIO_JPEG_BUS_MISR_CFG_OUT(v)                            \
        out_dword(HWIO_JPEG_BUS_MISR_CFG_ADDR,v)
#define HWIO_JPEG_BUS_MISR_CFG_OUTM(m,v)                         \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_BUS_MISR_CFG_ADDR,m,v,HWIO_JPEG_BUS_MISR_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_BUS_MISR_CFG_MISR_SIG_SEL_BMSK                    0x60000
#define HWIO_JPEG_BUS_MISR_CFG_MISR_SIG_SEL_SHFT                       0x11
#define HWIO_JPEG_BUS_MISR_CFG_MISR_READ_SEL_BMSK                   0x10000
#define HWIO_JPEG_BUS_MISR_CFG_MISR_READ_SEL_SHFT                      0x10
#define HWIO_JPEG_BUS_MISR_CFG_MISR1_SRC_BMSK                         0xc00
#define HWIO_JPEG_BUS_MISR_CFG_MISR1_SRC_SHFT                           0xa
#define HWIO_JPEG_BUS_MISR_CFG_MISR1_INPUT_SEL_BMSK                   0x300
#define HWIO_JPEG_BUS_MISR_CFG_MISR1_INPUT_SEL_SHFT                     0x8
#define HWIO_JPEG_BUS_MISR_CFG_MISR0_SRC_BMSK                           0xc
#define HWIO_JPEG_BUS_MISR_CFG_MISR0_SRC_SHFT                           0x2
#define HWIO_JPEG_BUS_MISR_CFG_MISR0_INPUT_SEL_BMSK                     0x3
#define HWIO_JPEG_BUS_MISR_CFG_MISR0_INPUT_SEL_SHFT                       0

#define HWIO_JPEG_BUS_MISR_READ_VALUE_ADDR                       (GEMINI_REG_BASE      + 0x00000030)
#define HWIO_JPEG_BUS_MISR_READ_VALUE_PHYS                       (GEMINI_REG_BASE_PHYS + 0x00000030)
#define HWIO_JPEG_BUS_MISR_READ_VALUE_RMSK                       0xffffffff
#define HWIO_JPEG_BUS_MISR_READ_VALUE_SHFT                                0
#define HWIO_JPEG_BUS_MISR_READ_VALUE_IN                         \
        in_dword_masked(HWIO_JPEG_BUS_MISR_READ_VALUE_ADDR, HWIO_JPEG_BUS_MISR_READ_VALUE_RMSK)
#define HWIO_JPEG_BUS_MISR_READ_VALUE_INM(m)                     \
        in_dword_masked(HWIO_JPEG_BUS_MISR_READ_VALUE_ADDR, m)
#define HWIO_JPEG_BUS_MISR_READ_VALUE_BUS_MISR_VALUE_BMSK        0xffffffff
#define HWIO_JPEG_BUS_MISR_READ_VALUE_BUS_MISR_VALUE_SHFT                 0

#define HWIO_JPEG_STATUS_ENCODE_OUTPUT_SIZE_ADDR                 (GEMINI_REG_BASE      + 0x00000034)
#define HWIO_JPEG_STATUS_ENCODE_OUTPUT_SIZE_PHYS                 (GEMINI_REG_BASE_PHYS + 0x00000034)
#define HWIO_JPEG_STATUS_ENCODE_OUTPUT_SIZE_RMSK                   0xffffff
#define HWIO_JPEG_STATUS_ENCODE_OUTPUT_SIZE_SHFT                          0
#define HWIO_JPEG_STATUS_ENCODE_OUTPUT_SIZE_IN                   \
        in_dword_masked(HWIO_JPEG_STATUS_ENCODE_OUTPUT_SIZE_ADDR, HWIO_JPEG_STATUS_ENCODE_OUTPUT_SIZE_RMSK)
#define HWIO_JPEG_STATUS_ENCODE_OUTPUT_SIZE_INM(m)               \
        in_dword_masked(HWIO_JPEG_STATUS_ENCODE_OUTPUT_SIZE_ADDR, m)
#define HWIO_JPEG_STATUS_ENCODE_OUTPUT_SIZE_OUTPUT_SIZE_BYTES_BMSK   0xffffff
#define HWIO_JPEG_STATUS_ENCODE_OUTPUT_SIZE_OUTPUT_SIZE_BYTES_SHFT          0

#define HWIO_JPEG_FE_CFG_ADDR                                    (GEMINI_REG_BASE      + 0x00000038)
#define HWIO_JPEG_FE_CFG_PHYS                                    (GEMINI_REG_BASE_PHYS + 0x00000038)
#define HWIO_JPEG_FE_CFG_RMSK                                       0xf0077
#define HWIO_JPEG_FE_CFG_SHFT                                             0
#define HWIO_JPEG_FE_CFG_IN                                      \
        in_dword_masked(HWIO_JPEG_FE_CFG_ADDR, HWIO_JPEG_FE_CFG_RMSK)
#define HWIO_JPEG_FE_CFG_INM(m)                                  \
        in_dword_masked(HWIO_JPEG_FE_CFG_ADDR, m)
#define HWIO_JPEG_FE_CFG_OUT(v)                                  \
        out_dword(HWIO_JPEG_FE_CFG_ADDR,v)
#define HWIO_JPEG_FE_CFG_OUTM(m,v)                               \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_CFG_ADDR,m,v,HWIO_JPEG_FE_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_CFG_FE_AREQP_STARVE_BMSK                       0xc0000
#define HWIO_JPEG_FE_CFG_FE_AREQP_STARVE_SHFT                          0x12
#define HWIO_JPEG_FE_CFG_FE_AREQP_DEFAULT_BMSK                      0x30000
#define HWIO_JPEG_FE_CFG_FE_AREQP_DEFAULT_SHFT                         0x10
#define HWIO_JPEG_FE_CFG_CBCR_ORDER_BMSK                               0x40
#define HWIO_JPEG_FE_CFG_CBCR_ORDER_SHFT                                0x6
#define HWIO_JPEG_FE_CFG_FE_BURST_LENGTH_BMSK                          0x30
#define HWIO_JPEG_FE_CFG_FE_BURST_LENGTH_SHFT                           0x4
#define HWIO_JPEG_FE_CFG_BYTE_ORDERING_BMSK                             0x7
#define HWIO_JPEG_FE_CFG_BYTE_ORDERING_SHFT                               0

#define HWIO_JPEG_FE_FRAME_CFG_ADDR                              (GEMINI_REG_BASE      + 0x0000003c)
#define HWIO_JPEG_FE_FRAME_CFG_PHYS                              (GEMINI_REG_BASE_PHYS + 0x0000003c)
#define HWIO_JPEG_FE_FRAME_CFG_RMSK                               0x1ff01ff
#define HWIO_JPEG_FE_FRAME_CFG_SHFT                                       0
#define HWIO_JPEG_FE_FRAME_CFG_IN                                \
        in_dword_masked(HWIO_JPEG_FE_FRAME_CFG_ADDR, HWIO_JPEG_FE_FRAME_CFG_RMSK)
#define HWIO_JPEG_FE_FRAME_CFG_INM(m)                            \
        in_dword_masked(HWIO_JPEG_FE_FRAME_CFG_ADDR, m)
#define HWIO_JPEG_FE_FRAME_CFG_OUT(v)                            \
        out_dword(HWIO_JPEG_FE_FRAME_CFG_ADDR,v)
#define HWIO_JPEG_FE_FRAME_CFG_OUTM(m,v)                         \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_FRAME_CFG_ADDR,m,v,HWIO_JPEG_FE_FRAME_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_FRAME_CFG_FRAME_HEIGHT_MCUS_BMSK             0x1ff0000
#define HWIO_JPEG_FE_FRAME_CFG_FRAME_HEIGHT_MCUS_SHFT                  0x10
#define HWIO_JPEG_FE_FRAME_CFG_FRAME_WIDTH_MCUS_BMSK                  0x1ff
#define HWIO_JPEG_FE_FRAME_CFG_FRAME_WIDTH_MCUS_SHFT                      0

#define HWIO_JPEG_FE_OUTPUT_CFG_ADDR                             (GEMINI_REG_BASE      + 0x00000040)
#define HWIO_JPEG_FE_OUTPUT_CFG_PHYS                             (GEMINI_REG_BASE_PHYS + 0x00000040)
#define HWIO_JPEG_FE_OUTPUT_CFG_RMSK                                0x107ff
#define HWIO_JPEG_FE_OUTPUT_CFG_SHFT                                      0
#define HWIO_JPEG_FE_OUTPUT_CFG_IN                               \
        in_dword_masked(HWIO_JPEG_FE_OUTPUT_CFG_ADDR, HWIO_JPEG_FE_OUTPUT_CFG_RMSK)
#define HWIO_JPEG_FE_OUTPUT_CFG_INM(m)                           \
        in_dword_masked(HWIO_JPEG_FE_OUTPUT_CFG_ADDR, m)
#define HWIO_JPEG_FE_OUTPUT_CFG_OUT(v)                           \
        out_dword(HWIO_JPEG_FE_OUTPUT_CFG_ADDR,v)
#define HWIO_JPEG_FE_OUTPUT_CFG_OUTM(m,v)                        \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_OUTPUT_CFG_ADDR,m,v,HWIO_JPEG_FE_OUTPUT_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_OUTPUT_CFG_TILE_LINE_FORMAT_BMSK               0x10000
#define HWIO_JPEG_FE_OUTPUT_CFG_TILE_LINE_FORMAT_SHFT                  0x10
#define HWIO_JPEG_FE_OUTPUT_CFG_PERIOD_BMSK                           0x700
#define HWIO_JPEG_FE_OUTPUT_CFG_PERIOD_SHFT                             0x8
#define HWIO_JPEG_FE_OUTPUT_CFG_BLOCK_PATTERN_BMSK                     0xff
#define HWIO_JPEG_FE_OUTPUT_CFG_BLOCK_PATTERN_SHFT                        0

#define HWIO_JPEG_FE_FRAME_ROTATION_CFG_ADDR                     (GEMINI_REG_BASE      + 0x00000044)
#define HWIO_JPEG_FE_FRAME_ROTATION_CFG_PHYS                     (GEMINI_REG_BASE_PHYS + 0x00000044)
#define HWIO_JPEG_FE_FRAME_ROTATION_CFG_RMSK                            0x3
#define HWIO_JPEG_FE_FRAME_ROTATION_CFG_SHFT                              0
#define HWIO_JPEG_FE_FRAME_ROTATION_CFG_IN                       \
        in_dword_masked(HWIO_JPEG_FE_FRAME_ROTATION_CFG_ADDR, HWIO_JPEG_FE_FRAME_ROTATION_CFG_RMSK)
#define HWIO_JPEG_FE_FRAME_ROTATION_CFG_INM(m)                   \
        in_dword_masked(HWIO_JPEG_FE_FRAME_ROTATION_CFG_ADDR, m)
#define HWIO_JPEG_FE_FRAME_ROTATION_CFG_OUT(v)                   \
        out_dword(HWIO_JPEG_FE_FRAME_ROTATION_CFG_ADDR,v)
#define HWIO_JPEG_FE_FRAME_ROTATION_CFG_OUTM(m,v)                \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_FRAME_ROTATION_CFG_ADDR,m,v,HWIO_JPEG_FE_FRAME_ROTATION_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_FRAME_ROTATION_CFG_FE_FRAME_ROTATION_BMSK          0x3
#define HWIO_JPEG_FE_FRAME_ROTATION_CFG_FE_FRAME_ROTATION_SHFT            0

#define HWIO_JPEG_FE_Y_FRAME_ROTATION_START_ADDR                 (GEMINI_REG_BASE      + 0x00000048)
#define HWIO_JPEG_FE_Y_FRAME_ROTATION_START_PHYS                 (GEMINI_REG_BASE_PHYS + 0x00000048)
#define HWIO_JPEG_FE_Y_FRAME_ROTATION_START_RMSK                  0x3ffffff
#define HWIO_JPEG_FE_Y_FRAME_ROTATION_START_SHFT                          0
#define HWIO_JPEG_FE_Y_FRAME_ROTATION_START_IN                   \
        in_dword_masked(HWIO_JPEG_FE_Y_FRAME_ROTATION_START_ADDR, HWIO_JPEG_FE_Y_FRAME_ROTATION_START_RMSK)
#define HWIO_JPEG_FE_Y_FRAME_ROTATION_START_INM(m)               \
        in_dword_masked(HWIO_JPEG_FE_Y_FRAME_ROTATION_START_ADDR, m)
#define HWIO_JPEG_FE_Y_FRAME_ROTATION_START_OUT(v)               \
        out_dword(HWIO_JPEG_FE_Y_FRAME_ROTATION_START_ADDR,v)
#define HWIO_JPEG_FE_Y_FRAME_ROTATION_START_OUTM(m,v)            \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_Y_FRAME_ROTATION_START_ADDR,m,v,HWIO_JPEG_FE_Y_FRAME_ROTATION_START_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_Y_FRAME_ROTATION_START_FE_START_OFFSET_BMSK  0x3ffffff
#define HWIO_JPEG_FE_Y_FRAME_ROTATION_START_FE_START_OFFSET_SHFT          0

#define HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_ADDR              (GEMINI_REG_BASE      + 0x0000004c)
#define HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_PHYS              (GEMINI_REG_BASE_PHYS + 0x0000004c)
#define HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_RMSK               0x3ffffff
#define HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_SHFT                       0
#define HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_IN                \
        in_dword_masked(HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_ADDR, HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_RMSK)
#define HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_INM(m)            \
        in_dword_masked(HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_ADDR, m)
#define HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_OUT(v)            \
        out_dword(HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_ADDR,v)
#define HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_OUTM(m,v)         \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_ADDR,m,v,HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_FE_START_OFFSET_BMSK  0x3ffffff
#define HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_FE_START_OFFSET_SHFT          0

#define HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_ADDR                    (GEMINI_REG_BASE      + 0x00000050)
#define HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_PHYS                    (GEMINI_REG_BASE_PHYS + 0x00000050)
#define HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_RMSK                      0x3ffffff
#define HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_SHFT                             0
#define HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_IN                      \
        in_dword_masked(HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_ADDR, HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_RMSK)
#define HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_INM(m)                  \
        in_dword_masked(HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_ADDR, m)
#define HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_OUT(v)                  \
        out_dword(HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_ADDR,v)
#define HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_OUTM(m,v)               \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_ADDR,m,v,HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_Y_ROW_JUMP_OFFSET_BMSK    0x3ffffff
#define HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_Y_ROW_JUMP_OFFSET_SHFT           0

#define HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_ADDR                 (GEMINI_REG_BASE      + 0x00000054)
#define HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_PHYS                 (GEMINI_REG_BASE_PHYS + 0x00000054)
#define HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_RMSK                   0x3ffffff
#define HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_SHFT                          0
#define HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_IN                   \
        in_dword_masked(HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_ADDR, HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_RMSK)
#define HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_INM(m)               \
        in_dword_masked(HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_ADDR, m)
#define HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_OUT(v)               \
        out_dword(HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_ADDR,v)
#define HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_OUTM(m,v)            \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_ADDR,m,v,HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_CBCR_ROW_JUMP_OFFSET_BMSK   0x3ffffff
#define HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_CBCR_ROW_JUMP_OFFSET_SHFT          0

#define HWIO_JPEG_FE_BLOCK_ROTATION_CFG_ADDR                     (GEMINI_REG_BASE      + 0x00000058)
#define HWIO_JPEG_FE_BLOCK_ROTATION_CFG_PHYS                     (GEMINI_REG_BASE_PHYS + 0x00000058)
#define HWIO_JPEG_FE_BLOCK_ROTATION_CFG_RMSK                      0x33f1f1f
#define HWIO_JPEG_FE_BLOCK_ROTATION_CFG_SHFT                              0
#define HWIO_JPEG_FE_BLOCK_ROTATION_CFG_IN                       \
        in_dword_masked(HWIO_JPEG_FE_BLOCK_ROTATION_CFG_ADDR, HWIO_JPEG_FE_BLOCK_ROTATION_CFG_RMSK)
#define HWIO_JPEG_FE_BLOCK_ROTATION_CFG_INM(m)                   \
        in_dword_masked(HWIO_JPEG_FE_BLOCK_ROTATION_CFG_ADDR, m)
#define HWIO_JPEG_FE_BLOCK_ROTATION_CFG_OUT(v)                   \
        out_dword(HWIO_JPEG_FE_BLOCK_ROTATION_CFG_ADDR,v)
#define HWIO_JPEG_FE_BLOCK_ROTATION_CFG_OUTM(m,v)                \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_BLOCK_ROTATION_CFG_ADDR,m,v,HWIO_JPEG_FE_BLOCK_ROTATION_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_BLOCK_ROTATION_CFG_BLOCK_ROTATION_BMSK       0x3000000
#define HWIO_JPEG_FE_BLOCK_ROTATION_CFG_BLOCK_ROTATION_SHFT            0x18
#define HWIO_JPEG_FE_BLOCK_ROTATION_CFG_PIXEL_START_OFFSET_BMSK    0x3f0000
#define HWIO_JPEG_FE_BLOCK_ROTATION_CFG_PIXEL_START_OFFSET_SHFT        0x10
#define HWIO_JPEG_FE_BLOCK_ROTATION_CFG_PIXEL_H_INCREMENT_BMSK       0x1f00
#define HWIO_JPEG_FE_BLOCK_ROTATION_CFG_PIXEL_H_INCREMENT_SHFT          0x8
#define HWIO_JPEG_FE_BLOCK_ROTATION_CFG_PIXEL_V_INCREMENT_BMSK         0x1f
#define HWIO_JPEG_FE_BLOCK_ROTATION_CFG_PIXEL_V_INCREMENT_SHFT            0

#define HWIO_JPEG_FE_BURST_MASK_ADDR                             (GEMINI_REG_BASE      + 0x0000005c)
#define HWIO_JPEG_FE_BURST_MASK_PHYS                             (GEMINI_REG_BASE_PHYS + 0x0000005c)
#define HWIO_JPEG_FE_BURST_MASK_RMSK                                 0xffff
#define HWIO_JPEG_FE_BURST_MASK_SHFT                                      0
#define HWIO_JPEG_FE_BURST_MASK_IN                               \
        in_dword_masked(HWIO_JPEG_FE_BURST_MASK_ADDR, HWIO_JPEG_FE_BURST_MASK_RMSK)
#define HWIO_JPEG_FE_BURST_MASK_INM(m)                           \
        in_dword_masked(HWIO_JPEG_FE_BURST_MASK_ADDR, m)
#define HWIO_JPEG_FE_BURST_MASK_OUT(v)                           \
        out_dword(HWIO_JPEG_FE_BURST_MASK_ADDR,v)
#define HWIO_JPEG_FE_BURST_MASK_OUTM(m,v)                        \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_BURST_MASK_ADDR,m,v,HWIO_JPEG_FE_BURST_MASK_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_BURST_MASK_CBCR_BURST_MASK_BMSK                 0xff00
#define HWIO_JPEG_FE_BURST_MASK_CBCR_BURST_MASK_SHFT                    0x8
#define HWIO_JPEG_FE_BURST_MASK_Y_BURST_MASK_BMSK                      0xff
#define HWIO_JPEG_FE_BURST_MASK_Y_BURST_MASK_SHFT                         0

#define HWIO_JPEG_FE_Y_WRITE_MASK_0_ADDR                         (GEMINI_REG_BASE      + 0x00000060)
#define HWIO_JPEG_FE_Y_WRITE_MASK_0_PHYS                         (GEMINI_REG_BASE_PHYS + 0x00000060)
#define HWIO_JPEG_FE_Y_WRITE_MASK_0_RMSK                         0xffffffff
#define HWIO_JPEG_FE_Y_WRITE_MASK_0_SHFT                                  0
#define HWIO_JPEG_FE_Y_WRITE_MASK_0_IN                           \
        in_dword_masked(HWIO_JPEG_FE_Y_WRITE_MASK_0_ADDR, HWIO_JPEG_FE_Y_WRITE_MASK_0_RMSK)
#define HWIO_JPEG_FE_Y_WRITE_MASK_0_INM(m)                       \
        in_dword_masked(HWIO_JPEG_FE_Y_WRITE_MASK_0_ADDR, m)
#define HWIO_JPEG_FE_Y_WRITE_MASK_0_OUT(v)                       \
        out_dword(HWIO_JPEG_FE_Y_WRITE_MASK_0_ADDR,v)
#define HWIO_JPEG_FE_Y_WRITE_MASK_0_OUTM(m,v)                    \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_Y_WRITE_MASK_0_ADDR,m,v,HWIO_JPEG_FE_Y_WRITE_MASK_0_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_Y_WRITE_MASK_0_Y_WRITE_MASK_1_BMSK          0xffff0000
#define HWIO_JPEG_FE_Y_WRITE_MASK_0_Y_WRITE_MASK_1_SHFT                0x10
#define HWIO_JPEG_FE_Y_WRITE_MASK_0_Y_WRITE_MASK_0_BMSK              0xffff
#define HWIO_JPEG_FE_Y_WRITE_MASK_0_Y_WRITE_MASK_0_SHFT                   0

#define HWIO_JPEG_FE_Y_WRITE_MASK_1_ADDR                         (GEMINI_REG_BASE      + 0x00000064)
#define HWIO_JPEG_FE_Y_WRITE_MASK_1_PHYS                         (GEMINI_REG_BASE_PHYS + 0x00000064)
#define HWIO_JPEG_FE_Y_WRITE_MASK_1_RMSK                         0xffffffff
#define HWIO_JPEG_FE_Y_WRITE_MASK_1_SHFT                                  0
#define HWIO_JPEG_FE_Y_WRITE_MASK_1_IN                           \
        in_dword_masked(HWIO_JPEG_FE_Y_WRITE_MASK_1_ADDR, HWIO_JPEG_FE_Y_WRITE_MASK_1_RMSK)
#define HWIO_JPEG_FE_Y_WRITE_MASK_1_INM(m)                       \
        in_dword_masked(HWIO_JPEG_FE_Y_WRITE_MASK_1_ADDR, m)
#define HWIO_JPEG_FE_Y_WRITE_MASK_1_OUT(v)                       \
        out_dword(HWIO_JPEG_FE_Y_WRITE_MASK_1_ADDR,v)
#define HWIO_JPEG_FE_Y_WRITE_MASK_1_OUTM(m,v)                    \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_Y_WRITE_MASK_1_ADDR,m,v,HWIO_JPEG_FE_Y_WRITE_MASK_1_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_Y_WRITE_MASK_1_Y_WRITE_MASK_3_BMSK          0xffff0000
#define HWIO_JPEG_FE_Y_WRITE_MASK_1_Y_WRITE_MASK_3_SHFT                0x10
#define HWIO_JPEG_FE_Y_WRITE_MASK_1_Y_WRITE_MASK_2_BMSK              0xffff
#define HWIO_JPEG_FE_Y_WRITE_MASK_1_Y_WRITE_MASK_2_SHFT                   0

#define HWIO_JPEG_FE_Y_WRITE_MASK_2_ADDR                         (GEMINI_REG_BASE      + 0x00000068)
#define HWIO_JPEG_FE_Y_WRITE_MASK_2_PHYS                         (GEMINI_REG_BASE_PHYS + 0x00000068)
#define HWIO_JPEG_FE_Y_WRITE_MASK_2_RMSK                         0xffffffff
#define HWIO_JPEG_FE_Y_WRITE_MASK_2_SHFT                                  0
#define HWIO_JPEG_FE_Y_WRITE_MASK_2_IN                           \
        in_dword_masked(HWIO_JPEG_FE_Y_WRITE_MASK_2_ADDR, HWIO_JPEG_FE_Y_WRITE_MASK_2_RMSK)
#define HWIO_JPEG_FE_Y_WRITE_MASK_2_INM(m)                       \
        in_dword_masked(HWIO_JPEG_FE_Y_WRITE_MASK_2_ADDR, m)
#define HWIO_JPEG_FE_Y_WRITE_MASK_2_OUT(v)                       \
        out_dword(HWIO_JPEG_FE_Y_WRITE_MASK_2_ADDR,v)
#define HWIO_JPEG_FE_Y_WRITE_MASK_2_OUTM(m,v)                    \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_Y_WRITE_MASK_2_ADDR,m,v,HWIO_JPEG_FE_Y_WRITE_MASK_2_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_Y_WRITE_MASK_2_Y_WRITE_MASK_5_BMSK          0xffff0000
#define HWIO_JPEG_FE_Y_WRITE_MASK_2_Y_WRITE_MASK_5_SHFT                0x10
#define HWIO_JPEG_FE_Y_WRITE_MASK_2_Y_WRITE_MASK_4_BMSK              0xffff
#define HWIO_JPEG_FE_Y_WRITE_MASK_2_Y_WRITE_MASK_4_SHFT                   0

#define HWIO_JPEG_FE_Y_WRITE_MASK_3_ADDR                         (GEMINI_REG_BASE      + 0x0000006c)
#define HWIO_JPEG_FE_Y_WRITE_MASK_3_PHYS                         (GEMINI_REG_BASE_PHYS + 0x0000006c)
#define HWIO_JPEG_FE_Y_WRITE_MASK_3_RMSK                         0xffffffff
#define HWIO_JPEG_FE_Y_WRITE_MASK_3_SHFT                                  0
#define HWIO_JPEG_FE_Y_WRITE_MASK_3_IN                           \
        in_dword_masked(HWIO_JPEG_FE_Y_WRITE_MASK_3_ADDR, HWIO_JPEG_FE_Y_WRITE_MASK_3_RMSK)
#define HWIO_JPEG_FE_Y_WRITE_MASK_3_INM(m)                       \
        in_dword_masked(HWIO_JPEG_FE_Y_WRITE_MASK_3_ADDR, m)
#define HWIO_JPEG_FE_Y_WRITE_MASK_3_OUT(v)                       \
        out_dword(HWIO_JPEG_FE_Y_WRITE_MASK_3_ADDR,v)
#define HWIO_JPEG_FE_Y_WRITE_MASK_3_OUTM(m,v)                    \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_Y_WRITE_MASK_3_ADDR,m,v,HWIO_JPEG_FE_Y_WRITE_MASK_3_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_Y_WRITE_MASK_3_Y_WRITE_MASK_7_BMSK          0xffff0000
#define HWIO_JPEG_FE_Y_WRITE_MASK_3_Y_WRITE_MASK_7_SHFT                0x10
#define HWIO_JPEG_FE_Y_WRITE_MASK_3_Y_WRITE_MASK_6_BMSK              0xffff
#define HWIO_JPEG_FE_Y_WRITE_MASK_3_Y_WRITE_MASK_6_SHFT                   0

#define HWIO_JPEG_FE_CBCR_WRITE_MASK_0_ADDR                      (GEMINI_REG_BASE      + 0x00000070)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_0_PHYS                      (GEMINI_REG_BASE_PHYS + 0x00000070)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_0_RMSK                      0xffffffff
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_0_SHFT                               0
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_0_IN                        \
        in_dword_masked(HWIO_JPEG_FE_CBCR_WRITE_MASK_0_ADDR, HWIO_JPEG_FE_CBCR_WRITE_MASK_0_RMSK)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_0_INM(m)                    \
        in_dword_masked(HWIO_JPEG_FE_CBCR_WRITE_MASK_0_ADDR, m)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_0_OUT(v)                    \
        out_dword(HWIO_JPEG_FE_CBCR_WRITE_MASK_0_ADDR,v)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_0_OUTM(m,v)                 \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_CBCR_WRITE_MASK_0_ADDR,m,v,HWIO_JPEG_FE_CBCR_WRITE_MASK_0_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_0_CBCR_WRITE_MASK_1_BMSK    0xffff0000
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_0_CBCR_WRITE_MASK_1_SHFT          0x10
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_0_CBCR_WRITE_MASK_0_BMSK        0xffff
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_0_CBCR_WRITE_MASK_0_SHFT             0

#define HWIO_JPEG_FE_CBCR_WRITE_MASK_1_ADDR                      (GEMINI_REG_BASE      + 0x00000074)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_1_PHYS                      (GEMINI_REG_BASE_PHYS + 0x00000074)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_1_RMSK                      0xffffffff
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_1_SHFT                               0
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_1_IN                        \
        in_dword_masked(HWIO_JPEG_FE_CBCR_WRITE_MASK_1_ADDR, HWIO_JPEG_FE_CBCR_WRITE_MASK_1_RMSK)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_1_INM(m)                    \
        in_dword_masked(HWIO_JPEG_FE_CBCR_WRITE_MASK_1_ADDR, m)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_1_OUT(v)                    \
        out_dword(HWIO_JPEG_FE_CBCR_WRITE_MASK_1_ADDR,v)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_1_OUTM(m,v)                 \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_CBCR_WRITE_MASK_1_ADDR,m,v,HWIO_JPEG_FE_CBCR_WRITE_MASK_1_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_1_CBCR_WRITE_MASK_3_BMSK    0xffff0000
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_1_CBCR_WRITE_MASK_3_SHFT          0x10
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_1_CBCR_WRITE_MASK_2_BMSK        0xffff
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_1_CBCR_WRITE_MASK_2_SHFT             0

#define HWIO_JPEG_FE_CBCR_WRITE_MASK_2_ADDR                      (GEMINI_REG_BASE      + 0x00000078)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_2_PHYS                      (GEMINI_REG_BASE_PHYS + 0x00000078)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_2_RMSK                      0xffffffff
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_2_SHFT                               0
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_2_IN                        \
        in_dword_masked(HWIO_JPEG_FE_CBCR_WRITE_MASK_2_ADDR, HWIO_JPEG_FE_CBCR_WRITE_MASK_2_RMSK)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_2_INM(m)                    \
        in_dword_masked(HWIO_JPEG_FE_CBCR_WRITE_MASK_2_ADDR, m)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_2_OUT(v)                    \
        out_dword(HWIO_JPEG_FE_CBCR_WRITE_MASK_2_ADDR,v)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_2_OUTM(m,v)                 \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_CBCR_WRITE_MASK_2_ADDR,m,v,HWIO_JPEG_FE_CBCR_WRITE_MASK_2_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_2_CBCR_WRITE_MASK_5_BMSK    0xffff0000
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_2_CBCR_WRITE_MASK_5_SHFT          0x10
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_2_CBCR_WRITE_MASK_4_BMSK        0xffff
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_2_CBCR_WRITE_MASK_4_SHFT             0

#define HWIO_JPEG_FE_CBCR_WRITE_MASK_3_ADDR                      (GEMINI_REG_BASE      + 0x0000007c)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_3_PHYS                      (GEMINI_REG_BASE_PHYS + 0x0000007c)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_3_RMSK                      0xffffffff
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_3_SHFT                               0
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_3_IN                        \
        in_dword_masked(HWIO_JPEG_FE_CBCR_WRITE_MASK_3_ADDR, HWIO_JPEG_FE_CBCR_WRITE_MASK_3_RMSK)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_3_INM(m)                    \
        in_dword_masked(HWIO_JPEG_FE_CBCR_WRITE_MASK_3_ADDR, m)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_3_OUT(v)                    \
        out_dword(HWIO_JPEG_FE_CBCR_WRITE_MASK_3_ADDR,v)
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_3_OUTM(m,v)                 \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_CBCR_WRITE_MASK_3_ADDR,m,v,HWIO_JPEG_FE_CBCR_WRITE_MASK_3_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_3_CBCR_WRITE_MASK_7_BMSK    0xffff0000
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_3_CBCR_WRITE_MASK_7_SHFT          0x10
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_3_CBCR_WRITE_MASK_6_BMSK        0xffff
#define HWIO_JPEG_FE_CBCR_WRITE_MASK_3_CBCR_WRITE_MASK_6_SHFT             0

#define HWIO_JPEG_FE_BUFFER_CFG_ADDR                             (GEMINI_REG_BASE      + 0x00000080)
#define HWIO_JPEG_FE_BUFFER_CFG_PHYS                             (GEMINI_REG_BASE_PHYS + 0x00000080)
#define HWIO_JPEG_FE_BUFFER_CFG_RMSK                             0x1fff1fff
#define HWIO_JPEG_FE_BUFFER_CFG_SHFT                                      0
#define HWIO_JPEG_FE_BUFFER_CFG_IN                               \
        in_dword_masked(HWIO_JPEG_FE_BUFFER_CFG_ADDR, HWIO_JPEG_FE_BUFFER_CFG_RMSK)
#define HWIO_JPEG_FE_BUFFER_CFG_INM(m)                           \
        in_dword_masked(HWIO_JPEG_FE_BUFFER_CFG_ADDR, m)
#define HWIO_JPEG_FE_BUFFER_CFG_OUT(v)                           \
        out_dword(HWIO_JPEG_FE_BUFFER_CFG_ADDR,v)
#define HWIO_JPEG_FE_BUFFER_CFG_OUTM(m,v)                        \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_BUFFER_CFG_ADDR,m,v,HWIO_JPEG_FE_BUFFER_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_BUFFER_CFG_CBCR_MCU_ROWS_BMSK               0x1fff0000
#define HWIO_JPEG_FE_BUFFER_CFG_CBCR_MCU_ROWS_SHFT                     0x10
#define HWIO_JPEG_FE_BUFFER_CFG_Y_MCU_ROWS_BMSK                      0x1fff
#define HWIO_JPEG_FE_BUFFER_CFG_Y_MCU_ROWS_SHFT                           0

#define HWIO_JPEG_FE_Y_PING_ADDR_ADDR                            (GEMINI_REG_BASE      + 0x00000084)
#define HWIO_JPEG_FE_Y_PING_ADDR_PHYS                            (GEMINI_REG_BASE_PHYS + 0x00000084)
#define HWIO_JPEG_FE_Y_PING_ADDR_RMSK                            0xffffffff
#define HWIO_JPEG_FE_Y_PING_ADDR_SHFT                                     0
#define HWIO_JPEG_FE_Y_PING_ADDR_IN                              \
        in_dword_masked(HWIO_JPEG_FE_Y_PING_ADDR_ADDR, HWIO_JPEG_FE_Y_PING_ADDR_RMSK)
#define HWIO_JPEG_FE_Y_PING_ADDR_INM(m)                          \
        in_dword_masked(HWIO_JPEG_FE_Y_PING_ADDR_ADDR, m)
#define HWIO_JPEG_FE_Y_PING_ADDR_OUT(v)                          \
        out_dword(HWIO_JPEG_FE_Y_PING_ADDR_ADDR,v)
#define HWIO_JPEG_FE_Y_PING_ADDR_OUTM(m,v)                       \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_Y_PING_ADDR_ADDR,m,v,HWIO_JPEG_FE_Y_PING_ADDR_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_Y_PING_ADDR_FE_Y_PING_START_ADDR_BMSK       0xffffffff
#define HWIO_JPEG_FE_Y_PING_ADDR_FE_Y_PING_START_ADDR_SHFT                0

#define HWIO_JPEG_FE_Y_PONG_ADDR_ADDR                            (GEMINI_REG_BASE      + 0x00000088)
#define HWIO_JPEG_FE_Y_PONG_ADDR_PHYS                            (GEMINI_REG_BASE_PHYS + 0x00000088)
#define HWIO_JPEG_FE_Y_PONG_ADDR_RMSK                            0xffffffff
#define HWIO_JPEG_FE_Y_PONG_ADDR_SHFT                                     0
#define HWIO_JPEG_FE_Y_PONG_ADDR_IN                              \
        in_dword_masked(HWIO_JPEG_FE_Y_PONG_ADDR_ADDR, HWIO_JPEG_FE_Y_PONG_ADDR_RMSK)
#define HWIO_JPEG_FE_Y_PONG_ADDR_INM(m)                          \
        in_dword_masked(HWIO_JPEG_FE_Y_PONG_ADDR_ADDR, m)
#define HWIO_JPEG_FE_Y_PONG_ADDR_OUT(v)                          \
        out_dword(HWIO_JPEG_FE_Y_PONG_ADDR_ADDR,v)
#define HWIO_JPEG_FE_Y_PONG_ADDR_OUTM(m,v)                       \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_Y_PONG_ADDR_ADDR,m,v,HWIO_JPEG_FE_Y_PONG_ADDR_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_Y_PONG_ADDR_FE_Y_PONG_START_ADDR_BMSK       0xffffffff
#define HWIO_JPEG_FE_Y_PONG_ADDR_FE_Y_PONG_START_ADDR_SHFT                0

#define HWIO_JPEG_FE_CBCR_PING_ADDR_ADDR                         (GEMINI_REG_BASE      + 0x0000008c)
#define HWIO_JPEG_FE_CBCR_PING_ADDR_PHYS                         (GEMINI_REG_BASE_PHYS + 0x0000008c)
#define HWIO_JPEG_FE_CBCR_PING_ADDR_RMSK                         0xffffffff
#define HWIO_JPEG_FE_CBCR_PING_ADDR_SHFT                                  0
#define HWIO_JPEG_FE_CBCR_PING_ADDR_IN                           \
        in_dword_masked(HWIO_JPEG_FE_CBCR_PING_ADDR_ADDR, HWIO_JPEG_FE_CBCR_PING_ADDR_RMSK)
#define HWIO_JPEG_FE_CBCR_PING_ADDR_INM(m)                       \
        in_dword_masked(HWIO_JPEG_FE_CBCR_PING_ADDR_ADDR, m)
#define HWIO_JPEG_FE_CBCR_PING_ADDR_OUT(v)                       \
        out_dword(HWIO_JPEG_FE_CBCR_PING_ADDR_ADDR,v)
#define HWIO_JPEG_FE_CBCR_PING_ADDR_OUTM(m,v)                    \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_CBCR_PING_ADDR_ADDR,m,v,HWIO_JPEG_FE_CBCR_PING_ADDR_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_CBCR_PING_ADDR_FE_CBCR_PING_START_ADDR_BMSK 0xffffffff
#define HWIO_JPEG_FE_CBCR_PING_ADDR_FE_CBCR_PING_START_ADDR_SHFT          0

#define HWIO_JPEG_FE_CBCR_PONG_ADDR_ADDR                         (GEMINI_REG_BASE      + 0x00000090)
#define HWIO_JPEG_FE_CBCR_PONG_ADDR_PHYS                         (GEMINI_REG_BASE_PHYS + 0x00000090)
#define HWIO_JPEG_FE_CBCR_PONG_ADDR_RMSK                         0xffffffff
#define HWIO_JPEG_FE_CBCR_PONG_ADDR_SHFT                                  0
#define HWIO_JPEG_FE_CBCR_PONG_ADDR_IN                           \
        in_dword_masked(HWIO_JPEG_FE_CBCR_PONG_ADDR_ADDR, HWIO_JPEG_FE_CBCR_PONG_ADDR_RMSK)
#define HWIO_JPEG_FE_CBCR_PONG_ADDR_INM(m)                       \
        in_dword_masked(HWIO_JPEG_FE_CBCR_PONG_ADDR_ADDR, m)
#define HWIO_JPEG_FE_CBCR_PONG_ADDR_OUT(v)                       \
        out_dword(HWIO_JPEG_FE_CBCR_PONG_ADDR_ADDR,v)
#define HWIO_JPEG_FE_CBCR_PONG_ADDR_OUTM(m,v)                    \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_FE_CBCR_PONG_ADDR_ADDR,m,v,HWIO_JPEG_FE_CBCR_PONG_ADDR_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_FE_CBCR_PONG_ADDR_FE_CBCR_PONG_START_ADDR_BMSK 0xffffffff
#define HWIO_JPEG_FE_CBCR_PONG_ADDR_FE_CBCR_PONG_START_ADDR_SHFT          0

#define HWIO_JPEG_FE_CMD_ADDR                                    (GEMINI_REG_BASE      + 0x00000094)
#define HWIO_JPEG_FE_CMD_PHYS                                    (GEMINI_REG_BASE_PHYS + 0x00000094)
#define HWIO_JPEG_FE_CMD_RMSK                                           0x3
#define HWIO_JPEG_FE_CMD_SHFT                                             0
#define HWIO_JPEG_FE_CMD_OUT(v)                                  \
        out_dword(HWIO_JPEG_FE_CMD_ADDR,v)
#define HWIO_JPEG_FE_CMD_OUTM(m,v)                               \
        out_dword_masked(HWIO_JPEG_FE_CMD_ADDR,m,v,HWIO_JPEG_FE_CMD_shadow)
#define HWIO_JPEG_FE_CMD_FE_READ_GO_CMD_BMSK                            0x2
#define HWIO_JPEG_FE_CMD_FE_READ_GO_CMD_SHFT                            0x1
#define HWIO_JPEG_FE_CMD_FE_BUFFER_RELOAD_CMD_BMSK                      0x1
#define HWIO_JPEG_FE_CMD_FE_BUFFER_RELOAD_CMD_SHFT                        0

#define HWIO_JPEG_WE_CFG_ADDR                                    (GEMINI_REG_BASE      + 0x00000098)
#define HWIO_JPEG_WE_CFG_PHYS                                    (GEMINI_REG_BASE_PHYS + 0x00000098)
#define HWIO_JPEG_WE_CFG_RMSK                                       0xf0f37
#define HWIO_JPEG_WE_CFG_SHFT                                             0
#define HWIO_JPEG_WE_CFG_IN                                      \
        in_dword_masked(HWIO_JPEG_WE_CFG_ADDR, HWIO_JPEG_WE_CFG_RMSK)
#define HWIO_JPEG_WE_CFG_INM(m)                                  \
        in_dword_masked(HWIO_JPEG_WE_CFG_ADDR, m)
#define HWIO_JPEG_WE_CFG_OUT(v)                                  \
        out_dword(HWIO_JPEG_WE_CFG_ADDR,v)
#define HWIO_JPEG_WE_CFG_OUTM(m,v)                               \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_CFG_ADDR,m,v,HWIO_JPEG_WE_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_CFG_WE_AREQP_STARVE_BMSK                       0xc0000
#define HWIO_JPEG_WE_CFG_WE_AREQP_STARVE_SHFT                          0x12
#define HWIO_JPEG_WE_CFG_WE_AREQP_DEFAULT_BMSK                      0x30000
#define HWIO_JPEG_WE_CFG_WE_AREQP_DEFAULT_SHFT                         0x10
#define HWIO_JPEG_WE_CFG_WE_OUTPUT_DISABLE_BMSK                       0x800
#define HWIO_JPEG_WE_CFG_WE_OUTPUT_DISABLE_SHFT                         0xb
#define HWIO_JPEG_WE_CFG_WE_BS_DISABLE_BMSK                           0x400
#define HWIO_JPEG_WE_CFG_WE_BS_DISABLE_SHFT                             0xa
#define HWIO_JPEG_WE_CFG_WE_BP_INPUT_SEL_BMSK                         0x300
#define HWIO_JPEG_WE_CFG_WE_BP_INPUT_SEL_SHFT                           0x8
#define HWIO_JPEG_WE_CFG_WE_BURST_LENGTH_BMSK                          0x30
#define HWIO_JPEG_WE_CFG_WE_BURST_LENGTH_SHFT                           0x4
#define HWIO_JPEG_WE_CFG_BYTE_ORDERING_BMSK                             0x7
#define HWIO_JPEG_WE_CFG_BYTE_ORDERING_SHFT                               0

#define HWIO_JPEG_WE_FRAME_ROTATION_CFG_ADDR                     (GEMINI_REG_BASE      + 0x0000009c)
#define HWIO_JPEG_WE_FRAME_ROTATION_CFG_PHYS                     (GEMINI_REG_BASE_PHYS + 0x0000009c)
#define HWIO_JPEG_WE_FRAME_ROTATION_CFG_RMSK                            0x3
#define HWIO_JPEG_WE_FRAME_ROTATION_CFG_SHFT                              0
#define HWIO_JPEG_WE_FRAME_ROTATION_CFG_IN                       \
        in_dword_masked(HWIO_JPEG_WE_FRAME_ROTATION_CFG_ADDR, HWIO_JPEG_WE_FRAME_ROTATION_CFG_RMSK)
#define HWIO_JPEG_WE_FRAME_ROTATION_CFG_INM(m)                   \
        in_dword_masked(HWIO_JPEG_WE_FRAME_ROTATION_CFG_ADDR, m)
#define HWIO_JPEG_WE_FRAME_ROTATION_CFG_OUT(v)                   \
        out_dword(HWIO_JPEG_WE_FRAME_ROTATION_CFG_ADDR,v)
#define HWIO_JPEG_WE_FRAME_ROTATION_CFG_OUTM(m,v)                \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_FRAME_ROTATION_CFG_ADDR,m,v,HWIO_JPEG_WE_FRAME_ROTATION_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_FRAME_ROTATION_CFG_WE_FRAME_ROTATION_BMSK          0x3
#define HWIO_JPEG_WE_FRAME_ROTATION_CFG_WE_FRAME_ROTATION_SHFT            0

#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START0_ADDR                (GEMINI_REG_BASE      + 0x000000a0)
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START0_PHYS                (GEMINI_REG_BASE_PHYS + 0x000000a0)
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START0_RMSK                  0xfffff8
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START0_SHFT                         0
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START0_IN                  \
        in_dword_masked(HWIO_JPEG_WE_Y_FRAME_ROTATION_START0_ADDR, HWIO_JPEG_WE_Y_FRAME_ROTATION_START0_RMSK)
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START0_INM(m)              \
        in_dword_masked(HWIO_JPEG_WE_Y_FRAME_ROTATION_START0_ADDR, m)
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START0_OUT(v)              \
        out_dword(HWIO_JPEG_WE_Y_FRAME_ROTATION_START0_ADDR,v)
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START0_OUTM(m,v)           \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_Y_FRAME_ROTATION_START0_ADDR,m,v,HWIO_JPEG_WE_Y_FRAME_ROTATION_START0_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START0_WE_Y_START_OFFSET0_BMSK   0xfffff8
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START0_WE_Y_START_OFFSET0_SHFT        0x3

#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START1_ADDR                (GEMINI_REG_BASE      + 0x000000a4)
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START1_PHYS                (GEMINI_REG_BASE_PHYS + 0x000000a4)
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START1_RMSK                  0xfffff8
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START1_SHFT                         0
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START1_IN                  \
        in_dword_masked(HWIO_JPEG_WE_Y_FRAME_ROTATION_START1_ADDR, HWIO_JPEG_WE_Y_FRAME_ROTATION_START1_RMSK)
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START1_INM(m)              \
        in_dword_masked(HWIO_JPEG_WE_Y_FRAME_ROTATION_START1_ADDR, m)
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START1_OUT(v)              \
        out_dword(HWIO_JPEG_WE_Y_FRAME_ROTATION_START1_ADDR,v)
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START1_OUTM(m,v)           \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_Y_FRAME_ROTATION_START1_ADDR,m,v,HWIO_JPEG_WE_Y_FRAME_ROTATION_START1_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START1_WE_Y_START_OFFSET1_BMSK   0xfffff8
#define HWIO_JPEG_WE_Y_FRAME_ROTATION_START1_WE_Y_START_OFFSET1_SHFT        0x3

#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START0_ADDR             (GEMINI_REG_BASE      + 0x000000a8)
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START0_PHYS             (GEMINI_REG_BASE_PHYS + 0x000000a8)
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START0_RMSK               0xfffff8
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START0_SHFT                      0
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START0_IN               \
        in_dword_masked(HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START0_ADDR, HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START0_RMSK)
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START0_INM(m)           \
        in_dword_masked(HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START0_ADDR, m)
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START0_OUT(v)           \
        out_dword(HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START0_ADDR,v)
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START0_OUTM(m,v)        \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START0_ADDR,m,v,HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START0_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START0_WE_CBCR_START_OFFSET0_BMSK   0xfffff8
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START0_WE_CBCR_START_OFFSET0_SHFT        0x3

#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START1_ADDR             (GEMINI_REG_BASE      + 0x000000ac)
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START1_PHYS             (GEMINI_REG_BASE_PHYS + 0x000000ac)
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START1_RMSK               0xfffff8
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START1_SHFT                      0
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START1_IN               \
        in_dword_masked(HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START1_ADDR, HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START1_RMSK)
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START1_INM(m)           \
        in_dword_masked(HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START1_ADDR, m)
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START1_OUT(v)           \
        out_dword(HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START1_ADDR,v)
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START1_OUTM(m,v)        \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START1_ADDR,m,v,HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START1_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START1_WE_CBCR_START_OFFSET1_BMSK   0xfffff8
#define HWIO_JPEG_WE_CBCR_FRAME_ROTATION_START1_WE_CBCR_START_OFFSET1_SHFT        0x3

#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET0_ADDR                   (GEMINI_REG_BASE      + 0x000000b0)
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET0_PHYS                   (GEMINI_REG_BASE_PHYS + 0x000000b0)
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET0_RMSK                     0xfffff8
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET0_SHFT                            0
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET0_IN                     \
        in_dword_masked(HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET0_ADDR, HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET0_RMSK)
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET0_INM(m)                 \
        in_dword_masked(HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET0_ADDR, m)
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET0_OUT(v)                 \
        out_dword(HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET0_ADDR,v)
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET0_OUTM(m,v)              \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET0_ADDR,m,v,HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET0_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET0_WE_Y_JUMP_OFFSET0_BMSK   0xfffff8
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET0_WE_Y_JUMP_OFFSET0_SHFT        0x3

#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET1_ADDR                   (GEMINI_REG_BASE      + 0x000000b4)
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET1_PHYS                   (GEMINI_REG_BASE_PHYS + 0x000000b4)
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET1_RMSK                     0xfffff8
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET1_SHFT                            0
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET1_IN                     \
        in_dword_masked(HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET1_ADDR, HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET1_RMSK)
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET1_INM(m)                 \
        in_dword_masked(HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET1_ADDR, m)
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET1_OUT(v)                 \
        out_dword(HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET1_ADDR,v)
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET1_OUTM(m,v)              \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET1_ADDR,m,v,HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET1_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET1_WE_Y_JUMP_OFFSET1_BMSK   0xfffff8
#define HWIO_JPEG_WE_Y_FRAME_JUMP_OFFSET1_WE_Y_JUMP_OFFSET1_SHFT        0x3

#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET0_ADDR                (GEMINI_REG_BASE      + 0x000000b8)
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET0_PHYS                (GEMINI_REG_BASE_PHYS + 0x000000b8)
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET0_RMSK                  0xfffff8
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET0_SHFT                         0
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET0_IN                  \
        in_dword_masked(HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET0_ADDR, HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET0_RMSK)
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET0_INM(m)              \
        in_dword_masked(HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET0_ADDR, m)
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET0_OUT(v)              \
        out_dword(HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET0_ADDR,v)
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET0_OUTM(m,v)           \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET0_ADDR,m,v,HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET0_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET0_WE_CBCR_JUMP_OFFSET0_BMSK   0xfffff8
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET0_WE_CBCR_JUMP_OFFSET0_SHFT        0x3

#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET1_ADDR                (GEMINI_REG_BASE      + 0x000000bc)
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET1_PHYS                (GEMINI_REG_BASE_PHYS + 0x000000bc)
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET1_RMSK                  0xfffff8
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET1_SHFT                         0
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET1_IN                  \
        in_dword_masked(HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET1_ADDR, HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET1_RMSK)
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET1_INM(m)              \
        in_dword_masked(HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET1_ADDR, m)
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET1_OUT(v)              \
        out_dword(HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET1_ADDR,v)
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET1_OUTM(m,v)           \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET1_ADDR,m,v,HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET1_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET1_WE_CBCR_JUMP_OFFSET1_BMSK   0xfffff8
#define HWIO_JPEG_WE_CBCR_FRAME_JUMP_OFFSET1_WE_CBCR_JUMP_OFFSET1_SHFT        0x3

#define HWIO_JPEG_WE_Y_THRESHOLD_ADDR                            (GEMINI_REG_BASE      + 0x000000c0)
#define HWIO_JPEG_WE_Y_THRESHOLD_PHYS                            (GEMINI_REG_BASE_PHYS + 0x000000c0)
#define HWIO_JPEG_WE_Y_THRESHOLD_RMSK                             0x1ff01ff
#define HWIO_JPEG_WE_Y_THRESHOLD_SHFT                                     0
#define HWIO_JPEG_WE_Y_THRESHOLD_IN                              \
        in_dword_masked(HWIO_JPEG_WE_Y_THRESHOLD_ADDR, HWIO_JPEG_WE_Y_THRESHOLD_RMSK)
#define HWIO_JPEG_WE_Y_THRESHOLD_INM(m)                          \
        in_dword_masked(HWIO_JPEG_WE_Y_THRESHOLD_ADDR, m)
#define HWIO_JPEG_WE_Y_THRESHOLD_OUT(v)                          \
        out_dword(HWIO_JPEG_WE_Y_THRESHOLD_ADDR,v)
#define HWIO_JPEG_WE_Y_THRESHOLD_OUTM(m,v)                       \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_Y_THRESHOLD_ADDR,m,v,HWIO_JPEG_WE_Y_THRESHOLD_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_Y_THRESHOLD_WE_DEASSERT_STALL_TH_BMSK        0x1ff0000
#define HWIO_JPEG_WE_Y_THRESHOLD_WE_DEASSERT_STALL_TH_SHFT             0x10
#define HWIO_JPEG_WE_Y_THRESHOLD_WE_ASSERT_STALL_TH_BMSK              0x1ff
#define HWIO_JPEG_WE_Y_THRESHOLD_WE_ASSERT_STALL_TH_SHFT                  0

#define HWIO_JPEG_WE_CBCR_THRESHOLD_ADDR                         (GEMINI_REG_BASE      + 0x000000c4)
#define HWIO_JPEG_WE_CBCR_THRESHOLD_PHYS                         (GEMINI_REG_BASE_PHYS + 0x000000c4)
#define HWIO_JPEG_WE_CBCR_THRESHOLD_RMSK                          0x1ff01ff
#define HWIO_JPEG_WE_CBCR_THRESHOLD_SHFT                                  0
#define HWIO_JPEG_WE_CBCR_THRESHOLD_IN                           \
        in_dword_masked(HWIO_JPEG_WE_CBCR_THRESHOLD_ADDR, HWIO_JPEG_WE_CBCR_THRESHOLD_RMSK)
#define HWIO_JPEG_WE_CBCR_THRESHOLD_INM(m)                       \
        in_dword_masked(HWIO_JPEG_WE_CBCR_THRESHOLD_ADDR, m)
#define HWIO_JPEG_WE_CBCR_THRESHOLD_OUT(v)                       \
        out_dword(HWIO_JPEG_WE_CBCR_THRESHOLD_ADDR,v)
#define HWIO_JPEG_WE_CBCR_THRESHOLD_OUTM(m,v)                    \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_CBCR_THRESHOLD_ADDR,m,v,HWIO_JPEG_WE_CBCR_THRESHOLD_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_CBCR_THRESHOLD_WE_DEASSERT_STALL_TH_BMSK     0x1ff0000
#define HWIO_JPEG_WE_CBCR_THRESHOLD_WE_DEASSERT_STALL_TH_SHFT          0x10
#define HWIO_JPEG_WE_CBCR_THRESHOLD_WE_ASSERT_STALL_TH_BMSK           0x1ff
#define HWIO_JPEG_WE_CBCR_THRESHOLD_WE_ASSERT_STALL_TH_SHFT               0

#define HWIO_JPEG_WE_Y_PING_BUFFER_CFG_ADDR                      (GEMINI_REG_BASE      + 0x000000c8)
#define HWIO_JPEG_WE_Y_PING_BUFFER_CFG_PHYS                      (GEMINI_REG_BASE_PHYS + 0x000000c8)
#define HWIO_JPEG_WE_Y_PING_BUFFER_CFG_RMSK                        0x7fffff
#define HWIO_JPEG_WE_Y_PING_BUFFER_CFG_SHFT                               0
#define HWIO_JPEG_WE_Y_PING_BUFFER_CFG_IN                        \
        in_dword_masked(HWIO_JPEG_WE_Y_PING_BUFFER_CFG_ADDR, HWIO_JPEG_WE_Y_PING_BUFFER_CFG_RMSK)
#define HWIO_JPEG_WE_Y_PING_BUFFER_CFG_INM(m)                    \
        in_dword_masked(HWIO_JPEG_WE_Y_PING_BUFFER_CFG_ADDR, m)
#define HWIO_JPEG_WE_Y_PING_BUFFER_CFG_OUT(v)                    \
        out_dword(HWIO_JPEG_WE_Y_PING_BUFFER_CFG_ADDR,v)
#define HWIO_JPEG_WE_Y_PING_BUFFER_CFG_OUTM(m,v)                 \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_Y_PING_BUFFER_CFG_ADDR,m,v,HWIO_JPEG_WE_Y_PING_BUFFER_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_Y_PING_BUFFER_CFG_WE_BUFFER_LENGTH_BMSK       0x7fffff
#define HWIO_JPEG_WE_Y_PING_BUFFER_CFG_WE_BUFFER_LENGTH_SHFT              0

#define HWIO_JPEG_WE_Y_PONG_BUFFER_CFG_ADDR                      (GEMINI_REG_BASE      + 0x000000cc)
#define HWIO_JPEG_WE_Y_PONG_BUFFER_CFG_PHYS                      (GEMINI_REG_BASE_PHYS + 0x000000cc)
#define HWIO_JPEG_WE_Y_PONG_BUFFER_CFG_RMSK                        0x7fffff
#define HWIO_JPEG_WE_Y_PONG_BUFFER_CFG_SHFT                               0
#define HWIO_JPEG_WE_Y_PONG_BUFFER_CFG_IN                        \
        in_dword_masked(HWIO_JPEG_WE_Y_PONG_BUFFER_CFG_ADDR, HWIO_JPEG_WE_Y_PONG_BUFFER_CFG_RMSK)
#define HWIO_JPEG_WE_Y_PONG_BUFFER_CFG_INM(m)                    \
        in_dword_masked(HWIO_JPEG_WE_Y_PONG_BUFFER_CFG_ADDR, m)
#define HWIO_JPEG_WE_Y_PONG_BUFFER_CFG_OUT(v)                    \
        out_dword(HWIO_JPEG_WE_Y_PONG_BUFFER_CFG_ADDR,v)
#define HWIO_JPEG_WE_Y_PONG_BUFFER_CFG_OUTM(m,v)                 \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_Y_PONG_BUFFER_CFG_ADDR,m,v,HWIO_JPEG_WE_Y_PONG_BUFFER_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_Y_PONG_BUFFER_CFG_WE_BUFFER_LENGTH_BMSK       0x7fffff
#define HWIO_JPEG_WE_Y_PONG_BUFFER_CFG_WE_BUFFER_LENGTH_SHFT              0

#define HWIO_JPEG_WE_CBCR_PING_BUFFER_CFG_ADDR                   (GEMINI_REG_BASE      + 0x000000d0)
#define HWIO_JPEG_WE_CBCR_PING_BUFFER_CFG_PHYS                   (GEMINI_REG_BASE_PHYS + 0x000000d0)
#define HWIO_JPEG_WE_CBCR_PING_BUFFER_CFG_RMSK                     0x7fffff
#define HWIO_JPEG_WE_CBCR_PING_BUFFER_CFG_SHFT                            0
#define HWIO_JPEG_WE_CBCR_PING_BUFFER_CFG_IN                     \
        in_dword_masked(HWIO_JPEG_WE_CBCR_PING_BUFFER_CFG_ADDR, HWIO_JPEG_WE_CBCR_PING_BUFFER_CFG_RMSK)
#define HWIO_JPEG_WE_CBCR_PING_BUFFER_CFG_INM(m)                 \
        in_dword_masked(HWIO_JPEG_WE_CBCR_PING_BUFFER_CFG_ADDR, m)
#define HWIO_JPEG_WE_CBCR_PING_BUFFER_CFG_OUT(v)                 \
        out_dword(HWIO_JPEG_WE_CBCR_PING_BUFFER_CFG_ADDR,v)
#define HWIO_JPEG_WE_CBCR_PING_BUFFER_CFG_OUTM(m,v)              \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_CBCR_PING_BUFFER_CFG_ADDR,m,v,HWIO_JPEG_WE_CBCR_PING_BUFFER_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_CBCR_PING_BUFFER_CFG_WE_BUFFER_LENGTH_BMSK    0x7fffff
#define HWIO_JPEG_WE_CBCR_PING_BUFFER_CFG_WE_BUFFER_LENGTH_SHFT           0

#define HWIO_JPEG_WE_CBCR_PONG_BUFFER_CFG_ADDR                   (GEMINI_REG_BASE      + 0x000000d4)
#define HWIO_JPEG_WE_CBCR_PONG_BUFFER_CFG_PHYS                   (GEMINI_REG_BASE_PHYS + 0x000000d4)
#define HWIO_JPEG_WE_CBCR_PONG_BUFFER_CFG_RMSK                     0x7fffff
#define HWIO_JPEG_WE_CBCR_PONG_BUFFER_CFG_SHFT                            0
#define HWIO_JPEG_WE_CBCR_PONG_BUFFER_CFG_IN                     \
        in_dword_masked(HWIO_JPEG_WE_CBCR_PONG_BUFFER_CFG_ADDR, HWIO_JPEG_WE_CBCR_PONG_BUFFER_CFG_RMSK)
#define HWIO_JPEG_WE_CBCR_PONG_BUFFER_CFG_INM(m)                 \
        in_dword_masked(HWIO_JPEG_WE_CBCR_PONG_BUFFER_CFG_ADDR, m)
#define HWIO_JPEG_WE_CBCR_PONG_BUFFER_CFG_OUT(v)                 \
        out_dword(HWIO_JPEG_WE_CBCR_PONG_BUFFER_CFG_ADDR,v)
#define HWIO_JPEG_WE_CBCR_PONG_BUFFER_CFG_OUTM(m,v)              \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_CBCR_PONG_BUFFER_CFG_ADDR,m,v,HWIO_JPEG_WE_CBCR_PONG_BUFFER_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_CBCR_PONG_BUFFER_CFG_WE_BUFFER_LENGTH_BMSK    0x7fffff
#define HWIO_JPEG_WE_CBCR_PONG_BUFFER_CFG_WE_BUFFER_LENGTH_SHFT           0

#define HWIO_JPEG_WE_Y_PING_ADDR_ADDR                            (GEMINI_REG_BASE      + 0x000000d8)
#define HWIO_JPEG_WE_Y_PING_ADDR_PHYS                            (GEMINI_REG_BASE_PHYS + 0x000000d8)
#define HWIO_JPEG_WE_Y_PING_ADDR_RMSK                            0xfffffff8
#define HWIO_JPEG_WE_Y_PING_ADDR_SHFT                                     0
#define HWIO_JPEG_WE_Y_PING_ADDR_IN                              \
        in_dword_masked(HWIO_JPEG_WE_Y_PING_ADDR_ADDR, HWIO_JPEG_WE_Y_PING_ADDR_RMSK)
#define HWIO_JPEG_WE_Y_PING_ADDR_INM(m)                          \
        in_dword_masked(HWIO_JPEG_WE_Y_PING_ADDR_ADDR, m)
#define HWIO_JPEG_WE_Y_PING_ADDR_OUT(v)                          \
        out_dword(HWIO_JPEG_WE_Y_PING_ADDR_ADDR,v)
#define HWIO_JPEG_WE_Y_PING_ADDR_OUTM(m,v)                       \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_Y_PING_ADDR_ADDR,m,v,HWIO_JPEG_WE_Y_PING_ADDR_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_Y_PING_ADDR_WE_Y_PING_START_ADDR_BMSK       0xfffffff8
#define HWIO_JPEG_WE_Y_PING_ADDR_WE_Y_PING_START_ADDR_SHFT              0x3

#define HWIO_JPEG_WE_Y_PONG_ADDR_ADDR                            (GEMINI_REG_BASE      + 0x000000dc)
#define HWIO_JPEG_WE_Y_PONG_ADDR_PHYS                            (GEMINI_REG_BASE_PHYS + 0x000000dc)
#define HWIO_JPEG_WE_Y_PONG_ADDR_RMSK                            0xfffffff8
#define HWIO_JPEG_WE_Y_PONG_ADDR_SHFT                                     0
#define HWIO_JPEG_WE_Y_PONG_ADDR_IN                              \
        in_dword_masked(HWIO_JPEG_WE_Y_PONG_ADDR_ADDR, HWIO_JPEG_WE_Y_PONG_ADDR_RMSK)
#define HWIO_JPEG_WE_Y_PONG_ADDR_INM(m)                          \
        in_dword_masked(HWIO_JPEG_WE_Y_PONG_ADDR_ADDR, m)
#define HWIO_JPEG_WE_Y_PONG_ADDR_OUT(v)                          \
        out_dword(HWIO_JPEG_WE_Y_PONG_ADDR_ADDR,v)
#define HWIO_JPEG_WE_Y_PONG_ADDR_OUTM(m,v)                       \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_Y_PONG_ADDR_ADDR,m,v,HWIO_JPEG_WE_Y_PONG_ADDR_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_Y_PONG_ADDR_WE_Y_PONG_START_ADDR_BMSK       0xfffffff8
#define HWIO_JPEG_WE_Y_PONG_ADDR_WE_Y_PONG_START_ADDR_SHFT              0x3

#define HWIO_JPEG_WE_CBCR_PING_ADDR_ADDR                         (GEMINI_REG_BASE      + 0x000000e0)
#define HWIO_JPEG_WE_CBCR_PING_ADDR_PHYS                         (GEMINI_REG_BASE_PHYS + 0x000000e0)
#define HWIO_JPEG_WE_CBCR_PING_ADDR_RMSK                         0xfffffff8
#define HWIO_JPEG_WE_CBCR_PING_ADDR_SHFT                                  0
#define HWIO_JPEG_WE_CBCR_PING_ADDR_IN                           \
        in_dword_masked(HWIO_JPEG_WE_CBCR_PING_ADDR_ADDR, HWIO_JPEG_WE_CBCR_PING_ADDR_RMSK)
#define HWIO_JPEG_WE_CBCR_PING_ADDR_INM(m)                       \
        in_dword_masked(HWIO_JPEG_WE_CBCR_PING_ADDR_ADDR, m)
#define HWIO_JPEG_WE_CBCR_PING_ADDR_OUT(v)                       \
        out_dword(HWIO_JPEG_WE_CBCR_PING_ADDR_ADDR,v)
#define HWIO_JPEG_WE_CBCR_PING_ADDR_OUTM(m,v)                    \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_CBCR_PING_ADDR_ADDR,m,v,HWIO_JPEG_WE_CBCR_PING_ADDR_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_CBCR_PING_ADDR_WE_CBCR_PING_START_ADDR_BMSK 0xfffffff8
#define HWIO_JPEG_WE_CBCR_PING_ADDR_WE_CBCR_PING_START_ADDR_SHFT        0x3

#define HWIO_JPEG_WE_CBCR_PONG_ADDR_ADDR                         (GEMINI_REG_BASE      + 0x000000e4)
#define HWIO_JPEG_WE_CBCR_PONG_ADDR_PHYS                         (GEMINI_REG_BASE_PHYS + 0x000000e4)
#define HWIO_JPEG_WE_CBCR_PONG_ADDR_RMSK                         0xfffffff8
#define HWIO_JPEG_WE_CBCR_PONG_ADDR_SHFT                                  0
#define HWIO_JPEG_WE_CBCR_PONG_ADDR_IN                           \
        in_dword_masked(HWIO_JPEG_WE_CBCR_PONG_ADDR_ADDR, HWIO_JPEG_WE_CBCR_PONG_ADDR_RMSK)
#define HWIO_JPEG_WE_CBCR_PONG_ADDR_INM(m)                       \
        in_dword_masked(HWIO_JPEG_WE_CBCR_PONG_ADDR_ADDR, m)
#define HWIO_JPEG_WE_CBCR_PONG_ADDR_OUT(v)                       \
        out_dword(HWIO_JPEG_WE_CBCR_PONG_ADDR_ADDR,v)
#define HWIO_JPEG_WE_CBCR_PONG_ADDR_OUTM(m,v)                    \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_CBCR_PONG_ADDR_ADDR,m,v,HWIO_JPEG_WE_CBCR_PONG_ADDR_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_CBCR_PONG_ADDR_WE_CBCR_PONG_START_ADDR_BMSK 0xfffffff8
#define HWIO_JPEG_WE_CBCR_PONG_ADDR_WE_CBCR_PONG_START_ADDR_SHFT        0x3

#define HWIO_JPEG_WE_Y_UB_CFG_ADDR                               (GEMINI_REG_BASE      + 0x000000e8)
#define HWIO_JPEG_WE_Y_UB_CFG_PHYS                               (GEMINI_REG_BASE_PHYS + 0x000000e8)
#define HWIO_JPEG_WE_Y_UB_CFG_RMSK                                0x1ff01ff
#define HWIO_JPEG_WE_Y_UB_CFG_SHFT                                        0
#define HWIO_JPEG_WE_Y_UB_CFG_IN                                 \
        in_dword_masked(HWIO_JPEG_WE_Y_UB_CFG_ADDR, HWIO_JPEG_WE_Y_UB_CFG_RMSK)
#define HWIO_JPEG_WE_Y_UB_CFG_INM(m)                             \
        in_dword_masked(HWIO_JPEG_WE_Y_UB_CFG_ADDR, m)
#define HWIO_JPEG_WE_Y_UB_CFG_OUT(v)                             \
        out_dword(HWIO_JPEG_WE_Y_UB_CFG_ADDR,v)
#define HWIO_JPEG_WE_Y_UB_CFG_OUTM(m,v)                          \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_Y_UB_CFG_ADDR,m,v,HWIO_JPEG_WE_Y_UB_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_Y_UB_CFG_WE_Y_UB_DEPTH_BMSK                  0x1ff0000
#define HWIO_JPEG_WE_Y_UB_CFG_WE_Y_UB_DEPTH_SHFT                       0x10
#define HWIO_JPEG_WE_Y_UB_CFG_WE_Y_UB_OFFSET_BMSK                     0x1ff
#define HWIO_JPEG_WE_Y_UB_CFG_WE_Y_UB_OFFSET_SHFT                         0

#define HWIO_JPEG_WE_CBCR_UB_CFG_ADDR                            (GEMINI_REG_BASE      + 0x000000ec)
#define HWIO_JPEG_WE_CBCR_UB_CFG_PHYS                            (GEMINI_REG_BASE_PHYS + 0x000000ec)
#define HWIO_JPEG_WE_CBCR_UB_CFG_RMSK                             0x1ff01ff
#define HWIO_JPEG_WE_CBCR_UB_CFG_SHFT                                     0
#define HWIO_JPEG_WE_CBCR_UB_CFG_IN                              \
        in_dword_masked(HWIO_JPEG_WE_CBCR_UB_CFG_ADDR, HWIO_JPEG_WE_CBCR_UB_CFG_RMSK)
#define HWIO_JPEG_WE_CBCR_UB_CFG_INM(m)                          \
        in_dword_masked(HWIO_JPEG_WE_CBCR_UB_CFG_ADDR, m)
#define HWIO_JPEG_WE_CBCR_UB_CFG_OUT(v)                          \
        out_dword(HWIO_JPEG_WE_CBCR_UB_CFG_ADDR,v)
#define HWIO_JPEG_WE_CBCR_UB_CFG_OUTM(m,v)                       \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_WE_CBCR_UB_CFG_ADDR,m,v,HWIO_JPEG_WE_CBCR_UB_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_WE_CBCR_UB_CFG_WE_CBCR_UB_DEPTH_BMSK            0x1ff0000
#define HWIO_JPEG_WE_CBCR_UB_CFG_WE_CBCR_UB_DEPTH_SHFT                 0x10
#define HWIO_JPEG_WE_CBCR_UB_CFG_WE_CBCR_UB_OFFSET_BMSK               0x1ff
#define HWIO_JPEG_WE_CBCR_UB_CFG_WE_CBCR_UB_OFFSET_SHFT                   0

#define HWIO_JPEG_WE_CMD_ADDR                                    (GEMINI_REG_BASE      + 0x000000f0)
#define HWIO_JPEG_WE_CMD_PHYS                                    (GEMINI_REG_BASE_PHYS + 0x000000f0)
#define HWIO_JPEG_WE_CMD_RMSK                                           0x1
#define HWIO_JPEG_WE_CMD_SHFT                                             0
#define HWIO_JPEG_WE_CMD_OUT(v)                                  \
        out_dword(HWIO_JPEG_WE_CMD_ADDR,v)
#define HWIO_JPEG_WE_CMD_OUTM(m,v)                               \
        out_dword_masked(HWIO_JPEG_WE_CMD_ADDR,m,v,HWIO_JPEG_WE_CMD_shadow)
#define HWIO_JPEG_WE_CMD_WE_BUFFER_RELOAD_CMD_BMSK                      0x1
#define HWIO_JPEG_WE_CMD_WE_BUFFER_RELOAD_CMD_SHFT                        0

#define HWIO_JPEG_ENCODE_CFG_ADDR                                (GEMINI_REG_BASE      + 0x000000f4)
#define HWIO_JPEG_ENCODE_CFG_PHYS                                (GEMINI_REG_BASE_PHYS + 0x000000f4)
#define HWIO_JPEG_ENCODE_CFG_RMSK                                   0x1ffff
#define HWIO_JPEG_ENCODE_CFG_SHFT                                         0
#define HWIO_JPEG_ENCODE_CFG_IN                                  \
        in_dword_masked(HWIO_JPEG_ENCODE_CFG_ADDR, HWIO_JPEG_ENCODE_CFG_RMSK)
#define HWIO_JPEG_ENCODE_CFG_INM(m)                              \
        in_dword_masked(HWIO_JPEG_ENCODE_CFG_ADDR, m)
#define HWIO_JPEG_ENCODE_CFG_OUT(v)                              \
        out_dword(HWIO_JPEG_ENCODE_CFG_ADDR,v)
#define HWIO_JPEG_ENCODE_CFG_OUTM(m,v)                           \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_ENCODE_CFG_ADDR,m,v,HWIO_JPEG_ENCODE_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_ENCODE_CFG_HUFFMAN_SEL_BMSK                       0x10000
#define HWIO_JPEG_ENCODE_CFG_HUFFMAN_SEL_SHFT                          0x10
#define HWIO_JPEG_ENCODE_CFG_RST_MARKER_PERIOD_BMSK                  0xffff
#define HWIO_JPEG_ENCODE_CFG_RST_MARKER_PERIOD_SHFT                       0

#define HWIO_JPEG_ENCODE_STATS_DC_BITS_Y_ADDR                    (GEMINI_REG_BASE      + 0x000000f8)
#define HWIO_JPEG_ENCODE_STATS_DC_BITS_Y_PHYS                    (GEMINI_REG_BASE_PHYS + 0x000000f8)
#define HWIO_JPEG_ENCODE_STATS_DC_BITS_Y_RMSK                     0x7ffffff
#define HWIO_JPEG_ENCODE_STATS_DC_BITS_Y_SHFT                             0
#define HWIO_JPEG_ENCODE_STATS_DC_BITS_Y_IN                      \
        in_dword_masked(HWIO_JPEG_ENCODE_STATS_DC_BITS_Y_ADDR, HWIO_JPEG_ENCODE_STATS_DC_BITS_Y_RMSK)
#define HWIO_JPEG_ENCODE_STATS_DC_BITS_Y_INM(m)                  \
        in_dword_masked(HWIO_JPEG_ENCODE_STATS_DC_BITS_Y_ADDR, m)
#define HWIO_JPEG_ENCODE_STATS_DC_BITS_Y_ENC_DC_BITS_Y_BMSK       0x7ffffff
#define HWIO_JPEG_ENCODE_STATS_DC_BITS_Y_ENC_DC_BITS_Y_SHFT               0

#define HWIO_JPEG_ENCODE_STATS_DC_BITS_CBCR_ADDR                 (GEMINI_REG_BASE      + 0x000000fc)
#define HWIO_JPEG_ENCODE_STATS_DC_BITS_CBCR_PHYS                 (GEMINI_REG_BASE_PHYS + 0x000000fc)
#define HWIO_JPEG_ENCODE_STATS_DC_BITS_CBCR_RMSK                  0x7ffffff
#define HWIO_JPEG_ENCODE_STATS_DC_BITS_CBCR_SHFT                          0
#define HWIO_JPEG_ENCODE_STATS_DC_BITS_CBCR_IN                   \
        in_dword_masked(HWIO_JPEG_ENCODE_STATS_DC_BITS_CBCR_ADDR, HWIO_JPEG_ENCODE_STATS_DC_BITS_CBCR_RMSK)
#define HWIO_JPEG_ENCODE_STATS_DC_BITS_CBCR_INM(m)               \
        in_dword_masked(HWIO_JPEG_ENCODE_STATS_DC_BITS_CBCR_ADDR, m)
#define HWIO_JPEG_ENCODE_STATS_DC_BITS_CBCR_ENC_DC_BITS_CBCR_BMSK  0x7ffffff
#define HWIO_JPEG_ENCODE_STATS_DC_BITS_CBCR_ENC_DC_BITS_CBCR_SHFT          0

#define HWIO_JPEG_ENCODE_STATS_AC_BITS_Y_ADDR                    (GEMINI_REG_BASE      + 0x00000100)
#define HWIO_JPEG_ENCODE_STATS_AC_BITS_Y_PHYS                    (GEMINI_REG_BASE_PHYS + 0x00000100)
#define HWIO_JPEG_ENCODE_STATS_AC_BITS_Y_RMSK                     0xfffffff
#define HWIO_JPEG_ENCODE_STATS_AC_BITS_Y_SHFT                             0
#define HWIO_JPEG_ENCODE_STATS_AC_BITS_Y_IN                      \
        in_dword_masked(HWIO_JPEG_ENCODE_STATS_AC_BITS_Y_ADDR, HWIO_JPEG_ENCODE_STATS_AC_BITS_Y_RMSK)
#define HWIO_JPEG_ENCODE_STATS_AC_BITS_Y_INM(m)                  \
        in_dword_masked(HWIO_JPEG_ENCODE_STATS_AC_BITS_Y_ADDR, m)
#define HWIO_JPEG_ENCODE_STATS_AC_BITS_Y_ENC_AC_BITS_Y_BMSK       0xfffffff
#define HWIO_JPEG_ENCODE_STATS_AC_BITS_Y_ENC_AC_BITS_Y_SHFT               0

#define HWIO_JPEG_ENCODE_STATS_AC_BITS_CBCR_ADDR                 (GEMINI_REG_BASE      + 0x00000104)
#define HWIO_JPEG_ENCODE_STATS_AC_BITS_CBCR_PHYS                 (GEMINI_REG_BASE_PHYS + 0x00000104)
#define HWIO_JPEG_ENCODE_STATS_AC_BITS_CBCR_RMSK                  0xfffffff
#define HWIO_JPEG_ENCODE_STATS_AC_BITS_CBCR_SHFT                          0
#define HWIO_JPEG_ENCODE_STATS_AC_BITS_CBCR_IN                   \
        in_dword_masked(HWIO_JPEG_ENCODE_STATS_AC_BITS_CBCR_ADDR, HWIO_JPEG_ENCODE_STATS_AC_BITS_CBCR_RMSK)
#define HWIO_JPEG_ENCODE_STATS_AC_BITS_CBCR_INM(m)               \
        in_dword_masked(HWIO_JPEG_ENCODE_STATS_AC_BITS_CBCR_ADDR, m)
#define HWIO_JPEG_ENCODE_STATS_AC_BITS_CBCR_ENC_AC_BITS_CBCR_BMSK  0xfffffff
#define HWIO_JPEG_ENCODE_STATS_AC_BITS_CBCR_ENC_AC_BITS_CBCR_SHFT          0

#define HWIO_JPEG_ENCODE_STATS_NONZERO_AC_Y_ADDR                 (GEMINI_REG_BASE      + 0x00000108)
#define HWIO_JPEG_ENCODE_STATS_NONZERO_AC_Y_PHYS                 (GEMINI_REG_BASE_PHYS + 0x00000108)
#define HWIO_JPEG_ENCODE_STATS_NONZERO_AC_Y_RMSK                   0xffffff
#define HWIO_JPEG_ENCODE_STATS_NONZERO_AC_Y_SHFT                          0
#define HWIO_JPEG_ENCODE_STATS_NONZERO_AC_Y_IN                   \
        in_dword_masked(HWIO_JPEG_ENCODE_STATS_NONZERO_AC_Y_ADDR, HWIO_JPEG_ENCODE_STATS_NONZERO_AC_Y_RMSK)
#define HWIO_JPEG_ENCODE_STATS_NONZERO_AC_Y_INM(m)               \
        in_dword_masked(HWIO_JPEG_ENCODE_STATS_NONZERO_AC_Y_ADDR, m)
#define HWIO_JPEG_ENCODE_STATS_NONZERO_AC_Y_ENC_NONZERO_AC_Y_BMSK   0xffffff
#define HWIO_JPEG_ENCODE_STATS_NONZERO_AC_Y_ENC_NONZERO_AC_Y_SHFT          0

#define HWIO_JPEG_ENCODE_STATS_NONZERO_AC_CBCR_ADDR              (GEMINI_REG_BASE      + 0x0000010c)
#define HWIO_JPEG_ENCODE_STATS_NONZERO_AC_CBCR_PHYS              (GEMINI_REG_BASE_PHYS + 0x0000010c)
#define HWIO_JPEG_ENCODE_STATS_NONZERO_AC_CBCR_RMSK                0xffffff
#define HWIO_JPEG_ENCODE_STATS_NONZERO_AC_CBCR_SHFT                       0
#define HWIO_JPEG_ENCODE_STATS_NONZERO_AC_CBCR_IN                \
        in_dword_masked(HWIO_JPEG_ENCODE_STATS_NONZERO_AC_CBCR_ADDR, HWIO_JPEG_ENCODE_STATS_NONZERO_AC_CBCR_RMSK)
#define HWIO_JPEG_ENCODE_STATS_NONZERO_AC_CBCR_INM(m)            \
        in_dword_masked(HWIO_JPEG_ENCODE_STATS_NONZERO_AC_CBCR_ADDR, m)
#define HWIO_JPEG_ENCODE_STATS_NONZERO_AC_CBCR_ENC_NONZERO_AC_CBCR_BMSK   0xffffff
#define HWIO_JPEG_ENCODE_STATS_NONZERO_AC_CBCR_ENC_NONZERO_AC_CBCR_SHFT          0

#define HWIO_JPEG_ENCODE_FSC_REGION_SIZE_ADDR                    (GEMINI_REG_BASE      + 0x00000110)
#define HWIO_JPEG_ENCODE_FSC_REGION_SIZE_PHYS                    (GEMINI_REG_BASE_PHYS + 0x00000110)
#define HWIO_JPEG_ENCODE_FSC_REGION_SIZE_RMSK                         0x11f
#define HWIO_JPEG_ENCODE_FSC_REGION_SIZE_SHFT                             0
#define HWIO_JPEG_ENCODE_FSC_REGION_SIZE_IN                      \
        in_dword_masked(HWIO_JPEG_ENCODE_FSC_REGION_SIZE_ADDR, HWIO_JPEG_ENCODE_FSC_REGION_SIZE_RMSK)
#define HWIO_JPEG_ENCODE_FSC_REGION_SIZE_INM(m)                  \
        in_dword_masked(HWIO_JPEG_ENCODE_FSC_REGION_SIZE_ADDR, m)
#define HWIO_JPEG_ENCODE_FSC_REGION_SIZE_OUT(v)                  \
        out_dword(HWIO_JPEG_ENCODE_FSC_REGION_SIZE_ADDR,v)
#define HWIO_JPEG_ENCODE_FSC_REGION_SIZE_OUTM(m,v)               \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_ENCODE_FSC_REGION_SIZE_ADDR,m,v,HWIO_JPEG_ENCODE_FSC_REGION_SIZE_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_ENCODE_FSC_REGION_SIZE_HV_REGION_BMSK               0x100
#define HWIO_JPEG_ENCODE_FSC_REGION_SIZE_HV_REGION_SHFT                 0x8
#define HWIO_JPEG_ENCODE_FSC_REGION_SIZE_REGION_SIZE_BMSK              0x1f
#define HWIO_JPEG_ENCODE_FSC_REGION_SIZE_REGION_SIZE_SHFT                 0

#define HWIO_JPEG_ENCODE_FSC_BUDGET_0_ADDR                       (GEMINI_REG_BASE      + 0x00000114)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_0_PHYS                       (GEMINI_REG_BASE_PHYS + 0x00000114)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_0_RMSK                       0xffffffff
#define HWIO_JPEG_ENCODE_FSC_BUDGET_0_SHFT                                0
#define HWIO_JPEG_ENCODE_FSC_BUDGET_0_IN                         \
        in_dword_masked(HWIO_JPEG_ENCODE_FSC_BUDGET_0_ADDR, HWIO_JPEG_ENCODE_FSC_BUDGET_0_RMSK)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_0_INM(m)                     \
        in_dword_masked(HWIO_JPEG_ENCODE_FSC_BUDGET_0_ADDR, m)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_0_OUT(v)                     \
        out_dword(HWIO_JPEG_ENCODE_FSC_BUDGET_0_ADDR,v)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_0_OUTM(m,v)                  \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_ENCODE_FSC_BUDGET_0_ADDR,m,v,HWIO_JPEG_ENCODE_FSC_BUDGET_0_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_ENCODE_FSC_BUDGET_0_BUDGET_REGION3_BMSK        0xff000000
#define HWIO_JPEG_ENCODE_FSC_BUDGET_0_BUDGET_REGION3_SHFT              0x18
#define HWIO_JPEG_ENCODE_FSC_BUDGET_0_BUDGET_REGION2_BMSK          0xff0000
#define HWIO_JPEG_ENCODE_FSC_BUDGET_0_BUDGET_REGION2_SHFT              0x10
#define HWIO_JPEG_ENCODE_FSC_BUDGET_0_BUDGET_REGION1_BMSK            0xff00
#define HWIO_JPEG_ENCODE_FSC_BUDGET_0_BUDGET_REGION1_SHFT               0x8
#define HWIO_JPEG_ENCODE_FSC_BUDGET_0_BUDGET_REGION0_BMSK              0xff
#define HWIO_JPEG_ENCODE_FSC_BUDGET_0_BUDGET_REGION0_SHFT                 0

#define HWIO_JPEG_ENCODE_FSC_BUDGET_1_ADDR                       (GEMINI_REG_BASE      + 0x00000118)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_1_PHYS                       (GEMINI_REG_BASE_PHYS + 0x00000118)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_1_RMSK                       0xffffffff
#define HWIO_JPEG_ENCODE_FSC_BUDGET_1_SHFT                                0
#define HWIO_JPEG_ENCODE_FSC_BUDGET_1_IN                         \
        in_dword_masked(HWIO_JPEG_ENCODE_FSC_BUDGET_1_ADDR, HWIO_JPEG_ENCODE_FSC_BUDGET_1_RMSK)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_1_INM(m)                     \
        in_dword_masked(HWIO_JPEG_ENCODE_FSC_BUDGET_1_ADDR, m)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_1_OUT(v)                     \
        out_dword(HWIO_JPEG_ENCODE_FSC_BUDGET_1_ADDR,v)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_1_OUTM(m,v)                  \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_ENCODE_FSC_BUDGET_1_ADDR,m,v,HWIO_JPEG_ENCODE_FSC_BUDGET_1_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_ENCODE_FSC_BUDGET_1_BUDGET_REGION7_BMSK        0xff000000
#define HWIO_JPEG_ENCODE_FSC_BUDGET_1_BUDGET_REGION7_SHFT              0x18
#define HWIO_JPEG_ENCODE_FSC_BUDGET_1_BUDGET_REGION6_BMSK          0xff0000
#define HWIO_JPEG_ENCODE_FSC_BUDGET_1_BUDGET_REGION6_SHFT              0x10
#define HWIO_JPEG_ENCODE_FSC_BUDGET_1_BUDGET_REGION5_BMSK            0xff00
#define HWIO_JPEG_ENCODE_FSC_BUDGET_1_BUDGET_REGION5_SHFT               0x8
#define HWIO_JPEG_ENCODE_FSC_BUDGET_1_BUDGET_REGION4_BMSK              0xff
#define HWIO_JPEG_ENCODE_FSC_BUDGET_1_BUDGET_REGION4_SHFT                 0

#define HWIO_JPEG_ENCODE_FSC_BUDGET_2_ADDR                       (GEMINI_REG_BASE      + 0x0000011c)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_2_PHYS                       (GEMINI_REG_BASE_PHYS + 0x0000011c)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_2_RMSK                       0xffffffff
#define HWIO_JPEG_ENCODE_FSC_BUDGET_2_SHFT                                0
#define HWIO_JPEG_ENCODE_FSC_BUDGET_2_IN                         \
        in_dword_masked(HWIO_JPEG_ENCODE_FSC_BUDGET_2_ADDR, HWIO_JPEG_ENCODE_FSC_BUDGET_2_RMSK)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_2_INM(m)                     \
        in_dword_masked(HWIO_JPEG_ENCODE_FSC_BUDGET_2_ADDR, m)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_2_OUT(v)                     \
        out_dword(HWIO_JPEG_ENCODE_FSC_BUDGET_2_ADDR,v)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_2_OUTM(m,v)                  \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_ENCODE_FSC_BUDGET_2_ADDR,m,v,HWIO_JPEG_ENCODE_FSC_BUDGET_2_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_ENCODE_FSC_BUDGET_2_BUDGET_REGION11_BMSK       0xff000000
#define HWIO_JPEG_ENCODE_FSC_BUDGET_2_BUDGET_REGION11_SHFT             0x18
#define HWIO_JPEG_ENCODE_FSC_BUDGET_2_BUDGET_REGION10_BMSK         0xff0000
#define HWIO_JPEG_ENCODE_FSC_BUDGET_2_BUDGET_REGION10_SHFT             0x10
#define HWIO_JPEG_ENCODE_FSC_BUDGET_2_BUDGET_REGION9_BMSK            0xff00
#define HWIO_JPEG_ENCODE_FSC_BUDGET_2_BUDGET_REGION9_SHFT               0x8
#define HWIO_JPEG_ENCODE_FSC_BUDGET_2_BUDGET_REGION8_BMSK              0xff
#define HWIO_JPEG_ENCODE_FSC_BUDGET_2_BUDGET_REGION8_SHFT                 0

#define HWIO_JPEG_ENCODE_FSC_BUDGET_3_ADDR                       (GEMINI_REG_BASE      + 0x00000120)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_3_PHYS                       (GEMINI_REG_BASE_PHYS + 0x00000120)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_3_RMSK                       0xffffffff
#define HWIO_JPEG_ENCODE_FSC_BUDGET_3_SHFT                                0
#define HWIO_JPEG_ENCODE_FSC_BUDGET_3_IN                         \
        in_dword_masked(HWIO_JPEG_ENCODE_FSC_BUDGET_3_ADDR, HWIO_JPEG_ENCODE_FSC_BUDGET_3_RMSK)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_3_INM(m)                     \
        in_dword_masked(HWIO_JPEG_ENCODE_FSC_BUDGET_3_ADDR, m)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_3_OUT(v)                     \
        out_dword(HWIO_JPEG_ENCODE_FSC_BUDGET_3_ADDR,v)
#define HWIO_JPEG_ENCODE_FSC_BUDGET_3_OUTM(m,v)                  \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_ENCODE_FSC_BUDGET_3_ADDR,m,v,HWIO_JPEG_ENCODE_FSC_BUDGET_3_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_ENCODE_FSC_BUDGET_3_BUDGET_REGION15_BMSK       0xff000000
#define HWIO_JPEG_ENCODE_FSC_BUDGET_3_BUDGET_REGION15_SHFT             0x18
#define HWIO_JPEG_ENCODE_FSC_BUDGET_3_BUDGET_REGION14_BMSK         0xff0000
#define HWIO_JPEG_ENCODE_FSC_BUDGET_3_BUDGET_REGION14_SHFT             0x10
#define HWIO_JPEG_ENCODE_FSC_BUDGET_3_BUDGET_REGION13_BMSK           0xff00
#define HWIO_JPEG_ENCODE_FSC_BUDGET_3_BUDGET_REGION13_SHFT              0x8
#define HWIO_JPEG_ENCODE_FSC_BUDGET_3_BUDGET_REGION12_BMSK             0xff
#define HWIO_JPEG_ENCODE_FSC_BUDGET_3_BUDGET_REGION12_SHFT                0

#define HWIO_JPEG_DMI_CFG_ADDR                                   (GEMINI_REG_BASE      + 0x00000124)
#define HWIO_JPEG_DMI_CFG_PHYS                                   (GEMINI_REG_BASE_PHYS + 0x00000124)
#define HWIO_JPEG_DMI_CFG_RMSK                                          0x7
#define HWIO_JPEG_DMI_CFG_SHFT                                            0
#define HWIO_JPEG_DMI_CFG_IN                                     \
        in_dword_masked(HWIO_JPEG_DMI_CFG_ADDR, HWIO_JPEG_DMI_CFG_RMSK)
#define HWIO_JPEG_DMI_CFG_INM(m)                                 \
        in_dword_masked(HWIO_JPEG_DMI_CFG_ADDR, m)
#define HWIO_JPEG_DMI_CFG_OUT(v)                                 \
        out_dword(HWIO_JPEG_DMI_CFG_ADDR,v)
#define HWIO_JPEG_DMI_CFG_OUTM(m,v)                              \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_DMI_CFG_ADDR,m,v,HWIO_JPEG_DMI_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_DMI_CFG_AUTO_INC_EN_BMSK                              0x4
#define HWIO_JPEG_DMI_CFG_AUTO_INC_EN_SHFT                              0x2
#define HWIO_JPEG_DMI_CFG_DMI_RAM_SEL_BMSK                              0x3
#define HWIO_JPEG_DMI_CFG_DMI_RAM_SEL_SHFT                                0

#define HWIO_JPEG_DMI_ADDR_ADDR                                  (GEMINI_REG_BASE      + 0x00000128)
#define HWIO_JPEG_DMI_ADDR_PHYS                                  (GEMINI_REG_BASE_PHYS + 0x00000128)
#define HWIO_JPEG_DMI_ADDR_RMSK                                       0x3ff
#define HWIO_JPEG_DMI_ADDR_SHFT                                           0
#define HWIO_JPEG_DMI_ADDR_IN                                    \
        in_dword_masked(HWIO_JPEG_DMI_ADDR_ADDR, HWIO_JPEG_DMI_ADDR_RMSK)
#define HWIO_JPEG_DMI_ADDR_INM(m)                                \
        in_dword_masked(HWIO_JPEG_DMI_ADDR_ADDR, m)
#define HWIO_JPEG_DMI_ADDR_OUT(v)                                \
        out_dword(HWIO_JPEG_DMI_ADDR_ADDR,v)
#define HWIO_JPEG_DMI_ADDR_OUTM(m,v)                             \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_DMI_ADDR_ADDR,m,v,HWIO_JPEG_DMI_ADDR_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_DMI_ADDR_DMI_ADDR_BMSK                              0x3ff
#define HWIO_JPEG_DMI_ADDR_DMI_ADDR_SHFT                                  0

#define HWIO_JPEG_DMI_DATA_LO_ADDR                               (GEMINI_REG_BASE      + 0x0000012c)
#define HWIO_JPEG_DMI_DATA_LO_PHYS                               (GEMINI_REG_BASE_PHYS + 0x0000012c)
#define HWIO_JPEG_DMI_DATA_LO_RMSK                               0xffffffff
#define HWIO_JPEG_DMI_DATA_LO_SHFT                                        0
#define HWIO_JPEG_DMI_DATA_LO_IN                                 \
        in_dword_masked(HWIO_JPEG_DMI_DATA_LO_ADDR, HWIO_JPEG_DMI_DATA_LO_RMSK)
#define HWIO_JPEG_DMI_DATA_LO_INM(m)                             \
        in_dword_masked(HWIO_JPEG_DMI_DATA_LO_ADDR, m)
#define HWIO_JPEG_DMI_DATA_LO_OUT(v)                             \
        out_dword(HWIO_JPEG_DMI_DATA_LO_ADDR,v)
#define HWIO_JPEG_DMI_DATA_LO_OUTM(m,v)                          \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_DMI_DATA_LO_ADDR,m,v,HWIO_JPEG_DMI_DATA_LO_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_DMI_DATA_LO_DMI_DATA_LO_BMSK                   0xffffffff
#define HWIO_JPEG_DMI_DATA_LO_DMI_DATA_LO_SHFT                            0

#define HWIO_JPEG_TESTGEN_CFG_ADDR                               (GEMINI_REG_BASE      + 0x00000130)
#define HWIO_JPEG_TESTGEN_CFG_PHYS                               (GEMINI_REG_BASE_PHYS + 0x00000130)
#define HWIO_JPEG_TESTGEN_CFG_RMSK                               0x1ff6ff7f
#define HWIO_JPEG_TESTGEN_CFG_SHFT                                        0
#define HWIO_JPEG_TESTGEN_CFG_IN                                 \
        in_dword_masked(HWIO_JPEG_TESTGEN_CFG_ADDR, HWIO_JPEG_TESTGEN_CFG_RMSK)
#define HWIO_JPEG_TESTGEN_CFG_INM(m)                             \
        in_dword_masked(HWIO_JPEG_TESTGEN_CFG_ADDR, m)
#define HWIO_JPEG_TESTGEN_CFG_OUT(v)                             \
        out_dword(HWIO_JPEG_TESTGEN_CFG_ADDR,v)
#define HWIO_JPEG_TESTGEN_CFG_OUTM(m,v)                          \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_TESTGEN_CFG_ADDR,m,v,HWIO_JPEG_TESTGEN_CFG_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_TESTGEN_CFG_TESTGEN_BLOCKS_BMSK                0x1ff00000
#define HWIO_JPEG_TESTGEN_CFG_TESTGEN_BLOCKS_SHFT                      0x14
#define HWIO_JPEG_TESTGEN_CFG_TESTGEN_DC_MODE_BMSK                  0x40000
#define HWIO_JPEG_TESTGEN_CFG_TESTGEN_DC_MODE_SHFT                     0x12
#define HWIO_JPEG_TESTGEN_CFG_TESTGEN_AC_MODE_BMSK                  0x20000
#define HWIO_JPEG_TESTGEN_CFG_TESTGEN_AC_MODE_SHFT                     0x11
#define HWIO_JPEG_TESTGEN_CFG_TESTGEN_DC_BMSK                        0xff00
#define HWIO_JPEG_TESTGEN_CFG_TESTGEN_DC_SHFT                           0x8
#define HWIO_JPEG_TESTGEN_CFG_TESTGEN_AC_MAG_BMSK                      0x7f
#define HWIO_JPEG_TESTGEN_CFG_TESTGEN_AC_MAG_SHFT                         0

#define HWIO_JPEG_TESTGEN_SEED_ADDR                              (GEMINI_REG_BASE      + 0x00000134)
#define HWIO_JPEG_TESTGEN_SEED_PHYS                              (GEMINI_REG_BASE_PHYS + 0x00000134)
#define HWIO_JPEG_TESTGEN_SEED_RMSK                                  0xffff
#define HWIO_JPEG_TESTGEN_SEED_SHFT                                       0
#define HWIO_JPEG_TESTGEN_SEED_IN                                \
        in_dword_masked(HWIO_JPEG_TESTGEN_SEED_ADDR, HWIO_JPEG_TESTGEN_SEED_RMSK)
#define HWIO_JPEG_TESTGEN_SEED_INM(m)                            \
        in_dword_masked(HWIO_JPEG_TESTGEN_SEED_ADDR, m)
#define HWIO_JPEG_TESTGEN_SEED_OUT(v)                            \
        out_dword(HWIO_JPEG_TESTGEN_SEED_ADDR,v)
#define HWIO_JPEG_TESTGEN_SEED_OUTM(m,v)                         \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_TESTGEN_SEED_ADDR,m,v,HWIO_JPEG_TESTGEN_SEED_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_TESTGEN_SEED_TESTGEN_SEED_BMSK                     0xffff
#define HWIO_JPEG_TESTGEN_SEED_TESTGEN_SEED_SHFT                          0

#define HWIO_JPEG_TESTGEN_CMD_ADDR                               (GEMINI_REG_BASE      + 0x00000138)
#define HWIO_JPEG_TESTGEN_CMD_PHYS                               (GEMINI_REG_BASE_PHYS + 0x00000138)
#define HWIO_JPEG_TESTGEN_CMD_RMSK                                      0x1
#define HWIO_JPEG_TESTGEN_CMD_SHFT                                        0
#define HWIO_JPEG_TESTGEN_CMD_IN                                 \
        in_dword_masked(HWIO_JPEG_TESTGEN_CMD_ADDR, HWIO_JPEG_TESTGEN_CMD_RMSK)
#define HWIO_JPEG_TESTGEN_CMD_INM(m)                             \
        in_dword_masked(HWIO_JPEG_TESTGEN_CMD_ADDR, m)
#define HWIO_JPEG_TESTGEN_CMD_OUT(v)                             \
        out_dword(HWIO_JPEG_TESTGEN_CMD_ADDR,v)
#define HWIO_JPEG_TESTGEN_CMD_OUTM(m,v)                          \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_TESTGEN_CMD_ADDR,m,v,HWIO_JPEG_TESTGEN_CMD_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_TESTGEN_CMD_TESTGEN_START_CMD_BMSK                    0x1
#define HWIO_JPEG_TESTGEN_CMD_TESTGEN_START_CMD_SHFT                      0

#define HWIO_JPEG_MONITOR_STATUS_0_ADDR                          (GEMINI_REG_BASE      + 0x0000013c)
#define HWIO_JPEG_MONITOR_STATUS_0_PHYS                          (GEMINI_REG_BASE_PHYS + 0x0000013c)
#define HWIO_JPEG_MONITOR_STATUS_0_RMSK                             0x7ffff
#define HWIO_JPEG_MONITOR_STATUS_0_SHFT                                   0
#define HWIO_JPEG_MONITOR_STATUS_0_IN                            \
        in_dword_masked(HWIO_JPEG_MONITOR_STATUS_0_ADDR, HWIO_JPEG_MONITOR_STATUS_0_RMSK)
#define HWIO_JPEG_MONITOR_STATUS_0_INM(m)                        \
        in_dword_masked(HWIO_JPEG_MONITOR_STATUS_0_ADDR, m)
#define HWIO_JPEG_MONITOR_STATUS_0_STB_TO_STB_CNT_BMSK              0x7ffff
#define HWIO_JPEG_MONITOR_STATUS_0_STB_TO_STB_CNT_SHFT                    0

#define HWIO_JPEG_MONITOR_STATUS_1_ADDR                          (GEMINI_REG_BASE      + 0x00000140)
#define HWIO_JPEG_MONITOR_STATUS_1_PHYS                          (GEMINI_REG_BASE_PHYS + 0x00000140)
#define HWIO_JPEG_MONITOR_STATUS_1_RMSK                             0xf3f3f
#define HWIO_JPEG_MONITOR_STATUS_1_SHFT                                   0
#define HWIO_JPEG_MONITOR_STATUS_1_IN                            \
        in_dword_masked(HWIO_JPEG_MONITOR_STATUS_1_ADDR, HWIO_JPEG_MONITOR_STATUS_1_RMSK)
#define HWIO_JPEG_MONITOR_STATUS_1_INM(m)                        \
        in_dword_masked(HWIO_JPEG_MONITOR_STATUS_1_ADDR, m)
#define HWIO_JPEG_MONITOR_STATUS_1_EOI_OVERFLOW_BMSK                0x80000
#define HWIO_JPEG_MONITOR_STATUS_1_EOI_OVERFLOW_SHFT                   0x13
#define HWIO_JPEG_MONITOR_STATUS_1_EOF_OVERFLOW_BMSK                0x40000
#define HWIO_JPEG_MONITOR_STATUS_1_EOF_OVERFLOW_SHFT                   0x12
#define HWIO_JPEG_MONITOR_STATUS_1_CH1_FIFO_UNDERFLOW_BMSK          0x20000
#define HWIO_JPEG_MONITOR_STATUS_1_CH1_FIFO_UNDERFLOW_SHFT             0x11
#define HWIO_JPEG_MONITOR_STATUS_1_CH0_FIFO_UNDERFLOW_BMSK          0x10000
#define HWIO_JPEG_MONITOR_STATUS_1_CH0_FIFO_UNDERFLOW_SHFT             0x10
#define HWIO_JPEG_MONITOR_STATUS_1_CH1_FIFO_LEVEL_BMSK               0x3f00
#define HWIO_JPEG_MONITOR_STATUS_1_CH1_FIFO_LEVEL_SHFT                  0x8
#define HWIO_JPEG_MONITOR_STATUS_1_CH0_FIFO_LEVEL_BMSK                 0x3f
#define HWIO_JPEG_MONITOR_STATUS_1_CH0_FIFO_LEVEL_SHFT                    0

#define HWIO_JPEG_MONITOR_STATUS_2_ADDR                          (GEMINI_REG_BASE      + 0x00000144)
#define HWIO_JPEG_MONITOR_STATUS_2_PHYS                          (GEMINI_REG_BASE_PHYS + 0x00000144)
#define HWIO_JPEG_MONITOR_STATUS_2_RMSK                          0xffffffff
#define HWIO_JPEG_MONITOR_STATUS_2_SHFT                                   0
#define HWIO_JPEG_MONITOR_STATUS_2_IN                            \
        in_dword_masked(HWIO_JPEG_MONITOR_STATUS_2_ADDR, HWIO_JPEG_MONITOR_STATUS_2_RMSK)
#define HWIO_JPEG_MONITOR_STATUS_2_INM(m)                        \
        in_dword_masked(HWIO_JPEG_MONITOR_STATUS_2_ADDR, m)
#define HWIO_JPEG_MONITOR_STATUS_2_TRANS_BUS_CYCLES_BMSK         0xffffc000
#define HWIO_JPEG_MONITOR_STATUS_2_TRANS_BUS_CYCLES_SHFT                0xe
#define HWIO_JPEG_MONITOR_STATUS_2_TRANS_CNT_BMSK                    0x3fff
#define HWIO_JPEG_MONITOR_STATUS_2_TRANS_CNT_SHFT                         0

#define HWIO_JPEG_TESTBUS_SEL_ADDR                               (GEMINI_REG_BASE      + 0x00000148)
#define HWIO_JPEG_TESTBUS_SEL_PHYS                               (GEMINI_REG_BASE_PHYS + 0x00000148)
#define HWIO_JPEG_TESTBUS_SEL_RMSK                                    0x30f
#define HWIO_JPEG_TESTBUS_SEL_SHFT                                        0
#define HWIO_JPEG_TESTBUS_SEL_IN                                 \
        in_dword_masked(HWIO_JPEG_TESTBUS_SEL_ADDR, HWIO_JPEG_TESTBUS_SEL_RMSK)
#define HWIO_JPEG_TESTBUS_SEL_INM(m)                             \
        in_dword_masked(HWIO_JPEG_TESTBUS_SEL_ADDR, m)
#define HWIO_JPEG_TESTBUS_SEL_OUT(v)                             \
        out_dword(HWIO_JPEG_TESTBUS_SEL_ADDR,v)
#define HWIO_JPEG_TESTBUS_SEL_OUTM(m,v)                          \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_TESTBUS_SEL_ADDR,m,v,HWIO_JPEG_TESTBUS_SEL_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_TESTBUS_SEL_DOMAIN_SEL_BMSK                         0x300
#define HWIO_JPEG_TESTBUS_SEL_DOMAIN_SEL_SHFT                           0x8
#define HWIO_JPEG_TESTBUS_SEL_BUS_SEL_BMSK                              0xf
#define HWIO_JPEG_TESTBUS_SEL_BUS_SEL_SHFT                                0

#define HWIO_JPEG_SPARE_ADDR                                     (GEMINI_REG_BASE      + 0x0000014c)
#define HWIO_JPEG_SPARE_PHYS                                     (GEMINI_REG_BASE_PHYS + 0x0000014c)
#define HWIO_JPEG_SPARE_RMSK                                     0xffffffff
#define HWIO_JPEG_SPARE_SHFT                                              0
#define HWIO_JPEG_SPARE_IN                                       \
        in_dword_masked(HWIO_JPEG_SPARE_ADDR, HWIO_JPEG_SPARE_RMSK)
#define HWIO_JPEG_SPARE_INM(m)                                   \
        in_dword_masked(HWIO_JPEG_SPARE_ADDR, m)
#define HWIO_JPEG_SPARE_OUT(v)                                   \
        out_dword(HWIO_JPEG_SPARE_ADDR,v)
#define HWIO_JPEG_SPARE_OUTM(m,v)                                \
        HWIO_INTLOCK(); \
        out_dword_masked_ns(HWIO_JPEG_SPARE_ADDR,m,v,HWIO_JPEG_SPARE_IN); \
        HWIO_INTFREE()
#define HWIO_JPEG_SPARE_SPARE_BMSK                               0xffffffff
#define HWIO_JPEG_SPARE_SPARE_SHFT                                        0

#endif /* GEMINI_HW_REG_H */
