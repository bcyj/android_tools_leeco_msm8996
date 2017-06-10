#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

"""Provides data handling capabilities for tuple data.

**Class Inheritance Diagram:**

.. inheritance-diagram:: coreutils.datautils.tuple_handler
    :parts: 2
"""


from base_handler import BaseValidator, BaseTransferMgr, \
    BaseCodeGenerator, BaseHandler


class TupleHandler(BaseHandler):
    """Data handler for tuple data.

    :params: Same as :class:`.base_handler.BaseHandler`.
    """

    def __init__(self, data_handler):
        """Initialized the various features."""
        BaseHandler.__init__(self, data_handler)
        self._i_validator = TupleValidator(self)
        self._i_transfer_mgr = TupleTransferMgr(self)
        self._i_code_generator = TupleCodeGenerator(self)

    #--------------------------------------------------------------------------
    # Various properties that must be defined by the data handler if they
    # support the feature.
    #--------------------------------------------------------------------------
    @property
    def _validator(self):
        """Returns the :class:`TupleValidator` object for the data type."""
        return self._i_validator

    @property
    def _transfer_mgr(self):
        """Returns the :class:`TupleTransferMgr` object for the data type."""
        return self._i_transfer_mgr

    @property
    def _code_generator(self):
        """Returns the :class:`TupleCodeGenerator` object for the data type."""
        return self._i_code_generator

    #--------------------------------------------------------------------------
    # Methods that must be implemented by the data handler
    #--------------------------------------------------------------------------
    @classmethod
    def get_data_handler_data_type(cls):
        """Returns the data type that is handled by this data handler"""
        return tuple

    @classmethod
    def get_data_handler(cls):
        """Returns the class reference of this data handler"""
        return TupleHandler


class TupleValidator(BaseValidator):
    """Validator for Tuple data.

    :params: Same as :class:`BaseValidator`.
    """

    def validate(self, i_value, i_format):
        """Checks if the i_value can be optional or is restricted to a
        set of values.

        Optional: If the i_format is a tuple of length one and of a certain data
            type, the i_value can be None.
            Ex: i_value is an optional string

            ::

                i_format: ('', )

        Optional Choice: If the i_format is a tuple of length greater than one
            of a certain data type, the i_value can be None or one of the values
            in the tuple.
            Ex: i_value can be None or 'hello' or 'world'.

            ::

                i_format: ('hello', 'world', )

        Choice: If the i_format is a tuple of tuple of length greater than one
            of a certain data type, the i_value must be one of the values in the
            tuple.
            Ex: i_value must be 'hello' or 'world':

            ::

                i_format: (('hello', 'world', ),)

        :params: See :meth:`.Data_Handler.BaseNormalizer.validate`
        :returns: See :meth:`.Data_Handler.BaseNormalizer.validate`
        """
        assert isinstance(i_format, tuple)
        o_value = i_value

        if isinstance(i_format[0], tuple):
            # Tuple contents are not optional
            i_format = i_format[0]
        elif o_value is None:
            # Tuple contents are optional
            return None

        # Call the validate for this particular data type
        o_value = self.generic_validate(o_value, i_format[0])

        if len(i_format) > 1:
            # o_value is restricted to a choice
            if o_value not in i_format:
                raise ValueError('o_value must be one of: ' + str(i_format) + '\n'
                                 '    ' + 'o_value: ' + str(o_value) + '\n'
                                 '    ' + 'i_value: ' + str(i_value))
        return o_value


class TupleTransferMgr(BaseTransferMgr):
    """TransferMgr for Tuple data.

    :params: Same as :class:`BaseTransferMgr`.
    """

    def transfer(self, i_value_from, i_value_to, i_structure, i_structure_parent):
        """See :meth:`.Data_Handler.BaseTransferMgr.transfer`"""
        assert isinstance(i_structure, tuple)
        return self.generic_transfer(i_value_from, i_value_to, i_structure[0], i_structure_parent)


class TupleCodeGenerator(BaseCodeGenerator):
    """CodeGenerator for Tuple data.

    :params: Same as :class:`BaseCodeGenerator`.
    """

    def generate_code(self, attr_key, attr_value):
        """See :meth:`.Data_Handler.BaseCodeGenerator.generate_code`"""
        assert isinstance(attr_key, str)
        assert isinstance(attr_value, tuple)
        attr_value = attr_value[0]
        return self.generic_generate_code(attr_key, attr_value)


#------------------------------------------------------------------------------
# Restrict all import
#------------------------------------------------------------------------------
__all__ = ['TupleHandler',
           'TupleValidator',
           'TupleTransferMgr',
           'TupleCodeGenerator',]

