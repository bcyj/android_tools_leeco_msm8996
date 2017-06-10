#ifndef _INITIALIZE_AUDCAL_H_
#define _INITIALIZE_AUDCAL_H_
/**
  \file **************************************************************************
 *
 *  I N I T I A L I Z E      A U D C A L       7 X 3 0
 *
 *DESCRIPTION
 *This header file contains all the definitions necessary for initializing audcal on 8x25.
 *
 *REFERENCES
 * None.
 *
 *
 *Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *Qualcomm Technologies Proprietary and Confidential.
 *******************************************************************************
 */
/**
 * FUNCTION : audcal_initialize
 *
 * DESCRIPTION : Initialize Audcal during driver initialization. Should be called
 *                         before any command is sent from PC
 *
 * DEPENDENCIES : None
 *
 * PARAMS: None
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
void audcal_initialize(void);

/**
 * FUNCTION : audcal_deinitialize
 *
 * DESCRIPTION : Deinitialize Audcal during driver termination.
 *
 * DEPENDENCIES : None
 *
 * PARAMS: None
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
void audcal_deinitialize(void);

#endif //_INITIALIZE_AUDCAL_H_
