/* Copyright (c) 2012, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef JPEGD_REG_CTRL_H
#define JPEGD_REG_CTRL_H
#include <sys/ioctl.h>
#include <fcntl.h>
#include <media/msm_jpeg.h>
#include <utils/Log.h>

#define jpegd_read(reg) \
  hw_cmd.type = MSM_JPEG_HW_CMD_TYPE_READ; \
  hw_cmd.n = 1; \
  hw_cmd.offset = JPEG_##reg##_ADDR; \
  hw_cmd.mask = JPEG_##reg##_BMSK; \
  hw_cmd.data = 0xDEADBEEF; \
  ioctl(jpegd_fd, MSM_JPEG_IOCTL_HW_CMD, &hw_cmd)

#define jpegd_write(reg, val) \
  hw_cmd.offset = JPEG_##reg##_ADDR; \
  hw_cmd.mask = JPEG_##reg##_BMSK; \
  hw_cmd.type = MSM_JPEG_HW_CMD_TYPE_WRITE; \
  hw_cmd.n = 1; \
  hw_cmd.data = val; \


#define p_jpegd_writes(reg, val) \
  p_hw_cmd->offset = JPEG_##reg##_ADDR; \
  p_hw_cmd->mask = JPEG_##reg##_BMSK; \
  p_hw_cmd->type = MSM_JPEG_HW_CMD_TYPE_WRITE; \
  p_hw_cmd->n = 1; \
  p_hw_cmd->data = val; \
  p_hw_cmd++

#define GET_FVAL(val, reg, field) ((val & JPEG_FMSK(reg, field)) >> \
  JPEG_SHFT(reg, field))

#define dword unsigned long

#define inp(port)    (*((dword *) (port)))
#define inpb(port)     (*((byte *) (port)))
#define inpw(port)   (*((word *) (port)))
#define inpdw(port)   (*((dword *)(port)))

#define outp(port, val)   (*((dword *) (port)) = ((dword) (val)))
#define outpb(port, val)   (*((byte *) (port)) = ((byte) (val)))
#define outpw(port, val)  (*((word *) (port)) = ((word) (val)))
#define outpdw(port, val) (*((dword *) (port)) = ((dword) (val)))


#define in_byte(addr)               (inp(addr))
#define in_byte_masked(addr, mask)  (inp(addr) & (byte)mask)
#define out_byte(addr, val)          outp(addr, val)
#define in_word(addr)               (inpw(addr))
#define in_word_masked(addr, mask)  (inpw(addr) & (word)mask)
#define out_word(addr, val)          outpw(addr, val)
#define in_dword(addr)              (inpdw(addr))
#define in_dword_masked(addr, mask) (inpdw(addr) & mask)
#define out_dword(addr, val)             outpdw(addr, val)

/* shadowed, masked output for write-only registers */
#define out_byte_masked(io, mask, val, shadow)  \
  do { \
      shadow = (shadow & (word)(~(mask))) | \
      ((word)((val) & (mask))); \
      (void) out_byte(io, shadow);\
  } while (0);

#define out_word_masked(io, mask, val, shadow)  \
  do { \
      shadow = (shadow & (word)(~(mask))) | \
      ((word)((val) & (mask))); \
      (void) out_word(io, shadow); \
  } while (0);

#define out_dword_masked(io, mask, val, shadow)  \
  do { \
      shadow = (shadow & (dword)(~(mask))) | \
      ((dword)((val) & (mask))); \
      (void) out_dword(io, shadow);\
  } while (0);

#define out_byte_masked_ns(io, mask, val, current_reg_content)  \
  (void) out_byte(io, ((current_reg_content & \
  (word)(~(mask))) | ((word)((val) & (mask)))))

#define out_word_masked_ns(io, mask, val, current_reg_content)  \
  (void) out_word(io, ((current_reg_content & \
  (word)(~(mask))) | ((word)((val) & (mask)))))

#define out_dword_masked_ns(io, mask, val, current_reg_content) \
  (void) out_dword(io, ((current_reg_content & \
  (dword)(~(mask))) | ((dword)((val) & (mask)))))

#define MEM_INF(val, reg, field)    ((val & JPEG_FMSK(reg, field)) >> \
  JPEG_SHFT(reg, field))

#define MEM_OUTF(mem, reg, field, val)\
  out_dword_masked_ns(mem, JPEG_FMSK(reg, field), \
  (unsigned  int)val<<JPEG_SHFT(reg, field), in_dword(mem))

#define MEM_OUTF2(mem, reg, field2, field1, val2, val1)  \
  out_dword_masked_ns(mem, (JPEG_FMSK(reg, field2)| \
  JPEG_FMSK(reg, field1)), \
  (((unsigned int)val2<<JPEG_SHFT(reg, field2))| \
  ((unsigned int)val1<<JPEG_SHFT(reg, field1))), in_dword(mem))

#define MEM_OUTF3(mem, reg, fld3, fld2, fld1, val3, val2, val1)  \
  out_dword_masked_ns(mem, (JPEG_FMSK(reg, fld3)| \
  JPEG_FMSK(reg, fld2)|JPEG_FMSK(reg, fld1)), \
  (((unsigned int)val3<<JPEG_SHFT(reg, fld3))| \
   ((unsigned int)val2<<JPEG_SHFT(reg, fld2))| \
   ((unsigned int)val1<<JPEG_SHFT(reg, fld1))), in_dword(mem))

#define JPEG_FMSK(reg, field)   JPEG_##reg##__##field##_BMSK
#define JPEG_SHFT(reg, field)   JPEG_##reg##__##field##_SHFT



#endif /*JPEGD_REG_CTRL_H*/
