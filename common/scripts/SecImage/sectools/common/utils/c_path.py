#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

'''
Created on Apr 19, 2013

@author: hraghav
'''

from c_logging import logger
import os
import re
import shutil
import stat
import sys
import time
import traceback

def normalize(path):
    """ Returns the normalized path """
    if path:
        path = path.strip()
        if path != '':
            if not path.startswith('\\\\'):
                path = os.path.abspath(path)
            path = os.path.normpath(path)
    return path

def join(dirname, *args):
    """ Returns the concatenation of directory and its subdir/file. """
    return normalize(os.path.join(normalize(dirname), *args))

def flatten(path):
    """
    Flattens the path provided, so that a directory can be created with
    that path.
    Flattening involves replacing all backslashes(\) and periods(.) with
    underscore(_).
    """
    return normalize(path).replace('\\', '_').replace(':', '').replace('.', '_')

def _handler_readonly(func, path, exc):
    """ Internal helper function for clean_dir """
    if not os.access(path, os.W_OK):
        os.chmod(path, stat.S_IWUSR)
        func(path)
    else:
        raise

def clean_file(filename):
    if filename is not None and os.path.isfile(filename):
        os.remove(filename)

def clean_dir(path, timer=30):
    """
    Removes all the files from the given directory.
    IMPORTANT: Use this method with caution.

    Parameters:
    1. Path (str): The directory that needs to be cleared of files.

    Returns:
    1. returnValue (bool): True
    2. returnError (str): Error if the cleaning of the directory failed
    """

    path = normalize(path)
    if not path:
        return False, 'Invalid path given'

    # Notify user if directory contains files
    if validate_dir(path):
        if not len(os.listdir(path)):
            return True, ''

        if not logger.warningTimerContinue('Cleaning directory with existing files: "{0}"'.format(path), timer=timer):
            return False, 'User canceled cleanup'

    # Try three times to clean the directory
    returnValue = False
    returnError = ''
    for i in range (0, 3):
        if i:
            time.sleep(i)

        # Try deleting the directory
        try:
            shutil.rmtree(path, ignore_errors=False, onerror=_handler_readonly)
        except OSError as (err, strerror):
            pass
        except Exception:
            logger.debug(traceback.format_exc())
            returnError += '{0}\n'.format(sys.exc_info()[1])

        # Verify directory deletion
        if validate_dir(path):
            returnError += 'Directory deletion failed\n'
            continue

        # Create directory
        try:
            os.makedirs(path)
        except OSError as (err, strerror):
            returnError += '{0}\n'.format(strerror)
            returnError += 'Directory creation failed\n'
            continue
        except Exception:
            logger.debug(traceback.format_exc())
            returnError += '{0}\n'.format(sys.exc_info()[1])

        # Verify directory creation
        if not validate_dir_write(path):
            returnError += 'Directory creation failed\n'
            continue

        returnValue = True
        break

    return returnValue, returnError.strip()

def validate_file(path):
    """ Returns True if path is an existing file and read access is available """
    return (os.access(path, os.F_OK or os.R_OK) and os.path.isfile(path))

def validate_file_write(path):
    """
    Returns True if read/write access is available to path and path is not
    a directory, so that a file can be created if not existing
    """
    returnValue = False
    path = normalize(path)
    if validate_dir_write(os.path.dirname(path)):
        if validate_file(path) or (not validate_dir(path)):
            returnValue = True
    return returnValue

def validate_dir(path):
    """ Returns True if path is an existing directory and read access is available """
    return (os.access(path, os.F_OK or os.R_OK) and os.path.isdir(path))

def validate_dir_write(path):
    """ Returns True if path is an existing directory and read/write access is available """
    returnValue = True

    if not ((validate_dir(path) and os.access(path, os.W_OK))):
        returnValue = False
    else:
        testfile = join(path, '_core_test_file')
        # Cleanup
        try: os.remove(testfile)
        except Exception: pass
        # Create file
        try: open(testfile, 'w').close()
        except Exception: returnValue = False
        # Cleanup
        try: os.remove(testfile)
        except Exception: pass

    return returnValue

def create_dir(directory):
    """Creates the directory if doesn't exist already"""
    if not validate_dir_write(directory):
        if not validate_dir(directory):
            os.makedirs(directory)
            if not validate_dir_write(directory):
                raise RuntimeError('Could not create directory')
        else:
            raise RuntimeError('No write access to directory')

def create_debug_dir(directory=None):
    """Creates the directory if doesn't exist already.
    If directory is None, returns without doing anything.
    """
    if directory is not None:
        return create_dir(directory)

def recursiveSearchDirectory(path, dirRegex=None, dirName=None,
                             maxDepth=3, _curDepth=0):
    """
    Searches for a directory in the given path.
    This is a recursive method.
    IMP: Only one of dirRegex and dirName should be provided.

    Parameters:
    1. path: Path to search for directories in
    2. dirRegex: Regular expression that specifies the directory. This is
        case-insensitive.
    3. dirName: Name of the directory to be searched. This is case
        insensitive.
    4. maxDepth: Maximum depth in the directory tree to search in.
    5. _curDepth: Current depth (internal). DONT set this.

    Return:
    1. returnValue: True if directory is found
    2. returnPath: Path of the directory if found
    """

    returnValue = False
    returnPath = ''

    if not ((dirRegex is None) ^ (dirName is None)):
        raise AttributeError('One of dirRegex or dirName must be given')

    # This marks the end of recursion
    if _curDepth > maxDepth:
        return returnValue, returnPath

    for eachFolder in os.listdir(path):
        dirFullPath = join(path, eachFolder)
        if not validate_dir(dirFullPath):
            continue

        if dirRegex and re.match(r'{0}'.format(dirRegex), eachFolder,
                                 re.IGNORECASE):
            returnValue = True
            returnPath = dirFullPath
            break
        elif dirName and (eachFolder.lower() == dirName.lower()):
            returnValue = True
            returnPath = dirFullPath
            break
        else:
            f_returnValue, f_returnPath = recursiveSearchDirectory(
                dirFullPath, dirRegex=dirRegex, dirName=dirName,
                             maxDepth=maxDepth, _curDepth=_curDepth + 1)
            if f_returnValue:
                returnValue = f_returnValue
                returnPath = f_returnPath
                break

    return returnValue, returnPath

def recursiveSearchFile(path, fileRegex, curDepth=0, maxDepth=3):
    """
    Searches for a file in the given path.
    This is a recursive method.

    Parameters:
    1. path: Path to search for file in.
    2. fileRegex: Regular expression that specifies the file. This is
        case-insensitive.
    3. maxDepth: Maximum depth in the directory tree to search in.
    4. _curDepth: Current depth (internal). DONT set this.

    Return:
    1. returnValue: True if file is found
    2. returnPath: Path of the file if found
    """

    returnValue = False
    returnPath = ''

    # This marks the end of recursion
    if curDepth > maxDepth:
        return returnValue, returnPath

    for eachFile in os.listdir(path):
        fileFullPath = join(path, eachFile)
        if not validate_dir(fileFullPath):
            if validate_file(fileFullPath) and re.match(
                    r'{0}'.format(fileRegex), eachFile, re.IGNORECASE):
                returnValue = True
                returnPath = fileFullPath
                break
        else:
            f_returnValue, f_returnPath = recursiveSearchFile(
                fileFullPath, fileRegex, curDepth + 1, maxDepth)
            if f_returnValue:
                returnValue = f_returnValue
                returnPath = f_returnPath
                break

    return returnValue, returnPath

def copyFile(src=None, dest=None, src_contents=None, force=True):
    """
    Copies the contents of src to the dest.
    IMP: Only one of src and src_contents should be provided.

    Parameters:
    1. src: Source file to copy
    2. dest: Destination file to create.
    3. src_contents: Contents of the source file to be used to create the
        destination file.
    4. force: Remove file if it already exists.

    Return:
    1. returnValue: True if destination file is created
    2. returnError: Error if file copy failed
    """

    if not ((src is None) ^ (src_contents is None)):
        raise AttributeError('One of src or src_contents must be given')

    if not validate_dir_write(os.path.dirname(dest)):
        return False, 'No write access to directory'

    if validate_file(dest):
        if not force:
            return False, 'Dest file already exists'
        else:
            try:
                os.chmod(dest, stat.S_IRWXU or stat.S_IRWXG or stat.S_IRWXO)
                os.remove(dest)
            except Exception:
                return False, 'Removing file: {0}'.format(sys.exc_info()[1])

    if validate_file(dest):
        return False, 'Could not remove dest file'

    if src:
        try:
            shutil.copy2(src, dest)
        except Exception:
            return False, 'Copying file: {0}'.format(sys.exc_info()[1])
    elif src_contents:
        try:
            fd = open(dest, 'w')
        except Exception:
            return False, 'Writing file: {0}'.format(sys.exc_info()[1])
        else:
            fd.write(src_contents)
            fd.flush()
            fd.close()

    return True, ''

def copyDir(src, dest, force=True):
    """
    Copies the files from src to dest.

    Parameters:
    1. src: Source directory.
    2. dest: Destination directory
    4. force: Clear directory if it already exists

    Return:
    1. returnValue: True if folder is copied
    2. returnError: Error if folder copy failed
    """

    if not validate_dir_write(os.path.dirname(dest)):
        return False, 'No write access to directory'

    if validate_dir(dest):
        if not force:
            return False, 'Dest folder already exists'
        else:
            f_returnValue, f_returnError = clean_dir(dest)
            if not f_returnValue:
                return False, 'Clearing dest dir. {0}'.format(f_returnError)
            try:
                os.rmdir(dest)
            except Exception:
                return False, 'Removing dir. {0}'.format(sys.exc_info()[1])

    if validate_dir(dest):
        return False, 'Could not remove dest dir'

    try:
        shutil.copytree(src, dest)
    except Exception:
        return False, 'Copying folder: {0}'.format(sys.exc_info()[1])

    return True, ''
