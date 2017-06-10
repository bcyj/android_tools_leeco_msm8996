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
ROOTNODE_NAME = 'SSD_METADATA'


#------------------------------------------------------------------------------
# Auto-generated XML Parser related information
#------------------------------------------------------------------------------
# XML - Basic information
XML_PY_FILE_NAME = 'auto_gen_ssd_xml_config.py'
XML_PY_PATH = c_path.join(PACKAGE_PATH, XML_PY_FILE_NAME)


# XML - lines at the beginning of the file
XML_PREPEND_LINES = '<?xml version="1.0" encoding="UTF-8"?>\n'

# XML - rootnode related constants
XML_ROOTNODE_NAMESPACE = 'xs:'
XML_ROOTNODE_NAME = ROOTNODE_NAME
XML_ROOTNODE = XML_ROOTNODE_NAMESPACE + XML_ROOTNODE_NAME

# XML - namespace related constants
XML_NAMESPACE_SSD = 'xmlns:tns="http://www.qualcomm.com/ssd_enc_param"'
XML_NAMESPACE_W3 = 'xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"'

XML_NAMESPACE_SCHEMA = 'xsi:schemaLocation="http://www.qualcomm.com/fuseblower ../xsd/fuseblower.xsd"'
XML_NAMESPACE_SCHEMA_USER = 'xsi:schemaLocation="http://www.qualcomm.com/fuseblower ../xsd/fuseblower_user.xsd"'

XML_NAMESPACE = XML_NAMESPACE_SSD + '\n\t' + XML_NAMESPACE_W3 + '\n\t' + XML_NAMESPACE_SCHEMA
XML_NAMESPACE_USER = XML_NAMESPACE_SSD + '\n\t' + XML_NAMESPACE_W3 + '\n\t' + XML_NAMESPACE_SCHEMA_USER

# XML - classname generator
XML_CLASSNAME_GEN = lambda x: 'complex_' + x

