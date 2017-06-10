/***************************************************************************
 *                             RTSPStringStream.h
 * DESCRIPTION
 *  RTSPStringStream class header file
 *
 * Copyright (c)  2013 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

#ifndef RTSPStringStream_H
#define RTSPStringStream_H

#include <string>
#include <math.h>
#include <cctype>

using namespace std;

class setw;
class setfill;

class RTSPStringStream
{
private:
   string s;
   int base;
   int wide;
   char fillch;
   int readIndex;
   int writeIndex;
   bool err;
   RTSPStringStream& append(string);
   void construct(string);
   string extractString();
   long long int extractNum();
   string to_string(long long int);

public:
   string str();
   void str(const string&);
   char fill(char);
   int width(int w);

   RTSPStringStream();
   RTSPStringStream(string);
   RTSPStringStream& operator<<(string);
   RTSPStringStream& operator<<(long long int);
   RTSPStringStream& operator<<(unsigned char *);
   RTSPStringStream& operator<<(RTSPStringStream& (RTSPStringStream&));
   RTSPStringStream& operator<<(setw);
   RTSPStringStream& operator<<(setfill);

   RTSPStringStream& operator>>(string&);
   RTSPStringStream& operator>>(long long int&);
   RTSPStringStream& operator>>(long long unsigned int&);
   RTSPStringStream& operator>>(int&);
   RTSPStringStream& operator>>(unsigned int&);
   RTSPStringStream& operator>>(RTSPStringStream& (RTSPStringStream&));

   operator void*() const { if (err) return 0; else return (void*)this; }

   friend RTSPStringStream& dec(RTSPStringStream&);
   friend RTSPStringStream& hex(RTSPStringStream&);
   friend RTSPStringStream& oct(RTSPStringStream&);
};

class setw
{
public:
   setw(int);
private:
   int val;

   friend class RTSPStringStream;
};

class setfill
{
public:
   setfill(char);
private:
   char val;

   friend class RTSPStringStream;
};


extern RTSPStringStream& dec(RTSPStringStream&);
extern RTSPStringStream& hex(RTSPStringStream&);
extern RTSPStringStream& oct(RTSPStringStream&);

#endif /* RTSPStringStream_H */
