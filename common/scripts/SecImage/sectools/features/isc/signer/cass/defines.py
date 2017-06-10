#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

import os

CASS_DIR = os.path.dirname(os.path.realpath(__file__))

CASS_CLIENT_REFAPP_DIR=os.path.join(CASS_DIR, 'cass-client-refapp')
CLIENT_REFAPP_JAR =os.path.join('bin', 'java', 'cass-client-refapp.jar')
CASS_CLIENT_REFAPP_CONFIG_DIR=os.path.join(CASS_CLIENT_REFAPP_DIR, 'config')
CASS_CLIENT_REFAPP_INPUT_DIR=os.path.join(CASS_CLIENT_REFAPP_DIR, 'input')
CASS_CLIENT_REFAPP_OUTPUT_DIR=os.path.join(CASS_CLIENT_REFAPP_DIR, 'output')
