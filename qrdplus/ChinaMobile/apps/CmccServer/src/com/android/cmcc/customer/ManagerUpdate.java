/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.android.cmcc.customer;

import com.android.cmcc.R;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.text.InputType;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import android.text.SpannableStringBuilder;
import android.telephony.PhoneNumberFormattingTextWatcher;
import android.text.Editable;
import android.text.method.DialerKeyListener;

public class ManagerUpdate extends Activity {
      private static final String TAG = "ManagerUpdate";
      private final boolean DBG =false;
      private View mBottom;
      private Button mBtnSave = null;
      private Button mBtnCanel = null;
      private View.OnClickListener mCancelListener;
      private View.OnClickListener mSaveListener;
      private EditText nameedit;
      private Intent passintent;
      private EditText phoneedit;
      private String oldName;
      private String oldPhone;

      @Override
      protected void onCreate(Bundle savedInstanceState) {
          super.onCreate(savedInstanceState);
          setContentView(R.layout.manager_update_view);
          setTitle(R.string.setting);
          this.passintent = getIntent();
          this.phoneedit = (EditText)findViewById(R.id.edit_managerphone);;
          if (phoneedit != null) {
              phoneedit.setKeyListener(DialerKeyListener.getInstance());
              phoneedit.addTextChangedListener(new PhoneNumberFormattingTextWatcherImpl());
          }

          if (this.passintent != null){
            String name = this.passintent.getExtras().getString("managername");
            this.oldName =name;
            this.nameedit =  (EditText)findViewById(R.id.edit_managername);
            this.nameedit.setText(name);
            String phone= this.passintent.getExtras().getString("managerphone");
            this.oldPhone =phone;
            this.phoneedit.setText(phone);
            this.phoneedit.setInputType(InputType.TYPE_CLASS_PHONE);
          }
      }

      public boolean onCreateOptionsMenu(Menu menu){
            return super.onCreateOptionsMenu(menu);
      }

      @Override
      public boolean onOptionsItemSelected(MenuItem item) {
               String name = this.nameedit.getText().toString();
              String  phone= this.phoneedit.getText().toString();
                switch (item.getItemId())
                  {
                  case 1:
                  if ((isNameEmpty(name)) || (isNameEmpty(phone)))
                      {
                      if (DBG)  Log.i("ManagerUpdate", " name or phone is empty");
                        Toast.makeText(this, R.string.can_not_save, Toast.LENGTH_LONG).show();
                      }else{
                      saveManagerInfo();
                      }
                      break;
                  case 2:
                          setResult(0);
                          finish();
                      break;
                  default:
                      break;
                  }
              return super.onOptionsItemSelected(item);
      }

      @Override
      public boolean onKeyDown(int keyCode, KeyEvent event) {
          if (DBG)Log.i("ManagerUpdate", "__onKeyDown___"+keyCode);
          String name = this.nameedit.getText().toString();
          String  phone= this.phoneedit.getText().toString();

          if (keyCode == 4) {
              if ((!(isNameEmpty(name)) || !(isNameEmpty(phone)))) {
                  if (DBG)Log.i("ManagerUpdate start saveManagerInfo", "__onKeyDown___" + keyCode);
                  saveManagerInfo();
              }
              else{
                  if (DBG)Log.i("ManagerUpdate Empty Cancle", "__onKeyDown___" + keyCode);
                  finish();
              }
              return true;
          } else {
              return super.onKeyDown(keyCode, event);
          }
      }

      public boolean isNameEmpty(String paramString){
          if ("".equals(paramString))
            return true;
          else
            return false;
      }

      private void saveManagerInfo() {
            String name = this.nameedit.getText().toString();
          String  phone= this.phoneedit.getText().toString();

        if (DBG) Log.i("ManagerUpdate", " name and phone are not  empty");

        if ((this.oldName.compareTo(name) == 0)&&(this.oldPhone.compareTo(phone) == 0)){
             if (DBG) Log.i("ManagerUpdate", " name and phone are same as before");
             setResult(0);
             finish();
             return;
        }else{
            if (DBG)Log.i("ManagerUpdate", "name and  same are different from before ");

            SharedPreferences.Editor localEditor = getSharedPreferences("managerinfo", 0).edit();
            localEditor.putString("managername", name);
            localEditor.putString("managerphone", phone);
            localEditor.commit();
            Toast.makeText(this, R.string.save_notify, 0).show();
            Intent localIntent = new Intent();
            Uri localUri = Uri.parse("***saved manager**");
            localIntent.setData(localUri);
            setResult(-1, localIntent);
            finish();
        }
      }

      @Override
      protected void onResume() {
          SharedPreferences sp = getSharedPreferences("managerinfo", 0);
          this.nameedit.setText(sp.getString("managername", ""));
          this.phoneedit.setText(sp.getString("managerphone", ""));
          super.onResume();
      }
    private class PhoneNumberFormattingTextWatcherImpl extends PhoneNumberFormattingTextWatcher {
          SpannableStringBuilder mAppend;
          public synchronized void afterTextChanged(Editable text) {
            super.afterTextChanged(text);
            mAppend = new SpannableStringBuilder();
            mAppend.append('W');
            mAppend.append('P');
            for(int i = 0;i<text.length();i++)
            {
                if(text.charAt(i) == ';')
                {
                    text.replace(i, i+1, mAppend,0,1);
                }
                else if(text.charAt(i) == ',')
                {
                    text.replace(i, i+1, mAppend,1,2);
                }
            }
          }
      }
    }
