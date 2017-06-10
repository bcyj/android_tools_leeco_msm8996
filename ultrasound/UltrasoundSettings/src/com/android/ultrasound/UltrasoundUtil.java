/******************************************************************************
 * @file    UltrasoundUtil.java
 * @brief   Holds common functions to be used by all daemons.
 *
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *  ******************************************************************************/

package com.android.ultrasound;

import java.io.*;
import android.util.Log;

public class UltrasoundUtil
{
  private static String logTag = "UltrasoundUtil";
  private static final String pidDirectory = "/data/usf/";
  private static final String pidFileExtention = ".pid";

  /*===========================================================================
  FUNCTION: getCfgFileLocationFromLinkFile
  ===========================================================================*/
  /**
   * getCfgFileLocationFromLinkFile function verifies the given
   * file is a symblic link, and returns its target.
   *
   * @return String null - the given file is not a symbolic link.
   *                else - the target of the given symbolic link.
   */
  public static String getCfgFileLocationFromLinkFile(String cfgFileLocation)
  {
    try
    {
      File cfgFile = new File(cfgFileLocation);
      if (false == isSymbolicLink(cfgFile))
      {
        Log.e(logTag,
              "Link File: " + cfgFile + " is not a symbolic link");
        return null;
      }
      return cfgFile.getCanonicalPath();
    }
    catch (IOException e)
    {
      Log.e(logTag,
            "Error: " + e.getMessage());
      return null;
    }
  }

  /*===========================================================================
  FUNCTION:  isSymbolicLink
  ===========================================================================*/
  /**
   * isSymbolicLink() function checks whether the given file is a
   * symbolic link.
   *
   * @param file The file to check whether is a symbolic link
   *
   * @return Boolean true - the given file is a symbolic link
   *                 false - the given file is not a symbilic
   *                 link.
   */
  private static Boolean isSymbolicLink(File file) throws IOException
  {
    File canon;
    if (null == file.getParent())
    {
      canon = file;
    }
    else
    {
      File canonDir = file.getParentFile().getCanonicalFile();
      canon = new File(canonDir,
                       file.getName());
    }
    return !canon.getCanonicalFile().equals(canon.getAbsoluteFile());
  }

  /*===========================================================================
  FUNCTION:  isCommentLine
  ===========================================================================*/
  /**
    isCommentLine() function receives a line and if the line
    starts with # or empty line return true, else return false.
  */
  public static boolean isCommentLine(String strLine) {
    if (strLine.trim().isEmpty() || '#' == strLine.charAt(0) ||
        ' ' == strLine.charAt(0)) {
      return true;
    }
    return false;
  }

  /*===========================================================================
  FUNCTION:  getPid
  ===========================================================================*/
  /**
   * getPid() function reads the daemon's pid file and returns its
   * pid.
   * @return int the current daemon's pid
   *             -1 in case of an error
   */
  public static int getPid(String daemonName)
  {
    String str="";
    StringBuffer buf = new StringBuffer();
    int retPid;
    BufferedReader reader = null;
    try
    {
      FileInputStream fStream = new FileInputStream(pidDirectory +
                                                    daemonName +
                                                    pidFileExtention);
      reader = new BufferedReader(new InputStreamReader(fStream));
      while (null != (str = reader.readLine()))
      {
        buf.append(str);
      }
    }
    catch (IOException e)
    {
      Log.e(logTag,
            "Daemon pid file does not exist");
      return -1;
    }
    finally
    {
      if (null != reader)
      {
        try
        {
          reader.close();
        }
        catch (IOException e)
        {
          Log.e(logTag,
                "Cannot close pid file");
          return -1;
        }
      }
    }

    try
    {
       retPid = Integer.parseInt(buf.toString());
    }
    catch (NumberFormatException e)
    {
      Log.e(logTag,
            "Daemon pid file does not contain an integer");
      return -1;
    }
    return retPid;
  }
}

