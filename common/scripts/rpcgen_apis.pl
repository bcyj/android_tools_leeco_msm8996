#!/usr/bin/perl -w
#===========================================================================
# rpcgen_apis.pl
#
# Description:  A script that updates the RPCGEN apis based on the
#               xdr files and post-processes them
#
# Usage: rpcgen_apis.pl -h  Help
#        rpcgen_apis.pl -api=<api_name> -has_cb=0/1 -sd=<source_dir>
#                       -sf=<ssource_file> -td=<target_dir>
#                       -tf=<target_file>
#
#
#==========================================================================
#==========================================================================
# Copyright (c) 2010 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#==========================================================================
#==========================================================================
#            EDIT HISTORY FOR FILE
#    When      Who   What
#   ======    ===== ======
# 05/11/2011  zhkait Made adding copyright conditional
# 02/17/2010    kr   Autogen of rpcgen_apis_verify.c
# 02/05/2010    kr   Initial Version
#==========================================================================

use FileHandle;
use IPC::Open3;
use Getopt::Long;
use Carp;
use POSIX qw(ceil floor);
use strict;


#-----------------------------------------
# Forward declaration of local functions
#-----------------------------------------
sub ExecuteCmd($);
sub InitXdrCommonDefs();
sub CreateCommonDefsXdr($$);
sub PrintAndLog($);
sub ProcessApi($$$$$$$);
sub ProcessApiCallBack($$$$$$$);
sub GetYear();
sub display_usage();
sub BuildApisList($);
sub GenerateRpcgenApisVerifySrc($$);
#-----------------------------------------

my %config = (
              api         => undef,
              verbose     => 0,
              cb          => 0,
              has_cb      => 0,
              add_copyright      => 0
             );

my @opt_defs = ('h', '-api=s', '-td=s', '-sd=s', '-has_cb=s', '-cb', '-tf=s', '-sf=s', '-verbose!', '-add_copyright=s' );

my $opt_result = GetOptions(\%config, @opt_defs);


my $startTime = time;
my $version = "1.01";

if (defined  $config{h}) {
  print "rpcgen_apis Version $version \n";
  display_usage();
  exit;
}

if (!defined($config{td}) || !defined($config{tf})) {
  print "rpcgen_apis Version $version \n";
  print "Target Directory OR Target File missing\n";
  display_usage();
  exit;
}


#-----------------------------------------
# Global Databases and Variables
#-----------------------------------------
my $target = "";
my $apiType = "";
my $line_separator = "------------------------------------------------------------------------------------------------------------\n";
my $rpcgenCommonApiName = "commondefs";
my %rpcgen_apis_list = ();

my @StatusLog;
my @Copyright;
my @CopyrightKernel;
my $CopyrightKernelText = "Copyright (c) ".GetYear()." Qualcomm Technologies, Inc. \n";
push @Copyright, "-------------------------------------------------------------------------------\n";
push @Copyright, "Copyright (c)".GetYear()." Qualcomm Technologies, Inc. \n";
push @Copyright, "All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.\n";
push @Copyright, "-------------------------------------------------------------------------------\n";
push @CopyrightKernel, "-------------------------------------------------------------------------------\n";
push @CopyrightKernel, $CopyrightKernelText;
push @CopyrightKernel, "-------------------------------------------------------------------------------\n";
my @XdrCommonDefs;

#-------------------------------------------------------------------
# Set target specific application location
#-------------------------------------------------------------------
if($config{tf} eq $rpcgenCommonApiName.".xdr") {
  InitXdrCommonDefs();
  CreateCommonDefsXdr($config{td}, $config{tf});
}
elsif($config{tf} eq "rpcgen_apis_verify.c") {
  if(!defined($config{sd})) {
    print "Source Directory Missing\n";
    display_usage();
    exit;
  }
  BuildApisList($config{sd});
  GenerateRpcgenApisVerifySrc($config{td}, $config{tf});
}
else
{
  if (!defined($config{sd}) || !defined($config{sf}) || !defined($config{api})) {
    print "Source Directory OR Source File OR Api missing\n";
    display_usage();
    exit;
  }
  if($config{sd} =~ /(modem-apis)\/(\w+)\/(api)/)
  {
    $target = $2;
  }
  if($config{sd} =~ /kernel/)
  {
    $apiType = "K";
  }
  elsif($config{sd} =~ /libs/)
  {
    $apiType = "U";
  }
  if(!$config{cb}) {
    ProcessApi($config{api},$apiType, $config{td}, $config{tf}, $config{sd}, $config{sf}, $config{add_copyright});
  }
  else
  {
    ProcessApiCallBack($config{api},$apiType, $config{td}, $config{tf}, $config{sd}, $config{sf}, $config{add_copyright});
  }
}


#=============================================================================
#
# FUNCTION: ProcessApi
#
# DESCRIPTION:  Generate <api>_rpcgen_clnt.c, <api>_rpcgen_xdr.c &
#               <api>_rpcgen_rpc.h file from <api>.xdr & post-process them
#
# PARAMETERS: api
#             api_type "U" for User-space, "K" for kernel-space
#             targetDirectory = $config{td}
#             targetFile = $config{tf}
#             sourceDirectory = $config{sd}
#             sourceFile = $config{sf}
#============================================================================*/
sub ProcessApi($$$$$$$)
  {
    my ($api) = shift(@_);
    my ($api_type) = shift(@_);
    my $targetDirectory = shift(@_);
    my $targetFile = shift(@_);
    my $sourceDirectory = shift(@_);
    my $sourceFile = shift(@_);
    my $copyright = shift(@_);
    my $build_cmd;
    my @cmd_out;

    #---------------------------------------------------------------
    # Create the RPC gen related files, apply fixes to output
    # and create makefile
    #---------------------------------------------------------------
    if($targetFile eq $api."_rpcgen_rpc.h")
    {
      # Build Header
      $build_cmd = "rpcgen -h -M  ".".\/".$sourceDirectory."\/".$sourceFile." -o ".".\/".$targetDirectory."\/".$api."_rpc.h";
      PrintAndLog($build_cmd."\n");
      @cmd_out = ExecuteCmd($build_cmd);

      $build_cmd = "mv .\/".$targetDirectory."\/".$api."_rpc.h .\/".$targetDirectory."\/".$targetFile;
      @cmd_out = ExecuteCmd($build_cmd);

      XdrReplaceRegex($targetDirectory,$targetFile, "<rpc\/rpc\.h>","\"librpc.h\"");
      if($api ne $rpcgenCommonApiName)
      {
        if($config{has_cb})
        {
          if($api eq "snd")
          {
            XdrAddInclude($targetDirectory,$targetFile, "#include\\s+\"librpc\\.h\"","#include \"".$api."_rpcgen_surf_rpc.h\"");
          }
          XdrAddInclude($targetDirectory,$targetFile,"#include\\s+\"librpc\\.h\"","#include \"".$api."_rpcgen_common_rpc.h\"");
        }
        XdrAddInclude($targetDirectory,$targetFile,"#include\\s+\"librpc\\.h\"","#include \"".$rpcgenCommonApiName."_rpcgen_rpc.h\"");
      }
      if ($copyright)
      {
        RpcgenAddCopyright($targetDirectory,$targetFile,$api_type);
      }
    }
    elsif($targetFile eq $api."_rpcgen_clnt.c")
    {
      # Build Client side code
      $build_cmd = "rpcgen -l -M  ".".\/".$sourceDirectory."\/".$sourceFile." -o ".".\/".$targetDirectory."\/".$api."_clnt.c";
      PrintAndLog($build_cmd."\n");
      @cmd_out = ExecuteCmd($build_cmd);

      $build_cmd = "mv .\/".$targetDirectory."\/".$api."_clnt.c .\/".$targetDirectory."\/".$targetFile;
      @cmd_out = ExecuteCmd($build_cmd);

      XdrReplaceRegex($targetDirectory,$targetFile,"\.\/".$sourceDirectory."\/$api\.h",$api."_rpcgen_rpc.h");
      if ($copyright)
      {
        RpcgenAddCopyright($targetDirectory,$targetFile,$api_type);
      }
    }
    elsif($targetFile eq $api."_rpcgen_xdr.c")
    {
      # Build Xdr routines
      $build_cmd = "rpcgen -c -M  ".".\/".$sourceDirectory."\/".$sourceFile." -o ".".\/".$targetDirectory."\/".$api."_xdr.c";
      PrintAndLog($build_cmd."\n");
      @cmd_out = ExecuteCmd($build_cmd);

      $build_cmd = "mv .\/".$targetDirectory."\/".$api."_xdr.c .\/".$targetDirectory."\/".$targetFile;
      @cmd_out = ExecuteCmd($build_cmd);

      # Fix include path for .c files, and add common include path
      XdrReplaceRegex($targetDirectory,$targetFile,"\.\/".$sourceDirectory."\/$api\.h",$api."_rpcgen_rpc.h");
      # For some reason RPCGEN adds register int32_t *buf which is never used, remove it
      if($api ne $rpcgenCommonApiName)
      {
        XdrReplaceRegex($targetDirectory,$targetFile,"\\sregister\\sint32_t\\s\\*buf","");
      }
      if ($copyright)
      {
        RpcgenAddCopyright($targetDirectory,$targetFile,$api_type);
      }
    }

  }


#=============================================================================
#
# FUNCTION: ProcessApiCallBack
#
# DESCRIPTION:  Generate <api>_rpcgen_common_xdr.c, <api>_rpcgen_common_rpc.h
#               files from <api>_common.xdr & post-process them
#
#               Generate <api>_rpcgen_cb_svc.c, <api>_rpcgen_cb_xdr.c,
#               <api>_rpcgen_rpc.h, <api>cb_appinit.c, <api>cb_appinit.h
#               files from <api>_cb.xdr file & post-process them
#
# PARAMETERS: api
#             api_type "U" for User-space, "K" for kernel-space
#             targetDirectory = $config{td}
#             targetFile = $config{tf}
#             sourceDirectory = $config{sd}
#             sourceFile = $config{sf}
#============================================================================*/
sub ProcessApiCallBack($$$$$$$)
  {
    my ($api) = shift(@_);
    my ($api_type) = shift(@_);
    my $targetDirectory = shift(@_);
    my $targetFile = shift(@_);
    my $sourceDirectory = shift(@_);
    my $sourceFile = shift(@_);
    my $copyright = shift(@_);
    my $build_cmd;
    my @cmd_out;

    #---------------------------------------------------------------
    # Create the RPC gen related files, apply fixes to output
    # and create makefile
    #---------------------------------------------------------------
    if($targetFile eq $api."_rpcgen_common_xdr.c")
    {
      # Build Server side Common client xdr
      $build_cmd = "rpcgen -c -M ".".\/".$sourceDirectory."\/".$sourceFile." -o ".".\/".$targetDirectory."\/".$api."_common_xdr.c";
      PrintAndLog($build_cmd."\n");
      @cmd_out = ExecuteCmd($build_cmd);

      $build_cmd = "mv .\/".$targetDirectory."\/".$api."_common_xdr.c .\/".$targetDirectory."\/".$targetFile;
      @cmd_out = ExecuteCmd($build_cmd);

      XdrReplaceRegex($targetDirectory,$targetFile,"<rpc\/rpc\.h>","\"librpc.h\"");
      XdrReplaceRegex($targetDirectory,$targetFile,"\.\/".$sourceDirectory."\/".$api."_common\.h",$api."_rpcgen_common_rpc.h");
      if ($copyright)
      {
        RpcgenAddCopyright($targetDirectory,$targetFile,$api_type);
      }
    }
    elsif($targetFile eq $api."_rpcgen_common_rpc.h")
    {
      # Build Server side Common header
      $build_cmd = "rpcgen -h -M ".".\/".$sourceDirectory."\/".$sourceFile." -o ".".\/".$targetDirectory."\/".$api."_common_rpc.h";
      PrintAndLog($build_cmd."\n");
      @cmd_out = ExecuteCmd($build_cmd);

      $build_cmd = "mv .\/".$targetDirectory."\/".$api."_common_rpc.h .\/".$targetDirectory."\/".$targetFile;
      @cmd_out = ExecuteCmd($build_cmd);

      # Copy API Common file to LINUX
      XdrReplaceRegex($targetDirectory,$targetFile,"<rpc\/rpc\.h>","\"librpc.h\"");
      XdrAddInclude($targetDirectory,$targetFile,"#include\\s+\"librpc\\.h\"","#include \""."commondefs_rpcgen_rpc.h\"");
      if ($copyright)
      {
        RpcgenAddCopyright($targetDirectory,$targetFile,$api_type);
      }
    }
    elsif($targetFile eq $api."_rpcgen_cb_svc.c")
    {
      # Callback section
      $build_cmd = "rpcgen -m -M ".".\/".$sourceDirectory."\/".$sourceFile." -o ".".\/".$targetDirectory."\/".$api."_cb_svc.c";
      PrintAndLog($build_cmd."\n");
      @cmd_out = ExecuteCmd($build_cmd);

      $build_cmd = "mv .\/".$targetDirectory."\/".$api."_cb_svc.c .\/".$targetDirectory."\/".$targetFile;
      @cmd_out = ExecuteCmd($build_cmd);

      XdrReplaceRegex($targetDirectory,$targetFile,"\.\/".$sourceDirectory."\/".$api."_cb\.h",$api."_rpcgen_cb_rpc.h");
      if ($copyright)
      {
        RpcgenAddCopyright($targetDirectory,$targetFile,$api_type);
      }
    }
    elsif($targetFile eq $api."_rpcgen_cb_xdr.c")
    {
      # Create RPCGEN Callback xdr c file
      $build_cmd = "rpcgen -c -M ".".\/".$sourceDirectory."\/".$sourceFile." -o ".".\/".$targetDirectory."\/".$api."_cb_xdr.c";
      PrintAndLog($build_cmd."\n");
      @cmd_out = ExecuteCmd($build_cmd);

      $build_cmd = "mv .\/".$targetDirectory."\/".$api."_cb_xdr.c .\/".$targetDirectory."\/".$targetFile;
      @cmd_out = ExecuteCmd($build_cmd);

      XdrReplaceRegex($targetDirectory,$targetFile,"\.\/".$sourceDirectory."\/".$api."_cb\.h",$api."_rpcgen_cb_rpc.h");
      XdrReplaceRegex($targetDirectory,$targetFile,"\\sregister\\sint32_t\\s\\*buf","");
      if ($copyright)
      {
        RpcgenAddCopyright($targetDirectory,$targetFile,$api_type);
      }
    }
    elsif($targetFile eq $api."_rpcgen_cb_rpc.h")
    {
      # Create RPCGEN Callback header file
      $build_cmd = "rpcgen -h -M ".".\/".$sourceDirectory."\/".$sourceFile." -o ".".\/".$targetDirectory."\/".$api."_cb_rpc.h";
      PrintAndLog($build_cmd."\n");
      @cmd_out = ExecuteCmd($build_cmd);

      $build_cmd = "mv .\/".$targetDirectory."\/".$api."_cb_rpc.h .\/".$targetDirectory."\/".$targetFile;
      @cmd_out = ExecuteCmd($build_cmd);

      XdrReplaceRegex($targetDirectory,$targetFile,"<rpc\/rpc\.h>","\"librpc.h\"");
      XdrAddInclude($targetDirectory,$targetFile,"#include\\s+\"librpc\\.h\"","#include \"".$api."_rpcgen_common_rpc.h\"");
      XdrAddInclude($targetDirectory,$targetFile,"#include\\s+\"librpc\\.h\"","#include \"".$rpcgenCommonApiName."_rpcgen_rpc.h\"");
      if ($copyright)
      {
        RpcgenAddCopyright($targetDirectory,$targetFile,$api_type);
      }
    }
    elsif($targetFile eq $api."_cb_sample.c")
    {
      # Create RPCGEN Callback Sample Function file
      $build_cmd = "rpcgen -Ss -M ".".\/".$sourceDirectory."\/".$sourceFile." -o ".".\/".$targetDirectory."\/".$targetFile;
      PrintAndLog($build_cmd."\n");
      @cmd_out = ExecuteCmd($build_cmd);

      # Fix include path for .c files, and add common include path
      XdrReplaceRegex($targetDirectory,$targetFile,"\.\/".$sourceDirectory."\/".$api."_cb\.h",$api."_rpcgen_cb_rpc.h");
      XdrReplaceRegexSampleCode($targetDirectory,$targetFile,"printf(\"\%s Called \\n\",__FUNCTION__);");
      XdrAddInclude($targetDirectory,$targetFile,"cb_rpc\.h","#include "."\<stdio.h\>");
      if ($copyright)
      {
        RpcgenAddCopyright($targetDirectory,$targetFile,$api_type);
      }
    }
    elsif($targetFile eq $api."cb_appinit.c")
    {
      CreateRpcGenCallbackAppInitFiles($api,$api_type,$targetDirectory,$targetFile,$sourceDirectory,$sourceFile);
      if ($copyright)
      {
        RpcgenAddCopyright($targetDirectory,$targetFile,$api_type)
      }
    }
    elsif($targetFile eq $api."cb_appinit.h")
    {
      CreateRpcGenCallbackAppInitFiles($api,$api_type,$targetDirectory,$targetFile,$sourceDirectory,$sourceFile);
      if ($copyright)
      {
        RpcgenAddCopyright($targetDirectory,$targetFile,$api_type);
      }
    }
  }


#=============================================================================
#
# FUNCTION: CreateRpcGenCallbackAppInitFiles
#
# DESCRIPTION:  Subfunction to generate <api>cb_appinit.* files
#
# PARAMETERS: api
#             api_type "U" for User-space, "K" for kernel-space
#             targetDirectory = $config{td}
#             targetFile = $config{tf}
#             sourceDirectory = $config{sd}
#             sourceFile = $config{sf}
#============================================================================*/
sub CreateRpcGenCallbackAppInitFiles($$$$$$)
{
    my ($api) = shift(@_);
    my ($api_type) = shift(@_);
    my ($targetDirectory) = shift(@_);
    my ($targetFile) = shift(@_);
    my ($sourceDirectory) = shift(@_);
    my ($sourceFile) = shift(@_);
    my @versions;
    my $fileFp;
    my $xdr_fileFp;
    my $filename =  ".\/".$targetDirectory."\/".$targetFile;
    my $cb_xdr_filename = ".\/".$sourceDirectory."\/".$sourceFile;

    open ($xdr_fileFp, "<", $cb_xdr_filename);
    print "Opening >$cb_xdr_filename \n";

    my $api_lc = $api;
    chomp($api_lc);
    my $api_uc = uc($api);
    my $api_LC = lc($api_uc);

    # Create a list of all the API Versions: version PING_MDM_RPCCBVERS_0001
    while(<$xdr_fileFp>)
    {
      if(/version\s(\U$api\ECBVERS_\d\d\d\d)/)
      {
         push @versions, $1;
      }
    }
    close($xdr_fileFp);

    if($targetFile eq $api."cb_appinit.c")
    {
      open ($fileFp, ">", $filename);

    my $data = <<"EOF";
#include "librpc.h"
#include "$api_lc\_rpcgen_rpc.h"
#include "$api_lc\_rpcgen_cb_rpc.h"


#define RPC_FUNC_VERSION_BASE(a,b) a ## b
#define RPC_CB_FUNC_VERS(a,b) RPC_FUNC_VERSION_BASE(a,b)


static SVCXPRT* svrPort = NULL;

EOF

foreach my $cb_func_ver (@versions)
{
   $data .="extern void RPC_CB_FUNC_VERS(".$api_lc."cbprog_,\U$cb_func_ver\E)(struct svc_req *rqstp, register SVCXPRT *transp);\n\n";
}
$data .="int ".$api_lc."cb_app_init(void)\n";
$data .=
"{

  /* Register a callback server to use the ".$api_lc."cbprog_0x00010001  */";
$data .= "
  if (svrPort == NULL) {
        svrPort = svcrtr_create();
  }
  if (!svrPort) return -1;

  xprt_register(svrPort);

";
foreach my $cb_func_ver (@versions)
{
$data .= "

  if(svc_register(svrPort, ".$api_uc."CBPROG,\U$cb_func_ver\E, RPC_CB_FUNC_VERS(".$api_lc."cbprog_,\U$cb_func_ver\E),0))
  {
     return 0;
  }
  else
  {
    return -1;
  }
";
}
$data .= "}\n";
print $fileFp $data;

$data = "void ".$api_lc."cb_app_deinit(void)
{

   if (svrPort == NULL)
   {
      return;
   }
";

foreach my $cb_func_ver (@versions)
{
$data .= "

  svc_unregister(svrPort, ".$api_uc."CBPROG,\U$cb_func_ver\E);
";
}
$data .= "}\n";
print $fileFp $data;

      close($fileFp);
    }
    elsif($targetFile eq $api."cb_appinit.h")
    {
      open ($fileFp, ">", $filename);
 print $fileFp "
/* Initialization function for callbacks */
int ".$api_lc."cb_app_init(); ";

 print $fileFp "\n\n";
 print $fileFp "void ".$api_lc."cb_app_deinit(); \n\n";

      close($fileFp);
    }
}




#=============================================================================
#
# FUNCTION: ExecuteCmd
#
# DESCRIPTION:  Execute Command
#
# PARAMETERS:  Command
#
#============================================================================*/
sub ExecuteCmd($)
  {
    my ($cmd) = shift(@_);
    my @returnCmdOut;
    my ($rdr, $wtr, $err)= (FileHandle->new, FileHandle->new, FileHandle->new);
    my $pid = open3($wtr, $rdr, $err, $cmd );
    my $line;
    while (<$rdr>) {
      push (@returnCmdOut,$_);
    }
    while (<$err>) {
      push (@returnCmdOut,$_);
    }
    close ($rdr);
    close ($wtr);
    close ($err);
    waitpid $pid,0;

    return @returnCmdOut;
  }


#=============================================================================
#
# FUNCTION: GetYear
#
# DESCRIPTION:  Get time and generate the year based on that. Used in
#               Copyright Messages
#
#============================================================================*/
sub GetYear()
  {
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
    $year = $year + 1900;
    return $year;
  }


#=============================================================================
#
# FUNCTION: InitXdrCommonDefs
#
# DESCRIPTION:  Initialize the contents of commondefs.xdr file in an array.
#
#============================================================================*/
sub InitXdrCommonDefs()
  {
    push @XdrCommonDefs, "unsigned char rpc_boolean";
    push @XdrCommonDefs,"unsigned long rpc_uint32";
    push @XdrCommonDefs,"unsigned short rpc_uint16";
    push @XdrCommonDefs,"unsigned char rpc_uint8";
    push @XdrCommonDefs,"long rpc_int32";
    push @XdrCommonDefs,"short rpc_int16";
    push @XdrCommonDefs,"char rpc_int8";
    push @XdrCommonDefs,"unsigned char rpc_byte";
    push @XdrCommonDefs,"unsigned short rpc_word";
    push @XdrCommonDefs,"unsigned long rpc_dword";
    push @XdrCommonDefs,"short rpc_int2";
    push @XdrCommonDefs,"long rpc_int4";
    push @XdrCommonDefs,"hyper rpc_int64";
    push @XdrCommonDefs,"unsigned hyper rpc_uint64";
    push @XdrCommonDefs,"unsigned long rpc_qword[2]";
  }


#=============================================================================
#
# FUNCTION: CreateCommonDefsXdr
#
# DESCRIPTION:  Put the contents of @XdrCommonDefs into commondefs.xdr file.
#
# PARAMETERS:  targetDirectory = $config{td}
#              targetFile = $config{tf}
#
#============================================================================*/
sub CreateCommonDefsXdr($$)
  {
    my $targetDirectory = shift(@_);
    my $targetFile = shift(@_);
    my $fp;
    my $def_entry;
    my $api=$rpcgenCommonApiName;
    my $build_cmd;
    my @cmd_out;
    my $api_type="U";
    my $fn = ".\/".$targetDirectory."\/".$targetFile;

    open($fp, ">", $fn) or die $!;
    foreach $def_entry (@XdrCommonDefs) {
      print $fp "typedef ".$def_entry.";\n\n";
    }
    close($fp);

  }


#=============================================================================
#
# FUNCTION: PrintAndLog
#
# DESCRIPTION:  Print and log info, saves data in StatusLog for later printing
#
# PARAMETERS:  Data to log
#
#============================================================================*/
sub PrintAndLog($)
  {
    my ($data) = shift(@_);
    print $data;
    push @StatusLog,$data;
  }


#=============================================================================
#
# FUNCTION: XdrReplaceRegex
#
# DESCRIPTION:  Used for substituting source pattern with a target pattern.
#               Used in include file adjustment
#
# PARAMETERS:  - fromDir = Directory where the file is available
#              - fromFile = File where the pattern has to be modified
#              - regex = source pattern
#              - modex = target pattern
#============================================================================*/
sub XdrReplaceRegex($$$$)
  {
    my $fromDir = shift(@_);
    my $fromFile = shift(@_);
    my $regex    = shift(@_);
    my $modex = shift(@_);
    my $fromFileFP;
    my $toFileFP;
    my $line;

    my $fromFileName = $fromDir."\/".$fromFile;
    my $toFile = $fromDir."\/temp_".$fromFile;

    if (! -f $fromFileName) {
      PrintAndLog("File does not exist $fromFileName in XdrReplaceRegex \n");
      return;
    }

    open($fromFileFP, "<", $fromFileName) or die  $!;
    open( $toFileFP, ">", $toFile) or die $!;

    while ($line = <$fromFileFP>) {

      if ($line =~ /$regex/ ) {
        $line =~ s/$regex/$modex/g;
        print $toFileFP $line;
      } else {
        print $toFileFP $line;
      }
    }
    close $fromFileFP;
    close $toFileFP;

    &ExecuteCmd("mv $toFile $fromFileName");
  }


#=============================================================================
#
# FUNCTION: RpcgenAddCopyright
#
# DESCRIPTION:  Add Copyright Contents in the rpcgen files
#
# PARAMETERS:  - fromDir = Directory where the file is available
#              - fromFile = File where the Copyright content has to be added.
#============================================================================*/
sub RpcgenAddCopyright($$$)
  {
    my $fromDir = shift(@_);
    my $fromFile = shift(@_);
    my $api_type = shift(@_);
    my $fromFileFP;
    my $toFileFP;
    my $line;
    my $done=0;

    my $fromFileName = $fromDir."\/".$fromFile;
    my $toFile = $fromDir."\/temp_".$fromFile;
    #print "ADDING Copyright to $fromFileName for APITYPE:$api_type \n";
    if (! -f $fromFileName) {
      PrintAndLog("File does not exist $fromFileName in RpcgenAddCopyright \n");
      return;
    }
    open($fromFileFP, "<", $fromFileName) or die $!;
    open( $toFileFP, ">", $toFile) or die $!;

    if ($api_type =~ /^U/) {
      #print "USER-SPACE COPYRIGHT \n";
      print $toFileFP "\/* @Copyright *\/ \n";
    } else {
      #print "Kernel COpyright @CopyrightKernel \n";
      print $toFileFP "\/* @CopyrightKernel *\/ \n";
    }

    while ($line = <$fromFileFP>) {
      print $toFileFP $line;
    }
    close $fromFileFP;
    close $toFileFP;
    my @cmd_out = &ExecuteCmd("mv $toFile $fromFileName");
    #print "@cmd_out \n";
  }


#=============================================================================
#
# FUNCTION: XdrAddInclude
#
# DESCRIPTION:  Function to add include headers in a rpcgen file
#
# PARAMETERS:  - fromDir = Directory where rpcgen file is present
#              - fromFile = File where include has to be added
#              - regex = pattern after which the new include has to be added.
#              - newInclude = Name of the new header file to be included.
#============================================================================*/
sub XdrAddInclude($$$$)
  {
    my $fromDir = shift(@_);
    my $fromFile = shift(@_);
    my $regex    = shift(@_);
    my $newInclude = shift(@_);
    my $fromFileFP;
    my $toFileFP;
    my $line;
    my $done=0;

    my $fromFileName = $fromDir."\/".$fromFile;
    my $toFile = $fromDir."\/temp_".$fromFile;
    if (! -f $fromFileName) {
      PrintAndLog("File does not exist $fromFileName in XdrAddInclude\n");
      return;
    }
    open($fromFileFP, "<", $fromFileName) or die $!;
    open( $toFileFP, ">", $toFile) or die $!;

    while ($line = <$fromFileFP>) {
      if ( ( $done == 0) && ($line =~ /$regex/ ) ) {
        print $toFileFP $line;
        print $toFileFP "$newInclude \n";
        $done=1;
      } else {
        print $toFileFP $line;
      }
    }
    close $fromFileFP;
    close $toFileFP;
    &ExecuteCmd("mv $toFile $fromFileName");
  }


#=============================================================================
#
# FUNCTION: XdrApiExceptionSnd
#
# DESCRIPTION:
#    Handle SND Exception, the rpc_snd_device_type must be extracted into
#    a separate file (snd_surf.xdr)
#
# PARAMETERS: work_dir_pathname = Directory where snd.xdr is present
#
#============================================================================*/
sub XdrApiExceptionSnd($$$)
{
  my $work_dir_pathname = shift(@_);
  my $xdr_filename = shift(@_);
  my $strip_xdr_filename = shift(@_);
  my $hdr;
  my $strip_content;

$hdr = <<"EOF";
/*============================================================================
                     S N D  S U R F . X D R

GENERAL DESCRIPTION
  This is an AUTO GENERATED file that provides an xdr compatible definition of
  an api that represents the grouping of the different callback functions the
  snd_surf API supports.

  ---------------------------------------------------------------------------
  Copyright (c) 2009 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
=============================================================================*/
EOF


  print $line_separator;
  #my $xdr_filepath = $xdr_path."\/".$xdr_filename;
  $xdr_filename  =~ /(.*).xdr$/;

  my $file_path = $work_dir_pathname."\/".$xdr_filename ;
 local $/ = undef;
  my $line;

  my $fh;
  my $strip_fh;

  print "open $file_path\n";
  open($fh, "<", $work_dir_pathname."\/".$xdr_filename) or die $!;

  my $xdrfile_content = <$fh>;
  close($fh);

  if($xdrfile_content =~ s/(enum\srpc_snd_device_type\s\{[^\}]*\}\;)//)
  {
      print("Sucessfully stripped rpc_snd_device_type  \n");
      $strip_content = $1;

  #print $xdrfile_content;

  open($fh, ">", $work_dir_pathname."\/".$xdr_filename) or die $!;
  print $fh $xdrfile_content;
  close($fh);

  open($strip_fh, ">", $work_dir_pathname."\/".$strip_xdr_filename) or die $!;

  print $strip_fh $hdr;
  print $strip_fh "\n\n";
  print $strip_fh $strip_content;
  close($strip_fh);
  }
}


#=============================================================================
#
# FUNCTION: XdrReplaceRegexSampleCode
#
# DESCRIPTION: Replace "insert server code here" pattern with a
#              printf statement in <api>_cb_sample.c file
#
# PARAMETERS: fromDir = Directory where <api>_cb_sample.c found
#             fromFile = File name
#             modex = Pattern to be used for substitution
#
#============================================================================*/
sub XdrReplaceRegexSampleCode($$$)
  {
    my $fromDir = shift(@_);
    my $fromFile = shift(@_);
    my $modex = shift(@_);
    my $fromFileFP;
    my $toFileFP;
    my $line;
    my $infile;

    local $/ = undef;

    my $fromFileName = $fromDir."\/".$fromFile;
    my $toFile = $fromDir."\/temp_".$fromFile;

    if (! -f $fromFileName) {
      PrintAndLog("File does not exist $fromFileName in XdrReplaceRegex \n");
      return;
    }

    open($fromFileFP, "<", $fromFileName) or die  $!;
    open( $toFileFP, ">", $toFile) or die $!;

    $infile = <$fromFileFP>;

    if($infile =~ /\/\*\s*\*\s*insert\sserver\scode\shere\s*\*\//)
    {
      $infile =~ s /\/\*\s*\*\s*insert\sserver\scode\shere\s*\*\//$modex/g;
    }

    print $toFileFP $infile;
    close $fromFileFP;
    close $toFileFP;

    &ExecuteCmd("mv $toFile $fromFileName");
  }

#=============================================================================
#
# FUNCTION: display_usage
#
# DESCRIPTION:  Display help message
#
#============================================================================*/
sub display_usage() {
  print "Usage: rpcgen_apis.pl [OPTION]\n";
  print "OPTION:\n";
  print "-h, --help              Prints this help message\n";
  print "-api=API_NAME           API for which RPCGEN has to\n";
  print "                        be invoked.\n";
  print "-sd=<PATH>              Path where xdr files can be found\n";
  print "-sf=<File name>         Name of the Xdr file used by rpcgen\n";
  print "-td=<PATH>              Path where generated rpcgen files have\n";
  print "                        to be stored.\n";
  print "-tf=<File name>         Name of the .c/.h file to be generated\n";
  print "-has_cb=0/1             If api has call-back corresponding updates\n";
  print "                        will be made in generated clnt.c, xdr.c &\n";
  print "                        rpc.h files.\n";
}


#=============================================================================
#
# FUNCTION: BuildApisList
#
# DESCRIPTION: Builds remote_apis_list & rpcgen_apis_list based on
#              remote_api_enables.mk for all the targets. It also populates
#              callback & version info of an rpcgen api
#
#============================================================================*/
sub BuildApisList($) {
  my $fd;
  my $line;
  my $key;
  my $sourceDirectory = shift(@_);
  my $api_enable_file_name = "remote_api_enables.mk";
  my $remote_apis_subdir = "api/libs/remote_apis";
  my $api_report_subpath = "api/ApiReport.txt";

  my $remote_apis_path;
  my $remote_apis_dir;
  my @all_remote_apis_dir = ();
  my $result;
  my $delete_api;
  my @delete_apis_list = ("COMMONDEFS");

  my $target_api_enables = $sourceDirectory."/".$api_enable_file_name;
  open($fd, "<", $target_api_enables) or next;
  while($line = <$fd>) {
    if($line =~ /(\w*)(_RPCGEN_ENABLE)/) {
      my $api = $1;
      if(!exists($rpcgen_apis_list{$api})) {
        $rpcgen_apis_list{$api}{'dir'} = lc($api);
        $rpcgen_apis_list{$api}{'cb'} = 0;
        $rpcgen_apis_list{$api}{'ver'} = 0;
      }
    }
  }
  close($fd);

  foreach $delete_api (@delete_apis_list) {
    delete($rpcgen_apis_list{$delete_api});
  }

  my $api_report_file = $sourceDirectory."/".$api_report_subpath;
  my $api_report_fh;
  my $api_report_data;
  open($api_report_fh, "<", $api_report_file) or die;
  while($api_report_data = <$api_report_fh>) {
    if($api_report_data =~ /^(\w+)(\s+)/) {
      my $api = $1;
      if(exists($rpcgen_apis_list{uc($api)})) {
        if($api_report_data =~ /^$api(.*)(\w{3})\|(\w{5})(.*)$/) {
          $rpcgen_apis_list{uc($api)}{'cb'} = 1;
        }
        if($api_report_data =~ /^$api(.*)(\w{8}):(\w{8})(.*)$/) {
          $rpcgen_apis_list{uc($api)}{'ver'} = hex($3);
        }
      }
    }
  }
  close($api_report_fh);

  foreach $key (sort keys %rpcgen_apis_list) {
    print "$key $rpcgen_apis_list{$key}{'dir'} $rpcgen_apis_list{$key}{'cb'} $rpcgen_apis_list{$key}{'ver'}\n";
  }
  print "\n";
}

#=============================================================================
#
# FUNCTION: GenerateRpcgenApisVerifySrc
#
# DESCRIPTION:  Generate rpcgen_apis_verify.c file based on the
#               rpcgen_apis_list, callback & version information
#
#============================================================================*/
sub GenerateRpcgenApisVerifySrc($$) {
  my $srcFp;
  my $targetDirectory = shift(@_);
  my $targetFilename = shift(@_);
  my $key;
  my $temp;
  my $src_path = $targetDirectory."/".$targetFilename;
  my $rpcgenApisSrcHeader;
  $rpcgenApisSrcHeader = <<"EOF";
/******************************************************************************
  \@file  test_librpc
  \@brief Linux user-space to verify basic functionality of librpc

  DESCRIPTION
  Librpc test program for Linux user-space to verify compatibility of APIS
  in user-space.

  -----------------------------------------------------------------------------
  Copyright (c) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
******************************************************************************/
/*=====================================================================
  Note: This file is auto-generated, do not modify
======================================================================*/
#include <stdio.h>
#include "librpc.h"

EOF

  open( $srcFp, ">",$src_path );
  print $srcFp $rpcgenApisSrcHeader;

  foreach $key ( sort keys %rpcgen_apis_list )
  {
    my $api = lc($key);
    print $srcFp "#ifdef FEATURE_EXPORT_".$key."_RPCGEN\n";
    print $srcFp "#include \"".$api."_rpcgen_rpc.h\"\n";
    if($rpcgen_apis_list{$key}{'cb'} == 1)
    {
      print $srcFp "#include \"".$api."_rpcgen_common_rpc.h\"\n";
      print $srcFp "#include \"".$api."_rpcgen_cb_rpc.h\"\n";
    }
    print $srcFp "#endif\n";
    print $srcFp "\n\n";
  }

  $temp = <<"EOF";
/*=====================================================================
     Constants
======================================================================*/
#define RESULT_ARRAY_SIZE  (100)
#define LOCAL_CBPROGHASH_INIT_VAL (0xfffffffe)
#define LOCAL_PROGHASH_INIT_VAL   (0xfffffffe)
#define REMOTE_CBPROGHASH_INIT_VAL (0xffffffff)
#define REMOTE_PROGHASH_INIT_VAL   (0xffffffff)


/*=====================================================================
      Macros
======================================================================*/
#define RPC_FUNC_VERSION_BASE(a,b) a ## b
#define RPC_FUNC_VERSION(a,b) RPC_FUNC_VERSION_BASE(a,b)

#define MINOR_VERS(a) \\
   (a & 0x0000ffff)

#define MAJOR_VERS(a) \\
   ( (a & 0x0fff0000) >> 16)

/*=====================================================================
       Type Definitions
======================================================================*/
typedef struct
{
   uint32 toolvers;
   uint32 features;
   uint32 proghash;
   uint32 cbproghash;
}api_data_type;

typedef struct
{
   const char *name;
   const char *location;
   uint32   prog;
   uint32   vers;
   api_data_type  local_api_info;
   api_data_type  remote_api_info;
   api_data_type  match_api_info;
   int   verified;
   int   found;
   int   valid;
   int   used;
}oncrpc_lib_verify_status_type;

/*=====================================================================
     Forward Declaration
======================================================================*/
static oncrpc_lib_verify_status_type oncrpc_lib_verify_status[RESULT_ARRAY_SIZE];
static uint32 verify_status_index = 0;
static void check_api_status(uint32 prog,
   uint32 vers,
   api_data_type *remote_api_info,
   api_data_type *local_api_info);


/*=====================================================================
     Local Data
======================================================================*/
static int mismatch_errors_found = 0;
static int api_not_found_error = 0;
static char *VersionStr="1.1";
static const char *TestName = "LIBRPC / RPCGEN Client API Verification Test";

EOF
  print $srcFp $temp;

  foreach $key ( sort keys %rpcgen_apis_list )
  {
    my $api = lc($key);
    print $srcFp "#ifdef FEATURE_EXPORT_".$key."_RPCGEN\n";
    print $srcFp "static CLIENT *".$api."_client;\n";
    print $srcFp "#endif\n";
    print $srcFp "\n";
  }

  $temp = <<"EOF";
/*=====================================================================
 FUNCTION  check_api_status
===========================================================================*/
/*!
\@brief
Save the status of this API in local database

\@return
None

\@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
static void check_api_status(uint32 prog,
   uint32 vers,
   api_data_type *remote_api_info,
   api_data_type *local_api_info)
{
   uint32 i;
   int found_index=-1;
   for( i=0;i<verify_status_index;i++ )
   {
      if( (prog == oncrpc_lib_verify_status[i].prog) && (vers == oncrpc_lib_verify_status[i].vers) )
      {
         /* API Exists in database, just update the data */
         oncrpc_lib_verify_status[i].local_api_info = *local_api_info;
         oncrpc_lib_verify_status[i].remote_api_info = *remote_api_info;
         found_index = i;
         break;
      }
   }

   if( found_index == -1 )
   {
      if( verify_status_index >= RESULT_ARRAY_SIZE )
      {
         printf("ERROR Cannot add entry, database full \\n");
         return;
      }

      /* Add a new entry*/
      oncrpc_lib_verify_status[verify_status_index].prog = prog;
      oncrpc_lib_verify_status[verify_status_index].vers = vers;
      oncrpc_lib_verify_status[verify_status_index].name = "unknown";
      oncrpc_lib_verify_status[verify_status_index].found = -1;
      oncrpc_lib_verify_status[verify_status_index].local_api_info = *local_api_info;
      oncrpc_lib_verify_status[verify_status_index].remote_api_info = *remote_api_info;
      found_index = verify_status_index;
      verify_status_index ++;
   }


   if( ( MAJOR_VERS(local_api_info->proghash) ==  MAJOR_VERS(remote_api_info->proghash) ) &&
       ( MINOR_VERS(local_api_info->proghash) <= MINOR_VERS(remote_api_info->proghash ) ) )
   {
      oncrpc_lib_verify_status[found_index].match_api_info.proghash = 1;
   } else
   {
      printf(" Api 0x%08x Versions Are not compatible Local:0x%08x, Remote:0x%08x ",((int)prog),((int)local_api_info->proghash),((int)remote_api_info->proghash));
   }

   if( ( (MAJOR_VERS(local_api_info->cbproghash) ==  MAJOR_VERS(remote_api_info->cbproghash) ) &&
         (MINOR_VERS(local_api_info->cbproghash) <=  MINOR_VERS(remote_api_info->cbproghash) ) )||
      ( (local_api_info->cbproghash == LOCAL_CBPROGHASH_INIT_VAL) &&
      (remote_api_info->cbproghash == REMOTE_CBPROGHASH_INIT_VAL) ) )
   {
      oncrpc_lib_verify_status[found_index].match_api_info.cbproghash = 1;

   } else
   {
      printf(" Api 0x%08x Callback Hashkey Differs Local:0x%08x, Remote:0x%08x ",((int)prog),((int)local_api_info->cbproghash),((int)remote_api_info->cbproghash));
   }

   if( (oncrpc_lib_verify_status[found_index].match_api_info.proghash  == 1) &&
      (oncrpc_lib_verify_status[found_index].match_api_info.cbproghash == 1) )
   {
      oncrpc_lib_verify_status[found_index].verified = 1;
      printf("OK API matches with remote server Api:0x%08x\\n",((int)prog));
   } else
   {
      if( oncrpc_lib_verify_status[found_index].found == 1 )
      {
         printf("\\n***ERROR*** API DOES NOT MATCH with remote server Api:0x%08x\\n",((int)prog));
         mismatch_errors_found++;
      }
   }
   printf("--------------------------------------------------------------------");
}

/*===========================================================================
    FUNCTION  update_status_lookup
===========================================================================*/
/*!
\@brief
Update and save the status of this API in local database

\@return
None

\@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
static void update_status_lookup(uint32 prog, uint32 vers, const char *name, const char *location,int found)
{
   uint32 i;
   for( i=0;i<verify_status_index;i++ )
   {
      if( (prog == oncrpc_lib_verify_status[i].prog) && (vers == oncrpc_lib_verify_status[i].vers) )
      {
         /* Exists in result db */
         oncrpc_lib_verify_status[i].found = found;
         oncrpc_lib_verify_status[i].name = name;
         return;
      }
   }
   if( verify_status_index >= RESULT_ARRAY_SIZE )
   {
      printf("Cannot add entry, database full \\n");
      return;
   }
   /* Add a new entry*/
   oncrpc_lib_verify_status[verify_status_index].prog = prog;
   oncrpc_lib_verify_status[verify_status_index].vers = vers;
   oncrpc_lib_verify_status[verify_status_index].name = name;
   oncrpc_lib_verify_status[verify_status_index].location = location;
   oncrpc_lib_verify_status[verify_status_index].found = found;
   verify_status_index++;
}


/*===========================================================================
  FUNCTION  print_status_db
===========================================================================*/
/*!
\@brief
  Print the status of an API based on current database entries.

\@return
  None

\@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static void print_status_db(void)
{
   uint32 i;
   int ver_hi;
   int ver_lo;
   for( i=0;i<verify_status_index;i++ )
   {
      printf("%16s, Prog:0x%08x, V:0x%08x",
         oncrpc_lib_verify_status[i].name,
         (unsigned int)oncrpc_lib_verify_status[i].prog,
         (unsigned int)oncrpc_lib_verify_status[i].vers);
      if( oncrpc_lib_verify_status[i].found )
      {
         printf(" Found    ");
         ver_lo = (int)oncrpc_lib_verify_status[i].remote_api_info.toolvers & 0xffff;
         ver_hi = ((int)oncrpc_lib_verify_status[i].remote_api_info.toolvers & 0xffff0000)>>16;

         printf("V:%d.%d Feat:0x%02x ",((int)ver_hi),((int)ver_lo),((int)oncrpc_lib_verify_status[i].remote_api_info.features));
         if( oncrpc_lib_verify_status[i].verified )
         {
            printf(" OK ");
         } else
         {
            printf(" ERR ");
         }
      } else
      {
         printf(" Not Found");
         printf("V:X Feat:----  N/A");
      }

      printf("\\n");
   }
}


EOF
  print $srcFp $temp;

  foreach $key ( sort keys %rpcgen_apis_list )
  {
    my $api = lc($key);
    my $version_hex = sprintf("0x%08x",$rpcgen_apis_list{$key}{'ver'});
    print $srcFp "#ifdef FEATURE_EXPORT_".$key."_RPCGEN\n";
    $temp = <<"EOF";
/*===========================================================================
  FUNCTION  $api\_null
===========================================================================*/
/*!
\@brief
  Test using data ping to modem

\@return
  0

\@note
  - Dependencies : LIBRPC, $key
*/
/*=========================================================================*/
static int $api\_null(CLIENT* client)
{
   uint32 args=0;
   enum clnt_stat result;
   rpc_$api\_rpc_glue_code_info_remote_rets rets;

   if( ! client )
   {
      return 0;
   }

   #if defined RPC_\U$api\E_NULL_VERSION
   result = RPC_FUNC_VERSION(rpc_$api\_rpc_glue_code_info_remote_,RPC_\U$api\E_NULL_VERSION)(&args,&rets,client);
   #elif defined  RPC_\U$api\E_API_VERSION_IS_HASHKEY
   result = RPC_FUNC_VERSION(rpc_$api\_rpc_glue_code_info_remote_,\U$api\EVERS)(&args,&rets,client);
   #else
   result = RPC_FUNC_VERSION(rpc_$api\_rpc_glue_code_info_remote_,\L$version_hex\E)(&args,&rets,client);
   #endif

   return (result == RPC_SUCCESS);
}

/*===========================================================================
  FUNCTION  $api\_rpc_glue_code_info_remote
===========================================================================*/
/*!
\@brief
  Wrapper function to get remote API info through rpcgen api

\@return
  0

\@note
  - Dependencies : LIBRPC, $key
*/
/*=========================================================================*/
static int $api\_rpc_glue_code_info_remote(
   CLIENT* client,
   uint32 *toolvers,
   uint32 *features,
   uint32 *proghash,
   uint32 *cbproghash
   )
{
   enum clnt_stat result;
   if( !client || !toolvers || !features || !proghash || !cbproghash )
      return 0;

   uint32 args=0;
   rpc_$api\_rpc_glue_code_info_remote_rets rets;

   if( ! client )
   {
      return 0;
   }

   #if defined RPC_\U$api\E_NULL_VERSION
   result = RPC_FUNC_VERSION(rpc_$api\_rpc_glue_code_info_remote_,RPC_\U$api\E_NULL_VERSION)(&args,&rets,client);
   #elif defined  RPC_\U$api\E_API_VERSION_IS_HASHKEY
   result = RPC_FUNC_VERSION(rpc_$api\_rpc_glue_code_info_remote_,\U$api\EVERS)(&args,&rets,client);
   #else
   result = RPC_FUNC_VERSION(rpc_$api\_rpc_glue_code_info_remote_,\L$version_hex\E)(&args,&rets,client);
   #endif

   if( result == RPC_SUCCESS )
   {
      *toolvers = rets.toolvers;
      *features = rets.features;
      *proghash = rets.proghash;
      *cbproghash = rets.cbproghash;
   }
   return (result == RPC_SUCCESS);
}

/*===========================================================================
  FUNCTION  $api\_rpc_glue_code_info_local
===========================================================================*/
/*!
\@brief
  Wrapper function to get local API info.

\@return
  0

\@note
  - Dependencies : LIBRPC, $key
*/
/*=========================================================================*/
static int $api\_rpc_glue_code_info_local(
   uint32 *toolvers,
   uint32 *features,
   uint32 *proghash,
   uint32 *cbproghash
   )
{
EOF
    print $srcFp $temp;

    print $srcFp "   *proghash = ".$key."VERS;\n";
    print $srcFp "#ifdef ".$key."CBVERS\n";
    print $srcFp "   *cbproghash =".$key."CBVERS;\n";
    print $srcFp "#else\n";
    print $srcFp "   *cbproghash =".$key."VERS;\n";
    print $srcFp "#endif\n";
    my $temp = <<"EOF";
    *features = 0;
   *toolvers = 0;
   return 1;
}
#endif

EOF
    print $srcFp $temp;
  }

  $temp = <<"EOF";
/*===========================================================================
  FUNCTION  main
===========================================================================*/
/*!
\@brief
  entry to test

\@return
  0

\@note
  - Dependencies
    - ONCRPC
  - Side Effects
*/
/*=========================================================================*/
int main(int argc, char *argv[])
{
   uint32 prog;
   uint32 vers;
   uint32 found;
   api_data_type  remote_api_info;
   api_data_type  local_api_info;
   uint32 i;

EOF
  print $srcFp $temp;

  foreach $key ( sort keys %rpcgen_apis_list )
  {
    my $api = lc($key);
    print $srcFp "#ifdef FEATURE_EXPORT_".$key."_RPCGEN\n";
    print $srcFp $api."_client = clnt_create(NULL,".$key."PROG, ".$key."VERS, NULL);\n";
    print $srcFp "#endif\n";
    print $srcFp "\n";
  }


  $temp = <<"EOF";

   printf("%s, version %s  Started\\n",TestName,VersionStr);

   memset(oncrpc_lib_verify_status,0,sizeof(oncrpc_lib_verify_status_type)*RESULT_ARRAY_SIZE);
   verify_status_index = 0;

   for( i=0;i<RESULT_ARRAY_SIZE;i++ )
   {
      oncrpc_lib_verify_status[i].remote_api_info.toolvers = -1;
      oncrpc_lib_verify_status[i].remote_api_info.features = 0;
      oncrpc_lib_verify_status[i].remote_api_info.proghash = 0xffffffff;
      oncrpc_lib_verify_status[i].remote_api_info.cbproghash = 0xffffffff;
      oncrpc_lib_verify_status[i].local_api_info.toolvers = -2;
      oncrpc_lib_verify_status[i].local_api_info.features = 0;
      oncrpc_lib_verify_status[i].local_api_info.proghash = 0xfffffffe;
      oncrpc_lib_verify_status[i].local_api_info.cbproghash = 0xfffffffe;
   }

EOF
  print $srcFp $temp;

  foreach $key ( sort keys %rpcgen_apis_list )
  {
    my $api = lc($key);
    print $srcFp "\n";
    print $srcFp "#ifdef FEATURE_EXPORT_".$key."_RPCGEN\n";
    print $srcFp "   printf(\"\\nVerifying API: ".$key."\\n\"); \n";
    print $srcFp "   prog = ".$key."PROG; \n";
    print $srcFp "   vers = ".$key."VERS; \n";
    print $srcFp "   found = ".$api."_null(".$api."_client); \n";
    print $srcFp "   if( found ) \n";
    print $srcFp "   { \n";
    print $srcFp "      update_status_lookup(prog,vers,\"".$key."\",\"libs\\\\remote_apis\\\\".$api."\",found); \n";
    print $srcFp "      ".$api."_rpc_glue_code_info_remote(".$api."_client,&remote_api_info.toolvers,&remote_api_info.features,&remote_api_info.proghash,&remote_api_info.cbproghash); \n";
    print $srcFp "      ".$api."_rpc_glue_code_info_local(&local_api_info.toolvers,&local_api_info.features,&local_api_info.proghash,&local_api_info.cbproghash); \n";
    print $srcFp "      check_api_status(prog,vers, &remote_api_info,&local_api_info);\n";
    print $srcFp "   } else\n";
    print $srcFp "   {\n";
    print $srcFp "      api_not_found_error ++; \n";
    print $srcFp "   } \n";
    print $srcFp "#endif\n";
    print $srcFp "\n";
  }


  $temp = <<"EOF";
   printf("\\n\\nResults: \\n");
   print_status_db();
   printf("\\n\\n");

   if( (mismatch_errors_found== 0) && (api_not_found_error == 0 ) )
   {
      printf("PASS \\n");
      return(0);
   } else
   {
      if( mismatch_errors_found != 0 )
      {
         printf("ERRORS FOUND, %d API(s) do not match, please look at log \\n",mismatch_errors_found);
      }

      if( api_not_found_error != 0 )
      {
         printf("ERRORS FOUND, %d API(s) were not found on remote processor, please look at log \\n",api_not_found_error );
      }
      printf("FAIL \\n");
      return(-1);
   }
}
EOF
  print $srcFp $temp;
  close $srcFp;
}
