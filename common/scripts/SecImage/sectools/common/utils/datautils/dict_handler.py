#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

"""Provides data handling capabilities for dict data.

**Class Inheritance Diagram:**

.. inheritance-diagram:: coreutils.datautils.dict_handler
    :parts: 2
"""


import sys

from base_handler import BaseValidator, BaseTransferMgr, \
    BaseCodeGenerator, BaseHandler, get_class_name, get_class_ref


_DICT_REPR_CODE = \
r'''    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))

'''


#------------------------------------------------------------------------------
# Keys that can be used in the i_format dictionary to specify validation
# options for dictionaries
#------------------------------------------------------------------------------
class DictHandlerKey_Attr:
    """Used as a key in the dictionary to hold a list of options.

    Ex usage:

    ::

        'person' :
        {
            DictHandlerKey_Attr : [DictHandlerAttr_Optional],
            'name' : '',
            'age' : 0,
        }
    """
    pass


class DictHandlerKey_Choice:
    """Used as a key in the dictionary to specify attributes whose value is
    restricted to a certain set.

    Ex usage:

    ::

        'shape' :
        {
            'type' : '',

            # The contents of this dictionary are choices
            DictHandlerKey_Choice :
            {
                # This provides a choice of perimeter objects
                'perimeter_choice' :
                {
                    # First choice is circle_data
                    'circle_data' :
                    {
                        'radius' : '',
                    }

                    # Second choice is quadrilateral_data
                    'quadrilateral_data' :
                    {
                        'length' : '',
                        'breadth' : '',
                    }
                }

                # This provides a choice of surface_area objects
                'surface_area_choice' :
                {
                    ...
                }
            }
        }
    """
    pass


#------------------------------------------------------------------------------
# Attributes used in the DictHandlerKey_Attr list
#------------------------------------------------------------------------------
class DictHandlerAttr_Optional:
    """Specifies that the i_value is optional."""
    pass


"""List of all the keys that are available for i_format."""
_DictHandlerKeysList = [DictHandlerKey_Attr, DictHandlerKey_Choice]


class DictHandler(BaseHandler):
    """Data handler for dict data.

    :params: Same as :class:`.base_handler.BaseHandler`.
    """

    def __init__(self, data_handler):
        """Initialized the various features."""
        BaseHandler.__init__(self, data_handler)
        self._i_validator = DictValidator(self)
        self._i_transfer_mgr = DictTransferMgr(self)
        self._i_code_generator = DictCodeGenerator(self)

    #--------------------------------------------------------------------------
    # Various properties that must be defined by the data handler if they
    # support the feature.
    #--------------------------------------------------------------------------
    @property
    def _validator(self):
        """Returns the :class:`DictValidator` object for the data type."""
        return self._i_validator

    @property
    def _transfer_mgr(self):
        """Returns the :class:`DictTransferMgr` object for the data type."""
        return self._i_transfer_mgr

    @property
    def _code_generator(self):
        """Returns the :class:`DictCodeGenerator` object for the data type."""
        return self._i_code_generator

    #--------------------------------------------------------------------------
    # Methods that must be implemented by the data handler
    #--------------------------------------------------------------------------
    @classmethod
    def get_data_handler_data_type(cls):
        """Returns the data type that is handled by this data handler"""
        return dict

    @classmethod
    def get_data_handler(cls):
        """Returns the class reference of this data handler"""
        return DictHandler


class DictValidator(BaseValidator):
    """Validator for Dict data.

    :params: Same as :class:`BaseValidator`.
    """

    def validate(self, i_value, i_format):
        """See :meth:`.Data_Handler.BaseValidator.validate`"""
        assert isinstance(i_format, dict)

        # Extract the DictHandlerKey_Attr data
        dict_attrs = i_format.get(DictHandlerKey_Attr, [])
        try:
            assert isinstance(dict_attrs, list)
        except Exception:
            raise RuntimeError('Dictionary contains "DictHandlerKey_Attr" key but the value is not list.' + '\n'
                               '    ' + 'DictHandlerKey_Attr : ' + '\n'
                               ('    ' * 2) + str(dict_attrs))

        # Extract the DictHandlerKey_Choice data
        dict_choices = i_format.get(DictHandlerKey_Choice, {})
        try:
            assert isinstance(dict_choices, dict)
        except Exception:
            raise RuntimeError('Dictionary contains "DictHandlerKey_Choice" key but the value is not dict.' + '\n'
                               '    ' + 'DictHandlerKey_Choice : ' + '\n'
                               ('    ' * 2) + str(dict_choices))

        # Check if the i_value is optional
        if DictHandlerAttr_Optional in dict_attrs and i_value is None:
            return i_value

        # Tag of the data being normalized
        attr_key = None
        o_value = i_value

        # Encapsulate the entire object to append the name of the tag being
        # normalized in any exceptions that are thrown.
        try:
            # Loop over the available choice groups
            for choice_group in dict_choices.values():
                assert isinstance(choice_group, dict)

                # Extract the DictHandlerKey_Attr data for choice group
                dict_choices_attrs = choice_group.get(DictHandlerKey_Attr, [])

                # Chosen data
                chosen_key, chosen_o_value, chosen_o_format = None, None, None

                # Loop over the choices from the choice group
                for choice, choice_format in choice_group.items():
                    if choice in _DictHandlerKeysList:
                        continue

                    # Get attribute from o_value
                    try: attr_value = getattr(o_value, choice)
                    except Exception: raise RuntimeError('o_value: "' + str(o_value) + '" missing attribute: "' + str(choice) + '"')

                    # Check if the attribute is valid
                    if attr_value is not None:
                        if chosen_key is None:
                            chosen_key = choice
                            chosen_o_value = attr_value
                            chosen_o_format = choice_format
                        else:
                            raise RuntimeError('Multiple tags given for choice: "' + str(choice_group.keys()) + '"')

                if chosen_key is None:
                    if DictHandlerAttr_Optional in dict_choices_attrs:
                        continue
                    else:
                        raise RuntimeError('Specify one tag from choice: "' + str(choice_group.keys()) + '"')

                # Normalize the chosen data
                attr_key = chosen_key
                chosen_o_value = self.generic_validate(chosen_o_value, chosen_o_format)
                setattr(o_value, chosen_key, chosen_o_value)

                # Clean the attr_key tag
                attr_key = None

            # Go over the mandatory attributes
            for i_format_key, i_format_value in i_format.items():
                if i_format_key in _DictHandlerKeysList:
                    continue

                # Ensure the structure data is of correct type
                assert isinstance(i_format_key, str)

                # Get attribute from o_value
                try: attr_value = getattr(o_value, i_format_key)
                except Exception: raise RuntimeError('o_value: "' + str(o_value) + '" missing attribute: "' + str(i_format_key) + '"')

                # Update the value using the normalized value
                attr_key = i_format_key
                attr_value = self.generic_validate(attr_value, i_format_value)
                setattr(o_value, i_format_key, attr_value)

                # Clean the attr_key tag
                attr_key = None

        except Exception as e:
            raise type(e), type(e)(('' if attr_key is None else (attr_key + '->')) + str(e)), sys.exc_info()[2]

        return o_value

    def validate_list(self, i_value_list, i_format):
        """See :meth:`.Data_Handler.BaseValidator.validate_list`"""
        assert isinstance(i_format, list)
        o_value_list = i_value_list

        # Ensure that the value is a list
        try: assert isinstance(o_value_list, list)
        except Exception: raise TypeError('o_value_list is not a list. o_value_list: ' + str(o_value_list) + ', i_value_list: ' + str(i_value_list))

        # Validate the values in the list
        try: o_value_list = [self.validate(val, i_format[0]) for val in o_value_list]
        except Exception as e:
            raise type(e), type(e)(str(e) + '\n'
                                   '    ' + 'i_value_list: ' + str(i_value_list) + '\n'
                                   '    ' + 'o_value_list: ' + str(o_value_list) + '\n'
                                   '    ' + 'i_format: ' + str(i_format)), sys.exc_info()[2]

        # Return the normalized dict list
        return o_value_list


class DictTransferMgr(BaseTransferMgr):
    """TransferMgr for Dict data.

    :params: Same as :class:`BaseTransferMgr`.
    """

    def transfer(self, i_value_from, i_value_to, i_structure, i_structure_parent):
        """See :meth:`.Data_Handler.BaseTransferMgr.transfer`"""
        assert isinstance(i_structure, dict)

        # Mark this as end of recursion
        if i_value_from is None:
            return None

        # Encapsulate the entire process in try block to append the name of the
        # tag being transferred in any exceptions that are thrown.
        try:
            if i_structure_parent in _DictHandlerKeysList:
                return i_value_to

            # Ensure the structure data is of correct type
            assert isinstance(i_structure_parent, str)
            assert isinstance(i_structure, dict)

            # Make choices mandatory
            dict_choices = i_structure.get(DictHandlerKey_Choice, {})
            assert isinstance(dict_choices, dict)

            # Loop over the available choice groups
            for choice_group in dict_choices.values():
                assert isinstance(choice_group, dict)

                # Loop over the choices from the choice group
                for choice, choice_format in choice_group.items():
                    if choice in i_structure.keys():
                        if choice_format != i_structure[choice]:
                            raise RuntimeError('Choice key "' + choice + '" already exists in the dictionary under: "' + str(i_structure_parent) + '"')
                    else:
                        i_structure[choice] = choice_format

            # Go over the mandatory attributes
            for i_structure_key, i_structure_value in i_structure.items():
                if i_structure_key in _DictHandlerKeysList:
                    continue

                # Ensure the structure data is of correct type
                assert isinstance(i_structure_key, str)

                # Get attribute from i_value_from
                try: value_from = getattr(i_value_from, i_structure_key)
                except Exception: raise RuntimeError('i_value_from: "' + str(i_value_from) + '" missing attribute: "' + str(i_structure_key) + '"')

                # Check if to_value does not exist
                if i_value_to is None:
                    i_value_to = get_class_ref(i_structure_parent, self.cb_dict)()

                try: value_to = getattr(i_value_to, i_structure_key)
                except Exception: raise RuntimeError('i_value_to: "' + str(i_value_to) + '" missing attribute: "' + str(i_structure_key) + '"')

                # Update the to_obj_value
                value_to = self.generic_transfer(value_from, value_to, i_structure_value, i_structure_key)

                # Set value in the i_value_to
                setattr(i_value_to, i_structure_key, value_to)

        except Exception as e:
            raise type(e), type(e)(('' if i_structure_key is None else (i_structure_key + '->')) + str(e)), sys.exc_info()[2]

        return i_value_to


class DictCodeGenerator(BaseCodeGenerator):
    """CodeGenerator for Dict data.

    :params: Same as :class:`BaseCodeGenerator`.
    """

    def generate_code(self, attr_key, attr_value, _classes={}):
        """See :meth:`.Data_Handler.BaseCodeGenerator.generate_code`"""
        return_val = ''

        # Ensure the structure data is of correct type
        assert isinstance(attr_key, str)
        assert isinstance(attr_value, dict)

        # Encapsulate the entire object to append the name of the tag being
        # used in any exceptions that are thrown.
        try:
            # Create the class definition
            class_name = get_class_name(attr_key, self.cb_dict)
            return_val += ('class ' + class_name + ':\n\n'
                           '    def __init__(self):\n')

            # Make choices mandatory
            dict_choices = attr_value.get(DictHandlerKey_Choice, {})
            assert isinstance(dict_choices, dict)

            # Loop over the available choice groups
            for choice_group in dict_choices.values():
                assert isinstance(choice_group, dict)

                # Loop over the choices from the choice group
                for choice, choice_format in choice_group.items():
                    if choice in attr_value.keys():
                        if choice_format != attr_value[choice]:
                            raise RuntimeError('Choice key "' + choice + '" already exists in the dictionary under: "' + str(attr_key) + '"')
                    else:
                        attr_value[choice] = choice_format

            if len(attr_value) == 0:
                return_val += ('        pass\n')
            else:
                for in_attr_key, in_attr_value in attr_value.items():
                    if in_attr_key in _DictHandlerKeysList:
                        continue

                    # Ensure the structure data is of correct type
                    assert isinstance(in_attr_key, str)

                    # Create the attr string
                    if isinstance(in_attr_value, dict):
                        attr_data = in_attr_key + ' = ' + get_class_name(in_attr_key, self.cb_dict) + '()'
                    else:
                        attr_data = self.generic_generate_code(in_attr_key, in_attr_value)

                    return_val += ('        self.' + attr_data + '\n')
            return_val += '\n'

            # Add the class representation code
            return_val += _DICT_REPR_CODE

            # Add a new line at the end of class code
            return_val += '\n'

            # Call function recursively for all the values with a dict representation
            for in_attr_key, in_attr_value in attr_value.items():
                if in_attr_key in _DictHandlerKeysList:
                    continue

                # Get the dictionary representation
                try: dict_rep = self.generic_get_dict_rep(in_attr_key, in_attr_value)
                except Exception: dict_rep = None

                if dict_rep is None:
                    continue

                assert isinstance(dict_rep, dict)
                matching_dict = _classes.get(in_attr_key, None)
                if matching_dict is not None:
                    if dict_rep == matching_dict:
                        # Skip generation of the same class again
                        continue
                    else:
                        raise RuntimeError('Found two classes with same name but different structure: "' + str(in_attr_key) + '"')

                # Update the list of _classes_dict
                _classes[in_attr_key] = dict_rep
                return_val += self.generate_code(in_attr_key, dict_rep, _classes)

        except Exception as e:
            raise type(e), type(e)(('' if attr_key is None else (attr_key + '->')) + str(e)), sys.exc_info()[2]

        return return_val

    def get_dict_rep(self, attr_key, attr_value):
        assert isinstance(attr_value, dict)
        return attr_value


#------------------------------------------------------------------------------
# Restrict all import
#------------------------------------------------------------------------------
__all__ = ['DictHandlerKey_Attr',
           'DictHandlerKey_Choice',
           'DictHandlerAttr_Optional',
           'DictHandler',
           'DictValidator',
           'DictTransferMgr',
           'DictCodeGenerator',]

