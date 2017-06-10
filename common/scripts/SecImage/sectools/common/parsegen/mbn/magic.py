#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

'''
Created on Mar 31, 2014

@author: hraghav
'''

from sectools.common.parsegen.mbn.defines import FLASH_CODE_WORD, MAGIC_NUM,\
    SBL_VIRTUAL_BLOCK_MAGIC_NUM, VIRTUAL_BLOCK_SIZE


def _compute():
    magic_cookie = ''
    for cookie in [FLASH_CODE_WORD, MAGIC_NUM, SBL_VIRTUAL_BLOCK_MAGIC_NUM]:
        cookie = hex(cookie).rstrip('L')[2:]
        if len(cookie) != 8:
            raise RuntimeError('One of the cookies is invalid')

        # This will reverse the hex numbers two numbers at a time and
        # convert the number to its character representation
        magic_cookie += ''.join(reversed([chr(int(cookie[i:i+2], 16)) for i in range(0, len(cookie), 2)]))
    return magic_cookie

def insert(data):
    ret_data = ''
    ret_cookies = []
    magic_cookie = _compute()

    vblk_size = VIRTUAL_BLOCK_SIZE

    # Put the magic numbers in the data
    while len(data) > vblk_size:
        ret_data += data[:vblk_size] + magic_cookie
        ret_cookies.append(magic_cookie)
        data = data[vblk_size:]

    ret_data += data
    return ret_data, ret_cookies, magic_cookie

def remove(data):
    ret_data = ''
    ret_cookies = []
    magic_cookie = _compute()

    magic_len = len(magic_cookie)
    vblk_size = VIRTUAL_BLOCK_SIZE

    # Extract the magic numbers in the data
    while len(data) > vblk_size:
        ret_data += data[:vblk_size]
        ret_cookies.append(data[vblk_size: vblk_size + magic_len])
        data = data[vblk_size + magic_len:]

    ret_data += data
    return ret_data, ret_cookies, magic_cookie

def validate(cookies, magic_cookie):
    # Validate the magic numbers:
    for cookie in cookies:
        # TODO: Should we allow the 0 magic cookie because of possible OTA
        if cookie not in [magic_cookie, '\0'*len(magic_cookie)]:
            raise RuntimeError('Unexpected magic cookie value encountered: ' + str([ord(c) for c in cookie]) + '\n'
                               '    ' + 'Expected magic cookie: ' + str([ord(c) for c in magic_cookie]))
