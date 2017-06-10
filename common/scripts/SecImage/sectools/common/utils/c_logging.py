#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

'''
Created on Apr 19, 2013

@author: hraghav

This module provides the logging interface to be used for console, file, GUI
logging.
'''

import logging
import sys
import os
import time
import traceback


class _SecStreamHandler(logging.StreamHandler):

    def flush(self):
        if self.stream and hasattr(self.stream, "flush") and not self.stream.closed:
            self.stream.flush()

def _addHandler(logger, stream, verbosity):
    """ Adds a new handler to the given logger """
    logFormat = logging.Formatter('%(message)s')
    logHandler = _SecStreamHandler(stream)
    logHandler.setFormatter(logFormat)
    logHandler.setLevel(verbosity)
    logger.addHandler(logHandler)
    return logHandler

def _removeHandler(logger, handler=None):
    """ Removes the given/all handlers from the given logger """
    if handler is None:
        for logHandler in logger.handlers:
            logger.removeHandler(logHandler)
    else:
        logger.removeHandler(handler)


class _FileLogger(object):
    """ Class that contains information about a particular file logger. """

    def __init__(self, filePath, verbosity, mode, logger):
        """ Initialize a file logger """
        try:
            self.fd = open(filePath, mode)
        except Exception:
            logger.debug(traceback.format_exc())
            raise ValueError('{0}    \nFile: "{1}"'.format(sys.exc_info()[1], filePath))
        else:
            self.filePath = filePath
            self.verbosity = verbosity
            self.logger = logger
            self.logHandler = _addHandler(self.logger, self.fd, verbosity)

    def deinit(self):
        """ De-initialize the file logger """
        _removeHandler(self.logger, self.logHandler)
        self.fd.close()
        self.logger = None
        self.logHandler = None
        self.fd = None
        self.filePath = None


class CoreLogger(object):
    """
    Class that provides a custom logging interface.
    Allows logging to multiple destinations: console, files, GUI.
    Provides some user interaction methods.
    """

    # Levels of verbosity allowed
    CRITICAL = logging.CRITICAL
    ERROR = logging.ERROR
    WARN = logging.WARN
    NOTE = int((logging.WARN + logging.INFO) / 2)
    INFO = logging.INFO
    DEBUG = logging.DEBUG
    DEBUG2 = int(logging.DEBUG / 2)

    def __init__(self):
        """
        Initializes internal variables. The following attributes are public
        and should be configured by the user to specify the type of logging

        Attributes:
        1. logToGui (bool): Specify if logging to GUI is enabled
        2. guiLogger (GUI): If application is GUI based, set this to the GUI
            logger object.
        3. verbosity (enum): Can be one of the following:
            [obj.CRITICAL, obj.ERR, obj.WARN, obj.NOTE, obj.INFO, obj.DEBUG, obj.DEBUG2]
        4. outputStream (stream): Stream to which console should log
        """

        # Internal loggers
        self.guiLogger = None
        self._fileLogger = logging.getLogger('tool_file_logging')
        self._consoleLogger = logging.getLogger('tool_console_logging')
        self._fileLoggerHandlers = []
        self._fileLogger.setLevel(self.DEBUG2)
        self._consoleLogger.setLevel(self.DEBUG2)

        # Specify if logging to GUI
        self.logToGui = False

        # Internal variables
        self._outputStream = None
        self._verbosity = self.INFO

        # Default callbacks for prompts
        self.defaultPromptYesNo = None
        self.defaultPromptList = None

    @property
    def verbosity(self):
        """ Return the current level of verbosity for GUI/console """
        return self._verbosity

    @verbosity.setter
    def verbosity(self, level):
        """ Set verbosity to one of the allowed levels """
        if level in [self.CRITICAL, self.ERROR, self.WARN, self.NOTE, self.INFO,
                     self.DEBUG, self.DEBUG2]:
            self._verbosity = level
            _removeHandler(self._consoleLogger)
            _addHandler(self._consoleLogger, self.outputStream, level)
        else:
            raise ValueError('Invalid logging level: "{0}".'.format(level))

    @property
    def outputStream(self):
        """ Return the current console stream """
        return self._outputStream

    @outputStream.setter
    def outputStream(self, stream):
        """ Set output stream """
        self._outputStream = stream
        _removeHandler(self._consoleLogger)
        _addHandler(self._consoleLogger, stream, self.verbosity)

    def add_file_logger(self, filePath, verbosity, mode='w'):
        """ Add a file logger to the given file path and returns a LoggerID """
        self._fileLoggerHandlers.append(_FileLogger(filePath, verbosity, mode,
                                                    self._fileLogger))
        return len(self._fileLoggerHandlers) - 1

    def removeFileLogger(self, fileLoggerID):
        """ Remove the given file logger """
        if fileLoggerID >= 0 and fileLoggerID < len(self._fileLoggerHandlers):
            self._fileLoggerHandlers[fileLoggerID].deinit()
            del self._fileLoggerHandlers[fileLoggerID]
        else:
            raise ValueError('Invalid logger ID: "{0}".'.format(fileLoggerID))

    def getFileLoggerPath(self, fileLoggerID):
        """ Get the file path for a file logger """
        if fileLoggerID >= 0 and fileLoggerID < len(self._fileLoggerHandlers):
            return self._fileLoggerHandlers[fileLoggerID].filePath
        else:
            raise ValueError('Invalid logger ID: "{0}".'.format(fileLoggerID))

    def _msg(self, msg, level, raw=False):
        """
        Internal method used to log a message to all the enabled loggers at the
        given level of verbosity.

        Parameters:
        1. msg (str): The message to log.
        2. level (lvl): The verbosity level to log the message at.
        3. raw (bool): True - message should be logged as is
                       False - use formatting specified for the logger
        """

        if self.logToGui:
            self.guiLogger(msg, raw)
        else:
            if raw and level >= self.verbosity:
                self.outputStream.write(msg)
                self.outputStream.flush()
            else:
                self._consoleLogger.log(level, msg)

        if self._fileLoggerHandlers:
            if raw:
                for fileLogger in self._fileLoggerHandlers:
                    if level >= fileLogger.verbosity:
                        fileLogger.fd.write(msg)
                        fileLogger.fd.flush()
            else:
                self._fileLogger.log(level, msg)

    def debug2(self, msg, raw=False):
        """
        Log a debug2 message. If raw is True, the message is logged as is
        without any extra formatting.
        """
        self._msg(('' if raw else 'DEBUG2: ') + str(msg), self.DEBUG2, raw)

    def debug(self, msg, raw=False):
        """
        Log a debug message. If raw is True, the message is logged as is
        without any extra formatting.
        """
        self._msg(('' if raw else 'DEBUG: ') + str(msg), self.DEBUG, raw)

    def info(self, msg, raw=False):
        """
        Log an info message. If raw is True, the message is logged as is
        without any extra formatting.
        """
        self._msg(msg, self.INFO, raw)

    def note(self, msg, raw=False):
        """
        Log a note message. If raw is True, the message is logged as is
        without any extra formatting.
        """
        self._msg(('' if raw else 'NOTE: ') + str(msg), self.NOTE, raw)

    def warning(self, msg, raw=False):
        """
        Log a warning message. If raw is True, the message is logged as is
        without any extra formatting.
        """
        self._msg(('' if raw else 'WARNING: ') + str(msg), self.WARN, raw)

    def error(self, msg, raw=False):
        """
        Log an error message. If raw is True, the message is logged as is
        without any extra formatting.
        """
        self._msg(('' if raw else 'ERROR: ') + str(msg), self.ERROR, raw)

    def critical(self, msg, raw=False):
        """
        Log a critical message. If raw is True, the message is logged as is
        without any extra formatting.
        """
        self._msg(('' if raw else 'CRITICAL: ') + str(msg), self.CRITICAL, raw)

    def printMsg(self, msg, verbosity):
        if verbosity >= self.verbosity:
            self.outputStream.write(msg)
            self.outputStream.flush()

    def dynamicMessage(self, verbosity):
        """
        Params:
        1. verbosity: One of verbosity enums.

        Returns:
        The appropriate dynamic message object.
        """
        if self.logToGui:
            return self.guiLogger._DynamicMessage(verbosity)
        else:
            return _CoreDynamicMessage(verbosity)

    def progressBar(self, totalCount, verbosity):
        """
        Params:
        1. verbosity: One of verbosity enums.

        Returns:
        The appropriate progress bar object.
        """
        if self.logToGui:
            return self.guiLogger._ProgressBar(totalCount, verbosity)
        else:
            return _CoreProgressBar(totalCount, verbosity)

    def askYesNo(self, askMessage):
        """
        Calls the correct logger based on priority to ask the user a yes/no
        question. The priority is: GUI > Console.

        Parameters:
        1. askMessage (str): The question to be asked from user.

        Return:
        1. returnValue (bool): True - User selected yes
                               False - User selected no
        """

        returnValue = False
        # Check with the callback first to get a reply
        if self.defaultPromptYesNo is not None:
            returnValue = self.defaultPromptYesNo(askMessage)
            if returnValue is not None:
                return returnValue
            else:
                returnValue = False

        if self.logToGui:
            returnValue = self.guiLogger.askYesNo(askMessage)
        else:
            reply = None
            while reply is None:
                self.error('\n{0}\nUSER INPUT NEEDED: Press "y" for yes, "n" for no: '.format(askMessage), raw=True)
                reply = raw_input()
                if reply == 'y':
                    self.error('User entered "yes"\n', raw=True)
                    returnValue = True
                elif reply == 'n':
                    self.error('User entered "no"\n', raw=True)
                    returnValue = False
                else:
                    self.error('INVALID input: "{0}". Try again...\n'.format(reply), raw=True)
                    reply = None
            self.error('\n')
        return returnValue

    def askListOptions(self, askMessage, optionsList):
        """
        Calls the correct logger based on priority to ask the user to select
        from a list of options. The priority is: GUI > Console.

        Parameters:
        1. askMessage (str): The question to be asked from user.
        2. optionsList (list[str]): The list of options available to the user.

        Return:
        1. returnValue (int): index in the list of user's selection.
        """

        returnValue = 0

        if len(optionsList) == 0:
            raise ValueError('Empty List')

        # Check with the callback first to get a reply
        if self.defaultPromptList is not None:
            returnValue = self.defaultPromptList(askMessage, optionsList)
            if ((returnValue is not None) and (isinstance(returnValue, int)) and
                    (returnValue >= 0) and (returnValue < len(optionsList))):
                return returnValue
            else:
                returnValue = 0

        if len(optionsList) > 1:
            if self.logToGui:
                returnValue = self.guiLogger.askListOptions(askMessage,
                                                            optionsList)
            else:
                returnValue = None
                while returnValue is None:
                    # Print the question
                    self.error('\n{0}:\n'.format(askMessage), raw=True)

                    # Print the options
                    for index, eachOption in enumerate(optionsList):
                        self.error('{0}. {1}\n'.format(index + 1, eachOption), raw=True)

                    self.error('\nUSER INPUT NEEDED: Please select from one of the options above: ', raw=True)
                    returnValue = raw_input()

                    try:
                        returnValue = int(returnValue)
                    except Exception:
                        returnValue = None
                    else:
                        if returnValue > len(optionsList) or returnValue <= 0:
                            self.error('INVALID option: "{0}"! Please try again...\n'.format(returnValue), raw=True)
                            returnValue = None
                        else:
                            self.error('User entered "{0}. {1}"\n'.format(returnValue, optionsList[returnValue - 1]), raw=True)
                            returnValue -= 1
                self.error('\n', raw=True)

        return returnValue

    def warningTimerContinue(self, msg, timer=30):
        """
        Calls the correct logger based on priority to print a warning message
        and ask the user to cancel before a timeout, otherwise returns to
        continue. The priority is: GUI > Console.

        Parameters:
        1. msg (str): The message to be printed as a warning.
        2. timer (int): The timeout for user to cancel.

        Return:
        1. returnValue (bool): True: User did not cancel.
                               False: User cancelled.
        """
        returnValue = True

        if self.logToGui:
            returnValue = self.guiLogger.warningTimerContinue(msg, timer)
        else:
            warningMsg = _CoreDynamicMessage(logger.WARN)
            try:
                # Print the warning message
                self.error(
"""
    ***** WARNING: {0} *****

    Terminate (Ctrl-C) now if you dont want to continue.\n\n""".format(msg), raw=True)

                # Start the count-down
                for i in reversed(range(0, timer)):
                    warningMsg.printMessage('    ***** CONTINUING IN "{0}" SECONDS. *****'.format(i))
                    time.sleep(1)

            except KeyboardInterrupt:
                warningMsg.printMessage('    ***** STOPPING *****')
                returnValue = False

            warningMsg.complete()
            self.error('\n', raw=True)

        return returnValue

    def joinMessages(self, seperator, tab, *messages):
        """ Joins two messages. """
        returnMessage = ''

        if tab: tab = '    '
        else: tab = ''

        for eachMessage in messages:
            if eachMessage.strip():
                if tab:
                    eachMessage = eachMessage.split('\n')
                    eachMessage = ('\n' + tab).join(eachMessage)
                if returnMessage.strip():
                    returnMessage = returnMessage + seperator + tab + eachMessage
                else:
                    returnMessage = eachMessage
        return returnMessage

    def increase_indent(self, lines, tab_len = 2):
        return '\n'.join([''.join([' ' for i in range(0, tab_len)]) + line for line in lines.split('\n')])

    def enable_file_logging(self, toolname, num_logs=50, log_dir=None):
        """ Find a tmp location for file logging """
        from c_misc import backup_file
        import c_path

        if log_dir is None:
            log_dir = ''
            for tmp_path in [os.getenv('TEMP'), os.getenv('TMP')]:
                if c_path.validate_dir(tmp_path):
                    log_dir = c_path.join(tmp_path, '{0}_logs'.format(toolname))
                    break
            else:
                self.warning('CANNOT FIND A LOCAL TEMP DIRECTORY TO CREATE LOGS')

        # Set Log Folder
        if log_dir:
            log_file = c_path.join(log_dir, '{0}_log.txt'.format(toolname))
            log_file_debug = c_path.join(log_dir, '{0}_log_debug.txt'.format(toolname))

            # Create the logs directory
            retval, reterr = True, ''
            try:
                c_path.create_dir(log_dir);
            except Exception as e:
                retval, reterr = False, str(e)

            if not retval:
                self.warning('CANNOT CREATE DIRECTORY FOR LOGGING FILE: {0}\n	Path: {1}'.format(reterr, log_dir))
            elif num_logs > 1 and c_path.validate_file(log_file) and not backup_file(log_file, num_logs):
                self.warning('FAILED TO BACKUP LOGGING FILE: \n	Path: {0}'.format(log_file))
            elif num_logs > 1 and c_path.validate_file(log_file_debug) and not backup_file(log_file_debug, num_logs):
                self.warning('FAILED TO BACKUP DEBUG LOGGING FILE: \n	Path: {0}'.format(log_file_debug))
            else:
                self.add_file_logger(log_file, self.INFO, 'w')
                self.info('Logging to {0}'.format(log_file))
                # Only create debug file if debug is enabled
                if self.verbosity < self.INFO:
                    self.add_file_logger(log_file_debug, self.verbosity, 'w')
                    self.debug('Debug logging to {0}'.format(log_file_debug))
                return log_file, log_file_debug
        return '', ''

""" Global instance variable of CoreLogger """
logger = CoreLogger()
logger.outputStream = sys.stdout


class _CoreDynamicMessage(object):
    """
    Class that provides a way to print dynamic messages to the console.
    This means all messages printed using the class are printed in one line.
    """

    def __init__(self, verbosity):
        """ Initializes internal variables """
        self.lastMessageLength = 0
        self.lastMessage = ''
        self.verbosity = verbosity

    def printMessage(self, message):
        """
        Prints the given message on the given line.

        Parameters:
        1. message (str): The message to be printed. The message must not end
            with a newline
        """
        messageLength = len(message)
        messageToPrint = '\r{0} '.format(message)
        for i in range(0, self.lastMessageLength - messageLength):
            messageToPrint += ' '
        logger.printMsg(messageToPrint, self.verbosity)
        self.lastMessage = message
        self.lastMessageLength = messageLength

    def clean(self):
        """ Cleans any message and returns cursor to the start of the line. """
        messageToPrint = '\r'
        for i in range(0, self.lastMessageLength):
            messageToPrint += ' '
        messageToPrint += '\r'
        logger.printMsg(messageToPrint, self.verbosity)
        self.lastMessageLength = 0

    def complete(self):
        """ Prints a newline. """
        self.clean()
        logger._msg(self.lastMessage, self.verbosity, raw=True)


class _CoreProgressBar(object):
    """ Class that provides a way to print a progress bar on the console. """

    def __init__(self, totalCount, verbosity):
        """
        Sets the total count for the progress.

        Parameters:
        1. totalCount (int): The number that specifies the progress is 100%
        """
        self.totalCount = totalCount
        self.totalBars = 20
        self.totalPercentage = 100
        self.lastPercentage = -1
        self.dynamicMessage = _CoreDynamicMessage(verbosity)
        self.verbosity = verbosity
        logger.printMsg('\n', self.verbosity)

    def updateProgress(self, updatedCount):
        """
        Updates the progress bar on screen based on the given count.

        Parameters:
        1. updatedCount (int): The number that specifies the current count out
            of the totalCount specified at object initialization.
        """
        updatedPercentage = self.totalPercentage * updatedCount
        updatedPercentage /= self.totalCount

        if self.lastPercentage != updatedPercentage:
            updatedBars = self.totalBars * updatedPercentage
            updatedBars /= self.totalPercentage

            messageToPrint = '    ['
            for i in range(0, self.totalBars):
                if i == self.totalBars / 2:
                    messageToPrint += ' {0} '.format(updatedPercentage)
                if i < updatedBars:
                    messageToPrint += '#'
                else:
                    messageToPrint += ' '
            messageToPrint += ']'
            self.dynamicMessage.printMessage(messageToPrint)
            self.lastPercentage = updatedPercentage

    def complete(self):
        """ Prints a newline. """
        self.dynamicMessage.complete()
        logger.printMsg('\n', self.verbosity)


