/*****************************************************************************/
/*                                                                           */
/*                 				Thread Abstraction Layer				             	 */
/*                     ITTIAM SYSTEMS PVT LTD, BANGALORE                     */
/*                             COPYRIGHT(C) 2010                             */
/*                                                                           */
/*  This program  is  proprietary to  Ittiam  Systems  Private  Limited  and */
/*  is protected under Indian  Copyright Law as an unpublished work. Its use */
/*  and  disclosure  is  limited by  the terms  and  conditions of a license */
/*  agreement. It may not be copied or otherwise  reproduced or disclosed to */
/*  persons outside the licensee's organization except in accordance with the*/
/*  terms  and  conditions   of  such  an  agreement.  All  copies  and      */
/*  reproductions shall be the property of Ittiam Systems Private Limited and*/
/*  must bear this notice in its entirety.                                   */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*  File Name         : ithread.h                                        */
/*                                                                           */
/*  Description       : This file contains all the necessary structure and   */
/*                      enumeration definitions needed for the Application   */
/*                      Program Interface(API) of the 						 */
/*						Thread Abstraction Layer        								 */
/*                                                                           */
/*  List of Functions : ithread_api_function                            */
/*                                                                           */
/*  Issues / Problems : None                                                 */
/*                                                                           */
/*  Revision History  :                                                      */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes (Describe the changes made)  */
/*         26 08 2010        			Draft                                */
/*                                                                           */
/*****************************************************************************/

#ifndef _ITHREAD_H
#define _ITHREAD_H

UWORD32 ithread_get_handle_size(void);

UWORD32 ithread_get_mutex_lock_size(void);

WORD32 	ithread_create(void *thread_handle, void *attribute, void *strt, void *argument);

void ithread_exit(void *val_ptr);

WORD32 	ithread_join(void *thread_id, void ** val_ptr);

WORD32 ithread_mutex_init(void *mutex);

WORD32 ithread_mutex_destroy(void *mutex);

WORD32 	ithread_mutex_lock(void *mutex);

WORD32 	ithread_mutex_unlock(void *mutex);

void 	ithread_yield(void);

void 	ithread_sleep(UWORD32 u4_time_in_us);

UWORD32 ithread_get_sem_strcut_size(void);

WORD32 ithread_sem_init(void *sem,WORD32 pshared,UWORD32 value);

WORD32 ithread_sem_post(void *sem);

WORD32 ithread_sem_wait(void *sem);

WORD32 ithread_sem_destroy(void *sem);

#endif /* _IITHREAD_H */
