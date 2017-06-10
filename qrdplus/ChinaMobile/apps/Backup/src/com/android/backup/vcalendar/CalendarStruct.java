/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */
/*
 * Copyright (C) 2007 The Android Open Source Project
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

package com.android.backup.vcalendar;

import java.util.List;
import java.util.ArrayList;

/**
 * Same comment as ContactStruct.
 */
public class CalendarStruct {

    public static class EventStruct {
        public String description;
        public String dtend;
        public String dtstart;
        public String duration;
        public String has_alarm;
        public String last_date;
        public String rrule;
        public String status;
        public String title;
        public String event_location;
        public String uid;
        // zhl add for meeting with telecom standard
        public String category;
        public String classfication;
        // add end
        public String allDay;

        public String timezone;
        public List<String> reminderList;
        public List<String> reminderTime;

        public void addReminderList(String method) {
            if (reminderList == null)
                reminderList = new ArrayList<String>();
            reminderList.add(method);
        }

        // zhl add
        public void addAlarmTimeList(String alarmTime) {
            if (reminderTime == null)
                reminderTime = new ArrayList<String>();
            reminderTime.add(alarmTime);
        }
        // add end
    }

    // zhl add for supporting vtodo analysis
    public static class TodoStruct {
        public String description;
        public String dtend;
        public String dtstart;
        public String duration;
        public String has_alarm;
        public String last_date;
        public String status;
        public String title;
        public String uid;
        public String category;
        public String priority;
        public String timezone;
        public String allDay;

        public List<String> reminderList;
        public List<String> reminderTime;

        public void addReminderList(String method) {
            if (reminderList == null)
                reminderList = new ArrayList<String>();
            reminderList.add(method);
        }

        public void addAlarmTimeList(String alarmTime) {
            if (reminderTime == null)
                reminderTime = new ArrayList<String>();
            reminderTime.add(alarmTime);
        }

    }

    // add end

    public String timezone;
    public List<EventStruct> eventList;

    // zhl add fot supporting vtodo
    public List<TodoStruct> todoList;

    public void addTodoList(TodoStruct stru) {
        if (todoList == null)
            todoList = new ArrayList<TodoStruct>();
        todoList.add(stru);
    }

    // add end
    public void addEventList(EventStruct stru) {
        if (eventList == null)
            eventList = new ArrayList<EventStruct>();
        eventList.add(stru);
    }
}
