#ifndef _RTSP_SESSION_H
#define _RTSP_SESSION_H
/***************************************************************************
 *                             rtsp_session.h
 * DESCRIPTION
 *  RTSP Session class for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2014 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/inc/rtsp_session.h#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_common.h"
#include "rtsp_wfd.h"
#include "RTSPStringStream.h"

/*
 * Strings used for constructing and parsing messages
 */
static const char clientPort[] = "client_port=";
static const char url[] = "rtsp://";
static const char wfdUrl[] = "rtsp://localhost/wfd1.0";
static const char cSeq[] = "CSeq:";
static const char successCode[] = "200";
static const char errorCode[] = "303 See Other";
static const char standbyNotApplicable[] = "406 in-standby-mode";
static const char success[] = "OK";
static const char session[] = "Session:";
static const char rtspVersion[] = "RTSP/1.0";
static const char contentLength[] = "Content-Length:";
static const char contentType[] = "Content-Type: text/parameters";
static const char crlf[] = "\r\n";
static const char transport[] = "Transport:";
static const char methods[] = "Public:";
static const char timeout[] = "timeout=";

/*
 * RTSP commands
 */
enum rtspCmds {
    invalidCmd = 0,
    wfdCmd,
    optionsCmd,
    getParameterCmd,
    setParameterCmd,
    setupCmd,
    playCmd,
    pauseCmd,
    teardownCmd
};

/*
 * RTSP message status
 */
enum rtspStatus {
    ok,
    error
};

/*
 * RTSP request/response
 */
enum rtspCmdType {
    cmdRequest = 1,
    cmdResponse
};

class rtspMesg {
public:
    rtspMesg() : cmd(invalidCmd), cmdName(""), type(cmdRequest), subCmd(invalidCmd), wfdParams(0) {}
    void reset()
    {
        cmd = subCmd = invalidCmd;
        cmdName = "";
        type = cmdRequest;
        wfdParams.reset();
        wfdOptionsParams.reset();
    }

    rtspCmds cmd;
    string cmdName;
    rtspCmdType type;
    rtspCmds subCmd;
    bitset<WFD_MAX_SIZE> wfdParams;
    bitset<WFD_MAX_SIZE> wfdOptionsParams;
};

/*
 * Valid bitmap to identify what's valid in the parsed data container class:
 * rtspParams
 */
enum rtspParamsField {
    mesgValid = 0x1,
    statusValid = 0x2,
    sessionValid = 0x4,
    cRxSeqValid = 0x8,
    cTxSeqValid = 0x10,
    dateValid = 0x20,
    cRtpPort0Valid = 0x40,
    cRtpPort1Valid = 0x80,
    uriValid = 0x100,
    wfdParamsValid = 0x200,
    wfdValid = 0x400,
};

/*
 * Contains the current state of an active session
 */
class rtspSessionState {
public:
    rtspSessionState() : sessionID(0), sock(0), txCseq(0), rxCseq(0), wfdSupp(false)
    {
        time_t tt = time(NULL);
        struct tm * ptm;
        ptm = gmtime(&tt);

        memset(date, 0, sizeof(date));
        if(ptm != NULL)
        {
            strftime(date, sizeof(date),
                 "Date: %a, %b %d %Y %H:%M:%S GMT", ptm);
        }
    }

    char *getDate()
    {
        time_t tt = time(NULL);
        struct tm * ptm;
        ptm = gmtime(&tt);

        memset(date, 0, sizeof(date));
        if(ptm != NULL)
        {
            strftime(date, sizeof(date),
                 "Date: %a, %b %d %Y %H:%M:%S GMT", ptm);
        }

        return date;
    }

    string sessStr;
    SESSION sessionID;
    SOCKET sock;
    unsigned txCseq;
    unsigned rxCseq;
    char date[MAXLEN];
    rtspWfd wfd;
    bool wfdSupp;
};

/*
 * Parsed message container class
 */
class rtspParams {
public:
    rtspParams() : valid(0), status(ok), next(NULL), respCode("") {}
    void reset()
    {
        valid = 0;
        next = NULL;
        rtsp_state = 0;
        mesg.reset();
    }

    unsigned valid;
    rtspMesg mesg;
    rtspStatus status;
    rtspSessionState state;
    rtspParams *next;
    string respCode;
    int rtsp_state;
};

/*
 * Contains pending command information for use in the pend queue
 */
class rtspPending {
public:
    rtspPending() : cmd(invalidCmd), session(NULL), time(0),timeOut(DEFAULT_TIMEOUT), seq(0), subCmd(invalidCmd) {}
    rtspPending(rtspCmds c, rtspSession *rs, unsigned t, unsigned s) :
                cmd(c), session(rs), time(t), timeOut(DEFAULT_TIMEOUT),seq(s), subCmd(invalidCmd) {}
    rtspPending(rtspCmds c, rtspSession *rs, unsigned t, unsigned s, rtspWfd w, rtspCmds sub) :
                cmd(c), session(rs), time(t), timeOut(DEFAULT_TIMEOUT),seq(s), wfd(w), subCmd(sub) {}
    void setTimeout(unsigned time) {timeOut = time;}
    rtspCmds cmd;
    rtspSession *session;
    unsigned time;
    unsigned timeOut;
    unsigned seq;
    rtspWfd wfd;
    rtspCmds subCmd;
};

struct rtspCmdTuple
{
    rtspCmds cmd;
    string cmdName;
};

/*
 * Supported RTSP commands
 */
static const rtspCmdTuple supportedCmds[] = {
                                              {invalidCmd, ""},
                                              {wfdCmd, "org.wfa.wfd1.0"},
                                              {optionsCmd, "OPTIONS"},
                                              {getParameterCmd, "GET_PARAMETER"},
                                              {setParameterCmd, "SET_PARAMETER"},
                                              {setupCmd, "SETUP"},
                                              {playCmd, "PLAY"},
                                              {pauseCmd, "PAUSE"},
                                              {teardownCmd, "TEARDOWN"},
                                            };
#define METHOD_MAX_SIZE  32

static const int numSupportedCmds = (int)(sizeof(supportedCmds) / sizeof(supportedCmds[0]));

/*
 * RTSP error codes
 */
enum rtspCodes {
    Continue,
    OK,
    Created,
    Low_on_Storage_Space,
    Multiple_Choices,
    Moved_Permanently,
    Moved_Temporarily,
    See_Other,
    Not_Modified,
    Use_Proxy,
    Bad_Request,
    Unauthorized,
    Payment_Required,
    Forbidden,
    Not_Found,
    Method_Not_Allowed,
    Not_Acceptable,
    Proxy_Authentication_Required,
    Request_Timeout,
    Gone,
    Length_Required,
    Precondition_Failed,
    Request_Entity_Too_Large,
    Request_URI_Too_Large,
    Unsupported_Media_Type,
    Parameter_Not_Understood,
    Conference_Not_Found,
    Not_Enough_Bandwidth,
    Session_Not_Found,
    Method_Not_Valid_in_This_State,
    Header_Field_Not_Valid_for_Resource,
    Invalid_Range,
    Parameter_Is_Read_Only,
    Aggregate_operation_not_allowed,
    Only_aggregate_operation_allowed,
    Unsupported_transport,
    Destination_unreachable,
    Internal_Server_Error,
    Not_Implemented,
    Bad_Gateway,
    Service_Unavailable,
    Gateway_Timeout,
    RTSP_Version_not_supported,
    Option_not_supported,
};
#define RTSP_STATUS_MAX_SIZE  43

struct rtspStatusTable {
    rtspCodes code;
    string num;
    string text;
};

static const rtspStatusTable rtspStatus[] = {
        { Continue, "100", "Continue"},
        { OK, "200", "OK" },
        { Created, "201", "Created" },
        { Low_on_Storage_Space, "250", "Low on Storage Space" },
        { Multiple_Choices, "300", "Multiple Choices" },
        { Moved_Permanently, "301", "Moved Permanently" },
        { Moved_Temporarily, "302", "Moved Temporarily" },
        { See_Other, "303", "See Other" },
        { Not_Modified, "304", "Not Modified" },
        { Use_Proxy, "305", "Use Proxy" },
        { Bad_Request, "400", "Bad Request" },
        { Unauthorized, "401", "Unauthorized" },
        { Payment_Required, "402", "Payment Required" },
        { Forbidden, "403", "Forbidden" },
        { Not_Found, "404", "Not Found" },
        { Method_Not_Allowed, "405", "Method Not Allowed" },
        { Not_Acceptable, "406", "Not Acceptable" },
        { Proxy_Authentication_Required, "407", "Proxy Authentication Required" },
        { Request_Timeout, "408", "Request Time-out" },
        { Gone, "410", "Gone" },
        { Length_Required, "411", "Length Required" },
        { Precondition_Failed, "412", "Precondition Failed" },
        { Request_Entity_Too_Large, "413", "Request Entity Too Large" },
        { Request_URI_Too_Large, "414", "Request-URI Too Large" },
        { Unsupported_Media_Type, "415", "Unsupported Media Type" },
        { Parameter_Not_Understood, "451", "Parameter Not Understood" },
        { Conference_Not_Found, "452", "Conference Not Found" },
        { Not_Enough_Bandwidth, "453", "Not Enough Bandwidth" },
        { Session_Not_Found, "454", "Session Not Found" },
        { Method_Not_Valid_in_This_State, "455", "Method Not Valid in This State" },
        { Header_Field_Not_Valid_for_Resource, "456", "Header Field Not Valid for Resource" },
        { Invalid_Range, "457", "Invalid Range" },
        { Parameter_Is_Read_Only, "458", "Parameter_Is_Read-Only" },
        { Aggregate_operation_not_allowed, "459", "Aggregate operation not allowed" },
        { Only_aggregate_operation_allowed, "460", "Only aggregate operation allowed" },
        { Unsupported_transport, "461", "Unsupported transport" },
        { Destination_unreachable, "462", "Destination unreachable" },
        { Internal_Server_Error, "500", "Internal Server Error" },
        { Not_Implemented, "501", "Not Implemented" },
        { Bad_Gateway, "502", "Bad Gateway" },
        { Service_Unavailable, "503", "Service Unavailable" },
        { Gateway_Timeout, "504", "Gateway Time-out" },
        { RTSP_Version_not_supported, "505", "RTSP Version not supported" },
        { Option_not_supported, "551", "Option not supported" }
    };
static const int numRtspStatusCodes = (int)(sizeof(rtspStatus) / sizeof(rtspStatus[0]));

class getParamCommand;
class setParamCommand;

/*
 * Wrapper and API's for the current session state
 */
class rtspSession {
public:
    rtspSession() : ipAddr("") {}

    /* Set/Dump WFD state */
    void setWfd(rtspWfd wfd) { state.wfd.assign(wfd); }
    rtspWfd& getWfd() { return state.wfd; }
    void dumpWfd() { state.wfd.dump(); }
    void copyWfd(rtspWfd &wfd) { state.wfd = wfd; }

    /* Get/Set the WFD session ID */
    void setSessionID(SESSION session) { state.sessionID = session; }
    SESSION getSessionID() const { return state.sessionID; }

    void setSessionStr(string str) { state.sessStr = str; }
    string getSessionStr() const { return state.sessStr; }

    /* Get/Set local RTSP socket */
    void setSocket(SOCKET sock) { state.sock = sock; }
    SOCKET getSocket() const { return state.sock; }

    /* Increment/Get tx sequence number */
    void incTxCseq() { state.txCseq++; }
    unsigned getTxCseq() const { return state.txCseq; }

    /* Get/Set the rx sequence number */
    unsigned getRxCseq() const { return state.rxCseq; }
    void setRxCseq(unsigned seq) { state.rxCseq = seq; }

    /* Get the date in GMT format */
    char *getDate() { return state.getDate(); }

    /* Set the UIBC TCP port */
    void setUibcPort(unsigned port) { if (port) { state.wfd.uibcCap.setPort(port); } }

    /* Get/Set RTP Port0 */
    unsigned getRtpPort0() const { return state.wfd.client.getRtpPort0(); }
    void setRtpPort0(unsigned port) { state.wfd.client.setRtpPort0(port); }

    /* Get/Set RTP Port1 */
    unsigned getRtpPort1() const { return state.wfd.client.getRtpPort1(); }
    void setRtpPort1(unsigned port) { state.wfd.client.setRtpPort1(port); }

    void setHdcpPort(unsigned port) { if (port) { state.wfd.contentProtection.setCtrlPort(port); } }
    unsigned getHdcpPort() const { return state.wfd.contentProtection.getCtrlPort(); }


    /* Get URI string */
    string getUri() const
    {
        string uri0 = state.wfd.uri.getUri0();
        string uri1 = state.wfd.uri.getUri1();
        return (uri0 == none) ? ((uri1 == none) ? "" : uri1) : uri0;
    }

    /* Get/Set source IP address */
    string getIp0() const { return state.wfd.uri.getIpAddr0(); }
    void setIp0(string ip) { state.wfd.uri.setIpAddr0(ip); }

    string getIp1() const { return state.wfd.uri.getIpAddr1(); }
    void setIp1(string ip) { state.wfd.uri.setIpAddr1(ip); }

    bool getStandby() const { return state.wfd.halt.getValid(); }
    bool getResume() const { return state.wfd.start.getValid(); }

    bool getCoupled() const { return state.wfd.coupledSink.isCoupled(); }
    void setCoupled() { state.wfd.coupledSink.setCoupled(); }

    string getCoupledMac() const { return state.wfd.coupledSink.getMacAddr(); }
    void setCoupledMac(string mac) { state.wfd.coupledSink.setMacAddr(mac); }

    bool isPrimarySink() const { return state.wfd.isPrimarySink; }
    bool isSecondarySink() const { return state.wfd.isSecondarySink; }

    string getIpAddr() const { return ipAddr; }
    void setIpAddr(string addr) { ipAddr = addr; }

    /* Allow overloaded Get/Set operators to access session state */
    friend RTSPStringStream &operator<<(RTSPStringStream &, getParamCommand *);
    friend RTSPStringStream &operator<<(RTSPStringStream &, setParamCommand *);

private:
    rtspSessionState state;
    string ipAddr;
};

/*
 * Base class that RTSP commands are derived from
 */
class rtspCommands {
public:
    rtspCommands()
    {
        setCmd(invalidCmd);
        setSubCmd(invalidCmd);
        resetWfdParams();
        cmdTimeout = DEFAULT_TIMEOUT;
    }
    rtspCommands(rtspCmdType type, rtspSession session) : state(session), status(OK)
    {
        setType(type);
        setCmd(invalidCmd);
        setSubCmd(invalidCmd);
        resetWfdParams();
        cmdTimeout = DEFAULT_TIMEOUT;
    }
    rtspCommands(rtspCmds cmd, rtspCmdType type, rtspSession session) : state(session), status(OK)
    {
        setCmd(cmd);
        setType(type);
        setSubCmd(invalidCmd);
        resetWfdParams();
        cmdTimeout = DEFAULT_TIMEOUT;
    }
    rtspCommands(rtspCmds cmd, rtspCmdType type, rtspSession session, rtspCodes code) : state(session), status(code)
    {
        setCmd(cmd);
        setType(type);
        setSubCmd(invalidCmd);
        resetWfdParams();
        cmdTimeout = DEFAULT_TIMEOUT;
    }
    rtspCommands(rtspCmds cmd, rtspCmdType type, rtspCmds subCmd, rtspSession session) : state(session), status(OK)
    {
        setCmd(cmd);
        setType(type);
        setSubCmd(subCmd);
        resetWfdParams();
        setWfdParams(wfd_trigger_method);
        cmdTimeout = DEFAULT_TIMEOUT;
    }
    rtspCommands(rtspCmds cmd, rtspCmdType type, bitset<WFD_MAX_SIZE> &wfd, rtspSession session) : state(session), status(OK)
    {
        setCmd(cmd);
        setType(type);
        setWfdParams(wfd);
        setSubCmd(invalidCmd);
        cmdTimeout = DEFAULT_TIMEOUT;
    }
    virtual ~rtspCommands() {}
    void setCmd(rtspCmds cmd) { mesg.cmd = cmd; }
    void setSubCmd(rtspCmds cmd) { mesg.subCmd = cmd; }
    void setType(rtspCmdType type) { mesg.type = type; }
    void setWfdParams(bitset<WFD_MAX_SIZE> &wfd) { mesg.wfdParams = wfd; }
    void setWfdParams(unsigned wfd) { mesg.wfdParams.set(wfd); }
    void resetWfdParams() { mesg.wfdParams.reset(); }
    void setCmdTimeout(unsigned time) {cmdTimeout = time;}
    unsigned getCmdTimeout () {return cmdTimeout;}
    string send();
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, rtspCommands *);
protected:
    rtspSession state;
    rtspMesg mesg;
    rtspCodes status;
    unsigned cmdTimeout;
};

class optionsCommand : public rtspCommands {
public:
    optionsCommand(rtspCmdType type, rtspSession &state, bitset<METHOD_MAX_SIZE> &supp) : rtspCommands(optionsCmd, type, state)
    { suppModes = supp; }
    string send();
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, optionsCommand *);
private:
    bitset<METHOD_MAX_SIZE> suppModes;
};

class getParamCommand : public rtspCommands {
public:
    getParamCommand(rtspCmdType type, rtspSession &state) : rtspCommands(getParameterCmd, type, state) {}
    getParamCommand(rtspCmdType type, bitset<WFD_MAX_SIZE> params, rtspSession &state) : rtspCommands(getParameterCmd, type, params, state) {}
    string send();
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, getParamCommand *);
};

class setParamCommand : public rtspCommands {
public:
    setParamCommand(rtspCmdType type, rtspSession &state) : rtspCommands(setParameterCmd, type, state) {}
    setParamCommand(rtspCmdType type, rtspCmds cmd, rtspSession &state) : rtspCommands(setParameterCmd, type, cmd, state) {}
    setParamCommand(rtspCmdType type, bitset<WFD_MAX_SIZE> params, rtspSession &state) : rtspCommands(setParameterCmd, type, params, state) {}
    string send();
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, setParamCommand *);
    int status;
};

class setupCommand : public rtspCommands {
public:
    setupCommand(rtspCmdType type, rtspSession &state) : rtspCommands(setupCmd, type, state) {}
    string send();
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, setupCommand *);
};

class playCommand : public rtspCommands {
public:
    playCommand(rtspCmdType type, rtspSession &state) : rtspCommands(playCmd, type, state) {}
    playCommand(rtspCmdType type, rtspSession &state, rtspCodes code) : rtspCommands(playCmd, type, state, code) {}
    string send();
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, playCommand *);
};

class pauseCommand : public rtspCommands {
public:
    pauseCommand(rtspCmdType type, rtspSession &state) : rtspCommands(pauseCmd, type, state) {}
    pauseCommand(rtspCmdType type, rtspSession &state, rtspCodes code) : rtspCommands(pauseCmd, type, state, code) {}
    string send();
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, pauseCommand *);
};

class teardownCommand : public rtspCommands {
public:
    teardownCommand(rtspCmdType type, rtspSession &state) : rtspCommands(teardownCmd, type, state) {}
    teardownCommand(rtspCmdType type, rtspSession &state, rtspCodes code) : rtspCommands(teardownCmd, type, state, code) {}
    string send();
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, teardownCommand *);
};

#endif /*_RTSP_SESSION_H*/
