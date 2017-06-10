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


import abc
import struct

from sectools.common.utils.c_misc import obj_repr


class StructBase(struct.Struct, object):
    """Wrapper over the python struct object to allow derived classes to
    automatically update internal attributes when pack and unpack are called.

    https://docs.python.org/2/library/struct.html
    """

    __metaclass__ = abc.ABCMeta

    def __init__(self, data=None, offset=0):
        struct.Struct.__init__(self, self.get_format())
        if data is None:
            self._unpack_data_list(struct.Struct.unpack_from(self, '\0' * self.size))
        else:
            self.unpack(data, offset)

    def pack(self):
        return struct.Struct.pack(self, *self._pack_data_list())

    def unpack(self, data, offset=0):
        self._unpack_data_list(struct.Struct.unpack_from(self, data, offset))
        self.validate()

    def validate(self):
        pass

    @abc.abstractmethod
    def _pack_data_list(self):
        return []

    @abc.abstractmethod
    def _unpack_data_list(self, unpacked):
        pass

    @classmethod
    def get_format(cls):
        return ''

    @classmethod
    def get_size(cls):
        return struct.Struct(cls.get_format()).size

    def __repr__(self):
        return obj_repr(self)
