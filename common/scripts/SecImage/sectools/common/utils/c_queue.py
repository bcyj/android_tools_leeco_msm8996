#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

'''
Created on Aug 6, 2012

@author: hraghav
'''

import Queue
import multiprocessing

class CoreQueue:
    queue = None
    processing_lock = None
    size = 0

    def __init__(self, size):
        ''' Create a queue with the maximum size specified. '''
        self.queue = Queue.Queue(size)
        self.processing_lock = multiprocessing.RLock()
        self.size = size

    def push (self, data):
        ''' Push the data to the queue. '''
        returnValue, returnError = False, ''
        if data:
            try:
                self.queue.put(data, block=False)
                returnValue = True
            except Queue.Full:
                returnError = 'Process queue reached max capacity - {0}.'.format(self.size)
        else:
            returnError = 'Empty data'
        return returnValue, returnError

    def pop (self):
        ''' Pop data from the queue. '''
        data = self.queue.get()
        self.processing_lock.acquire()
        return data

    def pop_complete (self):
        ''' Pop data from the queue. '''
        self.processing_lock.release()

    def empty(self):
        ''' Check if the queue is empty. '''
        retval = self.processing_lock.acquire(0)
        if retval:
            self.processing_lock.release()
        return (self.queue.empty() and retval)

    def close (self):
        ''' Close the queue. '''
        self.queue = None
        self.processing_lock = None
        self.size = 0
