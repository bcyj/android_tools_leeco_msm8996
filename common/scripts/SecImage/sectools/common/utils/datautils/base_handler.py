#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

"""Base objects to be used by the type-specific data handlers and the feature
handlers as base classes.

**Class Inheritance Diagram:**

.. inheritance-diagram:: coreutils.datautils.base_handler
    :parts: 2

**CB_DICT_KEY_BASE_ Keys:** These keys can be used while creating the cb_dict for
:class:`.DataHandler`

.. data::  CB_DICT_KEY_BASE_MODULES

    Key to specify the modules to look up for class definitions.

.. data::  CB_DICT_KEY_BASE_CLASSNAME_GEN

    Key to specify a callback method which can return the classname string from
    the given string.
"""


import abc


# Keys available for usage in cb_dict
CB_DICT_KEY_BASE_MODULES = 'base_modules'
CB_DICT_KEY_BASE_CLASSNAME_GEN = 'base_classname_gen'

# Characters used as delimiters for string -> list conversion
_CHAR_NEWLINE = '\n'
_CHAR_COMMA = ','


class BaseHandler(object):
    """Abstract base class to be used by all data handlers.

    :param data_handler: Handle to :class:`.DataHandler` that contains the
        current mapping of data handlers.

    .. data:: data_handler

        Reference to the data_handler passed via params.

    .. data:: cb_dict

        Reference to the cb_dict of the data_handler.
    """
    __metaclass__ = abc.ABCMeta

    def __init__(self, data_handler):
        """Do any initializations and checks."""
        from . import DataHandler
        assert isinstance(data_handler, DataHandler)

        self.data_handler = data_handler
        self.cb_dict = data_handler.cb_dict

    def _generic_get_data_handler(self, i_value):
        """Calls :meth:`.DataHandler.get_data_handler`."""
        return self.data_handler.get_data_handler(i_value)

    #--------------------------------------------------------------------------
    # Various properties that must be defined by the data handler if they
    # support the feature.
    #--------------------------------------------------------------------------
    @property
    def _normalizer(self):
        """Returns the :class:`BaseNormalizer` object for the data type."""
        raise NotImplementedError('Data handler: "' + self.__class__.__name__ + '" does not support data normalization.')

    @property
    def _validator(self):
        """Returns the :class:`BaseValidator` object for the data type."""
        raise NotImplementedError('Data handler: "' + self.__class__.__name__ + '" does not support data validation.')

    @property
    def _transfer_mgr(self):
        """Returns the :class:`BaseTransferMgr` object for the data type."""
        raise NotImplementedError('Data handler: "' + self.__class__.__name__ + '" does not support data transfer.')

    @property
    def _code_generator(self):
        """Returns the :class:`BaseCodeGenerator` object for the data type."""
        raise NotImplementedError('Data handler: "' + self.__class__.__name__ + '" does not support code generation.')

    #--------------------------------------------------------------------------
    # Suppress setting of these properties
    #--------------------------------------------------------------------------
    @_normalizer.setter
    def _normalizer(self, value):
        raise RuntimeError('Normalizer must not be set directly for data handler: ' + self.__class__.__name__)

    @_validator.setter
    def _validator(self, value):
        raise RuntimeError('Validator must not be set directly for data handler: ' + self.__class__.__name__)

    @_transfer_mgr.setter
    def _transfer_mgr(self, value):
        raise RuntimeError('TransferMgr must not be set directly for data handler: ' + self.__class__.__name__)

    @_code_generator.setter
    def _code_generator(self, value):
        raise RuntimeError('CodeGenerator must not be set directly for data handler: ' + self.__class__.__name__)

    #--------------------------------------------------------------------------
    # Methods that must be implemented by the derived data handler
    #--------------------------------------------------------------------------
    @classmethod
    def get_data_handler_data_type(cls):
        """Returns the data type that is handled by the data handler"""
        raise NotImplementedError('Data handler: "' + cls.__name__ + '" must implement get_data_handler_data_type.')

    @classmethod
    def get_data_handler(cls):
        """Returns the class reference of the data handler"""
        raise NotImplementedError('Data handler: "' + cls.__name__ + '" must implement get_data_handler.')

    #--------------------------------------------------------------------------
    # Implementation of the APIs exposed to DataHandler
    #--------------------------------------------------------------------------
    def normalize(self, *args, **kwargs):
        """Implements the normalization logic for the type of data supported
        by this class. Used by :meth:`.DataHandler.normalize`."""
        return self._normalizer.normalize(*args, **kwargs)

    def validate(self, *args, **kwargs):
        """Implements the validation logic for the type of data supported
        by this class. Used by :meth:`.DataHandler.validate`."""
        return self._validator.validate(*args, **kwargs)

    def transfer(self, *args, **kwargs):
        """Implements the transfer logic for the type of data supported
        by this class. Used by :meth:`.DataHandler.transfer`."""
        return self._transfer_mgr.transfer(*args, **kwargs)

    def generate_code(self, *args, **kwargs):
        """Implements the code generation logic for the type of data supported
        by this class. Used by :meth:`.DataHandler.generate_code`."""
        return self._code_generator.generate_code(*args, **kwargs)


def get_class_name(class_name, cb_dict):
    """Returns the updated class name using the the classname_gen.

    :param str class_name: The name of the class to query.
    :param dict cb_dict: See :class:`BaseHandler`.
    :returns: Reference to the class
    """
    classname_gen = cb_dict.get(CB_DICT_KEY_BASE_CLASSNAME_GEN, None)
    if classname_gen is None:
        raise RuntimeError('classname_gen not provided for getting the class name.\n'
                           '    ' + 'classname_gen: ' + str(classname_gen))
    return classname_gen(class_name)

def get_class_ref(class_name, cb_dict):
    """Returns the class reference corresponding to the class_name using the
        module to import the class from and the classname_gen to update the
        name of the class.

    :param str class_name: The name of the class to query.
    :param dict cb_dict: See :class:`BaseHandler`.
    :returns: Reference to the class
    """
    module = cb_dict.get(CB_DICT_KEY_BASE_MODULES, None)
    if module is None:
        raise RuntimeError('module not provided for getting the class reference.\n'
                           '    ' + 'module: ' + str(module))
    class_name = get_class_name(class_name, cb_dict)
    return getattr(module, class_name)


class BaseFeature(object):
    """Abstract base class to be used by all feature handlers.

    :param data_handler: Handle to a type specific data handler class derived
        from :class:`BaseHandler`.

    .. data:: data_handler

        Reference to the data_handler passed via params.

    .. data:: cb_dict

        Reference to the cb_dict of the data_handler.
    """
    __metaclass__ = abc.ABCMeta

    def __init__(self, data_handler):
        """Do any initializations and checks."""
        assert isinstance(data_handler, BaseHandler)
        self.data_handler = data_handler
        self.cb_dict = data_handler.cb_dict

    def _generic_get_data_handler(self, i_value):
        """Calls :meth:`.BaseHandler._generic_get_data_handler`."""
        return self.data_handler._generic_get_data_handler(i_value)


class BaseNormalizer(BaseFeature):
    """Abstract Normalizer base class to be used by the type-specific
    normalizers.

    :params: Same as :class:`BaseFeature`.
    """
    __metaclass__ = abc.ABCMeta

    def get_normalizer(self, i_value):
        """Gets the normalizer corresponding to the i_value"""
        return self._generic_get_data_handler(i_value)._normalizer

    def generic_normalize(self, *args, **kwargs):
        """Calls :meth:`.DataHandler.normalize`."""
        return self.data_handler.data_handler.normalize(*args, **kwargs)

    def generic_normalize_list(self, i_value_list):
        """Calls type-specific normalize_list"""
        return self.get_normalizer(i_value_list).normalize_list(i_value_list)

    @abc.abstractmethod
    def normalize(self, i_value):
        """See :meth:`.DataHandler.normalize`

        .. note::
            This method must be implemented by all derived classes.
        """
        raise NotImplementedError('Normalizer: "' + self.__class__.__name__ + '" must implement normalize.')

    def normalize_list(self, i_value_list):
        """Returns a list of normalized data from i_value_list.

        :param i_value_list: data to be normalized to a list
        :returns: Normalized i_value_list.
        :rtype: list

        .. note::
            This method must be implemented by derived classes that support
            normalizing data to a list.
        """
        raise NotImplementedError('Normalizer: "' + self.__class__.__name__ + '" does not support list normalization.')


class BaseValidator(BaseFeature):
    """Abstract Validator base class to be used by the type-specific
    validators.

    :params: Same as :class:`BaseFeature`.
    """
    __metaclass__ = abc.ABCMeta

    def get_validator(self, i_value):
        """Gets the validator corresponding to the i_value"""
        return self._generic_get_data_handler(i_value)._validator

    def generic_validate(self, *args, **kwargs):
        """Calls :meth:`.DataHandler.validate`."""
        return self.data_handler.data_handler.validate(*args, **kwargs)

    def generic_validate_list(self, i_value_list, i_format, i_type):
        """Calls type-specific validate_list"""
        return self.get_validator(i_type).validate_list(i_value_list, i_format)

    @abc.abstractmethod
    def validate(self, i_value, i_format):
        """See :meth:`.DataHandler.validate`

        .. note::
            This method must be implemented by all derived classes.
        """
        raise NotImplementedError('Validator: "' + self.__class__.__name__ + '" must implement validate.')

    def validate_list(self, i_value_list, i_format):
        """Returns a list of normalized and validated data from i_value_list.

        :param i_value_list: data to be normalized & validated
        :param i_format: Data for validation of i_value_list.
        :returns: Normalized i_value_list.
        :rtype: list

        .. note::
            This method must be implemented by derived classes that support
            validating list data
        """
        raise NotImplementedError('Validator: "' + self.__class__.__name__ + '" does not support list validation.')


class BaseTransferMgr(BaseFeature):
    """Abstract TransferMgr base class to be used by the type-specific
    transfer managers.

    :params: Same as :class:`BaseFeature`.
    """
    __metaclass__ = abc.ABCMeta

    def get_transfer_mgr(self, i_value):
        """Gets the transfer_mgr corresponding to the i_value"""
        return self._generic_get_data_handler(i_value)._transfer_mgr

    def generic_transfer(self, *args, **kwargs):
        """Calls :meth:`.DataHandler.transfer`."""
        return self.data_handler.data_handler.transfer(*args, **kwargs)

    @abc.abstractmethod
    def transfer(self, i_value_from, i_value_to, i_structure, i_structure_parent):
        """See :meth:`.DataHandler.transfer`

        .. note::
            This method must be implemented by all derived classes.
        """
        raise NotImplementedError('TransferMgr ' + str(self.__class__.__name__) + ' must implement transfer')


class BaseCodeGenerator(BaseFeature):
    """Abstract CodeGenerator base class to be used by the type-specific
    code generators.

    :params: Same as :class:`BaseFeature`.
    """
    __metaclass__ = abc.ABCMeta

    def get_code_generator(self, i_value):
        """Gets the code_generator corresponding to the i_value"""
        return self._generic_get_data_handler(i_value)._code_generator

    def generic_generate_code(self, *args, **kwargs):
        """Calls :meth:`.DataHandler.generate_code`."""
        return self.data_handler.data_handler.generate_code(*args, **kwargs)

    def generic_get_dict_rep(self, attr_key, attr_value):
        """Calls type-specific get_dict_rep"""
        return self.get_code_generator(attr_value).get_dict_rep(attr_key, attr_value)

    @abc.abstractmethod
    def generate_code(self, attr_key, attr_value):
        """See :meth:`.DataHandler.generate_code`

        .. note::
            This method must be implemented by all derived classes.
        """
        raise NotImplementedError('CodeGenerator ' + str(self.__class__.__name__) + ' must implement generate_code')

    def get_dict_rep(self, attr_key, attr_value):
        """Returns the dict representation of the attr_value.

        :params: same as :meth:`generate_code`
        :returns: Dictionary representation of the attr_value
        :rtype: dict

        .. note::
            This method must be implemented by derived classes that support
            dict representation of the attr_value data.
        """
        raise NotImplementedError('CodeGenerator ' + str(self.__class__.__name__) + ' does not support dict representation')


#------------------------------------------------------------------------------
# Restrict all import
#------------------------------------------------------------------------------
__all__ = ['CB_DICT_KEY_BASE_MODULES',
           'CB_DICT_KEY_BASE_CLASSNAME_GEN',
           'BaseHandler',
           'get_class_name',
           'get_class_ref',
           'BaseNormalizer',
           'BaseValidator',
           'BaseTransferMgr',
           'BaseCodeGenerator',]

