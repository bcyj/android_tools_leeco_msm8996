/**************************************************************************
 *
 * Copyright (c)  2011 - 2012 Qualcomm Technologies, Inc., All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 *
 **************************************************************************/

#include "rtsp_server.h"
#include <stdlib.h>
#include <stdio.h>
#include "MMThread.h"
#include <string>

#define WFD_RTSPSESSION_THREADS_DEFAULT_STACKSIZE 262144
#define THREADCOUNT 1

using namespace std;


class cback : public rtspCallbacks {
public:
    cback() {}
    void setupCallback(rtspApiMesg &mesg);
    void playCallback(rtspApiMesg &mesg);
    void pauseCallback(rtspApiMesg &mesg);
    void teardownCallback(rtspApiMesg &mesg);
    void closeCallback(rtspApiMesg &mesg);
    void openCallback(rtspApiMesg &mesg);
    void intersectCallback(rtspApiMesg &mesg);
    void getCallback(rtspApiMesg &mesg);
    void setCallback(rtspApiMesg &mesg);
    void finishCallback();
};

void printMesg(rtspApiMesg &mesg)
{
   switch(mesg.error) {
    case noError:
        //cout << "Success" << endl;
    break;
    case badStateError:
        //cout << "Error: bad state" << endl;
    break;
    case timeoutError:
        //cout << "Error: timeout" << endl;
    break;
    case remoteError:
        //cout << "Error: remote error" << endl;
    break;
    default:
        //cout << "Error: " << (unsigned int)mesg.error << endl;
    break;
    }

    //cout << "Session: " << mesg.session << endl;
    //cout << "Port0: " << mesg.rtpPort0 << endl;
    //cout << "Port1: " << mesg.rtpPort1 << endl;
    //cout << "IP Addr: " << mesg.ipAddr << endl;
}

void cback::getCallback(rtspApiMesg &mesg)
{
    //cout << "Callback: Received Get Parameter" << endl;
    printMesg(mesg);
    mesg.wfd.dump();
}

void cback::setCallback(rtspApiMesg &mesg)
{
    //cout << "Callback: Received Set Parameter" << endl;
    printMesg(mesg);
    mesg.wfd.dump();
    /*
     * Modify wfd parameters here
     */
}

/*
 * Used to overwrite the WFD parameters before
 * RTSP begins negotiation
 */
void cback::openCallback(rtspApiMesg &mesg)
{
    //cout << "Callback: Received Open" << endl;
    printMesg(mesg);
    mesg.wfd.dump();
    //mesg.wfd.isPrimarySink = true;
    /*
     * Modify wfd parameters here
     */
}

/*
 * Used to overwrite the WFD parameters before
 * RTSP begins negotiation
 */
void cback::intersectCallback(rtspApiMesg &mesg)
{
    //cout << "Callback: Received Intersect" << endl;
    printMesg(mesg);
    mesg.wfd.dump();
    /*
     * Modify wfd parameters here
     */
}

void cback::setupCallback(rtspApiMesg &mesg)
{
    //cout << "Callback: Received Setup" << endl;
    printMesg(mesg);
    mesg.wfd.dump();
}

void cback::playCallback(rtspApiMesg &mesg)
{
    //cout << "Callback: Received Play" << endl;
    printMesg(mesg);
}

void cback::pauseCallback(rtspApiMesg &mesg)
{
    //cout << "Callback: Received Pause" << endl;
    printMesg(mesg);
}

void cback::teardownCallback(rtspApiMesg &mesg)
{
    //cout << "Callback: Received Teardown" << endl;
    printMesg(mesg);
}

void cback::closeCallback(rtspApiMesg &mesg)
{
    //cout << "Callback: Received Close" << endl;
    printMesg(mesg);
}

void cback::finishCallback()
{
    //cout << "Callback: Received finish" << endl;
}


#ifdef WIN_BUILD
int commands(void *datap)
#else
int commands(void *datap)
#endif
{
UNUSED(datap);
   /*
    rtspServer *server = (rtspServer *)datap;
    string request;
    SESSION mySession;

    while (1) {
        while (cin >> request) {
            //cout << "Received " << request << endl;
            if (request == "play") {
                //cout << "Enter session ID: ";
                cin >> mySession;
                //cout << "Sending play" << endl;
                server->Play(mySession);
            } else if (request == "pause") {
                //cout << "Enter session ID: ";
                cin >> mySession;
                //cout << "Sending pause" << endl;
                server->Pause(mySession);
            } else if (request == "teardown") {
                //cout << "Enter session ID: ";
                cin >> mySession;
                //cout << "Sending teardown" << endl;
                server->Teardown(mySession);
            } else if (request == "set") {
                // Example for post-session establishment set command
                rtspWfd myWfd;
#if 0
                bitset<BITSET_8> eac(1048);

                myWfd.audioEac.setValid(true);
                myWfd.audioEac.setModes(eac);
                myWfd.audioEac.setLatency(100);

                myWfd.h264Chp.setValid(true);
                myWfd.h264Chp.setLevel(2);
                myWfd.h264Chp.setMaxHres(1024);
                myWfd.h264Chp.setMaxVres(768);
                myWfd.h264Chp.setHhBit(31);
                myWfd.h264Chp.setCeaBit(30);
                myWfd.h264Chp.setVesaBit(29);
                myWfd.h264Chp.setLatency(200);

                myWfd.videoHeader.setValid(true);
                myWfd.videoHeader.setNative(7);
#else
                myWfd.uibcSet.setSetting(false);
                //cout << myWfd.uibcSet << endl;
#endif
                //cout << "Enter session ID: ";
                cin >> mySession;
                //cout << "Sending set" << endl;
                server->Set(mySession, myWfd);
            } else if (request == "get") {
                // Example for post-session establishment set command
                rtspWfd myWfd;
                myWfd.client.setValid(true);

                //cout << "Enter session ID: ";
                cin >> mySession;
                //cout << "Sending get" << endl;
                server->Get(mySession, myWfd);
            } else if (request == "standby") {
                rtspWfd myWfd;

                myWfd.halt.setValid(true);

                //cout << "Enter session ID: ";
                cin >> mySession;
                //cout << "Sending standby" << endl;
                server->Set(mySession, myWfd);

            } else if (request == "resume") {
                rtspWfd myWfd;

                myWfd.start.setValid(true);

                //cout << "Enter session ID: ";
                cin >> mySession;
                //cout << "Sending resume" << endl;
                server->Set(mySession, myWfd);

            } else if (request == "stop") {
                //cout << "Sending session stop" << endl;
                server->Stop();
#ifdef WIN_BUILD
                return 0;
#else
                return 0;
#endif
            }
        }
    }
#ifdef WIN_BUILD
    return 0;
#else
    return 0;
#endif
*/

   return 0;
}


int
main(int argc, char *argv[])
{
    rtspCallbacks *events = MM_New (cback);
    if(events == NULL)
    {
        //cout<<"events memory allocation failed";
        return -1;
    }
    rtsp_wfd::rtspMode mode = rtsp_wfd::source;

    if (argc != 6) {
        //cout << "Usage: " << argv[0] << " [uri ipaddress] [cfg file] [rtsp port] [uibc port] [mode]" << endl;
        //cout << "mode: 1 (Source), 2 (Primary Sink)" << endl;
        if(events != NULL)
        {
            free(events);
            events = NULL;
        }
        return -1;
    }

    switch(atoi(argv[5])) {
    case 2:
        mode = rtsp_wfd::coupledPrimarySink;
        break;
    case 1:
    default:
        mode = rtsp_wfd::source;
        break;
    }

    rtspServer *server = MM_New_Args(rtspServer, (argv[1], events, argv[2], atoi(argv[3]), atoi(argv[4]), mode));
    if(server == NULL)
    {
        //cout<<"server memory allocation failed";
        return -1;
    }
    if (server->createServer() < 0)
    {
        if(server != NULL)
        {
            free(server);
            server = NULL;
        }
        return -1;
    }

#ifdef WIN_BUILD
    HANDLE threads[THREADCOUNT];
    DWORD threadId1;

    if ((threads[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)commands,
        (void *)server, 0, &threadId1)) == NULL)
        return -1;
#else
    MM_HANDLE thread1;

   if ( ( 0 !=  MM_Thread_CreateEx(0,
                       0,
                       commands,
                       (void *)server ,
                       WFD_RTSPSESSION_THREADS_DEFAULT_STACKSIZE ,
                        "RTSPSourceThread",
                       &thread1)))
   {
        //cout<<"RTSPSession thread creation failed";
        return -1;
   }
#endif

    server->eventLoop();

#ifdef WIN_BUILD
    for (int i = 0; i < THREADCOUNT; i++)
        CloseHandle(threads[i]);
#endif

    RTSP_DELETEIF (server);
    RTSP_DELETEIF (events);
    MM_Thread_Exit(thread1,0);
    return 0;

}
