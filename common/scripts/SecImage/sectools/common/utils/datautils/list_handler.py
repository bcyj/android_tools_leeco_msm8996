#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

"""Provides data handling capabilities for list data.

**Class Inheritance Diagram:**

.. inheritance-diagram:: coreutils.datautils.list_handler
    :parts: 2
"""


from base_handler import BaseValidator, BaseTransferMgr, \
    BaseCodeGenerator, BaseHandler, get_class_ref


class ListHandler(BaseHandler):
    """Data handler for list data.

    :params: Same as :class:`.base_handler.BaseHandler`.
    """

    def __init__(self, data_handler):
        """Initialized the various features."""
        BaseHandler.__init__(self, data_handler)
        self._i_validator = ListValidator(self)
        self._i_transfer_mgr = ListTransferMgr(self)
        self._i_code_generator = ListCodeGenerator(self)

    #--------------------------------------------------------------------------
    # Various properties that must be defined by the data handler if they
    # support the feature.
    #--------------------------------------------------------------------------
    @property
    def _validator(self):
        """Returns the :class:`ListValidator` object for the data type."""
        return self._i_validator

    @property
    def _transfer_mgr(self):
        """Returns the :class:`ListTransferMgr` object for the data type."""
        return self._i_transfer_mgr

    @property
    def _code_generator(self):
        """Returns the :class:`ListCodeGenerator` object for the data type."""
        return self._i_code_generator

    #--------------------------------------------------------------------------
    # Methods that must be implemented by the data handler
    #--------------------------------------------------------------------------
    @classmethod
    def get_data_handler_data_type(cls):
        """Returns the data type that is handled by this data handler"""
        return list

    @classmethod
    def get_data_handler(cls):
        """Returns the class reference of this data handler"""
        return ListHandler


class ListValidator(BaseValidator):
    """Validator for List data.

    :params: Same as :class:`BaseValidator`.
    """

    def validate(self, i_value, i_format):
        """See :meth:`.Data_Handler.BaseValidator.validate`"""
        assert isinstance(i_format, list)
        return self.generic_validate_list(i_value, i_format, i_format[0])


class ListTransferMgr(BaseTransferMgr):
    """TransferMgr for List data.

    :params: Same as :class:`BaseTransferMgr`.
    """

    def transfer(self, i_value_from, i_value_to, i_structure, i_structure_parent):
        """See :meth:`.Data_Handler.BaseTransferMgr.transfer`"""
        assert isinstance(i_structure, list)
        o_value = []
        i_structure = i_structure[0]
        if i_value_from is not None:
            assert isinstance(i_value_from, list)
            o_value = [self.generic_transfer(
                            value, get_class_ref(i_structure_parent, self.cb_dict)() if isinstance(i_structure, dict) else i_value_from,
                                 i_structure, i_structure_parent) for value in i_value_from]
        return o_value


class ListCodeGenerator(BaseCodeGenerator):
    """CodeGenerator for List data.

    :params: Same as :class:`BaseCodeGenerator`.
    """

    def generate_code(self, attr_key, attr_value):
        """See :meth:`.Data_Handler.BaseCodeGenerator.generate_code`"""
        assert isinstance(attr_key, str)
        assert isinstance(attr_value, list)
        return attr_key + ' = []'

    def get_dict_rep(self, attr_key, attr_value):
        """See :meth:`.Data_Handler.BaseCodeGenerator.get_dict_rep`"""
        assert isinstance(attr_value, list)
        if isinstance(attr_value[0], dict):
            return attr_value[0]
        BaseCodeGenerator.get_dict_rep(self, attr_key, attr_value)


#------------------------------------------------------------------------------
# Restrict all import
#------------------------------------------------------------------------------
__all__ = ['ListHandler',
           'ListValidator',
           'ListTransferMgr',
           'ListCodeGenerator',]

