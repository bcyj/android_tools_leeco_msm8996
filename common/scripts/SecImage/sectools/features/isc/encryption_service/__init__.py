#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

# API to register the encdecs
def _register_encdec(ident, encdec):
    if ident in ENCDECS:
        raise RuntimeError('Trying to register encdec: ' + str(encdec) + ' for ID: ' + str(ident) + '\n'
                           'Another encdec: ' + str(ENCDECS[ident]) + ' is already registered for this ID')
    ENCDECS[ident] = encdec

# API to query encdec class by ident
def _get_encdec_class_by_ident(ident):
    # Check encdec exists
    if ident not in ENCDECS:
        raise RuntimeError('Encdec unavailable for ID: ' + str(ident) + '\n'
                           '    ' + 'Available IDs are: ' + str(ENCDECS.keys()))

    # Get the encdec
    return ENCDECS[ident]

# API to query a encdec
def get_encdec(imageinfo):
    ident = imageinfo.encryption.selected_encryptor
    encdec_class = _get_encdec_class_by_ident(ident)
    return encdec_class(imageinfo=imageinfo, debug_dir=imageinfo.dest_image.debug_dir_encdec)

# Make sure this module is not already initialized
try:
    ONETIME
except Exception:
    ONETIME = True

if ONETIME is True:
    # Map of ident to encdec
    ENCDECS = {}

    # List of encdecs that are available
    from sectools.features.isc.encryption_service import unified
    from sectools.features.isc.encryption_service import ssd

    # Dont do this again
    ONETIME = False

