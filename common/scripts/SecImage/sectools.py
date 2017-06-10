#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

"""Provides a command line interface to the services provided by sectools

.. data:: SECTOOLS_TOOL_NAME

    Name of the tool

.. data:: SECTOOLS_TOOL_VERSION

    Version of the tool
"""


import optparse
import os
import sys
import traceback

from sectools import SECTOOLS_TOOL_NAME, SECTOOLS_TOOL_VERSION
from sectools.common.utils.c_logging import logger


# List of features
FEATURES_LIST = []
try:
    from sectools.features.fbc import fuseblower
    FEATURES_LIST.append(fuseblower)
except ImportError:
    pass
try:
    from sectools.features.isc import secimage
    FEATURES_LIST.append(secimage)
except ImportError:
    pass

if not FEATURES_LIST:
    raise RuntimeError('Sectools could not find any packaged features')

__version__ = SECTOOLS_TOOL_NAME + ' ' + SECTOOLS_TOOL_VERSION


class SectoolsParser(optparse.OptionParser):
    """Parser for command line arguments supported by Sectools."""

    def __init__(self):
        # Initialize the base parser
        optparse.OptionParser.__init__(self, usage=self.c_usage,
                                       description=self.c_description,
                                       version=self.c_version,
                                       epilog=self.c_epilog)
        self.c_add_options()
        self.opt_args, self.pos_args = self.parse_args(args=sys.argv[:2])

        if len(self.pos_args) == 1:
            self.print_help(sys.stdout)

    @property
    def c_usage(self):
        """(str) Returns the usage of the program.
        """
        return self.c_prog + ' [feature]'

    @property
    def c_prog(self):
        """(str) Returns the name of the program. By default this is the name
        of the python file being executed.
        """
        return os.path.basename(sys.argv[0])

    @property
    def c_description(self):
        """(str) Returns the description of the program."""
        return 'This program provides an interface to the sectools features'

    @property
    def c_version(self):
        """(str) Returns the version of the program."""
        return __version__

    @property
    def c_epilog(self):
        """(str) Returns the epilog for the program."""
        return ('\n'
                'Features available for sectools are: ' + '\n'
                '    ' + '\n    '.join(str(idx + 1) + '. ' + feature.CMD_ARG_TOOL_NAME for idx, feature in enumerate(FEATURES_LIST)) + '\n'
                '\n'
                'Example usage:' + '\n'
                '    ' + self.c_prog + ' ' + FEATURES_LIST[0].CMD_ARG_TOOL_NAME + ' -h' + '\n'
                '')

    def format_epilog(self, formatter):
        """This method is implemented to override the OptionParser's formatting
        of the epilog"""
        return self.epilog

    def c_add_options(self):
        """Adds the command line args supported by sectools."""
        pass

def main(args):
    """Parses the command line arguments, performs any basic operations based on
    the parsed arguments and starts processing using the isc module.
    """
    # Print the tool's launch command
    logger.debug2('\n\n    Sectools launched as: "' + ' '.join(sys.argv) + '"\n')

    if len(args) > 1:
        feature = args[1]
        for supported_feature in FEATURES_LIST:
            if feature == supported_feature.CMD_ARG_TOOL_NAME:
                supported_feature.main(supported_feature.parse_args(sys.argv[1:]))
                break
        else:
            raise RuntimeError('Feature provided from command line: "' + feature + '" is invalid.' + '\n'
                               '       ' + 'Please choose from : ' + str([f.CMD_ARG_TOOL_NAME for f in FEATURES_LIST]))

if __name__ == '__main__':
    try:
        # Check that the command line are valid and are normalized.
        args = SectoolsParser().pos_args

        main(args)

    except Exception:
        logger.debug(traceback.format_exc())
        logger.error(sys.exc_info()[1])
        sys.exit(1)

    except KeyboardInterrupt:
        print
        logger.error('Keyboard Interrupt Received. Exiting!')
        sys.exit(1)

    sys.exit(0)

