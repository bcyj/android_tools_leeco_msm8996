#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

"""Provides data handling capabilities for boolean data.

**Class Inheritance Diagram:**

.. inheritance-diagram:: coreutils.datautils.boolean_handler
    :parts: 2
"""


import re
import sys

from base_handler import BaseValidator, BaseTransferMgr, \
    BaseCodeGenerator, BaseHandler, BaseNormalizer, _CHAR_NEWLINE, _CHAR_COMMA


# Mapping of string boolean representations to the bool value.
_BOOLEAN_STR_MAPPING = {
                            'true' : True,
                            'false' : False,
                       }

# Delimiter used for splitting a string into a list of booleans.
_BOOLEAN_DELIM = _CHAR_NEWLINE + '|' + _CHAR_COMMA


class BooleanHandler(BaseHandler):
    """Data handler for boolean data.

    :params: Same as :class:`.base_handler.BaseHandler`.
    """

    def __init__(self, data_handler):
        """Initialized the various features."""
        BaseHandler.__init__(self, data_handler)
        self._i_normalizer = BooleanNormalizer(self)
        self._i_validator = BooleanValidator(self)
        self._i_transfer_mgr = BooleanTransferMgr(self)
        self._i_code_generator = BooleanCodeGenerator(self)

    #--------------------------------------------------------------------------
    # Various properties that must be defined by the data handler if they
    # support the feature.
    #--------------------------------------------------------------------------
    @property
    def _normalizer(self):
        """Returns the :class:`BooleanNormalizer` object for the data type."""
        return self._i_normalizer

    @property
    def _validator(self):
        """Returns the :class:`BooleanValidator` object for the data type."""
        return self._i_validator

    @property
    def _transfer_mgr(self):
        """Returns the :class:`BooleanTransferMgr` object for the data type."""
        return self._i_transfer_mgr

    @property
    def _code_generator(self):
        """Returns the :class:`BooleanCodeGenerator` object for the data type."""
        return self._i_code_generator

    #--------------------------------------------------------------------------
    # Methods that must be implemented by the data handler
    #--------------------------------------------------------------------------
    @classmethod
    def get_data_handler_data_type(cls):
        """Returns the data type that is handled by this data handler"""
        return bool

    @classmethod
    def get_data_handler(cls):
        """Returns the class reference of this data handler"""
        return BooleanHandler


class BooleanNormalizer(BaseNormalizer):
    """Normalizer for Boolean data.

    :params: Same as :class:`BaseNormalizer`.
    """

    def normalize(self, i_value):
        """See :meth:`.Data_Handler.BaseNormalizer.normalize`"""
        o_value = i_value

        # If the given value is a string, convert the string to its boolean
        # representation.
        if isinstance(o_value, str):
            o_value = _BOOLEAN_STR_MAPPING.get(o_value.strip().lower(), None)
            if o_value is None:
                raise TypeError('i_value is not a boolean string. i_value: ' + str(i_value))

        # Ensure that the value is a boolean
        try: assert isinstance(o_value, bool)
        except Exception: raise TypeError('o_value is not a boolean. o_value: ' + str(o_value) + ', i_value: ' + str(i_value))

        # Return the normalized boolean value
        return o_value

    def normalize_list(self, i_value_list):
        """See :meth:`.Data_Handler.BaseNormalizer.normalize_list`"""
        o_value_list = i_value_list

        # If i_value_list is a string, split it on the delimiter
        if isinstance(o_value_list, str):
            o_value_list = re.split(_BOOLEAN_DELIM, self.generic_normalize(o_value_list))

        # Ensure that the value is a list
        try: assert isinstance(o_value_list, list)
        except Exception: raise TypeError('o_value_list is not a list. o_value_list: ' + str(o_value_list) + ', i_value_list: ' + str(i_value_list))

        # Normalize the values in the list
        try: o_value_list = [self.normalize(val) for val in o_value_list]
        except Exception as e:
            raise type(e), type(e)(str(e) + '\n'
                                   '    ' + 'o_value_list: ' + str(o_value_list) + '\n'
                                   '    ' + 'i_value_list: ' + str(i_value_list)), sys.exc_info()[2]

        # Return the normalized boolean list
        return o_value_list


class BooleanValidator(BaseValidator):
    """Validator for Boolean data.

    :params: Same as :class:`BaseValidator`.
    """

    def validate(self, i_value, i_format):
        """See :meth:`.Data_Handler.BaseValidator.validate`"""
        assert isinstance(i_format, bool)
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

        # Return the normalized boolean list
        return o_value_list


class BooleanTransferMgr(BaseTransferMgr):
    """TransferMgr for Boolean data.

    :params: Same as :class:`BaseTransferMgr`.
    """

    def transfer(self, i_value_from, i_value_to, i_structure, i_structure_parent):
        """See :meth:`.Data_Handler.BaseTransferMgr.transfer`"""
        assert isinstance(i_structure, bool)
        o_value = i_value_from
        if o_value is not None:
            assert isinstance(o_value, bool)
        return o_value


class BooleanCodeGenerator(BaseCodeGenerator):
    """CodeGenerator for Boolean data.

    :params: Same as :class:`BaseCodeGenerator`.
    """

    def generate_code(self, attr_key, attr_value):
        """See :meth:`.Data_Handler.BaseCodeGenerator.generate_code`"""
        assert isinstance(attr_key, str)
        assert isinstance(attr_value, bool)
        return attr_key + ' = ' + str(attr_value)


#------------------------------------------------------------------------------
# Restrict all import
#------------------------------------------------------------------------------
__all__ = ['BooleanHandler',
           'BooleanNormalizer',
           'BooleanValidator',
           'BooleanTransferMgr',
           'BooleanCodeGenerator',]

