#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

"""Handles various operations on data including the following:

#. Normalization
#. Validation
#. Transfer
#. Generation of Code

The data types that support one or more of the aforementioned features are:

#. boolean
#. integer
#. string
#. path (custom)
#. tuple
#. list
#. dict

Most of the operations supported require use of dictionaries for providing
information about how to normalize/validate data.

.. seealso::
    Module :mod:`.samples`
        Usage of datautils.

"""


import copy
import os
import re
import sys

from ..c_logging import logger


class DataHandler(object):
    """The toplevel data handler class that uses the internal type-specific data
    handlers to provide data handling capabilities.

    :param cb_dict: Dictionary containing callbacks that maybe used by data
        handlers.

    .. data:: cb_dict

        Reference to the cb_dict that is provided in params.

    .. data:: data_handlers_map

        Running instance of the various data handlers which are all configured
        using the cb_dict.
    """

    # Mapping of the type of data to the handler of that data.
    _DATA_HANDLERS_MAP = dict()

    def __init__(self, cb_dict={}):
        """Do any initializations and checks."""
        assert isinstance(cb_dict, dict)

        self.cb_dict = cb_dict
        self.data_handlers_map = dict()

        for data_type, handler_class in self._DATA_HANDLERS_MAP.items():
            self.data_handlers_map[data_type] = handler_class(self)

    #--------------------------------------------------------------------------
    # APIs provided to the type-specific data handlers.
    #--------------------------------------------------------------------------
    @classmethod
    def register_data_handler(cls, data_type, handler_class):
        """Allows data handlers to register themselves for a certain data type.

        :param data_type: The type of data to handle. Eg: dict, list, etc
        :param handler_class: The class that will handle the data type. This
            class should be derived from :class:`.base_handler.BaseHandler`

        Example registration:
        ::

            DataHandler.register_data_handler(dict, DICT_HANDLER)

        """
        from base_handler import BaseHandler
        assert issubclass(handler_class, BaseHandler)

        if data_type not in cls._DATA_HANDLERS_MAP.keys():
            cls._DATA_HANDLERS_MAP[data_type] = handler_class
        else:
            raise RuntimeError('Cannot register data handler: "' + str(handler_class) + '" for data type: "' + str(data_type) + '"\n'
                               '    ' + 'Another data handler is registered for this data type: "' + str(cls._DATA_HANDLERS_MAP[data_type]) + '"')

    def get_data_handler(self, i_value):
        """Returns the data handler capable of handling the data type of i_value.

        :param i_value: The value for which the data handler is requested.
        :returns: Handler corresponding to the data type of i_value.
        """
        try:
            return self.data_handlers_map[type(i_value)]
        except Exception:
            logger.debug('get_data_handler: Exception raised for' + '\n'
                         '    ' + 'i_value: ' + str(i_value))
            raise RuntimeError('Data handler does not exist for data type :' + str(type(i_value)))

    #--------------------------------------------------------------------------
    # Public APIs for data handling
    #--------------------------------------------------------------------------
    def normalize(self, i_value):
        """Returns the normalized i_value.

        :param i_value: Data to be normalized.
        :returns: Normalized i_value.
        """
        return self.get_data_handler(i_value).normalize(i_value)

    def validate(self, i_value, i_format):
        """Validates i_value using the information in i_format. Returns the
        normalized i_value if validation passed.

        :param i_value: Data to be validated.
        :param i_format: Data for validation of i_value.
        :returns: Normalized & Validated config_value.
        """
        return self.get_data_handler(i_format).validate(i_value, copy.deepcopy(i_format))

    def transfer(self, i_value_from, i_value_to, i_structure, i_structure_parent):
        """Transfer data from one object to another based on the given
        i_structure.

        :param i_value_from: The object from which data is to be transfered.
        :param i_value_to: The object to be updated.
        :param i_structure: Specifies the format/structure of the objects.
        :param str i_structure_parent: Name of the parent obj_2 for i_structure.
        :returns: Updated i_value_to.
        """
        return self.get_data_handler(i_structure).transfer(i_value_from, i_value_to, copy.deepcopy(i_structure), i_structure_parent)

    def generate_code(self, i_attr_key, i_attr_value):
        """Returns a string of python code generated based on the given params.

        :param i_attr_key: The top level key for code generation.
        :param i_attr_value: Containing the value corresponding to the key.
        :returns: String containing python code.
        :rtype: str
        """
        return self.get_data_handler(i_attr_value).generate_code(i_attr_key, copy.deepcopy(i_attr_value))


#------------------------------------------------------------------------------
# Register the various data handlers
#------------------------------------------------------------------------------
import base_handler
import boolean_handler
import integer_handler
import string_handler
import path_handler
import tuple_handler
import list_handler
import dict_handler

_data_handlers_list = [boolean_handler.BooleanHandler,
                       integer_handler.IntegerHandler,
                       string_handler.StringHandler,
                       path_handler.PathHandler,
                       tuple_handler.TupleHandler,
                       list_handler.ListHandler,
                       dict_handler.DictHandler,]

for handler in _data_handlers_list:
    DataHandler.register_data_handler(handler.get_data_handler_data_type(),
                                       handler.get_data_handler())


#------------------------------------------------------------------------------
# More public APIs
#------------------------------------------------------------------------------
def get_cb_dict(base_modules=None,
                base_classname_gen=None,
                path_basepath=None,):
    """Returns the cb_dict created using the parameters provided.

    :param base_modules: See :mod:`base_handler`
    :param base_classname_gen: See :mod:`base_handler`
    :param path_basepath: See :mod:`path_handler`
    :returns: cb_dict (See :class:`DataHandler`)
    """
    return_val = dict()
    if base_modules is not None:
        return_val[base_handler.CB_DICT_KEY_BASE_MODULES] = base_modules
    if base_classname_gen is not None:
        return_val[base_handler.CB_DICT_KEY_BASE_CLASSNAME_GEN] = base_classname_gen
    if path_basepath is not None:
        return_val[path_handler.CB_DICT_KEY_PATH_BASEPATH] = path_basepath
    return return_val


#------------------------------------------------------------------------------
# Restrict all import
#------------------------------------------------------------------------------
__all__ = ['DataHandler',
           'dict_handler',
           'path_handler',
           'base_handler',
           'get_cb_dict',]
