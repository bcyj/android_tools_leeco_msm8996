#!/bin/sh
#***************************************************************************
#
#  @file    netmgr_gen_sm.sh
#  @brief   NETMGR generate state machine script
#
#  DESCRIPTION
#  This script generates the STM2 state machine files.  The input file
#  (netmgr_sm.stm) contains the STM2 specification for all states and
#  allowed transitions. There are four generated output files:
#       netmgr_sm_int.h  - Internal header         (do not modify)
#       netmgr_sm_int.c  - Internal implementation (do not modify)
#       netmgr_sm_ext.h  - External header         (do not modify)
#       netmgr_sm_stub.c - Base implementation     (modify as required)
#
#  Changes to the state machine specification should be made as follows:
#   1.  Update the netmgr_sm.stm file as required
#   2.  Execute this script to generate the four files
#   3.  Merge the changes to netmgr_sm_stub.c into the actual SM
#       implementation file.  Discard the stub file thereafter.
#   4.  Use the other generated files verbatum
#
#***************************************************************************
#===========================================================================
#
#  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved
#
#  Qualcomm Technologies Proprietary
#
#  Export of this technology or software is regulated by the U.S. Government.
#  Diversion contrary to U.S. law prohibited.
#
#  All ideas, data and information contained in or disclosed by
#  this document are confidential and proprietary information of
#  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
#  By accepting this material the recipient agrees that this material
#  and the information contained therein are held in confidence and in
#  trust and will not be used, copied, reproduced in whole or in part,
#  nor its contents revealed in any manner to others without the express
#  written permission of Qualcomm Technologies, Inc.
#
#===========================================================================

ROOTNAME=./netmgr_sm
CC=gcc
CFLAGS=-E
STMGEN=../../dsutils/src/stmcomp.pl
DEBUG_LVL="-v -v -v -v"

#${CC} ${CFLAGS}  ${ROOTNAME}.stm | \
   ${STMGEN} ${DEBUG_LVL}           \
             -oi ${ROOTNAME}_int.h  \
             -oc ${ROOTNAME}_int.c  \
             -oe ${ROOTNAME}_ext.h  \
             -os ${ROOTNAME}_stub.c \
             ${ROOTNAME}.stm
