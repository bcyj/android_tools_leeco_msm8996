#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

"""Provides data handling capabilities for path data.

**Class Inheritance Diagram:**

.. inheritance-diagram:: coreutils.datautils.path_handler
    :parts: 2

.. data::  type_path

    Global instance of the TypePath class.

**CB_DICT_KEY_PATH_ Keys:** These keys can be used while creating the cb_dict for
:class:`.Data_Handler`

.. data::  CB_DICT_KEY_PATH_BASEPATH

    Key to specify the basepath to use when normalizing paths.
"""


import re
import sys

from ..c_logging import logger
from base_handler import BaseValidator, BaseTransferMgr, \
    BaseCodeGenerator, BaseHandler, BaseNormalizer, _CHAR_NEWLINE


# Keys available for usage in cb_dict
CB_DICT_KEY_PATH_BASEPATH = 'path_basepath'

# Delimiter used for splitting a string into a list of paths.
_PATH_DELIM = _CHAR_NEWLINE

# String that is replaced with the path
_PATH_SUBST = './'


class TypePath(object):
    """Custom type to specify Path"""
    pass


# Global instance of the TypePath class.
type_path = TypePath()


class PathHandler(BaseHandler):
    """Data handler for path data.

    :params: Same as :class:`.base_handler.BaseHandler`.
    """

    def __init__(self, data_handler):
        """Initialized the various features."""
        BaseHandler.__init__(self, data_handler)
        self._i_normalizer = PathNormalizer(self)
        self._i_validator = PathValidator(self)
        self._i_transfer_mgr = PathTransferMgr(self)
        self._i_code_generator = PathCodeGenerator(self)

    #--------------------------------------------------------------------------
    # Various properties that must be defined by the data handler if they
    # support the feature.
    #--------------------------------------------------------------------------
    @property
    def _normalizer(self):
        """Returns the :class:`PathNormalizer` object for the data type."""
        return self._i_normalizer

    @property
    def _validator(self):
        """Returns the :class:`PathValidator` object for the data type."""
        return self._i_validator

    @property
    def _transfer_mgr(self):
        """Returns the :class:`PathTransferMgr` object for the data type."""
        return self._i_transfer_mgr

    @property
    def _code_generator(self):
        """Returns the :class:`PathCodeGenerator` object for the data type."""
        return self._i_code_generator

    #--------------------------------------------------------------------------
    # Methods that must be implemented by the data handler
    #--------------------------------------------------------------------------
    @classmethod
    def get_data_handler_data_type(cls):
        """Returns the data type that is handled by this data handler"""
        return TypePath

    @classmethod
    def get_data_handler(cls):
        """Returns the class reference of this data handler"""
        return PathHandler


class PathNormalizer(BaseNormalizer):
    """Normalizer for Path data.

    :params: Same as :class:`BaseNormalizer`.
    """

    def normalize(self, i_value):
        """See :meth:`.Data_Handler.BaseNormalizer.normalize`"""
        o_value = i_value

        # Ensure that the value is a path
        try: assert isinstance(o_value, str)
        except Exception: raise TypeError('o_value is not a path. o_value: ' + str(o_value) + ', i_value: ' + str(i_value))

        # Normalize the path
        o_value = self.generic_normalize(o_value)
        if o_value.startswith(_PATH_SUBST):
            basepath = self.cb_dict.get(CB_DICT_KEY_PATH_BASEPATH, None)
            if basepath is None:
                logger.debug('PathNormalizer.normalize: Exception raised for' + '\n'
                             '    ' + 'i_value: ' + str(i_value))
                raise RuntimeError('basepath not provided for normalizing path.\n'
                                   '    ' + 'basepath: ' + str(basepath))
            o_value = basepath + o_value[1:]

        # Return the normalized path value
        return o_value

    def normalize_list(self, i_value_list):
        """See :meth:`.Data_Handler.BaseNormalizer.normalize_list`"""
        o_value_list = i_value_list

        # If i_value_list is a string, split it on the delimiter
        if isinstance(o_value_list, str):
            o_value_list = re.split(_PATH_DELIM, self.generic_normalize(o_value_list))

        # Ensure that the value is a list
        try: assert isinstance(o_value_list, list)
        except Exception: raise TypeError('o_value_list is not a list. o_value_list: ' + str(o_value_list) + ', i_value_list: ' + str(i_value_list))

        # Normalize the values in the list
        try: o_value_list = [self.normalize(val) for val in o_value_list]
        except Exception as e:
            raise type(e), type(e)(str(e) + '\n'
                                   '    ' + 'o_value_list: ' + str(o_value_list) + '\n'
                                   '    ' + 'i_value_list: ' + str(i_value_list)), sys.exc_info()[2]

        # Return the normalized path list
        return o_value_list


class PathValidator(BaseValidator):
    """Validator for Path data.

    :params: Same as :class:`BaseValidator`.
    """

    def validate(self, i_value, i_format):
        """See :meth:`.Data_Handler.BaseValidator.validate`"""
        assert isinstance(i_format, TypePath)
        return self.data_handler.normalize(i_value)

    def validate_list(self, i_value_list, i_format):
        """See :meth:`.Data_Handler.BaseValidator.validate_list`"""
        assert isinstance(i_format, list)
        o_value_list = i_value_list

        # Normalize the i_value_list
        o_value_list = self.data_handler._normalizer.normalize_list(o_value_list)

        # Validate the values in the list
        try: o_value_list = [self.validate(val, i_format[0]) for val in o_value_list]
        except Exception as e:
            raise type(e), type(e)(str(e) + '\n'
                                   '    ' + 'i_value_list: ' + str(i_value_list) + '\n'
                                   '    ' + 'o_value_list: ' + str(o_value_list) + '\n'
                                   '    ' + 'i_format: ' + str(i_format)), sys.exc_info()[2]

        # Return the normalized path list
        return o_value_list


class PathTransferMgr(BaseTransferMgr):
    """TransferMgr for Path data.

    :params: Same as :class:`BaseTransferMgr`.
    """

    def transfer(self, i_value_from, i_value_to, i_structure, i_structure_parent):
        """See :meth:`.Data_Handler.BaseTransferMgr.transfer`"""
        assert isinstance(i_structure, TypePath)
        o_value = i_value_from
        if o_value is not None:
            assert isinstance(o_value, str)
        return o_value


class PathCodeGenerator(BaseCodeGenerator):
    """CodeGenerator for Path data.

    :params: Same as :class:`BaseCodeGenerator`.
    """

    def generate_code(self, attr_key, attr_value):
        """See :meth:`.Data_Handler.BaseCodeGenerator.generate_code`"""
        assert isinstance(attr_key, str)
        assert isinstance(attr_value, TypePath)
        return attr_key + ' = \'\''


#------------------------------------------------------------------------------
# Restrict all import
#------------------------------------------------------------------------------
__all__ = ['CB_DICT_KEY_PATH_BASEPATH',
           'TypePath',
           'type_path',
           'PathHandler',
           'PathNormalizer',
           'PathValidator',
           'PathTransferMgr',
           'PathCodeGenerator',]

