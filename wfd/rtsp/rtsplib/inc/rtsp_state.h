#ifndef _RTSP_STATE_H
#define _RTSP_STATE_H

/***************************************************************************
 *                             rtsp_state.h
 * DESCRIPTION
 *  RTSP State class for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2014 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/inc/rtsp_state.h#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/
#include "rtsp_common.h"
#include "rtsp_session.h"
#include <string>

using std::string;

class rtspFSM;
class rtspM1;
class rtspM2;
class rtspM3;
class rtspM4;
class rtspM5;
class rtspM6;
class rtspM7;
class rtspM8;
class rtspM9;
class rtspM10;

class rtspSource;
class rtspSink;
class rtspServer;
class rtspClient;
class rtspParams;

/*
 * FSM macro states
 */
enum rtspMacroState {
    init,
    ready,
    playing,
    paused,
    toredown,
    standby
};

/*
 * State machine base class that M[*] states are derived from
 */
class rtspBaseState {
public:
    rtspBaseState() : previousCmd(invalidCmd), name("NULL"), state(M1) {}
    rtspBaseState(string myName, rtspState myState) : previousCmd(invalidCmd),
                                                      name(myName),
                                                      state(myState) {}
    virtual ~rtspBaseState() {}
    virtual void request(rtspFSM *, rtspSource *, rtspCmds) = 0;
    virtual void request(rtspFSM *, rtspSource *, rtspCmds, rtspWfd&) = 0;
    virtual void request(rtspFSM *, rtspSource *) = 0;
    virtual void response(rtspFSM *, rtspSource *) = 0;
    virtual void request(rtspFSM *, rtspSink *, rtspCmds, rtspWfd&) = 0;
    virtual void request(rtspFSM *, rtspSink *) = 0;
    virtual void response(rtspFSM *, rtspSink *) = 0;
    string getName() {return name;}
    rtspState getState() {return state;}
    void cmdToState(rtspFSM *, int);
    template <class Type>
    bool isError(rtspFSM *, Type *, rtspParams *, rtspCmds);
    static void isKeepAlive(void* pMe);
    static void releaseKeepAliveTimer();

    static rtspM1 m1;
    static rtspM2 m2;
    static rtspM3 m3;
    static rtspM4 m4;
    static rtspM5 m5;
    static rtspM6 m6;
    static rtspM7 m7;
    static rtspM8 m8;
    static rtspM9 m9;
    static rtspM10 m10;
    rtspCmds previousCmd; //Used only for timeout, for trigger commands
protected:
    string name;
    rtspState state;
};

class rtspM1 : public rtspBaseState {
public:
    rtspM1() : rtspBaseState("M1", M1) {};
    void request(rtspFSM *, rtspSource *, rtspCmds) {}
    void request(rtspFSM *, rtspSource *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSource *);
    void response(rtspFSM *, rtspSource *);
    void request(rtspFSM *, rtspSink *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSink *);
    void response(rtspFSM *, rtspSink *);
};

class rtspM2 : public rtspBaseState {
public:
    rtspM2() : rtspBaseState("M2", M2) {};
    void request(rtspFSM *, rtspSource *, rtspCmds) {}
    void request(rtspFSM *, rtspSource *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSource *);
    void response(rtspFSM *, rtspSource *);
    void request(rtspFSM *, rtspSink *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSink *);
    void response(rtspFSM *, rtspSink *);
};

class rtspM3 : public rtspBaseState {
public:
    rtspM3() : rtspBaseState("M3", M3) {};
    void request(rtspFSM *, rtspSource *, rtspCmds) {}
    void request(rtspFSM *, rtspSource *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSource *);
    void response(rtspFSM *, rtspSource *);
    void request(rtspFSM *, rtspSink *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSink *);
    void response(rtspFSM *, rtspSink *);
};

class rtspM4 : public rtspBaseState {
public:
    rtspM4() : rtspBaseState("M4", M4) {};
    void request(rtspFSM *, rtspSource *, rtspCmds) {}
    void request(rtspFSM *, rtspSource *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSource *);
    void response(rtspFSM *, rtspSource *);
    void request(rtspFSM *, rtspSink *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSink *);
    void response(rtspFSM *, rtspSink *);
};

class rtspM5 : public rtspBaseState {
public:
    rtspM5() : rtspBaseState("M5", M5) {};
    void request(rtspFSM *, rtspSource *, rtspCmds);
    void request(rtspFSM *, rtspSource *, rtspCmds, rtspWfd&);
    void request(rtspFSM *, rtspSource *) {}
    void response(rtspFSM *, rtspSource *);
    void request(rtspFSM *, rtspSink *, rtspCmds, rtspWfd&);
    void request(rtspFSM *, rtspSink *) {}
    void response(rtspFSM *, rtspSink *);
};

class rtspM6 : public rtspBaseState {
public:
    rtspM6() : rtspBaseState("M6", M6) {};
    void request(rtspFSM *, rtspSource *, rtspCmds) {}
    void request(rtspFSM *, rtspSource *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSource *);
    void response(rtspFSM *, rtspSource *);
    void request(rtspFSM *, rtspSink *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSink *);
    void response(rtspFSM *, rtspSink *);
};

class rtspM7 : public rtspBaseState {
public:
    rtspM7() : rtspBaseState("M7", M7) {};
    void request(rtspFSM *, rtspSource *, rtspCmds) {}
    void request(rtspFSM *, rtspSource *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSource *);
    void response(rtspFSM *, rtspSource *);
    void request(rtspFSM *, rtspSink *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSink *);
    void response(rtspFSM *, rtspSink *);
};

class rtspM8 : public rtspBaseState {
public:
    rtspM8() : rtspBaseState("M8", M8) {};
    void request(rtspFSM *, rtspSource *, rtspCmds) {}
    void request(rtspFSM *, rtspSource *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSource *);
    void response(rtspFSM *, rtspSource *);
    void request(rtspFSM *, rtspSink *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSink *);
    void response(rtspFSM *, rtspSink *);
};

class rtspM9 : public rtspBaseState {
public:
    rtspM9() : rtspBaseState("M9", M9) {};
    void request(rtspFSM *, rtspSource *, rtspCmds) {}
    void request(rtspFSM *, rtspSource *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSource *);
    void response(rtspFSM *, rtspSource *);
    void request(rtspFSM *, rtspSink *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSink *);
    void response(rtspFSM *, rtspSink *);
};

class rtspM10 : public rtspBaseState {
public:
    rtspM10() : rtspBaseState("M10", M10) {};
    void request(rtspFSM *, rtspSource *, rtspCmds) {}
    void request(rtspFSM *, rtspSource *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSource *);
    void response(rtspFSM *, rtspSource *);
    void request(rtspFSM *, rtspSink *, rtspCmds, rtspWfd&) {}
    void request(rtspFSM *, rtspSink *);
    void response(rtspFSM *, rtspSink *);
};

/*
 * FSM class
 */
class rtspFSM {
public:
    rtspFSM() { state = &rtspBaseState::m1; macroState = init; }
    rtspFSM(rtspBaseState *start) { state = start; macroState = init; }

    template <class Type>
    void request(Type t) { state->request(this, t); }

    template <class Type>
    void request(Type t, rtspCmds c) { state->request(this, t, c); }

    template <class Type>
    void request(Type t, rtspWfd &w) { state->request(this, t, w); }

    template <class Type>
    void request(Type t, rtspCmds c, rtspWfd &w) { state->request(this, t, c, w); }

    template <class Type>
    void response(Type t) { state->response(this, t); }

    template <class Type>
    void play(Type t)
    {
        setState(&rtspBaseState::m7);
        state->request(this, t);
    }

    template <class Type>
    void pause(Type t)
    {
        setState(&rtspBaseState::m9);
        state->request(this, t);
    }

    template <class Type>
    void teardown(Type t)
    {
        setState(&rtspBaseState::m8);
        state->request(this, t);
    }
    rtspState getState(){ return (state!=NULL) ? state->getState() : INVALID_RTSP_STATE;}
    void setState(rtspBaseState *myState) { state = myState; }
    void setMacroState(rtspMacroState myState) { macroState = myState; }
    void reset() { setState(&rtspBaseState::m1); macroState = toredown; }
    bool isReady() const { return (macroState != init && macroState != toredown) ? true : false; }
    bool isToredown() const { return (macroState == toredown) ? true : false; }
    bool isInit() const { return (macroState == init) ? true : false; }
    bool isStandby() const { return (macroState == standby) ? true : false; }
    bool isPlaying() const { return (macroState == playing) ? true : false; }
    bool isPaused() const { return (macroState == paused) ? true : false; }
    void releaseKeepAliveTimer(){state->releaseKeepAliveTimer();}

private:
    rtspBaseState *state;
    rtspMacroState macroState;
};

#endif /*_RTSP_STATE_H*/
