/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;
/**
 * <p>
 * Implements the Myanmar-characters to digits map.
 * </p>
 */
public class DigitMapTableMyanmar extends DigitMapTableLatin{

       private static  DigitMapTableMyanmar sInstance= null;

       private static final char[][] EXTEND_MYANMAR_TABLE = {
           // number 0
           {
              0x1040
           },
           // number 1
           {
               0x1041, 0x102B, 0x102C, 0x102D, 0x102E, 0x102F,
               0X1030, 0X1031, 0X1032, 0X1033, 0X1034, 0X1035,
               0X1036, 0X1037, 0X1038, 0X1039, 0X103A, 0X103B,
               0X103C, 0X103D, 0X103E, 0x104C, 0x104D, 0x104E,
               0x104F
           },
           // number 2
           {
               0x1042, 0x1000, 0x1001, 0x1002, 0x1003, 0x1004,
               0x1005, 0x1006
           },
           // number 3
           {
               0x1043 , 0x1007, 0x1008, 0x1009, 0x100A, 0x100B,
               0x100C
           },
           // number 4
           {
               0x1044,  0x100D, 0x100E, 0x100F, 0x1010, 0x1011
           },
           // number 5
           {
               0x1045, 0x1012, 0x1013, 0x1014, 0x1015, 0x1016,
               0x1017, 0x1018
           },
           // number 6
           {
               0x1046 , 0x1019, 0x101A, 0x101B, 0x101C, 0x101D
           },
           // number 7
           {
               0x1047, 0x101E, 0x101F, 0x1020, 0x1021, 0x1022
           },
           // number 8
           {
               0x1048, 0x1023, 0x1024, 0x1025, 0x1026, 0x1027, 0x1028,
               0x1029
           },
           // number 9
           {
               0x1049, 0x102A, 0x103F
           },
       };
       public static DigitMapTableMyanmar getInstance() {
           if (sInstance == null) {
               sInstance = new DigitMapTableMyanmar();
           }
           return sInstance;
       }

       private DigitMapTableMyanmar() {
       }

       @Override
       public String toDigits(char c) {
           int idx = -1;
           for (int i = 0; i < KEY_NUM; i++) {
               if (contains(EXTEND_MYANMAR_TABLE [i], c)) {
                   idx = i;
                   break;
               }
           }

           if (idx == -1) {
               return super.toDigits(c);
           } else {
               return Integer.toString(idx);
           }
       }
}

