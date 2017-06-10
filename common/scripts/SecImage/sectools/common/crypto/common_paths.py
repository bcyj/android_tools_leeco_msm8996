#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

import os
import sys
from subprocess import Popen,PIPE

current_file_dir = os.path.dirname(os.path.realpath(__file__))
proj_dir = os.path.join(current_file_dir, '..', '..', '..')
proj_dir = os.path.abspath(proj_dir)

openssl_binary_path = os.path.join(proj_dir, 'bin', 'WIN','openssl.exe')
if sys.platform=='linux2':
    openssl_binary_path = Popen(["which", "openssl"], stdout=PIPE).communicate()[0].strip()
