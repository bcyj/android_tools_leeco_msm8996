#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

'''
Created on Apr 2, 2014

@author: hraghav
'''

#------------------------------------------------------------------------------
# ELF HEADER - Machine (e_machine)
#------------------------------------------------------------------------------
EM_NONE             = 0
EM_M32              = 1
EM_SPARC            = 2
EM_386              = 3
EM_68K              = 4
EM_88K              = 5
RESERVED            = 6
EM_860              = 7
EM_MIPS             = 8
EM_S370             = 9
EM_MIPS_RS3_LE      = 10
RESERVED            = 11
RESERVED            = 12
RESERVED            = 13
RESERVED            = 14
EM_PARISC           = 15
RESERVED            = 16
EM_VPP500           = 17
EM_SPARC32PLUS      = 18
EM_960              = 19
EM_PPC              = 20
EM_PPC64            = 21
RESERVED            = 22
RESERVED            = 23
RESERVED            = 24
RESERVED            = 25
RESERVED            = 26
RESERVED            = 27
RESERVED            = 28
RESERVED            = 29
RESERVED            = 30
RESERVED            = 31
RESERVED            = 32
RESERVED            = 33
RESERVED            = 34
RESERVED            = 35
EM_V800             = 36
EM_FR20             = 37
EM_RH32             = 38
EM_RCE              = 39
EM_ARM              = 40
EM_ALPHA            = 41
EM_SH               = 42
EM_SPARCV9          = 43
EM_TRICORE          = 44
EM_ARC              = 45
EM_H8_300           = 46
EM_H8_300H          = 47
EM_H8S              = 48
EM_H8_500           = 49
EM_IA_64            = 50
EM_MIPS_X           = 51
EM_COLDFIRE         = 52
EM_68HC12           = 53
EM_MMA              = 54
EM_PCP              = 55
EM_NCPU             = 56
EM_NDR1             = 57
EM_STARCORE         = 58
EM_ME16             = 59
EM_ST100            = 60
EM_TINYJ            = 61
RESERVED            = 62
RESERVED            = 63
RESERVED            = 64
RESERVED            = 65
EM_FX66             = 66
EM_ST9PLUS          = 67
EM_ST7              = 68
EM_68HC16           = 69
EM_68HC11           = 70
EM_68HC08           = 71
EM_68HC05           = 72
EM_SVX              = 73
EM_ST19             = 74
EM_VAX              = 75
EM_CRIS             = 76
EM_JAVELIN          = 77
EM_FIREPATH         = 78
EM_ZSP              = 79
EM_MMIX             = 80
EM_HUANY            = 81
EM_PRISM            = 82

EM_STRING           = 'Machine'
EM_DESCRIPTION = \
{
    EM_NONE         : 'No machine',
    EM_M32          : 'AT&T WE 32100',
    EM_SPARC        : 'SPARC',
    EM_386          : 'Intel 80386',
    EM_68K          : 'Motorola 68000',
    EM_88K          : 'Motorola 88000',
    RESERVED        : 'Reserved for future use',
    EM_860          : 'Intel 80860',
    EM_MIPS         : 'MIPS I Architecture',
    EM_S370         : 'IBM System/370 Processor',
    EM_MIPS_RS3_LE  : 'MIPS RS3000 Little-endian',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    EM_PARISC       : 'Hewlett-Packard PA-RISC',
    RESERVED        : 'Reserved for future use',
    EM_VPP500       : 'Fujitsu VPP500',
    EM_SPARC32PLUS  : 'Enhanced instruction set SPARC',
    EM_960          : 'Intel 80960',
    EM_PPC          : 'PowerPC',
    EM_PPC64        : '64-bit PowerPC',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    EM_V800         : 'NEC V800',
    EM_FR20         : 'Fujitsu FR20',
    EM_RH32         : 'TRW RH-32',
    EM_RCE          : 'Motorola RCE',
    EM_ARM          : 'Advanced RISC Machines ARM',
    EM_ALPHA        : 'Digital Alpha',
    EM_SH           : 'Hitachi SH',
    EM_SPARCV9      : 'SPARC Version 9',
    EM_TRICORE      : 'Siemens Tricore embedded processor',
    EM_ARC          : 'Argonaut RISC Core, Argonaut Technologies Inc.',
    EM_H8_300       : 'Hitachi H8/300',
    EM_H8_300H      : 'Hitachi H8/300H',
    EM_H8S          : 'Hitachi H8S',
    EM_H8_500       : 'Hitachi H8/500',
    EM_IA_64        : 'Intel IA-64 processor architecture',
    EM_MIPS_X       : 'Stanford MIPS-X',
    EM_COLDFIRE     : 'Motorola ColdFire',
    EM_68HC12       : 'Motorola M68HC12',
    EM_MMA          : 'Fujitsu MMA Multimedia Accelerator',
    EM_PCP          : 'Siemens PCP',
    EM_NCPU         : 'Sony nCPU embedded RISC processor',
    EM_NDR1         : 'Denso NDR1 microprocessor',
    EM_STARCORE     : 'Motorola Star*Core processor',
    EM_ME16         : 'Toyota ME16 processor',
    EM_ST100        : 'STMicroelectronics ST100 processor',
    EM_TINYJ        : 'Advanced Logic Corp. TinyJ embedded processor family',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    RESERVED        : 'Reserved for future use',
    EM_FX66         : 'Siemens FX66 microcontroller',
    EM_ST9PLUS      : 'STMicroelectronics ST9+ 8/16 bit microcontroller',
    EM_ST7          : 'STMicroelectronics ST7 8-bit microcontroller',
    EM_68HC16       : 'Motorola MC68HC16 Microcontroller',
    EM_68HC11       : 'Motorola MC68HC11 Microcontroller',
    EM_68HC08       : 'Motorola MC68HC08 Microcontroller',
    EM_68HC05       : 'Motorola MC68HC05 Microcontroller',
    EM_SVX          : 'Silicon Graphics SVx',
    EM_ST19         : 'STMicroelectronics ST19 8-bit microcontroller',
    EM_VAX          : 'Digital VAX',
    EM_CRIS         : 'Axis Communications 32-bit embedded processor',
    EM_JAVELIN      : 'Infineon Technologies 32-bit embedded processor',
    EM_FIREPATH     : 'Element 14 64-bit DSP Processor',
    EM_ZSP          : 'LSI Logic 16-bit DSP Processor',
    EM_MMIX         : 'Donald Knuth\'s educational 64-bit processor',
    EM_HUANY        : 'Harvard University machine-independent object files',
    EM_PRISM        : 'SiTera Prism',
}
