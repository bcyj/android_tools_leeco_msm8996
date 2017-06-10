# Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.

import os
import subprocess
import sys
#import shared_objects

log_levels={'SILENT'  :-4,
            'FATAL'   :-3,
            'ERROR'   :-2,
            'WARNING' :-1,
            'STATUS'  : 0,
            'VERBOSE' : 1,
            'DEBUG'   : 2,
            'DEBUG2'  : 3}

def inverse_dict(my_dict):
    return dict([[value, key] for key, value in my_dict.items()])

log_levels_reverse=inverse_dict(log_levels)

debug={'level':log_levels['STATUS']}
log_fp=None

#----------------------------------------------------------------------------
# log
#----------------------------------------------------------------------------
def log_at_level(*args, **kwargs):#source, message, level=log_levels['ERROR'], manual_string=False, my_log_fp=shared_objects.log_fp, prepend=''):
    source_found=False
    source=None
    message_found=False
    message=None
    level_found=False
    level=None
    manual_string_found=False
    manual_string=None
    my_log_fp_found=False
    my_log_fp=None
    prepend_found=False
    prepend=None

    if len(args)>6:
        error_str=''
        raise RuntimeError, error_str
    if len(args)>0:
        source_found=True
        source=args[0]
    if len(args)>1:
        message_found=True
        message=args[1]
    if len(args)>2:
        level_found=True
        level=args[2]
    if len(args)>3:
        manual_string_found=True
        manual_string=args[3]
    if len(args)>2:
        my_log_fp_found=True
        my_log_fp=args[2]
    if len(args)>2:
        prepend_found=True
        prepend=args[2]

    new_kwargs=kwargs
    if 'source' in new_kwargs.keys():
        if source_found:
            error_str=''
            raise RuntimeError, error_str
        source_found=True
        source=new_kwargs['source']
        new_kwargs=copy_dict_excluding_keys(new_kwargs, keys_to_exclude=['source'])

    if 'message' in new_kwargs.keys():
        if message_found:
            error_str=''
            raise RuntimeError, error_str
        message_found=True
        message=new_kwargs['message']
        new_kwargs=copy_dict_excluding_keys(new_kwargs, keys_to_exclude=['message'])

    if 'level' in new_kwargs.keys():
        if level_found:
            error_str=''
            raise RuntimeError, error_str
        level_found=True
        level=new_kwargs['level']
        new_kwargs=copy_dict_excluding_keys(new_kwargs, keys_to_exclude=['level'])

    if 'manual_string' in new_kwargs.keys():
        if manual_string_found:
            error_str=''
            raise RuntimeError, error_str
        manual_string_found=True
        manual_string=new_kwargs['manual_string']
        new_kwargs=copy_dict_excluding_keys(new_kwargs, keys_to_exclude=['manual_string'])

    if 'my_log_fp' in new_kwargs.keys():
        if my_log_fp_found:
            error_str=''
            raise RuntimeError, error_str
        my_log_fp_found=True
        my_log_fp=new_kwargs['my_log_fp']
        new_kwargs=copy_dict_excluding_keys(new_kwargs, keys_to_exclude=['my_log_fp'])

    if 'prepend' in new_kwargs.keys():
        if prepend_found:
            error_str=''
            raise RuntimeError, error_str
        prepend_found=True
        prepend=new_kwargs['prepend']
        new_kwargs=copy_dict_excluding_keys(new_kwargs, keys_to_exclude=['prepend'])

    if len(new_kwargs.keys()):
        error_str=''
        raise RuntimeErrror, error_str

    if not source_found:
        error_str=''
        raise RuntimeError, error_str

    if not message_found:
        error_str=''
        raise RuntimeError, error_str

    if not level_found:
        level=log_levels['ERROR']

    if not manual_string_found:
        manual_string=False

    if not prepend_found:
        prepend=''

    message_string="{0}{1}: {2} - {3}".format(prepend, log_levels_reverse[level], source, message)
    if manual_string:
        message_string=prepend+message
    if debug["level"]>=level:
        if not my_log_fp_found:
            log(s=message_string)
        else:
            log(s=message_string, my_log_fp=my_log_fp)

    return message_string

def log(*args, **kwargs):#s, my_log_fp=shared_objects.log_fp, recurse_count=0):
    recurse_limit=1
    using_dev_null=False
    s_found=False
    s=None
    my_log_fp_found=False
    my_log_fp=None
    recurse_count_found=False
    recurse_count=None

    if len(args)>3:
        error_str=''
        raise RuntimeError, error_str
    if len(args)>0:
        s_found=True
        s=args[0]
    if len(args)>1:
        my_log_fp_found=True
        my_log_fp=args[1]
    if len(args)>2:
        recurse_count_found=True
        recurse_count=args[2]

    new_kwargs=kwargs
    if 's' in new_kwargs.keys():
        if s_found:
            error_str=''
            raise RuntimeError, error_str
        s_found=True
        s=new_kwargs['s']
        new_kwargs=copy_dict_excluding_keys(new_kwargs, keys_to_exclude=['s'])

    if 'my_log_fp' in new_kwargs.keys():
        if my_log_fp_found:
            error_str=''
            raise RuntimeError, error_str
        my_log_fp_found=True
        my_log_fp=new_kwargs['my_log_fp']
        new_kwargs=copy_dict_excluding_keys(new_kwargs, keys_to_exclude=['my_log_fp'])

    if 'recurse_count' in new_kwargs.keys():
        if my_log_fp_found:
            error_str=''
            raise RuntimeError, error_str
        recurse_count_found=True
        recurse_count=new_kwargs['recurse_count']
        new_kwargs=copy_dict_excluding_keys(new_kwargs, keys_to_exclude=['recurse_count'])

    if len(new_kwargs.keys()):
        error_str=''
        raise RuntimeErrror, error_str

    if not s_found:
        error_str=''
        raise RuntimeError, error_str

    if not my_log_fp_found:
        my_log_fp=log_fp
    if not recurse_count_found:
        recurse_count=0

    ### DEBUG ###
    #print 'logger {0}'.format(shared_objects.log_fp)
    #print my_log_fp

    print s

    if my_log_fp is None:
        my_log_fp=open_file(os.devnull,"wb")
        using_dev_null=True
    try:
        my_log_fp.write(s)
        my_log_fp.write("\n")
    except Exception, e:
        error_str='ERROR: common_lib.log - Write to file operation failed. {0}'.format(e)
        print 'Recurse {0}'.format(recurse_count)
        log(s=error_str, my_log_fp=log_fp, recurse_count=recurse_count+1)

    if using_dev_null:
        try:
            my_log_fp.close()
        except Exception, e:
            error_str='ERROR: common_lib.log - Close file operation failed for /dev/null. {0}'.format(e)
            if recurse_count>recurse_limit:
                print error_str
            else:
                print 'Recurse {0}'.format(recurse_count)
                log(s=error_str, my_log_fp=log_fp, recurse_count=recurse_count+1)
            raise e

def open_file(file_name, mode='rb'):
    try:
        fp = open(file_name, mode)
    except Exception, e:
        error_str="The file '{0}' could not be opened: {1}".format(file_name,e)
        log_at_level(source='common_lib.open_file', message=error_str,
                     level=log_levels['ERROR'])
        raise e

    # File open has succeeded with the given mode, return the file object
    return fp

def set_log_fp(log_fp_passed):
    global log_fp
    log_fp = log_fp_passed

def set_debug_level(level):
    global debug
    debug["level"] = int(level)

def external_call(command, capture_output=True, print_capture=False, posix=None, shell=False, raise_except=True,
                  my_log_fp=log_fp):
    errors = None
    output = None
    cmd_list = None
    log_str=None

    if posix==None:
        if os.name.lower()=='nt':
            posix=False
        else:
            posix=True

    #Prepare a list conversion of the command line string if needed
    if capture_output:
        if isinstance(command, basestring):
            log_str="Calling SYSTEM with\n\"{0}\"".format(command)

            import shlex
            cmd_list=shlex.split(command,posix=posix)

            log_at_level(source='common_lib.external_call', message='shlex.split returned "{0}"'.format(repr(cmd_list)),
                         level=log_levels['ERROR'], my_log_fp=my_log_fp)

            cmd_string=command
        elif isinstance(command, list) or isinstance(command, tuple):
            cmd_string=' '.join(command)
            cmd_list=command
            log_str="Calling SYSTEM with\n\"{0}\"\n".format(command)
            log_str+="cmd_list is '{0}'".format(repr(cmd_list))
        else:
            short_error_str='command must be either a basestring, a list, or a tuple, or a derivitive of one of those'
            error_str=log_at_level(source='common_lib.external_call', message=short_error_str,
                                   level=log_levels['ERROR'], my_log_fp=my_log_fp)
            raise RuntimeError, error_str

    if log_str==None:
        pass
    else:
        log_at_level(source='common_lib.external_call', message=log_str,
                     level=log_levels['DEBUG'], my_log_fp=my_log_fp, prepend="\n")
    debug_str="Doing {0}system call: '{1}'".format('(captured) ' if capture_output else '',cmd_string)
    log_at_level(source='common_lib.external_call', message=debug_str,
                 level=log_levels['DEBUG'], my_log_fp=my_log_fp)

    #Execute command
    if capture_output:
        try:
            p = subprocess.Popen(cmd_list, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=shell)
        except Exception, e:
            short_error_str="Error executing command '{0}'. Reason: Error starting subprocess '{1}'".format(cmd_string,e)
            error_str=log_at_level(source='common_lib.external_call', message=short_error_str,
                                   level=log_levels['ERROR'], my_log_fp=my_log_fp)
            raise e

        try:
            (output, errors) = p.communicate() #blocks until process finishes
        except Exception, e:
            short_error_str="Error executing command '{0}'. Reason: Error communicating with subprocess. '{1}'".format(cmd_string,e)
            error_str=log_at_level(source='common_lib.external_call', message=short_error_str,
                                   level=log_levels['ERROR'], my_log_fp=my_log_fp)
            raise e

        debug_str="Command output>>>\n{0}<<<".format(output)
        if errors==None or errors=='':
            pass
        else:
            debug_str+="Process stderr: >>>\n{0}<<<".format(errors)
        log_at_level(source='common_lib.external_call', message=debug_str,
                     level=log_levels['DEBUG'], my_log_fp=my_log_fp)

        try:
            return_code=p.poll()
        except Exception, e:
            short_error_str="Error executing command '{0}'. Reason: Error polling subprocess for return code. '{1}'\n".format(cmd_string,e) + \
                            'Process stderr: {0}'.format(errors)
            error_str=log_at_level(source='common_lib.external_call', message=short_error_str,
                                   level=log_levels['ERROR'], my_log_fp=my_log_fp)
            raise e
        debug_str="Return code from app was '{0}'".format(return_code)
        log_at_level(source='common_lib.external_call', message=debug_str,
                     level=log_levels['DEBUG'], my_log_fp=my_log_fp)

        if return_code==0:
            pass
        else:
            short_error_str="Error executing command '{0}'. Reason: Subprocess returned a non-zero exit code. Stdout:\n".format(cmd_string) + \
                      "{0}\n".format(output) + \
                      "Process stderr:\n" + \
                      "{0}".format(errors)
            level_to_print_at=log_levels['DEBUG']
            if print_capture:
                level_to_print_at=log_levels['WARNING']
                if raise_except:
                    level_to_print_at=log_levels['ERROR']
            error_str=log_at_level(source='common_lib.external_call', message=short_error_str,
                                   level=level_to_print_at, my_log_fp=my_log_fp)
            if raise_except:
                #raise RuntimeError(error_str)
                if (not errors is None) and (not errors == ""):
                    cp_error_list=[return_code,cmd_list,errors,output]
                else:
                    cp_error_list=[return_code,cmd_list,error_str]
                if sys.version_info >= (2, 7):
                    cp_exception=subprocess.CalledProcessError(*(cp_error_list[0:3]))
                    if len(cp_error_list)>3:
                        cp_exception.stdout=cp_error_list[3]
                    raise cp_exception
                elif sys.version_info >= (2, 6):
                    new_cp_error_list=[cp_error_list[0],cp_error_list[1]]
                    cp_exception=subprocess.CalledProcessError(*new_cp_error_list)
                    cp_exception.output=cp_error_list[2]
                    if len(cp_error_list)>3:
                        cp_exception.stdout=cp_error_list[3]
                    raise cp_exception
                else:
                    raise RuntimeError, 'Python version is below required version. Need atleast 2.6, but version is' + \
                                        '%s' % '.'.join(sys.version_info)

        if print_capture:
            output_str=None
            if not output is None:
                output_str="Result: {0}".format(output)
            if (not errors is None) and (not errors == ""):
                output_str="{0}Process stderr: {1}".format(output_str+"\n" if output_str is not None else '', errors)
            if output_str is not None:
                log_at_level(source='common_lib.external_call', message=output_str,
                             level=log_levels['STATUS'], my_log_fp=my_log_fp)
    else:
        try:
            os.system(command)
        except Exception, e:
            short_error_str="Error executing command '%s'. Reason: %s" % (str(command), e)
            log_at_level(source='common_lib.external_call', message=short_error_str,
                         level=log_levels['ERROR'], my_log_fp=my_log_fp)
            #clean_up()
            raise e

    return (output,errors,return_code)

def find_command_location(command, exit_on_fail=False, my_log_fp=log_fp):
    import os

    abspath_command=command
    where_command=None
    output=''

    if os.name.lower()=='nt' and sys.platform is not 'cygwin':
        (head_path,tail_path)=os.path.split(abspath_command)
        if head_path==None or head_path=='':
            where_command=['where',abspath_command]
        else:
            where_command=['where','{0}:{1}'.format(head_path,tail_path)]
        #failure indicated by "INFO: Could not find files for the given pattern(s)." and non-zero exit code
    else:
        where_command=['which','-a',abspath_command]

    debug_str='external call command: {0}'.format(repr(where_command))
    log_at_level(source='common_lib.find_command_location', message=debug_str,
                 level=log_levels['DEBUG'], my_log_fp=my_log_fp)

    try:
        output_tuple=external_call(command=where_command, capture_output=True, print_capture=False, posix=None, shell=False, raise_except=True, my_log_fp=my_log_fp)
    except Exception, e:
        error_str='Path to command could not be verified; external_call threw an exception - "{0}"'.format(e)
        log_at_level(source='common_lib.find_command_location', message=error_str,
                     level=log_levels['WARNING'], my_log_fp=my_log_fp)
        if exit_on_fail:
            sys.exit(1)
    else:
        output=output_tuple[0]
        debug_str='output from external call >>>' + "\n" + \
                   output + "<<<"
        log_at_level(source='common_lib.find_command_location', message=debug_str,
                     level=log_levels['DEBUG'], my_log_fp=my_log_fp)

        output_lines=output.splitlines()

        abspath_command=os.path.abspath(output_lines[0])

    return abspath_command

def open_log_file(name, log_to_global=True):
    global log_fp
    my_log_fp = open_file(name, "wb")

    if log_to_global:
        log_fp=my_log_fp

    return my_log_fp

def close_log_file(my_log_fp=None):
    global log_fp

    try:
        if my_log_fp is None:
            if log_fp is not None:
                log_fp.close()
            log_fp=None
        else:
            my_log_fp.close()
    except Exception, e:
        error_str='Failed to close log file. {0}'.format(e)
        log_at_level(source='common_lib.close_log_file', message=error_str,
                     level=log_levels['ERROR'], my_log_fp=my_log_fp)
        raise e
    return

def copy_dict_excluding_keys(dict_to_copy, keys_to_exclude=[]):
    final_dict=dict_to_copy.copy()

    for cur_excluded_key in keys_to_exclude:
        del final_dict[cur_excluded_key]

    return final_dict

def sha256_of_file(filename):
    import hashlib
    abspath_filename=os.path.abspath(filename)
    file_obj=open_file(abspath_filename, 'rb')
    try:
        file_data=file_obj.read()
        file_obj.close()
    except Exception, e:
        error_str='Read or close file operation failed. {0}'.format(e)
        log_at_level(source='common_lib.sha256_of_file', message=error_str,
                     level=log_levels['ERROR'])
        raise e
    sha256_hash_obj=hashlib.sha256()
    sha256_hash_obj.update(file_data)
    hash_str=sha256_hash_obj.digest()

    return hash_str

def bin_to_hex_string(bin_str):
    return ''.join(["%02x"%ord(cur_char) for cur_char in bin_str])

def hex2int(hex_string):
    try:
        ret_val=int(hex_string, 16)
    except Exception, e:
        error_str='Error converting base 16 to base 10. {0}'.format(e)
        log_at_level(source='common_lib.hex2int', message=error_str,
                     level=log_levels['ERROR'])

    return ret_val

def hex_string_to_bin(hex_string):
    hex_len=len(hex_string)
    ret_string=None

    if len(hex_string)%2>0:
        error_str='Length of hex string must be a even number length (divisible by 2)'
        log_at_level(source='common_lib.hex_string_to_bin', message=error_str,
                     level=log_levels['ERROR'])

    for cur_char_index in range(0,hex_len,2):
        if cur_char_index+2<=hex_len:
            cur_str=hex_string[cur_char_index:cur_char_index+2]
            if ret_string is None:
                ret_string=''
            ret_string+=chr(hex2int(cur_str))
    return ret_string

def binary_to_hex_list(bin_str):
    return ["%02x"%ord(cur_char) for cur_char in bin_str]

def file_to_hex_list(filename):
    file_obj=open_file(file_name, mode='rb')
    try:
        file_data=file_obj.read()
        file_obj.close()
    except Exception, e:
        error_str='Read or close file operation failed. {0}'.format(e)
        log_at_level(source='common_lib.file_to_hex_list', message=error_str,
                     level=log_levels['ERROR'])
        raise e

    return binary_to_hex_list(file_data)

def hex_list_to_c_style_hex_variable(hex_list, variable_name, bytes_per_line=8):
    ret_str=None
    byte_count=0

    for cur_byte in hex_list:
        if byte_count+1>bytes_per_line:
            ret_str+="\n"
            byte_count=0
        if ret_str is None:
            ret_str=''
        ret_str+='0x{0},'.format(cur_byte)
        byte_count+=1

    if ret_str is not None:
        ret_str=ret_str[:-1]
        ret_str='uint8 {0}[] = '.format(variable_name)+"{\n"+ret_str+"\n};"
    else:
        ret_str='uint8 {0}[1] = '.format(variable_name)+'{NULL};'

    return ret_str

def pad_string(in_string, size, pad_char="\0", throw_except=True):
    if len(in_string)>size and throw_except:
        short_error_str='in_string supplied is larger than size'
        error_str=log_at_level(source='common_lib.pad_string', message=short_error_str,
                     level=log_levels['ERROR'])
        raise ValueError, error_str
    if not len(pad_char)==1:
        short_error_str='pad_char parameter must be a single character'
        error_str=log_at_level(source='common_lib.pad_string', message=short_error_str,
                     level=log_levels['ERROR'])
        raise ValueError, error_str
    my_string=in_string[0:size]
    padded_string=my_string+pad_char*(size-len(my_string))

    return padded_string

#----------------------------------------------------------------------------
# read_ini_file
# Reads the configuration file and stores the values in a dictionary variable
#----------------------------------------------------------------------------
def read_ini_file(ini_filename, section_name='default'):
    import ConfigParser

    config=ConfigParser.RawConfigParser()
    debug_str="Reading ini file '{0}'".format(ini_filename)

    log_at_level(source='common_lib.read_ini_file', message=debug_str,
                 level=log_levels['DEBUG'])

    try:
        config.read(ini_filename)
    except Exception, e:
        error_str="Reading ini file '{0}'".format(ini_filename)
        log_at_level(source='common_lib.read_ini_file', message=error_str,
                     level=log_levels['ERROR'])

    #initialize variables in dict
    return_dict={}

    debug_str="Reading section {0}".format(section_name)

    log_at_level(source='common_lib.read_ini_file', message=debug_str,
                 level=log_levels['DEBUG'])

    for cur_option in config.options(section_name):
        cur_value = config.get(section_name, cur_option)
        return_dict[cur_option]=cur_value

    return return_dict

#returns a list of keys to a dictionary sorted by their value, similar to an enum
def enum_keys(dict_to_sort):
    ret_list=None

    def enum_keys_sort_keys(x, y):
        return dict_to_sort[x] - dict_to_sort[y]

    ret_list=sorted(dict_to_sort.keys(), cmp=enum_keys_sort_keys)

    return ret_list

def unique_keys_as_string(source_dict):
    key_list=source_dict.keys()
    str_key_list=[str(cur_key) for cur_key in key_list]

    return dict.fromkeys(str_key_list).keys()

def unique_keys_as_int(source_dict):
    key_list=source_dict.keys()
    int_key_list=[int(cur_key) for cur_key in key_list]

    return dict.fromkeys(int_key_list).keys()

def copy_values_from_keys_to_keys(from_keylist, to_keylist, source_dict):
    new_dict=copy_dict_excluding_keys(dict_to_copy=source_dict)

    for cur_key_index in range(0, len(from_keylist)):
        cur_from_key=from_keylist[cur_key_index]
        cur_target_key=to_keylist[cur_key_index]
        value_to_copy=source_dict[cur_from_key]

        new_dict[cur_target_key]=value_to_copy

    return new_dict

def call_cygpath_path(path,win_unix_switch='-w'):
    result_path=''
    cmd_list=['cygpath',win_unix_switch,path]

    cmd_string=' '.join(cmd_list[:2]) + ' "{0}"'.format(path)

    debug_str="Doing system call '{0}'".format(cmd_string)
    log_at_level(source='common_lib.cygwin_to_win_path', message=debug_str,
                 level=log_levels['DEBUG'])

    external_call
    output_tuple=external_call(command=cmd_list, capture_output=True, print_capture=False, posix=None, shell=False, raise_except=True, my_log_fp=my_log_fp)
    stdout_str=output_tuple[0]

    result_path=stdout_str.strip()

    return result_path

def cygwin_to_win_path(path):

    return call_cygpath_path(path=path,win_unix_switch='-w')

def win_to_cygwin_path(path):

    return call_cygpath_path(path=path,win_unix_switch='-u')

def verify_version_is(version_list_1,version_list_2):
    if isinstance(version_list_1, basestring):
        version_list_1_list=version_list_1.split('.')
    elif isinstance(version_list_1,list) or isinstance(version_list_1,tuple):
        version_list_1_list=list(version_list_1)
    elif version_list_1 is None:
        version_list_1_list=[0]
    else:
        short_error_str='version_list_1 sent to verify_version_is must be of type basestring or a derivitive, a list, or a ' + \
                        'tuple'
        error_str=log_at_level(source='common_lib.verify_version_is', message=short_error_str,
                               level=log_levels['ERROR'])
        raise ValueError, error_str

    if isinstance(version_list_2,basestring):
        version_list_2_list=version_list_2.split('.')
    elif isinstance(version_list_2,list) or isinstance(version_list_2,tuple):
        version_list_2_list=list(version_list_2)
    else:
        short_error_str='version_list_2 sent to verify_version_is must be of type basestring or a derivitive, a list, or a ' + \
                        'tuple'
        error_str=log_at_level(source='common_lib.verify_version_is', message=short_error_str,
                               level=log_levels['ERROR'])
        raise ValueError, error_str

    version_list_1_copy = list(version_list_1_list[:])
    version_list_2_copy = list(version_list_2_list[:])

    for cur_pos_index in range(0,len(version_list_1_copy)):
        version_list_1_copy[cur_pos_index]=int(version_list_1_copy[cur_pos_index])

    for cur_pos_index in range(0,len(version_list_2_copy)):
        version_list_2_copy[cur_pos_index]=int(version_list_2_copy[cur_pos_index])

    if len(version_list_1_copy)<len(version_list_2_copy):
        version_list_1_copy.extend([0] * (len(version_list_2_copy)-len(version_list_1_copy)))
    elif len(version_list_2_copy)<len(version_list_1_copy):
        version_list_2_copy.extend([0] * (len(version_list_1_copy)-len(version_list_2_copy)))

    return version_list_1_copy==version_list_2_copy

def verify_min_version(version,min_version):
    if isinstance(version, basestring):
        version_list=version.split('.')
    elif isinstance(version,list) or isinstance(version,tuple):
        version_list=list(version)
    elif version is None:
        version_list=[0]
    else:
        short_error_str='version_list_1 sent to verify_min_version must be of type basestring or a derivitive, a list, or a ' + \
                        'tuple'
        error_str=log_at_level(source='common_lib.verify_min_version', message=short_error_str,
                               level=log_levels['ERROR'])
        raise ValueError, error_str

    if isinstance(version_list_2,basestring):
        version_list_2_list=version_list_2.split('.')
    elif isinstance(version_list_2,list) or isinstance(version_list_2,tuple):
        version_list_2_list=list(version_list_2)
    else:
        short_error_str='version_list_2 sent to verify_min_version must be of type basestring or a derivitive, a list, or a ' + \
                        'tuple'
        error_str=log_at_level(source='common_lib.verify_min_version', message=short_error_str,
                               level=log_levels['ERROR'])
        raise ValueError, error_str

    version_copy = list(version_list[:])
    min_version_copy = list(min_version_list[:])

    for cur_pos_index in range(0,len(version_copy)):
        version_copy[cur_pos_index]=int(version_copy[cur_pos_index])

    for cur_pos_index in range(0,len(min_version_copy)):
        min_version_copy[cur_pos_index]=int(min_version_copy[cur_pos_index])

    if len(version_copy)<len(min_version_copy):
        version_copy.extend([0] * (len(min_version_copy)-len(version_copy)))
    elif len(min_version_copy)<len(version_copy):
        min_version_copy.extend([0] * (len(version_copy)-len(min_version_copy)))

    return version_copy>=min_version_copy

def verify_max_version(version,max_version):
    if isinstance(version, basestring):
        version_list=version.split('.')
    elif isinstance(version,list) or isinstance(version,tuple):
        version_list=list(version)
    elif version is None:
        version_list=[0]
    else:
        short_error_str='version_list_1 sent to verify_max_version must be of type basestring or a derivitive, a list, or a ' + \
                        'tuple'
        error_str=log_at_level(source='common_lib.verify_max_version', message=short_error_str,
                               level=log_levels['ERROR'])
        raise ValueError, error_str

    if isinstance(version_list_2,basestring):
        version_list_2_list=version_list_2.split('.')
    elif isinstance(version_list_2,list) or isinstance(version_list_2,tuple):
        version_list_2_list=list(version_list_2)
    else:
        short_error_str='version_list_2 sent to verify_max_version must be of type basestring or a derivitive, a list, or a ' + \
                        'tuple'
        error_str=log_at_level(source='common_lib.verify_max_version', message=short_error_str,
                               level=log_levels['ERROR'])
        raise ValueError, error_str

    version_copy = list(version_list[:])
    max_version_copy = list(max_version_list[:])

    for cur_pos_index in range(0,len(version_copy)):
        version_copy[cur_pos_index]=int(version_copy[cur_pos_index])

    for cur_pos_index in range(0,len(max_version_copy)):
        max_version_copy[cur_pos_index]=int(max_version_copy[cur_pos_index])

    if len(version_copy)<len(max_version_copy):
        version_copy.extend([0] * (len(max_version_copy)-len(version_copy)))
    elif len(max_version_copy)<len(version_copy):
        max_version_copy.extend([0] * (len(version_copy)-len(max_version_copy)))

    return version_copy<=max_version_copy

def text_block_to_dict_of_lists(string_to_regex, find_sections_regex_str=None,header_new_line=False):
    return_dict={}

    if find_sections_regex_str==None:
        #find_sections_regex_str=r'^([^\s]+[^\r\n]*)\r?\n^((\s+).*\r?\n(?:^\s+.*\r?\n)*)'
        if header_new_line:
            find_sections_regex_str=r'^(?P<header>[^\s]+[^\r\n]*(?:\r?\n[^\s]+[^\r\n])*)(?P<child_data>\r?\n^((?P<space_num>\s+).*\r?\n(?:^\s+.*\r?\n)*))?'
        else:
            find_sections_regex_str=r'^(?P<header>[^\s]+[^\r\n]*)\r?\n^(?P<child_data>(?P<space_num>\s+).*\r?\n(?:^\s+.*\r?\n)*)?'
    find_sections_regex=re.compile(find_sections_regex_str,re.MULTILINE)

    counter=0
    for match in find_sections_regex.finditer(string_to_regex):
        space_size=0
        recurse_regex_str=''
        spaces_found=match.group('space_num')
        if not spaces_found==None:
            space_size=len(spaces_found)
            if header_new_line:
                recurse_regex_str=r'^{0}([^\s]+[^\r\n]*(?:\r?\n{0}[^\s]+[^\r\n])*)(\r?\n^({0}(\s+).*\r?\n(?:^{0}\s+.*\r?\n)*))?'.format('\s'*space_size)
            else:
                recurse_regex_str=r'^{0}(?P<header>[^\s]+[^\r\n]*)(\r?\n^(?P<child_data>{0}(?P<space_num>\s+).*\r?\n(?:^{0}\s+.*\r?\n)*))?'.format('\s'*space_size)
        section_header=''
        section_header_found=match.group('header')
        if not section_header_found==None:
            section_header=section_header_found
        child_block=''
        child_block_found=match.group('child_data')
        if not child_block_found==None:
            child_block=child_block_found
        #recurse_regex_str=r'^{0}([^\s]+[^\r\n]*)(\r?\n^(({0}\s+).*\r?\n(?:^{0}\s+.*\r?\n)*))?'.format('\s'*space_size,)

        debug_str= "Match: >>>{0}<<<".format(match.group())
        debug_str+="\n\tsection header is '{0}'".format(section_header)
        debug_str+="\n\tChild space size is {0}".format(space_size)
        debug_str+="\n\tdata is >>>{0}<<<".format(child_block)
        debug_str+="\n\trecurse regex is '{0}'".format(recurse_regex_str)

        log_at_level(source='common_lib.text_block_to_dict_of_lists', message=debug_str,
                     level=log_levels['DEBUG2'])

        if not return_dict.has_key(section_header):
            return_dict[section_header]=[]
        if len(recurse_regex_str)>0:
            child=text_block_to_dict_of_lists(string_to_regex=child_block, find_sections_regex_str=recurse_regex_str,
                                              header_new_line=header_new_line)
            return_dict[section_header].append(child)
        counter+=1
    if not counter>0 and len(string_to_regex)>0:
        no_white_space_str=string_to_regex
        no_white_space_str=no_white_space_str.replace(" ","")
        no_white_space_str=no_white_space_str.replace("\t","")
        no_white_space_str=no_white_space_str.replace("\n","")
        no_white_space_str=no_white_space_str.replace("\r","")

        debug_str='Remaining text "{0}"'.format(no_white_space_str)
        log_at_level(source='common_lib.text_block_to_dict_of_lists', message=debug_str,
                 level=log_levels['DEBUG'])
        if len(no_white_space_str)>0:
            short_error_str='Function has missed some valild text; the regex "{0}" did not match "{1}"'.format(find_sections_regex_str,string_to_regex)
            error_str=log_at_level(source='common_lib.text_block_to_dict_of_lists', message=short_error_str,
                                   level=log_levels['ERROR'])
            raise RuntimeError, error_str

    return return_dict

def get_openssl_version(openssl_command_path):
    openssl_version=None
    openssl_version_cmd_list=[openssl_command_path,'version']
    cur_module=dir(sys.modules[__name__])
    #Example to match: OpenSSL 1.0.1e 11 Feb 2013
    openssl_version_regex_str=r'^\s*OpenSSL\s+(?P<openssl_version>[^\s]+)(\s(?P<build_date>\d+\s+[^\s]+\s+\d+))?$'
    openssl_version_regex=re.compile(openssl_version_regex_str)

    if cur_module.has_key('openssl_version'):
        openssl_version=cur_module['openssl_version']

    if openssl_version==None:
        output_tuple=external_call(command=cmd_list, capture_output=True, print_capture=False, posix=None, shell=False, raise_except=True, my_log_fp=my_log_fp)
        stdout_str=output_tuple[0]

        debug_str='openssl returned success'
        log_at_level(source='common_lib.get_openssl_version', message=debug_str,
                     level=log_levels['DEBUG'])

        openssl_dict=text_block_to_dict_of_lists(stdout_str)

        debug_str='text block should have been read'
        log_at_level(source='common_lib.get_openssl_version', message=debug_str,
                     level=log_levels['DEBUG'])

        openssl_list=openssl_dict.keys()
        openssl_version_found_list=[]

        if len(openssl_list)>1:
            debug_str='There should only be one line for this response. Actual keys: {0}'.format(len(openssl_list))
            log_at_level(source='common_lib.get_openssl_version', message=debug_str,
                         level=log_levels['DEBUG'])

        for cur_str in openssl_list:
            if len(openssl_dict[cur_str])>0:
                error_str='DEBUG ERROR: common_lib.get_openssl_version - Items exist below version description in dict'
                log_at_level(source='common_lib.get_openssl_version', message=error_str,
                             level=log_levels['VERBOSE'], manual_string=True)
            version_matches=openssl_version_regex.search(cur_str)

            if not version_matches==None:
                cur_openssl_version=version_matches.group('openssl_version')
                cur_openssl_build_date=version_matches.group('build_date')

                cur_openssl_version=cur_openssl_version.strip()
                cur_openssl_build_date=cur_openssl_build_date.strip()
                openssl_version_found_list.append((cur_openssl_version,cur_openssl_build_date))
            else:
                short_error_str='Each version string should match regex, but does not: ' + \
                                '"{0}"'.format(cur_library_path_str)
                error_str=log_at_level(source='common_lib.get_openssl_version', message=short_error_str,
                                       level=log_levels['ERROR'])
                raise RuntimeError,error_str
        if len(openssl_version_found_list)>1:

            for cur_openssl_version in openssl_version_found_list:
                if openssl_version==None:
                    openssl_version=cur_openssl_version[0]
                else:
                    if not openssl_version==cur_openssl_version[0]:
                        short_error_str='Multiple openssl version strings found that do not match'
                        error_str=log_at_level(source='common_lib.get_openssl_version', message=short_error_str,
                                               level=log_levels['ERROR'])
                        raise RuntimeError,error_str
        elif len(openssl_version_found_list)>0:
            openssl_version=openssl_version_found_list[0][0]
        else:
            short_error_str='No version strings found that match regex'
            error_str=log_at_level(source='common_lib.get_openssl_version', message=short_error_str,
                                   level=log_levels['ERROR'])
            raise RuntimeError,error_str

        cur_module['openssl_version']=openssl_version

    debug_str='OpenSSL version found is "{0}"'.format(cur_module['openssl_version'])
    log_at_level(source='common_lib.get_openssl_version', message=debug_str,
                 level=log_levels['DEBUG'])

    return cur_module['openssl_version']

def is_readable_file(file_path, empty_ok=True):
    if os.path.exists(file_path):
        if os.path.isfile(file_path):
            if os.access(file_path, os.R_OK):
                if os.path.getsize(file_path)>0:
                    pass
                elif empty_ok:
                    pass
                else:
                    short_error_str='File specified is empty. Path "{0}"'.format(file_path)
                    error_str=log_at_level(source='common_lib.is_readable_file', message=short_error_str,
                                           level=log_levels['ERROR'])
                    raise RuntimeError, error_str
            else:
                short_error_str='File specified is not readable. Path "{0}"'.format(file_path)
                error_str=log_at_level(source='common_lib.is_readable_file', message=short_error_str,
                                       level=log_levels['ERROR'])
                raise RuntimeError, error_str
        else:
            short_error_str='File specified is not a file. Path "{0}"'.format(file_path)
            error_str=log_at_level(source='common_lib.is_readable_file', message=short_error_str,
                                   level=log_levels['ERROR'])
            raise RuntimeError, error_str
    else:
        short_error_str='File specified does not exist. Path "{0}"'.format(file_path)
        error_str=log_at_level(source='common_lib.is_readable_file', message=short_error_str,
                               level=log_levels['ERROR'])
        raise RuntimeError, error_str
    return True

def value_pairs_list_to_parse_args_list(csv_params_list):
    params_string=''
    params_list=[]

    for cur_item_index in range(0,len(csv_params_list),2):
        cur_key=csv_params_list[cur_item_index]
        cur_value=None
        dashes=None
        tmp_str=None

        if cur_key.find(' ')>0:
            short_error_str="Key '{0}' (item {1}) contains a space; keys may not contain spaces".format(cur_key, cur_item_index)
            error_str=log_at_level(source='common_lib.value_pairs_list_to_parse_args_list', message=short_error_str,
                                   level=log_levels['ERROR'])
            raise RuntimeError, error_str

        tmp_str=cur_key

        if len(cur_key)>=2:
            first_two_chars=cur_key[0:2]
            #print "first two {0}".format(first_two_chars)
            if first_two_chars=='--':
                dashes='--'
            elif first_two_chars[0]=='-':
                dashes='-'

        if cur_item_index+1<len(csv_params_list):
            cur_value=csv_params_list[cur_item_index+1]
            tmp_str+='={0}'.format(cur_value)

        if len(cur_key)==1 and cur_value is None and dashes is None:
            tmp_str="-{0}".format(tmp_str)
        elif dashes is not None:
            pass
        else:
            tmp_str="--{0}".format(tmp_str)
        #print "tmp_str is '{0}'".format(tmp_str)
        params_list.append(tmp_str)

    return params_list


def value_pairs_list_to_parse_args_str(params_list):
    return subprocess.list2cmdline(value_pairs_list_to_parse_args_list(params_list))

def value_pairs_list_from_parse_args_list(params_list):
    import re
    params_string=''
    csv_params_list=[]
    parameters_regex_str=r'(?P<dashes>(-)?-)(?P<key>[A-Za-z0-9_]+)(=(?P<value>.+))?'
    parameters_regex=re.compile(parameters_regex_str)

    for cur_item_index in range(0,len(params_list)):
        cur_param=params_list[cur_item_index]
        cur_key=None
        cur_value=None
        dashes=None
        tmp_list=None
        parameter_matches=parameters_regex.search(cur_param)

        if parameter_matches is None:
            short_error_str="Parameter '{0}' does not match the parameter regex '{1}'".format(cur_param, parameters_regex_str)
            error_str=log_at_level(source='common_lib.value_pairs_list_from_parse_args_list', message=short_error_str,
                                   level=log_levels['ERROR'])
            raise RuntimeError, error_str
        cur_dashes=parameter_matches.group('dashes')
        cur_key=parameter_matches.group('key')
        cur_value=parameter_matches.group('value')

        if cur_key.find(' ')>0:
            short_error_str="Key '{0}' (item {1}) contains a space; keys may not contain spaces".format(cur_key, cur_item_index)
            error_str=log_at_level(source='common_lib.value_pairs_list_from_parse_args_list', message=short_error_str,
                                   level=log_levels['ERROR'])
            raise RuntimeError, error_str

        tmp_list=[cur_key,cur_value]

        #print "tmp_str is '{0}'".format(tmp_str)
        csv_params_list.extend(tmp_str)

    return csv_params_list

def csv_to_list(csv_string):
    import StringIO
    import csv
    myargs_csv_file_obj=StringIO.StringIO()
    myargs_csv_file_obj.write(csv_string)
    myargs_csv_file_obj.seek(0)
    csv_list_of_lines=[x for x in csv.reader(myargs_csv_file_obj)]
    #print csv_list_of_lines
    if len(csv_list_of_lines)==0:
        csv_list_line_1=[]
    elif len(csv_list_of_lines)==1:
        csv_list_line_1=csv_list_of_lines[0]
    else:
        short_error_str='Wrong number of lines to parse; only one line allowed'
        error_str=secdatgen_common_lib.log_at_level(source='common_lib.csv_to_list',
                                                    message=short_error_str,
                                                    level=secdatgen_common_lib.log_levels['ERROR'])
        raise RuntimeErrorm, error_str
    myargs_csv_file_obj.close()
    #print csv_list_line_1

    return csv_list_line_1

def list_to_csv(list_of_lines):
    import StringIO
    import csv
    myargs_csv_file_obj=StringIO.StringIO()
    csv.writer(myargs_csv_file_obj)

    #print list_of_lines
    if len(list_of_lines)==0:
        csv_list_line_1=[]
    elif len(list_of_lines)==1:
        csv_list_line_1=list_of_lines[0]
    else:
        short_error_str='Wrong number of lines to parse; only one line allowed'
        error_str=secdatgen_common_lib.log_at_level(source='common_lib.list_to_csv',
                                                    message=short_error_str,
                                                    level=secdatgen_common_lib.log_levels['ERROR'])
        raise RuntimeErrorm, error_str
    myargs_csv_file_obj.close()
    #print csv_list_line_1
    csv_adder.writerow(csv_list_line_1)
    myargs_csv_file_obj.seek(0)
    csv_string=myargs_csv_file_obj.read()
    return csv_string.rstrip()

def str2bool(v):
    bool_val=None
    if v.lower() in ("yes", "true", "t", "1"):
        bool_val=True
    elif v.lower() in ("n", "false", "f", "0"):
        bool_val=False
    else:
        short_error_str="string '{0}' is not a valid boolean string".format(v)
        error_str=secdatgen_common_lib.log_at_level(source='common_lib.str2bool',
                                                    message=short_error_str,
                                                    level=secdatgen_common_lib.log_levels['ERROR'])
        raise RuntimeErrorm, error_str
    return bool_val

def normalize_line_endings(text, line_ending="\n"):
    import StringIO
    import re
    ending_regex_str=r'(.*)((\r)?\n)$'
    ending_regex=re.compile(ending_regex_str)
    intext=StringIO.StringIO()
    intext.write(text)
    intext.seek(0)
    new_text=''
    lines=intext.readlines()
    for line in lines:
        matches=ending_regex.search(line)
        txt=matches.groups()[0]
        new_line="{0}{1}".format(txt,line_ending)
        new_text+=new_line
    return new_text

def get_OrderedDict_class():
    if sys.version_info >= (2, 7):
        from collections import OrderedDict
    elif sys.version_info >= (2, 6):
        from ordereddict import OrderedDict
    else:
        raise RuntimeError, 'Python version is below required version. Need atleast 2.6, but version is' + \
                                        '%s' % '.'.join(sys.version_info)
    return OrderedDict