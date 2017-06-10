/*
 *     Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 *     Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.listen;

import com.qualcomm.listen.ListenTypes.EventData;

/**
 * Applications must provide an implementation of this
 * interface to receive events from ListenEngine.
 */
public interface IListenEventProcessor {

    /**
     * Processes event received from ListenEngine
     * <p>
     *
     * Acts as a callback to client when Listen event is serviced.
     * The application can ignore these events or look at the event
     * type and take some appropriate action.
     * <p> Each class that inherits from ListenReceiver
     * will send a different set of events to this callback.
     * <p> See ListenTypes in ListenTypes.java for a list of possible
     * events this method could receive.
     * <p> EventData should be locally used by processEvent method or
     * copied by application before processEvent is exited.
     *
     * @param  eventType [in] type of event from ListenTypes
     * @param  eventData [in] event payload allocated by Listen Engine.<p>
     *       EventData should be used or copied before processEvent is exitted
     */
	public void processEvent(int                 eventType,
                             EventData           eventData);
}