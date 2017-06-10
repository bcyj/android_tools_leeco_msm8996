/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;

/**
 * <p>
 * Implements the Malayalam-characters to digits map.
 * </p>
 */
public class DigitMapTableMalayalam extends DigitMapTableLatin{

    private static DigitMapTableMalayalam sInstance = null ;

    private static final char[][] EXTEND_MYLAYALAM_TABLE ={
      // number 0
        {
            0x0D66
        },
      // number 1
        {
            0x0D67,  0x0D02,  0x0D03,  0x0D3E,  0x0D3F,  0x0D40,
            0x0D41,  0x0D42,  0x0D43,  0x0D44,  0x0D46,  0x0D47,
            0x0D48,  0x0D4A , 0x0D4B,  0x0D4C , 0x0D4D,  0x0D57,
            0x0D62,  0x0D63
        },
      // number 2
        {
            0x0D68,  0x0D05,  0x0D06,  0x0D07,  0x0D08,  0x0D09,
            0x0D0A,  0x0D0B,  0x0D60,
        },
      // number 3
        {
            0x0D69,  0x0D0C,  0x0D61,  0x0D0E,  0x0D0F,  0x0D10,
            0x0D12,  0x0D13,  0x0D14
        },
      // number4
        {
            0x0D6A,  0x0D15,  0x0D16,  0x0D17,  0x0D18,  0x0D19,
            0x0D1A
        },
      // number 5
        {
            0x0D6B,  0x0D1B,  0x0D1C,  0x0D1D,  0x0D1E,  0x0D1F,
            0x0D20
        },
      // number 6
        {
            0x0D6C,  0x0D21,  0x0D22,  0x0D23,  0x0D24,  0x0D25,
            0x0D26
        },
      // number 7
        {
            0x0D6D,  0x0D27, 0x0D28,  0x0D2A,  0x0D2B,  0x0D2C,
            0x0D2D
        },
      // number 8
        {
            0x0D6E,  0x0D2E,  0x0D2F,  0x0D30,  0x0D31,  0x0D32,
            0x0D33,  0x0D34
        },
      // number 9
        {
            0x0D6F,  0x0D35,  0x0D36,  0x0D37,  0x0D38,  0x0D39,
            0x0D3D
        },
    };


    public static DigitMapTableMalayalam getInstance(){
        if(sInstance == null){
            sInstance = new DigitMapTableMalayalam();
        }
        return sInstance;

    }


   private  DigitMapTableMalayalam() {

   };

   @Override
   public String toDigits(char c){
       int idx = -1;
       for(int i =0;i < KEY_NUM;i++){
           if(contains(EXTEND_MYLAYALAM_TABLE[i], c)){
               idx = i;
               break;
           }
       }

       if(idx == -1){
           return super.toDigits(c);
       }else{
           return Integer.toString(idx);
       }
   }

}
