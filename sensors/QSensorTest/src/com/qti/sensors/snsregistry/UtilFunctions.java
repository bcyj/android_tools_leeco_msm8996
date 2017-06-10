/*============================================================================
@file UtilFunctions.java

@brief
Utility functions for the SNSRegistry application.

Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

package com.qti.sensors.snsregistry;

import android.widget.ScrollView;

final public class UtilFunctions {
   public static String toHexString(byte[] bytes){
      StringBuffer sb = new StringBuffer(bytes.length*2);
      for(int i = bytes.length - 1; i >= 0 ; i--){
          sb.append(toHex(bytes[i] >> 4));
          sb.append(toHex(bytes[i]));
      }

      return sb.toString();
  }
  public static String toHexString(byte input){
      StringBuffer sb = new StringBuffer(2);
      sb.append(toHex(input >> 4));
      sb.append(toHex(input));
      return sb.toString();
  }

  private static char toHex(int input){
      final char[] hexDigit = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
      return hexDigit[input & 0xF];
  }

  public static byte[] toByteArray(String input) {
     byte[] output = new byte[input.length() / 2];
     for (int i = 0; i < output.length; i++) {
        output[output.length - i - 1] = (byte)Integer.parseInt(input.substring(i * 2, i * 2 + 2), 16);
     }
     return output;
  }
  public static void scrollToBottom(final ScrollView scrollView, int delay) {
     scrollView.postDelayed(new Runnable() {
         public void run() {
            scrollView.fullScroll(ScrollView.FOCUS_DOWN);
         }
     }, delay);
  }
}
