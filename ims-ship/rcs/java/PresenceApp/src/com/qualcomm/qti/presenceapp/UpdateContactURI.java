/*************************************************************************
 Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
 *************************************************************************/

package com.qualcomm.qti.presenceapp;

import java.util.ArrayList;


import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class UpdateContactURI extends Activity {

    private TextView URI;

    private Button applyPrefix;
    private Button applysuffix;
    private Button applyToAll;

    private EditText prifix;
    private EditText suffix;
    private Bundle extras;

    private int contactIndex = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.setcontacturi);

        extras = getIntent().getExtras();
        contactIndex = extras.getInt("INDEX");

        URI = (TextView) findViewById(R.id.uri);

        prifix = (EditText) findViewById(R.id.editPrefix);
        suffix = (EditText) findViewById(R.id.editSuffix);

        applyPrefix = (Button) findViewById(R.id.applyprefix);
        applyPrefix.setText("Apply Prefix");

        applysuffix = (Button) findViewById(R.id.applysuffix);
        applysuffix.setText("Apply Suffix");

        applyToAll = (Button) findViewById(R.id.seturitoall);
        applyToAll.setText("Apply To All");

        handleURITextFromLast();
        handleApplyprefixButton();
        handleApplysuffixButton();
        handleApplyToALLButton();

    }

    void handleURITextFromLast()
    {
        Contact c;
        ArrayList<Contact> contacts = AppGlobalState.getContacts();
        c = contacts.get(contactIndex);
        if (c.getIndividualContactURI().equals(""))
        {
            URI.setText(c.getPhone());
        }
        else
        {
            URI.setText(c.getIndividualContactURI());
            prifix.setText(prifix(c.getIndividualContactURI()));
            suffix.setText(suffix(c.getIndividualContactURI()));
        }
    }

    void handleURIText()
    {
        Contact c;
        ArrayList<Contact> contacts = AppGlobalState.getContacts();
        c = contacts.get(contactIndex);
        String URIString = prifix.getText().toString() + c.getPhone() + suffix.getText().toString();
        URI.setText(URIString);
        c.setIndividualContactURI(URIString);
    }

    void handleURITextPrifix()
    {
        Contact c;
        ArrayList<Contact> contacts = AppGlobalState.getContacts();
        c = contacts.get(contactIndex);
        String URIString = prifix.getText().toString() + c.getPhone()
                + suffix(c.getIndividualContactURI());
        URI.setText(URIString);
        c.setIndividualContactURI(URIString);
    }

    void handleURITextSuffix()
    {
        Contact c;
        ArrayList<Contact> contacts = AppGlobalState.getContacts();
        c = contacts.get(contactIndex);
        String URIString = prifix(c.getIndividualContactURI()) + c.getPhone()
                + suffix.getText().toString();
        URI.setText(URIString);
        c.setIndividualContactURI(URIString);
    }

    void handleURITextToAll()
    {
        Contact c;
        ArrayList<Contact> contacts = AppGlobalState.getContacts();
        for (int i = 0; i < contacts.size(); i++)
        {
            c = contacts.get(i);
            String URIString = prifix.getText().toString() + c.getPhone()
                    + suffix.getText().toString();
            c.setIndividualContactURI(URIString);
        }

        SharedPreferences preferences = this.getSharedPreferences(
                "presencedata", Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putString("INDIVIDUAL_PRIFIX", prifix.getText().toString());
        editor.putString("INDIVIDUAL_SUFFIX", suffix.getText().toString());
        editor.commit();
    }

    void handleApplyprefixButton()
    {
        applyPrefix.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {
                handleURITextPrifix();
            }
        });
    }

    void handleApplysuffixButton()
    {
        applysuffix.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {
                handleURITextSuffix();
            }
        });
    }

    void handleApplyToALLButton()
    {
        applyToAll.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {
                handleURIText();
                handleURITextToAll();
            }
        });
    }

    String prifix(String URI)
    {
        StringBuffer prifix = new StringBuffer();
        for (int i = 0; i < URI.length(); i++)
        {
            if (URI.charAt(i) == ':')
            {
                prifix.append(URI.charAt(i));
                break;
            }
            else
            {
                prifix.append(URI.charAt(i));
            }
        }
        return prifix.toString();
    }

    String suffix(String URI)
    {
        StringBuffer suffix = new StringBuffer();
        for (int i = 0; i < URI.length(); i++)
        {
            if (URI.charAt(i) == '@')
            {
                suffix.append(URI.substring(i, URI.length()));
                break;
            }
        }
        return suffix.toString();
    }
}
