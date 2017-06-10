/*============================================================================
@file SNSRegistryActivity.java

@brief
Fragment to read and write values to the sensors registry.

Copyright (c) 2011-2012 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

package com.qti.sensors.snsregistry;

import java.util.regex.Pattern;

import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.sensors.qsensortest.R;
import com.qualcomm.sensors.sensortest.SensorsReg;

public class SNSRegistryActivity extends Fragment {
   public static final String TAG = "SNSRegistry";
   private static View view;

   /** Called when the activity is first created. */
   @Override
   public View onCreateView (LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
       super.onCreate(savedInstanceState);

       SNSRegistryActivity.view = inflater.inflate(R.layout.registry, null);

       Button readRegistry = (Button) SNSRegistryActivity.view.findViewById(R.id.button_read);
       readRegistry.setOnClickListener(new OnClickListener(){
         @Override
         public void onClick(View v) {
            EditText itemIDField = (EditText) SNSRegistryActivity.view.findViewById(R.id.edit_field_item_id);
            EditText itemLenField = (EditText) SNSRegistryActivity.view.findViewById(R.id.edit_field_item_len);
            EditText itemValField = (EditText) SNSRegistryActivity.view.findViewById(R.id.edit_field_item_val);

            ScrollView logScrollView = (ScrollView)SNSRegistryActivity.view.findViewById(R.id.scroll_view_log);
            TextView logTextView = (TextView) logScrollView.findViewById(R.id.text_view_log);

            int itemID;
            try{
               itemID = Integer.parseInt(itemIDField.getText().toString());
            } catch(NumberFormatException e){
               Toast.makeText(SNSRegistryActivity.this.getActivity(), "Unable to parse item ID", Toast.LENGTH_LONG).show();
               return ;
            }
            if( 0 > itemID || 65535 < itemID ){
               Toast.makeText(SNSRegistryActivity.this.getActivity(), "Item ID range exception", Toast.LENGTH_LONG).show();
               return ;
            }

            int err = SensorsReg.open();
            if(0 != err)
               logTextView.append("Sensor Open failed: " + err + "\n");
            else{
               byte readResult[] = null;

               try{
                  readResult = SensorsReg.getRegistryValue(itemID);
               } catch(Exception e){
                  logTextView.append("Read item " + itemID + " failed: " + e.getLocalizedMessage() + "\n");
               }

               if( null != readResult){
                  itemValField.setText(UtilFunctions.toHexString(readResult));
                  itemLenField.setText(String.valueOf(readResult.length));
                  logTextView.append("");
                  logTextView.append("Read item " + itemID + ": " + UtilFunctions.toHexString(readResult) + "(len: " + readResult.length + ")\n");
               }

               err = SensorsReg.close();
               if(0 != err)
                  logTextView.append("Sensor Close failed: " + err + "\n");
            }
            UtilFunctions.scrollToBottom(logScrollView, 0);
         }
       });

       Button writeRegistry = (Button) SNSRegistryActivity.view.findViewById(R.id.button_write);
       writeRegistry.setOnClickListener(new OnClickListener(){
         @Override
         public void onClick(View v) {
            EditText itemIDField = (EditText) SNSRegistryActivity.view.findViewById(R.id.edit_field_item_id);
            EditText itemLenField = (EditText) SNSRegistryActivity.view.findViewById(R.id.edit_field_item_len);
            EditText itemValField = (EditText) SNSRegistryActivity.view.findViewById(R.id.edit_field_item_val);

            ScrollView logScrollView = (ScrollView)SNSRegistryActivity.view.findViewById(R.id.scroll_view_log);
            TextView logTextView = (TextView) logScrollView.findViewById(R.id.text_view_log);

            int itemID;
            try{
               itemID = Integer.parseInt(itemIDField.getText().toString());
            } catch(NumberFormatException e){
               Toast.makeText(SNSRegistryActivity.this.getActivity(), "Unable to parse item ID", Toast.LENGTH_LONG).show();
               return ;
            }
            if( 0 > itemID || 65535 < itemID ){
               Toast.makeText(SNSRegistryActivity.this.getActivity(), "Item ID range exception", Toast.LENGTH_LONG).show();
               return ;
            }

            byte itemLen;
            try{
               itemLen = Byte.parseByte(itemLenField.getText().toString());
            } catch(NumberFormatException e){
               Toast.makeText(SNSRegistryActivity.this.getActivity(), "Unable to parse item length", Toast.LENGTH_LONG).show();
               return ;
            }

            String itemVal = itemValField.getText().toString();
            if(!Pattern.matches("[0-9a-zA-Z]{" + itemLen*2 + "}", itemVal)){
               Toast.makeText(SNSRegistryActivity.this.getActivity(), "Invalid item value", Toast.LENGTH_LONG).show();
               return ;
            }

            int err = SensorsReg.open();
            if(0 != err)
               logTextView.append("Sensor Open failed: " + err + "\n");
            else{
               int writeResult = SensorsReg.setRegistryValue(itemID, UtilFunctions.toByteArray(itemVal), itemLen);

               logTextView.append("Write to item " + itemID + ": " + itemVal
                     + "(len: " + itemLen + ") " + (writeResult == 0 ? " Succeeded" : " Failed (err " + writeResult + ")") + "\n");

               err = SensorsReg.close();
               if(0 != err)
                  logTextView.append("Sensor Close failed: " + err + "\n");
            }
            UtilFunctions.scrollToBottom(logScrollView, 0);
         }
       });

       return SNSRegistryActivity.view;
   }
}
