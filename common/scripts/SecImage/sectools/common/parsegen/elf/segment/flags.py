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

# Returns the least significant bit that is enabled
def _get_lsb(n):
    binrep = bin(n)[len(bin(0))-1:]
    lsb_index = binrep.rfind('1')
    if lsb_index < 0:
        raise RuntimeError('Bin index is less than 0')
    return len(binrep) - lsb_index - 1

# Returns the shift for the mask
PF_SHIFT     = lambda x: _get_lsb(x)

# Returns the region specific value based on the mask & bits
PF_VALUE     = lambda flag, mask, shift: (flag&mask)>>shift

#------------------------------------------------------------------------------
# Elf standard flags constants
#------------------------------------------------------------------------------
# Permission
PF_PERM_MASK        = 0x7
PF_PERM_SHIFT       = PF_SHIFT(PF_PERM_MASK)
PF_PERM_VALUE       = lambda x: PF_VALUE(x, PF_PERM_MASK, PF_PERM_SHIFT)

PF_X                = 0x1
PF_W                = 0x2
PF_R                = 0x4

PF_PERM_STRING      = 'Perm'
PF_PERM_DESCRIPTION = \
{
    PF_X                : 'E',
    PF_W                : 'W',
    PF_W | PF_X         : 'WE',
    PF_R                : 'R',
    PF_R | PF_X         : 'RE',
    PF_R | PF_W         : 'RW',
    PF_R | PF_W | PF_X  : 'RWE',
}

#------------------------------------------------------------------------------
# Elf OS Specific constants
#
# The bits 27-20 in p_flags is for OS specific flags.
# The bits in this byte are defined as follows:
#
#     Pool Index    - 7
#     Segment type  - 6-4
#     Access type   - 3-1
#     Page mode     - 0
#
#------------------------------------------------------------------------------
# Pool Index Type
PF_OS_POOL_INDEX_MASK           = 0x08000000
PF_OS_POOL_INDEX_SHIFT          = PF_SHIFT(PF_OS_POOL_INDEX_MASK)
PF_OS_POOL_INDEX_VALUE          = lambda x: PF_VALUE(x, PF_OS_POOL_INDEX_MASK, PF_OS_POOL_INDEX_SHIFT)

PF_OS_POOL_INDEX_0              = 0x0
PF_OS_POOL_INDEX_1              = 0x1

PF_OS_POOL_INDEX_STRING         = 'OS Pool Index'
PF_OS_POOL_INDEX_DESCRIPTION = \
{
    PF_OS_POOL_INDEX_0          : '0',
    PF_OS_POOL_INDEX_1          : '1',
}

# Segment Type
PF_OS_SEGMENT_TYPE_MASK         = 0x07000000
PF_OS_SEGMENT_TYPE_SHIFT        = PF_SHIFT(PF_OS_SEGMENT_TYPE_MASK)
PF_OS_SEGMENT_TYPE_VALUE        = lambda x: PF_VALUE(x, PF_OS_SEGMENT_TYPE_MASK, PF_OS_SEGMENT_TYPE_SHIFT)

PF_OS_SEGMENT_L4                = 0x0
PF_OS_SEGMENT_AMSS              = 0x1
PF_OS_SEGMENT_HASH              = 0x2
PF_OS_SEGMENT_BOOT              = 0x3
PF_OS_SEGMENT_L4BSP             = 0x4
PF_OS_SEGMENT_SWAPPED           = 0x5
PF_OS_SEGMENT_SWAP_POOL         = 0x6
PF_OS_SEGMENT_PHDR              = 0x7

PF_OS_SEGMENT_TYPE_STRING       = 'OS Segment Type'
PF_OS_SEGMENT_TYPE_DESCRIPTION = \
{
    PF_OS_SEGMENT_L4            : 'L4',
    PF_OS_SEGMENT_AMSS          : 'AMSS',
    PF_OS_SEGMENT_HASH          : 'HASH',
    PF_OS_SEGMENT_BOOT          : 'BOOT',
    PF_OS_SEGMENT_L4BSP         : 'L4BSP',
    PF_OS_SEGMENT_SWAPPED       : 'SWAPPED',
    PF_OS_SEGMENT_SWAP_POOL     : 'SWAP_POOL',
    PF_OS_SEGMENT_PHDR          : 'PHDR',
}

# Access Type
PF_OS_ACCESS_TYPE_MASK          = 0x00E00000
PF_OS_ACCESS_TYPE_SHIFT         = PF_SHIFT(PF_OS_ACCESS_TYPE_MASK)
PF_OS_ACCESS_TYPE_VALUE         = lambda x: PF_VALUE(x, PF_OS_ACCESS_TYPE_MASK, PF_OS_ACCESS_TYPE_SHIFT)

PF_OS_ACCESS_RW                 = 0x0
PF_OS_ACCESS_RO                 = 0x1
PF_OS_ACCESS_ZI                 = 0x2
PF_OS_ACCESS_NOTUSED            = 0x3
PF_OS_ACCESS_SHARED             = 0x4

PF_OS_ACCESS_TYPE_STRING        = 'OS Access Type'
PF_OS_ACCESS_TYPE_DESCRIPTION = \
{
    PF_OS_ACCESS_RW             : 'RW',
    PF_OS_ACCESS_RO             : 'RO',
    PF_OS_ACCESS_ZI             : 'ZI',
    PF_OS_ACCESS_NOTUSED        : 'NOTUSED',
    PF_OS_ACCESS_SHARED         : 'SHARED',
}

# Page/Non-Page Type
PF_OS_PAGE_MODE_MASK            = 0x00100000
PF_OS_PAGE_MODE_SHIFT           = PF_SHIFT(PF_OS_PAGE_MODE_MASK)
PF_OS_PAGE_MODE_VALUE           = lambda x: PF_VALUE(x, PF_OS_PAGE_MODE_MASK, PF_OS_PAGE_MODE_SHIFT)

PF_OS_NON_PAGED_SEGMENT         = 0x0
PF_OS_PAGED_SEGMENT             = 0x1

PF_OS_PAGE_MODE_STRING          = 'OS Page Mode'
PF_OS_PAGE_MODE_DESCRIPTION = \
{
    PF_OS_NON_PAGED_SEGMENT     : 'NON_PAGED',
    PF_OS_PAGED_SEGMENT         : 'PAGED',
}
