/*
 *Copyright (c) 2014 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.confdialer.conference;

import android.app.Fragment;
import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;

import android.content.Intent;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.net.Uri;
import android.telephony.PhoneNumberUtils;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.style.TypefaceSpan;
import android.text.Editable;
import android.text.InputFilter;
import android.text.InputFilter.LengthFilter;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.text.method.TextKeyListener;
import android.text.style.URLSpan;
import android.text.util.Linkify;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ViewGroup;
import android.view.View;
import android.view.View.OnKeyListener;
import android.view.ViewStub;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.provider.ContactsContract;
import android.provider.ContactsContract.QuickContact;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Intents;
import com.android.contacts.common.CallUtil;
import com.android.contacts.common.activity.TransactionSafeActivity;
import android.widget.AdapterView;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.Button;
import android.util.Log;
import android.os.Handler;
import android.os.Bundle;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.util.Set;
import com.qualcomm.qti.confdialer.R;
import com.qualcomm.qti.confdialer.conference.ChipsRecipientAdapter;
import com.qualcomm.qti.confdialer.conference.RecipientsEditor;
import com.qualcomm.qti.confdialer.data.Contact;
import com.qualcomm.qti.confdialer.data.ContactList;

public class ConferenceCallActivity extends TransactionSafeActivity
        implements View.OnClickListener {
    private static final String TAG = "ConferenceCallActivity";
    public static final int REQUEST_CODE_PICK = 101;
    public static final String EXTRA_DIAL_CONFERENCE_URI =
            "org.codeaurora.extra.DIAL_CONFERENCE_URI";
    public static final String EXTRA_START_CONFERENCE_WITH_NUMBERS =
            "confernece_number_key";
    public static final String ADD_PARTICIPANT_KEY = "add_participant";
    public static final String CURRENT_PARTICIPANT_LIST = "current_participant_list";
    private boolean isAddParticipants = false;
    private String currentParticipants;
    private Intent mIntent;
    private RecipientsEditor mRecipientsEditor;
    private ImageButton mRecipientsPicker;
    private ImageButton mRecipientsPickerGroups;
    private static int RecipientLimit =5;
    private int mExistsRecipientsCount = 0;
    private boolean mIsPickingContact = false;
    private ContactList mRecipientsPickList = null;
    private boolean mIsProcessPickedRecipients = false;
    private boolean mIsProcessRecipientsFromIntent = false;
    private Button mStartCallButton;
    private TextView mRecipientsNumber;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.conference_call_activity);

        getActionBar().setDisplayHomeAsUpEnabled(true);

        Contact.init(this);
        initRecipientsEditor();
        mExistsRecipientsCount = recipientCount();
    }

    // Get the recipients editor ready to be displayed onscreen.
    private void initRecipientsEditor() {

        if (isRecipientsEditorVisible()) {
            return;
        }
        mIsProcessRecipientsFromIntent = false;
        RecipientLimit = 5;

        ContactList recipients = getRecipients();

        mRecipientsPicker = (ImageButton) findViewById(R.id.contacts);
        mRecipientsPicker.setVisibility(View.VISIBLE);
        mRecipientsPicker.setOnClickListener(this);

        mRecipientsEditor = (RecipientsEditor) findViewById(R.id.recipients_editor);
        mRecipientsEditor.setVisibility(View.VISIBLE);

        mRecipientsEditor.setAdapter(new ChipsRecipientAdapter(this));
        mRecipientsEditor.addTextChangedListener(mRecipientsWatcher);
        mRecipientsEditor.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                if (!hasFocus) {
                    RecipientsEditor editor = (RecipientsEditor) v;
                    ContactList contacts = editor.constructContactsFromInput(false);
                    updateRecipientNumber(contacts);
                    Log.e(TAG, "onFocusChange, updateRecipientNumber");
                }
            }
        });

        mRecipientsNumber = (TextView) findViewById(R.id.contacts_number_left);
        mStartCallButton = (Button) findViewById(R.id.start_conference_call);
        mStartCallButton.setVisibility(View.VISIBLE);
        mStartCallButton.setOnClickListener(this);

        mIntent = getIntent();

        isAddParticipants = mIntent.getBooleanExtra(ADD_PARTICIPANT_KEY, false);
        if (isAddParticipants) {
            initAddParticipants();
        } else {
            initConferenceCall();
        }

        updateStartCallButtonState();
    }

    //Process start conference call with numbers.
    private void initConferenceCall() {
        String number = mIntent.getStringExtra(EXTRA_START_CONFERENCE_WITH_NUMBERS);
        Log.e(TAG, "initConferenceCall number = " + number);
        if (number != null) {
            String[] numbers = number.split(";");
            if (numbers.length > 0) {
                Set<String> numbersSet = new HashSet<String>();
                for (int k=0; k<numbers.length ; k++){
                    String regEx="[^0-9]";
                    Pattern p = Pattern.compile(regEx);
                    Matcher m = p.matcher(numbers[k]);
                    numbers[k] = m.replaceAll("").trim();
                    Log.e(TAG, "initConferenceCall number_ = " + numbers[k]);
                    numbersSet.add(numbers[k]);
                }
                processPickResult2(numbersSet);
            }
        }
    }

    //process for add Recipients to an existing conference call
    private void initAddParticipants(){
        currentParticipants = mIntent.getStringExtra(CURRENT_PARTICIPANT_LIST);
        mIsProcessRecipientsFromIntent = true;
        Log.d(TAG, "currentParticipantList" + currentParticipants);
        if (currentParticipants != null){
            String[] currentParticipantList = currentParticipants.split(";");
            if (currentParticipantList.length > 0) {
                Set<String> currentParticipantSet = new HashSet<String>();
                for (int k=0; k<currentParticipantList.length ;k++){
                    if (currentParticipantList[k].contains("@")){
                        String[] temp = currentParticipantList[k].split("@");
                        currentParticipantList[k] = temp[0];
                    }
                    currentParticipantSet.add(currentParticipantList[k]);
                }
                //processAddRecipients(currentParticipantSet,
                        //currentParticipantList.length);
            }
            RecipientLimit = RecipientLimit - (currentParticipantList.length-1);
            int numberLeft = RecipientLimit - mExistsRecipientsCount;
            mRecipientsNumber.setText(Integer.toString(numberLeft));
            mRecipientsEditor.setHint("Add up to " + Integer.toString(numberLeft) + " participants");
        }
        mStartCallButton.setText(R.string.button_add_participant);
    }

    private boolean isRecipientsEditorVisible() {
        return (null != mRecipientsEditor)
                    && (View.VISIBLE == mRecipientsEditor.getVisibility());
    }

    @Override
    public void onClick(View v) {
        if ((v == mRecipientsPicker)) {
            Log.e(TAG,"onClick, launchMultiplePhonePicker");
            launchMultiplePhonePicker();
        } else if (v == mStartCallButton) {
            Log.e(TAG,"onClick, StartCallButton pressed");
            startButtonPressed();
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                return true;
        }
        return false;
    }

    private static ContactList sEmptyContactList;
    private ContactList getRecipients() {
        if (isRecipientsEditorVisible()) {
            if (sEmptyContactList == null) {
                sEmptyContactList = new ContactList();
            }
            return sEmptyContactList;
        }
        return sEmptyContactList;
    }

    private final TextWatcher mRecipientsWatcher = new TextWatcher() {
        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {
            onUserInteraction();
        }

        @Override
        public void afterTextChanged(Editable s) {
            Log.v(TAG, "TextWatcher.afterTextChanged");
            if (!isRecipientsEditorVisible()) {
                IllegalStateException e = new IllegalStateException(
                        "afterTextChanged called with invisible mRecipientsEditor");
                Log.e(TAG,
                     "RecipientsWatcher: afterTextChanged called with invisible mRecipientsEditor");
                return;
            }

            if (mIsProcessPickedRecipients) {
                 return;
            }

            if (mRecipientsPickList != null) {
                // Update UI with mRecipientsPickList, which is picked from
                // People.
                updateRecipientNumber(mRecipientsPickList);
                mRecipientsPickList = null;
            } else {
                // If we have gone to zero recipients, we need to update the title.
                if (TextUtils.isEmpty(s.toString().trim())) {
                    ContactList contacts = mRecipientsEditor.constructContactsFromInput(false);
                    updateRecipientNumber(contacts);
                }

                // Walk backwards in the text box, skipping spaces. If the last
                // character is a comma, update the title bar.
                for (int pos = s.length() - 1; pos >= 0; pos--) {
                    char c = s.charAt(pos);
                    if (c == ' ') continue;

                    if (c == ',') {
                        ContactList contacts = mRecipientsEditor
                                .constructContactsFromInput(false);
                        updateRecipientNumber(contacts);
                    }

                    break;
                }
            }

            // If we have gone to zero recipients, disable send button.
            updateStartCallButtonState();
        }
    };

    private boolean checkForTooManyRecipients() {
        final int recipientLimit = RecipientLimit;
        if (recipientLimit != Integer.MAX_VALUE) {
            final int recipientCount = recipientCount();
            boolean tooMany = recipientCount > recipientLimit;
            if (tooMany) {
                String tooManyMsg = getString(R.string.too_many_recipients, recipientCount,
                        recipientLimit);
                Toast.makeText(ConferenceCallActivity.this,
                        tooManyMsg, Toast.LENGTH_SHORT).show();
                return true;
            }
        }
        return false;
    }

    private void processRepeatedRecipients(final ContactList list) {
        boolean repeatedRecipients = false;
        List<String> exsitNumbersList = mRecipientsEditor.getNumbers();
        int recipientCount = exsitNumbersList.size();
        final Set<String> tempNumbersSet = new HashSet<String>();
        tempNumbersSet.addAll(exsitNumbersList);
        exsitNumbersList.clear();
        exsitNumbersList.addAll(tempNumbersSet);
        if (recipientCount != exsitNumbersList.size()){
            //String exsitNumbers = mRecipientsEditor.getExsitNumbers();
            //Toast.makeText(this, "repeated recipients has been removed!", Toast.LENGTH_SHORT).show();
            final Handler handler = new Handler();
            final ProgressDialog progressDialog = new ProgressDialog(this);
            progressDialog.setTitle(getText(R.string.pick_too_many_recipients));
            progressDialog.setMessage(getText(R.string.adding_recipients));
            progressDialog.setIndeterminate(true);
            progressDialog.setCancelable(false);
            final Runnable showProgress = new Runnable() {
                @Override
                public void run() {
                }
            };
            handler.postDelayed(showProgress, 1000);
            new Thread(new Runnable() {
                @Override
                public void run() {
                    if (mRecipientsEditor != null) {
                        ContactList exsitList = mRecipientsEditor.constructContactsFromInput(true);
                        // Remove the repeat recipients.
                        if(exsitList.equals(list)){
                            exsitList.clear();
                            list.addAll(0, exsitList);
                        }else{
                            list.removeAll(exsitList);
                            list.addAll(0, exsitList);
                        }
                    }
                    // TODO: there is already code to update the contact header widget and recipients
                    // editor if the contacts change. we can re-use that code.
                    final Runnable populateWorker = new Runnable() {
                        @Override
                        public void run() {
                            mRecipientsEditor.removeTextChangedListener(mRecipientsWatcher);
                            mRecipientsEditor.populate(list);
                            mRecipientsPickList = list;
                            mRecipientsEditor.addTextChangedListener(mRecipientsWatcher);
                        }
                    };
                    handler.post(populateWorker);
                }
            }, "ConferenceCallActivity.processRepeatedRecipients").start();
        }
    }

    private int recipientCount() {
        int recipientCount;
        if (isRecipientsEditorVisible()) {
            recipientCount = mRecipientsEditor.getRecipientCount();
        } else {
            recipientCount = getRecipients().size();
        }
        return recipientCount;
    }

    private void launchMultiplePhonePicker() {
        Intent intent = new Intent("com.android.contacts.action.MULTI_PICK",Contacts.CONTENT_URI);
        String exsitNumbers = mRecipientsEditor.getExsitNumbers();
        Log.e(TAG,"launchMultiplePhonePicker, exsitNumbers = " + exsitNumbers);
        if (!TextUtils.isEmpty(exsitNumbers)) {
            intent.putExtra(Intents.EXTRA_PHONE_URIS, exsitNumbers);
        }
        try {
            mIsPickingContact = true;
            startActivityForResult(intent, REQUEST_CODE_PICK);
        } catch (ActivityNotFoundException ex) {
            Toast.makeText(this, R.string.contact_app_not_found, Toast.LENGTH_SHORT).show();
        }
    }

    private void startButtonPressed() {
        if (checkForTooManyRecipients()){
            return;
        }
        String exsitNumbers = mRecipientsEditor.getExsitNumbers();
        Log.d(TAG, "startButtonPressed, Numbers: " + exsitNumbers);
        exsitNumbers = formateNumbersFromContacts(exsitNumbers);
        Intent intent;
        if (!isAddParticipants) {
            intent = getDialConferenceCallIntent(exsitNumbers);
        } else {
            intent = getAddParticipantsCallIntent(currentParticipants, exsitNumbers);
            if (intent == null){
                Toast.makeText(this, "No new participant need to be added",
                        Toast.LENGTH_SHORT).show();
            }
        }
        hideInputMethod(getCurrentFocus());
        startActivity(intent);
        finish();
    }

    private String formateNumbersFromContacts(String exsitNumbers){
        String temp = exsitNumbers.replace("-", "");
        return temp;
    }

    private Intent getDialConferenceCallIntent(String exsitNumbers){
        String numbers = formConferenceUri(exsitNumbers);
        Log.d(TAG, "Dial ConferenceCall numbers: " + numbers);
        String[] mNumbers = numbers.split(";");
        String formatedNumbers = "";
        for (int k=0; k < mNumbers.length; k++){
            String tmp = (mNumbers[k].contains("tel:")
                    || mNumbers[k].contains("sip:")) ?
                    mNumbers[k] : ("tel:" + mNumbers[k]);
            formatedNumbers += tmp;
            if (k<mNumbers.length-1){
                formatedNumbers = formatedNumbers + ";";
            }
        }
        Intent dialintent = CallUtil.getCallIntent(CallUtil.getCallUri(formatedNumbers)
                , null, null);
        //dial conference add extra
        dialintent.putExtra(EXTRA_DIAL_CONFERENCE_URI, true);
        Log.d(TAG, "Dial ConferenceCall formatedNumbers: " + formatedNumbers);
        return dialintent;
    }

    private Intent getAddParticipantsCallIntent(String currentParticipants, String exsitNumbers){
        Log.e(TAG, "Add Participants exsitNumbers: " + exsitNumbers);
        String numbers = formConferenceUri(exsitNumbers);
        String[] currentParticipantsList = currentParticipants.split(";");
        for (int k=0; k < currentParticipantsList.length; k++){
            numbers = numbers.replace(currentParticipantsList[k]+";", "");
            numbers = numbers.replace(currentParticipantsList[k], "");
        }
        if (numbers != null) {
            Intent addparintent = CallUtil.getCallIntent(numbers);
            addparintent.putExtra(ADD_PARTICIPANT_KEY, true);
            Log.e(TAG, "Add Participants numbers: " + numbers);
            return addparintent;
        }
        return null;
    }

    private void hideInputMethod(View view) {
        final InputMethodManager imm = (InputMethodManager) getSystemService(
                Context.INPUT_METHOD_SERVICE);
        if (imm != null && view != null) {
            imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
        }
    }

    private String formConferenceUri(String exsitNumbers) {
        //Toast.makeText(this, exsitNumbers, Toast.LENGTH_LONG).show();
        String newString = exsitNumbers;
        if (exsitNumbers != null){
            String temp = exsitNumbers.replace("'","");
            newString = temp.replace(",",";");
        }
        return newString;
    }

    private void updateStartCallButtonState() {
        boolean enable = false;
        if (recipientCount() > 0){
            enable = true;
        }
        mStartCallButton.setEnabled(enable);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.v(TAG, "onActivityResult: requestCode=" + requestCode + ", resultCode=" + resultCode +
                 ", data=" + data);
        if (resultCode != RESULT_OK){
            Log.e(TAG, "bad!!, resultCode=" + resultCode);
            return;
        }
        switch (requestCode) {
            case REQUEST_CODE_PICK:
            if (data != null) {
                processPickResult(data);
            }
            break;
        default:
            Log.e(TAG, "unknown requestCode=" + requestCode);
        break;
        }
    }

    private void processPickResult(final Intent data) {
        Bundle bundle = data.getExtras().getBundle("result");
        final Set<String> keySet = bundle.keySet();
        Log.e(TAG, "ContactsResults: " + keySet);
        mIsProcessRecipientsFromIntent = false;
        final int recipientCount = (keySet != null) ? keySet.size() : 0;

        final int recipientLimit = 100;
        int totalRecipientsCount = mExistsRecipientsCount + recipientCount;
        if (recipientLimit != Integer.MAX_VALUE && totalRecipientsCount > recipientLimit) {
            new AlertDialog.Builder(this)
                    .setMessage(getString(R.string.too_many_recipients, totalRecipientsCount,
                            recipientLimit))
                    .setPositiveButton(android.R.string.ok, new OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            int newPickRecipientsCount = recipientLimit - mExistsRecipientsCount;
                            if (newPickRecipientsCount <= 0) {
                                return;
                            }
                            processAddRecipients(keySet, newPickRecipientsCount);
                        }
                    })
                    .setNegativeButton(android.R.string.cancel, null)
                    .create().show();
            return;
        }

        processAddRecipients(keySet, recipientCount);
    }

    private void processPickResult2(final Set<String> keySet) {
        Log.e(TAG, "processPickResult2: " + keySet);
        mIsProcessRecipientsFromIntent = true;
        final int recipientCount = (keySet != null) ? keySet.size() : 0;

        final int recipientLimit = 100;
        int totalRecipientsCount = mExistsRecipientsCount + recipientCount;
        if (recipientLimit != Integer.MAX_VALUE && totalRecipientsCount > recipientLimit) {
            new AlertDialog.Builder(this)
                    .setMessage(getString(R.string.too_many_recipients, totalRecipientsCount,
                            recipientLimit))
                    .setPositiveButton(android.R.string.ok, new OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            int newPickRecipientsCount = recipientLimit - mExistsRecipientsCount;
                            if (newPickRecipientsCount <= 0) {
                                return;
                            }
                            processAddRecipients(keySet, newPickRecipientsCount);
                        }
                    })
                    .setNegativeButton(android.R.string.cancel, null)
                    .create().show();
            return;
        }

        processAddRecipients(keySet, recipientCount);
    }

    public ContactList constructContactsFromInput2(List<String> numbers, boolean blocking) {
        ContactList list = new ContactList();
        for (String number : numbers) {
            Log.i(TAG, "numer = " + number);
            Log.i(TAG, "blocking = " + blocking);
            Contact contact = Contact.get(number, blocking);
            contact.setNumber(number);
            list.add(contact);
        }
        return list;
    }


    private void processAddRecipients(final Set<String> keySet, final int newPickRecipientsCount) {
        Log.e(TAG, "processAddRecipients, newPickRecipientsCount = "
                + newPickRecipientsCount);
        mIsProcessPickedRecipients = true;
        final Handler handler = new Handler();
        final ProgressDialog progressDialog = new ProgressDialog(this);
        progressDialog.setTitle(getText(R.string.pick_too_many_recipients));
        progressDialog.setMessage(getText(R.string.adding_recipients));
        progressDialog.setIndeterminate(true);
        progressDialog.setCancelable(false);

        final Runnable showProgress = new Runnable() {
            @Override
            public void run() {
                progressDialog.show();
            }
        };
        handler.postDelayed(showProgress, 1000);

        new Thread(new Runnable() {
            @Override
            public void run() {
                Uri[] newuris = new Uri[newPickRecipientsCount];
                List<String> list_string = new ArrayList<String>();
                final ContactList list;
                 try {
                    Iterator<String> it = keySet.iterator();
                    int i = 0;
                    while (it.hasNext()) {
                        String id = it.next();
                        Log.d(TAG, "buildUris id = " + id);
                        list_string.add(id);
                        if (!mIsProcessRecipientsFromIntent){
                            Uri tempUri = ContentUris.withAppendedId(Phone.CONTENT_URI, Integer.parseInt(id));
                            Log.d(TAG, "buildUris newUris = " + tempUri);
                            newuris[i++] = tempUri;
                        }
                        if (i == newPickRecipientsCount) {
                            break;
                        }
                    }
                    if (mIsProcessRecipientsFromIntent) {
                        list = constructContactsFromInput2(list_string, true);
                    } else {
                        list = ContactList.blockingGetByUris(newuris);
                    }
                } finally {
                    handler.removeCallbacks(showProgress);
                }
                if (mRecipientsEditor != null) {
                    Log.e(TAG, " mRecipientsEditor != null, list.size() = " + list.size());
                    ContactList exsitList = mRecipientsEditor.constructContactsFromInput(true);

                    // Remove the repeat recipients.
                    if(exsitList.equals(list)){
                        exsitList.clear();
                        list.addAll(0, exsitList);
                    }else{
                        list.removeAll(exsitList);
                        list.addAll(0, exsitList);
                    }
                }
                // TODO: there is already code to update the contact header widget and recipients
                // editor if the contacts change. we can re-use that code.
                final Runnable populateWorker = new Runnable() {
                    @Override
                    public void run() {
                        // We must remove this listener before dealing with the contact list.
                        // Because the listener will take a lot of time, this will cause an ANR.
                        mRecipientsEditor.removeTextChangedListener(mRecipientsWatcher);
                        mRecipientsEditor.populate(list);
                        // Set value for mRecipientsPickList and
                        // mRecipientsWatcher will update the UI.
                        mRecipientsPickList = list;
                        //updateRecipientNumber(list);

                        mRecipientsEditor.addTextChangedListener(mRecipientsWatcher);

                        // if process finished, then dismiss the progress dialog
                        progressDialog.dismiss();

                        // if populate finished, then recipients pick process end
                        mIsProcessPickedRecipients = false;
                    }
                };
                handler.post(populateWorker);
            }
        }, "ConferenceCallActivity.processPickResult").start();
    }

    private Uri[] buildUris(final Set<String> keySet, final int newPickRecipientsCount) {
        Uri[] newUris = new Uri[newPickRecipientsCount];
        Iterator<String> it = keySet.iterator();
        int i = 0;
        while (it.hasNext()) {
            String id = it.next();
            Log.d(TAG, "buildUris id = " + id);
            Uri tempUri = ContentUris.withAppendedId(Phone.CONTENT_URI, Integer.parseInt(id));
             Log.d(TAG, "buildUris newUris = " + tempUri);
            newUris[i++] = tempUri;
            if (i == newPickRecipientsCount) {
                break;
            }
        }
        return newUris;
    }

    //used to update the number for recipients that can be add to the conference call
    private void updateRecipientNumber(ContactList list) {
        Log.v(TAG, "updateRecipientNumber in");
        processRepeatedRecipients(list);
        Log.v(TAG, "updateRecipientNumber = " + list.size());
        mExistsRecipientsCount = list.size();
        mRecipientsNumber.setText(Integer.toString(RecipientLimit - mExistsRecipientsCount));
        if ((RecipientLimit -mExistsRecipientsCount)<0){
            mRecipientsNumber.setTextColor(0xFFC63131);
        } else {
            mRecipientsNumber.setTextColor(0xFF626262);
        }
    }
}
