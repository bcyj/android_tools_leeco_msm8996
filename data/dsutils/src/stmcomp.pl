#!/usr/bin/perl -w
##########################################################################
# ------------------------------------------------------------------------
# State Machine/Group Compiler
#
# Description:
#   This is the state machine compiler for use with the STM service.  For
#   full documentation, see the stm.c/h source code and examples/... directory.
#   A BNF grammar description of what is parsed by this compiler is given below.
#
#
# Copyright (c) 2007-2009 Qualcomm Technologies, Inc. All Rights Reserved
#
#                    Qualcomm Technologies Proprietary
#
# Export of this technology or software is regulated by the U.S. Government.
# Diversion contrary to U.S. law prohibited.
#
# All ideas, data and information contained in or disclosed by
# this document are confidential and proprietary information of
# Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
# By accepting this material the recipient agrees that this material
# and the information contained therein are held in confidence and in
# trust and will not be used, copied, reproduced in whole or in part,
# nor its contents revealed in any manner to others without the express
# written permission of Qualcomm Technologies, Inc.
# ------------------------------------------------------------------------
##########################################################################
#
#                         EDIT HISTORY FOR MODULE
#
# This section contains comments describing changes made to the module.
# Notice that changes are listed in reverse chronological order.
#
# $Header: /local/mnt/workspace/randrew/7x30_1/vendor/qcom-proprietary/data/common/src/RCS/stmcomp.pl,v 1.1 2010/02/17 23:56:34 randrew Exp $
#
# when       who     what, where, why
# --------   ---     -----------------------------------------------------
# 12/08/09   tjc     Exit cleanly if the STM file is empty
# 11/16/09   tjc     Include files using <> instead of "" to prevent lint
#                    warnings for external include files
# 05/23/08   trc     Fix to allow multiple child SMs in an SM
# 04/21/08   trc     Make non-top-level reset functions static
# 02/15/08   trc     Update to use 3 auto-generated file output paradigm
#                    new INPUT_DEF_FILE keyword support
# 09/25/07   trc     Prefix child SM name with parent SM name + "__" + state
# 08/07/07   trc     Adapt code for new calling SM param in stm_state_machine_t
# 07/26/07   trc     Add an externalized state-machine/state-name enum
# 05/08/07   trc     Use STATIC pseudo-keyword for toggle-able static data
# 05/01/07   trc     Allow visibility of data structures only by defining
#                    STM_DATA_STRUCTURES_ONLY
# 04/15/07   trc     Make generated header files extern "C" for C++ users
# 04/03/07   trc     Add unique SM instance ID
# 04/02/07   trc     Refactor state machine structure into non-const, all-inst
#                    const, and per-inst const structures for better static
#                    initialization and 'reset button' properties
# 01/17/07   trc     Initial version (2.0)
#
##########################################################################

##########################################################################
#
#  BNF Grammar for STM 2.2 language:
#
#  <{state-machine}> ::= STATE_MACHINE <sm name> <{sm-block}> ...
#
#  <{sm-block}> ::=      ENTRY          <function name>;                |
#                        EXIT           <function name>;                |
#                        INITIAL_STATE  <state name>;                   |
#                        ERROR_HOOK     <function name>;                |
#                        DEBUG_HOOK     <function name>;                |
#                        INPUT_DEF_FILE <file name>; ...                |
#                       [INSTANCES      <# of instances>;]              |
#                        STATE          <state name> <{state-block}> ...
#
#  <{state-block}> ::=   ENTRY          <function name>;                |
#                        EXIT           <function name>;                |
#                        INPUTS         <{inputs-block}>                |
#                       [STATE_MACHINE  <{sub-sm-block}>]
#
#  <{inputs-block}> ::= <input name> [<function name>]; ...
#
#  <{sub-sm-block}> ::=  ENTRY          <function name>;                |
#                        EXIT           <function name>;                |
#                        INITIAL_STATE  <state name>;                   |
#                       [ERROR_HOOK     <function name>;]               |
#                       [DEBUG_HOOK     <function name>;]               |
#                        STATE          <state name> <{state-block}> ...
#
##########################################################################

##########################################################################
#
#                            Imported Modules
#
##########################################################################

# Add a little negative reinforcement here to keep things tidy
use strict;
use warnings;

# Allow us to get the current year for copyright generation
use Time::localtime;

# Pull in the cmd line options processor
use Getopt::Long;

# Pull in the MD5 message digest algorithm for unique IDs based upon name
use Digest::MD5 qw(md5_hex);


##########################################################################
#
#                            Global Variables
#
##########################################################################

# Global debug level to set verbosity level
my $debuglevel = 1;

# Get the current year, for copyright verbiage
my $currentyear = localtime->year() + 1900;

# Global 'cleanup' variable for INT signal handler
my @files_created = ();


##########################################################################
#
#                              Subroutines
#
##########################################################################

# ------------------------------------------------------------------------
# Name       :  Debug
# Description:  Selectively print debugging messages based upon global
#               debuglevel and the level of the particular message.  If the
#               level of the message is < 0, this is considered fatal, so
#               perform an exit(-msglevel) and terminate.
# Parameters :
# Returns    :
# Notes      :
# ------------------------------------------------------------------------
sub Debug($$) {
  my $msglevel = shift;
  my $msg      = shift;
  print STDERR "$msg\n" unless ( $debuglevel <= $msglevel );
  exit( -$msglevel )    unless ( $msglevel >= 0 );
}

# ------------------------------------------------------------------------
# Name       :  TokenizeFile
# Description:  Takes a filename and token data reference.
#               Tokenizes file and populated token data structure.
# Parameters :
# Returns    :  token data array
# Notes      :
# ------------------------------------------------------------------------
sub TokenizeFile($$) {
  my ( $infilename, $tokendataref ) = @_;

  # Try slurping in the file we were given
  open( my $fh, "<$infilename" )
    || Debug( -1, "Error, can't open file: $infilename" );
  my @infile = <$fh>;
  close( $fh );
  Debug( 2,
         "Processing " . scalar( @infile ) . " lines from file: $infilename" );

  # Spin through the file and tokenize it (keeping track of filename/line #)
  my $incomment = 0;
  my $filename  = $infilename;
  my $linenum   = 1;
  for ( ; $_ = shift( @infile ) ; $linenum++ ) {

    #
    # Strip/skip any comments - the preprocessor should have done this, but
    # we'll just be redundant in-case someone is running this on a file
    # that doesn't need preprocessing (for #include, #ifdef, ...) and wants
    # to check the output or verify correct input syntax.
    #

    # If inside a multi-line comment
    if ( $incomment ) {

      # ... and no end of comment in sight, proceed to next
      next unless ( m|\*/| );

      # Otherwise, strip off the commented portion and continue processing
      s|^.*?\*/(.*)$|$1|;

      # We are now out of the multiline comment
      $incomment = 0;
    }

    # Remove single-line-bound comments: /* xxx */ or // xxx
    s|/\*.*?\*/||g;
    s|//.*$||;

    # Check if beginning of a multi-line comment
    if ( m|/\*.*[^*][^/]| ) {

      # Nuke the comment part, but keep the preceding text
      s|/\*.*$||;

      # Keep track of the fact that we are inside a multiline comment
      $incomment = 1;
    }

    # Skip purely blank lines, now that all the comments have been stripped
    next if ( m/^\s*$/ );

    #
    # Handle the preprocessor insertions  '#[line] <line #> "filename"'
    #
    if ( my ( $ppins ) = $_ =~ m/^\s*#\s*(.*)$/ ) {

      Debug( 4, "Preprocessor annotation:  $ppins" );

      # GNU cpp just does '# <number> <filename>', ARM does '#line <number> <filename>'
      if ( my ( $line, $file ) =
           $ppins =~ m/^(?:line)?\s*(\d+)\s+\"([^<].+[^>])\"/ )
      {
        # line # specified is the NEXT line in the file, so predecrement
        $linenum  = $line - 1;
        # Only grab the basename of the file, no need for paths
        # and the possible slash/backslash complications
        ( $filename ) = $file =~  m|([^/\\]+)$|;
        Debug( 3, "Using preprocessor line/file info:  $line/$filename" );
      } else {
        # Complain/warn if we hit a preprocessor directive we CAN'T handle
        Debug( 1 , "$filename:$linenum:Warning, preprocessor directive" .
                   " '$ppins' ignored.");
      }

      # Nothing more to do with this line, skip to next
      next;
    }

    # Pad any delimiters with whitespace so we can split them apart from
    # any adjacent words
    s/([,;{}\[\]])/ $1 /g;

    # Save the tokens and their file/line # info
    # File/line # info is cheesy, but it's needed so the lexer can report
    # roughly where syntax errors occur.
    foreach ( split ) {
      push( @{ $tokendataref }, "$_ $filename $linenum" );
    }

  }

  # Push an 'end of file' token so the lexer knows when there's no more
  push( @{ $tokendataref }, "_EOT_ $filename $linenum" );

  Debug( 2, "Extracted " . scalar( @$tokendataref ) . " tokens" );

  # Just return - lexer will catch/flag any syntax errors
}

# ------------------------------------------------------------------------
# Name       :  ExtractToken
# Description:  Extracts a token 'index' positions from the current stream head
# Parameters :  Input token data array reference, index of token needed
# Returns    :  token at specified index in token array or "" if past EOT
# Notes      :
# ------------------------------------------------------------------------
sub ExtractToken($$) {
  my ( $tokendataref, $index ) = @_;
  my $token = "";

  # Check num of tokens left
  if ( scalar( @$tokendataref ) > $index ) {
    $token = ( split( / /, $$tokendataref[ $index ] ) )[ 0 ];
  }

  return ( $token );
}

# ------------------------------------------------------------------------
# Name       :  ExtractFilename
# Description:  Extracts the filename associated with a token 'index' positions
#               from the current stream head
# Parameters :  Input token data array reference, index of token needed
# Returns    :  Filename or "" if no token exists
# Notes      :
# ------------------------------------------------------------------------
sub ExtractFilename($$) {
  my ( $tokendataref, $index ) = @_;
  my $filename = "";

  # Check num of tokens left
  if ( scalar( @$tokendataref ) > $index ) {
    $filename = ( split( / /, $$tokendataref[ $index ] ) )[ 1 ];
  }

  return ( $filename );
}

# ------------------------------------------------------------------------
# Name       :  ExtractLineNumber
# Description:  Extracts the line # associated with a token 'index' positions
#               from the current stream head
# Parameters :  Input token data array reference, index of token needed
# Returns    :  line number or "" if no token exists
# Notes      :
# ------------------------------------------------------------------------
sub ExtractLineNumber($$) {
  my ( $tokendataref, $index ) = @_;
  my $linenum = "";

  # Check num of tokens left
  if ( scalar( @$tokendataref ) > $index ) {
    $linenum = ( split( / /, $$tokendataref[ $index ] ) )[ 2 ];
  }

  return ( $linenum );
}

# ------------------------------------------------------------------------
# Name       :  ConsumeTokens
# Description:  Gobble up a number of tokens from the head of the token stream
# Parameters :  token data array, number of tokens to gobble
# Returns    :
# Notes      :
# ------------------------------------------------------------------------
sub ConsumeTokens($$) {
  my ( $tokendataref, $num ) = @_;

  # Consume $num tokens or as many as are left (ie. all of them)
  if ( scalar( @$tokendataref ) >= $num ) {
    splice( @$tokendataref, 0, $num );
  }
  else {
    @$tokendataref = ();
  }
}

# ------------------------------------------------------------------------
# Name       :  ErrorMsgTok
# Description:  Print an error message referencing the current token stream head
# Parameters :  Prefix text for the message, token data array reference
# Returns    :  Error message
# Notes      :
# ------------------------------------------------------------------------
sub ErrorMsgTok($$) {
  my ( $prefixtxt, $tokenstr ) = @_;
  my $msg;

  my ( $token, $filename, $linenum ) = split( / /, $tokenstr );
  if ( $token eq "_EOT_" ) {
    $msg = "$filename:$linenum:$prefixtxt: ";
  }
  else {
    $msg = "$filename:$linenum:$prefixtxt near '$token': ";
  }

  return ( $msg );
}

# ------------------------------------------------------------------------
# Name       :  ErrorMsgTokIndex
# Description:  Just like ErrorMsgTok, but allows an index to be specified
# Parameters :  Prefix, token array reference, and index of offending token
# Returns    :  Error message
# Notes      :
# ------------------------------------------------------------------------
sub ErrorMsgTokIndex($$$) {
  my ( $prefixtxt, $tokendataref, $index ) = @_;
  my $msg = "";

  # Check num of tokens left
  if ( scalar( @$tokendataref ) > $index ) {
    my ( $token, $filename, $linenum ) = split( / /, $$tokendataref[ $index ] );
    if ( $token eq "_EOT_" ) {
      $msg = "$filename:$linenum:$prefixtxt: ";
    }
    else {
      $msg = "$filename:$linenum:$prefixtxt near '$token': ";
    }
  }

  return ( $msg );
}

# ------------------------------------------------------------------------
# Name       :  IsEOT
# Description:  Check if at end of token stream
# Parameters :
# Returns    :  1 if at EOT, 0 otherwise
# Notes      :
# ------------------------------------------------------------------------
sub IsEOT($) {
  my $tokendataref = shift;
  my $result       = 0;

  # Check if we are at the _EOT_ marker or not in the tokenstream
  if ( ExtractToken( $tokendataref, 0 ) eq "_EOT_" ) {
    $result = 1;
  }

  return ( $result );
}

# ------------------------------------------------------------------------
# Name       :  ParseKeywordAssignment
# Description:  Match a keyword and capture its value (only one keyword allowed
#               per syntax block)
# Parameters :
# Returns    :
# Notes      :
# ------------------------------------------------------------------------
# Parameters: keyword array ref, token data ref, base hash ref, error text ref
# Return code: -1 on match but error, 0 on no match, 1 on success
#
sub ParseKeywordAssignment($$$$) {
  my ( $keywordsref, $tokendataref, $basehashref, $errortextref ) = @_;
  my $status;

  # Check that we were passed an existing hash reference (internal error)
  if ( !defined $basehashref ) {
    die( "Undefined hash value passed in for assignment" );
  }

  do {
    {

      # Check for keyword match, bail if not
      my $keyword = ExtractToken( $tokendataref, 0 );
      if ( 0 == grep $keyword eq $_, @$keywordsref ) {
        $$errortextref =
          "No (" . join( "|", @$keywordsref ) . ") assignment found";
        $status = 0;
        last;
      }

      # Check for valid argument
      my $argument = ExtractToken( $tokendataref, 1 );
      if ( $argument !~ m/^(\w+)$/ ) {
        $status = -1;
        $$errortextref =
          ErrorMsgTokIndex( "Syntax error", $tokendataref, 1 )
          . "Illegal argument";
        last;
      }

      # Check for semicolon terminator
      if ( ExtractToken( $tokendataref, 2 ) ne ";" ) {
        $status = -1;
        $$errortextref =
          ErrorMsgTokIndex( "Syntax error", $tokendataref, 1 )
          . "Expected trailing semicolon";
        last;
      }

      # Check that we can save off to the hash without duplication
      if ( defined $basehashref->{ $keyword } ) {
        $status = -1;
        $$errortextref =
          ErrorMsgTokIndex( "Syntax error", $tokendataref, 0 )
          . "Duplicate $keyword declaration inside same block";
        last;
      }

      # Eat the tokens we just used
      ConsumeTokens( $tokendataref, 3 );

      # Add the keyword to the 'ordered keys' array
      push( @{ $basehashref->{ _KEYS } }, $keyword );

      # Save the value into the hash and declare success
      $basehashref->{ $keyword } = $argument;
      $status = 1;

      Debug( 2, "Parsed: Keyword Assignment: $keyword = $argument" );

    }
  } while ( 0 );

  # Return our status
  return ( $status );
}

# ------------------------------------------------------------------------
# Name       :  ParseKeywordArrayAssignment
# Description:  Capture keyword assignment into syntax block array
# Parameters :
# Returns    :
# Notes      :
# ------------------------------------------------------------------------
# Parameters: keyword array ref, token data ref, base hash ref, error text ref
# Return code: -1 on match but error, 0 on no match, 1 on success
#
sub ParseKeywordArrayAssignment($$$$) {
  my ( $keywordsref, $tokendataref, $basehashref, $errortextref ) = @_;
  my $status;

  # Check that we were passed an existing hash reference (internal error)
  if ( !defined $basehashref ) {
    die( "Undefined hash value passed in for assignment" );
  }

  do {
    {

      # Check for keyword match, bail if not
      my $keyword = ExtractToken( $tokendataref, 0 );
      if ( 0 == grep $keyword eq $_, @$keywordsref ) {
        $$errortextref =
          "No (" . join( "|", @$keywordsref ) . ") assignment found";
        $status = 0;
        last;
      }

      # Check for valid argument
      my $argument = ExtractToken( $tokendataref, 1 );
      if ( $argument !~ m/^(\S+)$/ ) {
        $status = -1;
        $$errortextref =
          ErrorMsgTokIndex( "Syntax error", $tokendataref, 1 )
          . "Illegal argument";
        last;
      }

      # Check for semicolon terminator
      if ( ExtractToken( $tokendataref, 2 ) ne ";" ) {
        $status = -1;
        $$errortextref =
          ErrorMsgTokIndex( "Syntax error", $tokendataref, 1 )
          . "Expected trailing semicolon";
        last;
      }

      # Eat the tokens we just used
      ConsumeTokens( $tokendataref, 3 );

      # Check if this is the first time we've seen this keyword
      if ( !defined $basehashref->{ $keyword } ) {
        # Add the keyword to the 'ordered keys' array
        push( @{ $basehashref->{ _KEYS } }, $keyword );
      }


      # Save the value into the hash and declare success
      push( @{ $basehashref->{ $keyword } }, $argument);
      $status = 1;

      Debug( 2, "Parsed: Keyword Array Assignment: $keyword = $argument" );

    }
  } while ( 0 );

  # Return our status
  return ( $status );
}

# ------------------------------------------------------------------------
# Name       :  ParseGenericName
# Description:  Treat the first token as a generic name
# Parameters :
# Returns    :
# Notes      :
# ------------------------------------------------------------------------
# Parameters: token data ref, base hash ref, error text ref
# Return code: -1 on match but error, 0 on no match, 1 on success
#
sub ParseGenericName($$$) {
  my ( $tokendataref, $basehashref, $errortextref ) = @_;
  my $status;

  # Check that we were passed an existing hash reference (internal error)
  if ( !defined $basehashref ) {
    die( "Undefined hash value passed in for assignment" );
  }

  do {
    {

      # The first token should be the variable
      my $name = ExtractToken( $tokendataref, 0 );

      # Check for semicolon terminator
      if ( ExtractToken( $tokendataref, 1 ) ne ";" ) {

        # Bail out now with a non-match, since we can't
        # declare a syntax error inside a generic name (due to no
        # precise list of keywords to compare against)
        $$errortextref = "No generic name found";
        $status        = 0;
        last;
      }

      # Check that we can save off to the hash without duplication
      if ( defined $basehashref->{ $name } ) {
        $status = -1;
        $$errortextref =
          ErrorMsgTokIndex( "Syntax error", $tokendataref, 0 )
          . "Duplicate $name declaration inside same block";
        last;
      }

      # Eat the tokens we just used
      ConsumeTokens( $tokendataref, 2 );

      # Add the variable to the 'ordered keys' array
      push( @{ $basehashref->{ _KEYS } }, $name );

      # Save the empty scalar value into the hash and declare success
      $basehashref->{ $name } = "";
      $status = 1;

      Debug( 2, "Parsed: Generic Name: $name" );

    }
  } while ( 0 );

  # Return our status
  return ( $status );
}

# ------------------------------------------------------------------------
# Name       :  ParseGenericAssignment
# Description:  Treat the first token as a keyword and the second as value
# Parameters :
# Returns    :
# Notes      :
# ------------------------------------------------------------------------
# Parameters: token data ref, base hash ref, error text ref
# Return code: -1 on match but error, 0 on no match, 1 on success
#
sub ParseGenericAssignment($$$) {
  my ( $tokendataref, $basehashref, $errortextref ) = @_;
  my $status;

  # Check that we were passed an existing hash reference (internal error)
  if ( !defined $basehashref ) {
    die( "Undefined hash value passed in for assignment" );
  }

  do {
    {

      # The first token should be the variable
      my $variable = ExtractToken( $tokendataref, 0 );

      # The second token should be the assigned value

      # Check for valid argument or semicolon
      my $argument = ExtractToken( $tokendataref, 1 );
      if ( $argument !~ m/^(\w+)$/ ) {

        # Bail out now with a non-match, since we can't
        # declare a syntax error inside a generic assigment (due to no
        # precise list of keywords to compare against)
        $$errortextref = "No generic assignment found";
        $status        = 0;
        last;
      }

      # Check for semicolon terminator (if not given above)
      if ( ExtractToken( $tokendataref, 2 ) ne ";" ) {

        # Bail out now with a non-match, since we can't
        # declare a syntax error inside a generic assigment (due to no
        # precise list of keywords to compare against)
        $$errortextref = "No generic assignment found";
        $status        = 0;
        last;
      }

      # Check that we can save off to the hash without duplication
      if ( defined $basehashref->{ $variable } ) {
        $status = -1;
        $$errortextref =
          ErrorMsgTokIndex( "Syntax error", $tokendataref, 0 )
          . "Duplicate $variable assignment inside same block";
        last;
      }

      # Eat the tokens we just used
      ConsumeTokens( $tokendataref, 3 );

      # Add the variable to the 'ordered keys' array
      push( @{ $basehashref->{ _KEYS } }, $variable );

      # Save the value into the hash and declare success
      $basehashref->{ $variable } = $argument;
      $status = 1;

      Debug( 2, "Parsed: Generic Assignment: $variable = $argument" );

    }
  } while ( 0 );

  # Return our status
  return ( $status );
}

# ------------------------------------------------------------------------
# Name       :  ParseBlock
# Description:  Parse the beginning of a block:  { ...
# Parameters :
# Returns    :
# Notes      :
# ------------------------------------------------------------------------
# Parameters: keyword array ref, token data ref, output hash ref, output hash ref,error text ref
# Return code: -1 on match but error, 0 on no match, 1 on success
#
sub ParseBlock($$$$$) {
  my ( $keywordsref, $tokendataref, $basehashref, $outhashref, $errortextref ) =
    @_;
  my $status;

  # Check that we were passed an existing hash reference (internal error)
  if ( !defined $basehashref ) {
    die( "Undefined hash value passed in for assignment" );
  }

  do {
    {

      # Check for keyword match, bail if not
      my $keyword = ExtractToken( $tokendataref, 0 );
      if ( 0 == grep $keyword eq $_, @$keywordsref ) {
        $$errortextref = "No (" . join( "|", @$keywordsref ) . ") block found";
        $status = 0;
        last;
      }

      # Check for block begin
      if ( ExtractToken( $tokendataref, 1 ) ne "{" ) {
        $status = -1;
        $$errortextref =
          ErrorMsgTokIndex( "Syntax error", $tokendataref, 1 )
          . "Expected beginning of block '{'";
      }

      # Check that we can save off to the hash without duplication
      if ( defined $basehashref->{ $keyword } ) {
        $status = -1;
        $$errortextref =
          ErrorMsgTokIndex( "Syntax error", $tokendataref, 1 )
          . "Duplicate $keyword block";
        last;
      }

      # Eat the tokens we just used
      ConsumeTokens( $tokendataref, 2 );

      # Create the hash ref,
      # copy the hash ref outward, and declare success
      $basehashref->{ $keyword } = {};
      $$outhashref               = $basehashref->{ $keyword };
      $status                    = 1;

      Debug( 2, "Parsed: Unnamed Block: $keyword" );

    }
  } while ( 0 );

  # Return our status
  return ( $status );
}

# ------------------------------------------------------------------------
# Name       :  ParseNamedBlock
# Description:  Parse the beginning of a named block:  FOO { ...
# Parameters :
# Returns    :
# Notes      :
# ------------------------------------------------------------------------
# Parameters: keyword array ref, token data ref, output hash ref,
#             output hash ref,enforce unique,error text ref
# Return code: -1 on match but error, 0 on no match, 1 on success
#
sub ParseNamedBlock($$$$$$$) {
  my (
       $keywordsref, $tokendataref,   $basehashref, $outhashref,
       $nameref,     $enforce_unique, $errortextref
       ) = @_;
  my $status;

  # Check that we were passed an existing hash reference (internal error)
  if ( !defined $basehashref ) {
    die( "Undefined hash value passed in for assignment" );
  }

  do {
    {

      # Check for keyword match, bail if not
      my $keyword = ExtractToken( $tokendataref, 0 );
      if ( 0 == grep $keyword eq $_, @$keywordsref ) {
        $status = 0;
        $$errortextref = "No (" . join( "|", @$keywordsref ) . ") block found";
        last;
      }

      # Check for valid argument
      my $argument = ExtractToken( $tokendataref, 1 );
      if ( $argument !~ m/^(\w+)$/ ) {
        $status = -1;
        $$errortextref =
          ErrorMsgTokIndex( "Syntax error", $tokendataref, 1 )
          . "Illegal argument";
        last;
      }

      # Check for block begin
      if ( ExtractToken( $tokendataref, 2 ) ne "{" ) {
        $status = -1;
        $$errortextref =
          ErrorMsgTokIndex( "Syntax error", $tokendataref, 1 )
          . "Expected beginning of block '{'";
      }

      # Save the name of the block
      $$nameref = $argument;

      # Check if uniqueness is enforced (only one of these block types per hash)
      if ( $enforce_unique
           && defined $basehashref->{ $keyword } )
      {
        $status = -1;
        $$errortextref =
          ErrorMsgTokIndex( "Syntax error", $tokendataref, 0 )
          . "Only one $keyword block allowed";
        last;
      }

      # Check that we can save off to the hash without duplication
      if ( defined $basehashref->{ $keyword }->{ $argument } ) {
        $status = -1;
        $$errortextref =
          ErrorMsgTokIndex( "Syntax error", $tokendataref, 1 )
          . "Duplicate $keyword block: $argument";
        last;
      }

      # Eat the tokens we just used
      ConsumeTokens( $tokendataref, 3 );

      # Add the argument to the 'ordered keys' array
      push( @{ $basehashref->{ $keyword }->{ _KEYS } }, $argument );

      # Create the hash ref initialized with some source file info,
      # copy the hash ref outward, and declare success
      $basehashref->{ $keyword }->{ $argument } = {
                            FILENAME   => ExtractFilename( $tokendataref,   0 ),
                            LINENUMBER => ExtractLineNumber( $tokendataref, 0 )
                            };
      $$outhashref = $basehashref->{ $keyword }->{ $argument };
      $status      = 1;

      Debug( 2, "Parsed: Named Block: $keyword = $argument" );

    }
  } while ( 0 );

  # Return our status
  return ( $status );
}

# ------------------------------------------------------------------------
# Name       :  ParseEndBlock
# Description:  Parse the end of a block:  ... }
# Parameters :
# Returns    :
# Notes      :
# ------------------------------------------------------------------------
# Parameters: token data ref
# Return code: 0 on no match, 1 on success
#
sub ParseEndBlock($$) {
  my ( $tokendataref, $errortextref ) = @_;
  my $status;

  do {
    {

      # Check for keyword match, bail if not
      my $keyword = ExtractToken( $tokendataref, 0 );
      my $tokencount = 1;
      if ( $keyword ne "}" ) {
        $status        = 0;
        $$errortextref = "No end of block found";
        last;
      }

      # Check for optional semicolon terminator
      if ( ExtractToken( $tokendataref, 1 ) eq ";" ) {
        $tokencount++;
      }

      # Eat the tokens we just used
      ConsumeTokens( $tokendataref, $tokencount );

      # Declare success
      $status = 1;

      Debug( 2, "Parsed: End of block" );

    }
  } while ( 0 );

  # Return our status
  return ( $status );
}

# ------------------------------------------------------------------------
# Name       :  ParseInputs
# Description:  Parse an STM INPUTS { ... } block
# Parameters :
# Returns    :
# Notes      :
# ------------------------------------------------------------------------
sub ParseInputs($$$) {
  my ( $tokendataref, $inputsref, $errortextref ) = @_;
  my ( $status, $inputshashref, $blocktoken );

  do {
    {

      # We're looking for start of inputs block
      $status = ParseBlock( [ "INPUTS" ],    $tokendataref, $inputsref,
                            \$inputshashref, $errortextref );

      # Bail upon unsuccessful parse
      last if ( $status != 1 );

      # Save the start of block token for some error reporting
      $blocktoken = $tokendataref->[ 0 ];

      # If the above succeeded, parse the inputs.
      # The following can be in any order, so spin until
      # we've exhausted all possibilities
      while ( 1 ) {

        # Look for an input -> transition assignment
        $status =
          ParseGenericAssignment( $tokendataref, $inputshashref,
                                  $errortextref );
        if ( $status == -1 ) {
          last;
        }
        elsif ( $status == 1 ) {
          next;
        }

        # Look for an bare input assignment (ie. ignore this input)
        $status =
          ParseGenericName( $tokendataref, $inputshashref, $errortextref );
        if ( $status == -1 ) {
          last;
        }
        elsif ( $status == 1 ) {
          next;
        }

        # Look for an end block
        $status = ParseEndBlock( $tokendataref, $errortextref );
        if ( $status == 1 ) {
          last;
        }

        # Check if we have exhausted the token stream
        if ( IsEOT( $tokendataref ) ) {
          $status = -1;
          $$errortextref =
            ErrorMsgTokIndex( "End of file error", $tokendataref, 0 )
            . "Unterminated inputs block";
          last;
        }

        # If we get here, it's invalid syntax (for this context)
        $status = -1;
        $$errortextref =
          ErrorMsgTokIndex( "Syntax error", $tokendataref, 0 )
          . "Invalid syntax";
        last;
      }

    }
  } while ( 0 );

  # If we successfully parsed an INPUTS block, check that there are
  # > 0 entries in it.  Fail if not.
  if ( $status == 1 ) {
    if ( !defined $inputshashref->{ _KEYS } ) {
      $status = -1;
      $$errortextref =
        ErrorMsgTok( "Syntax error", $blocktoken ) . "Empty INPUTS block";
    }
  }

  # Let the caller know how things went
  return ( $status );
}

# ------------------------------------------------------------------------
# Name       :  ParseState
# Description:  Parse a STATE FOO { ... } STM block
# Parameters :
# Returns    :
# Notes      :
# ------------------------------------------------------------------------
sub ParseState($$$) {
  my ( $tokendataref, $stateref, $errortextref ) = @_;
  my ( $status, $statehashref, $statename, $blocktoken );

  do {
    {

      # We're looking for start of state block
      $status = ParseNamedBlock( [ "STATE" ],
                                 $tokendataref, $stateref, \$statehashref,
                                 \$statename, 0, $errortextref );

      # Bail upon unsuccessful parse
      last if ( $status != 1 );

      # Save the start of block token for some error reporting
      $blocktoken = $tokendataref->[ 0 ];

      # If the above succeeded, parse the state's contents.
      # The following can be in any order, so spin until
      # we've exhausted all possibilities
      while ( 1 ) {

        # Look for an entry/exit function definition
        $status = ParseKeywordAssignment( [ "ENTRY", "EXIT" ],
                                  $tokendataref, $statehashref, $errortextref );
        if ( $status == -1 ) {
          last;
        }
        elsif ( $status == 1 ) {
          next;
        }

        # Look for an inputs section
        $status = ParseInputs( $tokendataref, $statehashref, $errortextref );
        if ( $status == -1 ) {
          last;
        }
        elsif ( $status == 1 ) {
          next;
        }

        # Look for a sub-state machine
        $status = ParseSM( $tokendataref, $statehashref, $errortextref, 1 );
        if ( $status == -1 ) {
          last;
        }
        elsif ( $status == 1 ) {
          # Put a name derived from this state on the child SM
          $statehashref->{ STATE_MACHINE }->{ NAME } = "_" . $statename . "_SM";
          next;
        }

        # Look for an end block
        $status = ParseEndBlock( $tokendataref, $errortextref );
        if ( $status == 1 ) {
          last;
        }

        # Check if we have exhausted the token stream
        if ( IsEOT( $tokendataref ) ) {
          $status = -1;
          $$errortextref =
            ErrorMsgTokIndex( "End of file error", $tokendataref, 0 )
            . "Unterminated state block";
          last;
        }

        # If we get here, it's invalid syntax (for this context)
        $status = -1;
        $$errortextref =
          ErrorMsgTokIndex( "Syntax error", $tokendataref, 0 )
          . "Invalid syntax";
        last;
      }

    }
  } while ( 0 );

  # If we successfully parsed a STATE block, check a number of criteria
  if ( $status == 1 ) {

    # Make sure we had an INPUTS block
    if ( !defined $statehashref->{ INPUTS } ) {
      $status = -1;
      $$errortextref =
        ErrorMsgTok( "Syntax error", $blocktoken )
        . "Missing INPUTS block in state: $statename";
    }

    # Emit warnings for defaulted NULL ENTRY/EXIT functions
    if ( !defined $statehashref->{ ENTRY } ) {
      $statehashref->{ ENTRY } = "NULL";
      Debug( 0, "Warning: State: $statename, entry function implicitly NULL" );

    }
    if ( !defined $statehashref->{ EXIT } ) {
      $statehashref->{ EXIT } = "NULL";
      Debug( 0, "Warning: State: $statename, exit function implicitly NULL" );
    }
  }

  # Let the caller know how things went
  return ( $status );
}

# ------------------------------------------------------------------------
# Name       :  ParseSM
# Description:  Parse a named or unnamed STM STATE_MACHINE block
# Parameters :
# Returns    :
# Notes      :
# ------------------------------------------------------------------------
sub ParseSM($$$$) {
  my ( $tokendataref, $smref, $errortextref, $subsm ) = @_;
  my ( $status, $smhashref, $smname, $blocktoken, $keywords, $arrkeywords );

  do {
    {
      # Sub-SM's use an anonymous block, top-level SM's use a named block
      if ($subsm == 0) {
        # We're looking for start of a named state machine block
        $status = ParseNamedBlock( [ "STATE_MACHINE" ],
                                   $tokendataref, $smref,
                                   \$smhashref, \$smname,
                                   0, $errortextref );

        $keywords = [ "ENTRY", "EXIT", "INITIAL_STATE",
                      "INSTANCES", "ERROR_HOOK", "DEBUG_HOOK" ];
        $arrkeywords = [ "INPUT_DEF_FILE" ];
      } else {
        # We're looking for start of an anonymous state machine block
        $smname = "Anonymous";
        $status = ParseBlock( [ "STATE_MACHINE" ], $tokendataref, $smref,
                                \$smhashref, $errortextref );
        $keywords = [ "ENTRY", "EXIT", "INITIAL_STATE",
                      "ERROR_HOOK", "DEBUG_HOOK" ];
      }

      # Bail upon unsuccessful parse
      last if ( $status != 1 );

      # Save the start of block token for some error reporting
      $blocktoken = $tokendataref->[ 0 ];

      # If the above succeeded, parse the state machine's contents.
      # The following can be in any order, so spin until
      # we've exhausted all possibilities
      while ( 1 ) {

        # Look for an entry, exit, initial_state, instances, error_hook, or
        # debug_hook definition
        $status =
          ParseKeywordAssignment( $keywords, $tokendataref,
                                  $smhashref, $errortextref );
        if ( $status == -1 ) {
          last;
        }
        elsif ( $status == 1 ) {
          next;
        }

        # Input defn file is only valid in top level sm
        if ($subsm == 0) {
          # Look for an input definition file/s definition
          $status =
            ParseKeywordArrayAssignment( $arrkeywords, $tokendataref,
                                         $smhashref, $errortextref );
          if ( $status == -1 ) {
            last;
          }
          elsif ( $status == 1 ) {
            next;
          }
        }

        # Look for a STATE block
        $status = ParseState( $tokendataref, $smhashref, $errortextref );
        if ( $status == -1 ) {
          last;
        }
        elsif ( $status == 1 ) {
          next;
        }

        # Look for an end block
        $status = ParseEndBlock( $tokendataref, $errortextref );
        if ( $status == 1 ) {
          last;
        }

        # Check if we have exhausted the token stream
        if ( IsEOT( $tokendataref ) ) {
          $status = -1;
          $$errortextref =
            ErrorMsgTokIndex( "End of file error", $tokendataref, 0 )
            . "Unterminated state machine block";
          last;
        }

        # If we get here, it's invalid syntax (for this context)
        $status = -1;
        $$errortextref =
          ErrorMsgTokIndex( "Syntax error", $tokendataref, 0 )
          . "Invalid syntax";
        last;
      }

    }
  } while ( 0 );

  # If we successfully parsed a STATE_MACHINE block, check a number of criteria
  if ( $status == 1 ) {

    # clear the error string
    $$errortextref = "";

    # Make sure we had at least one STATE block
    if ( !defined $smhashref->{ STATE }->{ _KEYS } ) {
      $status = -1;
      $$errortextref .=
        ErrorMsgTok( "Syntax error", $blocktoken )
        . "No STATE blocks in state machine: $smname\n";
    }

    # We need at least one input definition file in the top level sm
    if ( ($subsm == 0) && !defined $smhashref->{ INPUT_DEF_FILE } ) {
      $status = -1;
      $$errortextref .=
        ErrorMsgTok( "Syntax error", $blocktoken )
        . "No INPUT_DEF_FILE specified in state machine: $smname\n";
    }

    # Make sure INITIAL_STATE is specified and valid
    if ( !defined $smhashref->{ INITIAL_STATE } ) {
      $status = -1;
      $$errortextref .=
        ErrorMsgTok( "Syntax error", $blocktoken )
        . "No INITIAL_STATE specified in state machine: $smname\n";
    }
    elsif ( !defined $smhashref->{ STATE }->{ $smhashref->{ INITIAL_STATE } } )
    {
      $status = -1;
      $$errortextref .=
        ErrorMsgTok( "Syntax error", $blocktoken )
        . "Invalid initial state: $smhashref->{INITIAL_STATE} "
        . "specified in state machine: $smname\n";
    }

    # In a top-level SM, if INSTANCES is not specified, validate,
    # otherwise default it to 1
    if ($subsm == 0) {
      if ( !defined $smhashref->{ INSTANCES } ) {
        $smhashref->{ INSTANCES } = "1";
      }
      elsif ( $smhashref->{ INSTANCES } !~ m/^\d+$/ )
      {
        $status = -1;
        $$errortextref .=
          ErrorMsgTok( "Syntax error", $blocktoken )
          . "Non-numeric instances specifier: $smhashref->{INSTANCES} "
          . "specified in state machine: $smname\n";
      }
    }

    # Clean any trailing newlines
    chomp( $$errortextref );

    # Emit warnings for defaulted NULL ENTRY/EXIT/ERROR_HOOK/DEBUG_HOOK fns
    if ( !defined $smhashref->{ ENTRY } ) {
      $smhashref->{ ENTRY } = "NULL";
      Debug( 0,
            "Warning: State machine: $smname, entry function implicitly NULL" );

    }
    if ( !defined $smhashref->{ EXIT } ) {
      $smhashref->{ EXIT } = "NULL";
      Debug( 0,
             "Warning: State machine: $smname, exit function implicitly NULL" );
    }
    if ( !$subsm && !defined $smhashref->{ ERROR_HOOK } ) {
      $smhashref->{ ERROR_HOOK } = "NULL";
      Debug( 0,
             "Warning: State machine: $smname, error hook function implicitly NULL" );
    }
    if ( !$subsm && !defined $smhashref->{ DEBUG_HOOK } ) {
      $smhashref->{ DEBUG_HOOK } = "NULL";
      Debug( 0,
             "Warning: State machine: $smname, debug hook function implicitly NULL" );
    }

  }

  # Let the caller know how things went
  return ( $status );
}

# ------------------------------------------------------------------------
# Name       :  GenerateSMCode
# Description:  Generate STM data structure code based upon STM definition
# Parameters :
# Returns    :
# Notes      :
# ------------------------------------------------------------------------
sub GenerateSMCode($$$$$);
sub GenerateSMCode($$$$$) {
  my ( $SMhashref, $SMprefix, $parentSMhashref, $errortextref, $nestlvl ) = @_;

  # Check that we were passed an existing hash reference (internal error)
  if ( !defined $SMhashref ) {
    die( "Undefined hash value passed in for assignment" );
  }

  my $status = 0;

  # Initialize the internal & external header & source
  $SMhashref->{ IntHeader } = "";
  $SMhashref->{ ExtHeader } = "";
  $SMhashref->{ IntSource } = "";

  # Build source for state machine source header comment here
  $SMhashref->{ SkelSource } .=  <<DONE;


/*===========================================================================

                 STATE MACHINE: $SMprefix

===========================================================================*/

DONE

  # Initialize our set of reset functions
  $SMhashref->{ ResetFns } = "";

  # Create the internal and external state enums
  my $stateenum = "enum\n{\n";
  my $ext_stateenum = "enum\n{\n";
  foreach my $statename ( @{ $SMhashref->{ STATE }->{ _KEYS } } ) {
    $stateenum .= "  $statename,\n";
    $ext_stateenum .= "  ${SMprefix}__${statename},\n";
    # put it here
  }
  $stateenum .= "};";
  $ext_stateenum .= "};";

  # Get the number of states
  my $numstates       = scalar( @{ $SMhashref->{ STATE }->{ _KEYS } } );

  # Build the SM entry exit prototypes
  my $SMentryexitprotos = "";

  # Get the SM entry fn name (defaults to NULL)
  my $SMentry = $SMhashref->{ ENTRY };
  if ( $SMhashref->{ ENTRY } ne "NULL" )
  {
    $SMentryexitprotos .= "void ${SMentry}(stm_state_machine_t *sm,void *payload);\n";
    # Build source for state machine entry fn
    $SMhashref->{ SkelSource } .=  <<DONE;
/*===========================================================================

  STATE MACHINE ENTRY FUNCTION:  $SMentry

===========================================================================*/
/*!
    \@brief
    Entry function for state machine $SMprefix

    \@detail
    Called upon activation of this state machine, with optional
    user-passed payload pointer parameter.

    \@return
    None

*/
/*=========================================================================*/
void $SMentry
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{

  //! \@todo Variable declarations go here

  STM_UNUSED( payload );

  /*-----------------------------------------------------------------------*/

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );

  /*-----------------------------------------------------------------------*/

  //! \@todo Code goes here

} /* $SMentry() */


DONE
  }

  # Get the SM exit fn name (defaults to NULL)
  my $SMexit = $SMhashref->{ EXIT };
  if ( $SMhashref->{ EXIT } ne "NULL" )
  {
    $SMentryexitprotos .= "void ${SMexit}(stm_state_machine_t *sm,void *payload);\n";
    # Build source for state machine entry fn
    $SMhashref->{ SkelSource } .=  <<DONE;
/*===========================================================================

  STATE MACHINE EXIT FUNCTION:  $SMexit

===========================================================================*/
/*!
    \@brief
    Exit function for state machine $SMprefix

    \@detail
    Called upon deactivation of this state machine, with optional
    user-passed payload pointer parameter.

    \@return
    None

*/
/*=========================================================================*/
void $SMexit
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{

  //! \@todo Variable declarations go here

  STM_UNUSED( payload );

  /*-----------------------------------------------------------------------*/

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );

  /*-----------------------------------------------------------------------*/

  //! \@todo Code goes here

} /* $SMexit() */


DONE
  }

  # Get the SM error fn name (defaults to NULL if top-level, otherwise parent's)
  if (!defined $SMhashref->{ ERROR_HOOK } &&
      defined $parentSMhashref->{ ERROR_HOOK }) {
    $SMhashref->{ ERROR_HOOK } = $parentSMhashref->{ ERROR_HOOK };
  }
  my $SMerror = $SMhashref->{ ERROR_HOOK };

  # Get the SM debug fn name (defaults to NULL if top-level, otherwise parent's)
  if (!defined $SMhashref->{ DEBUG_HOOK } &&
      defined $parentSMhashref->{ DEBUG_HOOK }) {
    $SMhashref->{ DEBUG_HOOK } = $parentSMhashref->{ DEBUG_HOOK };
  }
  my $SMdebug = $SMhashref->{ DEBUG_HOOK };

  # Get the initial state
  my $SMinitstate = $SMhashref->{ INITIAL_STATE };

  # Create the #include statements necessary
  my $Cincludes = "";
  foreach (@{ $SMhashref->{ INPUT_DEF_FILE } }) {
    $Cincludes .= "#include <$_>$/";
  }

  # Get the number of instances (if not top-level, use parent's)
  if (!defined $SMhashref->{ INSTANCES } &&
      defined $parentSMhashref->{ INSTANCES }) {
    $SMhashref->{ INSTANCES } = $parentSMhashref->{ INSTANCES };
  }
  my $numinstances = $SMhashref->{ INSTANCES };

  # Create the state name / entry / exit table
  # and create unique lists of all our inputs, transition functions, and
  # state entry/exit fns
  my $statetable = "static const stm_state_map_t\n";
  $statetable .= "  ${SMprefix}_states[ ${SMprefix}_NUM_STATES ] =\n{\n";
  my @allinputs            = ();
  my @alltransfns          = ();
  my @allstateentryexitfns = ();

  # Some hashes to figure out uniqueness
  my %allinputshash            = ();
  my %alltransfnshash          = ();
  my %allstateentryexitfnshash = ();
  foreach my $statename ( @{ $SMhashref->{ STATE }->{ _KEYS } } ) {
    # Build source comment header for state here
    $SMhashref->{ SkelSource } .=  <<DONE;
/*===========================================================================

     (State Machine: $SMprefix)
     STATE ENTRY/EXIT/TRANSITION FUNCTIONS: $statename

===========================================================================*/

DONE

    # Get the state's subhash
    my $state = $SMhashref->{ STATE }->{ $statename };

    # Get the entry fn name
    my $stateentry = $state->{ ENTRY };
    if ( $stateentry ne "NULL" )
    {
      # Store the function if unique, and mark it as stored
      if (!defined $allstateentryexitfnshash{ $stateentry }) {
        push( @allstateentryexitfns, $stateentry );
        # Build source for entry fn here
        $SMhashref->{ SkelSource } .=  <<DONE;
/*===========================================================================

  STATE ENTRY FUNCTION:  $stateentry

===========================================================================*/
/*!
    \@brief
    Entry function for state machine $SMprefix,
    state $statename

    \@detail
    Called upon entry into this state of the state machine, with optional
    user-passed payload pointer parameter.  The prior state of the state
    machine is also passed as the prev_state parameter.

    \@return
    None

*/
/*=========================================================================*/
void $stateentry
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         prev_state,  /*!< Previous state */
  void                *payload     /*!< Payload pointer */
)
{

  //! \@todo Variable declarations go here

  STM_UNUSED( payload );
  STM_UNUSED( prev_state );

  /*-----------------------------------------------------------------------*/

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );

  /*-----------------------------------------------------------------------*/

  //! \@todo Code goes here

} /* $stateentry() */


DONE
      }
      $allstateentryexitfnshash{ $stateentry }++;
    }

    # Get the exit fn name (defaults to NULL)
    my $stateexit = $state->{ EXIT };
    if ( $state->{ EXIT } ne "NULL" )
    {
      # Store the function if unique, and mark it as stored
      if (!defined $allstateentryexitfnshash{ $stateexit }) {
        push( @allstateentryexitfns, $stateexit );
        # Build source for exit fn here
        $SMhashref->{ SkelSource } .=  <<DONE;
/*===========================================================================

  STATE EXIT FUNCTION:  $stateexit

===========================================================================*/
/*!
    \@brief
    Exit function for state machine $SMprefix,
    state $statename

    \@detail
    Called upon exit of this state of the state machine, with optional
    user-passed payload pointer parameter.  The impending state of the state
    machine is also passed as the next_state parameter.

    \@return
    None

*/
/*=========================================================================*/
void $stateexit
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         next_state,  /*!< Next state */
  void                *payload     /*!< Payload pointer */
)
{

  //! \@todo Variable declarations go here

  STM_UNUSED( payload );
  STM_UNUSED( next_state );

  /*-----------------------------------------------------------------------*/

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );

  /*-----------------------------------------------------------------------*/

  //! \@todo Code goes here

} /* $stateexit() */


DONE
      }
      $allstateentryexitfnshash{ $stateexit }++;
    }

    # While we're in here, accumulate the total set of unique inputs
    # and transition functions
    foreach my $inputname ( @{ $state->{ INPUTS }->{ _KEYS } } ) {

      # Store the input if unique, and mark it as stored
      push( @allinputs, $inputname )
        unless defined $allinputshash{ $inputname };
      $allinputshash{ $inputname }++;

      # Store the transition fn if unique, and mark it as stored
      if ( defined( $state->{ INPUTS }->{ $inputname } ) &&
           ( $state->{ INPUTS }->{ $inputname } ne "" ) ) {

        my $transfnname = $state->{ INPUTS }->{ $inputname };
        # Now check for uniqueness and store
        if (!defined $alltransfnshash{ $transfnname }) {
          push( @alltransfns, $transfnname );
          # TODO build source for trans fn here
          $SMhashref->{ SkelSource } .=  <<DONE;
/*===========================================================================

  TRANSITION FUNCTION:  $transfnname

===========================================================================*/
/*!
    \@brief
    Transition function for state machine $SMprefix,
    state $statename,
    upon receiving input $inputname

    \@detail
    Called upon receipt of input $inputname, with optional
    user-passed payload pointer.

    \@return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t $transfnname
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */

  //! \@todo Additional variable declarations go here

  STM_UNUSED( payload );

  /*-----------------------------------------------------------------------*/

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );

  /*-----------------------------------------------------------------------*/

  //! \@todo Code goes here

  return( next_state );

} /* $transfnname() */


DONE
        }
        $alltransfnshash{ $state->{ INPUTS }->{ $inputname } }++;
      }
    }

    # Get the state's child state machine, if it exists
    my $statechildsm = "NULL";
    if ( defined $state->{ STATE_MACHINE }) {
      $statechildsm = $state->{ STATE_MACHINE }->{ NAME };

      # If the child state machine name is based upon solely it's state name,
      # prefix the parent SM name to it, followed by an underscore.  This
      # allows the child SM naming hierarchy to be visually explicit.
      if ( $statechildsm =~ m/_\w+/ ) {
        $statechildsm = $SMprefix . "_" . $statechildsm;
      }

      # TODO build source comment for child sm here
      $SMhashref->{ SkelSource } .=  <<DONE;
/*===========================================================================

     (State Machine: $SMprefix, State: $statename)
     CHILD STATE MACHINE

===========================================================================*/

DONE
      # I suppose this makes as good a place as any to recurse
      $status = GenerateSMCode( $state->{ STATE_MACHINE },$statechildsm,
                                $SMhashref, $errortextref, $nestlvl + 1 );
      if ($status != 0) {
        $$errortextref .= "Failed to generate code for sub-SM";
        return($status)
      }
      # Roll the child SM's C code up into the parent (not ext H code)
      $SMhashref->{ IntHeader } .= $state->{ STATE_MACHINE }->{ IntHeader };
      $SMhashref->{ IntSource } .= $state->{ STATE_MACHINE }->{ IntSource };
      $SMhashref->{ SkelSource } .= $state->{ STATE_MACHINE }->{ SkelSource };
      # Keep track of the child SM's reset function, so the parent can call it
      # if its reset handler is called
      $SMhashref->{ ResetFns } .= $state->{ STATE_MACHINE }->{ ResetFns };
    }

    # Add our line of source
    $statetable .= <<DONE;
  {\"$statename\",
#ifndef STM_DATA_STRUCTURES_ONLY
    $stateentry, $stateexit,
#else /* STM_DATA_STRUCTURES_ONLY */
    NULL, NULL,
#endif /* STM_DATA_STRUCTURES_ONLY */
    $statechildsm},
DONE
  }
  $statetable .= "};";

  # Create the list of prototypes for the transition functions
  my $transfnprotos = "";
  foreach my $transfn ( @alltransfns ) {
    $transfnprotos .= "stm_state_t ${transfn}(stm_state_machine_t *sm, void *payload);\n";
  }

  # Create the list of prototypes for state entry/exit functions
  my $stateentryexitprotos = "";
  foreach my $entryexitfn ( @allstateentryexitfns ) {
    $stateentryexitprotos .=
      "void ${entryexitfn}(stm_state_machine_t *sm,stm_state_t _state,void *payload);\n";
  }

  # Get the number of unique inputs
  my $numinputs       = scalar( @allinputs );

  # Create the input / input name table
  my $inputtable = "static const stm_input_map_t\n";
  $inputtable .= "  ${SMprefix}_inputs[ ${SMprefix}_NUM_INPUTS ] =\n{\n";
  foreach my $inputname ( @allinputs ) {
    $inputtable .= "  { \"$inputname\" , (stm_input_t) $inputname },\n";
  }
  $inputtable .= "};";

  # Create the transition table
  my $transtable = "static const stm_transition_fn_t\n";
  $transtable .= "  ${SMprefix}_transitions[ ${SMprefix}_NUM_STATES * ${SMprefix}_NUM_INPUTS ] =\n{\n";
  foreach my $statename ( @{ $SMhashref->{ STATE }->{ _KEYS } } ) {

    # Get the state's subhash
    my $state = $SMhashref->{ STATE }->{ $statename };

    # Add a comment
    $transtable .= "  /* Transition functions for state $statename */\n";

    # Spin through all possible inputs
    foreach my $inputname ( @allinputs ) {
      if ( !defined $state->{ INPUTS }->{ $inputname } ||
           ( $state->{ INPUTS }->{ $inputname } eq "" ) ) {
        $transtable .= "  NULL,";
      } else {
        $transtable .= "  $state->{INPUTS}->{$inputname},";
      }
      $transtable .= "    /* $inputname */\n";
    }
    $transtable .= "\n";
  }
  $transtable .= "};";

  # Create the reset function prototype. 
  my $reset_fn = "void ${SMprefix}_reset(void)";

  # Build the external source
  $SMhashref->{ ExtHeader } .= <<DONE;
/* Begin machine generated code for state machine array: ${SMprefix}\[\] */

/* Define a macro for the number of SM instances */
#define ${SMprefix}_NUM_INSTANCES $numinstances

/* External reference to state machine structure */
extern stm_state_machine_t ${SMprefix}\[ ${SMprefix}_NUM_INSTANCES \];

/* External enumeration representing state machine's states */
$ext_stateenum

#ifndef STM_DATA_STRUCTURES_ONLY
/* User called 'reset' routine.  Should never be needed, but can be used to
   effect a complete reset of all a given state machine's instances. */
extern $reset_fn;
#endif /* !STM_DATA_STRUCTURES_ONLY */

/* End machine generated code for state machine array: ${SMprefix}\[\] */
DONE

  # Build the internal header
  $SMhashref->{ IntHeader } .= <<DONE;
/* Begin machine generated internal header for state machine array: ${SMprefix}\[\] */

/* Suppress Lint suggestions to const-ify state machine and payload ptrs */
/*lint -esym(818,sm,payload) */

/* Define a macro for the number of SM instances */
#define ${SMprefix}_NUM_INSTANCES $numinstances

/* Define a macro for the number of SM states */
#define ${SMprefix}_NUM_STATES $numstates

/* Define a macro for the number of SM inputs */
#define ${SMprefix}_NUM_INPUTS $numinputs

#ifndef STM_DATA_STRUCTURES_ONLY
/* State Machine entry/exit function prototypes */
$SMentryexitprotos

/* State entry/exit function prototypes */
$stateentryexitprotos

/* Transition function prototypes */
$transfnprotos

/* State enumeration */
$stateenum

#endif /* !STM_DATA_STRUCTURES_ONLY */

/* End machine generated internal header for state machine array: ${SMprefix}\[\] */
DONE

  # Build the internal source
  $SMhashref->{ IntSource } .= <<DONE;
/* Begin machine generated internal source for state machine array: ${SMprefix}\[\] */

#ifndef STM_DATA_STRUCTURES_ONLY
/* Transition table */
$transtable
#endif /* !STM_DATA_STRUCTURES_ONLY */

/* State { name, entry, exit, child SM } table */
$statetable

/* Input { name, value } table */
$inputtable


/* Constant all-instance state machine data */
static const stm_state_machine_constdata_t ${SMprefix}_constdata =
{
  ${SMprefix}_NUM_INSTANCES, /* number of state machine instances */
  ${SMprefix}_NUM_STATES, /* number of states */
  ${SMprefix}_states, /* array of state mappings */
  ${SMprefix}_NUM_INPUTS, /* number of inputs */
  ${SMprefix}_inputs, /* array of input mappings */
#ifndef STM_DATA_STRUCTURES_ONLY
  ${SMprefix}_transitions, /* array of transition function mappings */
  $SMentry, /* state machine entry function */
  $SMexit, /* state machine exit function */
  $SMerror, /* state machine error hook function */
  $SMdebug, /* state machine debug hook function */
  $SMinitstate /* state machine initial state */
#else /* STM_DATA_STRUCTURES_ONLY */
  NULL, /* array of transition function mappings */
  NULL, /* state machine entry function */
  NULL, /* state machine exit function */
  NULL, /* state machine error hook function */
  NULL, /* state machine debug hook function */
  0 /* state machine initial state */
#endif /* STM_DATA_STRUCTURES_ONLY */
};

/* Constant per-instance state machine data */
static const stm_state_machine_perinst_constdata_t
  ${SMprefix}_perinst_constdata\[ ${SMprefix}_NUM_INSTANCES \] =
{
DONE

  # Create the array of SM instances
  for (my $thisinstance = 0; $thisinstance < $numinstances; $thisinstance++) {

    # Bracket the instance if numinstances > 1
    my $thisSMname = "${SMprefix}";
    if ($numinstances > 1) {
      $thisSMname .= "[$thisinstance]";
    }

    # Generate the MD5 digest of the name and take the lower-32 bits
    my $thisSMuid = "0x" . substr(md5_hex($thisSMname),-8,8);

    $SMhashref->{ IntSource } .= <<DONE;
  {
    \&${SMprefix}_constdata, /* state machine constant data */
    "$thisSMname", /* state machine name */
    $thisSMuid, /* state machine unique ID (md5("$thisSMname") & 0xFFFFFFFF) */
    $thisinstance  /* this state machine instance */
  },
DONE
  }

  $SMhashref->{ IntSource } .= <<DONE;
};

DONE

  $SMhashref->{ IntSource } .= <<DONE;
/* State machine instance array definition */
stm_state_machine_t
  $SMprefix\[ ${SMprefix}_NUM_INSTANCES \] =
{
DONE

  # Create the array of SM instances
  for (my $thisinstance = 0; $thisinstance < $numinstances; $thisinstance++) {

  # Bracket the instance if numinstances > 1
  my $thisSMname = "${SMprefix}";
  if ($numinstances > 1) {
    $thisSMname .= "[$thisinstance]";
  }

  $SMhashref->{ IntSource } .= <<DONE;
  {
    \&${SMprefix}_perinst_constdata\[ $thisinstance ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
DONE
  }

  #Build the reset function.  Make it static if we are not the top-level SM
  if ($nestlvl > 0) {
    $reset_fn = "static " . $reset_fn;
  }
  $SMhashref->{ IntSource } .= <<DONE;
};

#ifndef STM_DATA_STRUCTURES_ONLY
/* User called 'reset' routine.  Should never be needed, but can be used to
   effect a complete reset of all a given state machine's instances. */
$reset_fn
{
  uint32 idx;
  void **tricky;

  /* Reset all the child SMs (if any) */
  $SMhashref->{ ResetFns }

  /* Reset the parent */
  for( idx = 0; idx < ${SMprefix}_NUM_INSTANCES; idx++)
  {
    tricky = (void **)&${SMprefix}\[ idx ].pi_const_data; /* sleight of hand to assign to const ptr below */
    *tricky = (void *)\&${SMprefix}_perinst_constdata\[ idx ]; /* per instance constant data array */
    ${SMprefix}\[ idx ].current_state = STM_DEACTIVATED_STATE; /* current state */
    ${SMprefix}\[ idx ].curr_input_index = -1; /* current input index */
    ${SMprefix}\[ idx ].propagate_input = FALSE; /* propagate input to parent */
    ${SMprefix}\[ idx ].is_locked = FALSE; /* locked flag */
    ${SMprefix}\[ idx ].user_data = NULL; /* user defined per-instance data */
    ${SMprefix}\[ idx ].debug_mask = 0; /* user defined debug mask */
  }

}
#endif /* !STM_DATA_STRUCTURES_ONLY */

/* End machine generated internal source for state machine array: ${SMprefix}\[\] */
DONE

  # Put this at the bottom, so the toplevel guy doesn't call his own reset fn
  $SMhashref->{ ResetFns } .= <<DONE;
${SMprefix}_reset\(\)\;
DONE

  # Include file defns should only exist in the top-level SM
  # Tack them onto the top of the C source if so
  if ($Cincludes ne "") {
    $SMhashref->{ IntSource } =
      "/* Include INPUT_DEF_FILE specified files */\n" .
      $Cincludes .
      "\n" .
      $SMhashref->{ IntSource };
  }

  # Return with what we've got
  return ( $status );
}


# ------------------------------------------------------------------------
# Name       : abort_handler
# Description: This subroutine is run when an INT signal is caught.  It is
#              responsible for cleaning up any generated files.
# Parameters :
# Returns    :
# Notes      :
# ------------------------------------------------------------------------
# Print out usage and exit with passed-in status.
sub abort_handler {
  # Let the user know we were interrupted
  Debug(0,"SIGINT received!");

  # Let curious users know what we're trying to delete
  if (scalar(@files_created) > 0) {
    Debug(1,"Deleting generated file/s: " . join " ",@files_created);
    unlink @files_created;
  }
}


# ------------------------------------------------------------------------
# Name       :  Usage
# Description:  Print usage info
# Parameters :
# Returns    :
# Notes      :
# ------------------------------------------------------------------------
# Print out usage and exit with passed-in status.
sub Usage($) {
  my $status = shift;
  my ( $prgname ) = ( $0 =~ m|([^/\\]+)$| );
  print STDERR <<EOF;

Usage: $prgname [-h] [-v] -oi <filename> [-oc <filename>] [-oe <filename>] [-os <filename>] [input filename]
State Machine Compiler, version 2.5
Copyright (c) Qualcomm, 2007- $currentyear

Options:
  -h | -? | --help : Display this information
  -v | -v -v | ... : Verbosity/debugging level
  -oi filename     : Fully pathed output 'internal header' filename
  -oc filename     : Fully pathed output 'internal source' filename
  -oe filename     : Fully pathed output 'external header' filename
  -os filename     : Fully pathed output C state machine skeleton filename

Examples:
  1) To generate both internal and external source from an STM file, generally
     used in a Makefile or build script:
     $prgname -oi __foo_sm_int.h  -oc __foo_sm_int.c  -oe __foo_sm.h   foo_sm.stm
  2) To generate just the internal header from an STM file, in the case that
     no external entities need to operate on the state machine:
     $prgname -oi __bar_sm_int.h  -oc __bar_sm_int.c   bar_sm.stm
  3) "Development mode", where you want to generate a skeleton C state machine
     files (entry, exit, and transition functions) based upon an STM file:
     $prgname -oi __my_sm_int.h  -oc __my_sm_int.c   -os my_sm.c   my_sm.stm
     ** NOTE - the internal source and header files are also generated, and a #include is
     built automatically into the my_sm.c file. **

Notes:
  If input filename is omitted, input is taken from STDIN.
  Error and debug information is directed to STDERR.
  Exit code of 0 indicates success.

EOF
  exit( $status );
}

# ------------------------------------------------------------------------
# Name       :
# Description:
# Parameters :
# Returns    :
# Notes      :
# ------------------------------------------------------------------------
# Main routine
# Called upon program startup to parse args and do stuff
{
  # Set up a signal handler to catch Ctrl-C
  $SIG{INT} = \&abort_handler;

  # Cmd line option vars
  my $inth_fpname = "";
  my $intc_fpname = "";
  my $exth_fpname = "";
  my $skel_fpname = "";

  # Parse the command-line options, printing usage upon failure
  GetOptions(
              "help|h|?"      => sub { Usage( 0 ) },
              "inthfilename|oi=s"  => \$inth_fpname,
              "intcfilename|oc=s"  => \$intc_fpname,
              "exthfilename|oe=s"  => \$exth_fpname,
              "skelfilename|os=s"  => \$skel_fpname,
              "v+"                 => \$debuglevel
              ) || Usage( 1 );

  # Check what's left in ARGV, should be 0 or 1 'filename'
  # Bail if > 1 filename specified
  if ( scalar( @ARGV ) > 1 ) {
    Debug( 0, "Greater than one input filename argument!" );
    Usage( 1 );
  }
  elsif ( scalar( @ARGV ) == 0 ) {

    # No filename argument, make it "-" for STDIN
    push( @ARGV, "-" );
  }

  # Separate the optional path prefix from the filename
  my ($inth_pname,$inth_fname) = $inth_fpname =~ m|^(.*[/\\])?([^/\\]+)$|;
  my ($intc_pname,$intc_fname) = $intc_fpname =~ m|^(.*[/\\])?([^/\\]+)$|;
  my ($exth_pname,$exth_fname) = $exth_fpname =~ m|^(.*[/\\])?([^/\\]+)$|;
  my ($skel_pname,$skel_fname) = $skel_fpname =~ m|^(.*[/\\])?([^/\\]+)$|;

  # Ensure that at least the internal output filename was given
  if (!defined $inth_fname) {
    Debug( -1, "No output filename for internal header provided");
  }

  # Get a basename for the skeleton C file (strip off extension)
  my ($skel_bname) = $skel_fname =~ m|^(.*)[.].*$| unless ($skel_fpname eq "");

  # Ensure that the output directories exist, if specified
  if (defined $inth_pname && !(-d $inth_pname )) {
    Debug( -1, "Output path for internal header file does not exist: $inth_pname");
  }
  if (defined $intc_pname && !(-d $inth_pname )) {
    Debug( -1, "Output path for internal header file does not exist: $inth_pname");
  }
  if (defined $exth_pname && !(-d $exth_pname )) {
    Debug( -1, "Output path for external header file does not exist: $exth_pname");
  }
  if (defined $skel_pname && !(-d $skel_pname )) {
    Debug( -1, "Output path for skeleton SM file does not exist: $skel_pname");
  }

  # Create some header guards for the output header files based upon their names
  my ($inth_hguard, $exth_hguard) = (uc $inth_fname, uc $exth_fname);
  $inth_hguard =~ tr|[.]|[_]|;
  $exth_hguard =~ tr|[.]|[_]|;

  # Tokenized file data
  my @tokendata;

  # Tokenize the file
  TokenizeFile( $ARGV[ 0 ], \@tokendata );

  # Make sure we got some tokens (guard against possible empty/blank file)
  if ( IsEOT( \@tokendata ) ) {
    #Debug( -1, "Empty or blank input file");

    # Create the required files
    # @todo: Make this cleaner.  Maybe skip to the end to create the default
    #        empty files
    my $filehandle;
    if ( $exth_fpname ne "" ) {
      open( $filehandle, ">$exth_fpname" );
      close( $filehandle );
    }
    if ( $inth_fpname ne "" ) {
      open( $filehandle, ">$inth_fpname" );
      close( $filehandle );
    }
    if ( $intc_fpname ne "") {
      open( $filehandle, ">$intc_fpname" );
      close( $filehandle );
    }
    if ( $skel_fpname ne "" ) {
      open( $filehandle, ">$skel_fpname" );
      close( $filehandle );
    }
    exit( 0 );
  }

  # Get the true source filename from the token stream
  # (works w/ cpp'ed data or plain data, thanks to tokenizer)
  my $sourcefilename = ExtractFilename( \@tokendata, 0 );

  # Make sure we have a sane-looking source filename (ie. not "-")
  if ( $sourcefilename eq "-" ) {
    Debug( -1, "Ambiguous input filename, please preprocess STDIN or specify an input filename");
  }

  my $datatree     = {};
  my $errortext    = "";
  my $status       = 0;
  my $gotsmg       = 0;
  my $outIntHeader   = "";
  my $outIntSource   = "";
  my $outExtHeader   = "";
  my $outSkelSource = "";
  my $outTLAsource = "";
  my $instances = 0;
  my $SMnames = "";

  # Try to find some top-level SM's
  do {
    $status = ParseSM( \@tokendata, $datatree, \$errortext, 0 );
  } while ( $status == 1 );

  # Bail out if there was an error
  if ( $status != 0 ) {
    Debug( -1, "$errortext" );
  }

  # And also bail out if we didn't fetch any state machines, since that's
  # all we can do at this point
  if ( !defined $datatree->{ STATE_MACHINE }
       || ( scalar( @{ $datatree->{ STATE_MACHINE }->{ _KEYS } } ) == 0 ) )
  {
    Debug( -1, "Error: No state machines or groups parsed in $ARGV[0]" );
  }

  # Generate SM code for what we parsed
  foreach my $smk ( @{ $datatree->{ STATE_MACHINE }->{ _KEYS } } ) {
    my $sm = $datatree->{ STATE_MACHINE }->{ $smk };
    $status = GenerateSMCode( $sm, $smk,$sm ,\$errortext, 0 );
    last if ( $status != 0 );
  }

  # Handle any errors in the code generation
  if ( $status != 0 ) {
    Debug( -1, "$errortext" );
  }

  # Build the files for one or more state machine
  foreach my $smk ( @{ $datatree->{ STATE_MACHINE }->{ _KEYS } } ) {
    my $sm = $datatree->{ STATE_MACHINE }->{ $smk };

    # Header file can have info from all state machines,
    # regardless of whether they were in the parsed file or not
    $outExtHeader   .= $sm->{ ExtHeader };

    # However, the internal and skeleton source to be generated should only be
    # for the parsed file, and not anything it #includes
    if ( $sm->{ FILENAME } eq $sourcefilename ) {
      Debug(
             1,
             "Including external source for SM: $smk, "
               . "filename: $sm->{FILENAME} == $sourcefilename"
               );
      $outIntHeader   .= $sm->{ IntHeader };
      # Add the internal source to the internal header if only the header is
      # being generated
      if (defined $intc_pname)
      {
        $outIntSource   .= $sm->{ IntSource };
      }
      else
      {
        $outIntHeader   .= $sm->{ IntSource };
      }
      # Skeleton source always gets the same
      $outSkelSource  .= $sm->{ SkelSource };
      # Record the name of the SM
      $SMnames .= "  $smk \( $sm->{ INSTANCES } instance/s \)\n";
      # Grab the maximum # of instances in all the SM's
      if ($sm->{ INSTANCES } > $instances) {
        $instances = $sm->{ INSTANCES };
      }

    }
    else {
      Debug(
             1,
             "Excluding external source for SM: $smk, "
               . "filename: $sm->{FILENAME} != $sourcefilename"
               );
    }

  }

  # Let's re-use the disclaimer as much as possible
  my $disclaimer = <<DONE;
/*===========================================================================

  Copyright (c) $currentyear Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/
DONE


  # Now build the actual files and write them out
  # using the provided (or implied) path-prefixed basename
  my $filehandle;

  # Write out the external header file if requested
  if ($exth_fpname ne "") {
    Debug( 1, "Writing external header file: $exth_fpname");
    push @files_created,$exth_fpname;
    open( $filehandle, ">$exth_fpname" )
      || Debug( -1, "Error, can't write file: $exth_fpname" );
    print $filehandle <<EOF;
/*=============================================================================

    $exth_fname

Description:
  This file contains the machine generated header file for the state machine
  specified in the file:
  ${sourcefilename}

=============================================================================*/

$disclaimer

#ifndef $exth_hguard
#define $exth_hguard

#ifdef __cplusplus
/* If compiled into a C++ file, ensure symbols names are not mangled */
extern "C"
{
#endif

/* Include STM framework header */
#include <stm2.h>

$outExtHeader

#ifdef __cplusplus
} /* extern "C" {...} */
#endif

#endif /* $exth_hguard */
EOF
    close($filehandle);
  }


  # Write out the internal header file if requested
  if ($inth_fpname ne "") {
    Debug( 1, "Writing internal header file: $inth_fpname");
    push @files_created,$inth_fpname;
    open( $filehandle, ">$inth_fpname")
      || Debug( -1, "Error, can't write file: $inth_fpname" );
    print $filehandle <<EOF;
/*=============================================================================

    $inth_fname

Description:
  This file contains the machine generated header file for the state machine
  specified in the file:
  ${sourcefilename}

=============================================================================*/

$disclaimer

#ifndef $inth_hguard
#define $inth_hguard

#ifdef __cplusplus
/* If compiled into a C++ file, ensure symbols names are not mangled */
extern "C"
{
#endif

/* Include external state machine header */
#include "$exth_fname"

$outIntHeader

#ifdef __cplusplus
} /* extern "C" {...} */
#endif

#endif /* ! $inth_hguard */
EOF
    close($filehandle);
  }

  # Write out the internal source file if requested
  if ($intc_fpname ne "") {
    Debug( 1, "Writing internal source file: $intc_fpname");
    push @files_created,$intc_fpname;
    open( $filehandle, ">$intc_fpname")
      || Debug( -1, "Error, can't write file: $intc_fpname" );
    print $filehandle <<EOF;
/*=============================================================================

    $intc_fname

Description:
  This file contains the machine generated source file for the state machine
  specified in the file:
  ${sourcefilename}

=============================================================================*/

$disclaimer

/* Include STM compiler generated external and internal header files */
#include "$exth_fname"
#include "$inth_fname"

$outIntSource

EOF
    close($filehandle);
  }

  # Write out the skeleton SM C file if it is requested
  if ($skel_fpname ne "") {
    Debug( 1, "Writing skeleton state machine C file: $skel_fpname");
    push @files_created,$skel_fpname;
    open( $filehandle, ">$skel_fpname")
      || Debug( -1, "Error, can't write file: $skel_fpname" );
    print $filehandle <<EOF;
/*!
  \@file
  $skel_fname

  \@brief
  This module contains the entry, exit, and transition functions
  necessary to implement the following state machines:

  \@detail
$SMnames

  OPTIONAL further detailed description of state machines
  - DELETE this section if unused.

*/

$disclaimer

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

\$Header\: \$

when       who     what, where, why
--------   ---     ----------------------------------------------------------
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

/* Include STM external API */
#include <stm2.h>

//! \@todo Include necessary files here


/*===========================================================================

         STM COMPILER GENERATED PROTOTYPES AND DATA STRUCTURES

===========================================================================*/

/* Include STM compiler generated internal data structure file */
#include "$inth_fname"

EOF

    # Handle 1 or more instances differently
    if ($instances == 1) {
      print $filehandle <<EOF;
/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/


/*! \@brief Structure for state-machine per-instance local variables
*/
typedef struct
{
  int   internal_var;  /*!< My internal variable */
  void *internal_ptr;  /*!< My internal pointer */
  //! \@todo SM per-instance variables go here
} ${skel_bname}_type;


/*! \@brief Variables internal to module $skel_fname
*/
STATIC ${skel_bname}_type ${skel_bname};

EOF
    } else {
      print $filehandle <<EOF;
/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/


/*! \@brief Structure for state-machine per-instance local variables
*/
typedef struct
{
  int   internal_var;  /*!< My internal variable */
  void *internal_ptr;  /*!< My internal pointer */
  //! \@todo SM per-instance variables go here
} ${skel_bname}_instance_type;

/*! \@brief All variables internal to module $skel_fname
*/
typedef struct
{
  /*! My array of per-instance internal variables. */
  ${skel_bname}_instance_type instance[ $instances ];
  //! \@todo Other non-per-SM-instance vars go here
} ${skel_bname}_type;

/*! \@brief Variables internal to module $skel_fname
*/
STATIC ${skel_bname}_type ${skel_bname};

EOF
    }

    print $filehandle <<EOF;
$outSkelSource

EOF
    close($filehandle);
  }

  # Exit with success here
  exit( 0 );
}

