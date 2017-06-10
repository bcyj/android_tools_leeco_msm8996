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
# ELF HEADER - Identification (e_ident)
#------------------------------------------------------------------------------
# e_ident magic values
ELFMAG          = chr(0x7F) + 'ELF'

ELFMAG_STRING   = 'Magic'
ELFMAG_DESCRIPTION = \
{
    ELFMAG      : ELFMAG,
}

# e_ident class values
ELFCLASS32      = 1
ELFCLASS64      = 2

ELFCLASS_STRING = 'Class'
ELFCLASS_DESCRIPTION = \
{
    ELFCLASS32  : 'ELF32',
    ELFCLASS64  : 'ELF64',
}

# e_ident data values
ELFDATA2LSB     = 1
ELFDATA2MSB     = 2

ELFDATA_STRING  = 'Data'
ELFDATA_DESCRIPTION = \
{
    ELFDATA2LSB : '2\'s complement, little endian',
    ELFDATA2MSB : '2\'s complement, big endian',
}

# e_ident version values
ELFVERCURRENT   = 1

ELFVER_STRING   = 'Version'
ELFVER_DESCRIPTION = \
{
    ELFVERCURRENT   : '1 (Current)',
}

# e_ident OS/ABI values
ELFOSABI_NONE       = 0
ELFOSABI_HPUX       = 1
ELFOSABI_NETBSD     = 2
ELFOSABI_GNU        = 3
ELFOSABI_SOLARIS    = 6
ELFOSABI_AIX        = 7
ELFOSABI_IRIX       = 8
ELFOSABI_FREEBSD    = 9
ELFOSABI_TRU64      = 10
ELFOSABI_MODESTO    = 11
ELFOSABI_OPENBSD    = 12
ELFOSABI_OPENVMS    = 13
ELFOSABI_NSK        = 14
ELFOSABI_AROS       = 15
ELFOSABI_FENIXOS    = 16

ELFOSABI_STRING     = 'OS/ABI'
ELFOSABI_DESCRIPTION = \
{
    ELFOSABI_NONE       : 'No extensions or unspecified',
    ELFOSABI_HPUX       : 'Hewlett-Packard HP-UX',
    ELFOSABI_NETBSD     : 'NetBSD',
    ELFOSABI_GNU        : 'GNU',
    ELFOSABI_SOLARIS    : 'Sun Solaris',
    ELFOSABI_AIX        : 'AIX',
    ELFOSABI_IRIX       : 'IRIX',
    ELFOSABI_FREEBSD    : 'FreeBSD',
    ELFOSABI_TRU64      : 'Compaq TRU64 UNIX',
    ELFOSABI_MODESTO    : 'Novell Modesto',
    ELFOSABI_OPENBSD    : 'Open BSD',
    ELFOSABI_OPENVMS    : 'Open VMS',
    ELFOSABI_NSK        : 'Hewlett-Packard Non-Stop Kernel',
    ELFOSABI_AROS       : 'Amiga Research OS',
    ELFOSABI_FENIXOS    : 'The FenixOS highly scalable multi-core OS',
}

# e_ident ABI Version values
ELFABIVER_STRING    = 'ABI Version'
ELFABIVER_DESCRIPTION = \
{
}
