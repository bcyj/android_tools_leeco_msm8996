/***************************************************************************
 *                             RTSPStringStream.cpp
 * DESCRIPTION
 *  RTSPStringStream class implementation, with only some subset of the features
 *  from the standard C++ library
 *
 * Copyright (c)  2013 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

#include "RTSPStringStream.h"
#include <string>
#include <math.h>
#include <cctype>

using namespace std;

/**
 * Constructor helper for RTSPStringStream class.
 */
void RTSPStringStream::construct(string str)
{
   s = str;
   fillch = ' ';
   base = 10;
   wide = 0;
   readIndex = 0;
   writeIndex = 0;
   err = false;
}

/**
 * RTSPStringStream constructor that takes in a string.
 */
RTSPStringStream::RTSPStringStream(string str)
{
   construct(str);
}

/**
 * RTSPStringStream default constructor.
 */
RTSPStringStream::RTSPStringStream()
{
   construct("");
}

/**
 * Get the string of this RTSPStringStream.
 */
string RTSPStringStream::str()
{
   return s;
}

/**
 * Set the string of this RTSPStringStream.
 */
void RTSPStringStream::str(const string& new_str)
{
   s = new_str;
   readIndex = 0;
   writeIndex = 0;
   err = false;
}

/**
 * Private helper function to extract the string from current.
 * readIndex until the next whitespace.
 */
string RTSPStringStream::extractString()
{
   if ((unsigned int)readIndex >= s.length())
      return "";

   int start = -1, end = -1;

   for (unsigned int i = (unsigned int)readIndex; i < s.length(); i++)
   {
      if (start == -1 && !isspace(s[i]))
         start = i;
      if (start != -1 && isspace(s[i]))
      {
         end = i;
         readIndex = i+1;
         break;
      }
   }

   if (start != -1 && end != -1)
      return s.substr(start, end - start);
   else if (start != -1)
   {
      readIndex = (int)s.length();
      return s.substr(start);
   }
   else
      return "";
}

/**
 * Private helper function to convert a number to a string.
 */
string RTSPStringStream::to_string(long long int a)
{
   if (a == 0)
      return "0";

   string tmp;
   string res;
   if (a < 0)
   {
      a = 0 - a;
      res = res + "-";
   }
   while (a != 0)
   {
      char digit;
      if (a % base < 10)
         digit = static_cast<char>('0' + (char)(a % base));
      else
         digit = static_cast<char>('a' + (char)((a % base) - 10));
      tmp = tmp + digit;
      a = a / base;
   }

   for (int i = (int)tmp.length() - 1; i >= 0; i--)
      res = res + tmp[i];

   return res;
}

/**
 * Concatenate a string to this RTSPStringStream.
 */
RTSPStringStream& RTSPStringStream::operator<<(string a)
{
   return append(a);
}

/**
 * Concatenate a number to this RTSPStringStream.
 */
RTSPStringStream& RTSPStringStream::operator<<(long long int a)
{
   string tmp = this->to_string(a);
   return append(tmp);
}

/**
 * Concatenate the string pointed by 'ptr' to this RTSPStringStream.
 */
RTSPStringStream& RTSPStringStream::operator<<(unsigned char * ptr)
{
   return append(string((char *)ptr));
}

/**
 * Handles manipulators
 */
RTSPStringStream& RTSPStringStream::operator<<(RTSPStringStream& (*pf)(RTSPStringStream&))
{
   pf(*this);
   return *this;
}

/**
 * Set decimal as the number format.
 */
RTSPStringStream& dec(RTSPStringStream& ss)
{
   ss.base = 10;
   return ss;
}

/**
 * Set hexadecimal as the number format.
 */
RTSPStringStream& hex(RTSPStringStream& ss)
{
   ss.base = 16;
   return ss;
}

/**
 * Set octal as the number format.
 */
RTSPStringStream& oct(RTSPStringStream& ss)
{
   ss.base = 8;
   return ss;
}

/**
 * Set the fill character.
 */
char RTSPStringStream::fill(char c)
{
   char tmp = fillch;
   fillch = c;
   return tmp;
}

/**
 * Set the width for the next string that will be concatenated.
 */
int RTSPStringStream::width(int w)
{
   int tmp = wide;
   wide = w;
   return tmp;
}

/**
 * Private helper function to append string 'a' to the end of
 * this RTSPStringStream.
 */
RTSPStringStream& RTSPStringStream::append(string a)
{
   string res = "";

   if (wide > 0)
   {
      wide -= (int)a.length();
      while (wide > 0) {
         res = res + fillch;
         wide--;
      }
   }

   res = res + a;

   /*
    * This is used to imitate the original RTSPStringStream behavior, although
    * it's weird.
    *
    * Here is an example:
    *
    * RTSPStringStream ss("Hello my name is Alex");
    * cout << ss.str(); // prints "Hello my name is Alex"
    * ss << "Par";
    * cout << ss.str(); // prints "Parlo my name is Alex"
    * ss << "ty";
    * cout << ss.str(); // prints "Party my name is Alex"
    */
   if (writeIndex + res.length() < s.length())
      s = s.substr(0, writeIndex) + res + s.substr(writeIndex + res.length());
   else if ((unsigned int)writeIndex < s.length())
      s = s.substr(0, writeIndex) + res;
   else
      s = s + res;

   writeIndex += (int)res.length();

   return *this;
}

/**
 * Handles width manipulator.
 */
RTSPStringStream& RTSPStringStream::operator<<(setw w)
{
   this->width(w.val);
   return *this;
}

/**
 * Handles fill char manipulator.
 */
RTSPStringStream& RTSPStringStream::operator<<(setfill f)
{
   this->fill(f.val);
   return *this;
}

/**
 * Handles manipulators.
 */
RTSPStringStream& RTSPStringStream::operator>>(RTSPStringStream& (*pf)(RTSPStringStream&))
{
   *this << pf;
   return *this;
}

/**
 * Private helper function to extract a number from this
 * RTSPStringStream.
 */
long long int RTSPStringStream::extractNum()
{
   string str = extractString();

   int exp = 0;
   long long int num = 0;

   for (int i = (int)str.length()-1; i>= 0; i--)
   {
      char c = static_cast<char>(tolower(str[i]));
      int x;

      if (c == '-')
      {
         num = 0 - num;
         break;
      }
      else if (c >= '0' && c <= '9')
         x = c - '0';
      else if (c >= 'a' && c <= 'f')
         x = c - 'a' + 10;
      else
      {
         num = 0;
         err = true;
         break;
      }

      int factor = (int)round(pow(base, exp));
      num += factor * x;
      exp++;
   }

   return num;
}

/**
 * Extract a string from current readIndex until the next
 * whitespace and put it into 'str'.
 */
RTSPStringStream& RTSPStringStream::operator>>(string &str)
{
   str = extractString();
   if (str.length() == 0)
      err = true;
   return *this;
}

/**
 * Extract a number from current readIndex until the next
 * whitespace and put it into 'num'.
 */
RTSPStringStream& RTSPStringStream::operator>>(long long int &num)
{
   num = extractNum();
   return *this;
}

/**
 * Extract a number from current readIndex until the next
 * whitespace and put it into 'num'.
 */
RTSPStringStream& RTSPStringStream::operator>>(long long unsigned int &num)
{
   num = extractNum();
   return *this;
}

/**
 * Extract a number from current readIndex until the next
 * whitespace and put it into 'num'.
 */
RTSPStringStream& RTSPStringStream::operator>>(unsigned int &num)
{
   num = static_cast<unsigned int>(extractNum());
   return *this;
}

/**
 * Extract a number from current readIndex until the next
 * whitespace and put it into 'num'.
 */
RTSPStringStream& RTSPStringStream::operator>>(int &num)
{
   num = static_cast<int>(extractNum());
   return *this;
}

/**
 * Constructor for setw. This will be used as a manipulator for
 * RTSPStringStream.
 */
setw::setw(int x)
{
   val = x;
}

/**
 * Constructor for setfill. This will be used as a manipulator
 * for RTSPStringStream.
 */
setfill::setfill(char c)
{
   val = c;
}
