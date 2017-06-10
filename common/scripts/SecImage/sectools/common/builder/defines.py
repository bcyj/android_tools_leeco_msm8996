#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

import os

from sectools.common.utils import c_path


#------------------------------------------------------------------------------
# Common constants
#------------------------------------------------------------------------------
# Absolute path of the package
PACKAGE_PATH = os.path.dirname(os.path.realpath(__file__))

# Name of the root node of any config related objects
ROOTNODE_NAME = 'sectools_policy'


#------------------------------------------------------------------------------
# Auto-generated XML Parser related information
#------------------------------------------------------------------------------
# XML - Basic information
XML_PY_FILE_NAME = 'auto_gen_policy_parser.py'
XML_PY_PATH = c_path.join(PACKAGE_PATH, XML_PY_FILE_NAME)


# XML - lines at the beginning of the file
XML_PREPEND_LINES = '<?xml version="1.0" encoding="UTF-8"?>\n'

# XML - rootnode related constants
XML_ROOTNODE_NAMESPACE = 'tns:'
XML_ROOTNODE_NAME = ROOTNODE_NAME
XML_ROOTNODE = XML_ROOTNODE_NAMESPACE + XML_ROOTNODE_NAME

# XML - namespace related constants
XML_NAMESPACE_TNS = 'xmlns:tns="http://www.qualcomm.com/sectools_policy"'
XML_NAMESPACE_W3 = 'xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"'
XML_NAMESPACE_SCHEMA = 'xsi:schemaLocation="http://www.qualcomm.com/sectools_policy ../xsd/sectools_policy.xsd"'
XML_NAMESPACE = XML_NAMESPACE_TNS + '\n\t' + XML_NAMESPACE_W3 + '\n\t' + XML_NAMESPACE_SCHEMA

# XML - classname generator
XML_CLASSNAME_GEN = lambda x: 'complex_' + x

