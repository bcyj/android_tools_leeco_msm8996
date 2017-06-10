#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

from abc import ABCMeta, abstractmethod
import os
import sectools.common.crypto.utility_functions
import subprocess
import shutil
import sys

class BaseBuilderUtil(object):
    __metaclass__=ABCMeta

    @abstractmethod
    def isKeyEnable(self, key_list):
        raise NotImplementedError()

    @abstractmethod
    def printdebuginfo(self, mesg):
        raise NotImplementedError()

    @abstractmethod
    def printinfo(self, mesg):
        raise NotImplementedError()

    @abstractmethod
    def getenv(self, key, default):
        raise NotImplementedError()

    @abstractmethod
    def envsubst(self, string):
        raise NotImplementedError()

    @abstractmethod
    def execcmds(self, cmds, target):
        raise NotImplementedError()

    @abstractmethod
    def returnError(self, errorMsg):
        raise NotImplementedError()

    @abstractmethod
    def installas(self, target, source):
        raise NotImplementedError()

    @abstractmethod
    def getReturnValueInBuilder(self, target, source):
        raise NotImplementedError()

    @abstractmethod
    def loadToolScript(self, path):
        raise NotImplementedError()

    @abstractmethod
    def pilsplit(self, target, source):
        raise NotImplementedError()


class BuilderUtil(object):
    def __init__(self, env):
        '''
        Constructor
        '''
        if env is not None:
            self.builderutil = SconsBuilderUtil(env)
        else:
            self.builderutil = GenericBuilderUtil()


    def isKeyEnable(self, key_list):
        if key_list is None:
            return False

        return self.builderutil.isKeyEnable(key_list)

    def printdebuginfo(self, mesg):
        return self.builderutil.printdebuginfo(mesg)

    def printinfo(self, mesg):
        return self.builderutil.printinfo(mesg)

    def getenv(self, key, default):
        return self.builderutil.getenv(key, default)

    def envsubst(self, string):
        return self.builderutil.envsubst(string)

    def execcmds(self, cmds, target):
        return self.builderutil.execcmds(cmds, target)

    def returnError(self, errorMsg):
        return self.builderutil.returnError(errorMsg)

    def installas(self, target, source):
        return self.builderutil.installas(target, source)

    def getReturnValueInBuilder(self, target):
        return self.builderutil.getReturnValueInBuilder(target)

    def loadToolScript(self, path):
        return self.builderutil.loadToolScript(path)

    def pilsplit(self, target, source):
        return self.builderutil.pilsplit(target, source)

class SconsBuilderUtil(BaseBuilderUtil):
    def __init__(self, env):
        self.env = env

    def isKeyEnable(self, key_list):
        return self.env.IsKeyEnable(key_list)

    def printdebuginfo(self, mesg):
        self.env.PrintDebugInfo('sectools_builder', mesg)

    def printinfo(self, mesg):
        self.env.PrintInfo('sectools_builder: ' + mesg)

    def getenv(self, key, default):
        return self.env.get(key, default)

    def envsubst(self, string):
        return self.env.subst(string)

    def execcmds(self, cmds, target):
        data, err, rv = self.env.ExecCmds(' '.join(cmds), target=target)
        return rv

    def returnError(self, errorMsg):
        import SCons
        self.env.PrintError("")
        self.env.PrintError("-------------------------------------------------------------------------------")
        self.env.PrintError(errorMsg)
        self.env.PrintError("-------------------------------------------------------------------------------")
        self.env.PrintError("")
        SCons.Script.Exit(1)

    def installas(self, target, source):
        return self.env.InstallAs(target, source)

    def getReturnValueInBuilder(self, target):
        return None

    def loadToolScript(self, path):
        return self.env.LoadToolScript(path)

    def pilsplit(self, target, source):
        return self.env.PilSplitterBuilder(target, source)


class GenericBuilderUtil(BaseBuilderUtil):
    def _str2bool(self, v):
        return v.lower() in ("yes", "true", "t", "1")

    def isKeyEnable(self, key_list):
        ret = False

        if type(key_list) is not list:
            key_list = [key_list]

        for key in key_list:
            key_enable = self._str2bool(os.environ.get(key, "false"))
            ret = ret | key_enable
            #print "%s:%s" % (key, key_enable)

        return ret

    def printdebuginfo(self, mesg):
        print mesg

    def printinfo(self, mesg):
        print mesg

    def getenv(self, key, default):
        return os.environ.get(key, default)

    # No-op
    def envsubst(self, string):
        return string

    def execcmds(self, cmds, target):
        try:
            output = subprocess.check_output(cmds, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError, e:
            if self._isPython27Supported():
                print e.output
            raise e
        return output

    def _isPython27Supported(self):
        return sys.version_info>(2,7,0)

    def returnError(self, errorMsg):
        print ""
        print "-------------------------------------------------------------------------------"
        print "ERROR: " + errorMsg
        print "-------------------------------------------------------------------------------"
        print ""
        exit(1)

    def installas(self, target, source):
        path, file = os.path.split(target)
        if not os.path.exists(path):
            os.makedirs(path)
        shutil.copyfile(source, target)

        return target

    def getReturnValueInBuilder(self, target):
        return target

    def loadToolScript(self, path):
        raise NotImplementedError()

    def pilsplit(self, target, source):
        raise NotImplementedError()