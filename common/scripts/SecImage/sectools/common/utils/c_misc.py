#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

"""
Created on Aug 10, 2012

@author: hraghav

This module provides several useful classes and methods.
"""

import hashlib
import imp
import os
import re
import sys
import traceback

from c_base import CoreError, CoreErrorCode
from c_logging import logger
import c_path


class CoreDynamicModule(object):
    """ Class to load a dynamic module into the process. """

    def __init__(self, modulePath):
        """
        Initializes internal variables.

        Parameters:
        1. modulePath (str): path to the module to be loaded.

        Attributes:
        1. path (str): path to the module loaded.
        2. name (str): name of the module loaded.
        3. fd (fd): File Descriptor of the module loaded
        4. handle (obj): Handle to the object loaded
        """
        if not c_path.validate_file(modulePath):
            raise AttributeError('Module does not exist')

        moduleName, extention = os.path.splitext(os.path.basename(modulePath))
        moduleFd = open(modulePath, 'r')
        try:
            moduleHandle = imp.load_source(moduleName, modulePath, moduleFd)
        except Exception:
            logger.debug(traceback.format_exc())
            moduleFd.close()
            raise

        self.path = modulePath
        self.name = moduleName
        self.fd = moduleFd
        self.handle = moduleHandle

    def close(self):
        """ Closes any open handles """
        self.fd.close()
        self.handle = None


class CoreSelfGeneratingClass(object):
    """ Class that automatically generates attributes based on the dictionary given """

    def __init__(self, dataDict):
        """
        Recursively creates new objects to comply with the given dictionary.

        Parameters:
        1. dataDict (str): Dictionary that specifies the structure to be created.
        """
        for dataTag, dataValue in dataDict.items():
            setattr(self, dataTag, CoreSelfGeneratingClass(dataValue) if isinstance(dataValue, dict) else dataValue)


class TablePrinter(object):

    SEPARATOR = '|'

    class CENTER: pass
    class LEFT: pass
    class RIGHT: pass

    class Merged_Start:
        def __init__(self, row_end, column_end, data):
            self.row_end = row_end
            self.column_end = column_end
            self.data = data

    class Merged:
        def __init__(self, init_row, init_column):
            self.init_row = init_row
            self.init_column = init_column

    def __init__(self, sep_rows=[0]):
        self.rows = dict()
        self.max_row = 0
        self.max_column = 0
        self.sep_rows = sep_rows

    def insert_data(self, row, column, data, row_end=None, column_end=None, justify=CENTER):
        assert isinstance(row, int)
        assert isinstance(column, int)
        assert isinstance(row_end, (int, type(None)))
        assert isinstance(column_end, (int, type(None)))

        # Add justification to data if justify is other than CENTER
        if justify == TablePrinter.RIGHT:
            data = data + ' '
        elif justify == TablePrinter.LEFT:
            data = ' ' + data

        # Sanitize the input and set max
        if row > self.max_row:
            self.max_row = row
        if column > self.max_column:
            self.max_column = column

        if row_end is not None:
            if row_end < row:
                raise RuntimeError('Row End: "' + str(row_end) + '" should not be less than row: "' + str(row) + '"')
            if row_end > self.max_row:
                self.max_row = row_end
        elif column_end is not None:
            if column_end < column:
                raise RuntimeError('Column End: "' + str(column_end) + '" should not be less than column: "' + str(column) + '"')
            if column_end > self.max_column:
                self.max_column = column_end

        # Populate the cell
        row_data = self.rows.setdefault(row, dict())
        cell_data = row_data.get(column, None)
        if isinstance(cell_data, (self.Merged, self.Merged_Start)):
            raise RuntimeError('Cell is merged. It cannot be set. Row: ' + str(row) + ', Column: ' + str(column))
        row_data[column] = (data, justify)

        # Set the merge cells
        if row_end is not None or column_end is not None:
            row_data[column] = self.Merged_Start(row_end, column_end, (data, justify))
            if row_end is None: row_end = row
            if column_end is None: column_end = column

            for m_row in range(row, row_end + 1):
                for m_column in range(column, column_end + 1):
                    if m_row == row and m_column == column:
                        continue

                    # Set cell as merged
                    row_data = self.rows.setdefault(m_row, dict())
                    cell_data = row_data.get(m_column, None)
                    if isinstance(cell_data, (self.Merged, self.Merged_Start)):
                        raise RuntimeError('Cell is merged. It cannot be set. Row: ' + str(row) + ', Column: ' + str(column))
                    row_data[m_column] = self.Merged(row, column)
        pass

    def get_data(self):
        return_value = ''
        column_widths = [0] * (self.max_column + 1)

        # Get the max length of this column
        for row in range(self.max_row + 1):
            for column in range(self.max_column + 1):
                cell_data = self.rows.setdefault(row, dict()).get(column, ('', TablePrinter.CENTER))
                if isinstance(cell_data, (self.Merged, self.Merged_Start)):
                    continue
                cell_data_len = len(str(cell_data[0]))
                if cell_data_len > column_widths[column]:
                    column_widths[column] = cell_data_len + 2

        # Populate the return value
        for row in range(self.max_row + 1):
            return_value += self.SEPARATOR
            for column in range(self.max_column + 1):
                cell_data = self.rows.setdefault(row, dict()).get(column, ('', TablePrinter.CENTER))
                if isinstance(cell_data, self.Merged):
                    continue
                elif isinstance(cell_data, self.Merged_Start):
                    column_width = 0
                    for col in range(column, cell_data.column_end + 1):
                        column_width += column_widths[col] + 1
                    column_width -= 1
                    cell_data = cell_data.data
                    column_width = max(column_width, len(cell_data))
                else:
                    column_width = column_widths[column]
                data = str(cell_data[0])
                just = cell_data[1]
                return_value += ((data.rjust(column_width) if just == TablePrinter.RIGHT else
                                  data.ljust(column_width) if just == TablePrinter.LEFT else
                                  data.center(column_width)) + self.SEPARATOR)

            # Add the separator is needed
            if row in self.sep_rows:
                return_value += '\n' + self.SEPARATOR
                for column in range(self.max_column + 1):
                    column_width = column_widths[column]
                    cell_data = '-' * column_width
                    return_value += cell_data.center(column_width) + self.SEPARATOR

            return_value += '\n'
        return_value = return_value.strip() + '\n'
        return return_value


def PROGRESS_CB_PYPASS(status_string, progress_percent): pass

class ProgressNotifier(object):

    def __init__(self, total, cb, stages=1):
        self.status = ''
        self.cur = 0
        self.cur_stage = 0

        self._total = total
        self._cb = cb
        self._stages = stages

    def push(self):
        self.cur_stage += 1
        if self.cur_stage > self._stages:
            self.cur_stage = self._stages
        self._cb(self.status, (((self.cur * 100) / self._total) +
                               ((100 * self.cur_stage) / (self._total * self._stages))))

    def complete(self):
        self._cb('Complete!', 100)


def compressBufferContents(linesList):
    """ Compresses the buffer in linesList, removing any empty lines """
    compressedOutput = []
    for eachLine in linesList:
        eachLine = re.split('[\r\n|\r|\n|\b]', eachLine)
        for eachSplitLine in eachLine:
            eachSplitLine = eachSplitLine.strip()
            if eachSplitLine != '':
                compressedOutput.append(eachSplitLine)
    return compressedOutput

def compareFileContents(file1=None, file2=None, file1_contents=None,
                        file2_contents=None):
    """
    Compare the contents of two files.
    IMP: Only one of file1 and file1_contents should be provided.
    IMP: Only one of file2 and file2_contents should be provided.

    Parameters:
    1. file1: File 1 to be compared
    2. file2: File 2 to be compared
    3. file1_contents: Contents of file 1 to be used for comparison.
    4. file2_contents: Contents of file 2 to be used for comparison.

    Return:
    1. returnValue: True if files match
    """
    returnValue = False

    if not ((file1 is None) ^ (file1_contents is None)):
        raise AttributeError('One of file1 or file1_contents must be given')
    if not ((file2 is None) ^ (file2_contents is None)):
        raise AttributeError('One of file2 or file2_contents must be given')

    if file1_contents is None:
        try:
            fd = open(file1, 'r')
        except Exception:
            logger.debug(traceback.format_exc())
            raise CoreError(CoreErrorCode.GENERIC_FAILURE,
                                'Opening file1: {0}'.format(sys.exc_info()[1]))
        else:
            file1_contents = fd.read()
            fd.close()

    if file2_contents is None:
        try:
            fd = open(file2, 'r')
        except Exception:
            logger.debug(traceback.format_exc())
            raise CoreError(CoreErrorCode.GENERIC_FAILURE,
                                'Opening file2: {0}'.format(sys.exc_info()[1]))
        else:
            file2_contents = fd.read()
            fd.close()

    if file1_contents is not None and file2_contents is not None:
        try:
            hashtotal_file1 = hashlib.md5()
            hashtotal_file2 = hashlib.md5()
            hashtotal_file1.update(file1_contents)
            hashtotal_file2.update(file2_contents)
            returnValue = hashtotal_file1.hexdigest() == hashtotal_file2.hexdigest()
        except Exception:
            logger.debug(traceback.format_exc())
            raise CoreError(CoreErrorCode.GENERIC_FAILURE,
                                'Getting hash: {0}'.format(sys.exc_info()[1]))

    return returnValue

def compareDirContents(dir1, dir2):
    """
    Compare the contents of two directories.

    Parameters:
    1. dir1: Dir 1 to be compared
    2. dir2: Dir 2 to be compared

    Return:
    1. returnValue: True if directories match
    """
    hashValue1 = getMD5Directory(dir1)
    hashValue2 = getMD5Directory(dir2)
    return hashValue1.strip() and hashValue1 == hashValue2

def getMD5Buffer(buffer_):
    hashValue = ''
    try:
        hashtotal = hashlib.md5()
        hashtotal.update(buffer_)
        hashValue = hashtotal.hexdigest()
    except Exception:
        raise CoreError(CoreErrorCode.GENERIC_FAILURE,
                            'Getting hash: {0}'.format(sys.exc_info()[1]))
    return hashValue

def getMD5File(filePath):
    """ Return md5 checksum for a file. """
    try:
        fd = open(filePath, 'r')
        contents = fd.read()
        fd.close()
    except Exception:
        raise CoreError(CoreErrorCode.GENERIC_FAILURE,
                            'Opening file for hash: {0}'.format(sys.exc_info()[1]))
    hashValue = getMD5Buffer(contents)
    return hashValue

def getMD5Directory(directory):
    """ Return md5 checksum for a directory. """
    hashValue = ''
    try:
        hashtotal = hashlib.md5()

        for eachFolder, eachSubFolder, eachFiles in os.walk(directory):
            for eachFile in eachFiles:
                fd = open(c_path.join(eachFolder, eachFile), 'r')
                contents = fd.read()
                fd.close()
                hashtotal.update(contents)

        hashValue = hashtotal.hexdigest()
    except Exception:
        raise CoreError(CoreErrorCode.GENERIC_FAILURE,
                            'Getting hash: {0}'.format(sys.exc_info()[1]))
    return hashValue

def backup_file(filePath, maxBackups=10):
    """
    Create backup for the file.
        File.txt -> File_1.txt

    Parameters:
    1. filePath: File to be backed up
    2. maxBackups: Maximum number of backups to create in the location

    Return:
    1. returnValue: True if file back up is successful
    """
    returnValue = False
    filename, extention = os.path.splitext(filePath)

    if c_path.validate_file(filePath) and c_path.validate_file_write(filePath):
        for index in reversed(range(0, maxBackups)):
            backup_file_path = filename + '_{0}'.format(index + 1) + extention
            origFile = filename + '_{0}'.format(index) + extention
            if c_path.validate_file(backup_file_path):
                try:
                    os.remove(backup_file_path)
                except Exception:
                    logger.debug(traceback.format_exc())
                    raise CoreError(CoreErrorCode.GENERIC_FAILURE,
                            'Removing file: {0}'.format(sys.exc_info()[1]))
            if c_path.validate_file(origFile):
                try:
                    os.rename(origFile, backup_file_path)
                except Exception:
                    logger.debug(traceback.format_exc())
                    raise CoreError(CoreErrorCode.GENERIC_FAILURE,
                            'Renaming file: {0}'.format(sys.exc_info()[1]))

        backup_file_path = filename + '_{0}'.format(1) + extention
        f_retValue, f_retErr = c_path.copyFile(filePath, backup_file_path,
                                                     force=True)
        if not f_retValue:
            raise CoreError(CoreErrorCode.GENERIC_FAILURE,
                            'Backing up: {0}'.format(f_retErr))
        else:
            returnValue = True
    return returnValue

def load_data_from_file(file_path):
    fd = open(file_path, 'rb')
    data = fd.read()
    fd.close()
    return data

def store_data_to_file(file_path, data):
    if (isinstance(data, str)):
        fd = open(file_path, 'wb')
        fd.write(data)
        fd.close()
    else:
        raise RuntimeError('Data to write must be of string type')

def store_debug_data_to_file(file_name, data=None, debug_dir=None):
    if debug_dir is None or data is None:
        return
    store_data_to_file(c_path.join(debug_dir, file_name), data)

def get_dups_in_list(data_list):
    return list(set([x for x in data_list if data_list.count(x) > 1]))

def stabilize_deepcopy():
    """Bug fix for copy.deepcopy in python2.6

    Issue Thread: http://bugs.python.org/issue1515
    Patch: http://hg.python.org/cpython/rev/83c702c17e02
    """
    import types
    from copy import deepcopy, _deepcopy_dispatch
    def _deepcopy_method(x, memo):
        return type(x)(x.im_func, deepcopy(x.im_self, memo), x.im_class)
    if types.MethodType not in _deepcopy_dispatch:
        _deepcopy_dispatch[types.MethodType] = _deepcopy_method
    else:
        pass

stabilize_deepcopy()

def properties_repr(properties):
    table_printer = TablePrinter([])
    idx = 0
    for string, val in properties:
        table_printer.insert_data(idx, 0, str(string), justify=TablePrinter.LEFT)
        table_printer.insert_data(idx, 1, str(val), justify=TablePrinter.LEFT)
        idx += 1
    return table_printer.get_data()

def obj_repr(self):
    retval = []
    for attr in sorted(self.__dict__.keys()):
        value = self.__dict__[attr]
        value = hex_addr(value) if isinstance(value, (int, long)) else value
        retval.append((str(attr), str(value)))
    return properties_repr(retval)

def hexdump(src, length=16):
    FILTER = ''.join([(len(repr(chr(x))) == 3) and chr(x) or '.' for x in range(256)])
    lines = []
    for c in xrange(0, len(src), length):
        chars = src[c:c+length]
        hex = ' '.join(["%02x" % ord(x) for x in chars])
        printable = ''.join(["%s" % ((ord(x) <= 127 and FILTER[ord(x)]) or '.') for x in chars])
        lines.append("%04x  %-*s  %s\n" % (c, length*3, hex, printable))
    return ''.join(lines)

def hex_addr(x, num=8):
    if isinstance(x, str):
        try:
            x = int(x, 16)
        except Exception:
            return x
    return ('0x%0' + str(num) + 'x') % x
