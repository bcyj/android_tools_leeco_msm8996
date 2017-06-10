/**************************************************************************
 *
 * Copyright (c) 2011 - 2012 Qualcomm Technologies, Inc., All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 *
 **************************************************************************/

#include "rtsp_client.h"
#include <stdlib.h>
#include <stdio.h>
#include "MMThread.h"
#include <string>


#define WFD_RTSPSESSION_THREADS_DEFAULT_STACKSIZE 262144
#define THREADCOUNT 1

using namespace std;
SESSION mySession;

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
}

void cback::getCallback(rtspApiMesg &mesg)
{
    //cout << "Callback: Received Get Parameters" << endl;
    printMesg(mesg);
    mesg.wfd.dump();
}

void cback::setCallback(rtspApiMesg &mesg)
{
    //cout << "Callback: Received Set Parameters" << endl;
    printMesg(mesg);
    mesg.wfd.dump();
    /*
     * Modify wfd parameters here
     */
}

/*
 * Used to override the WFD parameters before
 * RTSP begins negotiation
 */
void cback::openCallback(rtspApiMesg &mesg)
{
    //static unsigned char data[] = "qwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyui";

    //cout << "Callback: Received Open" << endl;

    //mesg.wfd.edid.setPayload(2, data);

    printMesg(mesg);
    mesg.wfd.dump();
    /*
     * Modify wfd parameters here
     */
}

/*
 *
 *
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
    mySession = mesg.session;
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
    rtspClient *client = (rtspClient *)datap;
    rtspApiMesg mesg;
    string request;
/*
    while (1) {
        while (cin >> request) {
            //cout << "Received " << request << endl;
            if (request == "play") {
                //cout << "Sending play" << endl;
                client->Play(mySession);
            } else if (request == "pause") {
                //cout << "Sending pause" << endl;
                client->Pause(mySession);
            } else if (request == "teardown") {
                //cout << "Sending teardown" << endl;
                client->Teardown(mySession);
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
                myWfd.idrReq.setValid(true);
#endif
                //cout << "Sending set" << endl;
                client->Set(mySession, myWfd);
            } else if (request == "get") {
                // Example for post-session establishment set command
                rtspWfd myWfd;
                myWfd.client.setValid(true);

                //cout << "Sending get" << endl;
                client->Get(mySession, myWfd);
            } else if (request == "standby") {
                rtspWfd myWfd;

                myWfd.halt.setValid(true);

                //cout << "Sending standby" << endl;
                client->Set(mySession, myWfd);

            } else if (request == "resume") {
                rtspWfd myWfd;

                myWfd.start.setValid(true);

                //cout << "Sending resume" << endl;
                client->Set(mySession, myWfd);

            } else if (request == "stop") {
                //cout << "Sending session stop" << endl;
                client->Stop();
#ifdef WIN_BUILD
                return 0;
#else
                return NULL;
#endif
            }
        }
    }
*/
    return 0;
}

int
main(int argc, char *argv[])
{
    rtspCallbacks *events = MM_New (cback);
    if(events == NULL)
    {
        //cout<<"RTSP_LIB :: events memory allocation failed";
        return -1;
    }
    rtsp_wfd::rtspMode mode = rtsp_wfd::sink;
    string mac = "";

    if (argc != 7 && argc != 8) {
        //cout << "Usage: " << argv[0] << " [ip address] [rtp port0] [rtp port1] [cfg file] [rtsp port] [mode] [coupled mac address] [hdcp_port]" << endl;
        //cout << "mode: 1 (Sink), 2 (Primary Sink), 3 (Secondary Sink)" << endl;
        if(events != NULL)
        {
            free(events);
            events = NULL;
        }
        return -1;
    }

    switch(atoi(argv[6])) {
    case 2:
        mode = rtsp_wfd::coupledPrimarySink;
        if (argc != 8) {
            //cout << "Error: No Secondary Sink Mac Address" << endl;
            //cout << "Picking up MAC address from sink.xml" << endl;
            break;
        }
        mac = argv[7];
        if (mac.length() != MAC_ADDR_LEN) {
            //cout << "Error: Incorrect length for Secondary Sink Mac Address" << endl;
            if(events != NULL)
            {
                free(events);
                events = NULL;
            }
            return -1;
        }
        break;
    case 3:
        mode = rtsp_wfd::coupledSecondarySink;
        if (argc != 8) {
            //cout << "Error: No Primary Sink Mac Address" << endl;
            //cout << "Picking up MAC address from sink.xml" << endl;
            break;
        }
        mac = argv[7];
        if (mac.length() != MAC_ADDR_LEN) {
            //cout << "Error: Incorrect length for Primary Sink Mac Address" << endl;
            if(events != NULL)
            {
                free(events);
                events = NULL;
            }
            return -1;
        }
        break;
    case 1:
    default:
        mode = rtsp_wfd::sink;
        mac = "";
        break;
    }

    string ipaddr(argv[1]);
    rtspClient *client = MM_New_Args( rtspClient,(atoi(argv[2]), atoi(argv[3]), atoi(argv[7]),events, argv[4], atoi(argv[5]), mode, mac));
    if(client == NULL)
    {
        //cout<<"memory allocation failed";
        return -1;
    }
    if (client->startClient(ipaddr) < 0)
    {
        if(client != NULL)
        {
            free(client);
            client = NULL;
        }
        return -1;
    }

#ifdef WIN_BUILD
    HANDLE threads[THREADCOUNT];
    DWORD threadId1;

    if ((threads[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)commands,
        (void *)client, 0, &threadId1)) == NULL)
        return -1;
#else
    MM_HANDLE thread1;
    if ( ( 0 !=  MM_Thread_CreateEx(0,
                       0,
                       commands,
                       (void *)client  ,
                       WFD_RTSPSESSION_THREADS_DEFAULT_STACKSIZE ,
                       "RTSPSinkThread",
                       &thread1)))
   {
        //cout<<"RTSPSession thread creation failed";
        return -1;
   }
#endif

    client->eventLoop();

#ifdef WIN_BUILD
    for (int i = 0; i < THREADCOUNT; i++)
        CloseHandle(threads[i]);
#endif

    RTSP_DELETEIF(client);
    RTSP_DELETEIF(events);
    MM_Thread_Exit(thread1,0);
    return 0;
}
