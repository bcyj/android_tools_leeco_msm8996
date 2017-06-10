#******************************************************************************
#  @file    qcril_header_parser.py
#  @brief   Parses ril.h and generates qcril_feature_def.h
#
#  ---------------------------------------------------------------------------
#
#  Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
#  Qualcomm Technologies Proprietary and Confidential.
#  ---------------------------------------------------------------------------
#******************************************************************************/

import sys
import string

out_header = open(string.lstrip(str(sys.argv[2])) + '/qcril_qmi/qcril_features_def.h','w');
ril_header = open(string.lstrip(str(sys.argv[1])) + '/hardware/ril/include/telephony/ril.h', 'r');

def check_for_symbol(symbol):
    ril_header.seek(0);
    exist = False;
    for line in ril_header:
        if symbol in line:
            exist = True;
            return exist;

    return exist;

def append_feature_def_to_file(line):
    out_header.write(line + '\n');

# Push File header
append_feature_def_to_file('/******************************************************************************')
append_feature_def_to_file('  @file    qcril_features_def.h')
append_feature_def_to_file('  @brief   qcril feature definition information')
append_feature_def_to_file('')
append_feature_def_to_file('  DESCRIPTION')
append_feature_def_to_file('')
append_feature_def_to_file('  ---------------------------------------------------------------------------')
append_feature_def_to_file('')
append_feature_def_to_file('  Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.')
append_feature_def_to_file('  Qualcomm Technologies Proprietary and Confidential.')
append_feature_def_to_file('  ---------------------------------------------------------------------------')
append_feature_def_to_file('******************************************************************************/')

append_feature_def_to_file('#ifndef QCRIL_FEATURES_DEF_H')
append_feature_def_to_file('#define QCRIL_FEATURES_DEF_H')

# Introduce the rules for each feature definition here

# feature definition for extended error call cause
if check_for_symbol('CALL_FAIL_INVALID_TRANSIT_NW_SELECTION'):
    append_feature_def_to_file('#define EXTENDED_FAIL_ERROR_CAUSE_FOR_VOICE_CALL (1)')

append_feature_def_to_file('#endif /* QCRIL_FEATURES_DEF_H */')
ril_header.close();
out_header.close();
